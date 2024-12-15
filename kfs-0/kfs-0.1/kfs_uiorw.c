/* kfs_uiorw.c
 *
 * Copyright (c) 1993 by William W. Dorsey
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 1, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "kfs_int.h"
#include "crypt.h"


#ifndef MIN
#define MIN(a, b)		((a) < (b)? (a): (b))
#endif /* MIN */

#define CB_SIZ_LOG2		(12)				/* crypt block size */

#define CB_SIZ			(1 << CB_SIZ_LOG2)
#define CB_MASK			(CB_SIZ - 1)
#define CB_DOWN(x)		((x) & ~CB_MASK)
#define CB_UP(x)		CB_DOWN((x) + CB_MASK)
#define CB_REM(x)		((x) & CB_MASK)
#define CB_OH(x)		(((x) >> CB_SIZ_LOG2) << IV_SIZ_LOG2)

#define DES_BLK_LOG2		(3)				/* 8 byte DES crypt */
#define IV_SIZ_LOG2		DES_BLK_LOG2

#define DES_BLK			(1 << DES_BLK_LOG2)
#define DES_MSK			(DES_BLK - 1)
#define IV_SIZ			(1 << IV_SIZ_LOG2)			/* crypt IV */

#define CB_IOSIZ		(CB_SIZ + IV_SIZ)
#define OTW_MULT		(2)
#define OTW_MAXIO		(OTW_MULT * CB_IOSIZ)		/* otw block size (MUST be multiple) */
#define OTW_MAXSIZ		(OTW_MULT * CB_SIZ)

#define FHDR			sizeof(kfs_file_hdr)		/* maybe round up to next quad */

#if OTW_MAXIO > 8300
%%% error -- UDP packet size exceeded
#endif
#if CB_IOSIZ > OTW_MAXIO
%%% error
#endif


static void my_encrypt(long *out, int len, const long iv[2], const long *key_val);
static void my_decrypt(long *buf, int len, const long iv[2], const long *key_val);


/*
 * - read data specified by uio
 * - handle hidden IVs
 * - decypher as necessary
 * - we should consider if there is anything to be gained by moving data on
 *   word boundaries only
 *
 * - reads in blocks of at most OTW_MAXIO bytes
 * - assume OTW_MAXIO is a multiple of CB_IOSIZ
 * - assume IV is at end of data (&data[CB_SIZ])
 *
 * returns an errno
 */

int
kfs_uioread_cb(kfs_knode *f, struct uio *uiop, struct ucred *uc)
{
char	*buf;
int	rval;
u_long	bi, rlen, len, maxlen;
off_t	begin, end;
off_t	bbegin, bend, boffset;

	if( uiop->uio_offset >= f->fhdr.info.file_len.lo )					/* EOF */
		return 0;

	buf = kfs_xalloc(OTW_MAXIO);

	end =  uiop->uio_offset + uiop->uio_resid;
	bend = CB_UP(end);
	bend += f->key? CB_OH(bend): 0;

	rval = 0;
	while( uiop->uio_resid > 0 && uiop->uio_offset < f->fhdr.info.file_len.lo ) {
		begin = uiop->uio_offset;
		bbegin = CB_DOWN(begin) + (f->key? CB_OH(begin): 0);
		boffset = CB_REM(begin);
		maxlen = bend - bbegin;
		if( maxlen > OTW_MAXIO )
			maxlen = OTW_MAXIO;
		rlen = maxlen;
		if( (rval = kfs_vpread(k2rv(f), buf, &rlen, FHDR + bbegin, uc)) != 0 || rlen == 0 )	/* error or EOF */
			break;
		if( f->key ) {
			for( bi = 0; bi < rlen ; bi += CB_IOSIZ ) {
				if( bi + CB_IOSIZ <= rlen ) {
					my_decrypt((long *) &buf[bi], CB_SIZ, (long *) &buf[bi + CB_SIZ], f->key->key_val);
					len = MIN(bi + CB_SIZ - boffset, f->fhdr.info.file_len.lo - uiop->uio_offset);
					len = MIN(len, uiop->uio_resid);
					if( kfs_trace )
						printf("kfs_uioread_cb: %x[%x] len %lx\n", (int) bbegin, (int) boffset, len);
					if( (rval = uiomove(&buf[boffset], len, UIO_READ, uiop)) != 0 )
						goto done;
				} else {
					printf("kfs_uioread_cb: file \"%s\" too short (%lu > %lu)?\n",
											f->name, bi + CB_IOSIZ, rlen);
					rval = EIO;
					goto done;
				}
				if( uiop->uio_resid == 0 || uiop->uio_offset >= f->fhdr.info.file_len.lo )
					break;
				boffset += len + IV_SIZ;
			}
		} else {
			if( rlen < maxlen ) {
				bzero(&buf[rlen], maxlen - rlen);
				rlen = maxlen;
			}
			len = MIN(rlen - boffset, f->fhdr.info.file_len.lo - uiop->uio_offset);
			len = MIN(len, uiop->uio_resid);
			if( (rval = uiomove(&buf[boffset], len, UIO_READ, uiop)) != 0 )
				goto done;
		}
	}
done:
	kfs_free(buf, OTW_MAXIO);
	return rval;
}


