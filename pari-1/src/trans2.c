/********************************************************************/
/********************************************************************/
/**                                                                **/
/**                +++++++++++++++++++++++++++++++                 **/
/**                +                             +                 **/
/**                +  FONCTIONS TRANSCENDANTES   +                 **/
/**                +     (deuxieme partie)       +                 **/
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
/**                       FONCTION ARCTG                           **/
/**                                                                **/
/********************************************************************/
/********************************************************************/

GEN     mpatan(GEN x)
{
  long    l,l1,l2,n,m,u,i,avmacourant,av,lp;
  long    e,sgn,s;
  double  alpha,beta,gama=1.0,delta,fi;
  GEN     y,p1,p2,p3,p4 ,p5;

  if (typ(x)!=2) err(ataner1);
  sgn=signe(x);
  if (!sgn)
    {
      y=cgetr(3);y[1]=x[1];
      y[2]=0;
    }
  else
    {
      l=lp=lg(x);if(expo(x)>0) lp+=(expo(x)>>TWOPOTBITS_IN_LONG);
      y=cgetr(lp);avmacourant=avma;
      p1=cgetr(l+1);affrr(x,p1);
      if (sgn== -1) setsigne(p1,1);
      u=cmprs(p1,1);
      if (!u)
        {
          y=mppi(l+1);setexpo(y,-1);
        }
      else
        {
          if (u==1) divsrz(1,p1,p1);
          if(expo(p1)<-100)
            {
              alpha=log(PI)-expo(p1)*LOG2;
            }
          else
            {
              alpha=rtodbl(p1);
              alpha=log(PI/atan(alpha));
            }
          beta =(BITS_IN_LONG/2)*LOG2*(l-2);
          delta=LOG2+beta-alpha/2;
          fi=alpha-2*LOG2;
          if (delta<=0)
            {
              n=1;m=0;
            }
          else
            {
              if (delta>=gama*fi*fi/LOG2)
                {
                  n=(long)(1+sqrt(gama*delta/LOG2));
                  m=(long)(1+sqrt(delta/(gama*LOG2))-fi/LOG2);
                }
              else
                {
                  n=(long)(1+beta/fi);m=0;
                }
            }
          l2=l+1+(m>>TWOPOTBITS_IN_LONG);
          p2=cgetr(l2);p3=cgetr(l2);p4=cgetr(l2);
          p5=cgetr(l2);
          affrr(p1,p4);
        
          for (i=1;i<=m;i++)
            {
              mulrrz(p4,p4,p5);
              addsrz(1,p5,p5);
              av=avma;affrr(mpsqrt(p5),p5);avma=av;
              addsrz(1,p5,p5);
              divrrz(p4,p5 ,p4);
            }
          affrr(p4,p2);
          mulrrz(p4,p4 ,p3);
          l1=2;l2-=2;
          setlg(p4,4);setlg(p5,4);
          divssz(1,2*n+1,p4);
          s=0;
          setlg(p3,4);
          e=expo(p3);
        
          for (i=n;i>=1;i--)
            {
              mulrrz(p4,p3,p4);
              divssz(1,2*i-1,p5);
              s-=e;l1+=(s>>TWOPOTBITS_IN_LONG);
              if (l1>l2) l1=l2;
              s %= BITS_IN_LONG;
              setlg(p3,l1+2);
              setlg(p4,l1+2);
              setlg(p5,l1+2);
              subrrz(p5,p4,p4);
            }
        
          setlg(p4,l2+2);
          setlg(p5,l2+2);
          mulrrz(p2,p4,p4);
          setexpo(p4,expo(p4)+m);
          if (u==1)
            {
              p5=mppi(lp+1);setexpo(p5,0);
              subrrz(p5,p4,y);
            }
          else affrr(p4,y);
          avma=avmacourant;
        }
      if (sgn== -1) setsigne(y,-signe(y));
    }
  return y;
}

GEN     gatan(GEN x, long prec)
{
  long    av,tetpil,l,v;
  GEN     y,p1;

  switch(typ(x))
    {
    case 2 : y=mpatan(x);break;
    case 6 : av=avma;p1=cgetg(3,6);
      p1[1]=lneg((GEN)x[2]);
      p1[2]=x[1];tetpil=avma;
      y=gerepile(av,tetpil,gath(p1,prec));
      l=y[1];y[1]=y[2];
      y[2]=l;gnegz((GEN)l,(GEN)l);
      break;
    
    case 3 :
    case 7 : err(ataner2);
    
    case 11: av=avma;if(valp(x)<0) err(ataner4);
      v=varn(x);p1=gdiv(deriv(x,v),gaddsg(1,gmul(x,x)));
      if(valp(x)) {tetpil=avma;y=gerepile(av,tetpil,integ(p1,v));}
      else
        {
          y=integ(p1,v);p1=gatan((GEN)x[2],prec);tetpil=avma;
          y=gerepile(av,tetpil,gadd(p1,y));
        }
      break;
    
    default: y=transc(gatan,x,prec);
    }
  return y;
}

void gatanz(GEN x, GEN y)
{
  long    av,prec;
  GEN     p;

  prec=precision(y);
  if(!prec) err(ataner3);
  av=avma;p=gatan(x,prec);
  gaffect(p,y);avma=av;
}

/********************************************************************/
/********************************************************************/
/**                                                                **/
/**                      FONCTION ARCSINUS                         **/
/**                                                                **/
/********************************************************************/
/********************************************************************/

GEN     mpasin(GEN x)
{
  long    l,u,v,sgn,av;
  GEN     y,p1,p2;

  if (typ(x)!=2) err(asiner1);
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
          u=cmprs(x,1);v=cmpsr(-1,x);
          if (u==1 || v==1) err(asiner2);
          if (!u || !v)
            {
              y=mppi(lg(x));setexpo(y,0);
              if (sgn== -1) setsigne(y,-1);
            }
          else
            {
              l=lg(x);y=cgetr(l+1);
              av=avma;
              p1=cgetr(l+1);
              mulrrz(x,x,p1);
              subsrz(1,p1,p1);
              p2=mpsqrt(p1);
              divrrz(x,p2,p1);
              affrr(mpatan(p1),y);
              if (sgn== -1) setsigne(y,-1);
              avma=av;
            }
        }
    }
  return y;
}

GEN     gasin(GEN x, long prec)
{
  long    av,tetpil,l,v;
  GEN     y,p1,z;

  switch(typ(x))
    {
    case 2 : av=avma;
      if(gcmpgs(z=mpabs(x),1)<=0)
        {avma=av;y=mpasin(x);}
      else
        {
          tetpil=avma;y=cgetg(3,6);p1=mpach(z);
          setsigne(p1,-signe(p1));y[2]=(long)p1;
          y[1]=lmppi(lg(x));
          setexpo((GEN)y[1],0);
          if (signe(x)<0) gnegz(y,y);
          y=gerepile(av,tetpil,y);
        }
      break;
    
    case 6 : av=avma;p1=cgetg(3,6);
      p1[1]=lneg((GEN)x[2]);
      p1[2]=x[1];tetpil=avma;
      y=gerepile(av,tetpil,gash(p1,prec));
      l=y[1];y[1]=y[2];
      y[2]=l;gnegz((GEN)l,(GEN)l);
      break;
    
    case 3 :
    case 7 : err(asiner3);
    
    case 11: if(gcmp0(x)) y=gcopy(x);
    else
      {
        av=avma;if(valp(x)<0) err(asiner5);
        v=varn(x);p1=gdiv(deriv(x,v),gsqrt(gsubsg(1,gmul(x,x)),prec));
        if(valp(x)) {tetpil=avma;y=gerepile(av,tetpil,integ(p1,v));}
        else
          {
            y=integ(p1,v);p1=gasin((GEN)x[2],prec);tetpil=avma;
            y=gerepile(av,tetpil,gadd(p1,y));
          }
      }
      break;
    
    default: y=transc(gasin,x,prec);
    }
  return y;
}

