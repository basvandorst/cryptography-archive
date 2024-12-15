/*
 * pgpMacFile.c -- PgpFile implementation for MacBinary translation
 *
 * Copyright (C) 1997 Pretty Good Privacy, Inc. All rights reserved.
 *
 * $Id: pgpMacFile.c,v 1.9.2.3 1997/06/07 09:50:08 mhw Exp $
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <errno.h>
#include <stdio.h>

#include "MacStrings.h"
#include "pgpTypes.h"
#include "pgpFile.h"
#include "pgpMacFile.h"
#include "pgpMacUtils.h"
#include "pgpFileRef.h"
#include "pgpFileMod.h"
#include "pgpCFB.h"
#include "pgpMem.h"
#include "pgpErr.h"
#include "pgpUsuals.h"

/* XXX Is this stuff still necessary? */

#define DESKTOP   0x0001   // ---- ---- ---- ---1
#define COLOR     0x000E   // ---- ---- ---- 111-
#define INITED    0x0100   // ---- ---1 ---- ----
#define CHANGED   0x0200   // ---- --1- ---- ----
#define BUSY      0x0400   // ---- -1-- ---- ----
#define BOZO      0x0800   // ---- 1--- ---- ----
#define SYSTEM    0x1000   // ---1 ---- ---- ----
#define BUNDLE    0x2000   // --1- ---- ---- ----
#define INVISIBLE 0x4000   // -1-- ---- ---- ----
#define LOCKED    0x8000   // 1--- ---- ---- ----

#define NOMODIFY (DESKTOP | INITED | CHANGED | BUSY)

#pragma options align=mac68k

typedef struct MacBinaryHeader_
{
    uchar  filler;
    uchar  oldVersion;     /* 000: Must be zero for compatibility */
    uchar  name[64];       /* 001: Pascal string */
    FInfo  info1;          /* 065: Original Finder info: */
                           /*      File type (long) */
                           /*      File creator (long) */
                           /*      Flags (word, low byte must be zero) */
                           /*      File's location (Point) */
                           /*      File's window (short) */
    uchar  protectedBit;   /* 081: Low order bit */
    uchar  zero1;          /* 082: Must be zero for compatibility */
    long   dLength;        /* 083: Data fork length (in bytes) */
    long   rLength;        /* 087: Resource fork length (in bytes) */
    ulong  creation;       /* 091: Creation date */
    ulong  modification;   /* 095: Modification date */
    short  getInfoLength;  /* 099: Get info length */
    uchar  info2;          /* 101: Finder flags, low byte */
    uchar  dummy1;
    uchar  dummy2[18];     /* 103: Not used */
    uchar  dummy3;
    uchar  newVersion;     /* 122: Uploading program version number */
    uchar  minimumVersion; /* 123: Minimum version number needed */
    uchar  crc1;
    uchar  crc2;
    uchar  endFiller[2];    /* To pad out to 128 bytes */
} MacBinaryHeader;

#pragma options align=reset

static struct BinaryTypeRec
{
    OSType    creator;
    OSType    type;
    int       length;
    uchar *   ident;
} smartBinRecs[] = {
    { 'pZIP', 'pZIP',  2, "PK" },
    { 'LZIV', 'ZIVU',  3, "\x1F\x9D\x90" },
    { 'MIDI', 'Midi',  4, "MThd" },
    { 'PAK ', 'PAK ',  2, "\x1A\x0A" },
    { 'BOOZ', 'ZOO ',  4, "ZOO " },
    { 'JVWR', 'GIFf',  4, "GIF8" },
    { 'SIT!', 'SIT!',  4, "SIT!" },
    { 'CPCT', 'PACT',  2, "\x01\x01" },
    { 'arc*', 'mArc',  2, "\x1A\x08" },
    { 'arc*', 'mArc',  2, "\x1A\x09" },
    { 'JVWR', 'JPEG', 10, "\xFF\xD8\xFF\xE0\0\x10JFIF" },
    { 0, 0, 0, NULL }
};

static OSType nonMacBinaryTypes[] = {
    'TEXT', 'pZIP', 'ZIVU', 'Midi', 'PAK ', 'ZOO ', 'GIFf', 'mArc', 'JPEG', 0
};

#define kPGPMacRead         0x01L
#define kPGPMacWrite        0x02L
#define kPGPMacBinMode      0x04L
  /* For write mode only: whether we should check */
#define kPGPCheckMacBin     0x08L
  /* Strip out volatile information for detached sig */
#define kPGPMacBinHashOnly  0x10L
  /* CRC 0 in MacBin header is okay */
#define kPGPNoMacBinCRCOkay 0x20L

