#ifndef TEST
#include "..\bf\bf.h"
#include "..\bf\bfdat.h"
#else // TEST
#include "bf.h"
#include "bfdat.h"
#endif // TEST
#include <string.h>
#include <iostream.h>

/////////////////////////////////////////////////////////////////////////////
//                                                                         //
// Blow_this is an extension of the blowfish algorithm.  It is placed in   //
// the public domain.  It was written by Mike Morgan, except for the       //
// parts that were copied from Bruce Schneier.                             //
//                                                                         //
// Blowfish was invented by Bruce Schneier.  The algorithm is discussed    //
// in the April 1994 issue of Dr. Dobb's Journal.                          //
//                                                                         //
// This code may be used by anybody for any reason, so long as they        //
// give the author of this code written credit in any software             //
// documentation.                                                          //
//                                                                         //
// However, the author of this code will not be responsible for            //
// any damages resulting from the use or misuse of this code.              //
//                                                                         //
// WARNING:  Export of encryption technology is regulated by the           //
//           United States Government.  Contact an attorney before         //
//           you attempt to export this code.  You might save your-        //
//           self some trouble.                                            //
//                                                                         //
// WARNING:  The functions that encrypt UINT32 alligned buffers are        //
//           dangerous.  If you intend to use thes functions to            //
//           encrypt data on one machine, and decrypt it on another        //
//           You may have to rearrange your data.                          //
//                                                                         //
// Author:  Mike Morgan                                                    //
//          4134 Girard Ave. N.                                            //
//          Minneapolis, MN 55412                                          //
//          (612) 521-8028                                                 //
//                                                                         //
// Inventor:  Bruce Schneier                                               //
//            schneier@chinet.chinet.com                                   //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

//
// The following macro replaces the sblock function, F.
// It results in a significant speed up...
//
#define F(e)  (((blow_this::S0[(e>>24)&0x0ff] \
		+ blow_this::S1[(e>>16)&0x0ff])\
		^ blow_this::S2[(e>>8)&0x0ff])\
		+ blow_this::S3[(e&0x0ff)])

#define UINT32_2CB(b,u) {(b)[3]=u&0xff; \
                         (b)[2]=(u>>8)&0xff; \
                         (b)[1]=(u>>16)&0xff; \
                         (b)[0]=(u>>24)&0xff; }

#define CB2_UINT32(b)   (((UINT32)(b)[0])&0xff) <<24 \
			   |(((UINT32)(b)[1])&0xff)<<16 \
			   |(((UINT32)(b)[2])&0xff)<<8 \
			   |(((UINT32)(b)[3])&0xff) 

//
// Gets muted subkey data
//
blow_this::blow_this(UINT16 p_elements)
{
	INT16	i;

	if((p_elements<MIN_P_ELEMENTS)||(p_elements>MAX_P_ELEMENTS)){
		cerr<< "Number of elements in P must be between "
		    << MIN_P_ELEMENTS
		    << " and "
		    << MAX_P_ELEMENTS
		    <<".\n";

		exit(EXIT_FAILURE);
	}
	p_elements&=(-1^1);
//round down to even number
    blow_this::p_N=p_elements;
    {
        int x;
        x=blow_this::p_N;
        blow_this::P=new UINT32 [x];
    }
    blow_this::S0=new UINT32 [256];
    blow_this::S1=new UINT32 [256];
    blow_this::S2=new UINT32 [256];
    blow_this::S3=new UINT32 [256];
    if(blow_this::P==0||
    blow_this::S0==0||
    blow_this::S1==0||
    blow_this::S2==0||
    blow_this::S3==0){
		cerr << "Out of memory in blow_this constructor.\n";
		exit(EXIT_FAILURE);
	}
//
// Initialize P...
//
// NOTE:  p_N is the number of elements in P,
//        and not the number of p_N used in blow_this.
//
    for(i=0;i<blow_this::p_N;++i)
        blow_this::P[i]=pihex[i];
//
// Initialize S0,S1,S2,S3...
//
// NOTE:  S0-S3 always have 256 elements each, but the
//        initial data from the hexits of pi depend
//        on how many elements of P there were.
//
    for(i=0;i<256;++i)
        blow_this::S0[i]=pihex[blow_this::p_N+i];
	for(i=0;i<256;++i)
        blow_this::S1[i]=pihex[blow_this::p_N+256+i];
	for(i=0;i<256;++i)
        blow_this::S2[i]=pihex[blow_this::p_N+512+i];
	for(i=0;i<256;++i)
        blow_this::S3[i]=pihex[blow_this::p_N+768+i];

}