void gasinz(GEN x, GEN y)
{
  long    av,prec;
  GEN     p;

  prec=precision(y);
  if(!prec) err(asiner4);
  av=avma;p=gasin(x,prec);
  gaffect(p,y);avma=av;
}

/********************************************************************/
/********************************************************************/
/**                                                                **/
/**                      FONCTION ARCCOSINUS                       **/
/**                                                                **/
/********************************************************************/
/********************************************************************/

GEN     mpacos(GEN x)
{
  long    l,u,v,e,av;
  GEN     y,p1,p2,pitemp;

  if (typ(x)!=2) err(acoser1);
  else
    {
      u=cmprs(x,1);v=cmpsr(-1,x);
      if (u==1 || v==1) err(acoser2);
      if (!signe(x))
        {
          e=expo(x)>>TWOPOTBITS_IN_LONG;if (e>=0) e= -1;
          y=mppi(2-e);setexpo(y,0);
        }
      else
        {
          l=lg(x);
          if (!u)
            {
              y=cgetr(3);y[2]=0;
              y[1]=HIGHEXPOBIT-((l-2)<<(TWOPOTBITS_IN_LONG-1));
            }
          else
            {
              if (!v) y=mppi(lg(x));
              else
                {
                  e=expo(x);if (e<0) e-=1;
                  y=cgetr(l);
                  av=avma;
                  p1=cgetr(l+1);
                  if (e<= -2)
                    {
                      mulrrz(x,x,p1);
                      subsrz(1,p1,p1);
                      affrr(mpsqrt(p1),p1);
                      divrrz(x,p1,p1);
                      affrr(mpatan(p1),y);
                      pitemp=mppi(l);
                      setexpo(pitemp,0);
                      subrrz(pitemp,y,y);
                      avma=av;
                    }
                  else
                    {
                      p2=cgetr(l+1);
                      if (signe(x)>0)
                        {
                          addsrz(1,x,p1);
                          subsrz(2,p1,p2);
                          mulrrz(p1,p2,p1);
                          affrr(mpsqrt(p1),p1);
                          divrrz(p1,x,p1);
                          affrr(mpatan(p1),y);
                        }
                      else
                        {
                          subsrz(1,x,p1);
                          subsrz(2,p1,p2);
                          mulrrz(p1,p2,p1);
                          affrr(mpsqrt(p1),p1);
                          divrrz(p1,x,p1);
                          affrr(mpatan(p1),y);
                          pitemp=mppi(l);
                          addrrz(pitemp,y,y);
                        }
                    }
                  avma=av;
                }
            }
        }
    }
  return y;
}

GEN     gacos(GEN x, long prec)
{
  long    av,tetpil,l,v;
  GEN     y,p1;

  switch(typ(x))
    {
    case 2 : av=avma;
      if(gcmpgs(p1=mpabs(x),1)<=0)
        {avma=av;y=mpacos(x);}
      else
        {
          tetpil=avma;y=cgetg(3,6);y[2]=lmpach(p1);
          if(signe(x)>=0)
            {
              y[1]=zero;
              setsigne((GEN)y[2],-signe((GEN)y[2]));
            }
          else y[1]=lmppi(lg(x));
          y=gerepile(av,tetpil,y);
        }
      break;
    
    case 6 : y=gach(x,prec);
      l=y[1];y[1]=y[2];
      y[2]=l;gnegz((GEN)l,(GEN)l);
      break;
    
    case 3 :
    case 7 : err(acoser3);
    
    case 11: av=avma;if(valp(x)<0) err(acoser5);
      v=varn(x);
      p1=integ(gdiv(deriv(x,v),gsqrt(gsubsg(1,gmul(x,x)),prec)),v);
      if(gcmp1((GEN)x[2])&&(!valp(x)))
        {tetpil=avma;y=gerepile(av,tetpil,gneg(p1));}
      else
        {
          if(valp(x))
            {y=mppi(prec);setexpo(y,0);}
          else y=gacos((GEN)x[2],prec);
          tetpil=avma;y=gerepile(av,tetpil,gsub(y,p1));
        }
      break;
    
    default: y=transc(gacos,x,prec);
    }
  return y;
}

void gacosz(GEN x, GEN y)
{
  long    av,prec;
  GEN     p;

  prec=precision(y);
  if(!prec) err(acoser4);
  av=avma;p=gacos(x,prec);
  gaffect(p,y);avma=av;
}

/********************************************************************/
/********************************************************************/
/**                                                                **/
/**                      FONCTION ARGUMENT                         **/
/**                                                                **/
/********************************************************************/
/********************************************************************/

GEN     mparg(GEN x, GEN y)
{

  long    l,l1,sgnx,sgny,av,tetpil;
  GEN     theta,pitemp;

  if (typ(x)!=2 || typ(y)!=2) err(arger1);
  sgnx=signe(x);sgny=signe(y);
  if (!sgny)
    {
      if (!sgnx) err(arger2);
      l=lg(x);
      if (sgnx>0)
        {
          theta=cgetr(3);theta[1]=y[1]-expo(x);
          theta[2]=0;
        }
      else theta=mppi(l);
    }
  else
    {
      l=lg(y);l1=lg(x);if(l1>l) l=l1;
      if(!sgnx)
        {
          theta=mppi(l);setexpo(theta,0);
          if(sgny<0) setsigne(theta,-1);
        }
      else
        {
          if((expo(x)-expo(y))>-2)
            {
              av=avma;theta=divrr(y,x);tetpil=avma;
              theta=mpatan(theta);
              if(sgnx>0) theta=gerepile(av,tetpil,theta);
              else
                {
                  pitemp=mppi(l);tetpil=avma;
                  if(sgny>0) theta=gerepile(av,tetpil,gadd(pitemp,theta));
                  else theta=gerepile(av,tetpil,gsub(theta,pitemp));
                }
            }
          else
            {
              av=avma;
              pitemp=mppi(l);
              theta=mpatan(divrr(x,y));
              tetpil=avma;setexpo(pitemp,0);
              if(sgny>0) theta=gerepile(av,tetpil,gsub(pitemp,theta));
              else
                {
                  theta=gerepile(av,tetpil,gadd(pitemp,theta));
                  setsigne(theta,-signe(theta));
                }
            }
        }
    }
  return theta;
}

GEN     sarg(GEN x, GEN y, long prec)
{

  long tetpil,av=avma;
  GEN p1;

  switch(typ(x))
    {
    case 1 :
    case 4 :
    case 5 : p1=cgetr(prec);gaffect(x,p1);
      x=p1;break;
    default:;
    }
  switch(typ(y))
    {
    case 1 :
    case 4 :
    case 5 : p1=cgetr(prec);gaffect(y,p1);
      y=p1;break;
    default:;
    }
  if (av==(tetpil=avma)) return mparg(x,y);
  else return gerepile(av,tetpil,mparg(x,y));
}

