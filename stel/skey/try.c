#include <stdio.h>

main(argc, argv)
int	argc;
char	*argv[];
{
	if (argc != 2) {
		fprintf(stderr, "usage: %s name\n", argv[0]);
		exit(1);
	}

	if (skeydoall(argv[1]))
		puts("FAILED");
	else
		puts("SUCCEEDED");
}

#include <stdio.h>
#include "skey.h"

/*
 * Authenticate skeyuser with skey
 * returns:
 *	-1 for keyfile error or user nor found
 *	 0 ok
 *	 1 login incorrect
 */
skeydoall(skeyuser)
char	*skeyuser;
{
	struct 	skey skey;
	char	skeyprompt[80], buf[80];

	skeyprompt[0] = '\0';
	if (skeychallenge(&skey, skeyuser, skeyprompt))
		return(-1);
	printf("[%s]\n",skeyprompt);

	printf("Response: ");
	(void)fflush(stdout);
	fgets(buf, sizeof(buf), stdin);
	rip(buf);

	if (skeyverify(&skey,buf))
		return(1);
	if (skey.n < 5)
		printf("Warning! Change password soon\n");
	return(0);
}