/* This is the private data for macfiles */
typedef struct MacFile_
{
    ulong           flags;
    short           dataRef, resRef;
    long            dataOffset, resOffset, totalSize;
    long            filePos;
    PgpFileError    err;
    PGPError        error;
    PGPFileRef *    fileRef;
    PGPFileType     fileType;
    MacBinaryHeader macBinHeader;
} MacFile;

static PGPError PrepareToWrite(PgpFile *file);
static PGPError SetFileInfo(PgpFile *file);

    static void
macSetError(
    PgpFile *     file,
    PGPError      code)
{
    MacFile *     mf = (MacFile *)file->priv;

    mf->err.f = file;
    mf->err.fpos = mf->filePos;
    mf->error = mf->err.error = code;
    mf->err.syserrno = 0;
}

    static OSErr
macFileReadFork(
    PgpFile *     file,
    short         refNum,
    long          forkOffset,
    long          forkEnd,
    long *        sizeLeftPtr,
    void **       ptrPtr)
{
    MacFile *     mf = (MacFile *)file->priv;
    long          chunkSize;
    OSErr         result = noErr;
    IOParam       pb;

    if (mf->filePos >= forkOffset &&
        (chunkSize = min(*sizeLeftPtr, forkEnd - mf->filePos)) > 0)
    {
        pb.ioRefNum = refNum;
        pb.ioBuffer = (Ptr)*ptrPtr;
        pb.ioReqCount = chunkSize;
        pb.ioPosMode = fsFromStart;
        pb.ioPosOffset = mf->filePos - forkOffset;
        result = PBReadSync((ParmBlkPtr)&pb);
        mf->filePos += pb.ioActCount;
        *(Ptr *)ptrPtr += pb.ioActCount;
        *sizeLeftPtr -= pb.ioActCount;
        if ((mf->flags & kPGPMacBinMode) && result == eofErr)
        {
            /*
             * Clear any memory which was beyond the EOF. This is expected
             * in MacBinary because each fork is padded to a multiple of
             * 128-bytes.
             */
            chunkSize -= pb.ioActCount;
            pgpClearMemory((uchar *)*ptrPtr, chunkSize);
            mf->filePos += chunkSize;
            *(Ptr *)ptrPtr += chunkSize;
            *sizeLeftPtr -= chunkSize;
            result = noErr;
        }
        else if (result != noErr)
            macSetError(file, PGPERR_FILE_OPFAIL);
    }
    return result;
}

    static size_t
macFileRead(
    void *        ptr,
    size_t        size,
    PgpFile *     file)
{
    MacFile *     mf = (MacFile *)file->priv;
    long          sizeLeft = (long)size;
    long          chunkSize;
    OSErr         result = noErr;

    pgpAssert(mf->filePos >= 0);

    if (!(mf->flags & kPGPMacRead))
    {
        macSetError(file, PGPERR_FILE_BADOP);
        return 0;
    }

    /* Handle portion in the MacBinary header, if any */
    if ((chunkSize = min(sizeLeft, mf->dataOffset - mf->filePos)) > 0)
    {
        pgpCopyMemory((uchar *)&mf->macBinHeader + 1 + mf->filePos,
                      ptr, chunkSize);
        mf->filePos += chunkSize;
        ptr = (void *)((Ptr)ptr + chunkSize);
        sizeLeft -= chunkSize;
    }

    /* Handle portion in the data fork, if any */
    if (result == noErr && sizeLeft > 0)
        result = macFileReadFork(file, mf->dataRef, mf->dataOffset,
                                 mf->resOffset, &sizeLeft, &ptr);

    /*
     * Handle zeroing the first 128 bytes of the resource fork,
     * but only if kPGPMacBinHashOnly is set.
     */
    if (result == noErr
        && (mf->flags & kPGPMacBinMode) && (mf->flags & kPGPMacBinHashOnly)
        && (chunkSize = min(sizeLeft,
                            min(mf->resOffset + 128, mf->totalSize)
                            - mf->filePos)) > 0)
    {
        pgpClearMemory(ptr, chunkSize);
        mf->filePos += chunkSize;
        ptr = (void *)((Ptr)ptr + chunkSize);
        sizeLeft -= chunkSize;
    }

    /* Handle portion in the resource fork, if any */
    if (result == noErr && sizeLeft > 0 && (mf->flags & kPGPMacBinMode))
        result = macFileReadFork(file, mf->resRef, mf->resOffset,
                                 mf->totalSize, &sizeLeft, &ptr);

    return (size_t)(size - sizeLeft);
}

    static OSErr
