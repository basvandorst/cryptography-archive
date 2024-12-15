#include <stdio.h>

main()   /* streamlined version of copy input to output */
{
	int c;

	while ((c =getchar()) !=EOF)
	   {
	   if( c>= 97 && c <= 109 )
	      c=c+13;
	   else if( c >= 110 && c <= 122 )
	      c=c-13;
	      else if( c >= 65 && c <= 77 )
		 c=c+13;
		 else if( c >= 78 && c <= 90 )
		    c=c-13;
		putchar(c);
		}
}
