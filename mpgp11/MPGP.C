/*  MPGP ver 1.1  last modified 3/14/96 */
/*  compiles under Microsoft C ver 6.0  */

// must link with setargv.obj to process wildcards .. MS C
// link with wildcard.obj to process wildcards .. TurboC (I think)
// see the function exist() for a certain portability problem  (inline asm)
// inline asm in randseed() also

#define NORM_FILE 0x00     // DOS attribute for normal files
#define ALL_FILES 0x1e     // DOS attribute for all files (hidden,dir,etc.)


#include <stdio.h>
#include <conio.h>
#include <string.h>
#include <stdlib.h>
#include <memory.h>
#include <process.h>
#include <io.h>
#include <errno.h>
#include <time.h>
#include <fcntl.h>
#include <sys\types.h>
#include <sys\stat.h>
#include <dos.h>


// functions in mpgp.c :
int   main(int  argc,char  * *argv);
void  initialize(void);
void  banner(void);
void  help(void);
void  do_option(char  *s);
void  do_file(int  i,char  * *argv);
int   exist(char  *fname,int  attr);
void  fetch_c_password(void);
void  fetch_p_password(void);
void  lead_zero(char  *s,unsigned int  cnt);
void  cleanup(void);
void  no_exec(void);
int   is_public(char  *fname);
void  check_files(int  argc,char  * *argv);
void  fetch_keyid(void);
void randseed(int method);
void _interrupt _far newtimer( void );
unsigned int getrand(void);


struct options {
       int encrypt;
       int decrypt;
       int randomize;
       int zero;
       int public;      // set if public key encryption is to be used
       int keep;
       int overwrite;   // set to 1 if +force is to be used
       int something;   // set to one if at least one file is processed
       int ret;         // max error return from pgp.exe
       int locked;      // set to 1 when no more options acceptable
       char zparm1[4];
       char pass1[260];
       char pass2[260];
       char zparm2[4];
       char pub_pass[260];
       char key_id[120];
       int c_pass_fetched;   // set to 1 when conventional password gotten;
       int p_pass_fetched;   // set to 1 when public password gotten
       int pgppass_set;      // set to 1 if PGPPPASS is set (don't use -z)
       char *table;          // pointer of array to bytes describing files
       int files_checked;    // set to 1 when list of files checked for type
       int seeded;           // set to one when RNG seeded
       };

struct options o; // o.encrypt o.decrypt  o.randomize,etc. are global



int main(int argc, char *argv[])
{
   extern struct options o;
   int i;


   banner();

   if (argc<2) {
      help();
      return 1;
   }

   initialize();

   for(i=1;i<argc;i++) {
      if ( (*argv[i] == '/') || (*argv[i] == '-'))
         do_option(argv[i]);
      else {
         if (o.decrypt && !o.files_checked)
            check_files(argc,argv);
         do_file(i,argv);
      }
   }
   if ( (o.encrypt==0) && (o.decrypt==0) ) {
      help();
      return 1;
   }
   if (!o.something) {
      printf("Nothing to Do!\n");
      return 0;
   }

   return o.ret;

}  // end of main()



void initialize(void)
{
   extern struct options o;


   atexit(cleanup);

   o.zparm1[0]=' ';
   o.zparm1[1]='-';
   o.zparm1[2]='z';
   o.zparm1[3]='"';

   o.zparm2[0]=' ';
   o.zparm2[1]='-';
   o.zparm2[2]='z';
   o.zparm2[3]='"';


   if ( (getenv("PGPPASS")) != NULL) {
      o.p_pass_fetched =1;
      o.pgppass_set=1;
      o.zparm2[0]=0;
   }

}  // end of initialize()


void banner()
{
   printf("\nMPGP  ver 1.1 (3/14/96)   by Mark T. Anderson <mta@clark.net>\n\n");
}  // end of banner()



void help()
{
      printf("Usage: MPGP -option(s)  file1 [file2..] (Wildcards accepted)\n");
      printf("options: \n");
      printf("         -e Encrypt file(s)\n");
      printf("         -p[KeyId] use Public key encryption (default is conventional)\n");
      printf("         -r Randomize name(s) of encrypted file(s), keeping .PGP extension\n");
      printf("         -z set date/time of encrypted file(s) to Zero\n");
      printf("         -d Decrypt file(s)\n");
      printf("         -o allow Overwriting of file(s)\n");
      printf("         -k Keep file(s) after processing (default is to wipe/delete)\n");
      printf("\n");

}  // end of help()


