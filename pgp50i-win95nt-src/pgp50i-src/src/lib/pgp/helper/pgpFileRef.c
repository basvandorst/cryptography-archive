/*
 * pgpFileRef.c -- Platform independent filename handling module
 *
 * Copyright (C) 1997 Pretty Good Privacy, Inc. All rights reserved.
 *
 * $Id: pgpFileRef.c,v 1.35.2.4 1997/06/07 09:50:05 mhw Exp $
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#if HAVE_UNISTD_H
#include <unistd.h>
#endif

#if HAVE_PATHS_H
#include <paths.h>
#endif

#if UNIX || WIN32
#include <sys/types.h>
#include <sys/stat.h>
#if HAVE_FCNTL_H
#include <fcntl.h>
#endif
#endif

#include <errno.h>
#include <stdio.h>
#include <string.h>

#if MACINTOSH
#include <unix.h>
#endif

#include "pgpFileRef.h"
#include "pgpFileNames.h"
#include "pgpErr.h"
#include "pgpTypes.h"
#include "pgpDebug.h"
#include "pgpMem.h"

#if MACINTOSH
#include "MacFiles.h"
#include "MacStrings.h"
#include "pgpMacFile.h"
#include "pgpMacUtils.h"
#endif

#if !defined(_PATH_TMP) && UNIX && HAVE_MKSTEMP
#define _PATH_TMP "/tmp/"
#endif

#define kPGPFileRefMagic         0x251DEA9B
#define kPGPExternalFileRefMagic 0xD8AB1093

typedef enum PGPFileRefType_
{
    kPGPFileRefFullPath = 1,
    kPGPFileRefMacV1
} PGPFileRefType;

struct PGPFileRef_
{
    ulong           magic;
    size_t          size;
    PGPFileRefType  type;
};

#define pgpFileRefData(fileRef, type)  ((type *)((fileRef) + 1))

#define pgpaPGPFileRefValid(fileRef)                                \
        pgpaAddrValid(fileRef, PGPFileRef),                         \
        pgpaAssert(fileRef->magic == kPGPFileRefMagic)

#define pgpaPGPFileTypeValid(fileType)                              \
        pgpaAssert(fileType > kPGPFileTypeNone                      \
        && fileType < kPGPFileTypeNumTypes)

/* These must be kept in sync with the PGPFileType decl in pgpFileRef.h */

#if MACINTOSH  /* [ */

static struct MacFileTypeEntry
{
    OSType  creator, type;
} sMacFileTypeTable[kPGPFileTypeNumTypes] = {
    { 0L, 0L },                                  /* kPGPFileTypeNone */
    { 'pgpK', 'pref' },                          /* kPGPFileTypePrefs */
    { 'pgpK', 'pgPR' },                          /* kPGPFileTypePubRing */
    { 'pgpK', 'pgRR' },                          /* kPGPFileTypePrivRing */
    { 'pgpM', kPGPMacFileTypeDetachedSig },      /* kPGPFileTypeDetachedSig */
    { 'pgpK', 'pgRS' },                          /* kPGPFileTypeRandomSeed */
    { 'pgpK', 'TEXT' },                          /* kPGPFileTypeExportedKeys */
    { 'pgpM', kPGPMacFileTypeArmorFile },        /* kPGPFileTypeArmorFile */
    { 'pgpM', kPGPMacFileTypeEncryptedData },    /* kPGPFileTypeEncryptedData */
/* XXX: The below should be user preferences */
    { 'ttxt', 'TEXT' },                          /* kPGPFileTypeDecryptedText */
    { '????', 'BINA' },                          /* kPGPFileTypeDecryptedBin */
    { 'pgpM', kPGPMacFileTypeSignedData }        /* kPGPFileTypeSignedData */
};

#endif  /* ] MACINTOSH */

#if UNIX  /* [ */

static struct UnixFileTypeEntry
{
    mode_t  mask;  /* umask value for permissions */
} sUnixFileTypeTable[kPGPFileTypeNumTypes] = {
    { 0 },     /* kPGPFileTypeNone */
    { 0077 },  /* kPGPFileTypePrefs */
    { 0077 },  /* kPGPFileTypePubRing */
    { 0077 },  /* kPGPFileTypePrivRing */
    { 0077 },  /* kPGPFileTypeDetachedSig */
    { 0077 },  /* kPGPFileTypeRandomSeed */
    { 0077 },  /* kPGPFileTypeExportedKeys */
    { 0077 },  /* kPGPFileTypeArmorFile */
    { 0077 },  /* kPGPFileTypeEncryptedData */
    { 0077 },  /* kPGPFileTypeDecryptedText */
    { 0077 },  /* kPGPFileTypeDecryptedBin */
    { 0077 }  /* kPGPFileTypeSignedData */
};

