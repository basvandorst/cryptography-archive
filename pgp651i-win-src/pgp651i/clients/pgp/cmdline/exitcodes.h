/*____________________________________________________________________________
    exitcodes.h

    Copyright(C) 1998,1999 Network Associates, Inc.
    All rights reserved.

	PGP 6.5 Command Line 

	Possible error exit codes, taken from PGP262 for compatibility.
	User-interface functions may pass these codes around in function
	parameter ( int *perrorLvl ) which was formerly known as the global
	variable errorLvl.

	Not all of these are used.  Note that we don't use the ANSI
	EXIT_SUCCESS and EXIT_FAILURE.  To make things easier for compilers
	which don't support enum we use #defines

    $Id: exitcodes.h,v 1.3 1999/05/12 21:01:03 sluu Exp $
____________________________________________________________________________*/

#define EXIT_OK                       0
#define INVALID_FILE_ERROR            1
#define FILE_NOT_FOUND_ERROR          2
#define UNKNOWN_FILE_ERROR            3
#define NO_BATCH                      4
#define BAD_ARG_ERROR                 5
#define INTERRUPT                     6
#define OUT_OF_MEM                    7

/* Keyring errors: Base value = 10 */
#define KEYGEN_ERROR                 10
#define NONEXIST_KEY_ERROR           11
#define KEYRING_ADD_ERROR            12
#define KEYRING_EXTRACT_ERROR        13
#define KEYRING_EDIT_ERROR           14
#define KEYRING_VIEW_ERROR           15
#define KEYRING_REMOVE_ERROR         16
#define KEYRING_CHECK_ERROR          17
#define KEY_SIGNATURE_ERROR          18
#define KEYSIG_REMOVE_ERROR          19
#define KEYSIG_REVOKE_ERROR          KEY_SIGNATURE_ERROR

/* Encode errors: Base value = 20 */
#define SIGNATURE_ERROR              20
#define PK_ENCR_ERROR                21
#define ENCR_ERROR                   22
#define COMPRESS_ERROR               23

/* Decode errors: Base value = 30 */
#define SIGNATURE_CHECK_ERROR        30
#define PK_DECR_ERROR                31
#define DECR_ERROR                   32
#define DECOMPRESS_ERROR             33
