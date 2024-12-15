/* Basic Public-Domain Non-Copyrighted One Time Pad Program */
char *Usage="otp [-r] [-n newpad] [-c] [-s skipbytes] padfile [file]\nxor padfile with file or stdin\n-r - reuse padfile as many times as needed [disparaged]\n-p newpad - write unused portion of padfile to newpad\n";
char *Usage2="-c - output count of bytes read or skipped to stderr on completion\n-s skipbytes - skip that many bytes of pad file\nShortcomings: doesn't portably rewrite new pad onto old or overwrite file,\nand can't detect reuse of the pad\n";
char *permissions="While this is not a copyrighted program, you do not have my permission to use this program for initiation of force against other people\n";

#include <stdio.h>

main(argc, argv)
int argc; 
char **argv;
{
	register int c, p, n=0;
	extern char *optarg;
	extern int optind;
	FILE *pad, *newpad, *in;
	char *newpadname = 0;
	int reuseflag = 0;
	int skipbytes = 0;
	int outputcount = 0;

	/* getopt() is available public domain if your system doesn't have it.*/
	while ( (c=getopt(argc, argv, "crn:s:")) != -1) switch(c) {
	case 'c': outputcount++; break;
	case 'r': reuseflag++; break;
	case 'n': newpadname = optarg; 
		if ((newpad=fopen(optarg,"w"))==NULL) {
			fprintf(stderr,"OTP: Can't Open newpad %s\n", optarg);
			exit(98);
		}
		break;
	case 's': skipbytes = atoi(optarg); break;
	default:  fprintf(stderr, "%s%s", Usage, Usage2); exit(99);
	}

	if (optind==argc) {fprintf(stderr, "%s%s", Usage, Usage2); exit(99);}
	if (argv[optind][0]=='-' && argv[optind][1]=='\0') pad=stdin;
	else if ((pad=fopen(argv[optind],"r"))==NULL) {
		fprintf(stderr,"OTP: Can't Open %s\n", argv[optind]);
		exit(97);
	}
	if (skipbytes>0) {
		n = skipbytes;
		/* if can't fseek, have to do it the old-fashioned way */
		if (fseek(pad, (long) skipbytes, 0))
			while(skipbytes-->0) (void) getc(pad);
	}

	if ((++optind)==argc) in=stdin;
	else if ((in=fopen(argv[optind],"r"))==NULL) {
		fprintf(stderr,"OTP: Can't Open File %s\n", argv[optind]);
		exit(96);
	}

	/* read in the file, xor bytes with pad, output */
	/* this could perfectly well be a subroutine,
		but it's not obvious what is the correct use of 
		multiple files - so do them one at a time or use cat. */
	while ( (c=getc(in))!=EOF ) {
		n++;
		if ((p=getc(pad))==EOF ) {
			if (reuseflag) {
				if (fseek(pad, 0L,0)) {
					fprintf(stderr, "OTP: fseek(pad) failed!\n");
					exit(95);
				} else if ((p=getc(pad))==EOF) {
					fprintf(stderr, "OTP: rereading pad failed\n");
					exit(94);
				}
			} else {
				fprintf(stderr, "OTP: ran out of pad\n");
				exit(93);
			}
		}
		if (putchar(c^p)==EOF && ferror(stdout)) {
			fprintf(stderr, "OTP: stdout failed\n");
			exit(92);
		}
	}
	fclose(in);

	if (outputcount) fprintf(stderr,"%d\n",n);

	/* write remaining pad to newpad if desired */
	if (newpadname) {
		while ( (c=getc(pad))!=EOF ) putc(c,newpad);
		fclose(newpad);
	}
	fclose(pad);
	return 0;
}
