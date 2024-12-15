/********************************************************************/
/********************************************************************/
/**                                                                **/
/**                  BIBLIOTHEQUE  MATHEMATIQUE                    **/
/**                     (premiere partie)                          **/
/**                                                                **/
/**                     Copyright Babe Cool                        **/
/**                                                                **/
/********************************************************************/
/********************************************************************/

# include "genpari.h"


/********************************************************************/
/********************************************************************/
/**                                                                **/
/**                 DEVELOPPEMENTS  LIMITES                        **/
/**                                                                **/
/********************************************************************/
/********************************************************************/

GEN     tayl(GEN x, long v, long precdl)
{
  long tetpil,i,vx = gvar9(x), av=avma;
  GEN p1, y;
  
  if(v <= vx)
  {
    p1 = cgetg(3,11);
    p1[2] = zero; p1[1] = HIGHVALPBIT + precdl; setvarn(p1,v);
    tetpil = avma; y = gerepile(av, tetpil, gadd(p1,x));
  }
  else
  {
    p1=cgetg(v+2,17);
    for(i=0;i<vx;i++) p1[i+1]=lpolx[i];p1[vx+1]=lpolx[v];
    for(i=vx+1;i<v;i++) p1[i+1]=lpolx[i];p1[v+1]=lpolx[vx];
    y = tayl(changevar(x, p1), vx, precdl); tetpil=avma;
    y = gerepile(av, tetpil, changevar(y, p1));
  }
  return y;
}


GEN     ggrando(GEN x, long n)
{
  long m, v, tx=typ(x);
  GEN y;
  if (gcmp0(x)) err(grandoer1);
  if (tx>9)
  {
    y=cgetg(3,11);m=gval(x,v = gvar(x));
    y[1]=HIGHVALPBIT+n*m;y[2]=zero;setvarn(y,v);
  }
  else
  {
    if(tx!=1) err(grandoer1);
    y=cgetg(5,7);y[2]=lclone(x);
    y[3]=un;y[4]=zero;
    setvalp(y,n);setprecp(y,0);
  }
  return y;
}

/********************************************************************/
/********************************************************************/
/**                                                                **/
/**                POLYNOMES  DE  TCHEBICHEFF                      **/
/**                                                                **/
/********************************************************************/
/********************************************************************/

/*      T0=1; T1=X ;  T(n)=2*X*T(n-1)-T(n-2)    */

GEN  tchebi(long n)
{
  long  av1,av2,av3,av4,av5,decal,m;
  GEN  p0,p1,p2,px,q;
  
  if (n==0) return polun[0];
  else
    {
      av1=avma;
      px=cgetg(4,10);
      px[1]=evalsigne(1)+evallgef(4);px[2]=zero;px[3]=un;
      if (n==1) return px;
      else
        {
          av2=avma;
          p0=gcopy(polun[0]);
          av3=avma;
          p1=gcopy(px);
          for (m=1;m<n-1;m++)
            {
              q=gmul(px,gmulsg(2,p1));
              av4=avma;
              p2=gsub(q,p0);av5=avma;
              decal=lpile(av2,av3,0);
              p0=adecaler(p1,av3,av5)?p1+(decal>>TWOPOTBYTES_IN_LONG):p1;
	      p1=adecaler(p2,av3,av5)?p2+(decal>>TWOPOTBYTES_IN_LONG):p2;
              av3=av4+decal;
            }
          q=gmul(px,gmulsg(2,p1));av2=avma;
          return gerepile(av1,av2,gsub(q,p0));
        }
    }
}

/********************************************************************/
/********************************************************************/
/**                                                                **/
/**                  POLYNOMES  DE  LEGENDRE                       **/
/**                                                                **/
/********************************************************************/
/********************************************************************/

/* L0=1; L1=X ;(n+1)*L(n)=(2*n+1)*X*L(n)-n*L(n-1) */

GEN  legendre(long n)
{
  long  av1,av2,av3,av4,av5,av6,m,decal;
  GEN  p0,p1,p2,px,q2;
  
  if (n==0) return polun[0];
  else
  {
    av1=avma;
    px=cgetg(4,10);
    px[1]=evalsigne(1)+evallgef(4);px[2]=zero;px[3]=un;
    if (n==1) return px;
    else
    {
      av2=avma;
      p0=polun[0];
      av3=avma;
      p1=gmulsg(2,px);
      
      for (m=1;m<n;m++)
      {
        av4=avma;
        p2=gsub(gmul(px,gmulsg(4*m+2,p1)),gmulsg(4*m,p0));
        av5=avma;
        p2=gerepile(av4,av5,gdivgs(p2,m+1));
        av6=avma;decal=lpile(av2,av3,0);
        p0=adecaler(p1,av3,av6)?p1+(decal>>TWOPOTBYTES_IN_LONG):p1;
	p1=adecaler(p2,av3,av6)?p2+(decal>>TWOPOTBYTES_IN_LONG):p2;
        av3=av4+decal;
      }
      q2=mpshift(gun,n);av2=avma;
      return gerepile(av1,av2,gdiv(p1,q2));
    }
  }
}

GEN cyclo(long n)
{
  long av=avma,tetpil,d,q,m;
  GEN p1,yn,yd;

  if(n<=0) err(arither2);
  d=1;yn=gun;yd=gun;
  while(d*d<=n)
    {
      if(!(n%d))
	{
	  q=n/d;m=mu(stoi(q));
	  if(m)
	    {
	      p1=gsub(gpuigs(polx[0],d),gun);
	      if(m>0) yn=gmul(yn,p1);else yd=gmul(yd,p1);
	    }
	  if(q!=d)
	    {
	      m=mu(stoi(d));
	      if(m)
		{
		  p1=gsub(gpuigs(polx[0],q),gun);
		  if(m>0) yn=gmul(yn,p1);else yd=gmul(yd,p1);
		}
	    }
	}
      d++;
    }
  tetpil=avma;return gerepile(av,tetpil,gdiv(yn,yd));
}
	      

/********************************************************************/
/********************************************************************/
/**                                                                **/
/**              MATRICES DE HILBERT,PASCAL ...                    **/
/**                                                                **/
/********************************************************************/
/********************************************************************/

GEN     hilb(long n)
     /* matrice de hilbert d'ordre n */
               
     
     
{
  long i,j,sho[3];GEN p,a;
  p=cgetg(n+1,19);
  sho[0] = evaltyp(1)+evalpere(1)+evallg(3);
  sho[1] = evalsigne(1)+evallgef(3);
  for (j=1;j<=n;j++)
  {
    p[j]=lgetg(n+1,18);
    for (i=1;i<=n;i++)
    {
      a=cgetg(3,4);coeff(p,i,j)=(long)a;
      a[1]=un;sho[2]=i+j-1;a[2]=lcopy(sho);
    }
  }
  return p;
}

GEN     pasc(long n)
     /* matrice triangle de PASCAL d'ordre n */
               
     
{
  long i,j;GEN p;
  
  p=cgetg(n+2,19);
  for (j=1;j<=n+1;j++)  p[j]=lgetg(n+2,18);
  for (i=1;i<=n+1;i++)
  {
    coeff(p,i,1)=un;coeff(p,i,i)=un;
    for (j=2;j<i;j++)
      coeff(p,i,j)=laddii(gcoeff(p,i-1,j),gcoeff(p,i-1,j-1));
    for (j=i+1;j<=n+1;j++)
      coeff(p,i,j)=zero;
  }
  return p;
}

/********************************************************************/
/********************************************************************/
/**                                                                **/
/**              TRANSFORMEE DE LAPLACE D UNE SERIE                **/
/**                                                                **/
/********************************************************************/
/********************************************************************/

GEN    laplace(GEN x)
{
  
  long i,l,dec,ec,av1,av2,av3,av4;
  GEN  y,p1,p2;
  
  if(typ(x)!=11) err(laper1);
  if(gcmp0(x)) y=gcopy(x);
  else
  {
    ec=valp(x);if (ec<0) err(laper2);
    l=lg(x);y=cgetg(l,11);av1=avma;
    p1=mpfact(ec);y[1]=x[1];
    for(i=2;i<l;i++)
    {
      av2=avma;p2=gmul(p1,(GEN)x[i]);
      av3=avma;ec++;p1=gmulsg(ec,p1);
      av4=avma;dec=lpile(av1,av2,0)>>TWOPOTBYTES_IN_LONG;
      y[i]=adecaler(p2,av2,av4)?(long)(p2+dec):(long)p2;
      if(adecaler(p1,av2,av4)) p1+=dec;
      av1=av3+(dec<<TWOPOTBYTES_IN_LONG);
    }
    avma=av1;
  }
  return y;
}

/********************************************************************/
/********************************************************************/
/**                                                                **/
/**            PRODUIT DE CONVOLUTION DE DEUX SERIES               **/
/**                                                                **/
/********************************************************************/
/********************************************************************/

GEN    convol(GEN x, GEN y)
{
  long lx,ly,l,ex,ey,l1,i,j,v,f;
  GEN  z;
  
  if((typ(x)!=11) || (typ(y)!=11)) err(convol1);
  if(gcmp0(x) || gcmp0(y)) err(convol2);
  lx=lg(x);ly=lg(y);ex=valp(x);ey=valp(y);
  v=ex;if(ey>v) v=ey;
  l=ex+lx;l1=ey+ly;if(l1<l) l=l1;
  l=l-v;if(l<3) err(convol3);f=1;
  for(i=v+2;(i<(l+v))&&f;i++) f=(gcmp0((GEN)x[i-ex]) || gcmp0((GEN)y[i-ey]));
  if(i==(l+v)) 
  {
    z=cgetg(3,11); z[1]=HIGHVALPBIT-2+l+v; z[2]=zero;
  }
  else
  {
    z=cgetg(l-i+3+v,11); z[1]=evalsigne(1)+HIGHVALPBIT-3+i;
    for(j=i-1;j<l+v;j++) z[j-i+3]=lmul((GEN)x[j-ex],(GEN)y[j-ey]);
  }
  return z;
}

/********************************************************************/
/********************************************************************/
/**                                                                **/
/**         Conversion serie----->polynome ou fr. rat.             **/
/**                                                                **/
/**                            et                                  **/
/**                                                                **/
/**               p-adique-->entier ou rationnel                   **/
/**                                                                **/
/********************************************************************/
/********************************************************************/

GEN     gconvsp(GEN x)
{
  long  e ,av ,tetpil ,i ,v;
  GEN   p1 ,y;
  
  if(typ(x)!=11) err(csper1);
  v=varn(x);e=valp(x);av=avma;y=gcopy(x);settyp(y ,10);
  if (gcmp0(x)) {y[1]=2;setvarn(y,v);}
  else
    {
      for(i=lg(x)-1;(i>1)&&gcmp0((GEN)y[i]);--i);
      y[1]=evalsigne(1)+evallgef(1+i)+evalvarn(v);
      p1=gpuigs(polx[v],e);tetpil=avma;
      y=gerepile(av ,tetpil ,gmul(p1 ,y));
    }
  return y;
}

GEN gconvpe(GEN x)
{
  long av,tetpil;
  GEN p1;

  if(typ(x)!=7) err(csper1);
  if(!signe((GEN)x[4])) return gzero;
  av=avma;p1=gpuigs((GEN)x[2],valp(x));
  tetpil=avma;return gerepile(av,tetpil,gmul(p1,(GEN)x[4]));
}

/********************************************************************/
/********************************************************************/
/**                                                                **/
/**             CHANGEMENT DE PRECISION D'UN GEN                   **/
/**                                                                **/
/**                       OU DU DEFAUT                             **/
/**                                                                **/
/********************************************************************/
/********************************************************************/

GEN     gprec(GEN x, long l)
{
  long  tx=typ(x),lx=lg(x),i,pr;
  GEN   y;
  
  if(l<=0) err(precer1);
  switch(tx)
    {
    case 2 : pr=(long)(l*K1+3);y=cgetr(pr);affrr(x,y);break;
    case 7 : y=cgetg(lx,tx);y[1]=x[1];
      setprecp(y,l);y[2]=x[2];
      y[3]=lpuigs((GEN)x[2],l);
      y[4]=lmodii((GEN)x[4],(GEN)y[3]);break;
    case 11: 
      if(gcmp0(x))
        {
          y=cgetg(3,11);y[1]=HIGHVALPBIT+l;setvarn(y,varn(x));
        }
      else
        {
          y=cgetg(l+2,11); y[1]=x[1];
          if(l+1<lx)
            {
              for(i=2;i<l+2;i++)
                y[i]=lcopy((GEN)x[i]);
            }
          else
            {
              for(i=2;i<lx;i++)
                y[i]=lcopy((GEN)x[i]);
              for(i=lx;i<l+2;i++)
                y[i]=zero;
            }
        }
      break;
    case 6 :
    case 13:
    case 14:
    case 17:
    case 18:
    case 19: y=cgetg(lx,tx);
      for(i=1;i<lx;i++)
        y[i]=lprec((GEN)x[i],l);
      break;
    default: y=gcopy(x);
    }
  return y;
}

