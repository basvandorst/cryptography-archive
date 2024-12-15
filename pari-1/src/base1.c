/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*                                                                 */
/*                        CORPS DE NOMBRES                         */
/*                                                                 */
/*                                                                 */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

# include "genpari.h"
GEN differente(GEN nf);

#define coef1(a,i,j)      (*((long*)(*(a+(j)+1))+(i)+1))
#define gcoef1(a,i,j)     (GEN)coef1(a,i,j)

GEN rquot(GEN x, GEN y),ordmax(GEN f, GEN p, GEN e, GEN *ptdelta),rtran(GEN v, GEN w, GEN q),mtran(GEN v, GEN w, GEN q, GEN m),matinv(GEN x, GEN d);

void rowred(GEN a, long rlim, GEN rmod);

/*******************************************************************
                             ROUND 2

  Entree:     x polynome unitaire a coefficients dans Z de deg n
	    definissant un corps de nombres K=Q(theta);
              code 0, 1 ou (long)p selon que l'on veut base, smallbase
            ou factoredbase.
	      y pointeur sur un GEN destine a recevoir
	    le discriminant du corps K.
  Sortie:    retourne 1) un vecteur (horizontal) a n composantes, 
            de polynomes a coeff dans Q (de deg 0,1...n-1)
	    constituant une base de l'anneau des entiers de K.
	        2) le discriminant de K (dans *y).
	    Rem: le denominateur commun des coef. est dans da.
*******************************************************************/

GEN allbase(GEN x, long code, GEN *y)
{
  GEN p,a,at,bt,b,da,db,q;
  long av=avma,tetpil,n,h,j,je,k,r,s,t,pro,v;

  if(typ(x)!=10) err(allbaser1);
  n=lgef(x)-3;if(n<=0) err(allbaser1);
  v=varn(x);*y=discsr(x);
  switch(code)
    {
    case 0: p=auxdecomp(absi(*y),1);h=lg((GEN)p[1])-1;break; /* base */
    case 1: p=auxdecomp(absi(*y),0);h=lg((GEN)p[1])-1;break; /* smallbase */
    default: p=(GEN)code;
      if((typ(p)!=19)||(lg(p)!=3)) err(factoreder1); /* factoredbase */
      h=lg((GEN)p[1])-1;
      q=gun;for(je=1;je<=h;je++) q=gmul(q,gpui(gcoeff(p,je,1),gcoeff(p,je,2)));
      if(gcmp(absi(q),absi(*y))) err(factoreder2);
    }
  a=idmat(n);da=gun;
  for(je=1;je<=h;je++)
    {
      if(gcmpgs(gcoeff(p,je,2),1)>0)
	{
	  b=ordmax(x,gcoeff(p,je,1),gcoeff(p,je,2),&db);
	  a=gmul(db,a);b=gmul(da,b);
	  da=mulii(db,da);db=da;
	  at=gtrans(a);bt=gtrans(b);
	  for(r=n-1;r>=0;r--)
	    for(s=r;s>=0;s--)
	      while(signe(gcoef1(bt,s,r)))
		{
		  q=rquot(gcoef1(at,s,s),gcoef1(bt,s,r));
		  at[s+1]=(long)rtran((GEN)at[s+1],(GEN)bt[r+1],q);
		  for(t=s-1;t>=0;t--)
		    {
		      q=rquot(gcoef1(at,t,s),gcoef1(at,t,t));
		      at[s+1]=(long)rtran((GEN)at[s+1],(GEN)at[t+1],q);
		    }
		  pro=at[s+1];at[s+1]=bt[r+1];bt[r+1]=pro;
		}
	  for(j=n-1;j>=0;j--)
	    {
	      for(k=0;k<j;k++)
		{
		  while(signe(gcoef1(at,j,k)))
		    {
		      q=rquot(gcoef1(at,j,j),gcoef1(at,j,k));
		      at[j+1]=(long)rtran((GEN)at[j+1],(GEN)at[k+1],q);
		      pro=at[j+1];at[j+1]=at[k+1];at[k+1]=pro;
		    }
		}
	      if(signe(gcoef1(at,j,j))<0)
		for(k=0;k<=j;k++) coef1(at,k,j)=lnegi(gcoef1(at,k,j));
	      for(k=j+1;k<n;k++)
		{
		  q=rquot(gcoef1(at,j,k),gcoef1(at,j,j));
		  at[k+1]=(long)rtran((GEN)at[k+1],(GEN)at[j+1],q);
		}
	    }
	  for(j=1;j<n;j++)
	    if(!cmpii(gcoef1(at,j,j),gcoef1(at,j-1,j-1)))
	      {
		coef1(at,0,j)=zero;
		for(k=1;k<=j;k++)
		  coef1(at,k,j)=coef1(at,k-1,j-1);
	      }
	  a=gtrans(at);
	}
    }
  for(j=1;j<=n;j++)
    {
      *y=divii(mulii(gcoeff(a,j,j),*y),da);
      *y=divii(mulii(gcoeff(a,j,j),*y),da);
    }
  tetpil=avma;*y=gcopy(*y);at=cgetg(n+1,17);
  for(j=1;j<=n;j++)
    {
      q=cgetg(j+2,10);q[1]=evalsigne(1)+evallgef(2+j)+evalvarn(v);at[j]=(long)q;
      for(k=2;k<=j+1;k++) q[k]=ldiv(gcoeff(a,j,k-1),da);
    }
  pro=lpile(av,tetpil,0)>>TWOPOTBYTES_IN_LONG;at+=pro;(*y)+=pro;
  return at;
}

GEN base(GEN x, GEN *y)
{
  return allbase4(x,0,y);
}

GEN base2(GEN x, GEN *y)
{
  return allbase(x,0,y);
}

GEN smallbase(GEN x, GEN *y)
{
  return allbase4(x,1,y);
}

GEN factoredbase(GEN x, GEN p, GEN *y)
{
  return allbase4(x,(long)p,y);
}

GEN discf(GEN x)
{
  GEN y;
  long av,tetpil;

  av=avma;allbase4(x,0,&y);tetpil=avma;
  return gerepile(av,tetpil,gcopy(y));
}

GEN discf2(GEN x)
{
  GEN y;
  long av,tetpil;

  av=avma;allbase(x,0,&y);tetpil=avma;
  return gerepile(av,tetpil,gcopy(y));
}

GEN smalldiscf(GEN x)
{
  GEN y;
  long av,tetpil;

  av=avma;allbase4(x,1,&y);tetpil=avma;
  return gerepile(av,tetpil,gcopy(y));
}

