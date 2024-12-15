From msuinfo!agate!library.ucla.edu!psgrain!agora!agora.rdrop.com!jamesp Fri Jul 15 19:53:31 1994
Newsgroups: alt.sources
Path: msuinfo!agate!library.ucla.edu!psgrain!agora!agora.rdrop.com!jamesp
From: jamesp@agora.rdrop.com (James Price)
Subject: Re: ISBN Check Digit, Perl version
Nntp-Posting-Host: agora.rdrop.com
Keywords: isbn
References: <1994Jul14.142057.2642@aaf.alcatel.at>
Sender: news@agora.rdrop.com (USENET News)
Organization: RainDrop Laboratories
Date: Fri, 15 Jul 1994 05:20:09 GMT
Message-ID: <CsyutM.J45@agora.rdrop.com>
Lines: 83


back in march, to avoid studying for a test, i was reading through
_numbers, groups, and codes_ and couldnt resist writing up an
isbn program after learning how it worked. i emailed my friend
patricia my version, and after a few minutes, she emailed back her
version, which i think is far superior. i post these two versions
only as an example of the difference between a good programmer and a 
mediocre wannabe. her version is infinitely sexier, and almost certainly
much more efficient; mine clods along and gets the correct answer almost
coincidentally.  note that my version has at least one variable which
is declared but unused. 

------------my version-------------------------------
#include <stdio.h>
#include <math.h>

/*
 From _Numbers, Groups and Codes_ by J.F. Humphreys and M. Y. Prest,
Cambridge University Press, Avon: 1989. page 233.

  Example: A well-known example of error-correction is provided by the ISBN
(International Standard Book Number) of published books. This is a sequence
of nine digits a1a2...a9, where each ai is one ofthe numbers 0,1,...,9,
together with a check digit which is one of the symbols 0,1,2,3,4,5,6,7,8,9,
or X (with X representing 10). This last digit is included so as to give
a check that the previous 9 digits have been correctly transcribed, and is
calculated as follows.  Form the integer  10a1 + 9a2 + 8a3 +... +2a9 and 
reduce this modulo 11 to obtain an integer b between 1 and 11 inclusive.
The check digit is obtained by subtracting b from 11.

main()
{

long int isbn,ksbn, temp, count, level,krad;
printf("gimme the 1st 9 digits of an ISBN i will give you the final digit\n");
scanf("%d", &isbn);
printf("isbn entered: %d \n", isbn);

for (level =10; level > 1; level--){
   krad= floor((pow(10,level-2)+.5));
   temp = floor((isbn) / (krad));
   if (krad > 0) 
      isbn = ( (isbn) % (krad));
   count = count + temp * level;
   }

count = count % 11;
count = 11 - count;
printf(" magic crystal ball sez its a ");
if (count == 10) printf("X\n");
if (count == 11)  printf("0\n", count);
if (count < 10) printf ("%d\n", count);
}

----------------patricia's version------------------------
/* isbn.c - computes the checksum of an isbn number */

#include <stdio.h>
#include <math.h>

int main() {
	int i;
	int mult = 10;
	long int count = 0;
	int checksum;

	printf ("enter the first nine digits of an isbn code:\n\t");

	for (i = 0; i < 9; i++) {
		count = count + (mult * (getchar() - 48));
		--mult;
	}
	
	checksum = 11 - (count % 11);
	printf ("checksum is: ");
	if (checksum == 10) { printf ("X\n"); }
		else { printf ("%d\n", checksum); }
}

--
jamesp     I regret to say that we of the F.B.I. are powerless to act
@ agora    in cases of oral-genital intimacy, unless it has in some way
rdrop com  obstructed interstate commerce. - J Edgar Hoover

