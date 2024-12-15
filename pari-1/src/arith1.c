/*********************************************************************/
/*********************************************************************/
/**                                                                 **/
/**                    FONCTIONS ARITHMETIQUES                      **/
/**                                                                 **/
/**                       (premiere partie)                         **/
/**                                                                 **/
/**                      copyright Babe Cool                        **/
/**                                                                 **/
/**                                                                 **/
/*********************************************************************/
/*********************************************************************/

#include  "genpari.h"

/*********************************************************************/
/*       Decomposition binaire d'un I dans un vecteur                */
/*********************************************************************/

GEN binaire(GEN x)
{
  ulong m,u;
  long i,lx,ex,ly,tx=typ(x);
  GEN y,p1,p2;
  
  switch(tx)
    {
    case 1: lx=lgef(x);ly=BITS_IN_LONG+1;
      if (lx==2) {y=cgetg(2,17);y[1]=zero;}
      else
	{
	  m=HIGHBIT;u=x[2];
	  while(!(m&u)) {m>>=1;ly--;}
	  y=cgetg(ly+((lx-3)<<TWOPOTBITS_IN_LONG),17);ly=1;
	  do {y[ly]=m&u ? un : zero;ly++;} while(m>>=1);
	  for(i=3;i<lx;i++)
	    {
	      m=HIGHBIT;u=x[i];
	      do {y[ly]=m&u ? un : zero;ly++;} while(m>>=1);
	    }
	}
      break;
    case 2:
      ex=expo(x);
      if(!signe(x))
	{
	  lx=1+max(-ex,0);y=cgetg(lx,17);
	  for(i=1;i<lx;i++) y[i]=zero;
	}
      else
	{
	  lx=lg(x);y=cgetg(3,17);
	  if(ex>((lx-2)<<TWOPOTBITS_IN_LONG)) err(biner1);
	  p1=cgetg(max(ex,0)+2,17);p2=cgetg(((lx-2)<<TWOPOTBITS_IN_LONG)-ex,17);
	  y[1]=(long)p1;y[2]=(long)p2;
	  if(ex<0)
	    {
	      p1[1]=zero;for(i=1;i<=(-1-ex);i++) p2[i]=zero;
	      ly= -ex;i=2;m=HIGHBIT;
	    }
	  else
	    {
	      ly=1;
	      for(i=2;(i<lx)&&(ly<=ex+1);i++)
		{
		  m=HIGHBIT;u=x[i];
		  do {p1[ly]=m&u ? un : zero;ly++;} while((m>>=1)&&(ly<=ex+1));
		}
	      ly=1;
	      if(m) i--;else m=HIGHBIT;
	    }
	  for(;i<lx;i++)
	    {
	      u=x[i];
	      do {p2[ly]=m&u ? un : zero;ly++;} while(m>>=1);
	      m=HIGHBIT;
	    }
	}
      break;
    case 17: case 18: case 19:
      lx=lg(x);y=cgetg(lx,tx);for(i=1;i<lx;i++) y[i]=(long)binaire((GEN)x[i]);
      break;
    default: err(biner2);
    }
  return y;
}

/* Renvoie 0 ou 1 selon que le bit numero n de x est a 0 ou 1 */

GEN gbittest(GEN x, GEN n)
{
  long l,i,tx;
  GEN y;
  
  if((tx=typ(x))>=17) 
    {
      l=lg(x);y=cgetg(l,tx);for(i=1;i<l;i++) y[i]=(long)gbittest((GEN)x[i],n);
      return y;
    }
  if(tx!=1) err(arither1);
  if((tx=typ(n))>=17)
    {
      l=lg(n);y=cgetg(l,tx);for(i=1;i<l;i++) y[i]=(long)gbittest(x,(GEN)n[i]);
      return y;
    }
  if(tx!=1) err(arither1);
  return stoi(bittest(x,itos(n)));
}

int bittest(GEN x, long n)
{
  long l,n2;
  
  if((!signe(x))||(n<0)) return 0;
  l=lgef(x)-1-(n>>TWOPOTBITS_IN_LONG);
  if(l<=1) return 0;
  n2=(1L<<(n&(BITS_IN_LONG-1)))&x[l];return n2 ? 1 : 0;
}

/*********************************************************************/
/**                                                                 **/
/**            ORDRE DE x entier MODULO n dans (Z/nZ)*              **/
/**                                                                 **/
/*********************************************************************/

GEN order(GEN x)
{
  long av=avma,av1,i,e,u;
  GEN y,t,o,o1,m,p;
  
  if(typ(x)!=3) err(orderer);
  if(!gcmp1(mppgcd(m=(GEN)x[1],(GEN)x[2]))) err(orderer);
  t=decomp(o=phi(m));
  for(i=lg((GEN)t[1])-1;i;i--)
    {
      p=gcoeff(t,i,1);e=itos(gcoeff(t,i,2));
      do
	{ 
	  y=gpui(x,o1=divii(o,p));e--;
	  if(u=gcmp1((GEN)y[2])) o=o1;
	}
      while(e&&u);
    }
  av1=avma;
  return gerepile(av,av1,gcopy(o));
}

/******************************************************************/
/**               GENERATEUR DE (Z/mZ)*                           */
/******************************************************************/

GEN gener(GEN m)
{
  long av=avma,av1,s,k,i,e,u,tx;
  GEN fi,x,xi,y,t,q,p;
  
  if((tx=typ(m))>=17) 
    {
      k=lg(m);y=cgetg(k,tx);for(i=1;i<k;i++) y[i]=(long)gener((GEN)m[i]);
      return y;
    }
  if(tx!=1) err(arither1);
  s=signe(m);setsigne(m,1);if(gcmp1(m)) {setsigne(m,s);return gmodulcp(gzero,gun);}
  e=m[lgef(m)-1];
  if(!(e&3)) 
    {
      setsigne(m,s);
      if(cmpis(m,4)) err(generer);
      return gmodulcp(stoi(3),m);
    }
  if(!(e&1)) 
    {
      t=(GEN)(gener(q=gshift(m,-1))[2]);if(!mpodd(t)) t=addii(t,q);
      av1=avma;return gerepile(av,av1,gmodulcp(t,m));
    }
  t=decomp(m);
  if(lg((GEN)t[1])!=2) err(generer);
  p=gcoeff(t,1,1);e=itos(gcoeff(t,1,2));
  q=fi=subis(p,1);
  if(e>1) fi=mulii(q,gpuigs(p,e-1));
  t=decomp(q);k=lg((GEN)t[1])-1;
  xi=stoi(1);
  do
    {
      xi[2]++;
      i=k;u=1;
      if(gcmp1(mppgcd(xi,m)))
	{
	  x=gmodulcp(xi,m);
	  if(e>1)
	    {
	      y=gpui(x,divii(fi,p));
	      if(gcmp1((GEN)y[2])) {i=u=0;}
	    }
	  while(i)
	    {
	      y=gpui(x,divii(fi,gcoeff(t,i,1)));
	      if (gcmp1((GEN)y[2])) {i=u=0;} else i--;
	    }
	}
      else u=0;
    }
  while(!u);
  av1=avma;
  return gerepile(av,av1,gcopy(x));
}

/*********************************************************************/
/*********************************************************************/
/**                                                                 **/
/**                     FONCTION RACINE                             **/
/**                                                                 **/
/*********************************************************************/
/*********************************************************************/

GEN racine(GEN a)
{
  GEN x,y,z;
  long av,av2,k,tx,count=0,i;

  if((tx=typ(a))>=17) 
    {
      k=lg(a);y=cgetg(k,tx);for(i=1;i<k;i++) y[i]=(long)racine((GEN)a[i]);
      return y;
    }
  if(tx!=1) err(arither1);
  switch (signe(a))
    {
    case -1:
      x=cgetg(3,6); x[1]=zero;
      setsigne(a, 1); x[2]=(long) racine(a); setsigne(a, -1);
      return x;
    case 0 : return gzero;
    case 1 :
      k=(long)sqrt((double)(ulong)mant(a,1));
      x=shifts(k+1,(lgef(a)-3)*(BITS_IN_LONG/2));
      av=avma;
      y=cgeti(lgef(x));
      av2=avma;
      do
	{
	  divisz(addii(x,divii(a,x)),2,y);
	  z=y;y=x;x=z;
	  avma=av2;
	  count++;
	}
      while (cmpii(x,y)<0);
      if (odd(count)) x=y;else affii(y,x);
      avma=av;
      return x;
    }
  return gnil;
}