GEN     garg(GEN x, long prec)
{
  GEN  p1,y;
  long av,tx=typ(x),tetpil;

  if(gcmp0(x)) err(arger2);
  switch(tx)
    {
    case 2 : prec=lg(x);
    case 1 :
    case 4 :
    case 5 :
      if(signe(x)>0)
        {
          y=cgetr(3);y[1]=HIGHEXPOBIT-((prec-2)<<TWOPOTBITS_IN_LONG);
          y[2]=zero;
        }
      else y=mppi(prec);
      break;
    case 8 : av=avma;gaffsg(1,p1=cgetr(prec));p1=gmul(p1,x);
      tetpil=avma;y=gerepile(av,tetpil,garg(p1,prec));
      break;
    case 6 : y=sarg((GEN)x[1],(GEN)x[2],prec);
      break;
    default: if(tx>=17) y=transc(garg,x,prec);else err(arger1);
    }
  return y;
}

/********************************************************************/
/********************************************************************/
/**                                                                **/
/**                 FONCTION COSINUS HYPERBOLIQUE                  **/
/**                                                                **/
/********************************************************************/
/********************************************************************/

GEN     mpch(GEN x)
{
  long    l,av;
  GEN     y,p1,p2;

  if (typ(x)!=2) err(cher3);
  if(gcmp0(x)) y=gaddsg(1,x);
  else
    {
      l=lg(x);y=cgetr(l);
      av=avma;
      p1=cgetr(l+1);p2=cgetr(l+1);
      affrr(mpexp1(x),p1);
      addsrz(1,p1,p2);
      divsrz(1,p2,p2);
      addrrz(p1,p2,p2);
      addsrz(1,p2,y);
      setexpo(y,expo(y)-1);
      avma=av;
    }
  return y;
}

GEN     gch(GEN x, long prec)
{
  long    av,tetpil;
  GEN     y,p1;

  switch(typ(x))
    {
    case 2 : y=mpch(x);break;
    
    case 6 : av=avma;p1=gexp(x,prec);
      p1=gadd(p1,ginv(p1));tetpil=avma;
      y=gerepile(av,tetpil,gmul2n(p1,-1));
      break;
    
    case 3 :
    case 7 : err(cher1);
    
    case 11: av=avma;p1=gexp(x,prec);p1=gadd(p1,gdivsg(1,p1));
      tetpil=avma;y=gerepile(av,tetpil,gmul2n(p1,-1));
      break;
    
    default: y=transc(gch,x,prec);
    }
  return y;
}

void gchz(GEN x, GEN y)
{
  long    av,prec;
  GEN     p;

  prec=precision(y);
  if(!prec) err(cher2);
  av=avma;p=gch(x,prec);
  gaffect(p,y);avma=av;
}

/********************************************************************/
/********************************************************************/
/**                                                                **/
/**                 FONCTION SINUS HYPERBOLIQUE                    **/
/**                                                                **/
/********************************************************************/
/********************************************************************/

GEN     mpsh(GEN x)
{
  long    l,av;
  GEN     y,p1,p2;

  if (typ(x)!=2) err(sher3);
  if(!signe(x))
    {
      y=cgetr(3);y[1]=x[1];y[2]=0;
    }
  else
    {
      l=lg(x);y=cgetr(l);
      av=avma;
      p1=cgetr(l+1);p2=cgetr(l+1);
      affrr(mpexp1(x),p1);
      addsrz(1,p1,p2);
      divrrz(p1,p2,p2);
      addrrz(p1,p2,y);
      setexpo(y,expo(y)-1);
      avma=av;
    }
  return y;
}

GEN     gsh(GEN x, long prec)
{
  long    av,tetpil;
  GEN     y,p1;

  switch(typ(x))
    {
    case 2 : y=mpsh(x);break;
    
    case 6 : av=avma;p1=gexp(x,prec);
      p1=gsub(p1,ginv(p1));tetpil=avma;
      y=gerepile(av,tetpil,gmul2n(p1,-1));
      break;
    
    case 3 :
    case 7 : err(sher1);
    
    case 11: if(gcmp0(x)) y=gcopy(x);
    else
      {
        av=avma;p1=gexp(x,prec);p1=gsub(p1,gdivsg(1,p1));
        tetpil=avma;y=gerepile(av,tetpil,gmul2n(p1,-1));
      }
      break;
    
    default: y=transc(gsh,x,prec);
    }
  return y;
}

void gshz(GEN x, GEN y)
{
  long    av,prec;
  GEN     p;

  prec=precision(y);
  if(!prec) err(sher2);
  av=avma;p=gsh(x,prec);
  gaffect(p,y);avma=av;
}

/********************************************************************/
/********************************************************************/
/**                                                                **/
/**                 FONCTION TANGENTE HYPERBOLIQUE                 **/
/**                                                                **/
/********************************************************************/
/********************************************************************/

GEN     mpth(GEN x)
{
  long    l,av;
  GEN     y,p1,p2;

  if (typ(x)!=2) err(ther1);
  if (!signe(x))
    {
      y=cgetr(3);y[1]=x[1];y[2]=0;
    }
  else
    {
      l=lg(x);y=cgetr(l);
      av=avma;
      p1=cgetr(l+1);p2=cgetr(l+1);
      affrr(x,p1);
      setexpo(p1,expo(p1)+1);
      affrr(mpexp1(p1),p1);
      addsrz(2,p1,p2);
      divrrz(p1,p2,y);
      avma=av;
    }
  return y;
}

GEN     gth(GEN x, long prec)
{
  long    av,tetpil;
  GEN     y,p1,p2,p3;

  switch(typ(x))
    {
    case 2 : y=mpth(x);break;
    
    case 6 : av=avma;p1=gexp(gmul2n(x,1),prec);
      p1=gdivsg(-2,gaddgs(p1,1));tetpil=avma;
      y=gerepile(av,tetpil,gaddsg(1,p1));
      break;
    
    case 3 :
    case 7 : err(ther2);
    
    case 11: if(gcmp0(x)) y=gcopy(x);
    else
      {
        av=avma;p1=gexp(gmul2n(x ,1),prec);
        p2=gsubgs(p1,1);p3=gaddgs(p1,1);
        tetpil=avma;y=gerepile(av,tetpil,gdiv(p2,p3));
      }
      break;
    
    default: y=transc(gth,x,prec);
    }
  return y;
}

void gthz(GEN x, GEN y)
{
  long    av,prec;
  GEN     p;

  prec=precision(y);
  if(!prec) err(ther3);
  av=avma;p=gth(x,prec);
  gaffect(p,y);avma=av;
}

/********************************************************************/
/********************************************************************/
/**                                                                **/
/**             FONCTION ARGUMENT SINUS HYPERBOLIQUE               **/
/**                                                                **/
/********************************************************************/
/********************************************************************/

GEN     mpash(GEN x)
{
  long    l,av;
  GEN     y,p1;

  if (typ(x)!=2) err(asher1);
  if (!signe(x))
    {
      y=cgetr(3);y[1]=x[1];y[2]=0;
    }
  else
    {
      l=lg(x);y=cgetr(l);
      av=avma;
      p1=cgetr(l+1);
      affrr(x,p1);
      mulrrz(p1,p1,p1);
      addsrz(1,p1,p1);
      affrr(mpsqrt(p1),p1);
      addrrz(x,p1,p1);
      affrr(mplog(p1),y);
      avma=av;
    }
  return y;
}

