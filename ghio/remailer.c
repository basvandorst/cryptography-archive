#define DIR "/home/remailer"
#define ANONFROM "From: nobody@foo.com (Anonymous)\n"
#define REMAILERFROM "From: remailer@foo.com (Foo Remailer)\n"
#define REMAILERADDRESS "remailer@foo.com"
#define RETURN "remailer@foo.com"
#define DISCLAIMER "Comments: Please report misuse of this automated remailing service to <remailer-admin@foo.com>\n"
#define NONANONDISC "Comments: This message was forwarded by an automated remailing service.  No attempt was made to verify the sender's identity.  Please report misuse to <complaints@site>\n"
#define SPOOL "/var/spool/mail/remailer"
#define SPAM_THRESHOLD 25
#define WAIT_SEC 30
#define DEFAULT_LATENCY 3600

#define PGP "/usr/local/bin/pgp"
#define PGPPASS "password"
#define PGPPATH DIR
#define INEWS "/usr/lib/news/inews"
#define NNTPSERVER "127.0.0.1"
#define LS "/bin/ls"
#define SENDMAIL "/usr/lib/sendmail"

#define BLOCKFROM "source.block"
#define BLOCKTO "dest.block"
#define INQUEUE "in.queue"
#define OUTQUEUE "out.queue"
#define TEMPDIR "temp"
#define HELPFILE "remailer-help"
#define STATSDATA "statsdata"

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h> /* some os need this one also */
/*File io stuff:*/
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


FILE *infile;
FILE *outfile;
FILE *tempfile;
FILE *file2;
char from[256]="";
char from_address[256]="";
char cutmarks[256]="";
int anon_flag=0;
int help_flag=0;
int stat_flag=0;
int pgp_flag=0;
char replykey[80]="";
char idbuf[17];
int idcount=0;
struct timeval tp;
unsigned long latime;
int blockflag;

void getfrom(char *input){
  int x=0,y=0;
  while(input[x]!=':'){x=x+1;}
  x=x+1;
  while(input[x]<=32){x=x+1;}
  while(input[x]>32){
    from_address[y]=input[x];
    x=x+1;y=y+1;
  }
  from_address[y]=0;
  x=0; /* look for <address> */
  while(input[x]>31&&input[x]!='<'){x=x+1;}
  if(input[x]=='<'){
    y=0;x=x+1;
    while(input[x]>32&&input[x]!='>'){
      from_address[y]=input[x];
      x=x+1;y=y+1;
    }
    from_address[y]=0;
  }
}

void block_addr(char address[],char *file) {
  char input[256];
  int match=0;
  int x,y,z;
  int exclude;

  FILE *killfile;

  chdir(DIR);
  if(killfile=fopen(file,"r")){
    while(fscanf(killfile,"%s",input)>0) {
      if (input[0]!='#'&&input[0]>32) {
        x=0;exclude=0;z=0;
        if (input[0]=='!') {exclude=1;z++;} 
	while(address[x]!=0) {
          y=0;
          while ((address[x+y]|32)==(input[y+z]|32)&&input[y+z]!='*'
              &&input[y+z]!=0&&address[x+y]!=0) {
            y++;
	  }
          if (input[y+z]==0) match=(1^exclude);
          if (input[y+z]=='*') {z=z+y+1;x=x+y;}
          else x++;
        }
      }
    }
    fclose(killfile);
  }
  if (match==1) address[0]=0;
}

int search(char str1[],char str2[]) {
  int x=0;
  int y=0;
  int match=0;

  while(str2[x]!=0) {
    y=0;
    while ((str2[x+y]==str1[y]||str2[x+y]==(str1[y]-32))&&str2[x+y]!=0) {
      y++;
      if (str1[y]==0) match=1;
    }
    x++;
  }
  return(match);
}

