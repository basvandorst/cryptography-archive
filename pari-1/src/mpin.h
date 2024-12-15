/* mp.c ou mp.s */

GEN     gerepile(long l, long p, GEN q), icopy(GEN x), rcopy(GEN x);
GEN     mptrunc(GEN x),mpent(GEN x),shifts(long x, long y),shifti(GEN x, long n);
GEN     addsi(long x, GEN y),addsr(long x, GEN y),addii(GEN x, GEN y),addir(GEN x, GEN y),addrr(GEN x, GEN y), addss(long x, long y);
GEN     mulss(long x, long y),mulsi(long x, GEN y),mulsr(long x, GEN y),mulii(GEN x, GEN y),mulir(GEN x, GEN y),mulrr(GEN x, GEN y);
GEN     divsi(long x, GEN y),divis(GEN y, long x),divsr(long x, GEN y),divrs(GEN x, long y),divir(GEN x, GEN y);
GEN     divri(GEN x, GEN y),divrr(GEN x, GEN y),convi(GEN x),confrac(GEN x);
GEN     modss(long x, long y),resss(long x, long y),modsi(long x, GEN y),modii(GEN x, GEN y);
GEN     dvmdii(GEN x, GEN y, GEN *z),dvmdsi(long x, GEN y, GEN *z),dvmdis(GEN x, long y, GEN *z);
long    vals(long x);
int     cmpss(long x, long y),cmpsi(long x, GEN y),cmpii(GEN x, GEN y),cmprr(GEN x, GEN y);
void    affir(GEN x, GEN y),affrr(GEN x, GEN y);
void    diviiz(GEN x, GEN y, GEN z),cgiv(GEN x);
void    mpdivz(GEN x, GEN y, GEN z),modiiz(GEN x, GEN y, GEN z);
void    modiiz(GEN x, GEN y, GEN z);

inline long mulmodll(ulong a, ulong b, ulong c)
{
  divll(mulll(a,b),c);return hiremainder;
}

inline GEN cgeti(long x)
{
  unsigned long p1;
  GEN z;
  
  p1=avma-(((unsigned short)x)<<2);
  if(p1<bot) err(errpile);
  avma=p1;z=(GEN)p1;z[0]=0x1010000+x;
  return z;
}

inline GEN cgetr(long x)
{
  unsigned long p1;
  GEN z;
  
  p1=avma-(((unsigned short)x)<<2);
  if(p1<bot) err(errpile);
  avma=p1;z=(GEN)p1;z[0]=0x2010000+x;
  return z;
}

inline GEN icopy(GEN x)
{
  GEN y;
  long lx=lgef(x),i;
  
  y=cgeti(lx);
  for(i=1;i<lx;i++) y[i]=x[i];
  return y;
}

inline GEN rcopy(GEN x)
{
  GEN y;
  long lx=lg(x),i;
  
  y=cgetr(lx);
  for(i=1;i<lx;i++) y[i]=x[i];
  return y;
}

inline GEN stoi(long x)
{
  GEN y;
  
  if(!x) return gzero;
  y=cgeti(3);
  if(x>0) {y[1]=0x1000003;y[2]=x;}
  else{y[1]=0xff000003;y[2]= -x;}
  return y;
}

inline GEN cgetg(long x, long y)
{
  unsigned long p1;
  GEN z;
  
  p1=avma-(((unsigned short)x)<<2);
  if(p1<bot) err(errpile);
  avma=p1;z=(GEN)p1;z[0]=0x10000+x+(y<<24);
  return z;
}



inline GEN negi(GEN x)
{
  long s=signe(x);
  GEN y;
  
  if(!s) return gzero;
  y=icopy(x);setsigne(y,-s);
  return y;
}

inline GEN negr(GEN x)
{
  GEN y;
  
  y=rcopy(x);setsigne(y,-signe(x));
  return y;
}


inline GEN absi(GEN x)
{
  GEN y;
  long s=signe(x);
  
  if(!s) return gzero;
  y=icopy(x);setsigne(y,1);return y;
}

inline GEN absr(GEN x)
{
  GEN y;
  long s=signe(x);
  
  y=rcopy(x);
  if(s) setsigne(y,1);
  return y;
}

inline int expi(GEN x)
{
  long lx=x[1]&0xffff;return lx==2 ? -8388608 : ((lx-2)<<5)-bfffo(x[2])-1;
}

inline long itos(GEN x)
{
  long s=signe(x),p2;
  unsigned long p1;
  
  if(!s) return 0;
  if(lgef(x)>3) err(affer2);
  p1=x[2];if(p1>=0x80000000) err(affer2);
  p2=(s>0)?p1:(-((long)p1));return p2;
}

inline void affii(GEN x, GEN y)
{
  long lx=lgef(x),i;
  
  if(x==y) return;
  if(lg(y)<lx) err(affer3);
  for(i=1;i<lx;i++) y[i]=x[i];
}

