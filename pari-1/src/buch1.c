/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*                                                                 */
/*            ALGORITHMES SOUS-EXPONENTIELS DE CALCUL              */
/*            DU GROUPE DE CLASSES ET DU REGULATEUR                */
/*                      CORPS QUADRATIQUES                         */
/*                     (McCURLEY, BUCHMANN)                        */
/*                                                                 */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include "genpari.h"
const long CBUCH = 15; /*DOIT ETRE DE LA FORME 2^k-1 !!!*/
#ifdef __cplusplus
const long HASHT = 1024;
#else
#define HASHT 1024
#endif

void addcolumnmat(long **mat, long s, long cardsub, long *numbase, long *subbase, long *ex, long nbprimfact, long *primfact, long *expoprimfact, GEN form1, GEN log2precis, GEN vecexpo);
void addcolumnmat1(long **mat, long s, long cardsub, long *numbase, long *subbase, long *ex, long nbprimfact1, long *primfact1, long *expoprimfact1, GEN form1, long *fpd, long nbprimfact, long *primfact, long *expoprimfact, GEN form2, GEN log2precis, GEN vecexpo);
void addcolumnmat2(long **mat, long s, long cardsub, long *numbase, long *subbase, long *ex, long nbprimfact1, long *primfact1, long *expoprimfact1, GEN form1, long *fpd, long nbprimfact, long *primfact, long *expoprimfact, GEN form2, GEN log2precis, GEN vecexpo);
void initbuchreal(GEN D, double cbach, double cbach2, long *precreg, GEN *log2precis, long *qqq, GEN *dr, double *drc, double *logd, GEN *sqrtD, GEN *isqrtD, double *lim, long *limc, long *limbach, long prec);
long *subfactorbaseimag(GEN d, GEN *w, double ll, long kc, long* vectprime, long* vperm, long *ptnbram);
long *subfactorbasereal(GEN d, GEN *w, double ll, long precreg, long kc, long* vectprime, long* vperm, long *ptnbram, GEN isqrtD, GEN sqrtD, long sens);
long factorbasequad(GEN d, long n2, long n, long **ptnum, long **ptbase, long *ptkc, long *badprim, long *nbbadprim);
long factorisequad(GEN f, long n, long limp, long *ptlo, long *primfact, long *expoprimfact, long *badprim, long nbbadprim, long *base, long limhash);
long *largeprime(long q1, long *ex, long np, long nrho, long cardsubbase, long **hashtab);
long *largeprime2(long q1, long *ex, long np, long cardsubbase, long **hashtab);
GEN **powsubfactimag(GEN w, long n, long a);
GEN **powsubfactreal(GEN w, long n, long a, GEN D, GEN isqrtD, GEN sqrtD, long sens, long precreg);
GEN sqrealform3(GEN x, GEN D, GEN isqrtD, long sens);
GEN comprealform3(GEN x, GEN y, GEN D, GEN isqrtD, long sens);
GEN rhorealform3(GEN x, GEN D, GEN isqrtD);
GEN redrealform3(GEN x, GEN D, GEN isqrtD, long sens);
GEN initializeform3(long *ex, GEN **tabform, long cardtab, GEN d, GEN isqrtd, long sens);
GEN sqrealform5(GEN x, GEN D, GEN isqrtD, GEN sqrtD, long sens);
GEN powrealform5(GEN x, long n, GEN D, GEN isqrtD, GEN sqrtD, long sens, long precreg);
GEN comprealform5(GEN x, GEN y, GEN D, GEN isqrtD, GEN sqrtD, long sens);
GEN rhorealform5(GEN x, GEN D, GEN isqrtD, GEN sqrtD);
GEN redrealform5(GEN x, GEN D, GEN isqrtD, GEN sqrtD, long sens);
GEN initializeform5(long *ex, GEN **tabform, long cardtab, GEN d, GEN isqrtd, GEN sqrtd, long sens);
GEN redrealform(GEN x, GEN D, GEN isqrtD, GEN sqrtD, long sens, long precreg);
GEN gcdrealnoer(GEN a, GEN b);
GEN lfunc(GEN D);

void freehash(long *pt)
{
  long *q;
  while(pt) {q=(long*)*pt;free(pt);pt=q;}
}