void do_option(char *s)
{
   extern struct options o;
   size_t i;


   if (o.locked)
      return;


   for (i=1;i<strlen(s);i++) {

      switch (s[i]) {
         case 'D':
         case 'd':
            o.decrypt=1;
            o.encrypt=0;
            o.randomize=0;   // disallow randomize && decrypt
            o.zero=0;        // disallow zero && decrypt
            break;

         case 'K':
         case 'k':
            o.keep=1;
            break;

         case 'R':
         case 'r':
            o.decrypt=0;     // disallow randomize && decrpyt
            o.randomize=1;
            break;

         case 'O':
         case 'o':
            o.overwrite=1;
            break;

         case 'E':
         case 'e':
            o.decrypt=0;
            o.encrypt=1;
            break;

         case 'Z':
         case 'z':
            o.zero=1;
            o.decrypt=0;  // disallow zero && decrypt
            break;

         case 'P':
         case 'p':
            o.public=1;
            if (s[i+1])
               strncpy(o.key_id,&s[i+1],119);
            return;
            break;

         default:
            break;

      }  // end of switch (s[i])

   } // end of for()

}  // end of do_option()



void do_file (int i,char *argv[])
{
   extern struct options o;

   char drive[_MAX_DRIVE], dir[_MAX_DIR];
   char fname[_MAX_FNAME], ext[_MAX_EXT];
   char rname[_MAX_FNAME];
   char pathold[_MAX_PATH];
   char pathnew[_MAX_PATH];
   char buff[35];
   int  n,ret,handle;
   char *force;   // will point to "+force" or ""  depending on o.overwrite
   int attempts;  // number of attempts at unique filename

   o.locked = 1;  // disallow further options on command line
   attempts=0;

   if (o.overwrite)
      force="+force";
   else
      force="";

   _splitpath( argv[i], drive, dir, fname, ext );

   if (!exist(argv[i],NORM_FILE))
      return;

   if (o.encrypt) {
      if (stricmp(".PGP",ext) == 0)
         return;

      // seed RNG if necessary
      if ( (!o.seeded) && (o.randomize) ) {
         if (o.zero)
            randseed(1); // get better random seed to hide filetime
         else
            randseed(0); // simpler randseed will suffice

         o.seeded=1;
      }

      if (o.public)
         fetch_keyid();
      else
         fetch_c_password();  // aborts if invalid password
      o.something=1;

      if (o.public) {
         if (o.keep)
            ret=spawnlp(P_WAIT,"pgp.exe","mpgp","-e",force,argv[i],o.key_id,NULL);
         else
            ret=spawnlp(P_WAIT,"pgp.exe","mpgp","-ew",force,argv[i],o.key_id,NULL);
      }  // end of if(o.public)
      else {
         if (o.keep)
            ret=spawnlp(P_WAIT,"pgp.exe","mpgp","-c",force,o.zparm1,argv[i],NULL);
         else
            ret=spawnlp(P_WAIT,"pgp.exe","mpgp","-cw",force,o.zparm1,argv[i],NULL);
      }  // end of else !o.public

      if (ret== -1)
         no_exec();      // no_exec() exits to DOS
      o.ret=max(o.ret,ret);
      if (ret) {
         printf("Error encrypting %s   PGP returned %i\n",argv[i],ret);
         return;
      }

      _makepath(pathold,drive,dir,fname,".PGP");

      if (o.zero)  {
         if (_dos_open(pathold,O_RDONLY,&handle) !=0) {
            printf("Unable to set date on file %s\n",pathold);
            o.ret=max(o.ret,1);
         }
         else {
            _dos_setftime(handle,(unsigned)0,(unsigned)0);
            _dos_close(handle);
         }
      }  // end of if(o.zero)

      if (o.randomize) {

again:   rname[0]=0;

         for(n=0;n<4;n++) {
            itoa((getrand() &0x00ff),buff,16);
            lead_zero(buff,2);
            strcat(rname,buff);
         }

         strupr(rname);
         _makepath(pathnew,drive,dir,rname,".PGP");

         if (exist(pathnew,ALL_FILES)) {
            attempts++;
            if (attempts >32) {   // try 32 filenames before re-seeding
               randseed(1);
               attempts=0;
            }
            goto again;                    // real C programmers are not
         }                                 // afraid to use goto
         attempts=0;

         printf("renaming %s to %s\n\n\n",pathold,pathnew);
         rename(pathold,pathnew);

      }  // end of if (o.randomize)

   }  // end of if (o.encrypt)



   else if (o.decrypt) {
      if (stricmp(".PGP",ext) != 0)
         return;

      o.something=1;
      if (o.table[i] == 2) // IDEA
         ret=spawnlp(P_WAIT,"pgp.exe","mpgp",o.zparm1,"-p",force,argv[i],NULL);
      else  // RSA
         ret=spawnlp(P_WAIT,"pgp.exe","mpgp",o.zparm2,"-p",force,argv[i],NULL);

      if (ret == -1)
         no_exec();
      o.ret=max(o.ret,ret);
      if (ret) {
         printf("Error Decrypting %s PGP returned %i\n",argv[i],ret);
         return;
      }
      if (!o.keep) {
         printf("Deleting %s\n\n\n",argv[i]);
         unlink(argv[i]);
      }  // end of if (!o.keep)


   }  // end of else if (o.decrypt)


}  //end of do_file()



