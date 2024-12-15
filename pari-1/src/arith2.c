/*********************************************************************/
/*********************************************************************/
/**                                                                 **/
/**                    FONCTIONS ARITHMETIQUES                      **/
/**                                                                 **/
/**                       (deuxieme partie)                         **/
/**                                                                 **/
/**                      copyright Babe Cool                        **/
/**                                                                 **/
/**                                                                 **/
/*********************************************************************/
/*********************************************************************/

#include  "genpari.h"

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~                                                                     ~*/
/*~                     NOMBRES PREMIERS                                ~*/
/*~                                                                     ~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


GEN prime (long n)
{
  byteptr p=diffptr;
  long c,prime=0;
  while (n--) if (c= *p++) prime += c; else err(primer1);
  return stoi(prime);
}

GEN primes (long n)
{
  GEN y,z;
  byteptr p=diffptr;
  long c,prime=0;
  z=y=cgetg(n+1,17);
  while (n--)
    {
      if (c= *p++) prime+=c; else err(primer1);
      *++z=(long)stoi(prime);
    }
  return y;
}

byteptr initprimes(long maxnum)
{
  register long k,size=(maxnum+257)>>1;
  byteptr p=(byteptr)calloc(size,1);
  register byteptr q,r,s,fin=p+size;
  for(r=q=p,k=1;r<fin;)
    {
      do {r+=k; k+=2; r+=k;} while (*++q);
      for(s=r;s<fin;s+=k) *s=1;
    }
  r=p; *r++=2; *r++=1;
  for(s=q=r-1;;)
    {
      while (*++q);
      if (q>=fin) break;
      *r++=(q-s)<<1;
      s=q;
    }
  *r++=0;
  return (byteptr)realloc(p,r-p);
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~                                                                     ~*/
/*~                   FONCTIONS ARITHMETIQUES DE BASE                   ~*/
/*~                                                                     ~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/* Attention, le parametre doit etre une variable. */
#define pseudoprime(p) millerrabin(p,5*lgef(p))

GEN gmu(GEN n)
{
  long l,tx,i;GEN y;

  if((tx=typ(n))>=17)
    {
      l=lg(n);y=cgetg(l,tx);for(i=1;i<l;i++) y[i]=(long)gmu((GEN)n[i]);
      return y;
    }
  if(tx!=1) err(arither1);
  return stoi(mu(n));
}

long mu(GEN n)
{
  byteptr d=diffptr;
  unsigned char c;
  GEN p, f;
  long av=avma,s=1;
  if (typ(n)!=1) err(arither1);
  if (!signe(n)) err(arither2);
  if ((n[2]==1)&&(lgef(n)==3)) return 1;
  n=absi(n);
  p=cgeti(3);p[1]=evalsigne(1)+evallgef(3);p[2]=0;
  while (c= *d++)
    {
      p[2]+=c;
      if (!mpdivis(n,p,n)) continue;
      if (divise(n,p)) {avma=av;return 0;}
      s= -s;
      if ((n[2]==1)&&(lgef(n)==3)) break;
    }
  while ((n[2]!=1)||(lgef(n)!=3))
    {
      f=gcopy(n);
      while (!((cmpii(mulii(p,p),f)>0)||pseudoprime(f))) f=ellfacteur(f);
      diviiz(n,f,n);
      if (divise(n,f)) {avma=av;return 0;}
      s= -s;
    }
  avma=av;
  return s;
}

GEN phi(GEN n)
{
  byteptr d=diffptr;
  unsigned char c;
  GEN f,p,m;
  long av1,av2,tx,i,l;
  
  if((tx=typ(n))>=17)
    {
      l=lg(n);p=cgetg(l,tx);for(i=1;i<l;i++) p[i]=(long)phi((GEN)n[i]);
      return p;
    }
  if (tx!=1) err(arither1);
  if (!signe(n)) err(arither2);
  if ((n[2]==1)&&(lgef(n)==3)) return gun;
  m=cgeti(lgef(n));affsi(1,m);
  av1=avma;
  n=absi(n);
  p=cgeti(3);p[1]=evalsigne(1)+evallgef(3);p[2]=0;
  av2=avma;
  while (c= *d++)
    {
      p[2]+=c;
      if (!mpdivis(n,p,n)) continue;
      muliiz(m,subis(p,1),m);
      while (mpdivis(n,p,n)) muliiz(m,p,m);
      if ((n[2]==1)&&(lgef(n)==3)) break;
      avma=av2;
    }
  while ((n[2]!=1) || (lgef(n)!=3))
    {
      f=gcopy(n);
      while (!((cmpii(mulii(p,p),f)>0) || pseudoprime(f))) f = ellfacteur(f);
      mpdivis(n,f,n);
      muliiz(m,subis(f,1),m);
      while (mpdivis(n,f,n)) muliiz(m,f,m);
      avma=av2;
    }
  avma=av1;
  return m;
}

GEN auxdecomp(GEN n, long all)
{
  byteptr d=diffptr;
  unsigned char c;
  GEN p,z,p1,p2,f;
  long nb=0,nbx=0,j,k,lim,av1,av2,av3,av4,sn;
  
  if (typ(n)!=1) err(arither1);
  if (!(sn=signe(n))) err(arither2);
  if(sn<0) {stoi(-1);stoi(1);nb++;}
  if ((n[2]!=1) || (lgef(n)!=3))
    {
      av1=avma;
      n=absi(n);
      p=cgeti(3);p[1]=evalsigne(1)+evallgef(3);p[2]=0;
      av2=avma;lim=(all<=1)?VERYBIGINT:all;
      while ((c= *d++)&&(p[2]<=lim))
        {
          p[2]+=c;
          if (!mpdivis(n,p,n)) continue;
          nb++;
          for (k=1;mpdivis(n,p,n);k++);
          gcopy(p);
          stoi(k);
          if ((n[2]==1)&&(lgef(n)==3)) break;
        }
      if(!gcmp1(n))
	{
	  av3=avma;
	  if(carrecomplet(n,&f))
	    {
	      affii(f,n);avma=av3;
	      do
		{
		  av3=avma;
		  f=n;
		  if(all==1)
		    while (!((cmpii(mulii(p,p),f)>0)||pseudoprime(f))) 
		      {f=ellfacteur(f);nbx=1;}
		  av4=avma;
		  f=gerepile(av3,av4,gcopy(f));
		  nb++;
		  for (k=0;mpdivis(n,f,n);k++);
		  stoi(k<<1);
		}
	      while ((n[2]!=1)||(lgef(n)!=3));
	    }
	  else
	    {
	      do
		{
		  av3=avma;
		  f=n;
		  if(all==1)
		    while (!((cmpii(mulii(p,p),f)>0)||pseudoprime(f))) 
		      {f=ellfacteur(f);nbx=1;}
		  av4=avma;
		  f=gerepile(av3,av4,gcopy(f));
		  nb++;
		  for (k=0;mpdivis(n,f,n);k++);
		  stoi(k);
		}
	      while ((n[2]!=1)||(lgef(n)!=3));
	    }
	}
      gerepile(av1,av2,0);
    }
  p=(GEN)avma;
  z=cgetg(3,19);
  p1=cgetg(nb+1,18);z[1]=(long)p1;
  p2=cgetg(nb+1,18);z[2]=(long)p2;
  for (j=nb;j;j--)
    {
      p2[j]=(long)p;
      p+=lg(p);
      p1[j]=(long)p;
      p+=lg(p);
    }
  if((DEBUGLEVEL>3)&&nbx) {fprintf(errfile,"\n");fflush(stdout);}
  return z;
}

