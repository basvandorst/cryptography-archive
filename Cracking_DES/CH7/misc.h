
void GetUserInfo(unsigned char plaintextVector[32], 
        unsigned char plaintextXorMask[8], 
        unsigned char ciphertext0[8], unsigned char ciphertext[8],
        unsigned char *plaintextByteMask, int *useCBC, int *extraXor,
        int *quickStart, unsigned char startKey[7], long *numClocks);
void increment32(unsigned char *v);
void decrement32(unsigned char *v);
void desDecrypt(unsigned char m[8], unsigned char c[8], unsigned char k[7]);
void printHexString(char *tag, unsigned char *data, int len);
int hex2bin(char *hex, unsigned char *bin);

