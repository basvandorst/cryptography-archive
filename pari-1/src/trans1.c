/********************************************************************/
/********************************************************************/
/**                                                                **/
/**                +++++++++++++++++++++++++++++++                 **/
/**                +                             +                 **/
/**                +  FONCTIONS TRANSCENDANTES   +                 **/
/**                +                             +                 **/
/**                +     copyright Babe Cool     +                 **/
/**                +                             +                 **/
/**                +++++++++++++++++++++++++++++++                 **/
/**                                                                **/
/**                                                                **/
/********************************************************************/
/********************************************************************/

# include "genpari.h"

/********************************************************************/
/********************************************************************/
/**                                                                **/
/**                        FONCTION PI                             **/
/**                                                                **/
/********************************************************************/
/********************************************************************/

void constpi(long prec)
{
  long    l,n,n1,av1,av2;
  double  alpha;
  GEN p1,p2,p3, tokill;

#define k1     545140134
#define k2    13591409
#define k3      640320
#define alpha2 (1.4722004/(BYTES_IN_LONG/4))  /*   3*log(k3/12)/(32*log(2))   */

  if ((tokill = gpi) && (lg(gpi)>=prec)) return;

  gpi=newbloc(prec); *gpi = evaltyp(2)+evalpere(1)+evallg(prec);
  av1=avma;

  n=(long)(1+(prec-2)/alpha2);
  n1=6*n-1;
  p1=cgetr(prec);
  p2=addsi(k2,mulss(n,k1));
  affir(p2,p1);
  /*initialisation longueur mantisse*/
  if (prec>=4) l=4;else l=prec;alpha=l;
  setlg(p1,l);

  while (n)
  {
    av2=avma;
    if(n>1290)
    {
      if(n1>46340)
      p3=divrs(divrs(mulsr(n1-4,mulsr(n1,mulsr(n1-2,p1))),n*n),n);
      else
      p3=divrs(divrs(mulsr(n1-4,mulsr(n1*(n1-2),p1)),n*n),n);
    }
    else p3=divrs(mulsr(n1-4,mulsr(n1*(n1-2),p1)),n*n*n);
    p3=divrs(divrs(p3,100100025),327843840);
    subisz(p2,k1,p2);subirz(p2,p3,p1);
    avma=av2;
    alpha+=alpha2;l=(long)(1+alpha);/*nouvelle longueur mantisse*/
    if (l>prec) l=prec;
    setlg(p1,l);
    n--; n1-=6;
  }

  p1=divsr(53360,p1);
  mulrrz(p1,gsqrt(stoi(k3),prec),gpi);
  avma=av1;
  killbloc(tokill);
}

GEN mppi(long prec)
{
  GEN x;

  constpi(prec);
  affrr(gpi,x=cgetr(prec));
  return x;
}

/********************************************************************/
/********************************************************************/
/**                                                                **/
/**                      FONCTION EULER                            **/
/**                                                                **/
/********************************************************************/
/********************************************************************/

void consteuler(long prec)
{
  long    l,n,k,x,xx,av1,av2;
  GEN u,v,a,b,tokill;

  if ((tokill = geuler) && (lg(geuler)>=prec)) return;
  l=prec+1;
  geuler=newbloc(prec); *geuler = evaltyp(2)+evalpere(1)+evallg(prec);
  x=(long)(1+((l-2)<<(TWOPOTBITS_IN_LONG-2))*LOG2);xx=x*x;
  n=(long)(1+3.591*x);     /*    a1=3.591  :   a1*[ ln(a1)-1 ]=1 */
  av1=avma;
  affsr(x,a=cgetr(l));
  u=mplog(a);setsigne(u,-1);affrr(u,a);
  affsr(1,b=cgetr(l));
  affsr(1,v=cgetr(l));
  for (k=1;k<=n;k++)
  {
    av2=avma;
    divrsz(mulsr(xx,b),k*k,b);
    divrsz(addrr(divrs(mulsr(xx,a),k),b),k,a);
    addrrz(u,a,u);addrrz(v,b,v);
    avma=av2;
  }
  divrrz(u,v,geuler);
  avma=av1;
  killbloc(tokill);
}

GEN mpeuler(long prec)
{
  GEN x;

  consteuler(prec);
  affrr(geuler,x=cgetr(prec));
  return x;
}

/********************************************************************/
/********************************************************************/
/**                                                                **/
/**                     CONVERSION DE TYPES                        **/
/**                 POUR FONCTIONS TRANSCENDANTES                  **/
/**                                                                **/
/********************************************************************/
/********************************************************************/

#ifdef __cplusplus
GEN transc(GEN f (GEN, long), GEN x, long prec)
#else
GEN transc(GEN (*f) (GEN, long), GEN x, long prec)
#endif
{
  GEN p1,p2,y;
  long  lx,i,av,tetpil;

  av=avma;
  switch(typ(x))
  {
  case 1 :
  case 4 :
  case 5 : p1=cgetr(prec);gaffect(x,p1);tetpil=avma;
    y=gerepile(av,tetpil,(*f)(p1,prec));break;
  
  case 6 :
  case 8 : av=avma;p1=cgetr(prec);affsr(1,p1);
    p1=gmul(x,p1);tetpil=avma;
    y=gerepile(av,tetpil,(*f)(p1,prec));break;
  
  case 10:
  case 13:
  case 14: p1=tayl(x,gvar(x),precdl);tetpil=avma;
    y=gerepile(av,tetpil,(*f)(p1,prec));
    break;
  
  case 17:
  case 18:
  case 19: lx=lg(x);y=cgetg(lx,typ(x));
    for(i=1;i<lx;i++)
    y[i]=(long)(*f)((GEN)x[i],prec);break;
  case 9 : av=avma;p1=roots((GEN)x[1],prec);lx=lg(p1);p2=cgetg(lx,18);
    for(i=1;i<lx;i++) p2[i]=lpoleval((GEN)x[2],(GEN)p1[i]);
    tetpil=avma;y=cgetg(lx,18);
    for(i=1;i<lx;i++) y[i]=(long)(*f)((GEN)p2[i],prec);
    y=gerepile(av,tetpil,y);break;
  case 15:
  case 16: err(transcer1);
  default: y=(*f)(x,prec);
  }
  return y;
}


