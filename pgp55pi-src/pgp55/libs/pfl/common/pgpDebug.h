/*____________________________________________________________________________
	pgpDebug.h
	
	Copyright (C) 1996,1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	Assertion macro header

	$Id: pgpDebug.h,v 1.24 1997/09/18 22:54:49 heller Exp $
____________________________________________________________________________*/
#ifndef Included_pgpDebug_h	/* [ */
#define Included_pgpDebug_h

#include <stdlib.h>
#include <stdarg.h>

#ifdef PGP_WIN32
#include <crtdbg.h>
#elif ! PGP_MACINTOSH
#include <stdio.h>
#endif

#include "pgpTypes.h"


/*
 * pgpFixBeforeShip is a macro which forces a compile error unless
 * the UNFINISHED_CODE_ALLOWED macro is non-zero.
 * Example:  pgpFixBeforeShip("Does this really work?")
 */
#ifndef pgpFixBeforeShip
#if UNFINISHED_CODE_ALLOWED
#define pgpFixBeforeShip(msg)
#else
#define pgpFixBeforeShip(msg)	@@@@@@ #msg @@@@@@
#endif
#endif

/*
 * DEBUG_STRUCT_CONSTRUCTOR defines a null constructor for a struct
 * which initializes the structure which all 0xDDs if PGP_DEBUG is non-zero,
 * or does nothing otherwise.  It requires C++.  It shouldn't be used
 * for anything with virtual methods, because it will overwrite the
 * virtual dispatch table pointer.
 * Example:  foo { int a; DEBUG_STRUCT_CONSTRUCTOR(foo) }
 */
#if defined(__cplusplus) && PGP_DEBUG
#define DEBUG_STRUCT_CONSTRUCTOR(type)						\
		type(void)											\
		{													\
			pgpDebugFillMemory(this, sizeof(*this));		\
		}
#else
#define DEBUG_STRUCT_CONSTRUCTOR(type)
#endif

PGP_BEGIN_C_DECLARATIONS

/*
 * All the FmtMsg macros accept the following special sequences:
 *		%%	Replaced with a single '%'
 *		%c	Replaced with the char argument
 *		%s	Replaced with the C string argument
 *		%S	Replaced with the Pascal string argument
 *		%B	Replaced with the memory buffer as a string
 *				(two args: length (int), buffer)
 *		%lB	Replaced with the memory buffer as a string
 *				(two args: length (long), buffer)
 *		%d	Replaced with the signed integer value (base 10)
 *		%ld	Replaced with the signed long value (base 10)
 *		%u	Replaced with the unsigned integer value (base 10)
 *		%lu	Replaced with the unsigned long value (base 10)
 *		%x	Replaced with the unsigned integer value (base 16)
 *		%lx	Replaced with the unsigned long value (base 16)
 *		%o	Replaced with the unsigned integer value (base 8)
 *		%lo	Replaced with the unsigned long value (base 8)
 *		%b	Replaced with the unsigned integer value (base 2)
 *		%lb	Replaced with the unsigned long value (base 2)
 *		%p	Replaced with the pointer value, printed in hex
 */

#if PGP_DEBUG				/* [ */

#define PGP_DEBUG_BUF_SIZE		256

#if PGP_DEBUG_STACK_BUFFERS		/* [ */

#define PGP_DEBUG_ALLOC_BUF											\
		uchar	pgpaBuf[PGP_DEBUG_BUF_SIZE];						\
		int		pgpaBufSize = sizeof(pgpaBuf);
#define PGP_DEBUG_DEALLOC_BUF

#else

#define PGP_DEBUG_NUM_BUFS		2

#define PGP_DEBUG_ALLOC_BUF											\
		int		pgpaBufSize = PGP_DEBUG_BUF_SIZE;					\
		uchar	*pgpaBuf = (pgpaDebugBufStack -= PGP_DEBUG_BUF_SIZE);
#define PGP_DEBUG_DEALLOC_BUF										\
		pgpaDebugBufStack += PGP_DEBUG_BUF_SIZE;
extern  uchar	pgpaDebugBuf[PGP_DEBUG_BUF_SIZE
										* PGP_DEBUG_NUM_BUFS];
extern  uchar *	pgpaDebugBufStack;

#endif	/* ] PGP_DEBUG_STACK_BUFFERS */


#if PGP_MACINTOSH			/* [ */

#define pgpDebugPStr(pStr)			DebugStr(pStr)
#define pgpDebugHybridStr(hStr)		pgpDebugPStr( hStr ) 
		
#define pgpDebugMsg(message)										\
		do															\
		{															\
			uchar	PGP__msg_buf[256];								\
																	\
			pgpFormatPStr(PGP__msg_buf, sizeof(PGP__msg_buf),		\
							TRUE, "%s", (message));					\
			pgpDebugPStr(PGP__msg_buf);								\
		} while (0)

