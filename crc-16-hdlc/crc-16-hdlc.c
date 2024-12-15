From msuinfo!uwm.edu!cs.utexas.edu!uunet!mcsun!news.funet.fi!funic!nntp.hut.fi!nntp!iisakkil Tue Apr 20 12:50:56 1993
Newsgroups: comp.programming
Path: msuinfo!uwm.edu!cs.utexas.edu!uunet!mcsun!news.funet.fi!funic!nntp.hut.fi!nntp!iisakkil
From: iisakkil@beta.hut.fi (Mika Iisakkila)
Subject: Re: ALG WANTED: checksum
In-Reply-To: banshee@cats.ucsc.edu's message of 15 Apr 1993 22:55:37 GMT
Message-ID: <IISAKKIL.93Apr19162413@beta.hut.fi>
Sender: usenet@nntp.hut.fi (Usenet pseudouser id)
Nntp-Posting-Host: beta.hut.fi
Organization: Helsinki University of Technology, Finland
References: <1qkp59INNhe9@darkstar.UCSC.EDU>
Date: 19 Apr 93 14:24:13 GMT
Lines: 122

banshee@cats.ucsc.edu (Wailer at the Gates of Dawn) writes:
>I'm checksumming some byte strings by summing the (unsigned) bytes
>into a long.  Caveats?  Better ideas?

Not good. I'm enclosing an algorithm that I've been using for
everything ever since I discovered it. It's very reliable and can be
made very fast, I'm using an implementation of it on an 8051 with 1kB
packets with no problems (I can post that, too, if anyone is foolish
enough to ask).

All thanks to Dave and whoever the original authors are.

--
From medin@catbyte.b30.ingr.com Tue Dec  8 19:43:59 1992
Newsgroups: sci.electronics
From: medin@catbyte.b30.ingr.com (Dave Medin)
Subject: CCITT checksums
Reply-To: medin@catbyte.b30.ingr.com
Organization: Intergraph Corporation, Huntsville AL
Date: Mon, 7 Dec 1992 17:33:39 GMT

I've gotten quite a few replies on my offer to furnish checksum
examples and information. I was also encouraged to post the
test source of a fast table lookup algorithm for the 16 bit CCITT
checksum as used in HDLC (ISO STD 3309) and other applications.
Since the source is not long and is germane to this group, here it is.
It is strongly influenced by the Internet PPP RFC 1331, and credit
for the algorithm is properly attributed to original authors in that document.

Note that regardless of the packet content, the resultant FCS on the
receiving end is always the same if the packet is not corrupt.

----------------------------------Cut Here-----------------------------------
#define FCSINIT	0xFFFF
#define FCSGOOD 0xF0B8
#define FCSSIZE 2	/* 16 bit FCS */
#define BUFSIZE	18	/* No particular reason this has to be 18--it's
                         * just that way for test */

/* 16 bit FCS lookup table per RFC1331 */

