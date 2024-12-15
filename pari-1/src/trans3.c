/********************************************************************/
/********************************************************************/
/**                                                                **/
/**                +++++++++++++++++++++++++++++++                 **/
/**                +                             +                 **/
/**                +  FONCTIONS TRANSCENDANTES   +                 **/
/**                +     (troisieme partie)      +                 **/
/**                +                             +                 **/
/**                +     copyright Babe Cool     +                 **/
/**                +                             +                 **/
/**                +++++++++++++++++++++++++++++++                 **/
/**                                                                **/
/**                                                                **/
/********************************************************************/
/********************************************************************/

# include "genpari.h"

GEN qq(GEN x, long prec),inteta(GEN q);

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~                                                                     ~*/
/*~                     FONCTION K DE BESSEL                            ~*/
/*~                                                                     ~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

GEN     kbessel(GEN nu, GEN gx, long prec)
{
  GEN  x,y,p1,p2,zf,zz,s,t,q,r,u,v,e,f,c,d,ak,pitemp;
  GEN   nu2,w;
  long  l,lbin,av,av1,av2,k,k2,l1,n2,n,ex;

  if(typ(nu)==6) return kbessel2(nu,gx,prec);
  if(typ(gx)!=2) {l=prec;k=1;}
  else {l=lg(gx);k=0;x=gx;} 
  y=cgetr(l);l1=l+1;
  av=avma;if(k) gaffect(gx,x=cgetr(l));
  u=cgetr(l1);v=cgetr(l1);c=cgetr(l1);d=cgetr(l1);
  e=cgetr(l1);f=cgetr(l1);
  nu2=gmulgs(gmul(nu,nu),-4);
  n=(long)((BITS_IN_LONG/2)*(l-2)*LOG2+PI*sqrt(gtodouble(gnorm(nu)))/2);
  n2=(n<<1);pitemp=mppi(l1);
  lbin=10-((l-2)<<TWOPOTBITS_IN_LONG);av1=avma;
  if (gcmpgs(x,n)<0)
    {
      zf=gsqrt(gdivgs(pitemp,n2),prec);
      zz=cgetr(l1);gdivgsz(gun,(n2<<2),zz);
      s=gun;t=gzero;k2=2*n2+1;
      for (k=n2;k>0;--k)
        {
          k2-=2;
          if(k2>(MAXHALFULONG>>1)) p1=gadd(mulss(k2,k2),nu2);
          else p1=gaddsg(k2*k2,nu2);
          ak=gdivgs(gmul(p1,zz),-k);s=gaddsg(1,gmul(ak,s));
          t=gaddsg(k2,gmul(ak,t));
        }
      gmulz(s,zf,u);t=gmul2n(t,-1);
      gdivgsz(gadd(gmul(t,zf),gmul(u,nu)),-n2,v);avma=av1;
      affsr(n2,q=cgetr(l1));
      r=gmul2n(x,1);av1=avma;
      do
        {
          p1=divsr(5,q);
          if (expo(p1)>= -1) p1=ghalf;
          p2=subsr(1,divrr(r,q));
          if (gcmp(p1,p2)>0) p1=p2;
          gnegz(p1,c);avma=av1;
          k=1;gaffsg(1,d);
          affrr(u,e);affrr(v,f);av2=avma;
          do
            {
              w=gadd(gmul(gsubsg(k,ghalf),u),gmul(gsubgs(q,k),v));
              w=gadd(w,gmul(nu,gsub(u,gmul2n(v,1))));
              gdivgsz(gmul(q,v),k,u);
              gdivgsz(w,k,v);
              gmulz(d,c,d);
              gaddz(e,gmul(d,u),e);
              gaddz(f,p1=gmul(d,v),f);
              k++;ex=gexpo(p1)-gexpo(f);
              avma=av2;
            }
          while(ex>lbin);
          p1=u;u=e;e=p1;p1=v;v=f;f=p1;
          gmulz(q,gaddsg(1,c),q);
          p1=subrr(q,r);ex=expo(p1);avma=av1;
        }
      while(ex>lbin);
      gmulz(u,gpui(gdivgs(x,n),nu,prec),y);
    }
  else
    {
      p2=gmul2n(x,1);
      zf=gsqrt(gdiv(pitemp,p2),prec);
      zz=gdiv(gun,gmul2n(p2,2));
      s=gun;k2=2*n2+1;
      for (k=n2;k>0;--k)
        {
          k2-=2;if(k2>(MAXHALFULONG>>1)) p1=gadd(mulss(k2,k2),nu2);
          else p1=gaddsg(k2*k2,nu2);
          ak=gdivgs(gmul(p1,zz),k);
          s=gsubsg(1,gmul(ak,s));
        }
      gmulz(s,zf,y);
    }
  gdivz(y,mpexp(x),y);avma=av;
  return y;
}


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~                                                                     ~*/
/*~                     FONCTION U(a,b,z) GENERALE                      ~*/
/*~                                                                     ~*/
/*~                         ET CAS PARTICULIERS                         ~*/
/*~                                                                     ~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

GEN     hyperu(GEN a, GEN b, GEN gx, long prec)
    
                 
               
    
     /* On suppose gx>0 et a,b reels   pour l'instant   */
{
  GEN  x,y,p1,p2,p3,zf,zz,s,t,q,r,u,v,e,f,c,d;
  GEN  w,a1,gn;
  long  l,lbin,av,av1,av2,k,l1,n,ex;

  if(typ(gx)!=2) {l=prec;k=1;}
  else {l=lg(gx);k=0;x=gx;} 
  ex=(iscomplex(a)||iscomplex(b));
  if(ex) {y=cgetg(3,6);y[1]=lgetr(l);y[2]=lgetr(l);}
  else y=cgetr(l);
  l1=l+1;av=avma;if(k) gaffect(gx,x=cgetr(l));
  a1=gaddsg(1,gsub(a,b));
  if(ex)
  {
    u=cgetg(3,6);u[1]=lgetr(l1);u[2]=lgetr(l1);
    v=cgetg(3,6);v[1]=lgetr(l1);v[2]=lgetr(l1);
    c=cgetg(3,6);c[1]=lgetr(l1);c[2]=lgetr(l1);
    d=cgetg(3,6);d[1]=lgetr(l1);d[2]=lgetr(l1);
    e=cgetg(3,6);e[1]=lgetr(l1);e[2]=lgetr(l1);
    f=cgetg(3,6);f[1]=lgetr(l1);f[2]=lgetr(l1);
  }
  else
  {
    u=cgetr(l1);v=cgetr(l1);c=cgetr(l1);
    d=cgetr(l1);e=cgetr(l1);f=cgetr(l1);
  }
  n=(long)(BITS_IN_LONG*(l-2)*LOG2+PI*sqrt(gtodouble(gabs(gmul(a,a1),l1))));
  lbin=10-((l-2)<<TWOPOTBITS_IN_LONG);av1=avma;
  if (gcmpgs(x,n)<0)
    {
      zf=gpui(gn=stoi(n),gneg(a),l1);
      zz=gdivsg(-1,gn);
      s=gun;t=gzero;
      for (k=n-1;k>=0;--k)
        {
          p1=gdivgs(gmul(gmul(gaddgs(a,k),gaddgs(a1,k)),zz),k+1);
          s=gaddsg(1,gmul(p1,s));
          t=gadd(gaddsg(k,a),gmul(p1,t));
        }
      gmulz(s,zf,u);t=gmul(t,zz);gmulz(t,zf,v);avma=av1;
      affsr(n,q=cgetr(l1));
      r=x;av1=avma;
      do
        {
          p1=divsr(5,q);
          if (expo(p1)>= -1) p1=ghalf;
          p2=subsr(1,divrr(r,q));
          if (gcmp(p1,p2)>0) p1=p2;
          gnegz(p1,c);avma=av1;
          k=0;gaffsg(1,d);
          gaffect(u,e);gaffect(v,f);
          p3=gsub(q,b);av2=avma;
          do
            {
              w=gadd(gmul(gaddsg(k,a),u),gmul(gaddsg(-k,p3),v));
              k++;gdivgsz(gmul(q,v),k,u);
              gdivgsz(w,k,v);
              gmulz(d,c,d);
              gaddz(e,gmul(d,u),e);
              gaddz(f,p1=gmul(d,v),f);
              ex=gexpo(p1)-gexpo(f);
              avma=av2;
            }
          while(ex>lbin);
          p1=u;u=e;e=p1;p1=v;v=f;f=p1;
          gmulz(q,gaddsg(1,c),q);
          p1=subrr(q,r);ex=expo(p1);avma=av1;
        }
      while(ex>lbin);
      gaffect(u,y);
    }
  else
    {
      zf=gpui(x,gneg(a),l1);
      zz=gdivsg(-1,x);
      s=gun;
      for (k=n-1;k>=0;--k)
        {
          p1=gdivgs(gmul(gmul(gaddgs(a,k),gaddgs(a1,k)),zz),k+1);
          s=gaddsg(1,gmul(p1,s));
        }
      gmulz(s,zf,y);
    }
  avma=av;
  return y;
}