GEN decomp(GEN n)
{
  return auxdecomp(n,1);
}

GEN smallfact(GEN n)
{
  GEN p1,p2,p3,p4,p5,y;
  long av,tetpil,tx,i,l;

  if((tx=typ(n))>=17)
    {
      l=lg(n);y=cgetg(l,tx);for(i=1;i<l;i++) y[i]=(long)smallfact((GEN)n[i]);
      return y;
    }
  switch(tx)
    {
    case 1:return auxdecomp(n,0);
    case 5:av=avma;n=gred(n);
    case 4:if(typ(n)==4) av=avma;p1=auxdecomp((GEN)n[1],0);
      p2=auxdecomp((GEN)n[2],0);p4=concat((GEN)p1[1],(GEN)p2[1]);
      p5=concat((GEN)p1[2],gneg((GEN)p2[2]));p3=indexsort(p4);
      tetpil=avma;y=cgetg(3,19);y[1]=(long)extract(p4,p3);
      y[2]=(long)extract(p5,p3);return gerepile(av,tetpil,y);
    default: err(arither1);return gnil;
    }
}

GEN boundfact(GEN n, long lim)
{
  GEN p1,p2,p3,p4,p5,y;
  long av,tetpil,tx,i,l;

  if(lim<=1) lim=0;
  if((tx=typ(n))>=17)
    {
      l=lg(n);y=cgetg(l,tx);for(i=1;i<l;i++) y[i]=(long)boundfact((GEN)n[i],lim);
      return y;
    }
  switch(tx)
    {
    case 1:return auxdecomp(n,lim);
    case 5:av=avma;n=gred(n);
    case 4:if(typ(n)==4) av=avma;p1=auxdecomp((GEN)n[1],lim);
      p2=auxdecomp((GEN)n[2],lim);p4=concat((GEN)p1[1],(GEN)p2[1]);
      p5=concat((GEN)p1[2],gneg((GEN)p2[2]));p3=indexsort(p4);
      tetpil=avma;y=cgetg(3,19);y[1]=(long)extract(p4,p3);
      y[2]=(long)extract(p5,p3);return gerepile(av,tetpil,y);
    default: err(arither1);return gnil;
    }
}

GEN divisors(GEN n)
{
  long tetpil,av=avma,i,j,l,start;
  GEN p,t=decomp(n),p1=(GEN)t[1],p2=(GEN)t[2],t1,t2,t3,nbdiv=gun;
  l=lg(p1);
  start=1+((l>1)&&(signe((GEN)p1[1])<0));
  for(i=start;i<l;i++)nbdiv=mulis(nbdiv,1+itos((GEN)p2[i]));
  p=t=cgetg(itos(nbdiv)+1,17);
  *++p=un;
  for(i=start;i<l;i++)
    for(t1=t,j=itos((GEN)p2[i]);j;j--)
      {
	t2=p;
	for(t3=t1;t3<t2;)
	  *++p=lmulii((GEN)(*++t3),(GEN)p1[i]);
	t1=t2;
      }
  tetpil=avma;
  return gerepile(av,tetpil,sort(t));
}


GEN gomega(GEN n)
{
  long l,tx,i;GEN y;

  if((tx=typ(n))>=17)
    {
      l=lg(n);y=cgetg(l,tx);for(i=1;i<l;i++) y[i]=(long)gomega((GEN)n[i]);
      return y;
    }
  if(tx!=1) err(arither1);
  return stoi(omega(n));
}

long omega(GEN n)
{
  byteptr d=diffptr;
  unsigned char c;
  GEN p,f;
  long nb=0,av=avma,av2;
  if (typ(n)!=1) err(arither1);
  if (!signe(n)) err(arither2);
  if ((n[2]==1)&&(lgef(n)==3)) return 0;
  n=absi(n);
  p=cgeti(3);p[1]=evalsigne(1)+evallgef(3);p[2]=0;
  av2=avma;
  while (c= *d++)
    {
      p[2]+=c;
      if (!mpdivis(n,p,n)) continue;
      nb++;
      while (mpdivis(n,p,n));
      if ((n[2]==1)&&(lgef(n)==3)) break;
    }
  while ((n[2]!=1) || (lgef(n)!=3))
    {
      f=gcopy(n);
      while (!((cmpii(mulii(p,p),f)>0)||pseudoprime(f))) f=ellfacteur(f);
      nb++;
      while (mpdivis(n,f,n));
      avma=av2;
    }
  avma=av;
  return nb;
}


GEN gbigomega(GEN n)
{
  long l,tx,i;GEN y;

  if((tx=typ(n))>=17)
    {
      l=lg(n);y=cgetg(l,tx);for(i=1;i<l;i++) y[i]=(long)gbigomega((GEN)n[i]);
      return y;
    }
  if(tx!=1) err(arither1);
  return stoi(bigomega(n));
}

long bigomega(GEN n)
{
  byteptr d=diffptr;
  unsigned char c;
  GEN p,f;
  long nb=0,av=avma,av2;
  if (typ(n)!=1) err(arither1);
  if (!signe(n)) err(arither2);
  if ((n[2]==1)&&(lgef(n)==3)) return 0;
  n=absi(n);
  p=cgeti(3);p[1]=evalsigne(1)+evallgef(3);p[2]=0;
  av2=avma;
  while (c= *d++)
    {
      p[2]+=c;
      if (!mpdivis(n,p,n)) continue;
      do nb++;while (mpdivis(n,p,n));
      if ((n[2]==1)&&(lgef(n)==3)) break;
    }
  while ((n[2]!=1) || (lgef(n)!=3))
    {
      f=gcopy(n);
      while (!((cmpii(mulii(p,p),f)>0) || pseudoprime(f))) f = ellfacteur(f);
      while (mpdivis(n,f,n)) nb++;
      avma=av2;
    }
  avma=av;
  return nb;
}