/********************************************************************/
/********************************************************************/
/**                                                                **/
/**                        RACINE CARREE                           **/
/**                                                                **/
/********************************************************************/
/********************************************************************/

GEN mpsqrt(GEN x)
{
  long    l,l1,l0,l2,s,eps,n,i,alpha,ex,av;
  double  beta;
  GEN y,p1,p2,p3;

  if (typ(x)!=2) err(sqrter1);
  s=signe(x);
  if (s<0) err(sqrter2);
  if (!s)
  {
    y=cgetr(3);y[1]=HIGHEXPOBIT+(expo(x)>>1);
    y[2]=0;
  }
  else
  {
    l=lg(x);y=cgetr(l);
    av=avma;
    p1=cgetr(l+1);
    mpaff(x,p1);
    ex=expo(x);eps=ex % 2;ex /=2;
    if (eps<0)
    {
      eps+=2;ex-=1;
    }
    setexpo(p1,eps);setlg(p1,3);
    l1=1;
    beta=sqrt((eps+1)*(2+mant(p1,1)/C31));
    n=(long)(2+log((double) (l-2))/LOG2);
    p2=cgetr(l+1);
    setexpo(p2,0);setlg(p2,3);setsigne(p2,1);
    l2=3;
    alpha=(long)((beta-2)*C31);
    setmant(p2,1,alpha);
    p3=cgetr(l+1);l-=2;
    if (!alpha)
    {
      setmant(p2,1,HIGHBIT);
      setexpo(p2,1);
    }
  
    for (i=1;i<=n;i++)
    {
      l0=l1+l1;
      if (l0<=l)
      {
        l2=l2+l1;
        setlg(p2,l2);
        setlg(p3,l0+2);
        setlg(p1,l0+2);
        l1=l0;
      }
      else
      {
        l2=l2-l1+l+1;
        setlg(p2,l2);
        setlg(p3,l+3);setlg(p1,l+3);
        l1=l+1;
      }
      divrrz(p1,p2,p3);
      addrrz(p2,p3,p2);
      setexpo(p2,expo(p2)-1);
    }
    affrr(p2,y);
    setexpo(y,expo(y)+ex);
    avma=av;
  }
  return y;
}

GEN   pasqrt(GEN x)
{
  long    av,av2,l3,e,lp,lpp,pp,fl,re;
  GEN y,p1,p2;

  e=valp(x);
  y=cgetg(5,7);y[2]=x[2];
  if(gcmp0(x))
  {
    y[4]=zero;e=(e+1)>>1;
    y[3]=lpuigs((GEN)x[2],e);
    setvalp(y,e);setprecp(y,precp(x));
  }
  else
  {
    if(e&1) err(sqrter6);l3=lgef((GEN)x[3]);
    e>>=1;fl=cmpis((GEN)x[2],2);pp=precp(x);
    y[4]=lgeti(l3);av=avma;
    if(fl)
    {
      if(!mpsqrtmod((GEN)x[4],(GEN)x[2],&p1)) err(sqrter5);
      lp=1;affii(p1,(GEN)y[4]);avma=av;
    }
    else
    {
      affsi(1,(GEN)y[4]);
      re=(((GEN)x[4])[lgef((GEN)x[4])-1])&7;
      if((re!=1)&&(pp>=2))
      {if((pp!=2) || (re!=5)) err(sqrter5);}
      lp=3;
    }
    if(pp<=lp)
    {
      setprecp(y,1);y[3]=x[2];
    }
    else
    {
      y[3]=lgeti(l3);av=avma;
      p2=gcopy(x);setvalp(p2,0);av2=avma;
      setvalp(y,0);if(!fl) affsi(8,(GEN)y[3]);else affii((GEN)x[2],(GEN)y[3]);
      while(lp<pp)
      {
	if(fl)
	  {
	    lp<<=1;if(lp>=pp) {lp=pp;affii((GEN)x[3],(GEN)y[3]);}
	    else muliiz((GEN)y[3],(GEN)y[3],(GEN)y[3]);
	  }
	else
	  {
	    lpp=lp;lp=(lp<<1)-1;if(lp>=pp) {lp=pp;affii((GEN)x[3],(GEN)y[3]);}
	    else mpshiftz((GEN)y[3],lpp-1,(GEN)y[3]);
	  }
        setprecp(y,lp);p1=gadd(y,gdiv(p2,y));
        gdivz(p1,gdeux,y);
	if(!fl) 
	  {
	    setprecp(y,lp-1);if(lp<pp) lp--;
	    mpshiftz((GEN)y[3],-1,(GEN)y[3]);
	    p1=subii((GEN)y[4],(GEN)y[3]);if(signe(p1)>=0) affii(p1,(GEN)y[4]);
	  }
	avma=av2;
      }
      avma=av;
    }
    setvalp(y,e);
  }
  return y;
}

