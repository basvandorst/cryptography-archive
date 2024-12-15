/*
 * keyring scanner - unfriendly
 *
 * usage: ringsearch pubring.pgp keyid-16-hex-digits
 *
 * This file is COPYRIGHT - see the file COPYRIGHT in this
 * directory.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int e(ring)
     FILE *ring;
{
  if (!ferror(ring)) {
    fputs("Keyring error: premature EOF\n",stderr);
  }
}

int anythismatch=0;
int anymatches=0;
int inrightkey=0;

int main(argc, argv)
     int argc;
     char **argv;
{
  FILE *ring;
  unsigned char ctb;
  unsigned char packet[2048];
  unsigned char lenc[4];
  unsigned long len;
  unsigned char *q;
  unsigned char keyid[8];
  unsigned short mpil;
  char *p;
  unsigned int t;
  int il,i,l,n,bits=0;

  if (argc != 3) exit(1);
  ring= fopen(argv[1],"rb");
  if (!ring) { perror("Keyring error (open)"); exit(0); }
  if (strlen(argv[2])!=16) exit(1);
  for (p= argv[2], q=keyid, i= 8;
       i;
       p+=2, i--, q++) {
    if (sscanf(p,"%2x",&t) != 1) exit(1);
    *q= t;
  }
  for (il=16, q=keyid;
       il && !*q;
       il--, q++);
  for (;;) {
    n= fread(&ctb,1,1,ring);
    if (n!=1) break;
    l= ctb & 0x03;
    if (l == 0x03) {
      fputs("Keyring error: packet with unknown length",stderr);
      break;
    }
    l= 1<<l;
    memset(lenc,0,4);
    n= fread(lenc + (4-l),1,l,ring);
    if (n != l) { e(ring); break; }
    len= (((((lenc[0] << 8) | lenc[1]) << 8) | lenc[2]) << 8) | lenc[3];
    if (len > 2048) {
      fprintf(stderr,"Keyring error: packet too long (0x%lx)\n",len);
      break;
    }
    n= fread(packet,1,len,ring);
    if (n != len) { e(ring); break; }
    switch ((ctb >>2) & 0x0f) {
    case 0x6: /* public key */
      if (inrightkey) {
        if (!anythismatch)
          printf("Key %4d/%s - has no userids!\n",
                 bits,argv[2]);
        inrightkey= 0;
      }
      if (packet[0] != 2) continue;
      bits= (packet[8] << 8) | packet[9];
      mpil= (bits + 7) >> 3;
      if (mpil < il) continue;
      if (
          mpil < 8 ? memcmp(keyid + (8-mpil), packet+10, mpil)
                   : memcmp(keyid, packet+2+mpil, 8)
          ) continue;
      inrightkey= 1;
      anythismatch= 0;
      anymatches= 1;
      continue;
    case 0xd: /* userid */
      if (!inrightkey) continue;
      if (!anythismatch) {
        printf("Key %4d/%s  ",bits,argv[2]+8);
        anythismatch= 1;
      } else {
        printf("                   ");
      }
      fwrite(packet,1,len,stdout);
      putchar('\n');
      continue;
    default:;
    }
  }
  if (ferror(ring)) { perror("Keyring error (read)"); exit(0); }
  if (inrightkey && !anythismatch)
    printf("Key %4d/%s - is last in keyring and has no userids! keyring truncated?\n",
           bits,argv[2]);
  if (!anymatches) {
    printf("Key with ID (%.8s)%s not found in public keyring\n",argv[2],argv[2]+8);
  }
  return 0;
}
