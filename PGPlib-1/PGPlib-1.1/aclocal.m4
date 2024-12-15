# Local additions to Autoconf macros.

## ------------------------------------------------------- ##
## Find prefix for printing long long hexadecimal numbers. ##
##							   ##
## Copyright (C) 1997 Magne Rasmussen			   ##
## ------------------------------------------------------- ##

AC_DEFUN(mr_FUNC_PRINTF_LLX,
[AC_CACHE_CHECK(how to printf long long hex, mr_cv_func_printf_llx,
[AC_TRY_RUN([#include <stdio.h>
#include <string.h>
main()
{
    char       buf[ 32 ];
    FILE *     f = fopen("conftestval", "w");
    long long  var = 0x123456789ABCDEF0ll;
    if (!f) exit(1);
    sprintf(buf, "%llx", var);
    if (strcmp(buf, "123456789abcdef0") == 0) {
	fprintf(f, "llx");
	exit(0);
    }
    sprintf(buf, "%qx", var);
    if (strcmp(buf, "123456789abcdef0") == 0) {
	fprintf(f, "qx");
	exit(0);
    }
    exit(1);
}], mr_cv_func_printf_llx=`cat conftestval`, mr_cv_func_printf_llx=lx, mr_cv_func_printf_llx=lx)])
AC_DEFINE_UNQUOTED(PRIx64, "$mr_cv_func_printf_llx")
])