/*********************************************************************/
/*********************************************************************/
/**                                                                 **/
/**                     FONCTION CARRE PARFAIT                      **/
/**                                                                 **/
/*********************************************************************/
/*********************************************************************/

GEN gcarrecomplet(GEN x, GEN *pt)
{
  long tx,l,i;
  GEN z,y,p,t;
  
  if((tx=typ(x))>=17) 
    {
      l=lg(x);y=cgetg(l,tx);z=cgetg(l,tx);
      for(i=1;i<l;i++) 
	{
	  t=gcarrecomplet((GEN)x[i],&p);y[i]=(long)t;
	  z[i]=gcmp0(t) ? zero: (long)p;
	}
      *pt=z;return y;
    }
  if(tx!=1) err(arither1);
  return stoi(carrecomplet(x,pt));
}

int carrecomplet(GEN x, GEN *pt)
{
  
  static carresmod64[]={1,1,0,0,1,0,0,0,0,1,0,0,0,0,0,0,
			  1,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,
			  0,1,0,0,1,0,0,0,0,1,0,0,0,0,0,0,
			  0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0};
  static carresmod63[]={1,1,0,0,1,0,0,1,0,1,0,0,0,0,0,0,1,0,
			  1,0,0,0,1,0,0,1,0,0,1,0,0,0,0,0,0,0,
			  1,1,0,0,0,0,0,1,0,0,1,0,0,1,0,0,0,0,
			  0,0,0,0,1,0,0,0,0};
  static carresmod65[]={1,1,0,0,1,0,0,0,0,1,1,0,0,0,1,0,1,0,0,0,0,0,0,0,0,1,
			  1,0,0,1,1,0,0,0,0,1,1,0,0,1,1,0,0,0,0,0,0,0,0,1,0,1,
			  0,0,0,1,1,0,0,0,0,1,0,0,1};
  static carresmod11[]={1,1,0,1,1,1,0,0,0,1,0};
  
  GEN y;
  long av,t,result,tetpil;

  switch(signe(x))
    {
    case -1: return 0;
    case 0: {*pt=gzero;return 1;}
    case 1:  if (!carresmod64[63&(mant(x,lgef(x)-2))]) return 0;
    }
  av=avma;
  t=mant(modis(x,63),1);if (!carresmod63[t]) {avma=av;return 0;}
  t=mant(modis(x,65),1);if (!carresmod65[t]) {avma=av;return 0;}
  t=mant(modis(x,11),1);if (!carresmod11[t]) {avma=av;return 0;}
  y=racine(x);
  result=cmpii(mulii(y,y),x);
  if(result) {avma=av;return 0;} 
  else {tetpil=avma;*pt=gerepile(av,tetpil,gcopy(y));return 1;}
}

GEN gcarreparfait(GEN x)
{
  GEN p1,p2,p3,p4;
  long tx=typ(x),l,i,av,v;
  
  switch(tx)
    {
    case 1: return stoi(carreparfait(x));
    case 2: return gun;
    case 3: if(!signe((GEN)x[2])) return gun;
      av=avma;p1=factor(absi((GEN)x[1]));p2=(GEN)p1[1];p3=(GEN)p1[2];l=lg(p2);
      p4=cgeti(lgef((GEN)x[2]));
      for(i=1;i<l;i++)
	{
	  v=pvaluation((GEN)x[2],(GEN)p2[i],&p4);
	  if(v<itos((GEN)p3[i]))
	    {
	      if(v&1) {avma=av;return gzero;}
	      if(gegal((GEN)p2[i],gdeux))
		{
		  v=itos((GEN)p3[i])-v;
		  if(((v>=3)&&((p4[lgef(p4)-1]&7)!=1))||((v==2)&&((p4[lgef(p4)-1]&3)!=1))) {avma=av;return gzero;}
		}
	      else if(kronecker(p4,(GEN)p2[i])== -1) {avma=av;return gzero;}
	    }
	}
      avma=av;return gun;
    case 4: case 5: 
      av=avma;l=carreparfait(mulii((GEN)x[1],(GEN)x[2]));
      avma=av;return stoi(l);
    case 7: p4=(GEN)x[4];if(!signe(p4)) return gun;
      if(valp(x)&1) return gzero;
      if(gegal((GEN)x[2],gdeux)) 
	{
	  v=precp(x);
	  return (((v>=3)&&((p4[lgef(p4)-1]&7)!=1))||((v==2)&&((p4[lgef(p4)-1]&3)!=1)))?gzero:gun;
	}
      else return (kronecker((GEN)x[4],(GEN)x[2])== -1)?gzero:gun;
    case 10: if(!signe(x)) return gun;
      l=lgef(x)-3;if(l&1) return gzero;
      av=avma;p1=gtrunc(gsqrt(gadd(x,ggrando(polx[varn(x)],l+1)),0));
      v=gegal(gmul(p1,p1),x);avma=av;return v?gun:gzero;
    case 11: if(!signe(x)) return gun;
      if(valp(x)&1) return gzero;
      return gcarreparfait((GEN)x[2]);
    case 13: case 14: av=avma;l=itos(gcarreparfait(gmul((GEN)x[1],(GEN)x[2])));
      avma=av;return stoi(l);
    case 15: case 16: return gcarreparfait((GEN)x[1]);
    case 17: case 18: case 19:
      l=lg(x);p1=cgetg(l,tx);for(i=1;i<l;i++) p1[i]=(long)gcarreparfait((GEN)x[i]);
      return p1;
    default: err(impl,"issquare for this type");return gnil;
    }
}

int carreparfait(GEN x)
{
  GEN p1;
  long av=avma,f;
  
  f=carrecomplet(x,&p1);
  avma=av;return f;
}

/*********************************************************************/
/*********************************************************************/
/**                                                                 **/
/**                     SYMBOLE DE KRONECKER                        **/
/**                                                                 **/
/*********************************************************************/
/*********************************************************************/


GEN gkronecker(GEN x, GEN y)
{
  GEN z;
  long tx,ty,l,i;

  if((tx=typ(x))>=17) 
    {
      l=lg(x);z=cgetg(l,tx);for(i=1;i<l;i++) z[i]=(long)gkronecker((GEN)x[i],y);
      return z;
    }
  if(tx!=1) err(arither1);
  if((ty=typ(y))>=17) 
    {
      l=lg(y);z=cgetg(l,ty);for(i=1;i<l;i++) z[i]=(long)gkronecker(x,(GEN)y[i]);
      return z;
    }
  if(ty!=1) err(arither1);
  return stoi(kronecker(x,y));
}

long kronecker(GEN x, GEN y)
{
  GEN x1,y1,z;
  long av,r,s=1;

  av=avma;
  switch (signe(y))
    {
    case -1: y1=negi(y);if (signe(x)<0) s= -1;break;
    case 0: return (lgef(x)==3)&&(x[2]==1);
    case 1: y1=y;
    }
  if (r=vali(y1))
    if (mpodd(x))
      {
	if (odd(r)&&(abs((x[lgef(x)-1]&7)-4)==1)) s= -s;
	y1=shifti(y1,-r);
      }
    else {avma=av;return 0;}
  x1=modii(x,y1);
  while (signe(x1))
    {
      if (r=vali(x1))
	{
	  if (odd(r)&&(abs((y1[lgef(y1)-1]&7)-4)==1)) s= -s;
	  x1=shifti(x1,-r);
	}
      if ((y1[lgef(y1)-1]&2)&&(x1[lgef(x1)-1]&2)) s= -s;
      z=resii(y1,x1);y1=x1;x1=z;
    }
  avma=av;
  return cmpsi(1,y1) ? 0 : s;
}

long  kro8(GEN x, GEN y)
     
              
     
     /*  a usage interne: aucune verification de types  */
     
{
  GEN  p1,p2;
  long k,av=avma;
  
  p1=(GEN)x[1];p2=(GEN)(p1[3]);
  p1=gsub(gmul(p2,p2),gmul2n((GEN)p1[2],2));
  k=kronecker(p1,y);
  avma=av;return k;
}

GEN gkrogs(GEN x, long y)
{
  long l,i,tx;
  GEN t;
  
  if((tx=typ(x))>=17) 
    {
      l=lg(x);t=cgetg(l,tx);for(i=1;i<l;i++) t[i]=(long)gkrogs((GEN)x[i],y);
      return t;
    }
  if(tx!=1) err(arither1);
  return stoi(krogs(x,y));
}

