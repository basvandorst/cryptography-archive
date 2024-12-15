/************************************************************************
*
*	Ron Cohn's implementation	January 1992
*
*   31 Point Equiripple FIR Low-Pass Filter
*
*	Passband:  ripple = 0.5 dB, cutoff =  800 Hz
*	Stopband:  atten. =  40 dB, cutoff = 1240 Hz
*
* Inputs:
*    LEN    - Length of speech buffers
*    NSAMP  - Number of samples to filter
*    INBUF  - Input speech buffer
* Output:
*    LPBUF  - Low passed speech buffer
*/

void lpfilt31(inbuf, lpbuf)
float inbuf[], lpbuf[];
{
   int j;
   static float h0  =   -0.0097201988;
   static float h1  =   -0.0105179986;
   static float h2  =   -0.0083479648;
   static float h3  =    0.0005860774;
   static float h4  =    0.0130892089;
   static float h5  =    0.0217052232;
   static float h6  =    0.0184161253;
   static float h7  =    0.0003397230;
   static float h8  =   -0.0260797087;
   static float h9  =   -0.0455563702;
   static float h10 =   -0.0403068550;
   static float h11 =    0.0005029835;
   static float h12 =    0.0729262903;
   static float h13 =    0.1572008878;
   static float h14 =    0.2247288674;
   static float h15 =    0.2505359650;
	

   for(j=312+1-180; j<=312; j++)
   {
      lpbuf[j]  = h0  * (inbuf[j   ] + inbuf[j-30]);
      lpbuf[j] += h1  * (inbuf[j -1] + inbuf[j-29]);
      lpbuf[j] += h2  * (inbuf[j -2] + inbuf[j-28]);
      lpbuf[j] += h3  * (inbuf[j -3] + inbuf[j-27]);
      lpbuf[j] += h4  * (inbuf[j -4] + inbuf[j-26]);
      lpbuf[j] += h5  * (inbuf[j -5] + inbuf[j-25]);
      lpbuf[j] += h6  * (inbuf[j -6] + inbuf[j-24]);
      lpbuf[j] += h7  * (inbuf[j -7] + inbuf[j-23]);
      lpbuf[j] += h8  * (inbuf[j -8] + inbuf[j-22]);
      lpbuf[j] += h9  * (inbuf[j -9] + inbuf[j-21]);
      lpbuf[j] += h10 * (inbuf[j-10] + inbuf[j-20]);
      lpbuf[j] += h11 * (inbuf[j-11] + inbuf[j-19]);
      lpbuf[j] += h12 * (inbuf[j-12] + inbuf[j-18]);
      lpbuf[j] += h13 * (inbuf[j-13] + inbuf[j-17]);
      lpbuf[j] += h14 * (inbuf[j-14] + inbuf[j-16]);
      lpbuf[j] += h15 * (inbuf[j-15]              );
   }
}
