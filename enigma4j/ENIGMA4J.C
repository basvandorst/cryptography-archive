
/***************************************************************************/

/* ENIGMA4.C  A simulator for the WW II cryptographic engine ENIGMA        */
/*            (3 Rotor Version)                                            */

/* USAGE:     enigma [-cdt]                                                 */

/*            -c simulate a green cover over the lamps                     */
/*            -d show aditional information (what knows ENIGMA about the   */
/*               system it is running on, tracing information during       */
/*               encoding)                                                 */
/*            -t show the last encoded characters                          */ 

/* KEYS:      Some keys can be used alternate to the standart ones         */

/*            UpArrow:     Cntrl-P                                         */
/*            DownArrow:   Cntrl-N                                         */
/*            LeftArrow:   Cntrl-B                                         */
/*            RightArrow:  Cntrl-F                                         */

/*            F1:          1                                               */

/* SWITCHES: some #define's change the behaviour of ENIGMA                 */

/*            ATARI_ST          ENIGMA assumes it is running on an         */
/*                              Atari ST, VT52 is used                     */
/*            MSDOS, __MSDOS__  ENIGMA assumes it is running on a PC       */
/*            __GNUC__          test() says "compiled with Gnu C"          */
/*            MSDOSBW           On PC: black & white, uses ANSI.SYS        */
/*                              else:  inverse to default vt100            */
/*            VT320             *** Never use this one on a PC! ****       */
/*                              It uses some special ESC Sequences         */
/*                              unknown to ANSI.SYS. It might help if      */
/*                              cls() doesn't work on vt100 terminals.     */
/*                              I'm not sure if this Sequences are vt100.  */
/*                              I found them in a manual for a vt320       */
/*                              terminal                                   */
/*            VT52              use vt52 Graphics                          */
/*            CURSES            should be defined when getch() has to be   */
/*                              imported from the curses library           */
/*                              does minimal initialisation, but no        */ 
/*                              keypad(), no use of curses for output      */
/*            SWEARWORDS        use curses for output, too                 */
/*                              (includes CURSES)                          */
/*                              uses keypad() for input, this means F1 and */
/*                              cursor keys will be correctly recognized   */
/*                              without the need to know the scan codes    */
/*            BSDCURSES         don't use System V specific curses routines*/
/*                              (includes SWEARWORDS, but puts             */
/*                              restrictions on it)                        */
/*                              no keypad()                                */
/*            UNIX              test() says "compiled for UNIX"            */
/*            SUN               for SUN Workstations,                      */
/*                              uses CntrlL ('\014') for cls()             */
/*            NEEDSCR           defines puts(s) as printf("%s\r\n",s)      */

/*            DEBUG             You get a lot of strange messages.Not for  */
/*                              normal use                                 */

/*            These are set automatically:                                 */

/*            COLOUR            indicates that the system can display      */
/*                              colours. At the moment only with MSDOS     */
/*            VT                use vt100                                  */

/*            ANSI_COL          use ANSI.SYS for colours                   */

/* (c) 1994   by Andreas G. Lessig,Hamburg                                 */

/*            I don't care if you distribute this program, but carefully   */
/*            mark all changes in the source. Make certain that everybody  */
/*            knows (s)he is using a version you altered.                  */

/*            If you find a way to make money out of this program that's   */
/*            o.k. with me, but I want my share. So please contact me      */
/*            before using it comercially. I'm sure we'll come to an       */
/*            agreement. My Email-adress is                                */
/*                  1lessig@rzdspc1.informatik.uni-hamburg.de              */

/*            This program comes "as is". There is no implied warranty.    */
/*            You try it at own risk. If it doesn't work or trashes your   */
/*            system I won't pay. ( This doesn't mean I wouldn't answer    */
/*            questions, but I definitly won't pay ...)                    */

/*            Suggestions are welcome.                                     */


/*            Ich habe nichts gegen eine Weitergabe des Programms, aber    */
/*            jegliche kommerzielle Verwendung ist vorher mit mir          */
/*            abzusprechen.                                                */
  
/*            Ver"anderte Programmversionen sind zu kennzeichnen.          */

/*            F"ur Anregungen bin ich dankbar                              */

/* ENIGMA1.C                                                               */
/* 19.12.1994 Mock up, d.h. blo"s ein graphisches Interface, aber es wird  */
/*            nicht chiffriert                                             */
/*            gedr"uckte Tasten werden invertiert, zugeh"orige L"ampchen   */
/*               leuchten                                                  */
/* 21.12.1994 Verdrahtung von Rotoren und Reflektoren #define'd            */
/*            Variablen f"ur die Einstellungen                             */
/*            Rotoren,Reflektor,Steckerbrett verschl"usseln                */
/*            Rotoren drehen noch nicht, k"onnen aber mit den Pfeiltasten  */
/*               manuell verstellt werden                                  */
/*            Grundeinstellung kann noch nicht editiert werden             */
/* 27.12.1994 Rotoren drehen jetzt, es wird zwischen der Stellung des      */
/*            Ringes und der der Verdrahtung unterschieden                 */
/* 28.12.1994 Rotoren und der Reflektor k"onnen ausgew"ahlt werden         */
/*            Das Steckerbrett kann editiert werden.                       */
/*            Die Ringstellung kann noch nicht ver"andert werden           */
/* 30.12.1994 Codes f"ur die benutzten Tasten #define'd                    */
/*            Tastatur entsprechend dem Original berichtigt                */
/*            Codes f"ur Sonderzeichen als #define                         */
/*            Allgemeines Facelifting                                      */

/* ENIGMA2.C                                                               */
/*   4.1.1995 gr_init(): Dummyfunktion, bei Portierung u.U. n"otig,um      */
/*                       die Graphik zu initialisieren     */
/*            Variable f"ur den Rotortyp                                   */
/*            Eingabe der Rotoren "uberarbeitet -> askrotor()              */
/*            Variable f"ur den Reflektortyp                               */
/*            Ringstellung implementiert                                   */
/*            Eingabe des Steckerbrettes gegen Fehlbedienung abgesichert   */
/*            Im Setup Ausgabe der gegenw"artigen Einstellungen            */

/* ENIGMA3.C                                                               */
/*   4.1.1995 Erster Versuch Enigma portabel zu machen                     */
/*            neue define's:                                               */
/*               VT       vt100 benutzen                                   */
/*               COLOUR   Farbdarstellung                                  */
/*               MSDOSBW  vt100 unter DOS                                  */
/*               DOSGCC   Farbdarstellung unter DOS mittels gcc Library    */
/*               ANSI     dito, aber mittels ANSI.SYS                      */
/*            neue Farbdefinitionen f"ur s/w                               */
/*            Sonderzeichen f"ur portable Version neu                      */
/*            F"ur ANSI und vt100:                                         */
/*                                                                         */
/*               ScrenPutChar(),ScrenSetCursor()                           */
/*            Tasten: 1 alternativ f"ur F1                                 */
/*                    CntrlP f. UpArrw                                     */
/*                    CntrlN f. DwnArrw                                    */
/*                    CntrlF f. RghtArrw                                   */
/*                    CntrlB f. LftArrw                                    */

/*            Noch ein Bug beim Einstellen der Ringstellung:               */
/*            wenn window[n]!='a' gibt es M"ull                            */
/*   5.1.1995 Stellen zum Einsatz sp"aterer Routinen f"ur andere Systeme   */
/*            markiert                                                     */
/*            obigen Bug hoffentlich behoben                               */
/*            ScreenPutChar gibt Zeichen > 127 nur aus, wenn MSDOS         */
/*            definiert ist                                                */

/* ENIGMA4.C (international version)                                       */
/*   5.1.1995 English description/legal notice in header                   */
/*   6.1.1995 added initscr() followed by refresh()  if the CURSES switch  */
/*               is set. Needed it under UNIX. These, and getch() are the  */
/*               only functions I use from curses. The rest is vt100.      */
/*            added switch VT320 for use of not ANSI- compatible ESC       */
/*               sequences                                                 */
/*            added switch NOGETCH, getch() will be defined as getchar()   */
/*            aditional output in test()                                   */
/*            NOGETCH didn't do what it was supposed to. So I removed it.  */
/*            fixed a bug in s_rotors(). The rotor was changed, but this   */
/*               wasn't shown to the user.                                 */
/*            fixed the cls() routine for vt100. Should do it now.         */
/*            added option -c, which emulates a green cover over the lamps */
/*            inserted the USAGE and SWITCHES sections above for easier    */
/*               porting                                                   */
/*   7.1.1995 title()                                                      */
/*   8.1.1995 confirm()                                                    */
/*               - returns 0 on <Enter>                                    */
/*               - returns 1 on <Esc>                                      */
/*            test() tells                                                 */
/*               - that 80*25 text mode is assumed                         */
/*               - if curses is used                                       */
/*   9.1.1995 new switches:                                                */
/*               SUN     uses ^L for cls()                                 */
/*               NEEDSCR all non positioned output has an additional <CR>  */
/*               feeble attempts to hide the cursor.                       */
/*            mputs():                                                     */
/*               new function replacing puts(), implements NEEDSCR         */
/*            putxy():                                                     */
/*               debugging of vt100,MSDOSBW modes                          */
/*               cursor is put to 0,0 after operation                      */
/*            cls():                                                       */ 
/*               debugging of vt100,MSDOSBW modes                          */
/*            gr_exit():                                                   */
/*               cursor is set to 0,0 for leaving in VT320 mode            */
/*            title():                                                     */
/*                uses mputs()                                             */
/*            tests on the SPARCS with                                     */
/*                 xterm: works, cursor visible in upper left corner       */
/*                 cmdtool, commandline:                                   */
/*                       background white,                                 */
/*                       cursor visible  in upper left corner              */
/*  10.1.1995 tests on vt320 terminals:                                    */
/*                  slow output, especially writexy()                      */
/*                  cursor hidden                                          */
/*            test():                                                      */
/*                  uses mput()                                            */
/*            setup():                                                     */
/*                  removed surplus cls()                                  */
/*            gr_exit():                                                   */
/*                  restores now cursor                                    */
/*            cls():                                                       */
/*                  MSDOSBW works reverse when not on a PC                 */
/*                  to create the efffect metioned in the specification    */
/*                  that means on not PC's it's reverse to VT              */
/*  14.1.1994 replaced the define ANSI by ANSI_COL for compatibility with  */
/*               the pdcurses package                                      */
/*            DOSGCC isn't used any more                                   */
/*            replaced title() by intro() (trouble with pdcurses)          */
/*            s_plugboard():                                               */
/*               doesn't call cls() on every round (faster)                */
/*            ScreenSetCursor():                                           */
/*               removed redundant code                                    */
/*            gr_exit():                                                   */
/*               removed redundant code                                    */
/*            general facelifting                                          */

