/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~                                                                     ~*/
/*~                       OPERATIONS ARITHMETIQUES                      ~*/
/*~                                                                     ~*/
/*~                           SUR LES POLYNOMES                         ~*/
/*~                                                                     ~*/
/*~                           (deuxieme partie)                         ~*/
/*~                                                                     ~*/
/*~                          copyright Babe Cool                        ~*/
/*~                                                                     ~*/
/*~                                                                     ~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

# include "genpari.h"

/* setup for calling cmbf */

GEN cmbf_target, cmbf_lc, cmbf_abslc, cmbf_abslcxtarget, cmbf_mod,
    cmbf_modfax, cmbf_fax;
long cmbf_degree, cmbf_modfaxn, cmbf_faxn;

#define NOMBDEP 5

GEN releve(GEN x)
{
  long lx=lg(x),tx=typ(x),i;
  GEN y;
  switch(tx)
    {
    case 3:
    case 9: return (GEN)x[2];
    case 7: return (GEN)x[4];
    case 10: lx=lgef(x);
    case 11: if(!signe(x)) return x;
    case 6:
    case 17:
    case 18:
    case 19: y=cgetg(lx,tx);
      for(i=1;i<lontyp[tx];i++) y[i]=x[i];
      for(i=lontyp[tx];i<lx;i++)
	y[i]=(long)releve((GEN)x[i]);
      return y;
    default: return x;
    }
}

GEN centermod(GEN x, GEN p)
{
  GEN y,p1,ps2;
  long av=avma,tetpil,lx=lg(x),tx=typ(x),i;

  ps2=shifti(p,-1);
  switch(tx)
    {
    case 1: y=modii(x,p);
      if(gcmp(y,ps2)>=0)
	{tetpil=avma;y=gerepile(av,tetpil,subii(y,p));}
      return y;
    case 10: lx=lgef(x);
    case 11: if(!signe(x)) return x;
      y=cgetg(lx,tx);
      y[1]=x[1];for(i=2;i<lx;i++)
	{
	  p1=modii((GEN)x[i],p);
	  if(gcmp(p1,ps2)>=0) p1=subii(p1,p);
	  y[i]=(long)p1;
	}
      tetpil=avma;return gerepile(av,tetpil,gcopy(y));
    case 6:
    case 17:
    case 18:
    case 19: y=cgetg(lx,tx);
      for(i=1;i<lx;i++) y[i]=(long)centermod((GEN)x[i],p);
      return y;
    default: return x;
    }
}


/* This code was kindly written for us by Richard Schroeppel */


/* Note that PARI's idea of the maximum possible coefficient involves the
limit on the degree (klim).  Consider revising this.
If I don't respect the degree limit when testing potential factors,
there's the possibility that I might identify a high degree factor that
isn't irreducible, because it's lower degree divisors were missed because
they had a coefficient outside the Borne limit for klim, but the higher
degree factor had it's coefficients within Borne.  This would still have
the property that any factors of degree <= klim were guaranteed irr, but
higher degrees (> 2*klim) might not be irr. */



/* the subroutine:
   fxn points at the first unconsidered factor for the current combination
   psf is the product-so-far, or 0 for a null product
   dlim is the degree limit remaining for unconsidered divisors
   other arguments are "global" and must already be setup
   as factors are found, they are put in cmbf_fax; the count is kept in
   cmbf_faxn; and they are divided out of cmbf_target; the degree and
   leading coefficient are updated; and the constituent modular factors
   are deleted from cmbf_modfax.
   exit value is 1 if any factors are found.
   If psf is 0, all factors made up from pieces at or after fxn will be
   found & removed.  If psf is not 0, only the factor which is a
   continuation of psf will be found.
  */

int combine_factors(long fxn, GEN psf, long dlim)
{
  int val=0, val2=0;  /* assume failure */
  GEN newf, newpsf, quo, rem, cont; long newd;

  if (dlim <= 0) return 0;
  if (fxn > cmbf_modfaxn) return 0;
  /* first, try deeper factors without considering the current one */
  if (fxn < cmbf_modfaxn) { val = combine_factors(fxn+1,psf,dlim);
			    if (val&&psf) return val; };
  /* second, try including the current modular factor in the product */
  newf = (GEN)cmbf_modfax[fxn];
  if (!newf) return val;  /* modular factor already used */
  newd = lgef(newf)-3;
  if (newd > dlim) return val;  /* degree of new factor is too large */
  if (psf) { newpsf = centermod(gmul(psf,newf),cmbf_mod); }
     else { newpsf = centermod(gmul(cmbf_abslc,newf),cmbf_mod); };
  /* try out the new combination */
  quo = poldivres(cmbf_abslcxtarget,newpsf,&rem);
  if (!signe(rem))  /* found a factor */
   { 
     cont = content(newpsf);  /* hope this is always positive */
     newpsf = gdiv(newpsf,cont);
     cmbf_fax[++cmbf_faxn] = (long)newpsf; /* store factor */
     cmbf_modfax[fxn] = 0; /* remove used modular factor */
     /* fix up target */
     cmbf_target = gdiv(quo,(GEN)newpsf[lgef(newpsf)-1]);
     cmbf_degree = lgef(cmbf_target)-3;
     cmbf_lc = (GEN)cmbf_target[cmbf_degree+2];  /* leading coefficient */
     cmbf_abslc = gabs(cmbf_lc); /* |lc| */
     cmbf_abslcxtarget = gmul(cmbf_abslc,cmbf_target); /* abslc * target */
     return 1;
   }
  /* newpsf needs more; try for it */
  if (newd == dlim) return val;  /* no more room in degree limit */
  if (fxn == cmbf_modfaxn) return val;  /* no more modular factors to try */
  val2 = combine_factors(fxn+1,newpsf,dlim-newd);
  if (val2) cmbf_modfax[fxn] = 0; /* remove used modular factor */
  return val||val2;
}


