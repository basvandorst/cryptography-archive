#include <stdio.h>

void main(int argc,char *argv[]) {
  FILE *file;
  char input[256];
  int h1,m1,s1;
  int h2,m2,s2;
  int h3,m3,s3;
  int ntime1,ntime2;
  int x;
  int plusflag=0;
  int randflag=0;

  file=fopen(argv[1],"r");
  fgets(input,255,file);
  if (input[10]!=32) {
    h1=(input[10]-48)*10+(input[11]-48);
    m1=(input[13]-48)*10+(input[14]-48);
    s1=(input[16]-48)*10+(input[17]-48);
  }else{
    h1=(input[11]-48)*10+(input[12]-48);
    m1=(input[14]-48)*10+(input[15]-48);
    s1=(input[17]-48)*10+(input[18]-48);
  }
  fclose(file);
  ntime1=(h1*3600+m1*60+s1);
  printf("%d\n",ntime1);

  file=fopen(argv[2],"r");
  fgets(input,255,file);
  fclose(file);
  x=0;
  plusflag=0;randflag=0;
  h2=0;m2=0;s2=0;


  while((input[x]<'0'||input[x]>'9')&&input[x]>=32) {
    if (input[x]=='+') plusflag=1;
    if ((input[x]=='r')||(input[x]=='R')) randflag=1;
    x++;
  }
  while (input[x]>='0'&&input[x]<='9') {
    h2=(h2*10)+(input[x]-48);
    x++;
  }
  if(input[x]==':') {
    x++;
    while (input[x]>='0'&&input[x]<='9') {
      m2=(m2*10)+(input[x]-48);
      x++;
    }
    if(input[x]==':') {
      x++;
      while (input[x]>='0'&&input[x]<='9') {
        s2=(s2*10)+(input[x]-48);
        x++;
      }
    }
  }
  while(input[x]>=32) {
    if (input[x]=='+') plusflag=1;
    if ((input[x]=='r')||(input[x]=='R')) randflag=1;
    x++;
  }

  ntime2=(h2*3600+m2*60+s2);

  if(ntime2>=86400) ntime2=86399;

  if(plusflag==0) {
    ntime2=ntime2-ntime1;
    if(ntime2<0) ntime2+=86400;
  }

  if(randflag!=0) {
    ntime2=((ntime1*71)%86401+(ntime1%59)*3601)%ntime2;
    /* Okay, its a shitty PRNG.  Write a better one. */
  }

  ntime2=(ntime2+ntime1)%86400;

  printf("%d\n",ntime2);

}
