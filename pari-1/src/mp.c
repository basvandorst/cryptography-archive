/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~									~*/
/*~		       OPERATIONS DE BASE (NOYAU)			~*/
/*~									~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

# include "genpari.h"

GEN   mulsi(long x, GEN y)
{
  long s=signe(y),ly=lgef(y),i;
  GEN z;
  
  if((!x)||(!s)) return gzero;
  if(x<0) {s= -s;x= -x;}
  z=cgeti(ly+1);hiremainder=0;
  for(i=ly-1;i>=2;i--) z[i+1]=addmul(x,y[i]);
  if(hiremainder) {z[2]=hiremainder;z[1]=evallgef(ly+1)+evalsigne(s);}
  else {avma+=BYTES_IN_LONG;z[1]=z[0]-1;z++;z[1]=evallgef(ly)+evalsigne(s);}
  return z;
}

GEN stoi(long x)
{
  GEN y;
  
  if(!x) return gzero;
  y=cgeti(3);
  if(x>0) {y[1]=evalsigne(1)+evallgef(3);y[2]=x;}
  else {y[1]=evalsigne(-1)+evallgef(3);y[2]= -x;}
  return y;
}

GEN cgetg(long x, long y)
{
  ulong p1;
  GEN z;
  
  p1=avma-(((ulong)x)<<TWOPOTBYTES_IN_LONG);
  if(p1<bot) err(errpile);
  avma=p1;z=(GEN)p1;z[0]=evalpere(1)+evaltyp(y)+evallg(x);
  return z;
}

GEN cgeti(long x)
{
  ulong p1;
  GEN z;
  
  p1=avma-(((ulong)x)<<TWOPOTBYTES_IN_LONG);
  if(p1<bot) err(errpile);
  avma=p1;z=(GEN)p1;z[0]=evaltyp(1)+evalpere(1)+evallg(x);
  return z;
}

GEN cgetr(long x)
{
  ulong p1;
  GEN z;
  
  p1=avma-(((ulong)x)<<TWOPOTBYTES_IN_LONG);
  if(p1<bot) err(errpile);
  avma=p1;z=(GEN)p1;z[0]=evaltyp(2)+evalpere(1)+evallg(x);
  return z;
}

GEN icopy(GEN x)
{
  GEN y;
  long lx=lgef(x),i;
  
  y=cgeti(lx);
  for(i=1;i<lx;i++) y[i]=x[i];
  return y;
}

GEN rcopy(GEN x)
{
  GEN y;
  long lx=lg(x),i;
  
  y=cgetr(lx);
  for(i=1;i<lx;i++) y[i]=x[i];
  return y;
}


GEN negi(GEN x)
{
  long s=signe(x);
  GEN y;
  
  if(!s) return gzero;
  y=icopy(x);setsigne(y,-s);
  return y;
}

GEN negr(GEN x)
{
  GEN y;
  
  y=rcopy(x);setsigne(y,-signe(x));
  return y;
}


GEN absi(GEN x)
{
  GEN y;
  long s=signe(x);
  
  if(!s) return gzero;
  y=icopy(x);setsigne(y,1);return y;
}

GEN absr(GEN x)
{
  GEN y;
  long s=signe(x);
  
  y=rcopy(x);
  if(s) setsigne(y,1);
  return y;
}

int expi(GEN x)
{
  long lx=lgef(x);
  
  return lx==2 ? -HIGHEXPOBIT : ((lx-2)<<TWOPOTBITS_IN_LONG)-bfffo(x[2])-1;
}

long itos(GEN x)
{
  long s=signe(x),p2;
  ulong p1;
  
  if(!s) return 0;
  if(lgef(x)>3) err(affer2);
  p1=x[2];if(p1>=(ulong)HIGHBIT) err(affer2);
  p2=(s>0)?p1:(-((long)p1));return p2;
}

void mpaff(GEN x, GEN y)
{
  long tx=typ(x),ty=typ(y);
  if(tx==1)
    {
      if(ty==1) affii(x,y);else affir(x,y);
    }
  else
    {
      if(ty==1) affri(x,y);else affrr(x,y);
    }
}

void affsi(long s, GEN x)
{
  long lx;
  
  if(!s) {x[1]=2;return;}
  lx=lg(x);if(lx<3) err(affer1);
  if(s>0) {x[1]=evalsigne(1)+evallgef(3);x[2]=s;}
  else {x[1]=evalsigne(-1)+evallgef(3);x[2]= -s;}
}

void affsr(long s, GEN x)
{
  long l,i,d;
  
  if(!s)
    {
      l=(lg(x)-2)<<TWOPOTBITS_IN_LONG;x[1]=HIGHEXPOBIT-l;x[2]=0;
    }
  else
    {
      d=1;if(s<0) {d= -1;s= -s;}
      l=bfffo(s);x[1]=evalexpo((BITS_IN_LONG-1)-l)+evalsigne(d);
      x[2]=(s<<l);for(i=3;i<lg(x);i++) x[i]=0;
    }
}

void affii(GEN x, GEN y)
{
  long lx=lgef(x),i;
  
  if(x==y) return;
  if(lg(y)<lx) err(affer3);
  for(i=1;i<lx;i++) y[i]=x[i];
}

void affir(GEN x, GEN y)
{
  long lx=lgef(x),ly=lg(y),s,s1,i,l,k;
  
  s=signe(x);
  if(!s)
    {
      y[1]=HIGHEXPOBIT-((ly-2)<<TWOPOTBITS_IN_LONG);y[2]=0;
    }
  else
    {
      l=(lx-2)<<TWOPOTBITS_IN_LONG;s1=bfffo(x[2]);
      y[1]=evalsigne(s)+evalexpo(l-s1-1);
      if(s1)
	{
	  if(lx<=ly)
	    {
	      for(i=lx;i<ly;i++) y[i]=0;k=0;
	      for(i=lx-1;i>=2;i--) {y[i]=shiftl(x[i],s1)+k;k=hiremainder;}
	    }
	  else
	    {
	      shiftl(x[ly],s1);k=hiremainder;
	      for(i=ly-1;i>=2;i--) {y[i]=shiftl(x[i],s1)+k;k=hiremainder;}
	    }
	}
      else
	{
	  if(lx<=ly)
	    {
	      for(i=lx;i<ly;i++) y[i]=0;
	      for(i=lx-1;i>=2;i--) y[i]=x[i];
	    }
	  else for(i=ly-1;i>=2;i--) y[i]=x[i];
	}
    }
}

void affrr(GEN x, GEN y)
{
  long lx=lg(x),ly=lg(y),i;
  
  if(x==y) return;
  y[1]=x[1];
  if(!signe(x)) y[2]=0;
  else
    {
      if(lx<=ly)
	{
	  for(i=2;i<lx;i++) y[i]=x[i];
	  for(i=lx;i<ly;i++) y[i]=0;
	}
      else for(i=2;i<ly;i++) y[i]=x[i];
    }
}

GEN shifts(long x, long y)
{
  long t[3];
  
  if(!x) return gzero;
  t[0]=evaltyp(1)+evalpere(1)+evallg(3);
  if(x>0) {t[1]=evalsigne(1)+evallgef(3);t[2]=x;}
  else {t[1]=evalsigne(-1)+evallgef(3);t[2]= -x;}
  return shifti(t,y);
}

GEN shifti(GEN x, long n)
{
  long lx=lgef(x),i,s=signe(x),d,m,p1,p2,k;
  GEN y;
  
  if(!s) return gzero;
  if(!n) return icopy(x);
  if(n>0)
    {
      d=n>>TWOPOTBITS_IN_LONG;m=n&(BITS_IN_LONG-1);
      if(m)
	{
	  p1=shiftl(x[2],m);p2=hiremainder;k=0;
	  if(p2)
	    {
	      y=cgeti(lx+d+1);for(i=lx+1;i<=lx+d;i++) y[i]=0;
	      for(i=lx;i>=4;i--) {y[i]=shiftl(x[i-1],m)+k;k=hiremainder;}
	      y[3]=p1+k;y[2]=p2;
	    }
	  else
	    {
	      y=cgeti(lx+d);for(i=lx;i<lx+d;i++) y[i]=0;
	      for(i=lx-1;i>=3;i--) {y[i]=shiftl(x[i],m)+k;k=hiremainder;}
	      y[2]=p1+k;
	    }
	}
      else
	{
	  y=cgeti(lx+d);for(i=lx;i<lx+d;i++) y[i]=0;
	  for(i=lx-1;i>=2;i--) y[i]=x[i];
	}
    }
  else
    {
      n= -n;d=n>>TWOPOTBITS_IN_LONG;m=n&(BITS_IN_LONG-1);if(lx<d+3) return gzero;
      if(!m)
	{
	  y=cgeti(lx-d);for(i=2;i<lx-d;i++) y[i]=x[i];
	}
      else 
	{
	  m=BITS_IN_LONG-m;d++;
	  p1=shiftl(x[2],m);
	  if(hiremainder)
	    {
	      y=cgeti(lx-d+1);y[2]=hiremainder;
	      for(i=3;i<=lx-d;i++)
		{
		  p2=shiftl(x[i],m);y[i]=p1+hiremainder;p1=p2;
		}
	    }
	  else
	    {
	      if(lx==d+2) return gzero;
	      y=cgeti(lx-d);
	      for(i=3;i<=lx-d;i++) 
		{
		  p2=shiftl(x[i],m);y[i-1]=p1+hiremainder;p1=p2;
		}
	    }
	}
    }   
  y[1]=y[0];setsigne(y,s);return y;
}

GEN shiftr(GEN x, long n)
{
  long l;
  GEN y;
  
  y=rcopy(x);l=expo(x)+n;
  if(l>=HIGHEXPOBIT||l<-HIGHEXPOBIT) err(shier2);
  setexpo(y,l);return y;
}

GEN mptrunc(GEN x)
{
  long e,i,s,lx=lg(x),p1,p2,m;
  ulong d;
  GEN y;
  
  if(typ(x)==1) return icopy(x);
  s=signe(x);if(!s) return gzero;
  e=expo(x);if(e<0) return gzero;
  d=(e>>TWOPOTBITS_IN_LONG)+3;m=e&(BITS_IN_LONG-1);if(d>lx) err(truer2);
  y=cgeti(d);y[1]=evallgef(d)+evalsigne(s);
  if(m==(BITS_IN_LONG-1)) for(i=2;i<d;i++) y[i]=x[i];
  else
    {
      m++;p1=0;
      for(i=2;i<d;i++)
	{
	  p2=shiftl(x[i],m);y[i]=hiremainder+p1;p1=p2;
	}
    }
  return y;
}

GEN mpent(GEN x)
{
  long e,i,lx=lg(x),m,f,p1,p2;
  ulong d;
  GEN y,z;
  
  if(typ(x)==1) return icopy(x);
  if(signe(x)>=0) return mptrunc(x);
  e=expo(x);
  if(e<0) {y=cgeti(3);y[2]=1;y[1]=evalsigne(-1)+evallgef(3);return y;}
  d=(e>>TWOPOTBITS_IN_LONG)+3;m=e&(BITS_IN_LONG-1);if(d>lx) err(truer2);
  y=cgeti(d);y[1]=evalsigne(-1)+evallgef(d);
  if(m==(BITS_IN_LONG-1)) 
    {
      for(i=2;i<d;i++) y[i]=x[i];
      while((i<lx)&&(!x[i])) i++;
      f=(i<lx);
    }    
  else
    {
      m++;p1=0;
      for(i=2;i<d;i++)
	{
	  p2=shiftl(x[i],m);y[i]=hiremainder+p1;p1=p2;
	}
      if(p1) f=1;
      else
	{
	  while((i<lx)&&(!x[i])) i++;
	  f=(i<lx);
	}
    }
  if(f)
    {
      for(i=d-1;(i>=2)&&(y[i]==(long)MAXULONG);i--) y[i]=0;
      if(i>=2) y[i]++;
      else
	{
	  z=y;y=cgeti(1);*y=(*z)+1;y[1]=z[1]+1;
	}
    }
  return y;
}

int mpcmp(GEN x, GEN y)
{
  if(typ(x)==1) return (typ(y)==1) ? cmpii(x,y) : cmpir(x,y);
  return (typ(y)==1) ? -cmpir(y,x) : cmprr(x,y);
}

int cmpsi(long x, GEN y)
{
  ulong p;
  
  if(!x) return -signe(y);
  if(x>0)
    {
      if(signe(y)<=0) return 1;
      if(lgef(y)>3) return -1;
      p=y[2];if(p==x) return 0;
      return (p<(ulong)x) ? 1 : -1;
    }
  else
    {
      if(signe(y)>=0) return -1;
      if(lgef(y)>3) return 1;
      p=y[2];if(p== -x) return 0;
      return (p<(ulong)(-x)) ? -1 : 1;
    }
}

int cmpsr(long x, GEN y)
{
  int p;
  long av;
  GEN z;
  
  if(!x) return -signe(y);
  av=avma;z=cgetr(3);affsr(x,z);
  p=cmprr(z,y);avma=av;return p;
}	

int cmpii(GEN x, GEN y)
{
  long sx=signe(x),sy=signe(y),lx,ly,i;
  
  if(sx<sy) return -1;
  if(sx>sy) return 1;
  if(!sx) return 0;
  lx=lgef(x);ly=lgef(y);
  if(lx>ly) return sx;
  if(lx<ly) return -sx;
  for(i=2;(i<lx)&&(x[i]==y[i]);i++);
  if(i==lx) return 0;
  return ((ulong)x[i]>(ulong)y[i]) ? sx : -sx;
}

int cmpir(GEN x, GEN y)
{
  long av=avma;
  int p;
  GEN z;
  
  if(!signe(x)) return -signe(y);
  z=cgetr(lg(y));affir(x,z);
  p=cmprr(z,y);avma=av;return p;
}

int cmprr(GEN x, GEN y)
{
  long sx=signe(x),sy=signe(y),ex,ey,lx,ly,lz,i;
  
  if(sx<sy) return -1;
  if(sx>sy) return 1;
  if(!sx) return 0;
  ex=expo(x);ey=expo(y);
  if(ex>ey) return sx;
  if(ex<ey) return -sx;
  lx=lg(x);ly=lg(y);lz=(lx<ly)?lx:ly;
  for(i=2;(i<lz)&&(x[i]==y[i]);i++);
  if(i<lz) return ((ulong)x[i]>(ulong)y[i]) ? sx : -sx;
  if(lx>=ly)
    {
      while((i<lx)&(!x[i])) i++;
      return (i==lx) ? 0 : sx;
    }
  else
    {
      while((i<ly)&(!y[i])) i++;
      return (i==ly) ? 0 : -sx;
    }
}    

GEN mpadd(GEN x, GEN y)
{
  if(typ(x)==1) return (typ(y)==1) ? addii(x,y) : addir(x,y);
  return (typ(y)==1) ? addir(y,x) : addrr(x,y);
}

GEN addss(long x, long y)
{
  long t[3];
  
  if(!x) return stoi(y);
  t[0]=evaltyp(1)+evalpere(1)+evallg(3);
  if(x>0) {t[1]=evalsigne(1)+evallgef(3);t[2]=x;} 
  else {t[1]=evalsigne(-1)+evallgef(3);t[2]= -x;}
  return addsi(y,t);
}

GEN addsi(long x, GEN y)
{
  long sx,sy,ly,p,i;
  GEN z;
  
  if(!x) return icopy(y);
  sy=signe(y);if(!sy) return stoi(x);
  if(x<0) {sx= -1;x= -x;} else sx=1;
  ly=lgef(y);
  if(sx==sy)
    {
      p=addll(x,y[ly-1]);
      if(overflow)
	{
	  z=cgeti(ly+1);z[ly]=p;
	  for(i=ly-1;(i>2)&&(y[i-1]==(long)MAXULONG);i--) z[i]=0;
	  if(i>2)
	    {
	      z[i]=y[i-1]+1;i--;while(i>=3) {z[i]=y[i-1];i--;}
	      z[2]=z[1]=z[0]-1;z++;avma+=BYTES_IN_LONG;
	    }
	  else {z[2]=1;z[1]=z[0];}
	}
      else
	{
	  z=cgeti(ly);z[ly-1]=p;for(i=1;i<ly-1;i++) z[i]=y[i];
	}
      setsigne(z,sx);
    }
  else
    {
      if(ly==3)
	{
	  if((ulong)y[2]>(ulong)x)
	    {
	      z=cgeti(3);z[1]=evalsigne(sy)+evallgef(3);z[2]=y[2]-x;return z;
	    }
	  if(y[2]==x) return gzero;
	  z=cgeti(3);z[1]=evalsigne(-sy)+evallgef(3);z[2]=x-y[2];return z;
	}
      p=subll(y[ly-1],x);
      if(overflow)
	{
	  z=cgeti(ly);z[ly-1]=p;
	  for(i=ly-2;!(y[i]);i--) z[i]=(long)MAXULONG;
	  z[i]=y[i]-1;
	  if((i>2)||z[i]) {i--;for(;i>=1;i--) z[i]=y[i];}
	  else
	    {
	      z[2]=z[1]=z[0]-1;z++;avma+=BYTES_IN_LONG;setsigne(z,sy);
	    }
	}
      else
	{
	  z=cgeti(ly);z[ly-1]=p;for(i=1;i<ly-1;i++) z[i]=y[i];
	}    
    }
  return z;
}

GEN addii(GEN x, GEN y)
{
  long sx=signe(x),sy=signe(y),sz,lx=lgef(x),ly=lgef(y),i,j,p;
  GEN z;
  
  if(!sx) return icopy(y);
  if(!sy) return icopy(x);
  if(lx<ly) {z=x;x=y;y=z;sz=sx;sx=sy;sy=sz;sz=lx;lx=ly;ly=sz;}
  if(sx==sy)
    {
      z=cgeti(lx+1);overflow=0;
      for(i=ly-1,j=lx-1;i>=2;i--,j--) z[j+1]=addllx(x[j],y[i]);
      if(overflow)
	{
	  for(;(j>=2)&&(x[j]==(long)MAXULONG);j--) z[j+1]=0;
	  if(j>=2)
	    {
	      z[j+1]=x[j]+1;j--;
	      for(;j>=2;j--) z[j+1]=x[j];
	      z[1]=z[0]-1;z[2]=x[1];z++;avma+=BYTES_IN_LONG;
	    }
	  else {z[2]=1;z[1]=x[1]+1;}
	}
      else
	{
	  for(;j>=2;j--) z[j+1]=x[j];
	  z[1]=z[0]-1;z[2]=x[1];z++;avma+=BYTES_IN_LONG;
	}
    }
  else
    {
      if(lx==ly)
	{
	  setsigne(y,1);setsigne(x,1);p=cmpii(x,y);
	  setsigne(y,sy);setsigne(x,sx);if(!p) return gzero;
	  if(p<0) {z=x;x=y;y=z;sz=sx;sx=sy;sy=sz;}
	}
      z=cgeti(lx);overflow=0;
      for(i=ly-1,j=lx-1;i>=2;i--,j--) z[j]=subllx(x[j],y[i]);
      if(overflow)
	{
	  for(;!(x[j]);j--) z[j]=(long)MAXULONG;
	  z[j]=x[j]-1;j--;
	  for(;j>=2;j--) z[j]=x[j];
	}
      else
	{
	  for(;j>=2;j--) z[j]=x[j];
	}
      if(z[2]) z[1]=x[1];
      else
	{
	  for(j=3;(j<lx)&&(!z[j]);j++);
	  i=j-2;z[i+1]=z[i]=z[0]-i;z+=i;avma+=(i<<TWOPOTBYTES_IN_LONG);
	  setsigne(z,sx);
	}
    }
  return z;
}      

GEN addsr(long x, GEN y)
{
  long p[3];
  
  if(!x) return rcopy(y);
  p[0]=evaltyp(1)+evalpere(1)+evallg(3);
  if(x>0) {p[1]=evalsigne(1)+evallgef(3);p[2]=x;}
  else {p[1]=evalsigne(-1)+evallgef(3);p[2]= -x;}
  return addir(p,y);
}

GEN addir(GEN x, GEN y)
{
  long l,e,ly,av,i,l1;
  GEN z;
  
  if(!signe(x)) return rcopy(y);
  if(!signe(y))
    {
      l=lgef(x)-(expo(y)>>TWOPOTBITS_IN_LONG);if((l<3)||(l>32767)) err(adder3);
      z=cgetr(l);affir(x,z);return z;
    }
  else
    {
      e=expo(y)-expi(x);ly=lg(y);
      if(e>0)
	{
	  l=ly-(e>>TWOPOTBITS_IN_LONG);if(l<=2) return rcopy(y);
	}
      else
	{ 
	  l=ly+((-e)>>TWOPOTBITS_IN_LONG)+1;if(l>32767) err(adder3);
	}
      av=avma;z=cgetr(l);affir(x,z);l1=av-avma;l=l1>>TWOPOTBYTES_IN_LONG;
      z=addrr(z,y);
      for(i=lg(z)-1;i>=0;i--) z[i+l]=z[i];z+=l;avma+=l1;
    }
  return z;
}

GEN addrr(GEN x, GEN y)
{
  long sx=signe(x),sy=signe(y),lx=lg(x),ly=lg(y),lz,ex=expo(x),ey=expo(y),sz;
  long av0=avma,e,l,i,d,m,flag,lp1,lp2,av,k,j,cex,f2;
  GEN z,p1,p2;
  
  if(!sy)
    {
      if(!sx) {e=(ex>=ey)?ex:ey;z=cgetr(3);z[2]=0;z[1]=e+HIGHEXPOBIT;return z;}
      e=ex-ey;
      if(e<=0) {z=cgetr(3);z[2]=0;z[1]=ey+HIGHEXPOBIT;return z;}
      l=(e>>TWOPOTBITS_IN_LONG)+3;if(l>lx) l=lx;z=cgetr(l);
      for(i=1;i<l;i++) z[i]=x[i];return z;
    }
  e=ey-ex;
  if(!sx)
    {
      if(e<=0) {z=cgetr(3);z[2]=0;z[1]=ex+HIGHEXPOBIT;return z;}
      l=(e>>TWOPOTBITS_IN_LONG)+3;if(l>ly) l=ly;z=cgetr(l);
      for(i=1;i<l;i++) z[i]=y[i];return z;
    }
  if(e)
    {
      if(e<0) {z=x;x=y;y=z;lz=lx;lx=ly;ly=lz;ey=ex;e= -e;sz=sx;sx=sy;sy=sz;}
      d=(e>>TWOPOTBITS_IN_LONG);m=e&(BITS_IN_LONG-1);
      if(d>=ly-2) return rcopy(y);
      l=d+lx;
      if(l>=ly)
	{
	  flag=1;p1=cgetr(ly);lp1=ly;lp2=ly-d;
	}
      else
	{
	  flag=0;p1=cgetr(l+1);lp2=lx+1;lp1=l+1;
	}
      av=avma;
      if(m)
	{
	  p2=cgetr(lp2);m=BITS_IN_LONG-m;
	  if(flag) {shiftl(x[lp2-1],m);k=hiremainder;}
	  else k=0;
	  for(i=lp2-1;i>=3;i--) 
	    {
	      p2[i]=shiftl(x[i-1],m)+k;k=hiremainder;
	    }
	  p2[2]=k;
	}
      else p2=x;
    }
  else
    {
      l=(lx>ly)?ly:lx;p1=cgetr(l);av=avma;lp2=lp1=l;flag=2;p2=x;m=0;
    }
  if(sx==sy)
    {
      overflow=0;
      if(m+flag) for(i=lp1-1,j=lp2-1;j>=2;i--,j--) p1[i]=addllx(p2[j],y[i]);
      else 
	{
	  p1[lp1-1]=y[lp1-1];
	  for(i=lp1-2,j=lp2-2;j>=2;i--,j--) p1[i]=addllx(p2[j],y[i]);
	}
      if(overflow)
	{
	  for(;(i>=2)&&(y[i]==(long)MAXULONG);i--) p1[i]=0;
	  if(i>=2) {cex=0;p1[i]=y[i]+1;while(i>=3) {i--;p1[i]=y[i];}}
	  else 
	    {
	      cex=1;k=HIGHBIT;if(ey==(HIGHEXPOBIT-1)) err(adder4);
	      for(i=2;i<lp1;i++) {p1[i]=shiftlr(p1[i],1)+k;k=hiremainder;}
	    }
	}
      else {cex=0;for(;i>=2;i--) p1[i]=y[i];}
      p1[1]=evalsigne(sx)+ey+cex+HIGHEXPOBIT;
      avma=av;return p1;
    }
  else 
    {
      if(!e) 
	{
	  for(i=2;(i<l)&&(p2[i]==y[i]);i++);
	  if(i==l)
	    {
	      e=ex-((l-2)<<TWOPOTBITS_IN_LONG)+HIGHEXPOBIT;if(e<0) err(adder5);
	      if(e>EXPOBITS) err(adder4);
	      avma=av0;z=cgetr(3);z[2]=0;z[1]=e;return z;
	    }
	  else
	    {
	      f2=(((ulong)y[i])>((ulong)p2[i]))?1:0;
	    }
	}
      else f2=1;
      if(f2)
	{
	  overflow=0;
	  if(m+flag) for(i=lp1-1,j=lp2-1;j>=2;i--,j--) p1[i]=subllx(y[i],p2[j]);
	  else 
	    {
	      p1[lp1-1]=y[lp1-1];
	      for(i=lp1-2,j=lp2-2;j>=2;i--,j--) p1[i]=subllx(y[i],p2[j]);
	    }
	  if(overflow)
	    {
	      for(;(i>=2)&&(!y[i]);i--) p1[i]=(long)MAXULONG;
	      p1[i]=y[i]-1;while(i>=3) {i--;p1[i]=y[i];}
	    }
	  else for(;i>=2;i--) p1[i]=y[i];
	}
      else
	{
	  overflow=0;
	  if(m+flag) for(i=lp1-1;i>=2;i--) p1[i]=subllx(p2[i],y[i]);
	  else 
	    {
	      p1[lp1-1]=subllx(0,y[lp1-1]);
	      for(i=lp1-2;i>=2;i--) p1[i]=subllx(p2[i],y[i]);
	    }
	}
      for(i=2;!p1[i];i++);j=i-2;avma=av+(j<<TWOPOTBYTES_IN_LONG);p1[j]=p1[0]-j;p1+=j;
      m=bfffo(p1[2]);e=ey-(j<<TWOPOTBITS_IN_LONG)-m+HIGHEXPOBIT;
      if(e<0) err(adder5);
      p1[1]=f2 ? evalsigne(sy)+e : evalsigne(sx)+e;
      if(m)
	{
	  k=0;for(i=lp1-1-j;i>=2;i--) {p1[i]=shiftl(p1[i],m)+k;k=hiremainder;}
	}
      return p1;
    }
}

void addssz(long x, long y, GEN z)
{
  long av=avma;
  GEN p1;
  
  if(typ(z)==1) gops2ssz(addss,x,y,z);
  else
    {
      p1=cgetr(lg(z));affsr(x,p1);p1=addrs(p1,y);
      affrr(p1,z);avma=av;
    }
}

GEN mpsub(GEN x, GEN y)
{
  if(typ(x)==1) return (typ(y)==1) ? subii(x,y) : subir(x,y);
  return (typ(y)==1) ? subri(x,y) : subrr(x,y);
}

GEN subii(GEN x, GEN y)
{
  long s=signe(y);
  GEN z;
  
  if(x==y) return gzero;
  setsigne(y,-s);z=addii(x,y);setsigne(y,s);
  return z;
}

GEN subrr(GEN x, GEN y)
{
  long s=signe(y);
  GEN z;
  
  if(x==y)
    {
      z=cgetr(3);z[2]=0;z[1]=HIGHEXPOBIT-(lg(x)<<TWOPOTBITS_IN_LONG);return z;
    }
  setsigne(y,-s);z=addrr(x,y);setsigne(y,s);return z;
}

GEN subsi(long x, GEN y)
{
  long s=signe(y);
  GEN z;
  
  setsigne(y,-s);z=addsi(x,y);setsigne(y,s);return z;
}

GEN subsr(long x, GEN y)
{
  long s=signe(y);
  GEN z;
  
  setsigne(y,-s);z=addsr(x,y);setsigne(y,s);return z;
}

GEN subss(long x, long y)
{
  return addss(-y,x);
}


GEN subir(GEN x, GEN y)
{
  long s=signe(y);
  GEN z;
  
  setsigne(y,-s);z=addir(x,y);setsigne(y,s);return z;
}

GEN subri(GEN x, GEN y)
{
  long s=signe(y);
  GEN z;
  
  setsigne(y,-s);z=addir(y,x);setsigne(y,s);return z;
}

void subssz(long x, long y, GEN z)
{
  long av=avma;
  GEN p1;
  
  if(typ(z)==1) gops2ssz(addss,x,-y,z);
  else
    {
      p1=cgetr(lg(z));affsr(x,p1);p1=addrs(p1,-y);
      affrr(p1,z);avma=av;
    }
}

GEN mpmul(GEN x, GEN y)
{
  if(typ(x)==1) return (typ(y)==1) ? mulii(x,y) : mulir(x,y);
  return (typ(y)==1) ? mulir(y,x) : mulrr(x,y);
}

GEN mulss(long x, long y)
{
  long s,p1;
  GEN z;
  
  if((!x)||(!y)) return gzero;
  s=1;if(x<0) {s= -1;x= -x;} if(y<0) {s= -s;y= -y;}
  p1=mulll(x,y);
  if(hiremainder) 
    {
      z=cgeti(4);z[1]=evalsigne(s)+evallgef(4);z[2]=hiremainder;z[3]=p1;
    }
  else {z=cgeti(3);z[1]=evalsigne(s)+evallgef(3);z[2]=p1;}
  return z;
}

GEN mulsr(long x, GEN y)
{
  long lx,i,k,s,p1,p2,e;
  GEN z;
  
  if(!x) return gzero;
  s=signe(y);if(x<0) {s= -s;x= -x;}
  if(!s)
    {
      p1=bfffo(x);e=y[1]+(BITS_IN_LONG-1)-p1;if(e>EXPOBITS) err(muler2);
      z=cgetr(3);z[1]=e;z[2]=0;
    }
  else
    {
      if(x==1) {z=rcopy(y);setsigne(z,s);return z;}
      lx=lg(y);z=cgetr(lx);
      p2=mulll(x,y[lx-1]);
      for(i=lx-2;i>=2;i--) z[i+1]=addmul(x,y[i]);
      z[2]=hiremainder;p1=bfffo(hiremainder);
      if(p1)
	{
	  shiftl(p2,p1);k=hiremainder;
	  for(i=lx-1;i>=2;i--)
	    {
	      z[i]=shiftl(z[i],p1)+k;k=hiremainder;
	    }
	}
      e=BITS_IN_LONG-p1+expo(y);if(e>=HIGHEXPOBIT) err(muler2);
      z[1]=evalsigne(s)+evalexpo(e);
    }  
  return z;
}

GEN mulii(GEN x, GEN y)
{
  long i,j,lx=lgef(x),ly=lgef(y),sx,sy,lz,p1,p2;
  GEN z;
  
  sx=signe(x);if(!sx) return gzero;
  sy=signe(y);if(!sy) return gzero;
  if(sy<0) sx= -sx;
  if(lx>ly) {z=x;x=y;y=z;lz=lx;lx=ly;ly=lz;}
  lz=lx+ly-2;if(lz>LGBITS) err(muler1);
  z=cgeti(lz);z[1]=evallgef(lz)+evalsigne(sx);
  p1=x[lx-1];hiremainder=0;
  for(i=ly-1;i>=2;i--) z[lx+i-2]=addmul(p1,y[i]);
  z[lx-1]=hiremainder;
  for(j=lx-2;j>=2;j--)
    {
      p1=x[j];hiremainder=0;
      for(i=ly-1;i>=2;i--)
	{
	  p2=addmul(p1,y[i]);
	  z[i+j-1]=addll(p2,z[i+j-1]);hiremainder+=overflow;
	}
      z[j]=hiremainder;
    }
  if(!(z[2]))
    {
      z[2]=z[1]-1;z[1]=z[0]-1;z++;avma+=BYTES_IN_LONG;
    }
  return z;
}

GEN mulrr(GEN x, GEN y)
{
  long i,j,lx=lg(x),ly=lg(y),sx=signe(x),sy=signe(y),ex=expo(x),ey=expo(y);
  long e,flag,garde,p1,p2,lz;
  GEN z;
  
  e=ex+ey+HIGHEXPOBIT;if(e>=EXPOBITS) err(muler4);
  if(e<0) err(muler5);
  if((!sx)||(!sy)) {z=cgetr(3);z[2]=0;z[1]=e;return z;}
  if(sy<0) sx= -sx;
  if(lx>ly) {lz=ly;z=x;x=y;y=z;flag=1;} else {lz=lx;flag=(lx!=ly);}
  z=cgetr(lz);z[1]=evalsigne(sx)+e;
  if(flag) mulll(x[2],y[lz]);else hiremainder=0;
  if(lz==3)
    {
      garde=flag ? addmul(x[2],y[2]) : mulll(x[2],y[2]);
      if((long)hiremainder<0) {z[2]=hiremainder;z[1]++;}
      else {z[2]=(garde<0)?(hiremainder<<1)+1:(hiremainder<<1);}
      return z;
    }
  p1=x[lz-1];garde=hiremainder;
  if(p1)
    {
      mulll(p1,y[3]);p2=addmul(p1,y[2]);
      garde=addll(p2,garde);z[lz-1]=overflow+hiremainder;
    }
  else z[lz-1]=0;
  for(j=lz-2;j>=3;j--)
    {
      p1=x[j];
      if(p1)
	{
	  mulll(p1,y[lz+2-j]);
	  p2=addmul(p1,y[lz+1-j]);
	  garde=addll(p2,garde);hiremainder+=overflow;
	  for(i=lz-j;i>=2;i--)
	    {
	      p2=addmul(p1,y[i]);
	      z[i+j-1]=addll(p2,z[i+j-1]);hiremainder+=overflow;
	    }
	  z[j]=hiremainder;
	}
      else z[j]=0;
    }
  p1=x[2];p2=mulll(p1,y[lz-1]);
  garde=addll(p2,garde);hiremainder+=overflow;
  for(i=lz-2;i>=2;i--)
    {
      p2=addmul(p1,y[i]);
      z[i+1]=addll(p2,z[i+1]);hiremainder+=overflow;
    }
  z[2]=hiremainder;
  if((long)hiremainder>0)
    {
      overflow=(garde<0)?1:0;
      for(i=lz-1;i>=2;i--) {p1=z[i];z[i]=addllx(p1,p1);}
    }
  else z[1]++;
  return z;
}

GEN mulir(GEN x, GEN y)
{
  long sx=signe(x),sy,av,lz,ey,e,garde,p1,p2,i,j;
  GEN z,temp;
  
  if(!sx) return gzero;
  sy=signe(y);ey=expo(y);
  if(!sy)
    {
      z=cgetr(3);z[2]=0;e=expi(x)+ey+HIGHEXPOBIT;if(e>EXPOBITS) err(muler6);
      z[1]=e;return z;
    }
  lz=lg(y);if(sy<0) sx= -sx;
  z=cgetr(lz);av=avma;
  temp=cgetr(lz+1);affir(x,temp);x=y;y=temp;
  e=expo(y)+ey+HIGHEXPOBIT;if(e>=EXPOBITS) err(muler4);
  if(e<0) err(muler5);
  z[1]=evalsigne(sx)+e;
  mulll(x[2],y[lz]);
  if(lz==3)
    {
      garde=addmul(x[2],y[2]);
      if((long)hiremainder<0) {z[2]=hiremainder;z[1]++;}
      else {z[2]=(garde<0)?(hiremainder<<1)+1:(hiremainder<<1);}
      avma=av;return z;
    }
  garde=hiremainder;
  p1=x[lz-1];mulll(p1,y[3]);p2=addmul(p1,y[2]);
  garde=addll(p2,garde);z[lz-1]=overflow+hiremainder;
  for(j=lz-2;j>=3;j--)
    {
      p1=x[j];mulll(p1,y[lz+2-j]);
      p2=addmul(p1,y[lz+1-j]);
      garde=addll(p2,garde);hiremainder+=overflow;
      for(i=lz-j;i>=2;i--)
	{
	  p2=addmul(p1,y[i]);
	  z[i+j-1]=addll(p2,z[i+j-1]);hiremainder+=overflow;
	}
      z[j]=hiremainder;
    }
  p1=x[2];p2=mulll(p1,y[lz-1]);
  garde=addll(p2,garde);hiremainder+=overflow;
  for(i=lz-2;i>=2;i--)
    {
      p2=addmul(p1,y[i]);
      z[i+1]=addll(p2,z[i+1]);hiremainder+=overflow;
    }
  z[2]=hiremainder;
  if((long)hiremainder>0)
    {
      overflow=(garde<0)?1:0;
      for(i=lz-1;i>=2;i--) {p1=z[i];z[i]=addllx(p1,p1);}
    }
  else z[1]++;
  avma=av;return z;
}

void mulssz(long x, long y, GEN z)
{
  long av=avma;
  GEN p1;
  
  if(typ(z)==1) gops2ssz(mulss,x,y,z);
  else
    {
      p1=cgetr(lg(z));affsr(x,p1);p1=mulsr(y,p1);
      mpaff(p1,z);avma=av;
    }
}

GEN convi(GEN x)
{
  long lx,av=avma,lz;
  GEN z,p1,p2;  
  
  if(!signe(x))
    {
      z=cgeti(3);z[1]= -1;z[2]=0;avma=av;return z+3;
    }
  p1=absi(x);lx=lgef(p1);lz=((lx-2)*15)/14+3;z=cgeti(lz);z[1]= -1;
  for(p2=z+2;signe(p1);p2++) *p2=divisii(p1,1000000000,p1);
  avma=av;return p2;
}

GEN confrac(GEN x)
{
  long lx=lg(x),ex= -expo(x)-1,ex1,av=avma,ly,ey;
  long lr,nbdec,k,i,j;
  GEN y,res;
  
  ey=((lx-2)<<TWOPOTBITS_IN_LONG)+ex;
  ly=(ey+(2*BITS_IN_LONG-1))>>TWOPOTBITS_IN_LONG;
  y=cgeti(ly);
  ex1=ex>>TWOPOTBITS_IN_LONG; /* 95 dans mp.s faux? */
  for(i=0;i<ex1;i++) y[i]=0;
  ex&=(BITS_IN_LONG-1);
  if(!ex) for(j=2;j<lx;j++) y[i++]=x[j];
  else
    {
      k=0;
      for(j=2;j<lx;j++) {y[i++]=shiftlr(x[j],ex)+k;k=hiremainder;}
      y[ly-2]=k;
    }
  y[ly-1]=0;
  nbdec=ey*L2SL10+1;lr=(nbdec+17)/9;res=cgeti(lr);
  *res=nbdec;
  for(j=1;j<lr;j++)
    {
      hiremainder=0;
      for(i=ly-1;i>=0;i--) y[i]=addmul(y[i],1000000000);
      res[j]=hiremainder;
    }
  avma=av;return res;
}

