/*
 * This software may be freely distributed an modified without any restrictions
 * from the author.
 * Additional restrictions due to national laws governing the use, import or
 * export of cryptographic software is the responsibility of the software user,
 * importer or exporter to follow.
 *
 *					     _
 *					Stig Ostholm
 *					Department of Computer Engineering
 *					Chalmers University of Technology
 */

/*
 * list of semiweak keys.
 */

#define NO_SEMI_WEAK_KEYS	12

CONST des_cblock	semiweak_key[NO_SEMI_WEAK_KEYS] = {
#ifdef DES_LSB_FIRST
			{ 0x07, 0x7f, 0x07, 0x7f, 0x8f, 0x7f, 0x8f, 0x7f },
			{ 0x7f, 0x07, 0x7f, 0x07, 0x7f, 0x8f, 0x7f, 0x8f },
			{ 0xf8, 0x7f, 0xf8, 0x7f, 0x70, 0x7f, 0x70, 0x7f },
			{ 0x80, 0x7f, 0x80, 0x7f, 0x80, 0x7f, 0x80, 0x7f },
			{ 0xf8, 0x07, 0xf8, 0x07, 0x70, 0x8f, 0x70, 0x8f },
			{ 0x80, 0x07, 0x80, 0x07, 0x80, 0x8f, 0x80, 0x8f },
			{ 0x7f, 0xf8, 0x7f, 0xf8, 0x7f, 0x70, 0x7f, 0x70 },
			{ 0x07, 0xf8, 0x07, 0xf8, 0x8f, 0x70, 0x8f, 0x70 },
			{ 0x7f, 0x80, 0x7f, 0x80, 0x7f, 0x80, 0x7f, 0x80 },
			{ 0x07, 0x80, 0x07, 0x80, 0x8f, 0x80, 0x8f, 0x80 },
			{ 0x80, 0xf8, 0x80, 0xf8, 0x80, 0x70, 0x80, 0x70 },
			{ 0xf8, 0x80, 0xf8, 0x80, 0x70, 0x80, 0x70, 0x80 }
#else
			{ 0xe0, 0xfe, 0xe0, 0xfe, 0xf1, 0xfe, 0xf1, 0xfe },
			{ 0xfe, 0xe0, 0xfe, 0xe0, 0xfe, 0xf1, 0xfe, 0xf1 },
			{ 0x1f, 0xfe, 0x1f, 0xfe, 0x0e, 0xfe, 0x0e, 0xfe },
			{ 0x01, 0xfe, 0x01, 0xfe, 0x01, 0xfe, 0x01, 0xfe },
			{ 0x1f, 0xe0, 0x1f, 0xe0, 0x0e, 0xf1, 0x0e, 0xf1 },
			{ 0x01, 0xe0, 0x01, 0xe0, 0x01, 0xf1, 0x01, 0xf1 },
			{ 0xfe, 0x1f, 0xfe, 0x1f, 0xfe, 0x0e, 0xfe, 0x0e },
			{ 0xe0, 0x1f, 0xe0, 0x1f, 0xf1, 0x0e, 0xf1, 0x0e },
			{ 0xfe, 0x01, 0xfe, 0x01, 0xfe, 0x01, 0xfe, 0x01 },
			{ 0xe0, 0x01, 0xe0, 0x01, 0xf1, 0x01, 0xf1, 0x01 },
			{ 0x01, 0x1f, 0x01, 0x1f, 0x01, 0x0e, 0x01, 0x0e },
			{ 0x1f, 0x01, 0x1f, 0x01, 0x0e, 0x01, 0x0e, 0x01 }
#endif
		};