void scanline(char input[],char resend_address[]) {
  int x,y,z;
  int resend_flag=0;
  int cutmarks_flag=0;
  int post_flag=0;
  int latent_plusflag;
  int latent_randflag;
  int latent_h;
  int latent_m;
  int latent_s;

  /* Pass thru Subject, Content-Type, and In-Reply-To lines */
  if ((input[0]=='S'||input[0]=='s')&&input[1]=='u'&&input[2]=='b') {
     /* if the subject line is blank, drop it */
    if (input[8]!=0&&input[9]!=0) fprintf(outfile,"%s",input);
     /* and handle special case subjects for help and stats */
    if (search("remailer-stat",input)) {
      latime=tp.tv_sec; /* No latency */
      stat_flag=1;
    }
    if (search("remailer-help",input)||search("remailer-info",input)) {
      latime=tp.tv_sec; /* No latency */
      help_flag=1;
    }
  }
  if ((input[0]=='C'||input[0]=='c')&&input[1]=='o'&&input[2]=='n') {
    fprintf(outfile,"%s",input);
  }
  if ((input[0]=='I'||input[0]=='i')&&input[1]=='n'&&input[2]=='-') {
    fprintf(outfile,"%s",input);
  }
  
  /* Save the from line in case non-anonymous posting is requested */
  if ((input[0]=='F'||input[0]=='f')&&input[1]=='r'&&input[2]=='o') {
    getfrom(input);block_addr(from_address,BLOCKFROM);
    if(from_address[0]==0) blockflag=1; /* Source block */
    block_addr(input,BLOCKTO);
    strcpy(from,input);
  }

/* Fuzzy Match headers */
  x=0;
  /* Remail-To? */
  while (input[x]!=0&&(input[x]!=32||x<=2)&&input[x]!=10&&x<256) {
    if (input[x]=='R'||input[x]=='r') {
      while (input[x]!=0&&input[x]!=32&&input[x]!=10&&x<256) {
        if (input[x]=='M'||input[x]=='m') {
          while (input[x]!=0&&input[x]!=10&&x<256) {
            if ((input[x]=='T'||input[x]=='t') &&
              (input[x+1]=='O'||input[x+1]=='o')) {
              while (input[x]!=0&&input[x]!=':'&&input[x]!=32
                     &&input[x]!=10&&x<256) x++;
              if (input[x]==':') {
                resend_flag=1;
                anon_flag=1;
                x=256;
              }
            } else x++;
          }
        } else x++;
      }
    } else x++;
  }

  /* Anon-To? */
  x=0;
  while (input[x]!=0&&(input[x]!=32||x<=2)&&input[x]!=10&&x<256) {
    if (input[x]=='A'||input[x]=='a') { x++;
      if (input[x]=='N'||input[x]=='n') {
        while (input[x]!=0&&input[x]!=10&&x<256) {
          if ((input[x]=='T'||input[x]=='t') &&
              (input[x+1]=='O'||input[x+1]=='o')) {
            while (input[x]!=0&&input[x]!=':'&&input[x]!=32
                   &&input[x]!=10&&x<256) x++;
            if (input[x]==':') {
              resend_flag=1;
              anon_flag=1;
            }
            x=256;
          } else x++;
        }
      }
    } else x++;
  }

  /* Post? */
  x=0;
  while (input[x]!=0&&input[x]!=32&&input[x]!=10&&x<256) {
    if (input[x]=='P'||input[x]=='p') { x++;
      if (input[x]=='O'||input[x]=='o') { x++;
        if (input[x]=='S'||input[x]=='s') {
          post_flag=1;
          /* Post-To ? */
          while (input[x]!=0&&input[x]!=32&&input[x]!=10&&x<256) {
            if (input[x]=='T'||input[x]=='t') { x++;
              if (input[x]=='O'||input[x]=='o') { x++;
                if (input[x]==':') {
                  resend_flag=1;
                }
              }
            } else x++;
          }
          x=256;
        }
      }
    } else x++;
  }

  /* soda.berkeley style Send-To ? */
  x=0;
  while (input[x]!=0&&input[x]!=32&&input[x]!=10&&x<256) {
    if (input[x]=='S'||input[x]=='s') { x++;
      if (input[x]=='E'||input[x]=='e') { x++;
        if (input[x]=='N'||input[x]=='n') { x++;
          if (input[x]=='D'||input[x]=='d') {
            while (input[x]!=0&&input[x]!=32&&input[x]!=10&&x<256) {
              if (input[x]=='T'||input[x]=='t') { x++;
                if (input[x]=='O'||input[x]=='o') { x++;
                  if (input[x]==':') resend_flag=1;
                }
              } else x++;
            }
          }
        }
      }
    } else x++;
  }

/* Check for PGP...   I got a little sloppy here...ohwell*/
  if(input[0]=='E'&&input[1]=='n'&&input[2]=='c'
     &&input[3]=='r'&&input[4]=='y'&&input[5]=='p'
     &&input[6]=='t'&&input[7]=='e'&&input[8]=='d') {
    resend_flag=0;
    pgp_flag=1;
  }
  if(input[0]=='E'&&input[1]=='n'&&input[2]=='c'
     &&input[3]=='r'&&input[4]=='y'&&input[5]=='p'
     &&input[6]=='t'&&input[7]=='-') {
    x=0;y=0;
    while(input[x]!=':'){x=x+1;}
    x=x+1;
    if(input[x]==32){x=x+1;}
    z=x;
    while(input[x]>32){
      replykey[y]=input[x];
      x=x+1;y=y+1;
    }
    replykey[y]=0;
  
  }
  if(input[0]=='C'&&input[1]=='u'&&input[2]=='t') {
    cutmarks_flag=1;
  }

  if(resend_flag){
    x=2;y=0; /* x=2 in case Extropians-style ::Header */
    while(input[x]!=':'){x=x+1;}
    x=x+1;
    while(input[x]<=32){x=x+1;}
    z=x;
    if (post_flag==0) {
      while(input[x]>32){
        resend_address[y]=input[x];
        x=x+1;y=y+1;
      }
      resend_address[y]=0;
      x=0; /* look for <address> */
      while(input[x]>31&&input[x]!='<'){x=x+1;}
      if(input[x]=='<'){
        y=0;x=x+1;
        while(input[x]>32&&input[x]!='>'){
          resend_address[y]=input[x];
          x=x+1;y=y+1;
        }
        resend_address[y]=0;
      }
      /* Print out new To: line */
      fprintf(outfile,"To: ");
      while(input[z]>0){
        fprintf(outfile,"%c",input[z]);
        z=z+1;
      }
      block_addr(resend_address,BLOCKTO);
    }
    if (post_flag) {
      fprintf(outfile,"Newsgroups: ");
      while(input[z]>0){
        fprintf(outfile,"%c",input[z]);
        z=z+1;
      }
      resend_address[0]='p';
      resend_address[1]='o';
      resend_address[2]='s';
      resend_address[3]='t';
      resend_address[4]=0;
      block_addr(input,BLOCKTO);if (input[0]==0) resend_address[0]=0;
    }
  }

  if(cutmarks_flag){
    x=0;y=0;
    while(input[x]!=':'){x=x+1;}
    x=x+1;
    if(input[x]==32){x=x+1;}
    z=x;
    while(input[x]>32){
      cutmarks[y]=input[x];
      x=x+1;y=y+1;
    }
    cutmarks[y]=0;
  }

  if((input[0]|32)=='l'&&(input[1]|32)=='a'&&(input[2]|32)=='t') {
    x=0;
    while(input[x]!=':'){x=x+1;}
    x=x+1;
    if(input[x]==32){x=x+1;}

    latent_plusflag=0;latent_randflag=0;
    latent_h=0;latent_m=0;latent_s=0;

    while((input[x]<'0'||input[x]>'9')&&input[x]>=32) {
      if (input[x]=='+') latent_plusflag=1;
      if ((input[x]=='r')||(input[x]=='R')) latent_randflag=1;
      x++;
    }
    while (input[x]>='0'&&input[x]<='9') {
      latent_h=(latent_h*10)+(input[x]-48);
      x++;
    }
    if(input[x]==':') {
      x++;
      while (input[x]>='0'&&input[x]<='9') {
        latent_m=(latent_m*10)+(input[x]-48);
        x++;
      }
      if(input[x]==':') {
        x++;
        while (input[x]>='0'&&input[x]<='9') {
          latent_s=(latent_s*10)+(input[x]-48);
          x++;
        }
      }
    }
    while(input[x]>=32) {
      if (input[x]=='+') latent_plusflag=1;
      if ((input[x]=='r')||(input[x]=='R')) latent_randflag=1;
      x++;
    }

    latime=(latent_h*3600+latent_m*60+latent_s);

    if(latent_plusflag==0) {
      /* Not Supported - Is this really necessary? */
    }

    if(latent_randflag&&(latime>1)) {
      /* Simple randomizer */
      latime=abs((tp.tv_sec^latime)+tp.tv_usec+(getpid()*latime))%(latime+1);
    }

    latime+=tp.tv_sec;
  }
}