int exist(char *fname,int attr)   // returns true if file exists, false if not
{
  // portability problem follows
   _asm{
      mov ah,4eh   ; search for first match
      mov cx,attr  ;either 0 or 1eh .. norm files or all files
      mov dx,fname
      int 21h
      jc no_file
   }
   return 1;
no_file:
   return 0;

}  // end of exist()



void fetch_c_password()
{
   extern struct options o;
   char *pc;
   int i;

   if (o.c_pass_fetched)
      return;
   if (  (pc=getenv("MPGPPASS")) != NULL) {
      strncpy(o.pass1,pc,255);
      if (strchr(o.pass1,0x22) != NULL) {
         printf("\nIllegal character in passphrase... Aborting\n\n");
         exit (1);
      }
      strcat(o.pass1,"\x22");
      o.c_pass_fetched=1;
      return;
   }

   if (o.decrypt) {
      printf("\nEnter Passphrase: ");

      pc=o.pass1;
      for (i=0;i<257;i++) {
         if ( ((int)*pc=getch()) != 0x0d )
            pc++;
         else {
            *pc=0;
            break;
         }
      }
      *pc=0;

      printf("\n\n");

      if (o.pass1[0]==0) {
         printf("\nEmpty passphrase... Aborting\n\n");
         exit(1);
      }

      if (strchr(o.pass1,0x22) != NULL) {

         printf("\nIllegal character in passphrase... Aborting\n\n");
         exit (1);
      }
      strcat(o.pass1,"\x22");
      o.c_pass_fetched=1;
      return;
   }  // end of if (o.decrypt)

   if (o.encrypt)  {
      printf("\nEnter   Passphrase: ");

      pc=o.pass1;
      for (i=0;i<257;i++) {
         if ( ((int)*pc=getch()) != 0x0d )
            pc++;
         else {
            *pc=0;
            break;
         }
         *pc=0;
      }

      printf("\nReenter Passphrase: ");

      pc=o.pass2;
      for (i=0;i<257;i++)
      if (((int)*pc = getch()) != 0x0d )
         pc++;
      else {
         *pc=0;
         break;
      }
      *pc=0;

      printf("\n\n");

      if (strcmp(o.pass1,o.pass2) == 0) {

         if (o.pass1[0]==0) {
            printf("\nEmpty passphrase... Aborting\n\n");
            exit (1);
         }

         pc=o.pass2;
         for (i=0;i< sizeof o.pass2;i++)
            *(pc++)=0;


         if (strchr(o.pass1,0x22) != NULL) {

            printf("\nIllegal character in passphrase... Aborting\n\n");
            exit (1);
         }

         strcat(o.pass1,"\x22");
         o.c_pass_fetched=1;
         return;
      }  // end of if (they compare OK)

      else {
         printf("\nPassphrases did not match.. Aborting...\n");
         exit (1);
      }

   }  // end of if(o.encrypt)

}  // end of fetch_c_password()



