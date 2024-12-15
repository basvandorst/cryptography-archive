/*
	Filename: rng.h
	Application: Header file supporting NLRNG
	Codeword OTP V1.0 Vernam Cipher Utility
	R.James 11-08-1883
*/

#define N 12

int ivar;
char var[4];
unsigned long Rword;
unsigned long Lword;

static unsigned long c1[N]={
	0x5B54ED61L, 0x672A9746L, 0xD147396CL, 0xC3D8067BL,
    0x6C9A143FL, 0x64B3EC49L, 0x44BE3BD0L, 0x34F4CC69L,
	0xF0E27F08L, 0xC92B01BFL, 0x426A57E6L, 0xF0207BDCL };

static unsigned long c2[N]={
	0x706DF456L, 0xEC349A81L, 0x24CF78E2L, 0xF06214F5L,
	0x4C1294FFL, 0x6B404E6FL, 0x04133F7DL, 0x97A496A5L,
	0x186B693EL, 0x02C64FBBL, 0xA5355C27L, 0x780ED9D1L };
