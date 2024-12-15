/* smfpriv.h

   Private header file for the secure malloc and free DLL 
*/

#ifndef YARROW_SMF_PRIV_H

#define YARROW_SMF_PRIV_H


/* Structures */
typedef struct {
	DWORD OrgId;
	HANDLE hand;
	DWORD count;
	DWORD size;
} mminfo;

/* Forward declerations */
void SecureDelete(BYTE* start,DWORD size);

/* Constants */
/* Should have four random overwrites before and after */
#define NUMITER 27
static const BYTE ovr[NUMITER][3] = {	
	{0x55,0x55,0x55},
	{0xAA,0xAA,0xAA},
	{0x92,0x49,0x24},
	{0x49,0x24,0x92},
	{0x24,0x92,0x49},
	{0x00,0x00,0x00},
	{0x11,0x11,0x11},
	{0x22,0x22,0x22},
	{0x33,0x33,0x33},
	{0x44,0x44,0x44},
	{0x55,0x55,0x55},
	{0x66,0x66,0x66},
	{0x77,0x77,0x77},
	{0x88,0x88,0x88},
	{0x99,0x99,0x99},
	{0xAA,0xAA,0xAA},
	{0xBB,0xBB,0xBB},
	{0xCC,0xCC,0xCC},
	{0xDD,0xDD,0xDD},
	{0xEE,0xEE,0xEE},
	{0xFF,0xFF,0xFF},
	{0x92,0x49,0x24},
	{0x49,0x24,0x92},
	{0x24,0x92,0x49},
	{0x6D,0xB6,0xDB},
	{0xB6,0xDB,0x6D},
	{0xDB,0x6D,0xB6}
};



#endif