#elif defined(PGP_WIN32)	/* ] [ */


#define pgpDebugMsg(message)										\
		do															\
		{															\
			if (_CrtDbgReport(_CRT_ASSERT, NULL, 0, NULL, "\r\n%s",	\
											(message))==1)			\
				_CrtDbgBreak();										\
    } while (0)

		
#define pgpDebugHybridStr(hStr)	pgpDebugMsg((char *)(hStr) + 1)

#else	/* ] [ */

#define pgpDebugMsg(message)										\
		do															\
		{															\
			fprintf(stderr, "%s\n", (char *)(message));				\
			pgpDebugHook();											\
		} while (0)
#define pgpDebugHybridStr(hStr)	pgpDebugMsg((char *)(hStr) + 1)

#endif	/* ] */

#define pgpDebugFmtMsg(params)										\
		do															\
		{															\
			uchar	pgpaBuf[256];									\
			int		pgpaBufSize = sizeof(pgpaBuf);					\
																	\
			pgpDebugHybridStr(pgpDebugFormatHStr params);			\
		} while (0)

#define pgpDebugMsgIf(condition, message)							\
		do															\
		{															\
			if (condition)											\
				pgpDebugMsg(message);								\
		} while (0)

/*
 * The first couple parameters which must always be
 * passed as the beginning of a format list.
 */
#define pgpaFmtPrefix												\
		pgpaBuf, pgpaBufSize

/*
 * The general pgpa command to check a condition and
 * print out a formatted message if it's not true.
 */
#if !PGP_MACINTOSH	/* [ */

#define pgpaFailIf(condition, params)								\
		((void)(pgpaFailed || !(condition) ||						\
			((pgpaFailed = TRUE), (pgpDebugFormatHStr params))))

#else	/* ] !PGP_MACINTOSH [ */

/* XXX: This is a workaround for a CW11 68k codegen bug */
#define pgpaFailIf(condition, params)								\
		((void)(pgpaFailed || !((condition) ? 1 : 0) ||				\
			((pgpaFailed = TRUE), (pgpDebugFormatHStr params))))

#endif	/* ] !PGP_MACINTOSH */

/*
 * A simple pgpa command to assert a given condition.
 */
