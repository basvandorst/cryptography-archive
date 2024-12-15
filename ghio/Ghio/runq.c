#include <stdio.h>

/* Take list of files from arg1 file and create shell script in arg2 file
   to process the messages.  Takes crrent date/time from stdin */

void main(int argc,char *argv[]) {
  FILE *listfile;
  FILE *qfile;
  FILE *output;
  char fileinput[256];
  char filename[256];
  char input[256];
  int h1,m1,s1;
  int ntime1;
  int ntime2;
  int ntime3;
  int x,y;

/* Read and convert current time (exerpted from convtime.c) */
  fgets(input,255,stdin);
  if (input[10]!=32) {
    h1=(input[10]-48)*10+(input[11]-48);
    m1=(input[13]-48)*10+(input[14]-48);
    s1=(input[16]-48)*10+(input[17]-48);
  }else{
    h1=(input[11]-48)*10+(input[12]-48);
    m1=(input[14]-48)*10+(input[15]-48);
    s1=(input[17]-48)*10+(input[18]-48);
  }
/*  printf("%2d:%2d:%2d\n",h1,m1,s1);*/
/*  printf("+%2d:%1d%1d:%1d%1d\n",h3,m3/10,m3%10,s3/10,s3%10);*/
  ntime1=(h1*3600+m1*60+s1);
  /*printf("%d\n",ntime1);*/

  listfile=fopen(argv[1],"r");
  output=fopen(argv[2],"w");
  fgets(filename,79,listfile);
  while(filename[0]>31){
    y=0;x=filename[0];
    while(x!=0){
/*      printf(" -> %c %d\n",fileinput[y],x);*/
      if(x<32) filename[y]=0;
      y=y+1;x=filename[y];
    }
    /*printf("%s\n",filename);*/
    if (filename[0]=='q') {
      qfile=fopen(filename,"r");
      fscanf(qfile,"%d",&ntime2);
      /*printf("%d\n",ntime2);*/
      fscanf(qfile,"%d",&ntime3);
      /*printf("%d\n",ntime3);*/
      fgets(fileinput,255,qfile);
      fscanf(qfile,"%s",fileinput);
      fclose(qfile);

      /* ntime1=current ntime2=start ntime3=trigger */

      if ( (ntime1>ntime3&&ntime2<ntime3)
        || (ntime1<ntime2&&ntime2<ntime3)
        || (ntime1>ntime3&&ntime1<ntime2) ) {

        filename[0]='m';
        fprintf(output,"cat $REMAIL/queue/%s | /usr/lib/sendmail %s\n",filename,fileinput);
        fprintf(output,"rm %s\n",filename);
        filename[0]='q';
        fprintf(output,"rm %s\n",filename);
      }else{
/* If more that 12 hours have passed, reset start so we don't miss the event */
	if((ntime1+86400-ntime2)%86400>43200) {
	  qfile=fopen(filename,"w");
          fprintf(qfile,"%d\n",ntime1);
          fprintf(qfile,"%d\n",ntime3);
          fprintf(qfile,"%s",fileinput);
	  fclose(qfile);
        }
      }
    }
    filename[0]=0;
    fgets(filename,79,listfile);
  }
  fclose(listfile);
}