void mulsii(long x, GEN y, GEN z)
{
  long av=avma;
  GEN p1;
  
  p1=mulsi(x,y);affii(p1,z);avma=av;
}

void addsii(long x, GEN y, GEN z)
{
  long av=avma;
  GEN p1;
  
  p1=addsi(x,y);affii(p1,z);avma=av;
}

long divisii(GEN x, long y, GEN z)
{
  long av=avma,k;
  GEN p1;
  
  p1=divis(x,y);affii(p1,z);avma=av;
  k=hiremainder;return k;
}


/*
static int tv[37]={0, 0, 25, 1, 22, 26, 31, 2, 15, 23, 29, 27, 10, 0, 12, 3, 6, 16, 0, 24, 21, 30, 14, 28, 9, 11, 5, 0, 20, 13, 8, 4, 19, 7, 18, 17, 0};

#define vals(x) (x?tv[(x^(x-1))%37]:-1)
*/

long vals(long x)
{
  unsigned long y,z;
  long s;

  if(!x) return -1;
  s = 0;
#ifdef LONG_IS_64BIT
  z=x&0xffffffff;if(!z) {s+=32;z=((ulong)x)>>32;}
#else
  z=(ulong)x;
#endif
  y=z&0xffff;if(!y) {s+=16;y=z>>16;}
  z=y&0xff;if(!z) {s+=8;z=y>>8;}
  y=z&0xf;if(!y) {s+=4;y=z>>4;}
  z=y&0x3;if(!z) {s+=2;z=y>>2;}
  return (z&0x1) ? s : s+1;
}

