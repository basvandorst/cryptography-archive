/*
 * Charmap.c
 *
 * Mapping tables between different character sets.
 *
 * Written by colin Plumb.
 *
 * $Id: pgpCharMap.c,v 1.8 1997/09/15 21:21:54 hal Exp $
 */
#include <string.h>

#include "pgpConfig.h"

#include <ctype.h>

#include "pgpCharMap.h"
#include "pgpErrors.h"

/**********************************************************************/
/* *INDENT-OFF* */
unsigned char const charMapIdentity[256] = {
  0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,
 16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,
 32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,
 48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,
 64,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79,
 80,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90,  91,  92,  93,  94,  95,
 96,  97,  98,  99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127,
128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143,
144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159,
160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175,
176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191,
192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207,
208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223,
224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239,
240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255
};
/* *INDENT-ON* */

/* Conversion table from `latin1' charset to `cp850' charset.
 * Generated mechanically by GNU recode 3.4.
 *
 * The recoding should be reversible.
 */
static unsigned char const latin1_to_cp850[256] =
{
      0,   1,   2,   3,   4,   5,   6,   7,	/*   0 -   7 */
      8,   9,  10,  11,  12,  13,  14,  15,	/*   8 -  15 */
     16,  17,  18,  19,  20,  21,  22,  23,	/*  16 -  23 */
     24,  25,  26,  27,  28,  29,  30,  31,	/*  24 -  31 */
     32,  33,  34,  35,  36,  37,  38,  39,	/*  32 -  39 */
     40,  41,  42,  43,  44,  45,  46,  47,	/*  40 -  47 */
     48,  49,  50,  51,  52,  53,  54,  55,	/*  48 -  55 */
     56,  57,  58,  59,  60,  61,  62,  63,	/*  56 -  63 */
     64,  65,  66,  67,  68,  69,  70,  71,	/*  64 -  71 */
     72,  73,  74,  75,  76,  77,  78,  79,	/*  72 -  79 */
     80,  81,  82,  83,  84,  85,  86,  87,	/*  80 -  87 */
     88,  89,  90,  91,  92,  93,  94,  95,	/*  88 -  95 */
     96,  97,  98,  99, 100, 101, 102, 103,	/*  96 - 103 */
    104, 105, 106, 107, 108, 109, 110, 111,	/* 104 - 111 */
    112, 113, 114, 115, 116, 117, 118, 119,	/* 112 - 119 */
    120, 121, 122, 123, 124, 125, 126, 127,	/* 120 - 127 */
    195, 179, 218, 200, 186, 203, 213, 204,	/* 128 - 135 */
    219, 217, 254, 180, 238, 178, 196, 197,	/* 136 - 143 */
    201, 230, 202, 194, 247, 242, 185, 191,	/* 144 - 151 */
    223, 205, 220, 176, 250, 177, 206, 159,	/* 152 - 159 */
    255, 173, 189, 156, 207, 190, 221, 245,	/* 160 - 167 */
    249, 184, 166, 174, 170, 240, 169, 187,	/* 168 - 175 */
    248, 241, 253, 252, 239, 193, 244, 192,	/* 176 - 183 */
    188, 251, 167, 175, 172, 171, 243, 168,	/* 184 - 191 */
    183, 181, 182, 199, 142, 143, 146, 128,	/* 192 - 199 */
    212, 144, 210, 211, 222, 214, 215, 216,	/* 200 - 207 */
    209, 165, 227, 224, 226, 229, 153, 158,	/* 208 - 215 */
    157, 235, 233, 234, 154, 237, 231, 225,	/* 216 - 223 */
    133, 160, 131, 198, 132, 134, 145, 135,	/* 224 - 231 */
    138, 130, 136, 137, 141, 161, 140, 139,	/* 232 - 239 */
    208, 164, 149, 162, 147, 228, 148, 246,	/* 240 - 247 */
    155, 151, 163, 150, 129, 236, 232, 152	/* 248 - 255 */
};

/* Conversion table from `cp850' charset to `latin1' charset.
 * Generated mechanically by GNU recode 3.4.
 *
 * The recoding should be reversible.
 */
