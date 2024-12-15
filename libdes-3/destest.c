/* destest.c */
/* Copyright (C) 1993 Eric Young - see README for more details */
#include <stdio.h>
#include "des_locl.h" /* for des.h and bcopy macros */
/* tisk tisk - the test keys don't all have odd parity :-( */

/* test data */
#define NUM_TESTS 34
static unsigned char key_data[NUM_TESTS][8]={
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0x30,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,
	0x01,0x23,0x45,0x67,0x89,0xAB,0xCD,0xEF,
	0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0xFE,0xDC,0xBA,0x98,0x76,0x54,0x32,0x10,
	0x7C,0xA1,0x10,0x45,0x4A,0x1A,0x6E,0x57,
	0x01,0x31,0xD9,0x61,0x9D,0xC1,0x37,0x6E,
	0x07,0xA1,0x13,0x3E,0x4A,0x0B,0x26,0x86,
	0x38,0x49,0x67,0x4C,0x26,0x02,0x31,0x9E,
	0x04,0xB9,0x15,0xBA,0x43,0xFE,0xB5,0xB6,
	0x01,0x13,0xB9,0x70,0xFD,0x34,0xF2,0xCE,
	0x01,0x70,0xF1,0x75,0x46,0x8F,0xB5,0xE6,
	0x43,0x29,0x7F,0xAD,0x38,0xE3,0x73,0xFE,
	0x07,0xA7,0x13,0x70,0x45,0xDA,0x2A,0x16,
	0x04,0x68,0x91,0x04,0xC2,0xFD,0x3B,0x2F,
	0x37,0xD0,0x6B,0xB5,0x16,0xCB,0x75,0x46,
	0x1F,0x08,0x26,0x0D,0x1A,0xC2,0x46,0x5E,
	0x58,0x40,0x23,0x64,0x1A,0xBA,0x61,0x76,
	0x02,0x58,0x16,0x16,0x46,0x29,0xB0,0x07,
	0x49,0x79,0x3E,0xBC,0x79,0xB3,0x25,0x8F,
	0x4F,0xB0,0x5E,0x15,0x15,0xAB,0x73,0xA7,
	0x49,0xE9,0x5D,0x6D,0x4C,0xA2,0x29,0xBF,
	0x01,0x83,0x10,0xDC,0x40,0x9B,0x26,0xD6,
	0x1C,0x58,0x7F,0x1C,0x13,0x92,0x4F,0xEF,
	0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,
	0x1F,0x1F,0x1F,0x1F,0x0E,0x0E,0x0E,0x0E,
	0xE0,0xFE,0xE0,0xFE,0xF1,0xFE,0xF1,0xFE,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0x01,0x23,0x45,0x67,0x89,0xAB,0xCD,0xEF,
	0xFE,0xDC,0xBA,0x98,0x76,0x54,0x32,0x10};

static unsigned char plain_data[NUM_TESTS][8]={
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0x10,0x00,0x00,0x00,0x00,0x00,0x00,0x01,
	0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,
	0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,
	0x01,0x23,0x45,0x67,0x89,0xAB,0xCD,0xEF,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x01,0x23,0x45,0x67,0x89,0xAB,0xCD,0xEF,
	0x01,0xA1,0xD6,0xD0,0x39,0x77,0x67,0x42,
	0x5C,0xD5,0x4C,0xA8,0x3D,0xEF,0x57,0xDA,
	0x02,0x48,0xD4,0x38,0x06,0xF6,0x71,0x72,
	0x51,0x45,0x4B,0x58,0x2D,0xDF,0x44,0x0A,
	0x42,0xFD,0x44,0x30,0x59,0x57,0x7F,0xA2,
	0x05,0x9B,0x5E,0x08,0x51,0xCF,0x14,0x3A,
	0x07,0x56,0xD8,0xE0,0x77,0x47,0x61,0xD2,
	0x76,0x25,0x14,0xB8,0x29,0xBF,0x48,0x6A,
	0x3B,0xDD,0x11,0x90,0x49,0x37,0x28,0x02,
	0x26,0x95,0x5F,0x68,0x35,0xAF,0x60,0x9A,
	0x16,0x4D,0x5E,0x40,0x4F,0x27,0x52,0x32,
	0x6B,0x05,0x6E,0x18,0x75,0x9F,0x5C,0xCA,
	0x00,0x4B,0xD6,0xEF,0x09,0x17,0x60,0x62,
	0x48,0x0D,0x39,0x00,0x6E,0xE7,0x62,0xF2,
	0x43,0x75,0x40,0xC8,0x69,0x8F,0x3C,0xFA,
	0x07,0x2D,0x43,0xA0,0x77,0x07,0x52,0x92,
	0x02,0xFE,0x55,0x77,0x81,0x17,0xF1,0x2A,
	0x1D,0x9D,0x5C,0x50,0x18,0xF7,0x28,0xC2,
	0x30,0x55,0x32,0x28,0x6D,0x6F,0x29,0x5A,
	0x01,0x23,0x45,0x67,0x89,0xAB,0xCD,0xEF,
	0x01,0x23,0x45,0x67,0x89,0xAB,0xCD,0xEF,
	0x01,0x23,0x45,0x67,0x89,0xAB,0xCD,0xEF,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};