long setprecr(long n)
{
  long m=glbfmt[2];
  if(n>0) {glbfmt[2] = n;prec = (long)(n * K1 + 3);}
  return m;
}

long setserieslength(long n)
{
  long m=precdl;
  if(n>0) precdl=n;
  return m;
}

/********************************************************************/
/********************************************************************/
/**                                                                **/
/**                     POLYNOME RECIPROQUE       .                **/
/**                                                                **/
/********************************************************************/
/********************************************************************/

GEN     polrecip(GEN x)
{
  GEN   y;
  long  lx=lgef(x),i;
  
  if(typ(x)!=10) err(polrecer1);
  y=cgetg(lx,10);y[1]=x[1];
  for(i=2;i<lx;i++)
    y[i]=lcopy((GEN)x[lx+1-i]);
  normalizepol(&y);return y;
}

/********************************************************************/
/********************************************************************/
/**                                                                **/
/**                     COEFFICIENTS BINOMIAUX    .                **/
/**                                                                **/
/********************************************************************/
/********************************************************************/

GEN     binome(GEN x, long k)
{
  GEN   y,p1;
  long  av,tetpil,i;
  
  if (k<0) return gzero;
  if (!k) return gun;
  av=avma;y=gcopy(x);if(k==1) return y;
  for(i=k;i>=2;i--)
    {
      p1=gdivgs(gaddgs(x,i-1-k),i);tetpil=avma;
      y=gmul(y,p1);
    }
  return gerepile(av,tetpil,y);
}

/********************************************************************/
/********************************************************************/
/**                                                                **/
/**                         ALGORITHME LLL       .                 **/
/**                                                                **/
/********************************************************************/
/********************************************************************/

#define cosp(i,j)   (((GEN)(p3[i]))[j])

GEN    gscal(GEN x, GEN y)
     
              
     
     /* produit scalaire de x par y sans verification de compatibilite */
     
{
  GEN  z,p;
  long i,av,tetpil,lx=lg(x);
  
  z=gzero;if(lx==1) return z;
  av=avma;
  for (i=1;i<lx;i++)
    {
      p=gmul((GEN)x[i],(GEN)y[i]);
      tetpil=avma;
      z=gadd(z,p);
    }
  return gerepile(av,tetpil,z);
}

GEN    lll1(GEN x, long prec)
             
                

    /*     x est la matrice d'une base bi ;
	   retourne la matrice u ( entiere unimodulaire )
           d'une base LLL-reduite ci en fonction des bi.
	  ( la base reduite est  c=b*u )                         */
{
  long lx=lg(x), tx=typ(x),i,j,av,av1;
  GEN g;

  if(tx!=19) err(lller1);
  av=avma;
  g=cgetg(lx,19);
  for(j=1;j<lx;j++) g[j]=lgetg(lx,18);
  for(i=1;i<lx;i++)
    for(j=1;j<=i;j++) coeff(g,i,j)=coeff(g,j,i)=(long)gscal((GEN)x[i],(GEN)x[j]);
  av1=avma;
  return gerepile(av,av1,lllgram1(g,prec));
}


GEN    lllgram(GEN x, long prec)
             
                     /* x est ici la matrice de GRAM des bi.  */
     
{
  
  GEN  mu,u,B,BB,BK,p,q,r,cst,unreel,sv,mu1,mu2;
  long av0,av,tetpil,lim,l,i,j,k,lx=lg(x),tx=typ(x),n,temp,e;

  if(tx!=19) err(lllger1);
  if(lg((GEN)x[1])!=lx) err(lllger2);n=lx-1;
  if(n<=1) return idmat(n);
  av0=avma;
  cst=gdivgs(stoi(99),100); /* LLL proposent 0.75 */
  lim=(avma+bot)>>1;
  if (prec)
    {
      affsr(1,unreel=cgetr(prec+1));
      x=gmul(x,unreel);
      cst=gmul(cst,unreel);
    }
  av=avma;
  mu=sqred3(x);B=cgetg(lx,18);
/*  output(mu); */
  for(i=1,l=0;i<=n;i++)
    {
      if(gsigne((GEN)(B[i]=coeff(mu,i,i)))>0) l++;
      coeff(mu,i,i)=un;
    }
  if(l<n) err(lllger3);

  u=idmat(n);
  k=2;
  do
    {
/*      printf(" %d",k);fflush(stdout); */
      if(!gcmp0(r=grndtoi(gcoeff(mu,k,k-1),&e)))
        {
/*	  printf("k, l, e, r: %d, %d, %d, ",k,k-1,e);output(r); */
          u[k]=lsub((GEN)u[k],gmul(r,(GEN)u[k-1]));
          for(j=1;j<k-1;j++)
            coeff(mu,k,j)=lsub(gcoeff(mu,k,j),gmul(r,gcoeff(mu,k-1,j)));
          mu1=(GEN)(coeff(mu,k,k-1)=lsub(gcoeff(mu,k,k-1),r));
        }
      else mu1=gcoeff(mu,k,k-1);
      q=gmul((GEN)B[k-1],gsub(cst,mu2=gsqr(mu1)));
      if(gcmp(q,(GEN)B[k])>0)
        {
          BB=gadd((GEN)B[k],gmul((GEN)B[k-1],mu2));
          coeff(mu,k,k-1)=ldiv(gmul(mu1,(GEN)B[k-1]),BB);
          B[k]=lmul((GEN)B[k-1],BK=gdiv((GEN)B[k],BB));
          B[k-1]=(long)BB;
          temp=u[k];u[k]=u[k-1];u[k-1]=temp;
          for(j=1;j<=k-2;j++)
            {
              temp=(long)coeff(mu,k,j);coeff(mu,k,j)=coeff(mu,k-1,j);
              coeff(mu,k-1,j)=temp;
            }
          for(i=k+1;i<=n;i++)
            {
              p=(GEN)coeff(mu,i,k);
	      coeff(mu,i,k)=lsub(gcoeff(mu,i,k-1),gmul(mu1,p));
              coeff(mu,i,k-1)=ladd(gmul(BK,p),gmul(gcoeff(mu,k,k-1),gcoeff(mu,i,k-1)));
            }
          if(k>2) k--;
        }
      else
        {
          for(l=k-2;l;l--)
            {
              if(!gcmp0(r=grndtoi(gcoeff(mu,k,l),&e)))
                {
/*		  printf("k, l, e, r: %d, %d, %d, ",k,l,e);output(r); */
                  u[k]=lsub((GEN)u[k],gmul(r,(GEN)u[l]));
                  for(j=1;j<l;j++)
                    coeff(mu,k,j)=lsub(gcoeff(mu,k,j),gmul(r,gcoeff(mu,l,j)));
                  coeff(mu,k,l)=lsub(gcoeff(mu,k,l),r);
                }
            }
          k++;
        }
      if(avma<lim)
	{
	  tetpil=avma;
	  sv=cgetg(4,17);
	  sv[1]=lcopy(B);sv[2]=lcopy(u);sv[3]=lcopy(mu);
	  sv=gerepile(av,tetpil,sv);
	  B=(GEN)sv[1];u=(GEN)sv[2];mu=(GEN)sv[3];
	}
    }
  while(k<=n);
/*  printf("\n"); */
  tetpil=avma;return gerepile(av,tetpil,gcopy(u));
}

GEN    lll(GEN x, long prec)
             
                

    /*     x est la matrice d'une base bi ;
	   retourne la matrice u ( entiere unimodulaire )
           d'une base LLL-reduite ci en fonction des bi.
	  ( la base reduite est  c=b*u )                         */
{
  long lx=lg(x), tx=typ(x),i,j,av,av1;
  GEN g;

  if(tx!=19) err(lller1);
  av=avma;if(lx==1) return cgetg(1,19);
  g=cgetg(lx,19);
  for(j=1;j<lx;j++) g[j]=lgetg(lx,18);
  for(i=1;i<lx;i++)
    for(j=1;j<=i;j++) coeff(g,i,j)=coeff(g,j,i)=(long)gscal((GEN)x[i],(GEN)x[j]);
  av1=avma;
  return gerepile(av,av1,lllgram(g,prec));
}

GEN    lllint(GEN x)
{
  long lx=lg(x), tx=typ(x),i,j,av,av1;
  GEN g;

  if(tx!=19) err(lller1);
  av=avma;if(lx==1) return cgetg(1,19);
  g=cgetg(lx,19);
  for(j=1;j<lx;j++) g[j]=lgetg(lx,18);
  for(i=1;i<lx;i++)
    for(j=1;j<=i;j++) coeff(g,i,j)=coeff(g,j,i)=(long)gscal((GEN)x[i],(GEN)x[j]);
  av1=avma;return gerepile(av,av1,lllgramall(g,2));
}

GEN lllkerim(GEN x)
{
  long lx=lg(x), tx=typ(x),i,j,av,av1;
  GEN g;

  if(tx!=19) err(lller1);
  if(lx==1)
    {
      g=cgetg(3,17);g[1]=lgetg(1,19);g[2]=lgetg(1,19);return g;
    }
  if(lg((GEN)x[1])==1)
    {
      g=cgetg(3,17);g[1]=(long)idmat(lx-1);g[2]=lgetg(1,19);return g;
    }
  av=avma;
  g=cgetg(lx,19);
  for(j=1;j<lx;j++) g[j]=lgetg(lx,18);
  for(i=1;i<lx;i++)
    for(j=1;j<=i;j++) coeff(g,i,j)=coeff(g,j,i)=(long)gscal((GEN)x[i],(GEN)x[j]);
  av1=avma;return gerepile(av,av1,lllgramall(g,0));
}

GEN    lllrat(GEN x)
{
  return lll(x,0);
}

GEN    lllgram1(GEN x, long prec)
             
                     /* x est ici la matrice de GRAM des bi.  */
     
{
  
  GEN  mu,u,B,BB,BK,p,q,r,cst,unreel,sv,mu1,mu2;
  long av0,av,tetpil,lim,l,i,j,k,lx=lg(x),tx=typ(x),n,temp,e;

  if(tx!=19) err(lllger1);
  if(lg((GEN)x[1])!=lx) err(lllger2);n=lx-1;
  if(n<=1) return idmat(n);
  av0=avma;
  cst=gdivgs(stoi(99),100); /* LLL proposent 0.75 */
  lim=(avma+bot)>>1;
  if (prec)
    {
      affsr(1,unreel=cgetr(prec+1));
      x=gmul(x,unreel);
      cst=gmul(cst,unreel);
    }
  av=avma;
  mu=gtrans(sqred(x)); B=cgetg(lx,18);
  for(i=1,l=0;i<=n;i++)
    {
      if(gsigne((GEN)(B[i]=coeff(mu,i,i)))>0) l++;
      coeff(mu,i,i)=un;
    }
  if(l<n) err(lllger3);

  u=idmat(n);
  k=2;
  do
    {
      if(!gcmp0(r=grndtoi(gcoeff(mu,k,k-1),&e)))
        {
          u[k]=lsub((GEN)u[k],gmul(r,(GEN)u[k-1]));
          for(j=1;j<k-1;j++)
            coeff(mu,k,j)=lsub(gcoeff(mu,k,j),gmul(r,gcoeff(mu,k-1,j)));
          mu1=(GEN)(coeff(mu,k,k-1)=lsub(gcoeff(mu,k,k-1),r));
        }
      else mu1=gcoeff(mu,k,k-1);
      q=gmul((GEN)B[k-1],gsub(cst,mu2=gsqr(mu1)));
      if(gcmp(q,(GEN)B[k])>0)
        {
          BB=gadd((GEN)B[k],gmul((GEN)B[k-1],mu2));
          coeff(mu,k,k-1)=ldiv(gmul(mu1,(GEN)B[k-1]),BB);
          B[k]=lmul((GEN)B[k-1],BK=gdiv((GEN)B[k],BB));
          B[k-1]=(long)BB;
          temp=u[k];u[k]=u[k-1];u[k-1]=temp;
          for(j=1;j<=k-2;j++)
            {
              temp=(long)coeff(mu,k,j);coeff(mu,k,j)=coeff(mu,k-1,j);
              coeff(mu,k-1,j)=temp;
            }
          for(i=k+1;i<=n;i++)
            {
              p=(GEN)coeff(mu,i,k);
	      coeff(mu,i,k)=lsub(gcoeff(mu,i,k-1),gmul(mu1,p));
              coeff(mu,i,k-1)=ladd(gmul(BK,p),gmul(gcoeff(mu,k,k-1),gcoeff(mu,i,k-1)));
            }
          if(k>2) k--;
        }
      else
        {
          for(l=k-2;l;l--)
            {
              if(!gcmp0(r=grndtoi(gcoeff(mu,k,l),&e)))
                {
                  u[k]=lsub((GEN)u[k],gmul(r,(GEN)u[l]));
                  for(j=1;j<l;j++)
                    coeff(mu,k,j)=lsub(gcoeff(mu,k,j),gmul(r,gcoeff(mu,l,j)));
                  coeff(mu,k,l)=lsub(gcoeff(mu,k,l),r);
                }
            }
          k++;
        }
      if(avma<lim)
	{
	  tetpil=avma;
	  sv=cgetg(4,17);
	  sv[1]=lcopy(B);sv[2]=lcopy(u);sv[3]=lcopy(mu);
	  sv=gerepile(av,tetpil,sv);
	  B=(GEN)sv[1];u=(GEN)sv[2];mu=(GEN)sv[3];
	}
    }
  while(k<=n);
  tetpil=avma;return gerepile(av,tetpil,gcopy(u));
}