/*
 *
 * - enhance this to put more than one CB in buf
 */

int
kfs_uiowrite_cb(kfs_knode *f, struct uio *uiop, struct ucred *uc)
{
char		*buf;
int		rval;
long		len, wcount;
off_t		begin, bbegin, boffset;

	buf = kfs_xalloc(CB_IOSIZ);

	/* end = uiop->uio_offset + uiop->uio_resid; */

	rval = 0;
	while( uiop->uio_resid > 0 ) {
		begin = uiop->uio_offset;
		bbegin = CB_DOWN(begin);
		boffset = CB_REM(begin);
		len = uiop->uio_resid;
		if( boffset != 0 || len < CB_SIZ ) {			/* gap at beginning || end of block */
			if( (rval = kfs_read_1cb(f, bbegin, (long *) buf, uc)) != 0 )
				break;
			wcount = MIN(CB_SIZ - boffset, len);
		} else
			wcount = CB_SIZ;
		/* wcount = len - boffset; */
		if( (rval = uiomove(&buf[boffset], wcount, UIO_WRITE, uiop)) != 0 )
			break;
		if( (rval = kfs_write_1cb(f, bbegin, (long *) buf, uc)) != 0 )
			break;
		if( uiop->uio_offset > f->fhdr.info.file_len.lo ) {
			f->fhdr.info.file_len.lo = uiop->uio_offset;
			f->fhdr_dirty = 1;
		}
	}

	if( buf )
		kfs_free(buf, CB_IOSIZ);
	return rval;
}


/*
 * reads one crypt block
 * - decrypts data in place after reading
 * - assumes FHDR is already added to offset
 * - adds CB_OH
 * - buffer must be CB_IOSIZ bytes
 * - only CB_SIZ bytes are returned valid
 * - assume buf and nested IV are long aligned
 */

int
kfs_read_1cb(kfs_knode *f, off_t offset, long *buf, struct ucred *uc)
{
long	count;
int	rval;
u_long	iosiz;

	if( f->key ) {
		offset += CB_OH(offset);
		iosiz = CB_IOSIZ;
	} else
		iosiz = CB_SIZ;

	count = iosiz;
	if( (rval = kfs_vpread(k2rv(f), buf, &count, FHDR + offset, uc)) != 0 )
		goto done;
	if( count < iosiz )
		bzero(&buf[count / sizeof(long)], iosiz - count);

	if( f->key )
		my_decrypt(buf, CB_SIZ, &buf[CB_SIZ / sizeof(buf[0])], f->key->key_val);

done:
	return rval;
}


/*
 * write one crypt block (CB_SIZ bytes)
 * - selects an IV
 * - crypts data in place before writing
 * - assumes FHDR is not added to offset
 * - adds CB_OH
 * - buffer must be CB_IOSIZ bytes
 * - assume buf and nested IV are long aligned
 * - offset should be crypt-block aligned
 */

