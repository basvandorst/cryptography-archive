/*
 * pgpDebug.c -- Assertion support routines
 *
 * Copyright (C) 1996,1997 Pretty Good Privacy, Inc. All rights reserved.
 *
 * $Id: pgpDebug.c,v 1.13.2.1 1997/06/07 09:50:01 mhw Exp $
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string.h>
#include <stdarg.h>
#include <limits.h>
#include "pgpTypes.h"
#include "pgpDebug.h"
#include "pgpMem.h"


#if DEBUG && !PGP_DEBUG_STACK_BUFFERS
uchar   pgpaDebugBuf[PGP_DEBUG_BUF_SIZE * PGP_DEBUG_NUM_BUFS];
uchar * pgpaDebugBufStack = pgpaDebugBuf + sizeof(pgpaDebugBuf);
#endif


    static void
AppendChars(
    uchar *         dest,
    int   *         offset,
    int             max,
    Boolean         canonicalizeNLs,
    char const *    src,
    int             len)
{
    int             destIndex = *offset;
    int             srcIndex = 0;

    pgpa(pgpaAddrValid(src, char));

    if (canonicalizeNLs)
    {
        while (srcIndex < len && destIndex < max)
            switch (src[srcIndex])
            {
                case '\r':
                    if (srcIndex + 1 < len && src[srcIndex + 1] == '\n')
                        srcIndex++;
                    /* Fall through */
                case '\n':
#if MACINTOSH
                    dest[destIndex++] = '\r';
#elif defined(WIN32)
                    dest[destIndex++] = '\r';
                    if (destIndex < max)
                        dest[destIndex++] = '\n';
#else
                    dest[destIndex++] = '\n';
#endif
                    srcIndex++;
                    break;
                default:
                    dest[destIndex++] = src[srcIndex++];
                    break;
            }
    }
    else
    {
        while (srcIndex < len && destIndex < max)
            dest[destIndex++] = src[srcIndex++];
    }

    *offset = destIndex;
}

    static void
AppendUnsignedNumber(
    uchar *         dest,
    int *           offset,
    int             max,
    int             radix,
    ulong           num)
{
    static char const digit[17] = "0123456789ABCDEF";
    char        buf[36];
    int         i = sizeof(buf);

    pgpAssertMsg(radix >= 2 && radix <= 16, "Invalid radix");
    do
    {
        pgpAssert(i > 0);
        buf[--i] = digit[num % radix];
        num /= radix;
    } while (num > 0);
    AppendChars(dest, offset, max, FALSE, buf + i, sizeof(buf) - i);
}

    static void
AppendSignedNumber(
    uchar *         dest,
    int *           offset,
    int             max,
    int             radix,
    long            num)
{
    if (num < 0)
    {
        AppendChars(dest, offset, max, FALSE, "-", 1);
        AppendUnsignedNumber(dest, offset, max, radix, -num);
    }
    else
        AppendUnsignedNumber(dest, offset, max, radix, num);
}

/*
 * Formats a string into the given <buffer>, similarly to sprintf.
 * If <putLengthPrefix> is true, a pascal-style length prefix is put at the
 * beginning of the string. If <putNullTerminator> is true, a 0 byte is
 * added to the end. This allows creation of C strings, Pascal strings, or
 * hybrid strings which are both. <bufferSize> indicates the size of the
 * buffer, ie, the total number of bytes which can be modified, including
 * length prefix size and null-terminator. In all cases, the length of the
 * string in actual characters is returned.
 *
 * Recognizes the following special sequences in the format string:
 *       %%     Replaced with a single '%'
 *       %c     Replaced with the char argument
 *       %s     Replaced with the C string argument
 *       %S     Replaced with the Pascal string argument
 *       %B     Replaced with the memory buffer as a string
 *              (two args: length (int), buffer)
 *       %lB    Replaced with the memory buffer as a string
 *              (two args: length (long), buffer)
 *       %d     Replaced with the signed integer value (base 10)
 *       %ld    Replaced with the signed long value (base 10)
 *       %u     Replaced with the unsigned integer value (base 10)
 *       %lu    Replaced with the unsigned long value (base 10)
 *       %x     Replaced with the unsigned integer value (base 16)
 *       %lx    Replaced with the unsigned long value (base 16)
 *       %o     Replaced with the unsigned integer value (base 8)
 *       %lo    Replaced with the unsigned long value (base 8)
 *       %b     Replaced with the unsigned integer value (base 2)
 *       %lb    Replaced with the unsigned long value (base 2)
 *       %p     Replaced with the pointer value, printed in hex
 */
    int
