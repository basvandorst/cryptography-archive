/*Copy N 512-byte sectors from file1 to file2 */
/*copysect file1 file2 <n>  */

#include <stdlib.h>
#include <stdio.h>

int file_exists(char *);

FILE *fin, *fout;

int i,n,z,l,lw;

char buf[512];

void main(int argc,char *argv[])
 {
  if (argc < 3)
   {
    printf("\n\
CopySect 1.4\n\
 Format is CopySect file1 file2 <n>\n\
Copies n sectors from file1 to file2.\n\
If file2 already exists, the first n sectors are updated, but the\n\
filelength is not shortened.\n\
\n\
This utility is for use with RAWDRV11. See SECDRV.DOC for details.\n");
return;
   }

fin = fopen(argv[1],"rb");
if (fin == NULL)
 {
  printf("\nCould not open input file %s\n",argv[1]);
  exit(8);
 }

 if (file_exists(argv[2]))
  {
   fout = fopen(argv[2],"r+b");
   if (fout == NULL)
    {
     printf ("\nCould not open output file %s for Update\n",argv[2]);
     exit(8);
    }
   else
    {
     printf("\n Output file %s opened for update\n",argv[2]);
     z = fseek(fout,0l,SEEK_SET);
     if (z != 0)
      {
       printf("Fseek to start failed\n");
       exit(8);
      }
    }
  }
 else
  {
   fout = fopen(argv[2],"wb");
   if (fout == NULL)
    {
     printf ("\nCould not open new output file %s\n",argv[2]);
     exit(8);
    }
   else
    printf("\n Output file %s to be created\n",argv[2]);
  }

if (argc == 4)
 n = atoi(argv[3]);
else
 n=1;

if (n == 0)
 n=1;


for (i=0;i<n;i++)
 {
  l =  fread(buf,1,512,fin);
  lw = fwrite(buf,1,l,fout);
  if (l < 512)
   {
    printf("\nSector %d of file %s only %d bytes read\n",i+1,argv[1],l);
    printf("%d bytes written to %s\n",lw,argv[2]);
    return;
   }
  if (lw < 512)
   {
    printf("\nSector %d of file %s only %d bytes written\n",i+1,argv[2],lw);
    return;
   }
 }

z = fseek(fout,0l,SEEK_END);
if (z != 0)
 {
  printf("Fseek to end failed\n");
  exit(8);
 }
fclose(fout);
fclose(fin);

 }

int file_exists(char *filename)
 {
   return (access(filename, 0) == 0);
 }
