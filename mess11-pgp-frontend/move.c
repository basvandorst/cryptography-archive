#include "copy.h"

int main ( int argc, char *argv[] )
{
	if ( argc != 3 ) {
		fputs ( "Incorrect command-line args!", stderr );
		return 1;
	}
	
	if ( move ( argv[1], argv[2] ) ) {
		fputs ( "Error during move!", stderr );
		return 2;
	}
	return 0;
}
