/********************************************************************/
/********************************************************************/
/**                                                                **/
/**                    COURBES ELLIPTIQUES                         **/
/**                                                                **/
/**                     Copyright Babe Cool                        **/
/**                                                                **/
/********************************************************************/
/********************************************************************/

#include "genpari.h"

static int aux(GEN ak, int p, int l),aux2(GEN ak, int p, GEN pl),numroots2(int a, int b, int c, int p, int *mult),numroots3(int a, int b, int c, int p, int *mult);

GEN smallinitell(GEN x)
{
  GEN y,b2,b4,b6,b8,d,j,a11,a13,a33,a64,b81,b22,c4,c6;
  long i,av,tetpil;

  av=avma;y=cgetg(14,17);
  if ((typ(x) != 17) || (lg(x) < 6)) err(elliper1);
  for(i=1;i<=5;i++) y[i]=x[i];
  b2=gadd(a11=gmul((GEN)y[1],(GEN)y[1]),gmul2n((GEN)y[2],2));y[6]=(long)b2;
  b4=gadd(a13=gmul((GEN)y[1],(GEN)y[3]),gmul2n((GEN)y[4],1));y[7]=(long)b4;
  b6=gadd(a33=gmul((GEN)y[3],(GEN)y[3]),a64=gmul2n((GEN)y[5],2));y[8]=(long)b6;
  b81=gadd(gadd(gmul(a11,(GEN)y[5]),gmul(a64,(GEN)y[2])),gmul((GEN)y[2],a33));
  b8=gsub(b81,gmul((GEN)y[4],gadd((GEN)y[4],a13)));y[9]=(long)b8;
  c4=gadd(b22=gmul(b2,b2),gmulsg(-24,b4));y[10]=(long)c4;
  c6=gadd(gmul(b2,gsub(gmulsg(36,b4),b22)),gmulsg(-216,b6));y[11]=(long)c6;
  b81=gadd(gmul(b22,b8),gmulsg(27,gmul(b6,b6)));
  d=gsub(gmul(b4,gadd(gmulsg(9,gmul(b2,b6)),gmulsg(-8,gmul(b4,b4)))),b81);
  y[12]=(long)d;j=gcmp0(d)?gzero:gdiv(gmul(gmul(c4,c4),c4),d);y[13]=(long)j;
  tetpil=avma;return gerepile(av,tetpil,gcopy(y));
}

static GEN initellcom(GEN x, long prec, long hard)
{
  GEN y,b2,b4,b6,b8,d,j,a11,a13,a33,a64,b81,b22,c4,c6,p1,p2,p,u,w,w2;
  GEN aa0,bb0,aa1,bb1,r1,x0,x1,u2,q,pv,bmod0,bmod1,tau,pi2,pitemp,e0,e1;
  long ty,i,av,tetpil,e3,e4,ind,alpha,sw;

  av=avma;y=cgetg(20,17);
  if ((typ(x) != 17) || (lg(x) < 6)) err(elliper1);
  e3=BIGINT;
  for(i=1;i<=5;i++) 
    {
      q=(GEN)x[i];y[i]=(long)q;
      if(typ(q)==7) 
	{
	  e3=min(e3,signe((GEN)q[4])?precp(q)+valp(q):valp(q));
	  p=(GEN)q[2];
	}
    }
  if(e3<BIGINT)
    {
      q=ggrando(p,e3);for(i=1;i<=5;i++) y[i]=ladd(q,(GEN)x[i]);
    }
  b2=gadd(a11=gmul((GEN)y[1],(GEN)y[1]),gmul2n((GEN)y[2],2));y[6]=(long)b2;
  b4=gadd(a13=gmul((GEN)y[1],(GEN)y[3]),gmul2n((GEN)y[4],1));y[7]=(long)b4;
  b6=gadd(a33=gmul((GEN)y[3],(GEN)y[3]),a64=gmul2n((GEN)y[5],2));y[8]=(long)b6;
  b81=gadd(gadd(gmul(a11,(GEN)y[5]),gmul(a64,(GEN)y[2])),gmul((GEN)y[2],a33));
  b8=gsub(b81,gmul((GEN)y[4],gadd((GEN)y[4],a13)));y[9]=(long)b8;
  c4=gadd(b22=gmul(b2,b2),gmulsg(-24,b4));y[10]=(long)c4;
  c6=gadd(gmul(b2,gsub(gmulsg(36,b4),b22)),gmulsg(-216,b6));y[11]=(long)c6;
  b81=gadd(gmul(b22,b8),gmulsg(27,gmul(b6,b6)));
  d=gsub(gmul(b4,gadd(gmulsg(9,gmul(b2,b6)),gmulsg(-8,gmul(b4,b4)))),b81);
  y[12]=(long)d;j=gdiv(gmul(gmul(c4,c4),c4),d);y[13]=(long)j;
  if(gcmp0(d)) err(initeler2);
  ty=typ(d);
  if(prec&&(ty<=8)&&(ty!=3)&&(ty!=7))
    {
      p1=cgetg(6,10);p1[1]=evalsigne(1)+evallgef(6);p1[2]=(long)b6;
      p1[3]=lmul2n(b4,1);p1[4]=(long)b2;p1[5]=lstoi(4);
      if(hard) p1=rootslong(p1,prec);else p1=roots(p1,prec);
      if(gsigne(d)>0) 
        {
          p1=greal(p1);
          ind=1;e4=p1[1];for(i=2;i<=3;i++) 
	    if(gcmp((GEN)p1[i],(GEN)e4)>0) {ind=i;e4=p1[i];}
          p1[ind]=p1[1];p1[1]=e4;
          if(gcmp((GEN)p1[2],(GEN)p1[3])<0) {e4=p1[3];p1[3]=p1[2];p1[2]=e4;}
        }
      else p1[1]=lreal((GEN)p1[1]);
      y[14]=(long)p1;e1=(GEN)p1[1];p2=gadd(gmulsg(3,e1),gmul2n(b2,-2));
      w=gsqrt(gmul2n(gadd(b4,gmul(e1,gadd(b2,gmulsg(6,e1)))),1),prec);
      if(gsigne(p2)>0) w=gneg(w);
      aa1=gmul2n(gsub(w,p2),-2);sw=signe(w);
      bb1=gmul2n(w,-1);r1=gsub(aa1,bb1);x1=gmul2n(r1,-2);
      do
        {
          aa0=aa1;bb0=bb1;x0=x1;
          bb1=gsqrt(gmul(aa0,bb0),prec);setsigne(bb1,sw);
          aa1=gmul2n(gadd(gadd(aa0,bb0),gmul2n(bb1,1)),-2);
          r1=gsub(aa1,bb1);
	  x1=gmul(x0,gsqr(gmul2n(gaddsg(1,gsqrt(gdiv(gadd(x0,r1),x0),prec)),-1)));
        }
      while(gexpo(r1)>=gexpo(bb1)-((prec-2)<<TWOPOTBITS_IN_LONG)+5);
      u2=ginv(gmul2n(aa1,2));
      w=gaddsg(1,ginv(gmul2n(gmul(u2,x1),1)));
      if(gsigne(greal(w))>0) q=ginv(gadd(w,gsqrt(gaddgs(gmul(w,w),-1),prec)));
      else q=gsub(w,gsqrt(gaddgs(gmul(w,w),-1),prec));
      pitemp=mppi(prec);pi2=gmul2n(pitemp,1);if(gexpo(q)>=0) q=ginv(q);
      tau=gmul(gdiv(glog(q,prec),pi2),gneg(gi));
      y[19]=lmul(gmul(gsqr(pi2),gabs(u2,prec)),gimag(tau));
      u=gmul(pi2,gsqrt(gneg(u2),prec));w2=gmul(tau,u);
      if(sw<0) y[15]=(long)u;
      else
        {
          y[15]=lmul2n(gabs((GEN)w2[1],prec),1);
          q=gexp(gmul(gmul(pi2,gi),gdiv(w2,(GEN)y[15])),prec);
        }
      y[16]=(long)w2;
      p1=gdiv(gmul(pitemp,pitemp),gmulsg(6,(GEN)y[15]));q=gsqrt(q,prec);
      y[17]=lmul(p1,gdiv(thetanullk(q,3,prec),thetanullk(q,1,prec)));
      y[18]=ldiv(gsub(gmul((GEN)y[17],(GEN)y[16]),gmul(gi,pitemp)),(GEN)y[15]);
    }
  else 
    if(ty==7)
      {
        if(valp(j)>=0) err(initeler1);
        p=(GEN)d[2];alpha=valp(c4)>>1;
        setvalp(c4,0);setvalp(c6,0);e1=gdivgs(gdiv(c6,c4),6);
        c4=gdivgs(c4,48);c6=gdivgs(c6,864);
        do
          {
            e0=e1;p2=gmul(e0,e0);
            e1=gdiv(gadd(gmul2n(gmul(e0,p2),1),c6),gsub(gmulsg(3,p2),c4));
          }
        while(!gegal(e0,e1));
        setvalp(e1,valp(e1)+alpha);e1=gsub(e1,gdivgs(b2,12));
        w=gsqrt(gmul2n(gadd(b4,gmul(e1,gadd(b2,gmulsg(6,e1)))),1),0);
        p1=gaddgs(gdiv(gmulsg(3,e0),w),1);
        if(valp(p1)<=0) w=gneg(w);
        y[18]=(long)w;
        aa1=gmul2n(gsub(w,gadd(gmulsg(3,e1),gmul2n(b2,-2))),-2);
        bb1=gmul2n(w,-1);r1=gsub(aa1,bb1);x1=gmul2n(r1,-2);
	if(gegal(p,gdeux)) {pv=stoi(4);err(impl,"initell for 2-adic numbers");}
	else pv=p;
        bmod0=modii((GEN)bb1[4],pv);
        do
          {
            aa0=aa1;bb0=bb1;x0=x1;
            bb1=gsqrt(gmul(aa0,bb0),0);bmod1=modii((GEN)bb1[4],pv);
            if(!gegal(bmod1,bmod0)) bb1=gneg(bb1);
            aa1=gmul2n(gadd(gadd(aa0,bb0),gmul2n(bb1,1)),-2);
            r1=gsub(aa1,bb1);
            p1=gsqrt(gdiv(gadd(x0,r1),x0),0);
            if(!gegal(modii((GEN)p1[4],pv),gun)) p1=gneg(p1);
            x1=gmul(x0,gsqr(gmul2n(gaddsg(1,p1),-1)));
          }
        while(!gcmp0(r1));
        u2=ginv(gmul2n(aa1,2));
        w=gaddsg(1,ginv(gmul2n(gmul(u2,x1),1)));
        q=ginv(gadd(w,gsqrt(gaddgs(gmul(w,w),-1),0)));
        if(valp(q)<0) q=ginv(q);
        y[15]=(long)u2;
        y[16]=((kronecker((GEN)u2[4],p)>0)&&(!(valp(u2)&1)))?lsqrt(u2,0):zero;
        y[17]=(long)q;p1=cgetg(2,17);
        y[14]=(long)p1;p1[1]=(long)e1;y[19]=zero;
      }
    else  {y[14]=y[15]=y[16]=y[17]=y[18]=y[19]=zero;}
  tetpil=avma;return gerepile(av,tetpil,gcopy(y));
}

GEN initell2(GEN x, long prec)
{
  return initellcom(x, prec, 0);
}

GEN initell(GEN x, long prec)
{
  return initellcom(x, prec, 1);
}