GEN buchimag(GEN D, GEN gcbach, GEN gcbach2)
{
  long CO;
  double cbach,cbach2;
  long limc,limc2,mglob,m,cp,nbram,auxrel,lo,lo1,nlze,col;
  long *fpd,b1,b2,fpc,pp,ep,b,extrarel,c,limhash;
  long kc2,*numbase,*base,*subbase;
  long **mat,**matinit,*vectprime,*vperm,*vinvperm;
  long primfact[100],expoprimfact[100],primfact1[100],expoprimfact1[100];
  long badprim[100],nbbadprim;
  long av=avma,tetpil,kc,kcco,kccopro,i,j,*pro,p2,*p1,*ex,q,s,nbtest,mm,av3;
  long sizeofmit,k,nrelsup,nreldep;
  double drc,lim,logd;
  GEN dr,v,u1u2,u1,u2,c_1;
  GEN matc,matalpha,**vp,form,form1,pc,mit,met,mot,p3,p4,basecl;
  GEN extramat,extramatc,pdep;
  long* hashtab[HASHT];

  if(typ(D)!=1) err(bucher1);
  if(!signe(D)) err(bucher2);
  if(signe(D)>0) err(bucher3);
  s=D[lgef(D)-1]&3;
  if((s==1)||(s==2)) err(bucher4);
  CO=10;
  cbach=gtodouble(gcbach);cbach2=gtodouble(gcbach2);
  if((!cmpis(D,-3))||(!cmpis(D,-4)))
    {
      p3=cgetg(5,17);p3[1]=un;p3[2]=lgetg(1,17);p3[3]=lgetg(1,17);
      p3[4]=un;return p3;
    }
  dr=cgetr(3);affir(D,dr);drc=rtodbl(dr);logd=log(fabs(drc));
  lim=sqrt(fabs(drc)/3.0);cp=(long)exp(sqrt(logd*log(logd)/8.0));
 increaseimag:
  nreldep=nrelsup=0;
  limc=(long)(cbach*logd*logd);
  if(cp>limc) limc=cp;
  limc2=max(20,(long)(cbach2*logd*logd));if(limc>limc2) limc2=limc;
  kc2=factorbasequad(D,limc2,limc,&numbase,&base,&kc,badprim,&nbbadprim);
  if(!kc)
    {
      free(base);free(numbase);
      if(cbach>5.99)
	err(talker,"sorry, buchxxx is not able to compute this field PLEASE REPORT!!!");
      avma=av;cbach=min(2*cbach,6);cbach2=max(cbach2,cbach);goto increaseimag;
    }
  pro=(long*)malloc(sizeof(long)*(kc2+1));vectprime=(long*)malloc(sizeof(long)*(kc2+1));
  for(i=1;i<=kc2;i++)
    {
      p2=vectprime[i]=base[i];
      pro[i]=(p2>0) ? p2 : -p2;
    }
  free(base);base=pro;
  vperm=(long*)malloc(sizeof(long)*(kc+1));
  for(i=1;i<=kc;i++) vperm[i]=i;
  subbase=subfactorbaseimag(D,&v,lim,kc,vectprime,vperm,&nbram);
  if(nbram<0)
    {
      free(vperm);free(vectprime);free(base);free(numbase);free(subbase);
      if(cbach>5.99)
	err(talker,"sorry, buchxxx is not able to compute this field PLEASE REPORT!!!");
      avma=av;cbach=min(2*cbach,6);cbach2=max(cbach2,cbach);goto increaseimag;
    }
  kcco=kc+CO;
  mglob=m=subbase[0];
  vp=powsubfactimag(v,m,CBUCH+7);
  mat=(long **)malloc(sizeof(long*)*(kcco+1));
  matinit=(long **)malloc(sizeof(long*)*(kcco+1));
  for(i=1;i<=kcco;i++)
    {
      p1=(long *)malloc(sizeof(long)*(kc+1));matinit[i]=mat[i]=p1;
      for(j=1;j<=kc;j++) p1[j]=0;
    }
  ex=(long*)malloc(sizeof(long)*(m+1));
  q=BITS_IN_LONG-1-(long)ceil(log((double)CBUCH)/log(2.0));
  s=0;nbtest=0;
  mm=m+nbram+CO;
  limhash=(limc<(MAXHALFULONG>>1))?limc*limc:((ulong)(HIGHBIT)>>1);
  for(i=0;i<HASHT;i++) hashtab[i]=(long*)0;
  auxrel=0;
  while(s<mm)
    {
      for(i=1;i<=m;i++) ex[i]=(mymyrand()>>q)+1;
      av3=avma;
      form=vp[1][ex[1]];
      for (i=2;i<=m;i++) form=compimag(form,vp[i][ex[i]]);
      pc=primeform(D,stoi(base[1+(s%kc)]));
      form=compimag(form,pc);
      fpc=factorisequad(form,kc,limc,&lo,primfact,expoprimfact,badprim,nbbadprim,base,limhash);
      nbtest++;
      if(fpc>1)
	{
	  fpd=largeprime(fpc,ex,1+(s%kc),0,mglob,hashtab);
	  if(fpd)
	    {
	      auxrel++;
	      s++;
	      lo1=lo;
	      for(j=1;j<=lo1;j++) {primfact1[j]=primfact[j];expoprimfact1[j]=expoprimfact[j];}
	      form1=vp[1][fpd[2]];
	      for(i=2;i<=m;i++) 
		form1=compimag(form1,vp[i][fpd[i+1]]);
	      if(fpd[m+2])
		{
		  pc=primeform(D,stoi(base[fpd[m+2]]));
		  form1=compimag(form1,pc);
		}
	      b1=itos(modis((GEN)form1[2],(fpc<<1)));b2=itos(modis((GEN)form[2],(fpc<<1)));
	      factorisequad(form1,kc,limc,&lo,primfact,expoprimfact,badprim,nbbadprim,base,limhash);
	      if(b1==b2)
		{
		  for(i=1;i<=m;i++)
		    mat[s][numbase[subbase[i]]]=fpd[i+1]-ex[i];
		  mat[s][1+(s-1)%kc]--;
		  if(fpd[m+2]) mat[s][fpd[m+2]]++;
		  for(j=1;j<=lo;j++)
		    {
		      pp=primfact[j];ep=expoprimfact[j];
		      b1=itos(modis((GEN)form1[2],(pp<<1)));
		      if(b1>pp) ep= -ep;
		      mat[s][numbase[pp]]-=ep;
		    }
		  for(j=1;j<=lo1;j++)
		    {
		      pp=primfact1[j];ep=expoprimfact1[j];
		      b1=itos(modis((GEN)form[2],(pp<<1)));
		      if(b1>pp) ep= -ep;
		      mat[s][numbase[pp]]+=ep;
		    }
		}
	      else
		{
		  if((b1+b2)!=(fpc<<1)) {s--;auxrel--;}
		  else
		    {
		      for(i=1;i<=m;i++)
			mat[s][numbase[subbase[i]]]= -fpd[i+1]-ex[i];
		      mat[s][1+(s-1)%kc]--;
		      if(fpd[m+2]) mat[s][fpd[m+2]]--;
		      for(j=1;j<=lo;j++)
			{
			  pp=primfact[j];ep=expoprimfact[j];
			  b1=itos(modis((GEN)form1[2],(pp<<1)));
			  if(b1>pp) ep= -ep;
			  mat[s][numbase[pp]]+=ep;
			}
		      for(j=1;j<=lo1;j++)
			{
			  pp=primfact1[j];ep=expoprimfact1[j];
			  b1=itos(modis((GEN)form[2],(pp<<1)));
			  if(b1>pp) ep= -ep;
			  mat[s][numbase[pp]]+=ep;
			}
		    }
		}
	    }
	}	  
      if(fpc==1)
	{
	  s++;
	  for(i=1;i<=m;i++) mat[s][numbase[subbase[i]]]= -ex[i];
	  mat[s][1+(s-1)%kc]--;
	  for(j=1;j<=lo;j++)
	    {
	      pp=primfact[j];ep=expoprimfact[j];
	      b=itos(modis((GEN)form[2],(pp<<1)));
	      if(b>pp) ep= -ep;
	      mat[s][numbase[pp]]+=ep;
	    }
	}
      avma=av3;
    }
  while(s<kcco)
    {
      for(i=1;i<=m;i++) ex[i]=(mymyrand()>>q)+1;
      av3=avma;
      form=vp[1][ex[1]];
      for (i=2;i<=m;i++) form=compimag(form,vp[i][ex[i]]);
      pc=primeform(D,stoi(base[s+1-CO]));
      form=compimag(form,pc);
      fpc=factorisequad(form,kc,limc,&lo,primfact,expoprimfact,badprim,nbbadprim,base,limhash);
      nbtest++;
      if(fpc>1)
	{
	  fpd=largeprime(fpc,ex,s+1-CO,0,mglob,hashtab);
	  if(fpd)
	    {
	      auxrel++;
	      s++;lo1=lo;
	      for(j=1;j<=lo1;j++) {primfact1[j]=primfact[j];expoprimfact1[j]=expoprimfact[j];}
	      form1=vp[1][fpd[2]];
	      for(i=2;i<=m;i++) 
		form1=compimag(form1,vp[i][fpd[i+1]]);
	      if(fpd[m+2])
		{
		  pc=primeform(D,stoi(base[fpd[m+2]]));
		  form1=compimag(form1,pc);
		}
	      b1=itos(modis((GEN)form1[2],(fpc<<1)));b2=itos(modis((GEN)form[2],(fpc<<1)));
	      factorisequad(form1,kc,limc,&lo,primfact,expoprimfact,badprim,nbbadprim,base,limhash);
	      if(b1==b2)
		{
		  for(i=1;i<=m;i++)
		    mat[s][numbase[subbase[i]]]=fpd[i+1]-ex[i];
		  mat[s][s-CO]= -1;
		  if(fpd[m+2]) mat[s][fpd[m+2]]++;
		  for(j=1;j<=lo;j++)
		    {
		      pp=primfact[j];ep=expoprimfact[j];
		      b1=itos(modis((GEN)form1[2],(pp<<1)));
		      if(b1>pp) ep= -ep;
		      mat[s][numbase[pp]]-=ep;
		    }
		  for(j=1;j<=lo1;j++)
		    {
		      pp=primfact1[j];ep=expoprimfact1[j];
		      b1=itos(modis((GEN)form[2],(pp<<1)));
		      if(b1>pp) ep= -ep;
		      mat[s][numbase[pp]]+=ep;
		    }
		}
	      else
		{
		  if((b1+b2)!=(fpc<<1)) {s--;auxrel--;}
		  else
		   {
		      for(i=1;i<=m;i++)
			mat[s][numbase[subbase[i]]]= -fpd[i+1]-ex[i];
		      mat[s][s-CO]= -1;
		      if(fpd[m+2]) mat[s][fpd[m+2]]--;
		      for(j=1;j<=lo;j++)
			{
			  pp=primfact[j];ep=expoprimfact[j];
			  b1=itos(modis((GEN)form1[2],(pp<<1)));
			  if(b1>pp) ep= -ep;
			  mat[s][numbase[pp]]+=ep;
			}
		      for(j=1;j<=lo1;j++)
			{
			  pp=primfact1[j];ep=expoprimfact1[j];
			  b1=itos(modis((GEN)form[2],(pp<<1)));
			  if(b1>pp) ep= -ep;
			  mat[s][numbase[pp]]+=ep;
			}
		    }
		}
	    }
	}
      if(fpc==1)
	{
	  s++;
	  for(i=1;i<=m;i++) mat[s][numbase[subbase[i]]]= -ex[i];
	  mat[s][s-CO]= -1;
	  for(j=1;j<=lo;j++)
	    {
	      pp=primfact[j];ep=expoprimfact[j];
	      b=itos(modis((GEN)form[2],(pp<<1)));
	      if(b>pp) ep= -ep;
	      mat[s][numbase[pp]]+=ep;
	    }
	  if(!(mat[s][s-CO])) {for(i=1;i<=kc;i++) mat[s][i]=0;s--;}
	}
      avma=av3;
    }
  nbtest=auxrel=0;
  while(s<kc2)
    {
      for (i=1;i<=m;i++) ex[i]=(mymyrand()>>q)+1;
      av3=avma;
      form=vp[1][ex[1]];
      for (i=2;i<=m;i++) form=compimag(form,vp[i][ex[i]]);
      pp=base[s+1];
      pc=primeform(D,stoi(pp));
      form=compimag(form,pc);
      fpc=factorisequad(form,s,pp,&lo,primfact,expoprimfact,badprim,nbbadprim,base,limhash);
      nbtest++;
      if(fpc>1)
	{
	  fpd=largeprime2(fpc,ex,s+1,mglob,hashtab);
	  if(fpd&&(fpc!=pp)) {auxrel++;s++;}
	}
      if(fpc==1) s++;
      avma=av3;
    }
  matc=cgetg(kcco+1,19);for(i=1;i<=kcco;i++) matc[i]=lgetg(1,18);
  mit=hnfspec(mat,&pdep,&matc,vperm,&matalpha,kcco,kc,m,&nlze,&col);
  kccopro=kcco;
 morerelimag:
  if(nlze)
    {
      vinvperm=(long*)malloc(sizeof(long)*kc+1);for(i=1;i<=kc;i++) vinvperm[vperm[i]]=i;
      s=0;extrarel=nlze+2;
      extramat=cgetg(extrarel+1,19);
      for(j=1;j<=extrarel;j++) extramat[j]=lgetg(kc+1,18);
      while(s<extrarel)
	{
	  free(ex);ex=(long*)malloc(sizeof(long)*(nlze+1));
	  for (i=1;i<=nlze;i++) ex[i]=(mymyrand()>>q)+1;
	  form=gpuigs(primeform(D,stoi(labs(vectprime[vperm[1]]))),ex[1]);
	  for (i=2;i<=nlze;i++)
	    form=compimag(form,gpuigs(primeform(D,stoi(labs(vectprime[vperm[i]]))),ex[i]));
	  fpc=factorisequad(form,kc,limc,&lo,primfact,expoprimfact,badprim,nbbadprim,base,limhash);
	  if(fpc==1)
	    {
	      s++;p1=(GEN)extramat[s];
	      for(i=1;i<=nlze;i++) p1[i]=lstoi(-ex[i]);
	      for(i=nlze+1;i<=kc;i++) p1[i]=zero;
	      for(j=1;j<=lo;j++)
		{
		  pp=primfact[j];ep=expoprimfact[j];
		  b=itos(modis((GEN)form[2],(pp<<1)));
		  if(b>pp) ep= -ep;
		  k=vinvperm[numbase[pp]];
		  p1[k]=laddsg(ep,(GEN)p1[k]);
		}
	      if(gcmp0(p1)) s--;
	    }
	}
      extramatc=cgetg(extrarel+1,19);
      for(i=1;i<=extrarel;i++) extramatc[i]=lgetg(1,18);
      if(nrelsup) nlze=0;
      mit=hnfadd(mit,&pdep,&matc,vperm,&matalpha,kccopro,kc,col,&nlze,extramat,extramatc);
      free(vinvperm);kccopro+=extrarel;col=kccopro-(lg(matalpha)-1);
      if(nlze)
	{
	  nreldep++;
	  if(nreldep>5) 
	    {
	      free(vperm);free(vectprime);free(ex);free(base);free(numbase);free(subbase);
	      for(i=1;i<=kcco;i++) free(matinit[i]);free(matinit);free(mat);
	      for(i=1;i<=m;i++) free(vp[i]);free(vp);
	      for(i=1;i<HASHT;i++) freehash(hashtab[i]);
	      if(cbach>5.99)
		err(talker,"sorry, buchxxx is not able to compute this field PLEASE REPORT!!!");
	      else
		{
		  avma=av;cbach=min(2*cbach,6);cbach2=max(cbach2,cbach);
		  goto increaseimag;
		}
	    }
	  else goto morerelimag;
	}
    }
  p1=gun;sizeofmit=lg(mit)-1;
  for(i=1;i<=sizeofmit;i++) p1=mulii(p1,gcoeff(mit,i,i));
  c_1=gdiv(gmul(p1,mppi(4)),gmul(lfunc(D),gsqrt(absi(D),4)));
  if(gcmpgs(gmul2n(c_1,2),3)<0) err(talker,"bug check in buchimag, PLEASE REPORT!!!");
  if(gcmpgs(gmul2n(c_1,1),3)>0)
    {
      nrelsup++;
      if(nrelsup>5) 
	{
	  free(vperm);free(vectprime);free(ex);free(base);free(numbase);free(subbase);
	  for(i=1;i<=kcco;i++) free(matinit[i]);free(matinit);free(mat);
	  for(i=1;i<=m;i++) free(vp[i]);free(vp);
	  for(i=1;i<HASHT;i++) freehash(hashtab[i]);
	  if(cbach>5.99)
	    fprintf(errfile,"\n  ***   Warning: check is greater than 1.5, suggest increasing extra relations\n");
	  else 
	    {
	      avma=av;cbach=min(2*cbach,6);cbach2=max(cbach2,cbach);
	      goto increaseimag;
	    }
	}
      else {nlze=min(kc,nrelsup);goto morerelimag;}
    }
  u1u2=smith2(mit);u1=(GEN)u1u2[1];u2=(GEN)u1u2[2];
  met=gmul(u1,gmul(mit,u2));
  u1=ginv(u1);
  lo=lg(met)-1;
  c=0;for(i=1;i<=lo;i++) if(!gcmp1(gcoeff(met,i,i))) c++;
  basecl=cgetg(c+1,17);
  for(j=1;j<=c;j++)
    {
      p3=gpui(primeform(D,stoi(labs(vectprime[vperm[1]]))),gcoeff(u1,1,j));
      for(i=2;i<=lo;i++)
	p3=gmul(p3,gpui(primeform(D,stoi(labs(vectprime[vperm[i]]))),gcoeff(u1,i,j)));
      basecl[j]=(long)p3;
    }
  tetpil=avma;p4=cgetg(5,17);p4[1]=lcopy(p1);mot=cgetg(c+1,17);p4[2]=(long)mot;
  for(i=1;i<=c;i++) mot[i]=lcopy(gcoeff(met,i,i));
  p4[3]=lcopy(basecl);p4[4]=lcopy(c_1);
  free(base);free(vectprime);free(ex);free(subbase);free(numbase);
  free(vperm);
  for(i=1;i<=kcco;i++) free(matinit[i]);free(matinit);free(mat);
  for(i=1;i<=m;i++) free(vp[i]);free(vp);
  for(i=1;i<HASHT;i++) freehash(hashtab[i]);
  return gerepile(av,tetpil,p4);
}