GEN gsqrt(GEN x, long prec)
{
  long    av,tetpil,e;
  GEN y,p1,p2;

  switch(typ(x))
  {
  case 2 : if (signe(x)>=0) y =mpsqrt(x);
  else
    {
    y=cgetg(3,6);y[1]=zero;
    av=avma;p1=mpneg(x);tetpil=avma;
    y[2]=lpile(av,tetpil,mpsqrt(p1));
    }
    break;
  
  case 3 : y=cgetg(3,3);y[1]=copyifstack((GEN)x[1]);
    if(!mpsqrtmod((GEN)x[2],(GEN)y[1],(GEN*)(y+2))) err(sqrter5);
    break;
  
  case 6 : y=cgetg(3,6);av=avma;
    if(gcmp0((GEN)x[2]))
    {
      if(signe((GEN)x[1])>=0)
      {
        y[1]=lsqrt((GEN)x[1],prec);
        y[2]=zero;
      }
      else
      {
        y[1]=zero;p1=mpneg((GEN)x[1]);
        tetpil=avma;
        y[2]=lpile(av,tetpil,gsqrt(p1,prec));
      }
    }
    else
    {
      p1=gmul((GEN)x[1],(GEN)x[1]);
      p2=gmul((GEN)x[2],(GEN)x[2]);
      p1=gsqrt(gadd(p1,p2),prec);
      if (gcmp0(p1))
      {
        y[1]=lsqrt(p1,prec);
        y[2]=lcopy((GEN)y[1]);
      }
      else
      {
        if(gsigne((GEN)x[1])<0)
        {
          p2=gsub(p1,(GEN)x[1]);p1=gmul2n(p2,-1);
	  y[2]=lsqrt(p1,prec);y[1]=ldiv((GEN)x[2],gmul2n((GEN)y[2],1));
	  tetpil=avma;
	  y=gerepile(av,tetpil,gsigne((GEN)x[2])>0?gcopy(y):gneg(y));
        }
        else
        {
          p1=gmul2n(gadd(p1,(GEN)x[1]),-1);
          tetpil=avma;y[1]=lpile(av,tetpil,gsqrt(p1,prec));
          av=avma;p1=gmul2n((GEN)y[1],1);tetpil=avma;
          y[2]=lpile(av,tetpil,gdiv((GEN)x[2],p1));
        }
      }
    }
    break;
  
  case 7 : y = pasqrt(x); break;
  
  case 11: e=valp(x);
    if(gcmp0(x))
    {y=cgetg(3,11);y[1]=HIGHVALPBIT+((e+1)>>1);}
    else
    {
      av=avma;
      if(e&1) err(sqrter6);
      e>>=1;p2=gcopy(x);setvalp(p2,0);
      tetpil=avma;
      y=gerepile(av,tetpil,gpui(p2,ghalf,prec));
      setvalp(y,e);
    }
    setvarn(y,varn(x));
    break;
  
  default: y=transc(gsqrt,x,prec);
  }
  return y;
}

void gsqrtz(GEN x, GEN y)
{
  long    av,prec;
  GEN p;

  prec=precision(y);
  if(!prec) err(sqrter4);
  av=avma;p=gsqrt(x,prec);
  gaffect(p,y);avma=av;
}

/********************************************************************/
/********************************************************************/
/**                                                                **/
/**                    FONCTION EXPONENTIELLE-1                    **/
/**                                                                **/
/********************************************************************/
/********************************************************************/

GEN mpexp1(GEN x)
{
  long    l,l0,l1,l2,i,n,m,ex,s;
  long    sgn,av;
  double  alpha,beta,gama=2.0 /*optimise pour SUN3*/,aa,bb;
  GEN y,p1,p2,p3,p4;

  if (typ(x)!=2) err(exp1er);
  else
  {
    sgn=signe(x);
    if (!sgn)
    {
      y=cgetr(3);y[1]=x[1];
      y[2]=0;
    }
    else
    {
      l=lg(x);
      y=cgetr(l);     /*reservation pour resultat*/
      av=avma;
      p1=cgetr(l+1);
      mpabsz(x,p1);   /*p1 recoit |x|  */
      ex=expo(x);
      alpha= -1-log(2+mant(x,1)/C31)-ex*LOG2;
      beta=5+BITS_IN_LONG*(l-2)*LOG2;
      aa=sqrt((beta)/(gama*LOG2));
      bb=(alpha+0.5*log(beta*gama/LOG2))/LOG2;
      if (aa>=bb)
      {
        n=(long)(1+sqrt(beta*gama/LOG2));
        m=(long)(1+aa-bb);
        setexpo(p1,ex-m);
      }
      else
      {
        n=(long)(1+beta/alpha);
        m=0;
      }
      l2=l+1+(m>>TWOPOTBITS_IN_LONG);
      p2=cgetr(l2);
      affsr(1,p2);setlg(p2,4);
      s=0;
      p3=cgetr(l2);setlg(p3,4);
      p4=cgetr(l2);affrr(p1,p4);setlg(p4,4);
      l1=2;l2-=2;
    
      for (i=n;i>=2;--i)
      {
        divrsz(p4,i,p3);
        mulrrz(p3,p2,p2);
        ex=expo(p3);s=s-ex;l0=s>>TWOPOTBITS_IN_LONG;l1+=l0;
        if (l1>l2) l1=l2;
        s %= BITS_IN_LONG;
        setlg(p2,l1+2);setlg(p3,l1+2);
        setlg(p4,l1+2);
        addsrz(1,p2,p2);
      }
      l2+=2;
      setlg(p2,l2);setlg(p3,l2);setlg(p4,l2);
      mulrrz(p4,p2,p2);
    
      for (i=1;i<=m;i++)
      {
        addsrz(2,p2,p3);
        mulrrz(p3,p2,p2);
      }
    
      if (sgn== -1)
      {
        addsrz(1,p2,p2);divsrz(1,p2,p2);
        subrsz(p2,1,y);
      }
      else
      affrr(p2,y);
      avma=av;
    }
  }
  return y;
}

/********************************************************************/
/********************************************************************/
/**                                                                **/
/**                   FONCTION EXPONENTIELLE                       **/
/**                                                                **/
/********************************************************************/
/********************************************************************/