GEN     gash(GEN x, long prec)
{
  long    av,tetpil,v;
  GEN     y,p1;

  switch(typ(x))
    {
    case 2 : y=mpash(x);break;
    
    case 6 : av=avma;p1=gaddsg(1,gmul(x,x));
      p1=gadd(x,gsqrt(p1,prec));
      p1=glog(gmul(p1,p1),prec);tetpil=avma;
      y=gerepile(av,tetpil,gmul2n(p1,-1));
      break;
    
    case 3 :
    case 7 : err(asher2);
    
    case 11: if(gcmp0(x)) y=gcopy(x);
    else
      {
        av=avma;if(valp(x)<0) err(asher4);
        v=varn(x);p1=gdiv(deriv(x,v),gsqrt(gaddsg(1,gmul(x,x)),0));
        if(valp(x)) {tetpil=avma;y=gerepile(av,tetpil,integ(p1,v));}
        else
          {
            y=integ(p1,v);p1=gash((GEN)x[2],prec);tetpil=avma;
            y=gerepile(av,tetpil,gadd(p1,y));
          }
      }
      break;
    
    default: y=transc(gash,x,prec);
    }
  return y;
}

void gashz(GEN x, GEN y)
{
  long    av,prec;
  GEN     p;

  prec=precision(y);
  if(!prec) err(asher3);
  av=avma;p=gash(x,prec);
  gaffect(p,y);avma=av;
}

/********************************************************************/
/********************************************************************/
/**                                                                **/
/**            FONCTION ARGUMENT COSINUS HYPERBOLIQUE              **/
/**                                                                **/
/********************************************************************/
/********************************************************************/

GEN     mpach(GEN x)
{
  long    l,av;
  GEN     y,p1;

  if ((typ(x)!=2) || (gcmpgs(x,1)<0)) err(acher1);
  l=lg(x);y=cgetr(l);
  av=avma;
  p1=cgetr(l+1);
  affrr(x,p1);
  mulrrz(p1,p1,p1);
  subrsz(p1,1,p1);
  affrr(mpsqrt(p1),p1);
  addrrz(x,p1,p1);
  affrr(mplog(p1),y);
  avma=av;
  return y;
}

GEN     gach(GEN x, long prec)
{
  long    av,tetpil,v;
  GEN     y,p1;

  switch(typ(x))
    {
    case 2 : if(gcmpgs(x,1)>=0) y=mpach(x);
    else
      {
        y=cgetg(3,6);
        if(gcmpgs(x,-1)>=0)
          {
            y[2]=lmpacos(x);y[1]=zero;
          }
        else
          {
            av=avma;p1=mpach(gneg(x));tetpil=avma;
            y[1]=lpile(av,tetpil,gneg(p1));
            y[2]=lmppi(lg(x));
          }
      }
      break;
    
    case 6 : av=avma;p1=gaddsg(-1,gmul(x,x));
      p1=gadd(x,gsqrt(p1,prec));tetpil=avma;
      y=glog(p1,prec);
      if(signe((GEN)y[2])<0)
        {
          tetpil=avma;y=gneg(y);
        }
      y=gerepile(av,tetpil,y);
      break;
    
    case 3 :
    case 7 : err(acher2);
    
    case 11: av=avma;if(valp(x)<0) err(acher4);
      v=varn(x);p1=gdiv(deriv(x,v),gsqrt(gsubgs(gmul(x,x),1),prec));
      if(gcmp1((GEN)x[2])&&(!valp(x)))
        {tetpil=avma;y=gerepile(av,tetpil,integ(p1,v));}
      else
        {
          y=integ(p1,v);
          if(valp(x))
            {
              p1=cgetg(3,6);p1[1]=zero;p1[2]=lmppi(prec);
              setexpo((GEN)p1[2],0);
            }
          else p1=gach((GEN)x[2],prec);
          tetpil=avma;y=gerepile(av,tetpil,gadd(p1,y));
        }
      break;
    
    default: y=transc(gach,x,prec);
    }
  return y;
}

void gachz(GEN x, GEN y)
{
  long    av,prec;
  GEN     p;

  prec=precision(y);
  if(!prec) err(acher3);
  av=avma;p=gach(x,prec);
  gaffect(p,y);avma=av;
}

/********************************************************************/
/********************************************************************/
/**                                                                **/
/**           FONCTION ARGUMENT TANGENTE HYPERBOLIQUE              **/
/**                                                                **/
/********************************************************************/
/********************************************************************/

GEN     mpath(GEN x)
{
  long    av,tetpil;
  GEN     y,p1;

  if (typ(x)!=2) err(ather1);
  if (!signe(x))
    {
      y=cgetr(3);y[1]=x[1];y[2]=0;
    }
  else
    {
      av=avma;
      p1=addrs(divsr(2,subsr(1,x)),-1);
      tetpil=avma;y=gerepile(av,tetpil,mplog(p1));
      setexpo(y,expo(y)-1);
    }
  return y;
}

GEN     gath(GEN x, long prec)
{
  long    av,tetpil,v;
  GEN     y,p1;

  switch(typ(x))
    {
    case 2 : if(expo(x)<0) y=mpath(x);
    else
      {
        av=avma;p1=addrs(divsr(2,addsr(-1,x)),1);
        tetpil=avma;y=cgetg(3,6);
        p1=mplog(p1);setexpo(p1,expo(p1)-1);
        y[1]=(long)p1;
        y[2]=lmppi(lg(x));setexpo((GEN)y[2],0);
        y=gerepile(av,tetpil,y);
      }
      break;
    
    case 6 : av=avma;
      p1=gaddgs(gdivsg(2,gsubsg(1,x)),-1);
      p1=glog(p1,prec);tetpil=avma;
      y=gerepile(av,tetpil,gmul2n(p1,-1));
      break;
    
    case 3 :
    case 7 : err(ather2);
    
    case 11: av=avma;if(valp(x)<0) err(ather4);
      v=varn(x);p1=gdiv(deriv(x,v),gsubsg(1,gmul(x,x)));
      if(valp(x)) {tetpil=avma;y=gerepile(av,tetpil,integ(p1,v));}
      else
        {
          y=integ(p1,v);p1=gath((GEN)x[2],prec);tetpil=avma;
          y=gerepile(av,tetpil,gadd(p1,y));
        }
      break;
    
    default: y=transc(gath,x,prec);
    }
  return y;
}

void gathz(GEN x, GEN y)
{
  long    av,prec;
  GEN     p;

  prec=precision(y);
  if(!prec) err(ather3);
  av=avma;p=gath(x,prec);
  gaffect(p,y);avma=av;
}

/********************************************************************/
/********************************************************************/
/**                                                                **/
/**             FONCTION TABLEAU DES NOMBRES DE BERNOULLI          **/
/**                                                                **/
/********************************************************************/
/********************************************************************/

void mpbern(long nomb, long prec)
    
                        /* pour calculer B_0,B_2,...,B_2*nomb */
    
