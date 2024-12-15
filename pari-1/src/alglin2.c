/********************************************************************/
/********************************************************************/
/**                                                                **/
/**                  ++++++++++++++++++++++++++++++                **/
/**                  +                            +                **/
/**                  +     ALGEBRE LINEAIRE       +                **/
/**                  +                            +                **/
/**                  ++++++++++++++++++++++++++++++                **/
/**                                                                **/
/**                        (deuxieme partie)                       **/
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
/*                      POLYNOME CARACTERISTIQUE                   */
/*                                                                 */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

GEN     caract(GEN x, int v)
{
  long    n,k,l,tetpil,tx=typ(x);
  GEN     p1,p2,p3,p4,p5,p6;
  
  switch(tx)
    {
    case 1: case 2: case 3: case 4: case 5: case 7: p1=cgetg(4,10);
      p1[1]=evalsigne(1)+evallgef(4)+evalvarn(v);
      p1[2]=lneg(x);p1[3]=un;return p1;
    case 6: case 8: p1=cgetg(5,10);
      p1[1]=evalsigne(1)+evallgef(5)+evalvarn(v);p1[2]=lnorm(x);
      l=avma;p2=trace((GEN)x[1]);tetpil=avma;p1[3]=lpile(l,tetpil,gneg(p2));
      p1[4]=un;return p1;
    case 9: l=avma;p1=gsub(polx[MAXVARN],(GEN)x[2]);tetpil=avma;
      p1=subres((GEN)x[1],p1);
      if(varn(p1)==MAXVARN) {setvarn(p1,v);return gerepile(l,tetpil,p1);}
      else {tetpil=avma;return gerepile(l,tetpil,gsubst(p1,MAXVARN,polx[v]));}
    case 19: n=lg(x)-1;if(!n) return polun[v];
      if((n+1)!=lg((GEN)x[1])) err(mattype1);
      l=avma;p1=gzero;p2=gun;
      if(n%2) p2=gneg(p2);p5=cgetg(4,10);
      p5[1]=evalsigne(1)+evallgef(4)+evalvarn(v);p5[3]=un;
      p6=cgeti(3);p5[2]=zero;
      p6[1]=evalsigne(-1)+evallgef(3);p4=cgetg(3,14);p4[2]=(long)p5;
      for(k=0;k<=n;k++)
	{
	  p3=det(gsub(gscalsmat(k,n),x));p4[1]=lmul(p3,p2);p6[2]=k;
	  p1=gadd(p4,p1);p5[2]=(long)p6;
	  if(k!=n) p2=gdivgs(gmulsg(k-n,p2),k+1);
	}
      p2=mpfact(n);tetpil=avma;return gerepile(l,tetpil,gdiv((GEN)p1[1],p2));
    default: err(mattype1);return gnil;
    }
}


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*                      Methode des traces :
                        ce programme retourne le polynome caracteristique,
                        et si un pointeur non nul est fourni,celui pointe
                        sur la matrice adjointe a la sortie.       */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

GEN     caradj(GEN x, long v, GEN *py)
{
  long  i,j,k,l,f,av1,av2,av3,av4,decal;
  GEN   p,y,z,t;
  
  if(typ(x)!=19) err(mattype1);
  l=lg(x);
  if(l==1) {p=polun[v];if((long)py) *py=gcopy(x);return p;}
  if(l==2) {p=gsub(polx[v],trace(x));if((long)py) *py=gcopy(x);return p;}
  f=l&1;p=cgetg(l+2,10);setvarn(p,v);p[l+1]=un;
  av1=avma;t=trace(x);av2=avma;
  t=gerepile(av1,av2,gneg(t));
  p[l]=(long)t;
  av1=avma;
  y=cgetg(l,19);
  for (i=1;i<l;i++) y[i]=lgetg(l,18);
  for (i=1;i<l;i++)
    for (j=1;j<l;j++)
      {
        if (i==j) coeff(y,i,j)=ladd(gcoeff(x,i,j),t);
        else coeff(y,i,j)=coeff(x,i,j);
      }
  
  for (k=2;k<l-1;k++)
    {
      z=gmul(x,y);
      t=trace(z);av2=avma;
      t=gdivgs(t,-k);av3=avma;
      y=cgetg(l,19);
      for (i=1;i<l;i++) y[i]=lgetg(l,18);
      for (i=1;i<l;i++) for (j=1;j<l;j++)
        if (i==j) coeff(y,i,j)=ladd(gcoeff(z,i,j),t);
        else coeff(y,i,j)=lcopy(gcoeff(z,i,j));
      av4=avma;decal=lpile(av1,av2,0)>>TWOPOTBYTES_IN_LONG;
      p[l-k+1]=adecaler(t,av2,av4)?(long)(t+decal):(long)t;
      if(adecaler(y,av2,av4)) y+=decal;
      av1=av3+(decal<<TWOPOTBYTES_IN_LONG);
    }
  t=gzero;
  for (i=1;i<l;i++)
    t=gadd(t,gmul(gcoeff(x,1,i),gcoeff(y,i,1)));
  av2=avma;t=gneg(t);
  if ((long) py)
    {
      z=cgetg(l,19);
      for (i=1;i<l;i++) z[i]=lgetg(l,18);
      for (i=1;i<l;i++) for (j=1;j<l;j++)
        coeff(z,i,j)=f?lneg(gcoeff(y,i,j)):lcopy(gcoeff(y,i,j));
      av4=avma;decal=lpile(av1,av2,0)>>TWOPOTBYTES_IN_LONG;
      p[2]=adecaler(t,av2,av4)?(long)(t+decal):(long)t;
      *py=adecaler(z,av2,av4)?z+decal:z;
    }
  else p[2]=lpile(av1,av2,t);
  p[1]=evalsigne(1)+evallgef(l+2)+evalvarn(v);
  return p;
}


GEN     adj(GEN x)
{
  GEN     y;
  
  caradj(x,MAXVARN,&y);
  return y;
}

