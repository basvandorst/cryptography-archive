
#include "ext.h"
#include "frig.h"

byte	conv [256];
byte	used [256];
byte	ones [256];

main()

{
	int	i, j, l, k, m;

	srandom (time(0) + getpid());

	printf ("\n\n#include \"ext.h\"\n#include \"frig.h\"\n\n");
	printf("#ifdef FRIG\n");

	for (i = 0; i < 256; i++) {
		k = 0x80;
		j = 0;
		for (l = 0; l < 8; l++) {
			if (i & k)
				j++;
			k >>= 1;	
		}
		ones [i] = j;
		used [i] = 0;
	}

	printf ("byte conv_array [CONV_ARRAYS][256] = {\n");

for (m = 0; m < CONV_ARRAYS; m++) {
	memset (used, 0, 256);

	printf("\t{\n");
	for (i = 0; i < 256; i++) {
		j = 0;
		for (l = 0; l < 256; l++) {
			if (!used [l] && ones[l] == ones [i])
				j++;
		}
		k = ((random ()  >> 2)% j);
		for (l = 0; l < 256; l++) {
			if (!used [l] && ones[l] == ones [i]) {
				if (!k) {
					used [l] = 1;
					conv [i] = l;
					break;
				}
				else
					k--;
			}
		}
	}

	k = 0;
	for (i = 0; i < 32; i++) {
		putchar('\t');
		for (j = 0; j < 8; j++) {
			printf("0x%02X,", conv [k++]);
		}
		putchar ('\n');
	}
	printf ("},\n\n");
}
printf("};\n#endif\n\n");
}