#endif  /* ] UNIX */

    PGPFileRef *
pgpNewFileRefFromFullPath(
    char const *        path)
{
    PGPFileRef *        fileRef;
    size_t              size = sizeof(PGPFileRef) + strlen(path) + 1;

    fileRef = (PGPFileRef *)pgpAlloc(size);
    if (fileRef)
    {
        fileRef->magic = kPGPFileRefMagic;
        fileRef->size = size;
        fileRef->type = kPGPFileRefFullPath;
        strcpy(pgpFileRefData(fileRef, char), path);
    }
    return fileRef;
}

    PGPError
pgpFullPathFromFileRef(
    PGPFileRef const *  fileRef,
    char **             fullPathPtr)
{
    pgpa(pgpaPGPFileRefValid(fileRef));

    *fullPathPtr = NULL;
    switch (fileRef->type)
    {
        case kPGPFileRefFullPath:
        {
            char const *  path;
            size_t        size;

            path = pgpFileRefData(fileRef, char);
            size = strlen(path) + 1;
            *fullPathPtr = (char *)pgpAlloc(size);
            pgpCopyMemory(path, *fullPathPtr, size);
            break;
        }
#if MACINTOSH  /* [ */
        case kPGPFileRefMacV1:
        {
            uchar    path[256];

            if (FSpGetFullPath(pgpFileRefData(fileRef, FSSpec), path) != noErr)
                return PGPERR_NO_FILE;
            *fullPathPtr = (char *)pgpAlloc(path[0] + 1);
            pgpCopyMemory(path + 1, *fullPathPtr, path[0]);
            (*fullPathPtr)[path[0]] = '\0';
            break;
        }
#endif  /* ] MACINTOSH */
        default:
            pgpAssertMsg(0, "Unsupported PGPFileRef type");
            return PGPERR_NO_FILE;
    }
    return PGPERR_OK;
}

#if MACINTOSH  /* [ */

    PGPFileRef *
pgpNewFileRefFromFSSpec(
    FSSpec const *      spec)
{
    PGPFileRef *        fileRef;
    size_t              size = sizeof(PGPFileRef) + sizeof(FSSpec);

    fileRef = (PGPFileRef *)pgpAlloc(size);
    if (fileRef)
    {
        fileRef->magic = kPGPFileRefMagic;
        fileRef->size = size;
        fileRef->type = kPGPFileRefMacV1;
        pgpCopyMemory(spec, pgpFileRefData(fileRef, FSSpec), sizeof(FSSpec));
    }
    return fileRef;
}

    PGPError
pgpFSSpecFromFileRef(
    PGPFileRef const *  fileRef,
    FSSpec *            spec)
{
    pgpa(pgpaPGPFileRefValid(fileRef));

    switch (fileRef->type)
    {
#ifdef NOTYET
        case kPGPFileRefFullPath:
            break;
#endif
        case kPGPFileRefMacV1:
            pgpCopyMemory(pgpFileRefData(fileRef, FSSpec), spec,
                          sizeof(*spec));
            break;
        default:
            pgpAssertMsg(0, "Unsupported PGPFileRef type");
            return PGPERR_NO_FILE;
    }
    return PGPERR_OK;
}

#endif  /* ] MACINTOSH */

    PGPFileRef *
pgpCopyFileRef(
    PGPFileRef const *  fileRef)
{
    PGPFileRef *        newFileRef;

    pgpa(pgpaPGPFileRefValid(fileRef));

    /*
     * XXX In the future if PGPFileRefs contain pointers,
     *     this will need to be changed.
     */
    newFileRef = (PGPFileRef *)pgpAlloc(fileRef->size);
    if (newFileRef != NULL)
        pgpCopyMemory(fileRef, newFileRef, fileRef->size);
    return newFileRef;
}

    void
pgpFreeFileRef(
    PGPFileRef *    fileRef)
{
    pgpa(pgpaPGPFileRefValid(fileRef));

    pgpClearMemory((void *)fileRef, fileRef->size);
    pgpFree(fileRef);
}

/*
 * pgpGetFileRefName returns a string allocated with pgpAlloc(),
 * which the caller is responsible for freeing.
 */
    char *