long *subfactorbaseimag(GEN d, GEN *w, double ll, long kc, long* vectprime, long* vperm, long *ptnbram)
{
  long i,j,k,nbidp,pp,*subbase,pro[100];
  double prod;
  GEN p1;

  i=0;*ptnbram=0;prod=1;if(ll<=1.1) ll=1.1;
  for(j=1;(j<=kc)&&(prod<=ll);j++)
    {
      pp=vectprime[j];
      if(pp>0) {pro[++i]=pp;prod*=pp;vperm[i]=j;}
      else (*ptnbram)++;
    }
  if(prod<=ll) {*ptnbram= -1;return (long*)0;}
  nbidp=i;
  for(k=1;k<j;k++) if(vectprime[k]<=0) vperm[++i]=k;
  *w=cgetg(nbidp+1,18);
  for(j=1;j<=nbidp;j++) 
    {
      p1=primeform(d,stoi(pro[j]));(*w)[j]=(long)p1;
    }
  subbase=(long*)malloc(sizeof(long)*(nbidp+1));subbase[0]=nbidp;
  for(j=1;j<=nbidp;j++) subbase[j]=pro[j];
  return subbase;
}
  
GEN **powsubfactimag(GEN w, long n, long a)
{
  long i,j;
  GEN **x;

  x=(GEN**)malloc(sizeof(GEN*)*(n+1));
  for(i=1;i<=n;i++) x[i]=(GEN*)malloc(sizeof(GEN)*(a+1));
  for(i=1;i<=n;i++)
    {
      x[i][0]=gpuigs((GEN)w[1],0);
      for(j=1;j<=a;j++) x[i][j]=compimag(x[i][j-1],(GEN)w[i]);
    }
  return x;
}


long factorbasequad(GEN d, long n2, long n, long **ptnum, long **ptbase, long *ptkc, long *badprim, long *nbbadprim)
{
  byteptr delta=diffptr;
  long av2,i,pp,qq,fl,kr,r,*numbase,*base,sizemat;
  GEN p1;

  numbase=(long*)malloc(sizeof(long)*(n2+1));*ptnum=numbase;
  base=(long*)malloc(sizeof(long)*(n2+1));*ptbase=base;
  *ptkc=0;*nbbadprim=0;av2=avma;i=0;pp=*delta++;qq=2;fl=1;
  while(pp<=n2)
    {
      if((kr=krogs(d,pp))!=-1)
	{
	  if(kr) {i++;numbase[pp]=i;base[i]=pp;}
	  else
	    {
	      p1=divis(d,pp);
	      if(signe(modis(p1,pp))) {i++;numbase[pp]=i;base[i]= -pp;}
	      else
		{
		  if(pp==2)
		    {
		      r=p1[lgef(p1)-1]&7;if(signe(d)<0) r=8-r;
		      if(r>=4) {i++;numbase[pp]=i;base[i]= -pp;}
		      else badprim[++(*nbbadprim)]=pp;
		    }
		  else badprim[++(*nbbadprim)]=pp;
		}
	    }
	}
      pp+=*delta++;
      if((pp>n)&&fl) {sizemat=i;*ptkc=sizemat;fl=0;}
    }
  avma=av2;return i;
}

long factorisequad(GEN f, long n, long limp, long *ptlo, long *primfact, long *expoprimfact, long *badprim, long nbbadprim, long *base, long limhash)
{
  long sr,i,p,k,fl=1,av1,av2,q1,lo;
  GEN x,q,r;

  av1=avma;lo=0;x=(GEN)(absi((GEN)f[1]));if(gcmp1(x)) {avma=av1;*ptlo=0;return 1;}
  av2=avma;
  for(i=1;(i<=n)&&fl;i++)
    {
      p=base[i];q=dvmdis(x,p,&r);
      if(sr=(!signe(r)))
	{
	  primfact[++lo]=p;x=q;k=0;av2=avma;
	  while(sr)
	    {k++;q=dvmdis(x,p,&r);if(sr=(!signe(r))) {x=q;av2=avma;}}
	  expoprimfact[lo]=k;
	}
      else avma=av2;
      fl=(cmpis(q,p)>0);
    }
  if(!fl)
    {
      if(gcmp1(x)) {avma=av1;*ptlo=lo;return 1;}
      else
	{
	  if(cmpis(x,limp)<=0)
	    {
	      for(i=1;i<=nbbadprim;i++)
		if(!signe(modis(x,badprim[i]))) {avma=av1;*ptlo=lo;return 0;}
	      primfact[++lo]=itos(x);expoprimfact[lo]=1;avma=av1;*ptlo=lo;return 1;
	    }
	}
    }
  *ptlo=lo;
  if(cmpis(x,limhash)<=0)
    {q1=itos(x);avma=av1;return q1;}
  else {avma=av1;return 0;}
}

long *largeprime2(long q1, long *ex, long np, long cardsubbase, long **hashtab)
{
  long hashv,*pt,cpt,i,*p1;

  hashv=((q1&2047)-1)>>1;pt=hashtab[hashv];cpt=0;
  while(pt&&(q1!=pt[1])) {pt=(long*)(*pt);cpt++;}
  if(!pt)
    {
      if(!(p1=(long*)malloc((cardsubbase+4)<<TWOPOTBYTES_IN_LONG))) err(bucher6);
      p1[1]=q1;
      for(i=2;i<cardsubbase+2;i++) p1[i]=ex[i-1];p1[cardsubbase+2]=np;p1[cardsubbase+3]=0;
      p1[0]=cpt ? (long)hashtab[hashv] : 0;hashtab[hashv]=p1;return (long*)0;
    }
  else return (pt[cardsubbase+2]==np) ? (long*)0 : pt;
}

