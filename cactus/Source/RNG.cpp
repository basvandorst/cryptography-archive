#include  <string.h>
#include  <stdlib.h>
#include  <stdio.h>
#include <iomanip.h>
#include "cactus.hpp"
int main(int argc, char * argv[]){
char * key, * msk;
char *path;
char * seed;
char ch;
unsigned long i,j;
unsigned long seed_len;

MiCactus crypt;
if (!registered) cout << "\a >>> Please register RNG.EXE! \a\n";

if (argc < 4) {
   cout << "Random Number Generator using the CACTUS-Algorithm" << '\n'
        << "Author: Michael Mieves, Version 1.0, April 1996." << '\n'
        << "RNG writes random numbers into a file." << '\n'
        << "Usage: RNG <filename> <filelength> <preruns> [SEED]" << '\n';
        return 0;
   }
cout << "Please wait . . ." << '\n';
char name[256];
strcpy(name,argv[1]);
key = new char[crypt.lenall];
msk = new char[256];
for (i = 0;i<crypt.lenall;i++){
            key[i] = 0;
            }
for (i = 0;i<256;i++){
            msk[i] = 0;
            }
if(path = getenv("CACTUSKEY")){
      ifstream inkey(path,ios::binary|ios::ate);
      seed_len = inkey.tellg();
      inkey.seekg(0);
      j = 0;
      for (i = 0;i<crypt.lenall;i++){
           inkey >> key[i];
           j++;
           if (j == seed_len) {
              inkey.seekg(0);
              j = 0;
              }
           }
      for (i = 0;i<256;i++){
           if(inkey.eof()) inkey.seekg(0);
           inkey >> msk[i];
           j++;
           if (j == seed_len) {
              inkey.seekg(0);
              j = 0;
              }
           }
      inkey.close();
      }
if (argc == 5){
     seed_len = strlen(argv[4]);
     seed = new char[seed_len];
     strcpy(seed,argv[4]);
     j = 0;
     for (i = 0;i<crypt.lenall;i++){
           key[i] ^= seed[j];
           j++;
           if(j == seed_len) j = 0;
           }
      for (i = 0;i<256;i++){
           msk[i] ^= seed[j];
           j++;
           if(j == seed_len) j = 0;
           }
        }
crypt.initiate(key,msk);
crypt.reset(key,msk,strtoul(argv[3],NULL,10));
ofstream out(name,ios::binary);
for (i=0;i<strtoul(argv[2],NULL,10);i++){
   out << crypt.gen_byte();
   }
out.close();
return 1;
}