GEN factoreddiscf(GEN x, GEN p)
{
  GEN y;
  long av,tetpil;

  av=avma;allbase4(x,(long)p,&y);tetpil=avma;
  return gerepile(av,tetpil,gcopy(y));
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*                                                                 */
/*   Quotient et Reste normalises   ( -1/2 < r = x-q*y <= 1/2 )    */
/*                                                                 */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

GEN rquot(GEN x, GEN y)
{
  GEN u,v,w,p;
  long av,av1;

  av=avma;
  u=absi(y);v=shifti(x,1);w=shifti(y,1);
  if ( cmpii(u,v)>0) p=subii(v,u);
  else p=addsi(-1,addii(u,v));
  av1=avma;
  return gerepile(av,av1,divii(p,w));
}
 
GEN rrmdr(GEN x, GEN y)
{
  GEN p;
  long av,av1;

  av=avma;
  p=mulii(rquot(x,y),y);
  av1=avma;
  return gerepile(av,av1,subii(x,p));
}

GEN rinv(GEN x, GEN y)
{
  GEN a,c,q,r,t;
  long av,av1;

  av=avma;
  a=gun;c=gzero;
  while( signe(y))
	{
	  q=rquot(x,y);
	  r=subii(a,mulii(q,c));a=c;c=r;
	  t=subii(x,mulii(q,y));x=y;y=t;
	}
  av1=avma;
  if (signe(x)<0) a=negi(a);
  if (signe(c)) { av1=avma; a=rrmdr(a,c); }
  return gerepile(av,av1,a);
}

GEN rgcd(GEN x, GEN y)
{
  GEN z;
  long av,av1;

  av=avma;
  while(signe(y))
    {
      z=rrmdr(x,y);x=y;y=z;
    }
  av1=avma;
  return gerepile(av,av1,absi(x));
}

GEN rlcm(GEN x, GEN y)
{
  GEN d,z;
  long av,av1;

  av=avma;
  z=mulii(x,y);d=rgcd(x,y);
  av1=avma;
  return gerepile(av,av1,divii(z,d));
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*                                                                 */
/*           Matrice compagnon du polynome unitaire x              */
/*                                                                 */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

GEN companion(GEN x)
{
  long    i,j,l;
  GEN     y;
  
  l=lgef(x)-2;y=cgetg(l,19);
  for(i=1;i<l;i++) y[i]=lgetg(l,18);
  for(i=0;i<l-2;i++)
    for(j=0;j<l-1;j++) coef1(y,i,j)=((i+1)==j) ? un : zero;
  for(j=0;j<l-1;j++) coef1(y,l-2,j)=lneg((GEN)x[j+2]);
  return y;
}



GEN ordmax(GEN f, GEN p, GEN e, GEN *ptdelta)
{
  GEN m,hh,pp,dd,ppdd,index,q,r,s,b,c,t,jp,v,delta;
  GEN cf[20],w[20],a;
  long h,i,j,k,sp,epsilon,n=lgef(f)-3,av=avma,tetpil,dec;

  a=cgetg(n*n+1,19);
  for(j=1;j<=n*n;j++)
  {
    a[j]=lgetg(n+1,18);
    for(k=1;k<=n;k++) coeff(a,k,j)=zero;
  }
  v=cgetg(n+1,18);
  cf[0]=idmat(n);
  cf[1]=companion(f);
  for(j=2;j<n;j++) cf[j]=gmul(cf[1],cf[j-1]);
  delta=gun; epsilon=itos(e);
  m=idmat(n);

  do
    {
      pp=mulii(p,p);
      dd=mulii(delta,delta);
      ppdd=mulii(dd,pp);
      b=matinv(m,delta);
      for(i=0;i<n;i++)
	{
	  t=gscalsmat(0,n); /* t <--- matrice nulle d'ordre n */
	  for(h=0;h<n;h++)
	    for(j=0;j<n;j++)
	      for(k=0;k<n;k++)
		coef1(t,j,k)=(long)rrmdr(addii(gcoef1(t,j,k),mulii(gcoef1(m,i,h),gcoef1(cf[h],j,k))),ppdd);
	  c=gmul(t,b);
	  w[i]=gmul(m,c);
	  for(j=0;j<n;j++)
	    for(k=0;k<n;k++)
	      coef1(w[i],j,k)=(long)rrmdr(divii(gcoef1(w[i],j,k),dd),pp);
	}
      if(cmpis(p,n)>0)
	{
	  for(i=0;i<n;i++)
	    for(j=0;j<n;j++)
	      {
		coeff(t,i+1,j+1)=zero;
		for(k=0;k<n;k++)
		  for(h=0;h<n;h++)
		    {
		      r=modii(gcoef1(w[i],k,h),p);
		      s=modii(gcoef1(w[j],h,k),p);
		      coef1(t,i,j)=lmodii(addii(gcoef1(t,i,j),mulii(r,s)),p);
		    }
	      }
	}
      else
	{
	  for(j=0;j<n;j++)
	    {
	      for(i=0;i<n;i++)
		coef1(b,i,j)=(i==0)? un : zero;
/* ici la boucle en k calcule la puissance p mod p de w[j] */
	      sp=itos(p);
	      for(k=0;k<sp;k++)
		{
		  for(i=0;i<n;i++)
		    {
		      v[i+1]=zero;
		      for(h=0;h<n;h++)
			v[i+1]=lmodii(addii((GEN)v[i+1],mulii(gcoef1(b,h,j),gcoef1(w[j],h,i))),p);
		    }
		  for(i=0;i<n;i++) coef1(b,i,j)=v[i+1];
		}
	    }
	  q=p;t=b;
	  while(cmpis(q,n)<0)
	    {
	      q=mulii(q,p);
	      t=gmul(b,t);
	    }
	}
      for(i=0;i<n;i++)
	for(j=0;j<n;j++)
	  {
	    coef1(a,j,i)=(i==j)? (long)p : zero;
	    coef1(a,j,n+i)=lmodii(gcoef1(t,i,j),p);
	  }
      rowred(a,2*n-1,pp);
      for(i=0;i<n;i++)
	for(j=0;j<n;j++)
	  coef1(b,j,i)=coef1(a,j,i);
      jp=matinv(b,p);
      for(k=0;k<n;k++)
	{
	  t=gmul(jp,w[k]);
	  t=gmul(t,b);
	  for(i=0;i<n;i++)
	    for(j=0;j<n;j++)
	      coef1(t,i,j)=ldivii(gcoef1(t,i,j),p);
	  h=0;
	  for(i=0;i<n;i++)
	    for(j=0;j<n;j++)
	      {
		coef1(a,k,h)=coef1(t,i,j);
		h++;
	      }
	}
      rowred(a,n*n-1,pp);
      index=gun;
      for(i=0;i<n;i++)
	index=mulii(index,gcoef1(a,i,i));
      if (cmpsi(1,index))
	{
	  delta=mulii(index,delta);
	  for(i=0;i<n;i++)
	    for(j=0;j<n;j++)
	      coef1(c,i,j)=coef1(a,i,j);
	  b=matinv(c,index);
	  m=gmul(b,m);
	  hh=delta;
	  for(i=0;i<n;i++)
	    for(j=0;j<n;j++)
	      hh=rgcd(gcoef1(m,i,j),hh);
	  if(cmpis(hh,1)>1)
	    {
	      delta=divii(delta,hh);
	      for(i=0;i<n;i++)
		for(j=0;j<n;j++)
		  coef1(m,i,j)=ldivii(gcoef1(m,i,j),hh);
	    }
	  q=index;
	  while(!signe(modii(q,p)))
	    {
	      q=divii(q,p);
	      epsilon=epsilon-2;
	    }
	}
    }
  while(!gcmp1(index) && (epsilon>=2));
  tetpil=avma;delta=gcopy(delta);m=gcopy(m);
  dec=lpile(av,tetpil,0)>>TWOPOTBYTES_IN_LONG;
  *ptdelta=delta+dec;
  return m+dec;
}

void rowred(GEN a, long rlim, GEN rmod)
{
  long j,k,n,pro;
  GEN q;

  n=lg((GEN)a[1])-1;
  for(j=0;j<n;j++)
    {
      for(k=j+1;k<=rlim;k++)
	while (signe(gcoef1(a,j,k)))
	  {
	    q=rquot(gcoef1(a,j,j),gcoef1(a,j,k));
	    a[j+1]=(long)mtran((GEN)a[j+1],(GEN)a[k+1],q,rmod);
	    pro=a[j+1];a[j+1]=a[k+1];a[k+1]=pro;
	  }
      if (signe(gcoef1(a,j,j))<0)
	for(k=j;k<n;k++) coef1(a,k,j)=lnegi(gcoef1(a,k,j));
      for(k=0;k<j;k++)
	{
	  q=rquot(gcoef1(a,j,k),gcoef1(a,j,j));
	  a[k+1]=(long)mtran((GEN)a[k+1],(GEN)a[j+1],q,rmod);
	}
    }
}

GEN rtran(GEN v, GEN w, GEN q)
{
  long av,tetpil;
  GEN p1;

  if (signe(q))
    {
      av=avma;p1=gmul(q,w);tetpil=avma;
      return gerepile(av,tetpil,gsub(v,p1));
    }
  else return v;
}

GEN mtran(GEN v, GEN w, GEN q, GEN m)
{
  long k;
  
  if (signe(q))
    {
      for(k=0;k<lg(v)-1;k++)
	{
	  v[k+1]=(long)rrmdr(subii((GEN)v[k+1],modii(mulii(q,(GEN)w[k+1]),m)),m);
	}
    }
  return v;
}


GEN matinv(GEN x, GEN d)
             
/*=======================================================================
    Calcule d/x  ou  d est entier et x matrice triangulaire inferieure
  entiere dont les coeff diagonaux divisent d ( resultat entier).
========================================================================*/
{
  long n,i,j,k,av,av1;
  GEN y,h;

  av=avma;
  y=idmat(n=lg(x)-1);
  for(i=1;i<=n;i++)
    coeff(y,i,i)=ldivii(d,gcoeff(x,i,i));
  for(i=2;i<=n;i++)
    for(j=i-1;j;j--)
      {
	for(h=gzero,k=j+1;k<=i;k++)
	  h=gadd(h,mulii(gcoeff(y,i,k),gcoeff(x,k,j)));
	coeff(y,i,j)=ldivii(negi(h),gcoeff(x,j,j));
      }
  av1=avma;
  return gerepile(av,av1,gcopy(y));
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~									~*/
/*~			HERMITE NORMAL FORM REDUCTION			~*/
/*~									~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

GEN hnf(GEN x)
{
  long li,co,av,av0,tetpil,i,j,k,def,ldef,lim;
  GEN p1,p2,y,z,u,v,d,denx;

  if(typ(x)!=19) err(hnfer1);
  lim=(avma+bot)>>1;
  co=lg(x);if(co==1) return gcopy(x);
  li=lg((GEN)x[1]);av0=avma;denx=denom(x);
  av=avma;y=gcmp1(denx)?gcopy(x):gmul(denx,x);
  def=co;ldef=(li>co)?li-co+1:1;
  for(i=li-1;i>=ldef;i--)
    {
      def--;j=def-1;while(j&&(!signe(gcoeff(y,i,j)))) j--;
      while(j>1)
	{
	  d=bezout(gcoeff(y,i,j),gcoeff(y,i,j-1),&u,&v);
	  p1=gadd(gmul(u,(GEN)y[j]),gmul(v,(GEN)y[j-1]));
	  y[j]=lsub(gmul(divii(gcoeff(y,i,j),d),(GEN)y[j-1]),gmul(divii(gcoeff(y,i,j-1),d),(GEN)y[j]));
	  y[j-1]=(long)p1;
	  j--;while(j&&(!signe(gcoeff(y,i,j)))) j--;
	}
      if(j==1)
	{
	  d=bezout(gcoeff(y,i,1),gcoeff(y,i,def),&u,&v);
	  p1=gadd(gmul(u,(GEN)y[1]),gmul(v,(GEN)y[def]));
	  y[1]=lsub(gmul(divii(gcoeff(y,i,1),d),(GEN)y[def]),gmul(divii(gcoeff(y,i,def),d),(GEN)y[1]));
	  y[def]=(long)p1;
	}
      p1=(GEN)coeff(y,i,def);
      if(signe(p1)<0) {y[def]=lneg((GEN)y[def]);p1=(GEN)coeff(y,i,def);}
      if(signe(p1))
	{
	  for(j=def+1;j<co;j++)
	    {
	      p2=negi(gdivent(gcoeff(y,i,j),p1));
	      y[j]=ladd((GEN)y[j],gmul(p2,(GEN)y[def]));
	    }
	}
      else def++;
      if(avma<lim) {tetpil=avma;y=gerepile(av,tetpil,gcopy(y));}
    }
  for(i=0,j=1;j<co;j++) if(!gcmp0((GEN)y[j])) i++;
  tetpil=avma;z=cgetg(i+1,19);
  for(k=0,j=1;j<co;j++) if(!gcmp0((GEN)y[j])) z[++k]=lcopy((GEN)y[j]);
  if(gcmp1(denx)) return gerepile(av0,tetpil,z);
  else {tetpil=avma;return gerepile(av0,tetpil,gdiv(z,denx));}
}

GEN fasthnf(GEN x,GEN detmat)
{
  long li,co,av,tetpil,i,j,ii,jj,def,lim;
  GEN p1,p2,y,w,u,v,d,dms2,b;

/* usage interne pas de verification. */
  lim=(avma+bot)>>1;
  av=avma;co=lg(x);li=lg((GEN)x[1]);dms2=shifti(detmat,-1);y=x;
  def=co;
  for(i=li-1;i>=1;i--)
    {
      def--;j=co-li;while(j&&(!signe(gcoeff(y,i,j)))) j--;
      if(j)
	{
	  ii=i-1;while(ii&&(!signe(gcoeff(y,ii,def)))) ii--;
	  if(!ii)
	    {
	      p1=(GEN)coeff(y,i,def);
	      if(gcmp1(p1)) 
		{
		  for(jj=j;jj;jj--) coeff(y,i,jj)=zero;
		  j=0;
		}
	      else
		{
		  for(jj=j;jj;jj--) coeff(y,i,jj)=lmodii(gcoeff(y,i,jj),p1);
		  while(j&&(!signe(gcoeff(y,i,j)))) j--;
		}
	    }
	}
      while(j>1)
	{
	  d=bezout(gcoeff(y,i,j),gcoeff(y,i,j-1),&u,&v);
	  if(signe(u))
	    {
	      if(signe(v)) p1=gadd(gmul(u,(GEN)y[j]),gmul(v,(GEN)y[j-1]));
	      else p1=gmul(u,(GEN)y[j]);
	    }
	  else p1=gmul(v,(GEN)y[j-1]);
	  y[j]=lsub(gmul(divii(gcoeff(y,i,j),d),(GEN)y[j-1]),gmul(divii(gcoeff(y,i,j-1),d),(GEN)y[j]));
	  y[j]=(long)cleanmod((GEN)y[j],detmat,dms2);
	  y[j-1]=(long)cleanmod(p1,detmat,dms2);
	  j--;while(j&&(!signe(gcoeff(y,i,j)))) j--;
	  if(avma<lim) {tetpil=avma;y=gerepile(av,tetpil,gcopy(y));}
	}
      if(j==1)
	{
	  d=bezout(gcoeff(y,i,1),gcoeff(y,i,def),&u,&v);
	  if(signe(u))
	    {
	      if(signe(v)) p1=gadd(gmul(u,(GEN)y[1]),gmul(v,(GEN)y[def]));
	      else p1=gmul(u,(GEN)y[1]);
	    }
	  else p1=gmul(v,(GEN)y[def]);
	  y[1]=lsub(gmul(divii(gcoeff(y,i,1),d),(GEN)y[def]),gmul(divii(gcoeff(y,i,def),d),(GEN)y[1]));
	  y[1]=(long)cleanmod((GEN)y[1],detmat,dms2);
	  y[def]=(long)cleanmod(p1,detmat,dms2);
	}
      if(avma<lim) {tetpil=avma;y=gerepile(av,tetpil,gcopy(y));}
    }
  b=detmat;w=cgetg(li,19);def--;
  for(i=li-1;i>=1;i--)
    {
      d=bezout(gcoeff(y,i,i+def),b,&u,&v);w[i]=lmod(gmul(u,(GEN)y[i+def]),b);
      if(!signe(gcoeff(w,i,i))) coeff(w,i,i)=(long)d;
      if(i>1) b=divii(b,d);
    }
  for(i=li-2;i>=1;i--)
    {
      for(j=i+1;j<li;j++)
	{
	  p2=gdivent(gcoeff(w,i,j),gcoeff(w,i,i));w[j]=lsub((GEN)w[j],gmul(p2,(GEN)w[i]));
	}
      if(avma<lim) {tetpil=avma;w=gerepile(av,tetpil,gcopy(w));}
    }
  tetpil=avma;return gerepile(av,tetpil,gcopy(w));
}

GEN cleanmod(GEN x,GEN detmat,GEN detmatsur2)
{
  long lx=lg(x),i;
  GEN y,p1;

  y=cgetg(lx,18);
  for(i=1;i<lx;i++)
    {p1=modii((GEN)x[i],detmat);y[i]=(cmpii(p1,detmatsur2)>0) ? lsubii(p1,detmat) : (long)p1;}
  return y;
}

GEN allhnfmod(GEN x,GEN detmat,long all)
{
  long li,co,av,tetpil,i,j,jm1,def,ldef,lim;
  GEN b,q,w,p1,y,d,u,v,dms2;

  if(typ(x)!=19) err(hnfer1);
  lim=(avma+bot)>>1;
  av=avma;co=lg(x);if(co==1) return cgetg(1,19);
  li=lg((GEN)x[1]);dms2=shifti(detmat,-1);y=gcopy(x);
  def=co;ldef=(li>co)?li-co+1:1;
  for(i=li-1;i>=ldef;i--)
    {
      def--;j=def-1;while(j&&(!signe(gcoeff(y,i,j)))) j--;
      while(j)
	{
	  jm1=j-1;if(!jm1) jm1=def;
	  d=bezout(gcoeff(y,i,j),gcoeff(y,i,jm1),&u,&v);
	  if(signe(u))
	    {
	      if(signe(v)) p1=gadd(gmul(u,(GEN)y[j]),gmul(v,(GEN)y[jm1]));
	      else p1=gmul(u,(GEN)y[j]);
	    }
	  else p1=gmul(v,(GEN)y[jm1]);
	  y[j]=lsub(gmul(divii(gcoeff(y,i,j),d),(GEN)y[jm1]),gmul(divii(gcoeff(y,i,jm1),d),(GEN)y[j]));
	  y[j]=(long)cleanmod((GEN)y[j],detmat,dms2);
	  y[jm1]=(long)cleanmod(p1,detmat,dms2);
	  j--;while(j&&(!signe(gcoeff(y,i,j)))) j--;
	  if(avma<lim) {tetpil=avma;y=gerepile(av,tetpil,gcopy(y));}
	}
    }
  b=detmat;
  w=cgetg(li,19);def--;
  for(i=li-1;i>=1;i--)
    {
      d=bezout(gcoeff(y,i,i+def),b,&u,&v);w[i]=lmod(gmul(u,(GEN)y[i+def]),b);
      if(!signe(gcoeff(w,i,i))) coeff(w,i,i)=(long)d;
      if((i>1)&&all) b=divii(b,d);
    }
  for(i=li-2;i>=1;i--)
    {
      for(j=i+1;j<li;j++)
	{
	  q=gdivent(gcoeff(w,i,j),gcoeff(w,i,i));w[j]=lsub((GEN)w[j],gmul(q,(GEN)w[i]));
	}
      if(avma<lim) {tetpil=avma;w=gerepile(av,tetpil,gcopy(w));}
    }
  tetpil=avma;return gerepile(av,tetpil,gcopy(w));
}

GEN hnfmod(GEN x, GEN detmat)
{
  return allhnfmod(x,detmat,1);
}

GEN hnfmodid(GEN x, GEN p)
{
  return allhnfmod(x,p,0);
}

GEN hnfhavas(GEN x)
 /* rend [y,U,V] tel que y=V.x.U , V vecteur de permutation, U matrice 
    unimodulaire, y forme HNF de x */
{
  long li,co,av,av0,av1,tetpil,i,j,k,def,ldef,lim,fl,fl1,imin,jmin,vpk;
  long jpro,com,dec,vi;
  GEN p1,p2,p3,p12,p13,y,z,u,d,denx,vperm,mat1,col2,lil2,s,pmin,apro,bpro,cpro;

  if(typ(x)!=19) err(hnfer1);
  av0=avma;co=lg(x);if(co==1) return gcopy(x);
  li=lg((GEN)x[1]);denx=denom(x);
  vperm=cgeti(li);for(i=1;i<li;i++) vperm[i]=i;
  lim=(avma+bot)>>1;av=avma;u=idmat(co-1);
  y=gcmp1(denx)?gcopy(x):gmul(denx,x);def=co;ldef=(li>co)?li-co+1:1;
  for(i=li-1;i>=ldef;i--)
    {
      def--;av1=avma;mat1=cgetg(def+1,19);col2=cgetg(def+1,18);
      for(j=1;j<=def;j++)
	{
	  p1=cgetg(i+1,18);mat1[j]=(long)p1;s=gzero;
	  for(k=1;k<=i;k++)
	    {p2=gsqr(gcoeff(y,vperm[k],j));p1[k]=(long)p2;s=gadd(s,p2);}
	  col2[j]=(long)s;
	}
      lil2=cgetg(i+1,18);
      for(k=1;k<=i;k++)
	{
	  s=gzero;
	  for(j=1;j<=def;j++) s=gadd(s,gcoeff(mat1,k,j));
	  lil2[k]=(long)s;
	}
      fl=1;fl1=1;
      for(k=i;k>=1;k--)
	{
	  vpk=vperm[k];
	  if(signe((GEN)lil2[k]))
	    {
	      fl=0;
	      if(fl1||(cmpii((GEN)lil2[k],pmin)<=0))
		{
		  j=1;while(!signe(gcoeff(y,vpk,j))) j++;
		  if(fl1)
		    {
		      imin=k;jmin=j;pmin=mulii((GEN)lil2[k],(GEN)col2[j]);
		      cpro=absi(gcoeff(y,vpk,j));fl1=0;
		    }
		  jpro=j;apro=absi(gcoeff(y,vpk,j));j++;
		  for(;j<=def;j++)
		    {
		      if(signe(gcoeff(y,vpk,j))&&(com=cmpii((GEN)col2[j],(GEN)col2[jpro])<=0))
			{
			  if(com<0) {jpro=j;apro=absi(gcoeff(y,vpk,j));}
			  else
			    {
			      bpro=absi(gcoeff(y,vpk,j));
			      if(cmpii(bpro,apro)<0) {jpro=j;apro=bpro;}
			    }
			}
		    }
		  com=cmpii(p1=mulii((GEN)lil2[k],(GEN)col2[jpro]),pmin);
		  if((com<0)||((com==0)&&(cmpii(apro,cpro)<0)))
		    {pmin=p1;imin=k;jmin=jpro;cpro=apro;}
		}
	    }
	}
      avma=av1;
      if(fl) goto comterm;
      else
	{
	  if(jmin!=def) 
	    {
	      p1=(GEN)y[def];y[def]=y[jmin];y[jmin]=(long)p1;
	      p1=(GEN)u[def];u[def]=u[jmin];u[jmin]=(long)p1;
	    }
	  if(imin!=i) {fl=vperm[i];vperm[i]=vperm[imin];vperm[imin]=fl;}
	  vi=vperm[i];fl=1;
	  while(fl)
	    {
	      if(signe(gcoeff(y,vi,def))<0) 
		{
		  y[def]=lneg((GEN)y[def]);u[def]=lneg((GEN)u[def]);
		}
	      p1=gcoeff(y,vi,def);p12=shifti(p1,-1);p13=negi(p12);
	      for(j=1;j<def;j++)
		{
		  p2=dvmdii(gcoeff(y,vi,j),p1,&p3);
		  if(cmpii(p3,p13)<0) p2=addis(p2,-1);
		  else {if(cmpii(p3,p12)>0) p2=addis(p2,1);}
		  y[j]=ladd((GEN)y[j],gmul(p2=negi(p2),(GEN)y[def]));
		  u[j]=ladd((GEN)u[j],gmul(p2,(GEN)u[def]));
		}
	      j=1;while(!signe(gcoeff(y,vi,j))) j++;
	      if(j<def) 
		{
		  pmin=gnorml2((GEN)y[j]);jmin=j;apro=absi(gcoeff(y,vi,j));
		  j++;
		  for(;j<def;j++)
		    {
		      if(signe(gcoeff(y,vi,j)))
			{
			  p1=gnorml2((GEN)y[j]);com=cmpii(p1,pmin);
			  if((com<0)||((com==0)&&(cmpii(bpro=absi(gcoeff(y,vi,j)),apro)<0)))
			    {
			      pmin=p1;jmin=j;apro=bpro;
			    }
			}
		    }
		  p1=(GEN)y[def];y[def]=y[jmin];y[jmin]=(long)p1;
		  p1=(GEN)u[def];u[def]=u[jmin];u[jmin]=(long)p1;
		}
	      else fl=0;
	    }
	}
      vi=vperm[i];p1=gcoeff(y,vi,def);
      for(j=def+1;j<co;j++)
	{
	  p2=negi(gdivent(gcoeff(y,vi,j),p1));
	  y[j]=ladd((GEN)y[j],gmul(p2,(GEN)y[def]));
	  u[j]=ladd((GEN)u[j],gmul(p2,(GEN)u[def]));
	}
      if(avma<lim)
	{
	  tetpil=avma;y=gcopy(y);u=gcopy(u);
	  dec=lpile(av,tetpil,0)>>TWOPOTBYTES_IN_LONG;
	  y+=dec;u+=dec;
	}
    }
 comterm:
  tetpil=avma;z=cgetg(4,17);
  fl=gcmp1(denx);p1=cgetg(co,19);
  for(j=1;j<co;j++)
    {
      p2=cgetg(li,18);p1[j]=(long)p2;
      for(i=1;i<li;i++) p2[i]=fl?lcopy(gcoeff(y,vperm[i],j)):ldiv(gcoeff(y,vperm[i],j),denx);
    }
  z[1]=(long)p1;z[2]=lcopy(u);
  p1=cgetg(li,17);for(i=1;i<li;i++) p1[i]=lstoi(vperm[i]);
  z[3]=(long)p1;return gerepile(av0,tetpil,z);
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~									~*/
/*~			SMITH NORMAL FORM REDUCTION			~*/
/*~									~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

GEN smith(GEN x)
{
  long li,av,tetpil,i,j,k,l,lim,c,fl,n;
  GEN p1,p2,p3,p4,y,z,b,u,v,d;

  if(typ(x)!=19) err(hnfer1);
  lim=(avma+bot)>>1;
  av=avma;n=lg(x)-1;if(!n) return cgetg(1,17);
  li=lg((GEN)x[1])-1;y=gcopy(x);
  if(li!=n) err(hnfer2);
  for(i=n;i>=2;i--)
    {
      do
	{
	  c=0;
	  for(j=i-1;j>=1;j--)
	    {
	      p1=(GEN)coeff(y,i,j);
	      if(signe(p1))
		{
		  p2=(GEN)coeff(y,i,i);
		  if(gegal(p1,p2)) {d=p1;u=gun;v=gzero;p3=gun;p4=gun;}
		  else if(!signe(addii(p1,p2))) 
		    {
		      d=absi(p1);u=(signe(p2)>0)?gun:gneg(gun);
		      v=gzero;p3=u;p4=gneg(u);
		    }
		  else {d=bezout(p2,p1,&u,&v);p3=divii(p2,d);p4=divii(p1,d);}
		  for(k=1;k<=i;k++)
		    {
		      b=addii(mulii(u,gcoeff(y,k,i)),mulii(v,gcoeff(y,k,j)));
		      coeff(y,k,j)=lsubii(mulii(p3,gcoeff(y,k,j)),mulii(p4,gcoeff(y,k,i)));
		      coeff(y,k,i)=(long)b;
		    }
		}
	    }
	  for(j=i-1;j>=1;j--)
	    {
	      p1=(GEN)coeff(y,j,i);
	      if(signe(p1))
		{
		  p2=(GEN)coeff(y,i,i);
		  if(gegal(p1,p2)) {d=p1;u=gun;v=gzero;p3=gun;p4=gun;}
		  else if(!signe(addii(p1,p2))) 
		    {
		      d=absi(p1);u=(signe(p2)>0)?gun:gneg(gun);
		      v=gzero;p3=u;p4=gneg(u);
		    }
		  else {d=bezout(p2,p1,&u,&v);p3=divii(p2,d);p4=divii(p1,d);}
		  for(k=1;k<=i;k++)
		    {
		      b=addii(mulii(u,gcoeff(y,i,k)),mulii(v,gcoeff(y,j,k)));
		      coeff(y,j,k)=lsubii(mulii(p3,gcoeff(y,j,k)),mulii(p4,gcoeff(y,i,k)));
		      coeff(y,i,k)=(long)b;
		    }
		  c++;
		}
	    }
	  if(!c)
	    {
	      b=(GEN)coeff(y,i,i);fl=1;
	      if(signe(b))
		{
		  for(k=1;(k<i)&&fl;k++)
		    for(l=1;(l<i)&&fl;l++)
		      fl= !signe(modii(gcoeff(y,k,l),b));
		  if(!fl)
		    {
		      k--;
		      for(l=1;l<=i;l++)
			coeff(y,i,l)=laddii(gcoeff(y,i,l),gcoeff(y,k,l));
		    }
		}
	    }
	  if(avma<lim) {tetpil=avma;y=gerepile(av,tetpil,gcopy(y));}
	}
      while(c||(!fl));
    }
  tetpil=avma;z=cgetg(n+1,17);
  for(j=0,k=1;k<=n;k++) if(!signe(gcoeff(y,k,k))) z[++j]=zero;
  for(k=1;k<=n;k++) if(signe(gcoeff(y,k,k))) z[++j]=(long)gabs(gcoeff(y,k,k));
  return gerepile(av,tetpil,z);
}

GEN smith2(GEN x)
{
  long li,av,tetpil,i,j,k,l,lim,c,fl,n,dec;
  GEN p1,p2,p3,p4,y,z,b,u,v,d,ml,mr;

  if(typ(x)!=19) err(hnfer1);
  lim=(avma+bot)>>1;
  av=avma;n=lg(x)-1;
  if(!n) {z=cgetg(3,17);z[1]=lgetg(1,19);z[2]=lgetg(1,19);return z;}
  li=lg((GEN)x[1])-1;y=gcopy(x);
  if(li!=n) err(hnfer2);
  ml=idmat(n);mr=idmat(n);
  for(i=n;i>=2;i--)
    {
      do
	{
	  c=0;
	  for(j=i-1;j>=1;j--)
	    {
	      p1=(GEN)coeff(y,i,j);
	      if(signe(p1))
		{
		  p2=(GEN)coeff(y,i,i);
		  if(gegal(p1,p2)) {d=p1;u=gun;v=gzero;p3=gun;p4=gun;}
		  else if(!signe(addii(p1,p2))) 
		    {
		      d=absi(p1);u=(signe(p2)>0)?gun:gneg(gun);
		      v=gzero;p3=u;p4=gneg(u);
		    }
		  else {d=bezout(p2,p1,&u,&v);p3=divii(p2,d);p4=divii(p1,d);}
		  for(k=1;k<=i;k++)
		    {
		      b=addii(mulii(u,gcoeff(y,k,i)),mulii(v,gcoeff(y,k,j)));
		      coeff(y,k,j)=lsubii(mulii(p3,gcoeff(y,k,j)),mulii(p4,gcoeff(y,k,i)));
		      coeff(y,k,i)=(long)b;
		    }
		  b=gadd(gmul(u,(GEN)mr[i]),gmul(v,(GEN)mr[j]));
		  mr[j]=lsub(gmul(p3,(GEN)mr[j]),gmul(p4,(GEN)mr[i]));
		  mr[i]=(long)b;
		}
	    }
	  for(j=i-1;j>=1;j--)
	    {
	      p1=(GEN)coeff(y,j,i);
	      if(signe(p1))
		{
		  p2=(GEN)coeff(y,i,i);
		  if(gegal(p1,p2)) {d=p1;u=gun;v=gzero;p3=gun;p4=gun;}
		  else if(!signe(addii(p1,p2))) 
		    {
		      d=absi(p1);u=(signe(p2)>0)?gun:gneg(gun);
		      v=gzero;p3=u;p4=gneg(u);
		    }
		  else {d=bezout(p2,p1,&u,&v);p3=divii(p2,d);p4=divii(p1,d);}
		  for(k=1;k<=i;k++)
		    {
		      b=addii(mulii(u,gcoeff(y,i,k)),mulii(v,gcoeff(y,j,k)));
		      coeff(y,j,k)=lsubii(mulii(p3,gcoeff(y,j,k)),mulii(p4,gcoeff(y,i,k)));
		      coeff(y,i,k)=(long)b;
		    }
		  b=gadd(gmul(u,(GEN)ml[i]),gmul(v,(GEN)ml[j]));
		  ml[j]=lsub(gmul(p3,(GEN)ml[j]),gmul(p4,(GEN)ml[i]));
		  ml[i]=(long)b;
		  c++;
		}
	    }
	  if(!c)
	    {
	      b=(GEN)coeff(y,i,i);fl=1;
	      if(signe(b))
		{
		  for(k=1;(k<i)&&fl;k++)
		    for(l=1;(l<i)&&fl;l++)
		      fl= !signe(modii(gcoeff(y,k,l),b));
		  if(!fl)
		    {
		      k--;
		      for(l=1;l<=i;l++)
			coeff(y,i,l)=laddii(gcoeff(y,i,l),gcoeff(y,k,l));
		      ml[i]=ladd((GEN)ml[i],(GEN)ml[k]);
		    }
		}
	    }
	  if(avma<lim) 
	    {
	      tetpil=avma;y=gcopy(y);ml=gcopy(ml);mr=gcopy(mr);
	      dec=lpile(av,tetpil,0)>>TWOPOTBYTES_IN_LONG;y+=dec;ml+=dec;mr+=dec;
	    }
	}
      while(c||(!fl));
    }
  for(k=1;k<=n;k++) if(signe(gcoeff(y,k,k))<0) mr[k]=lneg((GEN)mr[k]);
  ml=gtrans(ml);tetpil=avma;z=cgetg(3,17);
  z[1]=lcopy(ml);z[2]=lcopy(mr);
  return gerepile(av,tetpil,z);
}

GEN transroot(GEN x, int i, int j)
{
  long n=lg(x),k;
  GEN y;

  y=cgetg(n,18);
  for(k=1;k<n;k++) y[k]=((k==i)||(k==j))?x[i+j-k]:x[k];
  return y;
}

GEN tschirnhaus(GEN x)
{
  long av=avma,tetpil,v,n,a,b,c;
  GEN u;

  if(typ(x)!=10) err(galer1);
  n=lgef(x)-3;if(n<=0) err(galer1);v=varn(x);
  if(v) {u=gcopy(x);setvarn(u,0);x=u;}
  do
    {
      a=mymyrand()&3;if(!a) a=1;b=mymyrand()&7;if(b>=4) b-=8;
      c=mymyrand()&7;if(c>=4) c-=8;
      u=caract(gmodulcp(gaddsg(c,gmul(polx[0],gaddsg(b,gmulsg(a,polx[0])))),x),v);
    }
  while(lgef(polgcd(u,deriv(u,v)))>=4);
  tetpil=avma;return gerepile(av,tetpil,gcopy(u));
}

int gpolcomp(GEN p1, GEN p2)
{
  int d,j;

  d=lgef(p1)-3;if((lgef(p2)-3)!=d) err(gpolcompbug1);
  j=d+1;while((j>=2)&&gegal(absi((GEN)p1[j]),absi((GEN)p2[j]))) j--;
  if(j==1) return 0;
  return gcmp(absi((GEN)p1[j]),absi((GEN)p2[j]));
}

GEN galois(GEN x, long prec)
{

  long av=avma,av1,i,j,k,n,f,l,l2,e,e1;
  GEN x1,p1,p2,p3,p4,p5,p6,y,z,el;
  static int ind5[20]={2,5,3,4,1,3,4,5,1,5,2,4,1,2,3,5,1,4,2,3};
  static int ind6[60]={3,5,4,6,2,6,4,5,2,3,5,6,2,4,3,6,2,5,3,4,1,4,5,6,1,5,3,6,1,6,3,4,1,3,4,5,1,6,2,5,1,2,4,6,1,5,2,4,1,3,2,6,1,2,3,5,1,4,2,3};

  if(typ(x)!=10) err(galer1);
  n=lgef(x)-3;if(n<=0) err(galer1);
  if(n>7) err(impl,"galois of degree higher than 7");
  x=gdiv(x,content(x));
  for(i=2;i<=n+2;i++) if(typ((GEN)x[i])!=1) err(galer2);
  p1=(GEN)x[n+2];
  if(!gcmp1(p1))
    {
      x1=cgetg(n+3,10);x1[1]=x[1];x1[n+2]=un;p2=gun;
      for(i=n+1;i>=2;i--) {x1[i]=lmul((GEN)x[i],p2);if(i>2) p2=gmul(p1,p2);}
      x=x1;
    }
  p1=factor(x);
  if((lg((GEN)p1[1])>2)||(!gcmp1(gcoeff(p1,1,2)))) err(impl,"galois of reducible polynomial");
  x1=gcopy(x);av1=avma;
  for(;;)
    {
      switch(n)
	{
	case 1: avma=av;y=cgetg(4,17);y[1]=y[3]=un;y[2]=lneg(gun);return y;
	case 2: avma=av;y=cgetg(4,17);y[1]=deux;y[3]=un;y[2]=lneg(gun);return y;
	case 3: f=carreparfait(discsr(x));avma=av;y=cgetg(4,17);y[3]=un;
	  if(f) {y[2]=un;y[1]=lstoi(3);return y;}
	  else {y[2]=lneg(gun);y[1]=lstoi(6);return y;}
	case 4: 
	  do
	    {
	      p1=rootslong(x,prec);p2=p1;
	      p3=gzero;for(i=1;i<=4;i++) p3=gadd(p3,gmul((GEN)p2[i],gsqr((GEN)p2[(i&3)+1])));
	      p4=gsub(polx[0],p3);p2=transroot(p1,1,2);
	      p3=gzero;for(i=1;i<=4;i++) p3=gadd(p3,gmul((GEN)p2[i],gsqr((GEN)p2[(i&3)+1])));
	      p4=gmul(p4,gsub(polx[0],p3));p2=transroot(p1,1,3);
	      p3=gzero;for(i=1;i<=4;i++) p3=gadd(p3,gmul((GEN)p2[i],gsqr((GEN)p2[(i&3)+1])));
	      p4=gmul(p4,gsub(polx[0],p3));p2=transroot(p1,1,4);
	      p3=gzero;for(i=1;i<=4;i++) p3=gadd(p3,gmul((GEN)p2[i],gsqr((GEN)p2[(i&3)+1])));
	      p4=gmul(p4,gsub(polx[0],p3));p2=transroot(p1,2,3);
	      p3=gzero;for(i=1;i<=4;i++) p3=gadd(p3,gmul((GEN)p2[i],gsqr((GEN)p2[(i&3)+1])));
	      p4=gmul(p4,gsub(polx[0],p3));p2=transroot(p1,3,4);
	      p3=gzero;for(i=1;i<=4;i++) p3=gadd(p3,gmul((GEN)p2[i],gsqr((GEN)p2[(i&3)+1])));
	      p4=gmul(p4,gsub(polx[0],p3));p5=grndtoi(greal(p4),&e);
	      e=max(e,gexpo(gimag(p4)));
	      if(e> -10) prec=(prec<<2)-2;
	    }
	  while(e> -10);
	  p6=ggcd(p5,deriv(p5,0));
	  f=(typ(p6)==10)&&(lgef(p6)>3);
	  if(f) goto tchi;
	  p1=factor(p5);p2=(GEN)p1[1];l=lg(p2)-1;
	  switch(l)
	    {
	    case 1: f=carreparfait(discsr(x));avma=av;y=cgetg(4,17);y[3]=un;
	      if(f) {y[2]=un;y[1]=lstoi(12);return y;}
	      else {y[2]=lneg(gun);y[1]=lstoi(24);return y;}
	    case 2: avma=av;y=cgetg(4,17);y[3]=un;y[2]=lneg(gun);y[1]=lstoi(8);return y;
	    case 3: l2=lgef((GEN)p2[1])-3;
	      if(l2==2) {avma=av;y=cgetg(4,17);y[3]=y[2]=un;y[1]=lstoi(4);return y;}
	      else {avma=av;y=cgetg(4,17);y[3]=un;y[2]=lneg(gun);y[1]=lstoi(4);return y;}
	    default: err(galbug1);
	    }
	case 5:
	  do
	    {
	      do
		{
		  p1=rootslong(x,prec);z=cgetg(7,17);el=cgeti(7);
		  for(l=1;l<=5;l++)
		    {
		      p2=(l==1)?p1:transroot(p1,1,l);
		      p3=gzero;k=0;for(i=1;i<=5;i++)
			{
			  p5=gadd(gmul((GEN)p2[ind5[k]],(GEN)p2[ind5[k+1]]),gmul((GEN)p2[ind5[k+2]],(GEN)p2[ind5[k+3]]));
			  p3=gadd(p3,gmul(gsqr((GEN)p2[i]),p5));k+=4;
			}
		      z[l]=lrndtoi(greal(p3),&e);
		      el[l]=max(e,gexpo(gimag(p3)));
		      p4=(l==1)?gsub(polx[0],p3):gmul(p4,gsub(polx[0],p3));
		    }
		  p2=transroot(p1,2,5);
		  p3=gzero;k=0;for(i=1;i<=5;i++)
		    {
		      p5=gadd(gmul((GEN)p2[ind5[k]],(GEN)p2[ind5[k+1]]),gmul((GEN)p2[ind5[k+2]],(GEN)p2[ind5[k+3]]));
		      p3=gadd(p3,gmul(gsqr((GEN)p2[i]),p5));k+=4;
		    }
		  z[6]=lrndtoi(greal(p3),&e);
		  el[6]=max(e,gexpo(gimag(p3)));
		  p4=gmul(p4,gsub(polx[0],p3));
		  p5=grndtoi(greal(p4),&e);
		  e=max(e,gexpo(gimag(p4)));
		  if(e> -10) prec=(prec<<2)-2;
		}
	      while(e> -10);
	      p6=ggcd(p5,deriv(p5,0));
	      f=(typ(p6)==10)&&(lgef(p6)>3);
	      if(f) goto tchi;
	      p3=factor(p5);l=lg((GEN)p3[1])-1;
	      f=carreparfait(discsr(x));
	      if(l==1)
		{
		  avma=av;y=cgetg(4,17);y[3]=un;
		  if(f) {y[2]=un;y[1]=lstoi(60);return y;}
		  else {y[2]=lneg(gun);y[1]=lstoi(120);return y;}
		}
	      else
		{
		  if(f) 
		    {
		      l=1;while((l<=6)&&((el[l]>-((prec-2)<<(TWOPOTBITS_IN_LONG-1)))||(!gcmp0(poleval(p5,(GEN)z[l]))))) l++;
		      if(l>6) err(galbug4);
		      p2=(l==6)?transroot(p1,2,5):transroot(p1,1,l);
		      p3=gzero;
		      for(i=1;i<=5;i++)
			{
			  j=(i%5)+1;
			  p3=gadd(p3,gmul(gmul((GEN)p2[i],(GEN)p2[j]),gsub((GEN)p2[j],(GEN)p2[i])));
			}
		      p5=gmul(p3,p3);p4=grndtoi(greal(p5),&e1);
		      e1=max(e1,gexpo(gimag(p5)));
		      if(e1<= -10)
			{
			  if(gcmp0(p4)) goto tchi;
			  f=carreparfait(p4);
			  avma=av;y=cgetg(4,17);y[3]=y[2]=un;y[1]=lstoi(f?5:10);return y;
			}
		      else prec=(prec<<2)-2;
		    }
		  else
		    {
		      avma=av;y=cgetg(4,17);y[3]=un;y[2]=lneg(gun);y[1]=lstoi(20);return y;
		    }
		}
	    }
	  while(e1> -10);
	case 6: 
	  do
	    {
	      do
		{
		  p1=rootslong(x,prec);
		  for(l=1;l<=6;l++)
		    {
		      p2=(l==1)?p1:transroot(p1,1,l);
		      p3=gzero;k=0;for(i=1;i<=5;i++) for(j=i+1;j<=6;j++)
			{
			  p5=gadd(gmul((GEN)p2[ind6[k]],(GEN)p2[ind6[k+1]]),gmul((GEN)p2[ind6[k+2]],(GEN)p2[ind6[k+3]]));
			  p3=gadd(p3,gmul(gsqr(gmul((GEN)p2[i],(GEN)p2[j])),p5));k+=4;
			}
		      p4=(l==1)?gsub(polx[0],p3):gmul(p4,gsub(polx[0],p3));
		    }
		  p5=grndtoi(greal(p4),&e);
		  e=max(e,gexpo(gimag(p4)));
		  if(e> -10) prec=(prec<<2)-2;
		}
	      while(e> -10);
	      p6=ggcd(p5,deriv(p5,0));
	      f=(typ(p6)==10)&&(lgef(p6)>3);
	      if(f) goto tchi;
	      p3=factor(p5);p2=(GEN)p3[1];l=lg(p2)-1;
	      switch(l)
		{
		case 1:	p3=gadd(gmul(gmul((GEN)p1[1],(GEN)p1[2]),(GEN)p1[3]),gmul(gmul((GEN)p1[4],(GEN)p1[5]),(GEN)p1[6]));
		  p4=gsub(polx[0],p3);
		  for(i=1;i<=3;i++)
		    for(j=4;j<=6;j++)
		      {
			p2=transroot(p1,i,j);
			p3=gadd(gmul(gmul((GEN)p2[1],(GEN)p2[2]),(GEN)p2[3]),gmul(gmul((GEN)p2[4],(GEN)p2[5]),(GEN)p2[6]));
			p4=gmul(p4,gsub(polx[0],p3));
		      }
		  p5=grndtoi(greal(p4),&e1);
		  e1=max(e1,gexpo(gimag(p4)));
		  if(e1<= 10)
		    {
		      p6=ggcd(p5,deriv(p5,0));
		      f=(typ(p6)==10)&&(lgef(p6)>3);
		      if(f) goto tchi;
		      p3=factor(p5);p2=(GEN)p3[1];l=lg(p2)-1;f=carreparfait(discsr(x));
		      avma=av;y=cgetg(4,17);y[3]=un;
		      if(l==1)
			{
			  if(f) {y[2]=un;y[1]=lstoi(360);return y;}
			  else {y[2]=lneg(gun);y[1]=lstoi(720);return y;}
			}
		      else
			{
			  if(f) {y[2]=un;y[1]=lstoi(36);return y;}
			  else {y[2]=lneg(gun);y[1]=lstoi(72);return y;}
			}
		    }
		  else prec=(prec<<2)-2;
		  break;
		  
		case 2: l2=lgef((GEN)p2[1])-3;if(l2>3) l2=6-l2;
		  switch(l2)
		    {
		    case 1: f=carreparfait(discsr(x));avma=av;y=cgetg(4,17);y[3]=un;
		      if(f) {y[2]=un;y[1]=lstoi(60);return y;}
		      else {y[2]=lneg(gun);y[1]=lstoi(120);return y;}
		    case 2: f=carreparfait(discsr(x));
		      if(f) {avma=av;y=cgetg(4,17);y[3]=y[2]=un;y[1]=lstoi(24);return y;}
		      else
			{
			  p3=(lgef((GEN)p2[1])==5)?(GEN)p2[2]:(GEN)p2[1];
			  f=carreparfait(discsr(p3));avma=av;y=cgetg(4,17);y[2]=lneg(gun);
			  if(f) {y[1]=lstoi(24);y[3]=deux;return y;}
			  else {y[1]=lstoi(48);y[3]=un;return y;}
			}
		    case 3: f=carreparfait(discsr((GEN)p2[1]))||carreparfait(discsr((GEN)p2[2]));
		      avma=av;y=cgetg(4,17);y[3]=un;y[2]=lneg(gun);y[1]=lstoi(f?18:36);
		      return y;
		    }
		case 3: for(l2=1;l2<=3;l2++) if(lgef((GEN)p2[l2])>=6) p3=(GEN)p2[l2];
		  if(lgef(p3)==6)
		    {
		      f=carreparfait(discsr(p3));avma=av;y=cgetg(4,17);y[2]=lneg(gun);
		      y[3]=un;y[1]=f?lstoi(6):lstoi(12);return y;
		    }
		  else
		    {
		      f=carreparfait(discsr(x));avma=av;y=cgetg(4,17);y[3]=un;
		      if(f) {y[2]=un;y[1]=lstoi(12);return y;}
		      else {y[2]=lneg(gun);y[1]=lstoi(24);return y;}
		    }
		case 4: avma=av;y=cgetg(4,17);y[1]=lstoi(6);y[2]=lneg(gun);
		  y[3]=deux;return y;
		default: err(galbug3);
		}
	    }
	  while(e1> -10);
	  
	case 7: 
	  do
	    {
	      p1=rootslong(x,prec);p4=gun;
	      for(i=1;i<=5;i++)
		for(j=i+1;j<=6;j++)
		  for(k=j+1;k<=7;k++)
		    p4=gmul(p4,gsub(polx[0],gadd(gadd((GEN)p1[i],(GEN)p1[j]),(GEN)p1[k])));
	      p5=grndtoi(greal(p4),&e);e=max(e,gexpo(gimag(p4)));
	      if(e> -10) prec=(prec<<2)-2;
	    }
	  while(e> -10);
	  p6=ggcd(p5,deriv(p5,0));
	  f=(typ(p6)==10)&&(lgef(p6)>3);
	  if(f) goto tchi;
	  p1=factor(p5);p2=(GEN)p1[1];l=lg(p2)-1;
	  switch(l)
	    {
	    case 1: f=carreparfait(discsr(x));avma=av;y=cgetg(4,17);y[3]=un;
	      if(f) {y[2]=un;y[1]=lstoi(2520);return y;}
	      else {y[2]=lneg(gun);y[1]=lstoi(5040);return y;}
	    case 2: f=lgef((GEN)p2[1])-3;avma=av;y=cgetg(4,17);y[3]=un;
	      if((f==7)||(f==28)) {y[2]=un;y[1]=lstoi(168);return y;}
	      else {y[2]=lneg(gun);y[1]=lstoi(42);return y;}
	    case 3: avma=av;y=cgetg(4,17);y[3]=y[2]=un;y[1]=lstoi(21);return y;
	    case 4: avma=av;y=cgetg(4,17);y[3]=un;y[2]=lneg(gun);y[1]=lstoi(14);return y;
	    case 5: avma=av;y=cgetg(4,17);y[3]=y[2]=un;y[1]=lstoi(7);return y;
	    default: err(galbug2);
	    }
	}
    tchi:
      avma=av1;x=tschirnhaus(x1);
    }
}

GEN initalg(GEN x, long prec)
{
  GEN y,p1,p2,p3,p4,p5,p6,p7,p10,p11,p12,fieldd,polr,ptrace,dx,adx,polmax,s,a,dxn,adxn,sn,phimax,ind;
  long tx=typ(x),n=lgef(x)-3,i,j,av=avma,av1,av2,av3,v,k,j1,j2,lgp,r1,r2,ru,imax,numb,flc,tetpil;

  if(DEBUGLEVEL) timer2();
  if((tx!=10)||(n<=0)) err(poltyper);
  p1=content(x);p1=gcmp1(p1) ? x: gdiv(x,content(x));
  for(k=2;k<=n+2;k++) if(typ((GEN)p1[k])!=1) err(impl,"general algebraic extension");
  if(gcmp1(p2=(GEN)p1[n+2])) x=p1;
  else
    {
      x=cgetg(n+3,10);x[1]=p1[1];x[n+2]=un;x[n+1]=p1[n+1];p3=p2;
      for(k=n;k>=2;k--)
	{
	  x[k]=lmulii(p3,(GEN)p1[k]);
	  if(k>2) p3=mulii(p2,p3);
	}
    }
  p1=factor(x);
  if(DEBUGLEVEL) {fprintf(errfile,"temps factpol: ");fprintf(errfile,"%ld\n",timer2());fflush(errfile);}
  if(lgef(gcoeff(p1,1,1))!=n+3) err(redper1);
  r1=sturm(x);p4=allbase4(x,0,&fieldd);
  if(DEBUGLEVEL) {fprintf(errfile,"temps round4: ");fprintf(errfile,"%ld\n",timer2());fflush(errfile);}
  if(r1<n)
    {
      polr=roots(x,prec);p3=cgetg(n+1,19);
      for(i=1;i<=n;i++)
	{
	  p1=cgetg(n+1,18);p3[i]=(long)p1;
	  for(j=1;j<=n;j++)
	    p1[j]=lsubst((GEN)p4[i],varn((GEN)p4[i]),(GEN)polr[j]);
	}
      p2=greal(gmul(gconj(gtrans(p3)),p3));
      p1=lllgram(p2,prec);
      for(s=gzero,i=1;i<=n;i++) s=gadd(s,gnorm((GEN)polr[i]));
    }
  else
    {
      ptrace=cgetg(n+1,17);ptrace[1]=lstoi(n);
      for(k=1;k<n;k++) 
	{
	  p3=gmulsg(k,(GEN)x[n-k+2]);
	  for(i=1;i<k;i++) p3=gadd(p3,gmul((GEN)x[n-i+2],(GEN)ptrace[k-i+1]));
	  ptrace[k+1]=lneg(p3);
	}
      p2=cgetg(n+1,19);
      for(i=1;i<=n;i++)
	{
	  p1=cgetg(n+1,18);p2[i]=(long)p1;
	  for(j=1;j<i;j++) p1[j]=lcopy(gcoeff(p2,i,j));
	  for(j=i;j<=n;j++)
	    {
	      p5=gres(gmul((GEN)p4[i],(GEN)p4[j]),x);p6=gzero;
	      for(k=0;k<=lgef(p5)-3;k++) p6=gadd(p6,gmul((GEN)p5[k+2],(GEN)ptrace[k+1]));
	      p1[j]=(long)p6;
	    }
	}
      p1=lllgramint(p2);
      s=(n>1)?gsub(gsqr((GEN)x[n+1]),gmul2n((GEN)x[n],1)):gsqr((GEN)x[2]);
    }
  if(DEBUGLEVEL) {fprintf(errfile,"temps matrice T2: ");fprintf(errfile,"%ld\n",timer2());fflush(errfile);}
  v=varn(x);dx=discsr(x);adx=absi(dx);polmax=x;imax=0;
  a=cgetg(n+1,18);for(i=1;i<=n;i++) a[i]=lmul(p4,(GEN)p1[i]);
  for(numb=0,i=1;i<=n;i++)
    {
      av1=avma;p3=gmodulcp((GEN)a[i],x);p7=content((GEN)p3[2]);
      if(gcmp1(p7)) p3=caract(p3,v);
      else
	{
	  p3=caract(gdiv(p3,p7),v);
	  p3=gmul(gpuigs(p7,lgef(p3)-3),gsubst(p3,v,gdiv(polx[v],p7)));
	}
      p5=ggcd(deriv(p3,v),p3);
      if(lgef(p5)==3)
	{
	  dxn=discsr(p3);adxn=absi(dxn);flc=gcmp(adxn,adx);numb++;
	  if(flc<=0)
	    {
	      if(r1<n) for(sn=gzero,j=1;j<=n;j++) sn=gadd(sn,gnorm(poleval((GEN)a[i],(GEN)polr[j])));
	      else sn=(n>1)?gsub(gsqr((GEN)p3[n+1]),gmul2n((GEN)p3[n],1)):gsqr((GEN)p3[2]);
	      if(flc<0) {dx=dxn;adx=adxn;s=sn;polmax=p3;imax=i;}
	      else 
		{
		  flc=gcmp(sn,s);
		  if((flc<0)||((!flc)&&(gpolcomp(p3,polmax)<0)))
		    {dx=dxn;adx=adxn;s=sn;polmax=p3;imax=i;}
		}
	    }
	}
    }
  if(!numb) err(polreder1);
  phimax=imax?(GEN)a[imax]:polx[v];
  j=n+1;
  while((j>=2)&&(!signe((GEN)polmax[j]))) j-=2;
  if((j>=2)&&(signe((GEN)polmax[j])>0))
    {
      if(polmax==x) polmax=gcopy(x);
      for(;j>=2;j-=2) setsigne((GEN)polmax[j],-signe((GEN)polmax[j]));
      phimax=gneg(phimax);
    }
  if(DEBUGLEVEL) {fprintf(errfile,"temps polmax: ");fprintf(errfile,"%ld\n",timer2());fflush(errfile);}
  p2=lift(gsubst(p4,v,polymodrecip(gmodulcp(phimax,x))));
  p3=cgetg(n+1,19);
  for(j=1;j<=n;j++)
    {
      p4=cgetg(n+1,18);p3[j]=(long)p4;
      for(i=1;i<=n;i++) p4[i]=(long)truecoeff((GEN)p2[j],i-1);
    }
  p4=denom(p3);p1=gmul(p4,p3);
  p2=gdiv(hnfmod(p1,detint(p1)),p4);p3=cgetg(n+1,17);
  for(j=1;j<=n;j++) 
    {
      p1=gzero;for(i=n;i;i--) p1=gadd(gcoeff(p2,i,j),gmul(p1,polx[v]));
      p3[j]=(long)p1;
    }
  if(!carrecomplet(divii(dx,fieldd),&ind)) err(initalgbug1);
  p1=cgetg(n+1,19);
  for(j=1;j<=n;j++)
    {
      p2=cgetg(n+1,18);p1[j]=(long)p2;
      for(i=1;i<=n;i++) p2[i]=(long)truecoeff((GEN)p3[j],i-1);
    }
  p5=cgetg(n*n+1,19);
  for(j=1;j<=n*n;j++) p5[j]=lgetg(n+1,18);
  p11=cgetg(n+1,19);for(j=1;j<=n;j++) p11[j]=lgetg(n+1,18);
  p12=polsym(polmax,lgef(polmax)-4);
  for(i=1;i<=n;i++)
    for(j=i;j<=n;j++)
      {
	p10=gmod(gmul((GEN)p3[j],(GEN)p3[i]),polmax);
	j1=j+(i-1)*n;j2=i+(j-1)*n;lgp=lgef(p10)-2;
	for(k=1;k<=lgp;k++) coeff(p5,k,j1)=coeff(p5,k,j2)=p10[k+1];
	for(;k<=n;k++) coeff(p5,k,j1)=coeff(p5,k,j2)=zero;
	coeff(p11,i,j)=coeff(p11,j,i)=(long)trace9(p10,p12);
      }
  if(DEBUGLEVEL) {fprintf(errfile,"temps table de mult: ");fprintf(errfile,"%ld\n",timer2());fflush(errfile);}
  p2=roots(polmax,prec);
  if(DEBUGLEVEL) {fprintf(errfile,"temps racines: ");fprintf(errfile,"%ld\n",timer2());fflush(errfile);}
  for(i=1;i<=r1;i++) p2[i]=lreal((GEN)p2[i]);
  tetpil=avma;y=cgetg(10,17);y[1]=lcopy(polmax);y[8]=(long)ginv(p1);
  y[9]=lmul((GEN)y[8],p5);p1=cgetg(3,17);
  p1[1]=lstoi(r1);p1[2]=lstoi(r2=(n-r1)>>1);y[2]=(long)p1;ru=r1+r2;
  y[3]=lcopy(fieldd);y[4]=lcopy(ind);
  p4=cgetg(n+1,19);
  for(j=1;j<=n;j++)
    {
      p6=cgetg(ru+1,18);p4[j]=(long)p6;
      for(i=1;i<=ru;i++) p6[i]=(long)gsubst((GEN)p3[j],0,(GEN)p2[max(i,(i<<1)-r1)]);
    }
  av2=avma;p7=cgetg(6,17);p7[1]=(long)p4;
  p5=cgetg(ru+1,19);
  for(j=1;j<=ru;j++)
    {
      p6=cgetg(n+1,18);p5[j]=(long)p6;
      for(i=1;i<=n;i++) 
	p6[i]=(j<=r1)?lconj(gcoeff(p4,j,i)):lmul2n(gconj(gcoeff(p4,j,i)),1);
    }
  p7[2]=(long)p5;p7[3]=(long)greal(gmul(p5,p4));
  p7[4]=lcopy(p11);p7[5]=zero;
  av3=avma;
  y[5]=lpile(av2,av3,gcopy(p7));p4=cgetg(ru+1,17);y[6]=(long)p4;
  for(i=1;i<=ru;i++) p4[i]=lcopy((GEN)p2[max(i,(i<<1)-r1)]);
  if(DEBUGLEVEL) {fprintf(errfile,"temps matrices: ");fprintf(errfile,"%ld\n",timer2());fflush(errfile);}
  y[7]=lcopy(p3);((GEN)y[5])[5]=(long)differente(y);
  if(DEBUGLEVEL) {fprintf(errfile,"temps fin differente: ");fprintf(errfile,"%ld\n",timer2());fflush(errfile);}
  return gerepile(av,tetpil,y);
}

GEN initalgall0(GEN x, long fldif, long prec)
{
  GEN y,p1,p2,p3,p4,p5,p6,p7,p10,p11,p12,fieldd,dx,ind;
  long tx=typ(x),n=lgef(x)-3,i,j,av=avma,av2,av3,v,k,r1,r2,ru,tetpil,j1,j2,lgp;

  if(DEBUGLEVEL) timer2();
  if((tx!=10)||(n<=0)) err(poltyper);
  for(k=2;k<=n+2;k++) if(typ((GEN)x[k])!=1) err(initer1);
  if(!gcmp1((GEN)x[n+2])) err(initer2);
  p1=factor(x);
  if(DEBUGLEVEL) {fprintf(errfile,"temps factpol: ");fprintf(errfile,"%ld\n",timer2());fflush(errfile);}
  if(lgef(gcoeff(p1,1,1))!=n+3) err(redper1);
  r1=sturm(x);p3=allbase4(x,0,&fieldd);
  if(DEBUGLEVEL) {fprintf(errfile,"temps round4: ");fprintf(errfile,"%ld\n",timer2());fflush(errfile);}
  v=varn(x);dx=discsr(x);
  if(!carrecomplet(divii(dx,fieldd),&ind)) err(initalgbug1);
  p1=cgetg(n+1,19);
  for(j=1;j<=n;j++)
    {
      p2=cgetg(n+1,18);p1[j]=(long)p2;
      for(i=1;i<=n;i++) p2[i]=(long)truecoeff((GEN)p3[j],i-1);
    }
  p5=cgetg(n*n+1,19);
  for(j=1;j<=n*n;j++) p5[j]=lgetg(n+1,18);
  p11=cgetg(n+1,19);for(j=1;j<=n;j++) p11[j]=lgetg(n+1,18);
  p12=polsym(x,lgef(x)-4);
  for(i=1;i<=n;i++)
    for(j=i;j<=n;j++)
      {
	p10=gmod(gmul((GEN)p3[j],(GEN)p3[i]),x);
	j1=j+(i-1)*n;j2=i+(j-1)*n;lgp=lgef(p10)-2;
	for(k=1;k<=lgp;k++) coeff(p5,k,j1)=coeff(p5,k,j2)=p10[k+1];
	for(;k<=n;k++) coeff(p5,k,j1)=coeff(p5,k,j2)=zero;
	coeff(p11,i,j)=coeff(p11,j,i)=(long)trace9(p10,p12);
      }
  if(DEBUGLEVEL) {fprintf(errfile,"temps table de mult: ");fprintf(errfile,"%ld\n",timer2());fflush(errfile);}
  p2=roots(x,prec);
  if(DEBUGLEVEL) {fprintf(errfile,"temps racines: ");fprintf(errfile,"%ld\n",timer2());fflush(errfile);}
  for(i=1;i<=r1;i++) p2[i]=lreal((GEN)p2[i]);
  tetpil=avma;y=cgetg(10,17);y[1]=lcopy(x);y[8]=(long)ginv(p1);
  y[9]=lmul((GEN)y[8],p5);p1=cgetg(3,17);
  p1[1]=lstoi(r1);p1[2]=lstoi(r2=(n-r1)>>1);y[2]=(long)p1;ru=r1+r2;
  y[3]=lcopy(fieldd);y[4]=lcopy(ind);
  p4=cgetg(n+1,19);
  for(j=1;j<=n;j++)
    {
      p6=cgetg(ru+1,18);p4[j]=(long)p6;
      for(i=1;i<=ru;i++) p6[i]=(long)gsubst((GEN)p3[j],0,(GEN)p2[max(i,(i<<1)-r1)]);
    }
  av2=avma;p7=cgetg(6,17);p7[1]=(long)p4;
  p5=cgetg(ru+1,19);
  for(j=1;j<=ru;j++)
    {
      p6=cgetg(n+1,18);p5[j]=(long)p6;
      for(i=1;i<=n;i++) 
	p6[i]=(j<=r1)?lconj(gcoeff(p4,j,i)):lmul2n(gconj(gcoeff(p4,j,i)),1);
    }
  p7[2]=(long)p5;p7[3]=(long)greal(gmul(p5,p4));
  p7[4]=lcopy(p11);p7[5]=zero;
  av3=avma;
  y[5]=lpile(av2,av3,gcopy(p7));p4=cgetg(ru+1,17);y[6]=(long)p4;
  for(i=1;i<=ru;i++) p4[i]=lcopy((GEN)p2[max(i,(i<<1)-r1)]);
  if(DEBUGLEVEL) {fprintf(errfile,"temps matrices: ");fprintf(errfile,"%ld\n",timer2());fflush(errfile);}
  y[7]=lcopy(p3);((GEN)y[5])[5]=fldif?(long)differente(y):zero;
  if(DEBUGLEVEL&&fldif) {fprintf(errfile,"temps fin differente: ");fprintf(errfile,"%ld\n",timer2());fflush(errfile);}
  return gerepile(av,tetpil,y);
}

GEN initalg0(GEN x, long prec)
{
  return initalgall0(x,1,prec);
}

GEN initalg2(GEN x, long prec)
{
  GEN p1,p2,p3,p4,p5,p6,p7,fieldd,polr,ptrace,dx,adx,polmax,s,a,dxn,adxn,sn,phimax;
  long tx=typ(x),n=lgef(x)-3,i,j,av=avma,av1,v,k,r1,imax,numb,flc,tetpil;

  if((tx!=10)||(n<=0)) err(poltyper);
  p1=content(x);p1=gcmp1(p1) ? x: gdiv(x,content(x));
  for(k=2;k<=n+2;k++) if(typ((GEN)p1[k])!=1) err(impl,"general algebraic extension");
  if(gcmp1(p2=(GEN)p1[n+2])) x=p1;
  else
    {
      x=cgetg(n+3,10);x[1]=p1[1];x[n+2]=un;x[n+1]=p1[n+1];p3=p2;
      for(k=n;k>=2;k--)
	{
	  x[k]=lmulii(p3,(GEN)p1[k]);
	  if(k>2) p3=mulii(p2,p3);
	}
    }
  p1=factor(x);
  if(lgef(gcoeff(p1,1,1))!=n+3) err(redper1);
  r1=sturm(x);p4=allbase(x,0,&fieldd);
  if(r1<n)
    {
      polr=roots(x,prec);p3=cgetg(n+1,19);
      for(i=1;i<=n;i++)
	{
	  p1=cgetg(n+1,18);p3[i]=(long)p1;
	  for(j=1;j<=n;j++)
	    p1[j]=lsubst((GEN)p4[i],varn((GEN)p4[i]),(GEN)polr[j]);
	}
      p2=greal(gmul(gconj(gtrans(p3)),p3));
      p1=lllgram(p2,prec);
      for(s=gzero,i=1;i<=n;i++) s=gadd(s,gnorm((GEN)polr[i]));
    }
  else
    {
      ptrace=cgetg(n+1,17);ptrace[1]=lstoi(n);
      for(k=1;k<n;k++) 
	{
	  p3=gmulsg(k,(GEN)x[n-k+2]);
	  for(i=1;i<k;i++) p3=gadd(p3,gmul((GEN)x[n-i+2],(GEN)ptrace[k-i+1]));
	  ptrace[k+1]=lneg(p3);
	}
      p2=cgetg(n+1,19);
      for(i=1;i<=n;i++)
	{
	  p1=cgetg(n+1,18);p2[i]=(long)p1;
	  for(j=1;j<i;j++) p1[j]=lcopy(gcoeff(p2,i,j));
	  for(j=i;j<=n;j++)
	    {
	      p5=gres(gmul((GEN)p4[i],(GEN)p4[j]),x);p6=gzero;
	      for(k=0;k<=lgef(p5)-3;k++) p6=gadd(p6,gmul((GEN)p5[k+2],(GEN)ptrace[k+1]));
	      p1[j]=(long)p6;
	    }
	}
      p1=lllgramint(p2);
      s=(n>1)?gsub(gsqr((GEN)x[n+1]),gmul2n((GEN)x[n],1)):gsqr((GEN)x[2]);
    }
  v=varn(x);dx=discsr(x);adx=absi(dx);polmax=x;imax=0;
  a=cgetg(n+1,18);for(i=1;i<=n;i++) a[i]=lmul(p4,(GEN)p1[i]);
  for(numb=0,i=1;i<=n;i++)
    {
      av1=avma;p3=gmodulcp((GEN)a[i],x);p7=content((GEN)p3[2]);
      if(gcmp1(p7)) p3=caract(p3,v);
      else
	{
	  p3=caract(gdiv(p3,p7),v);
	  p3=gmul(gpuigs(p7,lgef(p3)-3),gsubst(p3,v,gdiv(polx[v],p7)));
	}
      p5=ggcd(deriv(p3,v),p3);
      if(lgef(p5)==3)
	{
	  dxn=discsr(p3);adxn=absi(dxn);flc=gcmp(adxn,adx);numb++;
	  if(flc<=0)
	    {
	      if(r1<n) for(sn=gzero,j=1;j<=n;j++) sn=gadd(sn,gnorm(poleval((GEN)a[i],(GEN)polr[j])));
	      else sn=(n>1)?gsub(gsqr((GEN)p3[n+1]),gmul2n((GEN)p3[n],1)):gsqr((GEN)p3[2]);
	      if(flc<0) {dx=dxn;adx=adxn;s=sn;polmax=p3;imax=i;}
	      else 
		{
		  flc=gcmp(sn,s);
		  if((flc<0)||((!flc)&&(gpolcomp(p3,polmax)<0)))
		    {dx=dxn;adx=adxn;s=sn;polmax=p3;imax=i;}
		}
	    }
	}
    }
  if(!numb) err(polreder1);
  phimax=imax?(GEN)a[imax]:polx[v];
  j=n+1;
  while((j>=2)&&(!signe((GEN)polmax[j]))) j-=2;
  if((j>=2)&&(signe((GEN)polmax[j])>0))
    {
      if(polmax==x) polmax=gcopy(x);
      for(;j>=2;j-=2) setsigne((GEN)polmax[j],-signe((GEN)polmax[j]));
      phimax=gneg(phimax);
    }
  p2=gmodulcp(phimax,x);
  tetpil=avma;return gerepile(av,tetpil,polymodrecip(p2));
}


GEN galoisconj2(GEN x, long prec)
{
  long av=avma,tetpil,i,j,n,v;
  GEN y,w,polr,p1,p2,b,di;

  if(typ(x)!=10) err(galer1);
  n=lgef(x)-3;if(n<=0) err(galer1);
  v=varn(x);p1=factor(x);
  if((lg((GEN)p1[1])>2)||(!gcmp1(gcoeff(p1,1,2)))) err(galcer1);
  polr=roots(x,prec);p1=(GEN)polr[1];b=allbase(x,0,&di);
  w=cgetg(n+1,17);w[1]=un;for(i=2;i<=n;i++) w[i]=(long)gsubst((GEN)b[i],v,p1);
  y=cgetg(n+1,17);y[1]=(long)polx[v];
  for(i=2;i<=n;i++)
    {
      p1=lindep(concat(w,(GEN)polr[i]),prec);
      if(gcmp0((GEN)p1[n+1])) y[i]=zero;
      else
	{
	  p2=gzero;
	  for(j=1;j<=n;j++) p2=gadd(p2,gmul((GEN)p1[j],(GEN)b[j]));
	  p2=gdiv(p2,gneg((GEN)p1[n+1]));
	  if(gcmp0(gmod(gsubst(x,v,p2),x))) y[i]=(long)p2;else y[i]=zero;
	}
    }
  tetpil=avma;return gerepile(av,tetpil,gcopy(y));
}

GEN galoisconj(GEN x, long prec)
{
  long av=avma,tetpil,i,j,n,v;
  GEN y,w,polr,p1,p2;

  if(typ(x)!=10) err(galer1);
  n=lgef(x)-3;if(n<=0) err(galer1);
  v=varn(x);p1=factor(x);
  if((lg((GEN)p1[1])>2)||(!gcmp1(gcoeff(p1,1,2)))) err(galcer1);
  polr=roots(x,prec);p1=(GEN)polr[1];
  w=cgetg(n+1,17);w[1]=un;for(i=2;i<=n;i++) w[i]=lmul(p1,(GEN)w[i-1]);
  y=cgetg(n+1,17);y[1]=(long)polx[v];
  for(i=2;i<=n;i++)
    {
      p1=lindep(concat(w,(GEN)polr[i]),prec);
      if(gcmp0((GEN)p1[n+1])) y[i]=zero;
      else
	{
	  p2=gzero;
	  for(j=n;j;j--) p2=gadd((GEN)p1[j],gmul(p2,polx[v]));
	  p2=gdiv(p2,gneg((GEN)p1[n+1]));
	  if(gcmp0(gmod(gsubst(x,v,p2),x))) y[i]=(long)p2;else y[i]=zero;
	}
    }
  tetpil=avma;return gerepile(av,tetpil,gcopy(y));
}


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~									~*/
/*~		      COMPOSITUM OF TWO NUMBER FIELDS                   ~*/
/*~									~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#define sign2(x) (x>=0?0:1)

GEN compositum(GEN pol1, GEN pol2)
{
  long av=avma,tetpil,i,j,a,b,l,l1,l2,fl,c,degmax,a1;
  GEN m,sol,pro0,pro1,pro2,pro3,k1,p;

  if((typ(pol1)!=10)||(typ(pol2)!=10)) err(talker,"incorrect type in compositum");
  if((varn(pol1)!=0)||(varn(pol2)!=0)) err(talker,"not the x variable in compositum");
  l1=lgef(pol1)-3;l2=lgef(pol2)-3;
  if((l1<=0)||(l2<=0)) err(talker,"constant polynomial in compositum");
  l=l1*l2;m=cgetg(l+2,19);
  for(j=1;j<=l+1;j++) m[j]=lgetg(l+1,18);
  fl=1;sol=polun[0];degmax=0;
  for(c=1,a=1;(c<=l)&&fl;a=sign2(a)-a)
    {
      pro0=cgetg(3,10);pro0[1]=evallgef(3)+evalsigne(1);pro0[2]=(long)polun[MAXVARN];
      pro1=gadd(polx[0],gmulsg(a,polx[MAXVARN]));
      for(b=0;b<=l;b++)
	{
	  for(i=0;i<l1;i++)
	    {
	      pro3=truecoeff(pro0,i);setvarn(pro3,0);
	      pro2=gmod(pro3,pol2);
	      for(j=0;j<l2;j++)	coeff(m,i*l2+j+1,l+1-b)=(long)truecoeff(pro2,j);
	    }
	  if(b<l) pro0=gmod(gmul(pro0,pro1),pol1);
	}
      k1=kerint(m);c++;
      for(a1=1;(a1<lg(k1))&&fl;a1++)
	{
	  p=gtopoly((GEN)k1[a1],0);
	  pro0=(GEN)factor(p)[1];
	  for(i=1;(i<lg(pro0))&&fl;i++) 
	    if(lgef((GEN)pro0[i])>lgef(sol)) 
	      {
		sol=(GEN)pro0[i];degmax=lgef(sol)-3;fl=(degmax<l);
	      }
	}
    }
  tetpil=avma;return gerepile(av,tetpil,gdiv(sol,(GEN)sol[lgef(sol)-1]));
}

	  