GEN mpexp(GEN x)
{
  GEN y;
  long av,tetpil;

  if(gcmp0(x)) return addsr(1,x);
  if(signe(x)>=0) return addsr(1,mpexp1(x));
  av=avma;y=addsr(1,mpexp1(negr(x)));tetpil=avma;
  return gerepile(av,tetpil,divsr(1,y));
}

GEN paexp(GEN x)
{
  GEN y, r, p1;
  long k, av = avma, tetpil;
  long e = valp(x), pp = precp(x), n = e + pp;
  
  if (!signe((GEN)x[4])) return gaddgs(x, 1);
  if ((e <= 0) || (!cmpis((GEN)x[2], 2) && (e == 1))) err(paexper1);
  av = avma;
  if (cmpis((GEN)x[2], 2))
  {
    p1 = subis((GEN)x[2], 1);
    k = itos(dvmdii(subis(mulis(p1, n), 1), subis(mulis(p1, e), 1), &r));
    if (!signe(r)) k--;
  }
  else
  {
    k = (n - 1) / (e - 1); if (!((n - 1)%(e-1))) k--;
  }
  for(y = gun; k; k--)
  {
    tetpil = avma; y = gaddsg(1, gdivgs(gmul(y, x), k));
  }
  return gerepile(av, tetpil, y);
}

GEN gexp(GEN x, long prec)
{
  long    av,tetpil,i,ly,j,ex;
  GEN r,u,v;
  GEN y,p1,p2;

  switch(typ(x))
  {
    case 2 : y=mpexp(x); break;
    
    case 6 : y=cgetg(3,6);av=avma;
      r=gexp((GEN)x[1],prec);
      gsincos((GEN)x[2],&u,&v,prec);
      tetpil=avma;
      y[1]=lmul(r,v);y[2]=lmul(r,u);
      gerepile(av,tetpil,(GEN)1);
      break;
    
    case 7 : y = paexp(x); break;
    
    case 11: if(gcmp0(x)) y=gaddsg(1,x);
      else
      {
        ex=valp(x);if(ex<0) err(exper3);
        if(ex)
        {
          ly=lg(x)+ex;y=cgetg(ly,11); 
          y[1]=evalsigne(1)+HIGHVALPBIT;y[2]=un;setvarn(y,varn(x));
          for(i=3;i<ex+2;i++) y[i]=zero;
          for(i=ex+2;i<ly;i++)
          {
            av=avma;p1=gzero;
            for(j=ex;j<i-1;j++)
              p1=gadd(p1,gmulgs(gmul((GEN)x[j-ex+2],(GEN)y[i-j]),j));
            tetpil=avma;y[i]=lpile(av,tetpil,gdivgs(p1,i-2));
          }
        }
        else
        {
          av=avma;p1=gcopy(x);p1[2]=zero;
          normalize(&p1);p2=gexp(p1,prec);
          p1=gexp((GEN)x[2],prec);tetpil=avma;
          y=gerepile(av,tetpil,gmul(p1,p2));
        }
      }
    break;
    
    case 3 : err(exper1);
    
    default: y = transc(gexp,x,prec);
  }
  return y;
}

void gexpz(GEN x, GEN y)
{
  long    av,prec;
  GEN p;

  prec=precision(y);
  if(!prec) err(exper2);
  av=avma;p=gexp(x,prec);
  gaffect(p,y);avma=av;
}

/********************************************************************/
/********************************************************************/
/**                                                                **/
/**                      FONCTION LOGARITHME                       **/
/**                                                                **/
/********************************************************************/
/********************************************************************/

GEN mplog(GEN x)
{
  long    l,l0,l1,l2 ,m,m1,n,i,ex,s;
  long    sgn,avmacourant,av;
  double  alpha,beta,aa,bb,p1copie;
  GEN y,p1,p2,p3,p4,p5;

  if (typ(x)!=2) err(loger1);
  if (signe(x)<=0) err(loger2);
  l=lg(x);
  y=cgetr(l);
  avmacourant=avma;
  p1=cgetr(l+1);
  affrr(x,p1);
  sgn=cmpsr (1,p1);
  if (!sgn) affsr(0,y);
  else
  {
    if (sgn>0) divsrz(1,p1,p1);/* x<1 changer x en 1/x*/
    m1=0;
    while (expo(p1)>=1)
    {
      av=avma;affrr(mpsqrt(p1),p1);
      avma=av;m1+=1;
    }
    p1copie=2+mant(p1,1)/C31;
    if (p1copie==1) p1copie=p1copie+0.00000001;
    l-=2;
    alpha= -log(p1copie-1);
    beta=(BITS_IN_LONG/2)*l*LOG2;
    aa=alpha/LOG2;
    bb=sqrt((BITS_IN_LONG/2)*l/3.0);
    if (aa<=bb)
    {
      n=(long)(1+sqrt((BITS_IN_LONG/2)*3.0*l));
      m=(long)(1+bb-aa);
    }
    else
    {
      n=(long)(1+beta/alpha);
      m=0;
    }
    l2=l+3+(m>>TWOPOTBITS_IN_LONG);
    p2=cgetr(l2);p3=cgetr(l2);
    p4=cgetr(l2);p5=cgetr(l2);
    affrr(p1,p4);
  
    for (i=1;i<=m;i++)
    {
      av=avma;affrr(mpsqrt(p4),p4);
      avma=av;
    }
  
    subrsz(p4,1,p2);
    addsrz(1,p4,p3);
    divrrz(p2,p3,p2);
    mulrrz(p2,p2 ,p3);
    l1=2;
    setlg(p4,l1+2);setlg(p5,l1+2);
    divssz(1,2*n+1,p4);
    s=0;setlg(p3,4);
    ex=expo(p3);
    l2-=2;
  
    for (i=n;i>=1;--i)
    {
      mulrrz(p4,p3,p4);
      divssz(1,2*i-1,p5);
      s-=ex;l0=s>>TWOPOTBITS_IN_LONG;l1+=l0;
      if (l1>l2) l1=l2;
      s %= BITS_IN_LONG;
      setlg(p3,l1+2);
      setlg(p4,l1+2);
      setlg(p5,l1+2);
      addrrz(p4,p5,p4);
    }
  
    l2+=2;setlg(p4,l2);
    mulrrz(p2,p4,y);
    setexpo(y,expo(y)+m+1+m1);
    if (sgn==1) mpnegz(y,y);
  }
  avma=avmacourant;
  return y;
}