pgpGetFileRefName(
    PGPFileRef const *  fileRef)
{
    char const *        name;
    size_t              nameLen;
    char *              allocName;

    pgpa(pgpaPGPFileRefValid(fileRef));

    switch (fileRef->type)
    {
        case kPGPFileRefFullPath:
        {
            char const *  path;

            path = pgpFileRefData(fileRef, char);
            name = (char const *)pgpFileNameTail(path);
            nameLen = strlen(name);
            break;
        }
#if MACINTOSH  /* [ */
        case kPGPFileRefMacV1:
        {
            FSSpec const *  spec;

            spec = pgpFileRefData(fileRef, FSSpec);
            name = (char const *)spec->name + 1;
            nameLen = spec->name[0];
            break;
        }
#endif  /* ] MACINTOSH */
        default:
            pgpAssertMsg(0, "Unsupported PGPFileRef type");
            return NULL;
    }

    allocName = (char *)pgpAlloc(nameLen + 1);
    if (allocName != NULL)
    {
        pgpCopyMemory(name, allocName, nameLen);
        allocName[nameLen] = '\0';
    }
    return allocName;
}

    PGPError
pgpSetFileRefName(
    PGPFileRef **       fileRef1,
    char const *        newName)
{
    PGPFileRef *        fileRef = *fileRef1;
    PGPError            result = PGPERR_OK;
    size_t              nameLen;

    pgpa((
        pgpaPGPFileRefValid(fileRef),
        pgpaStrValid(newName)));

    nameLen = strlen(newName);
    switch (fileRef->type)
    {
        case kPGPFileRefFullPath:
        {
            char *    path;
            size_t    nameOffset;
            size_t    newSize;

            path = pgpFileRefData(fileRef, char);
            nameOffset = pgpFileNameTail(path) - path;

            newSize = sizeof(PGPFileRef) + nameOffset + nameLen + 1;
            result = pgpRealloc((void **)fileRef1, newSize);
            fileRef = *fileRef1;

            if (result == PGPERR_OK)
            {
                path = pgpFileRefData(fileRef, char);
                strcpy(path + nameOffset, newName);
                fileRef->size = newSize;
            }
            break;
        }
#if MACINTOSH  /* [ */
        case kPGPFileRefMacV1:
        {
            FSSpec *  spec;

            if (nameLen >= 64)
                return PGPERR_KEYDB_VALUETOOLONG;
            spec = pgpFileRefData(fileRef, FSSpec);
            spec->name[0] = nameLen;
            pgpCopyMemory(newName, (char *)spec->name + 1, nameLen);
            break;
        }
#endif /* ] MACINTOSH */
        default:
            pgpAssertMsg(0, "Unsupported PGPFileRef type");
            return PGPERR_BADPARAM;
    }
    return result;
}

    uchar *
pgpExportFileRef(
    PGPFileRef const *  fileRef,
    size_t *            size)
{
    PGPFileRef *        externFileRef = NULL;
    size_t              externSize;

    pgpa((
        pgpaPGPFileRefValid(fileRef),
        pgpaAddrValid(size, size_t)));

    *size = 0;

    /*
     * XXX In the future if PGPFileRefs contain pointers,
     *     this will need to be changed.
     */
    switch (fileRef->type)
    {
        case kPGPFileRefFullPath:
            externSize = fileRef->size;
            externFileRef = (PGPFileRef *)pgpAlloc(externSize);
            if (externFileRef != NULL)
            {
                pgpCopyMemory(fileRef, externFileRef, externSize);
                externFileRef->magic = kPGPExternalFileRefMagic;
                *size = externSize;
            }
            break;
#if MACINTOSH  /* [ */
        case kPGPFileRefMacV1:
        {
            FSSpec const *  spec;
            uchar           volumeName[256];
            HVolumeParam    pb;

            spec = pgpFileRefData(fileRef, FSSpec);
            pb.ioNamePtr = volumeName;
           pb.ioVRefNum = spec->vRefNum;
           pb.ioVolIndex = 0;
           if (PBHGetVInfoSync((HParmBlkPtr)&pb) == noErr)
           {
               externSize = fileRef->size + volumeName[0] + 1;
               externFileRef = (PGPFileRef *)pgpAlloc(externSize);
               if (externFileRef != NULL)
               {
                   pgpCopyMemory(fileRef, externFileRef, fileRef->size);
                   pgpCopyMemory(volumeName,
                                 (uchar *)externFileRef + fileRef->size,
                                 volumeName[0] + 1);
                   externFileRef->magic = kPGPExternalFileRefMagic;
                   *size = externSize;
               }
           }
           break;
        }
#endif  /* ] MACINTOSH */
        default:
            pgpAssertMsg(0, "Unsupported PGPFileRef type");
            break;
    }
    return (uchar *)externFileRef;
}

   PGPFileRef *
