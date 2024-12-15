/*
 *                                        _
 * Copyright (c) 1990,1991,1992,1993 Stig Ostholm.
 * All Rights Reserved
 *
 *
 * The author takes no responsibility of actions caused by the use of this
 * software and does not guarantee the correctness of the functions.
 *
 * This software may be freely distributed and modified for non-commercial use
 * as long as the copyright notice is kept. If you modify any of the files,
 * pleas add a comment indicating what is modified and who made the
 * modification.
 *
 * If you intend to use this software for commercial purposes, contact the
 * author.
 *
 * If you find any bugs or porting problems, please inform the author about
 * the problem and fixes (if there are any).
 *
 *
 * Additional restrictions due to national laws governing the use, import or
 * export of cryptographic software is the responsibility of the software user,
 * importer or exporter to follow.
 *
 *
 *                                              _
 *                                         Stig Ostholm
 *                                         Chalmers University of Technology
 *                                         Department of Computer Engineering
 *                                         S-412 96 Gothenburg
 *                                         Sweden
 *                                         ----------------------------------
 *                                         Email: ostholm@ce.chalmers.se
 *                                         Phone: +46 31 772 1703
 *                                         Fax:   +46 31 772 3663
 */

#include	<stdio.h>
#include	<string.h>
#include	<ctype.h>
#include	<des.h>
#include	"filekey.h"
#include	"strutil.h"
#include	"config.h"


typedef int	(*io_func)(
#ifdef __STDC__
			char *,
			unsigned,
			unsigned,
			FILE *
#endif
);

typedef struct	io_mode {
			char	*rname;
			char	*wname;
			FILE	*rfd;
			FILE	*wfd;
			io_func	read;
			io_func	write;
		} io_mode;


typedef struct	crypt_mode;

typedef int	(*crypt_func)(
#ifdef __STDC__
			des_cblock	 *input,
			des_cblock	 *output,
			int		 length,
			des_key_schedule schedule,
			des_cblock	 *ivec,
			int		 encrypt
#endif
);

typedef int	(*file_func)(
#ifdef __STDC__
			crypt_mode		*crypt,
			des_key_schedule	schedule,
			des_cblock		*ivec,
			int			mode
			io_mode			*io
#endif
);

typedef	struct	crypt_mode {
			char		*name;
			crypt_func	func;
			file_func	file;
		} crypt_mode;





char	*prog;

#define a_FLAG	0x0001L
#ifdef DES_STRING_MODE_CTH
# define b_FLAG	0x0002L
#endif /* DES_STRING_MODE_CTH */
#define c_FLAG	0x0004L
#define d_FLAG	0x0008L
#define e_FLAG	0x0010L
#define f_FLAG	0x0020L
#define h_FLAG	0x0040L
#define k_FLAG	0x0080L
#define m_FLAG	0x0100L
unsigned long	flags = 0L;

/*
 * Encryption/decryption functions.
 *
 */

#define BUF_SIZE 8192

/*
 * This routine does the encryption/decryption job without padding.
 */

static int	crypt_file_plain(
#ifdef __STDC__
	crypt_mode		*crypt,
	des_key_schedule	schedule,
	des_cblock		*ivec,
	int			mode,
	io_mode			*io)
#else
	crypt, schedule, ivec, mode, io)
crypt_mode		*crypt;
des_key_schedule	schedule;
des_cblock		*ivec;
int			mode;
io_mode			*io;
#endif
{
	register int	n;
	char		buf[BUF_SIZE];


	/* Do the encryption on compleat blocks */
	while ((n = fread(buf, sizeof(*buf), BUF_SIZE, io->rfd)) > 0) {

		(*crypt->func)((des_cblock *) buf, (des_cblock *) buf,
				n, schedule, ivec, mode); 
		
		VOID fwrite(buf, sizeof(*buf), n, io->wfd);
		if (ferror(io->wfd))
			return 1;
	}

	return 0;
}

/*
 * Bitmask for bits contaning padding information
 */

/* 0x00001000 - 0x00000001 -> 0x00000111 (8 - 1 -> 7) */
#define PAD_INFO_BITS (DES_BLOCK_BYTES - 1)