GEN   teich(GEN x)
{
  GEN aux,y,z,p1;
  long av,n,k;
  
  if(typ(x)!=7) err(teicher1);
  if(!signe((GEN)x[4])) return gcopy(x);
  y=cgetp(x); setvalp(y, 0);
  if(!cmpis((GEN)x[2],2))
  {
    if(((GEN)x[4])[lgef((GEN)x[4])-1]&2) subisz((GEN)x[3],1,(GEN)y[4]); 
    else affsi(1,(GEN)y[4]);
  }
  else
  {
    av=avma;p1=addsi(-1,(GEN)x[2]);aux=divii(addsi(-1,(GEN)x[3]),p1);
    z=(GEN)x[4];n=precp(x);
    for(k=1;k<n;k<<=1)
      z=modii(mulii(z,addsi(1,mulii(aux,addsi(-1,puissmodulo(z,p1,(GEN)x[3]))))),(GEN)x[3]);
    affii(z,(GEN)y[4]);avma=av;
  }
  return y;
}

GEN palogaux(GEN x)
{
  long av,av1=avma,tetpil,k,e,pp,al;
  GEN y,s,p1,y2;
  
  if(!cmpis((GEN)x[4],1)) 
  {
    y=gaddgs(x,-1);
    if(!cmpis((GEN)x[2],2))
    {
      setvalp(y,valp(y)-1);y[3]=(long)shifti((GEN)y[3],-1);
    }
    tetpil=avma;return gerepile(av1,tetpil,gcopy(y));
  }
  y=gdiv(gaddgs(x,-1),gaddgs(x,1));e=valp(y);pp=precp(y);
  if(cmpis((GEN)x[2],2))
  {
    av=avma;for(al=0,p1=stoi(e);cmpis(p1,pp+al+e)<0;al++) p1=mulii(p1,(GEN)x[2]);
    avma=av;
  }
  else al=1;
  k=(pp+al+e-2)/e;if(!odd(k)) k--;
  y2=gmul(y,y);s=gdivgs(gun,k);
  while(k>=3)
  {
    k-=2;s=gadd(gmul(y2,s),gdivgs(gun,k));
  }
  tetpil=avma;return gerepile(av1,tetpil,gmul(s,y));
}
  
GEN   palog(GEN x)
{
  long av=avma,tetpil;
  GEN p1,y;
  
  if (!signe((GEN)x[4])) err(loger6);
  if(cmpis((GEN)x[2],2))
  {
    y=cgetp(x);setvalp(y,0);
    p1=gsubgs((GEN)x[2],1);affii(puissmodulo((GEN)x[4],p1,(GEN)x[3]),(GEN)y[4]);
    y=gmulgs(palogaux(y),2);tetpil=avma;
    return gerepile(av,tetpil,gdiv(y,p1));
  }
  else
  {
    y=gsqr(x);setvalp(y,0);tetpil=avma;
    return gerepile(av,tetpil,palogaux(y));
  }
}

GEN glog(GEN x, long prec)
{
  long    av,tetpil,v;
  GEN y,p1,p2;

  switch(typ(x))
  {
  case 2 : if(signe(x)>=0) y=mplog(x);
  else
    {
    y=cgetg(3,6);y[2]=lmppi(lg(x));
    setsigne(x,1);y[1]=lmplog(x);
    setsigne(x,-1);
    }
    break;
  
  case 6 : y=cgetg(3,6);y[2]=larg(x,prec);
    av=avma;p1=glog(gnorm(x),prec);tetpil=avma;
    y[1]=lpile(av,tetpil,gmul2n(p1,-1));
    break;
  
  case 7 : y = palog(x); break;
  case 3 : err(loger3);
  
  case 11: av=avma;if(valp(x)) err(loger5);
    v=varn(x);p1=gdiv(deriv(x,v),x);
    if(gcmp1((GEN)x[2]))
    {
      tetpil=avma;y=gerepile(av,tetpil,integ(p1,v));
    }
    else
    {
      p1=integ(p1,v);p2=glog((GEN)x[2],prec);
      tetpil=avma;y=gerepile(av,tetpil,gadd(p1,p2));
    }
    break;
  
  default: y=transc(glog,x,prec);
  }
  return y;
}

void glogz(GEN x, GEN y)
{
  long    av,prec;
  GEN p;

  prec=precision(y);
  if(!prec) err(loger4);
  av=avma;p=glog(x,prec);
  gaffect(p,y);avma=av;
}

/********************************************************************/
/********************************************************************/
/**                                                                **/
/**                      FONCTION SINCOS-1                         **/
/**                                                                **/
/********************************************************************/
/********************************************************************/

