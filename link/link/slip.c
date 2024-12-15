
#define END       0300   /* end of packet */
#define ESC       0333   /* escape special chars */
#define ESC_END   0334   /* send ESC ESC_END  when end char in packet */
#define ESC_ESC   0335   /* send ESC ESC_ESC  when esc char in packet */

#include <stdio.h>
#include "common.h"   /* we use WRITE_REM for send_packet */

/* send_packet(data,len)   sends a packet of data in SLIP format. 
 *   END data END
 *   any occurance of characters 'ESC' or 'END' in the data portion
 *   of a packet are expanded with an ESC sequence.
 *   total expanded packet length not to exceed 200 bytes 
 *   output is done with the macro WRITE_REM(buf,len)
 */ 
send_packet(buf,len)
int len;
char *buf;
{
  unsigned char ex[200],a;
  int l;

  l=0;
  ex[l++] = END;                 
  while (len--) {
    switch(a = *buf++) {
      case ESC :  
                 ex[l++]=ESC;
                 ex[l++]=ESC_ESC;
                 break;
      case END : 
                 ex[l++]=ESC;
                 ex[l++]=ESC_END;
                 break;
      default:
                 ex[l++]=a;
    }
  }
  ex[l++]=END;

/*  write(fd,ex,l); */
  WRITE_REM(ex,l);
}


/*
 * decode_packets(buf,len)        decodes incomming stream into packets
 *   according to SLIP framing.  buf contains input stream data,
 *   a pointer to a linked list of packets is returned, the structure
 *   is:
 *            struct  *next    pointer to next node, or NULL
 *            int     len      number of bytes of data
 *            char    *data    pointer to the data
 * null packets and packets with incorrect SLIP syntax are dropped.
 * each slip packet must be limited to 200 bytes (after being
 * decoded ) or the extra data is dropped
 */

struct _nd {
      struct _nd *next;
      int len;
      char *data;
} ;         
typedef struct _nd nd;

#define MAXPLEN 2048 

nd *decode_packets(buf,len)
char *buf;
int len;
{
  static int escape=0,badpacket=0;  /* state flags */
  static int l=0;                   /* construction buffer and length */
  static char pak[MAXPLEN];        
  nd *root=0,*tail,*temp;
  unsigned char a,*p;

  while(len--) {
    a= *buf++;
    if(l==MAXPLEN)   /* cant fit anymore! */
      escape=0;
    if(escape) {
      escape=0;
      switch(a) {
        case ESC_ESC:
                 pak[l++]=ESC;
                 break;
        case ESC_END:
                 pak[l++]=END;
                 break;
        default:    /* bad nasty error!!! */
                 badpacket=1;
                 break;
      }
    }
    else            /* else not escape */
      switch(a) {
        case ESC:
            escape=1;
            break;
        case END:
            if(l==0) {       /* null packet, ignore */
/* fprintf(stderr,"Null packet.  dropped\n"); /* */
              break;
            }
            if(badpacket) {  /* bad packet, ignore */
fprintf(stderr,"Bad packet.  dropped\n");  /* */
              badpacket=0;
              l=0;
              break;
            } 
            if((temp=(nd *)malloc(sizeof(nd)))==0) {
fprintf(stderr,"Cant allocate. packet dropped\n");  /* */
              break;
            }
            if((p=(unsigned char *)malloc(l+2))==0) {
fprintf(stderr,"Cant allocate. packet dropped\n");  /* */
              free(temp);
              break;
            }
            temp->data = (char *)p;
            temp->next = 0;
            temp->len = l;
            if(root==0) 
              root=temp;
            else
              tail->next = temp;
            tail=temp;
            bcopy(pak,p,l); 
            l=0;
            break;
        default:
            if(l!=MAXPLEN)  /* make sure we have room */
            pak[l++]=a;
      }
  }   /* while len-- */

  return(root);
}

kill_packet_list(root)
nd *root;
{
  nd *temp;

  while(root) {
    temp=root->next;
    if(root->data) 
      free(root->data);
    free(root);
    root=temp;
  }
}
  