GEN lllgramintindep(GEN x)
{
  long av=avma,tetpil,lx=lg(x),tx=typ(x),i,j,k,l,p1,n,lim;
  GEN u,B,lam,q,r,h,la,bb,sv;

  if(tx!=19) err(lllger1);
  n=lx-1;if(n<=1) return idmat(n);
  if(lg((GEN)x[1])!=lx) err(lllger2);
  av=avma;lim=(avma+bot)>>1;
  B=cgetg(lx+1,18);
  B[1]=un;lam=cgetg(lx,19);
  for(j=1;j<=n;j++) lam[j]=lgetg(lx,18);
  for(i=1;i<=n;i++) 
    for(j=1;j<=i;j++)
      {
	u=(GEN)coeff(x,i,j);
	if(typ(u)!=1) err(lllger4);
	for(k=1;k<j;k++)
	  u=divii(subii(mulii((GEN)B[k+1],u),mulii(gcoeff(lam,i,k),gcoeff(lam,j,k))),(GEN)B[k]);
	if(j<i) coeff(lam,i,j)=(long)u;
	else 
	  {
	    if(signe(u)) B[i+1]=(long)u;
	    else err(lllger5);
	  }
	coeff(lam,j,i)=zero;
      }
  k=2;h=idmat(n);
  for(;;)
    {
      if(cmpii(absi(u=shifti(gcoeff(lam,k,k-1),1)),(GEN)B[k])>0)
	{
	  q=dvmdii(addii(u,(GEN)B[k]),shifti((GEN)B[k],1),&r);
	  if(signe(r)<0) q=addsi(-1,q);
	  h[k]=lsub((GEN)h[k],gmul(q,(GEN)h[k-1]));
	  coeff(lam,k,k-1)=lsubii(gcoeff(lam,k,k-1),mulii(q,(GEN)B[k]));
	  for(i=1;i<k-1;i++) coeff(lam,k,i)=lsubii(gcoeff(lam,k,i),mulii(q,gcoeff(lam,k-1,i)));
	}
      if(cmpii(mulsi(99,mulii((GEN)B[k],(GEN)B[k])),mulsi(100,addii(mulii((GEN)B[k-1],(GEN)B[k+1]),mulii(gcoeff(lam,k,k-1),gcoeff(lam,k,k-1)))))>0)
	{
	  la=(GEN)coeff(lam,k,k-1);p1=h[k-1];h[k-1]=h[k];h[k]=p1;
	  for(j=1;j<=k-2;j++) 
	    {
	      p1=(long)coeff(lam,k-1,j);coeff(lam,k-1,j)=coeff(lam,k,j);
	      coeff(lam,k,j)=p1;
	    }
	  for(i=k+1;i<=n;i++)
	    {
	      bb=(GEN)coeff(lam,i,k);
	      coeff(lam,i,k)=ldivii(subii(mulii((GEN)B[k+1],gcoeff(lam,i,k-1)),mulii(la,bb)),(GEN)B[k]);
	      coeff(lam,i,k-1)=ldivii(addii(mulii(la,gcoeff(lam,i,k-1)),mulii((GEN)B[k-1],bb)),(GEN)B[k]);
	    }
	  B[k]=ldivii(addii(mulii((GEN)B[k-1],(GEN)B[k+1]),mulii(la,la)),(GEN)B[k]);
	  if(k>2) k--;
	}
      else
	{
	  for(l=k-2;l>=1;l--)
	    {
	      if(cmpii(absi(u=shifti(gcoeff(lam,k,l),1)),(GEN)B[l+1])>0)
		{
		  q=dvmdii(addii(u,(GEN)B[l+1]),shifti((GEN)B[l+1],1),&r);
		  if(signe(r)<0) q=addsi(-1,q);
		  h[k]=lsub((GEN)h[k],gmul(q,(GEN)h[l]));
		  coeff(lam,k,l)=lsubii(gcoeff(lam,k,l),mulii(q,(GEN)B[l+1]));
		  for(i=1;i<l;i++) coeff(lam,k,i)=lsubii(gcoeff(lam,k,i),mulii(q,gcoeff(lam,l,i)));
		}
	    }
	  k++;
	  if(k>n) {tetpil=avma;return gerepile(av,tetpil,gcopy(h));}
	}
      if(avma<lim)
	{
	  tetpil=avma;
	  sv=cgetg(4,17);
	  sv[1]=lcopy(B);sv[2]=lcopy(h);sv[3]=lcopy(lam);
	  sv=gerepile(av,tetpil,sv);
	  B=(GEN)sv[1];h=(GEN)sv[2];lam=(GEN)sv[3];
	}

    }
}

GEN lllgramall(GEN x, long all)
{
  long av=avma,tetpil,lx=lg(x),tx=typ(x),i,j,k,l,p1,n,lim,dec;
  GEN u,B,lam,q,r,h,la,bb,p2,p3,p4,y,fl;

  if(tx!=19) err(lllger1);
  n=lx-1;
  if(!n)
    {
      if(all) return cgetg(1,19);
      else {y=cgetg(3,17);y[1]=lgetg(1,19);y[2]=lgetg(1,19);return y;}
    }
  if(n==1)
    {
      if(gcmp0(gcoeff(x,1,1))) 
	{
	  if(!all)
	    {y=cgetg(3,17);y[1]=(long)idmat(1);y[2]=lgetg(1,19);return y;}
	  return (all==1)?idmat(1):cgetg(1,19);
	}
      else
	{
	  if(!all) {y=cgetg(3,17);y[1]=lgetg(1,19);y[2]=(long)idmat(1);return y;}
	  return (all==1)?cgetg(1,19):idmat(1);
	}
    }
  if(lg((GEN)x[1])!=lx) err(lllger2);
  av=avma;lim=(avma+bot)>>1;
  B=cgetg(lx+1,18);
  fl=cgetg(lx,17);
  B[1]=un;lam=cgetg(lx,19);
  for(j=1;j<lx;j++) lam[j]=lgetg(lx,18);
  for(i=1;i<lx;i++) 
    for(j=1;j<=i;j++)
      {
	if((j<i)&&(!signe((GEN)fl[j]))) coeff(lam,i,j)=coeff(lam,j,i)=zero;
	else
	  {
	    u=(GEN)coeff(x,i,j);
	    if(typ(u)!=1) err(lllger4);
	    for(k=1;k<j;k++)
	      if(signe((GEN)fl[k])) u=divii(subii(mulii((GEN)B[k+1],u),mulii(gcoeff(lam,i,k),gcoeff(lam,j,k))),(GEN)B[k]);
	    if(j<i) {coeff(lam,i,j)=(long)u;coeff(lam,j,i)=zero;}
	    else 
	      {
		if(signe(u)) {B[i+1]=(long)u;coeff(lam,i,i)=fl[i]=un;}
		else {B[i+1]=B[i];coeff(lam,i,i)=fl[i]=zero;}
	      }
	  }
      }
  k=2;h=idmat(n);
  for(;;)
    {
      if(cmpii(absi(u=shifti(gcoeff(lam,k,k-1),1)),(GEN)B[k])>0)
	{
	  q=dvmdii(addii(u,(GEN)B[k]),shifti((GEN)B[k],1),&r);
	  if(signe(r)<0) q=addsi(-1,q);
	  h[k]=lsub((GEN)h[k],gmul(q,(GEN)h[k-1]));
	  coeff(lam,k,k-1)=lsubii(gcoeff(lam,k,k-1),mulii(q,(GEN)B[k]));
	  for(i=1;i<k-1;i++) coeff(lam,k,i)=lsubii(gcoeff(lam,k,i),mulii(q,gcoeff(lam,k-1,i)));
	}
      if(signe((GEN)fl[k-1])&&((cmpii(mulsi(99,mulii((GEN)B[k],(GEN)B[k])),mulsi(100,addii(p3=mulii((GEN)B[k-1],(GEN)B[k+1]),p4=mulii(la=(GEN)coeff(lam,k,k-1),gcoeff(lam,k,k-1)))))>0)||(!signe((GEN)fl[k]))))
	{
	  p1=h[k-1];h[k-1]=h[k];h[k]=p1;
	  for(j=1;j<=k-2;j++) 
	    {
	      p1=coeff(lam,k-1,j);coeff(lam,k-1,j)=coeff(lam,k,j);
	      coeff(lam,k,j)=p1;
	    }
	  if(signe((GEN)fl[k]))
	    {
	      for(i=k+1;i<=n;i++)
		{
		  bb=(GEN)coeff(lam,i,k);
		  coeff(lam,i,k)=ldivii(subii(mulii((GEN)B[k+1],gcoeff(lam,i,k-1)),mulii(la,bb)),(GEN)B[k]);
		  coeff(lam,i,k-1)=ldivii(addii(mulii(la,gcoeff(lam,i,k-1)),mulii((GEN)B[k-1],bb)),(GEN)B[k]);
		}
	      B[k]=ldivii(addii(p3,p4),(GEN)B[k]);
	    }
	  else
	    {
	      if(signe(la))
		{
		  p2=(GEN)B[k];p1=ldivii(p4,p2);
		  for(i=k+1;i<lx;i++)
		    coeff(lam,i,k-1)=ldivii(mulii(la,gcoeff(lam,i,k-1)),p2);
		  for(j=k+1;j<lx-1;j++)
		    for(i=j+1;i<lx;i++)
		      coeff(lam,i,j)=ldivii(mulii((GEN)p1,gcoeff(lam,i,j)),p2);
		  B[k+1]=B[k]=p1;
		  for(i=k+2;i<=lx;i++)
		    B[i]=ldivii(mulii((GEN)p1,(GEN)B[i]),p2);
		}
	      else
		{
		  coeff(lam,k,k-1)=zero;
		  for(i=k+1;i<lx;i++)
		    {coeff(lam,i,k)=coeff(lam,i,k-1);coeff(lam,i,k-1)=zero;}
		  B[k]=B[k-1];fl[k]=un;fl[k-1]=zero;
		}
	    }
	  if(k>2) k--;
	}
      else
	{
	  for(l=k-2;l>=1;l--)
	    {
	      if(cmpii(absi(u=shifti(gcoeff(lam,k,l),1)),(GEN)B[l+1])>0)
		{
		  q=dvmdii(addii(u,(GEN)B[l+1]),shifti((GEN)B[l+1],1),&r);
		  if(signe(r)<0) q=addsi(-1,q);
		  h[k]=lsub((GEN)h[k],gmul(q,(GEN)h[l]));
		  coeff(lam,k,l)=lsubii(gcoeff(lam,k,l),mulii(q,(GEN)B[l+1]));
		  for(i=1;i<l;i++) coeff(lam,k,i)=lsubii(gcoeff(lam,k,i),mulii(q,gcoeff(lam,l,i)));
		}
	    }
	  k++;
	  if(k>n) 
	    {
	      for(k=1;(k<=n)&&(!signe((GEN)fl[k]));k++);
	      tetpil=avma;
	      if(!all)
		{
		  y=cgetg(3,17);
		  p2=cgetg(k,19);for(i=1;i<k;i++) p2[i]=lcopy((GEN)h[i]);
		  y[1]=(long)p2;p2=cgetg(n-k+2,19);y[2]=(long)p2;
		  for(i=k;i<=n;i++) p2[i-k+1]=lcopy((GEN)h[i]);
		}
	      else
		{
		  if(all==1)
		    {
		      y=cgetg(k,19);for(i=1;i<k;i++) y[i]=lcopy((GEN)h[i]);
		    }
		  else
		    {
		      y=cgetg(n-k+2,19);
		      for(i=k;i<=n;i++) y[i-k+1]=lcopy((GEN)h[i]);
		    }
		}
	      return gerepile(av,tetpil,y);
	    }
	}
      if(avma<lim)
	{
	  tetpil=avma;
	  B=gcopy(B);h=gcopy(h);lam=gcopy(lam);fl=gcopy(fl);
	  dec=lpile(av,tetpil,0)>>TWOPOTBYTES_IN_LONG;
	  B+=dec;h+=dec;lam+=dec;fl+=dec;
	}
    }
}