GEN   kbessel2(GEN nu, GEN x, long prec)
{
  long av,tetpil,l;
  GEN  p1,p2,x2,a,pitemp;

  av=avma;x2=gshift(x,1);
  if(typ(x)==2) l=lg(x);else l=prec;
  pitemp=mppi(l);
  if(gcmp0(gimag(nu))) a=cgetr(l);
  else
    {a=cgetg(3,6);a[1]=lgetr(l);a[2]=lgetr(l);}
  gaddsgz(1,gshift(nu,1),a);
  p1=hyperu(gshift(a,-1),a,x2,prec);
  p1=gmul(gmul(p1,gpui(x2,nu,prec)),mpsqrt(pitemp));p2=gexp(x,prec);
  tetpil=avma;
  return gerepile(av,tetpil,gdiv(p1,p2));
}


GEN eint1(GEN x, long prec)
{
  long av,tetpil,l,n,i;
  GEN p1,p2,p3,p4,y;

  av=avma;
  if(typ(x)!=2) {gaffect(x,p1=cgetr(prec));x=p1;}
  if(expo(x)>=4)
    {
      tetpil=avma;y=gerepile(av,tetpil,incgam2(gzero,x,prec));
    }
  else
    {
      l=lg(x);
      n= -BITS_IN_LONG*(l-2)-1;
      p1=cgetr(l);affsr(1,p1);p2=p1;p3=p1;p4=p1;i=1;
      while(expo(p2)>=n)
        {
          i++;p1=gadd(p1,gdivgs(gun,i));p4=gdivgs(gmul(x,p4),i);
          p2=gmul(p4,p1);p3=gadd(p2,p3);
        }
      p3=gmul(x,gmul(mpexp(negr(x)),p3));
      consteuler(l);p1=gadd(mplog(x),geuler);tetpil=avma;
      y=gerepile(av,tetpil,gsub(p3,p1));
    }
  return y;
}

GEN   gerfc(GEN x, long prec)
{
  long av,tetpil,l;
  GEN p1,p2,x2,pitemp;

  av=avma;x2=gmul(x,x);p1=incgam(ghalf,x2,prec);
  if(typ(x)==2) l=lg(x);else l=prec;
  pitemp=mppi(l);
  p2=mpsqrt(pitemp);tetpil=avma;
  return gerepile(av,tetpil,gdiv(p1,p2));
}

GEN   incgam(GEN a, GEN x, long prec)
{
  long av,tetpil;
  GEN p1,p2,y;

  av=avma;
  if(typ(x)!=2) {gaffect(x,p1=cgetr(prec));x=p1;}
  if((gcmp(subrs(x,1),a)>0)||(gsigne(greal(a))<=0))
    {
      tetpil=avma;y=gerepile(av,tetpil,incgam2(a,x,prec));
    }
  else
    {
      p2=ggamma(a,prec);p1=incgam3(a,x,prec);tetpil=avma;
      y=gerepile(av,tetpil,gsub(p2,p1));
    }
  return y;
}

GEN  incgam1(GEN a, GEN x, long prec)
{
  long av=avma,tetpil,l,n,i;
  double m,mx;
  GEN p1,p2,p3,y;

  if(typ(x)!=2) {gaffect(x,p1=cgetr(prec));x=p1;}
  l=lg(x);mx=rtodbl(x);
  m=(long)(BITS_IN_LONG*(l-2)*LOG2);n=(long)(m/(log(m)-(1+log(mx))));
  gaffect(gaddsg(1,gsub(x,a)),p2=cgetr(l));p3=subrs(p2,n+1);
  for(i=n;i>=1;i--) p3=addrr(subrs(p2,i),divrr(mulsr(i,x),p3));
  y=gmul(mpexp(negr(x)),gpui(x,a,prec));tetpil=avma;
  return gerepile(av,tetpil,divrr(y,p3));
}