static unsigned char cipher_data[NUM_TESTS][8]={
	0x8C,0xA6,0x4D,0xE9,0xC1,0xB1,0x23,0xA7,
	0x73,0x59,0xB2,0x16,0x3E,0x4E,0xDC,0x58,
	0x95,0x8E,0x6E,0x62,0x7A,0x05,0x55,0x7B,
	0xF4,0x03,0x79,0xAB,0x9E,0x0E,0xC5,0x33,
	0x17,0x66,0x8D,0xFC,0x72,0x92,0x53,0x2D,
	0x8A,0x5A,0xE1,0xF8,0x1A,0xB8,0xF2,0xDD,
	0x8C,0xA6,0x4D,0xE9,0xC1,0xB1,0x23,0xA7,
	0xED,0x39,0xD9,0x50,0xFA,0x74,0xBC,0xC4,
	0x69,0x0F,0x5B,0x0D,0x9A,0x26,0x93,0x9B,
	0x7A,0x38,0x9D,0x10,0x35,0x4B,0xD2,0x71,
	0x86,0x8E,0xBB,0x51,0xCA,0xB4,0x59,0x9A,
	0x71,0x78,0x87,0x6E,0x01,0xF1,0x9B,0x2A,
	0xAF,0x37,0xFB,0x42,0x1F,0x8C,0x40,0x95,
	0x86,0xA5,0x60,0xF1,0x0E,0xC6,0xD8,0x5B,
	0x0C,0xD3,0xDA,0x02,0x00,0x21,0xDC,0x09,
	0xEA,0x67,0x6B,0x2C,0xB7,0xDB,0x2B,0x7A,
	0xDF,0xD6,0x4A,0x81,0x5C,0xAF,0x1A,0x0F,
	0x5C,0x51,0x3C,0x9C,0x48,0x86,0xC0,0x88,
	0x0A,0x2A,0xEE,0xAE,0x3F,0xF4,0xAB,0x77,
	0xEF,0x1B,0xF0,0x3E,0x5D,0xFA,0x57,0x5A,
	0x88,0xBF,0x0D,0xB6,0xD7,0x0D,0xEE,0x56,
	0xA1,0xF9,0x91,0x55,0x41,0x02,0x0B,0x56,
	0x6F,0xBF,0x1C,0xAF,0xCF,0xFD,0x05,0x56,
	0x2F,0x22,0xE4,0x9B,0xAB,0x7C,0xA1,0xAC,
	0x5A,0x6B,0x61,0x2C,0xC2,0x6C,0xCE,0x4A,
	0x5F,0x4C,0x03,0x8E,0xD1,0x2B,0x2E,0x41,
	0x63,0xFA,0xC0,0xD0,0x34,0xD9,0xF7,0x93,
	0x61,0x7B,0x3A,0x0C,0xE8,0xF0,0x71,0x00,
	0xDB,0x95,0x86,0x05,0xF8,0xC8,0xC6,0x06,
	0xED,0xBF,0xD1,0xC6,0x6C,0x29,0xCC,0xC7,
	0x35,0x55,0x50,0xB2,0x15,0x0E,0x24,0x51,
	0xCA,0xAA,0xAF,0x4D,0xEA,0xF1,0xDB,0xAE,
	0xD5,0xD4,0x4F,0xF7,0x20,0x68,0x3D,0x0D,
	0x2A,0x2B,0xB0,0x08,0xDF,0x97,0xC2,0xF2};