pgpImportFileRef(
   uchar const *        buffer,
   size_t               size)
{
    PGPFileRef *        fileRef;

    pgpa(pgpaAddrValid(buffer, uchar));
    fileRef = (PGPFileRef *)pgpAlloc(size);
    if (fileRef != NULL)
    {
        pgpCopyMemory(buffer, fileRef, size);
        if (size < sizeof(PGPFileRef) ||
            fileRef->magic != kPGPExternalFileRefMagic ||
            fileRef->size > size)
        {
            pgpFree(fileRef);
            return NULL;
        }
        fileRef->magic = kPGPFileRefMagic;
        pgpa(pgpaPGPFileRefValid(fileRef));

        /*
         * XXX In the future if PGPFileRefs contain pointers,
         *     this will need to be changed.
         */
        switch (fileRef->type)
        {
            case kPGPFileRefFullPath:
                break;
#if MACINTOSH  /* [ */
            case kPGPFileRefMacV1:
            {
                FSSpec *        spec;
                uchar           volumeName[256];
                uchar *         storedVolName;
                HVolumeParam    pb;

                spec = pgpFileRefData(fileRef, FSSpec);
                storedVolName = (uchar *)(spec + 1);
                pgpAssert(size >= sizeof(PGPFileRef) +
                          sizeof(FSSpec) +
                          storedVolName[0] + 1);
                /*
                 * Check to see if the vRefNum stored in the
                 * fileRef has the correct volume name.
                 */
                pb.ioNamePtr = volumeName;
                pb.ioVRefNum = spec->vRefNum;
                pb.ioVolIndex = 0;
                if (PBHGetVInfoSync((HParmBlkPtr)&pb) != noErr ||
                    !PStringsAreEqual(volumeName, storedVolName))
                {
                    /*
                     * The volume name didn't match. Try to look up
                     * the vRefNum from the volume name.
                     */
                    CopyPString(storedVolName, volumeName);
                    volumeName[++volumeName[0]] = ':';
                    pb.ioNamePtr = volumeName;
                    pb.ioVRefNum = -1;
                    pb.ioVolIndex = -1;
                    if (PBHGetVInfoSync((HParmBlkPtr)&pb) != noErr ||
                        !PStringsAreEqual(volumeName, storedVolName))
                    {
                        /*
                         * It couldn't find the volume by name. If
                         * it used to be the boot volume (vRefNum == -1),
                         * use the current boot volume. Otherwise, fail.
                         */
                        if (spec->vRefNum != -1)
                        {
                            pgpFree(fileRef);
                            return NULL;
                        }
                    }
                    else
                        spec->vRefNum = pb.ioVRefNum;
                }
                break;
            }
#endif  /* ] MACINTOSH */
            default:
                pgpAssertMsg(0, "Unsupported PGPFileRef type");
                pgpFree(fileRef);
                return NULL;
        }
    }
    if (fileRef != NULL)
        pgpRealloc((void **)&fileRef, fileRef->size);
    return fileRef;
}

#if MACINTOSH  /* [ */

    PGPError
pgpCalcFileSize(
    PGPFileRef const *  fileRef,
    PGPFileOpenFlags    flags,
    size_t *            fileSize)
{
    return pgpMacCalcFileSize(fileRef, flags, fileSize);
}

    PGPError
pgpCreateFile(
    PGPFileRef const *  fileRef,
    PGPFileType         fileType)
{
    PGPError            result = PGPERR_OK;
    OSErr               macResult;
    FSSpec              spec;

    pgpa((
        pgpaPGPFileRefValid(fileRef),
        pgpaPGPFileTypeValid(fileType)));

    if ((result = pgpFSSpecFromFileRef(fileRef, &spec)) == PGPERR_OK)
    {
        if ((macResult = HCreate(spec.vRefNum, spec.parID, spec.name,
                                 sMacFileTypeTable[fileType].creator,
                                 sMacFileTypeTable[fileType].type)) != noErr)
            result = pgpErrorFromMacError(macResult, PGPERR_NO_FILE);
    }
    return result;
}

    PGPError
pgpDeleteFile(
    PGPFileRef const *  fileRef)
{
    PGPError            result = PGPERR_OK;
    OSErr               macResult;
    FSSpec              spec;

    pgpa(pgpaPGPFileRefValid(fileRef));

    if ((result = pgpFSSpecFromFileRef(fileRef, &spec)) == PGPERR_OK)
    {
        if ((macResult = HDelete(spec.vRefNum, spec.parID,
                                 spec.name)) != noErr)
            result = pgpErrorFromMacError(macResult, PGPERR_NO_FILE);
    }
    return result;
}

    PGPError
