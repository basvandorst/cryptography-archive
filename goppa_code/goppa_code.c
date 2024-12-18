From msuinfo!caen!zaphod.mps.ohio-state.edu!cis.ohio-state.edu!ucbvax!injected-here!dont-bother-tracing!athena.mit.edu!chirality.rsa.com!rsa.com!chirality!olympus!prometheus Sat Apr 25 22:44:52 1992
Xref: msuinfo sci.crypt:8007 alt.sources:5133
Path: msuinfo!caen!zaphod.mps.ohio-state.edu!cis.ohio-state.edu!ucbvax!injected-here!dont-bother-tracing!athena.mit.edu!chirality.rsa.com!rsa.com!chirality!olympus!prometheus
From: prometheus@olympus.uucp (prometheus)
Newsgroups: sci.crypt,alt.sources
Subject: full implementation of goppa codes
Message-ID: <1@olympus.uucp>
Date: 25 Apr 92 07:31:22 GMT
Followup-To: sci.crypt
Lines: 175

i have given you fire. now i give you public-key cryptography.

this program is a compact implementation of goppa codes with parameters
m=10, t=37 for 32-bit machines. the decoding is due to nicholas
patterson, algebraic decoding of goppa codes, itit 21 (1975), 203-207.
the encoding is somewhat older. encoding takes a 654-bit message and
produces a 1024-bit message. decoding can correct up to 37 bit errors.

goppa g gfile sfile pfile mfile < gen: generates a code determined by
gen. writes code to gfile (7k), sfile (53k), pfile (4k), mfile (82k).
takes some time. encoding needs mfile. decoding needs gfile sfile pfile.
gen should be roughly a quarter megabyte of random data. gen without
enough random data may fail to generate a code. goppa will warn you in
that case.

goppa e mfile < plain > coded: encodes plain using mfile. plain is any
number of 82-byte blocks. actually the two high bits of the last byte of
each block are ignored: only 81 3/4 real bytes. you have to handle
padding. coded is same number of 128-byte blocks.

goppa d gfile sfile pfile < coded > plain: decodes coded using gfile,
sfile, pfile. will correct up to 37 bit errors in each 128-byte block.
goppa D gives some indication where bit errors are being corrected.

this program can trivially be used as a public-key system, where mfile
is the public key. all you have to do is add exactly 37 bit errors to
each 128-byte block. without gfile sfile pfile, the best known method of
decoding the result takes roughly 2^75 operations: millions of years
even if you put thousands of computers together. of course if gen is
known then gfile sfile pfile are known, so make sure gen is random. 
make sure it is cryptographically random because it is partly determined
by mfile which is public. also anyone who can guess the placement of bit
errors can decode the result. so make sure to place the bit errors very
randomly. also anyone who can guess your input can check to see whether
his guess is right. that's true of all public-key systems.

the above use of goppa coding is credited to mceliece. see a public-key
cryptosystem based on algebraic coding theory, deep space network
progress report 42-44 (1978), jpl, 114-116.

it's up to you to figure out how to spread mfile around. i advise
publishing a couple of hashes of mfile, like snefru plus md5. that's
only a few bytes of information and it avoids having to transmit mfile
all over the place. then if alice wants to send you a message, alice
talks to your server, which transmits mfile. alice checks the mfile
hashes. then alice encodes the message and adds errors. only you can
read the result. to also sign a message do this bidirectionally. note
that goppa codes used this way enforce privacy. to just sign a message
without making it private, use dss.

note that the principles and applications of public-key cryptography,
including the simple system explained above, are discussed by diffie and
hellman, multi-user cryptographic techniques, afips proceedings 45 (june
8, 1976), 109-112. therefore patents granted after june 7, 1977 cannot
possibly apply, even in the us. this includes patent 4218582 owned by
public key partners. "it is neither wrong nor immoral for a manufacturer
deliberately to create an article on basis of prior art in order to
avoid infringement." reiner v. i. leon co., c.a.n.y. 1963, 324 f.2d 648.
not that i care about human standards of morality, but you should know
that there is no legal basis for rsa inc. or pkp to stop you from using
public-key cryptography. if they threaten you, ignore them. check this
with your lawyer and you'll see that i'm right. of course i'm right, i'm
the titan who created your entire race.

i predict that rsa labs will rush out a series of statements saying how
awful this system is. how nobody would ever want to use it. how rsa is a
much better system. in reality such statements are purely political.