/*  16.1.1995 new switch SWEARWORDS, for using curses for output, too      */

/*  17.1.1995 debugging SWEARWORDS:                                        */

/*               gr_exit(): cls(MENU); curs_set(1);                        */

/*               No unpositioned output after gr_init()                    */
/*            testing SWEARWORDS:                                          */
/*               cmdtool,xterm, vt220 terminal:                            */
/*                  background colour isn't set                            */
/*                  cursor isn't hidden (now putting it at 80,25)          */
/*            gr_init(): added keypad(),                                   */ 
/*                  Cursorkeys and  F1 are interpreted by curses           */
/*  21.1.1995 made the CURSES switch compatible to BSD-curses              */
/*               this means I made keypad() exclusive to SWEARWORDS        */ 
/*  22.1.1995 removed surplus coding from cls()                            */
/*            New switch BSDCURSES:                                        */ 
/*               no use of keypad(),                                       */
/*               some commands are different                               */
/*               test :                                                    */
/*                  my version of bsd curses uses bit 7 (128) to show      */
/*                  standout mode. So strange characters appear on the     */
/*                  screen. (It's definitely a bug of the library)         */
/*            new function inkey():                                        */
/*               provides an interface to getch(), under MSDOS now the     */
/*               cursor keys should be recognized better                   */
/*            new switches:                                                */
/*               ATARI_ST       defines VT52 and special keys for Atari    */
/*               VT52           use vt52 graphics                          */
/*  26.1.1995 inkey() now forces characters to lower case                  */ 
/*  14.1.1995 just a few final changes. Now it's ready. (hopefully ...)    */
/*  27.4.1995 new option: -t (teletype mode) the last encrypted characters */
/*               are shown.Insufficient memory will cause the option to be */
/*               neglected. Output wil be presented in groups of five.     */
/*  15.5.1995 debugging the graphic interface                              */ 
/***************************************************************************/

#include <malloc.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>


#ifdef __MSDOS__
#ifndef MSDOS
#define MSDOS
#endif
#endif

#ifdef ATARI_ST
#define VT52
#endif
                                        /* for testing */
                                        /* should be commented out */
/*#define CURSES*/                          /* getch() is imported */
                                            /* from curses*/

/*#define DEBUG*/                           /* Display Information for */
                                            /* debugging */
                                                                        
/*#define MSDOSBW*/                         /* use vt100 b/w on the PC */
                                            /*inverse video on other systems*/

/*#define VT320*/                           /* use non ANSI compatible  */
                                            /* ESC sequences */

/*#undef __GNUC__*/                         /* Don't use GNU Library */
                                            /* under DOS */
                                            
/*#undef MSDOS*/                            /* plain vt100 7bit ASCII */


#ifdef BSDCURSES                            /* use curses for output, but */
                                            /* not the System V way       */

#define SWEARWORDS

#endif


#ifdef SWEARWORDS                       /* use curses for output ? */

#define CURSES

#else                                   /* curses prohibited for output */
#ifdef MSDOS                                /* MS DOS Computer */   
#ifdef MSDOSBW                                  /* but black & white */ 
    
#define VT 100                                      /* -> vt100 */

#else                                           /* colour */

#define COLOUR 1
#define ANSI_COL 1                                  /* -> use ansi.sys */


#endif
#else                                       /* No PC */
#ifndef VT52                               /* Neither PC nor Atari */

        /* ---- put in here : ----- */
        /* #ifdef <MySystem>        */
        
        /* #define COLOUR  (if it supports colours) */

        /* #else */

#define VT 100                              /* -> vt100 */

        /* ---- put in here : ----- */
        
        /* #endif */
#endif
#endif
#endif


#ifdef CURSES
#include <curses.h>
#endif



/*------------------------ defines for portability ------------------------ */




                 /* Farben */

#ifdef COLOUR           /* colour mode */                

                /* ---- put in here : ---- */
                /* if you don't like my definitions :*/
                
                /* #ifdef <MySystem> */
                
                /* #define ... */
                
                /* #else */             



#define KEY_NOR 112                     /* key normal: black on light grey  */
#define KEY_PRES 7                      /* key pressed: light grey on black */
#define BG 7                            /* background for the Enigma: brown */
#define BULB_NOR 8                      /* window normal: dark grey         */
                                        /*    on black                      */
#define BULB_LIT 15                     /* window lit: bright white on black*/
#define BULB_DIM  2                     /* Bulb shining through green       */
                                        /*    celluloid                     */
#define ROTOR 112                       /* Rotor: black on light grey       */
#define MENU 7                          /* normal writings in the menu      */
#define MENU_EM 15                      /* emphazised in menu               */
#define MENU_AL 4                       /* for errors in menu               */
#define MENU_CUR 112                    /* the character beneath the cursor */



                /* ---- put in here : ---- */
                /* if you don't like my definitions : */
    
                /* #endif */




#else                   /* black & white */

#define KEY_NOR 0                       /* black on white                   */
#define KEY_PRES 1                      /* white on black                   */
#define BG 1                            /* Hintergrundfarbe sw              */
#define BULB_NOR 1                      /* Leuchtfenster normal: wei"s      */
                                        /*    auf schwarz                   */
#define BULB_LIT 0                      /* Fenster leuchtet: wei"s auf      */
                                        /*    schwarz                       */
#define BULB_DIM 1                      /* look above */
#define ROTOR 0                         /* Rotor: Schwarz auf hellgrau      */
#define MENU 1                          /* Normale Schrift im Men"u         */
#define MENU_EM 0                       /* hervorgehobenes im Men"u         */
#define MENU_AL 0                       /* Fehler im Men"u */
#define MENU_CUR 0                      /* Das Zeichen unter dem Cursor     */

#endif


                /* special keys */

#define ESC         27
#define CntrlC      3
#define CntrlB      2
#define CntrlF      6
#define CntrlN      14
#define CntrlP      16

#ifdef SWEARWORDS
#ifndef BSDCURSES
                                        /* System V curses */
#define F1          KEY_F(1)
#define UpArrw      KEY_UP
#define DwnArrw     KEY_DOWN
#define RghtArrw    KEY_RIGHT
#define LftArrw     KEY_LEFT

#else
                                        /* BSD curses */
#define F1          315
#define UpArrw      328
#define DwnArrw     336
#define RghtArrw    333
#define LftArrw     331

#endif
#else
#ifdef ATARI_ST                         /* Atari ST */

#define F1          187
#define UpArrw      200
#define DwnArrw     208
#define RghtArrw    205
#define LftArrw     203


#else                                        /* default */
#define F1          315
#define UpArrw      328
#define DwnArrw     336
#define RghtArrw    333
#define LftArrw     331

#endif
#endif
                                        /* special characters */
                        /* MS-DOS */
#ifdef MSDOS

#define ULC 201                             /* upper left corner */
#define URC 187                             /* upper right corner */
#define LLC 200                             /* lower left corner */
#define LRC 188                             /* lower right corner */
#define VER 186                             /* vertical bar */
#define HOR 205                             /* horizontal bar */
#define TRI 30                              /* marker of current rotor */

#else
                        /* 7 Bit ASCII */

#define ULC ' '                             /* upper left corner  */
#define URC ' '                             /* upper right corner */
#define LLC ' '                             /* lower left corner  */
#define LRC ' '                             /* lower right corner */
#define VER ' '                             /* vertical bar       */
#define HOR ' '                             /* horizontal bar     */
#define TRI '^'                             /* marker of current rotor */

 
#endif

/* ------------------------ other defines -------------------------------- */


                                        /* System constants */
#define ROWS        25
#define COLUMNS     80
#define KEY_XOFFS   4                       /* x-offset of the keys */
#define TRACEXOFS   6                       /* x-offset of the tracing */
                                            /* information (shows the */
                                            /* steps of encryption) */

                                        /* wirings of the rotors */
#define ROT_t "abcdefghijklmnopqrstuvwxyz"      /* rotor for testing */
#define ROT_i "ekmflgdqvzntowyhxuspaibrcj"      /* rotor 1 */
#define ROT_ii "ajdksiruxblhwtmcqgznpyfvoe"     /* rotor 2 */
#define ROT_iii "bdfhjlcprtxvznyeiwgakmusqo"    /* rotor 3 */
#define ROT_iv "esovpzjayquirhxlnftgkdcmwb"     /* rotor 4 */
#define ROT_v "vzbrgityupsdnhlxawmjqofeck"


                                        /* in which position will this rotor*/
                                        /* will this rotor be turned */
                                        /* together with its left neighbour*/
#define NOTCH_t 'z'
#define NOTCH_i 'q'
#define NOTCH_ii 'e'
#define NOTCH_iii 'v'
#define NOTCH_iv 'j'
#define NOTCH_v 'z'

                                        /* reflectors */
#define REFL_t "abcdefghijklmnopqrstuvwxyz"     /* reflector f. test. */
#define REFL_b "yruhqsldpxngokmiebfzcwvjat"     /* reflector B, thick */
#define REFL_c "fvpjiaoyedrzxwgctkuqsbnmhl"     /* reflector C, thick */                                        