{
  long    n,m,i,j,d1,d2,av;
  GEN     p1,p2;

  if(nomb<0) nomb=0;
  if (bernzone)
    {
      if ((bernzone[1]>=nomb)&&(bernzone[2]>=prec)) return;
      killbloc(bernzone);
    }
  bernzone=newbloc(3+prec*(nomb+1));
  bernzone[1]=nomb;
  bernzone[2]=prec;
  av=avma;
  p1=cgetr(prec+1);p2=cgetr(prec+1);
  *((GEN)bern(0))=evaltyp(2)+evallg(prec);
  affsr(1,bern(0));

  for (i=1;i<=nomb;i++)
    {
      *((GEN)bern(i))=evaltyp(2)+evallg(prec);
      affsr(0,p1);affsr(4,p2);
      n=8;m=5;d1=i-1;d2=2*i-3;
      for (j=i-1;j>0;--j)
        {
          addrrz(bern(j),p1,p1);
          mulsrz(n*m,p1,p1);
          divrsz(p1,d1*d2,p1);
          mulsrz(4,p2,p2);
          n+=4;m+=2;d1--;d2-=2;
        }
      addsrz(1,p1,p1);
      divrsz(p1,2*i+1,p1);
      subsrz(1,p1,p1);
      divrrz(p1,p2,bern(i));
    }
  avma=av;
}

GEN bernreal(long n, long prec)
{
  long n1; GEN p1;
  
  if(n==1) {affsr(-1,p1=cgetr(prec));setexpo(p1,-1);return p1;}
  if((n<0)||(n&1)) return gzero;
  n1=n>>1;mpbern(n1+1,prec);
  p1=cgetr(prec);affrr(bern(n1),p1);
  return p1;
}

GEN bernvec(long nomb)
    
                   /* pour calculer le vecteur B_0,B_2,...,B_2*nomb */
    
{
  long    n,m,i,j,d1,d2,av,tetpil;
  GEN     p1,p2,y;

  y=cgetg(nomb+2,17);y[1]=un;
  for (i=1;i<=nomb;i++)
    {
      av=avma;
      p1=gzero;p2=stoi(4);
      n=8;m=5;d1=i-1;d2=2*i-3;
      for (j=i-1;j>0;--j)
        {
          p1=gdivgs(gmulsg(n*m,gadd(p1,(GEN)y[j+1])),d1*d2);
          p2=shifti(p2,2);
          n+=4;m+=2;d1--;d2-=2;
        }
      p1=gsubsg(1,gdivgs(gaddsg(1,p1),2*i+1));
      tetpil=avma;y[i+1]=lpile(av,tetpil,gdiv(p1,p2));
    }
  return y;
}

/********************************************************************/
/********************************************************************/
/**                                                                **/
/**                      FONCTION GAMMA                            **/
/**                                                                **/
/********************************************************************/
/********************************************************************/

GEN     mpgamma(GEN x)
{
  long    l,l1,l2,u,i,k,e,s,s1,n,p,av,av1;
  double  alpha,beta,dk;
  GEN     y,p1,p2,p3,p4,p5,p6,p7,p9,p91,pitemp;

  if (typ(x)!=2) err(gamer1);
  l=lg(x);y=cgetr(l);s1=signe(x);if (!s1) err(gamer2);
  av=avma;p1=cgetr(l+1);u=expo(x);
  if ((u<-1) || (s1<0))
    {
      av1=avma;p2=gfrac(x);
      if(gcmp0(p2)) err(gamer2);
      avma=av1;subsrz(1,x,p1);
    }
  else affrr(x,p1);
  alpha=rtodbl(p1);beta=((BITS_IN_LONG/2)*LOG2*(l-2)/PI)-alpha;
  if (beta>=0) n=(long)(1+K2*beta);else n=0;
  if(n) {p=(long)(1+PI*(alpha+n));l2=l+1+(n>>TWOPOTBITS_IN_LONG);}
  else
    {
      dk=K4*alpha/(l-2);beta=log(dk)/LOG2;
      if(beta>1.) beta+=(log(beta)/LOG2);
      p=(long)((BITS_IN_LONG/2)*(l-2)/beta+1);l2=l+1;
    }
  mpbern(p,l2);p91=cgetr(l2);
  p2=cgetr(l2);p3=cgetr(l2);p4=cgetr(l2);p5=cgetr(l2);p6=cgetr(l2);
  if(n) addsrz(n,p1,p2);else p2=p1;affrr(mplog(p2),p3);
  affsr(1,p4);setexpo(p4,expo(p4)-1);
  subrrz(p2,p4,p4);
  mulrrz(p4,p3,p4);subrrz(p4,p2,p4);
  p7=mppi(l2);setexpo(p7,2);affrr(mplog(p7),p3);setexpo(p3,-1);
  addrrz(p4,p3,p4);mulrrz(p2,p2,p3);divsrz(1,p3,p3);e=expo(p3);
  setlg(p3,4);setlg(p5,4);setlg(p6,4);
  if(bernzone[2]>l2) {affrr(bern(p),p91);p9=p91;} else p9=(GEN)bern(p);
  divrsz(p9,2*p*(2*p-1),p5);
  s=0;l1=2;l2-=2;
  for (k=p;k>1;k--)
    {
      mulrrz(p3,p5,p5);
      if(bernzone[2]>l2) {affrr(bern(k-1),p91);p9=p91;} else p9=(GEN)bern(k-1);
      divrsz(p9,(2*k-2)*(2*k-3),p6);
      s-=e;l1+=(s>>TWOPOTBITS_IN_LONG);if (l1>l2) l1=l2;
      s &= (BITS_IN_LONG-1);
      setlg(p3,l1+2);setlg(p5,l1+2);setlg(p6,l1+2);
      addrrz(p6,p5,p5);
    }
  setlg(p5,l2+2);
  divrrz(p5,p2,p5);addrrz(p4,p5,p4);affrr(mpexp(p4),p4);
  for (i=1;i<=n;i++)
    {
      subrsz(p2,1,p2);divrrz(p4,p2,p4);
    }
  if ((u<-1)||(s1<0))
    {
      pitemp=mppi(l+1);mulrrz(pitemp,x,p1);affrr(mpsin(p1),p1);
      mulrrz(p1,p4,p4);divrrz(pitemp,p4,y);
    }
  else affrr(p4,y);
  avma=av;return y;
}