inline void mpaff(GEN x, GEN y)
{
  long tx=typ(x),ty=typ(y);
  if(tx==1) {if(ty==1) affii(x,y);else affir(x,y);}
  else {if(ty==1) affri(x,y);else affrr(x,y);}
}

inline void affsi(long s, GEN x)
{
  long lx;
  
  if(!s) {x[1]=2;return;}
  lx=lg(x);if(lx<3) err(affer1);
  if(s>0) {x[1]=0x1000003;x[2]=s;}
  else {x[1]=0xff000003;x[2]= -s;}
}

inline void affsr(long s, GEN x)
{
  long l,i,d;
  
  if(!s) {l=((x[0]&0xffff)-2)<<5;x[1]=0x800000-l;x[2]=0;}
  else
    {
      d=1;if(s<0) {d= -1;s= -s;}
      l=bfffo(s);setexpo(x,31-l);setsigne(x,d);
      x[2]=(s<<l);for(i=3;i<lg(x);i++) x[i]=0;
    }
}


inline GEN shiftr(GEN x, long n)
{
  long l;
  GEN y;
  
  y=rcopy(x);l=expo(x)+n;
  if(l>0x7fffff||l<-0x800000) err(shier2);
  setexpo(y,l);return y;
}

inline int cmpir(GEN x, GEN y)
{
  long av=avma;
  int p;
  GEN z;
  
  if(!signe(x)) return -signe(y);
  z=cgetr(lg(y));affir(x,z);
  p=cmprr(z,y);avma=av;return p;
}

inline int mpcmp(GEN x, GEN y)
{
  if(typ(x)==1) return (typ(y)==1) ? cmpii(x,y) : cmpir(x,y);
  return (typ(y)==1) ? -cmpir(y,x) : cmprr(x,y);
}

inline int cmpsr(long x, GEN y)
{
  int p;
  long av;
  GEN z;
  
  if(!x) return -signe(y);
  av=avma;z=cgetr(3);affsr(x,z);
  p=cmprr(z,y);avma=av;return p;
}	


inline GEN mpadd(GEN x, GEN y)
{
  if(typ(x)==1) return (typ(y)==1) ? addii(x,y) : addir(x,y);
  return (typ(y)==1) ? addir(y,x) : addrr(x,y);
}

inline void addssz(long x, long y, GEN z)
{
  long av=avma;
  GEN p1;
  
  if(typ(z)==1) gops2ssz(addss,x,y,z);
  else {p1=cgetr(lg(z));affsr(x,p1);p1=addrs(p1,y);affrr(p1,z);avma=av;}
}


inline GEN subii(GEN x, GEN y)
{
  long s=signe(y);
  GEN z;
  
  if(x==y) return gzero;
  setsigne(y,-s);z=addii(x,y);setsigne(y,s);return z;
}

inline GEN subrr(GEN x, GEN y)
{
  long s=signe(y);
  GEN z;
  
  if(x==y) {z=cgetr(3);z[2]=0;z[1]=0x800000-(lg(x)<<5);return z;}
  setsigne(y,-s);z=addrr(x,y);setsigne(y,s);return z;
}

inline GEN subir(GEN x, GEN y)
{
  long s=signe(y);
  GEN z;
  
  setsigne(y,-s);z=addir(x,y);setsigne(y,s);return z;
}

inline GEN subri(GEN x, GEN y)
{
  long s=signe(y);
  GEN z;
  
  setsigne(y,-s);z=addir(y,x);setsigne(y,s);return z;
}

inline GEN mpsub(GEN x, GEN y)
{
  if(typ(x)==1) return (typ(y)==1) ? subii(x,y) : subir(x,y);
  return (typ(y)==1) ? subri(x,y) : subrr(x,y);
}

inline GEN subsi(long x, GEN y)
{
  long s=signe(y);
  GEN z;
  
  setsigne(y,-s);z=addsi(x,y);setsigne(y,s);return z;
}

inline GEN subsr(long x, GEN y)
{
  long s=signe(y);
  GEN z;
  
  setsigne(y,-s);z=addsr(x,y);setsigne(y,s);return z;
}

inline GEN subss(long x, long y) {return addss(-y,x);}

inline void subssz(long x, long y, GEN z)
{
  long av=avma;
  GEN p1;
  
  if(typ(z)==1) gops2ssz(addss,x,-y,z);
  else {p1=cgetr(lg(z));affsr(x,p1);p1=addrs(p1,-y);affrr(p1,z);avma=av;}
}

inline GEN mpmul(GEN x, GEN y)
{
  if(typ(x)==1) return (typ(y)==1) ? mulii(x,y) : mulir(x,y);
  return (typ(y)==1) ? mulir(y,x) : mulrr(x,y);
}

inline void mulssz(long x, long y, GEN z)
{
  long av=avma;
  GEN p1;
  
  if(typ(z)==1) gops2ssz(mulss,x,y,z);
  else {p1=cgetr(lg(z));affsr(x,p1);p1=mulsr(y,p1);mpaff(p1,z);avma=av;}
}