static int	crypt_file_padding(
#ifdef __STDC__
	crypt_mode		*crypt,
	des_key_schedule	schedule,
	des_cblock		*ivec,
	int			mode,
	io_mode			*io)
#else
	crypt, schedule, ivec, mode, io)
crypt_mode		*crypt;
des_key_schedule	schedule;
des_cblock		*ivec;
int			mode;
io_mode			*io;
#endif
{
	register int		i, j, nr, nw, corrupt;
	register unsigned int	pad;
	register unsigned char	pad_char;
	char			ibuf[BUF_SIZE + DES_BLOCK_BYTES];
	char			obuf[BUF_SIZE];
	des_cblock		pbuf;


	corrupt = 0;

	if (mode == DES_ENCRYPT) {

		/* Do the encryption on compleat blocks */
		while ((nr = fread(ibuf, sizeof(*ibuf), BUF_SIZE, io->rfd)) ==
			BUF_SIZE) {

			(*crypt->func)((des_cblock *) ibuf, (des_cblock *) obuf,
					nr, schedule, ivec, DES_ENCRYPT); 
		
			VOID (*io->write)(obuf, sizeof(*obuf), nr, io->wfd);
			if (ferror(io->wfd))
				return 1;
		}

		/* The last section handles padding to ensure that the	*/
		/* encryption is made on compleat eight byte block(s).	*/
		/* The last block will always contain 0 .. 7 bytes of	*/
		/* data, and the last byte contains a number indicating	*/
		/* how may bytes (0 .. 7) that are use for actual data.	*/
		/* Any remaining bits between data bits and the last is */
		/* padded with random bits.				*/
		pad = nr % DES_BLOCK_BYTES;
		nw = nr + (DES_BLOCK_BYTES - pad);
		VOID des_random_cblock((des_cblock *) pbuf);
		pad_char = (unsigned char) pbuf[DES_BLOCK_BYTES - 1];
		pad_char &= (unsigned char) ((~PAD_INFO_BITS) & 0xff);
		pad_char |= (unsigned char) pad;
		pbuf[DES_BLOCK_BYTES - 1] = (char) pad_char;
		for (i = nr, j = pad; i < nw; i++, j++)
			ibuf[i] = pbuf[j];

		(*crypt->func)((des_cblock *) ibuf, (des_cblock *) obuf, nw,
				schedule, ivec, DES_ENCRYPT); 

		VOID (*io->write)(obuf, sizeof(*obuf), nw, io->wfd);

	} else {

		/* Do the decryption on the source file */

		/* Read the first block, return with error if the file	*/
		/* has 0 size.						*/
		if (!(nw = nr = (*io->read)(ibuf, sizeof(*ibuf), BUF_SIZE,
		    io->rfd)))
			return 1;

		/* Decrypt the the current input buffer into the output	*/
		/* buffer.						*/
		VOID (*crypt->func)((des_cblock *) ibuf, (des_cblock *) obuf,
				     nr, schedule, ivec, DES_DECRYPT); 

		/* Read until end of file is found */
		while ((nr = (*io->read)(ibuf, sizeof(*ibuf), BUF_SIZE, io->rfd))
			> 0) {

			/* Write the decrypted contentse of the		*/
			/* previous buffer.				*/
			VOID fwrite(obuf, sizeof(*obuf), nw, io->wfd);
			if (ferror(io->wfd))
				return 0;

			nw = nr;

			/* Decrypt the the current input buffer into	*/
			/* the output buffer.				*/
			VOID (*crypt->func)((des_cblock *) ibuf,
					    (des_cblock *) obuf, nr, schedule,
					    ivec, DES_DECRYPT); 

		}

		/* The last buffer contains padding information that	*/
		/* has to be removed. The buffer must contain at least	*/
		/* on block.						*/
		if (nw >= DES_BLOCK_BYTES) {
			/* The last block must contain an multiple of	*/
			/* cblocks.					*/
			corrupt = nw % DES_BLOCK_BYTES;
			/* The last byte in the block contains padding	*/
			/* infomration. Remove the padding bytes and	*/
			/* the padding info.				*/
			pad = (unsigned int) obuf[nw - 1];
			pad &= PAD_INFO_BITS;
			nw -= (DES_BLOCK_BYTES - pad);
		} else
			corrupt = 1;

		/* Write the last output buffer with padding		*/
		/* information removed.					*/
		VOID fwrite(obuf, sizeof(*obuf), nw, io->wfd);

	}

	return corrupt;
}


