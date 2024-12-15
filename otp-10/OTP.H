/* The signatures */
#define CIPHER_SIGNATURE "OTP-PC 1.0 Cipher"
#define PAD_SIGNATURE "OTP-PC 1.0 Pad"


/* pad structure */
struct padstruc {
char signature[25];
long bookmark;
int type;             /* is this an encrypt or decrypt file?
                         Initially, the pad is undefined (0), but
			 the first time it is used it becomes either
			 an encode (1) or decode (2) type.   This helps
			 prevent  misuse */

};

/* encrypted structure */
struct encryptedstruc {
 unsigned long crc32;     /* 32 bit CRC */
 char outfilename[13];
 int compression;       /* 0 if no compression, positive integer value
			  represents type of compression.  Currently only
			  type 1 (Shareware LZH technique)*/
 int future;            /* reserved for future use */
};

/* this is the unencrypted structure that is used a the encrypted file header */

struct cipherstruc {
char signature[25];    /* signature to indicate this is a OPT-PC format */
char version[10];      /* version of program used */
long bookmark;         /* starting point in codebook (pad) */
long start;            /* starting point in cipher file */
char padfilename[13];  /* name of the pad file used to encrypt file */
};


struct general {
long CodeBookSize,   /* total size of usable space total in codebook */
     CodeBookRemain, /* unused space remaining */
     SessionBegin,   /* start of pad used in this session */
     SessionEnd,     /* end of pad used in this session */
     CompFileSize,   /* size of the compressed file (if any) */
     SourceFileSize;     /* size of source file (plain text) */
     unsigned long crc32;      /* 32 bit CRC (8 characters + /0 terminator */

     char fileone[80];        /* name of the input file */
     char filetwo[80],  /* file name for the pad (codebook) */

     outfile[80],       /* the name of the output file */
     compfile[80];      /* the name of the compressed temp file */
     int action;        /* action to take (encode, decode, etc) */
     int verbose;       /* verbose mode (makes program talks a lot ) */
     long padfilesize;  /* The size of the pad file (codebook) */
     int compression;   /*  is compression used */
} ;



/* function prototypes */
int encode(void);
int initcodeheader(char *filename);
int getcrc32( unsigned long *CRC32, char *filename);
int decode(void);
void copy (char *from,char *to,int first,int length);
void GetTempFile (char *path, char *ext);
int is_file(char *filename);
void error_exit( int error_code);
int CreatePad(void);
void lzhDecode( FILE *in, FILE *out );
int lzhEncode( FILE *in, FILE *out );
int LocateChar(char *string, char ch);
void SetOutName(char *ext);
int ProcessOptions(int argc,char *argv[]);
long unsigned GetFileSize( char *filename);
int wipefile(char *filename);
int extract_name(char *fullpath, char *name);
int SimpleXOR(void);
void force_extension(char *filename, char *extension);
void default_extension(char *filename, char *extension);
int find_right_char( char *string, char ch);
char yes_no(void);
void PrintArray(int start, char *array[]);
void WipeArea(FILE *file, long begin, long end);


/* this is for actions to take */
#define NONE   0
#define ENCODE 1
#define DECODE 2
#define CREATE_PAD 3
#define XOR 4
#define WIPE 5
#define HELP 6

#define TRUE 1
#define FALSE 0
#define MAX_STRING 200


/* Error messages defined */
#define NO_OPEN_IN 1
#define NO_PAD_FILE   2
#define INVALID_PAD 3
#define NO_FILES_GIVEN 4
#define NO_ACTION_GIVEN 5
#define INCORRECT_ARGS 6
#define INVALID_CIPHER 7
#define BAD_CRC 8
#define FILE_TOO_BIG 9
#define CANT_WIPE_FILE 10
#define NO_CREATE_PAD 11
#define BAD_OPTION 12
#define NOT_ENCODE_PAD 13
#define NOT_DECODE_PAD 14
#define NEED_MORE_NAMES 15
#define NO_OPEN_OUT 16
#define NO_OPEN_COMPRESSED 17
#define NO_HEADER_FILE 18

#define UNDEFINED 0
#define ENCODE 1
#define DECODE 2

