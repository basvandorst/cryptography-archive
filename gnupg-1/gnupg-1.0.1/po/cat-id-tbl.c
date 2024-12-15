/* Automatically generated by po2tbl.sed from gnupg.pot.  */

#if HAVE_CONFIG_H
# include <config.h>
#endif

#include "libgettext.h"

const struct _msg_ent _msg_tbl[] = {
  {"", 1},
  {"Warning: using insecure memory!\n", 2},
  {"operation is not possible without initialized secure memory\n", 3},
  {"(you may have used the wrong program for this task)\n", 4},
  {"yes", 5},
  {"yY", 6},
  {"no", 7},
  {"nN", 8},
  {"quit", 9},
  {"qQ", 10},
  {"general error", 11},
  {"unknown packet type", 12},
  {"unknown version", 13},
  {"unknown pubkey algorithm", 14},
  {"unknown digest algorithm", 15},
  {"bad public key", 16},
  {"bad secret key", 17},
  {"bad signature", 18},
  {"checksum error", 19},
  {"bad passphrase", 20},
  {"public key not found", 21},
  {"unknown cipher algorithm", 22},
  {"can't open the keyring", 23},
  {"invalid packet", 24},
  {"invalid armor", 25},
  {"no such user id", 26},
  {"secret key not available", 27},
  {"wrong secret key used", 28},
  {"not supported", 29},
  {"bad key", 30},
  {"file read error", 31},
  {"file write error", 32},
  {"unknown compress algorithm", 33},
  {"file open error", 34},
  {"file create error", 35},
  {"invalid passphrase", 36},
  {"unimplemented pubkey algorithm", 37},
  {"unimplemented cipher algorithm", 38},
  {"unknown signature class", 39},
  {"trust database error", 40},
  {"bad MPI", 41},
  {"resource limit", 42},
  {"invalid keyring", 43},
  {"bad certificate", 44},
  {"malformed user id", 45},
  {"file close error", 46},
  {"file rename error", 47},
  {"file delete error", 48},
  {"unexpected data", 49},
  {"timestamp conflict", 50},
  {"unusable pubkey algorithm", 51},
  {"file exists", 52},
  {"weak key", 53},
  {"invalid argument", 54},
  {"bad URI", 55},
  {"unsupported URI", 56},
  {"network error", 57},
  {"not encrypted", 58},
  {"not processed", 59},
  {"... this is a bug (%s:%d:%s)\n", 60},
  {"you found a bug ... (%s:%d)\n", 61},
  {"WARNING: using insecure random number generator!!\n", 62},
  {"\
The random number generator is only a kludge to let\n\
it run - it is in no way a strong RNG!\n\
\n\
DON'T USE ANY DATA GENERATED BY THIS PROGRAM!!\n\
\n", 63},
  {"\
\n\
Not enough random bytes available.  Please do some other work to give\n\
the OS a chance to collect more entropy! (Need %d more bytes)\n", 64},
  {"\
@Commands:\n\
 ", 65},
  {"|[file]|make a signature", 66},
  {"|[file]|make a clear text signature", 67},
  {"make a detached signature", 68},
  {"encrypt data", 69},
  {"encryption only with symmetric cipher", 70},
  {"store only", 71},
  {"decrypt data (default)", 72},
  {"verify a signature", 73},
  {"list keys", 74},
  {"list keys and signatures", 75},
  {"check key signatures", 76},
  {"list keys and fingerprints", 77},
  {"list secret keys", 78},
  {"generate a new key pair", 79},
  {"remove key from the public keyring", 80},
  {"remove key from the secret keyring", 81},
  {"sign a key", 82},
  {"sign a key locally", 83},
  {"sign or edit a key", 84},
  {"generate a revocation certificate", 85},
  {"export keys", 86},
  {"export keys to a key server", 87},
  {"import keys from a key server", 88},
  {"import/merge keys", 89},
  {"list only the sequence of packets", 90},
  {"export the ownertrust values", 91},
  {"import ownertrust values", 92},
  {"update the trust database", 93},
  {"|[NAMES]|check the trust database", 94},
  {"fix a corrupted trust database", 95},
  {"De-Armor a file or stdin", 96},
  {"En-Armor a file or stdin", 97},
  {"|algo [files]|print message digests", 98},
  {"\
@\n\
Options:\n\
 ", 99},
  {"create ascii armored output", 100},
  {"|NAME|encrypt for NAME", 101},
  {"|NAME|use NAME as default recipient", 102},
  {"use the default key as default recipient", 103},
  {"use this user-id to sign or decrypt", 104},
  {"|N|set compress level N (0 disables)", 105},
  {"use canonical text mode", 106},
  {"use as output file", 107},
  {"verbose", 108},
  {"be somewhat more quiet", 109},
  {"don't use the terminal at all", 110},
  {"force v3 signatures", 111},
  {"always use a MDC for encryption", 112},
  {"do not make any changes", 113},
  {"batch mode: never ask", 114},
  {"assume yes on most questions", 115},
  {"assume no on most questions", 116},
  {"add this keyring to the list of keyrings", 117},
  {"add this secret keyring to the list", 118},
  {"|NAME|use NAME as default secret key", 119},
  {"|HOST|use this keyserver to lookup keys", 120},
  {"|NAME|set terminal charset to NAME", 121},
  {"read options from file", 122},
  {"set debugging flags", 123},
  {"enable full debugging", 124},
  {"|FD|write status info to this FD", 125},
  {"do not write comment packets", 126},
  {"(default is 1)", 127},
  {"(default is 3)", 128},
  {"|FILE|load extension module FILE", 129},
  {"emulate the mode described in RFC1991", 130},
  {"set all packet, cipher and digest options to OpenPGP behavior", 131},
  {"|N|use passphrase mode N", 132},
  {"|NAME|use message digest algorithm NAME for passphrases", 133},
  {"|NAME|use cipher algorithm NAME for passphrases", 134},
  {"|NAME|use cipher algorithm NAME", 135},
  {"|NAME|use message digest algorithm NAME", 136},
  {"|N|use compress algorithm N", 137},
  {"throw keyid field of encrypted packets", 138},
  {"|NAME=VALUE|use this notation data", 139},
  {"\
@\n\
Examples:\n\
\n\
 -se -r Bob [file]          sign and encrypt for user Bob\n\
 --clearsign [file]         make a clear text signature\n\
 --detach-sign [file]       make a detached signature\n\
 --list-keys [names]        show keys\n\
 --fingerprint [names]      show fingerprints\n", 140},
  {"Please report bugs to <gnupg-bugs@gnu.org>.\n", 141},
  {"Usage: gpg [options] [files] (-h for help)", 142},
  {"\
Syntax: gpg [options] [files]\n\
sign, check, encrypt or decrypt\n\
default operation depends on the input data\n", 143},
  {"\
\n\
Supported algorithms:\n", 144},
  {"usage: gpg [options] ", 145},
  {"conflicting commands\n", 146},
  {"NOTE: no default option file `%s'\n", 147},
  {"option file `%s': %s\n", 148},
  {"reading options from `%s'\n", 149},
  {"%s is not a valid character set\n", 150},
  {"NOTE: %s is not for normal use!\n", 151},
  {"%s not allowed with %s!\n", 152},
  {"%s makes no sense with %s!\n", 153},
  {"selected cipher algorithm is invalid\n", 154},
  {"selected digest algorithm is invalid\n", 155},
  {"the given policy URL is invalid\n", 156},
  {"compress algorithm must be in range %d..%d\n", 157},
  {"completes-needed must be greater than 0\n", 158},
  {"marginals-needed must be greater than 1\n", 159},
  {"max-cert-depth must be in range 1 to 255\n", 160},
  {"NOTE: simple S2K mode (0) is strongly discouraged\n", 161},
  {"invalid S2K mode; must be 0, 1 or 3\n", 162},
  {"failed to initialize the TrustDB: %s\n", 163},
  {"--store [filename]", 164},
  {"--symmetric [filename]", 165},
  {"--encrypt [filename]", 166},
  {"--sign [filename]", 167},
  {"--sign --encrypt [filename]", 168},
  {"--clearsign [filename]", 169},
  {"--decrypt [filename]", 170},
  {"--sign-key user-id", 171},
  {"--lsign-key user-id", 172},
  {"--edit-key user-id [commands]", 173},
  {"--delete-secret-key user-id", 174},
  {"--delete-key user-id", 175},
  {"can't open %s: %s\n", 176},
  {"-k[v][v][v][c] [user-id] [keyring]", 177},
  {"dearmoring failed: %s\n", 178},
  {"enarmoring failed: %s\n", 179},
  {"invalid hash algorithm `%s'\n", 180},
  {"[filename]", 181},
  {"Go ahead and type your message ...\n", 182},
  {"can't open `%s'\n", 183},
  {"\
the first character of a notation name must be a letter or an underscore\n", 184},
  {"\
a notation name must have only letters, digits, dots or underscores and end \
with an '='\n", 185},
  {"dots in a notation name must be surrounded by other characters\n", 186},
  {"a notation value must not use any control characters\n", 187},
  {"armor: %s\n", 188},
  {"invalid armor header: ", 189},
  {"armor header: ", 190},
  {"invalid clearsig header\n", 191},
  {"nested clear text signatures\n", 192},
  {"invalid dash escaped line: ", 193},
  {"unexpected armor:", 194},
  {"invalid radix64 character %02x skipped\n", 195},
  {"premature eof (no CRC)\n", 196},
  {"premature eof (in CRC)\n", 197},
  {"malformed CRC\n", 198},
  {"CRC error; %06lx - %06lx\n", 199},
  {"premature eof (in Trailer)\n", 200},
  {"error in trailer line\n", 201},
  {"no valid OpenPGP data found.\n", 202},
  {"invalid armor: line longer than %d characters\n", 203},
  {"\
quoted printable character in armor - probably a buggy MTA has been used\n", 204},
  {"             Fingerprint:", 205},
  {"Fingerprint:", 206},
  {"sSmMqQ", 207},
  {"\
No trust value assigned to %lu:\n\
%4u%c/%08lX %s \"", 208},
  {"\
Please decide how far you trust this user to correctly\n\
verify other users' keys (by looking at passports,\n\
checking fingerprints from different sources...)?\n\
\n\
 1 = Don't know\n\
 2 = I do NOT trust\n\
 3 = I trust marginally\n\
 4 = I trust fully\n\
 s = please show me more information\n", 209},
  {" m = back to the main menu\n", 210},
  {" q = quit\n", 211},
  {"Your decision? ", 212},
  {"Certificates leading to an ultimately trusted key:\n", 213},
  {"\
Could not find a valid trust path to the key.  Let's see whether we\n\
can assign some missing owner trust values.\n\
\n", 214},
  {"\
No path leading to one of our keys found.\n\
\n", 215},
  {"\
No certificates with undefined trust found.\n\
\n", 216},
  {"\
No trust values changed.\n\
\n", 217},
  {"key %08lX: key has been revoked!\n", 218},
  {"Use this key anyway? ", 219},
  {"key %08lX: subkey has been revoked!\n", 220},
  {"%08lX: key has expired\n", 221},
  {"%08lX: no info to calculate a trust probability\n", 222},
  {"%08lX: We do NOT trust this key\n", 223},
  {"\
%08lX: It is not sure that this key really belongs to the owner\n\
but it is accepted anyway\n", 224},
  {"This key probably belongs to the owner\n", 225},
  {"This key belongs to us\n", 226},
  {"\
It is NOT certain that the key belongs to its owner.\n\
If you *really* know what you are doing, you may answer\n\
the next question with yes\n\
\n", 227},
  {"WARNING: Using untrusted key!\n", 228},
  {"WARNING: This key has been revoked by its owner!\n", 229},
  {"         This could mean that the signature is forgery.\n", 230},
  {"WARNING: This subkey has been revoked by its owner!\n", 231},
  {"Note: This key has expired!\n", 232},
  {"WARNING: This key is not certified with a trusted signature!\n", 233},
  {"\
         There is no indication that the signature belongs to the owner.\n", 234},
  {"WARNING: We do NOT trust this key!\n", 235},
  {"         The signature is probably a FORGERY.\n", 236},
  {"\
WARNING: This key is not certified with sufficiently trusted signatures!\n", 237},
  {"         It is not certain that the signature belongs to the owner.\n", 238},
  {"%s: skipped: %s\n", 239},
  {"%s: skipped: public key already present\n", 240},
  {"\
You did not specify a user ID. (you may use \"-r\")\n\
\n", 241},
  {"Enter the user ID: ", 242},
  {"No such user ID.\n", 243},
  {"skipped: public key already set as default recipient\n", 244},
  {"Public key is disabled.\n", 245},
  {"skipped: public key already set with --encrypt-to\n", 246},
  {"unknown default recipient `%s'\n", 247},
  {"%s: error checking key: %s\n", 248},
  {"%s: skipped: public key is disabled\n", 249},
  {"no valid addressees\n", 250},
  {"writing self signature\n", 251},
  {"writing key binding signature\n", 252},
  {"Please select what kind of key you want:\n", 253},
  {"   (%d) DSA and ElGamal (default)\n", 254},
  {"   (%d) DSA (sign only)\n", 255},
  {"   (%d) ElGamal (encrypt only)\n", 256},
  {"   (%d) ElGamal (sign and encrypt)\n", 257},
  {"   (%d) ElGamal in a v3 packet\n", 258},
  {"Your selection? ", 259},
  {"Do you really want to create a sign and encrypt key? ", 260},
  {"Invalid selection.\n", 261},
  {"\
About to generate a new %s keypair.\n\
              minimum keysize is  768 bits\n\
              default keysize is 1024 bits\n\
    highest suggested keysize is 2048 bits\n", 262},
  {"What keysize do you want? (1024) ", 263},
  {"DSA only allows keysizes from 512 to 1024\n", 264},
  {"keysize too small; 768 is smallest value allowed.\n", 265},
  {"keysize too large; %d is largest value allowed.\n", 266},
  {"\
Keysizes larger than 2048 are not suggested because\n\
computations take REALLY long!\n", 267},
  {"Are you sure that you want this keysize? ", 268},
  {"\
Okay, but keep in mind that your monitor and keyboard radiation is also very \
vulnerable to attacks!\n", 269},
  {"Do you really need such a large keysize? ", 270},
  {"Requested keysize is %u bits\n", 271},
  {"rounded up to %u bits\n", 272},
  {"\
Please specify how long the key should be valid.\n\
         0 = key does not expire\n\
      <n>  = key expires in n days\n\
      <n>w = key expires in n weeks\n\
      <n>m = key expires in n months\n\
      <n>y = key expires in n years\n", 273},
  {"Key is valid for? (0) ", 274},
  {"invalid value\n", 275},
  {"Key does not expire at all\n", 276},
  {"Key expires at %s\n", 277},
  {"\
Your system can't display dates beyond 2038.\n\
However, it will be correctly handled up to 2106.\n", 278},
  {"Is this correct (y/n)? ", 279},
  {"\
\n\
You need a User-ID to identify your key; the software constructs the user \
id\n\
from Real Name, Comment and Email Address in this form:\n\
    \"Heinrich Heine (Der Dichter) <heinrichh@duesseldorf.de>\"\n\
\n", 280},
  {"Real name: ", 281},
  {"Invalid character in name\n", 282},
  {"Name may not start with a digit\n", 283},
  {"Name must be at least 5 characters long\n", 284},
  {"Email address: ", 285},
  {"Not a valid email address\n", 286},
  {"Comment: ", 287},
  {"Invalid character in comment\n", 288},
  {"You are using the `%s' character set.\n", 289},
  {"\
You selected this USER-ID:\n\
    \"%s\"\n\
\n", 290},
  {"NnCcEeOoQq", 291},
  {"Change (N)ame, (C)omment, (E)mail or (O)kay/(Q)uit? ", 292},
  {"\
You need a Passphrase to protect your secret key.\n\
\n", 293},
  {"passphrase not correctly repeated; try again.\n", 294},
  {"\
You don't want a passphrase - this is probably a *bad* idea!\n\
I will do it anyway.  You can change your passphrase at any time,\n\
using this program with the option \"--edit-key\".\n\
\n", 295},
  {"\
We need to generate a lot of random bytes. It is a good idea to perform\n\
some other action (type on the keyboard, move the mouse, utilize the\n\
disks) during the prime generation; this gives the random number\n\
generator a better chance to gain enough entropy.\n", 296},
  {"Key generation can only be used in interactive mode\n", 297},
  {"DSA keypair will have 1024 bits.\n", 298},
  {"Key generation canceled.\n", 299},
  {"writing public certificate to `%s'\n", 300},
  {"writing secret certificate to `%s'\n", 301},
  {"public and secret key created and signed.\n", 302},
  {"\
Note that this key cannot be used for encryption.  You may want to use\n\
the command \"--edit-key\" to generate a secondary key for this purpose.\n", 303},
  {"Key generation failed: %s\n", 304},
  {"\
key has been created %lu second in future (time warp or clock problem)\n", 305},
  {"\
key has been created %lu seconds in future (time warp or clock problem)\n", 306},
  {"Really create? ", 307},
  {"%s: can't open: %s\n", 308},
  {"error creating passphrase: %s\n", 309},
  {"%s: WARNING: empty file\n", 310},
  {"reading from `%s'\n", 311},
  {"%s/%s encrypted for: %s\n", 312},
  {"%s: user not found: %s\n", 313},
  {"certificate read problem: %s\n", 314},
  {"key %08lX: not a rfc2440 key - skipped\n", 315},
  {"WARNING: nothing exported\n", 316},
  {"too many entries in pk cache - disabled\n", 317},
  {"too many entries in unk cache - disabled\n", 318},
  {"RSA key cannot be used in this version\n", 319},
  {"No key for user ID\n", 320},
  {"No user ID for key\n", 321},
  {"using secondary key %08lX instead of primary key %08lX\n", 322},
  {"can't open `%s': %s\n", 323},
  {"skipping block of type %d\n", 324},
  {"%lu keys so far processed\n", 325},
  {"error reading `%s': %s\n", 326},
  {"Total number processed: %lu\n", 327},
  {"          w/o user IDs: %lu\n", 328},
  {"              imported: %lu", 329},
  {"             unchanged: %lu\n", 330},
  {"          new user IDs: %lu\n", 331},
  {"           new subkeys: %lu\n", 332},
  {"        new signatures: %lu\n", 333},
  {"   new key revocations: %lu\n", 334},
  {"      secret keys read: %lu\n", 335},
  {"  secret keys imported: %lu\n", 336},
  {" secret keys unchanged: %lu\n", 337},
  {"key %08lX: no user ID\n", 338},
  {"key %08lX: no valid user IDs\n", 339},
  {"this may be caused by a missing self-signature\n", 340},
  {"key %08lX: public key not found: %s\n", 341},
  {"no default public keyring\n", 342},
  {"writing to `%s'\n", 343},
  {"can't lock keyring `%s': %s\n", 344},
  {"error writing keyring `%s': %s\n", 345},
  {"key %08lX: public key imported\n", 346},
  {"key %08lX: doesn't match our copy\n", 347},
  {"key %08lX: can't locate original keyblock: %s\n", 348},
  {"key %08lX: can't read original keyblock: %s\n", 349},
  {"key %08lX: 1 new user ID\n", 350},
  {"key %08lX: %d new user IDs\n", 351},
  {"key %08lX: 1 new signature\n", 352},
  {"key %08lX: %d new signatures\n", 353},
  {"key %08lX: 1 new subkey\n", 354},
  {"key %08lX: %d new subkeys\n", 355},
  {"key %08lX: not changed\n", 356},
  {"key %08lX: secret key imported\n", 357},
  {"key %08lX: already in secret keyring\n", 358},
  {"key %08lX: secret key not found: %s\n", 359},
  {"key %08lX: no public key - can't apply revocation certificate\n", 360},
  {"key %08lX: invalid revocation certificate: %s - rejected\n", 361},
  {"key %08lX: revocation certificate imported\n", 362},
  {"key %08lX: no user ID for signature\n", 363},
  {"key %08lX: unsupported public key algorithm\n", 364},
  {"key %08lX: invalid self-signature\n", 365},
  {"key %08lX: no subkey for key binding\n", 366},
  {"key %08lX: invalid subkey binding\n", 367},
  {"key %08lX: accepted non self-signed user ID '", 368},
  {"key %08lX: skipped user ID '", 369},
  {"key %08lX: skipped subkey\n", 370},
  {"key %08lX: non exportable signature (class %02x) - skipped\n", 371},
  {"key %08lX: revocation certificate at wrong place - skipped\n", 372},
  {"key %08lX: invalid revocation certificate: %s - skipped\n", 373},
  {"key %08lX: duplicated user ID detected - merged\n", 374},
  {"key %08lX: revocation certificate added\n", 375},
  {"key %08lX: our copy has no self-signature\n", 376},
  {"%s: user not found\n", 377},
  {"[revocation]", 378},
  {"[self-signature]", 379},
  {"1 bad signature\n", 380},
  {"%d bad signatures\n", 381},
  {"1 signature not checked due to a missing key\n", 382},
  {"%d signatures not checked due to missing keys\n", 383},
  {"1 signature not checked due to an error\n", 384},
  {"%d signatures not checked due to errors\n", 385},
  {"1 user ID without valid self-signature detected\n", 386},
  {"%d user IDs without valid self-signatures detected\n", 387},
  {"Already signed by key %08lX\n", 388},
  {"Nothing to sign with key %08lX\n", 389},
  {"\
Are you really sure that you want to sign this key\n\
with your key: \"", 390},
  {"\
The signature will be marked as non-exportable.\n\
\n", 391},
  {"Really sign? ", 392},
  {"signing failed: %s\n", 393},
  {"This key is not protected.\n", 394},
  {"Key is protected.\n", 395},
  {"Can't edit this key: %s\n", 396},
  {"\
Enter the new passphrase for this secret key.\n\
\n", 397},
  {"\
You don't want a passphrase - this is probably a *bad* idea!\n\
\n", 398},
  {"Do you really want to do this? ", 399},
  {"moving a key signature to the correct place\n", 400},
  {"quit this menu", 401},
  {"q", 402},
  {"save", 403},
  {"save and quit", 404},
  {"help", 405},
  {"show this help", 406},
  {"fpr", 407},
  {"show fingerprint", 408},
  {"list", 409},
  {"list key and user IDs", 410},
  {"l", 411},
  {"uid", 412},
  {"select user ID N", 413},
  {"key", 414},
  {"select secondary key N", 415},
  {"check", 416},
  {"list signatures", 417},
  {"c", 418},
  {"sign", 419},
  {"sign the key", 420},
  {"s", 421},
  {"lsign", 422},
  {"sign the key locally", 423},
  {"debug", 424},
  {"adduid", 425},
  {"add a user ID", 426},
  {"deluid", 427},
  {"delete user ID", 428},
  {"addkey", 429},
  {"add a secondary key", 430},
  {"delkey", 431},
  {"delete a secondary key", 432},
  {"delsig", 433},
  {"delete signatures", 434},
  {"expire", 435},
  {"change the expire date", 436},
  {"toggle", 437},
  {"toggle between secret and public key listing", 438},
  {"t", 439},
  {"pref", 440},
  {"list preferences", 441},
  {"passwd", 442},
  {"change the passphrase", 443},
  {"trust", 444},
  {"change the ownertrust", 445},
  {"revsig", 446},
  {"revoke signatures", 447},
  {"revkey", 448},
  {"revoke a secondary key", 449},
  {"disable", 450},
  {"disable a key", 451},
  {"enable", 452},
  {"enable a key", 453},
  {"can't do that in batchmode\n", 454},
  {"Secret key is available.\n", 455},
  {"Command> ", 456},
  {"Need the secret key to do this.\n", 457},
  {"Really sign all user IDs? ", 458},
  {"Hint: Select the user IDs to sign\n", 459},
  {"update of trustdb failed: %s\n", 460},
  {"You must select at least one user ID.\n", 461},
  {"You can't delete the last user ID!\n", 462},
  {"Really remove all selected user IDs? ", 463},
  {"Really remove this user ID? ", 464},
  {"You must select at least one key.\n", 465},
  {"Do you really want to delete the selected keys? ", 466},
  {"Do you really want to delete this key? ", 467},
  {"Do you really want to revoke the selected keys? ", 468},
  {"Do you really want to revoke this key? ", 469},
  {"Save changes? ", 470},
  {"Quit without saving? ", 471},
  {"update failed: %s\n", 472},
  {"update secret failed: %s\n", 473},
  {"Key not changed so no update needed.\n", 474},
  {"Invalid command  (try \"help\")\n", 475},
  {"This key has been disabled", 476},
  {"Delete this good signature? (y/N/q)", 477},
  {"Delete this invalid signature? (y/N/q)", 478},
  {"Delete this unknown signature? (y/N/q)", 479},
  {"Really delete this self-signature? (y/N)", 480},
  {"Deleted %d signature.\n", 481},
  {"Deleted %d signatures.\n", 482},
  {"Nothing deleted.\n", 483},
  {"Please remove selections from the secret keys.\n", 484},
  {"Please select at most one secondary key.\n", 485},
  {"Changing expiration time for a secondary key.\n", 486},
  {"Changing expiration time for the primary key.\n", 487},
  {"You can't change the expiration date of a v3 key\n", 488},
  {"No corresponding signature in secret ring\n", 489},
  {"No user ID with index %d\n", 490},
  {"No secondary key with index %d\n", 491},
  {"user ID: \"", 492},
  {"\
\"\n\
signed with your key %08lX at %s\n", 493},
  {"Create a revocation certificate for this signature? (y/N)", 494},
  {"You have signed these user IDs:\n", 495},
  {"   signed by %08lX at %s\n", 496},
  {"   revoked by %08lX at %s\n", 497},
  {"You are about to revoke these signatures:\n", 498},
  {"Really create the revocation certificates? (y/N)", 499},
  {"no secret key\n", 500},
  {"public key is %08lX\n", 501},
  {"public key encrypted data: good DEK\n", 502},
  {"encrypted with %u-bit %s key, ID %08lX, created %s\n", 503},
  {"encrypted with %s key, ID %08lX\n", 504},
  {"no secret key for decryption available\n", 505},
  {"public key decryption failed: %s\n", 506},
  {"decryption okay\n", 507},
  {"WARNING: encrypted message has been manipulated!\n", 508},
  {"decryption failed: %s\n", 509},
  {"NOTE: sender requested \"for-your-eyes-only\"\n", 510},
  {"original file name='%.*s'\n", 511},
  {"standalone revocation - use \"gpg --import\" to apply\n", 512},
  {"WARNING: invalid notation data found\n", 513},
  {"Notation: ", 514},
  {"Policy: ", 515},
  {"signature verification suppressed\n", 516},
  {"Signature made %.*s using %s key ID %08lX\n", 517},
  {"BAD signature from \"", 518},
  {"Good signature from \"", 519},
  {"                aka \"", 520},
  {"Can't check signature: %s\n", 521},
  {"standalone signature of class 0x%02x\n", 522},
  {"old style (PGP 2.x) signature\n", 523},
  {"invalid root packet detected in proc_tree()\n", 524},
  {"can't disable core dumps: %s\n", 525},
  {"WARNING: program may create a core file!\n", 526},
  {"Experimental algorithms should not be used!\n", 527},
  {"\
RSA keys are deprecated; please consider creating a new key and use this key \
in the future\n", 528},
  {"this cipher algorithm is depreciated; please use a more standard one!\n", 529},
  {"can't handle public key algorithm %d\n", 530},
  {"subpacket of type %d has critical bit set\n", 531},
  {"\
\n\
You need a passphrase to unlock the secret key for\n\
user: \"", 532},
  {"%u-bit %s key, ID %08lX, created %s", 533},
  {" (main key ID %08lX)", 534},
  {"can't query password in batchmode\n", 535},
  {"Enter passphrase: ", 536},
  {"Repeat passphrase: ", 537},
  {"data not saved; use option \"--output\" to save it\n", 538},
  {"Detached signature.\n", 539},
  {"Please enter name of data file: ", 540},
  {"reading stdin ...\n", 541},
  {"can't open signed data `%s'\n", 542},
  {"anonymous receiver; trying secret key %08lX ...\n", 543},
  {"okay, we are the anonymous recipient.\n", 544},
  {"old encoding of the DEK is not supported\n", 545},
  {"NOTE: cipher algorithm %d not found in preferences\n", 546},
  {"requesting key %08lX from %s ...\n", 547},
  {"can't get key from keyserver: %s\n", 548},
  {"no keyserver known (use option --keyserver)\n", 549},
  {"%s: not a valid key ID\n", 550},
  {"can't connect to `%s': %s\n", 551},
  {"error sending to `%s': %s\n", 552},
  {"success sending to `%s' (status=%u)\n", 553},
  {"failed sending to `%s': status=%u\n", 554},
  {"protection algorithm %d is not supported\n", 555},
  {"Invalid passphrase; please try again ...\n", 556},
  {"WARNING: Weak key detected - please change passphrase again.\n", 557},
  {"assuming bad MDC due to an unknown critical bit\n", 558},
  {"\
this is a PGP generated ElGamal key which is NOT secure for signatures!\n", 559},
  {"public key is %lu second newer than the signature\n", 560},
  {"public key is %lu seconds newer than the signature\n", 561},
  {"NOTE: signature key expired %s\n", 562},
  {"assuming bad signature due to an unknown critical bit\n", 563},
  {"%s signature from: %s\n", 564},
  {"can't create %s: %s\n", 565},
  {"signing:", 566},
  {"WARNING: `%s' is an empty file\n", 567},
  {"can't handle text lines longer than %d characters\n", 568},
  {"input line longer than %d characters\n", 569},
  {"trustdb rec %lu: lseek failed: %s\n", 570},
  {"trustdb rec %lu: write failed (n=%d): %s\n", 571},
  {"trustdb transaction too large\n", 572},
  {"%s: can't access: %s\n", 573},
  {"%s: directory does not exist!\n", 574},
  {"%s: can't create: %s\n", 575},
  {"%s: can't create lock\n", 576},
  {"%s: failed to create version record: %s", 577},
  {"%s: invalid trustdb created\n", 578},
  {"%s: trustdb created\n", 579},
  {"%s: invalid trustdb\n", 580},
  {"%s: failed to create hashtable: %s\n", 581},
  {"%s: error updating version record: %s\n", 582},
  {"%s: error reading version record: %s\n", 583},
  {"%s: error writing version record: %s\n", 584},
  {"trustdb: lseek failed: %s\n", 585},
  {"trustdb: read failed (n=%d): %s\n", 586},
  {"%s: not a trustdb file\n", 587},
  {"%s: version record with recnum %lu\n", 588},
  {"%s: invalid file version %d\n", 589},
  {"%s: error reading free record: %s\n", 590},
  {"%s: error writing dir record: %s\n", 591},
  {"%s: failed to zero a record: %s\n", 592},
  {"%s: failed to append a record: %s\n", 593},
  {"the trustdb is corrupted; please run \"gpg --fix-trustdb\".\n", 594},
  {"trust record %lu, req type %d: read failed: %s\n", 595},
  {"trust record %lu, type %d: write failed: %s\n", 596},
  {"trust record %lu: delete failed: %s\n", 597},
  {"trustdb: sync failed: %s\n", 598},
  {"error reading dir record for LID %lu: %s\n", 599},
  {"lid %lu: expected dir record, got type %d\n", 600},
  {"no primary key for LID %lu\n", 601},
  {"error reading primary key for LID %lu: %s\n", 602},
  {"get_dir_record: search_record failed: %s\n", 603},
  {"NOTE: secret key %08lX is NOT protected.\n", 604},
  {"key %08lX: secret key without public key - skipped\n", 605},
  {"key %08lX: secret and public key don't match\n", 606},
  {"key %08lX: can't put it into the trustdb\n", 607},
  {"key %08lX: query record failed\n", 608},
  {"key %08lX: already in trusted key table\n", 609},
  {"key %08lX: accepted as trusted key.\n", 610},
  {"enumerate secret keys failed: %s\n", 611},
  {"key %08lX.%lu: Good subkey binding\n", 612},
  {"key %08lX.%lu: Invalid subkey binding: %s\n", 613},
  {"key %08lX.%lu: Valid key revocation\n", 614},
  {"key %08lX.%lu: Invalid key revocation: %s\n", 615},
  {"key %08lX.%lu: Valid subkey revocation\n", 616},
  {"Good self-signature", 617},
  {"Invalid self-signature", 618},
  {"Valid user ID revocation skipped due to a newer self signature", 619},
  {"Valid user ID revocation", 620},
  {"Invalid user ID revocation", 621},
  {"Valid certificate revocation", 622},
  {"Good certificate", 623},
  {"Invalid certificate revocation", 624},
  {"Invalid certificate", 625},
  {"sig record %lu[%d] points to wrong record.\n", 626},
  {"duplicated certificate - deleted", 627},
  {"tdbio_search_dir failed: %s\n", 628},
  {"lid ?: insert failed: %s\n", 629},
  {"lid %lu: insert failed: %s\n", 630},
  {"lid %lu: inserted\n", 631},
  {"error reading dir record: %s\n", 632},
  {"%lu keys processed\n", 633},
  {"\t%lu keys with errors\n", 634},
  {"\t%lu keys inserted\n", 635},
  {"enumerate keyblocks failed: %s\n", 636},
  {"lid %lu: dir record w/o key - skipped\n", 637},
  {"\t%lu due to new pubkeys\n", 638},
  {"\t%lu keys skipped\n", 639},
  {"\t%lu keys updated\n", 640},
  {"Ooops, no keys\n", 641},
  {"Ooops, no user IDs\n", 642},
  {"check_trust: search dir record failed: %s\n", 643},
  {"key %08lX: insert trust record failed: %s\n", 644},
  {"key %08lX.%lu: inserted into trustdb\n", 645},
  {"key %08lX.%lu: created in future (time warp or clock problem)\n", 646},
  {"key %08lX.%lu: expired at %s\n", 647},
  {"key %08lX.%lu: trust check failed: %s\n", 648},
  {"user '%s' not found: %s\n", 649},
  {"problem finding '%s' in trustdb: %s\n", 650},
  {"user '%s' not in trustdb - inserting\n", 651},
  {"failed to put '%s' into trustdb: %s\n", 652},
  {"WARNING: can't yet handle long pref records\n", 653},
  {"\
the signature could not be verified.\n\
Please remember that the signature file (.sig or .asc)\n\
should be the first file given on the command line.\n", 654},
  {"input line %u too long or missing LF\n", 655},
  {"%s: can't create keyring: %s\n", 656},
  {"%s: keyring created\n", 657},
  {"WARNING: 2 files with confidential information exists.\n", 658},
  {"%s is the unchanged one\n", 659},
  {"%s is the new one\n", 660},
  {"Please fix this possible security flaw\n", 661},
  {"key is not flagged as insecure - can't use it with the faked RNG!\n", 662},
  {"skipped `%s': %s\n", 663},
  {"\
skipped `%s': this is a PGP generated ElGamal key which is not secure for \
signatures!\n", 664},
  {"File `%s' exists. ", 665},
  {"Overwrite (y/N)? ", 666},
  {"%s: unknown suffix\n", 667},
  {"Enter new filename", 668},
  {"writing to stdout\n", 669},
  {"assuming signed data in `%s'\n", 670},
  {"%s: new options file created\n", 671},
  {"%s: can't create directory: %s\n", 672},
  {"%s: directory created\n", 673},
  {"you have to start GnuPG again, so it can read the new options file\n", 674},
  {"%s encrypted data\n", 675},
  {"encrypted with unknown algorithm %d\n", 676},
  {"\
WARNING: message was encrypted with a weak key in the symmetric cipher.\n", 677},
  {"weak key created - retrying\n", 678},
  {"cannot avoid weak key for symmetric cipher; tried %d times!\n", 679},
  {"there is a secret key for this public key!\n", 680},
  {"use option \"--delete-secret-key\" to delete it first.\n", 681},
  {"can't do that in batchmode without \"--yes\"\n", 682},
  {"Delete this key from the keyring? ", 683},
  {"This is a secret key! - really delete? ", 684},
  {"\
It's up to you to assign a value here; this value will never be exported\n\
to any 3rd party.  We need it to implement the web-of-trust; it has nothing\n\
to do with the (implicitly created) web-of-certificates.", 685},
  {"If you want to use this revoked key anyway, answer \"yes\".", 686},
  {"If you want to use this untrusted key anyway, answer \"yes\".", 687},
  {"\
Enter the user ID of the addressee to whom you want to send the message.", 688},
  {"\
Select the algorithm to use.\n\
\n\
DSA (aka DSS) is the digital signature algorithm which can only be used\n\
for signatures.  This is the suggested algorithm because verification of\n\
DSA signatures are much faster than those of ElGamal.\n\
\n\
ElGamal is an algorithm which can be used for signatures and encryption.\n\
OpenPGP distinguishs between two flavors of this algorithms: an encrypt \
only\n\
and a sign+encrypt; actually it is the same, but some parameters must be\n\
selected in a special way to create a safe key for signatures: this program\n\
does this but other OpenPGP implementations are not required to understand\n\
the signature+encryption flavor.\n\
\n\
The first (primary) key must always be a key which is capable of signing;\n\
this is the reason why the encryption only ElGamal key is not available in\n\
this menu.", 689},
  {"\
Although these keys are defined in RFC2440 they are not suggested\n\
because they are not supported by all programs and signatures created\n\
with them are quite large and very slow to verify.", 690},
  {"Enter the size of the key", 691},
  {"Answer \"yes\" or \"no\"", 692},
  {"\
Enter the required value as shown in the prompt.\n\
It is possible to enter a ISO date (YYYY-MM-DD) but you won't\n\
get a good error response - instead the system tries to interpret\n\
the given value as an interval.", 693},
  {"Enter the name of the key holder", 694},
  {"please enter an optional but highly suggested email address", 695},
  {"Please enter an optional comment", 696},
  {"\
N  to change the name.\n\
C  to change the comment.\n\
E  to change the email address.\n\
O  to continue with key generation.\n\
Q  to to quit the key generation.", 697},
  {"Answer \"yes\" (or just \"y\") if it is okay to generate the sub key.", 698},
  {"Answer \"yes\" is you want to sign ALL the user IDs", 699},
  {"\
Answer \"yes\" if you really want to delete this user ID.\n\
All certificates are then also lost!", 700},
  {"Answer \"yes\" if it is okay to delete the subkey", 701},
  {"\
This is a valid signature on the key; you normally don't want\n\
to delete this signature because it may be important to establish a\n\
trust connection to the key or another key certified by this key.", 702},
  {"\
This signature can't be checked because you don't have the\n\
corresponding key.  You should postpone its deletion until you\n\
know which key was used because this signing key might establish\n\
a trust connection through another already certified key.", 703},
  {"\
The signature is not valid.  It does make sense to remove it from\n\
your keyring.", 704},
  {"\
This is a signature which binds the user ID to the key. It is\n\
usually not a good idea to remove such a signature.  Actually\n\
GnuPG might not be able to use this key anymore.  So do this\n\
only if this self-signature is for some reason not valid and\n\
a second one is available.", 705},
  {"\
Please enter the passhrase; this is a secret sentence \n\
  Blurb, blurb,.... ", 706},
  {"Please repeat the last passphrase, so you are sure what you typed in.", 707},
  {"Give the name of the file to which the signature applies", 708},
  {"Answer \"yes\" if it is okay to overwrite the file", 709},
  {"\
Please enter a new filename. If you just hit RETURN the default\n\
file (which is shown in brackets) will be used.", 710},
  {"No help available", 711},
  {"No help available for `%s'", 712},
};

int _msg_tbl_length = 712;