static unsigned char cipher_3ecb[NUM_TESTS-1][8]={
	0x92,0x95,0xB5,0x9B,0xB3,0x84,0x73,0x6E,
	0x19,0x9E,0x9D,0x6D,0xF3,0x9A,0xA8,0x16,
	0x2A,0x4B,0x4D,0x24,0x52,0x43,0x84,0x27,
	0x35,0x84,0x3C,0x01,0x9D,0x18,0xC5,0xB6,
	0x4A,0x5B,0x2F,0x42,0xAA,0x77,0x19,0x25,
	0xA0,0x6B,0xA9,0xB8,0xCA,0x5B,0x17,0x8A,
	0xAB,0x9D,0xB7,0xFB,0xED,0x95,0xF2,0x74,
	0x3D,0x25,0x6C,0x23,0xA7,0x25,0x2F,0xD6,
	0xB7,0x6F,0xAB,0x4F,0xBD,0xBD,0xB7,0x67,
	0x8F,0x68,0x27,0xD6,0x9C,0xF4,0x1A,0x10,
	0x82,0x57,0xA1,0xD6,0x50,0x5E,0x81,0x85,
	0xA2,0x0F,0x0A,0xCD,0x80,0x89,0x7D,0xFA,
	0xCD,0x2A,0x53,0x3A,0xDB,0x0D,0x7E,0xF3,
	0xD2,0xC2,0xBE,0x27,0xE8,0x1B,0x68,0xE3,
	0xE9,0x24,0xCF,0x4F,0x89,0x3C,0x5B,0x0A,
	0xA7,0x18,0xC3,0x9F,0xFA,0x9F,0xD7,0x69,
	0x77,0x2C,0x79,0xB1,0xD2,0x31,0x7E,0xB1,
	0x49,0xAB,0x92,0x7F,0xD0,0x22,0x00,0xB7,
	0xCE,0x1C,0x6C,0x7D,0x85,0xE3,0x4A,0x6F,
	0xBE,0x91,0xD6,0xE1,0x27,0xB2,0xE9,0x87,
	0x70,0x28,0xAE,0x8F,0xD1,0xF5,0x74,0x1A,
	0xAA,0x37,0x80,0xBB,0xF3,0x22,0x1D,0xDE,
	0xA6,0xC4,0xD2,0x5E,0x28,0x93,0xAC,0xB3,
	0x22,0x07,0x81,0x5A,0xE4,0xB7,0x1A,0xAD,
	0xDC,0xCE,0x05,0xE7,0x07,0xBD,0xF5,0x84,
	0x26,0x1D,0x39,0x2C,0xB3,0xBA,0xA5,0x85,
	0xB4,0xF7,0x0F,0x72,0xFB,0x04,0xF0,0xDC,
	0x95,0xBA,0xA9,0x4E,0x87,0x36,0xF2,0x89,
	0xD4,0x07,0x3A,0xF1,0x5A,0x17,0x82,0x0E,
	0xEF,0x6F,0xAF,0xA7,0x66,0x1A,0x7E,0x89,
	0xC1,0x97,0xF5,0x58,0x74,0x8A,0x20,0xE7,
	0x43,0x34,0xCF,0xDA,0x22,0xC4,0x86,0xC8,
	0x08,0xD7,0xB4,0xFB,0x62,0x9D,0x08,0x85
	};

static unsigned char cbc_key [8]={0x01,0x23,0x45,0x67,0x89,0xab,0xcd,0xef};
static unsigned char cbc3_key[8]={0xf0,0xe1,0xd2,0xc3,0xb4,0xa5,0x96,0x87};
static unsigned char cbc_iv  [8]={0xfe,0xdc,0xba,0x98,0x76,0x54,0x32,0x10};
static unsigned char cbc3_iv [8]={0x0f,0x1e,0x2d,0x3c,0x4b,0x5a,0x69,0x78};
static unsigned char cbc_data[40]="7654321 Now is the time for ";

static unsigned char cbc_ok[32]={
	0xcc,0xd1,0x73,0xff,0xab,0x20,0x39,0xf4,
	0xac,0xd8,0xae,0xfd,0xdf,0xd8,0xa1,0xeb,
	0x46,0x8e,0x91,0x15,0x78,0x88,0xba,0x68,
	0x1d,0x26,0x93,0x97,0xf7,0xfe,0x62,0xb4};

