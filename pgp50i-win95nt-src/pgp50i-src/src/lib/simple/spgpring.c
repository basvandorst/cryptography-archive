/*
* spgpring.c -- Simple PGP API helper -- Keyring access functions
*
* Copyright (C) 1996,1997 Pretty Good Privacy, Inc. All rights reserved.
*
* $Id: spgpring.c,v 1.25.2.3 1997/06/07 09:51:56 mhw Exp $
*/

#if HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>	/* For unlink() */
#endif

#ifdef HAVE_FCNTL_H
#include <fcntl.h>		/* For O_CREAT, O_EXCL */
#endif

#include "pgpDebug.h"
#include "spgp.h"
#include "spgpint.h"

#include "pgpKeyDB.h"
#include "pgpFileNames.h"
#include "pgpFileRef.h"

/*
* Open the default or specified keyring file(s).
* If only want one, leave pfXXX, rfXXX and rsXXX all null.
* If filenames are null or empty strings, uses default from env.
*/
int
spgpOpenRingfiles (PgpEnv *env, RingPool *ringpool,
	PGPFileRef *PublicKeyRingRef, PGPFileRef *PrivateKeyRingRef,
	PgpFile **pfpub, RingFile **rfpub, RingSet const **rspub,
	PgpFile **pfsec, RingFile **rfsec, RingSet const **rssec,
	Boolean writable)
{
	int					err = 0;
	PGPFileRef *		pubRingRef = NULL;
	PGPFileRef *		secRingRef = NULL;
	FILE *				pubFile = NULL;
	FILE *				secFile = NULL;
	PGPFileOpenFlags	openFlags;
	
	(void)env;

	openFlags = writable ? kPGPFileOpenReadWritePerm : kPGPFileOpenReadPerm;
	
	if (pgpGetKeyRingFileRefs (&pubRingRef, &secRingRef) != PGPERR_OK)
		return SIMPLEPGP_ERRORREADINGPUBRING;

	if (pfpub != NULL)
	{
		*pfpub = NULL;
		*rfpub = NULL;
		*rspub = NULL;
	}
	if (pfsec != NULL)
	{
		*pfsec = NULL;
		*rfsec = NULL;
		*rssec = NULL;
	}
	
	if (pfpub != NULL)
	{
		if (PublicKeyRingRef)
		{
			pgpFreeFileRef(pubRingRef);
			pubRingRef = pgpCopyFileRef (PublicKeyRingRef);
			if (pubRingRef == NULL)
			{
				err = KERNEL_OUT_OF_MEM;
				goto error;
			}
		}
		pubFile = pgpFileRefStdIOOpen(pubRingRef, openFlags,
									kPGPFileTypeNone, &err);
		if (pubFile == NULL)
		{
			if (err == PGPERR_FILE_WRITELOCKED)
			{
				err = SIMPLEPGP_KEYRINGWRITELOCKED;
				goto error;
			}
			else if (err == PGPERR_FILE_PERMISSIONS)
			{
				err = SIMPLEPGP_KEYRINGPERMISSIONS;
				goto error;
			}
		}
		*pfpub = pgpFileReadOpen (pubFile, NULL, NULL);
		if (*pfpub == NULL)
		{
			err = SIMPLEPGP_ERRORREADINGPUBRING;
			goto error;
		}
		*rfpub = ringFileOpen (ringpool, *pfpub, 1, &err);
		if (*rfpub == NULL)
		{
			err = SIMPLEPGP_ERRORREADINGPUBRING;
			goto error;
		}
		*rspub = ringFileSet(*rfpub);
	}

	if (pfsec != NULL)
	{
		if (PrivateKeyRingRef)
		{
			pgpFreeFileRef(secRingRef);
			secRingRef = pgpCopyFileRef (PrivateKeyRingRef);
			if (secRingRef == NULL)
			{
				err = KERNEL_OUT_OF_MEM;
				goto error;
			}
		}
		secFile = pgpFileRefStdIOOpen(secRingRef, openFlags,
									kPGPFileTypeNone, &err);
		if (secFile == NULL)
		{
			if (err == PGPERR_FILE_WRITELOCKED)
			{
				err = SIMPLEPGP_KEYRINGWRITELOCKED;
				goto error;
			}
			else if (err == PGPERR_FILE_PERMISSIONS)
			{
				err = SIMPLEPGP_KEYRINGPERMISSIONS;
				goto error;
			}
		}
		*pfsec = pgpFileReadOpen (secFile, NULL, NULL);
		if (*pfsec == NULL)
		{
			err = SIMPLEPGP_ERRORREADINGSECRING;
			goto error;
		}
		*rfsec = ringFileOpen (ringpool, *pfsec, 1, &err);
		if (*rfsec == NULL)
		{
			err = SIMPLEPGP_ERRORREADINGSECRING;
			goto error;
		}
		*rssec = ringFileSet(*rfsec);
	}
	err = 0;
	goto done;
	
error:
	if (pfpub != NULL)
	{
		if (*rfpub != NULL)
			spgpRingFileClose(*rfpub);
		if (*pfpub != NULL)
			pgpFileClose(*pfpub);
		if (pubFile != NULL)
			pgpStdIOClose(pubFile);
		*rspub = NULL;
		*rfpub = NULL;
		*pfpub = NULL;
	}
	if (pfsec != NULL)
	{
		if (*rfsec != NULL)
			spgpRingFileClose(*rfsec);
		if (*pfsec != NULL)
			pgpFileClose(*pfsec);
		if (secFile != NULL)
			pgpStdIOClose(secFile);
		*rssec = NULL;
		*rfsec = NULL;
		*pfsec = NULL;
	}
done:
	if (pubRingRef != NULL)
		pgpFreeFileRef(pubRingRef);
	if (secRingRef != NULL)
		pgpFreeFileRef(secRingRef);
	return err;
}