GEN coordch(GEN e, GEN ch)
{
  GEN y,p1,p2,v,v2,v3,v4,v6,r,s,t,u;
  long av,tetpil,i,lx=lg(e);
  
  if ((typ(e) != 17) || (lx < 14)) err(elliper1);
  u=(GEN)ch[1];r=(GEN)ch[2];s=(GEN)ch[3];t=(GEN)ch[4];
  av=avma;y=cgetg(lx,17);v=ginv(u);
  y[1]=lmul(v,gadd((GEN)e[1],gmul2n(s,1)));v2=gmul(v,v);
  y[2]=lmul(v2,gsub(gadd((GEN)e[2],gmulsg(3,r)),gmul(s,gadd((GEN)e[1],s))));
  v3=gmul(v,v2);
  y[3]=lmul(v3,p1=gadd(gadd(gmul(r,(GEN)e[1]),gmul2n(t,1)),(GEN)e[3]));
  v4=gmul(v,v3);
  p1=gsub((GEN)e[4],gadd(gmul(t,(GEN)e[1]),gmul(s,p1)));
  y[4]=lmul(v4,gadd(p1,gmul(r,gadd(gmul2n((GEN)e[2],1),gmulsg(3,r)))));
  p1=gadd((GEN)e[5],gmul(r,gadd((GEN)e[4],gmul(r,gadd((GEN)e[2],r)))));
  p2=gmul(t,gadd(gadd(gmul(r,(GEN)e[1]),t),(GEN)e[3]));v6=gmul(v2,v4);
  y[5]=lmul(v6,gsub(p1,p2));
  y[6]=lmul(v2,gadd((GEN)e[6],gmulsg(12,r)));
  y[7]=lmul(v4,gadd((GEN)e[7],gmul(r,gadd((GEN)e[6],gmulsg(6,r)))));
 
y[8]=lmul(v6,gadd((GEN)e[8],gmul(r,gadd(gmul2n((GEN)e[7],1),gmul(r,gadd((GEN)e[6],gmul2n(r,2)
))))));
  p1=gadd(gmulsg(3,(GEN)e[7]),gmul(r,gadd((GEN)e[6],gmulsg(3,r))));
  y[9]=lmul(gmul(v6,v2),gadd((GEN)e[9],gmul(r,gadd(gmulsg(3,(GEN)e[8]),gmul(r,p1)))));
  y[10]=lmul(v4,(GEN)e[10]);y[11]=lmul(v6,(GEN)e[11]);
  y[12]=lmul(gmul(v6,v6),(GEN)e[12]);y[13]=lcopy((GEN)e[13]);
  if(lx==14) {tetpil=avma;return gerepile(av,tetpil,gcopy(y));}
  p1=(GEN)e[14];
  if (gcmp0(p1))
  {
    y[14] = y[15] = y[16] = y[17] = y[18] = y[19] = zero;
  }
  else
  {
    p2=cgetg(4,18);
    for(i=1;i<=3;i++) p2[i]=lmul(v2,gsub((GEN)p1[i],r));
    y[14]=(long)p2;
    y[15]=lmul(u,(GEN)e[15]);if(typ((GEN)e[1])==7) y[15]=lmul(u,(GEN)y[15]);
    y[16]=lmul(u,(GEN)e[16]);
    y[17]=ldiv((GEN)e[17],u);y[18]=ldiv((GEN)e[18],u);
    y[19]=lmul(gmul(u,u),(GEN)e[19]);
  }
  tetpil=avma;return gerepile(av,tetpil,gcopy(y));
}

GEN pointch(GEN x, GEN ch)
{
  GEN y,p1,p2,p3,v,v2,v3,mor,r,s,t,u;
  long av,tetpil,tx,lx=lg(x),i;

  if ((typ(x) != 17) || (typ(ch) != 17)) err(elliper1);
  if (lx < 2) return gcopy(x);
  av=avma;u=(GEN)ch[1];r=(GEN)ch[2];s=(GEN)ch[3];t=(GEN)ch[4];
  tx=typ((GEN)x[1]);v=ginv(u);v2=gmul(v,v);v3=gmul(v,v2);
  mor=gneg(r);
  if(tx>=17)
    {
      y=cgetg(lx,tx);
      for(i=1;i<lx;i++)
        {
          p2=(GEN)x[i];
          if(lg(p2)<3) y[i]=(long)p2;
          else
            {
              p1=cgetg(3,17);
              p1[1]=lmul(v2,p3=gadd((GEN)p2[1],mor));
              p1[2]=lmul(v3,gsub((GEN)p2[2],gadd(gmul(s,p3),t)));
              y[i]=(long)p1;
            }
        }
    }
  else
    if(lg(x)<3) y=x;
    else
      {
	y=cgetg(3,17);
	y[1]=lmul(v2,p3=gadd((GEN)x[1],mor));
	y[2]=lmul(v3,gsub((GEN)x[2],gadd(gmul(s,p3),t)));
      }
  tetpil=avma;return gerepile(av,tetpil,gcopy(y));
}

int oncurve(GEN e, GEN z)
{
  GEN p1,p2,x,y;
  long av=avma,f;

  if ((typ(e)!=17)||(lg(e)<6)||(typ(z)!=17)) err(elliper1);
  if(lg(z)<3) return 1; /* cas du point a l'infini */
  x=(GEN)z[1];y=(GEN)z[2];
  p1=gmul(y,gadd(y,gadd((GEN)e[3],gmul((GEN)e[1],x))));
  p2=gadd((GEN)e[5],gmul(x,gadd((GEN)e[4],gmul(x,gadd((GEN)e[2],x)))));
  p1=gsub(p1,p2);
  f=precision(p1);
  if(!f) {f=gcmp0(p1);avma=av;return f;}
  f=gcmp0(p1)||(gexpo(p1)< -((f-2)<<TWOPOTBITS_IN_LONG)+20);avma=av;return f;
}

GEN hells(GEN e, GEN x, long prec)
{
  GEN w,z,t,mu,unreel;
  long av=avma,tetpil,n,lim;

  if(!oncurve(e,x)) err(heller1);
  affsr(1,unreel=cgetr(prec));
  t=gdiv(unreel,(GEN)x[1]);mu=gmul2n(glog(numer((GEN)x[1]),prec),-1);
  n=0;lim=(prec-2)<<(TWOPOTBITS_IN_LONG-1);
  do
    {
      w=gmul(t,gaddsg(4,gmul(t,gadd((GEN)e[6],gmul(t,gadd(gmul2n((GEN)e[7],1),gmul(t,(GEN)e[8])))))
));
      z=gsub(gun,gmul(gmul(t,t),gadd((GEN)e[7],gmul(t,gadd(gmul2n((GEN)e[8],1),gmul(t,(GEN)e[9]))))
));
      mu=gadd(mu,gmul2n(glog(z,prec),-((n<<1)+3)));t=gdiv(w,z);n++;
    }
  while(n<=(lim+5));
  tetpil=avma;return gerepile(av,tetpil,gcopy(mu));
}

GEN hell2(GEN e, GEN x, long prec)
{
  GEN ep,e3,ro,p1,p2,mu,d,xp;
  long av=avma,tetpil,lx,lc,i,j,tx;

  if(!oncurve(e,x)) err(heller1);
  d=(GEN)e[12];ro=(GEN)e[14];e3=(gsigne(d)<0)?(GEN)ro[1]:(GEN)ro[3];
  p1=cgetg(5,17);p1[1]=un;p1[2]=laddgs(gfloor(e3),-1);
  p1[3]=p1[4]=zero;ep=coordch(e,p1);xp=pointch(x,p1);
  if(typ((GEN)x[1])<17)
    {
      if(lg(x)<3) return gzero;
      tetpil=avma;return gerepile(av,tetpil,hells(ep,xp,prec));
    }
  else
    {
      lx=lg(x);tetpil=avma;mu=cgetg(lx,tx=typ(x));
      if(tx<19) for(i=1;i<lx;i++) mu[i]=(long)hells(ep,(GEN)xp[i],prec);
      else
        {
          lc=lg((GEN)x[1]);
          for(i=1;i<lx;i++)
            {
              p1=cgetg(lc,18);mu[i]=(long)p1;p2=(GEN)xp[i];
              for(j=1;j<lc;j++) p1[j]=(long)hells(ep,(GEN)p2[j],prec);
            }
        }
      return gerepile(av,tetpil,mu);
    }
}

GEN addell(GEN e, GEN z1, GEN z2)
{
  GEN y,p1,p2,x1,x2,x3,y1,y2,y3,al;
  long av=avma,tetpil;

  if((typ(e)!=17)||(lg(e)<6)||(typ(z1)!=17)||(typ(z2)!=17)) err(elliper1);
  if(lg(z1)<3) return gcopy(z2);
  if(lg(z2)<3) return gcopy(z1);
  x1=(GEN)z1[1];x2=(GEN)z2[1];y1=(GEN)z1[2];y2=(GEN)z2[2];
  if(gegal(x1,x2))
    if(!gegal(y1,y2)) {y=cgetg(2,17);y[1]=zero;return y;}
    else
      {
       
p1=gadd(gsub((GEN)e[4],gmul((GEN)e[1],y1)),gmul(x1,gadd(gmul2n((GEN)e[2],1),gmulsg(3,x1))));
        p2=gadd((GEN)e[3],gadd(gmul((GEN)e[1],x1),gmul2n(y1,1)));
        if(gcmp0(p2)) {avma=av;y=cgetg(2,17);y[1]=zero;return y;}
      }
  else {p1=gsub(y2,y1);p2=gsub(x2,x1);}
  al=gdiv(p1,p2);
  x3=gsub(gmul(al,gadd(al,(GEN)e[1])),gadd(gadd(x1,x2),(GEN)e[2]));
  y3=gneg(gadd(gadd(gadd((GEN)e[3],y1),gmul(x3,(GEN)e[1])),gmul(al,gsub(x3,x1))));
  tetpil=avma;y=cgetg(3,17);y[1]=lcopy(x3);y[2]=lcopy(y3);
  return gerepile(av,tetpil,y);
}

GEN subell(GEN e, GEN z1, GEN z2)
{
  GEN zp;
  long av=avma,tetpil;

  if((typ(e)!=17)||(lg(e)<6)||(typ(z2)!=17)) err(elliper1);
  if(lg(z2)<3) return gcopy(z1);
  zp=cgetg(3,17);zp[1]=z2[1];
  zp[2]=lneg(gadd(gadd((GEN)z2[2],(GEN)e[3]),gmul((GEN)e[1],(GEN)z2[1])));
  tetpil=avma;return gerepile(av,tetpil,addell(e,z1,zp));
}

GEN ordell(GEN e, GEN x, long prec)
{
  GEN p1,p2,p3,d,y,pn,pd;
  long av=avma,tetpil,td,i,lx,tx=typ(x);
  
  if((typ(e)!=17)||(lg(e)<6)) err(elliper1);
  if(tx<17)
    {
      p1=gadd((GEN)e[5],gmul(x,gadd((GEN)e[4],gmul(x,gadd((GEN)e[2],x)))));
      p2=gadd((GEN)e[3],gmul((GEN)e[1],x));d=gadd(gmul(p2,p2),gmul2n(p1,2));
      if((td=typ(d))==1)
        {
          if(signe(d)<0) {avma=av;return cgetg(1,17);}
          if(!carrecomplet(d,&p3)) {avma=av;return cgetg(1,17);} 
          p1=gsub(p3,p2);tetpil=avma;
          if(gsigne(d)) {y=cgetg(3,17);y[1]=lmul2n(p1,-1);y[2]=lsub((GEN)y[1],p3);}
          else {y=cgetg(2,17);y[1]=lmul2n(p1,-1);}
          return gerepile(av,tetpil,y);
        }
      if((td==4)||(td==5))
	{
	  pn=(GEN)d[1];pd=(GEN)d[2];
	  if(!carrecomplet(mulii(pn,pd),&p3)) {avma=av;return cgetg(1,17);} 
          p1=gsub(p3=gdiv(p3,pd),p2);tetpil=avma;
          if(signe(pn)) {y=cgetg(3,17);y[1]=lmul2n(p1,-1);y[2]=lsub((GEN)y[1],p3);}
          else {y=cgetg(2,17);y[1]=lmul2n(p1,-1);}
          return gerepile(av,tetpil,y);
        }
      if((td==3)&&gegal((GEN)d[1],gdeux))
	{
	  if(gcmp0(p2)) 
	    {
	      avma=av;y=cgetg(2,17);p3=cgetg(3,3);y[1]=(long)p3;
	      p3[1]=deux;p3[2]=gcmp0(p1)?zero:un;
	    }
	  else
	    if(gcmp0(p1))
	      {
		avma=av;y=cgetg(3,17);p3=cgetg(3,3);y[1]=(long)p3;
		p3[1]=deux;p3[2]=zero;p3=cgetg(3,3);y[2]=(long)p3;
		p3[1]=deux;p3[2]=un;
	      }
	    else {avma=av;y=cgetg(1,17);}
	  return y;
	}
      else
	{
	  if((td==3)&&(kronecker((GEN)d[2],(GEN)d[1])== -1))
	    {avma=av;y=cgetg(1,17);return y;}
	  p3=gsqrt(d,prec);p1=gsub(p3,p2);tetpil=avma;
	  if(!gcmp0(d))
	    {
	      y=cgetg(3,17);y[1]=lmul2n(p1,-1);y[2]=lsub((GEN)y[1],p3);
	    }
	  else {y=cgetg(2,17);y[1]=lmul2n(p1,-1);}
	  return gerepile(av,tetpil,y);
	}
    }
  else
    {
      lx=lg(x);y=cgetg(lx,tx);
      for(i=1;i<lx;i++) y[i]=(long)ordell(e,(GEN)x[i],prec);
      return y;
    }
}