void fetch_p_password(void)
{
   extern struct options o;
   char s[270];
   char *pc;
   int i;

   if (o.p_pass_fetched)
      return;

   printf("\nEnter Passphrase to unlock your RSA secret key: ");

   pc=o.pub_pass;
   for (i=0;i<257;i++) {
      if ( ((int)*pc=getch()) != 0x0d )
         pc++;
      else {
         *pc=0;
         break;
      }
   }
   *pc=0;

   printf("\n\n");

   if (o.pub_pass[0]==0) {
      printf("\nEmpty passphrase... Aborting\n\n");
      exit(1);
   }

   if (strchr(o.pub_pass,0x22) != NULL) {
      strcpy(s,"PGPPASS=");
      strncat(s,o.pub_pass,256);
      if (putenv(s) != 0) {
         printf("\nIllegal character in passphrase... Aborting\n\n");
         exit (1);
      }
      else {
         o.p_pass_fetched =1;
         o.pgppass_set=1;
         o.zparm2[0]=0;
      }
   }
   else {
      strcat(o.pub_pass,"\x22");
      o.p_pass_fetched=1;
      o.pgppass_set=0;
   }

}  // end of fetch p_password()



void lead_zero(char *s,unsigned int cnt)
{
   char tmp[40];

   while (strlen(s) <cnt) {
      strcpy(tmp,s);
      strcpy(s,"0");
      strcat(s,tmp);
   }

}  // end of lead_zero()



void cleanup(void)
{
   extern struct options o;
   int i;
   char *pc;
   char tmp[512];

   if (o.table)
      free(o.table);

   pc=getenv("PGPPASS");
   if (pc != NULL) {
      i=strlen(pc);
      strcpy(tmp,"PGPPASS=");
      pc=tmp+8;
      while (i--)
         *(pc++) = ' ';
      *pc=0;
      putenv(tmp);
      tmp[8]=0;
      putenv(tmp);


   }

   pc = getenv("MPGPPASS");
   if (pc != NULL){
      i=strlen(pc);
      strcpy(tmp,"MPGPPASS=");
      pc=tmp+9;
      while (i--)
         *(pc++) = ' ';
      *pc=0;
      putenv(tmp);
      tmp[9]=0;
      putenv(tmp);
   }

   pc = (char *) &o;
   for (i=0;i<sizeof(o);i++)
      *(pc++)=0;
   for (i=0;i< sizeof(tmp);i++)
      tmp[i]=0;
}  // end of cleanup()


void no_exec(void)
{
   switch (errno) {

      case ENOENT:
         printf("PGP.EXE not found.. Aborting\n");
         break;
      case ENOEXEC:
         printf("PGP.EXE not executable.. Aborting\n");
         break;
      case ENOMEM:
         printf("Not enough memory to run PGP.EXE.. Aborting\n");
         break;
      default:
         printf("Undefined error executing PGP.EXE.. errno %i Aborting\n",errno);
         break;
   } //end of switch(errno)

   exit(errno);

}  // end of no_exec();




int   is_public(char *fname)   // returns 1 if file appears to be encrypted
{                              // with a public key, 0 if not
   FILE *f;
   unsigned char byte_one;


   if ( (f=fopen(fname,"rb")) == NULL)
      return 0;

   if (fread(&byte_one,(size_t) 1, (size_t) 1,f) !=1) {
      fclose(f);
      return 0;
   }
   fclose(f);

   /* 1x00 01xx  <--  1st byte of files encrypted with a public key */
   byte_one &=0xbc;       // bc == 1011 1100
   if (byte_one == 0x84)  // 84 == 1000 0100
      return 1;
   else
      return 0;

}  // end of is_public()