GEN     cxgamma(GEN x, long prec)
{
  long    l,l1,l2,u,i,k,e,s,s1,n,p,av;
  double  alpha,beta,dk;
  GEN     y,p1,p2,p3,p4,p5,p6,p7,p9,p91,pitemp;

  if (typ(x)!=6) err(gamer1);
  l=(LGBITS>>2);if(typ((GEN)x[1])==2) l=precision((GEN)x[1]);
  if(typ((GEN)x[2])==2) {l1=precision((GEN)x[2]);if(l1<l) l=l1;}
  if(l==(LGBITS>>2)) l=prec;
  y=cgetg(3,6);y[1]=lgetr(l);y[2]=lgetr(l);
  s1=gsigne((GEN)x[1]);av=avma;
  p1=cgetg(3,6);p1[1]=lgetr(l+1);p1[2]=lgetr(l+1);
  if(s1||(typ((GEN)x[1])==2)) u=gexpo((GEN)x[1]);else u= -2;
  if ((s1<=0)||(u<-1)) gsubsgz(1,x,p1);
  else gaffect(x,p1);
  alpha=rtodbl(gabs(p1,4));beta=((BITS_IN_LONG/2)*LOG2*(l-2)/PI)-alpha;
  if (beta>=0) n=(long)(1+K2*beta);else n=0;
  if(n) {p=(long)(1+PI*(alpha+n));l2=l+1+(n>>TWOPOTBITS_IN_LONG);}
  else
    {
      dk=K4*alpha/(l-2);beta=log(dk)/LOG2;
      if(beta>1.) beta+=(log(beta)/LOG2);
      p=(long)((BITS_IN_LONG/2)*(l-2)/beta+1);l2=l+1;
    }
  mpbern(p,l2);p91=cgetr(l2);
  p2=cgetg(3,6);p2[1]=lgetr(l2);p2[2]=lgetr(l2);
  p3=cgetg(3,6);p3[1]=lgetr(l2);p3[2]=lgetr(l2);
  p4=cgetg(3,6);p4[1]=lgetr(l2);p4[2]=lgetr(l2);
  p5=cgetg(3,6);p5[1]=lgetr(l2);p5[2]=lgetr(l2);
  p6=cgetr(l2);
  if(n) {addsrz(n,(GEN)p1[1],(GEN)p2[1]);affrr((GEN)p1[2],(GEN)p2[2]);} else p2=p1;
  gaffect(glog(p2,l2),p3);
  affsr(1,(GEN)p4[1]);setexpo((GEN)p4[1],-1);
  subrrz((GEN)p2[1],(GEN)p4[1],(GEN)p4[1]);p4[2]=lcopy((GEN)p2[2]);
  gmulz(p4,p3,p4);gsubz(p4,p2,p4);
  p7=mppi(l2);setexpo(p7,2);affrr(mplog(p7),p6);setexpo(p6,-1);
  addrrz((GEN)p4[1],p6,(GEN)p4[1]);gmulz(p2,p2,p3);gdivsgz(1,p3,p3);e=gexpo(p3);
  setlg((GEN)p3[1],4);setlg((GEN)p5[1],4);setlg(p6,4);setlg((GEN)p3[2],4);setlg((GEN)p5[2],4);
  if(bernzone[2]>l2) {affrr(bern(p),p91);p9=p91;} else p9=(GEN)bern(p);
  gdivgsz(p9,2*p*(2*p-1),p5);
  s=0;l1=2;l2-=2;
  for (k=p;k>1;k--)
    {
      gmulz(p3,p5,p5);
      if(bernzone[2]>l2) {affrr(bern(k-1),p91);p9=p91;} else p9=(GEN)bern(k-1);
      divrsz(p9,(2*k-2)*(2*k-3),p6);
      s-=e;l1+=(s>>TWOPOTBITS_IN_LONG);if (l1>l2) l1=l2;
      s &= (BITS_IN_LONG-1);
      setlg((GEN)p3[1],l1+2);setlg((GEN)p5[1],l1+2);setlg(p6,l1+2);
      setlg((GEN)p3[2],l1+2);setlg((GEN)p5[2],l1+2);
      addrrz(p6,(GEN)p5[1],(GEN)p5[1]);
    }
  setlg((GEN)p5[1],l2+2);setlg((GEN)p5[2],l2+2);
  gdivz(p5,p2,p5);gaddz(p4,p5,p4);gaffect(gexp(p4,l),p4);
  for (i=1;i<=n;i++)
    {
      subrsz((GEN)p2[1],1,(GEN)p2[1]);gdivz(p4,p2,p4);
    }
  if ((s1<=0)||(u<-1))
    {
      pitemp=mppi(l+1);gmulz(pitemp,x,p1);gaffect(gsin(p1,l+1),p1);
      gmulz(p1,p4,p4);gdivz(pitemp,p4,y);
    }
  else gaffect(p4,y);
  avma=av;return y;
}
    
GEN     ggamma(GEN x, long prec)
{
  long    i,lx;
  GEN     y;

    switch(typ(x))
      {
      case 1: if(signe(x)<=0) err(gamer2);
	y=transc(ggamma,x,prec);break;
      case 2 : y=mpgamma(x);break;
      case 6 : y=(gcmp0((GEN)x[2])) ? ggamma((GEN)x[1],prec) : cxgamma(x,prec);break;
      case 7 : err(impl,"p-adic gamma function");
      case 3 : err(gamer3);
      case 11: y=gexp(glngamma(x,prec),prec);break;
      case 17:
      case 18:
      case 19: lx=lg(x);y=cgetg(lx,typ(x));
	for(i=1;i<lx;i++)
	  y[i]=lgamma((GEN)x[i],prec);
	break;
      default: y=transc(ggamma,x,prec);
      }
  return y;
}

void ggammaz(GEN x, GEN y)
{
  long    av,prec;
  GEN     p;

  prec=precision(y);
  if(!prec) err(gamer4);
  av=avma;p=ggamma(x,prec);
  gaffect(p,y);avma=av;
}

GEN     mplngamma(GEN x)
{
  long    l,l1,l2,u,i,k,e,f,s,s1,n,p,av,av1;
  double  alpha,beta,dk;
  GEN     y,p1,p2,p3,p4,p5,p6,p7,p8,p9,p91,pitemp;

  if (typ(x)!=2) err(gamer1);
  l=lg(x);y=cgetr(l);s1=signe(x);if (!s1) err(gamer2);
  av=avma;p1=cgetr(l+1);u=expo(x);
  if ((u<-1) || (s1<0))
    {
      av1=avma;p2=gfrac(x);
      if(gcmp0(p2)) err(gamer2);
      avma=av1;subsrz(1,x,p1);
    }
  else affrr(x,p1);
  if(expo(p1)>1000) 
    {
      n=0;beta=log(K4/(l-2))/LOG2+expo(p1);beta+=(log(beta)/LOG2);
      p=(long)((BITS_IN_LONG/2)*(l-2)/beta+1);l2=l+1;
    }
  else
    {
      alpha=rtodbl(p1);beta=((BITS_IN_LONG/2)*LOG2*(l-2)/PI)-alpha;
      if (beta>=0) n=(long)(1+K2*beta);else n=0;
      if(n) {p=(long)(1+PI*(alpha+n));l2=l+1+(n>>TWOPOTBITS_IN_LONG);}
      else
	{
	  dk=K4*alpha/(l-2);beta=log(dk)/LOG2;
	  if(beta>1.) beta+=(log(beta)/LOG2);
	  p=(long)((BITS_IN_LONG/2)*(l-2)/beta+1);l2=l+1;
	}
    }
  mpbern(p,l2);p91=cgetr(l2);
  p2=cgetr(l2);p3=cgetr(l2);p4=cgetr(l2);p5=cgetr(l2);p6=cgetr(l2);
  p8=cgetr(l2);if(n) addsrz(n,p1,p2);else p2=p1;affrr(mplog(p2),p3);
  affsr(1,p4);affsr(1,p8);setexpo(p4,expo(p4)-1);
  subrrz(p2,p4,p4);mulrrz(p4,p3,p4);subrrz(p4,p2,p4);
  p7=mppi(l2);setexpo(p7,2);affrr(mplog(p7),p3);setexpo(p3,-1);
  addrrz(p4,p3,p4);mulrrz(p2,p2,p3);divsrz(1,p3,p3);e=expo(p3);
  setlg(p3,4);setlg(p5,4);setlg(p6,4);
  if(bernzone[2]>l2) {affrr(bern(p),p91);p9=p91;} else p9=(GEN)bern(p);
  divrsz(p9,2*p*(2*p-1),p5);
  s=0;l1=2;l2-=2;
  for (k=p;k>1;k--)
    {
      mulrrz(p3,p5,p5);
      if(bernzone[2]>l2) {affrr(bern(k-1),p91);p9=p91;} else p9=(GEN)bern(k-1);
      divrsz(p9,(2*k-2)*(2*k-3),p6);
      s-=e;l1+=(s>>TWOPOTBITS_IN_LONG);if (l1>l2) l1=l2;
      s &= (BITS_IN_LONG-1);
      setlg(p3,l1+2);setlg(p5,l1+2);setlg(p6,l1+2);
      addrrz(p6,p5,p5);
    }
  setlg(p5,l2+2);
  divrrz(p5,p2,p5);addrrz(p4,p5,p4);
  for (i=1;i<=n;i++)
    {
      subrsz(p2,1,p2);mulrrz(p8,p2,p8);
    }
  f=signe(p8);subrrz(p4,mplog((f>0)?p8:negr(p8)),p4);
  if ((u<-1)||(s1<0))
    {
      pitemp=mppi(l+1);mulrrz(pitemp,x,p1);divrrz(pitemp,mpsin(p1),p1);
      f*=signe(p1);subrrz(mplog(absr(p1)),p4,y);
    }
  else affrr(p4,y);
  avma=av;if(f<0) {p2=cgetg(3,6);p2[1]=(long)y;p2[2]=(long)mppi(l);return p2;}
  else return y;
}