GEN buchreal(GEN D, GEN gsens, GEN gcbach, GEN gcbach2, long prec)
{
  long sens,RELSUP,nrelsup,nreldep,kccopro;
  double cbach,cbach2;
  long precreg,limc,limbach,cardsubbase,cardsub,nbram,auxrel;
  long nbprimfact,nbprimfact1,fl,fl2,lo,initform5,findecycle,memoirefindecycle;
  long nbrhocourant,nbrhocumule,nrho,*fpd,b1,b2,fpc,pp,ep,b,extrarel,c,limhash;
  long sizebach,*numbase,*base,*subbase;
  long **mat,**matinit,*vectprime,*vperm,*vinvperm;
  long primfact[100],expoprimfact[100],primfact1[100],expoprimfact1[100];
  long badprim[100],nbbadprim,nlze,col,sizeofmit;
  long av=avma,av1,av2,tetpil,dec,sizemat,sizematcol,i,j,k,*pro,p2,*p1,*ex,qqq,s,nbtest,mm;
  double drc,lim,logd;
  GEN dr,sqrtD,isqrtD,log2precis,tabprform;
  GEN matc,extramat,extramatc,**tabpowprform;
  GEN forminit,form,form0,form1,form2,pc,mit,met,mot,p3,p4,pdep,matalpha;
  GEN vecexpo;
  GEN reg,c_1,u1u2,u1,u2,basecl;
  long* hashtab[HASHT];

  if((typ(D)!=1)||(typ(gsens)!=1)) err(bucher1);
  if(!signe(D)) err(bucher2);
  if(signe(D)<0) err(bucher8);
  s=D[lgef(D)-1]&3;
  if((s==2)||(s==3)) err(bucher4);
  sens=labs(signe(gsens));
  if(sens) 
    {
      /* sens=0;fprintf(errfile,"\n  ***   Warning: narrow class group request not yet implemented, ignored\n"); */
    }
  RELSUP=10;
  cbach=gtodouble(gcbach);cbach2=gtodouble(gcbach2);
increasereal:
  nreldep=nrelsup=0;
  initbuchreal(D,cbach,cbach2,&precreg,&log2precis,&qqq,&dr,&drc,&logd,&sqrtD,&isqrtD,&lim,&limc,&limbach,prec);
  sizebach=factorbasequad(D,limbach,limc,&numbase,&base,&sizemat,badprim,&nbbadprim);
  if(!sizemat)
    {
      free(base);free(numbase);
      if(cbach>5.99)
	err(talker,"sorry, buchxxx is not able to compute this field PLEASE REPORT!!!");
      avma=av;cbach=min(2*cbach,6);cbach2=max(cbach2,cbach);goto increasereal;
    }
  if(!(pro=(long*)malloc(sizeof(long)*(sizebach+1)))) err(talker,"out of memory3!");
  if(!(vectprime=(long*)malloc(sizeof(long)*(sizebach+1)))) err(talker,"out of memory4!");
  for(i=1;i<=sizebach;i++)
    {p2=vectprime[i]=base[i];pro[i]=(p2>0) ? p2 : -p2;}
  free(base);base=pro;
  if(!(vperm=(long*)malloc(sizeof(long)*(sizemat+1)))) err(talker,"out of memory5!");
  for(i=1;i<=sizemat;i++) vperm[i]=i;
  subbase=subfactorbasereal(D,&tabprform,lim,precreg,sizemat,vectprime,vperm,&nbram,isqrtD,sqrtD,sens);
  if(nbram<0)
    {
      free(vperm);free(vectprime);free(base);free(numbase);free(subbase);
      if(cbach>5.99)
        err(talker,"sorry, buchxxx is not able to compute this field PLEASE REPORT!!!");
      avma=av;cbach=min(2*cbach,6);cbach2=max(cbach2,cbach);
      goto increasereal;
    }
  sizematcol=sizemat+RELSUP;
  cardsubbase=cardsub=subbase[0];
  tabpowprform=powsubfactreal(tabprform,cardsub,CBUCH+7,D,isqrtD,sqrtD,sens,precreg);
  vecexpo=cgetg(sizematcol+1,17);
  for(i=1;i<=sizematcol;i++) vecexpo[i]=lgetr(precreg);
  mat=(long**)malloc((sizematcol+1)*sizeof(long*));if(!((long)mat)) err(talker,"out of memory6!");
  matinit=(long**)malloc((sizematcol+1)*sizeof(long*));if(!((long)matinit)) err(talker,"out of memory7!");
  for(i=1;i<=sizematcol;i++)
    {
      matinit[i]=mat[i]=(long*)malloc((sizemat+1)*sizeof(long));
      if(!((long)(mat[i]))) err(talker,"out of memory8!");
    }
  for(i=1;i<=sizematcol;i++) for(j=1;j<=sizemat;j++) mat[i][j]=0;
  if(!(ex=(long*)malloc(sizeof(long)*(cardsub+1)))) err(talker,"out of memory9!");
  limhash=(limc<(MAXHALFULONG>>1))?limc*limc:((ulong)(HIGHBIT>>1));
  s=nbtest=auxrel=0;
  mm=cardsub+nbram+RELSUP;
  for(i=0;i<HASHT;i++) hashtab[i]=(long*)0;
  while(s<mm)
    {
      for(i=1;i<=cardsub;i++) ex[i]=(mymyrand()>>qqq)+1;
      av1=avma;
      form=form0=initializeform3(ex,tabpowprform,cardsub,D,isqrtD,sens);
      initform5=0;findecycle=memoirefindecycle=1;nbrhocourant=nbrhocumule=0;
      do
	{
	  av2=avma;
	  fpc=factorisequad(form,sizemat,limc,&nbprimfact,primfact,expoprimfact,badprim,nbbadprim,base,limhash);
	  if(fpc==1)
	    {
	      if(!initform5)
		{
		  form1=initializeform5(ex,tabpowprform,cardsub,D,isqrtD,sqrtD,sens);
		  initform5=1;
		}
	      for(i=1;i<=nbrhocourant;i++) form1=rhorealform5(form1,D,isqrtD,sqrtD);
	      nbrhocourant=0;s++;
	      addcolumnmat(mat,s,cardsub,numbase,subbase,ex,nbprimfact,primfact,expoprimfact,form1,log2precis,vecexpo);
	    }
	  else if(fpc>1)
	    {
	      fpd=largeprime(fpc,ex,0,nbrhocumule,cardsubbase,hashtab);
	      if(fpd)
		{
		  if(!initform5)
		    {
		      form1=initializeform5(ex,tabpowprform,cardsub,D,isqrtD,sqrtD,sens);
		      initform5=1;
		    }
		  for(i=1;i<=nbrhocourant;i++)
		    form1=rhorealform5(form1,D,isqrtD,sqrtD);
		  nbrhocourant=0;nbprimfact1=nbprimfact;
		  for(j=1;j<=nbprimfact1;j++)
		    {primfact1[j]=primfact[j];expoprimfact1[j]=expoprimfact[j];}
		  form2=tabpowprform[1][fpd[2]];
		  for(i=2;i<=cardsub;i++) 
		    form2=comprealform5(form2,tabpowprform[i][fpd[i+1]],D,isqrtD,sqrtD,sens);
		  for(i=1;i<=fpd[cardsub+3];i++)
		    form2=rhorealform5(form2,D,isqrtD,sqrtD);
		  if((!sens)&&(signe(addii((GEN)form2[1],(GEN)form2[3]))))
		    {setsigne((GEN)form2[1],1);setsigne((GEN)form2[3],-1);}
		  b1=itos(modis((GEN)form2[2],(fpc<<1)));b2=itos(modis((GEN)form1[2],(fpc<<1)));
		  factorisequad(form2,sizemat,limc,&nbprimfact,primfact,expoprimfact,badprim,nbbadprim,base,limhash);
		  if(b1==b2)
		    {
		      s++;
		      addcolumnmat1(mat,s,cardsub,numbase,subbase,ex,nbprimfact1,primfact1,expoprimfact1,form1,fpd,nbprimfact,primfact,expoprimfact,form2,log2precis,vecexpo);
		    }
		  else
		    {
		      if((b1+b2)==(fpc<<1))
			{
			  s++;
			  addcolumnmat2(mat,s,cardsub,numbase,subbase,ex,nbprimfact1,primfact1,expoprimfact1,form1,fpd,nbprimfact,primfact,expoprimfact,form2,log2precis,vecexpo);
			}
		    }
		}
	    }
	  form=rhorealform3(form,D,isqrtD);nbrhocourant++;nbrhocumule++;
	  if(!sens)
	    {
	      if(memoirefindecycle==findecycle)
		{
		  if(gegal((GEN)form[1],(GEN)form0[1])&&gegal((GEN)form[2],(GEN)form0[2])) memoirefindecycle=0;
		  if(gegal((GEN)form[1],negi((GEN)form0[1]))&&gegal((GEN)form[2],(GEN)form0[2])) memoirefindecycle=0;
		  tetpil=avma;form=gcopy(form);
		  if(initform5) form1=gcopy(form1);
		  dec=lpile(av2,tetpil,0)>>TWOPOTBYTES_IN_LONG;
		  form+=dec;if(initform5) form1+=dec;
		}
	      else {findecycle=0;avma=av2;}
	    }
	  else
	    {
	      if(memoirefindecycle==findecycle)
		{
		  if(gegal((GEN)form[1],(GEN)form0[1])&&gegal((GEN)form[2],(GEN)form0[2])) memoirefindecycle=0;
		  tetpil=avma;form=gcopy(form);
		  if(initform5) form1=gcopy(form1);
		  dec=lpile(av2,tetpil,0)>>TWOPOTBYTES_IN_LONG;
		  form+=dec;if(initform5) form1+=dec;
		}
	      else {findecycle=0;avma=av2;}
	    }
	}
      while((s<mm)&&findecycle);
      avma=av1;
    }
  while(s<sizematcol)
    {
      for(i=1;i<=cardsub;i++) ex[i]=(mymyrand()>>qqq)+1;
      av1=avma;
      form=initializeform3(ex,tabpowprform,cardsub,D,isqrtD,sens);
      pc=redrealform3(primeform(D,stoi(base[s+1-RELSUP]),precreg),D,isqrtD,sens);
      form=comprealform3(form,pc,D,isqrtD,sens);forminit=form;nrho=fl=fl2=0;
      do
	{
	  fpc=factorisequad(form,sizemat,limc,&nbprimfact,primfact,expoprimfact,badprim,nbbadprim,base,limhash);
	  nbtest++;
	  if(fpc>1)
	    {
	      fpd=largeprime(fpc,ex,s+1-RELSUP,nrho,cardsubbase,hashtab);
	      if(fpd)
		{
		  form=initializeform5(ex,tabpowprform,cardsub,D,isqrtD,sqrtD,sens);
		  pc=redrealform(primeform(D,stoi(base[s+1-RELSUP]),precreg),D,isqrtD,sqrtD,sens,precreg);
		  pc[4]=zero;affsr(1,(GEN)pc[5]);
		  form=comprealform5(form,pc,D,isqrtD,sqrtD,sens);
		  for(i=1;i<=nrho;i++) form=rhorealform5(form,D,isqrtD,sqrtD);
		  auxrel++;s++;nbprimfact1=nbprimfact;
		  for(j=1;j<=nbprimfact1;j++)
		    {primfact1[j]=primfact[j];expoprimfact1[j]=expoprimfact[j];}
		  form1=tabpowprform[1][fpd[2]];
		  for(i=2;i<=cardsub;i++) 
		    form1=comprealform5(form1,tabpowprform[i][fpd[i+1]],D,isqrtD,sqrtD,sens);
		  if(fpd[cardsub+2])
		    {
		      pc=redrealform(primeform(D,stoi(base[fpd[cardsub+2]]),precreg),D,isqrtD,sqrtD,sens,precreg);
		      pc[4]=zero;affsr(1,(GEN)pc[5]);
		      form1=comprealform5(form1,pc,D,isqrtD,sqrtD,sens);
		    }
		  for(i=1;i<=fpd[cardsub+3];i++) 
		    form1=rhorealform5(form1,D,isqrtD,sqrtD);
		  if((!sens)&&(signe(addii((GEN)form1[1],(GEN)form1[3]))))
		    {setsigne((GEN)form1[1],1);setsigne((GEN)form1[3],-1);}
		  b1=itos(modis((GEN)form1[2],(fpc<<1)));b2=itos(modis((GEN)form[2],(fpc<<1)));
		  factorisequad(form1,sizemat,limc,&nbprimfact,primfact,expoprimfact,badprim,nbbadprim,base,limhash);
		  if(b1==b2)
		    {
		      fl2=1;
		      for(i=1;i<=cardsub;i++) mat[s][numbase[subbase[i]]]=fpd[i+1]-ex[i];
		      mat[s][s-RELSUP]= -1;
		      if(fpd[cardsub+2]) mat[s][fpd[cardsub+2]]++;
		      for(j=1;j<=nbprimfact;j++)
			{
			  pp=primfact[j];ep=expoprimfact[j];b1=itos(modis((GEN)form1[2],(pp<<1)));
			  if(b1>pp) ep= -ep;mat[s][numbase[pp]]-=ep;
			}
		      for(j=1;j<=nbprimfact1;j++)
			{
			  pp=primfact1[j];ep=expoprimfact1[j];b1=itos(modis((GEN)form[2],(pp<<1)));
			  if(b1>pp) ep= -ep;mat[s][numbase[pp]]+=ep;
			}
		      affrr(shiftr(mpadd(mulir(mulsi(EXP220,subii((GEN)form[4],(GEN)form1[4])),log2precis),mplog(absr(divrr((GEN)form[5],(GEN)form1[5])))),-1),(GEN)vecexpo[s]);
		    }
		  else
		    {
		      if((b1+b2)!=(fpc<<1)) {s--;auxrel--;}
		      else
			{
			  fl2=1;
			  for(i=1;i<=cardsub;i++) mat[s][numbase[subbase[i]]]= -fpd[i+1]-ex[i];
			  mat[s][s-RELSUP]= -1;
			  if(fpd[cardsub+2]) mat[s][fpd[cardsub+2]]--;
			  for(j=1;j<=nbprimfact;j++)
			    {
			      pp=primfact[j];ep=expoprimfact[j];
			      b1=itos(modis((GEN)form1[2],(pp<<1)));
			      if(b1>pp) ep= -ep;mat[s][numbase[pp]]+=ep;
			    }
			  for(j=1;j<=nbprimfact1;j++)
			    {
			      pp=primfact1[j];ep=expoprimfact1[j];
			      b1=itos(modis((GEN)form[2],(pp<<1)));
			      if(b1>pp) ep= -ep;mat[s][numbase[pp]]+=ep;
			    }
			  affrr(shiftr(mpadd(mulir(mulsi(EXP220,addii((GEN)form1[4],(GEN)form[4])),log2precis),mplog(absr(mulrr((GEN)form1[5],(GEN)form[5])))),-1),(GEN)vecexpo[s]);
			}
		    }
		}
	    }
	  if ((fpc!=1)&&(!fl2))
	    {
	      form=rhorealform3(form,D,isqrtD);nrho++;
	      if((sens)||(!signe(addii((GEN)form[1],(GEN)form[3]))))
		{form=rhorealform3(form,D,isqrtD);nrho++;}
	      else {setsigne((GEN)form[1],1);setsigne((GEN)form[3],-1);}
	      fl=gegal((GEN)form[1],(GEN)forminit[1]);
	      if(fl) fl=gegal((GEN)form[2],(GEN)forminit[2]);
	      if(fl) fl=gegal((GEN)form[3],(GEN)forminit[3]);
	    }
	}
      while((fpc!=1)&&(!fl)&&(!fl2));
      if(fpc==1)
	{
	  form=initializeform5(ex,tabpowprform,cardsub,D,isqrtD,sqrtD,sens);
	  pc=redrealform(primeform(D,stoi(base[s+1-RELSUP]),precreg),D,isqrtD,sqrtD,sens,precreg);
	  pc[4]=zero;affsr(1,(GEN)pc[5]);
	  form=comprealform5(form,pc,D,isqrtD,sqrtD,sens);
	  for(i=1;i<=nrho;i++) form=rhorealform5(form,D,isqrtD,sqrtD);
	  s++;
	  for(i=1;i<=cardsub;i++) mat[s][numbase[subbase[i]]]= -ex[i];
	  mat[s][s-RELSUP]= -1;
 	  for(j=1;j<=nbprimfact;j++)
	    {
	      pp=primfact[j];ep=expoprimfact[j];b=itos(modis((GEN)form[2],(pp<<1)));
	      if(b>pp) ep= -ep;mat[s][numbase[pp]]+=ep;
	    }
	  affrr(shiftr(mpadd(mulir(mulsi(EXP220,(GEN)form[4]),log2precis),mplog(absr((GEN)form[5]))),-1),(GEN)vecexpo[s]);
	  if(!(mat[s][s-RELSUP])) {for(i=1;i<=sizemat;i++) mat[s][i]=0;s--;}
	}
      avma=av1;
    }
  nbtest=auxrel=0;
  while(s<sizebach)
    {
      for (i=1;i<=cardsub;i++) ex[i]=(mymyrand()>>qqq)+1;
      av1=avma;
      form=initializeform3(ex,tabpowprform,cardsub,D,isqrtD,sens);
      pp=base[s+1];
      pc=redrealform3(primeform(D,stoi(pp),precreg),D,isqrtD,sens);
      form=comprealform3(form,pc,D,isqrtD,sens);forminit=form;fl2=0;
      do
	{
	  fpc=factorisequad(form,s,pp,&nbprimfact,primfact,expoprimfact,badprim,nbbadprim,base,limhash);
	  nbtest++;
	  if(fpc>1)
	    {
	      fpd=largeprime2(fpc,ex,s+1,cardsubbase,hashtab);
	      if(fpd&&(fpc!=pp)){auxrel++;s++;fl2=1;}
	    }
	  if((fpc!=1)&&(!fl2))
	    {
	      form=rhorealform3(form,D,isqrtD);
	      if((sens)||(!signe(addii((GEN)form[1],(GEN)form[3])))) 
		form=rhorealform3(form,D,isqrtD);
	      else{setsigne((GEN)form[1],1);setsigne((GEN)form[3],-1);}
	      fl=gegal((GEN)form[1],(GEN)forminit[1]);
	      if(fl) fl=gegal((GEN)form[2],(GEN)forminit[2]);
	      if(fl) fl=gegal((GEN)form[3],(GEN)forminit[3]);
	    }
	}
      while((fpc!=1)&&(!fl)&&(!fl2));
      if(fpc==1) s++;
      avma=av1;
    }
  matc=cgetg(sizematcol+1,19);
  for(i=1;i<=sizematcol;i++){p1=cgetg(2,18);matc[i]=(long)p1;p1[1]=vecexpo[i];}
  mit=hnfspec(mat,&pdep,&matc,vperm,&matalpha,sizematcol,sizemat,cardsub,&nlze,&col);
  kccopro=sizematcol;
 morerelreal:
  if(nlze)
    {
      if(!(vinvperm=(long*)malloc(sizeof(long)*sizemat+1))) err(talker,"out of memory10!");
      for(i=1;i<=sizemat;i++) vinvperm[vperm[i]]=i;
      s=0;extrarel=nlze+2;
      extramat=cgetg(extrarel+1,19);
      for(j=1;j<=extrarel;j++) extramat[j]=lgetg(sizemat+1,18);
      extramatc=cgetg(extrarel+1,19);
      for(i=1;i<=extrarel;i++) extramatc[i]=lgetg(2,18);
      while(s<extrarel)
	{
	  free(ex);
	  if(!(ex=(long*)malloc(sizeof(long)*(nlze+1)))) err(talker,"out of memory11!");
	  for (i=1;i<=nlze;i++) ex[i]=(mymyrand()>>qqq)+1;
	  form=gpuigs(primeform(D,stoi(labs(vectprime[vperm[1]])),precreg),ex[1]);
	  for (i=2;i<=nlze;i++)
	    form=compreal(form,gpuigs(primeform(D,stoi(labs(vectprime[vperm[i]])),precreg),ex[i]));
	  fpc=factorisequad(form,sizemat,limc,&lo,primfact,expoprimfact,badprim,nbbadprim,base,limhash);
	  if(fpc==1)
	    {
	      s++;p1=(GEN)extramat[s];
	      for(i=1;i<=nlze;i++) p1[i]=lstoi(-ex[i]);
	      for(i=nlze+1;i<=sizemat;i++) p1[i]=zero;
	      for(j=1;j<=lo;j++)
		{
		  pp=primfact[j];ep=expoprimfact[j];
		  b=itos(modis((GEN)form[2],(pp<<1)));
		  if(b>pp) ep= -ep;
		  k=vinvperm[numbase[pp]];
		  p1[k]=laddsg(ep,(GEN)p1[k]);
		}
	      if(gcmp0(p1)) s--;
	      else coeff(extramatc,1,s)=form[4];
	    }
	}
      if(nrelsup) nlze=0;
      mit=hnfadd(mit,&pdep,&matc,vperm,&matalpha,kccopro,sizemat,col,&nlze,extramat,extramatc);
      free(vinvperm);kccopro+=extrarel;col=kccopro-(lg(matalpha)-1);
      if(nlze)
	{
	  nreldep++;
	  if(nreldep>5) 
	    {
	      free(vperm);free(vectprime);free(ex);free(base);free(numbase);free(subbase);
	      for(i=1;i<=sizematcol;i++) free(matinit[i]);free(matinit);free(mat);
	      for(i=1;i<=cardsub;i++) free(tabpowprform[i]);free(tabpowprform);
	      for(i=1;i<HASHT;i++) freehash(hashtab[i]);
	      if(cbach>5.99)
		err(talker,"sorry, buchxxx is not able to compute this field PLEASE REPORT!!!");
	      avma=av;cbach=min(2*cbach,6);cbach2=max(cbach2,cbach);
	      goto increasereal;
	    }
	  else goto morerelreal;
	}
    }
  p1=gun;sizeofmit=lg(mit)-1;
  for(i=1;i<=sizeofmit;i++) p1=mulii(p1,gcoeff(mit,i,i));
  extrarel=col-sizeofmit;
  reg=gabs(gcoeff(matc,1,1));
  for(i=2;i<=extrarel;i++) reg=gcdrealnoer(gcoeff(matc,1,i),reg);
  if(gexpo(reg)<=-3)
    {
      nrelsup++;
      if(nrelsup>5) 
	{
	  free(vperm);free(vectprime);free(ex);free(base);free(numbase);free(subbase);
	  for(i=1;i<=sizematcol;i++) free(matinit[i]);free(matinit);free(mat);
	  for(i=1;i<=cardsub;i++) free(tabpowprform[i]);free(tabpowprform);
	  for(i=1;i<HASHT;i++) freehash(hashtab[i]);
	  if(cbach>5.99)
	    err(talker,"sorry, buchxxx is not able to compute this field PLEASE REPORT!!!");
	  else
	    {
	      avma=av;cbach=min(2*cbach,6);cbach2=max(cbach2,cbach);
	      goto increasereal;
	    }
	}
      else {nlze=min(sizemat,nrelsup);goto morerelreal;}
    }
  c_1=gdiv(gmul2n(gmul(p1,reg),1),gmul(lfunc(D),sqrtD));  
  if(gcmpgs(gmul2n(c_1,2),3)<0) err(talker,"bug check in buchreal, PLEASE REPORT!!!");
  if(gcmpgs(gmul2n(c_1,1),3)>0)
    {
      nrelsup++;
      if(nrelsup>5) 
	{
	  free(vperm);free(vectprime);free(ex);free(base);free(numbase);free(subbase);
	  for(i=1;i<=sizematcol;i++) free(matinit[i]);free(matinit);free(mat);
	  for(i=1;i<=cardsub;i++) free(tabpowprform[i]);free(tabpowprform);
	  for(i=1;i<HASHT;i++) freehash(hashtab[i]);
	  if(cbach>5.99)
	    fprintf(errfile,"\n  ***   Warning: check is greater than 1.5, suggest increasing extra relations\n");
	  else
	    {
	      avma=av;cbach=min(2*cbach,6);cbach2=max(cbach2,cbach);
	      goto increasereal;
	    }
	}
      else {nlze=min(sizemat,nrelsup);goto morerelreal;}
    }
  u1u2=smith2(mit);u1=(GEN)u1u2[1];u2=(GEN)u1u2[2];
  met=gmul(u1,gmul(mit,u2));
  u1=ginv(u1);
  lo=lg(met)-1;
  c=0;for(i=1;i<=lo;i++) if(!gcmp1(gcoeff(met,i,i))) c++;
  basecl=cgetg(c+1,17);
  for(j=1;j<=c;j++)
    {
      p3=gpui(primeform(D,stoi(labs(vectprime[vperm[1]])),precreg),gcoeff(u1,1,j));
      for(i=2;i<=lo;i++)
        p3=gmul(p3,gpui(primeform(D,stoi(labs(vectprime[vperm[i]])),precreg),gcoeff(u1,i,j)));
      basecl[j]=(long)p3;
    }
  tetpil=avma;p4=cgetg(6,17);p4[1]=lcopy(p1);
  mot=cgetg(c+1,17);p4[2]=(long)mot;for(i=1;i<=c;i++) mot[i]=lcopy(gcoeff(met,i,i));
  p4[3]=lcopy(basecl);p4[4]=lcopy(reg);p4[5]=lcopy(c_1);
  free(base);free(vectprime);free(ex);free(subbase);free(numbase);
  free(vperm);
  for(i=1;i<=sizematcol;i++) free(matinit[i]);free(matinit);free(mat);
  for(i=1;i<=cardsub;i++) free(tabpowprform[i]);free(tabpowprform);
  for(i=1;i<HASHT;i++) freehash(hashtab[i]);
  return gerepile(av,tetpil,p4);
}


