
/* $Log:	oldbzio.c,v $
 * Revision 1.1  91/09/20  14:48:22  morain
 * Initial revision
 * 
 * Revision 1.1  91/09/20  14:07:42  morain
 * Initial revision
 *  */

Print(s, n, nd, nl)
	char *s;
	BigNum n;
	int nd, nl;
	{
	    printf("%s(%d)", s, nl);
	    BnToHex(n, nd, nl);
	    printf("\n");
	}

BnToHex(n, nd, nl)
	BigNum n;
	int nd, nl;
	{
	    nl += nd;
	    while(--nl >= nd)
	      printf("%x ",BnGetDigit(n, nl));
	}

char *zBToString(A, b)
	BigZ A;
	int b;
	{
	    char *s;
	
/* 	    Print("z=", A, 0, BnGetSize(A));*/
	    s = BzToString(A, b);
/*	    printf("Done !\n");*/
	    return(s);
	}