GEN lllall0(GEN x, long all)
{
  long av=avma,tetpil,lx=lg(x),tx=typ(x),i,j,k,l,p1,n,lim,dec,kmax;
  GEN u,B,lam,q,r,h,la,p2,p3,p4,y,fl;

  if(tx!=19) err(lllger1);
  n=lx-1;
  if(!n) 
    {
      if(all) return cgetg(1,19);
      else {y=cgetg(3,17);y[1]=lgetg(1,19);y[2]=lgetg(1,19);return y;}
    }
  if(n==1) 
    {
      if(gcmp0((GEN)x[1]))
	{
	  if(!all) 
	    {y=cgetg(3,17);y[1]=(long)idmat(1);y[2]=lgetg(1,19);return y;}
	  return (all==1)?idmat(1):cgetg(1,19);
	}
      else
	{
	  if(!all) {y=cgetg(3,17);y[1]=lgetg(1,19);y[2]=lcopy(x);return y;}
	  return (all==1)?cgetg(1,19):gcopy(x);
	}
    }
  av=avma;x=gcopy(x);lim=(avma+bot)>>1;
  B=cgetg(lx+1,18);for(i=1;i<=lx;i++) B[i]=zero;
  fl=cgetg(lx,17);for(i=1;i<lx;i++) fl[i]=zero;
  B[1]=un;lam=idmat(n);
  u=gscal((GEN)x[1],(GEN)x[1]);
  if(signe(u)) {B[2]=(long)u;coeff(lam,1,1)=fl[1]=un;}
  else {B[2]=un;coeff(lam,1,1)=fl[1]=zero;}
  k=2;h=idmat(n);kmax=1;
  for(;;)
    {
      if(k>kmax)
	{
	  kmax=k;
	  for(j=1;j<=k;j++)
	    {
	      if((j<k)&&(!signe((GEN)fl[j]))) coeff(lam,k,j)=zero;
	      else
		{
		  u=gscal((GEN)x[k],(GEN)x[j]);
		  if(typ(u)!=1) err(lllger4);
		  for(i=1;i<j;i++)
		    if(signe((GEN)fl[i])) u=divii(subii(mulii((GEN)B[i+1],u),mulii(gcoeff(lam,k,i),gcoeff(lam,j,i))),(GEN)B[i]);
		  if(j<k) coeff(lam,k,j)=(long)u;
		  else 
		    {
		      if(signe(u)) {B[k+1]=(long)u;coeff(lam,k,k)=fl[k]=un;}
		      else {B[k+1]=B[k];coeff(lam,k,k)=fl[k]=zero;}
		    }
		}
	    }
	}
      if(signe((GEN)fl[k-1])&&(!signe((GEN)fl[k])))
	{
	  if(cmpii(absi(u=shifti(gcoeff(lam,k,k-1),1)),(GEN)B[k])>0)
	    {
	      q=dvmdii(addii(u,(GEN)B[k]),shifti((GEN)B[k],1),&r);
	      if(signe(r)<0) q=addsi(-1,q);
	      h[k]=lsub((GEN)h[k],gmul(q,(GEN)h[k-1]));
	      x[k]=lsub((GEN)x[k],gmul(q,(GEN)x[k-1]));
	      coeff(lam,k,k-1)=lsubii(gcoeff(lam,k,k-1),mulii(q,(GEN)B[k]));
	      for(i=1;i<k-1;i++) coeff(lam,k,i)=lsubii(gcoeff(lam,k,i),mulii(q,gcoeff(lam,k-1,i)));
	    }
	  la=(GEN)coeff(lam,k,k-1);
	  p3=mulii((GEN)B[k-1],(GEN)B[k+1]);p4=mulii(la,la);
	  p1=h[k-1];h[k-1]=h[k];h[k]=p1;
	  p1=x[k-1];x[k-1]=x[k];x[k]=p1;
	  for(j=1;j<=k-2;j++) 
	    {
	      p1=(long)coeff(lam,k-1,j);coeff(lam,k-1,j)=coeff(lam,k,j);
	      coeff(lam,k,j)=p1;
	    }
	  if(signe(la))
	    {
	      p2=(GEN)B[k];p1=ldivii(p4,p2);
	      for(i=k+1;i<=kmax;i++)
		coeff(lam,i,k-1)=ldivii(mulii(la,gcoeff(lam,i,k-1)),p2);
	      for(j=k+1;j<kmax;j++)
		for(i=j+1;i<=kmax;i++)
		  coeff(lam,i,j)=ldivii(mulii((GEN)p1,gcoeff(lam,i,j)),p2);
	      B[k+1]=B[k]=p1;
	      for(i=k+2;i<=kmax+1;i++)
		B[i]=ldivii(mulii((GEN)p1,(GEN)B[i]),p2);
	    }
	  else 
	    {
	      for(i=k+1;i<=kmax;i++)
		{coeff(lam,i,k)=coeff(lam,i,k-1);coeff(lam,i,k-1)=zero;}
	      B[k]=B[k-1];fl[k]=un;fl[k-1]=zero;
	    }
	  if(k>2) k--;
	}
      else
	{
	  for(l=k-1;l>=1;l--)
	    {
	      if(cmpii(absi(u=shifti(gcoeff(lam,k,l),1)),(GEN)B[l+1])>0)
		{
		  q=dvmdii(addii(u,(GEN)B[l+1]),shifti((GEN)B[l+1],1),&r);
		  if(signe(r)<0) q=addsi(-1,q);
		  h[k]=lsub((GEN)h[k],gmul(q,(GEN)h[l]));
		  x[k]=lsub((GEN)x[k],gmul(q,(GEN)x[l]));
		  coeff(lam,k,l)=lsubii(gcoeff(lam,k,l),mulii(q,(GEN)B[l+1]));
		  for(i=1;i<l;i++) coeff(lam,k,i)=lsubii(gcoeff(lam,k,i),mulii(q,gcoeff(lam,l,i)));
		}
	    }
	  k++;
	  if(k>n) 
	    {
	      for(k=1;(k<=n)&&(!signe((GEN)fl[k]));k++);
	      tetpil=avma;
	      if(!all)
		{
		  y=cgetg(3,17);
		  p2=cgetg(k,19);for(i=1;i<k;i++) p2[i]=lcopy((GEN)h[i]);
		  y[1]=(long)p2;p2=cgetg(n-k+2,19);y[2]=(long)p2;
		  for(i=k;i<=n;i++) p2[i-k+1]=lcopy((GEN)h[i]);
		}
	      else
		{
		  if(all==1)
		    {
		      y=cgetg(k,19);for(i=1;i<k;i++) y[i]=lcopy((GEN)h[i]);
		    }
		  else
		    {
		      y=cgetg(n-k+2,19);
		      for(i=k;i<=n;i++) y[i-k+1]=lcopy((GEN)h[i]);
		    }
		}
	      return gerepile(av,tetpil,y);
	    }
	}
      if(avma<lim)
	{
	  tetpil=avma;
	  B=gcopy(B);h=gcopy(h);lam=gcopy(lam);fl=gcopy(fl);x=gcopy(x);
	  dec=lpile(av,tetpil,0)>>TWOPOTBYTES_IN_LONG;
	  B+=dec;h+=dec;lam+=dec;fl+=dec;x+=dec;
	}
    }
}

GEN newlllall0(GEN x, long all)
{
  long av=avma,tetpil,lx=lg(x),tx=typ(x),i,j,k,l,p1,n,lim,dec,kmax;
  GEN u,B,lam,q,r,h,la,p2,y,fl,a,b,c,d;

  if(tx!=19) err(lllger1);
  n=lx-1;
  if(!n) 
    {
      if(all) return cgetg(1,19);
      else {y=cgetg(3,17);y[1]=lgetg(1,19);y[2]=lgetg(1,19);return y;}
    }
  if(n==1) 
    {
      if(gcmp0((GEN)x[1]))
	{
	  if(!all) 
	    {y=cgetg(3,17);y[1]=(long)idmat(1);y[2]=lgetg(1,19);return y;}
	  return (all==1)?idmat(1):cgetg(1,19);
	}
      else
	{
	  if(!all) {y=cgetg(3,17);y[1]=lgetg(1,19);y[2]=lcopy(x);return y;}
	  return (all==1)?cgetg(1,19):gcopy(x);
	}
    }
  av=avma;x=gcopy(x);lim=(avma+bot)>>1;
  B=cgetg(lx+1,18);
  fl=cgetg(lx,17);
  B[1]=un;lam=cgetg(lx,19);
  for(j=1;j<lx;j++) lam[j]=lgetg(lx,18);
  u=gscal((GEN)x[1],(GEN)x[1]);
  if(signe(u)) {B[2]=(long)u;coeff(lam,1,1)=fl[1]=un;}
  else {B[2]=un;coeff(lam,1,1)=fl[1]=zero;}
  k=2;h=idmat(n);kmax=1;
  for(;;)
    {
      if(k>kmax)
	{
	  kmax=k;
	  for(j=1;j<=k;j++)
	    {
	      if((j<k)&&(!signe((GEN)fl[j]))) coeff(lam,k,j)=coeff(lam,j,k)=zero;
	      else
		{
		  u=gscal((GEN)x[k],(GEN)x[j]);
		  if(typ(u)!=1) err(lllger4);
		  for(i=1;i<j;i++)
		    if(signe((GEN)fl[i])) u=divii(subii(mulii((GEN)B[i+1],u),mulii(gcoeff(lam,k,i),gcoeff(lam,j,i))),(GEN)B[i]);
		  if(j<k) {coeff(lam,k,j)=(long)u;coeff(lam,j,k)=zero;}
		  else 
		    {
		      if(signe(u)) {B[k+1]=(long)u;coeff(lam,k,k)=fl[k]=un;}
		      else {B[k+1]=B[k];coeff(lam,k,k)=fl[k]=zero;}
		    }
		}
	    }
	}
/*      printf(" %ld",k);fflush(stdout); */
      if(signe((GEN)fl[k-1])&&(!signe((GEN)fl[k])))
	{
/*	  printf("\nswap"); */
	  la=(GEN)coeff(lam,k,k-1);
	  p2=bezout(la,(GEN)B[k],&a,&b);d=negi(divii(la,p2));c=divii((GEN)B[k],p2);
	  p1=ladd(gmul(a,(GEN)h[k]),gmul(b,(GEN)h[k-1]));
	  h[k-1]=ladd(gmul(c,(GEN)h[k]),gmul(d,(GEN)h[k-1]));h[k]=p1;
	  p1=ladd(gmul(a,(GEN)x[k]),gmul(b,(GEN)x[k-1]));
	  x[k-1]=ladd(gmul(c,(GEN)x[k]),gmul(d,(GEN)x[k-1]));x[k]=p1;
	  for(j=1;j<=k-2;j++) 
	    {
	      p1=laddii(mulii(a,gcoeff(lam,k,j)),mulii(b,gcoeff(lam,k-1,j)));
	      coeff(lam,k-1,j)=laddii(mulii(c,gcoeff(lam,k,j)),mulii(d,gcoeff(lam,k-1,j)));
	      coeff(lam,k,j)=p1;
	    }
	  p2=mulii(c,c);
	  for(i=k+1;i<=kmax;i++)
	    {coeff(lam,i,k)=ldivii(gcoeff(lam,i,k-1),p2);coeff(lam,i,k-1)=zero;}
	  for(j=k+1;j<kmax;j++)
	    for(i=j+1;i<=kmax;i++)
	      coeff(lam,i,j)=ldivii(gcoeff(lam,i,j),p2);
	  B[k+1]=ldivii((GEN)B[k+1],p2);
/*	  printf("\nd_k=");output(gdiv((GEN)B[k],p2));  */
	  B[k]=B[k-1];fl[k]=un;fl[k-1]=zero;
	  for(i=k+2;i<=kmax+1;i++)
	    B[i]=ldivii((GEN)B[i],p2);
	  if(k>2) k--;
	}
      else
	{
	  for(l=k-1;l>=1;l--)
	    {
	      if(cmpii(absi(u=shifti(gcoeff(lam,k,l),1)),(GEN)B[l+1])>0)
		{
/*		  printf("\nk k-L"); */
		  q=dvmdii(addii(u,(GEN)B[l+1]),shifti((GEN)B[l+1],1),&r);
		  if(signe(r)<0) q=addsi(-1,q);
		  h[k]=lsub((GEN)h[k],gmul(q,(GEN)h[l]));
		  x[k]=lsub((GEN)x[k],gmul(q,(GEN)x[l]));
		  coeff(lam,k,l)=lsubii(gcoeff(lam,k,l),mulii(q,(GEN)B[l+1]));
		  for(i=1;i<l;i++) coeff(lam,k,i)=lsubii(gcoeff(lam,k,i),mulii(q,gcoeff(lam,l,i)));
		}
	    }
	  k++;
	  if(k>n) 
	    {
	      for(k=1;(k<=n)&&(!signe((GEN)fl[k]));k++);
	      tetpil=avma;
	      if(!all)
		{
		  y=cgetg(3,17);
		  p2=cgetg(k,19);for(i=1;i<k;i++) p2[i]=lcopy((GEN)h[i]);
		  y[1]=(long)p2;p2=cgetg(n-k+2,19);y[2]=(long)p2;
		  for(i=k;i<=n;i++) p2[i-k+1]=lcopy((GEN)h[i]);
		}
	      else
		{
		  if(all==1)
		    {
		      y=cgetg(k,19);for(i=1;i<k;i++) y[i]=lcopy((GEN)h[i]);
		    }
		  else
		    {
		      y=cgetg(n-k+2,19);
		      for(i=k;i<=n;i++) y[i-k+1]=lcopy((GEN)h[i]);
		    }
		}
	      return gerepile(av,tetpil,y);
	    }
	}
      if(avma<lim)
	{
	  tetpil=avma;
	  B=gcopy(B);h=gcopy(h);lam=gcopy(lam);fl=gcopy(fl);x=gcopy(x);
	  dec=lpile(av,tetpil,0)>>TWOPOTBYTES_IN_LONG;
	  B+=dec;h+=dec;lam+=dec;fl+=dec;x+=dec;
	}
    }
}


