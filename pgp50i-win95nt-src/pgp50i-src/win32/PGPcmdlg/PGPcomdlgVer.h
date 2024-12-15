/*
 * Copyright (C) 1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 */
//: PGPcomdlgVer.h - PGPcomdlg version history
//
//	$Id: PGPcomdlgVer.h,v 1.19 1997/05/30 17:05:31 pbj Exp $
//
//	970204	0001	- first version
//
//	970207	0002	- added call to pgpSavePrefs
//					- added support for wordwrap
//					- removed "compress before encrypt"
//					- added controls (but no code) for new email options
//
//	970210	0003	- added code for new email options
//
//	970224	0004	- fixed bug which causes dialog to be recentered
//					- added code to broadcast keyring reload message
//					- added help buttons
//					- changed name to PGPcomdlg (from PGPprefs)
//					- added entropy collection dialog
//
//	970228	0005	- added PGPcomdlgGetPhrase routine
//					- added calls to pgpLibInit and pgpLibCleanup to
//					  entropy and pref routines
//
//	970303	0006	- cleaned up variable names and types
//					- changed passphrase code to handle unlimited phrases
//
//	970305	0007	- added initial version of PGPcomdlgCheckKeyRings
//					- added PGPcomdlgHelpAbout
//					- changed name (to PGPcomdlgPreferences) and return
//					  values of PGPcomdlgPropSheets
//
//	970306	0008	- added help button to PGPcomdlgCheckKeyRings dialog
//					- moved help file LoadString to DllMain
//					- renumbered resource constants for help purposes
//
//	970311	0009	- switched from HeapAlloc to pgpMemAlloc
//					- cleaned up tab order in dialogs
//					- fixed random dialog to accept keystrokes
//					- added code to get full help file path from registry
//					- added "fast key gen" checkbox
//					- added PGPcomdlgErrorMessage and PGPcomdlgErrorToString
//					- added PGPcomdlgTranslateSignatureResult
//
//	970314	0010	- added splash screen routine (partially implemented)
//					- changed help about dialog to include new bitmaps
//					- change "fast key gen" text
//					- built with static runtime linkage
//
//	970315	0011	- implemented palette and color depth code for splash
//					  screen and Help About
//					- added resizing code to Help About
//					- added painting of licensee info to bitmap
//
//	970317	0012	- fixed bug in PGPcomdlgTranslateSignatureResult which
//					  was causing stack problems in TED
//					- made header file more C++ friendly (I hope)
//					- added text strings for messages 31, 56, and 1202
//
//	970318	0013	- fixed bug in PGPcomdlgErrorMessage with usage of g_szbuf
//
//	970319	0014	- added OFN_HIDEREADONLY to openfile dialogs in PGPprefs
//					- added some "input buffer empty" message strings
//					- added shared data segment flags to prevent duplicate
//					  instances
//					- fixed string bug in PGPcomdlgErrorToString
//					- added PGPcomdlgNotifyKeyringChanges
//					- fixed pointer bug in PGPcomdlgHelpAbout
//
//	970319	0015	- added new email preferences
//
//	970321	0016	- added in Brett's timeout code (PGPexpire.c)
//					- changed text of PGP/MIME preference checkboxes
//					- splash dialog now "tool window" to prevent appearance
//					  in task bar
//
//	970324	0017	- renamed to PGPcmdlg
//					- changed "*.seckr" to "*.prvkr" in openfilename dialogs
//					- added code to support keyserver preferences
//					- added signing passphrase cache preferences
//
//	970325	0018	- fixed bug where cache edit boxes were not being
//					  enabled/disabled
//
//	970327	0019	- fixed radio button grouping on check file dialog
//					- "generified" text on entropy dialog
//					- moved buttons in help about dialog on top of bitmap
//					- cleaned up tab order and "Alt+" navigation in dialogs
//
//	970331	0020	- added "ISKEY" error message
//					- fixed browse problem when file not selected properly
//
//	970401	0021	- implemented 24-bit bitmaps for splash and HelpAbout
//
//	970402	0022	- added numerous error code messages
//
//	970403	0023	- added new error codes
//
//	970404	0024	- added flag bit to error messagebox to force it to
//					  foreground
//
//	970407	0025	- updated splash and credit bitmaps
//					- added new "warn on encrypt to marginal key" checkbox
//					- added signing key selection combo box to passphrase dlg
//
//	970408	0026	- actually implemented new "warn on marginal" checkbox
//					- changed behavior of PGPcomdlgGetPhrase when no default
//					  or no key avail
//					- changed PGPCOMDLG error constants and added them to
//					  PGPerror
//					- added context help ids for new combo and checkboxes
//
//	970408	0027	- disables passphrase when not needed or no key selected
//					- added code to exclude revoked and expired keys from list
//					- added main window delay to splash routine
//					- added code to limit splashes to one-a-day
//
//	970410	0028	- added code to support retaining selected signing key
//					- changed misc error code text
//
//	970414	0029	- added passphrase dialog with signing options
//					- added entropy routine calls to PGPcomdlgGetPhrase
//
//	970415	0030	- fixed bug with Shift-Tab in passphrase dialog
//
//	970416	0031	- changed text of PGPERR_KEYDB_DUPLICATE_USERID
//					  and PGPERR_KEYDB_DUPLICATE_CERT messages.
//
//	970417	0032	- changed default focus of passphrase dialog
//					- resets read-only warning when file is changed
//
//	970418	0033	- changed entropy collection dialog to new spec
//
//	970421	0034	- changed "encrypt to self" to "encrypt to default key"
//					- added read-only error code text
//
//	970422	0035	- changed behavior of check file dialog when files exist
//
//	970423	0036	- changed "PGPmail" to "PGP"
//					- added (Type/Size) info to signing key selection combobox
//
//	970424	0037	- warns and disallows wordwrap == 0
//					- changed "warn on marginal keys" to "show recipients ..."
//
//	970425	0038	- consolidated encrypt and sign PGP MIME preferences
//					- added limits to signing and encrypting passphrase caches
//					- unchecks cache if time set to zero
//					- changed to new file extensions
//					- added code to do checking for valid file names
//
//	970429	0039	- new splash screens reflecting new name
//					- changed text of read-only file warning
//
//	970430	0040	- added advertisement call PGPcomdlgAdvertisement
//					- started inserting id strings for cvs
//
//	970504	0041	- fixed PGPcomdlgRandom to return PGPCOMDLG_OK or
//					  PGPCOMDLG_CANCEL (partial fix of bug #667)
//
//	970505	0042	- new credits bitmap
//
//	970506	0043	- removed FirstVirtual bitmaps to reduce DLL size
//
//	970508	0044	- added passphrase security mods as per Damon/Colin
//
//	970512	0046	- new credits bitmap
//					- changed "DSA" to "DSS"
//					- added freeware versions of splash screens
//
//	970613	0047	- uncorrupted version of credits bitmap
//
//	970514	0048	- new credits bitmap
//					- added message box on browser launch error
//
//	970515	0049	- added filtering of "encrypt only" keys in signing key
//					  combo box of passphrase dialog
//
//	970520	0050	- added, then removed PGPcomdlgQueryAddKeys
//					- fixed bug where shift+keys caused visible passphrase
//					- changed email prefs as per Elrod
//
//	970521	0051	- forced passphrase dialog to foreground (bug #876)
//
//	970521	0052	- reverted email preferences
//					- added "fill in the blank" version info in Help|About
//
//	970522	0053	- now sets kPGPPrefMailSignPGPMIME to be same as
//					  kPGPPrefMailEncryptPGPMIME
//
//	970527	0054	- new credits bitmap
//
//	970527	0055	- added code to provide warning when mixing RSA encryption
//					  and DSA signing keys
//
//	970528	0056	- added semaphore to prevent multiple instances of
//					  preferences property sheet
//
//	970530	0057	- changed VERSIONINFO to new format
//