GEN  incgam2(GEN a, GEN x, long prec)
{
  long av=avma,tetpil,l,n,i;
  double m,mx;
  GEN p1,p2,p3,y;

  if(typ(x)!=2) {gaffect(x,p1=cgetr(prec));x=p1;}
  l=lg(x);mx=rtodbl(x);
  m=(BITS_IN_LONG/4)*(l-2)*LOG2+mx/4;n=(long)(1+m*m/mx);
  gaffect(gsub(x,a),p2=cgetr(l));
  p3=gdiv(gsubgs(a,n),addrs(p2,(n<<1)));
  for(i=n-1;i>=1;i--) p3=gdiv(gsubgs(a,i),addrr(addrs(p2,(i<<1)),gmulsg(i,p3)));
  p3=gaddsg(1,p3);y=gmul(mpexp(negr(x)),gpui(x,gsubgs(a,1),prec));
  tetpil=avma; return gerepile(av,tetpil,gmul(y,p3));
}

GEN  incgam3(GEN a, GEN x, long prec)
{
  long av=avma,tetpil,l,n,i;
  GEN p1,p2,p3,y;

  if(typ(x)!=2) {gaffect(x,p1=cgetr(prec));x=p1;}
  l=lg(x);
  n= -BITS_IN_LONG*(l-2)-1;
  p3=cgetr(l);affsr(1,p3);p2=gcopy(p3);i=0;
  while(expo(p2)>=n)
    {
      i++;p2=gdiv(gmul(x,p2),gaddgs(a,i));
      p3=gadd(p2,p3);
    }
  y=gdiv(gmul(mpexp(negr(x)),gpui(x,a,prec)),a);tetpil=avma;
  return gerepile(av,tetpil,gmul(y,p3));
}

GEN   incgam4(GEN a, GEN x, GEN z, long prec)
               
               

/* One assumes that z=gamma(a,prec) but no check */