static const crypt_mode	crypt_list[] = {
				{
					"CBC",
					(crypt_func) des_cbc_encrypt,
					(file_func) crypt_file_padding
				},
				{
					"ECB",
					(crypt_func) des_ecb2_encrypt,
					(file_func) crypt_file_padding
				},
				{
					"CFB8",
					(crypt_func) des_cfb8_encrypt,
					(file_func) crypt_file_plain
				},
				{
					"OFB8",
					(crypt_func) des_ofb8_encrypt,
					(file_func) crypt_file_plain
				},
				{
					"PCBC",
					(crypt_func) des_pcbc_encrypt,
					(file_func) crypt_file_padding
				},
				{
					(char *) 0,
					(crypt_func) 0,
					(file_func) 0
				}
			};


/*
 * Compute a 64-bit cryptographic checksum on the file.
 */

static int	crypt_file_checksum(
#ifdef __STDC__
	crypt_mode		*crypt,
	des_key_schedule	schedule,
	des_cblock		*ivec,
	int			mode,
	io_mode			*io)
#else
	crypt, schedule, ivec, mode, io)
crypt_mode		*crypt;
des_key_schedule	schedule;
des_cblock		*ivec;
int			mode;
io_mode			*io;
#endif
{
	register int	n;
	char		buf[BUF_SIZE + DES_BLOCK_BYTES];
	des_cblock 	sum;


	while ((n = fread(buf, sizeof(*buf), BUF_SIZE, io->rfd)) > 0)
		VOID des_cbc_cksum((des_cblock *) buf, (des_cblock *) sum,
				   n, schedule, ivec);

	VOID (*io->write)((char *) sum, sizeof(*sum), sizeof(sum), io->wfd);

	return 0;
}


static const crypt_mode	crypt_chksum = {
				"CBC CHKSUM",
				(crypt_func) 0,
				(file_func) crypt_file_checksum
			};


/*
 * ASCII input/output functions
 */

int	ascii_read(
#ifdef __STDC__
	char *ptr, unsigned ptr_size, unsigned items, FILE *fd)
#else
	ptr, ptr_size, items, fd)
char		*ptr;
unsigned	ptr_size;
unsigned	items;
FILE		*fd;
#endif
{
	register int	n, l, r;
	char		buf[BUF_SIZE];

	
	n = ptr_size * items;
	r = 0;
	while (n >= DES_BLOCK_BYTES) {
		if (fgets(buf, BUF_SIZE, fd) == NULL)
			break;
		l = strlen(buf) - 1;
		if (buf[l] != '\n')
			continue;
		buf[l] = '\0';
		if (des_hex_to_cblock(buf, (des_cblock *) ptr))
			continue;
		r += DES_BLOCK_BYTES;
		ptr += DES_BLOCK_BYTES;
		n -= DES_BLOCK_BYTES;
	}
	return r / ptr_size;
}

int	ascii_write(
#ifdef __STDC__
	char *ptr, unsigned ptr_size, unsigned items, FILE *fd)
#else
	ptr, ptr_size, items, fd)
char		*ptr;
unsigned	ptr_size;
unsigned	items;
FILE		*fd;
#endif
{
	register int	n;
	des_cblock	*b;
	FILE		*old_fd;
	extern FILE	*des_print_file; 

	
	old_fd = des_print_file;
	des_print_file = fd;
	b = (des_cblock *) ptr;
	n = ptr_size * items;
	n = des_print_cblock(b, n / DES_BLOCK_BYTES);
	des_print_file = old_fd;
	return n * DES_BLOCK_BYTES;
}

/*
 * Get the encryption/decryption key.
 */

int	getkey(
#ifdef __STDC__
	des_cblock	*key,
	char		*key_spec)
#else
	key, key_spec)