/* ------------------------- global constants ---------------------------- */

int rotor[4];                           /* actual rotor position (elektr.) */
int window[4];                          /* ----------"-----------(visuell) */
int rot_ini[4];                         /* first   -----"------- */
char *rot_wir[4];                       /* wiring of the rotors */
int rot_notch[4];                       /* switching positions */
int rot_typ[4];                         /* which rotor (t,1-5) */

char *refl;                             /* wiring of the reflector  */
int refl_typ;                           /* which reflektor ?*/

char *stecker;                          /* wirings of the plugboard */

int stepwidth;                          /*  0:test 1:normal */

char *ttystrip;                         /* a string containing the encoded */
                                        /* message */

int fivecount;                          /* used for grouping tty output*/

                                        /* options */

int debug;                                  /* shall the steps of encryption*/
                                            /* be displayed? */ 
                        
int cell_cover;                             /* green celluloid cover */
                                            /* over the bulbs */

int teletype;                               /* shall the last encrypted */
                                            /* characters be shown ? */
/* ------------------- Deklarations --------------------------------------- */

#ifdef __STDC__                         /* ANSI declarations */
 
int modulo(int n, int mod);

#else                                   /* K&R declarations */

int modulo();

#endif


/* ---------------------------- Graphik Interface --------------------- */

/*............................. Basic Functions ....................... */

int inkey()                            /* return a single integer */
                                        /* if a key is pressed */
{
    int ret;

    ret=getch();

#ifdef MSDOS                                /* On a MSDOS System getch() */
                                            /* might return twice on special*/
                                            /* keys. The first value is */
                                            /* zero, the second one is the */
                                            /* scancode.Some implementations*/
                                            /* of getch() return the */
                                            /* scancode plus 256 on special */
                                            /* characters. This behavior is */
                                            /* enforced by the following. */
    if(!ret)
        ret=256+getch();
#endif




    if((ret <= 255) && isupper(ret))
        ret=tolower(ret);

    return(ret);
}


mputs(s)                                /* as a replacement for puts() */
char *s;
{
#ifdef NEEDSCR
    printf("%s\r\n",s);
#else
    puts(s);
#endif
}

char *translate(x)                      /* 2 digit number to string */
                                        /* returns newly allocated string */ 
int x;                                  
{
    char s[3];
    int m,n;

    s[2]='\0';
    m=0;
    n=x;
    while(n>=10){
        n-=10;
        m++;
    }
    s[0]='0'+m;
    m=0;
    while(n>0){
        n--;
        m++;
    }
    s[1]='0'+m;
    if(s[0]=='0'){
        s[0]=s[1];
        s[1]=s[2];
    }
    
    return(strdup(s));
}


void gr_init()                          /* initialise graphics */
{
#ifdef CURSES                               /* CURSES library */
    initscr();                                  /* initialisation */
    cbreak();                                   /* imidiate input */
    noecho();                                   /* don't echo input */
#endif

#ifdef SWEARWORDS
#ifndef BSDCURSES                               /* Not supported by */
                                                /* BSD curses : */
    keypad(stdscr,TRUE);                            /* let curses interpret */
                                                    /* special keys */
    curs_set(0);                                    /* cusor invisible */
    refresh();
#endif
#endif

#ifdef VT52                                 /* vt52 */
    printf("\033E");                            /* cls */
    printf("\033f");
#endif

#ifdef VT320                                /* incompatible to ANSI.SYS */
    printf("\033[?25l");                        /* invisible Cursor */
#endif
}



void ScreenSetCursor(x,y)               /* move Cursor to position (x,y) */
                                        /* --- DUMMY --- */
int x,y;
{
}

void ScreenPutChar(ch,attr,x,y)         /* put a character on a certain */
                                        /* position in a certain colour */
int ch;                                     /* character */
int attr;                                   /* colour */
int x,y;                                    /* position */
{
    char *pos1,*pos2;
    
#ifndef MSDOS                               /* No MS-DOS? */
    if(ch>127)                                  /* -> Only 7Bit ASCII  */
        return;
#endif


#ifdef COLOUR                               /* Colour */
#ifdef ANSI_COL                                 /* ANSI.SYS */
    
                                                    /* move cursor */
    pos1=translate(x+1);
    pos2=translate(y+1);                                                
    printf("\033[%s;%sH",pos2,pos1);
    free(pos1);
    free(pos2);
                                                    /* set colour */
    printf("\033[0;8;");
                                                    
    switch(attr){
        case 2:
            printf("32;");
            printf("40");
            break;
        case 4:
            printf("31;");
            printf("40");
            break;
        case 7:
            printf("40;");
            printf("37");
            break;
        case 8:
            printf("1;30;40");
            break;
        case 14:
            printf("40;");
            printf("33;1");
            break;
        case 15:
            printf("37;1;");
            printf("40");
            break;
        case 96:
            printf("30;");
            printf("41");
            break;
        case 112:
            printf("30;");
            printf("47");
            break;          
    }   
        
    putchar('m');   
                                                /*  character */
    putchar(ch);
                                                /* move away cursor */
    printf("\033[25;80H");

    fflush(stdout);                                         
                                                            
#else                                       /* other colour routines */


       /*---- put your colour routine here ---*/


#endif
#else                                   /* black & white */

#ifdef VT                                   /* vt100 */
                                                /* move cursor */
    pos1=translate(x+1);
    pos2=translate(y+1);
    printf("\033[%s;%sH",pos2,pos1);
    free(pos1);
    free(pos2);
                                                /* invert if necessary */
    printf("\033[0;");  
    if(attr){
    }else{
        printf("7");
    }
    putchar('m');
    
    putchar(ch);                                /* character */

                                                /* move away cursor  */
    printf("\033[0;79H");

    fflush(stdout);
#else
#ifdef SWEARWORDS                           /* curses */
#ifdef BSDCURSES                                /* do it BSD */

    if(!attr)                                       /* revert if necessary */
        standout();
    else
        standend();

#else                                           /* do it System V */

    if(!attr)                                       /* revert if necessary */
        attrset(A_REVERSE);
    else
        attrset(A_NORMAL);

#endif

    mvaddch(y,x,ch);                            /* type character */
    move(ROWS - 1,COLUMNS - 1 );                /* move away cursor */


    refresh();                                  /* make it visible */

#else
#ifdef VT52                                 /* vt52 */
                                                /* move cursor */
    pos1=translate(x+1);
    pos2=translate(y+1);
    printf("\033Y%c%c",y+32,x+32);
    free(pos1);
    free(pos2);

                                               /* revert if necessary */
    if(attr){
        printf("\033b0");
        printf("\033c1");
    }else{
        printf("\033b1");
        printf("\033c0");
    }



    putchar(ch);                               /* type char */
/*    printf("\033Y%c%c",ROWS+31,COLUMNS+31); */   /* move away cursor */
    fflush(stdout);

#else


        /*--- put your own b/w routine here ---*/   


#endif
#endif
#endif
#endif
}