GEN  squff(GEN a, long klim)
{
  GEN borne,di,p1,p2,y,g,pe,pg,s,t,u,v,w,xmod,unmod,polxmodp;
  GEN ae,be,aprov,pev;
  GEN tabd[NOMBDEP][500],unmodp[NOMBDEP];
  unsigned long tabbit[60],tabkbit[60],tablbit[60],j1,rem,pro;
  byteptr pt=diffptr;
  long av=avma,tetpil,p=0,tabp[NOMBDEP],nfacp[NOMBDEP];
  long va=varn(a),da=lgef(a)-3,lbit,k,d0,fla,i,j,d,e,d1,d2;
  long np,nmax,imax,kd,lgg,nf,ev,nfd,nft;
  
  di=discsr(a);np=0;lbit=(da>>4)+1;nmax=da+1;imax=0;kd=da>>1;
  if((!klim)||(klim>kd)) klim=kd;
  while(np<NOMBDEP)
    {
      p+= *pt++;
      if(signe(gmodgs(di,p))&&signe(gmodgs((GEN)a[da+2],p)))
     /* require p doesn't divide leading coefficient */
	{
	  tabp[np]=p;unmodp[np++]=gmodulcp(gun,stoi(p));
	}
    }
  for(i=0;i<NOMBDEP;i++)
    {
      p=tabp[i];nfacp[i]=0;unmod=unmodp[i];
      tabkbit[lbit-1]=1;for(j=0;j<lbit-1;j++) tabkbit[j]=0;
      d=0;v=gmul(unmod,a);xmod=gmodulcp(polxmodp=gmul(unmod,polx[va]),v);w=xmod;
      while(d<(e=(lgef(v)-3))>>1)
	{
	  d++;
	  w=gpuigs(w,p);p1=(GEN)(gsub(w,xmod))[2];
	  g=ggcd(p1,v);
	  tabd[i][d]=g;lgg=lgef(g)-3;
	  if(lgg>0)
	    {
	      v=gdiv(v,g);w=gmodulcp(gmod((GEN)w[2],v),v);
	      xmod=gmodulcp(polxmodp,v);
	      nfacp[i]+=(lgg/d);
	      for(kd=d;kd<=lgg;kd+=d)
		{
		  d1=d>>4;d2=d&15;rem=0;
		  for(j=0;j<d1;j++) tablbit[lbit-1-j]=0;
		  for(j=d1;j<lbit;j++)
		    {
		      pro=tabkbit[lbit-1-j+d1]<<d2;
		      tablbit[lbit-1-j]=(pro&0xffff)+rem;rem=pro>>16;
		    }
		  for(j=0;j<lbit;j++) tabkbit[j] |= tablbit[j];
		}
	    }
	}
      if(e>0)
	{
	  tabd[i][e]=v;nfacp[i]++;
	  d1=e>>4;d2=e&15;rem=0;
	  for(j=0;j<d1;j++) tablbit[lbit-1-j]=0;
	  for(j=d1;j<lbit;j++)
	    {
	      pro=tabkbit[lbit-1-j+d1]<<d2;
	      tablbit[lbit-1-j]=(pro&0xffff)+rem;rem=pro>>16;
	    }
	  for(j=0;j<lbit;j++) tabkbit[j] |= tablbit[j];
	}
      if(nfacp[i]<nmax) {nmax=nfacp[i];imax=i;}
      for(j=d+1;j<e;j++) tabd[i][j]=polun[va];
      if(i)
	for(j=0;j<lbit;j++) tabbit[j] &= tabkbit[j];
      else
	{
	  for(j=0;j<lbit;j++) tabbit[j] = tabkbit[j];
	}
      fla=0;j1=1;
      for(j=lbit-1;(j>=0)&&(!fla);j--)
	{
	  for(k=0;(k<=15)&&(!fla);k++)
	    {
	      fla=tabbit[j]&j1;
	      if((!k)&&(j==lbit-1)) fla=0;
	      j1<<=1;
	    }
	  j1=1;
	}
      d0=((lbit-j-2)<<4)+k-1;
      if((d0==da)||(d0>klim))
	{
	  tetpil=avma;y=cgetg(2,18);y[1]=lcopy(a);
	  return gerepile(av,tetpil,y);
	}
    }
  p1=gzero;
  for(i=2;i<=da+2;i++) p1=gadd(p1,gmul((GEN)a[i],(GEN)a[i]));
  p1=gadd(p2=gabs((GEN)a[da+2],4),gaddsg(1,racine(p1)));
  borne=gmul(p1,binome(stoi(da-1),klim));
  p=tabp[imax];unmod=unmodp[imax];
  e=itos(gceil(gdiv(glog(gmul2n(gmul(p2,borne),1),4),glog(pg=stoi(p),4))));
  pev=gpuigs(pg,e);
  y=cgetg((nf=nfacp[imax])+1,18);k=0;
  p1=cgetg(nf+1,18);nft=0;
  for(d=1;nft<nf;d++)
    {
      g=tabd[imax][d];g=gdiv(g,(GEN)g[lgef(g)-1]);lgg=lgef(g)-3;
      if(lgg)
	{
	  nfd=lgg/d;p1[nft+1]=(long)g;
	  split(p,(GEN*)(p1+nft+1),d,p,shifti(gpuigs(pg,d),-1));
	  nft+=nfd;
	}
    }
/* do a Hensel lift */
  aprov=a;
  for(i=1;i<=nf-1;i++)
    {
      pe=pg;ev=1;p2=(GEN)p1[i];
      ae=gdiv(p2,(GEN)p2[lgef(p2)-1]);be=gdiv(aprov,ae);
      g=bezoutpol(ae,be,&u,&v);
      if(isnonscalar(g)) err(henser1);
      u=gdiv(u,(GEN)g[2]);v=gdiv(v,(GEN)g[2]);
      for(j=2;j<lgef(ae);j++) ae[j]=(long)releve((GEN)ae[j]);
      for(j=2;j<lgef(be);j++) be[j]=(long)releve((GEN)be[j]);
      for(j=2;j<lgef(u);j++) u[j]=(long)releve((GEN)u[j]);
      for(j=2;j<lgef(v);j++) v[j]=(long)releve((GEN)v[j]);
      do
	{
	  g=gdiv(gsub(aprov,gmul(ae,be)),pe);
	  t=poldivres(gmul(v,g),ae,&s);for(j=2;j<lgef(s);j++) s[j]=lmod((GEN)s[j],pe);
	  ae=gadd(ae,gmul(pe,s));
	  s=gadd(gmul(u,g),gmul(t,be));for(j=2;j<lgef(s);j++) s[j]=lmod((GEN)s[j],pe);
	  be=gadd(be,gmul(pe,s));
	  g=gdiv(gsub(gun,gadd(gmul(u,ae),gmul(v,be))),pe);
	  t=poldivres(gmul(v,g),ae,&s);
	  for(j=2;j<lgef(s);j++)
	    s[j]=lmod((GEN)s[j],pe);
	  v=gadd(v,gmul(pe,s));
	  s=gadd(gmul(u,g),gmul(t,be));for(j=2;j<lgef(s);j++) s[j]=lmod((GEN)s[j],pe);
	  u=gadd(u,gmul(pe,s));
	  pe=gsqr(pe);ev<<=1;
	}
      while(ev<e);
      p1[i]=(long)ae;if(i<nf-1) aprov=gdeuc(aprov,ae);
    }
  unmod=gmodulcp(gun,pev);
  p1[nf]=(long)centermod(gmul(be,(GEN)ginv(gmul((GEN)be[lgef(be)-1],unmod))[2]),pev);
  for(i=1;i<=nf;i++) 
    {g=(GEN)p1[i];if(!gcmp1((GEN)g[lgef(g)-1])) err(factpoler2);}
  cmbf_target = a;  /* target poly.  Assumes content removed  */
  cmbf_degree = lgef(cmbf_target)-3;
  cmbf_lc = (GEN)cmbf_target[cmbf_degree+2];  /* leading coefficient */
  cmbf_abslc = gabs(cmbf_lc); /* |lc| */
  cmbf_abslcxtarget = gmul(cmbf_abslc,cmbf_target); /* abslc * target */
  cmbf_mod = pev;   /* Modulus */
  cmbf_modfax = p1;  /* array of modular factors.  Each has LC 1.
			1 based indexing.  Product should be congruent to a/lc(a).  */
  cmbf_modfaxn = nf;  /* number of modular factors */
  cmbf_fax = cgetg(nf+2,18);
  /* Result array.  Extra cell for leftover constant. */
  cmbf_faxn = 0; /* pointer into result array; last used cell;
		    # of factors found */
  
  /* sorting factors decreasing by degree helps if klim is used */
  /* if klim isn't used, can start with first arg of 2 instead of 1,
     saving some time */
  combine_factors(1,0,klim);  /* the call */

  /* follow-up */

  if (signe(cmbf_lc)<0) cmbf_target = gneg(cmbf_target);
  if (cmbf_degree) cmbf_fax[++cmbf_faxn] = (long)cmbf_target; /* leftover factor */
/*  if (signe(cmbf_lc)<0) cmbf_fax[++cmbf_faxn] = stoi(-1); */
  tetpil=avma;y=cgetg(cmbf_faxn+1,18);
  for(i=1;i<=cmbf_faxn;i++) y[i]=lcopy((GEN)cmbf_fax[i]);
  return gerepile(av,tetpil,y);
}
  