{
  long av,tetpil;
  GEN p1,p2,y;

  av=avma;
  if(typ(x)!=2) {gaffect(x,p1=cgetr(prec));x=p1;}
  if((gcmp(subrs(x,1),a)>0)||(gsigne(greal(a))<=0))
    {
      tetpil=avma;y=gerepile(av,tetpil,incgam2(a,x,prec));
    }
  else
    {
      p2=incgam3(a,x,prec);tetpil=avma;
      y=gerepile(av,tetpil,gsub(z,p2));
    }
  return y;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~									~*/
/*~		        FONCTION ZETA DE RIEMANN			~*/
/*~									~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

GEN     czeta(GEN s, long prec)
{
  long av,n,p,n1,l,l1,flag1,flag2,flag3,flag4,i,i2;
  double st,sp,sn,ssig,ns,alpha,beta,maxbeta,xinf,x00,x11,y00,epsil,fepsil;
  GEN y,z,res,res2,sig,ms,t,z1,p1,p2,p3,p31,pitemp;

  if(gcmp1(s)) err(zetaer1);
  if(typ(s)==6)
    {
      l=(LGBITS>>2);
      if(typ((GEN)s[1])==2) l=precision((GEN)s[1]);
      if(typ((GEN)s[2])==2) {l1=precision((GEN)s[2]);if(l1<l) l=l1;}
      if(l==(LGBITS>>2)) l=prec;
      res=cgetg(3,6);res[1]=lgetr(l);res[2]=lgetr(l);
      av=avma;
      res2=cgetg(3,6);res2[1]=lgetr(l+1);res2[2]=lgetr(l+1);
      gaffect(s,res2);s=res2;
      sig=(GEN)s[1];
    }
  else
    {
      if(typ(s)!=2) err(zetaer2);
      res=(signe(s)) ? cgetr(lg(s)) : cgetr(((-expo(s))>>TWOPOTBITS_IN_LONG)+2);
      av=avma;
      res2=(signe(s)) ? cgetr(lg(s)+1) : cgetr(((-expo(s))>>TWOPOTBITS_IN_LONG)+3);
      affrr(s,res2);sig=s=res2;
    }
  if((signe(sig)<=0)||(expo(sig)<-1))
    {
      if(gcmp0(gimag(s))&&gcmp0(gfrac(gmul2n(sig,-1))))
	{
	  if(gcmp0(sig)) {gaffect(gneg(ghalf),res);avma=av;return res;}
	  else {gaffsg(0,res);avma=av;return res;}
	}
      else {flag1=1;s=gsubsg(1,s);sig=greal(s);}
    }
  else flag1=0;
  t=gimag(s);
  ssig=rtodbl(sig);st=fabs(rtodbl(t));maxbeta=pow(3.0,-2.5);
  if(st)
    {
      ns=ssig*ssig+st*st;
      alpha=C2*(prec-2)-0.39-0.5*(ssig-1.0)*log(ns)-log(ssig)+ssig*2*C1;
      beta=(alpha+ssig)/st-atan(ssig/st);
      if(beta<=0)
	{
	  if(ssig>=1.0)
	    {
	      p=0;sn=sqrt(ns);
	      n=(long)(ceil(exp(C2*(prec-2)/ssig)*pow(sn/(2*ssig),1.0/ssig)));
	    }
	  else
	    {
	      p=1;sn=ssig+1;n=(long)(ceil(sqrt(sn*sn+st*st)/(2*PI)));
	    }
	}
      else
	{
	  if(beta<maxbeta) xinf=beta+pow(3*beta,1.0/3.0);
	  else
	    {
	      epsil=0.0001;fepsil=0.0087;flag4=1;
	      x00=beta+PI/2.0;
	      while(flag4)
		{
		  y00=x00*x00;x11=(beta+atan(x00))*(1+y00)/y00-1/x00;
		  if((x00-x11)<fepsil) flag4=0;
		  else x00=x11;
		}
	      xinf=x11;
	    }
	  sp=1.0-ssig+st*xinf;
	  if(sp>0)
	    {
	      p=(long)(ceil(sp/2.0));sn=ssig+2*p-1;
	      n=(long)(ceil(sqrt(sn*sn+st*st)/(2*PI)));
	    }
	  else
	    {
	      p=0;sn=sqrt(ns);
	      n=(long)(ceil(exp(C2*(prec-2)/ssig)*pow(sn/(2*ssig),1.0/ssig)));
	    }
	}
    }
  else
    {
      beta=C2*(prec-2)+0.61+ssig*2*C1-ssig*log(ssig);
      if(beta>0)
	{
	  p=(long)ceil(beta/2.0);sn=ssig+2*p-1;
	  n=(long)ceil(sqrt(sn*sn+st*st)/(2*PI));
	}
      else
	{
	  p=0;sn=sqrt(ssig*ssig+st*st);
	  n=(long)ceil(exp(C2*(prec-2)/ssig)*pow(sn/(2*ssig),1.0/ssig));
	}
    }
  if(n<46340) {flag2=1;n1=n*n;} else flag2=0;
  y=gun;ms=gneg(s);
  for(i=2;i<=n;i++) y=gadd(y,p2=gexp(gmul(ms,glog(stoi(i),prec+1)),prec+1));
  flag3=((2*p)<46340);
  mpbern(p,prec+1);p31=cgetr(prec+1);
  z=gzero;
  for(i=p;i>=1;i--)
    {
      i2=i<<1;
      p1=gmul(gaddsg(i2-1,s),gaddsg(i2,s));
      p1=flag3 ? gdivgs(p1,i2*(i2+1)) : gdivgs(gdivgs(p1,i2),i2+1);
      p1=flag2 ? gdivgs(p1,n1) : gdivgs(gdivgs(p1,n),n);
      if(bernzone[2]>prec+1) {affrr(bern(i),p31);p3=p31;} else p3=(GEN)bern(i);
      z=gadd(divrs(p3,i),gmul(p1,z));
    }
  z1=gsub(gdivsg(n,gsubgs(s,1)),ghalf);
  z=gmul(gadd(z1,gmul(s,gdivgs(z,n<<1))),p2);
  if(!flag1) {gaffect(gadd(y,z),res);avma=av;}
  else
    {
      pitemp=mppi(prec+1);setexpo(pitemp,2);
      y=gmul(gmul(gadd(y,z),ggamma(s,prec+1)),gpui(pitemp,gneg(s),prec+1));
      setexpo(pitemp,0);
      y=gmul2n(gmul(y,gcos(gmul(pitemp,s),prec+1)),1);
      gaffect(y,res);avma=av;
    }
  return res;
}

GEN  izeta(GEN x, long prec)
{
  long av=avma,tetpil,k,fl,n,s;
  GEN y,p1,kk,pitemp,qn,z,q;

  if(typ(x)!=1) err(zetaer2);
  if(gcmp1(x)) err(zetaer1);
  s=signe(x);if(!s) return gneg(ghalf);
  if(s<0)
    {
      k=itos(x);
      if(k&1)
	{
	  y=bernreal(1-k,prec);tetpil=avma;
	  return gerepile(av,tetpil,gdivgs(y,k-1));
	}
      else return gzero;
    }
  else
    {
      if(gcmpgs(x,((prec-2)<<TWOPOTBITS_IN_LONG)+1)>0) {affsr(1,y=cgetr(prec));return y;}
      else
	{
	  k=itos(x);pitemp=mppi(prec);setexpo(pitemp,2);
	  if(k&1)
	    {
	      if((k&3)==3)
		{
		  y=gzero;kk=stoi(k+1);
		  for(n=0;n<=((k+1)>>1);n+=2)
		    {
		      p1=gmul(binome(kk,n),gmul(bernreal(k+1-n,prec),bernreal(n,prec)));
		      if(n==((k+1)>>1)) p1=gmul2n(p1,-1);
		      y=((n>>1)&1)?gsub(y,p1):gadd(y,p1);
		    }
		  y=gmul(divrr(gpuigs(pitemp,k,prec),mpfactr(k+1,prec)),y);
		  q=mpexp(pitemp);z=divsr(1,addrs(q,-1));qn=q;fl=1;
		  for(n=2;fl;n++)
		    {
		      qn=mulrr(qn,q);p1=divsr(1,mulir(gpuigs(stoi(n),k),addrs(qn,-1)));
		      z=gadd(z,p1);fl=gexpo(p1)>= -1-((prec-2)<<TWOPOTBITS_IN_LONG);
		    }
		  y=gadd(y,gmul2n(z,1));
		  tetpil=avma;return gerepile(av,tetpil,gneg(y));
		}
	      else
		{
		  y=gzero;kk=stoi(k+1);
		  for(n=0;n<=((k-1)>>1);n+=2)
		    {
		      p1=gmulsg(k+1-(n<<1),gmul(binome(kk,n),gmul(bernreal(k+1-n,prec),bernreal(n,prec))));
		      y=((n>>1)&1)?gsub(y,p1):gadd(y,p1);
		    }
		  y=divrs(gmul(divrr(gpuigs(pitemp,k,prec),mpfactr(k+1,prec)),y),k-1);
		  q=mpexp(pitemp);z=gzero;affsr(1,qn=cgetr(prec));fl=1;
		  for(n=1;fl;n++)
		    {
		      qn=mulrr(qn,q);p1=mulir(gpuigs(stoi(n),k),gsqr(addrs(qn,-1)));
		      p1=divrr(addrs(mulrr(qn,addsr(1,divrs(mulsr(n<<1,pitemp),k-1))),-1),p1);
		      z=gadd(z,p1);fl=gexpo(p1)>= -1-((prec-2)<<TWOPOTBITS_IN_LONG);
		    }
		  z=gmul2n(z,1);
		  tetpil=avma;return gerepile(av,tetpil,gsub(y,z));
		}
	    }
	  else
	    {
	      y=divrr(mulrr(gpuigs(pitemp,k,prec),absr(bernreal(k,prec))),mpfactr(k,prec));
	      tetpil=avma;return gerepile(av,tetpil,gmul2n(y,-1));
	    }
	}
    }
}
	  

GEN     gzeta(GEN x, long prec)
{
  long    i,lx;
  GEN     y;

    switch(typ(x))
      {
      case 1 : y=izeta(x,prec);break;
      case 2 : 
      case 6 : y=czeta(x,prec);break;
      case 3 : 
      case 7 : err(zetaer2);
      case 11: err(impl,"zeta of power series");
      case 17:
      case 18:
      case 19: lx=lg(x);y=cgetg(lx,typ(x));
	for(i=1;i<lx;i++)
	  y[i]=(long)gzeta((GEN)x[i],prec);
	break;
      default: y=transc(gzeta,x,prec);
      }
  return y;
}

void gzetaz(GEN x, GEN y)
{
  long    av,prec;
  GEN     p;

  prec=precision(y);
  if(!prec) err(zetaer3);
  av=avma;p=gzeta(x,prec);
  gaffect(p,y);avma=av;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~                                                                     ~*/
/*~                     FONCTION POLYLOGARITHME                         ~*/
/*~                                                                     ~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

GEN cxpolylog(long m, GEN x, long prec)

/* Ici, x peut etre complexe et le domaine de validite contient .005<|z|<230 */

{
  long av=avma,tetpil,fl,i,n;
  GEN p1,z,z2,h,q,s;

  if(gcmp1(x)) return izeta(stoi(m),prec);
  z=glog(x,prec);h=gneg(glog(gneg(z),prec));for(i=1;i<=m-1;i++) h=gadd(h,gdivsg(1,stoi(i)));
  q=gun;s=izeta(stoi(m),prec);
  for(n=1;n<=m+1;n++) {q=gdivgs(gmul(q,z),n);s=gadd(s,gmul((n==(m-1))?h:izeta(stoi(m-n),prec),q));}
  fl=1;z2=gmul(z,z);
  for(n=m+3;fl;n+=2)
    {
      q=gdivgs(gmul(q,z2),(n-1)*n);p1=gmul(izeta(stoi(m-n),prec),q);
      tetpil=avma;s=gadd(s,p1);fl=gexpo(p1)>= -((prec-2)<<TWOPOTBITS_IN_LONG)-1;
    }
  return gerepile(av,tetpil,s);
}

GEN polylog(long m, GEN x, long prec)
{
  long av,tetpil,l,e,sx,i;
  GEN p1,p2,unreel,n,y,logx,logx2;
  
  if(m<0) err(polyloger1);
  if(!m) return gneg(ghalf);
  if(gcmp0(x)) return gcopy(x);
  av=avma;
  if(m==1)
    {
      p1=glog(gsubsg(1,x),prec);tetpil=avma;return gerepile(av,tetpil,gneg(p1));
    }
  if(!(l=precision(x))) {l=prec;affsr(1,unreel=cgetr(l));x=gmul(unreel,x);}
  e=gexpo(gnorm(x));if((!e)||(e== -1)) return cxpolylog(m,x,prec);
  if(e>0) 
    {
      logx=glog(x,l);sx=gsigne(gimag(x));
      if(!sx) {if(m%2) sx=gsigne(greal(gsubsg(1,x)));else sx= -gsigne(greal(x));}
      x=ginv(x);
    }
  y=x;n=gun;p1=x;
  do
    {
      n=addsi(1,n);p1=gmul(x,p1);p2=gdiv(p1,gpuigs(n,m));
      tetpil=avma;y=gadd(y,p2);
    }
  while(gexpo(p2)>-BITS_IN_LONG*(l-2));
  if(e<=0) return gerepile(av,tetpil,y);
  logx2=gmul(logx,logx);
  p1=gneg(ghalf);
  for(i=m-2;i>=0;i-=2)
    {
      p1=gadd(izeta(stoi(m-i),l),gmul(p1,gdivgs(logx2,(i+1)*(i+2))));
    }
  if(m%2) p1=gmul(logx,p1);
  p2=cgetg(3,6);p2[1]=zero;p2[2]=ldiv(gmulsg(sx,mppi(l)),mpfact(m-1));
  p1=gadd(gmul2n(p1,1),gmul(p2,gpuigs(logx,m-1,prec)));tetpil=avma;
  y=(m%2)?gadd(p1,y):gsub(p1,y);
  return gerepile(av,tetpil,y);
}

GEN dilog(GEN x, long prec)
{
  GEN p1,p2,p3,y;
  long av,tetpil;

  av=avma;if(gcmpgs(gnorm(x),1)<1)
    {
      tetpil=avma;return gerepile(av,tetpil,polylog(2,x,prec));
    }
  y=gneg(polylog(2,ginv(x),prec));p3=mppi(prec);p2=gdivgs(gmul(p3,p3),6);
  p1=glog(gneg(x),prec);p1=gadd(p2,gmul2n(gmul(p1,p1),-1));
  tetpil=avma;return gerepile(av,tetpil,gsub(y,p1));
}

GEN polylogd(long m, GEN x, long prec)
{
  long k,l,av,tetpil,fl,m2;
  GEN p1,p2,p3,y,unreel;
  
  m2=m&1;av=avma;
  if(gcmp0(x)) return gcopy(x);
  if(gcmp1(x)&&(m>=2)) return m2?izeta(stoi(m),prec):gzero;
  if(!(l=precision(x))) {l=prec;affsr(1,unreel=cgetr(l));x=gmul(unreel,x);}
  p1=gabs(x,prec);fl=0;
  if(gcmpgs(p1,1)>0) {x=ginv(x);p1=gabs(x,prec);fl=!m2;}
  p1=gneg(glog(p1,prec));
  y=m2?greal(polylog(m,x,prec)):gimag(polylog(m,x,prec));p2=gun;
  for(k=1;k<=(m-1);k++)
    {
      p2=gdivgs(gmul(p2,p1),k);
      p3=m2?greal(polylog(m-k,x,prec)):gimag(polylog(m-k,x,prec));
      tetpil=avma;y=gadd(y,gmul(p2,p3));
    }
  if(m2)
    {
      p2=gdivgs(gmul(glog(gnorm(gsubsg(1,x)),prec),p1),2*m);tetpil=avma;y=gadd(y,p2);
    }
  if(fl) {tetpil=avma;return gerepile(av,tetpil,gneg(y));}
  else {return gerepile(av,tetpil,y);}
}

GEN polylogdold(long m, GEN x, long prec)
{
  long k,l,av,tetpil,fl,m2;
  GEN p1,p2,p3,y,unreel;
  
  m2=m&1;av=avma;
  if(gcmp0(x)) return gcopy(x);
  if(gcmp1(x)&&(m>=2)) return m2?izeta(stoi(m),prec):gzero;
  if(!(l=precision(x))) {l=prec;affsr(1,unreel=cgetr(l));x=gmul(unreel,x);}
  p1=gabs(x,prec);fl=0;
  if(gcmpgs(p1,1)>0) {x=ginv(x);p1=gabs(x,prec);fl=!m2;}
  p1=gneg(glog(p1,prec));
  y=m2?greal(polylog(m,x,prec)):gimag(polylog(m,x,prec));p2=gun;
  for(k=1;k<=(m-1);k++)
    {
      p2=gdivgs(gmul(p2,p1),k);
      p3=m2?greal(polylog(m-k,x,prec)):gimag(polylog(m-k,x,prec));
      tetpil=avma;y=gadd(y,gmul(p2,p3));
    }
  if(m2)
    {
      p2=gdivgs(gmul(p2,p1),-2*m);tetpil=avma;y=gadd(y,p2);
    }
  if(fl) {tetpil=avma;return gerepile(av,tetpil,gneg(y));}
  else {return gerepile(av,tetpil,y);}
}


GEN polylogp(long m, GEN x, long prec)
{
  long k,l,av,tetpil,fl,m2;
  GEN p1,p2,p3,p4,p5,p51,y,unreel;
  
  m2=m&1;av=avma;
  if(gcmp0(x)) return gcopy(x);
  if(gcmp1(x)&&(m>=2)) return m2?izeta(stoi(m),prec):gzero;
  if(!(l=precision(x))) {l=prec;affsr(1,unreel=cgetr(l));x=gmul(unreel,x);}
  p1=gabs(x,prec);fl=0;
  if(gcmpgs(p1,1)>0) {x=ginv(x);p1=gabs(x,prec);fl=!m2;}
  p1=gmul2n(glog(p1,prec),1);mpbern(m>>1,prec);p51=cgetr(prec);
  y=m2?greal(polylog(m,x,prec)):gimag(polylog(m,x,prec));p2=gun;
  if(m==1)
    {
      p2=gmul2n(p1,-2);tetpil=avma;y=gadd(y,p2);
    }
  else
    for(k=1;k<=(m-1);k++)
      {
	p2=gdivgs(gmul(p2,p1),k);
	if((!(k&1))||(k==1))
	  {
	    if(k!=1) 
	      {
		if(bernzone[2]>prec) {affrr(bern(k>>1),p51);p5=p51;}
		else p5=(GEN)bern(k>>1);
		p4=gmul(p2,p5);
	      }
	    else p4=gneg(gmul2n(p2,-1));
	    p3=m2?greal(polylog(m-k,x,prec)):gimag(polylog(m-k,x,prec));
	    tetpil=avma;y=gadd(y,gmul(p4,p3));
	  }
      }
  if(fl) {tetpil=avma;return gerepile(av,tetpil,gneg(y));}
  else {return gerepile(av,tetpil,y);}
}

GEN     gpolylog(long m, GEN x, long prec)
{
  long    i,lx,av=avma,tetpil,v,n;
  GEN     y,p1,p2;

  if(m<=0)
    {
      p1=polx[0];p2=gsubsg(1,p1);
      for(i=1;i<=(-m);i++) p1=gmul(polx[0],gadd(gmul(p2,deriv(p1,0)),gmulsg(i,p1)));
      p1=gdiv(p1,gpuigs(p2,1-m,prec));tetpil=avma;
      y=gerepile(av,tetpil,gsubst(p1,0,x));
    }
  else
    {
      switch(typ(x))
	{
	case 1 : 
	case 2 : 
	case 4 :
	case 5 :
	case 6 :
	case 8 : y=polylog(m,x,prec);break;
	case 3 : 
	case 7 : 
	case 9 : p1=roots((GEN)x[1],prec);lx=lg(p1);p2=cgetg(lx,18);
	  for(i=1;i<lx;i++) p2[i]=lpoleval((GEN)x[2],(GEN)p1[i]);
	  tetpil=avma;y=cgetg(lx,18);
	  for(i=1;i<lx;i++) y[i]=(long)polylog(m,(GEN)p2[i],prec);
	  y=gerepile(av,tetpil,y);break;
	case 10:
	case 13:
	case 14: p1=tayl(x,gvar(x),precdl);tetpil=avma;
	  y=gerepile(av,tetpil,gpolylog(m,p1,prec));
	  break;
	case 11: if(m<0) err(polyloger1);
	  if(!m) return gneg(ghalf);
	  if(m==1)
	    {
	      p1=glog(gsubsg(1,x),prec);tetpil=avma;return gerepile(av,tetpil,gneg(p1));
	    }
	  if(valp(x)<=0) err(impl,"polylog around a!=0");
	  v=varn(x);n=(lg(x)-2)/valp(x);y=ggrando(polx[v],lg(x)-2);
	  for(i=n;i>=1;i--)
	    {
	      p1=gadd(gpuigs(stoi(i),-m),y);tetpil=avma;
	      y=gmul(x,p1);
	    }
	  y=gerepile(av,tetpil,y);break;
	case 17:
	case 18:
	case 19: lx=lg(x);y=cgetg(lx,typ(x));
	  for(i=1;i<lx;i++)
	    y[i]=(long)gpolylog(m,(GEN)x[i],prec);
	  break;
	default: err(zetaer2);
	}
    }
  return y;
}

void gpolylogz(long m, GEN x, GEN y)
{
  long    av,prec;
  GEN     p;

  prec=precision(y);
  if(!prec) err(zetaer3);
  av=avma;p=gpolylog(m,x,prec);
  gaffect(p,y);avma=av;
}


GEN qq(GEN x, long prec)
{
  long av=avma,tetpil,l,tx=typ(x);
  GEN p1,p2,q;
  
  if(tx==7) return gcopy(x);
  if(tx<10)
  {
    if(!(l=precision(x))) l=prec;
    p1=mppi(l);setexpo(p1,2);p2=cgetg(3,6);p2[1]=zero;p2[2]=(long)p1;
    q=gmul(x,p2);tetpil=avma;
    return gerepile(av,tetpil,gexp(q,prec));
  }
  else return tayl(x,gvar(x),precdl);
}

GEN inteta(GEN q)
{
  long av=avma,tetpil,tx=typ(q),l,n,f,v;
  GEN p1,ps,qn,y0,y;

  y=gun;n=0;qn=gun;ps=gun;
  if(tx==7)
    {
      do
	{
	  n++;p1=gneg(gmul(ps,gmul(q,gmul(qn,qn))));y=gadd(y,p1);qn=gmul(qn,q);
	  ps=gmul(p1,qn);tetpil=avma;y0=y;y=gadd(y,ps);
	}
      while(!gegal(y0,y));
    }
  else
    {
      if(tx<10) l=precision(q);else v=gvar(q);
      do
	{
	  n++;p1=gneg(gmul(ps,gmul(q,gmul(qn,qn))));y=gadd(y,p1);qn=gmul(qn,q);
	  ps=gmul(p1,qn);tetpil=avma;y=gadd(y,ps);
	  f=(tx<10)?(gexpo(ps)-gexpo(y)>=-BITS_IN_LONG*(l-2)):((!gcmp0(ps))&&(gval(ps,v)<precdl));
	}
      while(f);
    }
  return gerepile(av,tetpil,y);
}

   
GEN eta(GEN x, long prec)
{
  long av=avma,tetpil;
  GEN q;

  q=qq(x,prec);tetpil=avma;
  return gerepile(av,tetpil,inteta(q));
}

GEN jell(GEN x, long prec)
{
  long av=avma,tetpil;
  GEN p1,p2,q;
  
  q=qq(x,prec);p1=gdiv(inteta(gmul(q,q)),inteta(q));
  p1=gmul2n(gmul(p1,p1),1);p1=gmul(q,gpuigs(p1,12,prec));p2=gaddsg(768,gadd(gmul(p1,p1),gdivsg(4096,p1)));
  p1=gmulsg(48,p1);tetpil=avma;
  return gerepile(av,tetpil,gadd(p2,p1));
}

GEN wf2(GEN x, long prec)
{
  long av=avma,tetpil;
  GEN p1,p2,q;
  
  q=qq(x,prec);p1=gmul(gdiv(inteta(gmul(q,q)),inteta(q)),gsqrt(gdeux,prec));
  p2=cgetg(3,6);p2[1]=zero;p2[2]=ldivrs(mppi(prec),12);p2=gexp(gmul(x,p2),prec);tetpil=avma;
  return gerepile(av,tetpil,gmul(p1,p2));
}

GEN wf(GEN x, long prec)
{
  long av=avma,tetpil;
  GEN p1,p2,q;
  
  q=qq(gmul2n(gaddgs(x,1),-1),prec);p1=gdiv(inteta(q),inteta(gmul(q,q)));
  p2=cgetg(3,6);p2[1]=zero;p2[2]=ldivrs(mppi(prec),-24);p2=gexp(gmul(p2,x),prec);tetpil=avma;
  return gerepile(av,tetpil,gmul(p1,p2));
}

GEN sagm(GEN x, long prec)
{
  GEN z,p1,a,b,a1,b1;
  long av,tetpil,pp,ep;

  if(gcmp0(x)) return gcopy(x);
  switch(typ(x))
    {
    case 2:
    case 6: av=avma;if(pp=precision(x)) prec=pp;
      a1=x;b1=gun;
      do
	{
	  a=a1;b=b1;
	  a1=gmul2n(gadd(a,b),-1);
	  b1=gsqrt(gmul(a,b),prec);
	}
      while(gexpo(gsub(b1,a1))>=gexpo(b1)-((prec-2)<<TWOPOTBITS_IN_LONG)+5);
      tetpil=avma;z=gerepile(av,tetpil,gcopy(a1));break;
    case 7: av=avma;a1=x;b1=gun;pp=precp(x);
      do
	{
	  a=a1;b=b1;
	  a1=gmul2n(gadd(a,b),-1);b1=gsqrt(gmul(a,b),0);
	  p1=gsub(b1,a1);ep=valp(p1)-valp(b1);
	  if(ep<=0) {b1=gneg(b1);p1=gsub(b1,a1);ep=valp(p1)-valp(b1);}
	}
      while((ep<pp)&&(!gcmp0(p1)));
      tetpil=avma;z=gerepile(av,tetpil,gcopy(a1));break;
    case 11: av=avma;a1=x;b1=gun;pp=lg(x)-2;
      do
	{
	  a=a1;b=b1;
	  a1=gmul2n(gadd(a,b),-1);b1=gsqrt(gmul(a,b),0);
	  p1=gsub(b1,a1);ep=valp(p1)-valp(b1);
/*	  if(ep<=0) {b1=gneg(b1);p1=gsub(b1,a1);ep=valp(p1)-valp(b1);} */
	}
      while(ep<pp&&(!gcmp0(p1)));
      tetpil=avma;z=gerepile(av,tetpil,gcopy(a1));break;
    case 3: err(impl,"agm of mod");
    default: z=transc(sagm,x,prec);
    }
  return z;
}

GEN agm(GEN x, GEN y, long prec)
{
  GEN z;
  long av,tetpil;

  if(typ(y)>=17)
    {
      if(typ(x)>=17) err(agmer1);
      {z=x;x=y;y=z;}
    }
  if(gcmp0(y)) return gcopy(y);
  av=avma;z=sagm(gdiv(x,y),prec);tetpil=avma;
  return gerepile(av,tetpil,gmul(y,z));
}
  
GEN logagm(GEN q)
{
  long av=avma,prec=lg(q),tetpil,s,n,lim;
  GEN y,q4,q1,pitemp;

  if(typ(q)!=2) err(loger1);
  if(signe(q)<=0) err(loger2);
  lim= -((prec-2)<<(TWOPOTBITS_IN_LONG-1));n=0;
  if(expo(q)>=0) {q=ginv(q);s=0;} else s=1;
  while(expo(q)>=lim) {q1=q;q=mulrr(q,q);n=n+1;}
  q4=gmul2n(q,2);pitemp=mppi(prec);
  if(!n) y=divrr(pitemp,agm(addsr(1,q4),gmul2n(gsqrt(q,prec),2),prec));
  else y=divrr(pitemp,agm(addsr(1,q4),gmul2n(q1,2),prec));
  tetpil=avma;y=gmul2n(y,-n);if(s) setsigne(y,-1);
  return gerepile(av,tetpil,y);
}

GEN glogagm(GEN x, long prec)
{
  long    av,tetpil,v;
  GEN y,p1,p2;

  switch(typ(x))
  {
  case 2 : if(signe(x)>=0) y=logagm(x);
  else
    {
    y=cgetg(3,6);y[2]=lmppi(lg(x));
    setsigne(x,1);y[1]=(long)logagm(x);
    setsigne(x,-1);
    }
    break;
  
  case 6 : y=cgetg(3,6);y[2]=larg(x,prec);
    av=avma;p1=glogagm(gnorm(x),prec);tetpil=avma;
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
      p1=integ(p1,v);p2=glogagm((GEN)x[2],prec);
      tetpil=avma;y=gerepile(av,tetpil,gadd(p1,p2));
    }
    break;
  
  default: y=transc(glogagm,x,prec);
  }
  return y;
}