GEN lllgramint(GEN x)
{
  return lllgramall(x,2);
}

GEN lllgramkerim(GEN x)
{
  return lllgramall(x,0);
}

/********************************************************************/
/********************************************************************/
/**                                                                **/
/**               DEPENDANCE LINEAIRE ET ALGEBRIQUE                **/
/**                                                                **/
/********************************************************************/
/********************************************************************/

GEN    lindep4(GEN x, long bit, long prec)    /* C.B. 27/11/91 */
     
{
  long lx=lg(x),cpt=0;
  long av0,av,tetpil,lim,l,i,j,k,n,e;
  GEN  G0,G1,G,D,mu,u,B,BB,BK,p,q,r,cst,unreel,sv,mu1,mu2,pro;

  av0=avma;
  cst=gdivgs(stoi(95),100); /* LLL proposent 0.75 */
  lim=(avma+bot)>>1;
  if (prec)
    {
      affsr(1,unreel=cgetr(prec+1));
      cst=gmul(cst,unreel);
    }
  G0=idmat(n=lx-1);bit=(long)(bit/L2SL10);
  for(i=1;i<=n;i++)
    {
      for(j=i+1;j<=n;j++)
	coeff(G0,i,j)=coeff(G0,j,i)=lshift(greal(gmul((GEN)x[i],(GEN)x[j])),bit);
      coeff(G0,i,i)=lshift(gnorm((GEN)x[i]),bit);
      if(i>1)
	coeff(G0,i,i)=laddgs(gcoeff(G0,i,i),1);
    }
  sor(G0,'e',12,0);
  av=avma;


  mu=idmat(n=lx-1);
  B=cgetg(lx,17);
  B[1]=lnorm((GEN)x[1]);
  B[2]=limag(gmul((GEN)x[1],gconj((GEN)x[2])));
  for(j=2;j<lx;j++)
    coeff(mu,j,1)=ldiv(greal(gmul((GEN)x[1],gconj((GEN)x[j]))),(GEN)B[1]);
  if(!gcmp0((GEN)B[2]))
     {
       for(j=3;j<lx;j++)
	 coeff(mu,j,2)=ldiv(gimag(gmul((GEN)x[1],gconj((GEN)x[j]))),(GEN)B[2]);
       B[2]=lshift(gdiv(gmul((GEN)B[2],(GEN)B[2]),(GEN)B[1]),bit);
     }

  else B[2 ]=(long)unreel;
  B[1]=lshift((GEN)B[1],bit);
  for(j=3;j<=n;j++) B[j]=(long)unreel;

  D=idmat(n);
  for(i=1;i<=n;i++) coeff(D,i,i)=B[i];
  G1=gmul(gmul(mu,D),gtrans(mu));
  sor(G1,'e',12,0);


  sor(mu,'e',12,0);
  sor(sqred3(G1),'e',12,0);
/*-- LLL : ---------------------------------------*/

  u=idmat(n);
  k=2;cpt=0;
  do
    {
/*      printf("k=%d  cpt=%d\n",k,cpt);fflush(stdout); */
      if(!gcmp0(r=grndtoi(gcoeff(mu,k,k-1),&e)))
        {
          u[k]=lsub((GEN)u[k],gmul(r,(GEN)u[k-1]));
          for(j=1;j<k-1;j++)
            coeff(mu,k,j)=lsub(gcoeff(mu,k,j),gmul(r,gcoeff(mu,k-1,j)));
	  mu1=(GEN)(coeff(mu,k,k-1)=lsub(gcoeff(mu,k,k-1),r));
	  affrr(mu1,pro=cgetr(prec));
	  mu1=(GEN)(coeff(mu,k,k-1)=(long)pro);
        }
      else mu1=gcoeff(mu,k,k-1);
      q=gmul((GEN)B[k-1],gsub(cst,mu2=gsqr(mu1)));
      if(gcmp(q,(GEN)B[k])>0)
        {
/********** ici on permute bk et b(k-1) ***************************/

          BB=gadd((GEN)B[k],gmul((GEN)B[k-1],mu2));
          coeff(mu,k,k-1)=ldiv(gmul(mu1,(GEN)B[k-1]),BB);
          B[k]=lmul((GEN)B[k-1],BK=gdiv((GEN)B[k],BB));
          B[k-1]=(long)BB;
          pro=(GEN)u[k];u[k]=u[k-1];u[k-1]=(long)pro;
          for(j=1;j<=k-2;j++)
            {
              pro=gcoeff(mu,k,j);coeff(mu,k,j)=coeff(mu,k-1,j);
              coeff(mu,k-1,j)=(long)pro;
            }
          for(i=k+1;i<=n;i++)
            {
              p=(GEN)coeff(mu,i,k);
	      coeff(mu,i,k)=lsub(gcoeff(mu,i,k-1),gmul(mu1,p));
              coeff(mu,i,k-1)=ladd(gmul(BK,p),gmul(gcoeff(mu,k,k-1),gcoeff(mu,i,k-1)));
            }
          if(k>2) {k--;cpt++;}
        }
      else
        {
/********** ici bk et b(k-1) OK on rend propre *******************/
          for(l=k-2;l;l--)
	    if(!gcmp0(r=grndtoi(gcoeff(mu,k,l),&e)))
	      {
		u[k]=lsub((GEN)u[k],gmul(r,(GEN)u[l]));
		for(j=1;j<l;j++)
		  coeff(mu,k,j)=lsub(gcoeff(mu,k,j),gmul(r,gcoeff(mu,l,j)));
		
		coeff(mu,k,l)=lsub(gcoeff(mu,k,l),r);
	      }
	  G=gmul(gmul(gtrans(u),G0),u);
/*  printf("sqred1 : cpt = %d\n",cpt); */
	  mu=gtrans(sqred1(G));
          k++;cpt++;
/******************************************************************/
        }
      if(avma<lim)
	{
	  tetpil=avma;
	  sv=cgetg(4,17);
	  sv[1]=lcopy(B);sv[2]=lcopy(u);sv[3]=lcopy(mu);
	  sv=gerepile(av,tetpil,sv);
	  B=(GEN)sv[1];u=(GEN)sv[2];mu=(GEN)sv[3];
	}
    }
  while(k<=n);
/*  printf("\n *** %d etapes dans LLL\n",cpt); */
  tetpil=avma;return gerepile(av,tetpil,gcopy((GEN)u[1]));
}

GEN    lindep3(GEN x, long bit, long prec)    /* C.B. 13/11/91 */

            
                   
     
{
  long lx=lg(x),cpt=0;
  long av0,av,tetpil,lim,l,i,j,k,n,e;
  GEN  mu,u,B,BB,BK,p,q,r,cst,unreel,sv,pro,mu1,mu2;



  av0=avma;
  cst=gdivgs(stoi(95),100); /* LLL proposent 0.75 */
  lim=(avma+bot)>>1;
  if (prec)
    {
      affsr(1,unreel=cgetr(prec+1));
      cst=gmul(cst,unreel);
    }
  av=avma;

  mu=idmat(n=lx-1);bit=(long)(bit/L2SL10);
  B=cgetg(lx,17);
  B[1]=lnorm((GEN)x[1]);
  B[2]=limag(gmul((GEN)x[1],gconj((GEN)x[2])));
  for(j=2;j<lx;j++)
    coeff(mu,j,1)=ldiv(greal(gmul((GEN)x[1],gconj((GEN)x[j]))),(GEN)B[1]);
  if(!gcmp0((GEN)B[2]))
     {
       for(j=3;j<lx;j++)
	 coeff(mu,j,2)=ldiv(gimag(gmul((GEN)x[1],gconj((GEN)x[j]))),(GEN)B[2]);
       B[2]=lshift(gdiv(gmul((GEN)B[2],(GEN)B[2]),(GEN)B[1]),bit);
     }

  else B[2 ]=(long)unreel;
  B[1]=lshift((GEN)B[1],bit);
  for(j=3;j<=n;j++) B[j]=(long)unreel;
/*-- LLL : ---------------------------------------*/

  u=idmat(n);
  k=2;cpt=0;
  do
    {

/*      printf("k=%d  cpt=%d\n",k,cpt);fflush(stdout); */
      if(!gcmp0(r=grndtoi(gcoeff(mu,k,k-1),&e)))
        {
          u[k]=lsub((GEN)u[k],gmul(r,(GEN)u[k-1]));
          for(j=1;j<k-1;j++)
            coeff(mu,k,j)=lsub(gcoeff(mu,k,j),gmul(r,gcoeff(mu,k-1,j)));
	  mu1=(GEN)(coeff(mu,k,k-1)=lsub(gcoeff(mu,k,k-1),r));
	  affrr(mu1,pro=cgetr(prec));
	  mu1=(GEN)(coeff(mu,k,k-1)=(long)pro);
        }
      else mu1=gcoeff(mu,k,k-1);
      q=gmul((GEN)B[k-1],gsub(cst,mu2=gsqr(mu1)));
      if(gcmp(q,(GEN)B[k])>0)
        {
          BB=gadd((GEN)B[k],gmul((GEN)B[k-1],mu2));
          coeff(mu,k,k-1)=ldiv(gmul(mu1,(GEN)B[k-1]),BB);
          B[k]=lmul((GEN)B[k-1],BK=gdiv((GEN)B[k],BB));
          B[k-1]=(long)BB;
          pro=(GEN)u[k];u[k]=u[k-1];u[k-1]=(long)pro;
          for(j=1;j<=k-2;j++)
            {
              pro=gcoeff(mu,k,j);coeff(mu,k,j)=coeff(mu,k-1,j);
              coeff(mu,k-1,j)=(long)pro;
            }
          for(i=k+1;i<=n;i++)
            {
              p=(GEN)coeff(mu,i,k);
	      coeff(mu,i,k)=lsub(gcoeff(mu,i,k-1),gmul(mu1,p));
              coeff(mu,i,k-1)=ladd(gmul(BK,p),gmul(gcoeff(mu,k,k-1),gcoeff(mu,i,k-1)));
            }
          if(k>2) {k--;cpt++;}
        }
      else
        {
          for(l=k-2;l;l--)
            {
              if(!gcmp0(r=grndtoi(gcoeff(mu,k,l),&e)))
                {
/*		  printf("k, l, e, r: %d, %d, %d, ",k,l,e);output(r); */
                  u[k]=lsub((GEN)u[k],gmul(r,(GEN)u[l]));
                  for(j=1;j<l;j++)
                    coeff(mu,k,j)=lsub(gcoeff(mu,k,j),gmul(r,gcoeff(mu,l,j)));

		  pro=(GEN)(coeff(mu,k,l)=lsub(gcoeff(mu,k,l),r));
		  affrr(pro,(GEN)(coeff(mu,k,l)=lgetr(prec)));
                }
            }
          k++;cpt++;
        }
      if(avma<lim)
	{
	  tetpil=avma;
	  sv=cgetg(4,17);
	  sv[1]=lcopy(B);sv[2]=lcopy(u);sv[3]=lcopy(mu);
	  sv=gerepile(av,tetpil,sv);
	  B=(GEN)sv[1];u=(GEN)sv[2];mu=(GEN)sv[3];
	}
    }
  while(k<=n);
/*  printf("\n *** %d etapes dans LLL\n",cpt); */
  tetpil=avma;return gerepile(av,tetpil,gcopy((GEN)u[1]));
}


GEN    lindep2(GEN x, long bit, long prec)
{
  long  tx=typ(x),lx=lg(x),ly,i,j,flag,av,tetpil,e;
  GEN   y,p1,p2,p3,p4,p5;
  
  if((tx<17)||(tx>18)) err(linder1);
  av=avma;p1=greal(x);p2=gimag(x);
  ly=(flag=!gcmp0(p2)) ? lx+2 : lx+1;
  p4=cgetg(lx,19);bit=(long)(bit/L2SL10);
  for(i=1;i<lx;i++)
  {
    p5=cgetg(ly,18);p4[i]=(long)p5;
    for(j=1;j<lx;j++) p5[j]=(i==j) ? un : zero;
    p5[lx]=lcvtoi(gshift((GEN)p1[i],bit),&e);
    if(flag) p5[lx+1]=lcvtoi(gshift((GEN)p2[i],bit),&e);
  }
  p5=gmul(p4,lllint(p4));p3=(GEN)p5[1];
  tetpil=avma;y=cgetg(lx,17);
  for(i=1;i<lx;i++) y[i]=lcopy((GEN)p3[i]);
  y=gerepile(av,tetpil,y);
  return y;
}

#define quazero(x) (gcmp0(x)||((typ(x)==2)&&(expo(x)< -((prec-2)<<TWOPOTBITS_IN_LONG)+2*n)))

