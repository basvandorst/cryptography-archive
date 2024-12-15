// PGPbuild.h	- common preprocessor symbol definitions for controlling
//				  build variations
//
//	$Id: pgpBuild.h,v 1.1.2.1 1997/06/12 15:22:09 pbj Exp $
//

// PGP_NO_RSA_OPERATIONS
// if defined, PGP_NO_RSA_OPERATIONS creates a version of PGP which
// can understand RSA keys (can be imported to keyring) but cannot
// generate them, sign or verify with them, or encrypt to them.
#define PGP_NO_RSA_OPERATIONS	