GEN powell(GEN e, GEN z, GEN n)
{
  GEN y,zp;
  long s=signe(n),av=avma,i,j,tetpil;
  ulong m;

  if((typ(e)!=17)||(lg(e)<6)||(typ(z)!=17)||(typ(n)!=1)) err(elliper1);
  if(!s) {y=cgetg(2,17);y[1]=zero;return y;}
  if(lg(z)<3) return gcopy(z);
  if(s<0) 
    {
      n=gneg(n);zp=cgetg(3,17);zp[1]=z[1];
      zp[2]=lneg(gadd(gadd((GEN)z[2],(GEN)e[3]),gmul((GEN)e[1],(GEN)z[1])));
    }
  else zp=z;
  if(gcmp1(n)) {tetpil=avma;return gerepile(av,tetpil,gcopy(zp));}
  y=cgetg(2,17);y[1]=zero;
  for (i=lgef(n)-1;i>2;i--)
    for (m=n[i],j=0;j<BITS_IN_LONG;j++,m>>=1)
      {
        if (m&1) y=addell(e,y,zp);
        zp=addell(e,zp,zp);
      }
  for (m=n[2];m>1;m>>=1)
    {
      if (m&1) y=addell(e,y,zp);
      zp=addell(e,zp,zp);
    }
  tetpil=avma;y=addell(e,y,zp);
  return gerepile(av,tetpil,y);
}

GEN mathell(GEN e, GEN x, long prec)
{
  GEN y,p1,p2,pdiag;
  long av=avma,tetpil,lx=lg(x),i,j,tx=typ(x);

  if((tx<17)||(tx>18)) err(elliper1);
  lx=lg(x);y=cgetg(lx,19);pdiag=cgetg(lx,18);
  for(i=1;i<lx;i++) {pdiag[i]=(long)ghell(e,(GEN)x[i],prec);y[i]=lgetg(lx,18);}
  for(i=1;i<lx;i++)
    {
      p1=(GEN)y[i];p1[i]=lmul2n((GEN)pdiag[i],1);
      for(j=i+1;j<lx;j++)
        {
	  p2=gsub(ghell(e,addell(e,(GEN)x[i],(GEN)x[j]),prec),gadd((GEN)pdiag[i],(GEN)pdiag[j]));
          p1[j]=(long)p2;((GEN)y[j])[i]=(long)p2;
        }
    }
  tetpil=avma;return gerepile(av,tetpil,gcopy(y));
}

/* A usage interne seulement. Pas de verifications */

GEN bilhells(GEN e, GEN z1, GEN z2, GEN h2, long prec)
{
  long lz1=lg(z1),tx1=typ(z1),av=avma,tetpil,i;
  GEN y,p1,p2;

  if(lz1==1) return cgetg(1,typ(z1));
  if(typ((GEN)z1[1])<17)
    {
      p1=ghell(e,addell(e,z1,z2),prec);p2=gadd(ghell(e,z1,prec),h2);
      tetpil=avma;return gerepile(av,tetpil,gsub(p1,p2));
    }
  else
    {
      tetpil=avma;y=cgetg(lz1,tx1);
      for(i=1;i<lz1;i++) y[i]=(long)bilhells(e,(GEN)z1[i],z2,h2,prec);
      return gerepile(av,tetpil,y);
    }
}
 
GEN bilhell(GEN e, GEN z1, GEN z2, long prec)
{
  GEN p1,h2;
  long av=avma,tetpil,tz1=typ(z1),tz2=typ(z2);

  if((tz1<17)||(tz2<17)) err(elliper1);
  if(lg(z1)==1) return cgetg(1,tz1);
  if(lg(z2)==1) return cgetg(1,tz2);
  if(typ((GEN)z1[1])<typ((GEN)z2[1])) {p1=z1;z1=z2;z2=p1;}
  if(typ((GEN)z2[1])<17) 
    {
      h2=ghell(e,z2,prec);
      tetpil=avma;return gerepile(av,tetpil,bilhells(e,z1,z2,h2,prec));
    }
  else {err(talker,"two vector/matrix types in bilhell");return gnil;}
}

GEN zell(GEN e, GEN z, long prec)
{
  long av=avma,tetpil,ty,sw,fl;
  GEN t,u,w,p1,p2,b2,b4,r0,r1,aa1,aa0,bb1,bb0,x0,x1,c0,e1,delta;
  GEN bmod0,bmod1,p,u2;

  if((typ(e)!=17)||(lg(e)<20)) err(elliper1);
  if(!oncurve(e,z)) err(heller1);
  ty=typ((GEN)e[12]);b2=(GEN)e[6];b4=(GEN)e[7];
  if(lg(z)<3) return (ty==7)?gun:gzero;
  e1=(GEN)((GEN)e[14])[1];
  w=gsqrt(gmul2n(gadd(b4,gmul(e1,gadd(b2,gmulsg(6,e1)))),1),prec);
  if((ty<=8)&&(ty!=3)&&(ty!=7))
    {
      p2=gadd(gmulsg(3,e1),r0=gmul2n(b2,-2));
      if(gsigne(p2)>0) w=gneg(w);
      aa1=gmul2n(gsub(w,p2),-2);sw=signe(w);r0=gadd(e1,r0);
      bb1=gmul2n(w,-1);r1=gsub(aa1,bb1);
      c0=gadd((GEN)z[1],gmul2n(r0,-1));
      if(gsigne(c0))
	{
	  delta=gdiv(gmul(aa1,r1),gsqr(c0));
	  x0=gmul2n(gmul(c0,gaddsg(1,gsqrt(gsubsg(1,gmul2n(delta,2)),prec))),-1);
	}
      else x0=gsqrt(gneg(gmul(aa1,r1)),prec);
      x1=gmul(x0,gsqr(gmul2n(gaddsg(1,gsqrt(gdiv(gadd(x0,r1),x0),prec)),-1)));
      for(fl=0;;)
        {
          aa0=aa1;bb0=bb1;x0=x1;r0=r1;
          bb1=gsqrt(gmul(aa0,bb0),prec);setsigne(bb1,sw);
          aa1=gmul2n(gadd(gadd(aa0,bb0),gmul2n(bb1,1)),-2);
          r1=gsub(aa1,bb1);
	  x1=gmul(x0,gsqr(gmul2n(gaddsg(1,gsqrt(gdiv(gadd(x0,r1),x0),prec)),-1)));
	  if(gexpo(gsub(x1,x0))<gexpo(x1)-((prec-2)<<TWOPOTBITS_IN_LONG)+5)
	    if (fl) break; else fl = 1;
	  else fl = 0;
        }
      t=gaddsg(1,u=gdiv(x1,aa1));
      if(gcmp0(t)||(gexpo(t)<-((prec-2)<<TWOPOTBITS_IN_LONG)+5)) t=gneg(gun);
      else t=gdiv(u,gsqr(gaddsg(1,gsqrt(t,prec))));
      u=gsqrt(ginv(gmul2n(aa1,2)),prec);t=glog(t,prec);t=gmul(t,u);
/* les deux lignes suivantes ont ete rajoutees au pif. A verifier */
      if(gsigne(c0)*gsigne(gadd((GEN)e[3],gadd(gmul((GEN)e[1],(GEN)z[1]),gmul2n((GEN)z[2],1))))<0)
	t=gneg(t);
      p1=gsub(p2=gdiv(gimag(t),(GEN)((GEN)e[16])[2]),gmul2n(gun,-2));
      if(gcmp(gabs(p1,prec),ghalf)>=0)
        {
          t=gsub(t,gmul((GEN)e[16],gfloor(gadd(p2,lisexpr("0.1")))));
        }
      if(signe(greal(t))<0) t=gadd(t,(GEN)e[15]);
      tetpil=avma;return gerepile(av,tetpil,gcopy(t));
    }
  else 
    if(ty==7)
      {
        w=(GEN)e[18];r0=gadd(e1,gmul2n(b2,-2));p=(GEN)((GEN)e[12])[2];
        aa1=gmul2n(gsub(w,gadd(gmulsg(3,e1),gmul2n(b2,-2))),-2);
        bb1=gmul2n(w,-1);r1=gsub(aa1,bb1);
        c0=gadd((GEN)z[1],gmul2n(r0,-1));delta=gdiv(gmul(aa1,r1),gsqr(c0));
	x0=gmul2n(gmul(c0,gaddsg(1,gsqrt(gsubsg(1,gmul2n(delta,2)),prec))),-1);
        bmod0=modii((GEN)bb1[4],p);
	x1=gmul(x0,gsqr(gmul2n(gaddsg(1,gsqrt(gdiv(gadd(x0,r1),x0),prec)),-1)));
        do
          {
            aa0=aa1;bb0=bb1;x0=x1;r0=r1;
            bb1=gsqrt(gmul(aa0,bb0),0);bmod1=modii((GEN)bb1[4],p);
            if(!gegal(bmod1,bmod0)) bb1=gneg(bb1);
            aa1=gmul2n(gadd(gadd(aa0,bb0),gmul2n(bb1,1)),-2);
            r1=gsub(aa1,bb1);
            p1=gsqrt(gdiv(gadd(x0,r1),x0),0);
            if(!gegal(modii((GEN)p1[4],p),gun)) p1=gneg(p1);
            x1=gmul(x0,gsqr(gmul2n(gaddsg(1,p1),-1)));
          }
        while(!gcmp0(r1));
        u2=ginv(gmul2n(aa1,2));
        if(!gcmp0((GEN)e[16]))
          {
            t=gsqrt(gaddsg(1,gdiv(x1,aa1)),prec);
            t=gdiv(gaddsg(-1,t),gaddsg(1,t));
          }
        else t=gaddsg(2,ginv(gmul(u2,x1)));
        tetpil=avma;return gerepile(av,tetpil,gcopy(t));
      }
    else err(zeller1);return gnil;
}  