GEN factpol(GEN x, long klim)
        
            

/* klim=0 habituellement, sauf si l'on ne veut chercher que les facteurs de degre <= klim */

{
  long av=avma,av2,lx,vv,k,i,j,i1,f,nbfac;
  GEN res,fa,p1,p2,y,d,a,ap,t,v,w;
  
  if((typ(x)!=10)||(!signe(x))) err(factpoler1);
  y=cgetg(3,19);if((lx=lgef(x))==3) 
    {y[1]=lgetg(1,18);y[2]=lgetg(1,18);return y;}
  if(lx==4)
    {
      p1=cgetg(2,18);y[1]=(long)p1;p1[1]=lcopy(x);
      p1=cgetg(2,18);y[2]=(long)p1;p1[1]=un;return y;
    }
  fa=cgetg(lx,17);for(i=1;i<lx;i++) fa[i]=zero;
  d=content(x);vv=varn(x);a=gdiv(x,d);ap=deriv(a,vv);t=ggcd(a,ap);v=gdiv(a,t);
  w=gdiv(ap,t);j=0;f=1;nbfac=0;
  while(f)
    {
      j++;w=gsub(w,deriv(v,vv));f=signe(w);
      if(f)
	{
	  res=ggcd(v,w);v=gdiv(v,res);w=gdiv(w,res);
	}
      else res=v;
      fa[j]=(lgef(res)>3) ? (long)squff(res,klim) : lgetg(1,18);
      nbfac+=(lg((GEN)fa[j])-1);
    }
  av2=avma;y=cgetg(3,19);p1=cgetg(nbfac+1,18);y[1]=(long)p1;
  p2=cgetg(nbfac+1,18);y[2]=(long)p2;
  for(i=1,k=0;i<=j;i++)
    for(i1=1;i1<lg((GEN)fa[i]);i1++)
      {
	p1[++k]=lcopy((GEN)((GEN)fa[i])[i1]);p2[k]=lstoi(i);
      }
  return gerepile(av,av2,y);
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~                                                                     ~*/
/*~                            FACTORISATION                            ~*/
/*~                                                                     ~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

GEN   factor(GEN x)
{
  long tx=typ(x),l,tetpil,i;
  GEN  y,p1,p2,a0,p,p3,p4,p5;

  if(gcmp0(x))
    {
      y=cgetg(3,19);p1=cgetg(2,18);p2=cgetg(2,18);y[1]=(long)p1;
      y[2]=(long)p2;p1[1]=zero;p2[1]=un;return y;
    }
  switch(tx)
  {
  case 1 : y=decomp(x);break;
  case 5 : l=avma;x=gred(x);
  case 4 : if(tx==4) l=avma;p1=decomp((GEN)x[1]);
    p2=decomp((GEN)x[2]);p4=concat((GEN)p1[1],(GEN)p2[1]);
    p5=concat((GEN)p1[2],gneg((GEN)p2[2]));p3=indexsort(p4);
    tetpil=avma;y=cgetg(3,19);y[1]=(long)extract(p4,p3);
    y[2]=(long)extract(p5,p3);y=gerepile(l,tetpil,y);break;
  case 10 : p1=content(x);if(!gcmp1(p1)) x=gdiv(x,p1);
    a0=(GEN)((GEN)x[2]);
    if(typ(a0)==3)
    {
      p=(GEN)(a0[1]);y=factmod(x,p);
    }
    else
    {
      if(typ(a0)==1) y=factpol(x,0);
      else err(impl,"factor of general polynomial");
    }
    break;
  case 14 : l=avma;x=gred(x);
  case 13 : if(tx==13) l=avma;p1=factor((GEN)x[1]);
    p2=factor((GEN)x[2]);p3=gneg((GEN)p2[2]);tetpil=avma;
    y=cgetg(3,19);y[1]=lconcat((GEN)p1[1],(GEN)p2[1]);
    y[2]=lconcat((GEN)p1[2],p3);
    y=gerepile(l,tetpil,y);break;
  case 17:
  case 18:
  case 19: l=lg(x);y=cgetg(l,tx);
    for(i=1;i<l;i++) y[i]=(long)factor((GEN)x[i]);break;
  default: err(impl,"general factorization");
  }
  return y;
}



/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*                                                                 */
/*                         PGCD GENERAL                            */
/*                                                                 */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

GEN scalcontent(GEN x);

GEN   ggcd(GEN x, GEN y)
{
  long l,l1,tetpil,i;
  long tx=typ(x),ty=typ(y),vx,vy;
  GEN p1,p2,z;

  if(tx>ty) {p1=x;x=y;y=p1;l=tx;tx=ty;ty=l;}
  if(ty>=17)
    {l=lg(y);z=cgetg(l,ty);for(i=1;i<l;i++) z[i]=lgcd(x,(GEN)y[i]);return z;}
  if(gcmp0(x)) return gcopy(y);
  if(gcmp0(y)) return gcopy(x);
  l=avma;
  if(tx<9)
    {
      if(ty<9)
	switch(tx)
	  {
	  case 1 : switch(ty)
	    {
	    case 1 : z=mppgcd(x,y);break;
	    case 3 : z=cgetg(3,3);
	      l=avma;p1=mppgcd((GEN)y[1],(GEN)y[2]);
	      if(!gcmp1(p1)) {tetpil=avma;p1=gerepile(l,tetpil,mppgcd(x,p1));}
	      z[2]=(long)p1;z[1]=copyifstack((GEN)y[1]);
	      break;
	    case 4 :
	    case 5 : z=cgetg(3,4);z[2]=lcopy((GEN)y[2]);
	      z[1]=lmppgcd(x,(GEN)y[1]);gredsp(&z);
	      break;
	    case 7 : z=gpuigs((GEN)y[2],min(valp(y),ggval(x,(GEN)y[2])));break;
	    case 6 :
	    case 8 : p1=scalcontent(y);tetpil=avma;
	      z=gerepile(l,tetpil,ggcd(x,p1));break;
	    default: z=gun;
	    } break;
	  case 3 : 
	    switch(ty)
	      {
	      case 3 : 
		z=cgetg(3,3);
		z[1]=gegal((GEN)x[1],(GEN)y[1]) ? copyifstack((GEN)x[1]):(long)mppgcd((GEN)x[1],(GEN)y[1]);
		if(gcmp1((GEN)z[1])) z[2]=zero;
		else
		  {
		    l=avma;p1=mppgcd((GEN)z[1],(GEN)x[2]);
		    if(!gcmp1(p1))
		      {
			tetpil=avma;
			p1=gerepile(l,tetpil,mppgcd(p1,(GEN)y[2]));
		      }
		    z[2]=(long)p1;
		  } break;
	      case 4 :
		p1=mppgcd((GEN)x[1],(GEN)y[2]);
		if(!gcmp1(p1)) err(gcder1);
		tetpil=avma;z=gerepile(l,tetpil,ggcd((GEN)y[1],x));
		break;
	      case 5 : p1=gred(y);tetpil=avma;
		z=gerepile(l,tetpil,ggcd(p1,z));
		break;
	      case 7 : z=gpuigs((GEN)y[2],min(valp(y),ggval(x,(GEN)y[2])));break;
	      case 6 :
	      case 8 : p1=scalcontent(y);tetpil=avma;
	      z=gerepile(l,tetpil,ggcd(x,p1));break;
	      default: z=gun;
	      } break;
	  case 4 :
	  case 5 : 
	    switch(ty)
	      {
	      case 4:
	      case 5: z=cgetg(3,4);z[2]=lmulii((GEN)x[2],(GEN)y[2]);l=avma;
		p1=mulii((GEN)x[1],(GEN)y[2]);p2=mulii((GEN)x[2],(GEN)y[1]);tetpil=avma;
		z[1]=lpile(l,tetpil,mppgcd(p1,p2));
		gredsp(&z);break;
	      case 6 :
	      case 8 : p1=scalcontent(y);tetpil=avma;
		z=gerepile(l,tetpil,ggcd(x,p1));break;
	      case 7 : z=gpuigs((GEN)y[2],min(valp(y),ggval(x,(GEN)y[2])));break;
	      default: z=gun;
	      }break;
	  case 6:
	    switch(ty)
	      {
	      case 6 : p1=gdiv(x,y);
		if(gcmp0((GEN)p1[2]))
		  {
		    tetpil=avma;p1=(GEN)p1[1];
		    switch(typ(p1))
		      {
		      case 1: z=gerepile(l,tetpil,gcopy(y));break;
		      case 4:
		      case 5: z=gerepile(l,tetpil,gdiv(y,(GEN)p1[2]));break;
		      default: avma=l;z=gun;
		      }
		  }
		else
		  {
		    if((typ((GEN)p1[1])==1)&&(typ((GEN)p1[2])==1))
		      {tetpil=avma;z=gerepile(l,tetpil,gcopy(y));}
		    else
		      {
			p1=gdiv(y,x);
			if((typ((GEN)p1[1])==1)&&(typ((GEN)p1[2])==1))
			  {tetpil=avma;z=gerepile(l,tetpil,gcopy(x));}
			else
			  {
			    p1=scalcontent(y);tetpil=avma;
			    z=gerepile(l,tetpil,ggcd(x,p1));
			  }
		      }
		  } break;
	      case 7 :
	      case 8 : p1=scalcontent(x);tetpil=avma;
		z=gerepile(l,tetpil,ggcd(p1,y));break;
	      } break;
	  case 7: 
	    switch(ty)
	      {
	      case 7 : 
		if(!gegal((GEN)x[2],(GEN)y[2])) z=gun;
		else z=gpuigs((GEN)y[2],min(valp(y),valp(x)));break;
	      case 8 : p1=scalcontent(y);tetpil=avma;
		z=gerepile(l,tetpil,ggcd(p1,x));break;
	      default: z=gun;
	      } break;
	  case 8 : p1=gdiv(x,y);
	    if(gcmp0((GEN)p1[3]))
	      {
		tetpil=avma;p1=(GEN)p1[2];
		z=gerepile(l,tetpil,(typ(p1)==1)?gcopy(y):gdiv(y,(GEN)p1[2]));
	      }
	    else
	      {
		if((typ((GEN)p1[2])==1)&&(typ((GEN)p1[3])==1))
		  {tetpil=avma;z=gerepile(l,tetpil,gcopy(y));}
		else
		  {
		    p1=gdiv(y,x);
		    if((typ((GEN)p1[2])==1)&&(typ((GEN)p1[3])==1))
		      {tetpil=avma;z=gerepile(l,tetpil,gcopy(x));}
		    else
		      {
			p1=scalcontent(y);tetpil=avma;
			z=gerepile(l,tetpil,ggcd(p1,x));
		      }
		  }
	      } break;
	  default: z=gun;
	  }
      else {p1=content(y);tetpil=avma;z=gerepile(l,tetpil,ggcd(x,p1));}
    }
  else /* ici tx et ty>=9 */
    {
      vx=gvar9(x);vy=gvar9(y);
      if(vy<vx) 
	{p1=content(y);tetpil=avma;return gerepile(l,tetpil,ggcd(p1,x));}
      if(vx<vy) 
	{p1=content(x);tetpil=avma;return gerepile(l,tetpil,ggcd(p1,y));}
      switch(tx)
	{
	case 9 : switch(ty)
	  {
	  case 9 : z=cgetg(3,9);
	    z[1]=gegal((GEN)x[1],(GEN)y[1]) ? copyifstack((GEN)x[1]):(long)ggcd((GEN)x[1],(GEN)y[1]);
	    if(lgef((GEN)z[1])<=3) z[2]=zero;
	    else
	      {
		l=avma;p1=ggcd((GEN)z[1],(GEN)x[2]);
		if(lgef(p1)>3) {tetpil=avma;p1=gerepile(l,tetpil,ggcd(p1,(GEN)y[2]));}
		z[2]=(long)p1;
	      } break;
	  case 10: z=cgetg(3,9);z[1]=copyifstack((GEN)x[1]);
	    l=avma;p1=ggcd((GEN)x[1],(GEN)x[2]);
	    if(lgef(p1)>3) {tetpil=avma;p1=gerepile(l,tetpil,ggcd(y,p1));}
	    z[2]=(long)p1;
	    break;
	  case 13:
	    p1=ggcd((GEN)x[1],(GEN)y[2]);
	    if(!gcmp1(p1)) err(gcder1);
	    tetpil=avma;z=gerepile(l,tetpil,ggcd((GEN)y[1],x));
	    break;
	  case 14 : p1=gred(y);tetpil=avma;
	    z=gerepile(l,tetpil,ggcd(p1,z));
	    break;
	  default: err(gcder4);
	  } break;
	  
	case 10: switch(ty)
	  {
	  case 10: z=polgcd(x,y);break;
	  case 11: z=gpuigs(polx[vx],min(valp(y),gval(x,vx)));
	    break;
	  case 13: z=cgetg(3,13);z[2]=y[2];z[1]=lgcd(x,(GEN)y[1]);
	    tetpil=avma;z=gerepile(l,tetpil,gred(z));
	    break;
	  case 14: z=cgetg(3,13);z[2]=lcopy((GEN)y[2]);z[1]=lgcd(x,(GEN)y[1]);break;
	  default: err(gcder2);
	  } break;
	case 11 : switch(ty)
	  {
	  case 11: z=gpuigs(polx[vx],min(valp(x),valp(y)));
	    break;
	  case 13:
	  case 14: z=gpuigs(polx[vx],min(valp(x),gval(y,vx)));
	    break;
	  default: err(gcder2);
	  } break;
	case 13 :
	case 14 : if(ty>=15) err(gcder3);
	  z=cgetg(3,13);
	  z[2]=lmul((GEN)x[2],(GEN)y[2]);l1=avma;
	  p1=gmul((GEN)x[1],(GEN)y[2]);
	  p2=gmul((GEN)x[2],(GEN)y[1]);tetpil=avma;
	  z[1]=lpile(l1,tetpil,ggcd(p1,p2));
	  if(ty==13) {tetpil=avma;z=gerepile(l,tetpil,gred(z));}
	  break;
	default: err(gcder4);
	}
    }
  return z;
}

GEN   glcm(GEN x, GEN y)
{
  long av=avma,tetpil,tx=typ(x),ty=typ(y),i,l;
  GEN p1,p2,z;

  if(ty>=17)
    {l=lg(y);z=cgetg(l,typ(y));for(i=1;i<l;i++) z[i]=(long)glcm(x,(GEN)y[i]);return z;}
  if(tx>=17)
    {l=lg(x);z=cgetg(l,typ(x));for(i=1;i<l;i++) z[i]=(long)glcm((GEN)x[i],y);return z;}
  if(gcmp0(x)) return gzero;
  p1=ggcd(x,y);p2=gmul(x,y);
  tetpil=avma;return gerepile(av,tetpil,gdiv(p2,p1));
}

GEN   polgcdnun(GEN x, GEN y)
{
  long  l,tetpil,tetpil2;
  GEN z,p1,p2,p3;

  if(gcmp0(y)) z=gcopy(x);
  else
  {
    p1=x;p2=y;l=avma;tetpil=0;
    while(!gcmp0(p2))
    {
      tetpil2=tetpil;tetpil=avma;p3=gres(p1,p2);
      p1=p2;p2=p3;
    }
    if(tetpil2)
    {
      avma=tetpil;
      if(tetpil2!=l) z=gerepile(l,tetpil2,p1);
      else z=p1;
    }
    else  {avma=l;z=gcopy(y);}
  }
  return z;
}

int typecorps(GEN x)
           
/* renvoie 1 si probablement un corps simple, 0 sinon */

{
  switch(typ(x))
    {
    case 2:
    case 3:
    case 7:
    case 11: return 1;
    case 6: return typecorps((GEN)x[1])|typecorps((GEN)x[2]);
    default: return 0;
    }
}

GEN    polgcd(GEN x, GEN y)
{
  GEN  p1,p2;
  long l,tetpil,v,e,lx,ty;

  if((typ(x)!=10)||(typ(y)!=10)) err(polgcder1);
  if(!signe(y)) return gcopy(x);
  if(!signe(x)) return gcopy(y);
  l=avma;
  if((v=varn(x))==varn(y))
  {
    if(ismonome(x))
    {
      lx=lgef(x);e=gval(y,v);if((lx-3)<e) e=lx-3;
      p1=ggcd((GEN)x[lgef(x)-1],content(y));p2=gpuigs(polx[v],e);
      tetpil=avma;return gerepile(l,tetpil,gmul(p1,p2));
    }
    if(ismonome(y))
    {
      lx=lgef(y);e=gval(x,v);if((lx-3)<e) e=lx-3;
      p1=ggcd((GEN)y[lgef(y)-1],content(x));p2=gpuigs(polx[v],e);
      tetpil=avma;return gerepile(l,tetpil,gmul(p1,p2));
    }
  }
  if(typecorps((GEN)x[lgef(x)-1])||typecorps((GEN)y[lgef(y)-1]))
    p1=polgcdnun(x,y);
  else p1=srgcd(x,y);
  if(gcmp0(p1)) return p1;
  if(typ(p1)==10) 
    {
      ty=typ((GEN)p1[lgef(p1)-1]);
      if((ty==3)||(ty>5)) return p1;
      if(gsigne((GEN)p1[lgef(p1)-1])<0) 
	{tetpil=avma;return gerepile(l,tetpil,gneg(p1));}
      else return p1;
    }
  else
    {
      tetpil=avma;return gerepile(l,tetpil,gmul(polun[v],p1));
    }
}

      
      
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~                                                                     ~*/
/*~                           BEZOUT GENERAL                            ~*/
/*~                                                                     ~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

GEN   gbezout(GEN x, GEN y, GEN *u, GEN *v)
{
  long tx=typ(x),ty=typ(y);

  if(ty==1)
  {
    if(tx==1) return bezout(x,y,u,v);
    if(tx==10) {*u=gzero;*v=gdivsg(1,y);return gun;}
    else err(bezoutpoler);
  }
  if(ty==10)
  {
    if(tx==10) return bezoutpol(x,y,u,v);
    if(tx<10) {*u=gdivsg(1,x);*v=gzero;return gun;}
    else err(bezoutpoler);
  }
  if(ty<10)
  {
    if(tx==10) {*u=gzero;*v=gdivsg(1,y);return gun;}
    else err(bezoutpoler);
  }
  err(bezoutpoler);return gnil;
}

GEN   vecbezout(GEN x, GEN y)
{
  GEN z;

  z=cgetg(4,17);z[3]=(long)gbezout(x,y,(GEN*)(z+1),(GEN*)(z+2));
  return z;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*                                                                 */
/*                    CONTENU ET PARTIE PRIMITIVE                  */
/*                                                                 */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


GEN   content(GEN x)
{
  long  l,lx,tetpil,i,f,tx=typ(x);
  GEN p1,p2;

  if(tx<9) return gcopy(x);
  lx=lg(x);
  switch(tx)
    {
    case 9: return content((GEN)x[2]);
    case 13:
    case 14: l=avma;p1=content((GEN)x[1]);p2=content((GEN)x[2]);
      tetpil=avma;return gerepile(l,tetpil,gdiv(p1,p2));
    case 19: if(lx==1) return gun;
      tetpil=l=avma;p1=content((GEN)x[1]);
      for(i=2;i<lx;i++) {tetpil=avma;p1=ggcd(p1,content((GEN)x[i]));}
      return gerepile(l,tetpil,p1);
    case 10: lx=lgef(x);
    case 11: if((!signe(x))&&((tx==11)||(lx==2))) return gzero;
    default: f=1;
      if(tx!=15) {for(i=lontyp[tx];(i<lx)&&f;i++) f=(typ((GEN)x[i])==1);i=lx-1;}
      else i=lx-2;
      p1=(GEN)x[i];l=avma;
      if(f)
	{
	  while((i>lontyp[tx])&&(!gcmp1(p1)))
	    {--i;tetpil=avma;p1=mppgcd(p1,(GEN)x[i]);}
	}
      else
	{
	  i--;for(;i>=lontyp[tx];i--)
	    {
	      tetpil=avma;p1=ggcd(p1,(GEN)x[i]);
	    }
	}
      if(l==avma) return gcopy(p1);
      else return gerepile(l,tetpil,p1);
    }
}

GEN scalcontent(GEN x)
{
  long tx=typ(x);
  switch(tx)
    {
    case 1:
    case 4:
    case 5: return gcopy(x);
    case 2:
    case 7: return gun;
    case 3:
    case 6: return ggcd((GEN)x[1],(GEN)x[2]);
    case 8: return ggcd((GEN)x[2],(GEN)x[3]);
    case 9: return content((GEN)x[2]);
    default: return content(x);
    }
}

GEN   primpart(GEN x)
{
  long  l,tetpil;
  GEN z,p1;

  if(!signe(x)) z=gcopy(x);
  else
  {
    l=avma;p1=content(x);
    if(gcmp1(p1)) {avma=l;z=gcopy(x);}
    else
    {
      tetpil=avma;
      z=gerepile(l,tetpil,gdiv(x,p1));
    }
  }
  return z;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*                                                                 */
/*                         SOUS RESULTANT                          */
/*                                                                 */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


GEN   subres(GEN x, GEN y)
{
  long  degq,av,tetpil,f,tx=typ(x),ty=typ(y),dx,dy,du,dv,dr,signh;
  GEN z,g,h,r,p1,p2,p3,p4,u,v;

  if(gcmp0(x)||gcmp0(y)) return gzero;
  if((tx<10)||(ty<10))
    {
      if(tx==10) return gpuigs(y,lgef(x)-3);
      if(ty==10) return gpuigs(x,lgef(y)-3);
      else return gun;
    }
  if((tx!=10)||(ty!=10)) err(subrer1);
  if(varn(x)!=varn(y))
    return (varn(x)<varn(y))?gpuigs(y,lgef(x)-3):gpuigs(x,lgef(y)-3);
  dx=lgef(x);dy=lgef(y);
  if (dx<dy)
  {p1=x;x=y;y=p1;f=dx;dx=dy;dy=f;}
  av=avma;p4=content(y);
  if(dy==3) {tetpil=avma;return gerepile(av,tetpil,gpuigs(p4,dx-3));}
  p3=content(x);
/*  p3=gun;p4=gun; */
  u=gdiv(x,p3);v=gdiv(y,p4);
  g=gun;h=gun;f=1;signh=1;
  while (f)
  {
    du=lgef(u);dv=lgef(v);degq=du-dv;
    r=psres(u,v);dr=lgef(r);
    if(dr<=2) f=0;
    else
    {
      u=v;
      if(degq==1) p1=gmul(h,g);
      else p1=gmul(gpuigs(h,degq),g);
      v=gdiv(r,p1);
      g=(GEN)u[lgef(u)-1];
      if(degq==1) h=g;
      else if(degq)
      {
        p1=gpuigs(g,degq);p2=gpuigs(h,degq-1);
        h=gdiv(p1,p2);
      }
      if(((du-3)*(dv-3))&1) signh= -signh;
      if(dr==3) f=0;
    }
  }
  if(dr==2) {z=gzero;avma=av;}
  else
  {
    if(dv==4)
    {
      tetpil=avma;p2=gcopy((GEN)v[2]);
    }
    else
    {
      if(dv-3)
      {
        p1=gpuigs((GEN)v[2],dv-3);tetpil=avma;
        p2=gdiv(p1,gpuigs(h,dv-4));
      }
      else
      {
        tetpil=avma;p2=gcopy(h);
      }
    }
    if(!gcmp1(p3))
    {
      p1=gpuigs(p3,dy-3);tetpil=avma;p2=gmul(p2,p1);
    }
    if(!gcmp1(p4))
    {
      p1=gpuigs(p4,dx-3);tetpil=avma;p2=gmul(p2,p1);
    }
    if(signh<0) {tetpil=avma;p2=gneg(p2);}
    z=gerepile(av,tetpil,p2);
  }
  return z;
}


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*                                                                 */
/*               RESULTANT PAR MATRICE DE SYLVESTER                */
/*                                                                 */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

GEN resultant2(GEN x, GEN y)
{
  long av=avma,tetpil,dx,dy,i,j,tx=typ(x),ty=typ(y),f;
  GEN p1,p2;


  if(gcmp0(x)||gcmp0(y)) return gzero;
  if((tx<10)||(ty<10))
    {
      if(tx==10) return gpuigs(y,lgef(x)-3);
      if(ty==10) return gpuigs(x,lgef(y)-3);
      else return gun;
    }
  if((tx!=10)||(ty!=10)) err(subrer1);
  if(varn(x)!=varn(y))
    return (varn(x)<varn(y))?gpuigs(y,lgef(x)-3):gpuigs(x,lgef(y)-3);
  dx=lgef(x)-3;dy=lgef(y)-3;
  if (dx<dy)
  {p1=x;x=y;y=p1;f=dx;dx=dy;dy=f;}
  p1=cgetg(dx+dy+1,19);
  for(j=1;j<=dy;j++)
    {
      p2=cgetg(dx+dy+1,18);p1[j]=(long)p2;
      for(i=1;i<j;i++) p2[i]=zero;
      for(i=j;i<=j+dx;i++) p2[i]=x[dx-i+j+2];
      for(i=j+dx+1;i<=dx+dy;i++) p2[i]=zero;
    }
  for(j=1;j<=dx;j++)
    {
      p2=cgetg(dx+dy+1,18);p1[j+dy]=(long)p2;
      for(i=1;i<j;i++) p2[i]=zero;
      for(i=j;i<=j+dy;i++) p2[i]=y[dy-i+j+2];
      for(i=j+dy+1;i<=dx+dy;i++) p2[i]=zero;
    }
  tetpil=avma;return gerepile(av,tetpil,det(p1));
}


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*                                                                 */
/*           P.G.C.D PAR L'ALGORITHME DU SOUS RESULTANT            */
/*                                                                 */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


GEN  srgcd(GEN x, GEN y)
{
  long  degq,av,av1,tetpil,f,vx,tx=typ(x),ty=typ(y),dx,dy,du,dv,dr;
  GEN d,g,h,r,p1,p2,p3,p4,u,v;

  if(gcmp0(x)||gcmp0(y)) return gzero;
  if((tx<10)||(ty<10)) return gun;
  if((tx!=10)||(ty!=10)) err(subrer1);
  if((vx=varn(x))!=varn(y)) return gun;
  dx=lgef(x);dy=lgef(y);
  if (dx<dy) {p1=x;x=y;y=p1;f=dx;dx=dy;dy=f;}
  av=avma;p3=content(x);p4=content(y);d=ggcd(p3,p4);
  tetpil=avma;d=gmul(d,polun[vx]);
  if(dy==3) return gerepile(av,tetpil,d);
  av1=avma;u=gdiv(x,p3);v=gdiv(y,p4);g=gun;h=gun;f=1;
  while (f==1)
  {
    du=lgef(u);dv=lgef(v);degq=du-dv;
    r=psres(u,v);dr=lgef(r);
    if(dr<=3) {f=(dr==3)?2:0;}
    else
    {
      u=v;
      if(degq==1) p1=gmul(h,g);
      else p1=gmul(gpuigs(h,degq),g);
      v=gdiv(r,p1);
      g=(GEN)u[lgef(u)-1];
      if(degq==1) h=g;
      else if(degq)
        {
          p1=gpuigs(g,degq);p2=gpuigs(h,degq-1);
          h=gdiv(p1,p2);
        }
    }
  }
  if(f) {avma=av1;return gerepile(av,tetpil,d);}
  p1=gdiv(v,content(v));tetpil=avma;
  return gerepile(av,tetpil,gmul(d,p1));
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*                                                                 */
/*                PSEUDO-DIVISION DES POLYNOMES                    */
/*                                                                 */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

GEN   psres(GEN x, GEN y)
{
  long  l,tetpil,degx,degy;
  GEN z,p1,p2;

  degx=lgef(x);degy=lgef(y);
  if (degx<degy) err(poler15);
  l=avma;
  p2=gpuigs((GEN)y[degy-1],degx-degy+1);
  p1=gmul(p2,x);
  tetpil=avma;
  z=gerepile(l,tetpil,gres(p1,y));
  return z;
}

GEN   discsr(GEN x)
{
  long  dx,av=avma,tetpil,tx=typ(x),i;
  GEN z,p1,p2;

  switch(tx)
    {
    case 6: z=stoi(-4);break;
    case 8: z=discsr((GEN)x[1]);break;
    case 10: dx=lgef(x);p1=deriv(x,varn(x));
      p1=subres(x,p1);tetpil=avma;p1=gdiv(p1,(GEN)x[dx-1]);
      if (((dx-3)&3)>1)
	{tetpil=avma;z=gerepile(av,tetpil,gneg(p1));}
      else z=gerepile(av,tetpil,p1);break;
    case 9: z=discsr((GEN)x[1]);break;
    case 15:
    case 16: p1=mulii((GEN)x[2],(GEN)x[2]);
      p2=shifti(mulii((GEN)x[1],(GEN)x[3]),2);tetpil=avma;
      z=gerepile(av,tetpil,subii(p1,p2));break;
    case 17: case 18: case 19:
      dx=lg(x);z=cgetg(dx,tx);for(i=1;i<dx;i++) z[i]=(long)discsr((GEN)x[i]);
      break;
    default: err(discsrer1);
    }
  return z;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~									~*/
/*~	              ALGORITHME DE STURM                               ~*/
/*~									~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

long sturm(GEN x)
{
  long  degq,av,tx=typ(x),dx,du,dv,dr,sr,s,t,r1,lr;
  GEN g,h,r,p1,p2,u,v;

  if(gcmp0(x)||(tx!=10)) err(poltyper);
  dx=lgef(x);
  if(dx<=4) return dx-3;
  av=avma;
  u=gdiv(x,content(x));v=deriv(x,varn(x));gdiv(v,content(v));
  g=gun;h=gun;s=gsigne((GEN)u[lgef(u)-1]);r1=1;
  t=(lgef(u)&1) ? -s:s;
  for(;;)
  {
    du=lgef(u);dv=lgef(v);degq=du-dv;
    r=psres(u,v);dr=lgef(r);
    if(dr<=2) err(sturmer2);
    if((gsigne((GEN)v[lgef(v)-1])>0)||(degq&1)) r=gneg(r);
    lr=lgef(r)-1;
    sr=gsigne((GEN)r[lr]);if(sr!=s) {s= -s;r1--;}
    if(lr&1) sr= -sr;if(sr!=t) {t= -t;r1++;}
    if(lr==2) {avma=av;return r1;}
    u=v;
    if(degq==1) p1=gmul(h,g);else p1=gmul(gpuigs(h,degq),g);
    v=gdiv(r,p1);g=gabs((GEN)u[lgef(u)-1]);
    if(degq==1) h=g;
    else if(degq)
      {
        p1=gpuigs(g,degq);p2=gpuigs(h,degq-1);
        h=gdiv(p1,p2);
      }
  }
}

long sturmpart(GEN x, GEN a, GEN b)
{
  long  degq,av,tx=typ(x),dx,du,dv,dr,sr,s,t,r1;
  GEN g,h,r,p1,p2,u,v;

  if(gcmp0(x)||(tx!=10)) err(poltyper);
  dx=lgef(x);
  if(dx==3) return 0;
  av=avma;
  u=gdiv(x,content(x));v=deriv(x,varn(x));gdiv(v,content(v));
  g=gun;h=gun;s=gsigne(poleval(u,b));t=gsigne(poleval(u,a));r1=0;
  if(sr=gsigne(poleval(v,b))) {if(!s) s=sr;else if(sr!=s) {s= -s;r1--;}}
  if(sr=gsigne(poleval(v,a))) {if(!t) t=sr;else if(sr!=t) {t= -t;r1++;}}
  for(;;)
  {
    du=lgef(u);dv=lgef(v);degq=du-dv;
    r=psres(u,v);dr=lgef(r);
    if(dr<=2) err(sturmer2);
    if((gsigne((GEN)v[lgef(v)-1])>0)||(degq&1)) r=gneg(r);
    if(sr=gsigne(poleval(r,b))) {if(!s) s=sr;else if(sr!=s) {s= -s;r1--;}}
    if(sr=gsigne(poleval(r,a))) {if(!t) t=sr;else if(sr!=t) {t= -t;r1++;}}
    if(dr==3) {avma=av;return r1;}
    u=v;
    if(degq==1) p1=gmul(h,g);else p1=gmul(gpuigs(h,degq),g);
    v=gdiv(r,p1);g=gabs((GEN)u[lgef(u)-1]);
    if(degq==1) h=g;
    else if(degq)
      {
        p1=gpuigs(g,degq);p2=gpuigs(h,degq-1);
        h=gdiv(p1,p2);
      }
  }
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*                                                                 */
/*         POLYNOME QUADRATIQUE ASSOCIE A UN DISCRIMINANT          */
/*                                                                 */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

GEN   quadpoly(GEN x)
{
  long    res,l,tetpil,i,tx;
  GEN   y,p1;

  if((tx=typ(x))>=17) 
    {
      l=lg(x);y=cgetg(l,tx);for(i=1;i<l;i++) y[i]=(long)quadpoly((GEN)x[i]);
      return y;
    }
  if(tx!=1) err(arither1);
  y=cgetg(5,10);
  y[1]=evalsigne(1)+evallgef(5);y[4]=un;l=avma;
  res=x[lgef(x)-1]&3;
  if((signe(x)<0)&&res) res=4-res;
  if(res>1) err(quader);
  p1=shifti(x,-2);
  tetpil=avma;
  if(res)
  {
    y[2] = lpile(l,tetpil, (signe(x)<0) ? gsub(gun,p1) : gneg(p1));
    y[3] = lneg(gun);
  }
  else
  {
    y[2] = lpile(l,tetpil,gneg(p1));
    y[3] = zero;
  }
  return y;
}

GEN quadgen(GEN x)
{
  GEN y=cgetg(4,8);y[1]=lquadpoly(x);
  y[2]=zero;y[3]=un;
  return y;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*                                                                 */
/*                    INVERSE MODULO GENERAL                       */
/*                                                                 */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

GEN   ginvmod(GEN x, GEN y)
{
  long tx=typ(x),ty=typ(y);

  if(ty==1)
  {
    if(tx==1) return mpinvmod(x,y);
    if(tx==10) return gzero;
    else err(ginvmoder);
  }
  if(ty==10)
  {
    if(tx==10) return polinvmod(x,y);
    if(tx<10) return gdivsg(1,x);
    else err(ginvmoder);
  }
  err(ginvmoder);return gnil;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~									~*/
/*~		          POLYGONE DE NEWTON				~*/
/*~									~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

GEN newtonpoly(GEN x, GEN p)
{
  GEN y;
  long n,*vval,i,a,b,c,u1,u2,r1,r2;

  if(typ(x)!=10) err(newter1);
  n=lgef(x)-3;if(n<=0) {y=cgetg(1,17);return y;}
  vval=(long *)newbloc(sizeof(long)*(n+1));
  for(i=0;i<=n;i++) vval[i]=(gcmp0((GEN)x[i+2])) ? EXP220 : ggval((GEN)x[i+2],p);
  a=0;b=1;y=cgetg(n+1,17);
  while(b<=n)
    {
      u1=vval[a]-vval[b];u2=b-a;
      for(c=b+1;c<=n;c++)
	{
	  r1=vval[a]-vval[c];r2=c-a;
	  if(u1*r2<=u2*r1) {u1=r1;u2=r2;b=c;}
	}
      for(i=a+1;i<=b;i++) y[i]=ldiv(stoi(u1),stoi(u2));
      a=b;b=a+1;
    }
  killbloc(vval);return y;
}

GEN oldpolfnf(GEN a, GEN t)

/* Factorisation du polynome a sur le corps de nombres defini par le
polynome t */
{
  GEN y,p1,p2,u,g,fa,n,r,unt,f,b,pro,ain;
  long av=avma,tetpil,lx,v,i,e,k;
  
  if((typ(a)!=10)||(typ(t)!=10)) err(polfnfer1);
  if(varn(t)) err(polfnfer2);
  if(gcmp0(a)) return gcopy(a);
  v=varn(a);ain=a;setvarn(a,0);unt=gmodulcp(gun,t);
  u=gdiv(a,ggcd(a,deriv(a,0)));u=gmul(unt,u);
  setvarn(u,MAXVARN);g=lift(u);setvarn(u,0);k= -2;
  do {k++;n=subres(t,gsubst(g,MAXVARN,gsub(polx[MAXVARN],gmulsg(k,polx[0]))));}
  while(!issquarefree(n));
  fa=(GEN)(factor(n)[1]);lx=lg(fa);y=cgetg(3,19);p1=cgetg(lx,18);y[1]=(long)p1;
  p2=cgetg(lx,18);y[2]=(long)p2;
  for(i=1;i<lx;i++)
    {
      setvarn((GEN)fa[i],0);
      f=gsubst((GEN)fa[i],0,gadd(polx[0],gmulsg(k,gmodulcp(polx[0],t))));
      pro=ggcd(u,gmul(unt,f));
      p1[i]=(typ(pro)==10)?ldiv(pro,(GEN)pro[lgef(pro)-1]):(long)pro;
      e=0;b=poldivres(a,(GEN)p1[i],&r);
      while(gcmp0(r)) {a=b;e++;b=poldivres(a,(GEN)p1[i],&r);}
      p2[i]=lstoi(e);if(v) p1[i]=lsubst((GEN)p1[i],0,polx[v]);
    }
  tetpil=avma;setvarn(ain,v);return gerepile(av,tetpil,gcopy(y));
}

GEN polfnf(GEN a, GEN t)

/* Factorisation du polynome a sur le corps de nombres defini par le
polynome t */
{
  GEN y,p1,p2,u,g,fa,n,r,unt,f,b,pro;
  long av=avma,tetpil,lx,v,i,e,k,vt;
  
  if((typ(a)!=10)||(typ(t)!=10)) err(polfnfer1);
  if(gcmp0(a)) return gcopy(a);
  vt=varn(t);v=varn(a);
  if(vt<v) err(talker,"polynomial variable must be of higher priority than number field variable\nin factornf");
  unt=gmodulcp(gun,t);u=gdiv(a,ggcd(a,deriv(a,v)));u=gmul(unt,u);
  setvarn(u,MAXVARN);g=lift(u);setvarn(u,v);k= -2;
  do 
    {
      k++;
      n=subres(t,gsubst(g,MAXVARN,gsub(polx[MAXVARN],gmulsg(k,polx[vt]))));
    }
  while(!issquarefree(n));
  fa=(GEN)(factor(n)[1]);lx=lg(fa);y=cgetg(3,19);p1=cgetg(lx,18);y[1]=(long)p1;
  p2=cgetg(lx,18);y[2]=(long)p2;
  for(i=1;i<lx;i++)
    {
      setvarn((GEN)fa[i],v);
      f=gsubst((GEN)fa[i],v,gadd(polx[v],gmulsg(k,gmodulcp(polx[vt],t))));
      pro=ggcd(u,gmul(unt,f));
      p1[i]=(typ(pro)==10)?ldiv(pro,(GEN)pro[lgef(pro)-1]):(long)pro;
      e=0;b=poldivres(a,(GEN)p1[i],&r);
      while(gcmp0(r)) {a=b;e++;b=poldivres(a,(GEN)p1[i],&r);}
      p2[i]=lstoi(e);
    }
  tetpil=avma;return gerepile(av,tetpil,gcopy(y));
}

GEN nfiso(GEN a, GEN b)
{
  long av=avma,tetpil,n,m,i,c,va,vb,lx;
  GEN p1,y,ain,bin,la,lb,da,db,fa,ex;

  if((typ(a)!=10)||(typ(b)!=10)) err(nfisoer1);
  n=lgef(a)-3;m=lgef(b)-3;if((n<=0)||(m<=0)) err(nfisoer2);
  if(n!=m) return gzero;
  ain=a;bin=b;va=varn(a);vb=varn(b);setvarn(a,0);setvarn(b,0);
  p1=content(a);if(!gcmp1(p1)) a=gdiv(a,content(a));
  p1=content(b);if(!gcmp1(p1)) b=gdiv(b,content(b));
  la=(GEN)a[n+2];
  if(!gcmp1(la)) a=gmul(gpuigs(la,n-1),gsubst(a,0,gdiv(polx[0],la)));
  lb=(GEN)b[n+2];
  if(!gcmp1(lb)) b=gmul(gpuigs(lb,n-1),gsubst(b,0,gdiv(polx[0],lb)));
  da=discsr(a);db=discsr(b);p1=gdiv(da,db);
  if(typ(p1)==4) p1=gmul((GEN)p1[1],(GEN)p1[2]);
  if(typ(p1)!=1) err(nfisoer3);
  if(!carreparfait(p1)) return gzero;
  fa=polfnf(a,b);ex=(GEN)fa[2];p1=(GEN)fa[1];lx=lg(p1);c=0;
  for(i=1;i<lx;i++) 
    {
      if(!gcmp1((GEN)ex[i])) err(nfisoer4);
      if(lgef((GEN)p1[i])==4) c++;
    }
  if(!c) return gzero;
  y=cgetg(c+1,17);c=0;
  for(i=1;i<lx;i++) if(lgef((GEN)p1[i])==4) y[++c]=lneg(lift((GEN)((GEN)p1[i])[2]));
  setvarn(ain,va);setvarn(bin,vb);
  tetpil=avma;return gerepile(av,tetpil,gcopy(y));
}
  
GEN nfincl(GEN a, GEN b)
{
  long av=avma,tetpil,n,m,i,c,va,vb,lx,q;
  GEN p1,y,ain,bin,la,lb,da,db,fa,ex;

  if((typ(a)!=10)||(typ(b)!=10)) err(nfisoer1);
  m=lgef(a)-3;n=lgef(b)-3;if((n<=0)||(m<=0)) err(nfisoer2);
  if(n%m) return gzero;
  ain=a;bin=b;va=varn(a);vb=varn(b);setvarn(a,0);setvarn(b,0);
  p1=content(a);if(!gcmp1(p1)) a=gdiv(a,content(a));
  p1=content(b);if(!gcmp1(p1)) b=gdiv(b,content(b));
  la=(GEN)a[m+2];
  if(!gcmp1(la)) a=gmul(gpuigs(la,m-1),gsubst(a,0,gdiv(polx[0],la)));
  lb=(GEN)b[n+2];
  if(!gcmp1(lb)) b=gmul(gpuigs(lb,n-1),gsubst(b,0,gdiv(polx[0],lb)));
  q=n/m;
  da=discsr(a);db=discsr(b);
  if((typ(da)!=1)||(typ(db)!=1)) err(nfisoer3);
  fa=factor(da);ex=(GEN)fa[2];p1=(GEN)fa[1];lx=lg(p1);
  for(i=1;i<lx;i++) 
    if((itos((GEN)ex[i])&1)&&(!divise(db,gpuigs((GEN)p1[i],q)))) return gzero;
  fa=polfnf(a,b);ex=(GEN)fa[2];p1=(GEN)fa[1];lx=lg(p1);c=0;
  for(i=1;i<lx;i++) 
    {
      if(!gcmp1((GEN)ex[i])) err(nfisoer4);
      if(lgef((GEN)p1[i])==4) c++;
    }
  if(!c) return gzero;
  y=cgetg(c+1,17);c=0;
  for(i=1;i<lx;i++) if(lgef((GEN)p1[i])==4) y[++c]=lneg(lift((GEN)((GEN)p1[i])[2]));
  setvarn(ain,va);setvarn(bin,vb);
  tetpil=avma;return gerepile(av,tetpil,gcopy(y));
}
  