#define pgpaAssert(condition)										\
		pgpaFailIf(!(condition),									\
			(pgpaFmtPrefix, "(%s) not true", #condition))

/*
 * Used to add more formatted information to an assertion failure
 */
#define pgpaFmtMsg(params)											\
		((void)(pgpaFailed && (pgpDebugPrefixFormatHStr params)))
#define pgpaMsg(message)											\
		pgpaFmtMsg((pgpaFmtPrefix, "%s", message))

/*
 * The first couple parameters which must always be
 * passed as the beginning of a pgpaCall parameter list.
 */
#define pgpaCallPrefix												\
		pgpaBuf, pgpaBufSize, pgpaFailed
#define pgpaCallPrefixDef											\
		uchar *pgpaBuf, int pgpaBufSize, PGPBoolean pgpaFailed

/*
 * The general pgpa command to call a pgpa-style function.
 */
#define pgpaCall(function, params)									\
		((void)(pgpaFailed || ((pgpaFailed = (function params)) != FALSE)))

/*
 * The main pgpa macro from which all the pgpa
 * macros must be used as parameters.
 */
#define pgpa(assertions)											\
		do															\
		{															\
			PGPBoolean		pgpaFailed = FALSE;							\
			PGP_DEBUG_ALLOC_BUF										\
																	\
			(assertions);											\
			pgpaFmtMsg((pgpaFmtPrefix,								\
						"ASSERTION FAILED at %s line %ld",			\
						__FILE__, (long)__LINE__));					\
			if (pgpaFailed)											\
				pgpDebugHybridStr(pgpaBuf);							\
			PGP_DEBUG_DEALLOC_BUF									\
		} while (0)

#define pgpaAddrValid(addr, pointedAtType)							\
		pgpaCall(pgpaInternalAddrValid,								\
				(pgpaCallPrefix, addr, alignof(pointedAtType), #addr))

#define pgpaStrLenValid(str, minLen, maxLen)						\
		pgpaCall(pgpaInternalStrLenValid,							\
				(pgpaCallPrefix, str, minLen, maxLen, #str))
#define pgpaPStrLenValid(pstr, minLen, maxLen)						\
		pgpaCall(pgpaInternalPStrLenValid,							\
				(pgpaCallPrefix, pstr, minLen, maxLen, #pstr))

/* pgpaStrValid currently assumes an arbitrary reasonable length */
#define pgpaStrValid(str)											\
		pgpaStrLenValid(str, 0, 32767)
#define pgpaPStrValid(pstr)											\
		pgpaPStrLenValid(pstr, 0, 255)

#else /* ] PGP_DEBUG [ */

#define pgpa(assertions)
#define pgpDebugMsgIf(cond, msg)
#define pgpDebugPStr(pStr)
#define pgpDebugFmtMsg(params)
#define pgpDebugMsg(msg)

#endif /* ] PGP_DEBUG */

/*
 * Convenient short-hands follow
 */

#define pgpAssert(condition)										\
		pgpa(pgpaAssert(condition))
#define pgpAssertMsg(condition, message)							\
		pgpa((														\
			pgpaAssert(condition),									\
			pgpaMsg(message)))

#define pgpAssertAddrValid(ptr, type)								\
		pgpa(pgpaAddrValid(ptr, type))
#define pgpAssertAddrValidMsg(ptr, type, message)					\
		pgpa((														\
			pgpaAddrValid(ptr, type),								\
			pgpaMsg(message)))

#define pgpAssertStrLenValid(str, minLen, maxLen)					\
		pgpa(pgpaStrLenValid(str, minLen, maxLen))
#define pgpAssertStrLenValidMsg(str, minLen, maxLen, message)		\
		pgpa((														\
			pgpaStrLenValid(str, minLen, maxLen),					\
			pgpaMsg(message)))

#define pgpAssertPStrLenValid(pstr, minLen, maxLen)					\
		pgpa(pgpaPStrLenValid(pstr, minLen, maxLen))
#define pgpAssertPStrLenValidMsg(pstr, minLen, maxLen, message)		\
		pgpa((														\
			pgpaPStrLenValid(pstr, minLen, maxLen),					\
			pgpaMsg(message)))

#define pgpAssertStrValid(str)										\
		pgpa(pgpaStrValid(str))
#define pgpAssertStrValidMsg(str, msg)								\
		pgpa((														\
			pgpaStrValid(str, minLen, maxLen),						\
			pgpaMsg(message)))

#define pgpAssertPStrValid(pstr)									\
		pgpa(pgpaPStrValid(pstr))
#define pgpAssertPStrValidMsg(pstr, message)						\
		pgpa((														\
			pgpaPStrValid(pstr, minLen, maxLen),					\
			pgpaMsg(message)))


#define pgpAssertNoErr( err ) 	pgpAssert( IsntPGPError( err ) || 	\
		err == kPGPError_UserAbort )
#define pgpAssertErrWithPtr(err, ptr) \
		pgpAssert( ( IsntPGPError( err ) && (ptr) != NULL ) || \
		( IsPGPError( err ) && (ptr) == NULL ) )
		
		
int			pgpFormatVAStr(uchar *buffer, int bufferSize,
				PGPBoolean putLengthPrefix, PGPBoolean putNullTerminator,
				PGPBoolean canonicalizeNLs, char const *formatStr,
				va_list args);
uchar *		pgpFormatHStr(uchar *buffer, int bufferSize,
						PGPBoolean canonicalizeNLs, char const *formatStr, ...);
uchar *		pgpFormatPStr(uchar *buffer, int bufferSize,
						PGPBoolean canonicalizeNLs, char const *formatStr, ...);
uchar *		pgpFormatPStr255(uchar *buffer, PGPBoolean canonicalizeNLs,
						char const *formatStr, ...);
char *		pgpFormatStr(char *buffer, int bufferSize,
					PGPBoolean canonicalizeNLs,
					char const *formatStr, ...);


#if PGP_DEBUG	/* [ */
#define pgpDebugWhackMemory( buffer, length )	\
	pgpFillMemory( buffer, length, 0xDD )
#else
#define pgpDebugWhackMemory( buffer, length )	/* nothing */
#endif

#if PGP_DEBUG	/* [ */

uchar  *	pgpDebugFormatHStr(uchar *buffer, int bufferSize,
								char const *formatStr, ...);
uchar  *	pgpDebugPrefixFormatHStr(uchar *buffer,
								int bufferSize, char const *formatStr, ...);

/*
 * These are internal routines which should only be called by the above macros
 */
void 		pgpDebugFillMemory(void *buffer, size_t length);
PGPBoolean		pgpaInternalAddrValid(pgpaCallPrefixDef,
									void const *addr, int alignSize,
									char const *varName);
PGPBoolean		pgpaInternalStrLenValid(pgpaCallPrefixDef,
									char const *str, PGPUInt32 minLen,
									PGPUInt32 maxLen, char const *varName);
PGPBoolean		pgpaInternalPStrLenValid(pgpaCallPrefixDef,
									uchar const *str, int minLen,
									int maxLen, char const *varName);

/*
 * Except on the Mac, when debugger messages are printed or assertions
 * fail, this routine will always be called.  Its only purpose is to be
 * used as a breakpoint location, to catch failed assertions from the
 * debugger.
 */
void		pgpDebugHook(void);

#endif /* ] PGP_DEBUG */

PGP_END_C_DECLARATIONS


#endif /* ] Included_pgpDebug_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