GEN pointell(GEN e, GEN z, long prec)
{
  long av=avma,tetpil,dec,lim,av1;
  GEN p1,pii2,pii4,pii6,a,tau,q,u,y,yp,u1,u2,qn,qnu2,qnu1,qnu,qnu4,qnu3,p2,v;

  if((typ(e)!=17)||(lg(e)<20)) err(elliper1);
  p1=mppi(prec);setexpo(p1,2);
  pii2=cgetg(3,6);pii2[1]=zero;pii2[2]=(long)p1;
  z=gdiv(z,(GEN)e[15]);tau=gdiv((GEN)e[16],(GEN)e[15]);
  a=ground(gdiv(gimag(z),gimag(tau)));z=gsub(z,gmul(a,tau));
  a=ground(greal(z));z=gsub(z,a);
  if(gcmp0(z)||gexpo(gnorm(z))< -((prec-2)<<(TWOPOTBITS_IN_LONG+1))+10) {avma=av;v=cgetg(2,17);v[1]=zero;return v;}
  q=gexp(gmul(pii2,tau),prec);u=gexp(gmul(pii2,z),prec);
  y=gadd(gdivgs(gun,12),gdiv(u,u2=gsqr(u1=gsubsg(1,u))));
  yp=gdiv(gaddsg(1,u),gmul(u1,u2));
  lim=(avma+bot)>>1;av1=avma;
  qn=q;pii2=gdiv(pii2,(GEN)e[15]);pii4=gmul(pii2,pii2);pii6=gmul(pii4,pii2);
  do
    {
      p1=gsub(gmul(u,gadd(gdivsg(1,qnu2=gsqr(qnu1=gsubsg(1,qnu=gmul(qn,u)))),gdivsg(1,qnu4=gsqr(qnu3=gsub(qn,u))))),gmul2n(gdivsg(1,gsqr(gsubsg(1,qn))),1));
      y=gadd(y,gmul(qn,p1));
      p2=gadd(gdiv(gaddsg(1,qnu),gmul(qnu1,qnu2)),gdiv(gadd(qn,u),gmul(qnu3,qnu4)));
      yp=gadd(yp,gmul(qn,p2));qn=gmul(q,qn);
      if(avma<lim)
	{
	  tetpil=avma;y=gcopy(y);yp=gcopy(yp);qn=gcopy(qn);
	  dec=lpile(av1,tetpil,0)>>TWOPOTBYTES_IN_LONG;
	  y+=dec;yp+=dec;qn+=dec;
	}
    }
  while(gexpo(qn)>-((prec-2)<<TWOPOTBITS_IN_LONG)-5);
  yp=gmul(u,gmul(pii6,yp));y=gsub(gmul(pii4,y),gdivgs((GEN)e[6],12));
  yp=gsub(yp,gadd((GEN)e[3],gmul((GEN)e[1],y)));
  tetpil=avma;v=cgetg(3,17);v[1]=lcopy(y);v[2]=lmul2n(yp,-1);
  return gerepile(av,tetpil,v);
}


GEN apell2(GEN e, GEN p)
{
/* Calcul de a_p par les sommes de Jacobi */
  GEN y,e71;
  long av=avma,av2,i,l,s,e72,e6,e8;

  if((typ(e)!=17)||(lg(e)<14)) err(elliper1);
  if(lgef(p)>3) err(apeller1);
  s=0;l=p[2];e71=gmul2n((GEN)e[7],1);av2=avma;
  if((ulong)l>=((ulong)HIGHBIT>>1)) err(apeller1);
  if((l<757)&&(l>2))
    {
      e6=itos(modis((GEN)e[6],l));e72=(itos(modis((GEN)e[7],l)))<<1;
      e8=itos(modis((GEN)e[8],l));
      for(i=0;i<l;i++) s=s+kross(e8+i*(e72+i*(e6+(i<<2))),l);
    }
  else
    {
      for(i=0;i<l;i++)
        {
          y=gadd((GEN)e[8],gmulsg(i,gadd(e71,gmulsg(i,gaddsg(i<<2,(GEN)e[6])))));
          s=s+krogs(y,l);avma=av2;
        }
    }
  avma=av;return stoi(-s);
}

GEN addsell(GEN e, GEN z1, GEN z2)
{
  GEN y,p1,p2,x1,x2,x3,y1,y2,y3,al;
  long av=avma,tetpil;

  if(lg(z1)<3) return gcopy(z2);
  if(lg(z2)<3) return gcopy(z1);
  x1=(GEN)z1[1];x2=(GEN)z2[1];y1=(GEN)z1[2];y2=(GEN)z2[2];
  if(gegal(x1,x2))
    {
      if(!gegal(y1,y2)) {y=cgetg(2,17);y[1]=zero;return y;}
      else
        {
          p1=gadd(e,gmul(x1,gmulsg(3,x1)));
          p2=gmul2n(y1,1);
          if(gcmp0(p2)) {avma=av;y=cgetg(2,17);y[1]=zero;return y;}
        }
    }
  else {p1=gsub(y2,y1);p2=gsub(x2,x1);}
  al=gdiv(p1,p2);
  x3=gsub(gmul(al,al),gadd(x1,x2));
  y3=gneg(gadd(y1,gmul(al,gsub(x3,x1))));
  tetpil=avma;y=cgetg(3,17);y[1]=lcopy(x3);y[2]=lcopy(y3);
  return gerepile(av,tetpil,y);
}

GEN doubsell(GEN e, GEN z1)
{
  GEN x1,x3,y3,y,y1,p1,p2,al;
  long av=avma,tetpil;

  if(lg(z1)<3) return gcopy(z1);
  x1=(GEN)z1[1];y1=(GEN)z1[2];
  p1=gadd(e,gmul(x1,gmulsg(3,x1)));p2=gmul2n(y1,1);
  if(gcmp0(p2)) {avma=av;y=cgetg(2,17);y[1]=zero;return y;}
  al=gdiv(p1,p2);
  x3=gsub(gmul(al,al),gadd(x1,x1));
  y3=gneg(gadd(y1,gmul(al,gsub(x3,x1))));
  tetpil=avma;y=cgetg(3,17);y[1]=lcopy(x3);y[2]=lcopy(y3);
  return gerepile(av,tetpil,y);
}

GEN subsell(GEN e, GEN z1, GEN z2)
{
  GEN zp;
  long av=avma,tetpil;
  
  if(lg(z2)<3) return gcopy(z1);
  zp=cgetg(3,17);zp[1]=z2[1];
  zp[2]=lneg((GEN)z2[2]);
  tetpil=avma;return gerepile(av,tetpil,addsell(e,z1,zp));
}


GEN powsell(GEN e, GEN z, GEN n)
{
  GEN y,zp;
  long s=signe(n),av=avma,i,j,tetpil;
  ulong m;

  if(!s) {y=cgetg(2,17);y[1]=zero;return y;}
  if(lg(z)<3) return gcopy(z);
  if(s<0) 
    {
      n=gneg(n);zp=cgetg(3,17);zp[1]=z[1];
      zp[2]=lneg((GEN)z[2]);
    }
  else zp=z;
  if(gcmp1(n)) {tetpil=avma;return gerepile(av,tetpil,gcopy(zp));}
  y=cgetg(2,17);y[1]=zero;
  for (i=lgef(n)-1;i>2;i--)
    {
      for (m=n[i],j=0;j<BITS_IN_LONG;j++,m>>=1)
        {
          if (m&1) y=addsell(e,y,zp);
          zp=doubsell(e,zp);
        }
    }
  for (m=n[2];m>1;m>>=1)
    {
      if (m&1) y=addsell(e,y,zp);
      zp=doubsell(e,zp);
    }
  tetpil=avma;y=addsell(e,y,zp);
  return gerepile(av,tetpil,y);
}

GEN powssell(GEN e, GEN z, long n)
{
  GEN y,zp;
  long av=avma,tetpil;
  ulong m;

  if(!n) {y=cgetg(2,17);y[1]=zero;return y;}
  if(lg(z)<3) return gcopy(z);
  if(n<0) 
    {
      n= -n;zp=cgetg(3,17);zp[1]=z[1];
      zp[2]=lneg((GEN)z[2]);
    }
  else zp=z;
  if(n==1) {tetpil=avma;return gerepile(av,tetpil,gcopy(zp));}
  y=cgetg(2,17);y[1]=zero;
  for (m=n;m>1;m>>=1)
    {
      if (m&1) y=addsell(e,y,zp);
      zp=doubsell(e,zp);
    }
  tetpil=avma;y=addsell(e,y,zp);
  return gerepile(av,tetpil,y);
}

#define HASHSP 255

GEN apell1(GEN e, GEN p)
{
  long av,av3,tetpil,k,k2,i,j,j1,lim,limite,succes,com,j2,s;
  GEN tabla, tablb, count, index, hash;
  GEN p1,p2,p3,q,h,hp,f,fh,fg,ftest;
  GEN unmodp,pordmin,u,p1p,p2p,acon,bcon,xp,yp,c4,c6,cp4;
  long flc,flcc,sucfin,fll,flf,x,pfinal;

  if((typ(e)!=17)||(lg(e)<14)) err(elliper1);
  if(gcmpgs(p,20)<0) return apell2(e,p);
  if(gexpo(p)>85) err(impl,"apell for p>10^25");
  tabla = newbloc(1000000);
  tablb = newbloc(1000000);
  count = newbloc(256);
  index = newbloc(257);
  hash = newbloc(1000000);

  av=avma;limite=(av+bot)>>1;
  unmodp=gmodulcp(gun,p);c4=gdivgs(gmul(unmodp,(GEN)e[10]),-48);
  c6=gdivgs(gmul(unmodp,(GEN)e[11]),-864);
  pordmin=gceil(gmul2n(gsqrt(p,4),2));p2p=gmul2n(p1p=gaddsg(1,p),1);
  x=0;flcc=0;flc=kronecker((GEN)c6[2],p);u=c6;acon=gzero;bcon=gun;
  sucfin=1;h=p1p;
  while(sucfin)
    {
      while((flc==flcc)||(!flc))
        {
          x++;u=gadd(c6,gmulsg(x,gaddgs(c4,x*x)));
          flc=kronecker((GEN)u[2],p);
        }
      flcc=flc;
      s=itos(gceil(gsqrt(gdiv(pordmin,bcon),3)))>>1;
      cp4=gmul(c4,yp=gsqr(u));
      xp=gmulsg(x,u);f=cgetg(3,17);f[1]=(long)xp;f[2]=(long)yp;
      fh=powsell(cp4,f,h);
      if (bcon != gun) f=powsell(cp4,f,bcon); /* sic */
      p1=fh;flf=1;
      for(i=0;i<=HASHSP;i++) count[i]=0;
      for(i=0;(i<=s-1)&&flf;i++)
        {
          if(lg(p1)==3)
            {
             
p2=(GEN)((GEN)p1[1])[2];tabla[i]=p2[lgef(p2)-1];j=tabla[i]&HASHSP;count[j]++;
/*       printf("%d ",j);fflush(stdout); */
              p2=(GEN)((GEN)p1[2])[2];tablb[i]=p2[lgef(p2)-1];
              p1=addsell(cp4,p1,f);
            }
          else flf=0;
        }
/*      printf("\nsj:\n"); */
      if(flf)
        {
          fg=powssell(cp4,f,s);ftest=fg;
          index[0]=0;for(i=0;i<=HASHSP-1;i++) index[i+1]=index[i]+count[i];
          for(i=0;i<=s-1;i++) hash[index[tabla[i]&HASHSP]++]=i;
          index[0]=0;for(i=0;i<=HASHSP;i++) index[i+1]=index[i]+count[i];
          succes=0;com=1;av3=avma;pfinal=p[lgef(p)-1];
          while(!succes)
            {
              p1=(GEN)((GEN)ftest[1])[2];k=p1[lgef(p1)-1];j=k&HASHSP;
/*      printf("%d ",j);fflush(stdout); */
              fll=(lg(ftest)==3);
              for(j1=index[j];(j1<index[j+1])&&(!succes);j1++)
                {
                  j2=hash[j1];
                  if((tabla[j2]==k)&&fll)
                    {
                      p2=(GEN)((GEN)ftest[2])[2];k2=p2[lgef(p2)-1];
                      if((tablb[j2]==k2)||(tablb[j2]==pfinal-k2))
                        {
                          p1=addsell(cp4,powssell(cp4,f,j2),fh);
                          succes=gegal((GEN)p1[1],(GEN)ftest[1]);
                        }
                    }
                }
              if(!succes)
                {
                  com++;
                  if(avma>=limite) ftest=addsell(cp4,ftest,fg);
                  else 
                    {
                     
tetpil=avma;ftest=gerepile(av3,tetpil,addsell(cp4,ftest,fg));
                    }
                  if (lg(ftest)<3) err(apeller2);
                }
            }
          h=addii(h,mulsi(j2,bcon));p2=mulsi(s,mulsi(com,bcon));
	  h=(!cmpii((GEN)((GEN)p1[2])[2],(GEN)((GEN)ftest[2])[2]))?subii(h,p2):addii(h,p2);
        }
      else h=addii(h,mulsi(i-1,bcon));
      p2=factor(h);
      p1=(GEN)p2[1];
      p2=(GEN)p2[2];
      for(i=1;i<lg(p1);i++)
        {
          p3=divii(h,(GEN)p1[i]);fh=powsell(cp4,f,p3);lim=itos((GEN)p2[i]);
          for(j=1;(j<=lim)&&(lg(fh)<3);j++)
            {
              h=p3;if(j<lim) {p3=divii(h,(GEN)p1[i]);fh=powsell(cp4,f,p3);}
            }
        }
      p1=gmodulcp(acon,bcon);p2=gmodulcp(gzero,h);
      p1=chinois(p1,p2);acon=(GEN)p1[2];bcon=(GEN)p1[1];
      if(gcmp(bcon,pordmin)>=0)
        {
          q=ground(gdiv(gsub(p1p,acon),bcon));sucfin=0;
          hp=addii(mulii(q,bcon),acon);tetpil=avma;
        }
      else
        {
          acon=modii(subii(p2p,acon),bcon);
          p1=subii(acon,bcon);if(signe(addii(acon,p1))>0) acon=p1;
          q=ground(gdiv(gsub(p1p,acon),bcon));
          h=addii(mulii(q,bcon),acon);
        }
    }
  killbloc(tabla); killbloc(tablb); killbloc(count);
  killbloc(index); killbloc(hash);
  return
(flc==1)?gerepile(av,tetpil,gsub(p1p,hp)):gerepile(av,tetpil,gsub(hp,p1p));
}

