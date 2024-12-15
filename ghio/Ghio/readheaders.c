#include <stdio.h>

#define KILLFILE "/home/remailer/remail/blocked.addresses"

/* Args: output cutoff-remainder latent-time key toencrypt */


FILE *outfile;

char from[256]="";
char cutmarks[256]="";
int anon_flag=0;
char replykey[80]="";

void scanline(char input[],char resend_address[],char timefile[]) {
  int x,y,z;
  int resend_flag=0;
  int cutmarks_flag=0;
  int post_flag=0;
  FILE *file2;

  /* Pass thru Subject, Content-Type, and In-Reply-To lines */
  if ((input[0]=='S'||input[0]=='s')&&input[1]=='u'&&input[2]=='b') {
    if (input[8]!=0&&input[9]!=0) fprintf(outfile,"%s",input);
    if (search("remailer-stat",input)) {
      resend_address[0]='s';resend_address[1]='t';resend_address[2]='a';
      resend_address[3]='t';resend_address[4]=0;
    }
    if (search("remailer-help",input)||search("remailer-info",input)) {
      resend_address[0]='h';resend_address[1]='e';resend_address[2]='l';
      resend_address[3]='p';resend_address[4]=0;
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
    block_addr(input);
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
    resend_address[0]='p';
    resend_address[1]='g';
    resend_address[2]='p';
    resend_address[3]=0;
    resend_flag=0;
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
      block_addr(resend_address);
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
      block_addr(input);if (input[0]==0) resend_address[0]=0;
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

  if(input[0]=='L'&&input[1]=='a'&&input[2]=='t') {
    file2=fopen(timefile,"w");
    x=0;
    while(input[x]!=':'){x=x+1;}
    x=x+1;
    if(input[x]==32){x=x+1;}
    while(input[x]>=32){
      fprintf(file2,"%c",input[x]);
      x=x+1;
    }
    fprintf(file2,"\n");
    fclose(file2);
  }

}

void main(int argc,char *argv[]) {
  char input[256];
  char resend_address[256]="";
  int stop;
  int x;

  outfile=fopen(argv[1],"w");

  fgets(input,255,stdin);
  /* Scan headers */
  while(input[0]!=10) {
    scanline(input,resend_address,argv[3]);
    input[0]=10;input[1]=0;
    fgets(input,255,stdin);
  }
  fgets(input,255,stdin);
  /* if first line is blank, skip it and look for a :: on the next line */
  if(resend_address[0]==0&&input[0]<32) fgets(input,255,stdin);

  /* And :: headers, if applicable */
  if(input[0]==':'&&input[1]==':') {
    while(input[0]!=10) {
      scanline(input,resend_address,argv[3]);
      input[0]=10;input[1]=0;
      fgets(input,255,stdin);
    }
    fgets(input,255,stdin);
  }

  /* or scan for headers anyway for idiots who forget the double colon */
  if(resend_address[0]==0) {
    scanline(input,resend_address,argv[3]);
    if(resend_address[0]!=0) {
      fgets(input,255,stdin);
      while(input[0]!=10) {
        scanline(input,resend_address,argv[3]);
        input[0]=10;input[1]=0;
        fgets(input,255,stdin);
      }
    }
    fgets(input,255,stdin);
  }

  if (from[0]==0) anon_flag=1;

  if (anon_flag) {
    fprintf(outfile,"From: nobody (Anonymous)\n");
    fprintf(outfile,"Comments: This message did not originate from the ");
    fprintf(outfile,"above address.  It was automatically remailed by an ");
    fprintf(outfile,"anonymous mail service.  Please ");
    fprintf(outfile,"report inappropriate use to <complaints@your.site>\n");
  }else{
    fprintf(outfile,"%s",from);
    fprintf(outfile,"Comments: This message was forwarded by an automated ");
    fprintf(outfile,"remailing service.  ");
    fprintf(outfile,"No attempt was made to verify the sender's identity.  ");
    fprintf(outfile,"Please report inappropriate use to <complaints@site>\n");
  }

  /* Paste in ## headers if present */
  if(input[0]=='#'&&input[1]=='#') {
   /* Kill Reply-To lines with blocked addresses to prevent
      mailbombs via alt.test */
    while(fgets(input,255,stdin)>0&&input[0]>31) {
      if ((input[0]=='R'||input[0]=='r')&&input[1]=='e'&&input[2]=='p') {
        block_addr(input);if (input[0]!=0) fprintf(outfile,"%s",input);
      }else { fprintf(outfile,"%s",input); }
    }
    fprintf(outfile,"\n");
  }else{
    fprintf(outfile,"\n%s",input);
    if(replykey[0]>0&&input[0]=='*'&&input[1]=='*') {
      fclose(outfile);
      outfile=fopen(argv[4],"w");
      fprintf(outfile,"%s",replykey);
      fclose(outfile);
      outfile=fopen(argv[5],"w");
      replykey[0]=0;
    }
  }

  /* Copy message */
  stop=0;
  while(fgets(input,255,stdin)>0&&(!stop)) {
    if (cutmarks[0]!=0) {
      x=0;
      while(cutmarks[x]==input[x]&&input[x]!=0&&cutmarks[x]!=0) {
        x++;
      }
      if (cutmarks[x]==0) stop=1;
    }
    if (!stop) fprintf(outfile,"%s",input);
    if(replykey[0]>0&&input[0]=='*'&&input[1]=='*') {
      fclose(outfile);
      outfile=fopen(argv[4],"w");
      fprintf(outfile,"%s",replykey);
      fclose(outfile);
      outfile=fopen(argv[5],"w");
      replykey[0]=0;
    }
  }
  fclose(outfile);

  if (stop==1&&input[0]==':'&&input[1]==':') {
    outfile=fopen(argv[2],"w");
    fprintf(outfile,"\n::\n");
    while(fgets(input,255,stdin)>0) {
      fprintf(outfile,"%s",input);
    }
    fclose(outfile);
  }
  printf("%s",resend_address);
}


void block_addr(char address[]) {
  char input[256];
  int match=0;
  int x,y;

  FILE *killfile;

  killfile=fopen(KILLFILE,"r");
  while(fscanf(killfile,"%s",input)>0) {
    x=0;
    while(address[x]!=0) {
      y=0;
      while ((address[x+y]==input[y]||address[x+y]==(input[y]-32))
            &&address[x+y]!=0) {
        y++;
        if (input[y]==0) match=1;
      }
    x++;
    }
  }
  fclose(killfile);
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
