
#include <fcntl.h>

main()
{
   char buf[8192];
   char *data="hey there\n";
   int a,x,log1,log2;

   log1=open("log1",O_CREAT|O_WRONLY);
   log2=open("log2",O_CREAT|O_WRONLY);
   a=(char)strlen(data);
   strcpy(buf+1,"hey there.\n");
   buf[0]=a;
   x=do_rsa("public",buf,a+1,8192);
   write(log1,buf,8192);
   do_rsa("secret",buf,x);
   write(log2,buf,8192);
   close(log1);
   close(log2);
   write(1,buf+1,(int) buf[0]);
   write(1,"\n",1);
   
}