long *subfactorbasereal(GEN d, GEN *w, double ll, long precreg, long kc, long* vectprime, long* vperm, long *ptnbram, GEN isqrtD, GEN sqrtD, long sens)
{
  long i,j,k,nbidp,pp,*subbase,pro[100];
  double prod;
  GEN p1;

  i=0;*ptnbram=0;prod=1;if(ll<=1.1) ll=1.1;
  for(j=1;(j<=kc)&&(prod<=ll);j++)
    {
      pp=vectprime[j];
      if(pp>0) {pro[++i]=pp;prod*=pp;vperm[i]=j;}
      else (*ptnbram)++;
    }
  if(prod<=ll) {*ptnbram= -1;return (long*)0;}
  nbidp=i;
  for(k=1;k<j;k++) if(vectprime[k]<=0) vperm[++i]=k;
  *w=cgetg(nbidp+1,18);
  for(j=1;j<=nbidp;j++)
    {
      p1=redrealform(primeform(d,stoi(pro[j]),precreg),d,isqrtD,sqrtD,sens,precreg);
      (*w)[j]=(long)p1;
    }
  if(!(subbase=(long*)malloc(sizeof(long)*(nbidp+1)))) err(talker,"out of memory12!");
  subbase[0]=nbidp;
  for(j=1;j<=nbidp;j++) subbase[j]=pro[j];
  return subbase;
}
  
