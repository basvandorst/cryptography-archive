#include  <string.h>
#include  <stdlib.h>
#include  <stdio.h>
#include <iomanip.h>
#include <fstream.h>
#include "cactus.hpp"

int main(int argc, char * argv[]){
char clr[256],cry[256];
char * key, * msk;
char *path;
char ch;
unsigned long i,j;
MiCactus crypt;
if (!registered) cout << "\a >>> Please register FCryptX.EXE! \a\n";
if (argc < 4) {
   cout << "File encryption using the CACTUS-Algorithm." << '\n'
        << "Author: Michael Mieves, Version 1.0, April 1996." << '\n'
        << "FCryptX produces an encrypted/decrypted file." << '\n'
        << "Usage: FCryptX -e|d <clearfile> <cryptfile> " << '\n'
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
    ifstream infile(clr,ios::binary);
    ofstream outfile(cry,ios::binary);
    while(infile.get(ch))outfile.put((char)(ch ^ crypt.gen_byte()));
    infile.close();
    outfile.close();

}
 else{
    crypt.reset(key,msk,0);
    ifstream infile(cry,ios::binary);
    ofstream outfile(clr,ios::binary);
    while(infile.get(ch))outfile.put((char)(ch ^ crypt.gen_byte()));
    infile.close();
    outfile.close();

}
return 1;
}
