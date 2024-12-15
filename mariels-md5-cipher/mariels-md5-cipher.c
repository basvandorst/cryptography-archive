From msuinfo!agate!ames!news.Hawaii.Edu!marielsn Sun Sep 25 19:28:38 1994
Newsgroups: sci.crypt
Path: msuinfo!agate!ames!news.Hawaii.Edu!marielsn
From: marielsn@Hawaii.Edu (Nathan Mariels)
Subject: My encryption algo (source)
Message-ID: <CwpIEq.1us@news.Hawaii.Edu>
Sender: news@news.Hawaii.Edu
Organization: University of Hawaii
X-Newsreader: TIN [version 1.2 PL2]
Date: Sun, 25 Sep 1994 22:28:02 GMT
Lines: 113

A few days ago I posted an algorithm for encryption
using MD5.  The basic feedback was that it was very
weak against know plaintext attacks.  After a little
thinking, I came up with the code below.  I have
also include sample plaintext with output.  If you
have any ideas on how to break it, or make it
better, please tell me.

-Nathan
marielsn@pulua.hcc.hawaii.edu


A few notes on the code:
short is a 16 bit variable
Ptr is char*
BlockMove is a macintosh built in memory copy routine.
(No, it is not stupid to call a OS call for this.  On a
PPC, it will run faster than anything I can do, until I
finish the PPC version.  I am too lazy to fix the 2
lines of code that don't work on a PPC.)

// the output of this function is between 8 and 16 bytes
// longer than the length passed to it

                      //          in,   len,  out
#define  Hash16(x,y)  MD5Buffer ((Ptr)x, 16, (Ptr)y)
#define  XOR8(x,y,z)  for (i = 0; i < 8; i++) z[i] = x[i] ^ y[i];
#define  BMove8(x,y)  BlockMove ((Ptr)x, (Ptr)y, 8)

OSErr EncryptBufferOther ( Ptr in,
                           Ptr out,
                           long length,
                           short decryptFlag,
                           Byte key[16])
{
short    i;
Byte    k1[8], k2[8], hash[16], temp[16], rand[8];
long    loop;
    
    BMove8 (key, k1);
    BMove8 (key + 8, k2);
    
    if (decryptFlag) {
        XOR8 (k1, in, out);
        BMove8 (out, temp + 8);
        in += 8;
    }
    else {
        GetRandom64 (rand);
        BMove8 (rand, temp + 8);
        XOR8 (k1, rand, out);
        out += 8;
    }
    
    BMove8 (k2, temp);
    
    Hash16 (temp, hash);
    
    for (loop = 0; loop < length + 8; loop += 8) {
        XOR8 (hash, in, out);
        BMove8 (k2, temp);
        BMove8 (hash + 8, temp + 8);
        Hash16 (temp, hash);
        
        in += 8;
        out += 8;
    }

    //---- zero out keys
    for (i = 0; i < 8; i++) {
        k1[i] = 0;
        k2[i] = 0;
        hash[i] = 0;
        hash[i + 8] = 0;
        temp[i] = 0;
        temp[i + 8] = 0;
    }
    
    return noErr;
}

-- begin sample input (this line not included in test) --
AAAAAAAAAAAAAAAAAAAAAAAA
AAAAAAAAAAAAAAAAAAAAAAAA

Have a nice day.
Have a nice day.

A couple of months in the laboratory
can frequently save a couple
of hours in the library.

WARNING: Reality.sys has become corrupted.
Reboot universe? y/n/q
-- end sample input (this line not included in test) --


sample output:

begin 0664 AFile
MS<Q3AI)_#M+[]S6$,KN:P;7]A)C6I2;*X^\_C`_JKPL<553<7:M\_?`2`2&L1
M5QVW_&UZ"+9YA6F=3B6R'_<?[/J)C:5,+Q^;T>_,!:2T5Z17/633/6I-UE(W"
M.5ZC>+\A]/1-`$7\+W#_D1*2;^?FB5-*8^.XX5_Z=8I3^G#IEYBY9HY*)"O/>
ME@"BG6+8!C=R4;T6]G].EK,]L:<HXK)T[-^W=U'X+UF&*S'K-8SDE8ID2K<&Y
M_%+JC&-+\B%%P'LP>)T$^"QM0CU3H>Z/[]M(79>(Z<66RMY,\<);LK!O\)O#"
MO^E&=*T,0HY]%P)E$:S<#^=@9'$H;C96L[`,=J?)6JF5=1*25P_MURUYI!;Q-
+F%6*(H.'_!E:KUEE_
``
end
size 281




