From msuinfo!agate!howland.reston.ans.net!pipex!uknet!mcsun!ieunet!tcdcs!rwallace Tue Nov 16 21:35:22 1993
Newsgroups: comp.ai.genetic,sci.crypt
Path: msuinfo!agate!howland.reston.ans.net!pipex!uknet!mcsun!ieunet!tcdcs!rwallace
From: rwallace@cs.tcd.ie (Russell Wallace)
Subject: Re: Strong random number generators?
Message-ID: <1993Nov7.152830.8556@cs.tcd.ie>
Organization: Computer Science, Trinity College Dublin
References: <1993Nov5.183248.29604@cs.tcd.ie> <2bfl7tINN3ne@redwood.csl.sri.com>
Date: Sun, 7 Nov 1993 15:28:30 GMT
Lines: 27
Xref: msuinfo comp.ai.genetic:1663 sci.crypt:20919

boucher@csl.sri.com (Peter K. Boucher) writes:

>In article <1993Nov5.183248.29604@cs.tcd.ie> rwallace@cs.tcd.ie (Russell Wallace) writes:
>>Could someone explain just what test a 'cryptographically strong' random
>>number generator (e.g. Blum-Blum-Shub) passes, that an LCG fails?  To be
>>sure, in principle the LCG is not random, but then neither is any
>>algorithm for generating random numbers; and LCG passes every test I've
>>ever heard about.

>It fails this.

[test program deleted... checks for relative frequencies of different
characters, and the probability of one character following another]

The following random number generator passes your test:

rand (void)
{
	randno = randno * 1103515245 + 12345;
	return (randno >> 16) & 0x7FFF;
}


-- 
"To summarize the summary of the summary: people are a problem"
Russell Wallace, Trinity College, Dublin
rwallace@cs.tcd.ie

