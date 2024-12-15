//  wp header file
//  header info for the Word for Windows Password Recovery tool
//  Lyal Collins, 1995

#define LINE_MAX 16          // Length of the longest line
#define LOOP_MAX 15          // Length of the longest line
#define BLOCK_MAX 64         // Length of the longest line
#define WORD2       0xA5DB   //sig in word 2 files
#define WORD6       0xCFD0   //sig in word 6 files
#define WORD6_EMBED 0xA5DC   //embedded sig in word 6 files
#define cmdlen		81		 //max command line len
#define  LOWER_Y        0x79
#define  UPPER_Y        0x59
#define  RETURN         0x0D


typedef unsigned char UCHAR;  // Define a UCHAR
typedef unsigned int UINT;    // Define a UINT
typedef struct hdrblock {
	UINT sig;
	unsigned char dummy[12];
    long password;
    unsigned char dummy2[6];
    long start_text;
    long end_text;
	} hdrblock;

hdrblock headerblock;

typedef char cmd_line[cmdlen];
cmd_line filename;                     //output filename buffer

FILE *in;                              // Input file
FILE *out;                             // Output file

//   char types
UCHAR   file_block[LINE_MAX+1];        // 1 file string of 16 bytes
UCHAR   enc_string[LINE_MAX+1];        // my derived encrypt string
UCHAR   count_string[LINE_MAX+1];      //working space while deriving keys
UCHAR   maxcount_string[LINE_MAX+1];   //working space while deriving keys
UCHAR   workblock[BLOCK_MAX][LINE_MAX];  //working space, 1024 byte array
UCHAR   test;                            //a test char
//      help screen stuff
char scrn_buffer[4096];		           //main screen buffer, when in help mode
char help_buffer[] = {                 //help display
"Usage - WP1 <in-file> <out-file> [/d] [/u] [/l]\n\
F1  Shows this help menu\n\
Recovers text ONLY in password protected Word for Windows files.\n\
Tested with Word Versions 2.0a-c and version 6.0a files.\n\
Input, output files, and at least one character list must be specified.\n\
Three character lists are available :\n\
   /d   selects digits 0 - 9, the the document contains many figures.\n\
   /u   selects UPPER case letters. Use this if the file has a lot of\n\
        UPPER case characters.\n\
   /l   selects lower case letters. This should be used first, as the\n\
        the majority of text in documents is lower case.\n\
At least one character lists must be selected.\n\
Once the key has been derived, you may need to adjust the key string to ensure\n\
the correct text is recovered.\n\
The LEFT and RIGHT arrow keys (<- and ->) to move the cursor sideways.\n\
The UP and Down arrow keys adjust the key until the correct text is found.\n\
Esc or Return end key adjustment stage.\n\
Pressing Y or y will then store the recovered text in <out file name>.\n\
Press a key to return to the main program."
};

//-----  My alphabet stuff  -------
UCHAR Alphabet[80] = " \r";            //make the alphabet in here
UCHAR *alpha_lower =       {"abcdefghijklmonpqrstuvwxyz"};
UCHAR *alpha_digits =    "0123456789";
UCHAR *alpha_upper =     "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
//    long types
long word6_sig[] = {WORD6};   //word 6 sig bytes
long word2_sig[] = {WORD2};   //word 2 sig bytes
long starttext;               //start of text pointer
long endtext;                 //end of text pointer
long blocks;                  //number of blocks read
long offset;                  //file pointer

//   int types
UINT i, j, k, l;
UINT test_sig;                //test spot to find sig
UINT enc_string_offset;
int ch;
int top = 1;                  //SCREEN VALUES
int left = 1;
int	right = 80;
int bottom = 25;


//-----  Function prototypes. ----------------
void main(int, char **);      // Main function
void Error(char *, char *);   // Error routine
void do_word2(void);          //do as a word for win 2 document
void do_word6(void);          //do as a word for win 6a document
int test_hdr(void);
void find_key(void);
void DO_MAIN_HELP(void);
void dispatch(void);
void display_workbuf(void);                 //display result buffer
void DO_KEY_LEFT(void);
void DO_KEY_RIGHT(void);
void DO_KEY_END(void);
void DO_KEY_HOME(void);
void DO_KEY_DOWN(void);
void DO_KEY_UP(void);
void DO_ESC_KEY(void);
void do_after_key(void);
void key_adjust(void);       //now, adjust the key, if necessary
void dump_to_disk();         //dump data to disk
void write_buf(FILE *stream, long blocks );
int is_ok(void);
void dump_to_disk(void);