long vali(GEN x)
{
  long i,lx=lgef(x);
  
  if(!signe(x)) return -1;
  for(i=lx-1;(i>=2)&&(!x[i]);i--);
  return ((lx-1-i)<<TWOPOTBITS_IN_LONG)+vals(x[i]);
}

GEN mpdiv(GEN x, GEN y)
{
  if(typ(x)==1) return (typ(y)==1) ? divii(x,y) : divir(x,y);
  return (typ(y)==1) ? divri(x,y) : divrr(x,y);
}

GEN divss(long x, long y)
{
  long p1;
  
  if(!y) err(diver1);
  hiremainder=0;p1=divll((ulong)labs(x),(ulong)labs(y));
  if(y<0) {hiremainder= -((long)hiremainder);p1= -p1;}
  if(x<0) p1= -p1;
  return stoi(p1);
}

GEN modss(long x, long y)
{
  long y1;
  
  if(!y) err(moder1);
  hiremainder=0;divll(labs(x),y1=labs(y));
  if(!hiremainder) return gzero;
  return (((long)hiremainder)<0) ? stoi(y1-hiremainder) : stoi(hiremainder);
}

GEN resss(long x, long y)
{
  if(!y) err(reser1);
  hiremainder=0;divll(labs(x),labs(y));
  return (y<0) ? stoi(-((long)hiremainder)) : stoi(hiremainder);
}