GEN numbdiv(GEN n)
{
  byteptr d=diffptr;
  unsigned char c;
  GEN p,f,m,m1,y;
  long l,av=avma,av2,av3,i,tx;

  if((tx=typ(n))>=17)
    {
      l=lg(n);y=cgetg(l,tx);for(i=1;i<l;i++) y[i]=(long)numbdiv((GEN)n[i]);
      return y;
    }
  if (tx!=1) err(arither1);
  if (!signe(n)) err(arither2);
  if ((n[2]==1)&&(lgef(n)==3)) return gun;
  n=absi(n);
  p=cgeti(3);p[1]=evalsigne(1)+evallgef(3);p[2]=0;
  av2=avma;
  m=stoi(1);
  while (c= *d++)
    {
      p[2]+=c;
      if (!mpdivis(n,p,n)) continue;
      for (l=2;mpdivis(n,p,n);l++);
      av3=avma;
      m1=mulsi(l,m);
      if (lgef (m1) ==lgef(m)) affii(m1,m);
      else m=gerepile(av2,av3,m1);
      if ((n[2]==1)&&(lgef(n)==3)) break;
    }
  while ((n[2]!=1) || (lgef(n)!=3))
    {
      f=gcopy(n);
      while (!((cmpii(mulii(p,p),f)>0) || pseudoprime(f))) f = ellfacteur(f);
      for (l=1;mpdivis(n,f,n);l++);
      av3=avma;
      m=gerepile(av2,av3,mulsi(l,m));
    }
  return gerepile(av,av2,m);
}

GEN sumdiv(GEN n)
{
  byteptr d=diffptr;
  unsigned char c;
  GEN p,f,m,m1,y;
  long av1=avma,av2,av3,i,tx,l;

  if((tx=typ(n))>=17)
    {
      l=lg(n);y=cgetg(l,tx);for(i=1;i<l;i++) y[i]=(long)sumdiv((GEN)n[i]);
      return y;
    }
  if (tx!=1) err(arither1);
  if (!signe(n)) err(arither2);
  if ((n[2]==1)&&(lgef(n)==3)) return gun;
  n=absi(n);
  p=cgeti(3);p[1]=evalsigne(1)+evallgef(3);p[2]=0;
  av2=avma;
  m=gun;
  while (c= *d++)
    {
      p[2]+=c;
      if (!mpdivis(n,p,n)) continue;
      m1=addsi(1,p);
      while (mpdivis(n,p,n)) m1=addsi(1,mulii(p,m1));
      av3=avma;m=gerepile(av2,av3,mulii(m1,m));
      if ((n[2]==1)&&(lgef(n)==3)) break;
    }
  while ((n[2]!=1)||(lgef(n)!=3))
    {
      f=gcopy(n);
      while (!((cmpii(mulii(p,p),f)>0)||pseudoprime(f))) f=ellfacteur(f);
      m1=gun;
      while (mpdivis(n,f,n)) m1=addsi(1,mulii(f,m1));
      av3=avma;m=gerepile(av2,av3,mulii(m1,m));
    }
  return gerepile(av1,av2,m);
}

GEN sumdivk(long k, GEN n)
{
  byteptr d=diffptr;
  unsigned char c;
  GEN p,p1,f,m,m1,pk,y;
  long av1=avma,av2,av3,k1,tx,i,l;

  if((tx=typ(n))>=17)
    {
      l=lg(n);y=cgetg(l,tx);for(i=1;i<l;i++) y[i]=(long)sumdivk(k,(GEN)n[i]);
      return y;
    }
  if (tx!=1) err(arither1);
  if (!signe(n)) err(arither2);
  if ((n[2]==1)&&(lgef(n)==3)) return gun;
  n=absi(n);k1=k;if(k<0) {k= -k;p1=gpuigs(n,k1);}
  p=cgeti(3);p[1]=evalsigne(1)+evallgef(3);p[2]=0;
  av2=avma;
  m=gun;
  while (c= *d++)
    {
      p[2]+=c;
      if (!mpdivis(n,p,n)) continue;
      pk=gpuigs(p,k);
      m1=addsi(1,pk);
      while (mpdivis(n,p,n)) m1=addsi(1,mulii(pk,m1));
      av3=avma;m=gerepile(av2,av3,mulii(m1,m));
      if ((n[2]==1)&&(lgef(n)==3)) break;
    }
  while ((n[2]!=1)||(lgef(n)!=3))
    {
      f=gcopy(n);
      while (!((cmpii(mulii(p,p),f)>0)||pseudoprime(f))) f=ellfacteur(f);
      pk=gpuigs(f,k);
      m1=gun;
      while (mpdivis(n,f,n)) m1=addsi(1,mulii(pk,m1));
      av3=avma;m=gerepile(av2,av3,mulii(m1,m));
    }
  av3=avma;return (k1>=0) ? gerepile(av1,av2,m) : gerepile(av1,av3,gmul(p1,m));
}

/* decomposition de nombres par la methode des courbes elliptiques.
   La seule fonction exportee est ellfacteur.
   Cette fonction renvoie un facteur non trivial de n.
   On suppose en entree que n n'est pas premier, et n'est divisible par
   aucun petit nombre premier (pas de facteur<65536,en tout cas.)        */

static GEN x1,y11,x2,y2,aux,w,n,global;
static long nombre,taillef;

static GEN cree(long nb)
{
  GEN z=cgetg(nb+1,17);
  long i;
  for(i=1;i<=nb;i++) z[i]=lgeti(taillef);
  return z;
}

static long pur(long x, long *p)
{
  byteptr d=diffptr;
  long m=0;
  do m+= *d++;while (x % m);
  do x /=m;while (!(x % m));
  *p=m;
  return x==1;
}

static GEN elladd(void)
{
  GEN v1,glob,lambda;
  long i,av=avma;
  for(i=1;i<=nombre;i++)
    {subiiz((GEN)x1[i],(GEN)x2[i],(GEN)aux[i]); if (signe((GEN)aux[i])<0) addiiz((GEN)aux[i],n,(GEN)aux[i]);}
  for(i=1;i<=nombre;i++)
    {modiiz(mulii((GEN)aux[i],(GEN)w[i]),n,(GEN)w[i+1]);avma=av;}
  if (!inversemodulo((GEN)w[nombre+1],n,&glob)) return glob;
  affii(glob,global);
  for(i=nombre;i;i--)
    {
      v1=modii(mulii(global,(GEN)w[i]),n);
      modiiz(mulii(global,(GEN)aux[i]),n,global);
      lambda=modii(mulii(subii((GEN)y11[i],(GEN)y2[i]),v1),n);
      modiiz(subii(mulii(lambda,lambda),addii((GEN)x2[i],(GEN)x1[i])),n,(GEN)x2[i]);
      modiiz(subii(mulii(lambda,subii((GEN)x1[i],(GEN)x2[i])),(GEN)y11[i]),n,(GEN)y2[i]);
      avma=av;
    }
  return (GEN)0;
}