static unsigned char const cp850_to_latin1[256] =
{
      0,   1,   2,   3,   4,   5,   6,   7,	/*   0 -   7 */
      8,   9,  10,  11,  12,  13,  14,  15,	/*   8 -  15 */
     16,  17,  18,  19,  20,  21,  22,  23,	/*  16 -  23 */
     24,  25,  26,  27,  28,  29,  30,  31,	/*  24 -  31 */
     32,  33,  34,  35,  36,  37,  38,  39,	/*  32 -  39 */
     40,  41,  42,  43,  44,  45,  46,  47,	/*  40 -  47 */
     48,  49,  50,  51,  52,  53,  54,  55,	/*  48 -  55 */
     56,  57,  58,  59,  60,  61,  62,  63,	/*  56 -  63 */
     64,  65,  66,  67,  68,  69,  70,  71,	/*  64 -  71 */
     72,  73,  74,  75,  76,  77,  78,  79,	/*  72 -  79 */
     80,  81,  82,  83,  84,  85,  86,  87,	/*  80 -  87 */
     88,  89,  90,  91,  92,  93,  94,  95,	/*  88 -  95 */
     96,  97,  98,  99, 100, 101, 102, 103,	/*  96 - 103 */
    104, 105, 106, 107, 108, 109, 110, 111,	/* 104 - 111 */
    112, 113, 114, 115, 116, 117, 118, 119,	/* 112 - 119 */
    120, 121, 122, 123, 124, 125, 126, 127,	/* 120 - 127 */
    199, 252, 233, 226, 228, 224, 229, 231,	/* 128 - 135 */
    234, 235, 232, 239, 238, 236, 196, 197,	/* 136 - 143 */
    201, 230, 198, 244, 246, 242, 251, 249,	/* 144 - 151 */
    255, 214, 220, 248, 163, 216, 215, 159,	/* 152 - 159 */
    225, 237, 243, 250, 241, 209, 170, 186,	/* 160 - 167 */
    191, 174, 172, 189, 188, 161, 171, 187,	/* 168 - 175 */
    155, 157, 141, 129, 139, 193, 194, 192,	/* 176 - 183 */
    169, 150, 132, 175, 184, 162, 165, 151,	/* 184 - 191 */
    183, 181, 147, 128, 142, 143, 227, 195,	/* 192 - 199 */
    131, 144, 146, 133, 135, 153, 158, 164,	/* 200 - 207 */
    240, 208, 202, 203, 200, 134, 205, 206,	/* 208 - 215 */
    207, 137, 130, 136, 154, 166, 204, 152,	/* 216 - 223 */
    211, 223, 212, 210, 245, 213, 145, 222,	/* 224 - 231 */
    254, 218, 219, 217, 253, 221, 140, 180,	/* 232 - 239 */
    173, 177, 149, 190, 182, 167, 247, 148,	/* 240 - 247 */
    176, 168, 156, 185, 179, 178, 138, 160	/* 248 - 255 */
};

/* Conversion table from `latin1' charset to `ebcdic' charset.
   Generated mechanically by GNU recode 3.4.

   The recoding should be reversible.
*/

static unsigned char const latin1_to_ebcdic[256] =
  {
      0,   1,   2,   3,  55,  45,  46,  47,	/*   0 -   7 */
     22,   5,  37,  11,  12,  13,  14,  15,	/*   8 -  15 */
     16,  17,  18,  19,  60,  61,  50,  38,	/*  16 -  23 */
     24,  25,  63,  39,  28,  29,  30,  31,	/*  24 -  31 */
     64,  79, 127, 123,  91, 108,  80, 125,	/*  32 -  39 */
     77,  93,  92,  78, 107,  96,  75,  97,	/*  40 -  47 */
    240, 241, 242, 243, 244, 245, 246, 247,	/*  48 -  55 */
    248, 249, 122,  94,  76, 126, 110, 111,	/*  56 -  63 */
    124, 193, 194, 195, 196, 197, 198, 199,	/*  64 -  71 */
    200, 201, 209, 210, 211, 212, 213, 214,	/*  72 -  79 */
    215, 216, 217, 226, 227, 228, 229, 230,	/*  80 -  87 */
    231, 232, 233,  74, 224,  90,  95, 109,	/*  88 -  95 */
    121, 129, 130, 131, 132, 133, 134, 135,	/*  96 - 103 */
    136, 137, 145, 146, 147, 148, 149, 150,	/* 104 - 111 */
    151, 152, 153, 162, 163, 164, 165, 166,	/* 112 - 119 */
    167, 168, 169, 192, 106, 208, 161,   7,	/* 120 - 127 */
     32,  33,  34,  35,  36,  21,   6,  23,	/* 128 - 135 */
     40,  41,  42,  43,  44,   9,  10,  27,	/* 136 - 143 */
     48,  49,  26,  51,  52,  53,  54,   8,	/* 144 - 151 */
     56,  57,  58,  59,   4,  20,  62, 225,	/* 152 - 159 */
     65,  66,  67,  68,  69,  70,  71,  72,	/* 160 - 167 */
     73,  81,  82,  83,  84,  85,  86,  87,	/* 168 - 175 */
     88,  89,  98,  99, 100, 101, 102, 103,	/* 176 - 183 */
    104, 105, 112, 113, 114, 115, 116, 117,	/* 184 - 191 */
    118, 119, 120, 128, 138, 139, 140, 141,	/* 192 - 199 */
    142, 143, 144, 154, 155, 156, 157, 158,	/* 200 - 207 */
    159, 160, 170, 171, 172, 173, 174, 175,	/* 208 - 215 */
    176, 177, 178, 179, 180, 181, 182, 183,	/* 216 - 223 */
    184, 185, 186, 187, 188, 189, 190, 191,	/* 224 - 231 */
    202, 203, 204, 205, 206, 207, 218, 219,	/* 232 - 239 */
    220, 221, 222, 223, 234, 235, 236, 237,	/* 240 - 247 */
    238, 239, 250, 251, 252, 253, 254, 255,	/* 248 - 255 */
  };

