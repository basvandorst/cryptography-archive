#ifndef _VERSION_H_
#define _VERSION_H_

#ifdef  VER_COMPANYNAME_STR
#undef  VER_COMPANYNAME_STR
#endif
#ifdef  VER_PRODUCTNAME_STR
#undef  VER_PRODUCTNAME_STR
#endif
#ifdef  VER_PRODUCTVERSION_STR
#undef  VER_PRODUCTVERSION_STR
#endif
#ifdef  VER_PRODUCTVERSION
#undef  VER_PRODUCTVERSION
#endif

#define VER_COMPANYNAME_STR         "Network Associates \0"
#define VER_FILEDESCRIPTION_STR     "PGPnet Client Driver.\0"
#define VER_INTERNALNAME_STR        "vpnmac.sys\0"
#define VER_LEGALCOPYRIGHT_STR      "Copyright \251 1998 " VER_COMPANYNAME_STR
#define VER_ORIGINALFILENAME_STR    VER_INTERNALNAME_STR
#define VER_PRODUCTNAME_STR         "PGPnet Client Driver.\0"


#endif // _VERSION_H_