typedef struct {int isnull; long x; long y;} sellpt;

long addmod(long a, long b, long p)
{
  long res = a + b;
  return (res >= p) ? res - p : res;
}

long submod(long a, long b, long p)
{
  long res = a - b;
  return (res >= 0) ? res : res + p;
}

extern long mulmodll(ulong,ulong,ulong);
#define mulmod(a,b,c) mulmodll(a,b,c)

static long divmod(long a, long b, long p)
{
  long v1 = 0, v2 = 1, v3, r, oldp = p;

  while (b > 1)
    {
      v3 = v1 - (p / b) * v2; v1 = v2; v2 = v3;
      r = p % b; p = b; b = r;
    }

  if (v2 < 0) v2 += oldp;
  return mulmod(a, v2, oldp);
}
  
void addsell1(long e, long p, sellpt *p1, sellpt *p2, sellpt *p3)
{
  long num, den, lambda;
  if (p1->isnull) {*p3 = *p2; return;}
  if (p2->isnull) {*p3 = *p1; return;}
  if (p1->x == p2->x)
    if ((p1->y)&&(p1->y == p2->y))
      {
        num = addmod(e, mulmod(3, mulmod(p1->x, p1->x, p), p), p);
        den = addmod(p1->y, p1->y, p);
      }
    else
      {
        p3->isnull = 1;
        return;
      }
  else
    {
      num = submod(p1->y, p2->y, p);
      den = submod(p1->x, p2->x, p);
    }
  lambda = divmod(num, den, p);
  p3->x = submod(mulmod(lambda, lambda, p), addmod(p1->x, p2->x, p), p);
  p3->y = submod(mulmod(lambda, submod(p2->x, p3->x, p), p), p2->y, p);
  p3->isnull = 0;
}

void powssell1(long e, long p, long n, sellpt *p1, sellpt *p2)
{
  sellpt p4, p3;
  p3 = *p1;
  if (n < 0) {n = -n; if (p3.y) p3.y = p - p3.y;}
  p2->isnull = 1;
  for(;;)
    {
      if (n&1) addsell1(e, p, p2, &p3, p2);
      n>>=1;
      if (!n) return;
      addsell1(e, p, &p3, &p3, &p4);
      p3 = p4;
    }
}

typedef struct {long x; long y; long i;} multiple;

int compare_multiples(multiple *a, multiple *b)
{
  return a->x - b->x;
}

GEN apell(GEN e, GEN pl)
{
  long av = avma,i,j,com,s;
  GEN p1,p2,unmodp;
  sellpt f,fh,fg,ftest,f2;
  long pordmin,u,p1p,p2p,acon,bcon,xp,yp,c4,c6,cp4;
  long flb,flc,flcc,x, q, h, p3, p, l, r, m;
  multiple *table;
 
  if((typ(e)!=17)||(lg(e)<14)) err(elliper1);
  if(divise((GEN)e[12],pl)) return stoi(((((pl[lgef(pl)-1]&3)+1)&2)-1)*kronecker((GEN)e[11],pl));
  if (gcmpgs(pl, 0x3fffffff) > 0) return apell1(e, pl);
  p = itos(pl);
  if (p <= 457) return apell2(e, pl);
  unmodp = gmodulcp(gun, pl);
  c4 = itos((GEN)gdivgs(gmul(unmodp,(GEN)e[10]), -48)[2]);
  c6 = itos((GEN)gdivgs(gmul(unmodp,(GEN)e[11]), -864)[2]);
  pordmin = (long)(1 + 4 * sqrt((float)p));
  p1p = p + 1; p2p = p1p << 1;
  x = 0; flcc = 0; flc = kross(c6, p); u = c6; acon = 0; bcon = 1;
  h = p1p;
  for(;;)
    {
      while((flc == flcc) || (!flc))
        {
          x++;
          u = addmod(c6, mulmod(x, c4 + mulmod(x, x, p), p), p);
          flc = kross(u,p);
        }
      flcc = flc;
      s = (long)(sqrt(((float)pordmin)/bcon) / 2);
      if (!s) s = 1;
      if(bcon==1) table=(multiple *)malloc((s+1)*sizeof(multiple));
      yp = mulmod(u, u, p);
      cp4 = mulmod(c4, yp, p);
      xp = mulmod(x, u, p);
      f.isnull = 0; f.x = xp; f.y = yp;
      powssell1(cp4, p, h, &f, &fh);
      if (bcon > 1) powssell1(cp4, p, bcon, &f, &f2);else f2=f;
      for(i=0; i < s; i++)
        if (fh.isnull)
          {
            h += bcon * i;
            goto trouve;
          }
        else
          {
            table[i].x = fh.x;
            table[i].y = fh.y;
            table[i].i = i;
            addsell1(cp4, p, &fh, &f2, &fh);
          }
      qsort(table, s, sizeof(multiple), (int (*) (const void *, const void *))compare_multiples);
      powssell1(cp4, p, s, &f2, &fg); ftest = fg;
      for(com = 1;; com++)
        {
          if (ftest.isnull) err(apeller3);
          l = 0; r = s;
          while (l < r)
            {
              m = (l + r) >> 1;
              if (table[m].x < ftest.x) l = m + 1; else r = m;
            }
          if ((r < s) && (table[r].x == ftest.x)) break;
          addsell1(cp4, p, &ftest, &fg, &ftest);
        }
      h += table[r].i * bcon;
      if (table[r].y == ftest.y) h -= s * com * bcon; else h += s * com *
bcon;

    trouve:
      p2=factor(stoi(h));
      p1=(GEN)p2[1];
      p2=(GEN)p2[2];
      for(i=1; i < lg(p1); i++)
        for(j = ((GEN)p2[i])[2]; j; j--)
        {
          p3 = h / ((GEN)p1[i])[2];
          powssell1(cp4, p, p3, &f, &fh);
          if (!fh.isnull) break;
          h = p3;
        }
      flb=0;
      if (bcon > 1)
        {
          p1 = gmodulcp(stoi(acon), stoi(bcon)); p2=gmodulcp(gzero, stoi(h));
          p1=chinois(p1,p2);acon=itos((GEN)p1[2]);bcon=((GEN)p1[1])[2];
	  if((bcon<0)||(lgef((GEN)p1[1])>3)) flb=1;
        }
      else
        bcon = h;
      if(flb||(bcon >= pordmin))
        {
          if(flb) h=acon;
	  else
	    {
	      q = ((ulong)(p2p + bcon - (acon << 1))) / (bcon << 1);
	      h = acon + q * bcon;
	    }
          break;
        }
      else
        {
          acon = (p2p - acon) % bcon;
          if ((acon << 1) > bcon) acon -= bcon;
          q = ((ulong)(p2p + bcon - (acon << 1))) / (bcon << 1);
          h = acon + q * bcon;
        }
    }
  avma = av;free(table);
  return stoi((flc == 1) ?  p1p - h : h - p1p);
}

#ifdef LONG_IS_32BIT
#define TEMPC 46337
#endif
#ifdef LONG_IS_64BIT
#define TEMPC 3037000493
#endif

/* TEMPC is the largest prime whose square is less than HIGHBIT */

GEN anell(GEN e, long n)
{
  long p, pk, i, m, av, tetpil, pl[3];
  GEN p1, p2, ap, an;
  
  if((typ(e)!=17)||(lg(e)<14)) err(elliper1);
  if (n <= 0) return cgetg(1, 17);
#ifdef LONG_IS_32BIT
  if(n>TEMPC) err(impl,"anell for n>=46338");
#endif
#ifdef LONG_IS_64BIT
  if(n>TEMPC) err(impl,"anell for n>=3037000494");
#endif
  an = cgetg(n+1, 17);
  an[1] = un;
  for(i=2; i <= n; i++) an[i] = 0;
  pl[0] = evaltyp(1)+evalpere(1)+evallg(3); pl[1] = evalsigne(1)+evallgef(3);
  for (p = 2; p <= n; p++) if (!an[p])
    {
      pl[2] = p;
      if (divise((GEN)e[12], pl)) /* mauvaise reduction */
        switch (((((p & 3) + 1) & 2) -1) * krogs((GEN)e[11], p)) /* renvoie (-c6 /
p) */
          {
          case -1:  /* non deployee */
            for(m = p; m <= n; m += p) if (an[m / p]) an[m] = lneg((GEN)an[m/p]);
            continue;
          case 0:   /* additive */
            for(m = p; m <= n; m += p) an[m] = zero;
            continue;
          case 1:   /* deployee */
            for(m = p; m <= n; m += p) if (an[m / p]) an[m] = lcopy((GEN)an[m/p]);
          }
      else /* bonne reduction */
        for(pk = p; pk <= n; pk *= p)
          {
            if (pk == p)
              an[pk] = (long)(ap = apell(e, pl));
            else
              {
                av = avma;
                p1 = mulii(ap, (GEN)an[pk / p]);
                p2 = mulsi(p, (GEN)an[pk / p / p]);
                tetpil = avma;
                an[pk] = lpile(av, tetpil, subii(p1, p2));
              }
            for(m = n / pk; m > 1; m--)
              if (an[m] && (m % p)) an[m * pk] = lmulii((GEN)an[m], (GEN)an[pk]);
          }
    }
  return an;
}

GEN akell(GEN e, GEN n)
{
  long i,j,ex,av=avma,tetpil;
  GEN p1,p2,ap,u,v,w,fac,y,pl;
  
  if((typ(e)!=17)||(lg(e)<14)) err(elliper1);
  if(typ(n)!=1) err(akeller1);
  if(signe(n)<= 0) return gzero;
  y=gun;if(gcmp1(n)) return y;
  fac=auxdecomp(n,1);p1=(GEN)fac[1];p2=(GEN)fac[2];
  for(i=1;(i<lg(p1))&&(!gcmp0(y));i++)
    {
      pl=(GEN)p1[i];
      if (divise((GEN)e[12], pl)) /* mauvaise reduction */
        {
	  j=((((pl[lgef(pl)-1]&3)+1)&2)-1)*kronecker((GEN)e[11],pl);
	  if((j<0)&&(mpodd((GEN)p2[i]))) {tetpil=avma;y=gneg(y);}
	  if(!j) {avma=av;y=gzero;}
	}
      else /* bonne reduction */
	{
	  ap=apell(e,pl);ex=itos((GEN)p2[i]);
	  u=ap;v=gun;
	  for(j=2;j<=ex;j++)
	    {
	      w=gsub(gmul(ap,u),gmul(pl,v));
	      v=u;u=w;
	    }
	  tetpil=avma;y=gmul(u,y);
	}
    }
  return (av==avma)?y:gerepile(av,tetpil,y);
}