static unsigned char cbc3_ok[32]={
	0x06,0x25,0x78,0x00,0x7F,0xE3,0xC6,0xC4,
	0x67,0xB3,0x72,0x07,0xF7,0x09,0xF4,0x38,
	0x14,0xCD,0x2B,0x1E,0x53,0x11,0x3F,0x3B,
	0x46,0x99,0xD1,0x21,0x8E,0xD3,0x87,0xA4};

static unsigned char pcbc_ok[32]={
	0xcc,0xd1,0x73,0xff,0xab,0x20,0x39,0xf4,
	0x6d,0xec,0xb4,0x70,0xa0,0xe5,0x6b,0x15,
	0xae,0xa6,0xbf,0x61,0xed,0x7d,0x9c,0x9f,
	0xf7,0x17,0x46,0x3b,0x8a,0xb3,0xcc,0x88};

static unsigned char cksum_ok[8]={
	0x1d,0x26,0x93,0x97,0xf7,0xfe,0x62,0xb4};

static unsigned char cfb_key[8]={0x01,0x23,0x45,0x67,0x89,0xab,0xcd,0xef};
static unsigned char cfb_iv[8]={0x12,0x34,0x56,0x78,0x90,0xab,0xcd,0xef};
static unsigned char cfb_buf1[40],cfb_buf2[40],cfb_tmp[8];
static unsigned char plain[24]=
	{
	0x4e,0x6f,0x77,0x20,0x69,0x73,
	0x20,0x74,0x68,0x65,0x20,0x74,
	0x69,0x6d,0x65,0x20,0x66,0x6f,
	0x72,0x20,0x61,0x6c,0x6c,0x20
	};
static unsigned char cfb_cipher8[24]= {
	0xf3,0x1f,0xda,0x07,0x01,0x14, 0x62,0xee,0x18,0x7f,0x43,0xd8,
	0x0a,0x7c,0xd9,0xb5,0xb0,0xd2, 0x90,0xda,0x6e,0x5b,0x9a,0x87 };
static unsigned char cfb_cipher16[24]={
	0xF3,0x09,0x87,0x87,0x7F,0x57, 0xF7,0x3C,0x36,0xB6,0xDB,0x70,
	0xD8,0xD5,0x34,0x19,0xD3,0x86, 0xB2,0x23,0xB7,0xB2,0xAD,0x1B };
static unsigned char cfb_cipher32[24]={
	0xF3,0x09,0x62,0x49,0xA4,0xDF, 0xA4,0x9F,0x33,0xDC,0x7B,0xAD,
	0x4C,0xC8,0x9F,0x64,0xE4,0x53, 0xE5,0xEC,0x67,0x20,0xDA,0xB6 };
static unsigned char cfb_cipher48[24]={
	0xF3,0x09,0x62,0x49,0xC7,0xF4, 0x30,0xB5,0x15,0xEC,0xBB,0x85,
	0x97,0x5A,0x13,0x8C,0x68,0x60, 0xE2,0x38,0x34,0x3C,0xDC,0x1F };
static unsigned char cfb_cipher64[24]={
	0xF3,0x09,0x62,0x49,0xC7,0xF4, 0x6E,0x51,0xA6,0x9E,0x83,0x9B,
	0x1A,0x92,0xF7,0x84,0x03,0x46, 0x71,0x33,0x89,0x8E,0xA6,0x22 };

static unsigned char ofb_key[8]={0x01,0x23,0x45,0x67,0x89,0xab,0xcd,0xef};
static unsigned char ofb_iv[8]={0x12,0x34,0x56,0x78,0x90,0xab,0xcd,0xef};
static unsigned char ofb_buf1[24],ofb_buf2[24],ofb_tmp[8];
static unsigned char ofb_cipher[24]=
	{
	0xf3,0x09,0x62,0x49,0xc7,0xf4,0x6e,0x51,
	0x35,0xf2,0x4a,0x24,0x2e,0xeb,0x3d,0x3f,
	0x3d,0x6d,0x5b,0xe3,0x25,0x5a,0xf8,0xc3
	};

unsigned long cbc_cksum_ret=0xB462FEF7;
unsigned char cbc_cksum_data[8]={0x1D,0x26,0x93,0x97,0xf7,0xfe,0x62,0xb4};

char *malloc();
char *pt();

