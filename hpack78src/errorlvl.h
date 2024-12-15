/****************************************************************************
*																			*
*							HPACK Multi-System Archiver						*
*							===========================						*
*																			*
*							  HPACK Error Return ID's						*
*							ERRORLVL.H  Updated 14/03/92					*
*																			*
* This program is protected by copyright and as such any use or copying of	*
*  this code for your own purposes directly or indirectly is highly uncool	*
*  					and if you do so there will be....trubble.				*
* 				And remember: We know where your kids go to school.			*
*																			*
*		Copyright 1991 - 1992  Peter C.Gutmann.  All rights reserved		*
*																			*
****************************************************************************/

/* The following are the error-level return values for HPACK.  To be ANSI-
   compliant they should start with EXIT_SUCCESS and EXIT_FAILURE, however
   few compilers seem to have these #defines, thus we use our own.  There
   are four levels of errors: No error = 0, severe errors = 0xx, standard
   errors = 1xx, and minor errors = 2xx.  Most programmers will just treat
   all 1xx and all 2xx errors as a single error type */

enum { EXIT_OK,					/* No error */

	 /* Severe errors */
	 EXIT_INT_ERR,				/* Internal error */
	 EXIT_NO_MEM,				/* Out of memory */
	 EXIT_NO_DISK,				/* Out of disk space */
	 EXIT_NO_ARCH,				/* Can't open archive file */
	 EXIT_NO_TEMP,				/* Can't open temp file */
	 EXIT_NO_PATH,				/* Can't find path */
	 EXIT_NO_BASE,				/* Can't access base directory */
	 EXIT_NO_MKDIR,				/* Can't create directory */
	 EXIT_BREAK,				/* User interrupt */
	 EXIT_FILE_ERR,				/* Unknown file error */
	 EXIT_DIR_CORRUPT,			/* Archive directory corrupted */

	 /* Standard errors */
	 EXIT_LONG_PATH = 100,		/* Path too long */
	 EXIT_NO_OVERRIDE,			/* Can't override base path */
	 EXIT_NEST,					/* Dirs.nested too deeply */
	 EXIT_SCRIPT_ERR,			/* Error(s) in script file */
	 EXIT_NOT_ARCH,				/* Not an HPACK archive */
	 EXIT_BAD_KEYFILE,			/* Bad keyfile */
	 EXIT_NOTHING,				/* Nothing to do */
	 EXIT_COMMAND,				/* Unknown command */
	 EXIT_OPTION,				/* Unknown option */

	 /* Minor errors */
	 EXIT_PASS = 200,			/* Differing or incorrect password(s) */
	 EXIT_CHANGE,				/* Bad attempt to change archive */
	 EXIT_NOLONG,				/* Long arg.format not supported */
	 EXIT_BADWILD,				/* Bad wildcard format */
	 EXIT_SECURITY,				/* General security/encryption error */
	 EXIT_NOCRYPT };			/* Cannot read encrypted archive */
