
#include <fcntl.h>

#define LEN 5          

struct _node {
  struct _node *next;
  int len;
  char *data;
};
typedef struct _node node;

extern node *decode_packets();
extern int send_packet();
extern int kill_packet_list();

main()
{
  int fd,fd2;
  int a;
  node *root,*b;
  char buf[LEN];
  char temp[200];

  fd=open("test",O_WRONLY|O_CREAT);
  send_packet(fd,"this \333s a test",14);
  send_packet(fd,"can y\300u see this?",17);
  send_packet(fd,"ok",2);
  close(fd);

  fd2=open("test2",O_WRONLY|O_CREAT);
  fd=open("test",O_RDONLY);
  while((a=read(fd,buf,LEN))>0) {
    root=decode_packets(buf,a);
    b=root;
    while(b!=0) {
      sprintf(temp,"Packet has :  (%d)\n",b->len);
      write(fd2,temp,strlen(temp));
      write(fd2,b->data,b->len);
      write(fd2,"\n",1);
      b=b->next;
    }
     kill_packet_list(root);
    
  }
  write(fd2,"\n",1);
  close(fd);
  close(fd2);
}

  