GEN dvmdss(long x, long y, GEN *z)
{
  GEN p1;

  p1=divss(x,y);*z=stoi(hiremainder);
  return p1;
}

void dvmdssz(long x, long y, GEN z, GEN t)
{
  long av=avma;
  GEN p1;

  p1=divss(x,y);affsi(hiremainder,t);
  mpaff(p1,z);avma=av;
}

GEN dvmdsi(long x, GEN y, GEN *z)
{
  GEN p1;
  p1=divsi(x,y);*z=stoi(hiremainder);
  return p1;
}

void dvmdsiz(long x, GEN y, GEN z, GEN t)
{
  long av=avma;
  GEN p1;
  
  p1=divsi(x,y);affsi(hiremainder,t);
  mpaff(p1,z);avma=av;
}

GEN dvmdis(GEN x, long y, GEN *z)
{
  GEN p1;
  p1=divis(x,y);*z=stoi(hiremainder);
  return p1;
}

void dvmdisz(GEN x, long y, GEN z, GEN t)
{
  long av=avma;
  GEN p1;
  
  p1=divis(x,y);affsi(hiremainder,t);
  mpaff(p1,z);avma=av;
}

void dvmdiiz(GEN x, GEN y, GEN z, GEN t)
{
  long av=avma;
  GEN p1,p2;

  p1=dvmdii(x,y,&p2);mpaff(p1,z);mpaff(p2,t);
  avma=av;
}