macFileWriteFork(
    PgpFile *     file,
    short         refNum,
    long          forkOffset,
    long          forkEnd,  /* Ignored unless kPGPMacBinMode flag set */
    long          forkLen,  /* Ignored unless kPGPMacBinMode flag set */
    long *        sizeLeftPtr,
    void const ** ptrPtr)
{
    MacFile *     mf = (MacFile *)file->priv;
    long          chunkSize;
    OSErr         result = noErr;
    IOParam       pb;

    chunkSize = *sizeLeftPtr;
    if (mf->flags & kPGPMacBinMode)
        chunkSize = min(chunkSize, forkEnd - mf->filePos);

    if (mf->filePos >= forkOffset && chunkSize > 0)
    {
        pb.ioRefNum = refNum;
        pb.ioBuffer = (Ptr)*ptrPtr;
        pb.ioPosMode = fsFromStart;
        pb.ioPosOffset = mf->filePos - forkOffset;
        pb.ioReqCount = chunkSize;
        if (mf->flags & kPGPMacBinMode)
            pb.ioReqCount = min(pb.ioReqCount, forkLen - pb.ioPosOffset);
        result = PBWriteSync((ParmBlkPtr)&pb);
        mf->filePos += chunkSize;
        *(Ptr *)ptrPtr += chunkSize;
        *sizeLeftPtr -= chunkSize;
        if (result != noErr)
            macSetError(file, PGPERR_FILE_OPFAIL);
    }
    if (mf->totalSize < mf->filePos)
        mf->totalSize = mf->filePos;
    return result;
}

    static size_t
macFileWrite(
    void const *  ptr,
    size_t        size,
    PgpFile *     file)
{
    MacFile *     mf = (MacFile *)file->priv;
    long          sizeLeft = (long)size;
    long          chunkSize;
    OSErr         result = noErr;

    pgpAssert(mf->filePos >= 0);

    if (!(mf->flags & kPGPMacWrite))
    {
        macSetError(file, PGPERR_FILE_BADOP);
        return 0;
    }

    /* Handle portion in the MacBinary header, if any */
    if ((chunkSize = min(sizeLeft, mf->dataOffset - mf->filePos)) > 0)
    {
        pgpAssert(mf->filePos + chunkSize <= 128);
        pgpCopyMemory(ptr, (uchar *)&mf->macBinHeader + 1 + mf->filePos,
                      chunkSize);
        mf->filePos += chunkSize;
        ptr = (void *)((Ptr)ptr + chunkSize);
        sizeLeft -= chunkSize;
        if (mf->totalSize < mf->filePos)
            mf->totalSize = mf->filePos;
    }

    /* Handle portion in the data fork, if any */
    if (result == noErr && sizeLeft > 0)
    {
        PrepareToWrite(file);
        result = macFileWriteFork(file, mf->dataRef, mf->dataOffset,
                                  mf->resOffset, mf->macBinHeader.dLength,
                                  &sizeLeft, &ptr);
    }

    /* Handle portion in the resource fork, if any */
    if (result == noErr && sizeLeft > 0 && (mf->flags & kPGPMacBinMode))
        result = macFileWriteFork(file, mf->resRef, mf->resOffset,
                                  mf->totalSize, mf->macBinHeader.rLength,
                                  &sizeLeft, &ptr);

    return (size_t)(size - sizeLeft);
    return 0;
}

    static int
macFileFlush(
    PgpFile *     file)
{
    MacFile *     mf = (MacFile *)file->priv;
    IOParam       pb;

    pb.ioRefNum = mf->dataRef;
    PBFlushFileSync((ParmBlkPtr)&pb);
    pb.ioRefNum = mf->resRef;
    PBFlushFileSync((ParmBlkPtr)&pb);
    return PGPERR_OK;
}

    static int
macFileClose(
    PgpFile *     file)
{
    MacFile *     mf = (MacFile *)file->priv;

    PrepareToWrite(file);
    if (mf->dataRef != 0)
        FSClose(mf->dataRef);
    if (mf->resRef != 0)
        FSClose(mf->resRef);
    SetFileInfo(file);
    pgpFreeFileRef(mf->fileRef);
    pgpClearMemory((void *)mf, sizeof(*mf));
    pgpFree(mf);
    pgpFree(file);
    return PGPERR_OK;
}

    static long
macFileTell(
    PgpFile *     file)
{
    MacFile *     mf = (MacFile *)file->priv;

    return mf->filePos;
}

    static int
macFileSeek(
    PgpFile *     file,
    long          offset,
    int           whence)
{
    MacFile *     mf = (MacFile *)file->priv;

    switch (whence)
    {
        case SEEK_SET:
            /* offset is correct, as is */
            break;
        case SEEK_CUR:
            offset += mf->filePos;
            break;
        case SEEK_END:
            offset += mf->totalSize;
            break;
    }

    if (offset < 0 || offset > mf->totalSize)
    {
        macSetError(file, PGPERR_FILE_OPFAIL);
        return PGPERR_FILE_OPFAIL;
    }

    mf->filePos = offset;
    return PGPERR_OK;
}

    static int
