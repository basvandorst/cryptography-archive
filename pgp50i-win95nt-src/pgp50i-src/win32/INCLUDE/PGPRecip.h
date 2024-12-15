//////////////////////////////////////////////////////////////////////
//
//	Copyright (C) 1997 Pretty Good Privacy, Inc.  All rights reserved.
//
//	PGPRecipientDialog.h - Recipient Encryption Dialog Headers
//
//////////////////////////////////////////////////////////////////////


#define	PGP_PASSPHRASE_SIZE		255
#define	PGP_USER_ID_SIZE		256

#define PGP_ENCRYPT					0x00000001
#define PGP_SIGN					0x00000002
#define PGP_ASCII_ARMOR				0x00000004	
#define PGP_CREATE_SIG_FILE			0x00000008	
#define PGP_ATLEASTONERSAKEY		0x00000010

#define DISABLE_ASCII_ARMOR			0x00000001
#define DISABLE_CREATE_SIG_FILE		0x00000002	
#define DISABLE_AUTO_POP_DIALOG     0x00000004

#define RCPT_DLG_ERROR_SUCCESS		0x00000000
#define RCPT_DLG_ERROR_FAILURE		0x00000001
#define RCPT_DLG_ERROR_VERSION		0x00000002

#define RCPT_DLG_VERSION			0x0101

		

typedef struct _RECIPIENTDIALOGSTRUCT
{
	USHORT	sVersion;		// Version Number For The Structure in form
							//		0x[HiVersion][LoVersion]
							// so version 1.0 = 0x0100 and 1.1 = 0x0101

	HWND	hwndParent;		// Parent Window of the RecipientDialog	

	char*	szTitle;		// String to be displayed in Title Bar of
							// the Dialog
	
	DWORD	dwPGPFlags;		// bit flags indicating the default state of all
							// dialogs options on the way in and what the
							// user chose on the way out

	DWORD	dwDisableFlags;	// bit flags used to disable certain options
							// in the dialog

	DWORD	dwNumRecipients;// How many items are contained in the
							// szRecipients field;

	char*	szRecipients;	// A NewLine '\n' delimited list of all the
							// User Ids the caller wants to have
							// already selected when the Dialog appears
							// On return this will be a NewLine '\n'
							// delimited list of KEY IDS for the caller
							// to encrypt to.

	char*	szPassPhrase;	// The user's passphase for use in signing	

	char*	szSigningKey;   // The signing key to use in hex format non \n
	                        // delimited

	long	HashAlg;		// The hash algorithm of the signing key


}RECIPIENTDIALOGSTRUCT, *PRECIPIENTDIALOGSTRUCT;


#ifdef __cplusplus
extern "C" {
#endif	// __cplusplus

/*--------------------------------------------------------------------------

In Order to use the PGPRecipientDialog, the programmer must first create a
RECIPIENTDIALOGSTRUCT.

I recommend zeroing out the structure.

Fill out the sVersion field with the appropriate Version Number, which in
this case is 0x0101. (RCPT_DLG_VERSION is defined for your coding pleasure)

hwndParent is the Parent Window of the Dialog.. NULL is a valid value and
will make the dialog a top level window.
				
If you wish to customize the Title displayed in the Dialog pass in a pointer
to a string in the szTitle field. If you wish to leave the title as the
default "Recipients Encryption Dialog" then pass in NULL for this field.

To have certain options Pre-Selected for the user, fill out the dwPGPFlags
field with the appropriate bits. Each bit set will cause that option to be
selected. When the function returns the bits will be set to reflect the
options chosen by the user in the dialog.

It is sometimes desirable to disable certain options. If you wish to Disable
any of the options set the appropriate bits in the  dwDisableFlags field.


dwNumRecipients specifies the expected number of PGP encrypted recipients on
the way in and the total number actually selected on the way out.


szRecipients is a New Line '\n' delimited list of all the User Ids the
caller wants to have already selected when the Dialog appears. There should
be the same number of User Ids as is indicated by dwNumRecipients.

EXAMPLE: "Mark B. Elrod <elrod@pgp>\nBrett A. Thomas <bat@pgp.com>\n"

On return this will be a New Line '\n' delimited list of KEY IDS for the
caller to encrypt to. Unfortunately it must return these with that silly
special BYTE at the beginning for Simple PGP API callers. Death to the Simple
PGP API!!!!

EXAMPLE: [0x02]"0xDEADBEEF\n0x12345678\n"

NOTE: The list should be allocated with pgpMemAlloc and will be disposed of
by PGPRecipientDialog and replaced with the returned list of recipients on
which the caller is responsible for calling pgpMemFree.


szPassPhrase will contain the user's passphase for use in signing.
NOTE: szPassPhrase should be a freed with pgpMemFree.



RETURN VALUE:	

PGPRecipientDialog returns RCPT_DLG_ERROR_SUCCESS if the user presses OK.
PGPRecipientDialog returns RCPT_DLG_ERROR_FAILURE if the user presses CANCEL.
PGPRecipientDialog returns RCPT_DLG_ERROR_VERSION if the structure is the
	wrong version.


Any Questions E-mail: elrod@pgp.com

Changes by wjb@pgp.com 3/18/97

Recipient Dialog now checks user names and tries to match on the
key rings automatically. If it has trouble, it pops the recipient
dialog. Otherwise, it outputs a list of KeyIDs without popping any
dialogs at all. The programmer can overid this automatic behavior
by setting the DISABLE_AUTO_POP_DIALOG flag in dwDisableFlags. This
will cause the dialog to be popped no matter what.

By "trouble" we mean if any of the user names could not be matched,
that is, the entire user name did not match any user name on the key
ring, or an email address in one of the keys in the keyring did not
match the email address in the user name. "Trouble" also means if any
of the validities on the matched keys are zero. We give the
user the chance to double check whether he wants to encrypt the message
to those people.

After all the selections have been made (whether the dialog was
popped or not) the recipient routine does a final check to see if
DSA and RSA keys were combined in the recipient list. If so, it
warns the user and may pop the dialog if he wants to change things
around.

Other changes in the recipient dialog include graphical dragging of
user IDs, MACish italicised and striked out text in the list view, and
images associated with each username.

wjb@pgp.com 3/24/97

Added in Message Recovery Key support based on Mark Weaver's
MacIntosh code. Also added in custom sorting routine to use library
sort call instead of standard Microsoft ListView sort. Updated the
recipient list to use Brett Thomas' keyserver menu routine. Changed
name to PGPRecip.h to conform to 8.3 braindead Installshield limitation.

wjb@pgp.com 4/3/97

Got rid of minimum validity bar as per mac. Added combo box displaying
private keys to select your signing key from. Added two more parameters
to the calling structure:

  szSigningKey   // This string returns a 64 bit 0x hex KeyID
  HashAlg        // This long returns the hash of that key
                 // (as per elrod's request)

wjb@pgp.com 4/20/97

Redid entire dialog. Did not change header to ensure compatiblity
with other modules. Unfortunately, this should eventually be changed.
--------------------------------------------------------------------------*/


UINT PGPExport PGPRecipientDialog(PRECIPIENTDIALOGSTRUCT prds);

#ifdef __cplusplus
}
#endif	// __cplusplus