GEN divsi(long x, GEN y)
{
  long s=signe(y),ly=lgef(y),p1;

  if(!s) err(diver2);
  if((!x)||(ly>3)||(y[2]<0)) {hiremainder=x;return gzero;}
  hiremainder=0;p1=divll(labs(x),y[2]);
  if(signe(y)<0) {hiremainder= -((long)hiremainder);p1= -p1;}
  if(x<0) p1= -p1;
  return stoi(p1);
}

GEN ressi(long x, GEN y)
{
  divsi(x,y);return stoi(hiremainder);
}

GEN modsi(long x, GEN y)
{
  long s;
  GEN p1;
  
  divsi(x,y);
  if(!hiremainder) return gzero;
  if(x>0) return stoi(hiremainder);
  else
    {
      s=signe(y);setsigne(y,1);p1=addsi(hiremainder,y);
      setsigne(y,s);return p1;
    }
}

GEN divis(GEN y, long x)
{
  long s=signe(y),ly=lgef(y),i,d;
  GEN z;
  
  if(!x) err(diver4);
  if(!s) {hiremainder=0;return gzero;}
  if(x<0) {s= -s;x= -x;} 
  if((ulong)x>(ulong)y[2])
    {
      if(ly==3) {hiremainder=itos(y);return gzero;}
      else 
	{
	  z=cgeti(ly-1);z[1]=evallgef(ly-1)+evalsigne(s);
	  d=1;hiremainder=y[2];
	}
    }
  else {z=cgeti(ly);z[1]=evallgef(ly)+evalsigne(s);d=0;hiremainder=0;}
  for(i=d+2;i<ly;i++) z[i-d]=divll(y[i],x);
  if(s<0) hiremainder= -((long)hiremainder);
  return z;
}