void check_files(int argc,char *argv[])
{
   extern struct options o;
   char drive[_MAX_DRIVE], dir[_MAX_DIR];
   char fname[_MAX_FNAME], ext[_MAX_EXT];
   int i;
   int rsa,idea;  //set these to 1 if such a file detected
   int rpass_fetched,ipass_fetched;

   rpass_fetched=ipass_fetched=0;
   rsa=idea=0;
   o.table=malloc( (size_t) argc);
   if (o.table==NULL) {
      printf("Memory Allocation Error... Aborting\n");
      exit(1);
   }
   memset(o.table,0,(size_t) argc);

   for (i=2;i<argc;i++){
      _splitpath( argv[i], drive, dir, fname, ext );
      if (stricmp(".PGP",ext) != 0) {
         o.table[i]=1;  // not a .pgp file
         continue;
      }
      if (!exist(argv[i],NORM_FILE)) {
         o.table[i]=1;  // not a .pgp file
         continue;
      }
      if (is_public(argv[i]))  {
         o.table[i]=3;     // RSA encrypted
         rsa=1;
         if (!rpass_fetched && (ipass_fetched || i>32) ) {
            fetch_p_password();
            rpass_fetched=1;
         }


      }
      else  {
         o.table[i]=2;     // IDEA encrypted
         idea=1;
         if (!ipass_fetched) {
            fetch_c_password();
            ipass_fetched=1;
         }
      }
   }  // end of for loop

   if(idea && !ipass_fetched)
      fetch_c_password();
   if(rsa && !rpass_fetched)
      fetch_p_password();

   o.files_checked=1;

}  // end of check_files()



void fetch_keyid(void)
{
   extern struct options o;
   char *pc;
   int i;

   if (!o.public)
      return;     // this is only for RSA encryption (-ep option)
   if (o.key_id[0])
      return;     // already fetched...

   printf("\nEnter recipient's KeyId: ");

   pc=o.key_id;
   for (i=0;i<118;i++) {
      if ( ((int)*pc=getch()) != 0x0d ) {
         if (*pc == 8) { //BackSpace
            *pc=0;
            i-=2;
            if (i<0)
                  i=-1;
            if (pc>o.key_id) {
               putchar(8);
               putchar(' ');
               putchar(8);
               pc--;
            }
         }  // end of if backspace
         else {
            putchar( (int) *pc);
            pc++;
         } // end of else (if ! BS)
      } // end of if ! 0x0d
      else {
         *pc=0;
         break;
      }  // end of else (if 0x0d)
   }  // end of for loop
   *pc=0;

   printf("\n\n");

} // end of fetch_keyid


unsigned long rand1;
unsigned long rand2;

unsigned int getrand(void)
{
   unsigned long tmp;

   rand1*=8121;
   rand1+=28411;
   rand1%=134456L;

   rand2*=4096;
   rand2+=150889L;
   rand2%=714025L;

   tmp= (rand1>>1) ^ (rand2>>2);
   tmp &=0xffff;
   return (unsigned int) tmp;

}  // end of getrand()


void (_interrupt _far *oldtimer)( void );   // declare pointer to a function
int timer_flag;

void _interrupt _far newtimer()
{
        timer_flag=1;
        _chain_intr( oldtimer );

}  // end of newtimer()


void randseed(int method)   // method: 0=simple 1=use timer tic interrupt
{
   unsigned long tmp1[256];
   unsigned long tmp2[256];
   int i;
   int *pi;
   unsigned long *pl1,*pl2;

   rand1-=(unsigned long)  time(NULL);
   rand2+=(unsigned long)  time(NULL);

   if (method !=0) {
      rand1 &=0x0000ffffL; // mask off msw of time
      rand2 |=0xffff0000L;
   }
   for(i=0;i<256;i++) {  // entropy in garbage?
      rand1^=tmp1[i];
      rand2^=tmp2[i];
   }

   oldtimer=_dos_getvect(0x1c);

   timer_flag=0;
   pi= &timer_flag;

   pl1= &rand1;
   pl2= &rand2;

   if (method != 0) {
      _dos_setvect( 0x1c, newtimer );

      _asm {

         mov bx,pi
         xor ax,ax

      agn:
         inc cx
         add dx,251     ; a prime number

         cmp [bx],ax    ; loop till newtimer() sets timer_flag
         jz agn

         mov bx,pl1
         add word ptr [bx],cx
         add bx,2
         xor word ptr [bx],dx

         mov bx,pl2
         sub word ptr [bx], dx
         add bx,2
         xor word ptr [bx], cx

      }  // end of _asm



      _dos_setvect( 0x1c, oldtimer );

   }  // end of if (method !=0)

}  // end of randseed()



// end of MPGP.C