pgpRenameFile(
    PGPFileRef const *  oldRef,
    PGPFileRef const *  newRef)
{
    PGPError            result = PGPERR_OK;
    OSErr               macResult;
    FSSpec              oldSpec;
    FSSpec              newSpec;

    pgpa((
        pgpaPGPFileRefValid(oldRef),
        pgpaPGPFileRefValid(newRef),
        pgpaAssert(oldRef->type == newRef->type)));

    if ((result = pgpFSSpecFromFileRef(oldRef, &oldSpec)) == PGPERR_OK &&
        (result = pgpFSSpecFromFileRef(newRef, &newSpec)) == PGPERR_OK)
    {
        pgpAssert(oldSpec.vRefNum == newSpec.vRefNum &&
                  oldSpec.parID == newSpec.parID);
        if ((macResult = HRename(oldSpec.vRefNum, oldSpec.parID,
                                 oldSpec.name, newSpec.name)) != noErr)
            result = pgpErrorFromMacError(macResult, PGPERR_NO_FILE);
    }
    return result;
}

    FILE *
pgpStdIOOpenTempFile(
    PGPFileRef **       tempFileRef,
    PGPError *          errorCode)
{
    FSSpec              spec;
    FILE *              stdIOFile = NULL;
    PGPError            result = PGPERR_OK;
    OSErr               macResult;
    long                i;

    pgpa(pgpaAddrValid(tempFileRef, PGPFileRef *));

    *tempFileRef = NULL;
    if ((macResult = FindFolder(kOnSystemDisk, kTemporaryFolderType,
                                kCreateFolder, &spec.vRefNum,
                                &spec.parID)) != noErr)
        goto macError;

    for (i = 0; ; i++)
    {
        pgpFormatPStr(spec.name, sizeof(spec.name), 0, "PGP Temp %d", i);
        macResult = HCreate(spec.vRefNum, spec.parID, spec.name,
                            '????', 'BINA');
        if (macResult == noErr)
            break;
        else if (macResult != dupFNErr)
            goto macError;
    }

    if ((*tempFileRef = pgpNewFileRefFromFSSpec(&spec)) == NULL)
    {
        result = PGPERR_NOMEM;
        goto error;
    }

    return pgpFileRefStdIOOpen(*tempFileRef, kPGPFileOpenStdUpdateFlags,
                               kPGPFileTypeDecryptedBin, errorCode);

macError:
    result = pgpErrorFromMacError(macResult, PGPERR_NO_FILE);
error:
    if (*tempFileRef != NULL)
    {
        pgpFreeFileRef(*tempFileRef);
        *tempFileRef = NULL;
    }
    if (errorCode != NULL)
    {
        pgpAssertAddrValid(errorCode, PGPError);
        *errorCode = result;
    }
    return NULL;
}

#else  /* ] MACINTOSH [ */

    PGPError
pgpCalcFileSize(
    PGPFileRef const *  fileRef,
    PGPFileOpenFlags    flags,
    size_t *            fileSize)
{
    PGPError            result = PGPERR_OK;
    char *              fullPath = NULL;
    struct stat         statBuf;

    pgpa((
        pgpaPGPFileRefValid(fileRef),
        pgpaAddrValid(fileSize, size_t)));

    (void)flags;    /* Avoid warning */
    *fileSize = 0;  /* In case there's an error */

    if ((result = pgpFullPathFromFileRef(fileRef, &fullPath)) == PGPERR_OK)
    {
        /* XXX Maybe make this more portable. Does this work on MSDOS? */
        if (stat(fullPath, &statBuf) != 0)
            result = PGPERR_NO_FILE;
        else if ((S_IFMT & statBuf.st_mode) == S_IFREG)
            *fileSize = statBuf.st_size;
        /* XXX Maybe return a special result if it's not a regular file */
    }
    pgpMemFree(fullPath);
    return result;
}

    PGPError
pgpCreateFile(
    PGPFileRef const *  fileRef,
    PGPFileType         fileType)
{
    PGPError            result = PGPERR_OK;
    char *              fullPath = NULL;

    pgpa((
        pgpaPGPFileRefValid(fileRef),
        pgpaPGPFileTypeValid(fileType)));

    if ((result = pgpFullPathFromFileRef(fileRef, &fullPath)) == PGPERR_OK)
    {
#if UNIX  /* [ */
        int     fileDes;

        /*
         * XXX Beware that calling this routine could be a security flaw,
         *     because there is a window between creating and opening
         *     the file, in between which the file might have been moved
         *     out of the way, thus possibly having you open a new file
         *     with the wrong permissions.
         */
        fileDes = open(fullPath,
                       O_WRONLY | O_CREAT | O_EXCL,
                       sUnixFileTypeTable[fileType].mask);
        if (fileDes > 0)
            close(fileDes);
        else
            result = PGPERR_NO_FILE;  /* XXX Use better error */
#else  /* ] UNIX [ */
        FILE *    stdIOFile;

        stdIOFile = fopen(fullPath, "ab+");
        if (stdIOFile != NULL)
            fclose(stdIOFile);
        else
            result = PGPERR_NO_FILE;  /* XXX Use better error */
#endif  /* ] UNIX */
    }
    pgpMemFree(fullPath);
    return result;
}

    PGPError
