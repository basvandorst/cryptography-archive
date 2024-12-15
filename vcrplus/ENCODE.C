
/*
   ENCODE.C

   Encode Version 1.1  by David W. Sanderson, Mark K. Mathews

   Purpose: Creates VCR-Plus code from date and time of show.


*/

#include <dos.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define		KEY001	(9371L)
#define		INVALID	(-987L)

/* prototypes */
long mixup(long x, long y );
long f1(long inval);
long lookup( long 	stime, long durat);
void interleave(long	tblidx, long cval , long *t8c5, long *t2c1);
long map_top(long day, long year, long top, long digits );
void offset( long day, long year, long top, long *ofout, long *topout );
void Syntax(const char * message);


int main( int argc, char *argv[] )
{
	long 	j, doneflag, trailers, tblidx ;
	long 	s1_out, bot3, top5, quo, rem, newspaper ;
	long 	s4_out, s5_out, ofout, cval ;
	long 	month, day, year, channel, starttime, duration ;

   printf("Encode Version 1.1  by David W. Sanderson, Mark K. Mathews\n\n");


	if(argc != 7 )  /* only 7 paramaters needed */
	   Syntax("Required paramaters missing");

   if (!strcmp(argv[1],"/?"))
	   Syntax("");


	month = atoi(argv[1]);
	day = atoi(argv[2]);
	year = atoi(argv[3]);
	channel = atoi(argv[4]);
	starttime = atoi(argv[5]);
	duration = atoi(argv[6]);

	year = year % 100 ;

	if( (month>12) || (month<1) || (day<1) || (day>31) )
	{
      Syntax("Invalid date");
   }


	if( (channel<1) || (channel>48) )
	{
      Syntax("Invalid channel number");
   }

	trailers = starttime % 100;

	if( ((trailers != 0) && (trailers != 30))  ||
	    (starttime < 0) || (starttime > 2330))
	{
      Syntax("Invalid start time");
   }

	trailers = duration % 30;
	if( (trailers!=0) || (duration<30) || (duration>300) )
	{
      Syntax("Invalid program length");
   }


	/* get the  t  bits and the  c  bits */
	cval = channel - 1 ;
	tblidx = lookup(starttime, duration);

	/* from them infer what must have been step 4 & step 5 results */
	interleave(tblidx, cval, (&s4_out), (&s5_out)) ;

	/* find the smallest unmapped_top giving correct mapped_top */
	top5 = 0;
	doneflag = 0;

	/* if the mapped_top is zero then top and offset are zero */
	if(s4_out == 0)
   {
      top5 = 0; ofout = 0; doneflag = 1;
   }

	while(doneflag == 0)
	{
		top5++;
		offset(day, year, top5, (&ofout), (&j)) ;
		if(j == s4_out)
         doneflag = 1;
	}

	/* have two of the three inputs to step 5; determine the rem */
	for(rem=0; rem<32; rem++)
	{
		j = (rem + (day*(month+1)) + ofout) % 32 ;
		if(j == s5_out)
         break ;
	}
	quo = (day - 1);


	/* assemble the output of step 1 */
	bot3 = 1 + rem + (32 * quo) ;
	s1_out = bot3 + (1000 * top5) ;

	/* invert the mixing */
	newspaper = f1(s1_out);

	printf("Date (month/day/year) = %2ld/%2ld/%2ld\n"
          "Magazine Channel      = %2ld\n"
          "Start_time            = %4ld\n"
	       "Length                = %3ld\n"
          "VCR-Plus code         = %6ld\n",
          month, day, year, channel, starttime, duration, newspaper);
	return(0);
}



/* function that performs initial scrambling */
long 	mixup(long 	x, long y )
{

	long 	j, k, sum;
   int i;

	long 	a[12], b[12], out[12] ;

	/* get the digits of x into a[] */
	j = x ;
	for(i=0; i<9; i++)
	{
		k = j % 10;
		a[i] = k;
		j = (j - k) / 10 ;
	}

	/* get the digits of y into b[] */
	j = y ;
	for(i=0; i<9; i++)
	{
		k = j % 10;
		b[i] = k;
		j = (j - k) / 10 ;
		out[i] = 0;
	}


	for(i=0; i<=8; i++)
	{
		for(j=0; j<=8; j++)
		{
			out[i+(int)j] += (b[(int)j] * a[(int)i]) ;
		}
	}

	j = 1;
	sum = 0;
	for(i=0; i<=8; i++)
	{
		sum += j * (out[i] % 10);
		j = j * 10 ;
	}
	return( sum ) ;
}



