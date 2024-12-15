#include <stdio.h>
extern check_perm();
extern nadda();

main(argc,argv)
int argc;
char **argv;
{
	if(!check_perm()) {
		printf("User authorization failure\n");
		nadda(argv);
	}
	else
		printf("Authorization ok\n");
}
	