int
kfs_write_1cb(kfs_knode *f, off_t offset, long *buf, struct ucred *uc)
{
struct timeval	tv;
u_long		seed[2];
int		rval;
u_long		iosiz;

	if( f->key ) {
		uniqtime(&tv);
		seed[0] = tv.tv_sec;
		seed[1] = tv.tv_usec;
		xfs_make_iv(f->key->key_val, (char *) &seed, &buf[CB_SIZ / sizeof(buf[0])]);
		my_encrypt(buf, CB_SIZ, &buf[CB_SIZ / sizeof(buf[0])], f->key->key_val);
		iosiz = CB_IOSIZ;
		offset += CB_OH(offset);
	} else
		iosiz = CB_SIZ;

	if( (rval = kfs_vpwrite(k2rv(f), buf, iosiz, FHDR + offset, uc)) != 0 )
		goto done;
done:
	return rval;
}


/*
 * read a segment of data
 * break segment into OTW_MAXIO chunks if necessary
 *
 * return count of bytes read in *max
 * func returns an errno
 */

int
kfs_vpread(struct vnode *vp, void *vbuf, long *max, off_t offset, struct ucred *uc)
{
struct uio	uio;
struct iovec	iov;
int		rval;
u_long		r;

	rval = 0;
	iov.iov_base = vbuf;
	iov.iov_len = *max;
	uio.uio_iov = &iov;
	uio.uio_iovcnt = 1;
	uio.uio_offset = offset;
	uio.uio_segflg = UIO_SYSSPACE;
	uio.uio_fmode = 0;
	uio.uio_resid = *max;

	while( uio.uio_resid > 0 ) {
		r = uio.uio_resid;
		if( (rval = VOP_RDWR(vp, &uio, UIO_READ, 0, uc)) != 0 )
			break;
		if( r == uio.uio_resid )			/* EOF */
			break;
	}
	*max -= uio.uio_resid;
	return rval;
}


/*
 * write an arbitrary segment of data (compliment of kfs_vpread)
 */

int
kfs_vpwrite(struct vnode *vp, const void *vbuf, long resid, off_t offset, struct ucred *uc)
{
struct uio	uio;
struct iovec	iov;
int		rval;

	rval = 0;
	iov.iov_base = (void *) vbuf;
	iov.iov_len = resid;
	uio.uio_iov = &iov;
	uio.uio_iovcnt = 1;
	uio.uio_offset = offset;
	uio.uio_segflg = UIO_SYSSPACE;
	uio.uio_fmode = 0;
	uio.uio_resid = resid;

	while( uio.uio_resid > 0 ) {
		if( (rval = VOP_RDWR(vp, &uio, UIO_WRITE, 0, uc)) != 0 )
			return rval;
	}
	return rval;
}


/* *** */


static void
my_encrypt(long *buf, int len, const long iv[2], const long *key_val)
{
	if( kfs_trace )
		printf("my_encrypt($%x, %d, $%x, $%x)\n", (int) buf, len, (int) iv, (int) key_val);

	if( (long) buf & 3 || (long) iv & 3 || len & 7 )
		printf("my_encrypt: oops\n");
	else {
		xfs_encrypt(key_val, iv, buf, buf, len);
	}
}


static void
my_decrypt(long *buf, int len, const long iv[2], const long *key_val)
{
long	t;

	if( kfs_trace )
		printf("my_decrypt($%x, %d, $%x, $%x)\n", (int) buf, len, (int) iv, (int) key_val);

	if( (long) buf & 3 || (long) iv & 3 || len & 7 )
		printf("my_decrypt: oops\n");
	else {
		t = iv[2];
		if( t != iv[2] )
			printf("my_decrypt: pre-test failed!\n");
		xfs_decrypt(key_val, iv, buf, buf, len);
		if( t != iv[2] )
			printf("my_decrypt: corruption at limit\n");
	}
}
