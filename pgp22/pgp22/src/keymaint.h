#ifndef KEYMAINT_H
#define KEYMAINT_H

int maint_update(char *ringfile);
int maint_check(char *ringfile, int options);
void init_trust_lst(void);

/* Show key in file f at keypos */
int show_key(FILE *f, long keypos, int what);

/* possible values for 'what' of show_key() */
#define	SHOW_TRUST		1
#define	SHOW_SIGS		2
#define	SHOW_HASH		4
#define	SHOW_LISTFMT	8
#define	SHOW_ALL		(SHOW_TRUST|SHOW_SIGS)

/* options for maint_check() */
#define	MAINT_CHECK				0x01
#define	MAINT_VERBOSE			0x02
#define	MAINT_SILENT			0x04


int readkpacket(FILE *f, byte *ctb, char *userid, byte *keyID, byte *sigkeyID);
int read_trust(FILE *f, byte *keyctrl);
void write_trust (FILE *f, byte trustbyte);
void write_trust_pos(FILE *f, byte keyctrl, long pos);

int ask_owntrust(char *userid, byte cur_trust);

char * user_from_keyID(byte *keyID);
int setkrent(char *keyring);
void endkrent();
int init_userhash();


extern char trust_lst[8][16];

#endif