GEN **powsubfactreal(GEN w, long n, long a, GEN D, GEN isqrtD, GEN sqrtD, long sens, long precreg)
{
  long i,j;
  GEN **x;

  if(!(x=(GEN**)malloc(sizeof(GEN*)*(n+1)))) err(talker,"out of memory13!");
  for(i=1;i<=n;i++) 
    {
      if(!(x[i]=(GEN*)malloc(sizeof(GEN)*(a+1)))) err(talker,"out of memory14!");
    }
  for(i=1;i<=n;i++)
    {
      j=0;x[i][j]=powrealform5((GEN)w[1],0,D,isqrtD,sqrtD,sens,precreg);
      while(j<a){j++;x[i][j]=comprealform5(x[i][j-1],(GEN)w[i],D,isqrtD,sqrtD,sens);}
    }
  return x;
}
  
long *largeprime(long q1, long *ex, long np, long nrho, long cardsubbase, long **hashtab)
{
  long hashv,*pt,cpt,i,*p1,fl;

  hashv=((q1&2047)-1)>>1;pt=hashtab[hashv];cpt=0;
  while(pt&&(q1!=pt[1])) {pt=(long*)(*pt);cpt++;}
  if(!pt)
    {
      if(!(p1=(long*)malloc((cardsubbase+4)<<TWOPOTBYTES_IN_LONG))) err(bucher6);
      p1[1]=q1;
      for(i=2;i<cardsubbase+2;i++) p1[i]=ex[i-1];p1[cardsubbase+2]=np;p1[cardsubbase+3]=nrho;
      p1[0]=cpt ? (long)hashtab[hashv] : 0;hashtab[hashv]=p1;return (long*)0;
    }
  else
    {
      fl=1;i=2;while(fl&&(i<(cardsubbase+2))) {fl=(pt[i]==ex[i-1]);i++;}
      if(fl) fl=(pt[i]==np);return fl ? (long*)0 : pt;
    }
}

GEN comprealform3(GEN x, GEN y, GEN D, GEN isqrtD, long sens)
{
  long av,tetpil;
  GEN s,n,d,d1,x1,x2,y1,y2,v1,v2,b3,c3,m,z,p1,r;
  
  av=avma;s=shifti(addii((GEN)x[2],(GEN)y[2]),-1);n=subii((GEN)y[2],s);
  d=bezout((GEN)y[1],(GEN)x[1],&y1,&x1);d1=bezout(s,d,&x2,&y2);
  v1=divii((GEN)x[1],d1);v2=divii((GEN)y[1],d1);
  m=addii(mulii(mulii(y1,y2),n),mulii((GEN)y[3],x2));setsigne(m,-signe(m));
  r=modii(m,v1);b3=shifti((p1=mulii(v2,r)),1);
  c3=addii(mulii((GEN)y[3],d1),mulii(r,addii((GEN)y[2],p1)));
  z=cgetg(4,17);z[1]=lmulii(v1,v2);z[2]=laddii((GEN)y[2],b3);z[3]=ldivii(c3,v1);
  tetpil=avma;return gerepile(av,tetpil,redrealform3(z,D,isqrtD,sens));
}