/* first function */
long 	f1(long 	inval)
{
	long 	ndigits, hashval ;
	long 	pwr ;


	ndigits = 0;
	pwr = 1;
	while(inval >= pwr)
   {
      ndigits++;
      pwr *= 10;
   }
	if(ndigits > 8)
      printf("\nPANIC: %ld has %ld digits\n", inval, ndigits);

	pwr = pwr / 10 ;

	hashval = inval;

again:	hashval = mixup(hashval, KEY001) % (pwr * 10) ;
	if(hashval < pwr)
      goto again ;

	return(hashval);
}




/* the 512-entry tables of starting time and pgm duration */
long 	lookup(	long 	stime, long durat)
{
	static	long 	start[480], leng[480] ;
	static	long 	initializer = 0;

	long 	s;
   int j;

	if(initializer == 0)
   {
	   initializer = 1;

	   for(j=0; j<512; j++)
      {
         start[j] = leng[j] = INVALID;
      }

	   leng[0]   =  30 ;
	   leng[100] = 120 ;
	   leng[101] =  90 ;
	   leng[102] = 120 ;
	   leng[103] = 120 ;
	   leng[104] =  90 ;
	   leng[105] =  90 ;
	   leng[106] =  30 ;
	   leng[107] =  30 ;
	   leng[108] = 120 ;
	   leng[109] =  90 ;
	   leng[10]  =  30 ;
	   leng[110] =  90 ;
	   leng[111] =  30 ;
	   leng[112] =  90 ;
	   leng[113] =  30 ;
	   leng[114] =  30 ;
	   leng[115] = 120 ;
	   leng[116] =  30 ;
	   leng[117] = 120 ;
	   leng[118] = 120 ;
	   leng[119] =  30 ;
	   leng[11]  =  30 ;
	   leng[120] =  90 ;
	   leng[121] =  60 ;
	   leng[122] =  60 ;
	   leng[123] =  60 ;
	   leng[124] = 120 ;
	   leng[125] =  90 ;
	   leng[126] =  60 ;
	   leng[127] =  60 ;
	   leng[128] =  90 ;
	   leng[129] = 120 ;
	   leng[12]  =  30 ;
	   leng[130] = 120 ;
	   leng[131] =  90 ;
	   leng[132] =  60 ;
	   leng[133] =  60 ;
	   leng[134] = 120 ;
	   leng[135] =  90 ;
	   leng[136] =  90 ;
	   leng[137] = 120 ;
	   leng[138] = 120 ;
	   leng[139] =  60 ;
	   leng[13]  = 120 ;
	   leng[140] =  60 ;
	   leng[141] =  90 ;
	   leng[142] =  90 ;
	   leng[143] = 120 ;
	   leng[144] =  90 ;
	   leng[145] = 120 ;
	   leng[146] =  90 ;
	   leng[147] = 120 ;
	   leng[148] = 120 ;
	   leng[149] =  90 ;
	   leng[14]  =  30 ;
	   leng[150] = 120 ;
	   leng[151] =  90 ;
	   leng[152] =  90 ;
	   leng[153] =  90 ;
	   leng[154] =  60 ;
	   leng[155] =  60 ;
	   leng[156] = 120 ;
	   leng[157] =  60 ;
	   leng[158] =  60 ;
	   leng[159] =  60 ;
	   leng[15]  =  30 ;
	   leng[160] = 120 ;
	   leng[161] =  60 ;
	   leng[162] =  90 ;
	   leng[163] =  90 ;
	   leng[164] =  60 ;
	   leng[165] =  90 ;
	   leng[166] =  90 ;
	   leng[167] =  60 ;
	   leng[168] =  90 ;
	   leng[169] = 120 ;
	   leng[16]  = 120 ;
	   leng[170] =  60 ;
	   leng[171] = 120 ;
	   leng[172] =  60 ;
	   leng[173] =  60 ;
	   leng[174] = 120 ;
	   leng[175] =  90 ;
	   leng[176] =  60 ;
	   leng[177] = 120 ;
	   leng[178] =  60 ;
	   leng[179] =  60 ;
	   leng[17]  = 120 ;
	   leng[180] =  60 ;
	   leng[181] =  90 ;
	   leng[182] =  60 ;
	   leng[183] = 120 ;
	   leng[184] =  60 ;
	   leng[185] =  60 ;
	   leng[186] =  90 ;
	   leng[187] =  60 ;
	   leng[188] =  90 ;
	   leng[189] = 120 ;
	   leng[18]  =  60 ;
	   leng[190] =  90 ;
	   leng[191] =  90 ;
	   leng[19]  = 120 ;
	   leng[1]   =  30 ;
	   leng[20]  =  60 ;
	   leng[21]  =  60 ;
	   leng[22]  =  60 ;
	   leng[23]  = 120 ;
	   leng[24]  =  60 ;
	   leng[25]  = 120 ;
	   leng[26]  =  30 ;
	   leng[27]  =  30 ;
	   leng[28]  =  30 ;
	   leng[29]  =  60 ;
	   leng[2]   =  30 ;
	   leng[30]  =  90 ;
	   leng[31]  =  30 ;
	   leng[32]  =  30 ;
	   leng[33]  =  30 ;
	   leng[34]  =  30 ;
	   leng[35]  =  60 ;
	   leng[36]  =  30 ;
	   leng[37]  =  60 ;
	   leng[38]  = 120 ;
	   leng[39]  =  60 ;
	   leng[3]   =  30 ;
	   leng[40]  =  30 ;
	   leng[41]  =  30 ;
	   leng[42]  =  30 ;
	   leng[43]  =  30 ;
	   leng[44]  = 120 ;
	   leng[45]  =  30 ;
	   leng[46]  =  30 ;
	   leng[47]  =  30 ;
	   leng[48]  = 120 ;
	   leng[49]  =  30 ;
	   leng[4]   =  30 ;
	   leng[50]  =  60 ;
	   leng[51]  =  60 ;
	   leng[52]  =  30 ;
	   leng[53]  =  30 ;
	   leng[54]  = 120 ;
	   leng[55]  =  60 ;
	   leng[56]  =  60 ;
	   leng[57]  =  30 ;
	   leng[58]  = 120 ;
	   leng[59]  =  30 ;
	   leng[5]   =  30 ;
	   leng[60]  = 120 ;
	   leng[61]  = 120 ;
	   leng[62]  = 120 ;
	   leng[63]  =  30 ;
	   leng[64]  =  90 ;
	   leng[65]  =  30 ;
	   leng[66]  =  30 ;
	   leng[67]  =  30 ;
	   leng[68]  = 120 ;
	   leng[69]  =  90 ;
	   leng[6]   =  30 ;
	   leng[70]  =  90 ;
	   leng[71]  =  60 ;
	   leng[72]  =  90 ;
	   leng[73]  =  90 ;
	   leng[74]  =  90 ;
	   leng[75]  =  90 ;
	   leng[76]  =  90 ;
	   leng[77]  =  60 ;
	   leng[78]  =  90 ;
	   leng[79]  =  30 ;
	   leng[7]   =  30 ;
	   leng[80]  = 120 ;
	   leng[81]  =  60 ;
	   leng[82]  =  90 ;
	   leng[83]  = 120 ;
	   leng[84]  =  60 ;
	   leng[85]  =  60 ;
	   leng[86]  = 120 ;
	   leng[87]  =  30 ;
	   leng[88]  =  90 ;
	   leng[89]  = 120 ;
	   leng[8]   =  30 ;
	   leng[90]  = 120 ;
	   leng[91]  =  90 ;
	   leng[92]  =  90 ;
	   leng[93]  =  90 ;
	   leng[94]  =  90 ;
	   leng[95]  = 120 ;
	   leng[96]  =  60 ;
	   leng[97]  = 120 ;
	   leng[98]  =  90 ;
	   leng[99]  =  30 ;
	   leng[9]   =  60 ;
	   start[0]   = 1830 ;
	   start[100] =  330 ;
	   start[101] = 1500 ;
	   start[102] = 1500 ;
	   start[103] = 2300 ;
	   start[104] = 1900 ;
	   start[105] =  800 ;
	   start[106] =  430 ;
	   start[107] =  300 ;
	   start[108] = 1330 ;
	   start[109] = 1000 ;
	   start[10]  = 1400 ;
	   start[110] =  700 ;
	   start[111] =  100 ;
	   start[112] = 2330 ;
	   start[113] =  330 ;
	   start[114] =  200 ;
	   start[115] = 2230 ;
	   start[116] =  400 ;
	   start[117] =  600 ;
	   start[118] =  400 ;
	   start[119] =  230 ;
	   start[11]  = 2030 ;
	   start[120] =  630 ;
	   start[121] =   30 ;
	   start[122] = 2230 ;
	   start[123] =  100 ;
	   start[124] =   30 ;
	   start[125] = 2300 ;
	   start[126] = 1630 ;
	   start[127] =  830 ;
	   start[128] =    0 ;
	   start[129] = 1930 ;
	   start[12]  = 1700 ;
	   start[130] =  930 ;
	   start[131] = 2030 ;
	   start[132] =  500 ;
	   start[133] = 1730 ;
	   start[134] =  200 ;
	   start[135] = 1930 ;
	   start[136] =  930 ;
	   start[137] = 1730 ;
	   start[138] =  630 ;
	   start[139] = 1830 ;
	   start[13]  = 1600 ;
	   start[140] = 1430 ;
	   start[141] = 1130 ;
	   start[142] =   30 ;
	   start[143] =  830 ;
	   start[144] = 1030 ;
	   start[145] = 1430 ;
	   start[146] =  100 ;
	   start[147] =  730 ;
	   start[148] = 2030 ;
	   start[149] =  300 ;
	   start[14]  = 2000 ;
	   start[150] =  300 ;
	   start[151] = 1330 ;
	   start[152] = 1230 ;
	   start[153] =  230 ;
	   start[154] = 2130 ;
	   start[155] = 1130 ;
	   start[156] = 1830 ;
	   start[157] =  630 ;
	   start[158] =  530 ;
	   start[159] =  200 ;
	   start[15]  = 1500 ;
	   start[160] = 1530 ;
	   start[161] =  730 ;
	   start[162] =  600 ;
	   start[163] = 1730 ;
	   start[164] =  400 ;
	   start[165] =  730 ;
	   start[166] =  430 ;
	   start[167] =  430 ;
	   start[168] =  130 ;
	   start[169] = 1230 ;
	   start[16]  = 2000 ;
	   start[170] =  130 ;
	   start[171] =  230 ;
	   start[172] = 1930 ;
	   start[173] =  300 ;
	   start[174] = 1030 ;
	   start[175] =  200 ;
	   start[176] =  330 ;
	   start[177] =  500 ;
	   start[178] =  930 ;
	   start[179] =  230 ;
	   start[17]  = 2100 ;
	   start[180] = 2030 ;
	   start[181] =  400 ;
	   start[182] = 1530 ;
	   start[183] =  430 ;
	   start[184] = 1330 ;
	   start[185] = 1230 ;
	   start[186] =  330 ;
	   start[187] = 1030 ;
	   start[188] =  500 ;
	   start[189] =  530 ;
	   start[18]  = 2000 ;
	   start[190] =  530 ;
	   start[191] = 1100 ;
	   start[19]  = 1800 ;
	   start[1]   = 1600 ;
	   start[20]  = 1900 ;
	   start[21]  = 2200 ;
	   start[22]  = 2100 ;
	   start[23]  = 1400 ;
	   start[24]  = 1500 ;
	   start[25]  = 2200 ;
	   start[26]  = 1130 ;
	   start[27]  = 1100 ;
	   start[28]  = 2300 ;
	   start[29]  = 1600 ;
	   start[2]   = 1930 ;
	   start[30]  = 2100 ;
	   start[31]  = 2100 ;
	   start[32]  = 1230 ;
	   start[33]  = 1330 ;
	   start[34]  =  930 ;
	   start[35]  = 1300 ;
	   start[36]  = 2130 ;
	   start[37]  = 1200 ;
	   start[38]  = 1000 ;
	   start[39]  = 1800 ;
	   start[3]   = 1630 ;
	   start[40]  = 2200 ;
	   start[41]  = 1200 ;
	   start[42]  =  800 ;
	   start[43]  =  830 ;
	   start[44]  = 1700 ;
	   start[45]  =  900 ;
	   start[46]  = 2230 ;
	   start[47]  = 1030 ;
	   start[48]  = 1900 ;
	   start[49]  =  730 ;
	   start[4]   = 1530 ;
	   start[50]  = 2300 ;
	   start[51]  = 1000 ;
	   start[52]  =  700 ;
	   start[53]  = 1300 ;
	   start[54]  =  700 ;
	   start[55]  = 1100 ;
	   start[56]  = 1400 ;
	   start[57]  = 1000 ;
	   start[58]  =  800 ;
	   start[59]  = 2330 ;
	   start[5]   = 1730 ;
	   start[60]  = 1300 ;
	   start[61]  = 1200 ;
	   start[62]  =  900 ;
	   start[63]  =  630 ;
	   start[64]  = 1800 ;
	   start[65]  =  600 ;
	   start[66]  =  530 ;
	   start[67]  =    0 ;
	   start[68]  = 2330 ;
	   start[69]  = 2200 ;
	   start[6]   = 1800 ;
	   start[70]  = 1300 ;
	   start[71]  =  900 ;
	   start[72]  = 1630 ;
	   start[73]  = 1600 ;
	   start[74]  = 1430 ;
	   start[75]  = 2000 ;
	   start[76]  = 1830 ;
	   start[77]  =  600 ;
	   start[78]  = 1200 ;
	   start[79]  =   30 ;
	   start[7]   = 1430 ;
	   start[80]  =  130 ;
	   start[81]  =    0 ;
	   start[82]  = 1700 ;
	   start[83]  =    0 ;
	   start[84]  =  800 ;
	   start[85]  =  700 ;
	   start[86]  = 2130 ;
	   start[87]  =  500 ;
	   start[88]  = 1530 ;
	   start[89]  = 1130 ;
	   start[8]   = 1900 ;
	   start[90]  = 1100 ;
	   start[91]  =  830 ;
	   start[92]  = 2230 ;
	   start[93]  =  900 ;
	   start[94]  = 2130 ;
	   start[95]  = 1630 ;
	   start[96]  = 2330 ;
	   start[97]  =  100 ;
	   start[98]  = 1400 ;
	   start[99]  =  130 ;
	   start[9]   = 1700 ;


	   s = 2330;
      for(j=192; j<240; j++)
	   {
         start[j] = s;
         leng[j] = 150;
         if(0==(j%2))
            s-=30;
         else
            s-=70;
      }

   	s = 2330;
   	for(j=240; j<288; j++)
   	{
         start[j] = s;
         leng[j] = 180;
         if(0==(j%2))
            s-=30;
         else
            s-=70;
      }

   	s = 2330;
   	for(j=288; j<336; j++)
   	{
         start[j] = s;
         leng[j] = 210;
         if(0==(j%2))
            s-=30;
         else
            s-=70;
      }

   	s = 2330;
   	for(j=336; j<384; j++)
   	{
         start[j] = s;
         leng[j] = 240;
         if(0==(j%2))
            s-=30;
         else
            s-=70;
      }

   	s = 2330;
   	for(j=384; j<432; j++)
   	{
         start[j] = s;
         leng[j] = 270;
         if(0==(j%2))
           s-=30;
         else
           s-=70;
      }

   	s = 2330;
   	for(j=432; j<480; j++)
   	{
         start[j] = s;
         leng[j] = 300;
         if(0==(j%2))
            s-=30;
         else
            s-=70;
      }

   } /* end of initializer block */


	for(j=0; j<481; j++)
	{
		if(stime == start[j])
      {
		   if(durat == leng[j])
            break ;
		}
	}

	if( j >= 480 )
	{
		printf("oops, I dont have an entry for index %ld\n", j);
		fflush(stdout);
		j = 0;
	}

	return(j);
}






