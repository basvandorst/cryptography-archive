#include  <string.h>
#include  <stdlib.h>
#include  <stdio.h>
#include <iomanip.h>
#include "cactus.hpp"
int main(int argc, char * argv[]){
char * key, * msk;
char *path;
char ch;
unsigned long i,j;
char i1,i2,i3,o1,o2,o3,o4;
char p[] ="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
MiCactus crypt;
if (!registered) cout << "\a >>> Please register StrCrypt.EXE! \a\n";
if (argc < 3) {
   cout << "String encryptor using the CACTUS-Algorithm and MIME coding." << '\n'
        << "Author: Michael Mieves, Version 1.0, April 1996." << '\n'
        << "StrCrypt returns an encrypted/decrypted string (max 255 bytes)." << '\n'
        << "Usage: StrCrypt -e|d <string> " << '\n'
        << "-e encrypts an clear string, -d decrypts a crypted string" << '\n';
        return -1;
   }
cout << "Please wait . . ." << '\n';

char clear_string[256];
char crypt_string[350];
char mode[3];
int crypt_flag;
strcpy(mode,argv[1]);

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
crypt.reset(key,msk,0);
if (mode[1] == 'e'){
    strcpy(clear_string,argv[2]);
    j = 0;
    crypt_string[0]='\0';
    for (i=0;i<strlen(clear_string);i=i+3){
       i1=i2=i3=0;
       i1=clear_string[i] ^ crypt.gen_byte();
       i2=clear_string[i+1] ^ crypt.gen_byte();
       i3=clear_string[i+2] ^ crypt.gen_byte();
       crypt_string[j] = p[((i1&0xfc)>>2)];
       crypt_string[j+1] = p[((i1&3)<<4) + ((i2&0xf0)>>4)];
       crypt_string[j+2] = p[((i2&0x0f)<<2) + ((i3&0xc0)>>6)];
       crypt_string[j+3] = p[(i3&0x3f)];
       j = j+4;
       }
    cout << "Plain: " << clear_string << '\n';
    cout << "Crypt: " << crypt_string << '\n';
    }
 else{
    crypt.reset(key,msk,0);
    strcpy(crypt_string,argv[2]);
    clear_string[0] = '\0';
    j = 0;
    for (i=0;i<strlen(crypt_string);i=i+4){
       o1=o2=o3=o4=0;
       o1 = crypt_string[i];
       o2 = crypt_string[i+1];
       o3 = crypt_string[i+2];
       o4 = crypt_string[i+3];
       clear_string[j] = (((strchr(p,o1)-p)<<2) + (((strchr(p,o2)-p)&0x30)>>4)) ^ crypt.gen_byte();
       clear_string[j+1] =((((strchr(p,o2)-p)&0xf)<<4) + (((strchr(p,o3)-p)&0x3c)>>2)) ^ crypt.gen_byte();
       clear_string[j+2] =((((strchr(p,o3)-p)&3) <<6 )+ (strchr(p,o4)-p)) ^ crypt.gen_byte();
       j = j+3;
       }

    cout << "Plain: " << clear_string << '\n';
    cout << "Crypt: " << crypt_string << '\n';
  }
return 0;
}
