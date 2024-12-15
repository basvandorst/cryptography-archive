#include "secure.h"
#include "arithmetic.h"

/***************************************************************
 *
 * Procedure usage
 *
 ***************************************************************/
#ifdef __STDC__

static void usage(
	int	  help
)

#else

static void usage(
	help
)
int	  help;

#endif

{

	aux_fprint_version(stderr);

        fprintf(stderr, "mk_trace: Produce a trace file for arithmetic operations\n\n\n");
	fprintf(stderr, "Description:\n\n");
	fprintf(stderr, "'mk_trace' traces the first n operations add, sub, mult and div\n");
	fprintf(stderr, "of a key generation process. This can be uses to detect errors in arithmetics.\n");
	fprintf(stderr, "To use 'mk_trace' SecuDE must be compiled with -DARITHMETIC_TEST (see CONFIG.make).\n\n\n");

        fprintf(stderr, "usage:\n\n");
	fprintf(stderr, "mk_trace [-o <tracefile>] [-n <number>] [-r <random>] [-h]\n\n");
 

        if(help == LONG_HELP) {
        	fprintf(stderr, "with:\n\n");
        	fprintf(stderr, "-o <tracefile>   name of the trace file (default: arithmetic_trace)\n");
        	fprintf(stderr, "-n <number>      number of arithmetics to trace (default: 1000)\n");
		fprintf(stderr, "-r <random>      a random number can be set to produce another sequence \n");
        	fprintf(stderr, "-h               write this help text\n");
        }

        if(MF_check) MF_fprint(stderr);

        exit(-1);                                /* IRREGULAR EXIT FROM PKLIST */
}

/***************************************************************
 *
 * Procedure main
 *
 ***************************************************************/
#ifdef __STDC__

int main(
	int	  cnt,
	char	**p
)

#else

int main(
	cnt,
	p
)
int	  cnt;
char	**p;

#endif
{
#ifdef ARITHMETIC_TEST

	BitString 	*sk, *pk;
	L_NUMBER 	ln[2];
	int		value;
	char		*trace_file = "trace.trc";
	char	        opt;

	extern char	*optarg;

/* set the random number generator to the following seed */
	ln[0] = 1;
	ln[1] = 12345678;



	arithmetic_trace_counter = 1000;
	while ( (opt = getopt(cnt, p, "o:n:r:h")) != -1 ) {
		switch (opt) {
		case 'o':
			trace_file = optarg;
			break;
		case 'n':
			value = atoi(optarg);
			if(value <= 0) usage(SHORT_HELP);
			arithmetic_trace_counter = value;
			break;
		case 'r':
			value = atoi(optarg);
			if(value <= 0) usage(SHORT_HELP);
			ln[1] = value;
			break;
		case 'h':
			usage(LONG_HELP);
			continue;
		default:
		case '?':
			usage(SHORT_HELP);
		}
	}

	sec_init_random(ln);


	sec_verbose = 1;


/* open the trace file */
	arithmetic_trace = fopen(trace_file, "w");
	if(!arithmetic_trace) exit(1);


/* call a key generation which uses many operations (enough for a 30 MB trace file) */
	rsa_gen_key( 512, &sk, &pk);


/* close the trace file */
	fclose(arithmetic_trace);


#else
	usage(LONG_HELP);
#endif
}
