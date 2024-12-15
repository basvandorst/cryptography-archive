
#include <fcntl.h>

main()
{
#define BUFLEN 100
  char buf[BUFLEN];
  int a,left=50,log;

  log=open("Logfile",O_WRONLY|O_CREAT|O_TRUNC);
  while((a=read(0,buf,BUFLEN)) >0 ) {
    write(log,buf,a);
    left-=a;
    if(left<0) break;
  }
  close(log);
printf("done\n");
}

