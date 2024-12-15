#include <stdio.h>

main(argc,argv)
int     argc;
char    **argv;
{
    static char a[512];
    int i, count, total;
    FILE *fi, *fo;
    struct MDCTX
    {
       unsigned char  D[48];   /* buffer for forming digest in */
                               /* At the end, D[0...15] form the message */
                               /*  digest */
       unsigned char  C[16];   /* checksum register */
       unsigned char  i;       /* number of bytes handled, modulo 16 */
       unsigned char  L;       /* last checksum char saved */
    } mdtemp ;
    
    total = 0;
    fi = stdin;
    fo = stdout;

    MDINIT (&mdtemp);

    if (argc != 1)
    {
        if (argc > 3)
        {
            fprintf(stderr, "Usage: %s [ input-file ] [ output-file ]\n", argv[0]);
            exit(1);
        }
        if ((fi = fopen(argv[1], "r")) == NULL)
	    {
	            perror(argv[1]);
		    exit(1);
	    }

	if (argc > 2)
	{	
	        if ((fo = fopen(argv[2], "w")) == NULL)
		    {
	            	perror(argv[2]);
			exit(1);
		    }
	}
    }

    while ((count=fread(a, 1, 512, fi)) > 0)
    {
	total += count;
	for (i=0;i<count;i++) MDUPDATE(&mdtemp,a[i]) ;
    }

    MDFINAL(&mdtemp);

    fprintf(fo, "%d bytes processed.\ncomputed digest: ", total);
    for (i=0; i<16; i++) fprintf(fo, "%02x", mdtemp.D[i]);
    fprintf(fo, "\n");

    exit(0);
}