/*
* Create a temp key file which is not used by anyone else.
* Return a stream pointer to the file, and set the file reference in
* *tmpFileRef1. Base the file reference on the given file.
* The returned file reference needs to be free'd with pgpFreeFileRef, and of
* course the returned file handle needs to be closed too.
* The implementation of this tends to be OS (and possibly file system)
* dependent.
*/
static FILE *
tmpkeyfile (PGPFileRef const *fileRef, PGPFileRef **tmpFileRef1,
			PGPFileOpenFlags flags, Boolean isPrivate)
{
	char *				file = NULL;
	char *				tmp = NULL;
	char *				nfile = NULL;
	char *				nfilep = NULL;
	PGPFileRef *		tmpFileRef = NULL;
	FILE *				stdFile = NULL;
	
	if ((file = pgpGetFileRefName(fileRef)) == NULL)
		goto error;
	if ((tmp = pgpFileNameContract(file)) == NULL)
		goto error;
	if ((nfile = pgpFileNameExtend(tmp, ".pg0")) == NULL)
		goto error;
	nfilep = nfile + strlen(file) - 1;
	if ((tmpFileRef = pgpCopyFileRef(fileRef)) == NULL)
		goto error;
	for ( ; ; ) {
		if ((pgpSetFileRefName(&tmpFileRef, nfile)) != PGPERR_OK)
			goto error;
		if ((stdFile = pgpFileRefStdIOOpen(tmpFileRef, flags,
							isPrivate ? kPGPFileTypePrivRing
								: kPGPFileTypePubRing, NULL)) != NULL)
			break;
		if ((*nfilep)++ > 'z')
			goto error;
	}
error:
	pgpMemFree(file);
	pgpMemFree(tmp);
	pgpMemFree(nfile);
	if (stdFile != NULL)
		*tmpFileRef1 = tmpFileRef;
	else if (tmpFileRef != NULL)
		pgpFreeFileRef(tmpFileRef);
	return stdFile;
}

