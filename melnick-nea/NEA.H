/*

		NEA.H

	Header file for NEA encryption module

*/

enum NEA_OPER { encrypt=0, decrypt=1 };

void keyp (char*);
void nea (char*,char*,enum NEA_OPER);