main()
	{
	int i,j;
	des_cblock in,out,outin,iv3a,iv3b;
	des_key_schedule ks,ks2;
	unsigned char cbc_in[40],cbc_out[40];
	unsigned long cs;
	unsigned char qret[4][4],cret[8];
	unsigned long lqret[4];
	char *str;

	printf("Doing ecb\n");
	for (i=0; i<NUM_TESTS; i++)
		{
		if ((j=key_sched((C_Block *)(key_data[i]),ks)) != 0)
			printf("Key error %2d:%d\n",i+1,j);
		bcopy(plain_data[i],in,8);
		bzero(out,8);
		bzero(outin,8);
		des_ecb_encrypt((C_Block *)in,(C_Block *)out,ks,DES_ENCRYPT);
		des_ecb_encrypt((C_Block *)out,(C_Block *)outin,ks,DES_DECRYPT);

		if (bcmp(out,cipher_data[i],8) != 0)
			{
			printf("Encryption error %2d\nk=%s p=%s o=%s act=%s\n",
				i+1,pt(key_data[i]),pt(in),pt(cipher_data[i]),
				pt(out));
			}
		if (bcmp(in,outin,8) != 0)
			{
			printf("Decryption error %2d\nk=%s p=%s o=%s act=%s\n",
				i+1,pt(key_data[i]),pt(out),pt(in),pt(outin));
			}
		}

	printf("Doing triple ecb\n");
	for (i=0; i<(NUM_TESTS-1); i++)
		{
		if ((j=key_sched((C_Block *)(key_data[i]),ks)) != 0)
			printf("Key error %2d:%d\n",i+1,j);
		if ((j=key_sched((C_Block *)(key_data[i+1]),ks2)) != 0)
			printf("Key error %2d:%d\n",i+2,j);
		bcopy(plain_data[i],in,8);
		bzero(out,8);
		bzero(outin,8);
		des_3ecb_encrypt((C_Block *)in,(C_Block *)out,ks,ks2,
			DES_ENCRYPT);
		des_3ecb_encrypt((C_Block *)out,(C_Block *)outin,ks,ks2,
			DES_DECRYPT);

		if (bcmp(out,cipher_3ecb[i],8) != 0)
			{
			printf("Encryption error %2d\nk=%s p=%s o=%s act=%s\n",
				i+1,pt(key_data[i]),pt(in),pt(cipher_3ecb[i]),
				pt(out));
			}
		if (bcmp(in,outin,8) != 0)
			{
			printf("Decryption error %2d\nk=%s p=%s o=%s act=%s\n",
				i+1,pt(key_data[i]),pt(out),pt(in),pt(outin));
			}
		}

	printf("Doing cbc\n");
	if ((j=key_sched((C_Block *)cbc_key,ks)) != 0)
		printf("Key error %d\n",j);
	bzero(cbc_out,40);
	bzero(cbc_in,40);
	des_cbc_encrypt((C_Block *)cbc_data,(C_Block *)cbc_out,
		(long)strlen(cbc_data)+1,ks,(C_Block *)cbc_iv,DES_ENCRYPT);
	if (bcmp(cbc_out,cbc_ok,32) != 0)
		printf("cbc_encrypt encrypt error\n");
	des_cbc_encrypt((C_Block *)cbc_out,(C_Block *)cbc_in,
		(long)strlen(cbc_data)+1,ks,(C_Block *)cbc_iv,DES_DECRYPT);
	if (bcmp(cbc_in,cbc_data,32) != 0)
		printf("cbc_encrypt decrypt error\n");

	printf("Doing triple cbc\n");
	if ((j=key_sched((C_Block *)cbc_key,ks)) != 0)
		printf("Key error %d\n",j);
	if ((j=key_sched((C_Block *)cbc3_key,ks2)) != 0)
		printf("Key error %d\n",j);
	bcopy(cbc_iv,iv3a,sizeof(cbc_iv));
	bcopy(cbc3_iv,iv3b,sizeof(cbc_iv));
	bzero(cbc_out,40);
	bzero(cbc_in,40);
	des_3cbc_encrypt((C_Block *)cbc_data,(C_Block *)cbc_out,
		(long)strlen(cbc_data)+1,ks,ks2,(C_Block *)iv3a,
		(C_Block *)iv3b,DES_ENCRYPT);
	if (bcmp(cbc_out,cbc3_ok,(strlen(cbc_data)+7)/8*8) != 0)
		printf("des_3cbc_encrypt encrypt error\n");
	bcopy(cbc_iv,iv3a,sizeof(cbc_iv));
	bcopy(cbc3_iv,iv3b,sizeof(cbc_iv));
	des_3cbc_encrypt((C_Block *)cbc_out,(C_Block *)cbc_in,
		(long)strlen(cbc_data)+1,ks,ks2,(C_Block *)iv3a,
		(C_Block *)iv3b,DES_DECRYPT);
	if (bcmp(cbc_in,cbc_data,strlen(cbc_data)+1) != 0)
		printf("des_3cbc_encrypt decrypt error\n");

	printf("Doing pcbc\n");
	if ((j=key_sched((C_Block *)cbc_key,ks)) != 0)
		printf("Key error %d\n",j);
	bzero(cbc_out,40);
	bzero(cbc_in,40);
	des_pcbc_encrypt((C_Block *)cbc_data,(C_Block *)cbc_out,
		(long)strlen(cbc_data)+1,ks,(C_Block *)cbc_iv,DES_ENCRYPT);
	if (bcmp(cbc_out,pcbc_ok,32) != 0)
		printf("pcbc_encrypt encrypt error\n");
	des_pcbc_encrypt((C_Block *)cbc_out,(C_Block *)cbc_in,
		(long)strlen(cbc_data)+1,ks,(C_Block *)cbc_iv,DES_DECRYPT);
	if (bcmp(cbc_in,cbc_data,32) != 0)
		printf("pcbc_encrypt decrypt error\n");

	printf("Doing ");
	printf("cfb8 ");
	cfb_test(8,cfb_cipher8);
	printf("cfb16 ");
	cfb_test(16,cfb_cipher16);
	printf("cfb32 ");
	cfb_test(32,cfb_cipher32);
	printf("cfb48 ");
	cfb_test(48,cfb_cipher48);
	printf("cfb64 ");
	cfb_test(64,cfb_cipher64);

	bcopy(cfb_iv,cfb_tmp,sizeof(cfb_iv));
	for (i=0; i<sizeof(plain); i++)
		des_cfb_encrypt(&(plain[i]),&(cfb_buf1[i]),
			8,(long)1,ks,(C_Block *)cfb_tmp,DES_ENCRYPT);
	if (bcmp(cfb_cipher8,cfb_buf1,sizeof(plain)) != 0)
		printf("cfb_encrypt small encrypt error\n");

	bcopy(cfb_iv,cfb_tmp,sizeof(cfb_iv));
	for (i=0; i<sizeof(plain); i++)
		des_cfb_encrypt(&(cfb_buf1[i]),&(cfb_buf2[i]),
			8,(long)1,ks,(C_Block *)cfb_tmp,DES_DECRYPT);
	if (bcmp(plain,cfb_buf2,sizeof(plain)) != 0)
		printf("cfb_encrypt small decrypt error\n");
	printf("done\n");

	printf("Doing ofb\n");
	key_sched((C_Block *)ofb_key,ks);
	bcopy(ofb_iv,ofb_tmp,sizeof(ofb_iv));
	des_ofb_encrypt(plain,ofb_buf1,64,(long)sizeof(plain)/8,ks,
		(C_Block *)ofb_tmp);
	if (bcmp(ofb_cipher,ofb_buf1,sizeof(ofb_buf1)) != 0)
		printf("ofb_encrypt encrypt error\n");
	bcopy(ofb_iv,ofb_tmp,sizeof(ofb_iv));
	des_ofb_encrypt(ofb_buf1,ofb_buf2,64,(long)sizeof(ofb_buf1)/8,ks,
		(C_Block *)ofb_tmp);
	if (bcmp(plain,ofb_buf2,sizeof(ofb_buf2)) != 0)
		printf("ofb_encrypt decrypt error\n");

	printf("Doing cbc_cksum\n");
	key_sched((C_Block *)cbc_key,ks);
	cs=cbc_cksum((C_Block *)cbc_data,(C_Block *)cret,
		(long)strlen(cbc_data),ks,(C_Block *)cbc_iv);
	if (cs != cbc_cksum_ret)
		printf("bad return value (%08X), should be %08X\n",
			cs,cbc_cksum_ret);
	if (bcmp(cret,cbc_cksum_data,8) != 0)
		printf("bad cbc_cksum block returned\n");

	printf("Doing quad_cksum\n");
	cs=quad_cksum((C_Block *)cbc_data,(C_Block *)qret,
		(long)strlen(cbc_data),2,(C_Block *)cbc_iv);
	for (i=0; i<4; i++)
		{
		lqret[i]=0;
		bcopy(&(qret[i][0]),&(lqret[i]),4);
		}
	{ /* Big-endian fix */
	static unsigned long l=1;
	static unsigned char *c=(unsigned char *)&l;
	unsigned long ll;

	if (!c[0])
		{
		ll=lqret[0]^lqret[3];
		lqret[0]^=ll;
		lqret[3]^=ll;
		ll=lqret[1]^lqret[2];
		lqret[1]^=ll;
		lqret[2]^=ll;
		}
	}
	if (cs != 0x70d7a63a)
		printf("quad_cksum error, ret %08x should be 70d7a63a\n",cs);
	if (lqret[0] != 0x327eba8d)
		printf("quad_cksum error, out[0] %08x is not %08x\n",
			lqret[0],0x327eba8d);
	if (lqret[1] != 0x201a49cc)
		printf("quad_cksum error, out[1] %08x is not %08x\n",
			lqret[1],0x201a49cc);
	if (lqret[2] != 0x70d7a63a)
		printf("quad_cksum error, out[2] %08x is not %08x\n",
			lqret[2],0x70d7a63a);
	if (lqret[3] != 0x501c2c26)
		printf("quad_cksum error, out[3] %08x is not %08x\n",
			lqret[3],0x501c2c26);

	printf("input word alignment test");
	for (i=0; i<4; i++)
		{
		printf(" %d",i);
		des_cbc_encrypt((C_Block *)&(cbc_out[i]),(C_Block *)cbc_in,
			(long)strlen(cbc_data)+1,ks,(C_Block *)cbc_iv,
			DES_ENCRYPT);
		}
	printf("\noutput word alignment test");
	for (i=0; i<4; i++)
		{
		printf(" %d",i);
		des_cbc_encrypt((C_Block *)cbc_out,(C_Block *)&(cbc_in[i]),
			(long)strlen(cbc_data)+1,ks,(C_Block *)cbc_iv,
			DES_ENCRYPT);
		}
	printf("\n");
	printf("fast crypt test ");
	str=crypt("testing","ef");
	if (strcmp("efGnQx2725bI2",str) != 0)
		printf("fast crypt error, %s should be efGnQx2725bI2\n",str);
	str=crypt("bca76;23","yA");
	if (strcmp("yA1Rp/1hZXIJk",str) != 0)
		printf("fast crypt error, %s should be yA1Rp/1hZXIJk\n",str);
	printf("\n");
	exit(0);
	}

