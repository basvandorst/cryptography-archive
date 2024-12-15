/*
 * pgpLeaks.h -- Leak tracking module for memory allocation routines
 *
 * Copyright (C) 1996,1997 Pretty Good Privacy, Inc. All rights reserved.
 *
 * $Id: pgpLeaks.h,v 1.11.2.1 1997/06/07 09:50:08 mhw Exp $
 */

#ifndef PGPLEAKS_H
#define PGPLEAKS_H

#if PRAGMA_IMPORT_SUPPORTED
#pragma import on
#endif

#include "pgpTypes.h"
#include "pgpDebug.h"

#ifndef USE_PGP_LEAKS
#define USE_PGP_LEAKS  DEBUG
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if USE_PGP_LEAKS  /* [ */

 /*
 * WARNING: The leaks library is currently not thread-safe
 */

#define kLeakDeallocTypeMagic  0xD073A915
#define kLeakAllocTypeMagic    0xE19C834B
#define kLeakItemMagic         0xB74D951F

typedef struct LeakDeallocType_ const  LeakDeallocType;
struct LeakDeallocType_
{
    ulong        magic;
    char const *    name;
};

typedef struct LeakAllocType_ const LeakAllocType;
struct LeakAllocType_
{
    ulong             magic;
    char const *      name;
    LeakDeallocType * deallocType;
};

#define LeakStructName(name)  name ## __struct

#define DECLARE_LEAKDEALLOCTYPE(varName, nameStr)                 \
        LeakDeallocType LeakStructName(varName) = {               \
                            kLeakDeallocTypeMagic,                \
                            nameStr },                            \
                        *varName = &LeakStructName(varName)

#define DECLARE_LEAKALLOCTYPE(varName, nameStr, deallocType)      \
        LeakAllocType LeakStructName(varName) = {                 \
                            kLeakAllocTypeMagic,                  \
                            nameStr,                              \
                            &LeakStructName(deallocType) },       \
                        *varName = &LeakStructName(varName)

/*
 * The LeakItems form a circularly-linked list.
 * Each session has its own list, each with its own dummy element.
 */
typedef struct LeakItem_    LeakItem;
struct LeakItem_
{
    ulong           magic;
    LeakAllocType * allocType;
    ulong           size;
    Boolean         ignore;
    LeakItem *      prev;
    LeakItem *      next;
    char const *    fileName;
    long            lineNumber;
    void const *    memory;
};

#define pgpaLeakDeallocTypeValid(type)                                      \
        pgpaFailIf((type) == NULL || (type)->magic != kLeakDeallocTypeMagic,\
                    (pgpaFmtPrefix, "Invalid LeakDeallocType magic"))
#define pgpaLeakAllocTypeValid(type)                                        \
        pgpaFailIf((type) == NULL || (type)->magic != kLeakAllocTypeMagic,  \
                    (pgpaFmtPrefix, "Invalid LeakAllocType magic"))
#define pgpaLeakItemValid(item)                                             \
        pgpaFailIf((item) == NULL || (item)->magic != kLeakItemMagic,       \
                    (pgpaFmtPrefix, "Invalid LeakInfo magic"))


void PGPExport  pgpLeaksBeginSession(char const *sessionName);
void PGPExport  pgpLeaksEndSession(void);

void PGPExport  pgpLeaksRememberItem(LeakItem *item, LeakAllocType *type,
                                     void const *memory, unsigned long size,
                                     char const *fileName,
                                     long lineNumber);
void PGPExport  pgpLeaksIgnoreItem(LeakItem *item);
void PGPExport  pgpLeaksIgnoreMemory( const void * memory);
void PGPExport  pgpLeaksForgetItem(LeakItem *item, LeakDeallocType *type);

LeakItem PGPExport *  pgpLeaksFindItem(void const *memory);

/*
 * Call this whenever a LeakItem moves in memory. <oldItem> will not be
 * referenced and need not contain any valid data. It need only indicate
 * where the item used to be in memory.
 */
void PGPExport  pgpLeaksNotifyItemMoved(LeakItem const *oldItem,
                                        LeakItem *newItem);

void PGPExport  pgpLeaksSuspend(void);
void PGPExport  pgpLeaksResume(void);

#else /* ] USE_PGP_LEAKS [ */

#define pgpLeaksBeginSession(name)
#define pgpLeaksEndSession()
#define pgpLeaksRememberItem(item, type, memory, fileName, lineNumber)
#define pgpLeaksIgnoreItem(item)
#define pgpLeaksIgnoreMemory(memory)
#define pgpLeaksForgetItem(item, type)
#define pgpLeaksSuspend()
#define pgpLeaksResume()

#endif /* ] USE_PGP_LEAKS */

#ifdef __cplusplus
}
#endif

#if PRAGMA_IMPORT_SUPPORTED
#pragma import reset
#endif

#endif /* PGPLEAKS_H */

/*
 * Local Variables:
 * tab-width: 4
 * End:
 * vi: ts=4 sw=4
 * vim: si
 */