void	interleave(long	tblidx, long cval , long *t8c5, long *t2c1)
{
	long 	small, big, b;

	small = 0;
	big = 0;

	/* build t2c1 */
	b = 0x00000001 & (tblidx >> 2) ;
	small += (b << 4);

	b = 0x00000001 & (tblidx >> 1) ;
	small += (b << 2);

	b = 0x00000001 & tblidx ;
	small += b;

	b = 0x00000001 & (cval >> 1) ;
	small += (b << 3);

	b = 0x00000001 & cval ;
	small += (b << 1);


	/* build t8c5 */
	b = 0x00000001 & (tblidx >> 8) ;
	big += (b << 9);

	b = 0x00000001 & (tblidx >> 7) ;
	big += (b << 7);

	b = 0x00000001 & (tblidx >> 6) ;
	big += (b << 5);

	b = 0x00000001 & (tblidx >> 5) ;
	big += (b << 4);

	b = 0x00000001 & (tblidx >> 4) ;
	big += (b << 3);

	b = 0x00000001 & (tblidx >> 3) ;
	big += b;

	b = 0x00000001 & (cval >> 5) ;
	big += (b << 8);

	b = 0x00000001 & (cval >> 4) ;
	big += (b << 6);

	b = 0x00000001 & (cval >> 3) ;
	big += (b << 2);

	b = 0x00000001 & (cval >> 2) ;
	big += (b << 1);


	*t8c5 = big;
	*t2c1 = small;
}