/* Conversion table from `ebcdic' charset to `latin1' charset.
   Generated mechanically by GNU recode 3.4.

   The recoding should be reversible.
*/

static unsigned char const ebcdic_to_latin1[256] =
  {
      0,   1,   2,   3, 156,   9, 134, 127,	/*   0 -   7 */
    151, 141, 142,  11,  12,  13,  14,  15,	/*   8 -  15 */
     16,  17,  18,  19, 157, 133,   8, 135,	/*  16 -  23 */
     24,  25, 146, 143,  28,  29,  30,  31,	/*  24 -  31 */
    128, 129, 130, 131, 132,  10,  23,  27,	/*  32 -  39 */
    136, 137, 138, 139, 140,   5,   6,   7,	/*  40 -  47 */
    144, 145,  22, 147, 148, 149, 150,   4,	/*  48 -  55 */
    152, 153, 154, 155,  20,  21, 158,  26,	/*  56 -  63 */
     32, 160, 161, 162, 163, 164, 165, 166,	/*  64 -  71 */
    167, 168,  91,  46,  60,  40,  43,  33,	/*  72 -  79 */
     38, 169, 170, 171, 172, 173, 174, 175,	/*  80 -  87 */
    176, 177,  93,  36,  42,  41,  59,  94,	/*  88 -  95 */
     45,  47, 178, 179, 180, 181, 182, 183,	/*  96 - 103 */
    184, 185, 124,  44,  37,  95,  62,  63,	/* 104 - 111 */
    186, 187, 188, 189, 190, 191, 192, 193,	/* 112 - 119 */
    194,  96,  58,  35,  64,  39,  61,  34,	/* 120 - 127 */
    195,  97,  98,  99, 100, 101, 102, 103,	/* 128 - 135 */
    104, 105, 196, 197, 198, 199, 200, 201,	/* 136 - 143 */
    202, 106, 107, 108, 109, 110, 111, 112,	/* 144 - 151 */
    113, 114, 203, 204, 205, 206, 207, 208,	/* 152 - 159 */
    209, 126, 115, 116, 117, 118, 119, 120,	/* 160 - 167 */
    121, 122, 210, 211, 212, 213, 214, 215,	/* 168 - 175 */
    216, 217, 218, 219, 220, 221, 222, 223,	/* 176 - 183 */
    224, 225, 226, 227, 228, 229, 230, 231,	/* 184 - 191 */
    123,  65,  66,  67,  68,  69,  70,  71,	/* 192 - 199 */
     72,  73, 232, 233, 234, 235, 236, 237,	/* 200 - 207 */
    125,  74,  75,  76,  77,  78,  79,  80,	/* 208 - 215 */
     81,  82, 238, 239, 240, 241, 242, 243,	/* 216 - 223 */
     92, 159,  83,  84,  85,  86,  87,  88,	/* 224 - 231 */
     89,  90, 244, 245, 246, 247, 248, 249,	/* 232 - 239 */
     48,  49,  50,  51,  52,  53,  54,  55,	/* 240 - 247 */
     56,  57, 250, 251, 252, 253, 254, 255,	/* 248 - 255 */
  };

