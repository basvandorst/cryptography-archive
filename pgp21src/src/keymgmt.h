/* keymgmt.h - headers for keymgmt.c 
*/

/*	Return printable public key fragment. */
char *keyIDstring(byte *keyID);
char *key2IDstring(unitptr n);

/* Do an RSA key pair generation, and write them out to the keyring files. */
int dokeygen(char *numstr, char *numstr2);

/* Edit the userid and/or pass phrase for an RSA key pair, and put them	back
   into the ring files */
int dokeyedit(char *mcguffin, char *ringfile);

/* Copy the first entry in key ring that has mcguffin string in userid and
   put it into keyfile */
int extract_from_keyring (char *mcguffin, char *keyfile, char *ringfile,
						  boolean transflag);

/* Lists all entries in keyring that have mcguffin string in userid */
int view_keyring(char *mcguffin, char *ringfile, boolean show_signatures, boolean show_hashes);

/* Signature-check all entries in keyring that have mcguffin string in userid */
/* or if chk_keyID is not NULL, all signatures made by the key with this keyID */
int dokeycheck(char *mcguffin, char *ringfile, byte *chk_keyID);

/* Allow user to remove signatures from keys in keyring that have mcguffin */
int remove_sigs(char *mcguffin, char *ringfile);

/* Remove the first entry in key ring that has mcguffin string in userid */
int remove_from_keyring(byte *keyID, char *mcguffin, char *ringfile, boolean secring_too);

/* Adds (prepends) key file to key ring file */
int addto_keyring(char *keyfile, char *ringfile, boolean query);

/* Extract key fragment from modulus n */
void extract_keyID(byteptr keyID, unitptr n);

/* Write message prefix keyID to a file */
void writekeyID(unitptr n, FILE *f);

/* Extract public key corresponding to keyID or userid from keyfile */
int getpublickey(boolean giveup, boolean showkey, char *keyfile,
				 long *file_position, int *pktlen, byte *keyID,
				 byte *timestamp, byte *userid, unitptr n, unitptr e);

/* Extract private key corresponding to keyID or userid from keyfile */
int getsecretkey(boolean giveup, boolean showkey, char *keyfile, byte *keyID,
				 byte *timestamp, char *passp, boolean *hkey, byte *userid,
				 unitptr n, unitptr e, unitptr d, unitptr p, unitptr q,
				 unitptr u);

/* Return true if ctb is one for a key in a keyring */
int is_key_ctb (byte ctb);

/* Read next key packet from file f, return its ctb in *pctb, and advance
 * the file pointer to point beyond the key packet.
 */
short nextkeypacket(FILE *f, byte *pctb);

/* Read the next key packet from file f, return info about it in the various
 * pointers.  Most pointers can be NULL without breaking it.
 */
short readkeypacket(FILE *f, boolean hidekey, byte *pctb,
	byte *timestamp, char *userid,
	unitptr n ,unitptr e, unitptr d, unitptr p, unitptr q, unitptr u,
	byte *sigkeyID, byte *keyctrl);

/* Starting at key_position in keyfile, scan for the userid packet which
 * matches C string userid.  Return the packet position and size.
 */
int getpubuserid(char *keyfile, long key_position, byte *userid,
	long *userid_position, int *userid_len, boolean exact_match);

int getpubusersig(char *keyfile, long user_position, byte *sigkeyID,
	long *sig_position, int *sig_len);

/* Write key control packet */
void write_trust (FILE *f, byte trustbyte);

int read_trust(FILE *f, byte *keyctrl);

int ask_owntrust(char *userid, byte cur_trust);

int readkpacket(FILE *f, byte *ctb, char *userid, byte *keyID, byte *sigkeyID);

/* Show key in file f at keypos */
int show_key(FILE *f, long keypos, int what);

/* possible values for 'what' */
#define	SHOW_TRUST		1
#define	SHOW_SIGS		2
#define	SHOW_ALL		(SHOW_TRUST|SHOW_SIGS)