GEN  caradj0(GEN x, long v)
{
  long tx=typ(x),l,tetpil;
  GEN p1,p2;

  switch(tx)
    {
    case 1: case 2: case 3: case 4: case 5: case 7: p1=cgetg(4,10);
      p1[1]=evalsigne(1)+evallgef(4)+evalvarn(v);
      p1[2]=lneg(x);p1[3]=un;return p1;
    case 6: case 8: p1=cgetg(5,10);
      p1[1]=evalsigne(1)+evallgef(5)+evalvarn(v);p1[2]=lnorm(x);
      l=avma;p2=trace(x);tetpil=avma;p1[3]=lpile(l,tetpil,gneg(p2));
      p1[4]=un;return p1;
    case 9: l=avma;p1=gsub(polx[MAXVARN],(GEN)x[2]);tetpil=avma;
      p1=subres((GEN)x[1],p1);
      if(varn(p1)==MAXVARN) {setvarn(p1,v);return gerepile(l,tetpil,p1);}
      else {tetpil=avma;return gerepile(l,tetpil,gsubst(p1,MAXVARN,polx[v]));}
    default: return caradj(x,v,0);
    }
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*                                                                 */
/*              INVERSION D'UNE MATRICE                            */
/*                                                                 */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


GEN     invmulmat(GEN a, GEN b)     /* calcule a^(-1)*b, b etant une matrice.
			      ( Il faut : nblig(a)=nbcol(a)=nblig(b) ) */
                 
     
{
  long  nbli,nbco,i,j,k,av,av1,av2,av3,av4,tp;
  GEN   aa,x,p,m,u;
  
  
  nbco=lg(b)-1;if(!nbco) return cgetg(1,19);
  nbli=(lg(a)==1)?0:lg((GEN)a[1])-1;
  if (lg(a)-1 != nbli) err(invmuler1);
  if (nbli!=lg((GEN)b[1])-1) err(invmuler1);
  av=avma;
  x=cgetg(nbco+1,19);
  for (j=1;j<=nbco;j++)
    {
      x[j]=lgetg(nbli+1,18);
      for (i=1;i<=nbli;i++)
	coeff(x,i,j)=coeff(b,i,j);
    }
  av1=avma;
  aa=cgetg(nbli+1,19);
  for (j=1;j<=nbli;j++)
    {
      aa[j]=lgetg(nbli+1,18);
      for (i=1;i<=nbli;i++) coeff(aa,i,j)=coeff(a,i,j);
    }
  for (i=1;i<nbli;i++)
    {
      p=gcoeff(aa,i,i);k=i;tp=typ(p);
      if ((gcmp0(p))||(((tp==2)||(tp==6))&&(gexpo(p)<-((lg(p)-2)<<TWOPOTBITS_IN_LONG))))
	{
	  for (k=i+1;(k<=nbli)&&gcmp0(gcoeff(aa,k,i));k++);
	  if (k>nbli) err(matinv2);
	  else
	    {
	      for (j=i;j<=nbli;j++)
		{
		  u=gcoeff(aa,i,j);coeff(aa,i,j)=coeff(aa,k,j);
		  coeff(aa,k,j)=(long)u;
		}
	      for (j=1;j<=nbco;j++)
		{
		  u=gcoeff(x,i,j);coeff(x,i,j)=coeff(x,k,j);
		  coeff(x,k,j)=(long)u;
		}
	      p=gcoeff(aa,i,i);
	    }
	}
      for (k=i+1;k<=nbli;k++)
	{
	  m=gcoeff(aa,k,i);
	  if (!gcmp0(m))
	    {
	      m=gdiv(m,p);
	      for (j=i+1;j<=nbli;j++)
		coeff(aa,k,j)=lsub(gcoeff(aa,k,j),gmul(m,gcoeff(aa,i,j)));
	      for (j=1;j<=nbco;j++)
		coeff(x,k,j)=lsub(gcoeff(x,k,j),gmul(m,gcoeff(x,i,j)));
	    }
	}
    }
  av2=avma;
  p=gcoeff(aa,nbli,nbli);
  if (gcmp0(p)) err(matinv2);
  else
    {
      for (j=1;j<=nbco;j++)
	{
	  coeff(x,nbli,j)=ldiv(gcoeff(x,nbli,j),p);
	  for (i=nbli-1;i>0;i--)
	    {
	      av3=avma;
	      m=gcoeff(x,i,j);
	      for (k=i+1;k<=nbli;k++)
		m= gsub(m,gmul(gcoeff(aa,i,k),gcoeff(x,k,j)));
	      av4=avma;
	      coeff(x,i,j)=lpile(av3,av4,gdiv(m,gcoeff(aa,i,i)));
	    }
	}
      av=lpile(av1,av2,0);
      for(i=1;i<=nbli;i++)
	for(j=1;j<=nbco;j++)
	  if (gcoeff(x,i,j)<=(GEN)av1) coeff(x,i,j)+=av;
/* ATTENTION: VALABLE SI COEFF EST LONG; SINON DIVISER av PAR 4 */
    }
  return x;
}

GEN invmat(GEN a)
{
  long av=avma,tetpil;
  GEN b;
  
  b=gscalmat(gun,lg(a)-1);tetpil=avma;
  return gerepile(av,tetpil,invmulmat(a,b));
}


GEN     invmulmatreel(GEN a, GEN b)    

 /* calcule a^(-1)*b, b etant une matrice.
( Il faut : nblig(a)=nbcol(a)=nblig(b) ) */
     
{
  long  nbli,nbco,i,j,k,av,av1,av2,av3,av4;
  GEN   aa,x,p1,p,m,u;
  
  
  nbco=lg(b)-1;nbli=lg((GEN)a[1])-1;
  nbli=(lg(a)==1)?0:lg((GEN)a[1])-1;
  if (lg(a)-1 != nbli) err(invmuler1);
  if (nbli!=lg((GEN)b[1])-1) err(invmuler1);
  av=avma;
  x=cgetg(nbco+1,19);
  for (j=1;j<=nbco;j++)
    {
      x[j]=lgetg(nbli+1,18);
      for (i=1;i<=nbli;i++)
	coeff(x,i,j)=coeff(b,i,j);
    }
  av1=avma;
  aa=cgetg(nbli+1,19);
  for (j=1;j<=nbli;j++)
    {
      aa[j]=lgetg(nbli+1,18);
      for (i=1;i<=nbli;i++) coeff(aa,i,j)=coeff(a,i,j);
    }
  for (i=1;i<nbli;i++)
    {
      p=mpabs(gcoeff(aa,i,i));k=i;
      for(j=i+1;j<=nbli;j++)
	if(gcmp(p1=gabs(gcoeff(aa,j,i),3),p)>0) {p=p1;k=j;}
      if (gcmp0(p)) err(matinv2);
      else
	{
	  if(k>i)
	    {
	      for (j=i;j<=nbli;j++)
		{
		  u=gcoeff(aa,i,j);coeff(aa,i,j)=coeff(aa,k,j);
		  coeff(aa,k,j)=(long)u;
		}
	      for (j=1;j<=nbco;j++)
		{
		  u=gcoeff(x,i,j);coeff(x,i,j)=coeff(x,k,j);
		  coeff(x,k,j)=(long)u;
		}
	    }
	  p=gcoeff(aa,i,i);
	}
      for (k=i+1;k<=nbli;k++)
	{
	  m=gcoeff(aa,k,i);
	  if (!gcmp0(m))
	    {
	      m=gdiv(m,p);
	      for (j=i+1;j<=nbli;j++)
		coeff(aa,k,j)=lsub(gcoeff(aa,k,j),gmul(m,gcoeff(aa,i,j)));
	      for (j=1;j<=nbco;j++)
		coeff(x,k,j)=lsub(gcoeff(x,k,j),gmul(m,gcoeff(x,i,j)));
	    }
	}
    }
  av2=avma;
  p=gcoeff(aa,nbli,nbli);
  if (gcmp0(p)) err(matinv2);
  else
    {
      for (j=1;j<=nbco;j++)
	{
	  coeff(x,nbli,j)=ldiv(gcoeff(x,nbli,j),p);
	  
	  for (i=nbli-1;i>0;i--)
	    {
	      av3=avma;
	      m=gcoeff(x,i,j);
	      for (k=i+1;k<=nbli;k++)
		m= gsub(m,gmul(gcoeff(aa,i,k),gcoeff(x,k,j)));
	      av4=avma;
	      coeff(x,i,j)=lpile(av3,av4,gdiv(m,gcoeff(aa,i,i)));
	    }
	}
      av=lpile(av1,av2,0);
      for(i=1;i<=nbli;i++)
	for(j=1;j<=nbco;j++)
	  if (gcoeff(x,i,j)<=(GEN)av1) coeff(x,i,j)+=av;
    }
  return x;
}

GEN invmatreel(GEN a)
{
  long av=avma,tetpil;
  GEN b;
  
  b=gscalmat(gun,lg(a)-1);tetpil=avma;
  return gerepile(av,tetpil,invmulmatreel(a,b));
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*                                                                 */
/*              FORME DE HESSENBERG D'UNE MATRICE                  */
/*                                                                 */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

GEN hess(GEN x)
{
  long tx=typ(x),lx=lg(x),av=avma,tetpil,m,i,j;
  GEN p1,p2,p3,y;
  
  if((tx!=19)||(lg((GEN)x[1])!=lx)) err(mattype1);
  y=gcopy(x);
  for(m=2;m<lx-1;m++)
    {
      p2=gzero;
      for(i=m+1;(i<lx)&&(gcmp0(p2=(GEN)coeff(y,i,m-1)));i++);
      if(!gcmp0(p2))
	{
	  for(j=m-1;j<lx;j++)
	    {
	      p1=(GEN)coeff(y,i,j);coeff(y,i,j)=coeff(y,m,j);coeff(y,m,j)=(long)p1;
	    }
	  p1=(GEN)y[i];y[i]=y[m];y[m]=(long)p1;
	  for(i=m+1;i<lx;i++)
	    {
	      if(!gcmp0(p3=(GEN)coeff(y,i,m-1)))
		{
		  p3=gdiv(p3,p2);coeff(y,i,m-1)=zero;
		  for(j=m;j<lx;j++)
		    coeff(y,i,j)=lsub(gcoeff(y,i,j),gmul(p3,gcoeff(y,m,j)));
		  for(j=1;j<lx;j++)
		    coeff(y,j,m)=ladd(gcoeff(y,j,m),gmul(p3,gcoeff(y,j,i)));
		}
	    }
	}
    }
  tetpil=avma;return gerepile(av,tetpil,gcopy(y));
}

GEN carhess(GEN x, long v)
{
  long av=avma,tetpil,tx=typ(x),lx=lg(x),r,i;
  GEN *y,p1,p2,p3,p4;
  
  if((tx!=19)||(lg((GEN)x[1])!=lx)) err(mattype1);
  y=(GEN *)newbloc(4*lx);
  y[0]=polun[v];p1=hess(x);p2=polx[v];
  for(r=1;r<lx;r++)
    {
      y[r]=gmul((GEN)y[r-1],gsub(p2,gcoeff(p1,r,r)));
      p3=gun;p4=gzero;
      for(i=1;i<r;i++)
	{
	  p3=gmul(p3,gcoeff(p1,r-i+1,r-i));
	  p4=gadd(p4,gmul(gmul(p3,gcoeff(p1,r-i,r)),(GEN)y[r-i-1]));
	}
      tetpil=avma;y[r]=gsub((GEN)y[r],p4);
    }
  p1=gerepile(av,tetpil,(GEN)y[lx-1]);
  killbloc((GEN)y);return p1;
}



/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*                                                                 */
/*                          NORME                                  */
/*                                                                 */
/*            Cree un GEN pointant sur la norme de x               */
/*                                                                 */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

GEN     gnorm(GEN x)
{
  long    l,tx,lx,i,tetpil;
  GEN     p1,p2,y;
  
  switch(tx=typ(x))
    {
    case 1 :
    case 2 : y=mpmul(x,x);break;
      
    case 3 : err(normer1);
      
    case 4 :
    case 5 : y=gmul(x,x);break;
      
    case 6 : l=avma;p1=gmul((GEN)x[1],(GEN)x[1]);
      p2=gmul((GEN)x[2],(GEN)x[2]);
      tetpil=avma;
      y=gerepile(l,tetpil,gadd(p1,p2));break;
      
    case 8 : l=avma;p1=(GEN)x[1];
      if (gcmp0((GEN)p1[3]))
        {
          p2=gmul((GEN)p1[2],gmul((GEN)x[3],(GEN)x[3]));
          p1=gmul((GEN)x[2],(GEN)x[2]);
          tetpil=avma;
          y=gerepile (l,tetpil,gadd(p1,p2));
        }
      else
        {
          p2=gmul((GEN)p1[2],gmul((GEN)x[3],(GEN)x[3]));
          p1=gmul((GEN)x[2],gadd((GEN)x[2],(GEN)x[3]));
          tetpil=avma;
          y=gerepile(l ,tetpil,gadd(p1,p2));
        }
      break;
    case 10:
    case 11:
    case 13:
    case 14: l=avma;p1=gmul(gconj(x),x);tetpil=avma;
      y=gerepile(l,tetpil,greal(p1));break;
    case 9 : y=subres((GEN)x[1],(GEN)x[2]);break;
    case 17: 
    case 18:
    case 19: lx=lg(x);y=cgetg(lx,tx);
      for(i=1;i<lx;i++) y[i]=lnorm((GEN)x[i]);
      break;
    default: err(normer1);
    }
  return y;
}

GEN     gnorml2(GEN x)
{
  GEN  y,p1;
  long i,tx=typ(x),lx=lg(x),av,tetpil;
  
  if(tx<17) return gnorm(x);
  y=gzero;
  if(lx>1)
    {
      av=avma;y=gnorml2((GEN)x[1]);
      for(i=2;i<lx;i++)
        {
          p1=gnorml2((GEN)x[i]);tetpil=avma;
          y=gadd(p1,y);
        }
      if(lx>2) y=gerepile(av,tetpil,y);
    }
  return y;
}


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*                                                                 */
/*                            CONJUGAISON                          */
/*                                                                 */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

GEN     gconj(GEN x)
{
  long    lx,i,tx=typ(x);
  GEN     z,p1;
  
  switch(tx)
    {
    case 1 :
    case 2 :
    case 3 :
    case 4 :
    case 5 :
    case 7 : z=gcopy(x);break;
      
    case 6 : z=cgetg(3,6);z[1]=lcopy((GEN)x[1]);
      z[2]=lneg((GEN)x[2]);
      break;
      
    case 8 : z=cgetg(4,8);z[1]=copyifstack((GEN)x[1]);
      p1=(GEN)x[1];
      z[3]=lneg((GEN)x[3]);
      if(gcmp0((GEN)p1[3])) z[2]=lcopy((GEN)x[2]);
      else z[2]=ladd((GEN)x[2],(GEN)x[3]);
      break;
      
    case 10: lx=lg(x);z=cgetg(lx,tx);
      z[1]=x[1];
      for(i=2;i<lgef(x);i++)
        z[i]=lconj((GEN)x[i]);
      break;
      
    case 11: lx=lg(x);z=cgetg(lx,tx);
      z[1]=x[1];
      if(!gcmp0(x))
        {
          for(i=2;i<lx;i++)
            z[i]=lconj((GEN)x[i]);
        }
      break;
      
    case 13:
    case 14: 
    case 17:
    case 18:
    case 19: lx=lg(x);z=cgetg(lx,tx);
      for(i=1;i<lx;i++)
        z[i]=lconj((GEN)x[i]);
      break;
      
    default: err(conjer1);
    }
  return z;
}

GEN conjvec(GEN x,long prec)
{
  long    lx,s,av,tetpil,i,tx=typ(x);
  GEN     z,y,p1,p2,p;
  
  switch(tx)
    {
    case 1: case 3: case 4: case 5: z=cgetg(2,18);z[1]=lcopy(x);break;
    case 6: case 8: z=cgetg(3,18);z[1]=lcopy(x);z[2]=lconj(x);break;
    case 17: case 18: lx=lg(x);
      z=cgetg(lx,19);for(i=1;i<lx;i++) z[i]=(long)conjvec((GEN)x[i],prec);
      s=lg((GEN)z[1]);
      for(i=2;i<lx;i++) 
	if(lg((GEN)z[i])!=s) err(conjvecer1);
      break;
    case 9:
      y=(GEN)x[1];lx=lgef(y);
      if(lx<=3) z=cgetg(1,18);
      else
	{
	  av=avma;p=gzero;
	  for(i=2;i<lx;i++)
	    {
	      tx=typ((GEN)y[i]);
	      if((tx>5)||(tx==2)) err(conjvecer2);
	      if(tx==3) p=(GEN)((GEN)y[i])[1];
	    }
	  if(!signe(p))
	    {
	      p1=roots(y,prec);
	      z=cgetg(lx-2,18);
	      for(i=1;i<=lx-3;i++)
		{
		  p2=(GEN)p1[i];if(gcmp0(gimag(p2))) p2=greal(p2);
		  z[i]=(long)poleval((GEN)x[2],(GEN)p1[i]);
		}
	      tetpil=avma;z=gerepile(av,tetpil,gcopy(z));
	    }
	  else
	    {
	      z=cgetg(lx-2,18);z[1]=lcopy(x);
	      for(i=2;i<=lx-3;i++) z[i]=(long)gpui((GEN)z[i-1],p);
	    }
	}
      break;
    default: err(conjer1);
    }
  return z;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*                                                                 */
/*                  PARTIES REELLE ET IMAGINAIRES                  */
/*                                                                 */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

GEN     greal(GEN x)
{
  long    lx,i,j,av,tetpil,tx=typ(x);
  GEN     p1,p2,z;
  
  switch(tx)
    {
    case 1 :
    case 2 :
    case 4 :
    case 5 : z=gcopy(x);break;
      
    case 6 : z=gcopy((GEN)x[1]);break;
      
    case 8 : z=gcopy((GEN)x[2]);break;
      
    case 10: lx=lgef(x);av=avma;
      for(i=lx-1;(i>=2)&&(gcmp0(greal((GEN)x[i])));i--);
      avma=av;
      if(i<2) {z=cgetg(2,tx);z[1]=evallgef(2);}
      else 
	{
	  z=cgetg(i+1,tx);z[1]=evalsigne(1)+evallgef(1+i);
	  for(j=2;j<=i;j++) z[j]=lreal((GEN)x[j]);
	}
      setvarn(z,varn(x));
      break;
      
    case 11: lx=lg(x);av=avma;
      if(gcmp0(x)) {z=cgetg(2,tx);z[1]=x[1];}
      else
	{
	  for(i=2;(i<lx)&&(gcmp0(greal((GEN)x[i])));i++);
	  avma=av;
	  if(i==lx)
	    {
	      z=cgetg(2,tx); setvalp(z, lx - 2 + valp(x));
	      setsigne(z,0); setvarn(z, varn(x));
	    }
	  else
	    {
	      z=cgetg(lx-i+2,tx);
	      for(j = 2; j <= lx - i + 1; j++) z[j] = lreal((GEN)x[j + i - 2]);
	      z[1] = x[1]; setvalp(z, valp(x) + i - 2);
	    }
	}
      break;
      
    case 13:
    case 14: av=avma;p1=gadd(gmul(greal((GEN)x[1]),greal((GEN)x[2])),gmul(gimag((GEN)x[1]),gimag((GEN)x[2])));
      p2=gadd(gsqr(greal((GEN)x[2])),gsqr(gimag((GEN)x[2])));tetpil=avma;
      z=gerepile(av,tetpil,gdiv(p1,p2));break;
    case 17:
    case 18:
    case 19: lx=lg(x);z=cgetg(lx,tx);
      for(i=1;i<lx;i++) z[i]=lreal((GEN)x[i]);
      break;
      
    default: err(realer1);
    }
  return z;
}

GEN     gimag(GEN x)
{
  long    lx,i,j,av,tetpil,tx=typ(x);
  GEN     p1,p2,z;
  
  switch(tx)
    {
    case 1 :
    case 2 :
    case 4 :
    case 5 : z=gzero;break;
      
    case 6 : z=gcopy((GEN)x[2]);
      break;
      
    case 8 : z=gcopy((GEN)x[3]);
      break;
      
    case 10: lx=lgef(x);av=avma;
      for(i=lx-1;(i>=2)&&(gcmp0(gimag((GEN)x[i])));i--);
      avma=av;
      if(i<2) {z=cgetg(2, tx);z[1]=2;}
      else 
	{
	  z=cgetg(i+1,tx);z[1]=evalsigne(1)+evallgef(1+i);
	  for(j=2;j<=i;j++) z[j]=limag((GEN)x[j]);
	}
      setvarn(z,varn(x));
      break;
      
    case 11: lx=lg(x);av=avma;
      if(gcmp0(x)) {z=cgetg(2,tx);z[1]=x[1];}
      else
	{
	  for(i=2;(i<lx)&&(gcmp0(gimag((GEN)x[i])));i++);
	  avma=av;
	  if(i==lx)
	    {
	      z=cgetg(2,tx); setvalp(z, lx - 2 + valp(x));
	      setsigne(z,0); setvarn(z, varn(x));
	    }
	  else
	    {
	      z=cgetg(lx-i+2,tx);
	      for(j = 2; j <= lx - i + 1; j++) z[j] = limag((GEN)x[j + i - 2]);
	      z[1] = x[1]; setvalp(z, valp(x) + i - 2);
	    }
	}
      break;
      
    case 13:
    case 14: av=avma;p1=gsub(gmul(gimag((GEN)x[1]),greal((GEN)x[2])),gmul(greal((GEN)x[1]),gimag((GEN)x[2])));
      p2=gadd(gsqr(greal((GEN)x[2])),gsqr(gimag((GEN)x[2])));tetpil=avma;
      z=gerepile(av,tetpil,gdiv(p1,p2));break;
    case 17:
    case 18:
    case 19: lx=lg(x);z=cgetg(lx,tx);
      for(i=1;i<lx;i++)
        z[i]=limag((GEN)x[i]);
      break;
      
    default: err(imager1);
    }
  return z;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*                                                                 */
/*                            TRACES                               */
/*                                                                 */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

GEN     assmat(GEN x)
{
  long    lx,i,j;
  GEN     y,p1;
  
  if((typ(x)!=10) || gcmp0(x)) err(mattype2);
  lx=lgef(x)-2;y=cgetg(lx,19);
  for(i=1;i<lx-1;i++)
    {
      p1=cgetg(lx,18);y[i]=(long)p1;
      for(j=1;j<lx;j++)
        {p1[j]=(j==(i+1)) ? un : zero;}
    }
  p1=cgetg(lx,18);y[i]=(long)p1;
  if(gcmp1((GEN)x[lx+1]))
    {
      for(j=1;j<lx;j++)
        p1[j]=lneg((GEN)x[j+1]);
    }
  else
    {
      gnegz((GEN)x[lx+1],(GEN)x[lx+1]);
      for(j=1;j<lx;j++)
        p1[j]=ldiv((GEN)x[j+1],(GEN)x[lx+1]);
      gnegz((GEN)x[lx+1],(GEN)x[lx+1]);
    }
  return y;
}

GEN     trace(GEN x)
{
  long    i,l,n,tx=typ(x),lx=lg(x),tetpil;
  GEN     y,p1,p2;
  
  switch(tx)
    {
    case 1 :
    case 2 :
    case 4 :
    case 5 : y=gmul2n(x,1);break;
      
    case 6 : y=gmul2n((GEN)x[1],1);break;
      
    case 8 : p1=(GEN)x[1];
      if (!gcmp0((GEN)p1[3]))
        {
          l=avma;p2=gmul2n((GEN)x[2],1);
          tetpil=avma;
          y=gerepile(l,tetpil,gadd((GEN)x[3],p2));
        }
      else y=gmul2n((GEN)x[2],1);
      break;
      
    case 10: lx=lg(x);y=cgetg(lx,tx);
      y[1]=x[1];
      for(i=2;i<lgef(x);i++)
        y[i]=ltrace((GEN)x[i]);
      break;
      
    case 11: lx=lg(x);y=cgetg(lx,tx);
      y[1]=x[1];
      if(!gcmp0(x))
        {
          for(i=2;i<lx;i++)
            y[i]=ltrace((GEN)x[i]);
        }
      break;
      
    case 9 : l=avma;p1=polsym((GEN)x[1],n=(lgef((GEN)x[1])-4));
      p2=gzero;for(i=0;i<=n;i++) p2=gadd(p2,gmul(truecoeff((GEN)x[2],i),(GEN)p1[i+1]));
      tetpil=avma;y=gerepile(l,tetpil,gcopy(p2));
      break;
      
    case 13:
    case 14: y=gadd(x,gconj(x));break;
      
    case 17:
    case 18: lx=lg(x);y=cgetg(lx,tx);
      for(i=1;i<lx;i++)
        y[i]=ltrace((GEN)x[i]);
      break;
      
    case 19: if (lx!=lg((GEN)x[1])) err(mattype3);
      l=avma;p1=gcopy(gcoeff(x,1,1));
      if(lx==2) return p1;
      else
        {
          for(i=2;i<lx-1;i++)
            p1=gadd(p1,gcoeff(x,i,i));
          tetpil=avma;
          y=gerepile(l,tetpil,gadd(p1,gcoeff(x,i,i)));
        }
      break;
    default: err(mattype3);
    }
  return y;
}

GEN     trace9(GEN x, GEN p1)
/* a usage interne. Pas de verifs.
Calcule trace(gmodulcp(x,pol)), ou p1=polsym(pol,lgef(pol)-4) */
{
  GEN p2,p3;
  long av=avma,tetpil,i;

  if(!signe(x)) return gzero;
  for(p2=gzero,i=2;i<lgef(x);i++)
    {
      p3=gmul((GEN)x[i],(GEN)p1[i-1]);tetpil=avma;
      p2=gadd(p2,p3);
    }
  return gerepile(av,tetpil,p2);
}

/*===================================*/
/*     Reduction en carres           */
/*===================================*/


/*=======================================================
  Reduction de Gauss ( Matrice definie >0 )
  ========================================================*/


GEN sqred1(GEN a)
{
  GEN b,p;
  long av,av1,n,i,j,k,lim;
  
  if (typ(a)!=19) err(kerer1);
  if (lg((GEN)a[1])!=(n=lg(a))) err(mattype1);
  lim=(avma+bot)>>1;
  n--;av=avma;
  b=gcopy(a);
  for(i=1;i<=n;i++)
    for(j=1;j<i;j++) coeff(b,i,j) = zero;
  for(k=1;k<=n;k++)
    {
      if(gsigne(p=gcoeff(b,k,k))<=0) err(sqreder1);
      for(i=k+1;i<=n;i++)
	for(j=i;j<=n;j++)
	  coeff(b,i,j)=lsub(gcoeff(b,i,j),gdiv(gmul(gcoeff(b,k,i),gcoeff(b,k,j)),p));
      for(j=k+1;j<=n;j++)
	coeff(b,k,j)=ldiv(gcoeff(b,k,j),p);
      if(avma<lim) {av1=avma;b=gerepile(av,av1,gcopy(b));}
    }
  av1=avma;
  return gerepile(av,av1,gcopy(b));
}

GEN sqred3(GEN a)
{
  long n,av=avma,tetpil,lim,i,j,k,l;
  GEN p1,z;

  if (typ(a)!=19) err(kerer1);
  if (lg((GEN)a[1])!=(n=lg(a))) err(mattype1);
  lim=(avma+bot)>>1;
  av=avma;z=cgetg(n,19);
  for(j=1;j<n;j++) 
    {
      p1=cgetg(n,18);z[j]=(long)p1;
      for(i=1;i<n;i++) p1[i]=zero;
    }
  for(i=1;i<n;i++)
    {
      for(k=1;k<i;k++)
	{
	  p1=gzero;
	  for(l=1;l<k;l++) p1=gadd(p1,gmul(gmul(gcoeff(z,l,l),gcoeff(z,k,l)),gcoeff(z,i,l)));
	  coeff(z,i,k)=ldiv(gsub(gcoeff(a,i,k),p1),gcoeff(z,k,k));
	}
      p1=gzero;
      for(l=1;l<i;l++) p1=gadd(p1,gmul(gmul(gcoeff(z,l,l),gcoeff(z,i,l)),gcoeff(z,i,l)));
      coeff(z,i,k)=lsub(gcoeff(a,i,i),p1);
      if(avma<lim) {tetpil=avma;z=gerepile(av,tetpil,gcopy(z));}
    }
  tetpil=avma;return gerepile(av,tetpil,gcopy(z));
}

/*=======================================================
  Reduction de Gauss (matrice symetrique quelconque)
  Signature d'une matrice symetrique
  ( seule la partie superieure est consideree )
  ========================================================*/

GEN sqred2(GEN a, long flg)
{
  GEN b,r,p,u;
  long av,av1,av2,lim,n,i,j,k,l,sp,sn,t;
  if (typ(a)!=19) err(kerer1);
  if (lg((GEN)a[1])!=(n=lg(a))) err(mattype1);
  av=avma;lim=(avma+bot)>>1;
  r=cgeti(n);for(i=1;i<n;i++) r[i]=1;
  av2=avma;b=gcopy(a);
  t=(--n);sp=sn=0;
  
  while (t)
    {
      for(k=1;(k<=n)&&(gcmp0(gcoeff(b,k,k))||(!r[k]));k++);
      if(k<=n)
	{
	  p=gcoeff(b,k,k);
	  if(gsigne(p)>0) sp++;else sn++;
	  r[k]=0;t--;
	  for(j=1;j<=n;j++) 
	    coeff(b,k,j)=r[j] ? ldiv(gcoeff(b,k,j),p) : zero;
	  
	  for(i=1;i<=n;i++) if (r[i])
	    {
	      for(j=1;j<=n;j++)
		coeff(b,i,j)=r[j] ? lsub(gcoeff(b,i,j),gmul(gmul(gcoeff(b,k,i),gcoeff(b,k,j)),p)) : zero;
	    }
	  coeff(b,k,k)=(long)p;
	}
      else
	{
	  for(k=1;k<=n;k++) if (r[k])
	    {
	      for(l=k+1;(l<=n)&&(gcmp0(gcoeff(b,k,l))||(!r[l]));l++);
	      if(l<=n)
		{
		  p=gcoeff(b,k,l);r[k]=r[l]=0;sp++;sn++;t-=2;
		  for(i=1;i<=n;i++) if(r[i])
		    {
		      for(j=1;j<=n;j++)
			coeff(b,i,j)=r[j]? lsub(gcoeff(b,i,j),gdiv(gadd(gmul(gcoeff(b,k,i),gcoeff(b,l,j)),gmul(gcoeff(b,k,j),gcoeff(b,l,i))),p)) : zero;
		    }
		  for(j=1;j<=n;j++) if (r[j])
		    {
		      u=gcoeff(b,k,j);
		      coeff(b,k,j)=ldiv(gadd(u,gcoeff(b,l,j)),p);
		      coeff(b,l,j)=ldiv(gsub(u,gcoeff(b,l,j)),p);
		    }
		  coeff(b,k,l)=un;coeff(b,l,k)=lneg(gun);
		  coeff(b,k,k)=lmul2n(p,-1);coeff(b,l,l)=lneg(gcoeff(b,k,k));
		  break;
		}
	      if(avma<lim) {av1=avma;b=gerepile(av2,av1,gcopy(b));}
	    }
	  if(k>n) break;
	}
    }
  if (flg) {av1=avma;return gerepile(av,av1,gcopy(b));}
  else
    {
      avma=av;
      b=cgetg(3,17);b[1]=lstoi(sp);b[2]=lstoi(sn);return b;
    }
}

GEN sqred(GEN a) {return sqred2(a,1); }
GEN signat(GEN a) {return sqred2(a,0); }



/*===========================================================================
  Diagonalisation d'une matrice symetrique REELLE;
  matrice de passage orthogonale R
  (  Renvoie un vecteur a 2 comp :
  1-re comp = vect des valeurs propres
  2-me comp = matr des vecteurs propres   ).
  ============================================================================*/

GEN jacobi(GEN a, long prec)
{
  long de,e,e1,e2,l,n,i,j,p,q,av1,av2,iter=0;
  GEN c,s,t,u,ja,lambda,r,unr,x,y,x1,y1;
  
  if (typ(a)!=19) err(mattype1);
  ja=cgetg(3,17);
  n=(l=lg(a))-1;
  e1=HIGHEXPOBIT-1;
  lambda=cgetg(l,18);ja[1]=(long)lambda;
  for(j=1;j<=n;j++)
    {
      gaffect(gcoeff(a,j,j),x = (GEN)(lambda[j]=lgetr(prec)));
/*      if(((e=expo(x))<e1)&&(gsigne(x))) e1=e; */
      if((e=expo(x))<e1) e1=e;
      
/* e1 = min des expo des coeff diagonaux
   e2 = max des expo des coeff extra-diagonaux
   Test d'arret: e2 < e1-precision  */

    }
  r=cgetg(l,19);ja[2]=(long)r;
  for(j=1;j<=n;j++)
    {
      r[j]=lgetg(l,18);
      for(i=1;i<l;i++)
	affsr(i==j,(GEN)(coeff(r,i,j)=lgetr(prec)));
    }
  av1=avma;
  e2=-HIGHEXPOBIT;
  c=cgetg(l,19);
  for(j=1;j<=n;j++)
    {
      c[j]=lgetg(j,18);
      for(i=1;i<j;i++)
	{
	  gaffect(gcoeff(a,i,j),x=(GEN)(coeff(c,i,j)=lgetr(prec)));
	  if((e=expo(x))>e2) {e2=e;p=i;q=j;}
	}
    }
  
  a=c;
  affsr(1,unr=cgetr(prec));
  de=((prec-2)<<TWOPOTBITS_IN_LONG);
  
  while(e1-e2<de)
    {
      iter++;
      /*calcul de la rotation associee dans le plan
	des p et q-iemes vecteurs de base   */
      av2=avma;
      x=divrr(subrr((GEN)lambda[q],(GEN)lambda[p]),shiftr(gcoeff(a,p,q),1));
      y=mpsqrt(addrr(unr,mulrr(x,x)));
      t=(gsigne(x)>0)? divrr(unr,addrr(x,y)) : divrr(unr,subrr(x,y));
      c=divrr(unr,mpsqrt(addrr(unr,mulrr(t,t))));
      s=mulrr(t,c);u=divrr(s,addrr(unr,c));
      
      /* Recalcul des transformees successives de la matrice a et de la matrice
	 cumulee (r) des rotations :  */
      
      
      for(i=1;i<p;i++)
	{
	  x=gcoeff(a,i,p); y=gcoeff(a,i,q);
	  x1=subrr(x,mulrr(s,addrr(y,mulrr(u,x))));
	  y1=addrr(y,mulrr(s,subrr(x,mulrr(u,y))));
	  affrr(x1,gcoeff(a,i,p));affrr(y1,gcoeff(a,i,q));
	}
      for(i=p+1;i<q;i++)
	{
	  x=gcoeff(a,p,i); y=gcoeff(a,i,q);
	  x1=subrr(x,mulrr(s,addrr(y,mulrr(u,x))));
	  y1=addrr(y,mulrr(s,subrr(x,mulrr(u,y))));
	  affrr(x1,gcoeff(a,p,i));affrr(y1,gcoeff(a,i,q));
	}
      for(i=q+1;i<=n;i++)
	{
	  x=gcoeff(a,p,i); y=gcoeff(a,q,i);
	  x1=subrr(x,mulrr(s,addrr(y,mulrr(u,x))));
	  y1=addrr(y,mulrr(s,subrr(x,mulrr(u,y))));
	  affrr(x1,gcoeff(a,p,i));affrr(y1,gcoeff(a,q,i));
	}
      x=(GEN)lambda[p];y=gcoeff(a,p,q);subrrz(x,mulrr(t,y),(GEN)lambda[p]);
      x=y;y=(GEN)lambda[q];addrrz(y,mulrr(t,x),y);
      /*      if((e=expo(lambda[p]))<e1) e1=e;
	      if((e=expo(lambda[q]))<e1) e1=e; */
      /*      affsr(0,x);      NON !  */
      setexpo(x,expo(x)-de-1);
      
      for(i=1;i<=n;i++)
	{
	  x=gcoeff(r,i,p); y=gcoeff(r,i,q);
	  x1=subrr(x,mulrr(s,addrr(y,mulrr(u,x))));
	  y1=addrr(y,mulrr(s,subrr(x,mulrr(u,y))));
	  affrr(x1,gcoeff(r,i,p));affrr(y1,gcoeff(r,i,q));
	}
      
      e2=expo(gcoeff(a,1,2));p=1;q=2;
      for(j=1;j<=n;j++)
	{
	  for(i=1;i<j;i++) if((e=expo(gcoeff(a,i,j)))>e2) {e2=e;p=i;q=j;}
	  for(i=j+1;i<=n;i++) if((e=expo(gcoeff(a,j,i)))>e2) {e2=e;p=j;q=i;}
	}
      avma=av2;
    }     /* Fin de la boucle (while) de recalcul */
  avma=av1; return ja;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~									~*/
/*~		    MATRICE RATIONNELLE-->ENTIERE			~*/
/*~									~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

GEN matrixqz(GEN x, GEN pp)
{
  long av=avma,av1,tetpil,i,j,j1,m,n,t,fl,lim,nfact;
  GEN p,p1,p2,p3,p4,p5,unmodp,pk,pt;

  if(typ(x)!=19) err(matqzer1);
  lim=(avma+bot)>>1;
  n=lg(x)-1;if(!n) return gcopy(x);
  m=lg((GEN)x[1])-1;if(n>m) err(matqzer2);
  if(n==m)
    {
      p1=det(x);if(gcmp0(p1)) err(matqzer4);
      return idmat(n);
    }
  p1=cgetg(n+1,19);
  for(j=1;j<=n;j++)
    {
      p2=gun;p3=(GEN)x[j];
      for(i=1;i<=m;i++)
	{
	  t=typ((GEN)p3[i]);if((t>5)||(t==3)) err(matqzer3);
	  p2=ggcd(p2,(GEN)p3[i]);
	}
      p1[j]=ldiv(p3,p2);
    }
  x=p1;
  if(gcmp0(pp))
    {
      pt=gtrans(x);
      p1=cgetg(n+1,19);
      for(j=1;j<=n;j++) p1[j]=pt[j];
      p2=det(p1);p1[n]=pt[n+1];p3=det(p1);
      p4=ggcd(p2,p3);if(!signe(p4)) err(impl,"matrixqz when the first 2 dets are zero");
      if(gcmp1(p4)) {tetpil=avma;return gerepile(av,tetpil,gcopy(x));}
      p3=factor(p4);p1=(GEN)p3[1];p2=(GEN)p3[2];nfact=lg(p1)-1;
      av1=avma;p3=cgetg(n+1,17);
      for(i=1;i<=nfact;i++)
	{
	  p=(GEN)p1[i];unmodp=gmodulcp(gun,p);fl=1;
	  while(fl)
	    {
	      pk=ker(gmul(unmodp,x));if(lg(pk)==1) fl=0;
	      else
		{
		  p5=centerlift(pk);p4=gdiv(gmul(x,p5),p);
		  for(i=1;i<=n;i++) p3[i]=0;
		  for(j=1;j<lg(p5);j++)
		    {
		      j1=n;while(gcmp0(gcoeff(p5,j1,j))) j1--;
		      p3[j1]=j;
		    }
		  for(j=1;j<=n;j++) if(p3[j]) x[j]=p4[p3[j]];
		}
	    }
	  if(avma<lim) {tetpil=avma;x=gerepile(av1,tetpil,gcopy(x));}
	}
    }
  else
    {
      unmodp=gmodulcp(gun,pp);fl=1;av1=avma;
      while(fl)
	{
	  pk=ker(gmul(unmodp,x));if(lg(pk)==1) fl=0;
	  else
	    {
	      p5=centerlift(pk);p4=gdiv(gmul(x,p5),pp);
	      for(i=1;i<=n;i++) p3[i]=0;
	      for(j=1;j<lg(p5);j++)
		{
		  j1=n;while(gcmp0(gcoeff(p5,j1,j))) j1--;
		  p3[j1]=j;
		}
	      for(j=1;j<=n;j++)	if(p3[j]) x[j]=p4[p3[j]];
	      if(avma<lim) {tetpil=avma;x=gerepile(av1,tetpil,gcopy(x));}
	    }
	}
    }
  tetpil=avma;return gerepile(av,tetpil,gcopy(x));
}

GEN matrixqz2(GEN x)
{
  long av=avma,tetpil,i,j,k,m,n,fl,lim,in[2];
  GEN p1;

  if(typ(x)!=19) err(matqzer1);
  lim=(avma+bot)>>1;
  n=lg(x)-1;if(!n) return gcopy(x);
  x=gcopy(x);
  m=lg((GEN)x[1])-1;
  for(i=1;i<=m;i++)
    {
      do
	{
	  for(fl=0,j=1;(j<=n)&&(fl<2);j++)
	    if(!gcmp0(gcoeff(x,i,j))) in[fl++]=j;
	  if(fl==2)
	    {
	      j=(gcmp(gabs(gcoeff(x,i,in[0]),3),gabs(gcoeff(x,i,in[1]),3))>0)?in[1]:in[0];
	      p1=(GEN)coeff(x,i,j);
	      for(k=1;k<=n;k++)
		if(k!=j) x[k]=lsub((GEN)x[k],gmul(ground(gdiv(gcoeff(x,i,k),p1)),(GEN)x[j]));
	    }
	}
      while(fl==2);
      j=1;while((j<=n)&&gcmp0(gcoeff(x,i,j))) j++;
      if(j<=n) x[j]=lmul(denom(gcoeff(x,i,j)),(GEN)x[j]);
      if(avma<lim) {tetpil=avma;x=gerepile(av,tetpil,gcopy(x));}
    }
  tetpil=avma;return gerepile(av,tetpil,hnf(x));
}

GEN matrixqz3(GEN x)
{
  long av=avma,av1,tetpil,i,j,j1,k,m,n,fl,lim,in[2];
  GEN p1,c,d;

  if(typ(x)!=19) err(matqzer1);
  lim=(avma+bot)>>1;
  n=lg(x)-1;if(!n) return gcopy(x);
  x=gcopy(x);m=lg((GEN)x[1])-1;
  c=cgeti(n+1);for(i=1;i<=n;i++) c[i]=0;
  d=cgeti(m+1);for(i=1;i<=m;i++) d[i]=0;
  av1=avma;
  for(k=1;k<=m;k++)
    {
      j=1;while((j<=n)&&(c[j]||gcmp0(gcoeff(x,k,j)))) j++;
      if(j<=n)
	{
	  x[j]=ldiv((GEN)x[j],gcoeff(x,k,j));
	  for(j1=1;j1<=n;j1++) if(j1!=j) x[j1]=lsub((GEN)x[j1],gmul(gcoeff(x,k,j1),(GEN)x[j]));
	  c[j]=k;d[k]=j;
	}
      if(avma<lim) {tetpil=avma;x=gerepile(av1,tetpil,gcopy(x));}
    }
  for(i=1;i<=m;i++)
    {
      do
	{
	  for(fl=0,j=1;(j<=n)&&(fl<2);j++)
	    if(!gcmp0(gcoeff(x,i,j))) in[fl++]=j;
	  if(fl==2)
	    {
	      j=(gcmp(gabs(gcoeff(x,i,in[0]),3),gabs(gcoeff(x,i,in[1]),3))>0)?in[1]:in[0];
	      p1=(GEN)coeff(x,i,j);
	      for(k=1;k<=n;k++)
		if(k!=j) x[k]=lsub((GEN)x[k],gmul(ground(gdiv(gcoeff(x,i,k),p1)),(GEN)x[j]));
	    }
	}
      while(fl==2);
      j=1;while((j<=n)&&gcmp0(gcoeff(x,i,j))) j++;
      if(j<=n) {p1=denom(gcoeff(x,i,j));if(!gcmp1(p1)) x[j]=lmul(p1,(GEN)x[j]);}
      if(avma<lim) {tetpil=avma;x=gerepile(av1,tetpil,gcopy(x));}
    }
  tetpil=avma;return gerepile(av,tetpil,hnf(x));
}

GEN kerint1(GEN x)
{
  long av=avma,tetpil;
  GEN p1,p2;

  p1=matrixqz3(ker(x));
  p2=lllint(p1);tetpil=avma;return gerepile(av,tetpil,gmul(p1,p2));
}

GEN kerint2(GEN x)
{
  long lx=lg(x), tx=typ(x),i,j,av,av1;
  GEN g,p1;

  if(tx!=19) err(lller1);
  av=avma;
  g=cgetg(lx,19);
  for(j=1;j<lx;j++) g[j]=lgetg(lx,18);
  for(i=1;i<lx;i++)
    for(j=1;j<=i;j++) coeff(g,i,j)=coeff(g,j,i)=(long)gscal((GEN)x[i],(GEN)x[j]);
  g=lllgramall(g,1);p1=lllint(g);
  av1=avma;return gerepile(av,av1,gmul(g,p1));
}

GEN kerint(GEN x)
{
  long av=avma,av1;
  GEN g,p1;

  g=lllall0(x,1);if(lg(g)==1) return g;
  p1=lllint(g);av1=avma;return gerepile(av,av1,gmul(g,p1));
}

GEN intersect(GEN x, GEN y)
{
  long av=avma,tetpil,i,j,k,p;
  GEN z,p1,r;

  if((typ(x)!=19)||(typ(y)!=19)) err(interer1);
  if((lg(x)==1)||(lg(y)==1)) return cgetg(1,19);
  z=ker(concat(x,y));k=lg(x);p=lg(z);
  r=cgetg(p,19);
  for(j=1;j<p;j++)
    {
      p1=cgetg(k,18);r[j]=(long)p1;
      for(i=1;i<k;i++) p1[i]=(long)coeff(z,i,j);
    }
  tetpil=avma;return gerepile(av,tetpil,gmul(x,r));
}
  
	      
		
	    
	  
      