/*
* This writes out a RingSet which is a replacement for a RingFile
* which is open. We do it carefully since on some OS's we can't
* rename open files. This closes the RingFile and the RingSet
* in the process, so the RingSet must be one for
* which this is legal (not from ringFileSet for example).
* Renames existing file.ext to file.bak.
*/
PGPError
spgpWriteoutSet (PgpEnv *env, PGPFileRef *fileRef, RingSet *set,
	RingFile *oldrfile, PgpFile *pfp, Boolean isPrivate)
{
	struct PgpFile *pfp2;
	char *tmp = NULL, *bak = NULL;
	char *fileName = NULL;
	PGPFileRef *tmpFileRef = NULL;
	PGPFileRef *bakFileRef = NULL;
	FILE *fp2;
	PGPError result = PGPERR_OK;

	(void)env;

	if (fileRef)
		fileRef = pgpCopyFileRef (fileRef);	/* so can free it below */
	else if ((result = pgpGetKeyRingFileRefs(&fileRef, NULL)) != PGPERR_OK)
		goto error;

	if ((fileName = pgpGetFileRefName(fileRef)) == NULL)
		goto memError;
	if ((tmp = pgpFileNameContract(fileName)) == NULL)
		goto memError;
#if MACINTOSH
	if ((bak = pgpFileNameExtend(tmp, " (backup)")) == NULL)
		goto memError;
#else
	if ((bak = pgpFileNameExtend(tmp, ".bak")) == NULL)
		goto memError;
#endif
	pgpMemFree(tmp);
	tmp = NULL;
	if ((bakFileRef = pgpCopyFileRef(fileRef)) == NULL)
		goto memError;
	if ((result = pgpSetFileRefName(&bakFileRef, bak)) != PGPERR_OK)
		goto error;
	fp2 = tmpkeyfile(fileRef, &tmpFileRef, kPGPFileOpenStdWriteFlags,
						isPrivate);
	if (fp2 == NULL)
	{
		result = PGPERR_NO_FILE;
		goto error;
	}
	/* Write out new data to temp file */
	pfp2 = pgpFileWriteOpen(fp2, NULL);
	result = ringSetWrite(set, pfp2, NULL,
			PGPVERSION_3, PGP_WRITETRUST_PUB);
	pgpFileClose(pfp2);	/* closes fp2 */
	ringSetDestroy(set);
	set = NULL;
	if (result != PGPERR_OK)
		goto error;

	if ((result = ringFileClose(oldrfile)) != PGPERR_OK)
	{
		/* Have a problem here, it should have closed */
		/* Probably due to a pesky RingSet leak! */
		pgpAssert (0);
		goto error;
	}
	/* Close the PgpFile */
	if (pfp)
	{
		pgpFileClose(pfp);
		pfp = NULL;
	}
	/* Now rename the files */
	pgpDeleteFile(bakFileRef);
	pgpRenameFile(fileRef, bakFileRef);
	pgpDeleteFile(fileRef);
	if (pgpRenameFile(tmpFileRef, fileRef) != PGPERR_OK) {
		result = PGPERR_NO_FILE;	/* XXX Correct error code? */
		goto error;
	}
	result = PGPERR_OK;
	goto done;
memError:
	result = PGPERR_NOMEM;
error:
	if (set != NULL)
		ringSetDestroy(set);
	if (pfp != NULL)
		pgpFileClose(pfp);
done:
	if (tmpFileRef != NULL)
	{
		pgpDeleteFile(tmpFileRef);
		pgpFreeFileRef(tmpFileRef);
	}
	if (bakFileRef != NULL)
		pgpFreeFileRef(bakFileRef);
	if (fileRef != NULL)
		pgpFreeFileRef(fileRef);
	pgpMemFree(bak);
	pgpMemFree(tmp);
	pgpMemFree(fileName);
	return result;
}

/*
 * Write out the specified RingSet in ascii armor form to pipeline.
 */