GEN comprealform5(GEN x, GEN y, GEN D, GEN isqrtD, GEN sqrtD, long sens)
{
  long av,tetpil,ss;
  GEN s,n,d,d1,x1,x2,y1,y2,v1,v2,b3,c3,m,z,p1,r;
  
  av=avma;s=shifti(addii((GEN)x[2],(GEN)y[2]),-1);n=subii((GEN)y[2],s);
  d=bezout((GEN)y[1],(GEN)x[1],&y1,&x1);d1=bezout(s,d,&x2,&y2);
  v1=divii((GEN)x[1],d1);v2=divii((GEN)y[1],d1);
  m=addii(mulii(mulii(y1,y2),n),mulii((GEN)y[3],x2));setsigne(m,-signe(m));
  r=modii(m,v1);b3=shifti((p1=mulii(v2,r)),1);
  c3=addii(mulii((GEN)y[3],d1),mulii(r,addii((GEN)y[2],p1)));
  z=cgetg(6,17);z[1]=lmulii(v1,v2);z[2]=laddii((GEN)y[2],b3);z[3]=ldivii(c3,v1);
  z[5]=lmulrr((GEN)x[5],(GEN)y[5]);
  if((ss=expo((GEN)z[5]))>=EXP220) {z[4]=laddii(addsi(1,(GEN)x[4]),(GEN)y[4]);setexpo((GEN)z[5],ss-EXP220);}
  else z[4]=laddii((GEN)x[4],(GEN)y[4]);
  tetpil=avma;return gerepile(av,tetpil,redrealform5(z,D,isqrtD,sqrtD,sens));
}

GEN sqrealform3(GEN x, GEN D, GEN isqrtD, long sens)
{
  long av,tetpil;
  GEN d1,x2,y2,v1,b3,c3,m,z,p1,r;
  
  av=avma;
  d1=bezout((GEN)x[2],(GEN)x[1],&x2,&y2);v1=divii((GEN)x[1],d1);
  m=mulii((GEN)x[3],x2);setsigne(m,-signe(m));
  r=modii(m,v1);b3=shifti((p1=mulii(v1,r)),1);
  c3=addii(mulii((GEN)x[3],d1),mulii(r,addii((GEN)x[2],p1)));
  z=cgetg(4,17);z[1]=lmulii(v1,v1);z[2]=laddii((GEN)x[2],b3);z[3]=ldivii(c3,v1);
  tetpil=avma;return gerepile(av,tetpil,redrealform3(z,D,isqrtD,sens));
}

GEN sqrealform5(GEN x, GEN D, GEN isqrtD, GEN sqrtD, long sens)
{
  long av,tetpil,ss;
  GEN d1,x2,y2,v1,b3,c3,m,z,p1,r;
  
  av=avma;
  d1=bezout((GEN)x[2],(GEN)x[1],&x2,&y2);v1=divii((GEN)x[1],d1);
  m=mulii((GEN)x[3],x2);setsigne(m,-signe(m));
  r=modii(m,v1);b3=shifti((p1=mulii(v1,r)),1);
  c3=addii(mulii((GEN)x[3],d1),mulii(r,addii((GEN)x[2],p1)));
  z=cgetg(6,17);z[1]=lmulii(v1,v1);z[2]=laddii((GEN)x[2],b3);z[3]=ldivii(c3,v1);
  z[5]=lmulrr((GEN)x[5],(GEN)x[5]);
  if((ss=expo((GEN)z[5]))>=EXP220) {z[4]=laddii(addsi(1,(GEN)x[4]),(GEN)x[4]);setexpo((GEN)z[5],ss-EXP220);}
  else z[4]=lshifti((GEN)x[4],1);
  tetpil=avma;return gerepile(av,tetpil,redrealform5(z,D,isqrtD,sqrtD,sens));
}

GEN rhorealform3(GEN x, GEN D, GEN isqrtD)
{
  long av,tetpil,s;
  GEN y,p1,nn;
  
  av=avma;y=cgetg(4,17);y[1]=lcopy((GEN)x[3]);
  s=signe((GEN)y[1]);setsigne((GEN)y[1],1);
  if(cmpii(isqrtD,(GEN)y[1])>=0) nn=divii(addii(isqrtD,(GEN)x[2]),p1=shifti((GEN)y[1],1));
  else nn=divii(addii((GEN)y[1],(GEN)x[2]),p1=shifti((GEN)y[1],1));
  p1=mulii(nn,p1);y[2]=lsubii(p1,(GEN)x[2]);
  setsigne((GEN)y[1],s);p1=shifti(subii(mulii((GEN)y[2],(GEN)y[2]),D),-2);y[3]=ldivii(p1,(GEN)y[1]);
  tetpil=avma;return gerepile(av,tetpil,gcopy(y));
}

GEN rhorealform5(GEN x, GEN D, GEN isqrtD, GEN sqrtD)
{
  long av,tetpil,s,ss;
  GEN y,p1,nn;
  
  av=avma;y=cgetg(6,17);y[1]=lcopy((GEN)x[3]);
  s=signe((GEN)y[1]);setsigne((GEN)y[1],1);
  if(cmpii(isqrtD,(GEN)y[1])>=0) nn=divii(addii(isqrtD,(GEN)x[2]),p1=shifti((GEN)y[1],1));
  else nn=divii(addii((GEN)y[1],(GEN)x[2]),p1=shifti((GEN)y[1],1));
  p1=mulii(nn,p1);y[2]=lsubii(p1,(GEN)x[2]);
  setsigne((GEN)y[1],s);p1=shifti(subii(mulii((GEN)y[2],(GEN)y[2]),D),-2);y[3]=ldivii(p1,(GEN)y[1]);
  y[5]=lmulrr(divrr(addir((GEN)x[2],sqrtD),subir((GEN)x[2],sqrtD)),(GEN)x[5]);
  if((ss=expo((GEN)y[5]))>=EXP220) {y[4]=laddsi(1,(GEN)x[4]);y[5]=lshiftr((GEN)y[5],-EXP220);}
  else y[4]=x[4];
  tetpil=avma;return gerepile(av,tetpil,gcopy(y));
}

GEN redrealform3(GEN x, GEN D, GEN isqrtD, long sens)
{
  long fl,av=avma,tetpil;
  GEN y,p1;
  
  y=cgetg(4,17);y[1]=x[1];y[2]=x[2];y[3]=x[3];
  if((signe((GEN)x[2])<=0)||(cmpii((GEN)x[2],isqrtD)>0)) fl=1;
  else
    {
      p1=subii(isqrtD,shifti(absi((GEN)x[1]),1));
      if(signe(p1)<0) fl=(cmpii((GEN)x[2],absi(p1))<0);else fl=(cmpii((GEN)x[2],p1)<=0);
    }
  while(fl)
    {
      y=rhorealform3(y,D,isqrtD);
      if((signe((GEN)y[2])<=0)||(cmpii((GEN)y[2],isqrtD)>0)) fl=1;
      else
	{
	  p1=subii(isqrtD,shifti(absi((GEN)y[1]),1));
	  if(signe(p1)<0) fl=(cmpii((GEN)y[2],absi(p1))<0);else fl=(cmpii((GEN)y[2],p1)<=0);
	}
    }
  if(signe((GEN)y[1])<0)
    {
      if(sens||(!signe(addii((GEN)y[1],(GEN)y[3]))))
	{tetpil=avma;return gerepile(av,tetpil,rhorealform3(y,D,isqrtD));}
      else
	{
	  tetpil=avma;y=gerepile(av,tetpil,gcopy(y));
	  setsigne((GEN)y[1],1);setsigne((GEN)y[3],-1);return y;
	}
    }
  tetpil=avma;return gerepile(av,tetpil,gcopy(y));
}

GEN redrealform5(GEN x, GEN D, GEN isqrtD, GEN sqrtD, long sens)
{
  long fl,av=avma,tetpil;
  GEN y,p1;
  
  y=cgetg(6,17);y[1]=x[1];y[2]=x[2];y[3]=x[3];y[4]=x[4];y[5]=x[5];
  if((signe((GEN)x[2])<=0)||(cmpii((GEN)x[2],isqrtD)>0)) fl=1;
  else
    {
      p1=subii(isqrtD,shifti(absi((GEN)x[1]),1));
      if(signe(p1)<0) fl=(cmpii((GEN)x[2],absi(p1))<0);else fl=(cmpii((GEN)x[2],p1)<=0);
    }
  while(fl)
    {
      y=rhorealform5(y,D,isqrtD,sqrtD);
      if((signe((GEN)y[2])<=0)||(cmpii((GEN)y[2],isqrtD)>0)) fl=1;
      else
	{
	  p1=subii(isqrtD,shifti(absi((GEN)y[1]),1));
	  if(signe(p1)<0) fl=(cmpii((GEN)y[2],absi(p1))<0);else fl=(cmpii((GEN)y[2],p1)<=0);
	}
    }
  if(signe((GEN)y[1])<0)
    {
      if(sens||(!signe(addii((GEN)y[1],(GEN)y[3]))))
	{tetpil=avma;return gerepile(av,tetpil,rhorealform5(y,D,isqrtD,sqrtD));}
      else
	{
	  tetpil=avma;y=gerepile(av,tetpil,gcopy(y));
	  setsigne((GEN)y[1],1);setsigne((GEN)y[3],-1);return y;
	}
    }
  tetpil=avma;return gerepile(av,tetpil,gcopy(y));
}