GEN    lindep(GEN x, long prec)
{
  GEN b[50],be[50],bn[50],m[50][50],y,c1,c2,c3,px,py,pxy;
  GEN p1,p2,p3,p4,p5,p6,p7,r,f,em;
  long av,av1,tetpil,lx=lg(x),tx=typ(x),n,i,j,fl,i1;
  long qzer[50];
  
  if((tx<17)||(tx>18)) err(linder1);
  if(lx>=50) err(linder2);
  av=avma;n=lx-1;p1=greal(x);p2=gimag(x);
  px=gscal(p1,p1);py=gscal(p2,p2);
  pxy=gscal(p1,p2);p3=mpsub(mpmul(px,py),mpmul(pxy,pxy));
  for(i=1;i<=n;i++)
  {
    be[i]=cgetg(lx,18);
    for(j=1;j<=n;j++) be[i][j]=lgetr(prec+1);
  }
  for(j=1;j<=n;j++) bn[j]=cgetr(prec+1);
  for(i=1;i<=n;i++)
  {
    for(j=1;j<i;j++)
      m[i][j]=cgetr(prec+1);
  }
  if(quazero(p1)) {p1=p2;px=py;fl=1;} else fl=quazero(p3);
  for(i=1;i<=n;i++)
  {
    b[i]=cgetg(lx,18);
    for(j=1;j<=n;j++)
      b[i][j]=(i==j) ? un : zero;
  }
  av1=avma;
  for(i=1;i<=n;i++)
  {
    if(fl) p4=gmul(gdiv(gscal(b[i],p1),px),p1);
    else
    {
      p4=gscal(b[i],p1);p5=gscal(b[i],p2);
      p6=gdiv(mpsub(mpmul(py,p4),mpmul(pxy,p5)),p3);
      p7=gdiv(mpsub(mpmul(px,p5),mpmul(pxy,p4)),p3);
      p4=gadd(gmul(p6,p1),gmul(p7,p2));
    }
    if(tx==18) p4=gsub(b[i],p4);
    else p4=gsub(b[i],gtrans(p4));
    for(j=1;j<i;j++)
      if(!qzer[j])
      {
        gdivz(gscal(b[i],be[j]),bn[j],m[i][j]);
        p4=gsub(p4,gmul(m[i][j],be[j]));
      }
      else affrr(bn[j],m[i][j]);
    gaffect(p4,be[i]);
    affrr(gscal(be[i],be[i]),bn[i]);
    qzer[i]=quazero(bn[i]);avma=av1;
  }
  while(qzer[n])
  {
    em=bn[1];j=1;av1=avma;
    for(i=2;i<n;i++)
    {
      p3=shiftr(bn[i],i);
      if(cmprr(p3,em)>0)
      {
        em=p3;j=i;
      }
    }
    avma=av1;
/*    printf("\n");
    for(i1=1;i1<=n;i1++)
      for(i=1;i<i1;i++) output(m[i1][i]); */
    i=j;i1=i+1;
    r=ground(m[i1][i]);f=subri(m[i1][i],r);
    p3=gsub(b[i1],gmul(r,b[i]));
    b[i1]=b[i];b[i]=p3;
    for(j=1;j<i;j++)
    {
      if(!qzer[j])
      {
        p3=mpsub(m[i1][j],mulir(r,m[i][j]));
        affrr(m[i][j],m[i1][j]);mpaff(p3,m[i][j]);
      }
    }
    c1=addrr(bn[i1],mulrr(mulrr(f,f),bn[i]));
    fl=quazero(c1);
    if(!fl)
    {
      c2=divrr(mulrr(bn[i],f),c1);affrr(c2,m[i1][i]);
      c3=divrr(bn[i1],c1);mulrrz(c3,bn[i],bn[i1]);
      affrr(c1,bn[i]);qzer[i1]=quazero(bn[i1]);qzer[i]=0;
      for(j=i+2;j<=n;j++)
      {
        p3=addrr(mulrr(m[j][i1],c3),mulrr(m[j][i],c2));
        subrrz(m[j][i],mulrr(f,m[j][i1]),m[j][i1]);
        affrr(p3,m[j][i]);
      }
    }
    else
    {
      qzer[i1]=qzer[i];affrr(bn[i],bn[i1]);
      affrr(c1,bn[i]);qzer[i]=1;
      for(j=i+2;j<=n;j++) affrr(m[j][i],m[j][i1]);
    }
  }
  p3=cgetg(lx,18);
  for(i=1;i<=n;i++) p3[i]=(i==n) ? un : zero;
  p3[n]=un;p5=cgetg(lx,19);
  for(i=1;i<=n;i++) p5[i]=lcopy(b[i]);
  p4=gauss(gtrans(p5),p3);tetpil=avma;
  y=gerepile(av,tetpil,gtrans(p4));
  return y;
}

GEN    algdep(GEN x, long n, long prec)
{
  long   tx=typ(x),av,tetpil,i,j,k;
  GEN    y,p1;
  
  if(tx>=10) err(algder1);
  if(tx==9) {y=gcopy((GEN)x[1]);setvarn(y,0);return y;}
  if(gcmp0(x)) return gzero;
  av=avma;p1=cgetg(n+2,18);p1[1]=un;
  for(i=2;i<=n+1;i++) p1[i]=lmul((GEN)p1[i-1],x);
  p1=lindep(p1,prec);
  tetpil=avma;y=cgetg(n+3,10);
  setsigne(y,1);setvarn(y,0);j=0;
  k=1;while(gcmp0((GEN)p1[k])) k++;
  for(i=0;i<=n+1-k;i++)
  {
    y[i+2]=lcopy((GEN)p1[k+i]);
    if(!gcmp0((GEN)p1[k+i])) j=i;
  }
  setlgef(y,j+3);
  if(gsigne((GEN)y[j+2])>0) return gerepile(av,tetpil,y);
  else {tetpil=avma;return gerepile(av,tetpil,gneg(y));}
}

GEN    algdep2(GEN x, long n, long bit, long prec)
{
  long   tx=typ(x),av,tetpil,i,j,k;
  GEN    y,p1;
  
  if(tx>=10) err(algder1);
  if(tx==9) {y=gcopy((GEN)x[1]);setvarn(y,0);return y;}
  if(gcmp0(x)) return gzero;
  av=avma;p1=cgetg(n+2,18);p1[1]=un;
  for(i=2;i<=n+1;i++) p1[i]=lmul((GEN)p1[i-1],x);
  p1=lindep2(p1,bit,prec);
  tetpil=avma;y=cgetg(n+3,10);
  setsigne(y,1);setvarn(y,0);j=0;
  k=1;while(gcmp0((GEN)p1[k])) k++;
  for(i=0;i<=n+1-k;i++)
  {
    y[i+2]=lcopy((GEN)p1[k+i]);
    if(!gcmp0((GEN)p1[k+i])) j=i;
  }
  setlgef(y,j+3);
  if(gsigne((GEN)y[j+2])>0) return gerepile(av,tetpil,y);
  else {tetpil=avma;return gerepile(av,tetpil,gneg(y));}
}

/********************************************************************/
/********************************************************************/
/**                                                                **/
/**                   CHANGEMENTS DE VARIABLES                     **/
/**                                                                **/
/********************************************************************/
/********************************************************************/

GEN  changevar(GEN x, GEN y)
     
              
     
/* Substitution globale des composantes du vecteur y aux variables de x */
  
{
  long tx=typ(x),ty=typ(y),lx=lg(x),ly=lg(y),vx,vy,i,av,tetpil;
  GEN  p1,p2,p3,z;
  
  if((ty<17) || (ty>18)) err(changer1);
  if(tx<9) return gcopy(x);
  if(tx==9)
    {
      av=avma;p1=changevar((GEN)x[1],y);p2=changevar((GEN)x[2],y);
      if(isonstack((GEN)x[1])) 
	{tetpil=avma;return gerepile(av,tetpil,gmodulcp(p1,p2));}
      else {tetpil=avma;return gerepile(av,tetpil,gmodulo(p1,p2));}
    }
  if((tx==13)||(tx==14))
    {
      av=avma;p1=changevar((GEN)x[1],y);p2=changevar((GEN)x[2],y);
      tetpil=avma;return gerepile(av,tetpil,gdiv(p1,p2));
    }
  if(tx>11)
  {
    z=cgetg(lx,tx);
    for(i=1;i<lx;i++) z[i]=lchangevar((GEN)x[i],y);
    return z;
  }
  vx=varn(x)+1;if(vx>=ly) return gcopy(x);
  if(!signe(x))
  {
    vy=gvar((GEN)y[vx]); if(vy>MAXVARN) err(changer1);
    z=gcopy(x);
    setvarn(z,vy);
  }
  else
  {
    av=avma;p1=(GEN)y[vx];
    if(tx==10)
    {
      lx=lgef(x);
      p2=changevar((GEN)x[lx-1],y);
      for(i=lx-2;i>=2;i--)
      {
        p2=gmul(p2,p1);p3=changevar((GEN)x[i],y);
        tetpil=avma;p2=gadd(p2,p3);
      }
      if(lx>3) z=gerepile(av,tetpil,p2);
      else z=p2;
    }
    else
    {
      p2=changevar((GEN)x[lx-1],y);
      for(i=lx-2;i>=2;i--)
      {
        p2=gmul(p2,p1);p3=changevar((GEN)x[i],y);
        p2=gadd(p2,p3);
      }
      p3=ggrando(p1,lx-2);tetpil=avma;
      z=gadd(p2,p3);
      if(valp(x))
      {
        p2=gpuigs(p1,valp(x));tetpil=avma;
        z=gerepile(av,tetpil,gmul(p2,z));
      }
      else z=gerepile(av,tetpil,z);
    }
  }
  return z;
}

GEN reorder(GEN x)
{
  long tx=typ(x), lx=lg(x), t1[MAXVAR], i, j, n;
  
  if((tx < 17) || (tx > 18)) err(reorder1);
  for(i=0; i<nvar; i++) t1[i] = 0;
  for(n=1; n<lx; n++)
  {
    i = gvar((GEN)x[n]);
    if (i >= nvar) err(reorder2);
    if (t1[i]) err(reorder3);
    t1[i] = 1;
  }
  for(i=n=0; i<nvar; i++)
    if (t1[i])
    {
      j = gvar((GEN)x[++n]);
      polvar[j+1]=lpolx[i];
      ordvar[i]=j;
    }
  varchanged=0;
  for(i=0;i<nvar;i++) if(ordvar[i]!=i) {varchanged=1;break;}
  return polvar;
}

/********************************************************************/
/********************************************************************/
/**                                                                **/
/**                       TRI PAR HEAPSORT                         **/
/**                                                                **/
/********************************************************************/
/********************************************************************/

GEN sort(GEN x)
{
  long av,tetpil,n,i,j,indxt,q,ir,l,tx=typ(x),lx=lg(x);
  GEN y,indx;

  if((tx<17)||(tx>18)) err(sorter1);
  av=avma;n=lx-1;if(n<=1) return gcopy(x);
  indx=cgeti(lx);for(j=1;j<=n;j++) indx[j]=j;
  l=(n>>1)+1;ir=n;
  for(;;)
  {
    if(l>1) q=x[(indxt=indx[--l])];
    else
    {
      q=x[(indxt=indx[ir])];indx[ir]=indx[1];
      if(--ir ==1)
      {
        indx[1]=indxt;tetpil=avma;y=cgetg(lx,tx);
        for(i=1;i<=n;i++) y[i]=lcopy((GEN)x[indx[i]]);
        return gerepile(av,tetpil,y);
      }
    }
    i=l;j=l<<1;
    while(j<=ir)
    {
      if(j<ir && gcmp((GEN)x[indx[j]],(GEN)x[indx[j+1]])<0) j++;
      if(gcmp((GEN)q,(GEN)x[indx[j]])<0) {indx[i]=indx[j];j+=(i=j);}
      else j=ir+1;
    }
    indx[i]=indxt;
  }
}

GEN lexsort(GEN x)
{
  long av,tetpil,n,i,j,indxt,q,ir,l,tx=typ(x),lx=lg(x);
  GEN y,indx;

  if((tx<17)||(tx>18)) err(sorter1);
  av=avma;n=lx-1;if(n<=1) return gcopy(x);
  indx=cgeti(lx);for(j=1;j<=n;j++) indx[j]=j;
  l=(n>>1)+1;ir=n;
  for(;;)
  {
    if(l>1) q=x[(indxt=indx[--l])];
    else
    {
      q=x[(indxt=indx[ir])];indx[ir]=indx[1];
      if(--ir ==1)
      {
        indx[1]=indxt;tetpil=avma;y=cgetg(lx,tx);
        for(i=1;i<=n;i++) y[i]=lcopy((GEN)x[indx[i]]);
        return gerepile(av,tetpil,y);
      }
    }
    i=l;j=l<<1;
    while(j<=ir)
    {
      if(j<ir && lexcmp((GEN)x[indx[j]],(GEN)x[indx[j+1]])<0) j++;
      if(lexcmp((GEN)q,(GEN)x[indx[j]])<0) {indx[i]=indx[j];j+=(i=j);}
      else j=ir+1;
    }
    indx[i]=indxt;
  }
}