GEN     cxlngamma(GEN x, long prec)
{
  long    l,l1,l2,u,i,k,e,s,s1,n,p,av,flag;
  double  alpha,beta,dk;
  GEN     y,p1,p2,p3,p4,p5,p6,p7,p8,p9,p91,pitemp;

  if (typ(x)!=6) err(gamer1);
  l=(LGBITS>>2);if(typ((GEN)x[1])==2) l=precision((GEN)x[1]);
  if(typ((GEN)x[2])==2) {l1=precision((GEN)x[2]);if(l1<l) l=l1;}
  if(l==(LGBITS>>2)) l=prec;
  y=cgetg(3,6);y[1]=lgetr(l);y[2]=lgetr(l);
  s1=gsigne((GEN)x[1]);av=avma;
  p1=cgetg(3,6);p1[1]=lgetr(l+1);p1[2]=lgetr(l+1);
  if(s1||(typ((GEN)x[1])==2)) u=gexpo((GEN)x[1]);else u= -2;
  if (((s1<=0)||(u<-1))&&(!gcmp0((GEN)x[2])&&(gexpo((GEN)x[2])<=16)))
    {gsubsgz(1,x,p1);flag=1;}
  else {gaffect(x,p1);flag=0;}
  p2=gabs(p1,4);
  if(expo(p2)>1000) 
    {
      n=0;beta=log(K4/(l-2))/LOG2+expo(p1);beta+=(log(beta)/LOG2);
      p=(long)((BITS_IN_LONG/2)*(l-2)/beta+1);l2=l+1;
    }
  else
    {
      alpha=rtodbl(p2);beta=((BITS_IN_LONG/2)*LOG2*(l-2)/PI)-alpha;
      if (beta>=0) n=(long)(1+K2*beta);else n=0;
      if(n) {p=(long)(1+PI*(alpha+n));l2=l+1+(n>>TWOPOTBITS_IN_LONG);}
      else
	{
	  dk=K4*alpha/(l-2);beta=log(dk)/LOG2;
	  if(beta>1.) beta+=(log(beta)/LOG2);
	  p=(long)((BITS_IN_LONG/2)*(l-2)/beta+1);l2=l+1;
	}
    }
  mpbern(p,l2);p91=cgetr(l2);
  p2=cgetg(3,6);p2[1]=lgetr(l2);p2[2]=lgetr(l2);
  p3=cgetg(3,6);p3[1]=lgetr(l2);p3[2]=lgetr(l2);
  p4=cgetg(3,6);p4[1]=lgetr(l2);p4[2]=lgetr(l2);
  p5=cgetg(3,6);p5[1]=lgetr(l2);p5[2]=lgetr(l2);
  p8=cgetg(3,6);p8[1]=lgetr(l2);p8[2]=lgetr(l2);
  gaffsg(1,p8);p6=cgetr(l2);
  if(n) {addsrz(n,(GEN)p1[1],(GEN)p2[1]);affrr((GEN)p1[2],(GEN)p2[2]);} else p2=p1;
  gaffect(glog(p2,l2),p3);
  affsr(1,(GEN)p4[1]);setexpo((GEN)p4[1],-1);
  subrrz((GEN)p2[1],(GEN)p4[1],(GEN)p4[1]);p4[2]=lcopy((GEN)p2[2]);
  gmulz(p4,p3,p4);gsubz(p4,p2,p4);
  p7=mppi(l2);setexpo(p7,2);affrr(mplog(p7),p6);setexpo(p6,-1);
  addrrz((GEN)p4[1],p6,(GEN)p4[1]);gmulz(p2,p2,p3);gdivsgz(1,p3,p3);e=gexpo(p3);
  setlg((GEN)p3[1],4);setlg((GEN)p5[1],4);setlg(p6,4);setlg((GEN)p3[2],4);setlg((GEN)p5[2],4);
  if(bernzone[2]>l2) {affrr(bern(p),p91);p9=p91;} else p9=(GEN)bern(p);
  gdivgsz(p9,2*p*(2*p-1),p5);
  s=0;l1=2;l2-=2;
  for (k=p;k>1;k--)
    {
      gmulz(p3,p5,p5);
      if(bernzone[2]>l2) {affrr(bern(k-1),p91);p9=p91;} else p9=(GEN)bern(k-1);
      divrsz(p9,(2*k-2)*(2*k-3),p6);
      s-=e;l1+=(s>>TWOPOTBITS_IN_LONG);if (l1>l2) l1=l2;
      s &= (BITS_IN_LONG-1);
      setlg((GEN)p3[1],l1+2);setlg((GEN)p5[1],l1+2);setlg(p6,l1+2);
      setlg((GEN)p3[2],l1+2);setlg((GEN)p5[2],l1+2);
      addrrz(p6,(GEN)p5[1],(GEN)p5[1]);
    }
  setlg((GEN)p5[1],l2+2);setlg((GEN)p5[2],l2+2);
  gdivz(p5,p2,p5);gaddz(p4,p5,p4);
  for (i=1;i<=n;i++)
    {
      subrsz((GEN)p2[1],1,(GEN)p2[1]);gmulz(p8,p2,p8);
    }
  gsubz(p4,glog(p8,l+1),p4);
  if (flag)
    {
      pitemp=mppi(l+1);gmulz(pitemp,x,p1);gdivz(pitemp,gsin(p1,l+1),p1);
      gsubz(glog(p1,l+1),p4,y);
    }
  else gaffect(p4,y);
  avma=av;return y;
}
    