pgpFormatVAStr(
    uchar *             buffer,
    int                 bufferSize,
    Boolean             putLengthPrefix,
    Boolean             putNullTerminator,
    Boolean             canonicalizeNLs,
    char const *        formatStr,
    va_list             args)
{
    int                 bufferIndex = putLengthPrefix ? 1 : 0;
    int                 formatStrIndex;
    int                 stringLength;

    pgpa((
        pgpaAddrValid(buffer, uchar),
        pgpaAssert(bufferSize >=
                        putLengthPrefix + (putNullTerminator ? 1 : 0)),
        pgpaAddrValid(formatStr, char),
        pgpaMsg("Invalid parameters to pgpaFormatVAStr")));

    for (formatStrIndex = 0; formatStr[formatStrIndex] != '\0';
            formatStrIndex++)
    {
        if (formatStr[formatStrIndex] != '%')
            AppendChars(buffer, &bufferIndex, bufferSize,
                        canonicalizeNLs, &formatStr[formatStrIndex], 1);
        else if (formatStr[formatStrIndex + 1] != '\0')
        {
            int         cmdStartIndex = formatStrIndex;
            Boolean     longFlag = FALSE;
            int         radix;

            formatStrIndex++;
            if (formatStr[formatStrIndex] == 'l')
            {
                longFlag = TRUE;
                formatStrIndex++;
            }
            switch (formatStr[formatStrIndex])
            {
                case '%':  /* Literal '%' */
                    AppendChars(buffer, &bufferIndex, bufferSize,
                                FALSE, &formatStr[formatStrIndex], 1);
                    break;

                case 'c':  /* Single character */
                {
                    char            ch = va_arg(args, char);

                    AppendChars(buffer, &bufferIndex, bufferSize,
                                canonicalizeNLs, &ch, 1);
                    break;
                }
                case 's':  /* C string argument */
                {
                    char const *    str = va_arg(args, char const *);

                    pgpa(pgpaAddrValid(str, char));
                    AppendChars(buffer, &bufferIndex, bufferSize,
                                canonicalizeNLs, str, strlen(str));
                    break;
                }
                case 'S': /* Pascal string argument */
                {
                    uchar const *   pstr = va_arg(args, uchar const *);

                    pgpa(pgpaAddrValid(pstr, uchar));
                    AppendChars(buffer, &bufferIndex, bufferSize,
                                canonicalizeNLs, (char *)(pstr + 1), pstr[0]);
                    break;
                }
                case 'B':  /* String buffer argument (length, buf) */
                {
                    long            length;
                    char const *    buf;

                    if (longFlag)
                        length = va_arg(args, long);
                    else
                        length = va_arg(args, int);

                    buf = va_arg(args, char const *);

                    pgpa((
                          pgpaAssert(length >= 0),
                          pgpaAddrValid(buf, char)));
                    if (length > INT_MAX)
                        length = INT_MAX;
                    AppendChars(buffer, &bufferIndex, bufferSize,
                                canonicalizeNLs, buf, length);
                    break;
                }
                case 'p':  /* pointer argument */
                    pgpAssert(sizeof(ulong) == sizeof(void *));
                    AppendChars(buffer, &bufferIndex, bufferSize,
                                FALSE, "0x", 2);
                    longFlag = TRUE;
                    radix = 16;
                    goto doUInt;

                case 'x':  /* unsigned int argument, base 16 */
                case 'X':
                    radix = 16;
                    goto doUInt;

                case 'o':  /* unsigned int argument, base 8 */
                    radix = 8;
                    goto doUInt;

                case 'b':  /* unsigned int argument, base 2 */
                    radix = 2;
                    goto doUInt;

                case 'u':  /* unsigned int argument, base 10 */
                    radix = 10;
                doUInt:
                    {
                        ulong           num;

                        if (longFlag)
                            num = va_arg(args, ulong);
                        else
                            num = va_arg(args, uint);

                        AppendUnsignedNumber(buffer, &bufferIndex, bufferSize,
                                             radix, num);
                    }
                    break;

                case 'd':  /* signed integer argument, base 10 */
                {
                    long        num;

                    if (longFlag)
                        num = va_arg(args, long);
                    else
                        num = va_arg(args, int);

                    AppendSignedNumber(buffer, &bufferIndex, bufferSize,
                                       10, num);
                    break;
                }
                default:
                {
                    char      errorBuf[32];

                    pgpFormatStr(errorBuf, sizeof(errorBuf), FALSE,
                                 "[INVALID %B]",
                                 formatStrIndex - cmdStartIndex + 1,
                                 formatStr + cmdStartIndex);
                    AppendChars(buffer, &bufferIndex, bufferSize,
                                canonicalizeNLs, errorBuf, strlen(errorBuf));
                    break;
                }
            }
        }
    }

    if (putNullTerminator)
    {
        if (bufferIndex >= bufferSize)
            bufferIndex = bufferSize - 1;
        buffer[bufferIndex] = '\0';
    }

    stringLength = bufferIndex - (putLengthPrefix ? 1 : 0);

    if (putLengthPrefix)
        buffer[0] = (stringLength <= 255) ? stringLength : 255;

    return stringLength;
}

    uchar *
