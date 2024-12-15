#include <stdio.h>

/***************************************************************
 *
 * Procedure main
 *
 ***************************************************************/
#ifdef __STDC__

int main(
)

#else

int main(
)

#endif
{

	char x[10000];

	while (scanf("%s\n", x) != EOF)
		printf("%s\n", x);
}
