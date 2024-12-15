#include  <string.h>
#include  <stdlib.h>
#include  <stdio.h>
#include <iomanip.h>
#include <fstream.h>
#include "cactus.hpp"
#include <string.hpp>
int main(int argc, char * argv[]){
char * key, * msk;
char *path;
char ch;
unsigned long length,clr_len;
String clr_str,cry_str;
unsigned long i,j,k;
char clr[256],cry[256];
char i1,i2,i3,o1,o2,o3,o4;
char p[] ="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
MiCactus crypt;
if (!registered) cout << "\a >>> Please register FCryptM.EXE! \a\n";
if (argc < 4) {
   cout << "File encryption using the CACTUS-Algorithm and MIME coding." << '\n'
        << "Author: Michael Mieves, Version 1.0, April 1996." << '\n'
        << "FCryptM produces a MIME coded encrypted/decrypted file." << '\n'
        << "Usage: FCryptM -e|d <clearfile> <cryptfile> " << '\n'
        << "-e encrypts an clear file, -d decrypts a crypted file." << '\n';
        return 0;
   }
cout << "Please wait . . ." << '\n';
char mode[3];
int crypt_flag;
strcpy(mode,argv[1]);
strcpy(clr,argv[2]);
strcpy(cry,argv[3]);
key = new char[crypt.lenall];
msk = new char[256];
for (i = 0;i<crypt.lenall;i++){
            key[i] = 0;
            }
for (i = 0;i<256;i++){
            msk[i] = 0;
            }
if(path == getenv("CACTUSKEY")){
      ifstream inkey(path,ios::binary);
      for (i = 0;i<crypt.lenall;i++){
           inkey >> key[i];
           }
      for (i = 0;i<256;i++){
           inkey >> msk[i];
           }
      inkey.close();
      }

crypt.initiate(key,msk);

if (mode[1] == 'e'){
    crypt.reset(key,msk,0);
    ifstream infile(clr,ios::binary|ios::ate);
    length = infile.tellg();
    infile.seekg(0);
    ofstream outfile(cry,ios::binary);
    outfile.flags(ios::left);
    outfile << setw(10) << length;
    for (k=0;k<length;k++){
        clr_str.put_at(k,infile.get());
        }
    for (i=0;i<clr_str.length();i=i+3){
       i1=i2=i3=0;
       i1=clr_str[i] ^ crypt.gen_byte();
       i2=clr_str[i+1] ^ crypt.gen_byte();
       i3=clr_str[i+2] ^ crypt.gen_byte();
       outfile.put((char)p[((i1&0xfc)>>2)]);
       outfile.put((char)p[((i1&3)<<4) + ((i2&0xf0)>>4)]);
       outfile.put((char)p[((i2&0x0f)<<2) + ((i3&0xc0)>>6)]);
       outfile.put((char)p[(i3&0x3f)]);
       }
    infile.close();
    outfile.close();
    }
 else{
    crypt.reset(key,msk,0);
    ifstream infile(cry,ios::binary|ios::ate);
    length = infile.tellg();
    infile.seekg(0);
    infile >> dec >> clr_len;
    infile.seekg(10);
    ofstream outfile(clr,ios::binary);
    for (k=0;k<length;k++){
        cry_str.put_at(k, infile.get());
        }
    j=0;
    for (i=0;i<cry_str.length();i=i+4){
       o1=o2=o3=o4=0;
       o1 = cry_str[i];
       o2 = cry_str[i+1];
       o3 = cry_str[i+2];
       o4 = cry_str[i+3];
       clr_str.put_at(j, (((strchr(p,o1)-p)<<2) + (((strchr(p,o2)-p)&0x30)>>4)) ^ crypt.gen_byte());
       clr_str.put_at(j+1,((((strchr(p,o2)-p)&0xf)<<4) + (((strchr(p,o3)-p)&0x3c)>>2)) ^ crypt.gen_byte());
       clr_str.put_at(j+2,((((strchr(p,o3)-p)&3) <<6 )+ (strchr(p,o4)-p)) ^ crypt.gen_byte());
       j = j+3;
       }
     clr_str = clr_str(0,clr_len);
     outfile << clr_str;
     infile.close();
     outfile.close();
  }

return 1;
}