//
// Hopefully, destroys mutated subkey data...
///
blow_this::~blow_this(void)
{
	INT16	i;

// Set P to 0...
    for(i=0;i<blow_this::p_N;++i)
        blow_this::P[i]=0;
// delete memory...
	{
		UINT32 *t;
		t=blow_this::P;
		delete t;
#ifdef NULL_THE_POINTERS
		blow_this::P=0;
#endif //NULL_THE_POINTERS
	}

// Wipeout S0,S1,S2,S3 to 0...
    for(i=0;i<256;++i)
        blow_this::S0[i]=0;
	for(i=0;i<256;++i)
        blow_this::S1[i]=0;
	for(i=0;i<256;++i)
        blow_this::S2[i]=0;
	for(i=0;i<256;++i)
        blow_this::S3[i]=0;
	{
		UINT32 *t;
		t=blow_this::S0;
		delete t;
		t=blow_this::S1;
		delete t;
		t=blow_this::S2;
		delete t;
		t=blow_this::S3;
		delete t;
#ifdef NULL_THE_POINTERS
		t=0;
		blow_this::S0=blow_this::S1=blow_this::S2=blow_this::S3=t;
#endif // NULL_THE_POINTERS
	}
// wipe any record of the number of p_N...
    blow_this::p_N=0;
}

//
// Mutates subkey data with key...
//
void blow_this::initbf(BUFFER key)
{
	INT16   i,j,k;
	UINT32  data,datal,datar;

	j=0;
//
// NOTE: keylength has been extended to be 4* number of P elements.
//        Thus for 18 P elements (16 rounds) you have a key length
//        of 576 bits.  If you used 200 rounds, your key length
//        becomes 6464 bits.
//
    if(key.N>4*blow_this::p_N){
        cerr << "\007Warning:  Key truncated for blow_this algorithm.\n";
    }
    for(i=0;i<blow_this::p_N;++i){
		data=0x0;
		for(k=0;k<4;++k){
			data=(data<<8)|key.buff[j];
			j+=1;
			if(j==key.N)
				j=0;
		}
        blow_this::P[i]^=data;
	}

	datal=0x0;
	datar=0x0;

    for(i=0;i<blow_this::p_N;i+=2){
        blow_this::bfe(&datal,&datar);
        blow_this::P[i]=datal;
        blow_this::P[i+1]=datar;
	}
	for(i=0;i<256;i+=2){
        blow_this::bfe(&datal,&datar);
        blow_this::S0[i]=datal;
        blow_this::S0[i+1]=datar;
	}
	for(i=0;i<256;i+=2){
        blow_this::bfe(&datal,&datar);
        blow_this::S1[i]=datal;
        blow_this::S1[i+1]=datar;
	}
	for(i=0;i<256;i+=2){
        blow_this::bfe(&datal,&datar);
        blow_this::S2[i]=datal;
        blow_this::S2[i+1]=datar;
	}
	for(i=0;i<256;i+=2){
        blow_this::bfe(&datal,&datar);
        blow_this::S3[i]=datal;
        blow_this::S3[i+1]=datar;
	}
}

