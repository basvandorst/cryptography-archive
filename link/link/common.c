
#include <stdio.h>
#include <fcntl.h>
#include <time.h>
#include "common.h"

/* slip stuff ..  from slip.c */
struct _nd {
  struct _nd *next;
  int len;
  char *data;
};
typedef struct _nd nd;
extern int send_packet();
extern nd *decode_packets();

/* des stuff from des.c */
extern en_des();
extern un_des();
extern des_setkey();


/*DEBUG */
extern int debug;
extern int errfd;

startup(remote,a)               /* initialize fd=remote, a=1 for secure end */
int a,remote;
{
  char c=0,d;
  int i;

  d=STARTCHAR;
  for(i=0;i<10;i++) {       /* loosely test for a remote end */
    sleep(1);
write(errfd,"Sent start\n",11);
    write(remote,&d,1);
    read(remote,&c,1);
write(debug,&c,1);
    if(c==STARTCHAR) 
      break;
  }
  if(i==10)
    return(-1);
write(errfd,"Got  start\n",11);
  write(remote,&d,1);    /* one for good luck */
  des_setkey(NULLKEY);   /* default key */
  if(a)
    do_newkey();         /* request a new one */
}


de_out(buf,len)     /* decrypt and output in buffer */
int len;
char *buf;
{
  nd *root,*t;
  unsigned char l;

/* DEBUG */
write(debug,buf,len);  /* */
  root=decode_packets(buf,len);
  for(t=root;t !=0; t=t->next) {
   if(t->len > 0)
     l=t->data[0];      /* num of characters */
   else
     l=0;
   if(l&EXTEND) {       /* extended packet   */
     de_extend(t->data,t->len);
     continue; 
   }
   if (t->len < 9 ) {   /* packet must always be longer than 9 */
     int j;
     fprintf(stderr,"Bad length! (%d)\n",t->len); 
     for(j=0;j<t->len;j++) 
       fprintf(stderr,"%x%x ",(t->data[j]>>4)&0xf,t->data[j]&0xf);
     fprintf(stderr,"\n");
     continue;           /* just drop bad packets */
   }
   un_des(t->data +1);
   if(l<9) {
     WRITE_LOC(t->data+1,(int)l); 
     /*write(fd,t->data+1,(int)l); /**/ 
   }
   else
     fprintf(stderr,"Bad packet len field.\n",(int)l);
  }
  return;
}


en_out(buf,len)       /* encrypt and output in buffer */
char *buf;            /* flush at least every FLUSHTIME */
int len;              /* if called with len =0 just check */
{                     /* time */
  static char buffer[10];
  static int l=1;
  static struct timeval lastflush = {0,0};
  struct timeval t;
  int flushed=0;
  char a;

  while(len--) {
    a= *buf++;
    buffer[l++] =a;
    if(l==9) {                   /* send out a full packet */
      buffer[0] = (char)8;
      en_des(buffer+1);
      send_packet(buffer,9);
/* DEBUG */
      /*write(debug,buffer,9); /* */
      l=1;
      flushed=1;
    }
  }
  if(flushed || l==1) {
    gettimeofday(&lastflush);
    return;
  }
 
  gettimeofday(&t);
  if (t.tv_usec < lastflush.tv_usec) {
    t.tv_usec += 1000000;   /* add a second */
    t.tv_sec --;            /* take off a second */
  }
  if((t.tv_sec > lastflush.tv_sec) ||
     ((t.tv_usec-lastflush.tv_usec) > FLUSHTIME )) {
    /* we have to flush it out now */
    buffer[0]=(char)(l-1);
    en_des(buffer+1);
    send_packet(buffer,9);
/*DEBUG */
   /* write(debug,buffer,9); /* */
    l=1;
    gettimeofday(&lastflush);
  }
}

unsigned char newkey[8];
int valid_newkey =0;

do_newkey()     /* pick a random key and tell the other guys */
{
  struct timeval a;
  char buffer[BUFSIZ];
  int i,b;
  
  gettimeofday(&a,0);
  srand(a.tv_usec);
  for(i=0;i<8;i++) {
    newkey[i]=(rand()>>8)&0xff;  
    buffer[i]=newkey[i];
  } 
  b=do_rsa(PRIVFILE,buffer,8,BUFSIZ); 
  if(b<0) 
    return;
fprintf(stderr,"Sent new key\n");
  extended_out(buffer,b,NEWKEY); 
  valid_newkey=1;
}

got_newkey(buffer,len)       /* we got a new key request */
char *buffer;
int len;
{
  int b=0;
  
  b=do_rsa(PUBFILE,buffer,len,len);
  if (b>7) {
    des_setkey(buffer);
    do_ack();
  }
}

do_ack()
{
  char buf[1];

  buf[0]=0;
  extended_out(buf,1,ACK);
} 

got_ack(data)
char *data;
{
  if(valid_newkey) {
    des_setkey(newkey);
    valid_newkey=0;
    do_ack2();
  }
}

do_ack2()
{
  char buf[1];
  
  buf[0]=0;
  extended_out(buf,1,ACK2);
}

got_ack2(data)
char *data;
{ 
  /* we dont really have to do anything */
  return;
}


unsigned int chksum(data,len)  /* 1's comp of the sum of the 1's comp */
unsigned char *data;
int len;
{
  int i=0;

  while(len--) 
    i+= ~(*data++);
  return(~i);
}

extended_out(data,len,type)
char *data;
int len;
unsigned char type;
{
  char buf[MAXEXTEND];
  int a;

  if(len+4 > MAXEXTEND) {
    fprintf(stderr,"Extended packet is too long\n");
    return;    
  }
  bcopy(data,buf+4,len);
 /* memcpy(buf+4,data,len);  */
  len+=4;
  buf[0]=EXTEND|(len);
  buf[1]=type;
  buf[2]=0;
  buf[3]=0;
  a=chksum(buf,len);
  buf[2]=(a>>8)&0xff;
  buf[3]=(a)&0xff;
  send_packet(buf,len);
}

de_extend(data,len)
int len;
unsigned char *data;
{
  int a,b;
  unsigned char dlen;

  dlen=data[0]&~EXTEND;  /* take off extend bit */
  if(dlen > len) {
    fprintf(stderr,"Extended packet has invalid length\n");
    return;
  }
  a=(data[2]<<8) | data[3];  /* check checksum */
  data[2]=0;
  data[3]=0;
  b=chksum(data,dlen)&0xffff;
  if(a != b) {
    fprintf(stderr,"Checksum error in extended packet. drop\n");
    return;
  }
  switch(data[1]) {
    case NEWKEY : 
fprintf(stderr,"Got newkey request.\n");
                  got_newkey(data+4,len-4);
                  break;
    case ACK    :
fprintf(stderr,"Got ack\n");
                  got_ack(data+4);
                  break;
    case ACK2   : 
fprintf(stderr,"Got ack2\n");
                  got_ack2(data+4);
                  break;
    default:
      fprintf(stderr,"Unrecongized packet type %d\n",(int)data[2]);
      break;
  }
}