unsigned short fcstab[256] = {
0x0000, 0x1189, 0x2312, 0x329b, 0x4624, 0x57ad, 0x6536, 0x74bf, 
0x8c48, 0x9dc1, 0xaf5a, 0xbed3, 0xca6c, 0xdbe5, 0xe97e, 0xf8f7, 
0x1081, 0x0108, 0x3393, 0x221a, 0x56a5, 0x472c, 0x75b7, 0x643e, 
0x9cc9, 0x8d40, 0xbfdb, 0xae52, 0xdaed, 0xcb64, 0xf9ff, 0xe876, 
0x2102, 0x308b, 0x0210, 0x1399, 0x6726, 0x76af, 0x4434, 0x55bd, 
0xad4a, 0xbcc3, 0x8e58, 0x9fd1, 0xeb6e, 0xfae7, 0xc87c, 0xd9f5, 
0x3183, 0x200a, 0x1291, 0x0318, 0x77a7, 0x662e, 0x54b5, 0x453c, 
0xbdcb, 0xac42, 0x9ed9, 0x8f50, 0xfbef, 0xea66, 0xd8fd, 0xc974, 
0x4204, 0x538d, 0x6116, 0x709f, 0x0420, 0x15a9, 0x2732, 0x36bb, 
0xce4c, 0xdfc5, 0xed5e, 0xfcd7, 0x8868, 0x99e1, 0xab7a, 0xbaf3, 
0x5285, 0x430c, 0x7197, 0x601e, 0x14a1, 0x0528, 0x37b3, 0x263a, 
0xdecd, 0xcf44, 0xfddf, 0xec56, 0x98e9, 0x8960, 0xbbfb, 0xaa72, 
0x6306, 0x728f, 0x4014, 0x519d, 0x2522, 0x34ab, 0x0630, 0x17b9, 
0xef4e, 0xfec7, 0xcc5c, 0xddd5, 0xa96a, 0xb8e3, 0x8a78, 0x9bf1, 
0x7387, 0x620e, 0x5095, 0x411c, 0x35a3, 0x242a, 0x16b1, 0x0738, 
0xffcf, 0xee46, 0xdcdd, 0xcd54, 0xb9eb, 0xa862, 0x9af9, 0x8b70, 
0x8408, 0x9581, 0xa71a, 0xb693, 0xc22c, 0xd3a5, 0xe13e, 0xf0b7, 
0x0840, 0x19c9, 0x2b52, 0x3adb, 0x4e64, 0x5fed, 0x6d76, 0x7cff, 
0x9489, 0x8500, 0xb79b, 0xa612, 0xd2ad, 0xc324, 0xf1bf, 0xe036, 
0x18c1, 0x0948, 0x3bd3, 0x2a5a, 0x5ee5, 0x4f6c, 0x7df7, 0x6c7e, 
0xa50a, 0xb483, 0x8618, 0x9791, 0xe32e, 0xf2a7, 0xc03c, 0xd1b5, 
0x2942, 0x38cb, 0x0a50, 0x1bd9, 0x6f66, 0x7eef, 0x4c74, 0x5dfd, 
0xb58b, 0xa402, 0x9699, 0x8710, 0xf3af, 0xe226, 0xd0bd, 0xc134, 
0x39c3, 0x284a, 0x1ad1, 0x0b58, 0x7fe7, 0x6e6e, 0x5cf5, 0x4d7c, 
0xc60c, 0xd785, 0xe51e, 0xf497, 0x8028, 0x91a1, 0xa33a, 0xb2b3, 
0x4a44, 0x5bcd, 0x6956, 0x78df, 0x0c60, 0x1de9, 0x2f72, 0x3efb, 
0xd68d, 0xc704, 0xf59f, 0xe416, 0x90a9, 0x8120, 0xb3bb, 0xa232, 
0x5ac5, 0x4b4c, 0x79d7, 0x685e, 0x1ce1, 0x0d68, 0x3ff3, 0x2e7a, 
0xe70e, 0xf687, 0xc41c, 0xd595, 0xa12a, 0xb0a3, 0x8238, 0x93b1, 
0x6b46, 0x7acf, 0x4854, 0x59dd, 0x2d62, 0x3ceb, 0x0e70, 0x1ff9, 
0xf78f, 0xe606, 0xd49d, 0xc514, 0xb1ab, 0xa022, 0x92b9, 0x8330, 
0x7bc7, 0x6a4e, 0x58d5, 0x495c, 0x3de3, 0x2c6a, 0x1ef1, 0x0f78
};

/* Arbitrary-sized input buffer (arbitrary data). Last two
characters of the array are used for FCS and are not initialized to any specific value */

unsigned char inbuf[BUFSIZE] = {0xff, 0x03, 0xc0, 0x21, 0x01, 0xc9, 0x00, 0x0c,
			       0x01, 0x04, 0x05, 0xdc, 0x08, 0x02, 0x07,
			       0x02};


main()
{
    int i;
    unsigned short fcsval = FCSINIT;

    printf("\n");
    
    for(i=0; i< BUFSIZE - FCSSIZE; i++)
	fcsval = (fcsval >>8) ^ fcstab[(fcsval ^ inbuf[i]) & 0xff];
    printf("FCS value %x\n", fcsval);
    fcsval = ~fcsval;
    inbuf[i++] = fcsval & 0xff;		/* Least significant byte of FCS */
    inbuf[i] = (fcsval >> 8) & 0xff;	/* MSB of FCS */

    printf("'Transmitted' packet:");
    for(i=0; i<BUFSIZE; i++)
	printf("%x ", inbuf[i]);
    printf("\n");

    printf("---Meanwhile, on the other 'side' of the link---\n\n");

    fcsval = FCSINIT;

    for(i=0; i<BUFSIZE; i++)
	fcsval = (fcsval >>8) ^ fcstab[(fcsval ^ inbuf[i]) & 0xff];

    printf("'Received' FCS result %x. ");

    if(fcsval == FCSGOOD)
	printf("FCS Good\n");
    else
	printf("FCS Bad\n");
	
    printf("\n");
}

--
Segmented Memory Helps Structure Software