GEN theta(GEN q, GEN z, long prec)
{
  long av=avma,tetpil,l,n;
  GEN p1,ps,qn,qnold,y,zy,lq,ps2,unreel,k,zold;

  if(gexpo(q)>=0) err(thetaer1);
  if(l=precision(q)) prec=l;
  affsr(1,unreel=cgetr(prec));z=gmul(unreel,z);
  if(!l) q=gmul(unreel,q);
  if(gcmp0(zy=gimag(z))) k=gzero;
  else
    {
      lq=glog(q,prec);k=ground(gdiv(zy,greal(lq)));
      if(!gcmp0(k)) {zold=z;z=gadd(z,gdiv(gmul(lq,k),gi));}
    }
  y=gsin(z,prec);n=0;qn=gun;ps=gneg(ps2=gmul(q,q));
  do
    {
      n++;p1=gsin(gmulsg(n+n+1,z),prec);qnold=qn;qn=gmul(qn,ps);
      ps=gmul(ps,ps2);p1=gmul(p1,qn);
      y=gadd(y,p1);
    }
  while(gexpo(qnold)>= -((prec-2)<<TWOPOTBITS_IN_LONG));
  if(!gcmp0(k))
    {
      y=gmul(y,gmul(gpui(q,gmul(k,k),prec),gexp(gmul2n(gmul(gmul(gi,zold),k),1),prec)));
      if(mpodd(k)) y=gneg(y);
    }
  p1=gmul2n(gsqrt(gsqrt(q,prec),prec),1);tetpil=avma;
  return gerepile(av,tetpil,gmul(p1,y));
}

