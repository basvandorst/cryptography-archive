970319
------
Changes Since Last Version

Fixed crash on verifying detached signatures if the selected file didn't exist
Added timeout code
Added support for broadcast message when keys are added


970318
------
Changes Since Last Version

Fixed problem with default signing key sometimes not being found
Now get a parent window OK


970317
------
Changes Since Last Version

Supports creation and verification of detached signature files
Removed seperate "Verify" menu item from encrypted file types (.asc and .pgp)
because we didn't actually do anything differently on that type; Verify-only
operations are now allowed on .sig files.


Known Bugs:

randseed.bin missing not handled
Multi-app locking on PGPkeys not handled (this probably isn't a problem, since
we don't do key stuff at this time)


970313
------
Changes since Last Version:

Asks for passphrase if bad phrase entered in recipients dialog
Updated to use common error handling/signature reporting routines
Handles re-entrancy (doesn't lock up explorer on encryption)
Progress dialog
MAPI "Encrypt and Send" support

To Do:

General string/message cleanup (place in resources, make more business-like)


970306
------
Changes Since Last Version:

Now uses caching passphrase dialog
Most errors now reported in English
Uses common dialogs


To Do:

Some error messages still need cleanup (we need a common solution for this...)
MAPI support


970228
------
Changes Since Last Version:

Added "Save As" functionality for encrypt/sign/decrypt
Changed handling of multiple files (only asks for passphrase, once, now)
Updated to use new PGPRecipientsDialog dialog
Changed to be more version-manager friendly on compile
Fixed compile-time include problems

Known Bugs:

Errors aren't reported in English

To Do:

Fix known bugs
Update to use new common dialogs