char* genid() { /* Generate ascii id from process id and time with shuffle */
  unsigned long int id1,id2;
  int x=0;
  
  id1=getpid()|(idcount<<16);
  id2=tp.tv_sec;
  idcount++;
  
  for(x=32;x--;){
    id1+=1234567890;
    id1^=0xABCDEF12;
    id1=(id1<<1)|(id1>>31);
    id2^=id1;
    id2+=0x12345678;
    id2^=0x9ABCDEF0;
    id2=(id2<<31)|(id2>>1);
    id1^=id2;
  }
  for(x=0;x<8;x++) {
    idbuf[x]=65+(id1&15);
    id1=id1>>4;
  }
  for(x=8;x<16;x++) {
    idbuf[x]=65+(id2&15);
    id2=id2>>4;
  }
  idbuf[16]=0;
  return(idbuf);
}

/* Re-encrypt messages for use with reply-blocks */
void reencrypt(){
  char input[256];
  int pipefd[2];
  int pipe2fd[2];

  input[255]=0;
  pipe(pipefd);
  pipe(pipe2fd);
  if(!fork()) {
    dup2(pipefd[0],0);
    dup2(pipe2fd[1],1);
    close(pipefd[1]);
    close(pipe2fd[0]);
    chdir(DIR);
    execl(PGP,"pgp","-fcta","+BATCHMODE","+ARMORLINES=0","-z",replykey,(char *)0);
  }
  close(pipefd[0]);close(pipe2fd[1]);
  file2=fdopen(pipefd[1],"w");
  while(fgets(input,255,infile)) {
    fprintf(file2,"%s",input);
  }
  fclose(file2);
  file2=fdopen(pipe2fd[0],"r");
  while(fgets(input,255,file2)) {
    fprintf(outfile,"%s",input);
  }
  fclose(file2);
}