//
// Encrypts non-alligned buffer...
//
// overloaded
blow_this::bfebuffer(BUFFER *buffer)
{
    UINT32  left,right;
	if(buffer->N%8){
		cerr << "Error:  buffer size not divisable by 8 in bfebuffer.\n";
		return NOT_DIVISABLE;
	}
// NOTE:  initbf must have already been called...
// Start encryption...

    for(int cnt=0;cnt<buffer->N/8;cnt++){
        left=CB2_UINT32(&buffer->buff[cnt*8]);
		right=CB2_UINT32(&buffer->buff[cnt*8+4]);
        blow_this::bfe(&left,&right);
		UINT32_2CB(&buffer->buff[cnt*8],left);
		UINT32_2CB(&buffer->buff[cnt*8+4],right);
	}
	left=0;
	right=0;
	return NO_ERROR;
}

//
// Encrypts UINT32 alligned buffer...
//
// overloaded
blow_this::bfebuffer(LBUFFER *buffer)
{

	if(buffer->N%2){
		cerr << "Error:  buffer size not divisable by 2 in bfebuffer.\n";
		return NOT_DIVISABLE;
	}
    for(int cnt=0;cnt<buffer->N;cnt+=2)
        blow_this::bfe(&buffer->buff[cnt],&buffer->buff[cnt+1]);
	return NO_ERROR;
}

//
// Unrolled, encryption of nonalligned buffer...
//
// overloaded
blow_this::bfebuffer16(BUFFER *buffer)
{
    UINT32  left,right;

	if(buffer->N%8){
		cerr << "Error:  buffer size not divisable by 8 in bfebuffer16.\n";
		return NOT_DIVISABLE;
	}

// Should check to make sure blow_this::p_N is 18
// (For 16 rounds...)


// Start encryption...
    for(int cnt=0;cnt<buffer->N/8;cnt++){
		left=CB2_UINT32(&buffer->buff[cnt*8]);
		right=CB2_UINT32(&buffer->buff[cnt*8+4]);
        blow_this::bfe16(&left,&right);
		UINT32_2CB(&buffer->buff[cnt*8],left);
		UINT32_2CB(&buffer->buff[cnt*8+4],right);
	}
	left=0;
	right=0;
	return NO_ERROR;
}

//
// Unrolled encryption of UINT32 alligned buffer...
//
//overloaded
blow_this::bfebuffer16(LBUFFER *buffer)
{

	if(buffer->N%2){
		cerr << "Error:  buffer size not divisable by 2 in bfebuffer16.\n";
		return NOT_DIVISABLE;
	}

// Start encryption...
    for(int cnt=0;cnt<buffer->N;cnt+=2)
        blow_this::bfe16(&buffer->buff[cnt],&buffer->buff[cnt+1]);
	return NO_ERROR;
}

//
// General (not unrolled) encryption of non-alligned buffer...
//
//overloaded....
blow_this::bfdbuffer(BUFFER *buffer)
{

    UINT32  left,right;
	if(buffer->N%8){
		cerr << "Error:  buffer size not divisable by 8 in bfdbuffer.\n";
		return NOT_DIVISABLE;
	}
// Start decryption...
    for(int cnt=0;cnt<buffer->N/8;cnt++){
		left=CB2_UINT32(&buffer->buff[cnt*8]);
		right=CB2_UINT32(&buffer->buff[cnt*8+4]);
        blow_this::bfd(&left,&right);
		UINT32_2CB(&buffer->buff[cnt*8],left);
		UINT32_2CB(&buffer->buff[cnt*8+4],right);
	}
	left=0;
	right=0;
	return NO_ERROR;
}


//
// General (not unrolled) encryption of UINT32 alligned buffer...
//
//overloaded....
blow_this::bfdbuffer(LBUFFER *buffer)
{
	if(buffer->N%2){
		cerr << "Error:  buffer size not divisable by 2 in bfdbuffer.\n";
		return NOT_DIVISABLE;
	}
// Start decryption...
    for(int cnt=0;cnt<buffer->N;cnt+=2)
        blow_this::bfd(&buffer->buff[cnt],&buffer->buff[cnt+1]);
	return NO_ERROR;
}


