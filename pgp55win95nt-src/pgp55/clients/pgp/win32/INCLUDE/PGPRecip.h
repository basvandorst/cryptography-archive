#ifndef PGPRECIPH
#define PGPRECIPH

/* PGPrecip:
------------

There have been a number of modifications to
the API of the recipient dialog for PGP 5.5.

Foremost is the use of KeySetRefs. You must give
the recipient dialog the KeySetRef you wish to 
select keys from in OriginalKeySetRef. This is usually
from a PGPOpenDefaultKeyRings call. The keys that
the user actually selects are placed in SelectedKeySetRef
which can be initialized to zero before the call.
This KeySetRef can then be used as an PGPOEncryptToKeySet
option etc.

Second is the use of a char ** array instead of
the older string with \n's (Viacrypt format). Each
pointer in the array points to a null terminated
string containing the recipient as given by the
email client or other application. The recipient
dialog will then attempt to match that name to 
a key on startup.

Third, all PGPrecip specific flags are gone. Instead
the flags used by the PGPcmdlg.dll's passphrase dialogs
are used. These flags are divided between option,
disable, and flag fields to facilitate easy passing
from the recipient dialog to the passphrase dialog,
since usually both will be used in tandom. Before,
messy logic was needed to convert the flags from
one calling convention to another.

The flags are contained in PGPcmdlg.h, but are listed here
in commented form for reference:

// options bits
#define PGPCOMDLG_ASCIIARMOR			0x0001
   Mark the Text Output option.
#define PGPCOMDLG_DETACHEDSIG			0x0002
   Not used in Recipient Dialog, but may be
   passed through as part of passphrase dialog
   call
#define PGPCOMDLG_PASSONLY				0x0004
   Euphanism for conventional encryption
#define PGPCOMDLG_WIPEORIG				0x0008
   Delete the file after encrypting it.

// disable bits
#define PGPCOMDLG_DISABLE_ASCIIARMOR    0x0001
   Don't allow user to change Text Output option
#define PGPCOMDLG_DISABLE_AUTOMODE		0x0002
   _Always_ pop recipient dialog (don't run silent)
#define PGPCOMDLG_DISABLE_PASSONLY		0x0004
   Don't allow user to change Passphrase only option
#define PGPCOMDLG_DISABLE_WIPEORIG		0x0008
   Don't allow user to change Wipe Original option

// flag bits
#define PGPCOMDLG_RSAENCRYPT			0x0001
   Used for communication between passphrase dialog
   and recipient dialog. Should be initialized to
   zero before calling either, and not messed with
   except to pass between the two. (This flag lets
   us know if we are encrypting to an RSA key and need
   a warning when signing with DSS)

Finally, when calling the recipient dialog, always
set the Version field equal to CurrentPGPrecipVersion.
This ensures that incompatible recipientdialogs
aren't used interchangably.

7/11/97 wjb@pgp.com
*/

#define CurrentPGPrecipVersion 'DUKE'

#ifdef _PGPRDDLL
# define PGPrdExport __declspec( dllexport )
#else
# define PGPrdExport __declspec( dllimport )
#endif

typedef struct _recipientdialogstruct
{
	DWORD			Version;
	HWND			hwndParent;
	char *			szTitle;
	PGPContextRef	Context;
	PGPKeySetRef 	OriginalKeySetRef;
	PGPKeySetRef 	SelectedKeySetRef;
	char **			szRecipientArray;
	DWORD			dwNumRecipients;
	DWORD			dwOptions;
	DWORD			dwFlags;
	DWORD			dwDisableFlags;
} RECIPIENTDIALOGSTRUCT, *PRECIPIENTDIALOGSTRUCT;


#ifdef __cplusplus
extern "C" {
#endif	// __cplusplus

UINT PGPrdExport PGPRecipientDialog(PRECIPIENTDIALOGSTRUCT prds);

#ifdef __cplusplus
}
#endif	// __cplusplus

#endif // PGPRECIPH

