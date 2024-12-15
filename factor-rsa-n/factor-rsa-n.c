/*-----
From msuinfo!caen!destroyer!cs.ubc.ca!van-bc!vanbc.wimsey.com!van-bc!vanbc.wimsey.com!not-for-mail Tue Jun 15 19:46:59 1993
Path: msuinfo!caen!destroyer!cs.ubc.ca!van-bc!vanbc.wimsey.com!van-bc!vanbc.wimsey.com!not-for-mail
From: markh@vanbc.wimsey.com (Mark C. Henderson)
Newsgroups: sci.crypt
Subject: Re: Security of RSA
Date: 10 Jun 1993 17:29:36 -0700
Organization: Wimsey
Lines: 294
Message-ID: <1v8jlg$8t4@vanbc.wimsey.com>
References: <82999@cup.portal.com> <EACHUS.93Jun9114201@spectre.mitre.org> <1993Jun9.184557.19364@kth.se> <1v6gb2$ahg@vanbc.wimsey.com>
NNTP-Posting-Host: vanbc.wimsey.com

In article <1v6gb2$ahg@vanbc.wimsey.com> markh@vanbc.wimsey.com (Mark C. Henderson) writes:
>In article <1993Jun9.184557.19364@kth.se> md85-epi@hemul.nada.kth.se (Urban Koistinen) writes:
>>Brassard state that while RSA has not been proved to be as hard as factoring,
>>the Blum-Goldwasser probabilistic encryption scheme has been.
>>"Even should factoring be genuinely hard, breaking RSA is not known to
>>be equally hard." pg 30.
>However, note that a efficient algorithm for computing d (the RSA secret
>exponent) can be used to give an efficient probabilistic algorithm
>for factoring the modulus n. (see Salomaa's book, _Public Key
>Cryptography_, Springer Verlag, 1990, Theorem 4.1 on pages 143-5)

In fact, I'll take this a little further. Here's some code which
will factor n given 
e,d and n in the RSA setup (e is the public exponent, d the private 
exponent and n the modulus)

This one runs in polynomial time (actually in practice it is very
fast, it usually doesn't take more than a couple of seconds to factor
n given e,d with a 512 bit n) and assumes the generalised Riemann
Hypothesis. It can be converted into a probabilistic polynomial time
algorithm which doesn't need the generalised Riemann Hypothesis to work.

I haven't spent much time testing this, so there might be some
bugs here. I've also included five sample (e,d,n) triples you can
try it out with, each with a ~512 bit n. (These keys were generated
with RIPEM).

What does this show. Practically finding the secret exponent d is just
as hard as factoring. Theoretically, there is still a gap.
----------*/
/* Given (n,e,d) where e,d are RSA public and secret exponents respectively
 *  factor n 
 * uses GNU gmp 1.3.1
 * input is e,d,n each on one line in base 16 
 * relies on the generalised Riemann Hypothesis
 * typically takes a few seconds to find the factors of n given (n,e,d)
 *  where n is 512 bits.
 * see: Salomaa, _Public-Key Cryptography_, Springer Verlag, pp. 143-145
 * Mark Henderson <markh@wimsey.bc.ca>. This is hereby placed in the public 
 * domain.
 */

#include <stdio.h>
#include "gmp.h"

