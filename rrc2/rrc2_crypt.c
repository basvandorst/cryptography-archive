/****************************************************************************
*                                                                            *
*                   Simple Sample Encryption Program using                   *
*                       RRC.2 - Ron Rivest Cipher Nr. 2                      *
*                                                                            *
*       Implementation done on 15.02.96 by Daniel Vogelheim.                 *
*                                                                            *
*       This document is placed into the public domain.                      *
*       I'd appriciate if my name was mentioned in derivative works.         *
*       (I'd be suprised if anybody wanted to derive anything from           *
*       this program, though.)                                               *
*                                                                            *
 ****************************************************************************/

/* Toggle encryption/decrytion by defining symbol DECRYPT */

#ifndef DECRYPT
	#define CRYPT		"encrypt"
	#define RRC2_CRYPT	rrc2_encrypt
#else
	#define CRYPT		"decrypt"
	#define RRC2_CRYPT	rrc2_decrypt
#endif

#include <stdio.h>
#include "rrc2.h"
#include <assert.h>
#include <string.h>

#define FNAME_LENGTH		1000 /* enough for a small letter */
#define BUF_SIZE		1024
#define FILE_ENDING		".rrc2"

int main(void)
{
	char	in_filename[FNAME_LENGTH];
	char	out_filename[FNAME_LENGTH];
	char	key[129];
	FILE	*in_file, *out_file;
	byte	data[BUF_SIZE];
	int	i,i2;
	#ifndef DECRYPT       /* just to avoid the warning... */
		int	i3;
	#endif
	
	printf("Simple Sample File %sion Program using RRC.2.\n",CRYPT);

	/* Check spec conformance, so others can decrypt! */	
	rrc2_test();
	
	/* Buffer must me multiple of BLOCK_SIZE */
	assert(BUF_SIZE%BLOCK_SIZE==0);

	printf("Please enter name of file to %s: ",CRYPT);
	scanf("%s",in_filename);
	printf("Please enter key to %s %s with (max. 128 characters): ",CRYPT,in_filename);
	scanf("%s",key);
	if (strlen(key)<5) {
		fprintf(stderr,"%d characters! What a whimpy key!\n",strlen(key));
	}
	
	strcpy(out_filename,in_filename);
	#ifdef DECRYPT
		strcat(in_filename,FILE_ENDING);
	#else
		strcat(out_filename,FILE_ENDING);
	#endif
	
	in_file = fopen(in_filename,"rb");
	if (in_file == NULL) {
		fprintf(stderr,"Error while opening input file %s!\n",in_filename);
		return 1;
	}
	out_file = fopen(out_filename,"wb");
	if (out_file == NULL) {
		fprintf(stderr,"Error while opening output file %s for writing!\n",out_filename);
		return 1;
	}
	
	printf("Thank you. %sting %s to %s ...",CRYPT,in_filename,out_filename);

	while (~ feof(in_file) ) {
		i2 = fread((char *)data,1,sizeof(data),in_file);
		if (i2 == sizeof(data)) {
			/* OK -> crypt */
			RRC2_CRYPT(data,sizeof(data),key,strlen(key));
			i2 = fwrite((char *)data,1,sizeof(data),out_file);
			if (i2 != sizeof(data)) {
				fprintf(stderr,"\nError while writing into file %s!\n",out_filename);
				return 1;
			}
		} else {
			/* End of File or error */
			if (feof(in_file)) {
				/* encrypt remainder */
				#ifndef DECRYPT
					if ( (i2 % BLOCK_SIZE) != 0) {
						fprintf(stderr,"\nWarning! Data will be 0-padded to become multiple of block size (%d bytes).\n",BLOCK_SIZE);
						i3 = BLOCK_SIZE - (i2 % BLOCK_SIZE);
						assert((i3>0)&&(i3<BLOCK_SIZE));
						for(i = 0;i < i3;i++) {
							data[i2+i] = 0;
						}
						i2 += i3;
					}
				#endif
				assert((i2 % BLOCK_SIZE) == 0);
				RRC2_CRYPT(data,i2,key,strlen(key));
				i = fwrite((char *)data,1,i2,out_file);
				if (i != i2) {
					fprintf(stderr,"\nError while writing in file %s!\n",out_filename);
					return 1;
				}
			} else {
				/* error */
				assert(ferror(in_file)!=0);
				fprintf(stderr,"\nError occured while reading from %s!\n",in_filename);
			}
			
			break;
		}	
	}
	
	fclose(in_file);
	fclose(out_file);
	
	printf(" done.\n");
	return 0;
}
/* FINI: rrc2_crypt.c */