GEN modis(GEN x, long y)
{
  divis(x,y);if(!hiremainder) return gzero;
  return (signe(x)>0) ? stoi(hiremainder) : stoi(labs(y)+hiremainder);
}

GEN resis(GEN x, long y)
{
  divis(x,y);return stoi(hiremainder);
}
     
void divisz(GEN x, long y, GEN z)
{
  long av=avma;
  GEN p1;
  
  if(typ(z)==1) gops2gsz(divis,x,y,z);
  else
    {
      p1=cgetr(lg(z));affir(x,p1);p1=divrs(p1,y);
      affrr(p1,z);avma=av;
    }
}

void divsiz(long x, GEN y, GEN z)
{
  long av=avma,lz;
  GEN p1,p2;
  
  if(typ(z)==1) gops2sgz(divsi,x,y,z);
  else
    {
      lz=lg(z);p1=cgetr(lz);p2=cgetr(lz);affsr(x,p1);affir(y,p2);
      p1=divrr(p1,p2);affrr(p1,z);avma=av;
    }
}

void divssz(long x, long y, GEN z)
{
  long av=avma;
  GEN p1;
  
  if(typ(z)==1) gops2ssz(divss,x,y,z);
  else
    {
      p1=cgetr(lg(z));affsr(x,p1);p1=divrs(p1,y);
      affrr(p1,z);avma=av;
    }
}

GEN divir(GEN x, GEN y)
{
  GEN xr,z;
  long av,ly;
  
  if(!signe(y)) err(diver5);
  if(!signe(x)) return gzero;
  ly=lg(y);z=cgetr(ly);av=avma;affir(x,xr=cgetr(ly+1));
  xr=divrr(xr,y);affrr(xr,z);avma=av;return z;
}

GEN divri(GEN x, GEN y)
{
  GEN yr,z;
  long av,lx,ex,s=signe(y);

  if(!s) err(diver8);
  if(!signe(x))
  {
    ex=expo(x)-expi(y)+HIGHEXPOBIT;
    if(ex<0) err(diver12);
    z=cgetr(3);z[1]=ex;z[2]=0;return z;
  }
  if((lg(y)==3)&&(y[2]>0)) return (s>0) ? divrs(x,y[2]) : divrs(x,-y[2]);
  lx=lg(x);z=cgetr(lx);av=avma;affir(y,yr=cgetr(lx+1));
  yr=divrr(x,yr);affrr(yr,z);avma=av;return z;
}

