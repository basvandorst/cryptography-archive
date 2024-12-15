/*
 *                                        _
 * Copyright (c) 1990,1991,1992,1993 Stig Ostholm.
 * All Rights Reserved
 *
 *
 * The author takes no responsibility of actions caused by the use of this
 * software and does not guarantee the correctness of the functions.
 *
 * This software may be freely distributed and modified for non-commercial use
 * as long as the copyright notice is kept. If you modify any of the files,
 * pleas add a comment indicating what is modified and who made the
 * modification.
 *
 * If you intend to use this software for commercial purposes, contact the
 * author.
 *
 * If you find any bugs or porting problems, please inform the author about
 * the problem and fixes (if there are any).
 *
 *
 * Additional restrictions due to national laws governing the use, import or
 * export of cryptographic software is the responsibility of the software user,
 * importer or exporter to follow.
 *
 *
 *                                              _
 *                                         Stig Ostholm
 *                                         Chalmers University of Technology
 *                                         Department of Computer Engineering
 *                                         S-412 96 Gothenburg
 *                                         Sweden
 *                                         ----------------------------------
 *                                         Email: ostholm@ce.chalmers.se
 *                                         Phone: +46 31 772 1703
 *                                         Fax:   +46 31 772 3663
 */

#include	<des.h>
#include	"config.h"

/*
 * The reference test examples has been collected from the following sources:
 *
 *
 * [1]		Cryptography: A New Dimension in Computer Data Security.
 *		Carl H. Meyer Stephen M. Matyas
 *		John Wiely & Sons, Inc. ISBN 0-471-04892-5
 *
 * [2]		The Standard Data Encryption Algorithm.
 *		Harry Katazan, Jr.
 *		Petrocelli Books, Inc. ISBN 0-89433-016-0
 */

typedef struct	des_verify_data {
			int		mode;	 /* Encryption or decryption */
			des_cblock	key;	 /* 64 bit DES key	     */
			des_cblock	ivec;    /* Init vector		     */
			des_cblock	in_data; /* Input data		     */
			des_cblock	out_data;/* Output data		     */
		} des_verify_data;

/*
 * dea_data
 *
 *	This list contains reference data for the des_dea function.
 */
const des_verify_data	dea_data[] = {
		{    /* [1], page 167 */
			DES_ENCRYPT,
			{ 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
			{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
			{ 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01 },
			{ 0x95, 0x8e, 0x6e, 0x62, 0x7a, 0x05, 0x55, 0x7b }
		}, { /* [1], page 167 */
			DES_DECRYPT,
			{ 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
			{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
			{ 0x85, 0x8e, 0x6e, 0x62, 0x7a, 0x05, 0x55, 0x7b },
			{ 0x8d, 0x48, 0x93, 0xc2, 0x96, 0x6c, 0xc2, 0x11 }
		}, { /* [1], page 167 */
			DES_DECRYPT,
			{ 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
			{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
			{ 0x95, 0x8e, 0x6e, 0x62, 0x7a, 0x05, 0x55, 0x7b },
			{ 0x6d, 0x4b, 0x94, 0x53, 0x76, 0x72, 0x53, 0x95 }
		}, { /* [2], page 78 - 86 */
			DES_ENCRYPT,
			{ 0x5b, 0x5a, 0x57, 0x67, 0x6a, 0x56, 0x67, 0x6e },
			{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
			{ 0x67, 0x5a, 0x69, 0x67, 0x5e, 0x5a, 0x6b, 0x5a },
			{ 0x97, 0x4a, 0xff, 0xbf, 0x86, 0x02, 0x2d, 0x1f }
		}
	};