macFileEof(
    PgpFile const * file)
{
    MacFile *       mf = (MacFile *)file->priv;

    /*
     * XXX This will always return TRUE when writing files.
     *     Is that semantically correct?
     */
    return mf->filePos >= mf->totalSize;
}

    static long
macFileSizeAdvise(
    PgpFile const * file)
{
    MacFile *       mf = (MacFile *)file->priv;

    return mf->totalSize;
}

    static PgpFileError const *
macFileError(
    PgpFile const *     file)
{
    MacFile *           mf = (MacFile *)file->priv;

    if (mf->error != PGPERR_OK)
        return &mf->err;
    else
        return NULL;
}

    static void
macFileClearError(
    PgpFile *     file)
{
    MacFile *     mf = (MacFile *)file->priv;

    mf->error = PGPERR_OK;
}

    static int
macFileWrite2Read(
    PgpFile *     file)
{
    macSetError(file, PGPERR_FILE_BADOP);
    return PGPERR_FILE_BADOP;
}

    static PgpCfbContext *
macFileCfb(
    PgpFile const * file)
{
    (void)file; /* Quiet compiler warning */
    /* XXX: I'm not really sure what should be done here */
    return NULL;
}

#ifndef __powerc

static ushort asm CalcCRC16Contin(ushort start, void *data, long len)
{
    fralloc +
    move.l  D3, -(SP)
    move.w  #0x1021, D3
    clr.l   D0
    move.w  start, D0
    move.l  data, A0
    move.l  len, D2
    bra     Loop1
Loop0:
    move.b  (A0)+, D1
    lsl.w   #8, D1
    eor.w   D1, D0
    moveq   #7, D1
ShiftLoop0:
    lsl.w   #1, D0
    bcc     ShiftLoop1
    eor.w   D3, D0
ShiftLoop1:
    dbra    D1, ShiftLoop0
Loop1:
    dbra    D2, Loop0
    move.l  (SP)+, D3
    frfree
#if GENERATINGCFM || defined(__CFM68K__)
    rtd     #12
#else
    rts
#endif
}

static uchar asm CalcChecksum8Contin(uchar start, void *data, long len)
{
    FRALLOC +
    CLR.L   D0
    MOVE.B  start, D0
    MOVE.L  data, A0
    MOVE.L  len, D1
    BRA     Loop1
Loop0:
    ADD.B   (A0)+, D0
Loop1:
    DBRA    D1, Loop0
    FRFREE
#if GENERATINGCFM || defined(__CFM68K__)
    rtd     #12
#else
    rts
#endif
}

static ulong asm CalcCRC32(void *data, long len)
{
    fralloc +
    move.l  data, A0
    clr.l   D0
    move.l  len, D1
    lsr.l   #2, D1
    bra     Loop1
Loop0:
    rol.l   #1, D0
    add.l   (A0)+, D0
Loop1:
    dbra    D1, Loop0
    moveq   #3, D2
    move.l  len, D1
    and.l   D2, D1
    lsl.w   D2, D1
    clr.l   D2
    not.l   D2
    lsr.l   D1, D2
    not.l   D2
    and.l   (A0), D2
    rol.l   #1, D0
    add.l   D2, D0
    frfree
#if GENERATINGCFM || defined(__CFM68K__)
    rtd     #8
#else
    rts
#endif
}

#else

static ushort CalcCRC16Contin(ushort crc, void *data, long len)
{
    char *dp=(char *)data;
    short i;

    while(len--)
    {
        crc ^= (ushort)(*dp++) << 8;
        for (i = 0; i < 8; ++i)
        {
            if (crc & 0x8000)
                crc = (crc << 1) ^ 0x1021;
            else
                crc <<= 1;
        }
    }
    return(crc);
}

static uchar CalcChecksum8Contin(uchar start, void *data, long len)
{
    char *dp=(char *)data;

    while(len--)
        start+=*dp++;
    return start;
}

static ulong CalcCRC32(void *data, register long len)
{
    register ulong *dp=(ulong *)data;
    register ulong crc=0, x;
    register long len2;
    ulong highSet;

    len2=len;
    len>>=2;
    while(--len>=0)
    {
        highSet=(crc & 0x80000000);
        crc<<=1;
        if(highSet)
            crc|=1;
        crc+=*dp++;
    }
    x=3;
    len2 &=x;
    len2<<=x;
    x=0;
    x=~x;
    x>>=len2;
    x=~x;
    x &= *dp;
    highSet=(crc & 0x80000000);
    crc<<=1;
    if(highSet)
        crc|=1;
    crc+=x;
    return crc;
}

