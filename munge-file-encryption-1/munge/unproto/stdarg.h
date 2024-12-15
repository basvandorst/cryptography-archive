 /*
  * @(#) stdarg.h 1.3 92/01/15 21:53:24
  * 
  * Sample stdarg.h file for use with the unproto filter.
  * 
  * This file serves two purposes.
  * 
  * 1 - On systems that do not have a /usr/include/stdarg.h file, it should be
  * included by C source files that implement ANSI-style variadic functions.
  * 
  * 2 - To configure the unprototyper itself. If the _VA_ALIST_ macro is
  * defined, its value will appear in the place of the "..." at the end of
  * argument lists of variadic function *definitions* (not declarations).
  * 
  * Compilers that always pass arguments via the stack can use the default code
  * at the end of this file (this usually applies for the vax, mc68k and
  * 80*86 architectures).
  * 
  * Special tricks are needed for compilers that pass some or all function
  * arguments via registers. Examples of the latter are given for the mips
  * and sparc architectures. Usually the compiler special-cases an argument
  * declaration such as "va_alist" or "__builtin_va_alist". For inspiration,
  * see the local /usr/include/varargs.h file.
  * 
  * You can use the varargs.c program provided with the unproto package to
  * verify that the stdarg.h file has been set up correctly.
  */

#ifdef sparc /* tested with SunOS 4.1.1 */

#define _VA_ALIST_		"__builtin_va_alist"
typedef char *va_list;
#define va_start(ap, p)		(ap = (char *) &__builtin_va_alist)
#define va_arg(ap, type)	((type *) __builtin_va_arg_incr((type *) ap))[0]
#define va_end(ap)

#elif defined(mips) /* tested with Ultrix 4.0 and 4.2 */

#define _VA_ALIST_		"va_alist"
#include <stdarg.h>		/* huh? */

#else /* vax, mc68k, 80*86 */

typedef char *va_list;
#define va_start(ap, p)		(ap = (char *) (&(p)+1))
#define va_arg(ap, type)	((type *) (ap += sizeof(type)))[-1]
#define va_end(ap)

#endif
