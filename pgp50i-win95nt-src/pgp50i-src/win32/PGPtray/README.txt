970319
------
Changes since Last Version:

Added call to PGPcomdlgIsTimedOut() (or whatever) to inforce beta timeout
Sends broadcast message when you add a key so that the KeyDB is updated

970318
------
Changes since Last Version:

Fixed problem with default signing key sometimes not being found
Now allow deletion of all text on clipboard


970317
------
Changes since Last Version:

Default icon now correct
Changed "Properties" menu item to "PGP Preferences" to comply with PGPkeys

To Do:

General string/message cleanup (place in resources, make more business-like)
Hot keys on menu probably need rethinking (ie, PGP Preferences should be 'P',
not 'R', PGPkeys should be 'K' not 'P')
randseed.bin missing not handled
Multi-app locking on PGPkeys not handled


970313
------
Changes Since Last Version

Uses common dialog error reporting and signature translation routines

Known Bugs:

Some messages apply more to 4.5 than 5.0, and some 5.0 messages aren't handled
randseed.bin missing not handled
Multi-app locking on PGPkeys not handled


970310
------
Changes Since Last Version

Changed reg key to find PGPkeys.exe
Unlimited retries on passphrase entry
Now use common dialog instead of PGPprefs dialog
Made signing use passphrase dialog if wrong singing phrase entered in
  recipients dialog
Fixed memory leak on multiple passphrase retries


Known Bugs:

Some messages apply more to 4.5 than 5.0, and some 5.0 messages aren't handled
randseed.bin missing not handled
Multi-app locking on PGPkeys not handled


970306
------
Changes Since Last Version

Finally fixed trailing garbage problems


Known Bugs:

Intermittant trailing garbage problem, maybe (haven't seen it in a while...)
Some messages apply more to 4.5 than 5.0, and some 5.0 messages aren't handled
randseed.bin missing not handled
Multi-app locking on PGPkeys not handled


970305
------
Changes Since Last Version

Added English error reporting
All viewers work with empty clipboard
Updated to use latest common dialogs
Added support for launching associated viewer (currently text is all that's
associated)
Greyed out "ASCII Armor" option on RecipientDialog
**Is now feature complete**

Known Bugs:

Intermittant trailing garbage problem, maybe (haven't seen it in a while...)
Some messages apply more to 4.5 than 5.0, and some 5.0 messages aren't handled
randseed.bin missing not handled
Multi-app locking on PGPkeys not handled

970303
------
Changes Since Last Version

Updated to use common passphrase dialog (including caching)
Updated to allow use of external viewers
Properly handles empty passphrases


Known Bugs:

Doesn't allow internal viewer to edit empty clipboard
Errors aren't reported in English
Occasional garbage in viewer-bug (unclear if it's on read or write)


970228
------
Changes Since Last Version:

Fixed length-chopping problem in sign (and potentially elsewhere) that
caused PGP BLOCK info to be truncated
Updated to use new PGPRecipientsDialog dialog
Changed to be more version-manager friendly on compile


Known Bugs:

Doesn't properly handle empty passphrases (ie, always prompts before calling)
Errors aren't reported in English


To Do:

Fix known bugs
Update to use new common dialogs
Change passphrase handling
Add handling for non-text types (including files?)
Add automatically launching viewer option
Allow empty clipboard to be viewed/edited
Add "launch registered viewer" functionality