des_cblock	*key;
char		*key_spec;
#endif
{
	if (flags & k_FLAG) {
		if (flags & h_FLAG) {
			if (des_hex_to_cblock(key_spec, key)) {
				fprintf(stderr, "key is not a 64 bit hexadecimal number\n");
				goto error;
			}
			VOID des_fixup_key_parity(key);
		} else 
			VOID des_string_to_key(key_spec, *key);
	} else if (flags & f_FLAG) {
		if (read_key(key, key_spec, (flags & h_FLAG))) {
			fprintf(stderr, "Could not read key from \"%s\"\n",
				key_spec);
			goto error;
		}
	} else {
		if ((flags & h_FLAG) ?
		    des_read_hexkey(key, "Hexkey: ", 1) :
		    des_read_password(key, "Key: ", 1))
			goto error;
	}
	
	return 0;

error:
	return 1;
}


crypt_mode	*get_crypt_mode(
#ifdef __STDC__
	char	*name)
#else
	name)
char	*name;
#endif
{
	register int	i;


	for (i = 0; name[i]; i++)
		if (islower((int) name[i]))
			name[i] = (char) toupper((int) name[i]);
	for (i = 0; crypt_list[i].name; i++)
		if (!strcmp(name, crypt_list[i].name))
			return &crypt_list[i];

	return (crypt_mode *) 0;
}