#endif

static uchar CalcChecksum8(void *data, long len)
{
    return CalcChecksum8Contin(0, data, len);
}

static ushort CalcCRC16(void *data, long len)
{
    return CalcCRC16Contin(0, data, len);
}

/*
 * This routine is called each time before writing to an actual fork. If
 * the kPGPCheckMacBin flag is set, it determines whether or not it's a
 * valid MacBinary header. If so, it creates the file and opens both the
 * data and resource forks. Otherwise it creates and opens the data fork,
 * and writes false header information to the data fork, using whatever
 * hints it can find to create the file with the correct type and creator.
 */
    static PGPError
PrepareToWrite(
    PgpFile *       file)
{
    MacFile *       mf = (MacFile *)file->priv;
    FSSpec          spec;
    HParamBlockRec  hio;
    PGPError        result = PGPERR_OK;
    OSErr           macResult;

    if (!(mf->flags & kPGPMacWrite))
        return PGPERR_OK;

    if (mf->flags & kPGPCheckMacBin)
    {
        if ((result = pgpFSSpecFromFileRef(mf->fileRef, &spec)) != PGPERR_OK)
            goto error;
        mf->flags &= ~(kPGPMacBinMode | kPGPCheckMacBin);
        if (mf->totalSize >= 126 && mf->macBinHeader.oldVersion == 0 &&
            mf->macBinHeader.minimumVersion <= 129 &&
            (mf->macBinHeader.info1.fdFlags & 0xFF) == 0 &&
            mf->macBinHeader.zero1 == 0 &&
            mf->macBinHeader.name[0] < 64)
        {
            ushort    crc;

            pgpAssert(sizeof(crc) == 2);
            pgpCopyMemory(&mf->macBinHeader.crc1, &crc, sizeof(crc));
            if (crc == CalcCRC16((uchar *)&mf->macBinHeader + 1, 124)
                || ((mf->flags & kPGPNoMacBinCRCOkay) && crc == 0))
                mf->flags |= kPGPMacBinMode;
        }
    }

    if (mf->dataRef != 0)
        return PGPERR_OK;

    if ((result = pgpFSSpecFromFileRef(mf->fileRef, &spec)) != PGPERR_OK)
        goto error;

    if (mf->flags & kPGPMacBinMode)
    {
        /* XXX Reconsider using a different file type for this */
        if ((result = pgpCreateFile(mf->fileRef, kPGPFileTypeDecryptedBin))
            != PGPERR_OK)
            goto error;

        hio.fileParam.ioNamePtr = spec.name;
        hio.fileParam.ioVRefNum = spec.vRefNum;
        hio.fileParam.ioDirID = spec.parID;
        hio.ioParam.ioPermssn = fsWrPerm;
        if ((macResult = PBHOpenDFSync(&hio)) != noErr)
            goto macError;
        mf->dataRef = hio.ioParam.ioRefNum;

        hio.fileParam.ioNamePtr = spec.name;
        hio.fileParam.ioVRefNum = spec.vRefNum;
        hio.fileParam.ioDirID = spec.parID;
        hio.ioParam.ioPermssn = fsWrPerm;
        if ((macResult = PBHOpenRFSync(&hio)) != noErr)
            goto macError;
        mf->resRef = hio.ioParam.ioRefNum;

        mf->dataOffset = 128;
        mf->resOffset = mf->dataOffset +
            (mf->macBinHeader.dLength + 127) & ~127L;
        mf->totalSize = mf->resOffset +
            (mf->macBinHeader.rLength + 127) & ~127L;
    }
    else
    {
        Boolean   created = FALSE;
        int       i;

        for (i = 0; smartBinRecs[i].ident != NULL; i++)
            if (pgpMemoryEqual((uchar *)&mf->macBinHeader + 1,
                               smartBinRecs[i].ident,
                               smartBinRecs[i].length))
            {
                if ((macResult = HCreate(spec.vRefNum, spec.parID,
                                         spec.name, smartBinRecs[i].creator,
                                         smartBinRecs[i].type)) != noErr)
                    goto macError;
                created = TRUE;
                break;
            }
        if (!created)
            if ((result = pgpCreateFile(mf->fileRef, mf->fileType))
                != PGPERR_OK)
                goto error;

        hio.fileParam.ioNamePtr = spec.name;
        hio.fileParam.ioVRefNum = spec.vRefNum;
        hio.fileParam.ioDirID = spec.parID;
        hio.ioParam.ioPermssn = fsWrPerm;
        if ((macResult = PBHOpenDFSync(&hio)) != noErr)
            goto macError;
        mf->dataRef = hio.ioParam.ioRefNum;

        mf->dataOffset = mf->resOffset = 0;

        {
            IOParam   io;

            io.ioRefNum = mf->dataRef;
            io.ioBuffer = (Ptr)&mf->macBinHeader + 1;
            io.ioPosMode = fsFromStart;
            io.ioPosOffset = 0;
            io.ioReqCount = mf->totalSize;
            pgpAssert(mf->totalSize <= 128);
            if ((macResult = PBWriteSync((ParmBlkPtr)&io)) != noErr)
                goto macError;
        }
    }

    return PGPERR_OK;
macError:
   result = pgpErrorFromMacError(macResult, PGPERR_FILE_OPFAIL);
error:
   macSetError(file, result);
   return result;
}