/* Crude, non-invertible accent-dropping mapping from Latin1 to Ascii */
static unsigned char const latin1_to_ascii[256] =
{
      0,   1,   2,   3,   4,   5,   6,   7,	/*   0 -   7 */
      8,   9,  10,  11,  12,  13,  14,  15,	/*   8 -  15 */
     16,  17,  18,  19,  20,  21,  22,  23,	/*  16 -  23 */
     24,  25,  26,  27,  28,  29,  30,  31,	/*  24 -  31 */
     32,  33,  34,  35,  36,  37,  38,  39,	/*  32 -  39 */
     40,  41,  42,  43,  44,  45,  46,  47,	/*  40 -  47 */
     48,  49,  50,  51,  52,  53,  54,  55,	/*  48 -  55 */
     56,  57,  58,  59,  60,  61,  62,  63,	/*  56 -  63 */
     64,  65,  66,  67,  68,  69,  70,  71,	/*  64 -  71 */
     72,  73,  74,  75,  76,  77,  78,  79,	/*  72 -  79 */
     80,  81,  82,  83,  84,  85,  86,  87,	/*  80 -  87 */
     88,  89,  90,  91,  92,  93,  94,  95,	/*  88 -  95 */
     96,  97,  98,  99, 100, 101, 102, 103,	/*  96 - 103 */
    104, 105, 106, 107, 108, 109, 110, 111,	/* 104 - 111 */
    112, 113, 114, 115, 116, 117, 118, 119,	/* 112 - 119 */
    120, 121, 122, 123, 124, 125, 126, 127,	/* 120 - 127 */
/*
 * The high 128 characters:
 *        dec  oct hex    ch   ISO_8859-1:1987
 */
   0, /*  128  200  80    PA   padding character (pad) */
 '?', /*  129  201  81    HO   high octet preset (hop) */
 '?', /*  130  202  82    BH   break permitted here (bph) */
 '?', /*  131  203  83    NH   no break here (nbh) */
 '?', /*  132  204  84    IN   index (ind) */
 '?', /*  133  205  85    NL   next line (nel) */
 '?', /*  134  206  86    SA   start of selected area (ssa) */
 '?', /*  135  207  87    ES   end of selected area (esa) */
 '?', /*  136  210  88    HS   character tabulation set (hts) */
 '?', /*  137  211  89    HJ   character tabulation with justification (htj) */
 '?', /*  138  212  8a    VS   line tabulation set (vts) */
 '?', /*  139  213  8b    PD   partial line forward (pld) */
 '?', /*  140  214  8c    PU   partial line backward (plu) */
 '?', /*  141  215  8d    RI   reverse line feed (ri) */
 '?', /*  142  216  8e    S2   single-shift two (ss2) */
 '?', /*  143  217  8f    S3   single-shift three (ss3) */
 '?', /*  144  220  90    DC   device control string (dcs) */
 '?', /*  145  221  91    P1   private use one (pu1) */
 '?', /*  146  222  92    P2   private use two (pu2) */
 '?', /*  147  223  93    TS   set transmit state (sts) */
 '?', /*  148  224  94    CC   cancel character (cch) */
 '?', /*  149  225  95    MW   message waiting (mw) */
 '?', /*  150  226  96    SG   start of guarded area (spa) */
 '?', /*  151  227  97    EG   end of guarded area (epa) */
 '?', /*  152  230  98    SS   start of string (sos) */
 '?', /*  153  231  99    GC   single graphic character introducer (sgci) */
 '?', /*  154  232  9a    SC   single character introducer (sci) */
 '?', /*  155  233  9b    CI   control sequence introducer (csi) */
 '?', /*  156  234  9c    ST   string terminator (st) */
 '?', /*  157  235  9d    OC   operating system command (osc) */
 '?', /*  158  236  9e    PM   privacy message (pm) */
 '?', /*  159  237  9f    AC   application program command (apc) */
 ' ', /*  160  240  a0    NS   no-break space */
 '!', /*  161  241  a1    !I   inverted exclamation mark */
 'c', /*  162  242  a2    Ct   cent sign */
 '#', /*  163  243  a3    Pd   pound sign */
 '$', /*  164  244  a4    Cu   currency sign */
 'Y', /*  165  245  a5    Ye   yen sign */
 '|', /*  166  246  a6    BB   broken bar */
 'S', /*  167  247  a7    SE   section sign */
 '"', /*  168  250  a8    ':   diaeresis */
 'C', /*  169  251  a9    Co   copyright sign */
 'a', /*  170  252  aa    -a   feminine ordinal indicator */
 '<', /*  171  253  ab    <<   left-pointing double angle quotation mark */
 '~', /*  172  254  ac    NO   not sign */
 '-', /*  173  255  ad    --   soft hyphen */
 'R', /*  174  256  ae    Rg   registered sign */
 '_', /*  175  257  af    '-   overline */
 'o', /*  176  260  b0    DG   degree sign */
 '?', /*  177  261  b1    +-   plus-minus sign */
 '2', /*  178  262  b2    2S   superscript two */
 '3', /*  179  263  b3    3S   superscript three */
'\'', /*  180  264  b4    ''   acute accent */
 'u', /*  181  265  b5    My   micro sign */
 'P', /*  182  266  b6    PI   pilcrow sign */
 '.', /*  183  267  b7    .M   middle dot */
 ',', /*  184  270  b8    ',   cedilla */
 '1', /*  185  271  b9    1S   superscript one */
 'o', /*  186  272  ba    -o   masculine ordinal indicator */
 '>', /*  187  273  bb    >>   right-pointing double angle quotation mark */
 '?', /*  188  274  bc    14   vulgar fraction one quarter */
 '?', /*  189  275  bd    12   vulgar fraction one half */
 '?', /*  190  276  be    34   vulgar fraction three quarters */
 '?', /*  191  277  bf    ?I   inverted question mark */
 'A', /*  192  300  c0    A!   latin capital letter a with grave */
 'A', /*  193  301  c1    A'   latin capital letter a with acute */
 'A', /*  194  302  c2    A>   latin capital letter a with circumflex */
 'A', /*  195  303  c3    A?   latin capital letter a with tilde */
 'A', /*  196  304  c4    A:   latin capital letter a with diaeresis */
 'A', /*  197  305  c5    AA   latin capital letter a with ring above */
 'E', /*  198  306  c6    AE   latin capital letter ae */
 'C', /*  199  307  c7    C,   latin capital letter c with cedilla */
 'E', /*  200  310  c8    E!   latin capital letter e with grave */
 'E', /*  201  311  c9    E'   latin capital letter e with acute */
 'E', /*  202  312  ca    E>   latin capital letter e with circumflex */
 'E', /*  203  313  cb    E:   latin capital letter e with diaeresis */
 'I', /*  204  314  cc    I!   latin capital letter i with grave */
 'I', /*  205  315  cd    I'   latin capital letter i with acute */
 'I', /*  206  316  ce    I>   latin capital letter i with circumflex */
 'I', /*  207  317  cf    I:   latin capital letter i with diaeresis */
 'D', /*  208  320  d0    D-   latin capital letter eth (icelandic) */
 'N', /*  209  321  d1    N?   latin capital letter n with tilde */
 'O', /*  210  322  d2    O!   latin capital letter o with grave */
 'O', /*  211  323  d3    O'   latin capital letter o with acute */
 'O', /*  212  324  d4    O>   latin capital letter o with circumflex */
 'O', /*  213  325  d5    O?   latin capital letter o with tilde */
 'O', /*  214  326  d6    O:   latin capital letter o with diaeresis */
 'x', /*  215  327  d7    *X   multiplication sign */
 'O', /*  216  330  d8    O/   latin capital letter o with stroke */
 'U', /*  217  331  d9    U!   latin capital letter u with grave */
 'U', /*  218  332  da    U'   latin capital letter u with acute */
 'U', /*  219  333  db    U>   latin capital letter u with circumflex */
 'U', /*  220  334  dc    U:   latin capital letter u with diaeresis */
 'Y', /*  221  335  dd    Y'   latin capital letter y with acute */
 'T', /*  222  336  de    TH   latin capital letter thorn (icelandic) */
 's', /*  223  337  df    ss   latin small letter sharp s (german) */
 'a', /*  224  340  e0    a!   latin small letter a with grave */
 'a', /*  225  341  e1    a'   latin small letter a with acute */
 'a', /*  226  342  e2    a>   latin small letter a with circumflex */
 'a', /*  227  343  e3    a?   latin small letter a with tilde */
 'a', /*  228  344  e4    a:   latin small letter a with diaeresis */
 'a', /*  229  345  e5    aa   latin small letter a with ring above */
 'e', /*  230  346  e6    ae   latin small letter ae */
 'e', /*  231  347  e7    c,   latin small letter c with cedilla */
 'e', /*  232  350  e8    e!   latin small letter e with grave */
 'e', /*  233  351  e9    e'   latin small letter e with acute */
 'e', /*  234  352  ea    e>   latin small letter e with circumflex */
 'e', /*  235  353  eb    e:   latin small letter e with diaeresis */
 'i', /*  236  354  ec    i!   latin small letter i with grave */
 'i', /*  237  355  ed    i'   latin small letter i with acute */
 'i', /*  238  356  ee    i>   latin small letter i with circumflex */
 'i', /*  239  357  ef    i:   latin small letter i with diaeresis */
 'd', /*  240  360  f0    d-   latin small letter eth (icelandic) */
 'n', /*  241  361  f1    n?   latin small letter n with tilde */
 'o', /*  242  362  f2    o!   latin small letter o with grave */
 'o', /*  243  363  f3    o'   latin small letter o with acute */
 'o', /*  244  364  f4    o>   latin small letter o with circumflex */
 'o', /*  245  365  f5    o?   latin small letter o with tilde */
 'o', /*  246  366  f6    o:   latin small letter o with diaeresis */
 '/', /*  247  367  f7    -:   division sign */
 'o', /*  248  370  f8    o/   latin small letter o with stroke */
 'u', /*  249  371  f9    u!   latin small letter u with grave */
 'u', /*  250  372  fa    u'   latin small letter u with acute */
 'u', /*  251  373  fb    u>   latin small letter u with circumflex */
 'u', /*  252  374  fc    u:   latin small letter u with diaeresis */
 't', /*  253  375  fd    y'   latin small letter y with acute */
 't', /*  254  376  fe    th   latin small letter thorn (icelandic) */
 'y'  /*  255  377  ff    y:   latin small letter y with diaeresis */
};