static GEN elldouble(void)
{
  GEN v1,v2,glob,lambda;
  long i,av=avma;
  for(i=1;i<=nombre;i++) {modiiz(shifti((GEN)y2[i],1),n,(GEN)aux[i]);avma=av;}
  for(i=1;i<=nombre;i++) {modiiz(mulii((GEN)aux[i],(GEN)w[i]),n,(GEN)w[i+1]);avma=av;}
  if (!inversemodulo((GEN)w[nombre+1],n,&glob)) return glob;
  affii(glob,global);
  for(i=nombre;i;i--)
    {
      v1=modii(mulii(global,(GEN)w[i]),n);
      modiiz(mulii(global,(GEN)aux[i]),n,global);
      lambda=modii(mulii(addsi(1,mulsi(3,mulii((GEN)x2[i],(GEN)x2[i]))),v1),n);
      v2=modii(subii(mulii(lambda,lambda),shifti((GEN)x2[i],1)),n);
      modiiz(subii(mulii(lambda,subii((GEN)x2[i],v2)),(GEN)y2[i]),n,(GEN)y2[i]);
      affii(v2,(GEN)x2[i]);
      avma=av;
    }
  return (GEN)0;
}

static GEN ellmult(long k)
{
  GEN result = (GEN)0;
  if (k>1)
    {
      if (result = ellmult(k/2)) return result;
      if (result = elldouble()) return result;
      if (k&1) result = elladd();
    }
  return result;
}