//
// Unrolled decryption of non-alligned buffer...
//
//overloaded...
blow_this::bfdbuffer16(BUFFER *buffer)
{

    UINT32  left,right;
	if(buffer->N%8){
		cerr << "Error:  buffer size not divisable by 8 in bfdbuffer16.\n";
		return NOT_DIVISABLE;
	}
// NOTE:  initbf must have already been called...
// Start decryption...
    for(int cnt=0;cnt<buffer->N/8;cnt++){
		left=CB2_UINT32(&buffer->buff[cnt*8]);
		right=CB2_UINT32(&buffer->buff[cnt*8+4]);
        blow_this::bfd16(&left,&right);
		UINT32_2CB(&buffer->buff[cnt*8],left);
		UINT32_2CB(&buffer->buff[cnt*8+4],right);
	}
	left=0;
	right=0;
	return NO_ERROR;
}

//
// Unrolled, UINT32 alligned decryption of buffer...
//
//overloaded...
blow_this::bfdbuffer16(LBUFFER *buffer)
{

	if(buffer->N%2){
		cerr << "Error:  buffer size not divisable by 2 in bfdbuffer16.\n";
		return NOT_DIVISABLE;
	}
// Start decryption...
    for(int cnt=0;cnt<buffer->N/2;cnt+=2)
        blow_this::bfd16(&buffer->buff[cnt],&buffer->buff[cnt+1]);
	return NO_ERROR;
}

//
// Encrypts 64 bits....
//
void blow_this::bfe(UINT32 *xl,UINT32 *xr)
{
	UINT32 Xl;
	UINT32 Xr;
	UINT32 temp;
	INT16   i;

#ifdef DEBUG
	printf("\nin");
	printf("\n left= %08lx", *xl);
	printf("\nright= %08lx", *xr);
#endif // DEBUG

//set Xl and Xr with lr64
    Xr=*xr;
	Xl=*xl;
    for(i=0;i<blow_this::p_N-2;++i){
        Xl^=blow_this::P[i];
		Xr^=F(Xl);
		temp=Xl;
		Xl=Xr;
		Xr=temp;
	}
	temp=Xl;
	Xl=Xr;
	Xr=temp;

    Xr^=blow_this::P[blow_this::p_N-2];
    Xl^=blow_this::P[blow_this::p_N-1];
//set lr64 with Xl and Xr
    *xr=Xr;
	*xl=Xl;

#ifdef DEBUG
	printf("\n left= %08lx", *xl);
	printf("\nright= %08lx", *xr);
	printf("\nout");
#endif // DEBUG
}