long	map_top(long day, long year, long top, long digits )
{
	long 	d2, d1, d0, y, poot ;
	long 	n2, n1, n0, f3, f2, f1, f0;

	y = year % 16 ;
	d2 = top / 100 ;
	d1 = (top % 100) / 10 ;
	d0 = top % 10 ;

	f0 = 1;
	f1 = (y + 1) % 10;
	f2 = ( ((y+1)*(y+2)) / 2 ) % 10 ;
	f3 = ( ((y+1)*(y+2)*(y+3)) / 6 ) % 10 ;


	if(digits == 1)
	{
		n0 = ( (d0*f0) + (day*f1) ) % 10 ;
		n1 = 0;
		n2 = 0;
	}

	if(digits == 2)
	{
		n0 = ( (d0*f0) + (d1*f1) + (day*f2) ) % 10 ;
		n1 = ( (d1*f0) + (day*f1) ) % 10 ;
		n2 = 0;
	}

	if(digits == 3)
	{
		n0 = ( (d0*f0) + (d1*f1) + (d2*f2) + (day*f3) ) % 10 ;
		n1 = ( (d1*f0) + (d2*f1) + (day*f2) ) % 10 ;
		n2 = ( (d2*f0) + (day*f1) ) % 10 ;
	}


	poot =  (100*n2) + (10*n1) + n0 ;

	return( poot );
}