void updatestats(int inccnt,int incpgp,int inclat,int incpost) {
  int m[24];
  int ccm=0;
  int p[24];
  int ccpgp=0;
  int l[24];
  int ccl=0;
  int u[24];
  int ccnews=0;
  char month[24][5];
  int date[24];
  int hour=0;
  int currenthour;
  FILE *datafile;
  int x;
  int y;
  struct tm *timestr;

  timestr=localtime(&(tp.tv_sec));

  if(datafile=fopen(STATSDATA,"r")){
    fscanf(datafile,"%d",&hour);
    fscanf(datafile,"%d %d %d %d",&ccm,&ccpgp,&ccl,&ccnews);
    for(x=0;x<24;x++) {
      fscanf(datafile,"%s %d %d %d %d %d",
             month[x],&date[x],&m[x],&p[x],&l[x],&u[x]); }
    fclose(datafile);
  }else{
    for(x=0;x<24;x++) {
      strcpy(month[x],"---");
      date[x]=0;m[x]=0;p[x]=0;l[x]=0;u[x]=0;
    }
  }
  currenthour=(*timestr).tm_hour;
  x=hour%24;
  while (x!=currenthour) {
    if (x>0) {
      strcpy(month[x],month[x-1]);
      date[x]=date[x-1];
    }else{
      if((*timestr).tm_mon==0) strcpy(month[0],"Jan");
      if((*timestr).tm_mon==1) strcpy(month[0],"Feb");
      if((*timestr).tm_mon==2) strcpy(month[0],"Mar");
      if((*timestr).tm_mon==3) strcpy(month[0],"Apr");
      if((*timestr).tm_mon==4) strcpy(month[0],"May");
      if((*timestr).tm_mon==5) strcpy(month[0],"Jun");
      if((*timestr).tm_mon==6) strcpy(month[0],"Jul");
      if((*timestr).tm_mon==7) strcpy(month[0],"Aug");
      if((*timestr).tm_mon==8) strcpy(month[0],"Sep");
      if((*timestr).tm_mon==9) strcpy(month[0],"Oct");
      if((*timestr).tm_mon==10) strcpy(month[0],"Nov");
      if((*timestr).tm_mon==11) strcpy(month[0],"Dec");
      date[0]=(*timestr).tm_mday;
    }
    m[x]=0;
    p[x]=0;
    l[x]=0;
    u[x]=0;
    x++;if (x>23) x=0;
  }

  if (hour!=currenthour) {
    m[hour]=ccm;
    p[hour]=ccpgp;
    l[hour]=ccl;
    u[hour]=ccnews;
    ccm=0;
    ccpgp=0;
    ccl=0;
    ccnews=0;
  }

  ccm+=inccnt;
  ccpgp+=incpgp;
  ccl+=inclat;
  ccnews+=incpost;

  if(datafile=fopen(STATSDATA,"w")){
    fprintf(datafile,"%d\n",currenthour);
    fprintf(datafile,"%d %d %d %d\n",ccm,ccpgp,ccl,ccnews);
    for(x=0;x<24;x++) {
      fprintf(datafile,"%s %d %d %d %d %d\n",
              month[x],date[x],m[x],p[x],l[x],u[x]);
    }
    fclose(datafile);
  } else fprintf(stderr,"remailer: can't write file %s\n",STATSDATA);
}