//
// Unrolled encryption of 64 bits...
//
void blow_this::bfe16(UINT32 *xl,UINT32 *xr)
{
	UINT32 Xl;
	UINT32 Xr;


#ifdef DEBUG
	printf("\nin");
	printf("\n left= %08lx", *xl);
	printf("\nright= %08lx", *xr);
#endif // DEBUG

//set Xl and Xr with lr64
    Xr=*xr;
	Xl=*xl;

// round 0...
    Xl^=blow_this::P[0];
	Xr^=F(Xl);

// round 1...
    Xr^=blow_this::P[1];
	Xl^=F(Xr);

// round 2...
    Xl^=blow_this::P[2];
	Xr^=F(Xl);

// round 3...
    Xr^=blow_this::P[3];
	Xl^=F(Xr);

// round 4...
    Xl^=blow_this::P[4];
	Xr^=F(Xl);

// round 5...
    Xr^=blow_this::P[5];
	Xl^=F(Xr);

// round 6...
    Xl^=blow_this::P[6];
	Xr^=F(Xl);

// round 7...
    Xr^=blow_this::P[7];
	Xl^=F(Xr);

// round 8...
    Xl^=blow_this::P[8];
	Xr^=F(Xl);

// round 9...
    Xr^=blow_this::P[9];
	Xl^=F(Xr);

// round 10...
    Xl^=blow_this::P[10];
	Xr^=F(Xl);

// round 11...
    Xr^=blow_this::P[11];
	Xl^=F(Xr);

// round 12...
    Xl^=blow_this::P[12];
	Xr^=F(Xl);

// round 13...
    Xr^=blow_this::P[13];
	Xl^=F(Xr);

// round 14...
    Xl^=blow_this::P[14];
	Xr^=F(Xl);

// round 15...
    Xr^=blow_this::P[15];
	Xl^=F(Xr);

    Xl^=blow_this::P[16];
    Xr^=blow_this::P[17];
// set lr64 with Xl and Xr
// NOTE:  Reverse order due to unrolling...
    *xr=Xl;
	*xl=Xr;

#ifdef DEBUG
	printf("\n left= %08lx", *xl);
	printf("\nright= %08lx", *xr);
	printf("\nout");
#endif // DEBUG
}
//
// General decryption of 64 bits...
//
void blow_this::bfd(UINT32 *xl,UINT32 *xr)
{
	UINT32 Xl;
	UINT32 Xr;
	UINT32 temp;
	INT16   i;

// set Xl and Xr with lr64...
    Xr=*xr;
	Xl=*xl;
    for(i=blow_this::p_N-1;i>1;--i){
        Xl^=blow_this::P[i];
		Xr^=F(Xl);
		temp=Xl;
		Xl=Xr;
		Xr=temp;
	}
	temp=Xl;
	Xl=Xr;
	Xr=temp;

    Xr^=blow_this::P[1];
    Xl^=blow_this::P[0];

// set lr64 with Xl and Xr
    *xr=Xr;
	*xl=Xl;
}
//
// Unrolled decryption of 64 bits....
//
void blow_this::bfd16(UINT32 *xl,UINT32 *xr)
{
	UINT32 Xl;
	UINT32 Xr;
#ifdef DEBUG
	printf("\nin");
	printf("\n left= %08lx", *xl);
	printf("\nright= %08lx", *xr);
#endif // DEBUG

//set Xl and Xr with lr64
    Xr=*xr;
	Xl=*xl;

// round 0...
    Xl^=blow_this::P[17];
	Xr^=F(Xl);

// round 1...
    Xr^=blow_this::P[16];
	Xl^=F(Xr);

// round 2...
    Xl^=blow_this::P[15];
	Xr^=F(Xl);

// round 3...
    Xr^=blow_this::P[14];
	Xl^=F(Xr);

// round 4...
    Xl^=blow_this::P[13];
	Xr^=F(Xl);

// round 5...
    Xr^=blow_this::P[12];
	Xl^=F(Xr);

// round 6...
    Xl^=blow_this::P[11];
	Xr^=F(Xl);

// round 7...
    Xr^=blow_this::P[10];
	Xl^=F(Xr);

// round 8...
    Xl^=blow_this::P[9];
	Xr^=F(Xl);

// round 9...
    Xr^=blow_this::P[8];
	Xl^=F(Xr);

// round 10...
    Xl^=blow_this::P[7];
	Xr^=F(Xl);

// round 11...
    Xr^=blow_this::P[6];
	Xl^=F(Xr);

// round 12...
    Xl^=blow_this::P[5];
	Xr^=F(Xl);

// round 13...
    Xr^=blow_this::P[4];
	Xl^=F(Xr);

// round 14...
    Xl^=blow_this::P[3];
	Xr^=F(Xl);

	// round 15...
    Xr^=blow_this::P[2];
	Xl^=F(Xr);

    Xl^=blow_this::P[1];
    Xr^=blow_this::P[0];

// set lr64 with Xl and Xr
// NOTE:  Reverse order (*xr=Xl) due to unrolling...
    *xr=Xl;
	*xl=Xr;

#ifdef DEBUG
printf("\n left= %08lx", *xl);
printf("\nright= %08lx", *xr);
printf("\nout");
#endif // DEBUG

}

