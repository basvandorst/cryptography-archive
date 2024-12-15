#ifndef _debug_h_
#  define _debug_h_

#  ifdef NETDEBUG
#    include <stdio.h>
#    include "ytalk.h"
#    include "usuals.h"

extern int init_debug ();
extern void exit_debug ();
extern void debug_hexdump (const byte *array, int length);
extern void debug_checkcpack (cpack *crec);
extern int __debug_printf (const char *format, ...);
#    define debug_printf(x) __debug_printf x
#  else
#    define init_debug()
#    define exit_debug()
#    define debug_hexdump(a, l)
#    define debug_checkcpack(c)
#    define debug_printf(x)
#  endif /* NETDEBUG */
#endif /* _debug_h_ */