long krogs(GEN x, long y)
{
  long av,r,s=1,x1,z;
  
  av=avma;
  if(y<=0)
    {
      if(y) {y= -y;if(signe(x)<0) s= -1;}
      else  return (lgef(x)==3)&&(x[2]==1);
    }
  if (r=vals(y))
    if (mpodd(x))
      {
	if (odd(r)&&(abs((x[lgef(x)-1]&7)-4)==1)) s= -s;
	y>>=r;
      }
    else return 0;
  x1=itos(modis(x,y));
  while (x1)
    {
      if (r=vals(x1))
	{
	  if (odd(r)&&(abs((y&7)-4)==1)) s= -s;
	  x1>>=r;
	}
      if ((y&2)&&(x1&2)) s= -s;
      z=y%x1;y=x1;x1=z;
    }
  avma=av;
  return (y==1) ? s : 0;
}

long  krosg(long s, GEN x)
{
  long av,y;
  
  av=avma;y=kronecker(stoi(s),x);
  avma=av;return y;
}

long kross(long x, long y)
{
  long r,s=1,x1,z;
  
  if(y<=0)
    {
      if(y) {y= -y;if(x<0) s= -1;}
      else  return (labs(x)==1);
    }
  if (r=vals(y))
    if (odd(x))
      {
	if (odd(r)&&(abs((x&7)-4)==1)) s= -s;
	y>>=r;
      }
    else return 0;
  x1=x%y;if(x1<0) x1+=y;
  while (x1)
    {
      if (r=vals(x1))
	{
	  if (odd(r)&&(abs((y&7)-4)==1)) s= -s;
	  x1>>=r;
	}
      if ((y&2)&&(x1&2)) s= -s;
      z=y%x1;y=x1;x1=z;
    }
  return (y==1) ? s : 0;
}

long  hil(GEN x, GEN y, GEN p)
     
               
     
#define eps(t) (((signe(t)*(t[lgef(t)-1]))&3)==3)
#define ome(t) ((((t[lgef(t)-1])&7)==3)||(((t[lgef(t)-1])&7)==5))
     