/*
 * This routine is called after closing a MacBinary file for writing. It
 * sets the finder information including type and creator.
 */
    static PGPError
SetFileInfo(
    PgpFile *     file)
{
    MacFile *     mf = (MacFile *)file->priv;
    FSSpec        spec;
    HFileParam    pb;

    if (!(mf->flags & kPGPMacWrite) || !(mf->flags & kPGPMacBinMode))
        return PGPERR_OK;

    if (pgpFSSpecFromFileRef(mf->fileRef, &spec) != PGPERR_OK)
        goto error;

    pb.ioNamePtr = spec.name;
    pb.ioVRefNum = spec.vRefNum;
    pb.ioDirID = spec.parID;
    pb.ioFlCrDat = mf->macBinHeader.creation;
    pb.ioFlMdDat = mf->macBinHeader.modification;
    pgpCopyMemory(&mf->macBinHeader.info1, &pb.ioFlFndrInfo,
                  sizeof(pb.ioFlFndrInfo));
    pb.ioFlFndrInfo.fdFlags &= 0xFF00;
    pb.ioFlFndrInfo.fdFlags |= mf->macBinHeader.info2;
    pb.ioFlFndrInfo.fdFlags &= ~NOMODIFY;
    pb.ioFlFndrInfo.fdLocation.h = 0;
    pb.ioFlFndrInfo.fdLocation.v = 0;
    pb.ioFlFndrInfo.fdFldr = 0;
    if (PBHSetFInfoSync((HParmBlkPtr)&pb) != noErr)
        goto error;

    return PGPERR_OK;

error:
    /* XXX Improve error reporting */
    macSetError(file, PGPERR_FILE_OPFAIL);
    return PGPERR_FILE_OPFAIL;
}

/*
 * It should be noted that writing to MacBinary files won't work if seeking
 * is used in certain ways. The MacBinary header only interpreted the first
 * time data is written past the first 128 bytes, and then the fork lengths
 * and offsets are fixed and immutable. This could be fixed at some point,
 * but there's no need for it now.
 *
 * The filename used to create the file is always the <fileRef> passed in,
 * not the name in the MacBinary header.
 */
    PgpFile *
pgpFileRefMacWriteOpen(
    PGPFileRef const *  fileRef,
    PGPFileType         fileType,
    PGPFileOpenFlags    flags,
    PGPError *          errorCode)
{
    PgpFile *   file = NULL;
    MacFile *   mf = NULL;
    FSSpec      spec;
    PGPError    result = PGPERR_OK;

    if ((file = (PgpFile *)pgpAlloc(sizeof(*file))) == NULL)
    {
        result = PGPERR_NOMEM;
        goto error;
    }

    if ((mf = (MacFile *)pgpAlloc(sizeof(*mf))) == NULL)
    {
        result = PGPERR_NOMEM;
        goto error;
    }

    pgpClearMemory(mf, sizeof(*mf));
    pgpClearMemory(file, sizeof(*file));

    mf->fileRef = pgpCopyFileRef(fileRef);
    if ((result = pgpFSSpecFromFileRef(mf->fileRef, &spec)) != PGPERR_OK)
        goto error;
    mf->fileType = fileType;

    /* Try to delete the file first, in case it already exists */
    /* XXX Maybe we should check the error, and/or truncate the file instead */
    HDelete(spec.vRefNum, spec.parID, spec.name);

    pgpAssert(sizeof(mf->macBinHeader) >= 129);
    pgpAssert((flags & kPGPFileOpenLocalEncodeHashOnly) == 0);

    mf->flags = kPGPMacWrite;

    if (flags & (kPGPFileOpenMaybeLocalEncode | kPGPFileOpenForceLocalEncode))
    {
        mf->flags |= kPGPCheckMacBin;
        if (flags & kPGPFileOpenNoMacBinCRCOkay)
            mf->flags |= kPGPNoMacBinCRCOkay;
        mf->dataOffset = mf->resOffset = 128;
    }
    else
        mf->dataOffset = mf->resOffset = 0;

    mf->dataRef = mf->resRef = 0;
    mf->totalSize = 0;
    mf->filePos = 0;

    file->priv = mf;
    file->read = macFileRead;
    file->write = macFileWrite;
    file->flush = macFileFlush;
    file->close = macFileClose;
    file->tell = macFileTell;
    file->seek = macFileSeek;
    file->eof = macFileEof;
    file->sizeAdvise = macFileSizeAdvise;
    file->error = macFileError;
    file->clearError = macFileClearError;
    file->write2read = macFileWrite2Read;
    file->cfb = macFileCfb;

    if (!(mf->flags & kPGPCheckMacBin))
        if ((result = PrepareToWrite(file)) != PGPERR_OK)
            goto error;

    return file;
error:
    if (mf != NULL)
    {
        if (mf->fileRef != NULL)
            pgpFreeFileRef(mf->fileRef);
        pgpFree(mf);
    }
    if (file != NULL)
        pgpFree(file);
    if (errorCode != NULL)
    {
        pgpAssertAddrValid(errorCode, PGPError);
        *errorCode = result;
    }
    return NULL;
}

    PgpFile *