void cls(color)                         /* paint screen in colour color */
int color;
{

    int attr,n;
    
    attr=color;
#ifdef COLOUR                               /* colour */
#ifdef ANSI_COL                                 /* with ANSI.SYS */         
    printf("\033[0;");
 
    switch(attr){                                   /* not all possible */
                                                    /* colours implemented */
                                                    /* just those needed by */
                                                    /* this program */
        case 2:
            printf("32;");
            printf("40");
            break;
        case 4:
            printf("31;");
            printf("40");
            break;
        case 7:
            printf("40;");
            printf("37");
            break;
        case 15:
            printf("1;");
            printf("40");
            break;
        case 96:
            printf("30;");
            printf("41");
            break;
        case 112:
            printf("30;");
            printf("47");
            break;          
    }           
    putchar('m');   
    printf("\033[2J");
    fflush(stdout);
#else

        /* ------- put your own colour routine here -------- */


#endif  
#else                                       /* black and white */

#ifdef SUN                                      /* cls on SUN */
    putchar('\014');
#endif

#ifdef MSDOSBW                                  /* inverted vt100 */
#ifndef VT320                                       /* ANSI.SYS */  
    printf("\033[0;");                                  /* Attributes */
#ifdef MSDOS
    if(attr){                                           /*normal under MSDOS*/
#else
    if(!attr){                                          /*reverse under UNIX*/
#endif
    }else{
        printf("7");
    }
    putchar('m');
    printf("\033[2J");
    fflush(stdout);
#else                                               
                                                    /* incompatible */
                                                    /* to ANSI.SYS*/

    printf("\033[?5");                                  /* background colour*/
#ifdef MSDOS
    if(attr)
#else
    if(!attr)                                           /* works reverse */ 
                                                        /* under UNIX */
#endif
        printf("h");
    else
        printf("l");
    printf("\033[2J");                                  /* cls */

    fflush(stdout);     
#endif  

#else 
#ifdef VT                                       /* not inverted vt100 */
                                                    /* ANSI.SYS */
#ifndef VT320
                                            

    printf("\033[0;");                                  /* Attributes */
    if(!attr){
    }else{
        printf("7");
    }
    putchar('m');
    printf("\033[2J");                                  /* cls */
    fflush(stdout);
#else                                               /* incompatible Version*/

    printf("\033[?5");                                  /* background colour*/
    if(!attr)
        printf("l");
    else
        printf("h");
    printf("\033[2J");                                  /* cls */

    fflush(stdout); 
#endif  

#else
#ifdef SWEARWORDS                           /* curses */
    

    erase();
    refresh();
#else
#ifdef VT52                                 /* vt52 */

    if(attr){
       printf("\033b0\033c1");
    }else{
       printf("\033b1\033c0");
     }
    printf("\033E");
    fflush(stdout);
#else

        /*----------- put your own b/w routine here ------- */

#endif
#endif
#endif
#endif
#endif  
 
 
}

void putxy(c,mode,x,y)                  /* just a frame */
int c,mode,x,y;
{
    ScreenSetCursor(x,y);
    ScreenPutChar(c,mode,x,y);
    return;
}

void writexy(s,mode,x,y)                /* writes String s at x,y */
                                        /* in colour mode */
char *s;
int mode,x,y;
{
    while(s[0]){
        putxy(s[0],mode,x,y);
        s++;
        x++;
    }
    
    return;
      
}

void gr_exit()                          /* clear up screen */
{



#ifdef ANSI_COL                 /* ANSI */
    cls(KEY_PRES);
#else
#ifdef MSDOSBW                  /* MSDOSBW */
    cls(MENU);

#else
#ifdef VT                       /* VT */

    cls(MENU_CUR);

#else
#ifdef SWEARWORDS               /* Curses which are used for output */

    cls(MENU_CUR);

#ifndef BSDCURSES                   /* System V */

    curs_set(1);                        /* make cursor visible */

#endif
#else

            /* ----- put your own routine here ----------- */

#endif
#endif
#endif
#endif


/*  putxy('\n',1,0,0);*/
/*  fflush(stdout);*/

#ifdef CURSES                   /* Curses (independ.) */
    endwin();
    refresh();
#endif

#ifdef VT320                    /* incompatible to ANSI */

    printf("\033[?25h");            /* restore cursor */
    fflush(stdout);
#endif

#ifdef VT52
    printf("\033e\033q\033b1\033c0\033E");
    fflush(stdout);
#endif

    return;
}

/* ............................ ENIGMA specific ................... */

int confirm()                           /* returns 0 if <enter>*/
                                            /* was pressed */
                                            /* 1 on <ESC>*/ 
{
    int c;

    while(1){
        c=inkey();
        switch(c){
            case '\r':
            case '\n':
                return(0);
            case ESC:
                return(1);
        }
    }
}

void intro()                            /* display a title screen */
{
    cls(MENU);
writexy("...........................................................",MENU,0,0);
writexy("....... ___/..  /... /.. /.... ___/.... /...... /....  /...",MENU,0,1);
writexy("...... /..... _/... /.. /.. _/...._/..  /..... /.. __/ _ /.",MENU,0,2);
writexy("..... /..... /. /. /.. /.. /......... __/.__  /.. /.... /..",MENU,0,3);
writexy(".... __/... /._/. /.. /.. /...__/... /.. /.. /.. /.... /...",MENU,0,4);
writexy("... /..... /.._/ /.. /.._/..... /.. /.._/.. /.. _____ /....",MENU,0,5);
writexy(".. /..... /.... /.. /..._/...._/.. /...... /.. /.... /.....",MENU,0,6);
writexy(".____/.._/...._/.._/.....____/..._/......_/.._/...._/......",MENU,0,7);
writexy("...........................................................",MENU,0,8);

writexy("ENIGMA, a Simulator for the three rotor Enigma used in WW II",MENU,0,9);


writexy("(c) 1995 by Andreas G. Lessig, Hamburg ",MENU,0,12);


writexy("Special thanks to Vesselin Bontchev, ",MENU,0,15);
writexy("who dug out all the information.",MENU,0,16);



writexy("<Return>",MENU,0,20);

    confirm();
    cls(MENU);
    
writexy("Your system might use scancodes different of those on the PC.",
    MENU,0,0);
writexy("For this reason you may type:",MENU,0,1);

writexy("   1          instead of <F1>",MENU,0,3);
writexy("   <ContrlP>  ----\"----- <UpArrow>",MENU,0,4);
writexy("   <ContrlN>  ----\"----- <DownArrow>",MENU,0,5);
writexy("   <ContrlB>  ----\"----- <LeftArrow>",MENU,0,6);
writexy("   <ContrlF>  ----\"----- <RightArrow>",MENU,0,7);
    
writexy("There are the following options to this program:",MENU,0,9);

writexy("   -c simulates a green celloloid cover over the lamps",MENU,0,11);
writexy("   -d shows the assumptions the program makes about your System",
    MENU, 0,12);
writexy("      the steps of encryption are shown",MENU,0,13);   
writexy("   -t the last 65 encrypted characters are shown in groups of five",
    MENU, 0,14);

writexy("<Return>",MENU,0,20);

    confirm();
    cls(MENU);

writexy("PLEASE NOTE:",MENU_AL,0,0);

writexy("1. You use this program at your own risk.",MENU,0,2);
writexy("   I won't pay for damage caused by its use.",MENU,0,3);
    
writexy("2. You might use and distribute it for privat purposes.",MENU,0,5);
    writexy("   Commercial use has to be licensed by me.",MENU,0,6);
    writexy("   (If you make money with it I want a share)",MENU,0,7);

writexy("3. If you change the source, mark it clearly in the",MENU,0,9); 
writexy("   source code and make sure that every user knows",MENU,0,10);
writexy("   (s)he is using an altered version. ",MENU,0,11);
writexy("   (e.g. by a addition to the titelpage)",MENU,0,12);
writexy("4. If you ported this program to another system or",MENU,0,14);
writexy("   did some improvements, please let me know.",MENU,0,15);

writexy("Andreas G. Lessig (1lessig@rzdspc1.informatik.uni-hamburg.de)",MENU,0,20);
    writexy("<Return>",MENU,0,24);

    confirm();
    cls(MENU);
    return;
}


int keyx(c)                             /* returns x coordinate of a */
                                        /* given key */
int c;
{
    switch(c){
        case 'a':
            return(6+KEY_XOFFS);
        case 'b':   
            return(42+KEY_XOFFS);
        case 'c':
            return(26+KEY_XOFFS);
        case 'd':
            return(22+KEY_XOFFS);
        case 'e':
            return(18+KEY_XOFFS);
        case 'f':
            return(30+KEY_XOFFS);
        case 'g':
            return(38+KEY_XOFFS);
        case 'h':
            return(46+KEY_XOFFS);
        case 'i':
            return(58+KEY_XOFFS);
        case 'j':   
            return(54+KEY_XOFFS);
        case 'k':   
            return(62+KEY_XOFFS);
        case 'l':
            return(66+KEY_XOFFS);
        case 'm':
            return(58+KEY_XOFFS);
        case 'n':   
            return(50+KEY_XOFFS);
        case 'o':   
            return(66+KEY_XOFFS);
        case 'p':   
            return(2+KEY_XOFFS);
        case 'q':
            return(2+KEY_XOFFS);
        case 'r':   
            return(26+KEY_XOFFS);
        case 's':   
            return(14+KEY_XOFFS);
        case 't':
            return(34+KEY_XOFFS);
        case 'u':   
            return(50+KEY_XOFFS);
        case 'v':   
            return(34+KEY_XOFFS);
        case 'w':   
            return(10+KEY_XOFFS);
        case 'x':   
            return(18+KEY_XOFFS);
        case 'y':   
            return(10+KEY_XOFFS);
        case 'z':   
            return(42+KEY_XOFFS);
        default :
            return(-1); 
    }
}
            
int keyy(c)                             /* returns the y cordinate */
                                        /* of a given key   */
int c;
{
    switch(c){
        case 'q':
        case 'w':
        case 'e':
        case 'r':
        case 't':
        case 'z':
        case 'u':
        case 'i':
        case 'o':
            return(19);
        case 'a':
        case 's':
        case 'd':
        case 'f':
        case 'g':
        case 'h':
        case 'j':
        case 'k':
            return(21);
        case 'y':
        case 'x':
        case 'c':
        case 'v':
        case 'b':
        case 'n':
        case 'm':
        case 'l':
        case 'p':
            return(23);
        default:
            return(-1);         
        
    }
}



void drawface()                         /* paint Enigma */
{
    cls(BG);                                /* screen black */
    
                                            /* head */
    writexy("*** ENIGMA 0.4 ***",BG,28,0);
    writexy("<ESC> Exit   <F1> Setup   <Cursorkey> move rotors",
        BG,6,1);

    putxy('Q',KEY_NOR,2+KEY_XOFFS,19);      /* keyboard */
    putxy(' ',KEY_NOR,1+KEY_XOFFS,19);
    putxy(' ',KEY_NOR,3+KEY_XOFFS,19);
    putxy('W',KEY_NOR,10+KEY_XOFFS,19);
    putxy(' ',KEY_NOR,9+KEY_XOFFS,19);
    putxy(' ',KEY_NOR,11+KEY_XOFFS,19);
    putxy('E',KEY_NOR,18+KEY_XOFFS,19);
    putxy(' ',KEY_NOR,17+KEY_XOFFS,19);
    putxy(' ',KEY_NOR,19+KEY_XOFFS,19);
    putxy('R',KEY_NOR,26+KEY_XOFFS,19);
    putxy(' ',KEY_NOR,25+KEY_XOFFS,19);
    putxy(' ',KEY_NOR,27+KEY_XOFFS,19);
    putxy('T',KEY_NOR,34+KEY_XOFFS,19); 
    putxy(' ',KEY_NOR,33+KEY_XOFFS,19); 
    putxy(' ',KEY_NOR,35+KEY_XOFFS,19); 
    putxy('Z',KEY_NOR,42+KEY_XOFFS,19);
    putxy(' ',KEY_NOR,41+KEY_XOFFS,19);
    putxy(' ',KEY_NOR,43+KEY_XOFFS,19);
    putxy('U',KEY_NOR,50+KEY_XOFFS,19);
    putxy(' ',KEY_NOR,49+KEY_XOFFS,19);
    putxy(' ',KEY_NOR,51+KEY_XOFFS,19);
    putxy('I',KEY_NOR,58+KEY_XOFFS,19);
    putxy(' ',KEY_NOR,59+KEY_XOFFS,19);
    putxy(' ',KEY_NOR,57+KEY_XOFFS,19);
    putxy('O',KEY_NOR,66+KEY_XOFFS,19);
    putxy(' ',KEY_NOR,65+KEY_XOFFS,19);
    putxy(' ',KEY_NOR,67+KEY_XOFFS,19);
    putxy('P',KEY_NOR,2+KEY_XOFFS,23);
    putxy(' ',KEY_NOR,3+KEY_XOFFS,23);
    putxy(' ',KEY_NOR,1+KEY_XOFFS,23);
    putxy(' ',KEY_NOR,5+KEY_XOFFS,21);
    putxy('A',KEY_NOR,6+KEY_XOFFS,21);
    putxy(' ',KEY_NOR,7+KEY_XOFFS,21);
    putxy(' ',KEY_NOR,13+KEY_XOFFS,21);
    putxy('S',KEY_NOR,14+KEY_XOFFS,21);
    putxy(' ',KEY_NOR,15+KEY_XOFFS,21);
    putxy(' ',KEY_NOR,21+KEY_XOFFS,21);
    putxy('D',KEY_NOR,22+KEY_XOFFS,21);
    putxy(' ',KEY_NOR,23+KEY_XOFFS,21);
    putxy(' ',KEY_NOR,29+KEY_XOFFS,21);
    putxy('F',KEY_NOR,30+KEY_XOFFS,21);
    putxy(' ',KEY_NOR,31+KEY_XOFFS,21);
    putxy(' ',KEY_NOR,37+KEY_XOFFS,21);
    putxy('G',KEY_NOR,38+KEY_XOFFS,21);
    putxy(' ',KEY_NOR,39+KEY_XOFFS,21);
    putxy(' ',KEY_NOR,45+KEY_XOFFS,21);
    putxy('H',KEY_NOR,46+KEY_XOFFS,21);
    putxy(' ',KEY_NOR,47+KEY_XOFFS,21);
    putxy(' ',KEY_NOR,53+KEY_XOFFS,21);
    putxy('J',KEY_NOR,54+KEY_XOFFS,21);
    putxy(' ',KEY_NOR,55+KEY_XOFFS,21);
    putxy(' ',KEY_NOR,61+KEY_XOFFS,21);
    putxy('K',KEY_NOR,62+KEY_XOFFS,21);
    putxy(' ',KEY_NOR,63+KEY_XOFFS,21);
    putxy(' ',KEY_NOR,65+KEY_XOFFS,23);
    putxy('L',KEY_NOR,66+KEY_XOFFS,23);
    putxy(' ',KEY_NOR,67+KEY_XOFFS,23);
    putxy(' ',KEY_NOR,9+KEY_XOFFS,23);
    putxy('Y',KEY_NOR,10+KEY_XOFFS,23);
    putxy(' ',KEY_NOR,11+KEY_XOFFS,23);
    putxy(' ',KEY_NOR,17+KEY_XOFFS,23);
    putxy('X',KEY_NOR,18+KEY_XOFFS,23);
    putxy(' ',KEY_NOR,19+KEY_XOFFS,23);
    putxy(' ',KEY_NOR,25+KEY_XOFFS,23);
    putxy('C',KEY_NOR,26+KEY_XOFFS,23);
    putxy(' ',KEY_NOR,27+KEY_XOFFS,23);
    putxy(' ',KEY_NOR,33+KEY_XOFFS,23);
    putxy('V',KEY_NOR,34+KEY_XOFFS,23);
    putxy(' ',KEY_NOR,35+KEY_XOFFS,23);
    putxy(' ',KEY_NOR,41+KEY_XOFFS,23);
    putxy('B',KEY_NOR,42+KEY_XOFFS,23);
    putxy(' ',KEY_NOR,43+KEY_XOFFS,23);
    putxy(' ',KEY_NOR,49+KEY_XOFFS,23);
    putxy('N',KEY_NOR,50+KEY_XOFFS,23);
    putxy(' ',KEY_NOR,51+KEY_XOFFS,23);
    putxy(' ',KEY_NOR,57+KEY_XOFFS,23);
    putxy('M',KEY_NOR,58+KEY_XOFFS,23);
    putxy(' ',KEY_NOR,59+KEY_XOFFS,23);
    if(!cell_cover){
    putxy('Q',BULB_NOR,2+KEY_XOFFS,11);                 /* windows */
    putxy(' ',BULB_NOR,1+KEY_XOFFS,11);
    putxy(' ',BULB_NOR,3+KEY_XOFFS,11);
    putxy('W',BULB_NOR,10+KEY_XOFFS,11);
    putxy(' ',BULB_NOR,9+KEY_XOFFS,11);
    putxy(' ',BULB_NOR,11+KEY_XOFFS,11);
    putxy('E',BULB_NOR,18+KEY_XOFFS,11);
    putxy(' ',BULB_NOR,17+KEY_XOFFS,11);
    putxy(' ',BULB_NOR,19+KEY_XOFFS,11);
    putxy('R',BULB_NOR,26+KEY_XOFFS,11);
    putxy(' ',BULB_NOR,25+KEY_XOFFS,11);
    putxy(' ',BULB_NOR,27+KEY_XOFFS,11);
    putxy('T',BULB_NOR,34+KEY_XOFFS,11); 
    putxy(' ',BULB_NOR,33+KEY_XOFFS,11); 
    putxy(' ',BULB_NOR,35+KEY_XOFFS,11); 
    putxy('Z',BULB_NOR,42+KEY_XOFFS,11);
    putxy(' ',BULB_NOR,41+KEY_XOFFS,11);
    putxy(' ',BULB_NOR,43+KEY_XOFFS,11);
    putxy('U',BULB_NOR,50+KEY_XOFFS,11);
    putxy(' ',BULB_NOR,49+KEY_XOFFS,11);
    putxy(' ',BULB_NOR,51+KEY_XOFFS,11);
    putxy('I',BULB_NOR,58+KEY_XOFFS,11);
    putxy(' ',BULB_NOR,59+KEY_XOFFS,11);
    putxy(' ',BULB_NOR,57+KEY_XOFFS,11);
    putxy('O',BULB_NOR,66+KEY_XOFFS,11);
    putxy(' ',BULB_NOR,65+KEY_XOFFS,11);
    putxy(' ',BULB_NOR,67+KEY_XOFFS,11);
    putxy('P',BULB_NOR,2+KEY_XOFFS,15);
    putxy(' ',BULB_NOR,1+KEY_XOFFS,15);
    putxy(' ',BULB_NOR,3+KEY_XOFFS,15);
    putxy(' ',BULB_NOR,5+KEY_XOFFS,13);
    putxy('A',BULB_NOR,6+KEY_XOFFS,13);
    putxy(' ',BULB_NOR,7+KEY_XOFFS,13);
    putxy(' ',BULB_NOR,13+KEY_XOFFS,13);
    putxy('S',BULB_NOR,14+KEY_XOFFS,13);
    putxy(' ',BULB_NOR,15+KEY_XOFFS,13);
    putxy(' ',BULB_NOR,21+KEY_XOFFS,13);
    putxy('D',BULB_NOR,22+KEY_XOFFS,13);
    putxy(' ',BULB_NOR,23+KEY_XOFFS,13);
    putxy(' ',BULB_NOR,29+KEY_XOFFS,13);
    putxy('F',BULB_NOR,30+KEY_XOFFS,13);
    putxy(' ',BULB_NOR,31+KEY_XOFFS,13);
    putxy(' ',BULB_NOR,37+KEY_XOFFS,13);
    putxy('G',BULB_NOR,38+KEY_XOFFS,13);
    putxy(' ',BULB_NOR,39+KEY_XOFFS,13);
    putxy(' ',BULB_NOR,45+KEY_XOFFS,13);
    putxy('H',BULB_NOR,46+KEY_XOFFS,13);
    putxy(' ',BULB_NOR,47+KEY_XOFFS,13);
    putxy(' ',BULB_NOR,53+KEY_XOFFS,13);
    putxy('J',BULB_NOR,54+KEY_XOFFS,13);
    putxy(' ',BULB_NOR,55+KEY_XOFFS,13);
    putxy(' ',BULB_NOR,61+KEY_XOFFS,13);
    putxy('K',BULB_NOR,62+KEY_XOFFS,13);
    putxy(' ',BULB_NOR,63+KEY_XOFFS,13);
    putxy(' ',BULB_NOR,65+KEY_XOFFS,15);
    putxy('L',BULB_NOR,66+KEY_XOFFS,15);
    putxy(' ',BULB_NOR,67+KEY_XOFFS,15);
    putxy(' ',BULB_NOR,9+KEY_XOFFS,15);
    putxy('Y',BULB_NOR,10+KEY_XOFFS,15);
    putxy(' ',BULB_NOR,11+KEY_XOFFS,15);
    putxy(' ',BULB_NOR,17+KEY_XOFFS,15);
    putxy('X',BULB_NOR,18+KEY_XOFFS,15);
    putxy(' ',BULB_NOR,19+KEY_XOFFS,15);
    putxy(' ',BULB_NOR,25+KEY_XOFFS,15);
    putxy('C',BULB_NOR,26+KEY_XOFFS,15);
    putxy(' ',BULB_NOR,27+KEY_XOFFS,15);
    putxy(' ',BULB_NOR,33+KEY_XOFFS,15);
    putxy('V',BULB_NOR,34+KEY_XOFFS,15);
    putxy(' ',BULB_NOR,35+KEY_XOFFS,15);
    putxy(' ',BULB_NOR,41+KEY_XOFFS,15);
    putxy('B',BULB_NOR,42+KEY_XOFFS,15);
    putxy(' ',BULB_NOR,43+KEY_XOFFS,15);
    putxy(' ',BULB_NOR,49+KEY_XOFFS,15);
    putxy('N',BULB_NOR,50+KEY_XOFFS,15);
    putxy(' ',BULB_NOR,51+KEY_XOFFS,15);
    putxy(' ',BULB_NOR,57+KEY_XOFFS,15);
    putxy('M',BULB_NOR,58+KEY_XOFFS,15);
    putxy(' ',BULB_NOR,59+KEY_XOFFS,15);
    }
    
    putxy(ULC,ROTOR,29,5);                  /* ROTOR 1*/
    putxy(HOR,ROTOR,30,5);
    putxy(URC,ROTOR,31,5);
    putxy(VER,ROTOR,29,6);
    putxy(' ',ROTOR,30,6);
    putxy(VER,ROTOR,31,6);
    putxy(LLC,ROTOR,29,7);
    putxy(HOR,ROTOR,30,7);
    putxy(LRC,ROTOR,31,7);
    
    putxy(ULC,ROTOR,39,5);                  /* ROTOR 2*/
    putxy(HOR,ROTOR,40,5);
    putxy(URC,ROTOR,41,5);
    putxy(VER,ROTOR,39,6);
    putxy(' ',ROTOR,40,6);
    putxy(VER,ROTOR,41,6);
    putxy(LLC,ROTOR,39,7);
    putxy(HOR,ROTOR,40,7);
    putxy(LRC,ROTOR,41,7);
    
    putxy(ULC,ROTOR,49,5);                  /* ROTOR 3*/
    putxy(HOR,ROTOR,50,5);
    putxy(URC,ROTOR,51,5);
    putxy(VER,ROTOR,49,6);
    putxy(' ',ROTOR,50,6);
    putxy(VER,ROTOR,51,6);
    putxy(LLC,ROTOR,49,7);
    putxy(HOR,ROTOR,50,7);
    putxy(LRC,ROTOR,51,7);
    
    return;

}



void light(c)                           /* light one bulb */
int c;
{
    int x,y;
 
    x=keyx(c);
    y=keyy(c)-8;
    if((y>0)&&(x>0)){
        if(cell_cover){
            putxy(toupper(c),BULB_DIM,x,y);
        }else{
            putxy(toupper(c),BULB_LIT,x,y);
            putxy(' ',BULB_LIT,x-1,y);
            putxy(' ',BULB_LIT,x+1,y);
        }
    }   
}

void dim(c)                             /* turn off bulb */
int c;
{
    int x,y;
 
    x=keyx(c);
    y=keyy(c)-8;
    if((y>0)&&(x>0)){
        if(cell_cover){
            writexy("   ",MENU,x-1,y);
        }else{
            putxy(toupper(c),BULB_NOR,x,y);
            putxy(' ',BULB_NOR,x-1,y);
            putxy(' ',BULB_NOR,x+1,y);
        }
    }   
}

void press(c)                           /* mark key as pressed */
int c;
{
    int x,y;
 
    x=keyx(c);
    y=keyy(c);
    if((y>0)&&(x>0)){
        putxy(' ',KEY_PRES,x-1,y); 
        putxy(toupper(c),KEY_PRES,x,y);
        putxy(' ',KEY_PRES,x+1,y);
    }
    
    return;   
}

void release(c)                         /* unmark key */
int c;
{
    int x,y;
 
    x=keyx(c);
    y=keyy(c);
    if((y>0)&&(x>0)){
        putxy(' ',KEY_NOR,x-1,y); 
        putxy(toupper(c),KEY_NOR,x,y);
        putxy(' ',KEY_NOR,x+1,y);
    }
    return;   
}

void rotor_act()                        /* actualize rotor window */
{
    putxy(toupper(window[0]),ROTOR,50,6);
    putxy(toupper(window[1]),ROTOR,40,6);
    putxy(toupper(window[2]),ROTOR,30,6);
    
    return;
}

int r_mark(n)                           /* mark rotor */
int n;
{
    int ret;
    ret=n;
    if(ret<0){
        ret=2;
    }
    if(ret>2){
        ret=0;
    }
    switch(ret){
        case 0:
            putxy(TRI,ROTOR,50,8);
            break;
        case 1:
            putxy(TRI,ROTOR,40,8);
            break;
        case 2:
            putxy(TRI,ROTOR,30,8);
            break;              
    }
    return(ret);
    
}

void r_unmark()                          /* cancel marking of rotor */
{
    putxy(' ',BG,30,8);
    putxy(' ',BG,40,8);
    putxy(' ',BG,50,8);
    
    return;
} 


void tty(c)                              /* simulates a tty connected to */
                                         /* the Enigma */
char c;
{
    int n,shift;    


    if(teletype){    shift=1;
        fivecount=modulo(fivecount+1,5);
        if(!fivecount){
            shift=2;
            ttystrip[78]=' ';
            ttystrip[79]='\0';
        }

        for(n=0; n<78; n++){
            ttystrip[n]=ttystrip[n+shift];  
        }

        ttystrip[77]=c;
        ttystrip[78]='\0';
        writexy(ttystrip,KEY_NOR,1,2);
    }    
}                  

/* -------------------Testroutinen ----------------------------------*/
void test()                             /* shows some basic data */
                                        /* (for debugging) */
{
    int mode,n;
    
    /*cls(MENU);*/

    for(n=0;n<ROWS;n++)
        printf("\r\n");

    ScreenSetCursor(0,0);
    
    mputs("*** ENIGMA Info ***");
#ifdef __STDC__

#ifdef NEEDSCR
    printf("compiled at %s\r\n",__DATE__);
#else
    printf("compiled at %s\n",__DATE__);
#endif
#endif

#ifdef DEBUG                              
    printf("0  mod 26 = %d \n", modulo(0,26));
    printf("-1 mod 26 = %d \n", modulo(-1,26));
    printf("25 mod 26 = %d \n", modulo(25,26));
    printf("26 mod 26 = %d \n", modulo(26,26));
    printf("27 mod 26 = %d \n", modulo(27,26));
    puts("translate: ");
    printf("0 trans %s\n",translate(0));
    printf("1 trans %s\n",translate(9));
    printf("10 trans %s\n",translate(10));
    printf("90 trans %s\n",translate(90));
#endif
    
#ifdef MSDOS
    mputs("Compiled for DOS");
#ifdef ANSI_COL
    mputs("ANSI.SYS Version");
#endif  
#ifdef MSDOSBW
    mputs("b/w Version (uses ANSI.SYS)");
#endif
#else
#ifdef VT
    mputs("vt100 Version");
#endif
#ifdef VT320
    mputs("   uses extended control sequences described");
    mputs("   in a vt320 terminal manual");
#endif
#ifdef VT52
    mputs("vt52 Version (tested on Atari)");
#endif
#ifdef UNIX
    mputs("Compiled for UNIX");
#endif
#ifdef MSDOSBW
    mputs(" Uses inverse b/w");
#endif
#ifdef ATARI_ST
   mputs("Compiled for Atari ST");
#endif
#endif
#ifdef __GNUC__
    mputs("Compiled with GCC");
#ifdef __GO32__
    mputs("Go32 Version");
#endif
#ifdef __EMX__
    mputs("EMX Version");
#endif

#endif


#ifdef MSDOS
#ifdef __GNUC__ 
    mputs("Assuming 80*25 text mode");

#else
    mputs("Assuming 80*25 text mode");
#endif
#else
    mputs("Assuming 80*25 text mode");         
#endif 



#ifdef CURSES
#ifndef SWEARWORDS
    mputs("Using Curses for getch(),");
    mputs("                 initialization of the screen");
    mputs("             and cleaning up after leaving");
#else
    mputs("Using curses for input and output!");
#ifdef BSDCURSES
    mputs("Using the BSD version ");
#else
    mputs("Using the System V version");
#endif
#endif
#endif


#ifdef SUN
    mputs("Using additional <CntrlL> for cls()"); 
#endif

#ifdef NEEDSCR
    mputs("Using additional <CR> for output");
#endif

    mputs("<Return>");
    fflush(stdout);
    getchar();
}

/* ------------------- Encryption ----------------------------------- */



int modulo(n,mod)                       /* simple modulo function */
int n,mod;
{
    int ret;
    
    
#ifdef DEBUG
    writexy("modulo()              ",BG,0,3);
#endif

    ret=n;
    
    if(mod==0)
        return(0);
    while(ret>=mod){
#ifdef DEBUG
        writexy("modulo: Operand>= Modul             ",BG,0,3);
        /*fprintf(stderr,"Operand %d RET %d Modul %d     ",n,ret,mod);*/
#endif          
        ret -=mod;         
    }
    while(ret<0){
#ifdef DEBUG
        writexy("modulo: Operand<0                  ",BG,0,3);
#endif  
        ret+=mod;
    }
#ifdef DEBUG
/*  drawface();  */
    writexy("modulo() finished              ",BG,0,3);
#endif
    
    return(ret);
}

int order(c)                            /* maps a...z -> 0..25 ab */            
int c;
{
    if(c<((int)'a')||c>((int)'z')){
        writexy("order:UNKNOWN CHARACTER        ",BG,0,4);
        putxy(c,BG,40,4);
        printf("--- %d ---\n",c);
        /*puts("<Enter>"); */
        getchar();
        exit(-1);
    }
    return(c-(int)'a');
}

int chaos(n)                            /* inverse function to order() */
int n;
{
    if((n<0)||n>25){
        writexy("chaos:UNKNOWN CHARACTER        ",BG,0,4);
        putxy(n+'a',BG,40,4);
        getchar();
        exit(-1);
    }
    return(n+(int)'a');

}

void r_turn(n,width)                    /* move rotor n  */
                                        /* width times */
int n,width;
{
    int x;
    
    x=order(rotor[n]);
    rotor[n]=chaos(modulo(x+width,26));
    x=order(window[n]);
    window[n]=chaos(modulo(x+width,26));
    
    return;
}

void turn()                             /* move rotors */
{
    int doit[4],n;
                                            /* first calculate */
                                            /* stepwidth for each rotor */
    doit[0]=stepwidth;
    
    if((rot_notch[0]==window[0])||(rot_notch[1]==window[1])){
        doit[1]=stepwidth;
    }else{
        doit[1]=0;
    }
    
    if(rot_notch[1]==window[1]){
        doit[2]=stepwidth;
    }else{
        doit[2]=0;
    }
                                            /* turn rotors simultanously */
    for(n=0;n<3;n++){
        r_turn(n,doit[n]);
    }
    
    return; 
}


int c_back(c,r)                         /* encrypts backward */
int c;                                      /* character */
int r;                                      /* which rotor */
{
    int n,m,offset,index,newchar;
    char *currotor;

    currotor=rot_wir[r];
#ifdef DEBUG    
    writexy("back: encrypt ",BG,0,3);   
    putxy(c,BG,24,3);
#endif

    n=order(c);
    offset=order(rotor[r]);
    n=modulo(n+offset,26);
    newchar=chaos(n);
    
    index=-1;
    for(m=0;m<25;m++){
        if(currotor[m]==newchar){
            index=m;
        }
    }
    index -= offset;
    index=modulo(index,26);

    if(index > -1){
        return(chaos(index));
    }else{
    
        writexy("back: couldn't map index back",BG,0,4);
        
        getchar();
        exit(-1);
    }
}

int c_forth(c,r)                        /* encrypt forward */
int c;                                      /* character */
int r;                                      /* which rotor */
{
    int n,offset,index,ret;
    char *currotor;

    currotor=rot_wir[r];
#ifdef DEBUG    
    writexy("c_forth: encrypt ",BG,0,3);
    putxy(c,BG,24,3);
#endif
    n=order(c);
#ifdef DEBUG        
    writexy("forth:calculating offset...                            ",BG,0,3);
#endif
    offset=order(rotor[r]);
    index=modulo(n+offset,26);
    if(index>25){
        writexy("forth: modulo returns rubbish      ",BG,0,4);
        printf("%d + %d = %d mod 26\n",n,offset,index);
        getchar();
        exit(-1);   
    }
    ret=order(currotor[index]);
    ret=modulo(ret-offset,26);
    ret=chaos(ret);

    return(ret);
}

int deflekt(c)                          /* simulates reflector */
int c;
{
    int n,ret;

    /*return(c);*/
#ifdef DEBUG    
    writexy("deflekt(): encrypting ",BG,0,3);   
    putxy(c,BG,30,3);
#endif  
    n=order(c); 
    ret=refl[n];

    return(ret);
        
}

int steck(c)                            /* simulates plugboard */
int c;
{
    int n,ret;

    /*return(c);*/
#ifdef DEBUG    
    writexy("steck(): encrypting ",BG,0,3);
    putxy(c,BG,30,3);
#endif  
    n=order(c); 
    ret=stecker[n];

    return(ret);
        
}

int crypted(c)                          /* main encryption function */
int c;
{
    int reto,reti,retii,retiii,retiv,retv,retvi,retvii,retviii;
    
    turn();                                 /* move rotors */
    
    reto=   steck(c);                       /* plugboard */
    reti=   c_forth(reto,0);                /* encrypt forward */
    retii=  c_forth(reti,1);
    retiii= c_forth(retii,2);
    retiv=  deflekt(retiii);                /* reflector */  
    retv=   c_back(retiv,2);                /* backwards */
    retvi=  c_back(retv,1);
    retvii= c_back(retvi,0);
    retviii=steck(retvii);                  /* plugboard */
    
                                            /* produce trace */

    if(debug){
        writexy("                                                      ",
        BG,0,3);
        putxy(c,BG,0+TRACEXOFS,3);
        putxy(reto,BG,2+TRACEXOFS,3);
        putxy(reti,BG,4+TRACEXOFS,3);
        putxy(retii,BG,6+TRACEXOFS,3);
        putxy(retiii,BG,8+TRACEXOFS,3);
        putxy(retiv,BG,10+TRACEXOFS,3);
        putxy(retv,BG,12+TRACEXOFS,3);
        putxy(retvi,BG,14+TRACEXOFS,3);
        putxy(retvii,BG,16+TRACEXOFS,3);
        putxy(retviii,BG,18+TRACEXOFS,3);   
    }
    
    return(retviii);
}

/*---------------------- Setup --------------------------------------------*/

void s_plugboard()                      /* make connections on the */
                                        /* plug board */
{
    int con,with,c,x,y,m,n;
    
    c=0;
    cls(MENU);                          /* menu */
    writexy("***************************************",MENU,0,0);
    writexy("*              Plugboard              *",MENU,0,1);
    writexy("***************************************",MENU,0,2);
    
    writexy("<ESC> back to setup",MENU, 0,4);

    writexy("Plugboard:",MENU,0,20);
    writexy("1. Plug",MENU,0,5);
    writexy("2. Plug",MENU,0,6);

        
    while(c!=27){
        putxy(' ',MENU,11,5);
        putxy(' ',MENU,11,6);
                    
        m=0;
        for(n=0;n<26;n++){                  /* show plugboard */
            if(n<13){
                putxy(' ',MENU,6*n,21);
                putxy(' ',MENU,6*n+1,21);
                putxy(' ',MENU,6*n+3,21);
                putxy(' ',MENU,6*n+4,21);
            }
            if(chaos(n)==stecker[n]){
            }else{
                if(chaos(n)<stecker[n]){
                    putxy('(',MENU,6*m,21);
                    putxy(chaos(n),MENU,6*m+1,21);
                    putxy(stecker[n],MENU,6*m+3,21);
                    putxy(')',MENU,6*m+4,21);
                    m++;
                }
            }   
        }
    

                                            /* enter 1st plug */

        con='a';
        putxy(con,MENU_CUR,11,5);
        while((c=inkey())!=ESC
            &&(c!='\n')
            &&(c!='\r')
            ){
            putxy(c,MENU_CUR,11,5);
            if(islower(c)){
                con=c;
            }else{
            }
        }
        putxy(con,MENU,11,5);
        if(iscntrl(c)&&(c!=ESC)){           /* enter 2nd plug  */
            with='a';
            putxy(with,MENU_CUR,11,6);
            while((c=inkey())!=ESC
                &&(c!='\r')
                &&(c!='\n')
                ){
                putxy(c,MENU_CUR,11,6);
                if(islower(c)){
                    with=c;
                }else{
                }
            }
            putxy(with,MENU,11,6);
            if(iscntrl(c)&&(c!=ESC)){       
                                            /* connect 1st plug*/
                 x=order(con);
                 if((y=stecker[x])!=con){       /* occupied?->disconnect*/  
                    stecker[order(y)]=y;
                    stecker[x]=con; 
                 }
                 stecker[x]=with;               /* plug in */
                                            /* connect 2nd plug */
                 x=order(with);
                 if((y=stecker[x])!=with){      /* occupied -> disc. */
                    stecker[order(y)]=y;
                    stecker[x]=con;
                 }
                 stecker[x]=con;                /* plug in */
            }

        }
    }
    return; 
}

int ssrotor(which,what)                 /* set wirings of a single rotor */
int which,what;
{
    switch(what){
        case 't':
            strcpy(rot_wir[which],ROT_t);
            rot_notch[which]=NOTCH_t;
            rot_typ[which]='t';
            break;
        case '1':
            strcpy(rot_wir[which],ROT_i);
            rot_notch[which]=NOTCH_i;
            rot_typ[which]='1';
            break;
        case '2':
            strcpy(rot_wir[which],ROT_ii);
            rot_notch[which]=NOTCH_ii;
            rot_typ[which]='2';
            break;
        case '3':
            strcpy(rot_wir[which],ROT_iii);
            rot_notch[which]=NOTCH_iii;
            rot_typ[which]='3';
            break;
        case '4':
            strcpy(rot_wir[which],ROT_iv);
            rot_notch[which]=NOTCH_iv;
            rot_typ[which]='4';
            break;
        case '5':
            strcpy(rot_wir[which],ROT_v);
            rot_notch[which]=NOTCH_v;
            rot_typ[which]='5';
            break;
        default:
            return(-1);
            
    }
    rot_ini[which]='a';
    rotor[which]='a';
    window[which]='a';
    
    return(0);
}

int askrotor(which)                         /* setting a single rotor */
                                            /*  interactivly */
int which;                                  /* which: 0-2 */
{
    int c,x,y;
    
    x=32;
    switch(which){
        case 2:
            y=10;
            writexy("Rotor 3 (left,slow) [1-5]:",MENU,0,y);
            break;
        case 1:
            y=13;
            writexy("Rotor 2 (middle) [1-5]:",MENU,0,y);
            break;
        case 0:
            y=16;
            writexy("Rotor 1 (right,fast) [1-5]:",MENU,0,y);
            break;
        default:
            writexy("Too much rotors",MENU_AL,0,20);
            return('E');
    }
    while(1){
        putxy(rot_typ[which],MENU_CUR,x,y);
        c=inkey();
        writexy(
        "                                                             ",
        MENU,0,20);
        if(isalnum(c))
            putxy(c,MENU,x,y);
        if(c=='t'){
            c=0;
        }
        if(isdigit(c)){
            if(ssrotor(which,c)<0){
                writexy("This rotor doesn't exist!",MENU_AL,0,20);
            }
            
        }
        if(c==ESC)
            return(c);
        if(c=='\r'||c=='\n'){
            c='\n';
            break;
        }   
    }
    putxy(rot_typ[which],MENU,x,y);
    writexy("Ringsetting:",MENU,0,y+1);
    while(1){
        /*putxy(rotor[which],MENU_CUR,x,y+1);*/
        putxy(chaos(modulo(order(window[which])-order(rotor[which]),26)),
            MENU_CUR,x,y+1);
    
        c=inkey();
        writexy(
        "                                                             ",
        MENU,0,20);
        if(islower(c)){
            putxy(c,MENU_CUR,x,y+1);
            rotor[which]=chaos(modulo(order(window[which])-order(c),26));
        }
            
        if(c==ESC)
            return(c);
        if(c=='\r'||c=='\n'){
            c='\n';
            break;
        }   
    }
    putxy(chaos(modulo(order(window[which])-order(rotor[which]),26)),
        MENU,x,y+1);
    
    return(c);
}



void s_rotors()                             /* Rotors & Refl. init. */
{
    int con,with,c,x,y,z,n;
    
    cls(MENU);                                  /* Menu */
    
    writexy("***************************************",MENU,0,0);
    writexy("*         Rotors and Reflektor        *",MENU,0,1);
    writexy("***************************************",MENU,0,2);
    
    writexy("<ESC> back to Setup",MENU, 0,4);

    
    c=0;
    while(c!=ESC){
        n=2;
        while(n>-1){
            c=askrotor(n);
            if(c==ESC)
                break;
            n--;
        }
        if(c==ESC)
            break;

        c=0;
        writexy("Reflector [b|c]:",MENU,0,19);
        while(1){
            putxy(refl_typ,MENU_CUR,32,19);
            c=inkey();
            writexy(
    "                                                                  ",
                MENU,0,20);     
            /*c = tolower(c);*/
            if(islower(c)){
                putxy(c,MENU,32,19);
                switch(c){
                    case 't':
                        strcpy(refl,REFL_t);
                        refl_typ='t';
                        break;
                    case 'b':
                        strcpy(refl,REFL_b);
                        refl_typ='b';
                        break;
                    case 'c':
                        strcpy(refl,REFL_c);
                        refl_typ='c';
                        break;
                    default:
                        writexy("This reflektor doesn't exist",MENU_AL,0,20);       
                }
            }
            if(c=='\r'||c=='\n')
                break;
            if(c==ESC)
                return; 
        }
        putxy(refl_typ,MENU,32,19);     
    }
    
    return;
}



void setup()                                /* Main menu */
{
    int c,n,m,refr;
    
    cls(MENU);
    writexy("***************************************",MENU,0,0);
    writexy("*           Enigma Setup              *",MENU,0,1);
    writexy("***************************************",MENU,0,2);

    writexy("<ESC> Exit",MENU,0,4);
    putxy('P',MENU_EM,0,10);

    writexy("lugboard",MENU,1,10);
    putxy('C',MENU_EM,0,11);
    writexy("hoose rotors and reflector ",MENU,1,11);
    putxy('E',MENU_EM,0,12);
    writexy("nigma",MENU,1,12);
    
    writexy("Rotors:",MENU,0,15);
    writexy("Ringsetting:",MENU,0,16);
    writexy("Window:",MENU,0,17);
    for(n=0;n<3;n++){
        putxy(rot_typ[n],MENU,(24-(2*n)),15);
        putxy(chaos(modulo(0-(order(rotor[n])-order(window[n])),26)),
            MENU,(24-2*n),16);
        putxy(window[n],MENU,(24-2*n),17);
    }
    writexy("Reflector:",MENU,0,18);
    putxy(refl_typ,MENU,20,18);
    
    writexy("Plugboard:",MENU,0,20);        
    m=0;
    for(n=0;n<26;n++){
        if(chaos(n)==stecker[n]){
        }else{
            if(chaos(n)<stecker[n]){
                putxy('(',MENU,6*m,21);
                putxy(chaos(n),MENU,6*m+1,21);
                putxy(stecker[n],MENU,6*m+3,21);
                putxy(')',MENU,6*m+4,21);
                m++;
            }
        }   
    }
    
        
    while(((c=inkey())!='e')){
        refr=0;
        switch(c){
            case 'p':
            case 's':
                s_plugboard();
                refr=1;
                break;
            case 'a':
            case 'c':
                s_rotors();
                refr=1;
                break;
            case ESC:                       /* <ESC> */
            case CntrlC:            
                gr_exit();
                exit(0);
                break;
        } 
        if(refr){
        cls(MENU);
        writexy("***************************************",MENU,0,0);
        writexy("*           Enigma Setup              *",MENU,0,1);
        writexy("***************************************",MENU,0,2);

        writexy("<ESC> Exit",MENU,0,4);
        putxy('P',MENU_EM,0,10);

        putxy('P',MENU_EM,0,10);
        writexy("lugboard",MENU,1,10);
        putxy('C',MENU_EM,0,11);
        writexy("hoose rotors and reflektor ",MENU,1,11);
        putxy('E',MENU_EM,0,12);
        writexy("nigma",MENU,1,12);
        
        writexy("Rotors:",MENU,0,15);
        writexy("Ringsetting:",MENU,0,16);
        writexy("Window:",MENU,0,17);
    
        for(n=0;n<3;n++){
            putxy(rot_typ[n],MENU,(24-(2*n)),15);
            putxy(chaos(modulo(0-(order(rotor[n])-order(window[n])),26)),
                MENU,(24-2*n),16);
            putxy(window[n],MENU,(24-2*n),17);
        }
        writexy("Reflector:",MENU,0,18);
        putxy(refl_typ,MENU,20,18);
        writexy("Plugboard:",MENU,0,20);        
        /*writexy("abcdefghijklmnopqrstuvwxyz",MENU,10,21);*/
        m=0;
        for(n=0;n<26;n++){
            if(chaos(n)==stecker[n]){
                /*putxy(stecker[n],MENU,10+n,22);*/
            }else{
                if(chaos(n)<stecker[n]){
                    putxy('(',MENU,6*m,21);
                    putxy(chaos(n),MENU,6*m+1,21);
                    putxy(stecker[n],MENU,6*m+3,21);
                    putxy(')',MENU,6*m+4,21);
                    m++;
                }
            }   
        }
        }
            
    }
    
    return; 
}

/* ------------------------- MCP --------------------------------------- */

int main(argc,argv)                 /* Master Controll Program (Hi,Tron!) */
int argc;
char **argv;
{
    int c,oldc,chiff,oldchiff,o,n,m,rot_edi;
    char *s;
                                        /* Options*/
    debug=0;
    cell_cover=0;                                   
    teletype=0;


    if(argc>1){
        for(n=1;n<argc;n++){
            s=argv[n];
            if(s[0]=='-'){
                for(m=1;s[m]!='\0';m++){
                    switch(s[m]){
                        case 'd':
                            debug=1;
                            break;
                        case 'c':
                            cell_cover=1;
                            break;
                        case 't':
                            ttystrip=(char *)malloc(80); /* initialising */
                                                         /* ttystrip */
                            if(ttystrip){
                                for(o=0;o<78;o++){
                                    ttystrip[o]=' ';
                                }
                                ttystrip[78]='\0'; 
                                teletype=1;
                            }
                            break; 
                    }
                }   
            }
        }
    }
                                        
                                        /* Init */
    n=0;
    oldc='a';
    window[0]=rot_ini[0]=rotor[0]='a';
    window[1]=rot_ini[1]=rotor[1]='a';
    window[2]=rot_ini[2]=rotor[2]='a';
    rot_wir[0]=strdup(ROT_i);
    rot_wir[1]=strdup(ROT_ii);
    rot_wir[2]=strdup(ROT_iii);
    rot_notch[0]=NOTCH_i;
    rot_notch[1]=NOTCH_ii;
    rot_notch[2]=NOTCH_iii;
    rot_typ[0]='1';
    rot_typ[1]='2';
    rot_typ[2]='3';
    refl=strdup(REFL_b);
    refl_typ='b';
    stecker=strdup("abcdefghijklmnopqrstuvwxyz");
    stepwidth=1;
    rot_edi=0;
    fivecount=3;
    if(!(rot_wir[0]                         /* everything allocated ? */
        &&rot_wir[1]
        &&rot_wir[2]
        &&refl
        &&stecker
    )){
        fprintf(stderr,"NOT ENOUGH MEMORY\n");
        exit(-1);
    }
    if(debug)                           /* show debug information */
        test();

    gr_init();                          /* initialise graphics */
    intro();                            /* show title */
    setup();                            /* set up Enigma */
    drawface();                         /* paint enigma */
    rotor_act();
    r_mark(rot_edi);
    tty(' ');
    /*light('a');*/
    while((c=inkey())!=ESC){            /* main loop */
        release(oldc);
        dim(oldchiff);
        if(c<'a'|c>'z'){
            switch(c){
                case CntrlF:
                case RghtArrw:                      /* RightArrow:*/
                                                    /* move marker right */
                    r_unmark();
                    rot_edi=r_mark(rot_edi - 1);
                    break;
                case CntrlB:    
                case LftArrw:                       /* LeftArrow: s.o. */
                    r_unmark();
                    rot_edi=r_mark(rot_edi+1);
                    break;
                case CntrlP:    
                case UpArrw:                         /* UpArrow: Rotor back */     
                    /*n=order(rotor[rot_edi]);*/
                    /*n=modulo(n+1,26);*/
                    /*rotor[rot_edi]=chaos(n);*/
                    r_turn(rot_edi,1);
                    break;
                case CntrlN:    
                case DwnArrw:                       /* DwnArrw: Rotor forth */
                    /*n=order(rotor[rot_edi]);*/
                    /*n=modulo(n-1,26);*/
                    /*rotor[rot_edi]=chaos(n);*/
                    r_turn(rot_edi,25);
                    break;
                case '1':       
                case F1:                            /* <F1> Setup   */  
                    setup();
                    drawface();                 
                    rotor_act();
                    r_mark(rot_edi);
                    
                    fivecount=4;                       /*start a new message*/
                    tty('+');
                    tty('+');
                    tty('+');
                    tty('+');
                    tty('+');
                    break;                      
            }
            rotor_act();
        }else{
            oldc=c;
            press(c);
            chiff=crypted(c);
            rotor_act();
            light(chiff);
            tty(chiff);
            oldchiff=chiff;
        }
    }
    gr_exit();                                   /* clean up screen */
    return(0);
}
