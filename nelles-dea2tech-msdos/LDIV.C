/*
 *
 *        LONG DIVIDE V. 2.61  (NORMAL LONG DIVISION)
 *        Copyright (c) 1990, 1991, 1992, 1993, 1994 by  V. Sadowsky
 *
 *           This program is used to view long division by PRIME
 *           denominators.  Decimals may be viewed in normal (0-9)
 *           format, or by the USD pattern mask method.
 *    NOTE:  to view the decimal expansion without any symbols (just 0-9
 *           digits), type a '-' before the digit when the prompt "Replace
 *           digit < > with symbol..." appears.  That is, instead of watching
 *           digit 5 (this will mask all other digits), type -5 to view ALL.
 *
 *     SET TAB setting to 2  *******
 *
 *  date: 02-01-1992
 *  date: 02-09-1992
 *  date: 04-25-1992  R.W.O.  modification:  average address gap / digit jump
 *  date: 05-02-1992  disk output: store the decimal expansion of fraction
 *
 *
*/


#include <stdio.h>
#include <stdlib.h>
#include <stdlib.h>
#include <conio.h>
#include <ctype.h>
#include <dos.h>

unsigned char RF_DIGITS[] = { "DIGITS.RF" };
FILE *fout;

	struct digit_record
										 {
											 unsigned long int last_digit_address,
																				 total_gaps,
																				 highest;

										 };


void clear_screen( char flag );
void cursor( char row, char col );
void delay_function( void );
void main( void );