void	offset(	long day, long year, long top, long *ofout, long *topout )
{
	long 	i, t, tx, off, digits, burp;
	long 	pwr ;


	pwr = 1 ; digits = 0;
	while(top >= pwr)
   {
      digits++;
      pwr *= 10;
   }
	pwr = pwr / 10 ;

	t = tx = top ;
	off = 0;
	while(tx > 0)
   {
      off += (tx % 10);
      tx /= 10;
   }

again:
	for(i=0; i<=(year%16); i++)
	{
		burp = ( (map_top(day, i, t, digits)) % 10 ) ;
		off += burp ;
	}

	t = map_top(day, year, t, digits) ;

	if(t < pwr)
      goto again ;


	*ofout = (off % 32) ;
	*topout = (t) ;
}


void Syntax(const char *message)
{
   if ( *message != NULL)
   {
      printf(message);
      printf("\n\n");
   }

   printf("Encode V1.1 Converts program time to VCR-Plus code.\n"
          "\n"
          "Encode [MM] [DD] [YY] [CH] [START] [LENGTH]\n"
          "\n"
          "   [MM]     - Month of program.\n"
          "   [DD]     - Day of program.\n"
          "   [YY]     - Year of program.\n"
          "   [CH]     - Channel # from magazine.\n"
          "   [START]  - Time program starts.\n"
          "              Example:\n"
          "                 If 9AM enter 900.\n"
          "                 If 6AM enter 1800 (600+1200).\n"
          "   [LENGTH] - How long the program is.\n"
          "   /?       - This help info.\n"
         );
	exit(-1) ;

}