pgpFormatHStr(
    uchar *         buffer,
    int             bufferSize,
    Boolean         canonicalizeNLs,
    char const *    formatStr,
    ...)
{
    va_list         args;

    va_start(args, formatStr);
    pgpFormatVAStr(buffer, bufferSize, TRUE, TRUE, canonicalizeNLs,
                   formatStr, args);
    va_end(args);
    return buffer;
}

    uchar *
pgpFormatPStr(
    uchar *         buffer,
    int             bufferSize,
    Boolean         canonicalizeNLs,
    char const *    formatStr,
    ...)
{
    va_list         args;

    va_start(args, formatStr);
    pgpFormatVAStr(buffer, bufferSize, TRUE, FALSE, canonicalizeNLs,
                   formatStr, args);
    va_end(args);
    return buffer;
}

    uchar *
pgpFormatPStr255(
    uchar *          buffer,
    Boolean          canonicalizeNLs,
    char const *     formatStr,
    ...)
{
    va_list          args;

    va_start(args, formatStr);
    pgpFormatVAStr(buffer, 256, TRUE, FALSE, canonicalizeNLs, formatStr, args);
    va_end(args);
    return buffer;
}

    char *
pgpFormatStr(
    char *           buffer,
    int              bufferSize,
    Boolean          canonicalizeNLs,
    char const *     formatStr,
    ...)
{
    va_list          args;

    va_start(args, formatStr);
    pgpFormatVAStr((uchar *)buffer, bufferSize, FALSE, TRUE, canonicalizeNLs,
                   formatStr, args);
    va_end(args);
    return buffer;
}

#if DEBUG  /* [ */

    uchar *
pgpDebugFormatHStr(
    uchar *          buffer,
    int              bufferSize,
    char const *     formatStr,
    ...)
{
    va_list          args;

    va_start(args, formatStr);
    pgpFormatVAStr(buffer, bufferSize, TRUE, TRUE, TRUE, formatStr, args);
    va_end(args);
    return buffer;
}

    uchar *
pgpDebugPrefixFormatHStr(
    uchar *          buffer,
    int              bufferSize,
    char const *     formatStr,
    ...)
{
    va_list          args;
    int              origSize;  /* Including null, but not length prefix */
    int              origOffset;
    int              prefixSize; /* Including length prefix, no null */
    int              totalLen;

    /*
     * Move the original string (except length prefix) to the end of the
     * buffer.
     */
    origSize = buffer[0] + 1;
    if (origSize == 256)
        origSize = strlen((char const *)buffer + 1) + 1;
    origOffset = bufferSize - origSize;
    pgpCopyMemory(buffer + 1, buffer + origOffset, origSize);

    /* Format the new prefix, without null-terminator */
    va_start(args, formatStr);
    prefixSize = 1 + pgpFormatVAStr(buffer, origOffset, TRUE, FALSE, TRUE,
                                    formatStr, args);
    va_end(args);

    /* Add a separating string */
    AppendChars(buffer, &prefixSize, origOffset, TRUE, ":\n  ", 4);

    /* Move the original part back so that it's adjacent to the prefix */
    pgpCopyMemory(buffer + origOffset, buffer + prefixSize, origSize);

    /* Set the length prefix */
    totalLen = (origSize - 1) + (prefixSize - 1);
    buffer[0] = (totalLen <= 255) ? totalLen : 255;

    return buffer;
}

    void