void diviiz(GEN x, GEN y, GEN z)
{
  long av=avma,lz;
  GEN p1,p2;
  
  if(typ(z)==1) {p1=divii(x,y);affii(p1,z);avma=av;}
  else
    {
      lz=lg(z);p1=cgetr(lz);p2=cgetr(lz);affir(x,p1);affir(y,p2);
      p1=divrr(p1,p2);affrr(p1,z);avma=av;
    }
}
     
void divrrz(GEN x, GEN y, GEN z)
{
  long av=avma;
  GEN p1;

  p1=divrr(x,y);mpaff(p1,z);avma=av;
}

void mpdivz(GEN x, GEN y, GEN z)
{
  long av=avma,lz;
  GEN p1,p2;

  if(typ(z)==1)
    {
      if(typ(x)==2||typ(y)==2) err(divzer1);
      p1=divii(x,y);affii(p1,z);avma=av;
    }
  else
    {
      if(typ(x)==1)
	{
	  if(typ(y)==2) {p1=divir(x,y);mpaff(p1,z);avma=av;}
	  else
	    {
	      lz=lg(z);p1=cgetr(lz);p2=cgetr(lz);affir(x,p1);affir(y,p2);
	      p1=divrr(p1,p2);affrr(p1,z);avma=av;
	    }
	}
      else
	{
	  if(typ(y)==2) {p1=divrr(x,y);affrr(p1,z);avma=av;}
	  else {p1=divri(x,y);affrr(p1,z);avma=av;}
	}
    }
}

GEN divsr(long x, GEN y)
{
  long av,ly;
  GEN p1,z;

  if(!signe(y)) err(diver3);
  if(!x) return gzero;
  ly=lg(y);z=cgetr(ly);av=avma;p1=cgetr(ly+1);affsr(x,p1);p1=divrr(p1,y);
  affrr(p1,z);avma=av;return z;
}

GEN modii(GEN x, GEN y)
{
  long av=avma,tetpil;
  GEN p1;

  p1=dvmdii(x,y,(GEN *)-1);
  if(signe(p1)>=0) return p1;
  tetpil=avma;p1=(signe(y)>0) ? addii(p1,y) : subii(p1,y);
  return gerepile(av,tetpil,p1);
}

void modiiz(GEN x, GEN y, GEN z)
{
  long av=avma;
  GEN p1;

  p1=modii(x,y);affii(p1,z);avma=av;
}

void resiiz(GEN x, GEN y, GEN z)
{
  long av=avma;
  GEN p1;

  p1=resii(x,y);affii(p1,z);avma=av;
}

GEN divrs(GEN x, long y)
{
  long i,k,lx,ex,garde,sh,s=signe(x);
  GEN z;

  if(!y) err(diver6);
  if(!s)
    {
      z=cgetr(3);z[2]=0;z[1]=x[1]-(BITS_IN_LONG-1)+bfffo(y);
      if(z[1]<0) err(diver7);return z;
    }
  if(y<0) {s= -s;y= -y;}
  if(y==1) {z=rcopy(x);setsigne(z,s);return z;}
  z=cgetr(lx=lg(x));hiremainder=0;
  for(i=2;i<lx;i++) z[i]=divll(x[i],y);
  garde=divll(0,y);sh=bfffo(z[2]);ex=expo(x)-sh;
  if((-ex)>HIGHEXPOBIT) err(diver7);
  z[1]=evalsigne(s)+evalexpo(ex);shiftl(garde,sh);k=hiremainder;
  for(i=lx-1;i>=2;i--) {z[i]=shiftl(z[i],sh)+k;k=hiremainder;}
  return z;
}

int mpdivis(GEN x, GEN y, GEN z)
{
  long av=avma;
  GEN p1,p2;

  p1=dvmdii(x,y,&p2);
  if(signe(p2)) {avma=av;return 0;}
  affii(p1,z);avma=av;return 1;
}

int divise(GEN x, GEN y)
{
  long av=avma;
  GEN p1;

  p1=dvmdii(x,y,(GEN *)-1);avma=av;
  return signe(p1) ? 0 : 1;
}


GEN dvmdii(GEN x, GEN y, GEN *z)
{
  long av,av2,lx,ly,lz,lp3,i,j,dec,sh,k,k1,sx=signe(x),sy=signe(y);
  long saux,k3,k4,av1,flk4;
  ulong si,qp;
  GEN p1,p2,p3,p4;
  
  if(!sy) err(dvmer1);
  if(!sx)
    {
      if(((long)z==(long)MAXULONG)||((long)z==0)) return gzero;
      *z=gzero;return gzero;
    }
  lx=lgef(x);ly=lgef(y);lz=lx-ly;
  if(lz<0)
    {
      if((long)z==(long)MAXULONG) return icopy(x);
      if(z==0) return gzero;
      *z=icopy(x);return gzero;
    }
  av=avma;if(sx<0) sy= -sy;
  if(ly==3)
    {
      si=y[2];
      if(si>(ulong)x[2]) {p1=cgeti(lx-1);hiremainder=x[2];dec=1;}
      else {p1=cgeti(lx);hiremainder=0;dec=0;}
      for(i=2+dec;i<lx;i++) p1[i-dec]=divll(x[i],si);
      if((long)z==(long)MAXULONG)
	{
	  avma=av;if(!hiremainder) return gzero;
	  p2=cgeti(3);p2[1]=evalsigne(sx)+evallgef(3);p2[2]=hiremainder;return p2;
	}
      if(lx!=(dec+2)) {p1[1]=p1[0];setsigne(p1,sy);} else {avma=av;p1=gzero;}
      if(z==0) return p1;
      if(!hiremainder) *z=gzero;
      else {p2=cgeti(3);p2[1]=evalsigne(sx)+evallgef(3);p2[2]=hiremainder;*z=p2;}
      return p1;
    }
  else
    {
      p1=cgeti(lx);
      sh=bfffo(y[2]);
      if(sh)
	{
	  p2=cgeti(ly);k=shiftl(y[2],sh);
	  for(i=3;i<ly;i++) 
	    {
	      k1=shiftl(y[i],sh);p2[i-1]=k+hiremainder;k=k1;
	    }
	  p2[ly-1]=k;k=0;
	  for(i=2;i<lx;i++)
	    {
	      k1=shiftl(x[i],sh);p1[i-1]=k+hiremainder;k=k1;
	    }
	  p1[lx-1]=k;
	}
      else {p1[1]=0;for(j=2;j<lx;j++) p1[j]=x[j];p2=y;}
      si=p2[2];saux=p2[3];
      for(i=1;i<=lz+1;i++)
	{
	  if(p1[i]==si) 
	    {
	      qp=(long)MAXULONG;k=addll(si,p1[i+1]);
	    }
	  else
	    {
	      hiremainder=p1[i];qp=divll(p1[i+1],si);
	      overflow=0;k=hiremainder;
	    }
	  if(!overflow)
	    {
	      k1=mulll(qp,saux);k3=subll(k1,p1[i+2]);k+=overflow;
	      flk4=((ulong)hiremainder>(ulong)k);k4=subll(hiremainder,k);
	      while(flk4) 
		{
		  qp--;k3=subll(k3,saux);k4-=overflow;
		  flk4=((ulong)k4>(ulong)si);k4=subll(k4,si);
		}
	    }
	  hiremainder=0;
	  for(j=ly-1;j>=2;j--)
	    {
	      k1=addmul(qp,p2[j]);
	      p1[i+j-1]=subll(p1[i+j-1],k1);hiremainder+=overflow;
	    }
	  if((ulong)p1[i]<(ulong)hiremainder)
	    {
	      overflow=0;qp--;
	      for(j=ly-1;j>=2;j--) p1[i+j-1]=addllx(p1[i+j-1],p2[j]);
	    }
	  p1[i]=qp;
	}
      av1=avma;
      if((long)z!=(long)MAXULONG)
	{
	  if(p1[1])
	    {
	      p3=cgeti(lp3=lz+3);for(j=2;j<lp3;j++) p3[j]=p1[j-1];
	    }
	  else
	    {
	      p3=cgeti(lp3=lz+2);
	      if(!lz) sy=0;else for(j=2;j<lp3;j++) p3[j]=p1[j];
	    }
	  if(lp3<3) p3[1]=evallgef(2);else {p3[1]=evallgef(lp3)+evalsigne(sy);}
	}
      if(z!=0)
	{
	  for(j=lz+2;(j<lx)&&(!p1[j]);j++);
	  if(j==lx) p4=gzero;
	  else
	    {
	      p4=cgeti(lp3=lx-j+2);p4[1]=p4[0];
	      if(!sh) for(i=2;j<lx;j++,i++) p4[i]=p1[j];
	      else
		{
		  hiremainder=0;k1=shiftlr(p1[j++],sh);k=hiremainder;
		  if(k1) {p4[2]=k1;dec=1;} 
		  else 
		    {
		      p4[1]=p4[0]-1;p4++;avma+=BYTES_IN_LONG;
		      p4[1]=p4[0];dec=0;
		    }
		  for(i=2+dec;j<lx;j++,i++)
		    {
		      p4[i]=shiftlr(p1[j],sh)+k;k=hiremainder;
		    }
		}
	      setsigne(p4,sx);
	    }
	}
      if((long)z==(long)MAXULONG) return gerepile(av,av1,p4);
      if((long)z==0) return gerepile(av,av1,p3);
      av2=avma;dec=lpile(av,av1,0)>>TWOPOTBYTES_IN_LONG;
      *z=adecaler(p4,av1,av2)?p4+dec:p4;
      return adecaler(p3,av1,av2)?p3+dec:p3;
    }
}