pgpDeleteFile(
    PGPFileRef const *  fileRef)
{
    PGPError            result = PGPERR_OK;
    char *              fullPath = NULL;

    pgpa(pgpaPGPFileRefValid(fileRef));

    if ((result = pgpFullPathFromFileRef(fileRef, &fullPath)) == PGPERR_OK)
    {
        if (remove(fullPath) != 0)
            result = PGPERR_NO_FILE;  /* XXX Use better error */
    }
    pgpMemFree(fullPath);
    return result;
}

    PGPError
pgpRenameFile(
    PGPFileRef const *  oldRef,
    PGPFileRef const *  newRef)
{
    PGPError            result = PGPERR_OK;
    char *              oldPath = NULL;
    char *              newPath = NULL;

    pgpa((
        pgpaPGPFileRefValid(oldRef),
        pgpaPGPFileRefValid(newRef),
        pgpaAssert(oldRef->type == newRef->type)));

    if ((result = pgpFullPathFromFileRef(oldRef, &oldPath)) == PGPERR_OK &&
        (result = pgpFullPathFromFileRef(newRef, &newPath)) == PGPERR_OK)
    {
        size_t      nameOffset;

        nameOffset = pgpFileNameTail(oldPath) - oldPath;
        pgpa((
            pgpaAssert(newPath + nameOffset == pgpFileNameTail(newPath)),
            pgpaAssert(pgpMemoryEqual(oldPath, newPath, nameOffset))));
        if (rename(oldPath, newPath) != 0)
            result = PGPERR_NO_FILE;    /* XXX Improve error */
    }
    pgpMemFree(oldPath);
    pgpMemFree(newPath);
    return result;
}

    FILE *
pgpStdIOOpenTempFile(
    PGPFileRef **       tempFileRef,
    PGPError *          errorCode)
{
    char                fileName[L_tmpnam];
    PGPError            result = PGPERR_OK;
    FILE *              stdIOFile = NULL;

    pgpa(pgpaAddrValid(tempFileRef, PGPFileRef *));

    *tempFileRef = NULL;

#if defined(_PATH_TMP) && HAVE_MKSTEMP  /* [ */

    {
        int     fd = -1;

        strncpy(fileName, _PATH_TMP "ptmpXXXXXX", sizeof(fileName));
        fileName[sizeof(fileName) - 1] = '\0';
        if ((fd = mkstemp(fileName)) == -1 ||
            (stdIOFile = fdopen(fd, "w+b")) == NULL)
        {
            if (fd != -1)
                close(fd);
            goto fileError;
        }
        if ((*tempFileRef = pgpNewFileRefFromFullPath(fileName)) == NULL)
        {
            result = PGPERR_NOMEM;
            goto error;
        }
    }

#else  /* ] defined(_PATH_TMP) && HAVE_MKSTEMP [ */

    if (tmpnam(fileName) != fileName)
        goto fileError;

    if ((*tempFileRef = pgpNewFileRefFromFullPath(fileName)) == NULL)
    {
        result = PGPERR_NOMEM;
        goto error;
    }

    /* XXX Perhaps check for an error and retry if necessary */
    stdIOFile = pgpFileRefStdIOOpen(*tempFileRef, kPGPFileOpenStdUpdateFlags,
                                    kPGPFileTypeDecryptedBin, errorCode);

#endif  /* ] defined(_PATH_TMP) && HAVE_MKSTEMP */

    if (stdIOFile == NULL)
    {
    fileError:
        result = PGPERR_NO_FILE;
        goto error;
    }

    return stdIOFile;

error:
    if (*tempFileRef != NULL)
    {
        pgpFreeFileRef(*tempFileRef);
        *tempFileRef = NULL;
    }
    if (errorCode != NULL)
    {
        pgpAssertAddrValid(errorCode, PGPError);
        *errorCode = result;
    }
    return NULL;
}

#endif  /* ] MACINTOSH */

    PgpFile *
