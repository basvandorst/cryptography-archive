/* Secure Drive FPART V1.3d */
/* Finds (possible) Partition boot records */

#include "secdrv.h"

int  tsr_not_installed;
int  tsr_wrong_version;
void setrldb(void far *real_dbios_vect);

unsigned char *buf;
int invalid_boot=0;

void main()
{
unsigned drive,firstcyl,firsthead;
unsigned cyl,head,sector,maxcyl,endcyl,maxhead,maxsector,secsize,t;
unsigned i,hdx;
unsigned serial[2];

char drvltr,r;

clrscr();


if(!(buf=malloc(512))) {
    printf("Error: unable to allocate memory for sector buffer.\n");
    exit(1); }


printf("\
Secure Drive Find Partition Utility Version 1.3d \n\
\n\
This program searches a physical hard disk for boot records.\n");

gettsradr();

if(tsr_wrong_version)
 {
  printf ("ERROR: Wrong Version (not 1.3D) of SECTSR is Installed\n");
  exit(1);
 }

if (tsr_not_installed)
  printf("Warning: SECTSR not installed.\n");

askdrive:
printf("Enter Physical Drive Number (0,1,2,...) to Scan: ");
while(!isdigit(drvltr=toupper(getch())));
printf("%c\n",drvltr);


drive=drvltr-'0';
printf("\n\
If you don't know how many Cylinders are on this drive, just enter a\n\
high value, e.g. 20000, and Abort when read errors are reported after\n\
running off the end.\n\n\
Enter number of Cylinders to scan : ");
scanf("%u",&endcyl);
drive+=0x80;

for (firstcyl=0;firstcyl<endcyl;firstcyl++)
 {
  for(firsthead=0;firsthead<2;firsthead++)
   {
    readsec(drive,firsthead,firstcyl,1,1,buf);
    if((buf[510]==0x55)&&(buf[511]==0xaa))
     {
      calcdiskparams(buf,&maxcyl,&maxhead,&maxsector,
                     &secsize,serial);
      if (!invalid_boot && (secsize & 511) == 0)
       {
        printf("\nPossible partition found on \
physical hard drive %u, at cylinder %u, head %u\n",
               drive-0x80,firstcyl,firsthead);

        printf("This partition has \
%u cylinders, %u sectors, %u heads, sector size %u bytes\n\n",
               maxcyl+1,maxsector,maxhead,secsize);

       printf("Continue to Scan? ");
       if(!getyn())
           {
           printf("\n");
           exit(0);
           }

       } /*if !invalid_boot*/
     } /* if boot sector*/
   } /*for firsthead*/
 } /*for firstcyl*/

printf("End of Scan\n");
exit(0);
}

struct tsrdata far *gettsradr(void)
{
unsigned seg;
unsigned ofs;
struct tsrdata far *ptr;
struct REGPACK rgs;

rgs.r_ax=0x0800;
rgs.r_dx=0x00f0;
intr(0x13,&rgs);
if(rgs.r_ax==0x0edcb)
 {
  tsr_wrong_version=1;
  return( (struct tsrdata far *) NULL);
 }
if(rgs.r_ax!=0x0edcc)
 {
  tsr_not_installed=1;
  return( (struct tsrdata far *) NULL);
 }
/* ptr=(long) rgs.r_dx+(long) 0x10000*rgs.r_cx; */
ptr = MK_FP(rgs.r_cx,rgs.r_dx);
if (memcmp(ptr->tsrver,"130D",4) != 0)
 {
  tsr_wrong_version=1;
  return( (struct tsrdata far *) NULL);
 }
setrldb(&ptr->real_dbios_vect);
return(ptr);
}

void calcdiskparams(unsigned char *buf,unsigned *maxtrack,
                    unsigned *maxhead,unsigned *maxsector,
                    unsigned *secsize,unsigned serial[2])
{
unsigned long i,l0,l1,l2,l3;

invalid_boot=0;
*maxsector=buf[0x18]+256*buf[0x19];
*maxhead=buf[0x1a]+256*buf[0x1b];
*secsize=buf[0x0b]+256*buf[0x0c];
serial[0]=buf[0x27]+256*buf[0x28];
serial[1]=buf[0x29]+256*buf[0x2a];
l0=(unsigned char)buf[0x20];
l1=(unsigned char)buf[0x21];
l2=(unsigned char)buf[0x22];
l3=(unsigned char)buf[0x23];
i=l0+256*(l1+256*(l2+256*(l3)));
if(i==0) i=(unsigned)buf[0x13]+256*buf[0x14];
if (*maxsector == 0 || *maxhead == 0)
 {
  invalid_boot=1;
  return;
 }
*maxtrack=i / *maxsector / *maxhead;
if((i%(*maxsector * *maxhead))==0) (*maxtrack)--;
}

void readsec(unsigned drive,unsigned head,unsigned track,
             unsigned sector,unsigned nsects,void *buffer)
{
unsigned i;
char c;
for(;;) {
    for(i=0;i<3;i++)
        if(!rldbios(2,drive,head,track,sector,nsects,buffer)) return;
    printf("\nRead error: drive %02x, head %u, track %u\n",
            drive,head,track);
    printf("Abort, Retry, Ignore? ");
    c=toupper(getch()); printf("%c\n",c);
    if(c=='I') return;
    if(c=='A') exit(1);
    }
}

int getyn(void)
{
char c;
for(;;)
    {
    c=getch();
    if(c=='y'||c=='Y') { printf("Yes\n\n"); return(TRUE); }
    if(c=='n'||c=='N') { printf("No\n\n"); return(FALSE); }
    }
}