GEN divrr(GEN x, GEN y)
{
  long sx=signe(x),sy=signe(y),lx,ly,lz,ex,ex1,i,z0;
  long ldif,y0,y1,si,saux,qp,k,k1,k3,k4,j,flk4;
  GEN z;
  
  if(!sy) err(diver9);
  ex=expo(x)-expo(y)+HIGHEXPOBIT;
  if(ex<=0) err(diver10);
  if(ex>EXPOBITS) err(diver11);
  if(!sx)
    {
      z=cgetr(3);z[1]=ex;z[2]=0;return z;
    }
  lx=lg(x);ly=lg(y);lz=(lx<=ly)?lx:ly;
  z=cgetr(lz);if(sy<0) sx= -sx;
  ex1=evalsigne(sx)+ex;
  if(ly==3)
    {
      i=x[2];si=(lx>3)?x[3]:0;
      if((ulong)i<(ulong)y[2])
	{
	  hiremainder=i;z[2]=divll(si,y[2]);
	  z[1]=ex1-1;return z;
	}
      else
	{
	  hiremainder=((ulong)i)>>1;
	  z[2]=(i&1)?divll((((ulong)si)>>1)|(HIGHBIT),y[2]):divll(((ulong)si)>>1,y[2]);
	  z[1]=ex1;return z;
	}
    }
  z0= *z;*z=0;
  for(i=2;i<=lz-1;i++) z[i-1]=x[i];
  z[lz-1]=(lx>lz) ? x[lz] : 0;
  ldif=ly-lz;if(!ldif) {y0=y[lz];y[lz]=0;}
  if(ldif<=1) {y1=y[lz+1];y[lz+1]=0;}
  si=y[2];saux=y[3];
  for(i=0;i<lz-1;i++)
    {
      if(z[i]==si) 
	{
	  qp=(long)MAXULONG;k=addll(si,z[i+1]);
	}
      else
	{
	  hiremainder=z[i];qp=divll(z[i+1],si);
	  overflow=0;k=hiremainder;
	}
      if(!overflow)
	{
	  k1=mulll(qp,saux);k3=subll(k1,z[i+2]);k+=overflow;
	  flk4=((ulong)hiremainder>(ulong)k);k4=subll(hiremainder,k);
	  while(flk4) {qp--;k3=subll(k3,saux);k4-=overflow;flk4=((ulong)k4>(ulong)si);k4=subll(k4,si);}
	}
      mulll(qp,y[lz+1-i]);
      for(j=lz-i;j>=2;j--)
	{
	  k1=addmul(qp,y[j]);
	  z[i+j-1]=subll(z[i+j-1],k1);hiremainder+=overflow;
	}
      if((ulong)z[i]<(ulong)hiremainder)
	{
	  overflow=0;qp--;
	  for(j=lz-i;j>=2;j--) z[i+j-1]=addllx(z[i+j-1],y[j]);
	}
      z[i]=qp;
    }
  if(!ldif) y[lz]=y0;if(ldif<=1) y[lz+1]=y1;
  for(j=lz-1;j>=2;j--) z[j]=z[j-1];
  if(*z)
    {
      k=HIGHBIT;
      for(j=2;j<lz;j++) {z[j]=shiftlr(z[j],1)+k;k=hiremainder;}
    }
  else ex1--;
  z[1]=ex1;*z=z0;return z;
}

double rtodbl(GEN x)
{
  double y,ma;
#ifdef LONG_IS_64BIT
  double co=64.0;
#else
  double co=32.0;
#endif
  long ex,s=signe(x);
  
  if((!s)||((ex=expo(x))< -1023)) return 0.0;
  if(ex>=0x3ff) err(rtodber);
  ma=(ulong)x[2]+(ulong)x[3]/exp2(co);
  y=exp2(ex+1-co);
  return (s<0) ? -y*ma : y*ma;
}

GEN dbltor(double x)
{
  GEN z;

#ifdef LONG_IS_64BIT
  double co=64.0;
#else
  double co=32.0;
#endif
  ulong y;
  long ex;
  int n;
  
  if(x==0) {z=cgetr(3);z[2]=0;z[1]=HIGHEXPOBIT-308;return z;}
  z=cgetr(4);ex=floor(log2(x));
  if(x<0) {z[1]=evalexpo(ex)+evalsigne(-1);x= -x;}
  else z[1]=evalexpo(ex)+evalsigne(1);
  x=x*exp2(-ex+co-1);y=x;x-=y;z[2]=y;
  y=exp2(co)*x;z[3]=y;

  if(!(z[2]&HIGHBIT))
    {
      n=bfffo(z[2]);z[2]=shiftl(z[2],n);z[3]=shiftl(z[3],n);
      z[2]+=hiremainder;setexpo(z,ex-n);
    }
  return z;
}

double  gtodouble(GEN x)
{
  GEN     x1;
  long  t=typ(x);
  static long reel4[4]={evaltyp(2)+evalpere(1)+evallg(4),0,0,0};
  
  if (t==2) x1=x;
  else gaffect(x,x1=(GEN)reel4);
  return rtodbl(x1);
}
  
GEN gerepile(long l, long p, GEN q)
{
  long av,declg,tl;
  GEN ll,pp,l1,l2,l3;

  declg=l-p;if(declg<=0) return q;
  for(ll=(GEN)l,pp=(GEN)p;pp>(GEN)avma;) *--ll= *--pp;
  av=(long)ll;
  while((ll<(GEN)l)||((ll==(GEN)l)&&(long)q))
  {
    l2=ll+lontyp[tl=typ(ll)];
    if(tl==10) {l3=ll+lgef(ll);ll+=lg(ll);if(l3>ll) l3=l2;}
    else {ll+=lg(ll);l3=ll;} 
    for(;l2<l3;l2++) 
      {
	l1=(GEN)(*l2);
	if((l1<(GEN)l)&&(l1>=(GEN)avma))
	  {
	    if(l1<(GEN)p) *l2+=declg;
	    else
	      if(ll<=(GEN)l) err(gerper);
	  }
      }
  }
  if((!((long)q))||((q<(GEN)p)&&(q>=(GEN)avma)))
  {
    avma=av;return q+(declg>>TWOPOTBYTES_IN_LONG);
  }
  else {avma=av;return q;}
}

void cgiv(GEN x)
{
  long p;

  if((p=pere(x))==MAXUBYTE) return;
  if((x!=(GEN)avma)||(p>1)) {setpere(x,p-1);return;}
  do x+=lg(x);while(!pere(x));
  avma=(long)x;
  return;
}