pgpFileRefOpen(
    PGPFileRef const *  fileRef,
    PGPFileOpenFlags    flags,
    PGPFileType         fileType,
    PGPError *          errorCode)
{
    FILE *              stdIOFile;

#if MACINTOSH  /* [ */
        switch (flags & (kPGPFileOpenReadWritePerm |
                         kPGPFileOpenAppend | kPGPFileOpenTruncate |
                         kPGPFileOpenCreate | kPGPFileOpenTextMode))
        {
            case kPGPFileOpenReadPerm:
                return pgpFileRefMacReadOpen(fileRef, flags, errorCode);
            case kPGPFileOpenStdWriteFlags:
                return pgpFileRefMacWriteOpen(fileRef, fileType, flags,
                                              errorCode);
            default:
                break;
        }
        pgpAssertMsg((flags & (kPGPFileOpenMaybeLocalEncode |
                               kPGPFileOpenForceLocalEncode)) == 0,
               "This combination of flags not currently supported");
#endif  /* ] MACINTOSH */

    stdIOFile = pgpFileRefStdIOOpen(fileRef, flags, fileType, errorCode);
    if (stdIOFile == NULL)
        return NULL;

    switch (flags & kPGPFileOpenReadWritePerm)
    {
        case kPGPFileOpenReadPerm:
            return pgpFileReadOpen(stdIOFile, NULL, NULL);
        case kPGPFileOpenWritePerm:
            return pgpFileWriteOpen(stdIOFile, NULL);
        default:
            pgpAssertMsg(0, "No support for simultaneous read/write yet");
            break;
    }
    fclose(stdIOFile);
    return NULL;
}

    PgpFileRead *
pgpFileRefReadCreate(
    PGPFileRef const *  fileRef,
    PGPFileOpenFlags    flags,
    PGPError *          errorCode)
{
    PgpFile *           pgpFile;
    PgpFileRead *       pgpFileRead;

    if ((pgpFile = pgpFileRefOpen(fileRef, flags,
                                  kPGPFileTypeNone, errorCode)) == NULL)
    return NULL;

    if ((pgpFileRead = pgpPGPFileReadCreate(pgpFile, TRUE)) == NULL)
    {
        pgpFileClose(pgpFile);
        return NULL;
    }
    return pgpFileRead;
}

/* XXX Compatibility routines follow, intended to be phased out */

/*
 * pgpFileRefStdIOOpen is currently the heart of the implementation
 * for opening files, but this may change later.
 */

    FILE *
pgpFileRefStdIOOpen(
    PGPFileRef const *  fileRef,
    PGPFileOpenFlags    flags,
    PGPFileType         fileType,
    PGPError *          errorCode)
{
    char        mode[5];
    FILE *      stdIOFile = NULL;
    PGPError    result = PGPERR_OK;

#if DEBUG
    pgpa(pgpaPGPFileRefValid(fileRef));
    if (flags & kPGPFileOpenCreate)
        pgpa(pgpaPGPFileTypeValid(fileType));
#endif

    switch (flags & ~(kPGPFileOpenTextMode |
                      kPGPFileOpenMaybeLocalEncode |
                      kPGPFileOpenForceLocalEncode |
                      kPGPFileOpenLocalEncodeHashOnly |
                      kPGPFileOpenNoMacBinCRCOkay))
    {
        case kPGPFileOpenReadPerm:
            strcpy(mode, "r");
            break;
        case kPGPFileOpenStdWriteFlags:
            strcpy(mode, "w");
            break;
        case kPGPFileOpenStdAppendFlags:
            strcpy(mode, "a");
            break;
        case kPGPFileOpenReadWritePerm:
            strcpy(mode, "r+");
            break;
        case kPGPFileOpenStdUpdateFlags:
            strcpy(mode, "w+");
            break;
        case kPGPFileOpenStdAppendFlags | kPGPFileOpenReadPerm:
            strcpy(mode, "a+");
            break;
        default:
            pgpAssertMsg(0, "Unsupported open mode");
            result = PGPERR_BADPARAM;
            goto error;
    }
    if (!(flags & kPGPFileOpenTextMode))
        strcat(mode, "b");

#if WIN32
    strcat (mode, "c");     /* Commit-to-disk mode, flushes writes out */
#endif

#if MACINTOSH  /* [ */
    {
        FSSpec   spec;
        short    refNum;
        SInt8    permission;
        OSErr    macResult;

        if (pgpFSSpecFromFileRef(fileRef, &spec) == PGPERR_OK)
        {
            if (flags & kPGPFileOpenCreate)
            {
                macResult = HCreate(spec.vRefNum, spec.parID, spec.name,
                                    sMacFileTypeTable[fileType].creator,
                                    sMacFileTypeTable[fileType].type);
                if (macResult != noErr && macResult != dupFNErr)
                {
                    result = pgpErrorFromMacError(macResult, PGPERR_NO_FILE);
                    goto error;
                }
            }
            switch (flags & kPGPFileOpenReadWritePerm)
            {
                case kPGPFileOpenReadPerm:
                    permission = fsRdPerm;
                    break;
                case kPGPFileOpenWritePerm:
                    permission = fsWrPerm;
                    break;
                case kPGPFileOpenReadWritePerm:
                    permission = fsRdWrPerm;
                    break;
                default:
                    pgpAssertMsg(0, "Unsupported open mode");
                    result = PGPERR_BADPARAM;
                    goto error;
            }
            if ((macResult = FSpOpenDF(&spec, permission, &refNum)) != noErr)
            {
                result = pgpErrorFromMacError(macResult, PGPERR_NO_FILE);
                goto error;
            }
            stdIOFile = fdopen(refNum, mode);
            if (stdIOFile == NULL)
                FSClose(refNum);
        }
    }
#else  /* ] MACINTOSH [ */
    {
        char *    fullPath = NULL;
#if UNIX
        mode_t    oldMask;
#endif

        if (pgpFullPathFromFileRef(fileRef, &fullPath) == PGPERR_OK)
        {
#if UNIX
            oldMask = umask(sUnixFileTypeTable[fileType].mask);
#endif
            stdIOFile = fopen(fullPath, mode);
            if (stdIOFile == NULL)
            {
                if (errno == EACCES
#ifdef EROFS
                    || errno == EROFS
#endif
                   )
                {
                    result = PGPERR_FILE_PERMISSIONS;
                }
                else
                {
                    result = PGPERR_NO_FILE;
                }
            }
#if UNIX
            umask(oldMask);
#endif
            pgpMemFree(fullPath);
            if (stdIOFile == NULL)
                goto error;
        }
    }
#endif  /* ] MACINTOSH */

    if (stdIOFile == NULL)
    {
        result = PGPERR_NO_FILE;
        goto error;
    }
    return stdIOFile;

error:
    if (errorCode != NULL)
    {
        pgpAssertAddrValid(errorCode, PGPError);
        *errorCode = result;
    }
    return NULL;
}