char *pt(p)
unsigned char *p;
	{
	static char bufs[10][20];
	static int bnum=0;
	char *ret;
	int i;
	static char *f="0123456789ABCDEF";

	ret= &(bufs[bnum++][0]);
	bnum%=10;
	for (i=0; i<8; i++)
		{
		ret[i*2]=f[(p[i]>>4)&0xf];
		ret[i*2+1]=f[p[i]&0xf];
		}
	ret[16]='\0';
	return(ret);
	}
	
int cfb_test(bits,cfb_cipher)
int bits;
char *cfb_cipher;
	{
	des_key_schedule ks;
	int i;

	key_sched((C_Block *)cfb_key,ks);
	bcopy(cfb_iv,cfb_tmp,sizeof(cfb_iv));
	des_cfb_encrypt(plain,cfb_buf1,bits,(long)sizeof(plain),ks,
		(C_Block *)cfb_tmp,DES_ENCRYPT);
	if (bcmp(cfb_cipher,cfb_buf1,sizeof(plain)) != 0)
		{
		printf("cfb_encrypt encrypt error\n");
		for (i=0; i<24; i+=8)
			printf("%s\n",pt(&(cfb_buf1[i])));
		}
	bcopy(cfb_iv,cfb_tmp,sizeof(cfb_iv));
	des_cfb_encrypt(cfb_buf1,cfb_buf2,bits,(long)sizeof(plain),ks,
		(C_Block *)cfb_tmp,DES_DECRYPT);
	if (bcmp(plain,cfb_buf2,sizeof(plain)) != 0)
		{
		printf("cfb_encrypt decrypt error\n");
		for (i=0; i<24; i+=8)
			printf("%s\n",pt(&(cfb_buf1[i])));
		}
	}