pgpFileRefMacReadOpen(
    PGPFileRef const *  fileRef,
    PGPFileOpenFlags    flags,
    PGPError *          errorCode)
{
    PgpFile *           file = NULL;
    MacFile *           mf = NULL;
    FSSpec              spec;
    HFileParam          pb;
    HParamBlockRec      io;
    ushort              crc;
    PGPError            result = PGPERR_OK;

    if ((file = (PgpFile *)pgpAlloc(sizeof(*file))) == NULL)
    {
        result = PGPERR_NOMEM;
        goto error;
    }

    if ((mf = (MacFile *)pgpAlloc(sizeof(*mf))) == NULL)
    {
        result = PGPERR_NOMEM;
        goto error;
    }

    pgpClearMemory(mf, sizeof(*mf));
    pgpClearMemory(file, sizeof(*file));

    mf->fileRef = pgpCopyFileRef(fileRef);
    if ((result = pgpFSSpecFromFileRef(mf->fileRef, &spec)) != PGPERR_OK)
        goto error;
    mf->fileType = kPGPFileTypeNone;

    pb.ioNamePtr = spec.name;
    pb.ioVRefNum = spec.vRefNum;
    pb.ioDirID = spec.parID;
    pb.ioFDirIndex = 0;
    if (PBHGetFInfoSync((HParmBlkPtr)&pb) != noErr)
    {
        result = pgpErrorFromMacError(pb.ioResult, PGPERR_NO_FILE);
        goto error;
    }

    pgpAssert(sizeof(mf->macBinHeader) >= 129);
    pgpClearMemory(&mf->macBinHeader, sizeof(mf->macBinHeader));
    CopyPString(spec.name, mf->macBinHeader.name);
    mf->macBinHeader.info2 = pb.ioFlFndrInfo.fdFlags & 0x00FF;
    pb.ioFlFndrInfo.fdFlags &= 0xFF00;
    pb.ioFlFndrInfo.fdLocation.h = 0;
    pb.ioFlFndrInfo.fdLocation.v = 0;
    pb.ioFlFndrInfo.fdFldr = 0;
    mf->macBinHeader.info1 = pb.ioFlFndrInfo;
    mf->macBinHeader.dLength = pb.ioFlLgLen;
    mf->macBinHeader.rLength = pb.ioFlRLgLen;
    mf->macBinHeader.creation = pb.ioFlCrDat;
    mf->macBinHeader.modification = pb.ioFlMdDat;
    mf->macBinHeader.newVersion = 129;
    mf->macBinHeader.minimumVersion = 129;
    crc = CalcCRC16((uchar *)&mf->macBinHeader + 1, 124);
    pgpAssert(sizeof(crc) == 2);
    pgpCopyMemory(&crc, &mf->macBinHeader.crc1, sizeof(crc));

    mf->flags = kPGPMacRead;
    if (flags & kPGPFileOpenMaybeLocalEncode)
    {
        int   i;
        mf->flags |= kPGPMacBinMode;
        for (i = 0; nonMacBinaryTypes[i] != 0; i++)
            if (pb.ioFlFndrInfo.fdType == nonMacBinaryTypes[i])
            {
                mf->flags &= ~kPGPMacBinMode;
                break;
            }
    }
    else if (flags & kPGPFileOpenForceLocalEncode)
        mf->flags |= kPGPMacBinMode;

    io.fileParam.ioNamePtr = spec.name;
    io.fileParam.ioVRefNum = spec.vRefNum;
    io.fileParam.ioDirID = spec.parID;
    io.ioParam.ioPermssn = fsRdPerm;
    if (PBHOpenDFSync(&io) != noErr)
    {
        result = pgpErrorFromMacError(io.ioParam.ioResult, PGPERR_NO_FILE);
        goto error;
    }
    mf->dataRef = io.ioParam.ioRefNum;

    if (mf->flags & kPGPMacBinMode)
    {
        io.fileParam.ioNamePtr = spec.name;
        io.fileParam.ioVRefNum = spec.vRefNum;
        io.fileParam.ioDirID = spec.parID;
        io.ioParam.ioPermssn = fsRdPerm;
        if (PBHOpenRFSync(&io) != noErr)
        {
            result = pgpErrorFromMacError(io.ioParam.ioResult, PGPERR_NO_FILE);
            goto error;
        }
        mf->resRef = io.ioParam.ioRefNum;

        mf->dataOffset = 128;
        mf->resOffset = mf->dataOffset + (pb.ioFlLgLen + 127) & ~127L;
        mf->totalSize = mf->resOffset + (pb.ioFlRLgLen + 127) & ~127L;
        if (flags & kPGPFileOpenLocalEncodeHashOnly)
        {
            mf->flags |= kPGPMacBinHashOnly;
            /* Clear the finder flags */
            mf->macBinHeader.info1.fdFlags = 0;
            mf->macBinHeader.info2 = 0;
            /* Clear the creation and modification dates */
            mf->macBinHeader.creation = 0;
            mf->macBinHeader.modification = 0;
            /* Clear the filename */
            pgpClearMemory(mf->macBinHeader.name,
                           sizeof(mf->macBinHeader.name));
            /*
             * Clear the CRC so that if this output gets used to
             * recreate a file (which it shouldn't), it won't work
             * properly and someone will notice the problem.
             */
            pgpClearMemory(&mf->macBinHeader.crc1, sizeof(crc));
        }
    }
    else
    {
        mf->resRef = 0;
        mf->dataOffset = 0;
        mf->totalSize = mf->resOffset = pb.ioFlLgLen;
    }

    mf->filePos = 0;

    file->priv = mf;
    file->read = macFileRead;
    file->write = macFileWrite;
    file->flush = macFileFlush;
    file->close = macFileClose;
    file->tell = macFileTell;
    file->seek = macFileSeek;
    file->eof = macFileEof;
    file->sizeAdvise = macFileSizeAdvise;
    file->error = macFileError;
    file->clearError = macFileClearError;
    file->write2read = macFileWrite2Read;
    file->cfb = macFileCfb;
    return file;
error:
    if (mf != NULL)
    {
        if (mf->fileRef != NULL)
            pgpFreeFileRef(mf->fileRef);
        pgpFree(mf);
    }
    if (file != NULL)
        pgpFree(file);
    if (errorCode != NULL)
    {
        pgpAssertAddrValid(errorCode, PGPError);
        *errorCode = result;
    }
    return NULL;
}

    PGPError