static int
spgpWriteArmoredSetPipe (PgpPipeline *ppipe, RingSet const *set, PgpEnv *env)
{
	PgpFile *pfp;
	int rslt;

	/* Convert to local line endings */
	if (!pgpTextFiltCreate (&ppipe,
			pgpenvGetPointer (env, PGPENV_CHARMAPTOLATIN1, NULL,
			 		 		 	NULL),
			0, SimplePGPGetLineEndType())) {
		ppipe->teardown(ppipe);
		return PGPERR_NOMEM;
	}

	if (!pgpArmorWriteCreate(&ppipe, env, &pgpByteFifoDesc,
				NULL, PGPVERSION_2_6, PGP_ARMOR_NORMAL)) {
		ppipe->teardown(ppipe);
		return PGPERR_NOMEM;
	}
	pfp = pgpFilePipelineOpen(ppipe);
	if (!pfp) {
		ppipe->teardown(ppipe);
		return PGPERR_NOMEM;
	}
	rslt = ringSetWrite (set, pfp, NULL, PGPVERSION_2_6, 0);
	pgpFileClose (pfp);
	return rslt;
}

/* Write out the specified RingSet in ascii armor form to fp. */
int
spgpWriteArmoredSetFile (FILE *fp, RingSet const *set, PgpEnv *env)
{
	PgpFile *pfp;
	PgpPipeline *ppl = NULL;
	
	pfp = pgpFileWriteOpen(fp, NULL);
	if (!pfp) {
		pgpStdIOClose(fp);
		return PGPERR_NOMEM;
	}
	if (!pgpFileWriteCreate (&ppl, pfp, 1)) {
		pgpFileClose(pfp);
		return PGPERR_NOMEM;
	}
	return spgpWriteArmoredSetPipe (ppl, set, env);
}

/*
* Write out the specified RingSet in ascii armor form to buf.
* Set *buflen to number of bytes we wrote or would have written.
* Read the size of buf from *buflen on entry.
*/
int
spgpWriteArmoredSetBuffer (byte *buf, size_t *buflen, RingSet const *set,
	PgpEnv *env)
{
	PgpPipeline	*ppl = NULL,
		*pipebuf;
	PgpFile	*pfp;
	size_t	nbuf = *buflen,
		written;
	int	rslt;
	
	pipebuf = pgpMemModCreate (&ppl, buf, nbuf);

	/* Convert to local line endings */
	if (!pgpTextFiltCreate (&ppl,
			pgpenvGetPointer (env, PGPENV_CHARMAPTOLATIN1, NULL,
							NULL),
			0, SimplePGPGetLineEndType())) {
		ppl->teardown(ppl);
		return PGPERR_NOMEM;
	}

	if (!pgpArmorWriteCreate(&ppl, env, &pgpByteFifoDesc,
				NULL, PGPVERSION_2_6, PGP_ARMOR_NORMAL)) {
		ppl->teardown(ppl);
		return PGPERR_NOMEM;
	}
	pfp = pgpFilePipelineOpen(ppl);
	if (!pfp) {
		ppl->teardown(ppl);
		return PGPERR_NOMEM;
	}
	rslt = ringSetWrite (set, pfp, NULL, PGPVERSION_2_6, 0);
	if (!rslt) {
		/* Figure out how many bytes would be written */
		ppl->sizeAdvise (ppl, 0);
		pipebuf->annotate (pipebuf, NULL, PGPANN_MEM_BYTECOUNT,
			(byte *)&written, sizeof (written));
		*buflen = written;
	}
	pgpFileClose (pfp);
	return rslt;
}


/*
* Close the specified ringfile, raising an assertion if it doesn't
* want to close. That is a sign that we have a "ringset leak" and
* this will eventually lead to running out of ringsets (we only can
* have 16 at a time).
*/
void
spgpRingFileClose (RingFile *rfile)
{
	pgpAssert (!ringFileCheckClose (rfile));
	ringFileClose (rfile);
}


/*
* Local Variables:
* tab-width: 4
* End:
* vi: ts=4 sw=4
* vim: si
*/