/* 
 * Characters not recognized (mapped to '?'):
 * - Control characters 129-159
 * - Plus-minus (Mapping to either + or - could be *very* misleading!)
 * - Fractions 1/4, 1/2, 3/4
 *
 * Differences from PGP 2.x's encoding:
 * - 128 (pad) is mapped to 0 (nul), the usual pad character
 * - << and >> are mapped to '<' and '>' instead of '"' and '"'
 * - Pilcrow (paragraph symbol) is mapped to 'P' rather than '-'
 * - AE and ae are mapped to 'E' and 'e' rather than 'A' and 'a', based
 *   on english conventions like encyclopaedia -> encyclopedia
 */
/* *INDENT-ON* */
/**********************************************************************/

/* Charset tables */
static struct ConfMapping {
	char const *name;
	unsigned char const *toLatin1, *toLocal;
} confMappings[] = {
	{ "noconv",	charMapIdentity,	charMapIdentity },
	{ "ascii",	charMapIdentity,	latin1_to_ascii },
	{ "cp850",	cp850_to_latin1,	latin1_to_cp850 },
	{ "ebcdic",	ebcdic_to_latin1,	latin1_to_ebcdic },
	{ (char const *)0, 0, 0 }
};

/* Case-insensitive memory compare */
static int
xmemicmp (char const *in1, char const *in2, int len)
{
	while (len--) {
		if (tolower(*in1) != tolower(*in2))
			return 1;
		in1++;
		in2++;
	}
	return 0;
}

PGPError
pgpCharmaps (char const *charset, int setlen,
	     PGPByte const **toLocalP, PGPByte const **toLatin1P)
{
	int i;

	if (!charset || !toLocalP || !toLatin1P)
		return kPGPError_BadParams;

	for (i=0; confMappings[i].name; i++) {
		if ((size_t)setlen != strlen (confMappings[i].name) ||
		    xmemicmp (charset, confMappings[i].name, setlen) != 0)
			continue;

		*toLocalP = confMappings[i].toLocal;
		*toLatin1P = confMappings[i].toLatin1;
		return kPGPError_NoErr;
	}
	return kPGPError_UnknownCharMap;
}

void
pgpCharmapConvert (PGPByte const *input, size_t inlen, PGPByte *output,
		   PGPByte const *charmap)
{
	while (inlen--)
		output[inlen] = charmap[input[inlen]];
}