GEN mpsc1(GEN x, long *ptmod8)
{
  long l,l0,l1,l2,l4,l5,ee,i,n,m,s,t,avmacourant,av,mmax,mod4;
  double alpha,beta,aa,bb,cc,dd;
  GEN y,p1,p2,p3,p4,pitemp;
  
  mmax=23169;
/* Note: on a 64-bit machine with true 128 bit/64 bit division, one could
take mmax=1518500248; on the alpha it does not seem worthwhile */
  if(typ(x)!=2) err(sc1er1);
  else
  {
    if(!signe(x))
    {
      *ptmod8=0;y=cgetr(3);y[2]=0;y[1]=HIGHEXPOBIT-1+(expo(x)<<1);
    }
    else
    {
      l=lg(x)+1;y=cgetr(l-1);avmacourant=avma;pitemp=mppi(l+1);setexpo(pitemp,-1);
      p1=cgetr(l+1);addrrz(x,pitemp,p1);setexpo(pitemp,0);
      if(expo(p1)>=(BITS_IN_LONG*(l-2)+3)) err(sc1er2);
      av=avma;p3=divrr(p1,pitemp);p2=mpent(p3);
      if(signe(p2)) subrrz(x,mulir(p2,pitemp),p1);else affrr(x,p1);
      *ptmod8=(signe(p1)==-1)*4;mod4=0;
      if(signe(p2))
      {
        mod4=mant(p2,lgef(p2)-2)&3;if((signe(p2)<0)&&(mod4)) mod4=4-mod4;
        *ptmod8+=mod4;
      }
      avma=av;
      if(gcmp0(p1)) alpha=1000000.0;
      else {m=expo(p1);alpha=(m< -1023) ? -1-m*LOG2 : -1-log(fabs(rtodbl(p1)));}
      beta=5+BITS_IN_LONG*(l-2)*LOG2;aa=0.5/LOG2;bb=0.5*aa;
      cc=aa+sqrt((beta+bb)/LOG2);
      dd=((beta/cc)-alpha-log(cc))/LOG2;
      if(dd>=0) {m=(long)(1+dd);n=(long)((1+cc)/2.0);setexpo(p1,expo(p1)-m);}
      else {m=0;n=(long)((1+beta/alpha)/2.0);}
      l5=l2=l+1+(m>>TWOPOTBITS_IN_LONG);
      p2=cgetr(l2);p3=cgetr(l2);p4=cgetr(l2);
      s=0;l1=2;
      affrr(p1,p4);affsr(1,p2);mulrrz(p4,p4,p4);
      setlg(p2,l1+2);setlg(p3,l1+2);setlg(p4,l1+2);
      l4=l1+2;l2-=2;
      if(n<=mmax)
      {
	setlg(p4,4);setlg(p3,4);divrsz(p4,(2*n+1)*(2*n+2),p3);ee= -expo(p3);
	s=0;l1=1+(ee>>TWOPOTBITS_IN_LONG);l4=l1+2;
	if(l4<=l5) {setlg(p2,l4);setlg(p3,l4);setlg(p4,l4);}
        for(i=n;i>=2;--i)
        {
          divrsz(p4,2*i*(2*i-1),p3);mulrrz(p3,p2,p2);s-=expo (p3);
	  t=s&(BITS_IN_LONG-1);if(!t) l0=(s>>TWOPOTBITS_IN_LONG);else l0=(s>>TWOPOTBITS_IN_LONG)+1;l1+=l0;
          if(l1>l2) {l0+=l2-l1;l1=l2;}l4+=l0;
          if(l4<=l5) {setlg(p2,l4);setlg(p3,l4);setlg(p4,l4);}
          subsrz(1,p2,p2);
        }
      }
      else
      {
	setlg(p4,4);setlg(p3,4);divrsz(p4,2*n+1,p3);divrsz(p3,2*n+2,p3);ee= -expo(p3);
	s=0;l1=1+(ee>>TWOPOTBITS_IN_LONG);l4=l1+2;
	if(l4<=l5) {setlg(p2,l4);setlg(p3,l4);setlg(p4,l4);}
        for(i=n;i>=2;--i)
        {
          divrsz(p4,2*i,p3);divrsz(p3,2*i-1,p3);mulrrz(p3,p2,p2);s-=expo (p3);
	  t=s&(BITS_IN_LONG-1);if(!t) l0=(s>>TWOPOTBITS_IN_LONG);else l0=(s>>TWOPOTBITS_IN_LONG)+1;l1+=l0;
          if(l1>l2) {l0+=l2-l1;l1=l2;}l4+=l0;
          if(l4<=l5) {setlg(p2,l4);setlg(p3,l4);setlg(p4,l4);}
          subsrz(1,p2,p2);
        }
      }
      if(l4<=l5) {setlg(p2,l4);setlg(p3,l4);setlg(p4,l4);}
      setexpo(p4,expo(p4)-1);setsigne(p4,-signe (p4));
      mulrrz(p4,p2,p2);
      for(i=1;i<=m;i++)
      {
        addsrz(2,p2,p3);mulrrz(p2,p3,p2);setexpo(p2,expo(p2)+1);
      }
      mpaff(p2,y);avma=avmacourant;
    }
  }
  return y;
}

/********************************************************************/
/********************************************************************/
/**                                                                **/
/**                      FONCTION sqrt(-x*(x+2))                   **/
/**                                                                **/
/********************************************************************/
/********************************************************************/

GEN mpaut(GEN x)
{
  long    av,tetpil;
  GEN y,p1;

  av=avma;
  p1=addsr(2,x);
  p1=mulrr(x,p1);
  setsigne(p1,-signe(p1));tetpil=avma;
  y=gerepile(av,tetpil,mpsqrt(p1));
  return y;
}

/********************************************************************/
/********************************************************************/
/**                                                                **/
/**                       FONCTION COSINUS                         **/
/**                                                                **/
/********************************************************************/
/********************************************************************/

GEN mpcos(GEN x)
{
  long    mod8,av,tetpil;
  GEN y,p1;

  if(typ(x)!=2) err(coser1);
  if(!signe(x))
  {
    y=addsr(1,x);
  }
  else
  {
    av=avma;
    p1=mpsc1(x,&mod8);tetpil=avma;
    switch(mod8)
    {
    case 0: y=addsr(1,p1);break;
    case 1: y=mpaut(p1);setsigne(y,-signe(y));break;
    case 2: y=subsr(-1,p1);break;
    case 3: y=mpaut(p1);break;
    case 4: y=addsr(1,p1);break;
    case 5: y=mpaut(p1);break;
    case 6: y=subsr(-1,p1);break;
    case 7: y=mpaut(p1);setsigne(y,-signe(y));break;
    default:;
    }
    y=gerepile(av,tetpil,y);
  }
  return y;
}

