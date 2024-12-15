#include <stdio.h>

void main(int argc,char *argv[]) {
  /* month day hour [+plv] */

  int m[24];
  int ccm;
  int p[24];
  int ccpgp;
  int l[24];
  int ccl;
  char month[24][5];
  int date[24];
  int hour;
  int currenthour;
  FILE *datafile;
  int x;
  int y;

  datafile=fopen(argv[5],"r");

  fscanf(datafile,"%d",&hour);
  fscanf(datafile,"%d %d %d",&ccm,&ccpgp,&ccl);
  for(x=0;x<24;x++) {
    fscanf(datafile,"%s %d %d %d %d",month[x],&date[x],&m[x],&p[x],&l[x]); }

  fclose(datafile);

  currenthour=argv[3][0]-48;
  if (argv[3][1]>0) currenthour=(currenthour*10)+(argv[3][1]-48);

  x=hour;
  while (x!=currenthour) {
    if (x>0) {
      strcpy(month[x],month[x-1]);
      date[x]=date[x-1];
    }else{
      strcpy(month[0],argv[1]);
      date[0]=argv[2][0]-48;
      if (argv[2][1]>0) date[0]=(date[0]*10)+(argv[2][1]-48);
    }
    m[x]=0;
    p[x]=0;
    l[x]=0;
    x++;if (x>23) x=0;
  }

  if (hour!=currenthour) {
    m[hour]=ccm;
    p[hour]=ccpgp;
    l[hour]=ccl;
    ccm=0;
    ccpgp=0;
    ccl=0;
  }

  if (argv[4][0]=='+') ccm++;
  if (argv[4][0]=='p') ccpgp++;
  if (argv[4][0]=='l') ccl++;

  datafile=fopen(argv[5],"w");

  fprintf(datafile,"%d\n",currenthour);
  fprintf(datafile,"%d %d %d\n",ccm,ccpgp,ccl);
  for(x=0;x<24;x++) {
    fprintf(datafile,"%s %d %d %d %d\n",month[x],date[x],m[x],p[x],l[x]);
  }

  fclose(datafile);

  if (argv[4][0]=='v') {
    printf("From: remailer@unknown");
    printf(" (via automated remailer software)\n");
    printf("Subject: Re: Remailer Statistics\n");
    printf("\n");
    printf("Statistics for last 24 hours from anonymous remailer at\n");
    printf("e-mail address: remailer@unknown\n");
    printf("\n");
    printf("Number of messages per hour from %s %d %d:00 to %s %d %d:59\n",
     month[23],date[23],currenthour,month[0],date[0],(currenthour+23)%24);
    printf("\n");
    /*ccm=0;ccpgp=0;ccl=0;*/
    for(x=0;x<24;x++) {
      printf(" %2d:00 (%2d) ",x,m[x]);
      if (m[x]>0) {
        y=0;while((y<m[x])&&(y<67)) {
          printf("*");
          y++;
        }
        ccm+=m[x];
        ccpgp+=p[x];
        ccl+=l[x];
      }
      printf("\n");
    }
    printf("\n");
    printf("Total messages remailed in last 24 hours: %d\n",ccm);
    printf("Number of messages encrypted with PGP: %d\n",ccpgp);
    printf("Number of messages queued with latency: %d\n",ccl);
  }
}