long gveccmp(GEN x,GEN y,GEN k)
{
  long lk=lg(k),i=1,fl=0;
  while((i<lk)&&((fl=gcmp((GEN)x[k[i]],(GEN)y[k[i]]))==0)) i++;
  return fl;
}

GEN vecsort(GEN x, GEN k)
{
  long av,tetpil,n,i,j,indxt,q,ir,l,t,tx=typ(x),lx=lg(x),tk=typ(k),lk;
  GEN y,indx,kk;

  if(tx<17) err(sorter1);
  av=avma;n=lx-1;if(n<=1) return gcopy(x);
  if(tk==1) {y=cgetg(2,17);y[1]=(long)k;k=y;}
  else if((tk<17)||(tk>18)) err(vecsorter1);
  q=0;lk=lg(k);kk=cgeti(lk);
  for(i=1;i<lk;i++) 
    {
      l=itos((GEN)k[i]);if(l<=0) err(vecsorter2);
      q=max(q,l);kk[i]=l;
    }
  indx=cgeti(lx);
  for(j=1;j<=n;j++)
    {
      indx[j]=j;t=typ((GEN)x[j]);
      if((t<17)||(t>18)) err(sorter1);
      if(lg((GEN)x[j])<=q) err(vecsorter2);
    }
  l=(n>>1)+1;ir=n;
  for(;;)
  {
    if(l>1) q=x[(indxt=indx[--l])];
    else
    {
      q=x[(indxt=indx[ir])];indx[ir]=indx[1];
      if(--ir ==1)
      {
        indx[1]=indxt;tetpil=avma;y=cgetg(lx,tx);
        for(i=1;i<=n;i++) y[i]=lcopy((GEN)x[indx[i]]);
        return gerepile(av,tetpil,y);
      }
    }
    i=l;j=l<<1;
    while(j<=ir)
    {
      if(j<ir && gveccmp((GEN)x[indx[j]],(GEN)x[indx[j+1]],kk)<0) j++;
      if(gveccmp((GEN)q,(GEN)x[indx[j]],kk)<0) {indx[i]=indx[j];j+=(i=j);}
      else j=ir+1;
    }
    indx[i]=indxt;
  }
}

GEN indexsort(GEN x)
{
  long av,tetpil,n,i,j,indxt,q,ir,l,tx=typ(x),lx=lg(x);
  GEN y,indx;

  if((tx<17)||(tx>18)) err(sorter1);
  av=avma;n=lx-1;if(n<=1) {y=cgetg(lx,tx);y[1]=un;return y;}
  indx=cgeti(lx);for(j=1;j<=n;j++) indx[j]=j;
  l=(n>>1)+1;ir=n;
  for(;;)
  {
    if(l>1) q=x[(indxt=indx[--l])];
    else
    {
      q=x[(indxt=indx[ir])];indx[ir]=indx[1];
      if(--ir ==1)
      {
        indx[1]=indxt;tetpil=avma;y=cgetg(lx,tx);
        for(i=1;i<=n;i++) y[i]=lstoi(indx[i]);
        return gerepile(av,tetpil,y);
      }
    }
    i=l;j=l<<1;
    while(j<=ir)
    {
      if(j<ir && gcmp((GEN)x[indx[j]],(GEN)x[indx[j+1]])<0) j++;
      if(gcmp((GEN)q,(GEN)x[indx[j]])<0) {indx[i]=indx[j];j+=(i=j);}
      else j=ir+1;
    }
    indx[i]=indxt;
  }
}

/********************************************************************/
/********************************************************************/
/**                                                                **/
/**                  INTERPOLATION POLYNOMIALE                     **/
/**                                                                **/
/********************************************************************/
/********************************************************************/

GEN  polint(GEN xa, GEN ya, GEN x, GEN *dy)
{
  long av,av1,tetpil,dec,i,m,ns=1,tx=typ(xa),ty=typ(ya),n,lx=lg(xa),ly=lg(ya);
  GEN den,dif,dift,ho,hp,w,y,c,d;

  if((tx<17)||(tx>18)||(ty<17)||(ty>18)||(lx!=ly)) err(polinter1);
  n=lx-1;if(n<=1) {y=gcopy((GEN)ya[1]);*dy=gcopy(y);return y;}
  av=avma;c=cgetg(ly,ty);d=cgetg(ly,ty);
  dif=gabs(gsub(x,(GEN)xa[1]),5);
  for(i=1;i<=n;i++)
    {
      if(gcmp((dift=gabs(gsub(x,(GEN)xa[i]),5)),dif)<0) {ns=i;dif=dift;}
      c[i]=ya[i];d[i]=ya[i];
    }
  y=(GEN)ya[ns--];
  for(m=1;m<n;m++)
    {
      for(i=1;i<=n-m;i++)
        {
          ho=gsub((GEN)xa[i],x);hp=gsub((GEN)xa[i+m],x);w=gsub((GEN)c[i+1],(GEN)d[i]);
          if(gcmp0(den=gsub(ho,hp))) err(polinter2);
          den=gdiv(w,den);d[i]=lmul(hp,den);c[i]=lmul(ho,den);
        }
      *dy=(2*ns<(n-m))?(GEN)c[ns+1]:(GEN)d[ns--];
      tetpil=avma;y=gadd(y,*dy);
    }
  *dy=gcopy(*dy);av1=avma;dec=lpile(av,tetpil,0)>>TWOPOTBYTES_IN_LONG;
  if(adecaler(y,tetpil,av1)) y+=dec;
  if(adecaler(*dy,tetpil,av1)) (*dy)+=dec;
  return y;
}

/********************************************************************/
/********************************************************************/
/**                                                                **/
/**                    POLRED ET COMPAGNIE                         **/
/**                                                                **/
/********************************************************************/
/********************************************************************/

GEN polsym(GEN x, long n)
{
  long av1,av2,tx=typ(x),dx=lgef(x)-3,i,k;
  GEN s,y;

  if((tx!=10)||(!signe(x))) err(poltyper);
  y=cgetg(n+2,18);y[1]=lstoi(dx);
  if(n<0) err(impl,"polsym of a negative n");
  for(k=1;k<=n;k++)
    {
      av1=avma;s=(dx>=k) ? gmulsg(k,(GEN)x[dx+2-k]) : gzero;
      for(i=1;(i<k)&&(i<=dx);i++)
	s=gadd(s,gmul((GEN)y[k-i+1],(GEN)x[dx+2-i]));
      if(!gcmp1((GEN)x[dx+2])) s=gdiv(s,(GEN)x[dx+2]);
      av2=avma;y[k+1]=lpile(av1,av2,gneg(s));
    }
  return y;
}

GEN allpolred(GEN x, GEN *pta, long code, long prec)
{
  GEN y,p1,p2,p3,p4,p5,p6,p7,ptrace,a;
  long tx=typ(x),n=lgef(x)-3,i,j,k,dec,av=avma,av1,tet1,tet2,tetpil,v;
 
  if(tx!=10) err(poltyper);
  if(!signe(x)) return gcopy(x); 
  if(!gcmp1((GEN)x[n+2])) err(poltyper);
  p4=allbase4(x,code,&p2);
  if(sturm(x)<n)
    {
      p2=roots(x,prec);p3=cgetg(n+1,19);
      for(i=1;i<=n;i++)
	{
	  p1=cgetg(n+1,18);p3[i]=(long)p1;
	  for(j=1;j<=n;j++)
	    p1[j]=lsubst((GEN)p4[i],varn((GEN)p4[i]),(GEN)p2[j]);
	}
      p2=greal(gmul(gconj(gtrans(p3)),p3));
      p1=lllgram(p2,prec);
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
    }
  v=varn(x);tet1=avma;
  a=cgetg(n+1,18);for(i=1;i<=n;i++) a[i]=lmul(p4,(GEN)p1[i]);
  tetpil=avma;y=cgetg(n+1,18);
  for(i=1;i<=n;i++)
    {
      av1=avma;p3=gmodulcp((GEN)a[i],x);p7=content((GEN)p3[2]);
      if(gcmp1(p7)) p3=caract(p3,v);
      else
	{
	  p3=caract(gdiv(p3,p7),v);
	  p3=gmul(gpuigs(p7,lgef(p3)-3),gsubst(p3,v,gdiv(polx[v],p7)));
	}
      p5=ggcd(deriv(p3,v),p3);
      p6=(GEN)p5[lgef(p5)-1];if(!gcmp1(p6)) p5=gdiv(p5,p6);
      tet2=avma;p3=gerepile(av1,tet2,gdiv(p3,p5));
      y[i]=(long)p3;j=lgef(p3)-2;
      while((j>=2)&&(!signe((GEN)p3[j]))) j-=2;
      if((j>=2)&&(signe((GEN)p3[j])>0))
	{
	  for(;j>=2;j-=2) setsigne((GEN)p3[j],-signe((GEN)p3[j]));
	  gnegz((GEN)a[i],(GEN)a[i]);
	}
    }
  if(pta) {dec=lpile(av,tet1,0)>>TWOPOTBYTES_IN_LONG;*pta=a+dec;return y+dec;}
  else return gerepile(av,tetpil,y);
}

GEN polred(GEN x, long prec)
{
  return allpolred(x,(GEN*)0,0,prec);
}

GEN smallpolred(GEN x, long prec)
{
  return allpolred(x,(GEN*)0,1,prec);
}

GEN factoredpolred(GEN x, GEN p, long prec)
{
  return allpolred(x,(GEN*)0,(long)p,prec);
}

GEN polred2(GEN x, long prec)
{
  GEN y;

  y=cgetg(3,19);
  y[2]=(long)allpolred(x,(GEN*)(y+1),0,prec);
  return y;
}

GEN smallpolred2(GEN x, long prec)
{
  GEN y;

  y=cgetg(3,19);
  y[2]=(long)allpolred(x,(GEN*)(y+1),1,prec);
  return y;
}

GEN factoredpolred2(GEN x, GEN p, long prec)
{
  GEN y;

  y=cgetg(3,19);
  y[2]=(long)allpolred(x,(GEN*)(y+1),(long)p,prec);
  return y;
}

GEN ordred(GEN x, long prec)
{
  GEN y,p1,p2,p3,p4,p5,p6,p7;
  long tx=typ(x),n=lgef(x)-3,i,j,av=avma,v,av1,tet1,tet2;
 
  if(tx!=10) err(poltyper);
  if(!signe(x)) return gcopy(x); 
  if(!gcmp1((GEN)x[n+2])) err(poltyper);
  p2=roots(x,prec);p3=cgetg(n+1,19);
  p4=cgetg(n+1,17);for(i=1;i<=n;i++) p4[i]=lpuigs(polx[varn(x)],i-1);
  for(i=1;i<=n;i++)
    {
      p1=cgetg(n+1,18);p3[i]=(long)p1;
      for(j=1;j<=n;j++)
	p1[j]=lpuigs((GEN)p2[j],i-1);
    }
  p2=greal(gmul(gconj(gtrans(p3)),p3));
  p1=lllgram(p2,prec);v=varn(x);tet1=avma;y=cgetg(n+1,18);
  for(i=1;i<=n;i++)
    {
      av1=avma;p3=gmodulcp(gmul(p4,(GEN)p1[i]),x);p7=content((GEN)p3[2]);
      if(gcmp1(p7)) p3=caract(p3,v);
      else
	{
	  p3=caract(gdiv(p3,p7),v);
	  p3=gmul(gpuigs(p7,lgef(p3)-3),gsubst(p3,v,gdiv(polx[v],p7)));
	}
      p5=ggcd(deriv(p3,v),p3);
      p6=(GEN)p5[lgef(p5)-1];if(!gcmp1(p6)) p5=gdiv(p5,p6);
      tet2=avma;p3=gerepile(av1,tet2,gdiv(p3,p5));
      y[i]=(long)p3;j=lgef(p3)-2;
      while((j>=2)&&(!signe((GEN)p3[j]))) j-=2;
      if((j>=2)&&(signe((GEN)p3[j])>0))
	{for(;j>=2;j-=2) setsigne((GEN)p3[j],-signe((GEN)p3[j]));}
    }
  return gerepile(av,tet1,y);
}

/*===================================================
  Nombre de vecteurs minimaux du reseau defini par
              la matrice de GRAM  a
====================================================*/