main(argc, argv)
int	argc;
char	*argv[];
{
	register int		o, cr, mode;
	register char		*crypt_name, *key_spec;
	register crypt_mode	*crypt;
	register file_func	file = (file_func) 0;
	io_mode			io;
	des_key_schedule	schedule;
	des_cblock		key, ivec;
	extern int		optind;
	extern char		*optarg;


	/* The program name. */
	prog = (prog = strrchr(*argv, '/')) ? (prog + 1) : *argv;

	/* Set initial values. */
	cr = 0;
	crypt = &crypt_list[0];
	mode = DES_ENCRYPT;
	crypt_name = key_spec = NULL;
	io.rname = "<stdin>";
	io.wname = "<stdout>";
	io.rfd = stdin;
	io.wfd = stdout;
	io.read = (io_func) fread;
	io.write = (io_func) fwrite;
	DES_ZERO_CBLOCK(ivec);
	des_return_ivec = DES_RETURN_IVEC;

	/* Get the command line arguments. */
#ifdef b_FLAG
	while ((o = getopt(argc, argv, "abcdef:hk:m:")) != EOF)
#else  /* b_FLAG */
	while ((o = getopt(argc, argv, "acdef:hk:m:")) != EOF)
#endif /* b_FLAG */
		switch (o) {
		case 'a':
			if (flags & a_FLAG)
				goto usage;
			io.read = (io_func) ascii_read;
			io.write = (io_func) ascii_write;
			/* Ascii input/output requires padding */
			file = (file_func) crypt_file_padding;
			flags |= a_FLAG;
			break;
#ifdef b_FLAG
		case 'b':
			if (flags & (b_FLAG | h_FLAG))
				goto usage;
			VOID des_set_string_mode(DES_STRING_MODE_CTH);
			flags |= b_FLAG;
			break;
#endif /* b_FLAG */
		case 'c':
			if (flags & (c_FLAG | d_FLAG | e_FLAG | m_FLAG))
				goto usage;
			crypt = &crypt_chksum;
			flags |= c_FLAG;
			break;
		case 'd':
			if (flags & (c_FLAG | d_FLAG | e_FLAG))
				goto usage;
			mode = DES_DECRYPT;
			flags |= d_FLAG;
			break;
		case 'e':
			if (flags & (c_FLAG | d_FLAG | e_FLAG))
				goto usage;
			mode = DES_ENCRYPT;
			flags |= e_FLAG;
			break;
		case 'f':
			if (flags & (f_FLAG | k_FLAG))
				goto usage;
			if ((key_spec = new_string(optarg)) == NULL)
				goto memalloc;
			/* Destroy the argument vector key. */
			null_string(optarg);
			flags |= f_FLAG;
			break;
		case 'h':
#ifdef b_FLAG
			if (flags & (h_FLAG | b_FLAG))
#else  /* b_FLAG */
			if (flags & h_FLAG)
#endif /* b_FLAG */
				goto usage;
			flags |= h_FLAG;
			break;
		case 'k':
			if (flags & (f_FLAG | k_FLAG))
				goto usage;
			if ((key_spec = new_string(optarg)) == NULL)
				goto memalloc;
			/* Destroy the argument vector key. */
			null_string(optarg);
			flags |= k_FLAG;
			break;
		case 'm':
			if (flags & (c_FLAG | m_FLAG))
				goto usage;
			if ((crypt_name = new_string(optarg)) == NULL)
				goto memalloc;
			if (!(crypt = get_crypt_mode(crypt_name)))
				goto nomode;
			free(crypt_name);
			crypt_name = NULL;
			flags |= m_FLAG;
			break;
		case '?':
		default:
			goto usage;
		}
	if (!(flags & (c_FLAG | d_FLAG | e_FLAG)))
		goto usage;

	switch (argc - optind) {
	case 2:
		io.wname = argv[optind + 1];
		io.wfd = NULL;
	case 1:
		io.rname = argv[optind];
		io.rfd = NULL;
	case 0:
		break;
	default:
		goto usage;
	}

	/* If there was no key set on the argument line, fetch one from	*/
	/* the tty.							*/
	if (getkey((des_cblock *) key, key_spec))
		goto nokey;
	if (flags & (k_FLAG | f_FLAG)) {
		null_string(key_spec);
		free(key_spec);
	}

	/* Make a key schedule */
	if (des_key_sched((des_cblock *) key, schedule) < 0)
		goto weakkey;
	DES_ZERO_CBLOCK(key);

	/* Open the file(s) */
	if (io.rfd == NULL) {
		/* Open the source file. */
		if ((io.rfd = fopen(io.rname , "r")) == NULL)
			goto rfilerr;
		clearerr(io.rfd);
	}
	if (io.wfd == NULL) {
		/* Open the destination file. */
		if ((io.wfd = fopen(io.wname , "w")) == NULL)
			goto wfilerr;
		clearerr(io.wfd);
	}

	/* Encrypt/decrypt the file */
	/* set the correct file crypt function */
	cr = (*(file ? file : crypt->file))(crypt, schedule, (des_cblock *) ivec, mode, &io);

	/* Was there any error during read or write ? */
	if (ferror(io.rfd))
		goto rfilerr;
	if (ferror(io.wfd))
		goto wfilerr;

	/* No check is neccessary here */
	VOID fclose(io.rfd);
	io.rfd = NULL;

	/* This check is neccessary when dealing with AFS. */
	if (fclose(io.wfd) == EOF) {
		io.wfd = NULL;
		goto wfilerr;
	} else
		io.wfd = NULL;

	/* Was the encrypted file corrupt ? */
	if (cr)
		goto corrupt;

	exit(0);

	/* Error handling section */
usage:
#ifdef b_FLAG
	VOID fprintf(stderr, "Usage: %s -e|-d|-c [-h|-b] [-k strkey|-f keyfile] [-m func] [-a] [-t] [infile [outfile]]\n", prog);
#else  /* b_FLAG */
	VOID fprintf(stderr, "Usage: %s -e|-d|-c [-h] [-k strkey|-f keyfile] [-m func] [-a] [-t] [infile [outfile]]\n", prog);
#endif /* b_FLAG */
	exit(1);

memalloc:
	fputs(prog, stderr);
	perror(": Could not allocate memory, ");
	exit(1);

rfilerr:
	VOID fprintf(stderr, "%s: Open/read error on \"%s\"", prog, io.rname);
	perror(", ");
	goto close_file;

wfilerr:
	VOID fprintf(stderr, "%s: Open/read error on \"%s\"", prog, io.wname);
	perror(", ");
	goto close_file;

weakkey:
	VOID fprintf(stderr, "%s: The key generated is weak\n", prog);
	exit(1);

nomode:
	VOID fprintf(stderr, "%s: No crypt mode \"%s\" available.\n", prog, crypt_name);
	free(crypt_name);
	exit(1);

nokey:
	VOID fprintf(stderr, "%s: Can not obtain key\n", prog);
	exit(1);

corrupt:
	VOID fprintf(stderr, "%s: Encrypted file \"%s\" is corrupt\n", prog, io.rname);
#ifdef b_FLAG
	if (!(flags & (b_FLAG | h_FLAG)))
		VOID fprintf(stderr, "%s: Try using the `-b' option\n", prog);
#endif /* b_FLAG */
	exit(1);

close_file:
	if (io.rfd)
		VOID fclose(io.rfd);
	if (io.wfd)
		VOID fclose(io.wfd);
	exit(1);
}
