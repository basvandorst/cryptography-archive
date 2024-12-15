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

#include	<stdio.h>
#include	<string.h>
#include	"vrfy.h"

#define v_FLAG 0x1L

static unsigned long	flags = 0L;

#define BITS_PER_BYTE (DES_BLOCK_BITS / DES_BLOCK_BYTES)


char	*prog;

static int	tests = 0;


int	cmp_cblock(
#ifdef __STDC__
	des_cblock	b1,
	des_cblock	b2)
#else
	b1, b2)
des_cblock	b1, b2;
#endif
{
	register int	err = 0;


	if (flags & v_FLAG)
		if (tests && ! (tests % 50))
#ifdef DES_DEA3
			printf("\n                    ");
#else  /* DES_DEA3 */
			printf("\n                   ");
#endif /* DES_DEA3 */
	if (memcmp(b1, b2, DES_BLOCK_BYTES)) {
		VOID putchar('E');
		fflush(stdout);
		err = 1;
		tests++;
	} else if (flags & v_FLAG) {
		VOID putchar('.');
		fflush(stdout);
		tests++;
	}
	return err;
}

#ifdef DES_DEA3
int	test_des_dea3(
#else  /* DES_DEA3 */
int	test_des_dea(
#endif /* DES_DEA3 */
#ifdef __STDC__ 
	void
#endif
)
{
	register int		errors, i, j, mode;
	des_cblock		buf, rbuf;
#ifdef DES_DEA3
	des_key_schedule	schedule[2];
	des_cblock		rkey[2];
#else  /* DES_DEA3 */
	des_key_schedule	schedule;
	des_cblock		rkey;
#endif /* DES_DEA3 */

	
	tests = errors = 0;

	if (flags & v_FLAG)
#ifdef DES_DEA3
		VOID fputs("Testing des_dea3(): ", stdout);
#else  /* DES_DEA3 */
		VOID fputs("Testing des_dea(): ", stdout);
#endif /* DES_DEA3 */

	/*
	 * Test the function against some reference examples.
	 */
	for (i = 0; i < (sizeof(dea_data) / sizeof(des_verify_data)); i++) {

		mode = dea_data[i].mode;

#ifdef DES_DEA3
		VOID des_sched((des_cblock *) dea_data[i].key, schedule[0]);
		VOID des_sched((des_cblock *) dea_data[i].key, schedule[1]);

		VOID des_dea3((des_cblock *) dea_data[i].in_data,
			      (des_cblock *) buf, schedule, mode);
#else  /* DES_DEA3 */
		VOID des_sched((des_cblock *) dea_data[i].key, schedule);

		VOID des_dea((des_cblock *) dea_data[i].in_data,
			     (des_cblock *) buf, schedule, mode);
#endif /* DES_DEA3 */

		errors += cmp_cblock(buf, dea_data[i].out_data);

		mode = (mode == DES_ENCRYPT) ? DES_DECRYPT : DES_ENCRYPT;

#ifdef DES_DEA3
		VOID des_dea3((des_cblock *) buf, (des_cblock *) buf,
			      schedule, mode);
#else  /* DES_DEA3 */
		VOID des_dea((des_cblock *) buf, (des_cblock *) buf,
			     schedule, mode);
#endif /* DES_DEA3 */

		errors += cmp_cblock(buf, dea_data[i].in_data);

	}

	/*
	 * Test all bits in an encryption/decryption block.
	 */
	DES_ZERO_CBLOCK(rbuf);
	for (i = 0; i < DES_BLOCK_BITS; i++) {

		j = i / BITS_PER_BYTE;
		rbuf[j] = 0x80 >> (i % BITS_PER_BYTE);

#ifdef DES_DEA3
		VOID des_dea3((des_cblock *) rbuf, (des_cblock *) buf,
			      schedule, DES_ENCRYPT);
		VOID des_dea3((des_cblock *) buf, (des_cblock *) buf,
			      schedule, DES_DECRYPT);
#else  /* DES_DEA3 */
		VOID des_dea((des_cblock *) rbuf, (des_cblock *) buf,
			     schedule, DES_ENCRYPT);
		VOID des_dea((des_cblock *) buf, (des_cblock *) buf,
			     schedule, DES_DECRYPT);
#endif /* DES_DEA3 */

		errors += cmp_cblock(rbuf, buf);
		rbuf[j] = 0x00;

	}

	/*
	 * Test a number of random generated keys and blocks.
	 */
	for (i = 0; i < 1000; i++) {

#ifdef DES_DEA3
		VOID des_random_key((des_cblock *) rkey[0]);
		VOID des_random_key((des_cblock *) rkey[1]);
		VOID des_sched((des_cblock *) rkey[0], schedule[0]);
		VOID des_sched((des_cblock *) rkey[1], schedule[1]);
#else  /* DES_DEA3 */
		VOID des_random_key((des_cblock *) rkey);
		VOID des_sched((des_cblock *) rkey, schedule);
#endif /* DES_DEA3 */

		VOID des_random_cblock((des_cblock *) rbuf);

#ifdef DES_DEA3
		VOID des_dea3((des_cblock *) rbuf, (des_cblock *) buf,
			      schedule, DES_ENCRYPT);
		VOID des_dea3((des_cblock *) buf, (des_cblock *) buf,
			      schedule, DES_DECRYPT);
#else  /* DES_DEA3 */
		VOID des_dea((des_cblock *) rbuf, (des_cblock *) buf,
			     schedule, DES_ENCRYPT);
		VOID des_dea((des_cblock *) buf, (des_cblock *) buf,
			     schedule, DES_DECRYPT);
#endif /* DES_DEA3 */

		errors += cmp_cblock(rbuf, buf);

	}

	if (flags & v_FLAG) {
		VOID puts(" done.");
		VOID printf("%s passed %d tests with %d %s.\n\n",
#ifdef DES_DEA3
			    "des_dea3()",
#else  /* DES_DEA3 */
			    "des_dea()",
#endif /* DES_DEA3 */
			    tests, errors,
			    ((errors == 1) ? "error" : "errors"));
	}

	return errors;
#undef CMP_CBLOCK
}


main(argc, argv)
int	argc;
char	*argv[];
{
	register int	o, errors;
	extern int	optind;


	prog = (prog = strrchr(*argv, '/')) ? (prog + 1) : *argv;

        while ((o = getopt(argc, argv, "v")) != EOF)
                switch (o) {
                case 'v':
                        flags |= v_FLAG;
                        break;
                case '?':
                default:
                        goto usage;
                }
	if (optind != argc)
		goto usage;


#ifdef DES_DEA3
	errors = test_des_dea3();
#else  /* DES_DEA3 */
	errors = test_des_dea();
#endif /* DES_DEA3 */

	/*
	 * INSERT TESTS FOR THE OTHER FUNCTIONS HERE.
	 *
	 * errors += test_???();
	 */

	if (flags & v_FLAG)
		VOID printf("The implementation passed all tests with %d %s.\n",
			    errors, ((errors == 1) ? "error" : "errors"));

	exit(errors);

usage:
	VOID fprintf(stderr, "Usage: %s [-v]\n", prog);
	exit(1);
}