void viewstats() {
  int m[24];
  int ccm;
  int p[24];
  int ccpgp;
  int l[24];
  int ccl;
  int u[24];
  int ccnews;
  char month[24][5];
  int date[24];
  int hour;
  int currenthour;
  FILE *datafile;
  int x;
  int y;

  datafile=fopen(STATSDATA,"r");

  fscanf(datafile,"%d",&hour);
  fscanf(datafile,"%d %d %d %d",&ccm,&ccpgp,&ccl,&ccnews);
  for(x=0;x<24;x++) {
    fscanf(datafile,"%s %d %d %d %d %d",
           month[x],&date[x],&m[x],&p[x],&l[x],&u[x]); }
  fclose(datafile);

  fprintf(outfile,"Subject: Re: Remailer Statistics\n");
  fprintf(outfile,"\n");
  fprintf(outfile,"Statistics for last 24 hours from anonymous remailer at\n");
  fprintf(outfile,"e-mail address: %s\n",REMAILERADDRESS);
  fprintf(outfile,"\n");
  fprintf(outfile,
    "Number of messages per hour from %s %d %d:00 to %s %d %d:59\n",
    month[23],date[23],hour,month[0],date[0],(hour+23)%24);
  fprintf(outfile,"\n");
  for(x=0;x<24;x++) {
    fprintf(outfile," %2d:00 (%2d) ",x,m[x]);
    if (m[x]>0) {
      y=0;while((y<m[x])&&(y<66)) {
        fprintf(outfile,"*");
        y++;
      }
      ccm+=m[x];
      ccpgp+=p[x];
      ccl+=l[x];
      ccnews+=u[x];
    }
    fprintf(outfile,"\n");
  }
  fprintf(outfile,"\n");
  fprintf(outfile,"Total messages remailed in last 24 hours: %d\n",ccm);
#ifdef PGP
  fprintf(outfile,"Number of messages encrypted with PGP: %d\n",ccpgp);
#endif
  fprintf(outfile,"Number of messages queued with latency: %d\n",ccl);
#ifdef INEWS
  fprintf(outfile,"Number of posts to usenet: %d\n",ccnews);
#endif
}

