/*
 * by David Vincenzetti, vince@ghost.dsi.unimi.it
 *   - this is what I call a simple program -
 * modified by Bill Davidsen, davidsen@crd.ge.com
 *   - enter with an index to the string # and a pointer to a buffer,
 *     return with the buffer filled.
 * slightly modified by vince@ghost
 *   - changed ENC and DEC macros
 */

#include <stdio.h>

#define ENC(x)	((((x - key1) ^ key2) - key3) ^ key4)

int	main(argc, argv)
int	argc;
char	*argv[];
{
	register	i, j, maxlen = 0;
	unsigned long	allkeys;
	unsigned char	key1, key2, key3, key4;

	if (argc < 2)
		fprintf(stderr, "usage: %s string [strings..]\n", argv[0]),
		exit(1);
	
	/*
	** Generate a random number. Yes, I know this is not random at all,
	** but we are just scrambling to prevent strings(1) examinations.
	*/
	srand(time(NULL));
	allkeys = 938221L * rand() * getpid() + time(NULL) + rand();
	key1 = allkeys & 0xff; allkeys >>= 8;
	key2 = allkeys & 0xff; allkeys >>= 8;
	key3 = allkeys & 0xff; allkeys >>= 8;
	key4 = allkeys & 0xff;
	
	printf("/* Automatically generated hidden strings */\n");

	/* get the length of the longest string and allocate a buffer */
	for (i = 0; i < argc; ++i)
		if ((j = strlen(argv[i])) > maxlen)
			maxlen = j;
	printf("/* internal storage buffer */\n");
	printf("static char internal_buf[%d];\n", maxlen+2);
	printf("#define DEC(x)\t((((x ^ key4) + key3) ^ key2) + key1)\n\n");

	printf("char *\n");
	printf("gethiddenstr(string_num)\n");
	printf("  int string_num;\n");
	printf("{\n");
	printf("\tunsigned char\tkey1 = %d;\n", key1);
	printf("\tunsigned char\tkey2 = %d;\n", key2);
	printf("\tunsigned char\tkey3 = %d;\n", key3);
	printf("\tunsigned char\tkey4 = %d;\n", key4);
	printf("\tchar *retval = internal_buf;\n");
	printf("\tchar *buffer_adrs = internal_buf;\n");
	printf("\n");
	printf("\tswitch (string_num) {\n");
	for (i = 1; i < argc; i++) {
		printf("\t  case %2d:  /* %-40.40s */\n", i, argv[i]);
		j = 0;
		while (argv[i][j])
			printf("\t\t*(buffer_adrs++) = DEC(%d);\n",
							ENC(argv[i][j++]));
		printf("\t\tbreak;\n");
		if (j > maxlen) maxlen = j;
	}
	printf("\t  default:  /* %-40s */\n", "Out of range index");
	printf("\t\tretval = (char *)0;\n");
	printf("\t}\n");
	printf("\t*buffer_adrs = '\\0';\n");
	printf("\treturn(retval);\n");
	printf("}\n");

	printf("\n");
	printf("#ifdef TEST_HIDE\n");
	printf("main() {\n");
	printf("\tint i;\n");
	printf("\tchar *ptr;\n");
	printf("\n");
	printf("\tfor (i = 1; (ptr = gethiddenstr(i)) != (char *)0; ++i) {\n");
	printf("\t\tprintf(\"String %%2d: %%s\\n\", i, ptr);\n");
	printf("\t}\n");
	printf("\texit(0);\n");
	printf("}\n");
	printf("#endif\n");

	exit(0);
}