GEN gcos(GEN x, long prec)
{
  long    av,tetpil;
  GEN r,u,v;
  GEN y,p1,p2;

  switch(typ(x))
  {
  case 2 : y=mpcos(x);break;
  case 6 : y=cgetg(3,6);av=avma;
    r=gexp((GEN)x[2],prec);p1=ginv(r);
    p2=gmul2n(mpadd(p1,r),-1);
    p1=mpsub(p2,r);
    gsincos((GEN)x[1],&u,&v,prec);
    tetpil=avma;
    y[1]=lmul(p2,v);y[2]=lmul(p1,u);
    gerepile(av,tetpil,(GEN)1);
    break;
  
  case 3 :
  case 7 : err(coser2);
  
  case 11: if(gcmp0(x)) y=gaddsg(1,x);
  else
    {
    av=avma;if(valp(x)<0) err(coser4);
    gsincos(x,&u,&v,prec);tetpil=avma;
    y=gerepile(av,tetpil,gcopy(v));
    }
    break;
  
  default: y=transc(gcos,x,prec);
  }
  return y;
}

void gcosz(GEN x, GEN y)
{
  long    av,prec;
  GEN p;

  prec=precision(y);
  if(!prec) err(coser3);
  av=avma;p=gcos(x,prec);
  gaffect(p,y);avma=av;
}

/********************************************************************/
/********************************************************************/
/**                                                                **/
/**                       FONCTION SINUS                           **/
/**                                                                **/
/********************************************************************/
/********************************************************************/

GEN mpsin(GEN x)
{
  long    mod8,av,tetpil;
  GEN y,p1;

  if(typ(x)!=2) err(siner1);
  if(!signe(x))
  {
    y=cgetr(3);y[1]=x[1];y[2]=0;
  }
  else
  {
    y=cgetr(lg(x));
    av=avma;
    p1=mpsc1(x,&mod8);tetpil=avma;
    switch(mod8)
    {
    case 0: y=mpaut(p1);break;
    case 1: y=addsr(1,p1);break;
    case 2: y=mpaut(p1);setsigne(y,-signe(y));break;
    case 3: y=subsr(-1,p1);break;
    case 4: y=mpaut(p1);setsigne(y,-signe(y));break;
    case 5: y=addsr(1,p1);break;
    case 6: y=mpaut(p1);break;
    case 7: y=subsr(-1,p1);break;
    default:;
    }
    y=gerepile(av,tetpil,y);
  }
  return y;
}

GEN gsin(GEN x, long prec)
{
  long    av,tetpil;
  GEN r,u,v;
  GEN y,p1,p2;

  switch(typ(x))
  {
  case 2 : y=mpsin(x);break;
  case 6 : y=cgetg(3,6);av=avma;
    r=gexp((GEN)x[2],prec);p1=ginv(r);
    p2=gmul2n(mpadd(p1,r),-1);
    p1=mpsub(p2,p1);
    gsincos((GEN)x[1],&u,&v,prec);
    tetpil=avma;
    y[1]=lmul(p2,u);y[2]=lmul(p1,v);
    gerepile(av,tetpil,(GEN)1);
    break;
  
  case 3 :
  case 7 : err(siner2);
  
  case 11: if(gcmp0(x)) y=gcopy(x);
  else
    {
    av=avma;if(valp(x)<0) err(siner4);
    gsincos(x,&u,&v,prec);tetpil=avma;
    y=gerepile(av,tetpil,gcopy(u));
    }
    break;
  
  default: y=transc(gsin,x,prec);
  }
  return y;
}

void gsinz(GEN x, GEN y)
{
  long    av,prec;
  GEN p;

  prec=precision(y);
  if(!prec) err(siner3);
  av=avma;p=gsin(x,prec);
  gaffect(p,y);avma=av;
}

/********************************************************************/
/********************************************************************/
/**                                                                **/
/**                    PROCEDURE SINUS,COSINUS                     **/
/**                                                                **/
/********************************************************************/
/********************************************************************/

void mpsincos(GEN x, GEN *s, GEN *c)
{
  long    av,av1,tetpil,dec,mod8;
  GEN p1;

  if(typ(x)!=2) err(sicoer1);
  if(!signe(x))
  {
    p1=cgetr(3);*s=p1;p1[1]=x[1];
    p1[2]=0;*c=addsr(1,x);
  }
  else
  {
    av=avma;
    p1=mpsc1(x,&mod8);tetpil=avma;
  
    switch(mod8)
    {
      case 0: *c=addsr(1,p1);*s=mpaut(p1);break;
      case 1: *s=addsr(1,p1);*c=mpaut(p1);
           setsigne(*c,-signe(*c)); break;
      case 2: *c=subsr(-1,p1);*s=mpaut(p1);
           setsigne(*s,-signe(*s)); break;
      case 3: *s=subsr(-1,p1);*c=mpaut(p1); break;
      case 4: *c=addsr(1,p1);*s=mpaut(p1);
           setsigne(*s,-signe(*s)); break;
      case 5: *s=addsr(1,p1);*c=mpaut(p1); break;
      case 6: *c=subsr(-1,p1);*s=mpaut(p1); break;
      case 7: *s=subsr(-1,p1);*c=mpaut(p1);
           setsigne(*c,-signe(*c)); break;
    }
    av1=avma;dec=lpile(av,tetpil,0)>>TWOPOTBYTES_IN_LONG;
    if(adecaler(*s,tetpil,av1)) (*s)+=dec;
    if(adecaler(*c,tetpil,av1)) (*c)+=dec;
  }
}