pgpMacCalcFileSize(
    PGPFileRef const *  fileRef,
    PGPFileOpenFlags    flags,
    size_t *            fileSize)
{
    FSSpec      spec;
    HFileParam  pb;
    Boolean     macBinMode;
    PGPError    result = PGPERR_OK;

    *fileSize = 0;  /* In case there's an error */

    if ((result = pgpFSSpecFromFileRef(fileRef, &spec)) != PGPERR_OK)
        return result;

    pb.ioNamePtr = spec.name;
    pb.ioVRefNum = spec.vRefNum;
    pb.ioDirID = spec.parID;
    pb.ioFDirIndex = 0;
    if (PBHGetFInfoSync((HParmBlkPtr)&pb) != noErr)
        return pgpErrorFromMacError(pb.ioResult, PGPERR_NO_FILE);

    macBinMode = FALSE;
    if (flags & kPGPFileOpenMaybeLocalEncode)
    {
        int   i;

        macBinMode = TRUE;
        for (i = 0; nonMacBinaryTypes[i] != 0; i++)
            if (pb.ioFlFndrInfo.fdType == nonMacBinaryTypes[i])
            {
                macBinMode = FALSE;
                break;
            }
    }
    else if (flags & kPGPFileOpenForceLocalEncode)
        macBinMode = TRUE;

    if (macBinMode)
    {
        *fileSize = 128 +                /* MacBinary header */
            ((pb.ioFlLgLen + 127) & ~127L) +  /* Data fork */
            ((pb.ioFlRLgLen + 127) & ~127L);  /* Resource fork */
    }
    else
        *fileSize = pb.ioFlLgLen;

    return PGPERR_OK;
}

/*
 * Local Variables:
 * tab-width: 4
 * End:
 * vi: ts=4 sw=4
 * vim: si
 */