main()
{
    MP_INT e,d,p,q,r,pp,t,w,nMinus1,oldt,u,n;
    char nbuf[2048];
    unsigned int ww,s,nbits,i;
    int found;
    
    mpz_init(&e); mpz_init(&d); mpz_init(&p); mpz_init(&q); mpz_init(&r);
    mpz_init(&pp); mpz_init(&t); mpz_init(&w), mpz_init(&nMinus1);
    mpz_init(&oldt);mpz_init(&u);mpz_init(&n);


    gets(nbuf);
    mpz_set_str(&e,nbuf,16);
    gets(nbuf);
    mpz_set_str(&d,nbuf,16);
    gets(nbuf);
    mpz_set_str(&n,nbuf,16);
    printf("\n\n");

    printf("e="); mpz_out_str(stdout, 16, &e);printf("\n");
    printf("d="); mpz_out_str(stdout, 16, &d);printf("\n");
    printf("n="); mpz_out_str(stdout, 16, &n);printf("\n");

    /* nMinus1 = n - 1 */
    mpz_sub_ui(&nMinus1,&n,1);

    /* pp = ed - 1 */
    mpz_mul(&pp,&e,&d); mpz_sub_ui(&pp,&pp,1);
    printf("ed-1="); mpz_out_str(stdout, 16, &pp);printf("\n");


    /* find number of bits in n */
    nbits = 0; 

    /* t = n */
    mpz_set(&t,&n);

    /* for (;t!=0;t/=2) */
    for (;mpz_cmp_ui(&t,0);mpz_div_2exp(&t,&t,1)) 
            nbits++;

    /* express pp as 2^s * r where r is odd */

        /* r = pp */
        mpz_set(&r,&pp);

        s = 0;

        /* for (; r is even ; r/=2,s++) */
        for (;!(mpz_get_ui(&r) % 2);mpz_div_2exp(&r,&r,1),s++)
            ;

    printf("r="); mpz_out_str(stdout, 16, &r);printf("\n");
    printf("s=%d\n",s);
    printf("nbits=%d\n",nbits);
    

    /* now we check values of w between st 1 < w < 2*nbits 
        - which is overkill*/

    found = 0;

    /* really this is simple: the algorithm goes as follows:
       our goal is to find p, which is a non-trivial factor of n.

    pseudocode
    for (w=2; w < 2*nbits; w++) {
        t[0] = w^r mod n;
        t[1] = t[0]*t[0] mod n;
        if (gcd(w,n) != 1) {
            n had a small factor -- we're done 
            p=gcd(w,n);
            exit;
        }
        if (t[0] != 1 && t[0] != (n-1)) {
            for (i=1;i<=s;i++) {
                 LOOP INVARIANT t[i-1] != 1 && t[i-1] != (n-1) 
                    && t[i] = w^{(2^i) * r)} mod n 
                if (t[i] == 1 && t[i-1] != (n-1)) {
                    we've found a non-trivial square root of 1 mod n 
                    p=gcd(t[i-1]+1,n);
                    we're done, p is a factor
                    exit;
                }
                if (t[i] == 1 || t[i] == (n-1))
                    don't bother, go on to next value of w 
                    break;
                t[i+1] = t[i] * t[i] mod n;
            }
        }
    }
    */
                
    
    for (ww=2; !found && ww < (nbits+nbits); ww++) {

        /* w = ww */
        mpz_set_ui(&w,ww);
        printf("\nw="); mpz_out_str(stdout, 16, &w);printf("\n");

        /* if gcd(w,n) != 1 */
        mpz_gcd(&t,&w,&n);
        if (mpz_cmp_ui(&t,1))  {
            /* we've found a factor of n */
            mpz_set(&p,&t);
            found = 1;
            break;
        }

        /* t = w^r mod n */
        mpz_powm(&t,&w,&r,&n);
        printf("w^r mod n="); mpz_out_str(stdout, 16, &t);printf("\n");

        /* if t != n-1 and t != 1*/
        if (mpz_cmp_ui(&t,1) && mpz_cmp(&t,&nMinus1)) { 

            printf("t="); mpz_out_str(stdout, 16, &t);printf("\n");
            /* oldt = t */
            mpz_set(&oldt,&t);

            /* t = t^2 mod n */
            mpz_mul(&t,&t,&t); mpz_mod(&t,&t,&n);

            for (i=1;!found && i<=s;i++) {

                printf("t="); mpz_out_str(stdout, 16, &t);printf("\n");

                /* if t == 1 */
                if (!mpz_cmp_ui(&t,1)) {

                    /* if oldt != (-1) */
                    if (mpz_cmp(&oldt,&nMinus1)) {
                        
                        /* oldt is a non-trivial square root of 1 mod n */

                        /* p = gcd(oldt+1,n) */
                        mpz_set(&u,&oldt); mpz_add_ui(&u,&u,1); 
                        mpz_gcd(&p,&u,&n);
                        found = 1;
                        printf("AHA, oldt="); 
                        mpz_out_str(stdout, 16, &oldt);
                        printf(" is a non-trivial square root of 1 mod n\n");
                        break;

                    }
                    else 
                        /* we're in the case where t==1 and oldt==(n-1) */
                        break;
                }
                
                /* if t==(n-1) or 1--we're not going to find a ntsqrt here */
                if (!mpz_cmp(&t,&nMinus1) || !mpz_cmp_ui(&t,1))
                    break; 
                
                /* oldt = t */
                mpz_set(&oldt,&t);

                /* t = t^2 mod n */
                mpz_mul(&t,&t,&t); mpz_mod(&t,&t,&n);
            } /* loop over i=1,...,s */
        }
    } /* loop over w=2,...,2*nbits */

    if (!found) {
        printf("FAILED\n");
        exit(1);
    }
    /* p is a factor */
    mpz_div(&q,&n,&p);
    printf("p="); mpz_out_str(stdout, 16, &p);printf("\n");
    printf("q="); mpz_out_str(stdout, 16, &q);printf("\n");
}