/*
 * Due to the way memory is handled on WIN32 systems with static linking,
 * fclose must be called from the same library which called fopen.
 * This function servers that purpose. It should be called by external
 * libraries and apps to close FILE * handles returned by this module.
 */
    PGPError
pgpStdIOClose(
    FILE *      stdIOFile)
{
    if (fclose (stdIOFile)) {
        return PGPERR_FILE_OPFAIL;
    }
    return PGPERR_OK;
}


    PgpFile *
pgpFileRefReadOpen(
    PGPFileRef const *  fileRef,
    PgpUICb const *     ui,
    void *              ui_arg,
    PGPError *          errorCode)
{
    pgpAssert(ui == NULL && ui_arg == NULL);
    (void)ui;    /* Avoid warnings */
    (void)ui_arg;

    return pgpFileRefOpen(fileRef, kPGPFileOpenReadPerm, kPGPFileTypeNone,
                          errorCode);
}

    PgpFile *
pgpFileRefWriteOpen(
    PGPFileRef const *  fileRef,
    PGPFileType         fileType,
    PgpCfbContext *     cfb,
    PGPError *          errorCode)
{
    pgpAssert(cfb == NULL);
    (void)cfb;    /* Avoid warning */

    return pgpFileRefOpen(fileRef, kPGPFileOpenStdWriteFlags, fileType,
                          errorCode);
}

    PgpFile *
pgpFileRefProcWriteOpen(
    PGPFileRef const *  fileRef,
    PGPFileType         fileType,
    int                 (*doClose)  (FILE *file, void *arg),
    void *              arg,
    PGPError *          errorCode)
{
    FILE *      stdIOFile;
    PgpFile *   pgpFile;

    if ((stdIOFile = pgpFileRefStdIOOpen(fileRef, kPGPFileOpenStdWriteFlags,
                                         fileType, errorCode)) == NULL)
        return NULL;

    if ((pgpFile = pgpFileProcWriteOpen(stdIOFile, doClose, arg)) == NULL)
    {
        fclose(stdIOFile);
        if (errorCode != NULL)
        {
            pgpAssertAddrValid(errorCode, PGPError);
            *errorCode = PGPERR_NO_FILE;
        }
        return NULL;
    }

    return pgpFile;
}

/*
 * Local Variables:
 * tab-width: 4
 * End:
 * vi: ts=4 sw=4
 * vim: si
 */