void main(int argc,char *argv[]) {
  char input[256];
  char resend_address[256]="";
  int stop;
  int x;
  pid_t mypid,otherpid;
  char filename[256];
  char filename2[256];
  int pipefd[2];
  int pipe2fd[2];
  char envstr[256];

  gettimeofday(&tp,0);
  if(chdir(DIR))
   {fprintf(stderr,"remailer: Fatal Error: can't chdir to %s\n",DIR);exit(1);}
  mkdir(INQUEUE,0700); /* Create it if it doesn't exist */
  mkdir(TEMPDIR,0700); /* And the temp dir */  
  /* Create a temporary file in TEMPDIR */
  strcpy(filename,TEMPDIR);
  strcat(filename,"/");
  strcat(filename,genid());
  if((outfile=fopen(filename,"w"))==0){
    fprintf(stderr,"remailer: Fatal Error: can't create temporary file\n");
    exit(1);
  }
  while(fgets(input,255,stdin)) fprintf(outfile,"%s",input);
  fclose(outfile);
  strcpy(filename2,INQUEUE);
  strcat(filename2,"/");
  strcat(filename2,genid());
  if(rename(filename,filename2)){
    fprintf(stderr,"remailer: Fatal Error: can't move %s to %s\n",
      filename,filename2);
    exit(1);
  }

  mypid=getpid();otherpid=0;
  if(infile=fopen("pid","rb")) {
    fread(&otherpid,sizeof(pid_t),1,infile);
    fclose(infile);
  }
  /* If there is a remailer process already running, leave the message
     in in.queue and exit */
  if(otherpid) {
    if(kill(otherpid,SIGCONT)==0) exit(0); }
  if(outfile=fopen("pid","wb")) {
    fwrite(&mypid,sizeof(pid_t),1,outfile);
    fclose(outfile);
  } else fprintf(stderr,"remailer: can't write pid file\n");

in_loop:

  /* Open an input file from in.queue */
  chdir(DIR);chdir(INQUEUE);
  pipe(pipefd);
  filename[0]=0;
  if(!fork()) {
    dup2(pipefd[1],1);
    close(pipefd[0]);
    execl(LS,"ls","-1",(char *)0);
  }
  x=0;close(pipefd[1]);
  infile=fdopen(pipefd[0],"r");
  while(fgets(filename,256,infile)) x++;
  fclose(infile);
  if(filename[0]==0) exit(0);
#ifdef SPAM_THRESHOLD
  if(x>SPAM_THRESHOLD) exit(0);
#endif
  for(x=0;filename[x]>32;x++){} filename[x]=0;
  if(!(infile=fopen(filename,"r"))){}
  
  /* Open the output file */
  chdir(DIR);
  mkdir(OUTQUEUE,0700); /* Create it if it doesn't exist */
  if(chdir(OUTQUEUE))
    {fprintf(stderr,"remailer: Error - can't chdir to %s\n",OUTQUEUE);exit(1);}
  if(!(outfile=fopen(filename,"w")))
    {fprintf(stderr,"remailer: can't write output file, message left in %s\n",INQUEUE);exit(1);}

  /* Create blank space for fields in output file */
  latime=0;resend_address[0]=0;resend_address[255]=0;
  fwrite(&latime,sizeof(long),1,outfile);
  fwrite(resend_address,256,1,outfile);

  /* Initialize latency time & misc */
  latime=tp.tv_sec;
  from[0]=0;cutmarks[0]=0;replykey[0]=0;
  anon_flag=0;help_flag=0;stat_flag=0;pgp_flag=0;blockflag=0;

#ifdef DEFAULT_LATENCY
  /* Randomly reorder messages if DEFAULT_LATENCY is set */
  if(DEFAULT_LATENCY>1) {
    latime=tp.tv_sec+abs(tp.tv_sec+tp.tv_usec+getpid())%(DEFAULT_LATENCY+1);
  }
#endif

  /* Scan headers */
  fgets(input,255,infile);
  while(input[0]!=10) {
    scanline(input,resend_address);
    input[0]=10;input[1]=0;
    fgets(input,255,infile);
  }
  fgets(input,255,infile); /* end of headers, skip a line */
  
  /* if first line is blank, skip it and look for a :: on the next line */
  if(resend_address[0]==0&&input[0]<32) fgets(input,255,infile);
  /* Also skip "blank" lines with a space in them: */
  if(resend_address[0]==0){
    for(x=0;(input[x]<=32)&&(input[x]);x++){}
    if(input[x]==0) fgets(input,255,infile);
  }

  /* Scan :: headers, if applicable */
  if(input[0]==':'&&input[1]==':') {
    while(input[0]!=10) {
      scanline(input,resend_address);
      input[0]=10;input[1]=0;
      fgets(input,255,infile);
    }
    fgets(input,255,infile);
  }

  /* or scan for headers anyway for idiots who forget the double colon */
  if(resend_address[0]==0) {
    scanline(input,resend_address);
    if(resend_address[0]!=0) {
      fgets(input,255,infile);
      while(input[0]!=10) {
        scanline(input,resend_address);
        input[0]=10;input[1]=0;
        fgets(input,255,infile);
      }
    }
    fgets(input,255,infile);
  }

  /* Exec PGP? */
  if (pgp_flag) {
    fclose(outfile);
    chdir(DIR);chdir(OUTQUEUE);
    unlink(filename);
    pipe(pipefd);
    pipe(pipe2fd);
    if(!fork()) {
      dup2(pipefd[0],0);
      dup2(pipe2fd[1],1);
      close(pipefd[1]);
      close(pipe2fd[0]);
      chdir(DIR);
#ifdef PGPPATH
      strcpy(envstr,"PGPPATH=");
      strcat(envstr,PGPPATH);
      putenv(envstr);
#endif
      execl(PGP,"pgp","-f","-z",PGPPASS,(char *)0);
    }
    close(pipefd[0]);close(pipe2fd[1]);
    fseek(infile,0,0);
    outfile=fdopen(pipefd[1],"w");
    while((fgets(input,255,infile)>0)
         &&(strcmp(input,"-----BEGIN PGP MESSAGE-----\n")!=0)) {}
    fprintf(outfile,"%s",input);
    while(fgets(input,255,infile)
         &&(strcmp(input,"-----END PGP MESSAGE-----\n")!=0)) {
      fprintf(outfile,"%s",input);
    }
    fprintf(outfile,"%s",input);
    fclose(outfile);
    file2=fdopen(pipe2fd[0],"r");
    chdir(DIR);chdir(INQUEUE);
    outfile=fopen(genid(),"w");
    fprintf(outfile,"\n");
    while(fgets(input,255,file2)) {
      fprintf(outfile,"%s",input);
    }
    fclose(file2);
    /* Append rest of message to decrypted reply-block */
    while(fgets(input,255,infile)) {
      fprintf(outfile,"%s",input);
    }
    fclose(infile);fclose(outfile);
    unlink(filename);/* Remove the original message from in.queue */
    chdir(DIR);
    updatestats(0,1,0,0);
    goto in_loop;
  }

  if (from[0]==0) anon_flag=1;

  if (anon_flag) {
    fprintf(outfile,ANONFROM);
    fprintf(outfile,DISCLAIMER);
  }else{
    fprintf(outfile,"%s",from);
    fprintf(outfile,NONANONDISC);
  }

  /* Paste in ## headers if present */
  if(input[0]=='#'&&input[1]=='#') {
   /* Kill Reply-To lines with blocked addresses to prevent
      mailbombs via alt.test */
    while(fgets(input,255,infile)>0&&input[0]>31) {
      if ((input[0]=='R'||input[0]=='r')&&input[1]=='e'&&input[2]=='p') {
        block_addr(input,BLOCKTO);if (input[0]!=0) fprintf(outfile,"%s",input);
      /* Block ## pasted Newsgroups: */
      }else if((input[0]|32=='n')&&input[1]=='e'&&input[2]=='w'&&input[3]=='s')
      {
        block_addr(input,BLOCKTO);if (input[0]!=0) fprintf(outfile,"%s",input);
      }else fprintf(outfile,"%s",input);
    }
    fprintf(outfile,"\n");
  }else{
    fprintf(outfile,"\n%s",input);
    if(replykey[0]>0&&input[0]=='*'&&input[1]=='*') {
      reencrypt();
    }
  }

  /* Copy message */
  stop=0;
  while(fgets(input,255,infile)>0&&(!stop)) {
    if (cutmarks[0]!=0) {
      x=0;
      while(cutmarks[x]==input[x]&&input[x]!=0&&cutmarks[x]!=0) {
        x++;
      }
      if (cutmarks[x]==0) stop=1;
    }
    if (!stop) fprintf(outfile,"%s",input);
    if(replykey[0]>0&&input[0]=='*'&&input[1]=='*') {
      reencrypt();
    }
  }
  
  /* If help or stats were requested, set destination address to reply
     to sender */
  if((resend_address[0]==0)&&(help_flag||stat_flag)){
    strcpy(resend_address,from_address);
  } else {help_flag=0;stat_flag=0;}

  /* Save time and destination address in binary data table at
     begining of file */
  if (blockflag) resend_address[0]=0;
  fseek(outfile,0,0);
  fwrite(&latime,sizeof(long),1,outfile);
  fwrite(resend_address,256,1,outfile);
  if(help_flag||stat_flag){
    chdir(DIR);
    fprintf(outfile,"%s",REMAILERFROM);
    if(help_flag) {
      if(file2=fopen(HELPFILE,"r")){
        while(fgets(input,255,file2)){
          for(x=0;input[x];x++){
            if(input[x]=='['&&input[x+1]=='a'&&input[x+2]=='d'
               &&input[x+3]=='d'&&input[x+4]=='r'&&input[x+5]==']')
            {
              fprintf(outfile,"%s",REMAILERADDRESS);x=x+5;
            }
            else
            {
              fprintf(outfile,"%c",input[x]);
            }   
          }
        }
        fclose(file2);
      } else resend_address[0]=0;
    }
    if(stat_flag) {viewstats();}
  }
  fclose(outfile);

  chdir(DIR);chdir(INQUEUE);

  /* Second message?  Put message following cutmarks into inqueue */
  if (stop==1&&input[0]==':'&&input[1]==':') {
    outfile=fopen(genid(),"w");
    fprintf(outfile,"\n::\n");
    while(fgets(input,255,infile)>0) {
      fprintf(outfile,"%s",input);
    }
    fclose(outfile);
  }

  /* Write non-remailer messages into operator's mailbox */
  if (resend_address[0]==0&&from[0]!=0){
    fseek(infile,0,0);
    outfile=fopen(SPOOL,"a");
    while(fgets(input,255,infile)) {
      fprintf(outfile,"%s",input);
    }
    fclose(infile);
    fprintf(outfile,"\n");
    fclose(outfile);
    unlink(filename);
    chdir(DIR);chdir(OUTQUEUE);
    unlink(filename);
  }else{
    fclose(infile);
    unlink(filename);
    if(strcmp(resend_address,"null")==0
     ||strcmp(resend_address,"/dev/null")==0) resend_address[0]=0;
    if(resend_address[0]==0){ /* drop empty messages */
      chdir(DIR);chdir(OUTQUEUE);
      unlink(filename);
    }else{
      chdir(DIR);
      if((latime-tp.tv_sec)>2) updatestats(0,0,1,0);
      updatestats(1,0,0,0); /* Add one remailed message to stats */
    }
  }

  /* Deliver messages in out.queue */
  gettimeofday(&tp,0);
  chdir(DIR);chdir(OUTQUEUE);
  pipe(pipefd);
  filename[0]=0;
  if(!fork()) {
    dup2(pipefd[1],1);
    close(pipefd[0]);
    execl(LS,"ls","-1",(char *)0);
  }
  x=0;close(pipefd[1]);
  file2=fdopen(pipefd[0],"r");
  while(fgets(filename,256,file2)&&filename[0]!=0) {
    for(x=0;filename[x]>32;x++){} filename[x]=0;
    if(infile=fopen(filename,"r")){
      fread(&latime,sizeof(long),1,infile);
      fread(resend_address,256,1,infile);
      if (latime<=tp.tv_sec) {
        pipe(pipe2fd);/*pipe(pipe3fd);*/
        if(!fork()) {
          /*Child*/
          dup2(pipe2fd[0],0);close(pipe2fd[1]);
          /*dup2(pipe3fd[1],1);close(pipe3fd[0]);*/
          if(strcmp(resend_address,"post")){
            execl(SENDMAIL,SENDMAIL,
#ifdef RETURN
                                    "-f",RETURN,
#endif
                                                resend_address,(char *)0);
            exit(0);
          }else{
#ifdef INEWS
#ifdef NNTPSERVER
            strcpy(envstr,"NNTPSERVER=");
            strcat(envstr,NNTPSERVER);
            putenv(envstr);
#endif
            execl(INEWS,"inews","-h",(char *)0);
#endif
            exit(0);
          }
        }else{
          /*Parent*/
          close(pipe2fd[0]);/*close(pipe3fd[1]);*/
          outfile=fdopen(pipe2fd[1],"w");
          if(strcmp(resend_address,"post")){
            /* We are talking to sendmail */
            while(fgets(input,255,infile)>0) {
              fprintf(outfile,"%s",input);
            }
            fclose(outfile);
            /* At this point, it's a safe bet that sendmail will deliver
               the message, so the remailer can delete its copy.  If
               sendmail execution had failed for some reason, this
               process would have been killed by a SIGPIPE */
            unlink(filename);
          }else{
            /* We are talking to inews */
#ifdef INEWS
            while(fgets(input,255,infile)>0) {
              fprintf(outfile,"%s",input);
            }
            /* There should be a way to analyze the response from inews
               and requeue messages that could not be posted due to server
               failure.  Now, the messages just get deleted :( */
            unlink(filename);
#else
            /* If posting is not allowed, delete the failed message */
            unlink(filename);
#endif
          }
        }
#ifdef WAIT_SEC
        sleep(WAIT_SEC);
#endif
        gettimeofday(&tp,0);
      }
      fclose(infile);
    }
  }
  fclose(file2);

  goto in_loop;
}