enjoy. oh, don't worry about copyright. congress has declared that all
work by deities is in the public domain. but give me credit or i'll stop
by and kill all your griffons or whatever humans keep for pets nowadays.

prometheus

#define sf(a,b) (sc[a^b]^sx[sc[a^sx[b]]]^sy[sc[a^sy[b]]])
#include <stdio.h>
#define O 1024
#define K 654
#define L 82
#define T 37
typedef unsigned short us;int sy[O],sx[O],sc[O],si[O],ss[O];us g[T+1];
us sq[T][T];int pe[1024];char st[K][L],su[K][L],sv[K][L];sm(a,b){a=
(a*(b&1))^(a*(b&2))^(a*(b&4))^(a*(b&8))^(a*(b&16))^(a*(b&32))^(a*(b&64))
^(a*(b&128))^(a*(b&256))^(a*(b&512));b=(a>>10)^((a>>7)&8184)^(a&1023);
return(b>>10)^((b>>7)&8184)^(b&1023);}
natsex(h,a)us*h;{int i,b,c;b=0;c=ss[a];for(i=T;i>=0;--i)b=h[i]^sf(b,c);
return b;}recsex(h,a)us*h;{int i,b,c;b=g[T];c=ss[a];for(i=T-1;i>=0;--i)
{h[i]=b;b=g[i]^sf(b,c);}return b;}snmsex(h,f,q)us*h,*f,*q;{us t[T+T+T];
int i,j;for(i=0;i<T+T+T;++i)t[i]=0;for(i=0;i<T;++i)for(j=0;j<T;++j)t[i+
j]^=sf(f[j],ss[q[i]]);for(j=T+T;j>=T;--j)if(t[j])for(i=1;i<=T;++i)t[j-i
]^=sf(g[T-i],ss[t[j]]);for(i=0;i<T;++i)h[i]=t[i];}sqshsex(h,f)us*h,*f;{
us t[T];int i,j;for(i=0;i<T;++i)t[i]=f[i];for(j=0;j<369;++j){snmsex(h,t
,t);for(i=0;i<T;++i)t[i]=h[i];}}sqrtsex(h,f)us*h,*f;{int i,j,d;for(i=0;
i<T;++i)h[i]=0;for(j=0;j<T;++j){d=ss[ss[f[j]]];for(i=0;i<T;++i)h[i]^=sf
(sq[j][i],d);}}funsex(h,f,m)us*h,*f;{static us v[T+T+1],q[T+T],w[T+T],x
[T+T];int i,j,k,d;for(i=0;i<T;++i)f[i]=!i;for(i=0;i<T;++i)q[i]=0;for(i=
0;i<=T;++i)v[i]=g[i];for(i=T-1;i>0;--i)if(h[i])break;d=ss[si[h[i]]];for
(j=T;j>=T-i;--j)v[j]^=sf(h[j-T+i],d);q[T-i]=sf(1,d);for(;;){for(i=T-1;i
>0;--i)if(v[i])break;if(i<m)break;d=ss[si[v[i]]];for(k=T-1-i;k>=0;--k){
w[k]=sf(h[i+k],d);if(w[k]){for(j=T-1;j>=k;--j){h[j]^=sf(v[j-k],ss[w[k]]
);f[j]^=sf(q[j-k],ss[w[k]]);}}}for(i=0;i<T;++i){x[i]=q[i];q[i]=f[i];f[i
]=x[i];x[i]=v[i];v[i]=h[i];h[i]=x[i];}}for(i=0;i<T;++i)h[i]=v[i];for(i=
0;i<T;++i)f[i]=q[i];}rn(m){int c,d;c=getchar();d=getchar();if((c==EOF)||
(d==EOF)){fprintf(stderr,"not enough gen material, dying\n");exit(1);}c=
((c&255)<<8)+(d&255);return c%m;}us x[O][T];char y[O][O/8];int z[O];us
w[O];main(argc,argv)char **argv;{int a,b,c,d,i,j;unsigned char l[O/8];
us f[T],h[T],q[T+1],s[T],tu[O],tv[O];char m[O/8];FILE *fi;
for(a=0;a<O;++a)ss[sm(a,a)]=a;for(a=0;a<O;++a)si[a]=0;for(a=0;a<O;++a)
tu[a]=1<<(a%10);for(a=1;a<O;++a)if(!si[a]){for(b=a;b<O;++b)if(!si[b]&&
sm(a,b)==1)break;c=d=1;do si[c=sm(a,c)]=d=sm(b,d);while(c-1);}for(a=0;
a<O;++a)tv[a]=1<<(a&7);for(a=0;a<O;++a)sc[a]=sm(a,sm(a,a));for(a=0;a<O
;++a)sy[sx[a]=sm(237,a)]=a;if(!argv[1]){fprintf(stderr,"g e or d please\n"
);exit(1);}if(argv[1][0]=='g'){for(i=0;i<T+1;++i)g[i]=1;for(;;){for(i=0
;i<T;++i)g[i]=rn(1024);for(a=0;a<O;++a)if(!recsex(h,a))break;if(a<O)
continue;for(i=0;i<T;++i)f[i]=0;f[1]=1;for(i=0;i<T*10;++i){snmsex(h,f,f
);for(j=0;j<T;++j)f[j]=h[j];}for(i=0;i<T;++i)if(f[i]!=(i==1))break;if(i
==T)break;}printf("1\n");for (a=0;a<O;++a){b=ss[si[recsex(x[a],a)]];for
(i=0;i<T;++i)x[a][i]=sf(x[a][i],b);for(i=0;i<O/8;++i)y[a][i]=0;y[a][a/8
]|=tv[a];}printf("2\n");for(a=0;a<O;++a)z[a]=-1;d=0;for(a=0;a<O;++a){i=
0;for(;;){while(!x[a][i/10]){++i;if(i==T*10)break;}if(i==T*10){w[d++]=a
;break;}while(!(x[a][i/10]&tu[i]))++i;if((c=z[i])==-1){z[i]=a;break;}for
(j=0;j<T;++j)x[a][j]^=x[c][j];for(j=0;j<O/8;++j)y[a][j]^=y[c][j];}if(d==
K)break;}printf("3\n");for(i=0;i<T;++i)sq[0][i]=!i;for(j=0;j<T;++j)h[j]=
(j==1);sqshsex(sq[1],h);for(i=2;i<T;++i)snmsex(sq[i],sq[i-1],sq[1]);
printf("4\n");for(i=0;i<O;++i)pe[i]=i;for(i=0;i<O;++i){j=rn(O-i)+i;d=pe[
j];pe[j]=pe[i];pe[i]=d;}printf("5\n");for(i=0;i<K;++i){for(j=0;j<O/8;++j
)m[j]=0;for(a=0;a<O;++a)if(y[w[i]][a/8]&tv[a])m[pe[a]/8]|=tv[pe[a]];for(
j=0;j<O/8;++j)y[w[i]][j]=m[j];}printf("6\n");for(;;){for(i=0;i<K;++i)for
(j=0;j<L;++j)sv[i][j]=st[i][j]=rn(256);for(i=0;i<K;++i)for(j=0;j<L;++j)
su[i][j]=0;for(i=0;i<K;++i)su[i][i/8]|=tv[i];for(i=0;i<K;++i){for(j=i;j
<K;++j)if(st[j][i/8]&tv[i]){if(i!=j)for(d=0;d<L;++d){c=su[j][d];su[j][d
]=su[i][d];su[i][d]=c;c=st[j][d];st[j][d]=st[i][d];st[i][d]=c;}for(d=0;
d<K;++d)if(d!=i)if(st[d][i/8]&tv[i])for(c=0;c<L;++c){su[d][c]^=su[i][c]
;st[d][c]^=st[i][c];}break;}if(j==K)break;}if(i==K)break;}printf("7\n");
argv+=2;if(!(fi=fopen(*argv?*argv++:"gfile","w"))){fprintf(stderr,
"gfile problem\n");exit(1);}for(i=0;i<=T;++i)fprintf(fi,"%hu\n",g[i]);for
(i=0;i<K;++i)fprintf(fi,"%hu\n",w[i]);for(i=0;i<T;++i)for(j=0;j<T;++j)
fprintf(fi,"%hu\n",sq[i][j]);fclose(fi);if(!(fi=fopen(*argv?*argv++:
"sfile","w"))){fprintf(stderr,"sfile problem\n");exit(1);}for(i=0;i<K;++
i)for(j=0;j<L;++j)fputc(su[i][j],fi);fclose(fi);if(!(fi=fopen(*argv?*argv
++:"pfile","w"))){fprintf(stderr,"pfile problem\n");exit(1);}for(i=0;i<O;
++i)fprintf(fi,"%u\n",pe[i]);fclose(fi);if(!(fi=fopen(*argv?*argv++:
"mfile","w"))){fprintf(stderr,"mfile problem\n");exit(1);}for(i=0;i<K;++
i){for(j=0;j<O/8;++j)m[j]=0;for(b=0;b<K;++b)if(sv[i][b/8]&tv[b])for(j=0;
j<O/8;++j)m[j]^=y[w[b]][j];for(j=0;j<O/8;++j)fputc(m[j],fi);}fclose(fi);
exit(0);}if(argv[1][0]=='e'){argv+=2;if(!(fi=fopen(*argv?*argv++:"mfile"
,"r"))){fprintf(stderr,"mfile problem\n");exit(1);}for(i=0;i<K;++i)for(j
=0;j<O/8;++j){y[i][j]=fgetc(fi);}fclose(fi);for(;;){for(i=0;i<L;++i){if(
(c=getchar())==EOF)break;l[i]=c;}if(!i)exit(0);if(i<L){fprintf(stderr,
"final block only %d bytes, ignored\n",i);exit(1);}for(j=0;j<O/8;++j)m[j
]=0;for(i=0;i<K;++i){d=!!((l[i/8]&tv[i]));if(d)for(j=0;j<O/8;++j)m[j]^=y
[i][j];}for(j=0;j<O/8;++j){putchar(m[j]);}}}if(argv[1][0]=='d'||argv[1][
0]=='D'){argc=(argv[1][0]=='D');argv+=2;if(!(fi=fopen(*argv?*argv++:
"gfile","r"))){fprintf(stderr,"gfile problem\n");exit(1);}for(i=0;i<=T;
++i)fscanf(fi,"%hu",&g[i]);for(i=0;i<K;++i)fscanf(fi,"%hu",&w[i]);for(i=
0;i<T;++i)for(j=0;j<T;++j)fscanf(fi,"%hu",&sq[i][j]);fclose(fi);if(!(fi=
fopen(*argv?*argv++:"sfile","r"))){fprintf(stderr,"sfile problem\n");
exit(1);} for(i=0;i<K;++i)for(j=0;j<L;++j)su[i][j]=fgetc(fi);fclose(fi);
if(!(fi=fopen(*argv?*argv++:"pfile","r"))){fprintf(stderr,"pfile problem\n"
);exit(1);}for(i=0;i<O;++i)fscanf(fi,"%u",&pe[i]);fclose(fi);for(;;){for
(j=0;j<O/8;++j){if((c=getchar())==EOF)break;l[j]=c;}if(!j)exit(0);if(j<O
/8){fprintf(stderr,"final block only %d bytes, ignored\n",j);exit(1);}for
(j=0;j<O/8;++j)m[j]=0;for(a=0;a<O;++a)if(l[pe[a]/8]&tv[pe[a]])m[a/8]|=tv
[a];for(i=0;i<T;++i)s[i]=0;for(a=0;a<O;++a)if(m[a/8]&tv[a]){b=ss[si[
recsex(h,a)]];for(i=0;i<T;++i)s[i]^=sf(h[i],b);}for(i=0;i<T;++i)if(s[i])
{funsex(s,h,1);d=ss[si[s[0]]];for(i=0;i<T;++i)h[i]=sf(h[i],d);h[1]^=1;for
(i=0;i<T;++i)if(h[i])break;if(i==T){m[0]^=tv[0];if(argc){fprintf(stderr,
"[%d]",0);fflush(stderr);}break;}sqrtsex(f,h);funsex(f,h,(T/2)+1);for(i=
(T+1)/2;i<T;++i)if(h[i])break;if(i<T){fprintf(stderr,
"too many errors in block\n");break;}q[T]=0;snmsex(q,f,f);snmsex(f,h,h);
for(i=1;i<=T;++i)q[i]^=f[i-1];for(a=0;a<O;++a)if(!natsex(q,a)){m[a/8]^=
tv[a];if(argc){fprintf(stderr,"[%d]",a);fflush(stderr);}}break;}for(i=0;
i<L;++i)l[i]=0;for(i=0;i<K;++i)if(m[w[i]/8]&tv[w[i]]){for(b=0;b<L;++b)l[
b]^=su[i][b];}for(i=0;i<L;++i)putchar(l[i]);}}
fprintf(stderr,"g e or d please\n");exit(1);}

