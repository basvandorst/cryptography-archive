/* crypto.h - C headers for crypto.c 
*/

/* Used to determine if nesting should be allowed */
boolean legal_ctb(byte ctb);

/* Write an RSA-signed message digest of input file to specified output
   file, and append input file to output file */
int signfile(boolean nested, boolean separate_signature, char *mcguffin,
			 char *infile, char *outfile, char lit_mode, char *literalfile);

/* Reads the first count bytes from infile into header */
int get_header_info_from_file(char *infile, byte *header, int count);

/* RSA-encrypt a file */
int encryptfile(char *mcguffin, char *infile, char *outfile, 
	boolean attempt_compression);

/* Encrypt file with IPES/IDEA cipher */
int idea_encryptfile(char *infile, char *outfile, 
	boolean attempt_compression);

/* Prepend a CTB_LITERAL prefix to a file */
int make_literal(char *infile, char *outfile, char lit_mode, char *literalfile);

/* Decrypt a file (RSA or IPES/IDEA) */
int decryptfile(char *infile, char *outfile);

/* Check signature in infile for validity.  Strip off the signature and
   write the remaining packet to outfile */
int check_signaturefile(char *infile, char *outfile, boolean strip_signature,
			char *preserved_name);

/* Decrypt file with IPES/IDEA only */
int idea_decryptfile(char *infile, char *outfile);

/* Decompress a file with ZIP algorithm */
int decompress_file(char *infile, char *outfile);

/* Strip off literal prefix from infile, copying to outfile */
int strip_literal(char *infile, char *outfile, char *preserved_name,
		char *lit_mode);

/* Convert C <-> Quiche strings */
void CToPascal(char *s);
void PascalToC( char *s );

/* Reads IPES/IDEA random key and random number seed from file, cranks the
   the seed through the idearand strong pseudorandom number generator, and
   writes them back out */
int strong_pseudorandom(byte *buf, int bufsize);

/*	Return date string, given pointer to 32-bit timestamp */
char *cdate(word32 *tstamp);

/*	Return date and time string, given pointer to 32-bit timestamp */
char *ctdate(word32 *tstamp);

/* Return current timestamp as a byte array and as a 32-bit word */
word32 get_timestamp(word32 *timestamp);

/*	Returns the length of a packet according to the CTB and the length field */
word32 getpastlength(byte ctb, FILE *f);

/* Write a CTB with the appropriate length field */
void write_ctb_len (FILE *f, byte ctb_type, word32 length, boolean big);

/* Print an error message and return nonzero if val != checkval */
int version_error (int val, int checkval);

int check_key_sig(FILE *fkey, long fpkey, int keypktlen, char *keyuserid,
	 FILE *fsig, long fpsig, char *keyfile, char *siguserid, byte *xtimestamp,
	 byte *sigclass);

int squish_file(char *infile, char *outfile);

int signkey(char *keyguffin, char *sigguffin, char *keyfile);