void gsincos(GEN x, GEN *s, GEN *c, long prec)
{
  long    av,av1,tetpil,dec,ii,i,j,ex,ex2,lx,ly;
  GEN r,u,v,u1,v1,p1,p2,p3,p4,ps,pc;

  switch(typ(x))
  {
    case 1 :
    case 4 :
    case 5 : av=avma;p1=cgetr(prec);gaffect(x,p1);
      tetpil=avma;mpsincos(p1,s,c);av1=avma;
      dec=lpile(av,tetpil,0)>>TWOPOTBYTES_IN_LONG;
      if(adecaler(*s,tetpil,av1)) *s+=dec;
      if(adecaler(*c,tetpil,av1)) *c+=dec;
      break;
    case 2 : mpsincos(x,s,c);break;
    case 6 : ps=cgetg(3,6);pc=cgetg(3,6);
      *s=ps;*c=pc;av=avma;
      r=gexp((GEN)x[2],prec);p1=ginv(r);
      p2=gmul2n(mpadd(p1,r),-1);
      p1=mpsub(p2,p1);r=mpsub(p2,r);
      gsincos((GEN)x[1],&u,&v,prec);
      tetpil=avma;
      ps[1]=lmul(p2,u);ps[2]=lmul(p1,v);
      pc[1]=lmul(p2,v);pc[2]=lmul(r,u);
      av1=avma;dec=lpile(av,tetpil,0);
      if(adecaler((GEN)ps[1],tetpil,av1)) ps[1]+=dec;
      if(adecaler((GEN)ps[2],tetpil,av1)) ps[2]+=dec;
      if(adecaler((GEN)pc[1],tetpil,av1)) pc[1]+=dec;
      if(adecaler((GEN)pc[2],tetpil,av1)) pc[2]+=dec;
      break;
    
    case 11: if(gcmp0(x)) {*c=gaddsg(1,x);*s=gcopy(x);}
    else
    {
      ex=valp(x);lx=lg(x);ex2=2*ex+2;
      if(ex<0) err(sicoer3);
      if(ex2>lx)
      {
        *s=gcopy(x);av=avma;p1=gdivgs(gmul(x,x),2);
        tetpil=avma;*c=gerepile(av,tetpil,gsubsg(1,p1));
      }
      if(!ex)
      {
        av=avma;p1=gcopy(x);p1[2]=zero;
        normalize(&p1);gsincos(p1,&u,&v,prec);
        gsincos((GEN)x[2],&u1,&v1,prec);
        p1=gmul(v1,v);p2=gmul(u1,u);p3=gmul(v1,u);
        p4=gmul(u1,v);tetpil=avma;
        *c=gsub(p1,p2);*s=gadd(p3,p4);
        av1=avma;dec=lpile(av,tetpil,0)>>TWOPOTBYTES_IN_LONG;
        if(adecaler(*c,tetpil,av1)) (*c)+=dec;
	if(adecaler(*s,tetpil,av1)) (*s)+=dec;
      }
      else
      {
        ly=lx+2*ex;
        pc=cgetg(ly,11);ps=cgetg(lx,11);
        *c=pc;*s=ps;pc[1]=evalsigne(1)+HIGHVALPBIT;setvarn(pc,varn(x));
        pc[2]=un;ps[1]=x[1];
        for(i=2;i<ex+2;i++) ps[i]=lcopy((GEN)x[i]);
        for(i=3;i<ex2;i++) pc[i]=zero;
        for(i=ex2;i<ly;i++)
        {
          ii=i-ex;av=avma;p1=gzero;
          for(j=ex;j<ii-1;j++)
            p1=gadd(p1,gmulgs(gmul((GEN)x[j-ex+2],(GEN)ps[ii-j]),j));
          tetpil=avma;
          pc[i]=lpile(av,tetpil,gdivgs(p1,2-i));
          if(ii<lx)
          {
            av=avma;p1=gzero;
            for(j=ex;j<=i-ex2;j++)
              p1=gadd(p1,gmulgs(gmul((GEN)x[j-ex+2],(GEN)pc[i-j]),j));
            p1=gdivgs(p1,i-2);tetpil=avma;
            ps[i-ex]=lpile(av,tetpil,gadd(p1,(GEN)x[i-ex]));
          }
        }
      }
    }
    break;
    
    default: err(sicoer2);
  }
  return;
}

/********************************************************************/
/********************************************************************/
/**                                                                **/
/**                      FONCTION TANGENTE                         **/
/**                                                                **/
/********************************************************************/
/********************************************************************/

GEN mptan(GEN x)
{
  long    av,tetpil;
  GEN s,c,y;

  av=avma;
  mpsincos(x,&s,&c);tetpil=avma;
  y=gerepile(av,tetpil,divrr(s,c));
  return y;
}

GEN gtan(GEN x, long prec)
{
  long    av,tetpil;
  GEN s,c;
  GEN y;

  switch(typ(x))
  {
  case 2 : y=mptan(x);break;
  case 6 : av=avma;gsincos(x,&s,&c,prec);
    tetpil=avma;
    y=gerepile(av,tetpil,gdiv(s,c));
    break;
  
  case 3 :
  case 7 : err(taner1);
  
  case 11: if(gcmp0(x)) y=gcopy(x);
    else
    {
      av=avma;if(valp(x)<0) err(taner3);
      gsincos(x,&s,&c,prec);tetpil=avma;
      y=gerepile(av,tetpil,gdiv(s,c));
    }
    break;
  
  default: y=transc(gtan,x,prec);
  }
  return y;
}

void gtanz(GEN x, GEN y)
{
  long    av,prec;
  GEN p;

  prec=precision(y);
  if(!prec) err(taner2);
  av=avma;p=gtan(x,prec);
  gaffect(p,y);avma=av;
}