inline void mulsii(long x, GEN y, GEN z)
{
  long av=avma;
  GEN p1;
  
  p1=mulsi(x,y);affii(p1,z);avma=av;
}

inline void addsii(long x, GEN y, GEN z)
{
  long av=avma;
  GEN p1;
  
  p1=addsi(x,y);affii(p1,z);avma=av;
}

inline long divisii(GEN x, long y, GEN z)
{
  long av=avma,k;
  GEN p1;
  
  p1=divis(x,y);affii(p1,z);avma=av;
  k=hiremainder;return k;
}

inline long vali(GEN x)
{
  long i,lx=lgef(x);
  
  if(!signe(x)) return -1;
  for(i=lx-1;(i>=2)&&(!x[i]);i--);
  return ((lx-1-i)<<5)+vals(x[i]);
}

inline GEN divss(long x, long y)
{
  long p1;
  
  if(!y) err(diver1);
  hiremainder=0;p1=divll((ulong)labs(x),(ulong)labs(y));
  if(y<0) {hiremainder= -((long)hiremainder);p1= -p1;}
  if(x<0) p1= -p1;
  return stoi(p1);
}

inline GEN mpdiv(GEN x, GEN y)
{
  if(typ(x)==1) return (typ(y)==1) ? divii(x,y) : divir(x,y);
  return (typ(y)==1) ? divri(x,y) : divrr(x,y);
}

inline GEN dvmdss(long x, long y, GEN *z)
{
  GEN p1;

  p1=divss(x,y);*z=stoi(hiremainder);return p1;
}

inline void dvmdssz(long x, long y, GEN z, GEN t)
{
  long av=avma;
  GEN p1;

  p1=divss(x,y);affsi(hiremainder,t);mpaff(p1,z);avma=av;
}

inline GEN dvmdsi(long x, GEN y, GEN *z)
{
  GEN p1;
  p1=divsi(x,y);*z=stoi(hiremainder);return p1;
}

inline void dvmdsiz(long x, GEN y, GEN z, GEN t)
{
  long av=avma;
  GEN p1;
  
  p1=divsi(x,y);affsi(hiremainder,t);mpaff(p1,z);avma=av;
}

inline GEN dvmdis(GEN x, long y, GEN *z)
{
  GEN p1;
  p1=divis(x,y);*z=stoi(hiremainder);return p1;
}

inline void dvmdisz(GEN x, long y, GEN z, GEN t)
{
  long av=avma;
  GEN p1;
  
  p1=divis(x,y);affsi(hiremainder,t);mpaff(p1,z);avma=av;
}

inline void dvmdiiz(GEN x, GEN y, GEN z, GEN t)
{
  long av=avma;
  GEN p1,p2;

  p1=dvmdii(x,y,&p2);mpaff(p1,z);mpaff(p2,t);avma=av;
}

inline GEN ressi(long x, GEN y)
{
  divsi(x,y);return stoi(hiremainder);
}

inline GEN modis(GEN x, long y)
{
  divis(x,y);if(!hiremainder) return gzero;
  return (signe(x)>0) ? stoi(hiremainder) : stoi(labs(y)+hiremainder);
}

inline GEN resis(GEN x, long y)
{
  divis(x,y);return stoi(hiremainder);
}
     
inline void divisz(GEN x, long y, GEN z)
{
  long av=avma;
  GEN p1;
  
  if(typ(z)==1) gops2gsz(divis,x,y,z);
  else {p1=cgetr(lg(z));affir(x,p1);p1=divrs(p1,y);affrr(p1,z);avma=av;}
}

inline void divsiz(long x, GEN y, GEN z)
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

inline void divssz(long x, long y, GEN z)
{
  long av=avma;
  GEN p1;
  
  if(typ(z)==1) gops2ssz(divss,x,y,z);
  else {p1=cgetr(lg(z));affsr(x,p1);p1=divrs(p1,y);affrr(p1,z);avma=av;}
}

inline void divrrz(GEN x, GEN y, GEN z)
{
  long av=avma;
  GEN p1;

  p1=divrr(x,y);mpaff(p1,z);avma=av;
}

inline void resiiz(GEN x, GEN y, GEN z)
{
  long av=avma;
  GEN p1;

  p1=resii(x,y);affii(p1,z);avma=av;
}

inline int mpdivis(GEN x, GEN y, GEN z)
{
  long av=avma;
  GEN p1,p2;

  p1=dvmdii(x,y,&p2);
  if(signe(p2)) {avma=av;return 0;}
  affii(p1,z);avma=av;return 1;
}

inline int divise(GEN x, GEN y)
{
  long av=avma;
  GEN p1;

  p1=dvmdii(x,y,(GEN *)-1);avma=av;return signe(p1) ? 0 : 1;
}

inline double  gtodouble(GEN x)
{
  GEN     x1;
  long  t=typ(x);
  static long reel4[4]={0x2010004,0,0,0};
  
  if (t==2) x1=x;
  else gaffect(x,x1=(GEN)reel4);
  return rtodbl(x1);
}