GEN hell(GEN e, GEN a, long prec)
{
  long av=avma,tetpil,n;
  GEN p1,p2,y,z,q,psi2,pi2surw,pi2isurw,qn,ps;

  if((typ(e)!=17)||(lg(e)<20)) err(elliper1);
  pi2surw=gdiv(gmul2n(mppi(prec),1),(GEN)e[15]);
  pi2isurw=cgetg(3,6);pi2isurw[1]=zero;pi2isurw[2]=(long)pi2surw;
  z=gmul(greal(zell(e,a,prec)),pi2surw);
  q=greal(gexp(gmul((GEN)e[16],pi2isurw),prec));
  psi2=gadd((GEN)e[3],gadd(gmul((GEN)e[1],(GEN)a[1]),gmul2n((GEN)a[2],1)));
  y=gsin(z,prec);n=0;qn=gun;ps=gneg(q);
  do
    {
      n++;p1=gsin(gmulsg(n+n+1,z),prec);qn=gmul(qn,ps);
      ps=gmul(ps,q);p1=gmul(p1,qn);
      y=gadd(y,p1);
    }
  while(gexpo(qn)>= -((prec-2)<<TWOPOTBITS_IN_LONG));
  p1=gmul(gsqr(gdiv(gmul2n(y,1),psi2)),pi2surw);
  p2=gsqr(gsqr(gdiv(p1,gsqr(gsqr(denom((GEN)a[1]))))));
  p1=gdiv(gmul(p2,q),(GEN)e[12]);
  p1=gmul2n(glog(gabs(p1,prec),prec),-5);
  tetpil=avma;return gerepile(av,tetpil,gneg(p1));
}

GEN ghell(GEN e, GEN a, long prec)
{
  long av=avma,av2,tetpil,lx,i,n,n2,grandn,ta=typ(a);
  GEN p,p1,p2,x,y,z,phi2,psi2,psi3,logdep;

/* On suppose que la courbe est a coefficients entiers donne par un
modele minimal */

  if((typ(e)!=17)||(lg(e)<20)||(ta<17)) err(elliper1);
  if((lx=lg(a))==1) return cgetg(1,ta);
  if(typ((GEN)a[1])>=17)
    {
      z=cgetg(lx,ta);for(i=1;i<lx;i++) z[i]=(long)ghell(e,(GEN)a[i],prec);
      return z;
    }
  else
    {
      if(!oncurve(e,a)) err(heller1);
      if(lg(a)<3) return gzero;
      x=(GEN)a[1];y=(GEN)a[2];
      psi2=numer(gadd((GEN)e[3],gadd(gmul((GEN)e[1],x),gmul2n(y,1))));
      p2=gadd(gmulsg(3,(GEN)e[7]),gmul(x,gadd((GEN)e[6],gmulsg(3,x))));
      psi3=numer(gadd((GEN)e[9],gmul(x,gadd(gmulsg(3,(GEN)e[8]),gmul(x,p2)))));
      if((!signe(psi2))||(!signe(psi3))) {avma=av;return gzero;}
      phi2=numer(gsub(gadd((GEN)e[4],gmul(x,gadd(shifti((GEN)e[2],1),gmulsg(3,x)))),gmul((GEN)e[1],y)));
      p1=(GEN)factor(mppgcd(psi2,phi2))[1];lx=lg(p1);
      tetpil=avma;z=hell(e,a,prec);av2=avma;
      for(i=1;i<lx;i++)
	{
	  p=(GEN)p1[i];n2=ggval(psi2,p);logdep=gneg(glog(p,prec));
	  if(signe(resii((GEN)e[10],p)))
	    {
	      grandn=ggval((GEN)e[12],p);n=n2<<1;
	      if(grandn)
		{
		  if(n>grandn) n=grandn;
		  p2=divrs(mulsr(n*(grandn+grandn-n),logdep),(grandn<<3));
		  tetpil=avma;z=gadd(z,p2);
		}
	      else avma=av2;
	    }
	  else
	    {
	      n=ggval(psi3,p);
	      if(n>=3*n2) p2=gdivgs(mulsr(n2,logdep),3);
	      else p2=gmul2n(mulsr(n,logdep),-3);
	      tetpil=avma;z=gadd(z,p2);
	    }
	}
      return gerepile(av,tetpil,z);
    }
}

GEN ghell2(GEN e, GEN a, long prec)
{
  long av=avma,tetpil,av2,lx,i,n,n2,grandn;
  GEN p,p1,p2,x,y,z,phi2,psi2,psi3,logdep;

/* On suppose que la courbe est a coefficients entiers donne par un
modele minimal */

  if((typ(e)!=17)||(lg(e)<20)) err(elliper1);
  if(!oncurve(e,a)) err(heller1);
  if(lg(a)<3) return gzero;
  x=(GEN)a[1];y=(GEN)a[2];
  psi2=numer(gadd((GEN)e[3],gadd(gmul((GEN)e[1],x),gmul2n(y,1))));
  p2=gadd(gmulsg(3,(GEN)e[7]),gmul(x,gadd((GEN)e[6],gmulsg(3,x))));
  psi3=numer(gadd((GEN)e[9],gmul(x,gadd(gmulsg(3,(GEN)e[8]),gmul(x,p2)))));
  if(!signe(psi2)) return gzero;
  phi2=numer(gsub(gadd((GEN)e[4],gmul(x,gadd(shifti((GEN)e[2],1),gmulsg(3,x)))),gmul((GEN)e[1],
y)));
  p1=(GEN)factor(mppgcd(psi2,phi2))[1];lx=lg(p1);
  tetpil=avma;z=hell2(e,a,prec);av2=avma;
  if(lx==1) return gerepile(av,tetpil,z);
  for(i=1;i<lx;i++)
    {
      p=(GEN)p1[i];n2=ggval(psi2,p);logdep=gneg(glog(p,prec));
      if(signe(resii((GEN)e[10],p)))
        {
          grandn=ggval((GEN)e[12],p);n=n2<<1;
	  if(grandn)
	    {
	      if(n>grandn) n=grandn;
	      p2=divrs(mulsr(n*(grandn+grandn-n),logdep),(grandn<<3));
	      tetpil=avma;z=gadd(z,p2);
	    }
	  else avma=av2;
        }
      else
        {
          n=ggval(psi3,p);
          if(n>=3*n2) p2=gdivgs(mulsr(n2,logdep),3);
          else p2=gmul2n(mulsr(n,logdep),-3);
          tetpil=avma;z=gadd(z,p2);
        }
    }
  return gerepile(av,tetpil,z);
}

GEN lseriesell(GEN e, GEN s, GEN N, GEN A, long prec)
{
  long av=avma,av1,tetpil,lim,l,n,eps;
  GEN z,p1,p2,cg,cg1,v,cga,cgb,s2,ns,gs;

  lim=(avma+bot)>>1;
  if((typ(e)!=17)||(lg(e)<14)||(typ(N)!=1)||(!signe(N))) err(elliper1);
  if(gsigne(A)<=0) err(lserieser1);
  if(gcmpgs(A,1)<0) A=ginv(A);
  eps=signe(N);if(eps<0) N=gneg(N);
  cg1=mppi(prec);setexpo(cg1,2);cg=divrr(cg1,gsqrt(N,prec));
  cga=gmul(cg,A);cgb=gdiv(cg,A);
  l=(long)((C2*(prec-2)+fabs(gtodouble(s)-1.)*log(rtodbl(cga)))/rtodbl(cgb)+1);
  v=anell(e,min(l,TEMPC));
  s2=gsubsg(2,s);ns=gpui(cg,gsubgs(gmul2n(s,1),2),prec);
  z=gzero;
  if(typ(s)==1)
    {
      if(signe(s)>0) gs=mpfactr(itos(s)-1,prec);
      else {avma=av;return gzero;}
    }
  else gs=ggamma(s,prec);/* gs2=ggamma(s2,prec); */
  av1=avma;
  for(n=1;n<=l;n++)
    {
      p1=gdiv(incgam4(s,gmulsg(n,cga),gs,prec),gpui(stoi(n),s,prec));
      p2=gdiv(gmul(ns,incgam(s2,gmulsg(n,cgb),prec)),gpui(stoi(n),s2,prec));
      if(eps<0) p2=gneg(p2);
      z=gadd(z,gmul(gadd(p1,p2),(n<=TEMPC)?(GEN)(v[n]):akell(e,stoi(n))));
      if(avma<lim) {tetpil=avma;z=gerepile(av1,tetpil,gcopy(z));}
    }
  tetpil=avma;return gerepile(av,tetpil,gdiv(z,gs));
}
    
/********************************************************************/
/********************************************************************/
/**                                                                **/
/**                     Algorithme de Tate                         **/
/**                       (cf Anvers IV)                           **/
/**             Type de Kodaira, modele minimal global             **/
/**                                                                **/
/********************************************************************/
/********************************************************************/

/*
  Etant donnes une courbe elliptique sous forme longue e, dont les coefficients
  sont entiers, et un nombre premier p1, renvoie le type de la fibre en p du
  modele de Neron de la courbe, ainsi que les changements de variables 
  necessaires, sous la forme d'un quadruplet [f, kod, v, c].
  
  L'entier f est l'exposant du conducteur.
  
  l'entier kod, est le type de kodaira. les types II, III et IV sont codes 2,
  3, et 4 respectivement. Les types II*, III* et IV* donnent -2, -3 et -4. Le
  type I0* donne -1, les types Inu et Inu* donnent 4+nu et -4-nu. Enfin, le
  type I0 donne 1.
  
  v est un quadruplet [u, r, s, t] qui permet de passer a un modele minimal.
  En general, on ne s'interessera a ce vecteur que si u <> 1.

  c est le nombre de Tamagawa.
  
  L'algorithme est bien sur celui de Tate dans Anvers IV. Compte tenu des
  remarques du bas de la page 46, l'algorithme long n'est utilise que pour 
  p = 2 ou p = 3.
  
 */

static void cumule(GEN *vtotal, GEN *e, GEN u, GEN r, GEN s, GEN t), cumule1(GEN *vtotal, GEN *e, GEN v2);