GEN ellfacteur(GEN n1)
{
  static long i,k,m,av;
  static GEN glob;
  taillef=lgef(n1);
  nombre=5*(lgef(n1)-2);
  global=cgeti(taillef);
  av=avma;
  n=absi(n1);
  x1=cree(nombre);
  x2=cree(nombre);
  y11=cree(nombre);
  y2=cree(nombre);
  aux=cree(nombre);
  w=cree(nombre+1);
  w[1]=un;
  for(i=nombre;i;i--) {affsi(mymyrand(),(GEN)x2[i]);affsi(mymyrand(),(GEN)y2[i]);}
  for (m=2;;m++)
    if (pur(m,&k))
      {
        for(i=nombre;i;i--) {affii((GEN)x2[i],(GEN)x1[i]);affii((GEN)y2[i],(GEN)y11[i]);}
	if(DEBUGLEVEL>3) {fprintf(errfile,".");fflush(stdout);}
        if((glob = ellmult(k))&&(cmpii(glob,n)))
          {
            if (cmpii(mulii(glob,glob),n)>0) diviiz(n,glob,global);
            else affii(glob,global);
            avma=av;
            return global;
          }
      }
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~                                                                     ~*/
/*~                   COMPOSITION DES FORMES QUADRATIQUES               ~*/
/*~                                                                     ~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

GEN  qfi(GEN x, GEN y, GEN z)
{
  GEN t;
  
  if((typ(x)!=1)||(typ(y)!=1)||(typ(z)!=1)) err(qfer1);
  t=cgetg(4,16);
  t[1] = lcopy(x); t[2] = lcopy(y); t[3] = lcopy(z);
  return t;
}


GEN  qfr(GEN x, GEN y, GEN z, GEN d)
{
  GEN t;
  
  if((typ(x)!=1)||(typ(y)!=1)||(typ(z)!=1)||(typ(d)!=2)) err(qfer1);
  t=cgetg(5,15);
  t[1]=lcopy(x);t[2]=lcopy(y);t[3]=lcopy(z);t[4]=lcopy(d);
  return t;
}

GEN compimag(GEN x, GEN y)
{
  long av,tetpil;
  GEN s,n,d,d1,d2,x1,x2,y1,y2,v1,v2,v3,b3,c3,m,z,p1,r;
  
  if(cmpii((GEN)x[1],(GEN)y[1])>0) {s=x;x=y;y=s;}
  av=avma;s=shifti(addii((GEN)x[2],(GEN)y[2]),-1);n=subii((GEN)y[2],s);
  d=bezout((GEN)y[1],(GEN)x[1],&y1,&x1);d1=bezout(s,d,&x2,&y2);
  v1=divii((GEN)x[1],d1);v2=divii((GEN)y[1],d1);
  d2=(gcmp1(d1))?gun:mppgcd(d1,mppgcd((GEN)x[3],mppgcd((GEN)y[3],n)));
  v3=mulii(d2,v1);
  m=addii(mulii(mulii(y1,y2),n),mulii((GEN)y[3],x2));setsigne(m,-signe(m));
  r=modii(m,v3);b3=shifti((p1=mulii(v2,r)),1);
  c3=addii(mulii((GEN)y[3],d1),mulii(r,addii((GEN)y[2],p1)));
  z=cgetg(4,16);z[1]=lmulii(v3,v2);z[2]=laddii((GEN)y[2],b3);z[3]=ldivii(c3,v3);
  tetpil=avma;return gerepile(av,tetpil,redimag(z));
}

GEN compreal(GEN x, GEN y)
{
  long av,tetpil;
  GEN s,n,d,d1,d2,x1,x2,y1,y2,v1,v2,v3,b3,c3,m,z,p1,r;
  
  av=avma;s=shifti(addii((GEN)x[2],(GEN)y[2]),-1);n=subii((GEN)y[2],s);
  d=bezout((GEN)y[1],(GEN)x[1],&y1,&x1);d1=bezout(s,d,&x2,&y2);
  v1=divii((GEN)x[1],d1);v2=divii((GEN)y[1],d1);
  d2=(gcmp1(d1))?gun:mppgcd(d1,mppgcd((GEN)x[3],mppgcd((GEN)y[3],n)));
  v3=mulii(d2,v1);
  m=addii(mulii(mulii(y1,y2),n),mulii((GEN)y[3],x2));setsigne(m,-signe(m));
  r=modii(m,v3);b3=shifti((p1=mulii(v2,r)),1);
  c3=addii(mulii((GEN)y[3],d1),mulii(r,addii((GEN)y[2],p1)));
  z=cgetg(5,15);z[1]=lmulii(v3,v2);z[2]=laddii((GEN)y[2],b3);z[3]=ldivii(c3,v3);
  z[4]=laddrr((GEN)x[4],(GEN)y[4]);tetpil=avma;return gerepile(av,tetpil,redreal(z));
}

GEN comprealraw(GEN x, GEN y)
{
  long av,tetpil;
  GEN s,n,d,d1,d2,x1,x2,y1,y2,v1,v2,v3,b3,c3,m,z,p1,r;
  
  av=avma;s=shifti(addii((GEN)x[2],(GEN)y[2]),-1);n=subii((GEN)y[2],s);
  d=bezout((GEN)y[1],(GEN)x[1],&y1,&x1);d1=bezout(s,d,&x2,&y2);
  v1=divii((GEN)x[1],d1);v2=divii((GEN)y[1],d1);
  d2=(gcmp1(d1))?gun:mppgcd(d1,mppgcd((GEN)x[3],mppgcd((GEN)y[3],n)));
  v3=mulii(d2,v1);
  m=addii(mulii(mulii(y1,y2),n),mulii((GEN)y[3],x2));setsigne(m,-signe(m));
  r=modii(m,v3);b3=shifti((p1=mulii(v2,r)),1);
  c3=addii(mulii((GEN)y[3],d1),mulii(r,addii((GEN)y[2],p1)));
  z=cgetg(5,15);z[1]=lmulii(v3,v2);z[2]=laddii((GEN)y[2],b3);z[3]=ldivii(c3,v3);
  z[4]=laddrr((GEN)x[4],(GEN)y[4]);tetpil=avma;return gerepile(av,tetpil,gcopy(z));
}

GEN nucomp(GEN x, GEN y, GEN l)
               
     
     /* l=floor((|d|/4)^(1/4)) */
     
{
  long av=avma,tetpil,cz;
  GEN s,n,d,d1,v,u1,u,v1,v2,z,p1,p2,p3;
  GEN a,b,e,f,g,a1,a2,a3,b3,v3,q,t2,t3,c3,q1,q2,q3,q4;
  
  if((typ(x)!=16)||(typ(y)!=16)) err(nucomper1);
  if(x==y) return nudupl(x,l);
  if(cmpii((GEN)x[1],(GEN)y[1])<0) {s=x;x=y;y=s;}
  s=shifti(addii((GEN)x[2],(GEN)y[2]),-1);n=subii((GEN)y[2],s);
  a1=(GEN)x[1];a2=(GEN)y[1];d=bezout(a2,a1,&u,&v);
  if(gcmp1(d)) {a=negi(gmul(u,n));d1=d;}
  else
    if(divise(s,d)) 
      {
	a=negi(gmul(u,n));d1=d;a1=divii(a1,d1);a2=divii(a2,d1);s=divii(s,d1);
      }
    else
      {
	d1=bezout(s,d,&u1,&v1);
	if(!gcmp1(d1)) 
	  {
	    a1=divii(a1,d1);a2=divii(a2,d1);s=divii(s,d1);d=divii(d,d1);
	  }
	p1=resii((GEN)x[3],d);p2=resii((GEN)y[3],d);
	p3=modii(negi(mulii(u1,addii(mulii(u,p1),mulii(v,p2)))),d);
	a=subii(mulii(p3,divii(a1,d)),mulii(u,divii(n,d)));
      }
  a=modii(a,a1);p1=subii(a1,a);if(cmpii(a,p1)>0) a=negi(p1);
  v=gzero;d=a1;v2=gun;v3=a;cz=0;
  while(cmpii(v3,l)>=0)
    {
      q=dvmdii(d,v3,&t3);t2=subii(v,mulii(q,v2));
      v=v2;d=v3;v2=t2;v3=t3;cz++;
    }
  if(!cz)
    {
      q1=mulii(a2,v3);q2=addii(q1,n);f=divii(q2,d);
      g=divii(addii(mulii(v3,s),(GEN)y[3]),d);
      a3=mulii(d,a2);c3=addii(mulii(v3,f),mulii(g,d1));
      b3=addii(shifti(q1,1),(GEN)y[2]);
    }
  else
    {
      if(cz&1) {v3=negi(v3);v2=negi(v2);}
      b=divii(addii(mulii(a2,d),mulii(n,v)),a1);q1=mulii(b,v3);
      q2=addii(q1,n);f=divii(q2,d);
      e=divii(addii(mulii(s,d),mulii((GEN)y[3],v)),a1);
      q3=mulii(e,v2);q4=subii(q3,s);g=divii(q4,v);
      if(!gcmp1(d1)) {v2=mulii(d1,v2);v=mulii(d1,v);}
      a3=addii(mulii(d,b),mulii(e,v));c3=addii(mulii(v3,f),mulii(g,v2));
      b3=addii(addii(q1,q2),mulii(d1,addii(q3,q4)));
    }
  z=cgetg(4,16);z[1]=(long)a3;z[2]=(long)b3;z[3]=(long)c3;  
  tetpil=avma;return gerepile(av,tetpil,redimag(z));
}

GEN nudupl(GEN x, GEN l)
             
     
     /* l=floor((|d|/4)^(1/4)) */

{
  long av=avma,tetpil,cz;
  GEN u,v,d,d1,p1,a,b,c,b2,z,v2,v3,q,t2,t3,e,g;

  d1=bezout((GEN)x[2],(GEN)x[1],&u,&v);a=divii((GEN)x[1],d1);b=divii((GEN)x[2],d1);
  c=modii(negi(mulii(u,(GEN)x[3])),a);p1=subii(a,c);if(cmpii(c,p1)>0) c=negi(p1);
  v=gzero;d=a;v2=gun;v3=c;cz=0;
  while(cmpii(v3,l)>=0)
    {
      q=dvmdii(d,v3,&t3);t2=subii(v,mulii(q,v2));
      v=v2;d=v3;v2=t2;v3=t3;cz++;
    }
  if(!cz)
    {
      g=divii(addii(mulii(b,v3),(GEN)x[3]),d);
      z=cgetg(4,16);z[1]=lmulii(d,d);
      z[2]=laddii((GEN)x[2],shifti(mulii(d,v3),1));
      z[3]=laddii(mulii(v3,v3),mulii(g,d1));
    }
  else
    {
      if(cz&1) {v=negi(v);d=negi(d);}
      e=divii(addii(mulii((GEN)x[3],v),mulii(b,d)),a);
      g=divii(subii(mulii(e,v2),b),v);b2=addii(mulii(e,v2),mulii(v,g));
      if(!gcmp1(d1)) {b2=mulii(d1,b2);v=mulii(d1,v);v2=mulii(d1,v2);}
      z=cgetg(4,16);z[1]=laddii(mulii(d,d),mulii(e,v));
      z[2]=laddii(b2,shifti(mulii(d,v3),1));
      z[3]=laddii(mulii(v3,v3),mulii(g,v2));
    }
  tetpil=avma;return gerepile(av,tetpil,redimag(z));
}

GEN sqcomp(GEN x)
{
  long av,tetpil;
  GEN d1,d2,x2,y2,v1,v3,b3,c3,m,z,p1,r;
  
  av=avma;
  d1=bezout((GEN)x[2],(GEN)x[1],&x2,&y2);v1=divii((GEN)x[1],d1);
  d2=(gcmp1(d1))?gun:mppgcd(d1,(GEN)x[3]);v3=mulii(d2,v1);
  m=mulii((GEN)x[3],x2);setsigne(m,-signe(m));
  r=modii(m,v3);b3=shifti((p1=mulii(v1,r)),1);
  c3=addii(mulii((GEN)x[3],d1),mulii(r,addii((GEN)x[2],p1)));
  z=cgetg(4,16);z[1]=lmulii(v3,v1);
  z[2]=laddii((GEN)x[2],b3);z[3]=ldivii(c3,v3);
  tetpil=avma;return gerepile(av,tetpil,redimag(z));
}

GEN sqcompreal(GEN x)
{
  long av,tetpil;
  GEN d1,d2,x2,y2,v1,v3,b3,c3,m,z,p1,r;
  
  av=avma;
  d1=bezout((GEN)x[2],(GEN)x[1],&x2,&y2);v1=divii((GEN)x[1],d1);
  d2=(gcmp1(d1))?gun:mppgcd(d1,(GEN)x[3]);v3=mulii(d2,v1);
  m=mulii((GEN)x[3],x2);setsigne(m,-signe(m));
  r=modii(m,v3);b3=shifti((p1=mulii(v1,r)),1);
  c3=addii(mulii((GEN)x[3],d1),mulii(r,addii((GEN)x[2],p1)));
  z=cgetg(5,15);z[1]=lmulii(v3,v1);z[2]=laddii((GEN)x[2],b3);z[3]=ldivii(c3,v3);
  z[4]=lshiftr((GEN)x[4],1);tetpil=avma;return gerepile(av,tetpil,redreal(z));
}

GEN sqcomprealraw(GEN x)
{
  long av,tetpil;
  GEN d1,d2,x2,y2,v1,v3,b3,c3,m,z,p1,r;
  
  av=avma;
  d1=bezout((GEN)x[2],(GEN)x[1],&x2,&y2);v1=divii((GEN)x[1],d1);
  d2=(gcmp1(d1))?gun:mppgcd(d1,(GEN)x[3]);v3=mulii(d2,v1);
  m=mulii((GEN)x[3],x2);setsigne(m,-signe(m));
  r=modii(m,v3);b3=shifti((p1=mulii(v1,r)),1);
  c3=addii(mulii((GEN)x[3],d1),mulii(r,addii((GEN)x[2],p1)));
  z=cgetg(5,15);z[1]=lmulii(v3,v1);z[2]=laddii((GEN)x[2],b3);z[3]=ldivii(c3,v3);
  z[4]=lshiftr((GEN)x[4],1);tetpil=avma;return gerepile(av,tetpil,gcopy(z));
}

GEN powrealraw(GEN x, long n, long prec)
{
  long av,tetpil;
  GEN p1,p2,y,z;

  if(n<0) err(impl,"powrealraw for negative exponents");
  if(n==1) return gcopy(x);
  z=x;av=avma;y=cgetg(5,15);y[1]=un;
  p1=subii(mulii((GEN)x[2],(GEN)x[2]),shifti(mulii((GEN)x[1],(GEN)x[3]),2));
  p2=racine(p1);if(mpodd(subii(p2,(GEN)x[2]))) p2=addsi(-1,p2);
  y[2]=(long)p2;y[3]=lshifti(subii(mulii(p2,p2),p1),-2);
  p1=cgetr(prec);y[4]=(long)p1;p1[1]=HIGHEXPOBIT-((prec-2)<<TWOPOTBITS_IN_LONG);
  p1[2]=0;tetpil=avma;y=gerepile(av,tetpil,gcopy(y));
  if(!n) return y;
  else
    {
      for(;n>1;n>>=1)
      {
        if (n&1) y=comprealraw(y,z);
        z=sqcomprealraw(z);
      }
      tetpil=avma;return gerepile(av,tetpil,comprealraw(y,z));
    }
}

GEN nupow(GEN x, GEN n)
{
  long av,tetpil,i,j;
  unsigned long m;
  GEN p1,p2,y,z,l;

  if(typ(n)!=1) err(nupower1);
  if(gcmp1(n)) return gcopy(x);
  z=x;av=avma;y=cgetg(4,16);y[1]=un;y[2]=mpodd((GEN)x[2]) ? un : zero;
  p1=mulii((GEN)x[1],(GEN)x[3]);p2=shifti(mulii((GEN)x[2],(GEN)x[2]),-2);y[3]=lsubii(p1,p2);
  if(!signe(n)) {tetpil=avma;return gerepile(av,tetpil,gcopy(y));}
  else
    {
      l=racine(shifti(racine((GEN)y[3]),1));
      for (i=lgef(n)-1;i>2;i--)
      {
        for (m=n[i],j=0;j<BITS_IN_LONG;j++,m>>=1)
        {
          if (m&1) y=nucomp(y,z,l);
          z=nudupl(z,l);
        }
      }
      for (m=n[2];m>1;m>>=1)
      {
        if (m&1) y=nucomp(y,z,l);
        z=nudupl(z,l);
      }
      tetpil=avma;y=nucomp(y,z,l);
      if ((signe(n)<0)&&cmpii((GEN)y[1],(GEN)y[2])&&cmpii((GEN)y[1],(GEN)y[3]))
	setsigne((GEN)y[2],-signe((GEN)y[2]));
      return gerepile(av,tetpil,y);
    }
}

GEN redimag(GEN x)
{
  long av,tetpil,s,fl,fg;
  GEN b1,c1,p1,a,b,c,d,z;
  
  av=avma;a=(GEN)x[1];b=(GEN)x[2];c=(GEN)x[3];
  fl=cmpii(a,c);s=signe(b);setsigne(b,labs(s));fg=cmpii(a,b);
  while((fl>0)||(fg<0))
    {
      p1=shifti(c,1);d=dvmdii(b,p1,&b1);
      setsigne(b,s);
      if(s>=0)
        {
          if(cmpii(b1,c)<=0) {setsigne(d,-signe(d));setsigne(b1,-signe(b1));}
          else {d=subsi(-1,d);b1=subii(p1,b1);}
        }
      else
        {
          if(cmpii(b1,c)>=0) {d=addsi(1,d);b1=subii(b1,p1);}
        }
      c1=addii(a,mulii(d,shifti(subii(b,b1),-1)));a=c;
      b=b1;c=c1;
      fl=cmpii(a,c);s=signe(b);setsigne(b,labs(s));fg=cmpii(a,b);
    }
  if(fl&&fg&&(s<0)) setsigne(b,s);tetpil=avma;
  z=cgetg(4,16);z[1]=lcopy(a);z[2]=lcopy(b);z[3]=lcopy(c);
  return gerepile(av,tetpil,z);
}

GEN rhoreal(GEN x)
{
  long av,tetpil,s,l;
  GEN y,p1,nn,sqrtD,isqrtD,D;
  
  if(typ(x)!=15) err(rhoer1);
  av=avma;y=cgetg(5,15);
  l=max(lg((GEN)x[4]),((BITS_IN_LONG-1-expo((GEN)x[4]))>>TWOPOTBITS_IN_LONG)+2);if(l<=2) l=3;
  y[1]=lcopy((GEN)x[3]);sqrtD=gsqrt(D=subii(mulii((GEN)x[2],(GEN)x[2]),shifti(mulii((GEN)x[1],(GEN)x[3]),2)),l);
  isqrtD=mptrunc(sqrtD);
  s=signe((GEN)y[1]);setsigne((GEN)y[1],1);
  if(cmpii(isqrtD,(GEN)y[1])>=0) nn=divii(addii(isqrtD,(GEN)x[2]),p1=shifti((GEN)y[1],1));
  else nn=divii(addii((GEN)y[1],(GEN)x[2]),p1=shifti((GEN)y[1],1));
  p1=mulii(nn,p1);y[2]=lsubii(p1,(GEN)x[2]);
  setsigne((GEN)y[1],s);p1=shifti(subii(mulii((GEN)y[2],(GEN)y[2]),D),-2);y[3]=ldivii(p1,(GEN)y[1]);
  y[4]=laddrr(shiftr(mplog(absr(divrr(addir((GEN)x[2],sqrtD),subir((GEN)x[2],sqrtD)))),-1),(GEN)x[4]);
  tetpil=avma;return gerepile(av,tetpil,gcopy(y));
}

GEN redreal(GEN x)
{
  long fl,av=avma,tetpil,l,s;
  GEN y,p1,sqrtD,isqrtD,D,z,nn;
  
  if(typ(x)!=15) err(rhoer1);
  l=max(lg((GEN)x[4]),((BITS_IN_LONG-1-expo((GEN)x[4]))>>TWOPOTBITS_IN_LONG)+2);if(l<=2) l=3;
  sqrtD=gsqrt(D=subii(mulii((GEN)x[2],(GEN)x[2]),shifti(mulii((GEN)x[1],(GEN)x[3]),2)),l);
  isqrtD=mptrunc(sqrtD);
  y=cgetg(5,15);y[1]=x[1];y[2]=x[2];y[3]=x[3];y[4]=x[4];
  if((signe((GEN)x[2])<=0)||(cmpii((GEN)x[2],isqrtD)>0)) fl=1;
  else
    {
      p1=subii(isqrtD,shifti(absi((GEN)x[1]),1));
      if(signe(p1)<0) fl=(cmpii((GEN)x[2],absi(p1))<0);else fl=(cmpii((GEN)x[2],p1)<=0);
    }
  while(fl)
    {
      z=cgetg(5,15);
      z[1]=y[3];s=signe((GEN)z[1]);setsigne((GEN)z[1],1);
      if(cmpii(isqrtD,(GEN)z[1])>=0) nn=divii(addii(isqrtD,(GEN)y[2]),p1=shifti((GEN)z[1],1));
      else nn=divii(addii((GEN)z[1],(GEN)y[2]),p1=shifti((GEN)z[1],1));
      p1=mulii(nn,p1);z[2]=lsubii(p1,(GEN)y[2]);
      setsigne((GEN)z[1],s);p1=shifti(subii(mulii((GEN)z[2],(GEN)z[2]),D),-2);z[3]=ldivii(p1,(GEN)z[1]);
      z[4]=laddrr(shiftr(mplog(absr(divrr(addir((GEN)y[2],sqrtD),subir((GEN)y[2],sqrtD)))),-1),(GEN)y[4]);
      y=z;
      if((signe((GEN)y[2])<=0)||(cmpii((GEN)y[2],isqrtD)>0)) fl=1;
      else
	{
	  p1=subii(isqrtD,shifti(absi((GEN)y[1]),1));
	  if(signe(p1)<0) fl=(cmpii((GEN)y[2],absi(p1))<0);else fl=(cmpii((GEN)y[2],p1)<=0);
	}
    }
  tetpil=avma;return gerepile(av,tetpil,gcopy(y));
}

GEN rhorealnod(GEN x, GEN isqrtD)
{
  long av,tetpil,s;
  GEN y,p1,nn,D;
  
  if(typ(x)!=15) err(rhoer1);
  if(typ(isqrtD)!=1) err(arither1);
  av=avma;y=cgetg(5,15);
  y[1]=lcopy((GEN)x[3]);D=subii(mulii((GEN)x[2],(GEN)x[2]),shifti(mulii((GEN)x[1],(GEN)x[3]),2));
  s=signe((GEN)y[1]);setsigne((GEN)y[1],1);
  if(cmpii(isqrtD,(GEN)y[1])>=0) nn=divii(addii(isqrtD,(GEN)x[2]),p1=shifti((GEN)y[1],1));
  else nn=divii(addii((GEN)y[1],(GEN)x[2]),p1=shifti((GEN)y[1],1));
  p1=mulii(nn,p1);y[2]=lsubii(p1,(GEN)x[2]);
  setsigne((GEN)y[1],s);p1=shifti(subii(mulii((GEN)y[2],(GEN)y[2]),D),-2);y[3]=ldivii(p1,(GEN)y[1]);
  y[4]=x[4];
  tetpil=avma;return gerepile(av,tetpil,gcopy(y));
}

GEN redrealnod(GEN x, GEN isqrtD)
{
  long fl,av=avma,tetpil,s;
  GEN y,p1,D,z,nn;
  
  if(typ(x)!=15) err(rhoer1);
  if(typ(isqrtD)!=1) err(arither1);
  D=subii(mulii((GEN)x[2],(GEN)x[2]),shifti(mulii((GEN)x[1],(GEN)x[3]),2));
  y=cgetg(5,15);y[1]=x[1];y[2]=x[2];y[3]=x[3];
  if((signe((GEN)x[2])<=0)||(cmpii((GEN)x[2],isqrtD)>0)) fl=1;
  else
    {
      p1=subii(isqrtD,shifti(absi((GEN)x[1]),1));
      if(signe(p1)<0) fl=(cmpii((GEN)x[2],absi(p1))<0);else fl=(cmpii((GEN)x[2],p1)<=0);
    }
  while(fl)
    {
      z=cgetg(5,15);
      z[1]=y[3];s=signe((GEN)z[1]);setsigne((GEN)z[1],1);
      if(cmpii(isqrtD,(GEN)z[1])>=0) nn=divii(addii(isqrtD,(GEN)y[2]),p1=shifti((GEN)z[1],1));
      else nn=divii(addii((GEN)z[1],(GEN)y[2]),p1=shifti((GEN)z[1],1));
      p1=mulii(nn,p1);z[2]=lsubii(p1,(GEN)y[2]);
      setsigne((GEN)z[1],s);p1=shifti(subii(mulii((GEN)z[2],(GEN)z[2]),D),-2);z[3]=ldivii(p1,(GEN)z[1]);
      y=z;
      if((signe((GEN)y[2])<=0)||(cmpii((GEN)y[2],isqrtD)>0)) fl=1;
      else
	{
	  p1=subii(isqrtD,shifti(absi((GEN)y[1]),1));
	  if(signe(p1)<0) fl=(cmpii((GEN)y[2],absi(p1))<0);else fl=(cmpii((GEN)y[2],p1)<=0);
	}
    }
  y[4]=x[4];
  tetpil=avma;return gerepile(av,tetpil,gcopy(y));
}
#ifdef __cplusplus
GEN primeform(GEN x, GEN p, long prec = 0)
#else
GEN primeform(GEN x, GEN p, ...)
#endif
{
  long av,tetpil,s,t,sb,sx=signe(x);
  GEN y,b,c;
  
  if((typ(x)!=1)||(!sx)) err(arither1);
  if(gcmpgs(p,2))
    {
      y=(sx<0) ? cgetg(4,16) : cgetg(5,15);y[1]=lcopy(p);y[2]=lgeti(lgef(p));
      av=avma;
      if(!mpsqrtmod(x,p,&b)) err(sqrter5);
      s=b[lgef(b)-1]&1;t=x[lgef(x)-1]&1;
      if(odd(s+t)) subiiz(p,b,(GEN)y[2]);else affii(b,(GEN)y[2]);
      c=shifti(subii(mulii((GEN)y[2],(GEN)y[2]),x),-2);tetpil=avma;
      y[3]=lpile(av,tetpil,divii(c,p));
    }
  else
    {
      s=x[lgef(x)-1]&7;if(signe(x)<0) s=8-s;
      switch(s)
        {
        case 0:
        case 8: sb=0;break;
        case 1: sb=1;break;
        case 4: sb=2;break;
        default: err(sqrter5);
        }
      y=(sx<0) ? cgetg(4,16) : cgetg(5,15);y[1]=lcopy(gdeux);y[2]=lstoi(sb);
      av=avma;c=gsubsg(sb*sb,x);tetpil=avma;
      y[3]=lpile(av,tetpil,shifti(c,-3));
    }
  if(sx>0) affsr(0,(GEN)(y[4]=lgetr(prec)));
  return y;
}

GEN classno(GEN x)
           
     
     /* calcul de h(x) pour x<0 par la methode de Shanks */
     
{
  static long prem,ptforme;
  long av,av1,av2,av3,tetpil,k,k2,i,j,j1,lim,limite,succes,com,j2,s,tx;
  GEN tabla, tablb, count, index, hash;
  static long court[3]={evaltyp(1)+evalpere(1)+evallg(3),evalsigne(1)+evallgef(3),0};
  GEN p1,p2,p3,hin,q,formes[100],l,h,hp,f,fh,fg,ftest,pm2;
  static byteptr p;
  
  if((tx=typ(x))>=17) 
    {
      k=lg(x);p1=cgetg(k,tx);for(i=1;i<k;i++) p1[i]=(long)classno((GEN)x[i]);
      return p1;
    }
  if(tx!=1) err(arither1);
  if (!(s=signe(x))) err(arither2);
  if(s>0) return classno2(x);
  k=x[lgef(x)-1]&3;
  if((k==1)||(k==2)) err(classer2);
  if(gcmpgs(x,-12)>=0) return gun;
  tabla = newbloc(10000);
  tablb = newbloc(10000);
  count = newbloc(256);
  index = newbloc(257);
  hash = newbloc(10000);
  prem=ptforme=0;p=diffptr;av=avma;limite=(av+bot)>>1;
  p1=divrr(gsqrt(absi(x),4),mppi(4));
  l=gtrunc(shiftr(gsqrt(gsqrt(absi(x),4),4),1));
  if(gcmpgs(l,1000)<0) affsi(1000,l);
  while((gcmpgs(l,prem)>=0)&&(*p))
    {
      prem+= *p++;
      k=krogs(x,prem); 
      if(k)
	{
	  av2=avma;
	  if(k>0)
	    {
	      divrsz(mulsr(prem,p1),prem-1,p1);avma=av2;
	      if((ptforme<100)&&(prem>2))
		{
		  court[2]=prem;
		  formes[ptforme++]=redimag(primeform(x,court));
		}
	    }
	  else {divrsz(mulsr(prem,p1),prem+1,p1);avma=av2;}
	}
    }
  hin=ground(p1);h=gcopy(hin);f=formes[0];fh=gpui(f,h);
  s=2*itos(gceil(gpui(p1,gdivgs(gun,4),4)));
  if(s>=10000) s=10000;
  p1=fh;av2=avma;
  for(i=0;i<=255;i++) count[i]=0;
  for(i=0;i<=s-1;i++)
    {
      p2=(GEN)p1[1];tabla[i]=p2[lgef(p2)-1];j=tabla[i]&255;count[j]++;
      p2=(GEN)p1[2];tablb[i]=p2[lgef(p2)-1];
      p1=compimag(p1,f);
    }
  fg=gpuigs(f,s);ftest=gpuigs(p1,0);
  index[0]=0;for(i=0;i<=254;i++) index[i+1]=index[i]+count[i];
  for(i=0;i<=s-1;i++) hash[index[tabla[i]&255]++]=i;
  index[0]=0;for(i=0;i<=255;i++) index[i+1]=index[i]+count[i];
  succes=0;com=0;av3=avma;
  while(!succes)
    {
      p1=(GEN)ftest[1];k=p1[lgef(p1)-1];j=k&255;
      pm2=negi((GEN)ftest[2]);
      for(j1=index[j];(j1<index[j+1])&&(!succes);j1++)
	{
	  j2=hash[j1];
	  if(tabla[j2]==k)
	    {
	      p2=(GEN)ftest[2];k2=p2[lgef(p2)-1];
	      if((tablb[j2]==k2)||(tablb[j2]== -k2))
		{
		  p1=gmul(gpuigs(f,j2),fh);
		  succes=(!cmpii((GEN)p1[1],(GEN)ftest[1]))&&((!cmpii((GEN)p1[2],(GEN)ftest[2]))||(!cmpii((GEN)p1[2],pm2)));
		}
	    }
	}
      if(!succes)
	{
	  com++;
	  if(avma>=limite) ftest=gmul(ftest,fg);
	  else {tetpil=avma;ftest=gerepile(av3,tetpil,gmul(ftest,fg));}
	  if (gcmp1((GEN)ftest[1]))
	    {
	      err(impl, "classno with too small order");
	    }
	}
    }
  h=addis(h,j2);p2=mulsi(s,stoi(com));tetpil=avma;
  h=(!cmpii((GEN)p1[2],(GEN)ftest[2]))?subii(h,p2):addii(h,p2);
  p2=factor(h);
  p1=(GEN)p2[1];
  p2=(GEN)p2[2];
  for(i=1;i<lg(p1);i++)
    {
      p3=divii(h,(GEN)p1[i]);fh=gpui(f,p3);lim=itos((GEN)p2[i]);
      for(j=1;(j<=lim)&&(!cmpii((GEN)fh[1],gun));j++)
	{
	  h=p3;
	  if(j<lim) {p3=divii(h,(GEN)p1[i]);fh=gpui(f,p3);}
	}
    }
  q=ground(gdiv(hin,h));tetpil=avma;
  hp=mulii(q,h);av1=avma;
  for(i=1;(i<=10)&&(i<ptforme);i++)
    {
      f=formes[i];com=0;
      fg=gpui(f,h);
      fh=gpui(fg,q);
      ftest=fg;
      if(cmpis((GEN)fh[1],1))
	{
	  for(;;)
	    {
	      com++;
	      if ((!cmpii((GEN)fh[1],(GEN)ftest[1]))&&((!cmpii((GEN)fh[2],(GEN)ftest[2]))||(!cmpii((GEN)fh[2],negi((GEN)ftest[2]))))) break;
	      ftest=gmul(ftest,fg);
	    }
	  if(!cmpii((GEN)fh[2],(GEN)ftest[2])) com= -com;
	  p2=mulsi(com,h);q=addsi(com,q);tetpil=avma;
	  hp=addii(hp,p2);av1=avma;
	}
    }
  avma=av1;
  killbloc(tabla); killbloc(tablb); killbloc(count);
  killbloc(index); killbloc(hash);
  return gerepile(av,tetpil,hp);
}