GEN     glngamma(GEN x, long prec)
{
  long    i,lx,av,tetpil,v,n;
  GEN     y,p1;

    switch(typ(x))
      {
      case 1: if(signe(x)<=0) err(gamer2);
	y=transc(glngamma,x,prec);break;
      case 2 : y=mplngamma(x);break;
      case 6 : y=(gcmp0((GEN)x[2])) ? glngamma((GEN)x[1],prec) : cxlngamma(x,prec);break;
      case 7 : err(impl,"p-adic lngamma function");
      case 3 : err(gamer3);
      case 11: av=avma;if(valp(x)) err(loger5);
	v=varn(x);if(!gcmp1((GEN)x[2])) err(impl,"lngamma around a!=1");
	p1=gsubsg(1,x);n=(lg(x)-3)/valp(p1);
	y=ggrando(polx[v],lg(x)-2);
	for(i=n;i>=2;i--)
	  {
	    y=gmul(p1,gadd(gdivgs(izeta(stoi(i),prec),i),y));
	  }
	y=gadd(mpeuler(prec),y);tetpil=avma;
	y=gerepile(av,tetpil,gmul(p1,y));break;
      case 17:
      case 18:
      case 19: lx=lg(x);y=cgetg(lx,typ(x));
	for(i=1;i<lx;i++)
	  y[i]=(long)glngamma((GEN)x[i],prec);
	break;
      default: y=transc(glngamma,x,prec);
      }
  return y;
}

void glngammaz(GEN x, GEN y)
{
  long    av,prec;
  GEN     p;

  prec=precision(y);
  if(!prec) err(gamer4);
  av=avma;p=glngamma(x,prec);
  gaffect(p,y);avma=av;
}

/********************************************************************/
/********************************************************************/
/**                                                                **/
/**             FONCTION GAMMA DES DEMI-ENTIERS                    **/
/**                                                                **/
/********************************************************************/
/********************************************************************/

GEN     mpgamd(long x, long prec)
{
  long    i,j,a,l,av;
  GEN     y,p1;

  a=labs(x);
  l=prec+1+(a>>TWOPOTBITS_IN_LONG);
  if (l>(LGBITS>>1)) err(gamder1);
  y=cgetr(prec);
  av=avma;
  p1=mpsqrt(mppi(l));
  if (x>=0)
    {
      j= -1;
      for (i=0;i<x;i++)
        {
          j+=2;mulsrz(j,p1,p1);
          setexpo(p1,expo(p1)-1);
        }
    }
  else
    {
      j=1;
      for (i=0;i<a;i++)
        {
          j-=2;divrsz(p1,j,p1);
          setexpo(p1,expo(p1)+1);
        }
    }
  affrr(p1,y);
  avma=av;
  return y;
}

void mpgamdz(long s, GEN y)
{
  long    l,av;
  GEN     p1;

  av=avma;
  l=lg(y);
  p1=mpgamd(s,l);
  affrr(p1,y);
  avma=av;
}

GEN     ggamd(GEN x, long prec)
{
  long    av,tetpil,i,lx;
  GEN     y,p1;

  switch(typ(x))
    {
    case 1 : y=mpgamd(itos(x),prec);break;
    case 2 :
    case 4 :
    case 5 :
    case 6 :
    case 8 : av=avma;p1=gadd(x,ghalf);tetpil=avma;
      y=gerepile(av,tetpil,ggamma(p1,prec));
      break;
    case 17:
    case 18:
    case 19: lx=lg(x);y=cgetg(lx,typ(x));
      for(i=1;i<lx;i++)
        y[i]=lgamd((GEN)x[i],prec);
      break;
    case 3 :
    case 7 : err(gamder2);
    case 11: err(impl,"gamd of a power series");
    default: y=transc(ggamd,x,prec);
    }
  return y;
}

void ggamdz(GEN x, GEN y)
{
  long    av,prec;
  GEN     p;

  prec=precision(y);
  if(!prec) err(gamder3);
  av=avma;p=ggamd(x,prec);
  gaffect(p,y);avma=av;
}


/********************************************************************/
/********************************************************************/
/**                                                                **/
/**                      FONCTION PSI                              **/
/**                                                                **/
/********************************************************************/
/********************************************************************/


GEN     mppsi(GEN z)        /*   version p=2 */
{
  long  l,n,k,x,xx,av1,av2,tetpil;
  GEN   zk,u,v,a,b;

  av1=avma;l=lg(z);
  x=(long)(1+(BITS_IN_LONG/2)*(l-2)*LOG2+1.58*rtodbl(absr(z)));xx=x*x;
  n=(long)(1+3.591*x);
  affsr(x,a=cgetr(l));
  a=mplog(a);
  gaffect(a,u=cgetr(l));
  gaffsg(1,b=cgetr(l));
  gaffsg(1,v=cgetr(l));
  for (k=1;k<=n;k++)
    {
      av2=avma;
      zk=(k>1) ? gaddsg(k-1,z) : z;
      gdivz(gmulsg(xx,b),gmul(zk,zk),b);
      gdivz(gsub(gdiv(gmulsg(xx,a),zk),b),zk,a);
      gaddz(u,a,u);gaddz(v,b,v);
      avma=av2;
    }
  tetpil=avma;return gerepile(av1,tetpil,gdiv(u,v));
}

GEN     cxpsi(GEN z, long prec)        /*   version p=2 */
                  
               

{
  long  l,l1,n,k,x,xx,av1,av2,tetpil;
  GEN   zk,u,v,a,b;

  l=(LGBITS>>2);if(typ((GEN)z[1])==2) l=precision((GEN)z[1]);
  if(typ((GEN)z[2])==2) {l1=precision((GEN)z[2]);if(l1<l) l=l1;}
  if(l==(LGBITS>>2)) l=prec;
  av1=avma;x=(long)(1+(BITS_IN_LONG/2)*(l-2)*LOG2+1.58*rtodbl(gabs(z,4)));xx=x*x;
  n=(long)(1+3.591*x);
  a=cgetg(3,6);a[1]=lgetr(l);a[2]=lgetr(l);gaffsg(x,a);
  b=cgetg(3,6);b[1]=lgetr(l);b[2]=lgetr(l);gaffsg(1,b);
  u=cgetg(3,6);u[1]=lgetr(l);u[2]=lgetr(l);
  v=cgetg(3,6);v[1]=lgetr(l);v[2]=lgetr(l);gaffsg(1,v);
  a=glog(a,l);gaffect(a,u);
  for (k=1;k<=n;k++)
    {
      av2=avma;
      zk=(k>1) ? gaddsg(k-1,z) : z;
      gdivz(gmulsg(xx,b),gmul(zk,zk),b);
      gdivz(gsub(gdiv(gmulsg(xx,a),zk),b),zk,a);
      gaddz(u,a,u);gaddz(v,b,v);
      avma=av2;
    }
  tetpil=avma;return gerepile(av1,tetpil,gdiv(u,v));
}



GEN     gpsi(GEN x, long prec)
{
  long    i,lx;
  GEN     y;

  switch(typ(x))
    {
    case 2 : y=mppsi(x);break;
    case 6 : y=cxpsi(x,prec);break;
    case 3 :
    case 7 : err(psier1);
    case 11: err(impl,"psi of power series");
    case 17:
    case 18:
    case 19: lx=lg(x);y=cgetg(lx,typ(x));
      for(i=1;i<lx;i++)
        y[i]=lpsi((GEN)x[i],prec);
      break;
    default: y=transc(gpsi,x,prec);
    }
  return y;
}

void gpsiz(GEN x, GEN y)
{
  long    av,prec;
  GEN     p;

  prec=precision(y);
  if(!prec) err(psier2);
  av=avma;p=gpsi(x,prec);
  gaffect(p,y);avma=av;
}