GEN localreduction(GEN e, GEN p1)
{
  long av = avma, tetpil, k, p, f, kod, c, nu, nuj, nudelta;
  int a21, a42, a63, a32, a64, theroot, al, be, ga;
  GEN pk, p2k, pk1, p4, p6, a2prime, a3prime;
  GEN p2, p3, r = gzero, s = gzero, t = gzero, v, result;
  GEN c4, c6, delta, unmodp, xun, tri, var, p4k, p6k;
  
  if ((typ(e) != 17) || (lg(e) < 14)) err(elliper1);
  
  v = cgetg(5,17); v[1] = un; v[2] = v[3] = v[4] = zero;
  nudelta = ggval((GEN)e[12], p1);
  if (gcmpgs(p1, 3) > 0)       /* p different de 2 ou 3 */
    {
      nuj = gcmp0((GEN)e[13]) ? 0 : - ggval((GEN)e[13], p1);
      k = (nuj > 0 ? nudelta - nuj : nudelta) / 12;
      c4 = (GEN)e[10]; c6 = (GEN)e[11]; delta = (GEN)e[12];
      if (k > 0) /* modele non minimal */
        {
          pk = gpuigs(p1, k);
          if (mpodd((GEN)e[1]))
            s = shifti(subii(pk, (GEN)e[1]), -1);
          else
            s = negi(shifti((GEN)e[1], -1));
          p2k = mulii(pk, pk);
          p4k = mulii(p2k, pk);
          p6k = mulii(p4k, p2k);
          
          a2prime = subii((GEN)e[2], mulii(s, addii((GEN)e[1], s)));
          switch(itos(modis(a2prime, 3)))
            {
            case 0: r = negi(divis(a2prime, 3)); break;
            case 1: r = divis(subii(p2k, a2prime), 3); break;
            case 2: r = negi(divis(addii(a2prime, p2k), 3)); break;
            default: err(tater1);
            }
          a3prime = addii((GEN)e[3], mulii(r, (GEN)e[1]));
          if (mpodd(a3prime))
            t = shifti(subii(mulii(pk, p2k), a3prime), -1);
          else
            t = negi(shifti(a3prime, -1));
          v[1] = (long)pk; v[2] = (long)r; v[3] = (long)s; v[4] = (long)t;
          nudelta -= 12 * k;
          c4 = divii(c4, p4k); c6 = divii(c6, p6k);
          delta = divii(delta, mulii(p6k, p6k));
        }
      if (nuj > 0) switch(nudelta - nuj)
        {
        case 0: f = 1; kod = 4 + nuj;                     /* Inu */
          switch(kronecker(gneg(c6),p1))
            {
            case 1: c = nudelta; break;
            case -1: c = 2 - (nudelta % 2); break;
            default: err(tater1);
            }
          break;  
        case 6: f = 2; kod = - 4 - nuj;                   /* Inu* */
          if (nuj & 1)
            c = 3 + kronecker(divii(mulii(c6, delta),gpuigs(p1,9 + nuj)), p1);
          else
            c = 3 + kronecker(divii(delta, gpuigs(p1, 6 + nuj)), p1);
          break;
        default: err(tater1);
        }
      else switch(nudelta)
        {
        case 0: f = 0; kod = 1; c = 1; break;             /* I0, regulier */
        case 2: f = 2; kod = 2; c = 1; break;             /* II   */
        case 3: f = 2; kod = 3; c = 2; break;             /* III  */
        case 4: f = 2; kod = 4;                           /* IV   */
          c = 2 + kronecker(gdiv(mulis(c6, -6), mulii(p1, p1)), p1);
          break;
        case 6: f = 2; kod = -1;                          /* I0*  */
          p2 = mulii(p1, p1);
          unmodp = gmodulcp(gun,p1);
          var = gmul(unmodp,polx[0]);
          tri = gsub(gsqr(var),gmul(divii(gmulsg(3, c4), p2),unmodp));
          tri = gsub(gmul(tri, var),
            gmul(divii(gmulsg(2, c6), mulii(p2, p1)),unmodp));
          xun = gmodulcp(var,tri);
          c = lgef(ggcd((GEN)(gsub(gpui(xun,p1),xun))[2], tri)) - 2;
          break;
        case 8: f = 2; kod = -4;                          /* IV*  */
          c = 2 + kronecker(gdiv(mulis(c6, -6), gpuigs(p1, 4)), p1);
          break;
        case 9: f = 2; kod = -3; c = 2; break;            /* III* */
        case 10: f = 2; kod = -2; c = 1; break;           /* II*  */
        default: err(tater1);
        }
    }
  else for(;;)
    {
 /* ici, p = 2 ou p = 3 */
      if (!nudelta) {f = 0; kod = 1; c = 1; goto exit;}                             
 /* I0   */
      p = itos(p1);
      if (!divise((GEN)e[6], p1))
        {
          f =  1;
          kod = 4 + nudelta;
          if (itos(modis(gneg((GEN)e[11]), p == 2 ? 8 : 3)) == 1)
            c = nudelta;
          else
            c = 2 - (nudelta & 1);
          goto exit;        
        }        
 /* Inu  */
      if (p == 2)
        {
          r = modis((GEN)e[4], 2);
          s = modis(addii(r, (GEN)e[2]), 2);
          if (signe(r)) t = modis(addii(addii((GEN)e[4], (GEN)e[5]), s), 2);
          else t = modis((GEN)e[5], 2);
        }
      else /* p == 3 */
        {
          r = negi(modis((GEN)e[8], 3));
          s = modis((GEN)e[1], 3);
          t = modis(addii((GEN)e[3], mulii((GEN)e[1], r)), 3);
        }
      cumule(&v, &e, gun, r, s, t);       /* p | a1, a2, a3, a4 et a6 */
      p2 = stoi(p*p);
      if(!divise((GEN)e[5], p2)) {f = nudelta; kod = 2; c = 1; goto exit;}               
  /* II   */
      p3 = stoi(p*p*p);
      if(!divise((GEN)e[9], p3)) {f = nudelta - 1; kod = 3; c = 2; goto exit;}           
  /* III  */
      if (!divise((GEN)e[8], p3))
        {
          f = nudelta - 2;
          kod = 4;
          if (itos(modis((GEN)e[8], p == 2 ? 32 : 27)) == (p * p))
            c = 3;
          else
            c = 1;
          goto exit;        
        }                
  /* IV   */
      
      /* now for the last five cases... */
      
      if (!divise((GEN)e[5], p3))
        cumule(&v, &e, gun, gzero, gzero, p == 2 ? stoi(2) : modis((GEN)e[3], 9));
      /* p | a1, a2; p^2  | a3, a4; p^3 | a6 */
      a21 = aux((GEN)e[2], p, 1); a42 = aux((GEN)e[4], p, 2); 
      a63 = aux((GEN)e[5], p, 3);
      switch (numroots3(a21, a42, a63, p, &theroot))
        {
        case 3: f = nudelta - 4; kod = -1;
          if (p == 2)
            c = 1 + (a63 == 0) + ((a21 + a42 + a63) & 1);
          else
            c = 1 + (a63 == 0) + (((1 + a21 + a42 + a63) % 3) == 0)
                  + (((1 - a21 + a42 - a63) % 3) == 0);
          goto exit;                        
 /* I0*  */
        case 2: /* calcul de nu */
          if (theroot) cumule(&v, &e, gun, stoi(theroot * p), gzero, gzero);
          /* p | a1; p^2  | a2, a3; p^3 | a4; p^4 | a6 */
          nu = 1;
          pk = p2;
          p2k = stoi(p * p * p * p);
          for(;;)
            {
              if (numroots2(al = 1, be = aux2((GEN)e[3], p, pk),
                           ga = -aux2((GEN)e[5], p, p2k), p, &theroot) == 2)
		break;
              if (theroot) cumule(&v, &e, gun, gzero, gzero, mulsi(theroot,pk));
              pk1 = pk; pk = mulsi(p, pk); p2k = mulsi(p, p2k);
              nu++;
              if (numroots2(al = a21, be = aux2((GEN)e[4], p, pk),
                            ga = aux2((GEN)e[5], p, p2k), p, &theroot) == 2)
		break;
              if (theroot) cumule(&v, &e, gun, mulsi(theroot, pk1), gzero, gzero);
              p2k = mulsi(p, p2k);
              nu++;
            }
          if (p == 2)
            c = 4 - 2 * (ga & 1);
          else
            c = 3 + kross(be * be - al * ga, 3);
          f = nudelta - 4 - nu; kod = -4 - nu; goto exit;                    
  /* Inu* */
        case 1:
          if (theroot) cumule(&v, &e, gun, stoi(theroot * p), gzero, gzero); 
          /* p | a1; p^2  | a2, a3; p^3 | a4; p^4 | a6 */
          a32 = aux((GEN)e[3], p, 2); a64 = aux((GEN)e[5], p, 4);
          if(numroots2(1, a32, -a64, p, &theroot) == 2)
            {
              f = nudelta - 6;
              kod = -4;
              if (p == 2)
                c = 3 - 2 * a64;
              else
                c = 2 + kross(a32 * a32 + a64, 3);
              goto exit;
            }                          
  /* IV*  */
          if (theroot) cumule(&v, &e, gun, gzero, gzero, stoi(theroot*p*p));
          /* p | a1; p^2 | a2; p^3 | a3, a4; p^5 | a6 */
          p4 = mulii(p2, p2);
          if (!divise((GEN)e[4], p4)) {f = nudelta - 7; kod = -3; c = 2; goto exit;}     
  /* III* */
          p6 = mulii(p4, p2);
          if (!divise((GEN)e[5], p6)) {f = nudelta - 8; kod = -2; c = 1; goto exit;}     
  /* II*  */
          cumule(&v, &e, p1, gzero, gzero, gzero);  /* non minimal, on repart
pour un tour */
          nudelta -= 12;
        }
    }
 exit:
  tetpil = avma;
  result = cgetg(5, 17);
  result[1] = lstoi(f); result[2] = lstoi(kod);
  result[3] = lcopy(v); result[4] = lstoi(c);
  return gerepile(av, tetpil, result);
}

/*
  Calcul de toutes les fibres non elliptiques d'une courbe sur Z.
  Etant donne une courbe elliptique sous forme longue e, dont les coefficients
  sont entiers, renvoie une matrice dont les lignes sont de la forme
  [p, fp, kodp, cp]. Il y a une ligne par diviseur premier du discriminant.
  Ceci n'est pas implemente dans Gp. Est-ce utile ?
*/

GEN globaltatealgo(GEN e)
{
  long k, l,av;
  GEN p1, p2, p3, p4, prims, result;

  if ((typ(e) != 17) || (lg(e) < 14)) err(elliper1);

  prims = decomp((GEN)e[12]);
  l = lg(p1 = (GEN)prims[1]);
  p2 = (GEN)prims[2];
  if ((long)prims == avma) cgiv(prims);
  result = cgetg(5, 19);
  result[1] = (long)p1;
  result[2] = (long)p2;
  result[3] = (long)(p3 = cgetg(l, 18));
  for(k = 1; k < l; k++) p3[k] = lgeti(3);
  result[4] = (long)(p4 = cgetg(l, 18));
  for(k = 1; k < l; k++) p4[k] = lgeti(3);
  av = avma;
  for(k = 1; k < l; k++)
    {
      GEN q = localreduction(e, (GEN)p1[k]);
      affii((GEN)q[1],(GEN)p2[k]);
      affii((GEN)q[2],(GEN)p3[k]);
      affii((GEN)q[4],(GEN)p4[k]);
      avma = av;
    }
  return result;
}

/*
  Algorithme de reduction d'une courbe sur Q a sa forme standard.
  Etant donne une courbe elliptique sous forme longue e, dont les coefficients
  sont rationnels, renvoie son [N, [u, r, s, t], c], ou N est le conducteur
  arithmetique de e, [u, r, s, t] est le changement de variables qui reduit
  e a sa forme minimale globale dans laquelle a1 et a3 valent 0 ou 1, et a2
  vaut -1, 0 ou 1 et tel que u est un rationnel positif. Enfin c est
  le produit des nombres de Tamagawa locaux cp.
*/

GEN globalreduction(GEN e1)
{
  long i, k, l, m, tetpil, av = avma;
  GEN p1, c = gun, prims, result, N = gun, u = gun, r, s, t;
  GEN v = cgetg(5, 17), a = cgetg(7, 17), e = cgetg(20, 17);

  if ((typ(e1) != 17) || (lg(e1) < 14)) err(elliper1);

  for(i = 1; i < 5; i++) a[i] = e1[i]; a[5] = zero; a[6] = e1[5];
  prims = decomp(denom(a));
  l = lg(p1 = (GEN)prims[1]);
  for(k = 1; k < l; k++)
    {
      int n = 0;
      for(i = 1; i < 7; i++)
        if (!gcmp0((GEN)a[i])) 
          {
            m = i * n + ggval((GEN)a[i], (GEN)p1[k]);
            while(m < 0) {n++; m += i;}
          }
      u = gmul(u, gpuigs((GEN)p1[k], n));
   }
  v[1] = linv(u); v[2] = v[3] = v[4] = zero;
  for(i = 1; i < 14; i++) e[i] = e1[i];
  for(; i < 20; i++) e[i] = zero;
  if (!gcmp1(u)) e = coordch(e, v);
  prims = decomp((GEN)e[12]);
  l = lg(p1 = (GEN)prims[1]);
  for(k = (signe((GEN)e[12]) < 0) + 1; k < l; k++)
    {
      GEN q = localreduction(e, (GEN)p1[k]);
      GEN v1 = (GEN)q[3];
      N = mulii(N, gpui((GEN)p1[k],(GEN)q[1]));
      c = mulii(c, (GEN)q[4]);
      if (!gcmp1((GEN)v1[1])) cumule1(&v, &e, v1);
    }
  s = gdiventgs((GEN)e[1], -2);
  r = gdiventgs(gaddgs(gsub(gsub((GEN)e[2], gmul(s,(GEN)e[1])), gsqr(s)), 1), -3);
  t = gdiventgs(gadd((GEN)e[3], gmul(r,(GEN)e[1])), -2);
  cumule(&v, &e, gun, r, s, t);
  tetpil = avma;
  result = cgetg(4, 17); result[1] = lcopy(N); result[2] = lcopy(v);
  result[3] = lcopy(c);
  return gerepile(av, tetpil, result);
}

/* renvoie a_{k,l} avec les notations de Tate */

