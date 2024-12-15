/*
 * Copyright (C) 1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 */
//: PGPkeysVer.h - PGPkeys application version history
//
//	$Id: PGPkeysVer.h,v 1.23 1997/05/30 17:05:36 pbj Exp $
//
//	970131	0030	- begin version history tracking
//
//	970202	0031	- modified treelist to version 025
//
//	970204	0032	- added close box to key properties dialog
//					- changed trust slider to only have 3 positions
//					  for public keys
//					- modified treelist to version 026
//					- added "yes to all" on deletes
//					- fixed NT 3.51 bug in passphrase dialogs code
//					- moved PGP preferences to PGPprefs.dll
//
//	970205	0033	- added timer call to allow window update before reload
//					- changed key info to property sheet
//					- allowed multiple key info sheets
//					- fixed bug in use bad phrase dialog
//					- changed loading... dialog to only show on long loads
//					- removed sort on full loads
//
//	970207	0034	- added support for generating DSA/ElGamal keys
//					- added display of key type on property sheet
//					- added DSA/ElGamal change passphrase code
//					- added code to save path name to registry
//					- updated PGPprefs to version 0002
//					- updated TreeList to version 027
//					- fixed bug in cert icon processing
//					- fixed some bugs regarding multiple property sheets
//
//	970211	0035	- added terminating null to copy to clipboard buffer
//					- upgraded to PGPprefs 0003
//					- updated entropy acquisition code for DSA/ElGamal
//					- implemented drag text FROM
//					- implemented multiple certs
//
//	970212	0036	- upgraded to TreeList 028 (header click notifications)
//					- added code to support sorting on fields other than name
//					- changed drag FROM to use FILEDESCRIPTOR (not HDROP)
//					- fixed string bug in add user name
//					- accepts pasted file names
//
//	970215	0037	- added 20 byte fingerprints for DSA keys
//					- fixed bug causing race condition on DSA key generation
//					- now deletes master DSA keys when subkey generation fails
//					- added support for dropped text
//
//	970224	0038	- fixed bug in keyboard autoselect when not sorted on name
//					- added code to prevent "drag to self" condition
//					- change to only support "copies" when dragging out
//					- fixed enable/disable/revoke that didn't update menus
//					- fixed bug so that phrase-less keys can certify
//					- added support for reload keys broadcast message
//					- added help buttons to selected dialogs
//					- upgraded TreeList to 029 (for sorting reinsertion)
//					- upgraded to PGPcomdlg 0004 (formerly PGPprefs)
//					- moved entropy dialog to PGPcomdlg DLL
//
//	970228	0039	- added support for DELETE key
//					- fixed bug in change passphrase for DSA/ELGAMAL keys
//					- updated TreeList 030 keyboard support (DEL key)
//					- moved passphrase dialog to PGPcomdlg
//					- updated PGPcomdlg to 0005
//					- fixed bug in KMUpdateKeyInTree in "add name",
//					  "delete primary user id", and property sheet dialog
//					- fully implemented key sorting
//					- added calls to PGPphrase caching routine
//					- disabled trust slider for invalid and axiomatic keys
//
//	970303	0040	- added axiomatic check box
//					- changed key info dialog handling of enable/disable
//					- fixed bug where default key was losing bold on update
//					- upgraded to TreeList version 031
//					- cleanup code to deal with unlimited userid lengths
//					- change alot of code in KMChange, KMCreate, KMProps and
//					  KMKeyOps to deal with unlimited passphrases
//					- upgraded to PGPcomdlg 0006 and PGPphrase 002 for
//					  unlimited phrases
//
//	970305	0041	- finished support for unlimited userids using new library
//					  functionality
//					- added code to flag axiomatic keys for TreeList
//					- upgraded to TreeList 032 for painting axiomatic bars
//					  in hatched
//					- added distinct bitmap for DSA/ElGamal keys
//					- adding call to PGPcomdlgCheckKeyRings
//					- changed Help About to PGPcomdlgHelpAbout
//					- upgraded to PGPcomdlg 0007
//
//	970307	0042	- upgraded to TreeList 033 to eliminate first chance
//					  exception
//					- upgraded to PGPcomdlg 0008 (added help to
//					  PcomdlgCheckKeyRings)
//					- added code to support logo bitmap
//					- fixed context help bugs; created PGPmail50.hlp project
//					- added (where necessary) loop on passphrase failure
//
//	970311	0043	- upgraded to TreeList 034 (dotted lines)
//					- cleaned up dialog tab order
//					- upgraded to PGPcomdlg 0009 (many changes)
//					- moved error message box to PGPcomdlg
//
//	970312	0044	- added logo
//					- upgraded to TreeList 035 (bug fixes)
//					- added check on return of pgpOpenDefaultKeyRings
//					- added additional error checking
//
//	970314	0045	- updated Key properties dialog to match Mac version
//					  (Axiomatic check box, hiding change phrase button, etc)
//					- removed passphrase check on setting axiomatic
//					- added "Key Properties" to userids and certs
//					- improved logo background erasing
//					- changed working dialog clockface to reduce flicker
//					- upgraded to PGPcomdlg 0010 (splash screen, help about)
//					- added DSA revoked and expired icons
//					- new icons all around
//
//	970316	0046	- removed DeleteObject of hBrushBackground
//					- added code to get Help file path from registry
//					- added KMSetFocus call on reloading keyring to avoid bad
//					  pointer crash
//					- added pointer check when displaying properties to avoid
//					  bad pointer crash
//					- added KMLoadKeyRingIntoTree call after delete to avoid
//					  problem with subsequent get of certifying key properties
//					- changed version info in Help About to use reg key value
//
//	970318	0047	- added support for enabled/disable secret keys
//					- added distinct icons for dis/rev/exp secret keys
//					- unified key context menus
//					- changed main dropdown to correspond to context menu
//					- changed version info in Help About back to report
//					  PGPkeys build number
//					- added code to broadcast PGPM_RELOADKEYRING message
//					  on call to pgpCommitKeyRingChanges
//
//	970318	0048	- fixed bug where revoking key was not changing icon
//					- disabled "axiomatic" checkbox for revoked/expired keys
//					- fixed bug where "Certifying Key Properties" was not
//					  being disabled
//
//	970319	0049	- added code to "deminimize" window when activated by
//					  second instance
//					- new icons
//
//	970321	0050	- added call to PGPcomdlgIsExpired
//					- added MRK control to Key Properties dialog
//
//	970325	0051	- fixed bug in KMKeyIO that resulted in file names with
//					  bad characters
//					- changed fingerprint control so that it is selected
//					  w/single click
//					- fixed MRK code to handle NULL keys
//					- removed logo
//					- added "/s" switch to suppress splash screen
//					- upgraded to TreeList version 0037 (better keyboard
//					  support)
//					- added support for keyserver dll
//
//	970327	0052	- moved to RCS version control
//					- changed new key dialog title to "New Key"
//					- fixed Ctrl+N accelerator key
//					- changed "certify" in all menus to "sign"
//					- changed "PGP Preferences" to "Preferences"
//					- now traps blank user name on key generation and pops
//					  messagebox
//					- tabbing order fixed in new key generation dialog
//					- added confirmation dialog on key revocation
//					- changed wording on "bad passphrase" dialog
//					- added messagebox on setting trust on invalid key
//					- fixed tab orders and "Alt+" keys on all dialogs
//					- disabled "sign" menu item for keys/userids with my
//					  signature
//
//	970328	0053	- added key menu navigation to expand/collapse item/all
//					- fixed file extension filters on importing files
//
//	970331	0054	- upgraded to TreeList 039
//					- added check for standard name format (KMCheckUserName)
//					- changed some text strings
//					- changed export extension to *.pgp
//
//	970401	0055	- implemented "deceptive-sized" (tm) keys
//					- fixed revocation code to update trust in treelist
//					- added hourglass cursor on commits, reloads, and updates
//					- handles selection and focus better on deletes
//					- cleaned-up memory allocation in KMGetSelectedKeys
//
//	970402	0056	- added confirmation message on revoking default key
//					- implemented "/" key sizes on DSA/ElGamal keys
//
//	970403	0057	- added code to set new key to default if it is the first
//					  key on ring
//
//	970404	0058	- fixed problem with small DSA key generation
//					- fixed bug where failure in key generation didn't
//					  dismiss working dialog
//					- fixed bug in DSA fingerprint conversion routine
//
//	970407	0059	- fixed bug where revoked/expired keys did not disable
//					  trust slider
//					- prepended "0x" on key id
//					- added "Unavailable, " text to unavailable certifiers
//					- removed code to disable "sign" menu for previously
//					  signed keys
//					- updated for new passphrase dialog which allows key
//					  selection
//
//	970408	0060	- changed passphrase calls to PGPcomdlgGetPhrase and
//					  corrected error codes
//					- upgraded to TreeList 041
//					- upgraded to PGPcomdlg 0027
//					- upgraded to PPhrase 010
//
//	970409	0061	- fixed bug causing miscalculation of needed entropy
//					  on DSA key gen
//
//	970410	0062	- added separate name/email edit boxes to key gen and
//					  add name dialogs
//					- added code to retain selected signing key on failed
//					  signing passphrase
//
//	970411	0063	- updated keyserver menus and added keyserver cert support
//
//	970414	0064	- changed decepto-bits to give a 2048 DSA key for
//					  3072<=ElGamal<=4096
//					- disabled main window when changing passphrase
//					- added code to prevent duplicate Key Properties dialogs
//					- updated to PGPcomdlg 0029
//					- updated to PGPphrase 012
//
//	970415	0065	- fixed shift-tab bug in key gen and change phrase dialogs
//					- Key Properties fixes for modified keys/keyrings.
//
//	970416	0066	- added code to select/expand newly imported keys
//					- fixed bug causing crash during signing of multiple keys
//					- changed message text when signing keys
//					- upgraded to TreeList 042
//					- added *.aexpk to import filter
//
//	970417	0067	- added support for read-only keyring files
//
//	970418	0068	- changed read-only warning dialog text
//					- fixed bug causing keyboard disabling following
//					  canceled keygen
//					- upgraded to TreeList 043
//					- upgraded to PGPcomdlg 0033
//
//	970421	0069	- merged with Brett Thomas's keygen wizard code
//					- updated calls to Brett Thomas's keyserver code
//					- fixed code to expand/select on keyserver importation
//					- added code to call KMCreateKey if pref not set and no
//					  secret keys
//					- added distinct RSA/DSA userid icons
//					- added "send to server" checkbox to signing confirmation
//					  dialog
//
//	970421	0070	- fixed bug causing infinite loop on bad keyring
//
//	970422	0071	- fixed bug where PGPKeySet* leak was causing crash in
//					  keyserver code
//					- fixed focus bug after importation
//					- added expansion/selection after importation of new
//					  userid/sig
//					- disabled setting trust on invalid keys
//					- implemented "send to server" checkbox
//					- upgraded to PGPcomdlg 0035
//
//	970423	0072	- disabled main window during keygen wizard
//					- now hides main window on autolaunch of keygen wizard
//					- changed "PGPmail" to "PGP"
//					- changed registry keys to new convention
//					- switched DSA and ElGamal sizes in "Size" column
//					- increased default column width for "Size" column
//
//	970424	0073	- added "set as primary userid" menu items
//					- handles DSA keys w/o ElGamal subkeys more cleanly
//
//	970425	0074	- grayed out context menu "disable" for axiomatic keys
//					- fixed bug in properties when setting disabled keys to
//					  axiomatic
//					- updated file extensions and filters
//					- upgraded to TreeList 044
//					- upgraded to PGPcomdlg 0038
//
//	970429	0075	- fixed bug where date was being displayed in GMT instead
//					  of local time
//					
//	970430	0076	- added First Virtual menu item
//
//	970502	0077	- removed second warning on signing multiple keys
//					- cleaned-up keygen wizard text
//					- changed default to not send key to keyserver
//					- centered wizard on screen
//					- fixed bug where canceling wizard caused crash
//					- fixed Alt+key navigation
//					- changed max expiration days to 2000
//					- fixed bug where RSA keys were always 1024
//					- after no name/email message boxes, focus is set to
//					  offending edit box
//					- moved all strings to string table
//					- added in passphrase text hiding/handling
//					- disabled cancel after key has been uploaded to server
//					- added calls to help system
//					- added 1,4,8 bit bitmaps to wizard
//					- fixed bug causing "Next" button to be enabled during
//					  key generation
//					- added code to make sure AVI gets shown at least once
//
//	970504	0078	- removed a couple more "PGPmail"s from the wizard
//					  (e.g. bug #665)
//					- removed First Virtual ad
//					- fixed text of dialog (bug #664)
//
//	970505	0079	- changed version string from resource to included
//					  header file
//					- fixed memory leak on key resort
//					- upgraded to TreeList 045
//					- added code to support promiscuous selection and menuing
//					- new application icons
//
//	970507	0080	- new icons
//					- fixed bugs related to cancelling sending to server
//					  during keygen (bugs #584, #621)
//					- upgraded to TreeList 046 to fix notification when
//					  click on nothing bug (bug #694)
//
//	970508	0081	- upgraded to TreeList 047
//					- implemented fixes to passphrase code as suggested by
//					  Damon/Colin
//
//	970509	0082	- wizard tweaking (fixed passphrase not the same bug
//					  (#717) ???)
//
//	970510	0083	- fixed bug (introduced in 0081) preventing "Never
//					  expires" key from being generated.
//					- fixed bug preventing "return" key from pressing
//					  default button in wizard passphrase dialogs (bug #727)
//					- allows but no longer forces entire AVI to be viewed
//				
//	970512	0084	- changed "DSA" to "DSS/Diffie-Hellman"
//					- added freeware condition compile flag to disable RSA
//					  key generation in freeware version
//					- now backs-up both public and private keyrings
//
//	970512	0085	- fixed bug which allowed > 2048 bit RSA keys
//
//	970513	0086	- fixed bug that caused Z-order problem when dismissing
//					  preferences property sheet
//
//	970513	0087	- changed DSS/DH key generation to limit DSS to 1024 bits
//
//	970514	0088	- added OFN_NOREADONLYRETURN to all GetSaveFileName calls
//					- fixed bug that could enable "Next" button during key
//					  send to server in keygen wizard
//
//	970514	0089	- fixed bug where only first imported key was selected
//
//	970515	0090	- added reload message broadcast after keyserver reports
//					  updated keyring
//
//	970516	0091	- made key gen wizard modal
//
//	970519	0092	- upgraded to TreeList 048
//					- changed dialog text when signing keys
//
//	970522	0093	- fixed bug where invalid key dates caused crash (bug 893)
//
//	970527	0094	- change to new passphrase API for handling warning on
//					  mixing RSA encryption and DSA signing.
//
//	970528	0095	- does not set main window to foreground if activating
//					  existing preferences property sheet
//
//	970530	0096	- changed VERSIONINFO information to new format
//