GEN thetanullk(GEN q, long k, long prec)
{
  long av=avma,tetpil,l,n;
  GEN p1,ps,qn,y,ps2,unreel;

  if(gexpo(q)>=0) err(thetaer1);
  if(!(k&1)) return gzero;
  n=0;qn=gun;ps=gneg(ps2=gmul(q,q));
  y=gun;if(!(l=precision(q))) 
    {
      l=prec;affsr(1,unreel=cgetr(prec));q=gmul(unreel,q);
    }
  do
    {
      n++;p1=gpuigs(stoi(n+n+1),k);qn=gmul(qn,ps);
      ps=gmul(ps,ps2);p1=gmul(p1,qn);
      y=gadd(y,p1);
    }
  while(gexpo(p1)>= -((l-2)<<TWOPOTBITS_IN_LONG));
  p1=gmul2n(gsqrt(gsqrt(q,prec),prec),1);tetpil=avma;
  if(k&2) {p1=gneg(p1);tetpil=avma;}
  return gerepile(av,tetpil,gmul(p1,y));
}

GEN jbesselh(GEN n, GEN z, long prec)
{
  long av,tetpil,k,l,i,lz;
  GEN y,p1,p2,zinv,p0,s,c;

  if(typ(n)!=1) err(jbesselher1);
  k=itos(n);if(k<0) err(impl,"ybesselh");
  
  switch(typ(z))
    {
    case 2 : 
    case 6 :
      if(gcmp0(z)) return gzero;
      av=avma;zinv=ginv(z);
      l=precision(z);if(l>prec) prec=l;
      gsincos(z,&s,&c,prec);
      p0=gmul(zinv,s);p1=gmul(zinv,gsub(p0,c));
      if(!k) p1=p0;
      else
	{
	  for(i=2;i<=k;i++)
	    {
	      p2=gsub(gmul(gmulsg(2*i-1,zinv),p1),p0);p0=p1;p1=p2;
	    }
	}
      p2=gsqrt(gdiv(gmul2n(z,1),mppi(prec)),prec);
      tetpil=avma;y=gerepile(av,tetpil,gmul(p2,p1));
      break;

    case 7 : err(impl,"p-adic jbessel function");
    case 3 : err(gamer3);
    case 11: err(impl,"jbessel of power series");
    case 17:
    case 18:
    case 19: lz=lg(z);y=cgetg(lz,typ(z));
      for(i=1;i<lz;i++)
        y[i]=(long)jbesselh(n,(GEN)z[i],prec);
      break;
    case 1 :
    case 4 :
    case 5 : av=avma;p1=cgetr(prec);gaffect(z,p1);tetpil=avma;
      y=gerepile(av,tetpil,jbesselh(n,p1,prec));break;
    case 8 : av=avma;p1=cgetr(prec);affsr(1,p1);
      p1=gmul(z,p1);tetpil=avma;
      y=gerepile(av,tetpil,jbesselh(n,p1,prec));break;
    case 10:
    case 13:
    case 14: av=avma;p1=tayl(z,gvar(z),precdl);tetpil=avma;
      y=gerepile(av,tetpil,jbesselh(n,p1,prec));
      break;
    case 9 : av=avma;p1=roots((GEN)z[1],prec);lz=lg(p1);p2=cgetg(lz,18);
      for(i=1;i<lz;i++) p2[i]=lpoleval((GEN)z[2],(GEN)p1[i]);
      tetpil=avma;y=cgetg(lz,18);
      for(i=1;i<lz;i++) y[i]=(long)jbesselh(n,(GEN)p2[i],prec);
      y=gerepile(av,tetpil,y);break;
    case 15:
    case 16: err(transcer1);
    }
  return y;
}