{
  long a,b,av,tx=typ(x),ty=typ(y),z;
  GEN p1,p2,u,v;
  
  if(gcmp0(x)||gcmp0(y)) return 0;
  if(tx>ty) {p1=x;x=y;y=p1;av=tx,tx=ty;ty=av;}
  av=avma;
  switch(tx)
    {
    case 1: switch(ty)
      {
      case 1: if(signe(p)<=0) {z=((signe(x)<0)&&(signe(y)<0))?-1:1; return z;}
	a=pvaluation(x,p,&u);b=pvaluation(y,p,&v);
	if(cmpsi(2,p))
          {
            z=((odd(a)&&odd(b)&&eps(p)))? -1:1;
            if(odd(a)&&(kronecker(v,p)<0)) z= -z;
            if(odd(b)&&(kronecker(u,p)<0)) z= -z;
          }
	else
          {
            z=(eps(u)&&eps(v))? -1:1;
            if(odd(a)&&ome(v)) z= -z;
            if(odd(b)&&ome(u)) z= -z;
          }
	avma=av;return z;
      case 2: z=((signe(x)<0)&&(signe(y)<0))?-1:1; return z;
      case 3: if(!cmpsi(2,(GEN)y[1])) err(hiler1);
	return hil(x,(GEN)y[2],(GEN)y[1]);
      case 4:
      case 5: p1=mulii((GEN)y[1],(GEN)y[2]);z=hil(x,p1,p);
	avma=av;return z;
      case 7: 
	if((!cmpsi(2,(GEN)y[2]))&&precp(y)<=2) err(hiler1);
	p1=(odd(valp(y)))?mulii((GEN)y[2],(GEN)y[4]):(GEN)y[4];
	z=hil(x,p1,(GEN)y[2]);avma=av;return z;
      default: err(hiler2);
      }
    case 2: if((ty<4)||(ty>5)) err(hiler2);
      if(signe(x)>0) return 1;
      return signe((GEN)y[1])*signe((GEN)y[2]);
    case 3: if(!cmpsi(2,(GEN)y[1])) err(hiler1);
      switch(ty)
	{
        case 3: if(cmpii((GEN)x[1],(GEN)y[1])) err(hiler2);
          return hil((GEN)x[2],(GEN)y[2],(GEN)x[1]);
        case 4:
        case 5: return hil((GEN)x[2],y,(GEN)x[1]);
        case 7: if(cmpii((GEN)x[1],(GEN)y[2])) err(hiler2);
          return hil((GEN)x[2],y,(GEN)x[1]);
        default: err(hiler2);
	}
    case 4:
    case 5: p1=mulii((GEN)x[1],(GEN)x[2]);switch(ty)
      {
      case 4:
      case 5: p2=mulii((GEN)y[1],(GEN)y[2]);z=hil(p1,p2,p);avma=av;return z;
      case 7: z=hil(p1,y,0);avma=av;return z;
      default: err(hiler2);
      }
    case 7: if((ty>7)||(cmpii((GEN)x[2],(GEN)y[2]))) err(hiler2);
      p1=(odd(valp(x)))?mulii((GEN)x[2],(GEN)x[4]):(GEN)x[4];
      p2=(odd(valp(y)))?mulii((GEN)y[2],(GEN)y[4]):(GEN)y[4];z=hil(p1,p2,(GEN)x[2]);
      avma=av;return z;
    default: err(hiler2);return 0;
    }
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*                                                                 */
/*                      RACINE CARREE MODULO                       */
/*                                                                 */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#define sqrmod(x,p) (modii(mulii(x,x),p))

int mpsqrtmod(GEN a, GEN p, GEN *pr)
{
  long  l,av0,av1,av3,f,i,k,e,r;
  GEN   p1,p2,p3,m,m1,v,y,w,n;
  
  if ((typ(a)!=1) || (typ(p)!=1)) err(arither1);
  av0=avma;
  l=lg(p);v=cgeti(l);av1=avma;
  w=cgeti(l);y=cgeti(l);
  p1=addsi(-1,p);e=vali(p1);
  p2=shifti(p1,-e);n=stoi(2);av3=avma;
  if(e==1) affii(p1,y);
  else do
    {
      m=puissmodulo(n,p2,p);m1=m;
      for(i=1,f=0;(i<e)&&!f;i++)
	f=gcmp1(m=sqrmod(m,p));
      if(f) addsiz(1,n,n);else  affii(m1,y);
      avma=av3;
    }
  while(f);
  
  /*      y contient un generateur de (Z/pZ)* eleve a la puis (p-1)/2^e   */
  
  p1=shifti(p2,-1);
  p2=puissmodulo(a,p1,p);
  if(!signe(p2)) {avma=av0;*pr=gzero;return 1;}
  p3=mulii(p2,a);modiiz(p3,p,v);
  p1=mulii(mulii(p2,p2),a);modiiz(p1,p,w);
  avma=av3;
  r=e;
  while(!gcmp1(w))
    {
      for(k=1 ,p1=w;!gcmp1(p1=sqrmod(p1,p));k++);
      avma=av3;
      if(k==r) {avma=av0;return 0;}
      for(i=1,p1=y;i<r-k;i++)  p1=sqrmod(p1,p);
      modiiz(mulii(p1,v),p,v);
      p1=sqrmod(p1,p);modiiz(mulii(p1,w),p, w);
      affii(p1,y);avma=av3;r=k;
    }
  p1=subii(p,y);if(cmpii(y,p1)>0) affii(p1,y);
  *pr=v;avma=av1;return 1;
}


/*********************************************************************/
/*********************************************************************/
/**                                                                 **/
/**                     FONCTION PGCD                               **/
/**                                                                 **/
/*********************************************************************/
/*********************************************************************/

GEN mppgcd1(GEN a, GEN b)
{
  GEN d,d1;
  long av,count=0;
  if ((typ(a)!=1) || (typ(b)!=1)) err(arither1);
  if (lgef(a)<lgef(b))
    {
      d=b;b=a;a=d;
    }
  d=gcopy(a);
  av=avma;
  d1=gcopy(b);
  while (signe(d1))
    {
      resiiz(d,d1,d);
      a=d;d=d1;d1=a;
      count++;
    }
  if (odd(count))
    {
      affii(d,d1);d=d1;
    }
  avma=av;
  if (signe(d)== -1) setsigne(d,1);
  return d;
}

GEN mppgcd2(GEN a, GEN b)
{
  GEN r;
  long av=avma,tetpil;
  if ((typ(a)!=1) || (typ(b)!=1)) err(arither1);
  if (lgef(a)<lgef(b))
    {
      r=b;b=a;a=r;
    }
  while(signe(b)) {r=resii(a,b);a=b;b=r;}
  tetpil=avma;return gerepile(av,tetpil,absi(a));
}

GEN mppgcd(GEN a, GEN b)
{
  GEN t;
  long av=avma,tetpil,st,k,va,vb;
  if ((typ(a)!=1) || (typ(b)!=1)) err(arither1);
  if (lgef(a)<lgef(b))
    {
      t=b;b=a;a=t;
    }
  b=absi(b);tetpil=avma;a=absi(a);
  if(signe(b)) {t=resii(a,b);a=b;b=t;} else return gerepile(av,tetpil,a);
  if(!signe(b)) {avma=tetpil;return a;}
  va=vali(a);vb=vali(b);k=min(va,vb);a=shifti(a,-va);b=shifti(b,-vb);
  t=subii(a,b);st=signe(t);if(st) setsigne(t,1);
  while(st)
    {
      t=shifti(t,-vali(t));
      if(st>0) a=t;else b=t;
      t=subii(a,b);st=signe(t);if(st) setsigne(t,1);
    }
  tetpil=avma;return gerepile(av,tetpil,shifti(a,k));
}

/*********************************************************************/
/*********************************************************************/
/**                                                                 **/
/**                     FONCTION BEZOUT                             **/
/**                                                                 **/
/*********************************************************************/
/*********************************************************************/

GEN bezout(GEN a, GEN b, GEN *u, GEN *v)
{
  GEN p,u1,v1,d1,d,q,uu,vv,*bof;
  long av,av2,count=0;
  if ((typ(a)!=1) || (typ(b)!=1)) err(arither1);
  if (lgef(b)>lgef(a))
    {
      u1=b;b=a;a=u1;bof=u;u=v;v=bof;
    }
  d=gcopy(a);
  if(!signe(b))
    {
      *u=gun;*v=gzero;
    }
  else
    {
      *u=uu=cgeti(lgef(b));
      *v=vv=cgeti(lgef(a));affsi(0,vv);
      av=avma;
      v1=cgeti(lgef(a));affsi(1,v1);
      d1=gcopy(b);
      q=cgeti(lgef(a));
      av2=avma;
      
      while (signe(d1))
	{
	  dvmdiiz(d,d1,q,d);
	  subiiz(vv,mulii(q,v1),vv);
	  p=vv;vv=v1;v1=p;
	  p=d;d=d1;d1=p;
	  avma=av2;
	  count++;
	}
      if (odd(count))
	{
	  affii(vv,v1);vv=v1;affii(d,d1);d=d1;
	}
      
      diviiz(subii(d,mulii(b,vv)),a,uu);
      avma=av;
      if (signe(d)== -1)
	{
	  setsigne(d,1);setsigne(uu,-signe(uu));setsigne(vv,-signe(vv));
	}
    }
  return d;
}

GEN bezout1(GEN a, GEN b, GEN *u, GEN *v)
{
  long av=avma,tetpil,sa,sb,va,vb,vt,f1,f2,st,i,dec,k;
  GEN u1,v1,t1,v3,t3,q,r,d;

  if ((typ(a)!=1) || (typ(b)!=1)) err(arither1);
  if (lgef(b)>lgef(a)) {u1=b;b=a;a=u1;f1=1;} else f1=0;
  sb=signe(b);b=absi(b);tetpil=avma;sa=signe(a);a=absi(a);
  if(!sb)
    {
      a=gerepile(av,tetpil,a);
      if(f1) {*u=gzero;*v=(sb>=0)?gun:negi(gun);} 
      else {*u=(sa>=0)?gun:negi(gun);*v=gzero;}
      return a;
    }
  q=dvmdii(a,b,&r);a=b;b=r;
  if(!signe(b)) 
    {
      avma=tetpil;
      if(f1) {*u=(sa>=0)?gun:negi(gun);*v=gzero;}
      else {*u=gzero;*v=(sb>=0)?gun:negi(gun);}
      return a;
    }
  va=vali(a);vb=vali(b);k=min(va,vb);if(k) {a=shifti(a,-k);b=shifti(b,-k);}
  if(mpodd(b)) f2=0;
  else {f2=1;u1=b;b=a;a=u1;}
  u1=gun;d=a;v1=v3=b;
  if(mpodd(a)) {t1=gzero;t3=b;st= -1;}
  else {t1=shifti(addsi(1,b),-1);t3=shifti(a,-1);st=1;}
  while(st)
    {
      vt=vali(t3);
      if(vt)
	{
	  t3=shifti(t3,-vt);
	  for(i=1;i<=vt;i++)
	    t1=mpodd(t1)?shifti(addii(t1,b),-1):shifti(t1,-1);
	}
      if(st>0) {u1=t1;d=t3;} else {v1=subii(b,t1);v3=t3;}
      t1=subii(u1,v1);t3=subii(d,v3);if(signe(t1)<0) t1=addii(t1,b);
      st=signe(t3);if(st) setsigne(t3,1);
    }
  v1=divii(subii(d,mulii(a,u1)),b);if(f2) {t1=u1;u1=v1;v1=t1;}
  u1=subii(u1,mulii(v1,q));if(!f1){t1=u1;u1=v1;v1=t1;}
  tetpil=avma;u1=(sa>=0)?gcopy(u1):negi(u1);v1=(sb>=0)?gcopy(v1):negi(v1);
  d=shifti(d,k);dec=lpile(av,tetpil,0)>>TWOPOTBYTES_IN_LONG;u1+=dec;v1+=dec;d+=dec;
  *u=u1;*v=v1;return d;
}

/*********************************************************************/
/*********************************************************************/
/**                                                                 **/
/**                     FONCTION CHINOISE                           **/
/**                                                                 **/
/*********************************************************************/
/*********************************************************************/

GEN chinois(GEN x, GEN y)
{
  GEN z,p1,p2,p3,p4,d,u,v;
  long av,av1,tetpil,dec;
  
  z=cgetg(3,typ(x));
  av=avma;
  d=gbezout((GEN)x[1],(GEN)y[1],&u,&v);
  if(gcmp(gmod((GEN)x[2],d),gmod((GEN)y[2],d))) err(chiner);
  p1=gdiv((GEN)x[1],d);
  p2=gdiv((GEN)y[1],d);
  p3=gmul((GEN)y[2],gmul(u,p1));
  p4=gmul((GEN)x[2],gmul(v,p2));
  p3=gadd(p3,p4);
  tetpil=avma;
  p1=gmul((GEN)x[1],p2);p2=gmod(p3,p1);
  av1=avma;dec=lpile(av,tetpil,0)>>TWOPOTBYTES_IN_LONG;
  z[1]=adecaler(p1,tetpil,av1)?(long)(p1+dec):(long)p1;
  z[2]=adecaler(p2,tetpil,av1)?(long)(p2+dec):(long)p2;
  return z;
}

/*********************************************************************/
/*********************************************************************/
/**                                                                 **/
/**                     FONCTION INVERSE MODULO                     **/
/**                                                                 **/
/**    si a est inversible,on renvoie TRUE et l'inverse dans res   **/
/**    dans le cas contraire,on renvoie FALSE et le pgcd dans res  **/
/**                                                                 **/
/*********************************************************************/
/*********************************************************************/

int inversemodulo(GEN a, GEN b, GEN *res)
{
  GEN u,u1,d1,q;
  long av,av2,count=0;
  if ((typ(a)!=1) || (typ(b)!=1)) err(arither1);
  if (!signe(b))  {*res=mpabs(a);return 0;}
  *res=mpabs(b);
  av=avma;
  u=cgeti(lgef(b));affsi(0,u);
  u1=cgeti(lgef(b));affsi(1,u1);
  d1=cgeti(lgef(b));
  modiiz(a,*res,d1);
  q=cgeti(lgef(b));
  av2=avma;
  
  while (signe(d1))
    {
      dvmdiiz(*res,d1,q,*res);
      subiiz(u,mulii(q,u1),u);
      a=u;u=u1;u1=a;
      a= *res;*res=d1;d1=a;
      avma=av2;
      count++;
    }
  if (odd(count))
    {
      affii(*res,d1);*res=d1;affii(u,u1);u=u1;
    }
  
  if (cmpis(*res,1)) {avma=av;return 0;}
  modiiz(u,b,*res);avma=av;return 1;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*                                                                 */
/*                      INVERSE MODULO                             */
/*                                                                 */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

GEN     mpinvmod(GEN a, GEN m)
{
  GEN   res;
  
  if (inversemodulo(a,m,&res)) return res;
  err(invmoder,"",gmodulcp(res,m));return gnil;
}


/*********************************************************************/
/*********************************************************************/
/**                                                                 **/
/**                     PUISSANCE MODULO                            **/
/**     le resultat occupe autant de place que le module            **/
/**                                                                 **/
/*********************************************************************/
/*********************************************************************/

GEN puissmodulo(GEN a, GEN n, GEN m)
{
  GEN res,aux;
  long av,*p,man,k,nb;
  
  if ((typ(a)!=1) || (typ(n)!=1) || (typ(m)!=1)) err(arither1);
  res=cgeti(lgef(m));
  av=avma;
  switch (signe(n))
    {
    case -1:
      if (inversemodulo(a,m,&aux))
	{
	  affii(puissmodulo(aux,mpabs(n),m),res);
	  avma=av;
	}
      else err(puissmoder);
      break;
    case 0: if(divise(a,m)) affsi(0,res); else affsi(1,res);break;
    case 1: modiiz(a,m,res); p=n+2;
      for (man= *p,k=(BITS_IN_LONG-1);man>0;man<<=1) k--;
      man<<=1;/* le premier bit est implicite */
      for (nb=lgef(n)-2;nb;man= *++p,k=BITS_IN_LONG,nb--)
        for(;k;man<<=1,k--)
	  {
	    modiiz(mulii(res,res),m,res);
	    if (man<0) modiiz(mulii(res,a),m,res);
	    avma=av;
	  }
    }
  return res;
}

/*********************************************************************/
/*********************************************************************/
/**                                                                 **/
/**                     PSEUDO PRIMALITE                            **/
/**             n est-il pseudo-premier fort en base a ?            **/
/**                                                                 **/
/*********************************************************************/
/*********************************************************************/


int pseudopremier(GEN n, GEN a)
{
  long r,av,av2,result;
  GEN t,z,c,b;

  av=avma;
  t=subis(mpabs(n),1);
  if ((r=vali(t))>0)
    {
      b=puissmodulo(a,shifti(t,-r),n);
      if (cmpsi(1,b))
	{
	  c=cgeti(lgef(n));
	  av2=avma;
	  for(;r;r--)
	    {
	      modiiz(mulii(b,b),n,c);
	      z=b;b=c;c=z;
	      avma=av2;
	      if (!cmpsi(1,b)) break;
	    }
	  if (r) result=!cmpii(c,t);
	  else result=0;
	}
      else result=1;
    }
  else result=!cmpsi(1,t);/* t impair ou nul,tester n=2 ou-2 */
  avma=av;
  return result;
}

GEN gpseudopremier(GEN n, GEN a)
{
  long i,l,tx,ty;
  GEN z;

  if((tx=typ(n))>=17) 
    {
      l=lg(n);z=cgetg(l,tx);for(i=1;i<l;i++) z[i]=(long)gpseudopremier((GEN)n[i],a);
      return z;
    }
  if(tx!=1) err(arither1);
  if((ty=typ(a))>=17) 
    {
      l=lg(a);z=cgetg(l,tx);for(i=1;i<l;i++) z[i]=(long)gpseudopremier(n,(GEN)a[i]);
      return z;
    }
  if(ty!=1) err(arither1);
  return stoi(pseudopremier(n,a));
}

/*********************************************************************/
/*********************************************************************/
/**                                                                 **/
/**                     MILLER-RABIN                                **/
/**                                                                 **/
/**             tester k fois la primalite de n                     **/
/**                                                                 **/
/*********************************************************************/
/*********************************************************************/

int millerrabin(GEN n, long k)
{
  long r,r1,av,av2,result;
  GEN t,t1,z,c,b;

  av=avma;
  t=subis(mpabs(n),1);
  if ((r1=vali(t))>0)
    {
      b=cgeti(lgef(n));
      c=cgeti(lgef(n));
      t1=shifti(t,-r1);
      av2=avma;
      result=1;
      for(;k;k--)
	{
	  do z=modii(stoi(mymyrand()),n);
	  while(!signe(z));
	  affii(puissmodulo(z,t1,n),b);
	  if (cmpsi(1,b))
	    {
	      for(r=r1;r;r--)
		{
		  modiiz(mulii(b,b),n,c);
		  z=b;b=c;c=z;
		  avma=av2;
		  if (!cmpsi(1,b)) break;
		}
	      if (r) result=!cmpii(c,t);
	      else result=0;
	    }
	  else result=1;
	  if (!result) break;
	}
    }
  else result=!cmpsi(1,t);/* t impair ou nul,tester n=2 ou-2 */
  avma=av;
  return result;
}

GEN gmillerrabin(GEN n, long k)
{
  long i,l,tx;
  GEN z;

  if((tx=typ(n))>=17)
    {
      l=lg(n);z=cgetg(l,tx);for(i=1;i<l;i++) z[i]=(long)gmillerrabin((GEN)n[i],k);
      return z;
    }
  if (tx!=1) err(arither1);
  return stoi(millerrabin(n,k));
}

GEN     bigprem(GEN n)
{
  long    av1,av2,av3,k,tx,i;
  GEN y;

  av1=avma;
  if((tx=typ(n))>=17) 
    {
      k=lg(n);y=cgetg(k,tx);for(i=1;i<k;i++) y[i]=(long)bigprem((GEN)n[i]);
      return y;
    }
  if(tx!=1) err(arither1);
  if(gcmp(n,gdeux)<=0) return gdeux;
  if(!mpodd(n)) n=addsi(1,n);else n=gcopy(n);
  av3=av2=avma;
  while(!millerrabin(n,10)) {av2=avma;n=addsi(2,n);}
  if (av2!=av3) return gerepile(av1,av2,n);
  return n;
}

GEN gisprime(GEN x)
{
  long tx,l,i;
  GEN y;

  if((tx=typ(x))>=17) 
    {
      l=lg(x);y=cgetg(l,tx);for(i=1;i<l;i++) y[i]=(long)gisprime((GEN)x[i]);
      return y;
    }
  if(tx!=1) err(arither1);
  return stoi(isprime(x));
}

int isprime(GEN x)
{
  long av=avma;

  x=absi(x);if(gcmpgs(x,3)<=0) {avma=av;return !gcmp1(x);}
  if(!mpodd(x)) {avma=av;return 0;}
  avma=av;return millerrabin(x,10);
}

GEN gispsp(GEN x)
{
  long tx,l,i;
  GEN y;

  if((tx=typ(x))>=17) 
    {
      l=lg(x);y=cgetg(l,tx);for(i=1;i<l;i++) y[i]=(long)gispsp((GEN)x[i]);
      return y;
    }
  if(tx!=1) err(arither1);
  return stoi(ispsp(x));
}

int ispsp(GEN x)
{
  long av=avma;

  x=absi(x);if(gcmpgs(x,3)<=0) {avma=av;return !gcmp1(x);}
  if(!mpodd(x)) {avma=av;return 0;}
  avma=av;return millerrabin(x,1);
}

GEN gissquarefree(GEN x)
{
  long tx,lx,i;
  GEN y;

  if((tx=typ(x))>=17) 
    {
      lx=lg(x);y=cgetg(lx,tx);for(i=1;i<lx;i++) 
	y[i]=(long)gissquarefree((GEN)x[i]);
      return y;
    }
  return stoi(issquarefree(x));
}

int issquarefree(GEN x)
{
  long av=avma,tx,lx,i;
  GEN p1;
  
  if(gcmp0(x)) return 0;
  if((tx=typ(x))==1) 
    {
      p1=(GEN)(auxdecomp(x,1)[2]);
      lx=lg(p1);
      for(i=1;(i<lx)&&gcmp1((GEN)p1[i]);i++);
      avma=av;return (i==lx);
    }
  else
    {
      if(tx!=10) err(issquer1);
      p1=ggcd(x,deriv(x,varn(x)));
      avma=av;return (lgef(p1)==3);
    }
}

GEN gisfundamental(GEN x)
{
  long k,i,tx;
  GEN y;
  
  if((tx=typ(x))>=17) 
    {
      k=lg(x);y=cgetg(k,tx);for(i=1;i<k;i++) y[i]=(long)gisfundamental((GEN)x[i]);
      return y;
    }
  if(tx!=1) err(arither1);
  return stoi(isfundamental(x));
}

int isfundamental(GEN x)
{
  long av,f,r;
  GEN p1;
  
  if(gcmp0(x)) return 0;
  r=x[lgef(x)-1]&3;
  if(r==0) 
    {
      av=avma;p1=shifti(x,-2);r=p1[lgef(p1)-1]&3;
      if(r==0) return 0;
      if(signe(x)<0) r=4-r;
      f=(r==1)?0:issquarefree(p1);avma=av;return f;
    }
  if(signe(x)<0) r=4-r;
  return (r==1)?issquarefree(x):0;
}

GEN quaddisc(GEN x)
{
  long av=avma,tetpil,i,r,tx=typ(x);
  GEN p1,p2,f,s;

  if((tx!=1)&&(tx!=4)&&(tx!=5)) err(arither1);
  f=factor(x);p1=(GEN)f[1];p2=(GEN)f[2];
  s=gun;
  for(i=1;i<lg(p1);i++)
    if((((GEN)p2[i])[2])&1) {tetpil=avma;s=gmul(s,(GEN)p1[i]);}
  r=s[lgef(s)-1]&3;if(signe(x)<0) r=4-r;
  if(r>1) {tetpil=avma;s=shifti(s,2);}
  return gerepile(av,tetpil,s);
}

/*********************************************************************/
/*********************************************************************/
/**                                                                 **/
/**                     FONCTION FACTORIELLE                        **/
/**                                                                 **/
/*********************************************************************/
/*********************************************************************/

GEN mpfact(long n)
{
  long av = avma, tetpil, limite = (bot + avma) / 2, k;
  GEN f = gun;

  if (n < 2) if (n < 0) err(facter); else return f;
  for (k = 2; k < n; k++)
    if (avma < limite)
      {
	tetpil = avma;
	f = gerepile(av, tetpil, mulsi(k,f));
      }
    else f = mulsi(k,f);
  tetpil = avma;
  return gerepile(av, tetpil, mulsi(k,f));
}

GEN mpfactr(long n, long prec)
{
  long av = avma, tetpil, limite = (bot + avma) / 2, k;
  GEN f;

  affsr(1,f=cgetr(prec));
  if (n < 2) if (n < 0) err(facter); else return f;
  for (k = 2; k < n; k++)
    if (avma < limite)
      {
	tetpil = avma;
	f = gerepile(av, tetpil, mulsr(k,f));
      }
    else f = mulsr(k,f);
  tetpil = avma;
  return gerepile(av, tetpil, mulsr(k,f));
}

/*********************************************************************/
/*********************************************************************/
/**                                                                 **/
/**                     LUCAS ET FIBONACCI                          **/
/**                                                                 **/
/*********************************************************************/
/*********************************************************************/

void lucas(long n, GEN *ln, GEN *ln1)
{
  long taille,av;
  GEN z,t;
  if (n)
    {
      taille=(long)(C3*(1+labs(n))+3);
      *ln=cgeti(taille);
      *ln1=cgeti(taille);
      av=avma;
      lucas(n/2,&z,&t);
      switch(n % 4)
	{
	case -3:
	  addsiz(2,mulii(z,z),*ln1);
	  subiiz(addsi(1,mulii(z,t)),*ln1,*ln);break;
	case -2:
	  addsiz(2,mulii(z,z),*ln);addsiz(1,mulii(z,t),*ln1);break;
	case -1:
	  subisz(mulii(z,z),2,*ln1);
	  subiiz(subis(mulii(z,t),1),*ln1,*ln);break;
	case  0: subisz(mulii(z,z),2,*ln);subisz(mulii(z,t),1,*ln1);break;
	case  1: subisz(mulii(z,t),1,*ln);addsiz(2,mulii(t,t),*ln1);break;
	case  2: addsiz(2,mulii(z,z),*ln);addsiz(1,mulii(z,t),*ln1);break;
	case  3: addsiz(1,mulii(z,t),*ln);subisz(mulii(t,t),2,*ln1);
	}
      avma=av;
    }
  else
    {
      *ln=stoi(2);
      *ln1=stoi(1);
    }
}

GEN fibo(long n)
{
  long taille,av;
  GEN ln,ln1,f;

  taille=(long)(C3*labs(n)+3);
  f=cgeti(taille);
  av=avma;
  lucas(n-1,&ln,&ln1);
  divisz(addii(mulsi(2,ln),ln1),5,f);
  avma=av;
  return f;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*                                                                 */
/*                      FRACTIONS CONTINUES                        */
/*                                                                 */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

GEN  gcf(GEN x)
{
  return sfcont(x,x,0);
}

GEN  gboundcf(GEN x, long k)
{
  return sfcont(x,x,k);
}

GEN     sfcont(GEN x, GEN x1, long k)
{
  long  av,lx=lg(x),tx=typ(x),e,i,j,l,l1,l2,lx1,tetpil,f,f1;
  GEN   y,p1,p2,p3,p4,yp;
  
  if(tx<10)
    {
      if((tx>=6)||(tx==3)) err(sfconter1);
      if(gcmp0(x)) {y=cgetg(2,17);y[1]=zero;}
      else switch(tx)
        {
        case 1 : y=cgetg(2,17);y[1]=lcopy(x);break;
        case 2 : l=avma;p1=cgetg(3,5);
          p2=gcopy(x);settyp(p2,1);setlgef(p2,lx);
          p1[1]=(long)p2;
          e=((lx-2)<<TWOPOTBITS_IN_LONG)-1-expo(x);
          if(e<0) err(sfconter2);
          l1=(e>>TWOPOTBITS_IN_LONG)+3;p2=cgeti(l1);
          p2[1]=evalsigne(1)+evallgef(l1);
          p2[2]=(1L<<(e&(BITS_IN_LONG-1)));
          for(i=3;i<l1;p2[i]=0,i++);
          p1[2]=(long)p2;p3=cgetg(3,5);
          p3[2]=lcopy(p2);
          p3[1]=laddsi(signe(x),(GEN)p1[1]);
          p1=sfcont(p1,p1,k);tetpil=avma;
          p3=sfcont(p3,p1,k);
          y=gerepile(l,tetpil,p3);
          break;
        case 4 :
        case 5 : l=avma;l1=(long)((double)BYTES_IN_LONG/4.0*46.093443*((lx1=lgef((GEN)x[2]))-2)+3);
	  if((k>0)&&(l1>k+1)) l1=k+1;
	  if(l1>LGBITS) l1=LGBITS;
          if(lgef((GEN)x[1])>=lx1) p1=gcopy((GEN)x[1]);
	  else affii((GEN)x[1],p1=cgeti(lx1));
	  p2=gcopy((GEN)x[2]);l2=avma;lx1=lg(x1);
          y=cgetg(l1,17);f1=1;f=(x!=x1);i=0;
          while(f1&&(!gcmp0(p2))&&(i<=l1-2))
            {
              i++;y[i]=ldvmdii(p1,p2,&p3);
              if(signe(p3)<0)
                {
                  p4=addii(p3,p2);affii(p4,p1);
                  cgiv(p4);cgiv(p3);cgiv((GEN)y[1]);
                  y[1]=laddsi(-1,(GEN)y[1]);
                }
              else {affii(p3,p1);cgiv(p3);}
              p4=p1;p1=p2;p2=p4;
              if(f)
                f1=(i<lx1)&&(!cmpii((GEN)y[i],(GEN)x1[i]));
            }
          if(!f1)
	    {
	      av=avma;f1=gcmp1(absi(subii((GEN)x1[i],(GEN)y[i])));avma=av;
	      if((i>=lx1)||(!f1)) --i;
	      else if((i>=lx1-1)||(!gcmp1((GEN)x1[i+1]))) affii((GEN)x1[i],(GEN)y[i]);
	    }
	  if((i>=2)&&gcmp1((GEN)y[i])) 
	    {cgiv((GEN)y[i]);--i;cgiv((GEN)y[i]);y[i]=laddsi(1,(GEN)y[i]);}
          setlg(y,i+1);l2=l2-((l1-i-1)<<TWOPOTBYTES_IN_LONG);
          y=gerepile(l,l2,y);
        }
    }
  else
    {
      switch(tx)
	{
	case 10: y=cgetg(2,17);y[1]=lcopy(x);break;
	case 11: av=avma;p1=gtrunc(x);tetpil=avma;
	  y=gerepile(av,tetpil,sfcont(p1,p1,k));break;
	case 13:
	case 14:
	  av=avma;l1=lgef((GEN)x[1]);if(lgef((GEN)x[2])>l1) l1=lgef((GEN)x[2]);
	  if((k>0)&&(l1>k+1)) l1=k+1;
	  yp=cgetg(l1,17);p1=(GEN)x[1];i=0;p2=(GEN)x[2];
	  while((!gcmp0(p2))&&(i<=l1-2))
	    {
	      i++;yp[i]=ldivres(p1,p2,&p3);
	      p1=p2;p2=p3;
	    }
	  tetpil=avma;y=cgetg(i+1,17);
	  for(j=1;j<=i;j++) y[j]=lcopy((GEN)yp[j]);
	  y=gerepile(av,tetpil,y);break;
	default: err(sfconter1);
	}
    }
  return y;
}

GEN     gcf2(GEN b, GEN x)
{
  long lb,tb=typ(b),i,av,tetpil;
  GEN y;
  
  if(tb<17) err(sfconter1);
  lb=lg(b);if(lb==1) return cgetg(1,17);
  if(tb==19)
    {
      if(lg((GEN)b[1])==1) return sfcont(x,x,0);
      av=avma;y=cgetg(lb,17);
      for(i=1;i<lb;i++) y[i]=coeff(b,1,i);
      tetpil=avma;return gerepile(av,tetpil,sfcont2(y,x));
    }
  else return sfcont2(b,x);
}

GEN  sfcont2(GEN b, GEN x)
{
  long  av=avma,tx=typ(x),l1=lg(b),i,j,tetpil,f;
  GEN   y,z,p1;
  
  l1=lg(b);y=cgetg(l1,17);
  if(l1==1) return y;
  if(tx<10)
    {
      if((tx>=6)||(tx==3)) err(sfconter1);
    }
  else if(tx==11) x=gtrunc(x);
  if(!gcmp1((GEN)b[1])) x=gmul((GEN)b[1],x);
  y[1]=lfloor(x);p1=gsub(x,(GEN)y[1]);f=!gcmp0(p1);i=2;
  for(;(i<l1)&&f;i++)
    {
      x=gdiv((GEN)b[i],p1);y[i]=lfloor(x);p1=gsub(x,(GEN)y[i]);f=!gcmp0(p1);
    }
  tetpil=avma;z=cgetg(i,17);for(j=1;j<i;j++) z[j]=lcopy((GEN)y[j]);
  return gerepile(av,tetpil,z);
}

GEN  pnqn(GEN x)
{
  long av=avma,tetpil,lx,ly,tx=typ(x),i;
  GEN y,p0,p1,q0,q1,a,b,p2,q2;
  
  if(tx<17) err(pnqner1);
  lx=lg(x);if(lx==1) return idmat(2);
  if(tx<19)
    {
      p0=q1=gun;q0=gzero;p1=(GEN)x[1];
      for(i=2;i<lx;i++)
	{
	  a=(GEN)x[i];
	  p2=gadd(gmul(a,p1),p0);p0=p1;p1=p2;
	  q2=gadd(gmul(a,q1),q0);q0=q1;q1=q2;
	}
      tetpil=avma;y=cgetg(3,19);
      p2=cgetg(3,18);y[1]=(long)p2;p2[1]=lcopy(p1);p2[2]=lcopy(q1);
      p2=cgetg(3,18);y[2]=(long)p2;p2[1]=lcopy(p0);p2[2]=lcopy(q0);
      return gerepile(av,tetpil,y);
    }
  else
    {
      ly=lg((GEN)x[1]);if((ly==1)||(ly>3)) err(pnqner2);
      if(ly==2) 
	{
	  p1=cgetg(lx,17);for(i=1;i<lx;i++) p1[i]=(long)(((GEN)x[i])[1]);
	  tetpil=avma;return gerepile(av,tetpil,pnqn(p1));
	}
      else
	{
	  p0=gun;q0=gzero;p1=gcoeff(x,2,1);q1=gcoeff(x,1,1);
	  for(i=2;i<lx;i++)
	    {
	      a=gcoeff(x,2,i);b=gcoeff(x,1,i);
	      p2=gadd(gmul(a,p1),gmul(b,p0));p0=p1;p1=p2;
	      q2=gadd(gmul(a,q1),gmul(b,q0));q0=q1;q1=q2;
	    }
	  tetpil=avma;y=cgetg(3,19);
	  p2=cgetg(3,18);y[1]=(long)p2;p2[1]=lcopy(p1);p2[2]=lcopy(q1);
	  p2=cgetg(3,18);y[2]=(long)p2;p2[1]=lcopy(p0);p2[2]=lcopy(q0);
	  return gerepile(av,tetpil,y);
	}
    }
}

GEN bestappr(GEN x, GEN k)
{
  long av=avma,tetpil,tx,lx,i;
  GEN p_0,p_1,p,q_0,q_1,q,a,y;

  if((typ(k)==3)||(typ(k)>5)) err(bester1);
  if(gcmp(k,gun)<0) k=gun;
  tx=typ(x);if(tx==5) x=gdiv((GEN)x[1],(GEN)x[2]);
  switch(tx)
    {
    case 1: 
      tetpil=avma;return (av!=avma)?gerepile(av,tetpil,gcopy(x)):gcopy(x);
    case 4: 
      if(gcmp((GEN)x[2],k)<=0)
	{
	  tetpil=avma;return (av!=avma)?gerepile(av,tetpil,gcopy(x)):gcopy(x);
	}
    case 2: p_1=gun;p_0=gfloor(x);q_1=gzero;q_0=gun;a=p_0;
      while(gcmp(q_0,k)<=0)
	{
	  x=gsub(x,a);
	  if(gcmp0(x)) {tetpil=avma;return gerepile(av,tetpil,gdiv(p_0,q_0));}
	  x=gdiv(gun,x);if(gcmp(x,k)<0) a=gfloor(x);else a=k;
	  p=gadd(gmul(a,p_0),p_1);q=gadd(gmul(a,q_0),q_1);
	  p_1=p_0;q_1=q_0;p_0=p;q_0=q;
	}
      tetpil=avma;return gerepile(av,tetpil,gdiv(p_1,q_1));
    case 3: case 7: case 9: case 15: case 16: err(bester2);
    case 10: lx=lgef(x);
    case 6 :
    case 11:
    case 13:
    case 14: 
    case 17:
    case 18:
    case 19: lx=lg(x);y=cgetg(lx,tx);
      for(i=1;i<lontyp[tx];i++) y[i]=x[i];
      for(i=lontyp[tx];i<lx;i++) y[i]=(long)bestappr((GEN)x[i],k);
      return y;
    default: err(bester2);
      return gnil;
    }
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~                                                                     ~*/
/*~                  UNITE FONDAMENTALE ET REGULATEUR                   ~*/
/*~                                                                     ~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

GEN   fundunit(GEN x)
{
  GEN  p1,q1,y,u,v,a,u1,v1,sqd,f,m,c;
  long av,tetpil,r,p4,lx,flp,flq,av2,tx,i;
  
  if((tx=typ(x))>=17) 
    {
      lx=lg(x);y=cgetg(lx,tx);for(i=1;i<lx;i++) y[i]=(long)fundunit((GEN)x[i]);
      return y;
    }
  if(tx!=1) err(arither1);
  if (signe(x)<=0) err(funder1);
  r=(x[lgef(x)-1])&3;
  if((r==2)||(r==3)) err(funder2);
  p4=lclone(quadpoly(x));
  av=avma;sqd=racine(x);lx=lgef(sqd)+1;
  affsi(2,v=cgeti(lx));affsi(r,u=cgeti(lx));
  affii(shifti(addsi(r,sqd),-1),a=cgeti(lx));
  m=cgetg(3,19);
  c=cgetg(3,18);m[1]=(long)c;
  c[1]=(long)a;c[2]=un;
  c=cgetg(3,18);m[2]=(long)c;
  c[1]=un;c[2]=zero;
  av2=avma;
  f=cgetg(3,19);
  c=cgetg(3,18);f[1]=(long)c;
  c[1]=lcopy(a);c[2]=un;
  c=cgetg(3,18);f[2]=(long)c;
  c[1]=un;c[2]=zero;
  do
    {
      u1=subii(mulii(a,v),u);flp=cmpii(u,u1);affii(u1,u);
      v1=divii(subii(x,mulii(u,u)),v);flq=cmpii(v,v1);affii(v1,v);
      diviiz(addii(sqd,u),v,a);
      tetpil=avma;if(flp&&flq) f=gerepile(av2,tetpil,gmul(f,m));
    }
  while(flp&&flq);
  c=(GEN)(f[2]);p1=(GEN)(c[1]);q1=(GEN)(c[2]);
  y=cgetg(4,8);y[1]=p4;
  if(r) { y[2]=lsubii(p1,q1);y[3]=lcopy(q1);}
  else {y[2]=lcopy(p1);y[3]=lcopy(q1);}
  if(!flq)
    {
      f=gmul(f,m);
      c=(GEN)(f[2]);p1=(GEN)(c[1]);q1=(GEN)(c[2]);
      v1=cgetg(4,8);v1[1]=p4;
      if(r) { v1[2]=lsubii(p1,q1);v1[3]=lcopy(q1);}
      else {v1[2]=lcopy(p1);v1[3]=lcopy(q1);}
      u1=gconj(y);tetpil=avma;y=gdiv(v1,u1);
    }
  else
    {
      u1=gconj(y);tetpil=avma;y=gdiv(y,u1);
    }
  if(signe((GEN)y[3])<0) {tetpil=avma;y=gneg(y);}
  return gerepile(av,tetpil,y);
}

GEN   regula(GEN x, long prec)
{
  GEN  reg,reg1,rsqd;
  GEN  rexp,y,u,v,a,u1,v1,sqd;
  long av,tetpil,r,lx,flp,flq,av2,tx,i;
  
  if((tx=typ(x))>=17) 
    {
      lx=lg(x);y=cgetg(lx,tx);for(i=1;i<lx;i++) y[i]=(long)regula((GEN)x[i],prec);
      return y;
    }
  if(tx!=1) err(arither1);
  if (signe(x)<=0) err(funder1);
  r=(x[lg(x)-1])&3;
  if((r==2)||(r==3)) err(funder2);
  av=avma;sqd=racine(x);if(gegal(mulii(sqd,sqd),x)) err(reguler1);
  rsqd=gsqrt(x,prec);affsi(0,rexp=cgeti(4));
  lx=lgef(sqd)+1;affsr(2,reg=cgetr(prec));
  affsi(2,v=cgeti(lx));affsi(r,u=cgeti(lx));
  affii(shifti(addsi(r,sqd),-1),a=cgeti(lx));
  av2=avma;
  do
    {
      u1=subii(mulii(a,v),u);flp=cmpii(u,u1);affii(u1,u);
      reg1=divri(addir(u,rsqd),v);
      v1=divii(subii(x,mulii(u,u)),v);flq=cmpii(v,v1);
      if(flp&&flq) {affii(v1,v);diviiz(addii(sqd,u),v,a);}
      tetpil=avma;
      if(flp&&flq)
        {
          reg=gerepile(av2,tetpil,mulrr(reg,reg1));
          r=expo(reg);addsiz(r,rexp,rexp);setexpo(reg,0);
        }
    }
  while(flp&&flq);
  reg=shiftr(mulrr(reg,reg),-1);
  if(!flq)
    {
      u1=subii(mulii(a,v),u);reg1=divri(addir(u,rsqd),v);
      reg=divri(mulrr(reg,reg1),v);
    }
  else reg=divri(reg,v);
  y=glog(reg,prec);u1=mpshift(mulir(rexp,glog(gdeux,prec)),1);
  tetpil=avma;return gerepile(av,tetpil,gadd(y,u1));
}


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~                                                                     ~*/
/*~                         NOMBRE DE CLASSES                           ~*/
/*~                                                                     ~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

GEN classno2(GEN x)
{
  long av=avma,tetpil,n,i,k,s=signe(x),fl2,ex,lx,tx;
  GEN p1,p2,p3,p4,p5,p6,p7,p9,pi4,d,reg,logd,y;
  
  if((tx=typ(x))>=17) 
    {
      lx=lg(x);y=cgetg(lx,tx);for(i=1;i<lx;i++) y[i]=(long)classno2((GEN)x[i]);
      return y;
    }
  if(tx!=1) err(arither1);
  if (!s) err(arither2);
  p1=auxdecomp(absi(x),1);p2=(GEN)p1[2];p1=(GEN)p1[1];n=lg(p1);p3=gun;fl2=0;
  for(i=1;i<n;i++)
    {
      ex=itos((GEN)p2[i]);
      if(ex>=2)
	{
	  p4=(GEN)p1[i];
	  if(ex&1) p3=mulii(p3,p4);
	  if(gegal(p4,gdeux)) fl2=1;
	}
      else p3=mulii(p3,(GEN)p1[i]);
    }
  if((p3[lgef(p3)-1]&3)!=(2-s))
    {
      if(fl2) p3=shifti(p3,2);
      else err(classer2);
    }
  else fl2=0;
  p9=gun;x=(s<0) ? gneg(p3) : p3;
  for(i=1;i<n;i++)
    {
      ex=itos((GEN)p2[i]);p4=(GEN)p1[i];
      if(gegal(p4,gdeux)&&fl2) ex-=2;
      if(ex>=2)
	{
	  p9=mulii(p9,subis(p4,kronecker(x,p4)));
	  if(ex>=4) p9=mulii(p9,gpuigs(p4,(ex>>1)-1));
	}
    }
  pi4=mppi(4);
  if(s>0)
    {
      reg=regula(x,4);logd=glog(x,4);
      p1=gsqrt(gdiv(gmul(x,logd),gmul2n(pi4,1)),4);
      p2=gsubsg(1,gmul2n(gdiv(glog(reg,4),logd),1));
      p3=gsqrt(gdivsg(2,logd),4);
      if(gcmp(p2,p3)>=0) p1=gmul(p2,p1);
      p1=gtrunc(p1);
      if(lgef(p1)!=3) err(classer1);
      n=p1[2];if(n<0) err(classer1);
      p1=gsqrt(x,4);p4=divri(pi4,x);p3=gzero;
      p7=divsr(1,mpsqrt(pi4));
      for(i=1;i<=n;i++)
        {
          k=krogs(x,i);
          if(k)
            {
              p6=mulir(mulss(i,i),p4);p5=subsr(1,mulrr(p7,incgam3(ghalf,p6,4)));
              p5=addrr(divrs(mulrr(p1,p5),i),eint1(p6,4));
              if(k>0) p3=addrr(p3,p5);else p3=subrr(p3,p5);
            }
        }
      p3=shiftr(divrr(p3,reg),-1);
    }
  else
    {
      d=p3;
      if(gcmpgs(x,-11)>=0) {tetpil=avma;return gerepile(av,tetpil,gcopy(p9));}
      p1=gtrunc(gsqrt(gdiv(gmul(d,glog(d,4)),gmul2n(pi4,1)),4));
      if(lgef(p1)!=3) err(classer1);
      n=p1[2];if(n<0) err(classer1);
      p1=gdiv(gsqrt(d,4),pi4);p4=divri(pi4,d);p3=gzero;
      p7=divsr(1,mpsqrt(pi4));
      for(i=1;i<=n;i++)
        {
          k=krogs(x,i);
          if(k)
            {
              p6=mulir(mulss(i,i),p4);p5=subsr(1,mulrr(p7,incgam3(ghalf,p6,4)));
              p5=addrr(p5,divrr(divrs(p1,i),mpexp(mulir(mulss(i,i),p4))));
              if(k>0) p3=addrr(p3,p5);else p3=subrr(p3,p5);
            }
        }
    }
  p3=ground(p3);tetpil=avma;return gerepile(av,tetpil,gmul(p9,p3));
}

GEN classno3(GEN x)
{
  long d,a,b,h,b2,f,av,tetpil;
  GEN y;
  
  d= -itos(x);if((d>0)||((d&3)>1)) return gzero;
  if(!d) return gdivgs(gun,-12);
  h=0;b=d&1;b2=(b-d)>>2;f=0;
  if(!b)
    {
      for(a=1;a*a<b2;a++) {if(!(b2%a)) h++;}
      f=(a*a==b2);b=2;b2=(4-d)>>2;
    }
  while(b2*3+d<0)
    {
      if(!(b2%b)) h++;
      for(a=b+1;a*a<b2;a++) {if(!(b2%a)) h+=2;}
      if(a*a==b2) h++;
      b+=2;b2=(b*b-d)>>2;
    }
  if(b2*3+d==0) 
    {
      av=avma;y=gdivgs(gun,3);tetpil=avma;
      return gerepile(av,tetpil,gaddsg(h,y));
    }
  if(f) return gaddsg(h,ghalf);else return stoi(h);
}