static int aux(GEN ak, int p, int l)
{
  long av = avma, pl = p, res;
  while(--l) pl *= p;
  res = itos(modis(divis(ak, pl), p));
  avma = av;
  return res;
}

static int aux2(GEN ak, int p, GEN pl)
{
  long av = avma, res;
  res = itos(modis(divii(ak, pl), p));
  avma = av;
  return res;
}

/* renvoie le nombre de racines distinctes du polynome XXX + aXX + bX + c
   modulo p s'il y a une racine multiple, elle est renvoyee dans *mult */

static int numroots3(int a, int b, int c, int p, int *mult)
{       
  if (p == 2)
    if ((c + a * b) & 1) return 3;
    else {*mult = b; return (a + b) & 1 ? 2 : 1;}
  else
    if (a % 3) {*mult = a * b; return (a * b * (1 - b) + c) % 3 ? 3 : 2;}
    else {*mult = -c; return b % 3 ? 3 : 1;}
}

/* idem pour aXX +bX + c */

static int numroots2(int a, int b, int c, int p, int *mult)
{
  if (p == 2) {*mult = c; return b & 1 ? 2 : 1;}
  else {*mult = a * b; return (b * b - a * c) % 3 ? 2 : 1;}
}

/* cumule les effets de plusieurs chgts de variable. On traite a part les cas
   particuliers frequents, tels que soit u = 1, soit r' = s' = t' = 0 */

static void cumulev(GEN *vtotal, GEN u, GEN r, GEN s, GEN t)
{
  long av = avma, tetpil;
  GEN temp, v = *vtotal, v3 = cgetg(5, 17);
  if (gcmp1((GEN)v[1]))
    {
      v3[1] = lcopy(u);
      v3[2] = ladd((GEN)v[2], r);
      v3[3] = ladd((GEN)v[3], s);
      av = avma;
      temp = gadd((GEN)v[4], gmul((GEN)v[3], r));
      tetpil = avma;
      v3[4] = lpile(av, tetpil, gadd(temp, t));
    }
  else if (gcmp0(r) && gcmp0(s) && gcmp0(t))
    {
      v3[1] = lmul((GEN)v[1], u);
      v3[2] = lcopy((GEN)v[2]);
      v3[3] = lcopy((GEN)v[3]);
      v3[4] = lcopy((GEN)v[4]);
    }
  else /* cas general */
    {
      v3[1] = lmul((GEN)v[1], u);
      temp = gmul((GEN)v[1],(GEN)v[1]);
      v3[2] = ladd(gmul(temp, r), (GEN)v[2]);
      v3[3] = ladd(gmul((GEN)v[1], s), (GEN)v[3]);
      v3[4] = ladd((GEN)v[4], gmul(temp, gadd(gmul((GEN)v[1], t), gmul((GEN)v[3], r))));    
            
      tetpil = avma;
      v3 = gerepile(av, tetpil, gcopy(v3));
    }
  *vtotal = v3;
}

static void cumule(GEN *vtotal, GEN *e, GEN u, GEN r, GEN s, GEN t)
{
  long av = avma, tetpil;
  GEN v2 = cgetg(5, 17);
  v2[1] = (long)u; v2[2] = (long)r; v2[3] = (long)s; v2[4] = (long)t;
  tetpil = avma;
  *e = gerepile(av, tetpil, coordch(*e, v2));
  cumulev(vtotal, u, r, s, t);
}

static void cumule1(GEN *vtotal, GEN *e, GEN v2)
{
  *e = coordch(*e, v2);
  cumulev(vtotal, (GEN)v2[1], (GEN)v2[2], (GEN)v2[3], (GEN)v2[4]);
}

/********************************************************************/
/********************************************************************/
/**                                                                **/
/**                   Parametrisation modulaire                    **/
/**                                                                **/
/********************************************************************/
/********************************************************************/


GEN taniyama(GEN e)
{
  GEN v,w,c,d,s1,s2,s3;
  long n,m,av=avma,tetpil;

  if ((typ(e) != 17) || (lg(e) < 14)) err(elliper1);
  v=cgetg(precdl+3,11);v[1]=evalsigne(1)+HIGHVALPBIT-2;v[2]=un;
  c=gtoser(anell(e,precdl+1),0);setvalp(c,1);
  d=ginv(c);c=gsqr(d);
  for(n=-3;n<=precdl-4;n++)
    {
      if(n!=2)
	{
	  s3=n?gzero:(GEN)e[7];
	  if(n>-3) s3=gadd(s3,gmul((GEN)e[6],(GEN)v[n+4]));
	  s2=gzero;
	  for(m=-2;m<=n+1;m++)
	    s2=gadd(s2,gmulsg(m*(n+m),gmul((GEN)v[m+4],(GEN)c[n-m+4])));
	  s2=gmul2n(s2,-1);
	  s1=gzero;
	  for(m=-1;m+m<=n;m++)
	    {
	      if(m+m==n) s1=gadd(s1,gsqr((GEN)v[m+4]));
	      else s1=gadd(s1,gmul2n(gmul((GEN)v[m+4],(GEN)v[n-m+4]),1));
	    }
	  v[n+6]=ldivgs(gsub(gadd(gmulsg(6,s1),s3),s2),(n+2)*(n+1)-12);
	}
      else
	{
	  setlg(v,9);v[8]=(long)polx[MAXVARN];
	  w=deriv(v,0);setvalp(w,-2);
	  s1=gadd((GEN)e[8],gmul(v,gadd(gmul2n((GEN)e[7],1),gmul(v,gadd((GEN)e[6],gmul2n(v,2))))));
	  setlg(v,precdl+3);
	  s2=gsub(s1,gmul(c,gsqr(w)));
	  s2=gsubst((GEN)s2[2],MAXVARN,polx[0]);
/*	  if(lgef(s2)!=4) err(talker,"bug in taniyama"); */
	  v[n+6]=lneg(gdiv((GEN)s2[2],(GEN)s2[3]));
	}
    }
  w=gsub(gmul(polx[0],gmul(d,deriv(v,0))),gcmp0((GEN)e[1])?(GEN)e[3]:gadd((GEN)e[3],gmul((GEN)e[1],v)));
  tetpil=avma;s1=cgetg(3,17);s1[1]=lcopy(v);s1[2]=lmul2n(w,-1);
  return gerepile(av,tetpil,s1);
}


/********************************************************************/
/********************************************************************/
/**                                                                **/
/**                       Points de torsion                        **/
/**                                                                **/
/********************************************************************/
/********************************************************************/

GEN ratroot(GEN p)
/* For internal use only. p is a polynomial of degree exactly 3 with integral
coefficients and leading term 4. Outputs the vector of rational roots of p */
{
  GEN v,a,y,ld;
  long av=avma,tetpil,i,t;

  i=2;while(!signe((GEN)p[i])) i++;
  if(i==5) {v=cgetg(2,17);v[1]=zero;return v;}
  if(i==4) {v=cgetg(3,17);v[1]=zero;v[2]=ldivgs((GEN)p[4],-4);return v;}
  ld=divisors(gmul2n((GEN)p[i],2));v=cgetg(4,17);t=0;if(i==3) v[++t]=zero;
  for(i=1;i<lg(ld);i++)
    {
      if(!gsigne(gsubst(p,0,a=gmul2n((GEN)ld[i],-2)))) v[++t]=(long)a;
      if(!gsigne(gsubst(p,0,a=gneg(a)))) v[++t]=(long)a;
    }
  tetpil=avma;y=cgetg(t+1,17);
  for(i=1;i<=t;i++) y[i]=lcopy((GEN)v[i]);
  return gerepile(av,tetpil,y);
}

GEN orderell(GEN e, GEN p)
{
  GEN p1;
  long av=avma,k;

  if((typ(e)!=17)||(lg(e)<6)||(typ(p)!=17)) err(elliper1);
  k=typ((GEN)e[13]);
  if((k!=1)&&(k!=4)&&(k!=5)) 
    err(impl,"orderell for nonrational elliptic curves");
  p1=p;k=1;
  while((k<=15)&&(lg(p1)==3)) {p1=addell(e,p1,p);k++;}
  avma=av;return (lg(p1)==3)?gzero:stoi(k);
}

GEN torsell(GEN e)
{
  GEN n,d,ld,pol,p1,pk,pkprec,lr,v,w;
  long i,j,l,nlr,t,t2,k,k2,fl,av=avma,tetpil,av1;

  if ((typ(e)!=17)||(lg(e)<14)) err(elliper1);
  v=cgetg(17,17);t=1;p1=cgetg(2,17);p1[1]=zero;v[1]=(long)p1;
  pol=gadd((GEN)e[8],gmul(polx[0],gadd(gmul2n((GEN)e[7],1),gmul(polx[0],gadd((GEN)e[6],gmul2n(polx[0],2))))));
  lr=ratroot(pol);nlr=lg(lr)-1;
  for(i=1;i<=nlr;i++)
    {
      p1=cgetg(3,17);p1[1]=lr[i];
      p1[2]=ldivgs(gadd(gmul((GEN)e[1],(GEN)lr[i]),(GEN)e[3]),-2);
      v[++t]=(long)p1;
    }
  t2=t;
  ld=factor(gabs((GEN)e[12]));n=stoi(4);
  for(i=1;i<lg((GEN)ld[1]);i++)
    n=gmul(n,gpui(gcoeff(ld,i,1),shifti(gcoeff(ld,i,2),-1)));
  ld=divisors(n);
  for(j=1;j<lg(ld);j++)
    {
      d=(GEN)ld[j];lr=ratroot(gsub(pol,gmul(d,d)));
      for(i=1;i<lg(lr);i++)
	{
	  p1=cgetg(3,17);p1[1]=lr[i];
	  p1[2]=lmul2n(gsub(d,gadd(gmul((GEN)e[1],(GEN)lr[i]),(GEN)e[3])),-1);
	  pk=p1;fl=0;
	  for(k=2;(k<=6)&&(!fl);k++)
	    {
	      pk=addell(e,pk,p1);
	      if(lg(pk)==2) fl=1;
	      for(l=2;(!fl)&&(l<=t2);l++) 
		fl=gegal((GEN)pk[1],(GEN)((GEN)v[l])[1]);
	      if((!fl)&&(k>=3)&&(k<=5)&&(lg(pkprec)>2))
		fl=gegal((GEN)pk[1],(GEN)pkprec[1]);
	      if(!fl) pkprec=pk;
	    }
	  if(fl)
	    {
	      v[++t]=(long)p1;p1=gcopy(p1);p1[2]=lsub((GEN)p1[2],d);
	      v[++t]=(long)p1;
	    }
	}
    }
  if(t==1) 
    {
      avma=av;w=cgetg(4,17);w[1]=un;w[2]=lgetg(1,17);w[3]=lgetg(1,17);
      return w;
    }
  else
    {
      tetpil=avma;w=cgetg(4,17);w[1]=lstoi(t);
      if(nlr<3)
	{
	  p1=cgetg(2,17);p1[1]=lstoi(t);w[2]=(long)p1;
	  av1=avma;
	  k=2;while((k<=t)&&(itos(orderell(e,(GEN)v[k]))!=t)) k++;
	  avma=av1;
	  if(k>t) err(talker,"bug1 in torsell, please report");
	  p1=cgetg(2,17);p1[1]=lcopy((GEN)v[k]);w[3]=(long)p1;
	  return gerepile(av,tetpil,w);
	}
      else
	{
	  if(t&3) err(talker,"bug2 in torsell, please report");
	  p1=cgetg(3,17);t2=t>>1;p1[1]=lstoi(t2);p1[2]=(long)gdeux;
	  w[2]=(long)p1;av1=avma;
	  k=2;while((k<=t)&&(itos(orderell(e,(GEN)v[k]))!=t2)) k++;
	  if(k>t) err(talker,"bug3 in torsell, please report");
	  k2=2;p1=powell(e,(GEN)v[k],stoi(t>>2));
	  if((lg(p1)==3)&&gegal((GEN)v[2],p1)) k2++;
	  avma=av1;
	  p1=cgetg(3,17);p1[1]=lcopy((GEN)v[k]);p1[2]=lcopy((GEN)v[k2]);
	  w[3]=(long)p1;return gerepile(av,tetpil,w);
	}
    }
}
