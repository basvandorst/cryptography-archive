From kweber Fri Jul  2 11:38 EDT 1993
Return-Path: <kweber>
Received: from Monkey.mcs.kent.edu by condor.mcs.kent.edu (1.37.109.4/condor.93.06.04)
	id AA23902; Fri, 2 Jul 93 11:38:28 -0400
From: kweber (Ken Weber)
Received: by Monkey.mcs.kent.edu (4.1/92.12.20)
	id AA04218; Fri, 2 Jul 93 11:38:23 EDT
Message-Id: <9307021538.AA04218@Monkey.mcs.kent.edu>
Subject: Re: mpn_rshift
To: tege@sics.se (Torbjorn Granlund)
Date: Fri, 2 Jul 1993 11:38:22 -0400 (EDT)
In-Reply-To: <9307021514.AA18738@sics.se> from "Torbjorn Granlund" at Jul 2, 93 05:14:51 pm
X-Mailer: ELM [version 2.4 PL21]
Mime-Version: 1.0
Content-Type: text/plain; charset=US-ASCII
Content-Transfer-Encoding: 7bit
Content-Length: 1147      
Status: RO

> 
>   I must have used the code in the sparc directory, for I only used mpn_sub_n.
>   There is no assembly routine for rshift in either sparc directory, however,
>   so I used the C routine.
> 
>   Should there be an assembly routine for rshift on the sparc?
> 
> No, I think it would not help.  gcc -O2 should produce good code.
> 
.
.
.
This is what I thought.  I guess the fetch latency is so long that here it 
might make sense to put the right shift into the same loop.

Since the binary algorithm isn't going to be used except for < 8 limbs
it's a hard decision to make--considering all the extra work of adding another
assembly routine.

I guess at this point I'd say don't add any more assembly routines.

*Generic C routines added that could later be made into asm routines.
-----------------------------------------------------------------------
> You need to add acommernt before each macro that describes what it
> does.  Write it with as few words as possible, but with all
> information.
> 
OK

*done
-----------------------------------------------------------------------

> 
> I think it would be a slightly faster to put the `if (NOT_SC ...' into
> the default case (in SC_MUL_SWITCH in mpn_bmod.c).
> 

OK
*Eliminated

From kweber Fri Jul  2 12:21 EDT 1993
Return-Path: <kweber>
Received: from Monkey.mcs.kent.edu by condor.mcs.kent.edu (1.37.109.4/condor.93.06.04)
	id AA01446; Fri, 2 Jul 93 12:21:03 -0400
From: kweber (Ken Weber)
Received: by Monkey.mcs.kent.edu (4.1/92.12.20)
	id AA04355; Fri, 2 Jul 93 12:20:57 EDT
Message-Id: <9307021620.AA04355@Monkey.mcs.kent.edu>
Subject: Re: mpn_rshift
To: tege@sics.se (Torbjorn Granlund)
Date: Fri, 2 Jul 1993 12:20:57 -0400 (EDT)
In-Reply-To: <9307021614.AA21780@sics.se> from "Torbjorn Granlund" at Jul 2, 93 06:14:53 pm
X-Mailer: ELM [version 2.4 PL21]
Mime-Version: 1.0
Content-Type: text/plain; charset=US-ASCII
Content-Transfer-Encoding: 7bit
Content-Length: 236       
Status: RO

> 
> Your umul_pphmm macros lacks the CLOBBER_CC stuff.
> gcc2 generates incorrect code without it.
> 
OK--I'll put this in.
Do you have any ideas about why I never noticed a problem?  Or does it 
generate inefficient but correct code?

*My code assumes that the mpn_...mul routines check for this special case.

