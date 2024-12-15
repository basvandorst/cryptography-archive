#include <stdio.h>

void main(int argc,char *argv[]) {
  char input[256];
  char from_address[256]="";
  int x,y,z;

  fgets(input,255,stdin);
  fgets(input,255,stdin);
  /* Scan headers */
  while(input[0]!=10) {
    if ((input[0]=='F'||input[0]=='f')&&input[1]=='r'&&input[2]=='o') {
      x=0;y=0;
      while(input[x]!=':'){x=x+1;}
      x=x+1;
      while(input[x]<=32){x=x+1;}
      z=x;
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
    input[0]=10;input[1]=0;
    fgets(input,255,stdin);
  }
  printf("%s",from_address);
}