GEN minim1(GEN a)
{
  GEN u,r,unr;
  long n1=lg(a),n,av,nonnul,i,j,k,s,borne,norme,*x;
  double eps=0.000001,p,b,c;
  double **q,*v,*y,*z;

  n=n1-1;
  x=(long*)malloc(n1*sizeof(long));
  y=(double*)malloc(n1*sizeof(double));
  z=(double*)malloc(n1*sizeof(double));
  v=(double*)malloc(n1*sizeof(double));
  q=(double**)malloc(n1*sizeof(double*));
  for(j=1;j<=n;j++) q[j]=(double*)malloc(n1*sizeof(double));

  av=avma;
  affsr(1,unr=cgetr(6));
  u=lllgram(a,6);
  a=gmul(a,unr);a=gmul(gtrans(u),gmul(a,u));
  r=sqred1(a);
  for(j=1;j<=n;j++)
    {
      v[j]=rtodbl(gcoeff(r,j,j));
      for(i=1;i<j;i++)
	q[i][j]=rtodbl(gcoeff(r,i,j));
    }
  b=rtodbl(gcoeff(a,1,1));
  for(i=2;i<=n;i++)
    if((c=rtodbl(gcoeff(a,i,i)))<b) b=c;
  avma=av;
  borne=(long)(b+eps);
  s=0;
  do
    {
      k=n;
      y[n]=z[n]=0;
      x[n]=(long)(sqrt(borne/v[n]+eps));
      do
	{
	  do
	    {
	      if(k>1)
		{
		  k--;
		  z[k]=0;
		  for(j=k+1;j<=n;j++) z[k]=z[k]+q[k][j]*x[j];
		  p=x[k+1]+z[k+1];
		  y[k]=y[k+1]+p*p*v[k+1];
		  x[k]=(long)(floor(sqrt((borne-y[k]+eps)/v[k])-z[k]));
		}
	      while(v[k]*(x[k]+z[k])*(x[k]+z[k])>borne-y[k]+eps)
		{k++;x[k]--;}
	    }
	  while(k>1);
	  if(nonnul=(x[1]||(y[1]>eps)))
	    {
	      norme=(long)(y[1]+v[1]*(x[1]+z[1])*(x[1]+z[1])+eps);
	      if (norme==borne)	{ s++;x[k]--;}
	      else
		{ s=0; borne=norme;}
	    }
	}
      while(nonnul && s);
    }
  while(nonnul);
  free(x);free(y);free(z);
  for(j=1;j<=n;j++) free(q[j]);free(q);
  u=cgetg(3,17);
  u[1]=lstoi(s<<1);u[2]=lstoi(borne);return u;
}

GEN minim(GEN a, long borne, long stockmax)
{
  GEN u,r,unr,S,S1;
  long n1=lg(a),n,av,av1,nonnul,i,j,k,s,norme,normax,flg,*x;
  double p,b,c;
  double **q,*v,*y,*z;
  double eps=0.000001;

  n=n1-1;
  x=(long*)malloc(n1*sizeof(long));
  y=(double*)malloc(n1*sizeof(double));
  z=(double*)malloc(n1*sizeof(double));
  v=(double*)malloc(n1*sizeof(double));
  q=(double**)malloc(n1*sizeof(double*));
  for(j=1;j<=n;j++) q[j]=(double*)malloc(n1*sizeof(double));
  av=avma;
  affsr(1,unr=cgetr(6));
  u=lllgram(a,6);
  a=gmul(a,unr);a=gmul(gtrans(u),gmul(a,u));
  r=sqred1(a);
  for(j=1;j<=n;j++)
    {
      v[j]=rtodbl(gcoeff(r,j,j));
      for(i=1;i<j;i++)
	q[i][j]=rtodbl(gcoeff(r,i,j));
    }
   if(!(flg=borne))   /* flg=0 <==> chercher les vec min */
     {
       b=rtodbl(gcoeff(a,1,1));
       for(i=2;i<=n;i++)
	 if((c=rtodbl(gcoeff(a,i,i)))<b) b=c;
       borne=(long)(b+eps);
     }
  normax=0;
  if(stockmax) S=cgetg(stockmax+1,19);
  s=0;k=n;
  y[n]=z[n]=0;
  x[n]=(long)(sqrt(borne/v[n]+eps));
  do
    {
      do
	{
	  if(k>1)
	    {
	      k--;
	      z[k]=0;
	      for(j=k+1;j<=n;j++) z[k]=z[k]+q[k][j]*x[j];
	      p=x[k+1]+z[k+1];
	      y[k]=y[k+1]+p*p*v[k+1];
	      x[k]=(long)floor(sqrt((borne-y[k]+eps)/v[k])-z[k]);
	    }
	  while(v[k]*(x[k]+z[k])*(x[k]+z[k])>borne-y[k]+eps)
	    {k++;x[k]--;}
	}
      while(k>1);
      if(nonnul=(x[1]||(y[1]>eps)))
	{
	  norme=(long)(y[1]+v[1]*(x[1]+z[1])*(x[1]+z[1])+eps);
	  if(!flg&&(norme<borne)) {s=0; borne=norme;}
	  if(norme>normax) normax=norme;
	  s++;
	  if(s<=stockmax)
	    {
	      S[s]=lgetg(n+1,18);
	      for(i=1;i<=n;i++) coeff(S,i,s)=lstoi(x[i]);
	    }
	  x[k]--;
	}
    }
  while(nonnul);
  free(x);free(y);free(z);free(v);
  for(j=1;j<=n;j++) free(q[j]);free(q);
  if(stockmax) 
    {
      av1=avma;
      k=(s<stockmax)? s:stockmax;
      S1=cgetg(k+1,19);for(j=1;j<=k;j++) S1[j]=lmul(u,(GEN)S[j]);
      S=gerepile(av,av1,S1);
    }
  else {avma=av;S=cgetg(1,19);}
  u=cgetg(4,17);
  u[1]=lstoi(s<<1);
  u[2]=(flg)?lstoi(normax):lstoi(borne);
  u[3]=(long)S;
  return u;
}

long cgcd(long a,long b)
{
  long r;
  while(b) {r=a%b;a=b;b=r;}
  return a;
}

long ccontent(long* x,long n)
{
  long s,i;
  s=abs(x[1]);for(i=2;(i<=n)&&(s>1);i++) s=cgcd(abs(x[i]),s);
  return s;
}

GEN minimprim(GEN a, long borne, long stockmax)
/* pour usage dans buchall */
{
  GEN u,r,unr,S,S1,LN,pro;
  long n1=lg(a),n,av,av1,nonnul,i,j,k,s,norme,normax,*x;
  double p,b,c;
  double **q,*v,*y,*z;
  double eps=0.000001;

  n=n1-1;
  x=(long*)malloc(n1*sizeof(long));
  y=(double*)malloc(n1*sizeof(double));
  z=(double*)malloc(n1*sizeof(double));
  v=(double*)malloc(n1*sizeof(double));
  q=(double**)malloc(n1*sizeof(double*));
  for(j=1;j<=n;j++) q[j]=(double*)malloc(n1*sizeof(double));
  av=avma;
  affsr(1,unr=cgetr(6));
  u=lllgram(a,6);
  a=gmul(a,unr);a=gmul(gtrans(u),gmul(a,u));
  r=sqred1(a);
  for(j=1;j<=n;j++)
    {
      v[j]=rtodbl(gcoeff(r,j,j));
      for(i=1;i<j;i++)
	q[i][j]=rtodbl(gcoeff(r,i,j));
    }
  normax=0;
  if(stockmax) 
    {
      S=cgetg(stockmax+1,18);
      LN=cgetg(stockmax+1,17);
    }
  s=0;k=n;
  y[n]=z[n]=0;
  x[n]=(long)(sqrt(borne/v[n]+eps));
  do
    {
      do
	{
	  if(k>1)
	    {
	      k--;
	      z[k]=0;
	      for(j=k+1;j<=n;j++) z[k]=z[k]+q[k][j]*x[j];
	      p=x[k+1]+z[k+1];
	      y[k]=y[k+1]+p*p*v[k+1];
	      x[k]=(long)floor(sqrt((borne-y[k]+eps)/v[k])-z[k]);
	    }
	  while(v[k]*(x[k]+z[k])*(x[k]+z[k])>borne-y[k]+eps)
	    {k++;x[k]--;}
	}
      while(k>1);
      if(nonnul=(x[1]||(y[1]>eps)))
	{
	  norme=(long)(y[1]+v[1]*(x[1]+z[1])*(x[1]+z[1])+eps);
	  if(norme>normax) normax=norme;
	  if(ccontent(x,n)==1)
	    {
	      s++;
	      if(s<=stockmax)
		{
		  LN[s]=norme; /* sic, pas lstoi */
		  pro=cgetg(n+1,18);S[s]=(long)pro;
		  for(i=1;i<=n;i++) pro[i]=lstoi(x[i]);
		}
	    }
	  x[k]--;
	}
    }
  while(nonnul);
  free(x);free(y);free(z);free(v);
  for(j=1;j<=n;j++) free(q[j]);free(q);
  av1=avma;
  k=min(s,stockmax);
  S1=cgetg(k+1,18);
  for(j=1;j<=k;j++) 
    {
      pro=cgetg(3,17);S1[j]=(long)pro;
      pro[1]=lstoi(LN[j]);
      pro[2]=lmul(u,(GEN)S[j]);
    }
  S=gerepile(av,av1,S1);
  u=cgetg(4,17);
  u[1]=lstoi(s<<1);
  u[2]=lstoi(normax);
  u[3]=(long)S;
  return u;
}


long perf(GEN a)
/*  Perfection d'un reseau a (Gram), renvoie le 
    rang de la famille des matrices sym xx~ , x vec min de a */

{
  GEN m,u;
  long av,n,N,s,i,j,k,I;

  n=lg(a);N=n*(n-1)/2;
  av=avma;
  m=minim(a,0,5000);s=itos((GEN)m[1])/2;m=(GEN)m[3];
  u=cgetg(s+1,19);
  for(k=1;k<=s;k++)
    {
      u[k]=lgetg(N+1,17);
      for(I=1,i=1;i<n;i++)
	for(j=i;j<n;j++,I++) 
	  coeff(u,I,k)=lmulii(gcoeff(m,i,k),gcoeff(m,j,k));
    }
  s=rank(u);
  avma=av;return s;
}

GEN polymodrecip(GEN x)
{
  long v,i,j,n,av,tetpil,lx;
  GEN p1,p2,p3,p,phi,y,col;

  if(typ(x)!=9) err(polymoder1);
  p=(GEN)x[1];phi=(GEN)x[2];if(gcmp0(phi)) err(polymoder2);
  v=varn(p);n=lgef(p)-3;if(n<=0) return gcopy(x);
  if(n==1)
    {
      y=cgetg(3,9);p1=cgetg(4,10);y[1]=(long)p1;
      p1[1]=p[1];p1[2]=(typ(phi)==10) ? lneg((GEN)phi[2]) : lneg(phi);
      p1[3]=un;p1=cgetg(3,10);y[2]=(long)p1;
      if(gcmp0((GEN)p[2])) p1[1]=2;
      else
	{
	  p1[1]=p[1]-1;av=avma;p2=gdiv((GEN)p[2],(GEN)p[3]);
	  tetpil=avma;p1[2]=lpile(av,tetpil,gneg(p2));
	}
      setvarn(p1,v);
      return y;
    }
  else
    {
      av=avma;y=cgetg(n+1,19);p1=cgetg(n+1,18);
      y[1]=(long)p1;p1[1]=un;for(i=2;i<=n;i++) p1[i]=zero;
      p2=phi;
      for(j=2;j<=n;j++)
	{
	  lx=lgef(p2);p1=cgetg(n+1,18);y[j]=(long)p1;
	  for(i=1;i<=lx-2;i++) p1[i]=p2[i+1];
	  for(i=lx-1;i<=n;i++) p1[i]=zero;
	  if(j<n) p2=gmod(gmul(p2,phi),p);
	}
      col=cgetg(n+1,18);col[1]=zero;col[2]=un;
      for(i=3;i<=n;i++) col[i]=zero;
      p1=gauss(y,col);p2=gtopolyrev(p1,v);
      p3=caract(x,v);
      tetpil=avma;return gerepile(av,tetpil,gmodulcp(p2,p3));
    }
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~									~*/
/*~			         RANDOM					~*/
/*~									~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

int mymyrand()
{
/* BSD rand gives this: */
/*  pari_randseed=1103515245*pari_randseed+12345; */
#ifdef LONG_IS_32BIT
  pari_randseed=1000276549*pari_randseed+12347;
#endif
#ifdef LONG_IS_64BIT
  pari_randseed=1000000000000654397*pari_randseed+12347;
#endif
  if(pari_randseed<0) pari_randseed+=HIGHBIT;
  return pari_randseed;
}

GEN genrand(void)
{
  return stoi(mymyrand());
}

GEN setrand(int seed)
{
  pari_randseed=seed;return stoi(seed);
}

GEN getrand() {return stoi(pari_randseed);}

GEN getstack() {return stoi(top-avma);}

GEN gettime()  {return stoi(timer2());}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~									~*/
/*~			     PERMUTATIONS				~*/
/*~									~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

GEN permute(long n, GEN x)
{
  long av=avma,tetpil,i,a,r;
  GEN v,w,y;

  v=cgetg(2,17);v[1]=1;
  for(r=2;r<=n;r++)
    {
      x=dvmdis(x,r,&w);a=itos(w);
      w=cgetg(r+1,17);for(i=1;i<=a;i++) w[i]=v[i];
      w[a+1]=r;for(i=a+2;i<=r;i++) w[i]=v[i-1];
      v=w;
    }
  tetpil=av;y=cgetg(n+1,17);
  for(i=1;i<=n;i++) y[i]=lstoi(v[i]);
  return gerepile(av,tetpil,y);
}