#ifdef TEST
#include <time.h>
void info(void);
void speed_test();
void integrity_test(void);
void reversability_test();
main()
{
	info();
	integrity_test();
	reversability_test();
	speed_test();
	return 0;
}

void speed_test()
{
#define BUFFSIZE 4096l
#define NUM_BUFFS 4096l

	long    cnt;


	static  char  kk[]="Who is John Galt?";
	BUFFER	key;
	BUFFER	buffer;
    LBUFFER lbuffer;


	cout << "\n\nTest:  Speed...\n\n";


	buffer.buff=new char [BUFFSIZE];
	if(buffer.buff==0){
		cerr << "\n\n\007Error:  Out of memory2.\n\n";
		exit(1);
	}
	buffer.N=BUFFSIZE;
	for(cnt=0;cnt<BUFFSIZE;cnt++)
		buffer.buff[cnt]=char(cnt%255);

    lbuffer.buff=new UINT32 [BUFFSIZE/4];
    if(lbuffer.buff==0){
        cerr << "\n\n\007Error:  Out of memory3.\n\n";
        exit(1);
    }
    lbuffer.N=BUFFSIZE/4;
    for(cnt=0;cnt<BUFFSIZE/4;cnt++)
        lbuffer.buff[cnt]=(UINT32)cnt;

	blow_this b(18);
	key.buff=kk;
	key.N=strlen(kk);

// send in the key...
    b.initbf(key);

	{
        double  time1,time2,time3,time4,time5;
        UINT32  left,right;

        clock_t t1s,t1f,t2s,t2f,t3s,t3f,t4s,t4f,t5s,t5f;


        cout << "General, unalligned...\007";
		cout << flush;
		cerr << flush;
		t1s = clock();
		for(cnt=0;cnt<NUM_BUFFS/2;cnt++)
			b.bfebuffer(&buffer);
		for(cnt=0;cnt<NUM_BUFFS/2;cnt++)
			b.bfdbuffer(&buffer);
		t1f = clock();


// Assumes data is UINT32 alligned
        cout << "\nGeneral, alligned...\007";
		cout << flush;
		cerr << flush;
        t2s=clock();
	for(cnt=0;cnt<NUM_BUFFS/2;cnt++)
            b.bfebuffer(&lbuffer);
	for(cnt=0;cnt<NUM_BUFFS/2;cnt++)
            b.bfdbuffer(&lbuffer);
        t2f=clock();

        cout << "\nUnrolled, unalligned...\007";
		cout << flush;
		cerr << flush;
        t3s=clock();
		for(cnt=0;cnt<NUM_BUFFS/2;cnt++)
			b.bfebuffer16(&buffer);
		for(cnt=0;cnt<NUM_BUFFS/2;cnt++)
			b.bfdbuffer16(&buffer);
        t3f=clock();

// Assumes date is UINT32 alligned, 16 round blowfish
        cout << "\nUnrolled, alligned...\007";
		cout << flush;
		cerr << flush;
        t4s=clock();
		for(cnt=0;cnt<NUM_BUFFS/2;cnt++)
            b.bfebuffer16(&lbuffer);
		for(cnt=0;cnt<NUM_BUFFS/2;cnt++)
            b.bfdbuffer16(&lbuffer);
        t4f=clock();



// The unrealistic method...
        cout << "\nIdeal...     \007";
		cout << flush;
		cerr << flush;
        t5s = clock();
        for(cnt=0;cnt<(NUM_BUFFS*BUFFSIZE/16l);cnt++)
			b.bfe16(&left,&right);
        for(cnt=0;cnt<(NUM_BUFFS*BUFFSIZE/16l);cnt++)
			b.bfd16(&left,&right);
        t5f = clock();

//NOTE: No rounding except for display purposes...
        time1=(double)(t1f-t1s)/(double)CLOCKS_PER_SEC;
		time2=(double)(t2f-t2s)/(double)CLOCKS_PER_SEC;
        time3=(double)(t3f-t3s)/(double)CLOCKS_PER_SEC;
        time4=(double)(t4f-t4s)/(double)CLOCKS_PER_SEC;
        time5=(double)(t5f-t5s)/(double)CLOCKS_PER_SEC;
        cout << "\n\nBytes Processed:  "
            << (long)((long)NUM_BUFFS*(long)BUFFSIZE)
            <<" bytes\n\n";
		{
// round 3 or 1 digit for fractional part, and output timings...

            long    tmp,ip,fp;

            tmp=(long)(time1*1000.);
			ip=tmp/1000l;
			fp=tmp%1000l;
            cout <<" General, unalligned = "<<ip<< "." <<fp << " Seconds\n";
            tmp=(long)(10.*(double)(NUM_BUFFS*BUFFSIZE)/(1024.0*time1));
			ip=tmp/10l;
			fp=tmp%10l;
            cout <<"                     = "<<ip<< "." <<fp << " kB/Sec.\n";

            tmp=(long)(time2*1000.);
			ip=tmp/1000l;
			fp=tmp%1000l;
            cout <<"   General, alligned = "<<ip<< "." <<fp << " Seconds\n";
            tmp=(long)(10.*(double)(NUM_BUFFS*BUFFSIZE)/(1024.0*time2));
			ip=tmp/10l;
			fp=tmp%10l;
            cout <<"                     = "<<ip<< "." <<fp << " kB/Sec.\n";

            tmp=(long)(time3*1000.);
			ip=tmp/1000l;
			fp=tmp%1000l;
            cout <<"Unrolled, unalligned = "<<ip<< "." <<fp << " Seconds\n";
            tmp=(long)(10.*(double)(NUM_BUFFS*BUFFSIZE)/(1024.0*time3));
			ip=tmp/10l;
			fp=tmp%10l;
            cout <<"                     = "<<ip<< "." <<fp << " kB/Sec.\n";


            tmp=(long)(time4*1000.);
			ip=tmp/1000l;
			fp=tmp%1000l;
            cout <<"  Unrolled, alligned = "<<ip<< "." <<fp << " Seconds\n";
            tmp=(long)(10.*(double)(NUM_BUFFS*BUFFSIZE)/(1024.0*time4));
			ip=tmp/10l;
			fp=tmp%10l;
            cout <<"                     = "<<ip<< "." <<fp << " kB/Sec.\n";

            tmp=(long)(time5*1000.);
			ip=tmp/1000l;
			fp=tmp%1000l;
            cout <<"               Ideal = "<<ip<< "." <<fp << " Seconds\n";
            tmp=(long)(10.*(double)(NUM_BUFFS*BUFFSIZE)/(1024.0*time5));
			ip=tmp/10l;
			fp=tmp%10l;
            cout <<"                     = "<<ip<< "." <<fp << " kB/Sec.\n";
		}
	}
	delete buffer.buff;
	buffer.buff=0;
}
// This test the reversability of encryption
// for 0 to 224 rounds
void reversability_test()
{
	static  char  kk[]="Who is John Galt?";

	LBUFFER lbuffer;
	BUFFER	key,buffer;
    UINT32  l,r;
	int		cnt,i,failed;

	key.buff=kk;
	key.N=strlen(kk);

	buffer.buff=new char[256];
	buffer.N=256;
	lbuffer.buff=new UINT32[64];
	lbuffer.N=64;
	cout << "\n\nTest:  Reversability...\n\n";
#define RL 0xffff0000
#define RR 0x0000ffff
	for(cnt=MAX_P_ELEMENTS;cnt>=MIN_P_ELEMENTS;cnt-=2){
		{
			blow_this test(cnt);
			test.initbf(key);
			failed=0;

			l=RL;
			r=RR;

			cout << "Rounds = " << cnt -2 << "\t";
			test.bfe(&l,&r);
			test.bfd(&l,&r);
			if(l!=RL || r!=RR)
				failed=1;
			for(i=0;i<256;i++)
				buffer.buff[i]=(char)i;
			test.bfebuffer(&buffer);
			test.bfdbuffer(&buffer);
			for(i=0;i<256;i++)
				if(buffer.buff[i]!=(char)i)
					failed=1;
			for(i=0;i<64;i++)
				lbuffer.buff[i]=(UINT32)i;
			test.bfebuffer(&lbuffer);
			test.bfdbuffer(&lbuffer);
			for(i=0;i<64;i++)
				if(lbuffer.buff[i]!=(UINT32)i)
					failed=1;
			if(failed)
				cerr << "\007Test failed...\n";
				else
				cout << "Test passed...\n";

		}
	}
#undef RL
#undef RR
}