pgpDebugFillMemory(
    void *           buffer,
    size_t           length)
{
    pgpFillMemory(buffer, length, 0xDD);
}

    static char
pgpMemAccessTest(
    void const *     addr)
{
    return *(volatile char const *)addr;
}

    Boolean
pgpaInternalAddrValid(
    pgpaCallPrefixDef,
    void const *     addr,
    int              alignSize,
    char const *     varName)
{
#if MACINTOSH
    /* XXX alignof() doesn't match up with reality...bummer */
    if (alignSize > 2)
        alignSize = 2;
#endif

  pgpaAssert(addr != NULL);
  pgpaAssert(alignSize > 0);
  pgpaAssert(alignSize == 1 || (ulong)addr % alignSize == 0);
#if MACINTOSH
  pgpaAssert((ulong)addr >= 256);
#endif
  pgpaFmtMsg((pgpaFmtPrefix,
              "pgpaAddrValidAligned failed on %s (%p) (align %d)",
              varName, addr, alignSize));

  if (!pgpaFailed)
      pgpMemAccessTest(addr);

  return pgpaFailed;
}

    Boolean
pgpaInternalStrLenValid(
    pgpaCallPrefixDef,
    char const *     str,
    ulong            minLen,
    ulong            maxLen,
    char const *     varName)
{
    ulong            i;

    pgpaAddrValid(str, char);

    for (i = 0; !pgpaFailed && str[i] != '\0'; i++)
    {
        pgpaFailIf(i >= maxLen, (pgpaFmtPrefix,
                "longer than %lu chars", maxLen));
        pgpaFailIf((uchar)(str[i]) < '\b', (pgpaFmtPrefix,
                "suspicious char 0x%x at index %lu", str[i], i));
    }

    pgpaFailIf(i < minLen, (pgpaFmtPrefix, "shorter than %lu chars", minLen));
    pgpaFmtMsg((pgpaFmtPrefix, "pgpaStrLenValid failed on %s (%p)",
                varName, str));

    return pgpaFailed;
}

    Boolean
pgpaInternalPStrLenValid(
    pgpaCallPrefixDef,
    uchar const *    str,
    int              minLen,
    int              maxLen,
    char const *     varName)
{
    int              len;
    int              i;

    pgpa((
          pgpaAssert(minLen >= 0),
          pgpaAssert(maxLen <= 255),
          pgpaMsg("Invalid parameters to pgpaPStrLenValid")));

    pgpaAddrValid(str, uchar);

    if (!pgpaFailed)
    {
        len = str[0];

        pgpaFailIf(len > maxLen, (pgpaFmtPrefix,
                    "longer than %d chars", maxLen));
        pgpaFailIf(len < minLen, (pgpaFmtPrefix,
                    "shorter than %d chars", minLen));

        for (i = 1; !pgpaFailed && i <= len; i++)
            pgpaFailIf(str[i] < '\b', (pgpaFmtPrefix,
                    "suspicious char 0x%x at index %d", str[i], i));

        pgpaFailIf(i < minLen, (pgpaFmtPrefix,
                    "shorter than %lu chars", minLen));
    }

    pgpaFmtMsg((pgpaFmtPrefix,
                    "pgpaPStrLenValid failed on %s (%p)", varName, str));

    return pgpaFailed;
}

    void
pgpDebugHook(void)
{
}

#endif  /* ] DEBUG */

/*
* Local Variables:
* tab-width: 4
* End:
* vi: ts=4 sw=4
* vim: si
*/
