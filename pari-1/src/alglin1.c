/********************************************************************/
/********************************************************************/
/**                                                                **/
/**                  ++++++++++++++++++++++++++++++                **/
/**                  +                            +                **/
/**                  +     ALGEBRE LINEAIRE       +                **/
/**                  +                            +                **/
/**                  ++++++++++++++++++++++++++++++                **/
/**                                                                **/
/**                        (premiere partie)                       **/
/**                                                                **/
/**                       copyright Babe Cool                      **/
/**                                                                **/
/**                                                                **/
/********************************************************************/
/********************************************************************/

# include "genpari.h"

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*                                                                 */
/*                      TRANSPOSITION                              */
/*                                                                 */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

GEN     gtrans(GEN x)
{
  long    i,j,lx,tx,dx;
  GEN     y,p1;
  
  tx=typ(x);if(tx<17) err(gtraner);
  else
    switch(tx)
      {
      case 17: y=gcopy(x);settyp(y,18);break;
        
      case 18: y=gcopy(x);settyp(y,17);break;
        
      case 19: if((lx=lg(x))==1) return cgetg(1,19);
	dx=lg((GEN)x[1]);y=cgetg(dx,tx);
        for(i=1;i<dx;i++)
          {
            p1=cgetg(lx,18);y[i]=(long)p1;
            for(j=1;j<lx;j++)
              p1[j]=lcopy(gcoeff(x,i,j));
          }
        break;
        
      default: y=gcopy(x);break;
      }
  return y;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~                                                               ~*/
/*~                  CONCATENATION ET EXTRACTION                  ~*/
/*~                                                               ~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

GEN     concat(GEN x, GEN y)
{
  GEN  z,p1;
  long tx=typ(x),ty=typ(y),lx=lg(x),ly=lg(y),i,dx;
  
  if((tx==19)&&(lx==1)) 
    {
      if((ty!=17)||(ly==1)) return gtomat(y);
      else err(concater);
    }
  if((ty==19)&&(ly==1)) 
    {
      if((tx!=17)||(lx==1)) return gtomat(x);
      else err(concater);
    }
  if(tx<17)
    {
      if(ty<17)
        {
          z=cgetg(3,17);z[1]=lcopy(x);
          z[2]=lcopy(y);
        }
      else
        {
          if(ty!=19)
            {
              z=cgetg(ly+1,ty);z[1]=lcopy(x);
              for(i=2;i<=ly;i++)
                z[i]=lcopy((GEN)y[i-1]);
            }
          else
            {
              if(lg((GEN)y[1])!=2) err(concater);
              z=cgetg(ly+1,ty);p1=cgetg(2,18);
              z[1]=(long)p1;p1[1]=lcopy(x);
              for(i=2;i<=ly;i++)
                z[i]=lcopy((GEN)y[i-1]);
            }
        }
    }
  else
    {
      switch(tx)
        {
        case 17:
          if(ty<17)
            {
              z=cgetg(lx+1,tx);z[lx]=lcopy(y);
              for(i=1;i<lx;i++)
                z[i]=lcopy((GEN)x[i]);
            }
          else
            {
              switch(ty)
                {
                case 17: z=cgetg(lx+ly-1,tx);
                  for(i=1;i<lx;i++)
                    z[i]=lcopy((GEN)x[i]);
                  for(i=1;i<ly;i++)
                    z[lx+i-1]=lcopy((GEN)y[i]);
                  break;
                case 18:
                  if(lx<=2) z=(lx==1)?gcopy(y):concat((GEN)x[1],y);
                  else
                    {
                      if(ly>=3) err(concater);
                      z=(ly==1)?gcopy(x):concat(x,(GEN)y[1]);
                    }
                  break;
                case 19: if(lx!=ly) err(concater);
                  z=cgetg(ly,ty);
                  for(i=1;i<ly;i++)
                    z[i]=lconcat((GEN)x[i],(GEN)y[i]);
                  break;
                default:;
                }
            }
          break;
        case 18:
          if(ty<17)
            {
              z=cgetg(lx+1,tx);z[lx]=lcopy(y);
              for(i=1;i<lx;i++)
                z[i]=lcopy((GEN)x[i]);
            }
          else
            {
              switch(ty)
                {
                case 17:
                  if(lx<=2) z=(lx==1)?gcopy(y):concat((GEN)x[1],y);
                  else
                    {
                      if(ly>=3) err(concater);
                      z=(ly==1)?gcopy(x):concat(x,(GEN)y[1]);
                    }
                  break;
                case 18: z=cgetg(lx+ly-1,tx);
                  for(i=1;i<lx;i++)
                    z[i]=lcopy((GEN)x[i]);
                  for(i=1;i<ly;i++)
                    z[lx+i-1]=lcopy((GEN)y[i]);
                  break;
                case 19: if(lx!=lg((GEN)y[1])) err(concater);
                  z=cgetg(ly+1,ty);
                  z[1]=lcopy (x);
                  for(i=2;i<=ly;i++)
                    z[i]=lcopy((GEN)y[i-1]);
                  break;
                default:;
                }
            }
          break;
        case 19: dx=lg((GEN)x[1]);
          if(ty<17)
            {
              if(dx!=1) err(concater);
              z=cgetg(lx+1,tx);
              for(i=1;i<lx;i++)
                z[i]=lcopy((GEN)x[i]);
              p1=cgetg(2,18);z[lx]=(long)p1;
              p1[1]=lcopy(y);
            }
          else
            {
              switch(ty)
                {
                case 17: if(lx!=ly) err(concater);
                  z=cgetg(lx,tx);
                  for(i=1;i<lx;i++)
                    z[i]=lconcat((GEN)x[i],(GEN)y[i]);
                  break;
                case 18: if(dx!=ly) err(concater);
                  z=cgetg(lx+1,tx);
                  for(i=1;i<lx;i++)
                    z[i]=lcopy((GEN)x[i]);
                  z[lx]=lcopy(y);
                  break;
                case 19: if(dx!=lg((GEN)y[1])) err(concater);
                  z=cgetg(lx+ly-1,tx);
                  for(i=1;i<lx;i++)
                    z[i]=lcopy((GEN)x[i]);
                  for(i=1;i<ly;i++)
                    z[lx+i-1]=lcopy((GEN)y[i]);
                  break;
                default:;
                }
            }
          break;
        default:;
        }
    }
  return z;
}

GEN    vectextract(GEN x, GEN l)
     
              
     
     /* extraction des composantes de x suivants les bits du masque l      */
     /* a usage interne donc aucune verification n'est faite. voir extract */
     
{
  GEN  y;
  long i,tx=typ(x),lx=lg(x),av,tetpil,f;
  
  if(!signe(l)) return cgetg(1,tx);
  av=avma;i=1;
  while(!mpodd(l))
    {
      l=shifti(l,-1);i++;
    }
  if(i>=lx) err(extracter3);
  l=shifti(l,-1);tetpil=avma;
  y=cgetg(2,tx);y[1]=lcopy((GEN)x[i]);
  i++;
  while(!gcmp0(l)&&(i<lx))
    {
      f=mpodd(l);l=shifti(l,-1);tetpil=avma;
      if(f) y=concat(y,(GEN)x[i]);
      i++;
    }
  if(!gcmp0(l)) err(extracter3);
  y=gerepile(av,tetpil,y);
  return y;
}

GEN    extract(GEN x, GEN l)
{
  long tl=typ(l),ll,lx,i,tx=typ(x),in;
  GEN y;
  
  if(tx<17) err(extracter1);
  if(tl==1) return vectextract(x,l);
  if((tl==17)||(tl==18))
    {
      ll=lg(l);y=cgetg(ll,tx);lx=lg(x);
      for(i=1;i<ll;i++)
        {
          in=itos((GEN)l[i]);if((in>=lx)||(in<=0)) err(extracter3);
          y[i]=lcopy((GEN)x[in]);
        }
      return y;
    }
  err(extracter2);return gnil;
}

GEN    matextract(GEN x, GEN l1, GEN l2)
{
  GEN  y;
  long av,tetpil;
  
  if(typ(x)!=19) err(matextrer);
  av=avma;y=extract(gtrans(extract(x,l2)),l1);tetpil=avma;
  return gerepile(av,tetpil,gtrans(y));
}


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*                                                                 */
/*              OPERATIONS SCALAIRES-MATRICES                      */
/*                                                                 */
/*                        ET DIVERS                                */
/*                                                                 */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


GEN     gscalmat(GEN x, long n)       /* cree la matrice carree n X n */
     /* contenant x*I                */
               
               
     
{
  long    i,j,z;
  GEN     y;
  
  z = lcopy(x);
  y=cgetg(n+1,19);
  for(i=1;i<=n;i++)
    {
      y[i]=lgetg(n+1,18);
      for(j=1;j<=n;j++)
        coeff(y,j,i)=(i==j ? z : zero);
    }
  return y;
}

GEN     gscalsmat(long x, long n)      /* idem au precedent avec x long du C   */
     
                 
     
{
  long    i,j,z;
  GEN     y;
  
  z=lstoi(x);
  y=cgetg(n+1,19);
  for(i=1;i<=n;i++)
    {
      y[i]=lgetg(n+1,18);
      for(j=1;j<=n;j++)
        coeff(y,j,i)=(i==j ? z : zero);
    }
  return y;
}

GEN   idmat(long n)
{
  return gscalmat(gun,n);
}

GEN   gtomat(GEN x)
{
  GEN  y,p1;
  long tx=typ(x),lx,i;
  
  if(tx<17)
    {
      y=cgetg(2,19);p1=cgetg(2,18);y[1]=(long)p1;
      p1[1]=lcopy(x);
    }
  else
    switch(tx)
      {
      case 17: lx=lg(x);y=cgetg(lx,19);
        for(i=1;i<lx;i++)
          {
            p1=cgetg(2,18);p1[1]=lcopy((GEN)x[i]);
            y[i]=(long)p1;
          }
        break;
      case 18: y=cgetg(2,19);y[1]=lcopy(x);break;
      case 19: y=gcopy(x);break;
      }
  return y;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*                                                                 */
/*                   ADDITION SCALAIRE +  MATRICE                  */
/*                                                                 */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

GEN     gaddmat(GEN x, GEN y)        /* cree la matrice carree contenant x*I+y     */
     
                 
     
{
  long    ly,dy,i,j;
  GEN     z;
  
  ly=lg(y);dy=lg((GEN)y[1]);
  if((typ(y)!=19) || (ly!=dy)) err(gadmaer);
  z=cgetg(ly,19);
  for(i=1;i<ly;i++)
    {
      z[i]=lgetg(dy,18);
      for(j=1;j<dy;j++)
	coeff(z,j,i)=(i==j ? ladd(x,gcoeff(y,j,i)) : lcopy(gcoeff(y,j,i)));
    }
  return z;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*                                                                 */
/*                      ADDITION SHORT +  MATRICE                  */
/*                                                                 */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

GEN     gaddsmat(long s, GEN y)       /* idem au precedent avec x long du C   */
     
               
               
     
{
  long    ly,dy,i,j;
  GEN     z;
  
  ly=lg(y);dy=lg((GEN)y[1]);
  if((typ(y)!=19) || (ly!=dy)) err(gadsmaer);
  z=cgetg(ly,19);
  for(i=1;i<ly;i++)
    {
      z[i]=lgetg(dy,18);
      for(j=1;j<dy;j++)
	coeff(z,j,i)=(i==j ? laddsg(s,gcoeff(y,j,i)) : lcopy(gcoeff(y,j,i)));
    }
  return z;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*                                                                 */
/*                      RESOLUTION DE A X=B                        */
/*                      (METHODE DE GAUSS)                         */
/*                                                                 */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

GEN     gauss(GEN a, GEN b)
{
  long  nbli,nbco,i,j,k,av1,av2,av3,av4;
  GEN   aa,x,p,m,u;
  
  if(typ(b)==19) return invmulmat(a,b);
  nbco=lg(a)-1;nbli=lg((GEN)a[1])-1;
  if (nbco!=nbli) err(gausser1);
  x=cgetg(nbli+1,18);av1=avma;
  for (j=1;j<=nbco;j++) x[j]=b[j];
  aa=cgetg(nbco+1,19);
  for (j=1;j<=nbco;j++)
    {
      aa[j]=lgetg(nbli+1,18);
      for (i=1;i<=nbli;i++) coeff(aa,i,j)=coeff(a,i,j);
    }
  for (i=1;i<nbli;i++)
    {
      p=gcoeff(aa,i,i);k=i;
      if (gcmp0(p))
	{
	  for (k=i+1;(k<=nbli)&&gcmp0(gcoeff(aa,k,i));k++);
	  if (k>nbco) err(matinv1);
	  else
	    {
	      for (j=i;j<=nbco;j++)
		{
		  u=gcoeff(aa,i,j);coeff(aa,i,j)=coeff(aa,k,j);
		  coeff(aa,k,j)=(long)u;
		}
	      u=(GEN)x[i];x[i]=x[k];x[k]=(long)u;
	      p=gcoeff(aa,i,i);
	    }
	}
      for (k=i+1;k<=nbli;k++)
	{
	  m=gcoeff(aa,k,i);
	  if (!gcmp0(m))
	    {
	      m=gdiv(m,p);
	      for (j=i+1;j<=nbco;j++)
		coeff(aa,k,j)=lsub(gcoeff(aa,k,j),gmul(m ,gcoeff(aa,i,j)));
	      x[k]=lsub((GEN)x[k],gmul(m,(GEN)x[i]));
	    }
	}
    }
  
  /* Resolution systeme triangularise */
  av2=avma;
  p=gcoeff(aa,nbli,nbco);
  if (gcmp0(p)) err(matinv1);
  else
    {
      x[nbli]=ldiv((GEN)x[nbli],p);
      for (i=nbli-1;i>0;i--)
	{
	  av3=avma;m=(GEN)x[i];
	  for (j=i+1;j<=nbco;j++)
	    m=gsub(m,gmul(gcoeff(aa,i,j),(GEN)x[j]));
	  av4=avma;
	  x[i]=lpile(av3,av4,gdiv(m,gcoeff(aa,i,i)));
	}
    }
  gerepile(av1,av2,(GEN)1);
  return x;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*                                                                 */
/*            RANG D'UNE MATRICE m lignes x n colonnes             */
/*                                                                 */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

long rank(GEN x)
{
  GEN c,l,p0,p1;
  long i,j,k,r,t,n,m,av,av1,av2,lim;
  
  if (typ(x)!=19) err(kerer1);
  n=lg(x)-1;if(!n) return 0;
  m=lg((GEN)x[1])-1;av=avma;
  c=cgeti(m+1);l=cgeti(n+1);
  av1=avma;
  lim=(3*bot+avma)>>2;
  x=gcopy(x);
  for(k=1;k<=m;k++) c[k]=n;
  for(r=0,k=1;(r<m)&&(k<=n);k++)
    {
      for(j=1;j<k;j++) if(t=l[j])
	  {
	    p0=p1;p1=gcoeff(x,t,j);
	    for(i=1;i<=m;i++) if(c[i]>j)
	      {
		coeff(x,i,k)=lsub(gmul(p1,gcoeff(x,i,k)),gmul(gcoeff(x,i,j),gcoeff(x,t,k)));
		if(j>1) coeff(x,i,k)=ldiv(gcoeff(x,i,k),p0);
	      }
	  }
      for(i=1;(i<=m)&&((c[i]<k)||(gcmp0(gcoeff(x,i,k))));i++);
      if(i<=m) {r++;l[k]=i;c[i]=k;}
      else l[k]=0;
      if(avma<lim) {av2=avma;x=gerepile(av1,av2,gcopy(x));}
    }
  avma=av;return r;
}

GEN indexrank(GEN x)
{
  GEN c,d,mun,p,y,p1,p2;
  long i,j,k,r,t,n,m,av,tetpil;
  
  if (typ(x)!=19) err(kerer1);
  r=n=lg(x)-1;if(!r) {y=cgetg(3,17);y[1]=lgetg(1,17);y[2]=lgetg(1,17);return y;}
  m=lg((GEN)x[1])-1;av=avma;
  x=gcopy(x);c=cgeti(m+1);d=cgeti(n+1);
  mun=gneg(gun);
  for(k=1;k<=m;k++) c[k]=0;
  for(k=1;k<=n;k++)
    {
      j=1;
      while((j<=m)&&(c[j]||gcmp0(gcoeff(x,j,k)))) j++;
      if (j<=m)
	{
	  p=gdivsg(-1,gcoeff(x,j,k));
	  coeff(x,j,k)=(long)mun;
	  for(i=k+1;i<=n;i++) coeff(x,j,i)=lmul(p,gcoeff(x,j,i));
	  for(t=1;t<=m;t++)
	    if(t!=j)
	      {
		p=gcoeff(x,t,k);
		for(i=k+1;i<=n;i++) coeff(x,t,i)=ladd(gcoeff(x,t,i),gmul(p,gcoeff(x,j,i)));
		coeff(x,t,k)=zero;
	      }
	  c[j]=k;d[k]=j;
	}				  
      else {r--;d[k]=0;}
    }
  p1=cgetg(r+1,17);p2=cgetg(r+1,17);
  for(i=0,k=1;k<=n;k++) if(d[k]) {p1[++i]=lstoi(d[k]);p2[i]=lstoi(k);}
  tetpil=avma;y=cgetg(3,17);y[1]=(long)sort(p1);y[2]=lcopy(p2);
  return gerepile(av,tetpil,y);
}

/* Utilise autrfois dans buchxxx. Retourne le determinant du reseau engendre
par les colonnes de x */

GEN detint(GEN x)
{
  GEN pass,c,v,det1,piv,pivprec,vi,p,p1;
  long i,j,k,rg,t,n,n1,m,m1,nbmot,av,av1,tetpil,cm=0;
  
  nbmot=20;n=(n1=lg(x))-1;if(!n) return gun;
  m=(m1=lg((GEN)x[1]))-1;av=avma;
  pivprec=cgeti(nbmot);affsi(0,det1=cgeti(nbmot));affsi(1,piv=cgeti(nbmot));pass=cgetg(m1,19);
  for(j=1;j<=m;j++)
    {p=(GEN)(pass[j]=lgetg(m1,18));for(i=1;i<=m;i++) p[i]=lgeti(nbmot);}
  c=cgeti(m1);for(k=1;k<=m;k++) c[k]=0;
  v=cgetg(m1,18);av1=avma;k=1;rg=0;
  while((k<=n)&&(rg<m))
    {
      for(t=0,i=1;i<=m;i++)
	if(!c[i])
	  {
	    vi=mulii(piv,gcoeff(x,i,k));
	    for(j=1;j<=m;j++)
	      if(c[j]) vi=addii(vi,mulii(gcoeff(pass,i,j),gcoeff(x,j,k)));
	    v[i]=(long)vi;if(!t) if(signe(vi)) t=i;
	  }
      if (t)
	{
	  rg++;c[t]=k;affii(piv,pivprec);affii((GEN)v[t],piv);
	  if(rg<m)
	    {
	      for(i=1;i<=m;i++)
		if(!c[i])
		  for(j=1;j<=m;j++)
		    if(c[j]&&(j!=t))
		      {
			p1=subii(mulii(piv,gcoeff(pass,i,j)),mulii((GEN)v[i],gcoeff(pass,t,j)));
			if(rg>1) diviiz(p1,pivprec,gcoeff(pass,i,j));
			else affii(p1,gcoeff(pass,i,j));
		      }
	      for(i=1;i<=m;i++) if(!c[i]) mpnegz((GEN)v[i],gcoeff(pass,i,t));
	    }
	  else c[t]=0;
	}
      if(rg==m) 
	{cm=1;affii(ggcd(piv,det1),det1);affii(pivprec,piv);rg--;}
      avma=av1;k++;
    }
  if(rg+cm==m) {tetpil=avma;return gerepile(av,tetpil,absi(det1));}
  else {avma=av;return gzero;}
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*                                                                 */
/*          NOYAU D'UNE MATRICE m lignes x n colonnes              */
/*   ( Retourne une matrice de n-rang vecteurs independants )      */
/*                                                                 */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

GEN     keri(GEN x)     /* Programme pour types ENTIERS */
            
     
{
  GEN c,d,y,v,pp,p,p0,p1,q;
  long i,j,k,r,t,n,n1,m,nbmot,av,av1;
  
  if (typ(x)!=19) err(kerer1);
  n1=lg(x);n=n1-1;if(!n) return cgetg(1,19);
  m=lg((GEN)x[1])-1;av=avma;
  nbmot=200;
  c=cgetg(n1,19);
  for(j=1;j<=n;j++)
    {
      p=(GEN)(c[j]=lgetg(m+1,18));
      for(i=1;i<=m;i++)
	affii(gcoeff(x,i,j),(GEN)(p[i]=lgeti(nbmot)));
    }
  x=c;
  p=gun;
  pp=cgetg(n+1,18);
  for(j=1;j<=n;j++) pp[j]=lgeti(nbmot);
  c=cgeti(m+1);for(k=1;k<=m;k++) c[k]=0;
  d=cgeti(n1);
  av1=avma;
  for(r=0,k=1;k<=n;k++)
    {
      j=1;
      while((j<=m)&&(c[j]||!signe(gcoeff(x,j,k)))) j++;
      if (j<=m)
	{
	  p0=p;p=gcoeff(x,j,k);
	  for(t=1;t<=m;t++)
	    if(t!=j)
	      {
		q=gcoeff(x,t,k);
		for(i=k+1;i<=n;i++)
		  {
		    p1=subii(mulii(p,gcoeff(x,t,i)),mulii(q,gcoeff(x,j,i)));
		    if(k>1) diviiz(p1,p0,gcoeff(x,t,i));
		    else affii(p1,gcoeff(x,t,i));
		  }
	      }
	  c[j]=k;d[k]=j;
	  avma=av1;
	}		  
       else {r++;d[k]=0;affii(p,(GEN)pp[k]);}

    }
  if(r)    /* Il y a un noyau non nul */
    {
      av1=avma;
      y=cgetg(r+1,19);
      for(j=k=1;j<=r;j++,k++)
	{
	  while(d[k]) k++;
	  y[j]=(long)(v=cgetg(n1,18));
	  for(i=1;i<k;i++) v[i]= d[i]? lcopy(gcoeff(x,d[i],k)) : zero;
	  v[k]=lnegi((GEN)pp[k]);
	  for(i=k+1;i<=n;i++) v[i]=zero;
	}
      return gerepile(av,av1,y);
    }
  else {avma=av;y=cgetg(1,19);return y;}
}

GEN deplin(GEN x)
{
  long i,j,k,t,nbc,nbl,av;
  long *c,*l;
  GEN d,y,q;

  av=avma;
  x=gcopy(x);
  nbc=lg(x)-1;
  nbl=lg((GEN)x[1])-1;
  c=(long*)malloc((nbl+1)<<TWOPOTBYTES_IN_LONG);
  l=(long*)malloc((nbc+1)<<TWOPOTBYTES_IN_LONG);
  d=cgetg(nbl+1,17);
  for(i=1;i<=nbl;i++) d[i]=un;
  for(i=1;i<=nbl;i++) c[i]=0;
  k=1;t=1;
  while((t<=nbl)&&(k<=nbc))
    {
      for(j=1;j<k;j++)
	for(i=1;i<=nbl;i++)
	  if(i!=l[j])
	    coeff(x,i,k)=lsub(gmul((GEN)d[j],gcoeff(x,i,k)),gmul(gcoeff(x,i,j),gcoeff(x,l[j],k)));
      t=1;
      while((t<=nbl)&&(c[t]||gcmp0(gcoeff(x,t,k)))) t++;
      if (t<=nbl)
	{
	  d[k]=(long)coeff(x,t,k);
	  c[t]=k;l[k++]=t;
	}
    }
  if(k>nbc) 
    {
      free(c);free(d);
      avma=av;y=cgetg(nbc+1,18);for(j=1;j<=nbc;j++) y[j]=zero;return y;
    }
  else
    {
      y=cgetg(nbc+1,18);
      y[1]=(k>1) ? (long)coeff(x,l[1],k):un;
      for(q=gun,j=2;j<k;j++)
	{
	  q=gmul(q,(GEN)d[j-1]);
	  y[j]=lmul(gcoeff(x,l[j],k),q);
	}
      if(k>1) y[k]=lneg(gmul(q,(GEN)d[k-1]));
      for(j=k+1;j<=nbc;j++) y[j]=zero;
      free(c);free(l);
      d=content(y);
      t=avma;return gerepile(av,t,gdiv(y,d));
    }
}

GEN     ker(GEN x)     /* Programme pour types exacts */
     
{
  GEN c,d,y,mun,p;
  long i,j,k,r,t,n,n1,m,av,av1,av2;
  
  if (typ(x)!=19) err(kerer1);
  n1=lg(x);n=n1-1;if(!n) return cgetg(1,19);
  m=lg((GEN)x[1])-1;av=avma;x=gcopy(x);mun=gneg(gun);r=0;
  c=cgeti(m+1);for(k=1;k<=m;k++) c[k]=0;
  d=cgeti(n1);
  av1=avma;
  for(k=1;k<=n;k++)
    {
      j=1;
      while((j<=m)&&(c[j]||gcmp0(gcoeff(x,j,k)))) j++;
      if (j<=m)
	{
	  
	  p=gdivsg(-1,gcoeff(x,j,k));
	  coeff(x,j,k)=(long)mun;
	  for(i=k+1;i<=n;i++) coeff(x,j,i)=lmul(p,gcoeff(x,j,i));
	  for(t=1;t<=m;t++)
	    if(t!=j)
	      {
		p=gcoeff(x,t,k);
		for(i=k+1;i<=n;i++) coeff(x,t,i)=ladd(gcoeff(x,t,i),gmul(p,gcoeff(x,j,i)));
		coeff(x,t,k)=zero;
	      }
	  c[j]=k;d[k]=j;
	  av2=avma;
	  x=gerepile(av1,av2,gcopy(x));
	}		  
      else {r++;d[k]=0;}
    }
  if(r)
    {
      av1=avma;
      y=cgetg(r+1,19);
      for(j=k=1;j<=r;j++,k++)
	{
	  while(d[k]) k++;
	  y[j]=(long)(p=cgetg(n1,18));
	  for(i=1;i<k;i++) p[i]=d[i]? lcopy(gcoeff(x,d[i],k)):zero;
	  p[k]=un;
	  for(i=k+1;i<=n;i++) p[i]=zero;
	}
      return gerepile(av,av1,y);
    }
  else {avma=av;y=cgetg(1,19);return y;}
}

GEN     image(GEN x)     /* Programme pour types exacts */
{
  GEN c,d,y,mun,p,x1;
  long i,j,k,r,t,n,n1,m,av,av1,av2;
  
  if (typ(x)!=19) err(kerer1);
  n1=lg(x);n=n1-1;if(!n) return cgetg(1,19);
  m=lg((GEN)x[1])-1;av=avma;x1=gcopy(x);mun=gneg(gun);r=0;
  c=cgeti(m+1);for(k=1;k<=m;k++) c[k]=0;
  d=cgeti(n1);
  av1=avma;
  for(k=1;k<=n;k++)
    {
      j=1;
      while((j<=m)&&(c[j]||gcmp0(gcoeff(x1,j,k)))) j++;
      if (j<=m)
	{
	  p=gdivsg(-1,gcoeff(x1,j,k));
	  coeff(x1,j,k)=(long)mun;
	  for(i=k+1;i<=n;i++) coeff(x1,j,i)=lmul(p,gcoeff(x1,j,i));
	  for(t=1;t<=m;t++)
	    if(t!=j)
	      {
		p=gcoeff(x1,t,k);
		for(i=k+1;i<=n;i++) coeff(x1,t,i)=ladd(gcoeff(x1,t,i),gmul(p,gcoeff(x1,j,i)));
		coeff(x1,t,k)=zero;
	      }
	  c[j]=k;d[k]=j;
	  av2=avma;
	  x1=gerepile(av1,av2,gcopy(x1));
	}		  
      else {r++;d[k]=0;}
    }
  if(r)
    {
      av1=avma;
      y=cgetg(n-r+1,19);
      for(j=k=1;j<=n-r;j++,k++)
	{
	  while(!d[k]) k++;
	  y[j]=lcopy((GEN)x[k]);
	}
      return gerepile(av,av1,y);
    }
  else {avma=av;return gcopy(x);}
}

GEN     imagereel(GEN x, long prec)     /* Programme pour types inexacts */
{
  GEN c,d,y,mun,p,x1,eps;
  long i,j,k,r,t,n,n1,m,av,av1,av2;
  
  if (typ(x)!=19) err(imagerer);
  n1=lg(x);n=n1-1;if(!n) return cgetg(1,19);
  m=lg((GEN)x[1])-1;av=avma;
  eps=cgetr(3);eps[2]=HIGHBIT;
  eps[1]=evalsigne(1)+HIGHEXPOBIT+16-((prec-2)<<(TWOPOTBITS_IN_LONG-1));
  x1=gcopy(x);mun=gneg(gun);r=0;
  c=cgeti(m+1);for(k=1;k<=m;k++) c[k]=0;
  d=cgeti(n1);
  av1=avma;
  for(k=1;k<=n;k++)
    {
      j=1;
      while((j<=m)&&(c[j]||(gcmp(gabs(gcoeff(x1,j,k),5),eps)<0))) j++;
      if (j<=m)
	{
	  p=gdivsg(-1,gcoeff(x1,j,k));
	  coeff(x1,j,k)=(long)mun;
	  for(i=k+1;i<=n;i++) coeff(x1,j,i)=lmul(p,gcoeff(x1,j,i));
	  for(t=1;t<=m;t++)
	    if(t!=j)
	      {
		p=gcoeff(x1,t,k);
		for(i=k+1;i<=n;i++) coeff(x1,t,i)=ladd(gcoeff(x1,t,i),gmul(p,gcoeff(x1,j,i)));
		coeff(x1,t,k)=zero;
	      }
	  c[j]=k;d[k]=j;
	  av2=avma;
	  x1=gerepile(av1,av2,gcopy(x1));
	}		  
      else {r++;d[k]=0;}
    }
  if(r)
    {
      av1=avma;
      y=cgetg(n-r+1,19);
      for(j=k=1;j<=n-r;j++,k++)
	{
	  while(!d[k]) k++;
	  y[j]=lcopy((GEN)x[k]);
	}
      return gerepile(av,av1,y);
    }
  else {avma=av;return gcopy(x);}
}

GEN imagecompl(GEN x)     /* Programme pour types exacts */
{
  GEN c,d,y,mun,p,x1;
  long i,j,k,r,t,n,n1,m,av,av1,av2;
  
  if (typ(x)!=19) err(kerer1);
  n1=lg(x);n=n1-1;if(!n) return cgetg(1,19);
  m=lg((GEN)x[1])-1;av=avma;x1=gcopy(x);mun=gneg(gun);r=0;
  c=cgeti(m+1);for(k=1;k<=m;k++) c[k]=0;
  d=cgeti(n1);
  av1=avma;
  for(k=1;k<=n;k++)
    {
      j=1;
      while((j<=m)&&(c[j]||gcmp0(gcoeff(x1,j,k)))) j++;
      if (j<=m)
	{
	  
	  p=gdivsg(-1,gcoeff(x1,j,k));
	  coeff(x1,j,k)=(long)mun;
	  for(i=k+1;i<=n;i++) coeff(x1,j,i)=lmul(p,gcoeff(x1,j,i));
	  for(t=1;t<=m;t++)
	    if(t!=j)
	      {
		p=gcoeff(x1,t,k);
		for(i=k+1;i<=n;i++) coeff(x1,t,i)=ladd(gcoeff(x1,t,i),gmul(p,gcoeff(x1,j,i)));
		coeff(x1,t,k)=zero;
	      }
	  c[j]=k;d[k]=j;
	  av2=avma;
	  x1=gerepile(av1,av2,gcopy(x1));
	}		  
      else {r++;d[k]=0;}
    }
  av1=avma;y=cgetg(r+1,17);
  for(j=k=1;j<=r;j++,k++)
    {
      while(d[k]) k++;
      y[j]=lstoi(k);
    }
  return gerepile(av,av1,y);
}


GEN sinverseimage(GEN mat, GEN y)
{
  long av=avma,nbcol,i,j,l,tetpil;
  GEN met,noyau,lastcoeff,invimag;

  if ((typ(mat)!=19)||(typ(y)!=18)) err(kerer1);
  nbcol=lg(mat);
  met=cgetg(nbcol+1,19);
  for(j=1;j<=nbcol-1;j++) met[j]=mat[j];
  met[nbcol]=(long)y;
  noyau=ker(met);l=lg(noyau)-1;
  if(!l) {avma=av;return cgetg(1,18);}
  lastcoeff=gneg(gcoeff(noyau,nbcol,l));
  if(gcmp0(lastcoeff)) {avma=av;return cgetg(1,18);}
  tetpil=avma;
  invimag=cgetg(nbcol,18);
  for(i=1;i<=nbcol-1;i++) invimag[i]=ldiv(gcoeff(noyau,i,l),lastcoeff);
  return gerepile(av,tetpil,invimag);
}

GEN inverseimage(GEN m,GEN v)
/* Calcule l'image reciproque de v par m en utilisant sinverseimage */
{
   long av=avma,tetpil,j,lv,tv=typ(v);
   GEN mat;

   if(tv==18) return sinverseimage(m,v);
   if(tv<=17) err(kerer1);
   lv=lg(v)-1;
   mat=cgetg(lv+1,19);
   for(j=1;j<=lv;j++)
     mat[j]=(long)sinverseimage(m,(GEN)v[j]);
   tetpil=avma;return gerepile(av,tetpil,gcopy(mat));
}

GEN     kerreel(GEN x, long prec)    /* Programme pour types non exacts    */
     /* gestion de pile a la fin seulement */
            
               
{
  GEN c,d,y,mun,p,eps;
  long i,j,k,r,t,n,n1,m,av,av1;
  
  if (typ(x)!=19) err(kerer1);
  n1=lg(x);n=n1-1;if(!n) return cgetg(1,19);
  m=lg((GEN)x[1])-1;av=avma;
  eps=cgetr(3);eps[2]=HIGHBIT;eps[1]=evalsigne(1)+HIGHEXPOBIT+16-((prec-2)<<(TWOPOTBITS_IN_LONG-1));
  x=gcopy(x);
  mun=gneg(gun);r=0;
  c=cgeti(m+1);for(k=1;k<=m;k++) c[k]=0;
  d=cgeti(n1);
  for(k=1;k<=n;k++)
    {
      j=1;
      while((j<=m)&&(c[j]||(gcmp(gabs(gcoeff(x,j,k),5),eps)<0))) j++;
      if (j<=m)
	{
	  
	  p=gdivsg(-1,gcoeff(x,j,k));
	  coeff(x,j,k)=(long)mun;
	  for(i=k+1;i<=n;i++) coeff(x,j,i)=lmul(p,gcoeff(x,j,i));
	  for(t=1;t<=m;t++)
	    if(t!=j)
	      {
		p=gcoeff(x,t,k);
		for(i=k+1;i<=n;i++) coeff(x,t,i)=ladd(gcoeff(x,t,i),gmul(p,gcoeff(x,j,i)));
		coeff(x,t,k)=zero;
	      }
	  c[j]=k;d[k]=j;
	}				  
      else{r++;d[k]=0;}
    }
  if(r)
    {
      av1=avma;
      y=cgetg(r+1,19);
      for(j=k=1;j<=r;j++,k++)
	{
	  while(d[k]) k++;
	  y[j]=(long)(p=cgetg(n1,18));
	  for(i=1;i<k;i++) p[i]=d[i]? lcopy(gcoeff(x,d[i],k)):zero;
	  p[k]=un;
	  for(i=k+1;i<=n;i++) p[i]=zero;
	}
      return gerepile(av,av1,y);
    }
  else {avma=av;y=cgetg(1,19);return y;}
}

/* Etant donnee une matrice nxk de rang k<=n, on trouve une matrice nxn
inversible dont les k premieres colonnes forment la matrice initiale;
on ne verifie pas que les k colonnes sont lineairement independantes. */

GEN suppl(GEN x)
{
  long av=avma,tetpil,k,n,s,t;
  GEN y,p1,p2;

  if(typ(x)!=19) err(kerer1);
  k=lg(x)-1;if(!k) err(suppler1);
  n=lg((GEN)x[1])-1;if(k>n) err(suppler2);
  s=0;y=idmat(n);
  while(s<k)
    {
      s++;p1=gauss(y,(GEN)x[s]);t=s;
      while((t<=n)&&gcmp0((GEN)p1[t])) t++;
      if(t>n) err(suppler2);
      p2=(GEN)y[s];y[s]=x[s];if(s!=t) y[t]=(long)p2;
    }
  tetpil=avma;return gerepile(av,tetpil,gcopy(y));
}

GEN image2(GEN x)
{
  long av=avma,tetpil,k,n,i;
  GEN p1,p2;

  if(typ(x)!=19) err(kerer1);
  k=lg(x)-1;if(!k) return gcopy(x);
  n=lg((GEN)x[1])-1;p1=ker(x);k=lg(p1)-1;
  if(k) p1=suppl(p1);else p1=idmat(n);
  n=lg(p1)-1;
  tetpil=avma;p2=cgetg(n-k+1,19);
  for(i=k+1;i<=n;i++) p2[i-k]=lmul(x,(GEN)p1[i]);
  return gerepile(av,tetpil,p2);
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*                                                                 */
/*                         VECTEURS PROPRES                        */
/*            (matrice de vecteurs propres independants            */
/*             classes par valeurs propres croissantes )           */
/*                                                                 */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

GEN eigen(GEN x, long prec)
{
  GEN y,z,rr,p,ssesp,eps,r1,r2,r3;
  long j,k,n,ly,av,av1,nbrac,nk,flag;
  
  
  n=lg(x);
  av=avma;
  eps=cgetr(3);eps[2]=HIGHBIT;eps[1]=evalsigne(1)+HIGHEXPOBIT+16-((prec-2)<<TWOPOTBITS_IN_LONG);
  y=cgetg(n,19);ly=1;
  z=gcopy(x);
  p=caradj(x,0,0);rr=roots(p,prec);nbrac=lg(rr)-1;
/* Bien sur ce n'est pas comme cela qu'on doit calculer les valeurs propres !*/
  for(k=1;k<=nbrac;k++)
    {
      r2=(GEN)rr[k];flag=0;
      if(k>1) if(gcmp(gabs(gsub(r1,r2),5),eps)>0) flag=1;
      if(flag||(k==1))
	{
	  r3=ground(r2);if(gcmp(gabs(gsub(r2,r3),5),eps)<0) r2=r3;
	  {
	    for(j=1;j<n;j++) coeff(z,j,j)=lsub(gcoeff(x,j,j),r2);
	    ssesp=kerreel(z,prec);
	    nk=lg(ssesp)-1;
	    for(j=1;j<=nk;j++,ly++) y[ly]=ssesp[j];
	  }
	  r1=r2;
	}
    }
  z=cgetg(ly,19);
  av1=avma;
  for(k=1;k<ly;k++) z[k]=y[k];
  return gerepile(av,av1,gcopy(z));
}
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*                                                                 */
/*                           DETERMINANT                           */
/*                                                                 */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/* ===================================================================*/
/*       Determinant types exacts : 1er pivot non nul                 */
/*--------------------------------------------------------------------*/

GEN     det2(GEN a)
{
  long  u,nbli,nbco,i,j,k,av,av1,s;
  GEN   aa,p1,x,p,m;
  
  if (typ(a)!=19) err(mattype1);
  nbco=lg(a)-1;if(!nbco) return gun;
  nbli=lg((GEN)a[1])-1;
  if (nbco!=nbli) err(mattype1);
  av=avma;x=gun;s=1;
  aa=cgetg(nbco+1,19);
  
  for (j=1;j<=nbco;j++)
    {
      aa[j]=lgetg(nbli+1,18);
      for (i=1;i<=nbli;i++) coeff(aa,i,j)=coeff(a,i,j);
    }
  
  for (i=1;i<nbco;i++)
    {
      p=gcoeff(aa,i,i);k=i;
      if(gcmp0(p))
	{
	  
	  for (k=i+1;(k<=nbco)&&gcmp0(gcoeff(aa,i,k));k++);
	  if (k>nbco)
	    {
	      avma=av;return gzero;
	    }
	  else
	    {
	      p=gcoeff(aa,i,k);
	      u=aa[k];aa[k]=aa[i];aa[i]=u;
	      s= -s;
	    }
	}
      x=gmul(x,p);
      
      for (k=i+1;k<=nbco;k++)
	{
	  m=gcoeff(aa,i,k);
	  if (!gcmp0(m))
	    {
	      m=gdiv(m,p);
	      for (j=i+1;j<=nbli;j++)
		{
		  p1=gmul(m,gcoeff(aa,j,i));
		  coeff(aa,j,k)=lsub(gcoeff(aa,j,k),p1);
		}
	    }
	}
    }
  if(s<0) x=gneg(x);
  av1=avma;
  return gerepile(av,av1,gmul(x,gcoeff(aa,nbli,nbco)));
}

/* ===================================================================*/
/*     Determinant dans un anneau A : Tous les calculs dans A         */
/*     division par le pivot precedent ( methode de Bareiss)          */
/*--------------------------------------------------------------------*/

GEN     det(GEN a)
{
  long  u,nbli,nbco,i,j,k,av,av1,s;
  GEN   aa,p1,p,m,pprec;
  
  
  if (typ(a)!=19) err(mattype1);
  nbco=lg(a)-1;if(!nbco) return gun;
  nbli=lg((GEN)a[1])-1;
  if (nbco!=nbli) err(mattype1);
  av=avma;
  aa=cgetg(nbco+1,19);
  
  for (j=1;j<=nbco;j++)
    {
      aa[j]=lgetg(nbli+1,18);
      for (i=1;i<=nbli;i++) coeff(aa,i,j)=coeff(a,i,j);
    }
  pprec=gun;s=1;
  for (i=1;i<nbco;i++)
    {
      p=gcoeff(aa,i,i);k=i;
      if(gcmp0(p))
        {
          
          for (k=i+1;(k<=nbco)&&gcmp0(gcoeff(aa,i,k));k++);
          if (k>nbco)
            {
              avma=av;return gzero;
            }
          else
            {
              p=gcoeff(aa,i,k);
              u=aa[k];aa[k]=aa[i];aa[i]=u;
              s= -s;
            }
        }
      for (k=i+1;k<=nbco;k++)
        {
          m=gcoeff(aa,i,k);
          for (j=i+1;j<=nbli;j++)
            {
              p1=gsub(gmul(p,gcoeff(aa,j,k)),gmul(m,gcoeff(aa,j,i)));
	      if((typ(p1)==10)&&(typ(pprec)==10)&&(varn(p1)==varn(pprec)))
		coeff(aa,j,k)=ldeuc(p1,pprec);
              else coeff(aa,j,k)=ldiv(p1,pprec);
            }
        }
      pprec=p;
    }
  av1=avma;
  return (s>0) ? gerepile(av,av1,gcopy(gcoeff(aa,nbli,nbco))) : gerepile(av,av1,gneg(gcoeff(aa,nbli,nbco)));
}

/* ===================================================================*/
/*              Determinant reel : pivot maximal                      */
/*--------------------------------------------------------------------*/


GEN     detreel(GEN a)
{
  long  u,nbli,nbco,i,j,k,av,av1,s;
  GEN   aa,p1,x,p,m;
  
  if (typ(a)!=19) err(mattype1);
  nbco=lg(a)-1;if(!nbco) return gun;
  nbli=lg((GEN)a[1])-1;
  if (nbco!=nbli) err(mattype1);
  av=avma;s=1;x=gun;
  aa=cgetg(nbco+1,19);
  
  for (j=1;j<=nbco;j++)
    {
      aa[j]=lgetg(nbli+1,18);
      for (i=1;i<=nbli;i++) coeff(aa,i,j)=coeff(a,i,j);
    }
  
  for (i=1;i<nbco;i++)
    {
      p=gabs(gcoeff(aa,i,i));k=i;
      for(j=i+1;j<=nbco;j++)
	if(gcmp(p1=gabs(gcoeff(aa,i,j),3),p)>0) {p=p1;k=j;}
      if(gcmp0(p))
	{
	  av1=avma;return gerepile(av,av1,gcopy(p));
	}
      else
	{
	  p=gcoeff(aa,i,k);
	  if(k>i)
	    {
	      u=aa[k];aa[k]=aa[i];aa[i]=u;
	      s= -s;
	    }
	}
      x=gmul(x,p);
      
      for (k=i+1;k<=nbco;k++)
	{
	  m=gcoeff(aa,i,k);
	  if (!gcmp0(m))
	    {
	      m=gdiv(m,p);
	      for (j=i+1;j<=nbli;j++)
		coeff(aa,j,k)=lsub(gcoeff(aa,j,k),gmul(m,gcoeff(aa,j,i)));
	    }
	}
    }
  if(s<0) x=gneg(x);
  av1=avma;return gerepile(av,av1,gmul(x,gcoeff(aa,nbli,nbco)));
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*                                                                 */
/*                      HNF   SPECIAL                              */
/*                                                                 */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/* On se donne une matrice mxn mat de long, une matrice matc rxn de GEN (sous
forme de pointeur ptmatc), un vecteur vec et un entier k0<=m.
On suppose que les k0 premieres lignes de mat sont 
(peut-etre) denses, mais que les suivantes sont creuses. On ressort avec 
une matrice matgen contenant la partie non-archimedienne gauche, matalpha
contenant la partie droite, matc est modifiee, vec est permute. La 
permutation est contenue dans v, et v[i]=i pour i<=k0.

A usage interne. Pas de verifications */

int compte(long **mat, long row, long longueur, long *firstnonzero)
{
  int n,j;
  long p;

  n=0;
  for (j=1;j<=longueur;j++)
    {
      p=mat[j][row];
      if(p) {if(labs(p)>=2) return (MAXHALFULONG>>1);else {n++;*firstnonzero=j;}}
    }
  return n;
}

int compte2(long **mat, long row, long longueur, long *firstone)
{
  int n=0,j;
  long p;

  *firstone=0;
  for(j=1;j<=longueur;j++)
    {
      p=labs(mat[j][row]);
      if(p) 
	{
	  n++;if(p==1) *firstone=j;
	}
    }
  return n;
}

GEN hnfspec(long** mat, GEN* ptpdep, GEN* ptmatc,long* vperm,GEN* ptmatalpha,long co,long li,long k0,long* ptnlze,long* ptcol)
{
  long av=avma,av2,tetpil,*p,i,i0,i1,j,j1,j2,k,fl,lk0,col,lig,*ww,*permpro;
  long nb,n,s,s1,t,limt,dec,sizemax,lim,nlze,lnz,colnew,lp3;
  GEN p1,p2,p3,p4,matgen,vmax,matu,matc,matid;
  GEN u1u2,u1,u2,wpro;
  GEN matalpha,matalphanew,wpronew,matcnew,pdep;

  matgen=cgetg(co+1,19);
  for(j=1;j<=co;j++)
    {
      p1=cgetg(li+1,18);matgen[j]=(long)p1;
      for(i=1;i<=li;i++) p1[i]=lstoi(mat[j][vperm[i]]);
    }
  if(DEBUGLEVEL>2)
    {
      fprintf(errfile,"avant hnfspec :\n");
      fprintf(errfile,"matgen =\n");
      output(matgen);
    }
  matu=cgetg(co+1,19);
  for(j=1;j<=co;j++)
    {
      p1=cgetg(k0+1,18);matu[j]=(long)p1;
      for(i=1;i<=k0;i++) p1[i]=lstoi(mat[j][vperm[i]]);
    }
  vmax=cgeti(co+1);
  av2=avma;lim=(avma+bot)>>1;
  lig=li;col=co;fl=1;lk0=k0;matid=idmat(co);
  while((lig>lk0)&&fl)
    {
      for(nb=(MAXHALFULONG>>1),i=lig;(i>lk0)&&((nb=compte(mat,vperm[i],col,&n))>1);i--);
      if(!nb) {lk0++;if(i>lk0) {s=vperm[i];vperm[i]=vperm[lk0];vperm[lk0]=s;}}
      else
	{
	  if(nb==1)
	    {
	      p=mat[col];mat[col]=mat[n];mat[n]=p;
	      p1=(GEN)matid[col];matid[col]=matid[n];matid[n]=(long)p1;
	      if(mat[col][vperm[i]]<0)
		{
		  for(k=k0+1;k<=li;k++) mat[col][vperm[k]]= -mat[col][vperm[k]];
		  matid[col]=lneg((GEN)matid[col]);
		}
	      if(i<lig) {s=vperm[i];vperm[i]=vperm[lig];vperm[lig]=s;}
	      lig--;col--;
	    }
	  else fl=0;
	}
    }
  if(avma<lim) {tetpil=avma;matid=gerepile(av2,tetpil,gcopy(matid));}
  matgen=cgetg(co+1,19);
  for(j=1;j<=co;j++)
    {
      p1=cgetg(li-k0+1,18);matgen[j]=(long)p1;
      for(i=k0+1;i<=li;i++) p1[i-k0]=lstoi(mat[j][vperm[i]]);
    }
  if(DEBUGLEVEL>3)
    {
      fprintf(errfile,"apres phase1 :\n");
      fprintf(errfile,"matgen =\n");
      output(matgen);
    }
  fl=1;sizemax=0;
  while((lig>lk0)&&fl&&(sizemax<=((long)MAXULONG>>2)))
    {
      for(nb=(MAXHALFULONG>>1),i=lig;(i>lk0)&&((nb=compte(mat,vperm[i],col,&n))==(MAXHALFULONG>>1));i--);
      if(!nb) {lk0++;if(i>lk0) {s=vperm[i];vperm[i]=vperm[lk0];vperm[lk0]=s;}}
      else
	{
	  if(nb==(MAXHALFULONG>>1)) fl=0;
	  else
	    {
	      p=mat[col];mat[col]=mat[n];mat[n]=p;
	      p1=(GEN)matid[col];matid[col]=matid[n];matid[n]=(long)p1;
	      if(mat[col][vperm[i]]<0)
		{
		  for(k=k0+1;k<=li;k++) mat[col][vperm[k]]= -mat[col][vperm[k]];
		  matid[col]=lneg((GEN)matid[col]);
		}
	      if(i<lig) {s=vperm[i];vperm[i]=vperm[lig];vperm[lig]=s;}
	      if(nb>1)
		{
		  for(j=1;j<n;j++)
		    {
		      if(t=mat[j][vperm[lig]])
			{
			  if(t==1)
			    {
			      for(i=k0+1;i<=li;i++)
				{
				  mat[j][vperm[i]]-=mat[col][vperm[i]];
				  sizemax=max(sizemax,labs(mat[j][vperm[i]]));
				}
			      matid[j]=lsub((GEN)matid[j],(GEN)matid[col]);
			    }
			  else
			    {
			      for(i=k0+1;i<=li;i++)
				{
				  mat[j][vperm[i]]+=mat[col][vperm[i]];
				  sizemax=max(sizemax,labs(mat[j][vperm[i]]));
				}
			      matid[j]=ladd((GEN)matid[j],(GEN)matid[col]);
			    }
			}
		    }
		}
	      lig--;col--;
	    }
	}
      if(avma<lim) {tetpil=avma;matid=gerepile(av2,tetpil,gcopy(matid));}
    }
  for(j=1;j<=co;j++)
    {
      s=0;for(i=k0+1;i<=li;i++) s=max(s,labs(mat[j][i]));
      vmax[j]=s;
    }
  fl=1;
  while((lig>lk0)&&fl)
    {
      for(i=lig;i>lk0;i--)
	{
	  nb=compte2(mat,vperm[i],col,&n);
	  if(n||(!nb)) break;
	}
      if(!nb) {lk0++;if(i>lk0) {s=vperm[i];vperm[i]=vperm[lk0];vperm[lk0]=s;}}
      else
	{
	  if(!n) fl=0;
	  else
	    {
	      p=mat[col];mat[col]=mat[n];mat[n]=p;
	      s=vmax[col];vmax[col]=vmax[n];vmax[n]=s;
	      p1=(GEN)matid[col];matid[col]=matid[n];matid[n]=(long)p1;
	      if(mat[col][vperm[i]]<0)
		{
		  for(k=k0+1;k<=li;k++) mat[col][vperm[k]]= -mat[col][vperm[k]];
		  matid[col]=lneg((GEN)matid[col]);
		}
	      if(i<lig) {s=vperm[i];vperm[i]=vperm[lig];vperm[lig]=s;}
	      for(j=1;j<col;j++)
		{
		  if(vmax[col]) limt=(((ulong)MAXULONG>>1)-vmax[j])/vmax[col];
		  if(t=mat[j][vperm[lig]])
		    {
		      if((!vmax[col])||(labs(t)<=limt))
			{
			  s=0;
			  for(i=k0+1;i<=li;i++)
			    {
			      s1=mat[j][vperm[i]]-=t*mat[col][vperm[i]];
			      s=max(s,labs(s1));
			    }
			  vmax[j]=s;
			  matid[j]=lsub((GEN)matid[j],gmulsg(t,(GEN)matid[col]));
			}
		      else fl=0;
		    }
		}
	      if(fl) {lig--;col--;}
	    }
	}
      if(avma<lim) {tetpil=avma;matid=gerepile(av2,tetpil,gcopy(matid));}
    }
  matgen=cgetg(co+1,19);
  for(j=1;j<=co;j++)
    {
      p1=cgetg(li-k0+1,18);matgen[j]=(long)p1;
      for(i=k0+1;i<=li;i++) p1[i-k0]=lstoi(mat[j][vperm[i]]);
    }
  if(DEBUGLEVEL>3) 
    {
      fprintf(errfile,"apres phase2 :\n");
      fprintf(errfile,"matgen =\n");
      output(matgen);
    }
  for(i=li-1;i>lig;i--)
    {
      p2=(GEN)matgen[i+co-li];
      for(j=i+co-li+1;j<=co;j++)
	{
	  p1=(GEN)matgen[j];p3=(GEN)p1[i-k0];
	  if(s=signe(p3))
	    {
	      i0=i-k0;p1[i0]=zero;
	      if(gcmp(absi(p3),gun)>0)
		{
		  for(i1=1;i1<i0;i1++) p1[i1]=lsub((GEN)p1[i1],gmul(p3,(GEN)p2[i1]));
		  matid[j]=lsub((GEN)matid[j],gmul(p3,(GEN)matid[i+co-li]));
		}
	      else
		{
		  if(s>0)
		    {
		      for(i1=1;i1<i0;i1++) p1[i1]=lsub((GEN)p1[i1],(GEN)p2[i1]);
		      matid[j]=lsub((GEN)matid[j],(GEN)matid[i+co-li]);
		    }
		  else
		    {
		      for(i1=1;i1<i0;i1++) p1[i1]=ladd((GEN)p1[i1],(GEN)p2[i1]);
		      matid[j]=ladd((GEN)matid[j],(GEN)matid[i+co-li]);
		    }
		}
	    }
	}
      if(avma<lim) 
	{
	  tetpil=avma;matid=gcopy(matid);matgen=gcopy(matgen);
	  dec=lpile(av2,tetpil,0)>>TWOPOTBYTES_IN_LONG;
	  matid+=dec;matgen+=dec;
	}
    }
  if(DEBUGLEVEL>3) 
    {
      fprintf(errfile,"apres nettoyage identite :\n");
      fprintf(errfile,"matgen =\n");
      output(matgen);
    }
  nlze=lk0-k0;lnz=lig-nlze;
  av2=avma;p1=cgetg(li+1,17);
  for(i=1;i<=nlze;i++) p1[i]=lstoi(vperm[i+k0]);
  for(i=1;i<=k0;i++) p1[i+nlze]=lstoi(vperm[i]);
  for(i=1;i<=lk0;i++) vperm[i]=itos((GEN)(p1[i]));
  avma=av2;
  matu=gmul(matu,matid);matc=gmul(*ptmatc,matid);
  p1=cgetg(col+1,19);
  for(j=1;j<=col;j++)
    {
      p2=cgetg(lnz+1,18);p1[j]=(long)p2;
      for(i=1;i<=k0;i++) p2[i]=coeff(matu,i,j);
      for(i=k0+1;i<=lnz;i++) p2[i]=coeff(matgen,i+nlze-k0,j);
    }
  p3=imagecompl(gtrans(p1));lp3=lg(p3)-1;
  if(!(ww=(long*)malloc((lnz+1)*sizeof(long)))) err(amemer);
  for(i=1;i<=lnz;i++) ww[i]=1;
  if(!(permpro=(long*)malloc((lnz+1)*sizeof(long)))) err(amemer);
  for(i=1;i<=lp3;i++) {permpro[i]=itos((GEN)(p3[i]));ww[permpro[i]]=0;}
  for(j=1;j<=lnz;j++) {if(ww[j]){permpro[i]=j;i++;}}
  free(ww);
  p4=cgetg(col+1,19);
  for(j=1;j<=col;j++)
    {
      p2=cgetg(lnz+1-lp3,18);p4[j]=(long)p2;
      for(i=lp3+1;i<=lnz;i++) p2[i-lp3]=coeff(p1,permpro[i],j);
    }
  pdep=cgetg(col+1,19);
  for(j=1;j<=col;j++)
    {
      p2=cgetg(nlze+lp3+1,18);pdep[j]=(long)p2;
      for(i=1;i<=nlze;i++) p2[i]=zero;
      for(;i<=nlze+lp3;i++) p2[i]=coeff(p1,permpro[i-nlze],j);
    }
  matalpha=cgetg(co-col+1,19);
  for(j=col+1;j<=co;j++)
    {
      p2=cgetg(lig+1,18);matalpha[j-col]=(long)p2;
      for(i=1;i<=nlze;i++) p2[i]=coeff(matgen,i,j);
      for(i=1;i<=lnz;i++) 
	{
	  k=permpro[i];
	  p2[i+nlze]=(k<=k0)?coeff(matu,k,j):coeff(matgen,k-k0+nlze,j);
	}
    }
  av2=avma;p1=cgetg(li+1,17);
  for(i=nlze+1;i<=lig;i++) p1[i]=lstoi(vperm[permpro[i-nlze]+nlze]);
  for(i=nlze+1;i<=lig;i++) vperm[i]=itos((GEN)p1[i]);
  avma=av2;free(permpro);
  p1=p4;
  tetpil=avma;pdep=gcopy(pdep);
  wpronew=hnffinal(p1,&pdep,&matc,vperm,&matalpha,lnz-lp3,co,li,col,lig,nlze+lp3,&colnew);
  dec=lpile(av,tetpil,0)>>TWOPOTBYTES_IN_LONG;
  matalpha+=dec;wpronew+=dec;matc+=dec;pdep+=dec;
  *ptmatc=matc;*ptmatalpha=matalpha;
  *ptnlze=nlze+lp3;*ptcol=colnew;*ptpdep=pdep;
  return wpronew;
}

GEN hnffinal(GEN matgen, GEN* ptpdep, GEN* ptmatc,long* vperm,GEN* ptmatalpha,long lnz,long co,long li,long col,long lig,long nlze,long* ptcol)
{
  GEN u1u2,u1,u2,p1,p2,p3,p4,detmat,wpro,wpronew,matalphanew,matcnew;
  GEN matalpha=*ptmatalpha,matc=*ptmatc,pdep=*ptpdep,pdepnew;
  long i,j,k,s,i1,j1,j2,av2,tetpil,av,dec;
  int fl;

  if(DEBUGLEVEL>3) 
    {
      fprintf(errfile,"avant hnffinal :\n");
      fprintf(errfile,"mit =\n");output(matgen);
      if(nlze) {fprintf(errfile,"pdep =\n");output(pdep);}
      fprintf(errfile,"matalpha =\n");output(matalpha);
    }
  av=avma;
  u1u2=lllkerim(matgen);u1=(GEN)u1u2[1];u2=(GEN)u1u2[2];
  if(lg(u2)<=lnz) err(maxranker);
  p1=gmul(matgen,u2);detmat=absi(det(p1));
  wpro=hnfmod(p1,detmat);
  p2=gmul(u1,lllint(u1));
  p3=gmul(u2,invmulmat(p1,wpro));
  p4=cgetg(col+1,19);
  for(j=1;j<lg(p2);j++) p4[j]=p2[j];
  for(j=lg(p2);j<=col;j++) p4[j]=p3[j+1-lg(p2)];
  p3=cgetg(col+1,19);for(j=1;j<=col;j++) p3[j]=matc[j];
  p3=gmul(p3,p4);
  if(nlze) pdep=gmul(pdep,p4);
  matcnew=cgetg(co+1,19);
  for(j=1;j<=col;j++) matcnew[j]=p3[j];
  for(j=col+1;j<=co;j++) matcnew[j]=matc[j];
  for(s=0,i=lnz;i>0;i--)
    {
      p1=gcoeff(wpro,i,i);if(fl=gcmp1(p1)) s++;
      for(j=col+1;j<=co;j++)
	{
	  p2=fl ? gcoeff(matalpha,i+nlze,j-col) : dvmdii(gcoeff(matalpha,i+nlze,j-col),p1,(GEN*)0);
	  for(k=1;k<=nlze;k++)
	    coeff(matalpha,k,j-col)=lsub(gcoeff(matalpha,k,j-col),gmul(p2,gcoeff(pdep,k,i+col-lnz)));
	  for(k=nlze+1;k<=lig;k++)
	    coeff(matalpha,k,j-col)=lsub(gcoeff(matalpha,k,j-col),gmul(p2,gcoeff(wpro,k-nlze,i)));
	  matcnew[j]=lsub((GEN)matcnew[j],gmul(p2,(GEN)matcnew[i+col-lnz]));
	}
    }
  av2=avma;p1=cgetg(li+1,17);
  for(i=1,i1=0,j1=0;i<=lnz;i++) 
    {
      if(gcmp1(gcoeff(wpro,i,i)))
	p1[(++j1)+lig-s]=lstoi(vperm[i+nlze]);
      else
	p1[(++i1)+nlze]=lstoi(vperm[i+nlze]);
    }
  for(i=nlze+1;i<=lig;i++) vperm[i]=itos((GEN)(p1[i]));
  avma=av2;
  tetpil=avma;
  wpronew=cgetg(lnz-s+1,19);matalphanew=cgetg(co-col+s+1,19);
  matc=cgetg(co+1,19);if(nlze) pdepnew=cgetg(lnz-s+1,19);
  for(j1=1;j1<=col-lnz;j1++) matc[j1]=lcopy((GEN)matcnew[j1]);
  for(j=1,j1=0,j2=0;j<=lnz;j++)
    {
      if(!gcmp1(gcoeff(wpro,j,j)))
	{
	  p1=cgetg(lnz-s+1,18);wpronew[++j1]=(long)p1;
	  matc[j1+col-lnz]=lcopy((GEN)matcnew[j+col-lnz]);
	  if(nlze) pdepnew[j1]=lcopy((GEN)pdep[j+col-lnz]);
	  for(i=1,i1=0;i<=lnz;i++)
	    if(!gcmp1(gcoeff(wpro,i,i))) p1[++i1]=lcopy(gcoeff(wpro,i,j));
	}
      else
	{
	  p1=cgetg(lig-s+1,18);matalphanew[++j2]=(long)p1;
	  matc[j2+col-s]=lcopy((GEN)matcnew[j+col-lnz]);
	  for(i=1;i<=nlze;i++) p1[i]=lcopy(gcoeff(pdep,i,j+col-lnz));
	  for(i=1,i1=0;i<=lnz;i++)
	    if(!gcmp1(gcoeff(wpro,i,i))) p1[(++i1)+nlze]=lcopy(gcoeff(wpro,i,j));
	}
    }
  for(j=col+1;j<=co;j++) 
    {
      p1=cgetg(lig-s+1,18);matalphanew[j-col+s]=(long)p1;
      for(i=1;i<=nlze;i++) p1[i]=lcopy(gcoeff(matalpha,i,j-col));
      matc[j]=lcopy((GEN)matcnew[j]);
    }
  for(i=1,i1=0;i<=lnz;i++)
    {
      if(!gcmp1(gcoeff(wpro,i,i)))
	{
	  i1++;
	  for(j=col+1;j<=co;j++) 
	    coeff(matalphanew,i1+nlze,j-col+s)=lcopy(gcoeff(matalpha,i+nlze,j-col));
	}
    }
  dec=lpile(av,tetpil,0)>>TWOPOTBYTES_IN_LONG;if(nlze) pdepnew+=dec;
  matalphanew+=dec;wpronew+=dec;matc+=dec;*ptmatc=matc;*ptmatalpha=matalphanew;
  if(nlze) *ptpdep=pdepnew;
  *ptcol=col-s;
  if(DEBUGLEVEL>3) 
    {
      fprintf(errfile,"apres hnffinal :\n");
      fprintf(errfile,"mit =\n");output(wpronew);
      if(nlze) {fprintf(errfile,"pdep =\n");output(pdepnew);}
      fprintf(errfile,"matalpha =\n");output(matalphanew);
    }
  return wpronew;
}

GEN hnfadd(GEN mit, GEN* ptpdep, GEN* ptmatc,long* vperm,GEN* ptmatalpha,long co,long li,long col,long* ptnlze,GEN extramat,GEN extramatc)
{
  GEN extramat1,extramat2,p1,p2,extramatnew,matalpha=*ptmatalpha,matc=*ptmatc;
  GEN matcnew,matcnew2,matalphac,extramatcnew,p3,p4,matalphanew,pdep=*ptpdep;
  long i,j,extrarel,sizeofmit,lig,colnew,colshort,tetpil,av=avma,av2,dec,nlze;
  long lp3,*ww,*permpro;

  if(DEBUGLEVEL>3) 
    {
      fprintf(errfile,"\nextramat =\n");output(extramat);
    }
  extrarel=lg(extramat)-1;nlze=*ptnlze;
  sizeofmit=lg(mit)-1;lig=sizeofmit+nlze;
  extramat1=cgetg(extrarel+1,19);extramat2=cgetg(extrarel+1,19);
  for(j=1;j<=extrarel;j++)
    {
      p1=cgetg(lig+1,18);extramat1[j]=(long)p1;
      p2=cgetg(li-lig+1,18);extramat2[j]=(long)p2;
      for(i=1;i<=lig;i++) p1[i]=coeff(extramat,i,j);
      for(i=lig+1;i<=li;i++) p2[i-lig]=coeff(extramat,i,j);
    }
  matalphac=cgetg(co-col+1,19);for(j=col+1;j<=co;j++) matalphac[j-col]=matc[j];
  if((lg(matalpha)>1))
    {
      extramatnew=gsub(extramat1,gmul(matalpha,extramat2));
      extramatcnew=gsub(extramatc,gmul(matalphac,extramat2));
    }
  else {extramatnew=extramat1;extramatcnew=extramatc;}
  colshort=extrarel+sizeofmit;
  extramat=cgetg(colshort+1,19);
  matcnew=cgetg(co-col+colshort+1,19);
  for(j=1;j<=extrarel;j++) 
    {
      extramat[j]=extramatnew[j];matcnew[j]=extramatcnew[j];
    }
  for(j=extrarel+1;j<=colshort;j++)
    {
      p1=cgetg(lig+1,18);extramat[j]=(long)p1;
      for(i=1;i<=nlze;i++) p1[i]=coeff(pdep,i,j-extrarel);
      for(i=nlze+1;i<=lig;i++) p1[i]=coeff(mit,i-nlze,j-extrarel);
    }
  if(DEBUGLEVEL>3) 
    {
      fprintf(errfile,"extramat apres traitement =\n");output(extramat);
    }
  for(j=extrarel+1;j<=co-col+colshort;j++)
    matcnew[j]=matc[j-extrarel+col-sizeofmit];
  p3=imagecompl(gtrans(extramat));lp3=lg(p3)-1;
  if(!(ww=(long*)malloc((lig+1)*sizeof(long)))) err(amemer);
  for(i=1;i<=lig;i++) ww[i]=1;
  if(!(permpro=(long*)malloc((lig+1)*sizeof(long)))) err(amemer);
  for(i=1;i<=lp3;i++) {permpro[i]=itos((GEN)(p3[i]));ww[permpro[i]]=0;}
  for(j=1;j<=lig;j++) {if(ww[j]){permpro[i]=j;i++;}}
  free(ww);
  p4=cgetg(colshort+1,19);
  for(j=1;j<=colshort;j++)
    {
      p2=cgetg(lig+1-lp3,18);p4[j]=(long)p2;
      for(i=lp3+1;i<=lig;i++) p2[i-lp3]=coeff(extramat,permpro[i],j);
    }
  pdep=cgetg(colshort+1,19);
  for(j=1;j<=colshort;j++)
    {
      p2=cgetg(lp3+1,18);pdep[j]=(long)p2;
      for(i=1;i<=lp3;i++) p2[i]=coeff(extramat,permpro[i],j);
    }
  matalphanew=cgetg(li-lig+1,19);
  for(j=1;j<=li-lig;j++)
    {
      p2=cgetg(lig+1,18);matalphanew[j]=(long)p2;
      for(i=1;i<=lig;i++) p2[i]=coeff(matalpha,permpro[i],j);
    }
  matalpha=matalphanew;
  av2=avma;p1=cgetg(lig+1,17);
  for(i=1;i<=lig;i++) p1[i]=lstoi(vperm[permpro[i]]);
  for(i=1;i<=lig;i++) vperm[i]=itos((GEN)p1[i]);
  avma=av2;free(permpro);
  mit=hnffinal(p4,&pdep,&matcnew,vperm,&matalpha,lig-lp3,co-col+colshort,li,colshort,lig,lp3,&colnew);
  tetpil=avma;matalpha=gcopy(matalpha);mit=gcopy(mit);pdep=gcopy(pdep);
  matcnew2=cgetg(co+extrarel+1,19);
  for(j=1;j<=col-sizeofmit;j++) matcnew2[j]=lcopy((GEN)matc[j]);
  for(j=1;j<=co-col+sizeofmit+extrarel;j++) 
    matcnew2[j+col-sizeofmit]=lcopy((GEN)matcnew[j]);
  dec=lpile(av,tetpil,0)>>TWOPOTBYTES_IN_LONG;matcnew2+=dec;mit+=dec;
  pdep+=dec;matalpha+=dec;*ptmatc=matcnew2;*ptmatalpha=matalpha;*ptnlze=lp3;
  *ptpdep=pdep;return mit;
}