// This tests the standard 16 round blow_fish, using
// both the unrolled and rolled version of the bfe
// function.
void integrity_test()
{
	// Simple test, test known plaintext cyphertext pair
	    // to see if you get the correct result
#define PL 0xfedcba98l
#define PR 0x76543210l
#define CL 0xcc91732bl
#define CR 0x8022f684l
	    static  char  kk[]="Who is John Galt?";
	BUFFER	key;
	UINT32	l,
	    r;

// declare class, construct for 18-2=16 p_N...
    blow_this test(18);

	key.buff=kk;
	key.N=strlen(kk);
// init with key
    test.initbf(key);



	cout << "\n\nTest:  Integrity...\n\n";

// Test bfe16
    cout << "Testing bfe16....\n";
	l=PL;
	r=PR;
	cout << "pl= " << l << "\t";
	cout << "pr= " << r << "\n";
	test.bfe16(&l,&r);
	cout << "cl= " << l << "\t";
	cout << "cr= " << r << "\n";
	if(l!=CL || r !=CR)
		cerr << "\007...test failed.\n\n";
		else
		cout << "...test passed.\n\n";
	// Test bfe
	    cout << "Testing bfe....\n";
	l=PL;
	r=PR;
	cout << "pl= " << l << "\t";
	cout << "pr= " << r << "\n";
	test.bfe(&l,&r);
	cout << "cl= " << l << "\t";
	cout << "cr= " << r << "\n";
	if(l!=CL || r !=CR)
		cerr << "\007...test failed.\n\n";
    else
		cout << "...test passed.\n\n";

// Test bfd16...
    cout << "Testing bfd16...\n";
	l=CL;
	r=CR;
	cout << "cl= " << l << "\t";
	cout << "cr= " << r << "\n";
	test.bfd16(&l,&r);
	cout << "pl= " << l << "\t";
	cout << "pr= " << r << "\n";
	if(l!=PL || r !=PR)
		cerr << "\007...test failed.\n\n";
		else
		cout << "...test passed.\n\n";

	// Test bfd...
	    cout << "Testing bfd...\n";
	l=CL;
	r=CR;
	cout << "cl= " << l << "\t";
	cout << "cr= " << r << "\n";
	test.bfd(&l,&r);
	cout << "pl= " << l << "\t";
	cout << "pr= " << r << "\n";
	if(l!=PL || r !=PR)
		cerr << "\007...test failed.\n\n";
		else
		cout << "...test passed.\n\n";

}

void info(void)
{
	static	char *ragged[]={
		"This is a benchmark program for the c++ implementation",
		"of the Blowfish encryption algorithm.  The algorithm is",
		"described in the April 1994 issue of Dr. Dobb's Journal.",
		"BlowFish was designed by the author of the article,",
		"Bruce Schneier.",
		"\nThis c++ implementation was written by Mike Morgan.",
		"\nExport of encryption technology from the United States",
		"is strictly regulated, and in most cases, illegal.",
		"\nDO NOT EXPORT THIS PROGRAM!!!",
		"\nThis program may be used only for educational purposes."
				};
	int	cnt;

	for(cnt=0;cnt<10;cnt++)
		cout << "\n" << ragged[cnt];
	cout << "\n";

}
#endif // TEST