GEN redrealform(GEN x, GEN D, GEN isqrtD, GEN sqrtD, long sens, long precreg)
{
  long fl,av=avma,tetpil;
  GEN y,p1;
  
  y=cgetg(6,17);y[1]=x[1];y[2]=x[2];y[3]=x[3];y[4]=zero;affsr(1,(GEN)(y[5]=lgetr(precreg)));
  if((signe((GEN)x[2])<=0)||(cmpii((GEN)x[2],isqrtD)>0)) fl=1;
  else
    {
      p1=subii(isqrtD,shifti(absi((GEN)x[1]),1));
      if(signe(p1)<0) fl=(cmpii((GEN)x[2],absi(p1))<0);else fl=(cmpii((GEN)x[2],p1)<=0);
    }
  while(fl)
    {
      y=rhorealform5(y,D,isqrtD,sqrtD);
      if((signe((GEN)y[2])<=0)||(cmpii((GEN)y[2],isqrtD)>0)) fl=1;
      else
	{
	  p1=subii(isqrtD,shifti(absi((GEN)y[1]),1));
	  if(signe(p1)<0) fl=(cmpii((GEN)y[2],absi(p1))<0);else fl=(cmpii((GEN)y[2],p1)<=0);
	}
    }
  if(signe((GEN)y[1])<0)
    {
      if(sens||(!signe(addii((GEN)y[1],(GEN)y[3]))))
	{tetpil=avma;return gerepile(av,tetpil,rhorealform5(y,D,isqrtD,sqrtD));}
      else
	{
	  tetpil=avma;y=gerepile(av,tetpil,gcopy(y));
	  setsigne((GEN)y[1],1);setsigne((GEN)y[3],-1);return y;
	}
    }
  tetpil=avma;return gerepile(av,tetpil,gcopy(y));
}

GEN powrealform5(GEN x, long n, GEN D, GEN isqrtD, GEN sqrtD, long sens, long precreg)
{
  GEN y,p1;
  long av,tetpil,fl;

  if(!n)
    {
      y=cgetg(6,17);y[1]=un;
      if(mpodd((GEN)x[2])) y[2]=(mpodd(isqrtD)) ? lcopy(isqrtD) : laddsi(-1,isqrtD);
      else {y[2]=lcopy(isqrtD);(((GEN)y[2])[lgef(isqrtD)-1])&=(~0x1);}
      av=avma;p1=subii(mulii((GEN)y[2],(GEN)y[2]),D);tetpil=avma;
      y[3]=lpile(av,tetpil,shifti(p1,-2));y[4]=zero;affsr(1,(GEN)(y[5]=lgetr(precreg)));
      return y;
    }
  av=avma;
  if(n<0)
    {
      p1=cgetg(6,17);p1[1]=x[1];p1[2]=lnegi((GEN)x[2]);p1[3]=x[3];
      p1[4]=lnegi(addsi(1,(GEN)x[4]));p1[5]=lshiftr(divir(gun,(GEN)x[5]),EXP220);n= -n;x=p1;
    }
  if(n==1) 
    {tetpil=avma;return gerepile(av,tetpil,redrealform(x,D,isqrtD,sqrtD,sens,precreg));}
  for(fl=0;n>1;n>>=1)
    {
      if(n&1)
	{if(fl) y=comprealform5(y,x,D,isqrtD,sqrtD,sens);else {fl=1;y=x;}}
      x=sqrealform5(x,D,isqrtD,sqrtD,sens);
    }
  tetpil=avma;y=fl ? comprealform5(y,x,D,isqrtD,sqrtD,sens) : gcopy(x);
  return gerepile(av,tetpil,y);
}

GEN initializeform3(long *ex, GEN **tabform, long cardtab, GEN d, GEN isqrtd, long sens)
{
  long av,tetpil,i;
  GEN form;

  av=avma;form=tabform[1][ex[1]];
  for(i=2;i<=cardtab;i++)
    {form=comprealform3(form,tabform[i][ex[i]],d,isqrtd,sens);}
  tetpil=avma;
  return gerepile(av,tetpil,gcopy(form));
}

GEN initializeform5(long *ex, GEN **tabform, long cardtab, GEN d, GEN isqrtd, GEN sqrtd, long sens)
{
  long av,tetpil,i;
  GEN form;

  av=avma;form=tabform[1][ex[1]];
  for(i=2;i<=cardtab;i++)
    form=comprealform5(form,tabform[i][ex[i]],d,isqrtd,sqrtd,sens);
  tetpil=avma;
  return gerepile(av,tetpil,gcopy(form));
}

void addcolumnmat(long **mat, long s, long cardsub, long *numbase, long *subbase, long *ex, long nbprimfact, long *primfact, long *expoprimfact, GEN form1, GEN log2precis, GEN vecexpo)
{
  long i,j,pp,b,ep;

  for(i=1;i<=cardsub;i++) mat[s][numbase[subbase[i]]]= -ex[i];
  for(j=1;j<=nbprimfact;j++)
    {
      pp=primfact[j];ep=expoprimfact[j];b=itos(modis((GEN)form1[2],(pp<<1)));
      if(b>pp) ep= -ep;mat[s][numbase[pp]]+=ep;
    }
  affrr(shiftr(mpadd(mulir(mulsi(EXP220,(GEN)form1[4]),log2precis),
		     mplog(absr((GEN)form1[5]))),-1),(GEN)vecexpo[s]);
}

void addcolumnmat1(long **mat, long s, long cardsub, long *numbase, long *subbase, long *ex, long nbprimfact1, long *primfact1, long *expoprimfact1, GEN form1, long *fpd, long nbprimfact, long *primfact, long *expoprimfact, GEN form2, GEN log2precis, GEN vecexpo)
{
  long i,j,pp,b1,ep;

  for(i=1;i<=cardsub;i++) mat[s][numbase[subbase[i]]]=fpd[i+1]-ex[i];
  for(j=1;j<=nbprimfact;j++)
    {
      pp=primfact[j];ep=expoprimfact[j];b1=itos(modis((GEN)form2[2],(pp<<1)));
      if(b1>pp) ep= -ep;mat[s][numbase[pp]]-=ep;
    }
  for(j=1;j<=nbprimfact1;j++)
    {
      pp=primfact1[j];ep=expoprimfact1[j];b1=itos(modis((GEN)form1[2],(pp<<1)));
      if(b1>pp) ep= -ep;mat[s][numbase[pp]]+=ep;
    }
  affrr(shiftr(mpadd(mulir(mulsi(EXP220,subii((GEN)form1[4],(GEN)form2[4])),log2precis),
		     mplog(absr(divrr((GEN)form1[5],(GEN)form2[5])))),-1),(GEN)vecexpo[s]);
}

void addcolumnmat2(long **mat, long s, long cardsub, long *numbase, long *subbase, long *ex, long nbprimfact1, long *primfact1, long *expoprimfact1, GEN form1, long *fpd, long nbprimfact, long *primfact, long *expoprimfact, GEN form2, GEN log2precis, GEN vecexpo)
{
  long i,j,pp,b1,ep;

  for(i=1;i<=cardsub;i++) mat[s][numbase[subbase[i]]]=-fpd[i+1]-ex[i];
  for(j=1;j<=nbprimfact;j++)
    {
      pp=primfact[j];ep=expoprimfact[j];b1=itos(modis((GEN)form2[2],(pp<<1)));
      if(b1>pp) ep= -ep;mat[s][numbase[pp]]+=ep;
    }
  for(j=1;j<=nbprimfact1;j++)
    {
      pp=primfact1[j];ep=expoprimfact1[j];b1=itos(modis((GEN)form1[2],(pp<<1)));
      if(b1>pp) ep= -ep;mat[s][numbase[pp]]+=ep;
    }
  affrr(shiftr(mpadd(mulir(mulsi(EXP220,addii((GEN)form1[4],(GEN)form2[4])),log2precis),mplog(absr(mulrr((GEN)form1[5],(GEN)form2[5])))),-1),(GEN)vecexpo[s]);
}

void initbuchreal(GEN D, double cbach, double cbach2, long *precreg, GEN *log2precis, long *qqq, GEN *dr, double *drc, double *logd, GEN *sqrtD, GEN *isqrtD, double *lim, long *limc, long *limbach, long prec)
{
  long cp;
	 
  /* precision en digits decimaux=(#digits decimaux de D)+50 */
  /* ici CBUCH=15,q=27 */
  /* on prendra les p decomposes tels que prod(p)>lim dans la subbase */
  /* limc=Max(c.(log(D))^2,exp((1/8).sqrt(log(D).loglog(D)))) */
  /* limbach=Max(6.(log(D))^2,exp((1/8).sqrt(log(D).loglog(D)))) */
  /* subbase contient les p decomposes tels que prod(p)>sqrt(D) */
  /* cardsubbase=cardsub=subbase[0]=#subbase; */
  /* tabprform est la table des form[p] pour p dans subbase */
  /* nbram est le nombre de p divisant D elimines dans subbase */
  /* tabpowprform est la table des puissances des formes dans tabprform */

  *precreg=max(prec,(gexpo(D)>>TWOPOTBITS_IN_LONG)+5);
  *log2precis=glog(gdeux,*precreg);
  (*qqq)=BITS_IN_LONG-1-(long)ceil(log((double)CBUCH)/log(2.0));
  *dr=cgetr(3);affir(D,*dr);
  *drc=rtodbl(*dr);
  *logd=log(*drc);
  *sqrtD=gsqrt(D,*precreg);
  *isqrtD=gfloor(*sqrtD);
  *lim=sqrt(*drc);
  (*limc)=max((long)(cbach*(*logd)*(*logd)),13);
  cp=(long)exp(sqrt((*logd)*log((*logd))/8.0));
  if(cp>(*limc)) *limc=cp;
  (*limbach)=max(20,(long)(cbach2*(*logd)*(*logd)));
  if((*limc)>(*limbach)) *limbach=*limc;
}

#define LIMP 30000

GEN lfunc(GEN D)
{
  GEN y;
  long av=avma,tetpil,prime=0;
  byteptr p=diffptr;
  
  prime=*p++;affsr(1,y=cgetr(4));
  do
    {
      if(!*p) err(recprimer);
      y=mulsr(prime,divrs(y,prime-krogs(D,prime)));
      prime+=*p++;
    }
  while(prime<=LIMP);
  tetpil=avma;return gerepile(av,tetpil,gcopy(y));
}