void main( void )
{
									auto unsigned long int divisor, top, modresult,
											 out, j, length = 0L, flag;

									auto char buff[20];
									auto long int count = 0, i;
									auto int ponly = 0, WAIT_FLAG = 0;
									auto long int WAIT_DELAY = 0L;
									auto int toscreen = 0;
									auto long int total_writes = 0L, X = 0L;
									auto long int PL = 0L;             /* the period length  */
									auto signed int remainder = 0;
									auto long int CLN[10];
									auto struct digit_record DR[10];
									auto long int USDAVG = 0L;
									auto float dummy = 0.0000001; /* force compiler to load  */
																								/* the floating point LIB. */

									auto unsigned char *F_DIGIT;


CLN[0] = CLN[1] = CLN[2] = CLN[3] = CLN[4] = CLN[5] = CLN[6] = CLN[7] = CLN[8] = CLN[9] = 0L;
/* 0       1         2        3        4        5        6       7         8       9    */

						/*  Zero the structure members of digit_record  */

		 for ( toscreen = 0; toscreen < 10; toscreen++ )
		 {
					 DR[toscreen].last_digit_address = 0L;
					 DR[toscreen].total_gaps = 0L;
					 DR[toscreen].highest = 0L;
		 }

		toscreen = 0;

REDO:

clear_screen( 0 );

printf("\t\t       The Long Divider V. 2.61\n" );
printf("\t\t  -----------------------------------\n\n");
printf("\t\t    Copyright (c) V. Sadowsky, 1992\n");
printf("\t\t           All Rights Reserved\n\n\n" );


				printf("Please enter the numerator: ");
				top = atol( gets( buff ) );
						if ( top < 1 )
								 goto REDO;

				printf("Please enter the PRIME denominator: ");
				divisor = atol( gets( buff ) );

				printf("Enter the decimal length (default: %ld): ", (divisor - 1) );
				length = atol( gets( buff ) );
				if ( length == 0 )
						length = divisor;

				printf("Wait for keystroke before showing next page: (Y/N) ");
				WAIT_FLAG = getche( );
				WAIT_FLAG = toupper( WAIT_FLAG );

				if ( WAIT_FLAG != 'Y' )
				{
					printf("\nSelect wait delay: ( 0 - 100,000,000 ) " );
					WAIT_DELAY = atol( gets(buff) );
				}

				if ( WAIT_FLAG == 'Y' )
							printf("\n");

				printf("Replace digit (0-9) <   > with þ\b\b\b\b\b\b\b\b\b\b");
				flag = atol( gets( buff ) );   /*  char. 254   (ALT + 254  numeric K)  */

					 if ( (flag < 0)  ||  ( flag > 9) )
									flag = 999;

			 if ( flag != 999 )
			 {
				printf("Show pattern only ? (Y/N) ");
				ponly = getche(  );
				ponly = toupper( ponly );
			 }

if ( (top == 0) || (divisor == 0) )
				goto REDO;


				if ( (fout = fopen(RF_DIGITS, "wb")) == NULL )
				{
					printf("Error: Cannot open file < %s > for writing\n", RF_DIGITS );
					exit( 1 );
				}



cursor( 24, 0 );
printf("Press any key to start...");

		 while ( !kbhit( ) )
							;
		 getch( );


		cursor( 0, 0 );
		clear_screen( 0 );

	if ( top / divisor == 0 )
	{
			modresult = ( top % divisor ) * 10;
			if ( ponly == 89 )
						 ;
			 else
			 {
						printf(" 0.");
						toscreen += 2;
			 }
	}

	else
	{
			printf("%ld.", top / divisor );
			modresult = ( top % divisor ) * 10;
			toscreen++;
	}

	for ( j = 1; j < length; j++ )
			{
				out = modresult / divisor;
				(*F_DIGIT) = (unsigned char) out;
				(*F_DIGIT) += 48;

				fwrite( F_DIGIT, sizeof( unsigned char), 1, fout );

				CLN[out] += 1;

				if ( DR[out].last_digit_address == 0L )
								 DR[out].last_digit_address = j;
				else
						 {
							if ( DR[out].last_digit_address > 0 )
							{
							 if ( ( j - DR[out].last_digit_address ) > DR[out].highest )
										 DR[out].highest = ( j - DR[out].last_digit_address );
							}

							 DR[out].total_gaps += ( j - DR[out].last_digit_address );
							 DR[out].last_digit_address = j;

						 }


												 if ( out == flag )
												 {
														printf("þ" );  /*  the selected WATCH DIGIT  */
														toscreen++;    /*  may be changed to suit needs */
												 }  /*  char. 254   (ALT + 254  numeric K)  */


							 if ( ponly != 89 )
								{
								 printf("%d", out );
								 toscreen++;
								}

					 else if ( ponly == 89 )
								{
								 printf("°" );   /*  background character, may be altered  */
								 toscreen++;     /*  char. 176   (ALT + 176  numeric K)  */
								}


				if ( toscreen >= 1999 )
				{
							total_writes += toscreen;
							toscreen = 0;

					if ( WAIT_FLAG == 'Y' )
					{
							while ( !kbhit( ) )
										 ;
							getch( );
					}
					else
							{
								for ( i = 0; i < WAIT_DELAY; i++ )
											 delay_function( );
							}

							clear_screen( 0 );
							cursor( 12, 10 );
							printf("Numerator: %ld", top );
							cursor( 13, 8 );
							printf("Denominator: %ld", divisor );
							cursor( 14, 4 );
							printf("Decimal  length: %ld", length );
							cursor( 15, 4 );
							printf("Replacing digit < %d > with pattern", flag );
							cursor( 16, 0 );
							printf("Total digits processed: %ld", total_writes );

					if ( WAIT_FLAG == 'Y' )
					{
							cursor( 23, 10 );
							printf("Press any key when cursor is in bottom-right of screen for next page");
					}
					else if ( WAIT_FLAG == 'Y' )
							{
							cursor( 23, 10 );
							printf("Next page will be shown in a few moments...");
							}

							cursor( 0, 0 );
				}

				modresult = modresult % divisor;

					 if ( modresult == top  )  /*  check if a remainder occurrs which  */
					 {                         /*  is equal to the numerator in the    */
									PL = j;            /*  original rational fraction.  This   */
																		 /*  indicates that the decimal period   */
									X = (divisor / PL);/*  has been found, and is about to     */
									printf("\a\a\a");  /*  start over again.                   */
									break;
					 }

				modresult *= 10;

			}

fclose( fout );
cursor( 23, 10 );
printf("Press any key for statistics page 1 ...                                  ");
cursor( 23, 49 );

		 while ( !kbhit( ) )
							;

						 getch( );

		 clear_screen( 0 );

cursor( 0, 0 );
printf("Decimal Period for rational fraction:  %ld / %ld = << %ld >> digits\n\n", top, divisor, PL );
printf("                        ((%ld x %ld) + 1) = %ld\n                        ((%ld - 1) ö %ld) = %ld", PL, X, ((PL * X) + 1), divisor, X, ((divisor - 1) / X) );
cursor( 4, 0 ); printf("[ X ] = %6ld          * Decimal Period = ((Prime Denominator - 1) / [ X ]) *\n\n", X );
printf("--------- Occurances of digits 0 through 9 within the Decimal Period ---------\n\n" );
printf("                    digit 0: ... %ld\n", CLN[0] );
printf("                          1: ... %ld\n", CLN[1] );
printf("                          2: ... %ld\n", CLN[2] );
printf("                          3: ... %ld\n", CLN[3] );
printf("                          4: ... %ld\n", CLN[4] );
printf("                          5: ... %ld\n", CLN[5] );
printf("                          6: ... %ld\n", CLN[6] );
printf("                          7: ... %ld\n", CLN[7] );
printf("                          8: ... %ld\n", CLN[8] );
printf("                          9: ... %ld\n", CLN[9] );
printf("                             USD Totals: %ld\n\n", ( CLN[0] + CLN[1] + CLN[2] + CLN[3] + CLN[4] + CLN[5] + CLN[6] + CLN[7] + CLN[8] + CLN[9] ) );

printf("Average occurance of each USD digit within decimal period: %ld\n", (PL / 10) );

 if (  ((PL * X) + 1) == divisor )
 {
			cursor( 21, 17 );
			printf("VERIFICATION: *** PASSED ***" );
 }
 else
		 {
			cursor( 21, 17 );
			printf("VERIFICATION: *** FAILED ***" );
		 }


cursor( 23, 10 );
printf("Press any key for statistics page 2 ...                                   ");
cursor( 23, 49 );

		 while ( !kbhit( ) )
							;

						 getch( );

		 clear_screen( 0 );

cursor( 0, 0 );

printf("Decimal Period for rational fraction:  %ld / %ld = << %ld >> digits\n\n", top, divisor, PL );
printf("          AAAAAAAAA      BBBBBBBBB      CCCCCCCCC       DDDDDDDDD\n" );
printf(" digit 0: %9ld .... %9ld .... %-2.4E ... %9ld\n", DR[0].total_gaps, CLN[0], ((float) DR[0].total_gaps / (float) CLN[0]), DR[0].highest );
printf(" digit 1: %9ld .... %9ld .... %-2.4E ... %9ld\n", DR[1].total_gaps, CLN[1], ((float) DR[1].total_gaps / (float) CLN[1]), DR[1].highest );
printf(" digit 2: %9ld .... %9ld .... %-2.4E ... %9ld\n", DR[2].total_gaps, CLN[2], ((float) DR[2].total_gaps / (float) CLN[2]), DR[2].highest );
printf(" digit 3: %9ld .... %9ld .... %-2.4E ... %9ld\n", DR[3].total_gaps, CLN[3], ((float) DR[3].total_gaps / (float) CLN[3]), DR[3].highest );
printf(" digit 4: %9ld .... %9ld .... %-2.4E ... %9ld\n", DR[4].total_gaps, CLN[4], ((float) DR[4].total_gaps / (float) CLN[4]), DR[4].highest );
printf(" digit 5: %9ld .... %9ld .... %-2.4E ... %9ld\n", DR[5].total_gaps, CLN[5], ((float) DR[5].total_gaps / (float) CLN[5]), DR[5].highest );
printf(" digit 6: %9ld .... %9ld .... %-2.4E ... %9ld\n", DR[6].total_gaps, CLN[6], ((float) DR[6].total_gaps / (float) CLN[6]), DR[6].highest );
printf(" digit 7: %9ld .... %9ld .... %-2.4E ... %9ld\n", DR[7].total_gaps, CLN[7], ((float) DR[7].total_gaps / (float) CLN[7]), DR[7].highest );
printf(" digit 8: %9ld .... %9ld .... %-2.4E ... %9ld\n", DR[8].total_gaps, CLN[8], ((float) DR[8].total_gaps / (float) CLN[8]), DR[8].highest );
printf(" digit 9: %9ld .... %9ld .... %-2.4E ... %9ld\n\n", DR[9].total_gaps, CLN[9], ((float) DR[9].total_gaps / (float) CLN[9]), DR[9].highest );
printf("    Legend:\n" );
printf("            AAAAAAAAA - accumulated address differences between two\n" );
printf("                        digit occurances (last & present) in the D.P.\n" );
printf("            BBBBBBBBB - number of times digit actually occurs in D.P.\n" );
printf("            CCCCCCCCC - computed average address gap per digit jump\n" );
printf("            DDDDDDDDD - highest address difference found between two points\n\n" );
printf("[ X ] = %6ld          * Decimal Period = ((Prime Denominator - 1) / [ X ]) *\n", X );

cursor( 24, 10 );
printf("Press any key to return to DOS...");

		 while ( !kbhit( ) )
							;

						 getch( );

	 clear_screen( 0 );

}


/*         clear_screen( )

							 from C Programming Guide 3rd Edition, pg. 396
*/

void clear_screen( char flag )
{

																							union REGS ireg;


			 ireg.h.ah = 0x07;
			 ireg.h.al = 0;
			 ireg.h.ch = 0;
			 ireg.h.cl = 0;
			 ireg.h.dh = 24;
			 ireg.h.dl = 79;

	if ( flag )
			 ireg.h.bh = 0x70;

	else
			 ireg.h.bh = 0x07;


							cursor( 0, 0 );
							int86(0x10, &ireg, &ireg );

}

void cursor( char row, char col )
{

																							 union REGS ireg;


					 ireg.h.ah = 2;
					 ireg.h.bh = 0;
					 ireg.h.dh = row;
					 ireg.h.dl = col;

							 int86(0x10, &ireg, &ireg );

}


void delay_function( void )
{

														auto int x = 0;
														auto int y = 1;
														auto int j = 3;
														auto int f = 9;

			 x = ( y + j + f );
			 x += x;

}