/*--------
SAMPLE INPUTS (uuencoded)


begin 640 sample1
M,#$P,#`Q"C`Q.$,Q,$$P1#=",#,T0T4X-S%%1C(P13%#1C%"0CDS1#!$,$4V
M13DW-C-"1C9%,#)"0T,X0C4Y,3@S-S<V13=$145&-T9%-3DP03$Y,3`T,3$R
M,40Y1D-&135$-$%%-$4X,T1$0T1!,#<U.38V13@Q038Q,S8Y,T4X.#4Q1#,P
M0SE!.0HP,D,T-S,T,T8P,$,W1C$R.#5",S@W,$,P,$9!,C0S-T,Y034S0S5$
M,S`Q-CA&131!,3$Y-D8Q0T8X1C!!,3$V1C<U1#8Y,T,S1C8U0T-!,#E#,C1$
M-T5%045$03=%,$,Y.4(S-$0U-C<S-#(W-S4V,4$W.$(T.4(Y-CA!-T$W,$,P
$.#<*"D5$
`
end


begin 640 sample2
M,#$P,#`Q"C=",4%!-T8T-#(Q134W.3$U,D,W0S0S-S$T1#9!-35#1#,Q1#A&
M,S8W04$U,S,Y.#`P-S8Q1D4Q0S9%-D8T,3=#-S1".$(V-30R,40T.$5!.#5#
M04)$.#)%0S,R,45",C@P-38T1C!&-#8X0T$Q0D,W13(T.3<W1#<X-S%%0C$V
M1D0*1#E#-C(W1#8Q0S<Y-3,W04$V.#<S,C8V0SE%.3DU0D(W,$1$0C$X,T-&
M-3E",T,S,#8W.4,U1$(W13`R149$,$$V1C<Y-4-&.$9&.38Y1C8R-S,Y1D$U
M030X1C8R145$-C%&,S`T-$9$-38U1#1$1#9$,S)"-#,X149$-S0T-S8U.0H*
`
end



begin 640 sample3
M,#$P,#`Q"C`T.41",D%$1D1"-#(U13!!130U-S8R0S8U0D8V-$)#1#E!140X
M,3=&,C<P-38U,T0U1C<X-T%!-41&,C=%,$(W.35$0T(P13DU.3!&-#0W13!"
M,T0R-$5$130Y,D,Y13!$,4-".31$-T%!,T$Q0D8Q-3(Y-#0V1$-$,$$V1C-!
M,C@X,0HP-C,U.49&1C$S0S,V0S(Y,#<V-$1$-C5#-D,Q-#E%,D8T-C0Q-#$P
M1CE!.$)%13DW,3@U1#9!1#4V-C4U,40Q0T8U13@P-S%!,C0T,$8Q,SE&,#5!
M,#5&1D%%,D4Y-$8S,44X0T(S,D0T-S`X,$(U1C(W0S=&.39%0C9"14(W031!
#.3<*
`
end



begin 640 sample4
M,#$P,#`Q"C`S-D,S03,R.#DP13$V,S`P,$4R-49!0S4R,D4Q0C-"04(V,#@V
M.#,W139%1C`Q0T%$0T(T04$V1$)$1C`R-C=$-CDU1D%"030Y04)",#1",S4Y
M13`U,41#13<R1D,S-S=&135#.3DY1#<Y1#4T,S@V,3DS.#(S,S0X,44P1#0Y
M1#$*,#4W0T$X1C9#034P-D,V-$9#.$)".#,T.#)&-D5$1#9#.4%&-D5&,D5"
M,C,U,C$W-C@P1C="-S8P-S)#13,R,#$Y-C,U-4,X.4,P-C<P0C,W1#9&,CDT
M1D$T.#$W144Q13<P,C(U-C9&,3=#.$9",C1#.$(U041!,4$Y0D$Q,35!-PH*
`
end




begin 640 sample5
M,#$P,#`Q"C`S-D,S03,R.#DP13$V,S`P,$4R-49!0S4R,D4Q0C-"04(V,#@V
M.#,W139%1C`Q0T%$0T(T04$V1$)$1C`R-C=$-CDU1D%"030Y04)",#1",S4Y
M13`U,41#13<R1D,S-S=&135#.3DY1#<Y1#4T,S@V,3DS.#(S,S0X,44P1#0Y
M1#$*,#4W0T$X1C9#034P-D,V-$9#.$)".#,T.#)&-D5$1#9#.4%&-D5&,D5"
M,C,U,C$W-C@P1C="-S8P-S)#13,R,#$Y-C,U-4,X.4,P-C<P0C,W1#9&,CDT
M1D$T.#$W144Q13<P,C(U-C9&,3=#.$9",C1#.$(U041!,4$Y0D$Q,35!-PH*
`
end


to run compile gmp and this and then
facdemo <sample1
facdemo <sample2
facdemo <sample3

&c.
-- 
Mark Henderson
markh@wimsey.bc.ca (personal account)
RIPEM key available by key server/finger/E-mail
  MD5OfPublicKey: F1F5F0C3984CBEAF3889ADAFA2437433
----------*/
