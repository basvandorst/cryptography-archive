
#include <stdio.h>
#include <math.h>

#include "ext.h";

int	freq [256];
int	bits [256];
int	expect [256];

main ()

{
	int	c;
	long	ones = 0l;
	long	zeros = 0l;
	long	byte_count = 0l;
	byte	m;
	int	i;
	double	one_freq;
	double	prob;
	long	sqd = 0l;
	long	sqf = 0l;

	for (i = 0; i < 256; i++) {
		m = 0x80;
		while (m) {
			if (i & m)
				bits[i]++;
			m >>= 1;
		}
	}

	while ((c = getchar ()) != EOF) {
		freq [c]++;
		byte_count++;

		ones += bits [c];
		zeros += (8-bits[c]);
	}

	printf ("%d ones, %d zeros, %d%% ones.\n", ones, zeros,
		(100*ones)/(ones+zeros));

	one_freq = ((double)ones) / (double)(ones + zeros);

#define M 2048
	for (i = 0; i < 256; i++) {
		prob = 1.0;
		for (c = 0; c < bits [i]; c++) {
			prob *= one_freq;
		}
		for (; c < 8; c++) {
			prob *= (1.0-one_freq);
		}

		expect [i] = (int)(prob * (double)byte_count);

		sqd += (expect[i] - freq[i]) * (expect[i] - freq[i]);
		sqf += freq[i] * freq[i];
	}

	printf ("Frequencies of bytes : \n");
	for (i = 0; i < 256; i++) 
		printf ("Byte %02X, count %d - expected %d\n", i, freq[i],
			expect [i]);

	printf ("RMS difference = %f (total %ld), RMS frequency = %f\n", 
		sqrt (sqd/256.0),
		sqd,
		sqrt (sqf/256.0));
}

