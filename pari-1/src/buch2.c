/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*                                                                 */
/*            ALGORITHMES SOUS-EXPONENTIELS DE CALCUL              */
/*            DU GROUPE DE CLASSES ET DU REGULATEUR                */
/*                   CORPS DE NOMBRES GENERAUX                     */
/*                      (McCURLEY, BUCHMANN)                       */
/*                                                                 */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include "genpari.h"
const long CBUCHG = 15; /*DOIT ETRE DE LA FORME 2^k-1 !!!*/

GEN initalgall0(GEN x, long fldif, long prec);
GEN subfactorbasegen(long N, long m, GEN vectbase, long *vperm, long* ptss);
GEN **powsubfactgen(GEN nf, GEN w, long a, long PRECREG, long PRECREGINT);
GEN idealmulprimered(GEN nf, GEN x, GEN vp, long PRECREG);
GEN idealmulprimeredall(GEN nf, GEN x, GEN vp, long PRECREG,long PRECREGINT);
GEN getfu(GEN nf, GEN xarch, GEN reg, long *pte, long PRECREG);
long factorisegen(GEN nf, GEN ideal, long kcz, long limp, long *primfact, long *expoprimfact, long *primfactorbase, GEN *idealbase, long *numideal, long *numprimfactorbase, long limhash);
long factorisealpha(GEN nf, GEN alpha, long kcz, long limp, long *primfact, long *expoprimfact, long *primfactorbase, GEN *idealbase, long *numideal, long *numprimfactorbase, long limhash);
long factorbasegen(GEN nf, long n2, long n, long **ptnumprim, long **ptprim, long **ptnum, GEN **ptideal, long *ptkc, long *ptkcz, long *ptkcz2, GEN *ptlfun);
GEN cleancol(GEN x,long N,long RU,long PRECREG);

GEN buchall(GEN P, GEN gcbach, GEN gcbach2, GEN gRELSUP, long flun, long prec)
{
  long i,j,k,ss,lgsub,n1,cp,av=avma,av0,tetpil,*p1,*ex,q,s,nbtest,av3;
  long N,R1,R2,RU,CO,PRECREG,PRECREGINT,LIMC,LIMC2,lim,KC,KC2,KCZ,KCZ2,KCCO,KCCOPRO;
  long ip,c,lo,extrarel,iz,pz,ncz,a,b,comp;
  long col,mitcol,k0,nlze,sizeofmit,sreg,ep,colnew;
  long **mat,**matinit,*vperm,*vinvperm,*subvperm;
  long limhash=0,fpc,bou,sinit;
  long expoprimfact[500],primfact[500],*numprim,*prim,*numideal,nrelsup,nreldep;
  double cbach,cbach2,drc,mr,LOGD;
  GEN pgen1,pgen2,pgen3,pgen4,pgen5,pgen6,pgen21,pgen41,genb;
  GEN dr,ideal,idealpro,ideal2,**vp,*idealbase,matarch,exu,fu,zu;
  GEN nf,D,F,BINV,primsubfactorbase,vectbase;
  GEN xarch,xreal,vei,p3,met,mot,mit,reg,mdet,lfun,z,clh;
  GEN matalpha,u1u2,u1,u2,RES,basecl,extramat,extramatc;
  GEN lambda,sublambda_1,c_1,den,image_mdet,pdep;
  FILE *temp;

  if(DEBUGLEVEL) timer2();
  if(abs(flun)>1) RES=cgetg(11,17);
  else RES=flun?cgetg(9,17):cgetg(8,17);
  if(typ(gRELSUP)!=1) gRELSUP=gtrunc(gRELSUP);
  if(gsigne(gRELSUP)<=0) err(bucher5);
  N=lgef(P)-3;
  if(N<=1)
    {
      RES[1]=(long)polx[0];pgen1=cgetg(3,17);RES[2]=(long)pgen1;pgen1[1]=un;
      pgen1[2]=zero;pgen1=cgetg(3,17);RES[3]=(long)pgen1;pgen1[1]=un;pgen1[2]=un;
      pgen1=cgetg(2,17);RES[4]=(long)pgen1;pgen1[1]=un;
      pgen1=cgetg(4,17);RES[5]=(long)pgen1;pgen1[1]=un;pgen1[2]=lgetg(1,17);
      pgen1[3]=lgetg(1,17);
      RES[6]=un;RES[7]=un;
      if(flun) 
	{
	  pgen1=cgetg(3,17);RES[8]=(long)pgen1;
	  pgen1[1]=deux;pgen1[2]=lneg(gun);
	}
      if(abs(flun)>1) {RES[9]=lgetg(1,19);RES[10]=lstoi(EXP220);}
      if(flun>=0) {z=cgetg(2,19);z[1]=(long)RES;return z;}
      else
	{
	  z=cgetg(9,17);
	  z[1]=lgetg(1,19);z[2]=lgetg(1,19);z[3]=lgetg(1,19);
	  z[4]=lgetg(1,19);z[5]=lgetg(1,18);z[6]=lgetg(1,17);
	  z[7]=(long)initalg0(polx[0],5);
	  ncz=(flun==-1)?4:6;
	  pgen1=cgetg(ncz+1,17);z[8]=(long)pgen1;
	  for(i=1;i<=ncz;i++) pgen1[i]=lcopy((GEN)RES[i+4]);
	  return z;
	}
    }
  pgen1=factor(P);pgen2=(GEN)pgen1[1];pgen3=(GEN)pgen1[2];
  if((lg(pgen2)>2)||(cmpis((GEN)pgen3[1],1)>0)) err(bucher10);
  if(DEBUGLEVEL) {fprintf(errfile,"temps trivialites: ");fprintf(errfile,"%ld\n",timer2());fflush(errfile);}
  nf=initalgall0(P,flun>=0?0:1,max(8,prec));
  if(DEBUGLEVEL) {fprintf(errfile,"temps initalg0: ");fprintf(errfile,"%ld\n",timer2());fflush(errfile);}
  RES[1]=nf[1];RES[2]=nf[2];pgen1=cgetg(3,17);RES[3]=(long)pgen1;pgen1[1]=nf[3];pgen1[2]=nf[4];
  R1=itos((GEN)((GEN)nf[2])[1]);R2=(N-R1)/2;RU=R1+R2;
  F=(GEN)nf[4];D=(GEN)nf[3];RES[4]=nf[7];
  BINV=(GEN)nf[8];
  zu=rootsof1(nf);
  CO=itos(gRELSUP)+RU-1;
  PRECREGINT=(gexpo(D)>>TWOPOTBITS_IN_LONG)+N+3;
  PRECREG=max(prec,PRECREGINT);
  dr=cgetr(3);affir(D,dr);drc=rtodbl(dr);
  LOGD=log(fabs(drc));mr=LOGD*log(LOGD);
  lim=max((long)(exp(-(N+0.))*sqrt(2*PI*N*fabs(drc))*pow(4/PI,R2+0.)),3);
  cbach=gtodouble(gcbach);cbach2=gtodouble(gcbach2);cp=(long)exp(sqrt(mr/8.0));
  av0=avma;
 increasegen:
  if(DEBUGLEVEL) fprintf(errfile,"cbach = %lf\n",cbach);
  nreldep=nrelsup=0;
  LIMC=(long)(cbach*LOGD*LOGD);
  if(cp>LIMC) LIMC=cp;
  LIMC2=max(3*N,(long)(cbach2*LOGD*LOGD));
  if(LIMC>LIMC2) LIMC2=LIMC;
  if(DEBUGLEVEL) {fprintf(errfile,"temps rootsof1 et/ou trivialites: ");fprintf(errfile,"%ld\n",timer2());fflush(errfile);}
  KC2=factorbasegen(nf,LIMC2,LIMC,&numprim,&prim,&numideal,&idealbase,&KC,&KCZ,&KCZ2,&lfun);
  if(DEBUGLEVEL) {fprintf(errfile,"temps creation de la factor base: ");fprintf(errfile,"%ld\n",timer2());fflush(errfile);}
  if(DEBUGLEVEL) {fprintf(errfile,"KC = %ld\n",KC);fflush(errfile);}
  if(!KC)
    {
      if(cbach>11.99) 
	err(talker,"sorry, buchxxx is not able to compute this field PLEASE REPORT!!!");
      avma=av0;cbach=min(2*cbach,12);cbach2=max(cbach2,cbach);goto increasegen;
    }
  z=gmul(gmul(gmul(gpuigs(gdeux,-R1),gpuigs(gmul2n(mppi(5),1),-R2)),gsqrt(gabs(D),5)),lfun);
  vectbase=cgetg(KC+1,18);
  for(i=1;i<=KCZ;i++) 
    {
      ip=numideal[prim[i]];pgen1=idealbase[i];n1=lg(pgen1);
      for(j=1;j<n1;j++) vectbase[ip+j]=pgen1[j];
    }
  vperm=(long*)malloc(sizeof(long)*(KC+1));for(i=1;i<=KC;i++) vperm[i]=i;
  primsubfactorbase=subfactorbasegen(N,min(lim,LIMC2),vectbase,vperm,&ss);
  if(DEBUGLEVEL) {fprintf(errfile,"temps creation de la sous-factor base: ");fprintf(errfile,"%ld\n",timer2());fflush(errfile);}
  if(ss==-1) 
    {
      free(vperm);
      if(cbach>11.99) 
	err(talker,"sorry, buchxxx is not able to compute this field PLEASE REPORT!!!");
      avma=av0;cbach=min(2*cbach,12);cbach2=max(cbach2,cbach);goto increasegen;
    }
  if(DEBUGLEVEL>2) {fprintf(errfile,"sous-factor base =\n");output(primsubfactorbase);}
  lgsub=lg(primsubfactorbase);
  if(DEBUGLEVEL) fprintf(errfile,"sous-factor base a %ld elements\n",lgsub-1);
  subvperm=(long*)malloc(sizeof(long)*lgsub);
  for(i=1;i<lgsub;i++) subvperm[i]=vperm[i];
  vp=powsubfactgen(nf,primsubfactorbase,CBUCHG+1,PRECREG,PRECREGINT);
  if(DEBUGLEVEL) {fprintf(errfile,"temps powsubfactgen: ");fprintf(errfile,"%ld\n",timer2());fflush(errfile);}
  KCCO=KC+max(CO,ss);
  if(DEBUGLEVEL) {fprintf(errfile,"KCCO = %ld\n",KCCO);fflush(errfile);}
  mat=(long**)malloc(sizeof(long*)*(KCCO+1));
  matinit=(long**)malloc(sizeof(long*)*(KCCO+1));
  matarch=cgetg(KCCO+1,19);
  for(i=1;i<=KCCO;i++)
    {
      p1=(long *)malloc(sizeof(long)*(KC+1));matinit[i]=mat[i]=p1;
      for(j=1;j<=KC;j++) p1[j]=0;
      pgen1=cgetg(RU+1,18);matarch[i]=(long)pgen1;
      for(j=1;j<=RU;j++) 
	{
	  pgen2=cgetg(3,6);pgen1[j]=(long)pgen2;
	  pgen2[1]=lgetr(PRECREG);pgen2[2]=lgetr(PRECREG);
	}
    }
  ex=(long*)malloc(lgsub<<TWOPOTBYTES_IN_LONG);exu=cgetg(RU+1,17);
  q=BITS_IN_LONG-1-(long)ceil(log((double)CBUCHG)/log(2.0));
  s=0;
  for(i=1;i<=KCZ;i++)
    {
      ip=numideal[prim[i]];pgen1=idealbase[i];
      n1=lg(pgen1);k=0;
      for(j=1;j<n1;j++) k+=(itos((GEN)((GEN)pgen1[j])[3])*itos((GEN)((GEN)pgen1[j])[4]));
      if(k==N)
	{
	  s++;for(j=1;j<n1;j++) mat[s][ip+j]=itos((GEN)((GEN)pgen1[j])[3]);
	  for(j=1;j<=RU;j++) gaffsg(0,(GEN)((GEN)matarch[s])[j]);
	}
    }
  nbtest=ss=s;
  if(DEBUGLEVEL)
    {
      fprintf(errfile,"temps creation mat et matarch: ");fprintf(errfile,"%ld\n",timer2());fflush(errfile);
      fprintf(errfile,"Apres relations triviales, s = nbtest = %ld\n",s);fflush(errfile);
    }
/* Recherche de relations par elements de petite norme */
/*
  pgen1=minimprim((GEN)((GEN)nf[5])[3],(long)(N*pow(4.*((double)LIMC)*LIMC,(double)(3./N))),1000);
  k=itos((GEN)pgen1[1])>>1;pgen1=(GEN)pgen1[3];
  if(DEBUGLEVEL)
    {
      fprintf(errfile,"temps minimprim: ");fprintf(errfile,"%ld\n",timer2());fflush(errfile);
      fprintf(errfile,"place de stockage prevue dans minim = 1000, nombre trouve = %ld\n",k);fflush(errfile);
    }
  pgen1=vecsort(pgen1,gun);
  for(k=1;(s<KCCO)&&(k<lg(pgen1));k++)
    {
      av3=avma;
      pgen2=(GEN)((GEN)pgen1[k])[2];
      j=N;while((j>=2)&&(!signe((GEN)pgen2[j]))) --j;
      if(j>1)
	{
	  fpc=factorisealpha(nf,pgen2,KCZ,LIMC,primfact,expoprimfact,prim,idealbase,numideal,numprim,limhash);
	  nbtest++;
	  if(fpc)
	  {
	    pgen5=gmul((GEN)((GEN)nf[5])[1],pgen2);
	    pgen4=cgetg(RU+1,17);
	    for(i=1;i<=R1;i++) pgen4[i]=(long)glog((GEN)pgen5[i],PRECREG);
	    for(i=R1+1;i<=RU;i++) pgen4[i]=lmul2n(glog((GEN)pgen5[i],PRECREG),1);
	      s++;
	      if(DEBUGLEVEL>1) fprintf(errfile," %ld",s);
	      for(j=1;j<=primfact[0];j++) mat[s][primfact[j]]=expoprimfact[j];
	      for(j=1;j<=RU;j++) gaffect((GEN)(pgen4)[j],(GEN)((GEN)matarch[s])[j]);
	    }
	}
      avma=av3;
    }
  if(DEBUGLEVEL)
    {
      fprintf(errfile,"temps relations de petite norme: ");fprintf(errfile,"%ld\n",timer2());fflush(errfile);
      fprintf(errfile,"Apres relations de petite norme, s/nbtest = %ld/%ld\n",s,nbtest);fflush(errfile);
    }
*/
/* recherche de relations a partir de a+bx */
/*
  pgen3=idmat(N);
  pgen1=(GEN)((GEN)((GEN)nf[5])[1])[2];
  pgen4=cgetg(N+1,19);
  pgen21=cgetg(N+1,18);for(i=1;i<=N;i++) pgen21[i]=zero;
  for(b=1;(b<=5)&&(s<KCCO);b++)
    {
      genb=stoi(b);
      for(j=1;(j<=R1)&&(s<KCCO);j++) 
	{
	  pgen2=ground(gmulsg(b,(GEN)pgen1[j]));
	  for(a=-20;(a<=20)&&(s<KCCO);a++)
	    {
	      av3=avma;
	      pgen21[1]=(long)addsi(a,pgen2);pgen21[2]=(long)genb;
	      if(gcmp1(mppgcd((GEN)pgen21[1],(GEN)pgen21[2])))
		{
		  for(i=1;i<=N;i++) pgen4[i]=(long)element_mulh(nf,i,2,(GEN)pgen3[i],pgen21);
		  pgen6=hnfmod(pgen4,detint(pgen4));
a remplacer par factorisealpha
		  fpc=factorisegen(nf,pgen6,KCZ,LIMC,primfact,expoprimfact,prim,idealbase,numideal,numprim,limhash);
		  nbtest++;
		  if(fpc)
		    {
		      ideal=cgetg(3,17);ideal[1]=(long)pgen6;
		      pgen5=gmul((GEN)((GEN)nf[5])[1],pgen21);
		      pgen41=cgetg(RU+1,17);ideal[2]=(long)pgen41;
		      for(i=1;i<=R1;i++) pgen41[i]=(long)glog((GEN)pgen5[i],PRECREG);
		      for(i=R1+1;i<=RU;i++) pgen41[i]=lmul2n(glog((GEN)pgen5[i],PRECREG),1);
		      s++;
		      if(DEBUGLEVEL>1) 
			fprintf(errfile,"(a,b)=(%ld,%ld), s=%ld ",a,b,s);
		      for(j=1;j<=primfact[0];j++) mat[s][primfact[j]]=expoprimfact[j];
		      for(j=1;j<=RU;j++) gaffect((GEN)((GEN)ideal[2])[j],(GEN)((GEN)matarch[s])[j]);
		    }
		}
	      avma=av3;
	    }
	}
    }
  if(DEBUGLEVEL)
    {
      fprintf(errfile,"temps relations a+bw: ");fprintf(errfile,"%ld\n",timer2());fflush(errfile);
      fprintf(errfile,"Apres relations a+bw, s/nbtest = %ld/%ld\n",s,nbtest);fflush(errfile);
    }
*/
  pgen2=cgetg(2,17);pgen2[1]=(long)vp[1][0];
  for(iz=1;s<KCCO;iz++)
    {
      pgen1=(iz<=KCZ)?idealbase[numprim[pz=prim[iz]]]:pgen2;
      for(j=1;j<lg(pgen1);j++) 
	{
	  for(i=1;i<lgsub;i++) ex[i]=mymyrand()>>q;
	  av3=avma;sinit=s;
	  ideal=(iz<=KCZ)?idealmulprime(nf,vp[1][0],(GEN)pgen1[j]):vp[1][0];
	  for (i=1;i<lgsub;i++) ideal=idealmulh(nf,ideal,(GEN)vp[i][ex[i]]);
	  for(bou=1;(bou<=RU)&&(s==sinit);bou++)
	    {
	      if(bou==1) {for(i=1;i<=RU;i++) exu[i]=lstoi((mymyrand()>>q)+1);}
	      else
		{
		  for(i=1;i<=RU;i++) exu[i]=zero;
		  exu[bou]=lstoi(10);
		}
	      idealpro=ideallllredpart1(nf,(GEN)ideal[1],exu,PRECREGINT);
	      fpc=factorisegen(nf,idealpro,KCZ,LIMC,primfact,expoprimfact,prim,idealbase,numideal,numprim,limhash);
	      if((DEBUGLEVEL==2)&&(!fpc)) fprintf(errfile,".");
	      if(DEBUGLEVEL>3)
		{
		  fprintf(errfile,"s = %ld, nbtest = %ld, ideal = ",s,nbtest);
		  output((GEN)idealpro[1]);
		}
	      nbtest++;
	      if (fpc&&(s<KCCO))
		{
		  ideal2=ideallllredpart2(nf,(GEN)ideal[2],(GEN)idealpro[2],PRECREG);
		  s++;
		  if(DEBUGLEVEL==2) fprintf(errfile," %ld",s);
		  if(DEBUGLEVEL==3)
		    {
		      fprintf(errfile,"s = %ld, nbtest = %ld, ideal = ",s,nbtest);
		      output((GEN)ideal[1]);
		    }
		  if(DEBUGLEVEL>3) fprintf(errfile,"\n");
		  for(i=1;i<lgsub;i++) mat[s][subvperm[i]]= -ex[i];
		  for(i=1;i<=primfact[0];i++) mat[s][primfact[i]]+=expoprimfact[i];
		  if(iz<=KCZ) mat[s][numideal[pz]+j]--;
		  for(i=1;i<=RU;i++) gaffect((GEN)ideal2[i],(GEN)((GEN)matarch[s])[i]);
		}
	    }
	  avma=av3;if((sinit==s)&&(s<KCCO)) j--;
	}
    }
  if(DEBUGLEVEL)
    {
      if(DEBUGLEVEL>1) fprintf(errfile,"\n");      
      fprintf(errfile,"temps pour trouver les relations aleatoires: ");
      fprintf(errfile,"%ld\n",timer2());fflush(errfile);
      fprintf(errfile,"nbrelations/nbtest = %ld/%ld\n",s,nbtest);fflush(errfile);
    }
  if(DEBUGLEVEL>2)
    {
      if(KCZ2>KCZ)
	fprintf(errfile,"be honest for primes from %ld to %ld\n",prim[KCZ+1],prim[KCZ2]);
    }
  for(iz=KCZ+1;iz<=KCZ2;iz++)
    {
      pgen1=idealbase[numprim[pz=prim[iz]]];
      if(DEBUGLEVEL>2)
	{
	  fprintf(errfile,"treating p = %ld\n",pz);
	}
      for(j=1;j<lg(pgen1);j++) 
	{
	  sinit=s;nbtest=0;
	  do
	    {
	      for(i=1;i<lgsub;i++) ex[i]=mymyrand()>>q;
	      av3=avma;
	      ideal=idealmulprime(nf,idmat(N),(GEN)pgen1[j]);
	      for (i=1;i<lgsub;i++) ideal=idealmulh(nf,ideal,(GEN)vp[i][ex[i]]);
	      for(bou=1;(bou<=RU)&&(s==sinit);bou++)
		{
		  if(bou==1) {for(i=1;i<=RU;i++) exu[i]=lstoi((mymyrand()>>q)+1);}
		  else
		    {
		      for(i=1;i<=RU;i++) exu[i]=zero;
		      exu[bou]=lstoi(10);
		    }
		  idealpro=ideallllredpart1(nf,ideal,exu,PRECREGINT);
		  nbtest++;
		  fpc=factorisegen(nf,idealpro,iz-1,prim[iz-1],primfact,expoprimfact,prim,idealbase,numideal,numprim,limhash);
		  if(fpc) s++;
		}
	      avma=av3;
	    }
	  while((sinit==s)&&(nbtest<100));
	  if(sinit==s) err(talker,"too many iterations in be honest");
	}
    }
  if(DEBUGLEVEL)
    {
      fprintf(errfile,"temps be honest: ");fprintf(errfile,"%ld\n",timer2());fflush(errfile);
    }
  k0=lgsub-1;
  mit=hnfspec(mat,&pdep,&matarch,vperm,&matalpha,KCCO,KC,k0,&nlze,&col);
  if(DEBUGLEVEL)
    {
      fprintf(errfile,"temps hnfspec: ");fprintf(errfile,"%ld\n",timer2());fflush(errfile);
    }
  mitcol=lg(mit)-1;
  KCCOPRO=KCCO;
 morerelgen:
  if(nlze)
    {
      mitcol=lg(mit)-1;
      vinvperm=(long*)malloc(sizeof(long)*(KC+1));
      for(i=1;i<=KC;i++) vinvperm[vperm[i]]=i;
      s=0;extrarel=nlze;
      nbtest=0;
      extramat=cgetg(extrarel+1,19);extramatc=cgetg(extrarel+1,19);
      if(DEBUGLEVEL>1)
	fprintf(errfile,"recherche de %ld relations supplementaires\n",extrarel);
      for(j=1;j<=extrarel;j++) 
	{
	  extramat[j]=lgetg(KC+1,18);pgen1=cgetg(RU+1,18);
	  extramatc[j]=(long)pgen1;
	  for(i=1;i<=RU;i++) 
	    {
	      pgen2=cgetg(3,6);pgen1[i]=(long)pgen2;
	      pgen2[1]=lgetr(PRECREG);pgen2[2]=lgetr(PRECREG);
	    }
	}
      comp=0;
      while(s<extrarel)
	{
	  for (i=1;i<lgsub;i++) ex[i]=mymyrand()>>q;
	  av3=avma;sinit=s;ss=nrelsup?(s+comp)%(lgsub-1)+1:s+1;comp++;
	  ideal=idealmulprime(nf,vp[1][0],(GEN)vectbase[vperm[ss]]);
	  for (i=1;i<lgsub;i++) ideal=idealmulh(nf,ideal,(GEN)vp[i][ex[i]]);
	  for(bou=1;(bou<=RU)&&(s==sinit);bou++)
	    {
	      if(bou==1) {for(i=1;i<=RU;i++) exu[i]=lstoi((mymyrand()>>q)+1);}
	      else
		{
		  for(i=1;i<=RU;i++) exu[i]=zero;
		  exu[bou]=lstoi(10);
		}
	      idealpro=ideallllredpart1(nf,(GEN)ideal[1],exu,PRECREGINT);nbtest++;
	      if(DEBUGLEVEL==2) fprintf(errfile,".");
	      if(DEBUGLEVEL>2)
		{
		  fprintf(errfile,"s = %ld, nbtest = %ld, ideal = ",s,nbtest);
		  output((GEN)idealpro[1]);
		}
	      fpc=factorisegen(nf,idealpro,KCZ,LIMC,primfact,expoprimfact,prim,idealbase,numideal,numprim,limhash);
	      if(fpc==1)
		{
		  ideal2=ideallllredpart2(nf,(GEN)ideal[2],(GEN)idealpro[2],PRECREG);
		  s++;if(DEBUGLEVEL>1) fprintf(errfile," %ld",s);
		  pgen1=(GEN)extramat[s];
		  for(i=1;i<=KC;i++) pgen1[i]=zero;
		  for(i=1;i<lgsub;i++) pgen1[vinvperm[subvperm[i]]]=lstoi(-ex[i]);
		  pgen1[ss]=laddsi(-1,(GEN)pgen1[ss]);
		  for(i=1;i<=primfact[0];i++)
		    {
		      k=vinvperm[primfact[i]];
		      ep=expoprimfact[i];
		      pgen1[k]=laddsg(ep,(GEN)pgen1[k]);
		    }
		  if(gcmp0(pgen1)) s--;
		  else 
		    {
		      for(i=1;i<=RU;i++) 
			gaffect((GEN)ideal2[i],(GEN)((GEN)extramatc[s])[i]);
		    }
		  av3=avma;
		}
	    }
	  if(s==sinit) avma=av3;
	}
      if(DEBUGLEVEL)
	{
	  if(DEBUGLEVEL==2) fprintf(errfile,"\n");
	  fprintf(errfile,"temps calcul relations supplementaires: ");fprintf(errfile,"%ld\n",timer2());fflush(errfile);
	}
      if(nrelsup) nlze=0;
      if(DEBUGLEVEL>3) 
	{
	  fprintf(errfile,"avant hnfadd :\n");
	  fprintf(errfile,"vectbase[vperm[]] = \n");
	  fprintf(errfile,"[");
	  temp=outfile;outfile=errfile;
	  for(i=1;i<=KC;i++)
	    {
	      brute((GEN)vectbase[vperm[i]],'g',-1);if(i<KC) fprintf(errfile,",");
	    }
	  fprintf(errfile,"]~\n");fflush(errfile);outfile=temp;
	}
      mit=hnfadd(mit,&pdep,&matarch,vperm,&matalpha,KCCOPRO,KC,col,&nlze,extramat,extramatc);
      if(DEBUGLEVEL) {fprintf(errfile,"temps hnfadd: ");fprintf(errfile,"%ld\n",timer2());fflush(errfile);}
      free(vinvperm);KCCOPRO+=extrarel;col=KCCOPRO-(lg(matalpha)-1);
      if(nlze)
	{
	  nreldep++;
	  if(nreldep>5) 
	    {
	      for(i=1;i<lg(primsubfactorbase);i++) free(vp[i]);free(vp);
	      free(numprim);free(prim);free(numideal);free(idealbase);
	      free(ex);for(i=1;i<=KCCO;i++) free(matinit[i]);free(matinit);free(mat);
	      free(vperm);free(subvperm);
	      if(cbach>11.99) 
		err(talker,"sorry, buchxxx is not able to compute this field PLEASE REPORT!!!");
	      else
		{
		  avma=av0;cbach=min(2*cbach,12);cbach2=max(cbach2,cbach);
		  goto increasegen;
		}
	    }
	  else goto morerelgen;
	}
    }
  sizeofmit=lg(mit)-1;sreg=col-sizeofmit;
  xarch=cgetg(sreg+1,19);for(j=1;j<=sreg;j++) xarch[j]=matarch[j];
  xreal=greal(xarch);
  vei=cgetg(RU+1,18);for(i=1;i<=RU;i++) vei[i]=(i<=R1)?un:deux;
  mdet=cgetg(sreg+2,19);for(j=2;j<=sreg+1;j++) mdet[j]=xreal[j-1];
  mdet[1]=(long)vei;
  if(DEBUGLEVEL) {fprintf(errfile,"temps trivialites xarch, xreal, mdet: ");fprintf(errfile,"%ld\n",timer2());fflush(errfile);}
  image_mdet=imagereel(mdet,PRECREG);
  if(DEBUGLEVEL) {fprintf(errfile,"temps imagereel: ");fprintf(errfile,"%ld\n",timer2());fflush(errfile);}
  if(lg(image_mdet)!=(RU+1)) affsr(0,reg=cgetr(PRECREG));
  else reg=gdivgs(gabs(detreel(image_mdet),PRECREG),N);
  if(DEBUGLEVEL) {fprintf(errfile,"temps detreel: ");fprintf(errfile,"%ld\n",timer2());fflush(errfile);}
  if(gexpo(reg)<=-4)
    {
      nrelsup++;
      if(nrelsup>10) 
	{
	  for(i=1;i<lg(primsubfactorbase);i++) free(vp[i]);free(vp);
	  free(numprim);free(prim);free(numideal);free(idealbase);
	  free(ex);for(i=1;i<=KCCO;i++) free(matinit[i]);free(matinit);free(mat);
	  free(vperm);free(subvperm);
	  if(cbach>11.99) 
	    err(talker,"sorry, buchxxx is not able to compute this field PLEASE REPORT!!!");
	  else
	    {
	      avma=av0;cbach=min(2*cbach,12);cbach2=max(cbach2,cbach);
	      goto increasegen;
	    }
	}
      else 
	{
	  nlze=1;goto morerelgen;
	}
    }
  lambda=invmulmatreel(image_mdet,xreal);
  sublambda_1=cgetg(sreg+1,19);
  for(i=1;i<=sreg;i++) 
    {
      pgen1=cgetg(RU,18);sublambda_1[i]=(long)pgen1;pgen2=(GEN)lambda[i];
      for(j=1;j<RU;j++) pgen1[j]=pgen2[j+1];
    }
  if(DEBUGLEVEL) {fprintf(errfile,"temps trivialites lambda: ");fprintf(errfile,"%ld\n",timer2());fflush(errfile);}
  if(sizeofmit)
    {
      pgen1=gun;
      for(i=1;i<=sizeofmit;i++) pgen1=mulii(pgen1,gcoeff(mit,i,i));
      clh=pgen1;
      u1u2=smith2(mit);u1=(GEN)u1u2[1];u2=(GEN)u1u2[2];
      met=gmul(u1,gmul(mit,u2));u1=ginv(u1);
    }
  else {clh=gun;met=cgetg(1,19);u1=cgetg(1,19);}
  if(DEBUGLEVEL) {fprintf(errfile,"temps smith/groupe de classes: ");fprintf(errfile,"%ld\n",timer2());fflush(errfile);}
  c_1=gmul2n(gdiv(reg,gdiv(gmul(z,(GEN)zu[1]),clh)),1);
  sublambda_1=bestappr(sublambda_1,c_1);
  den=denom(sublambda_1);
  if(gcmp(den,c_1)>0) err(talker,"denominator too large, sorry");
  pgen1=(GEN)lllkerim(gmul(sublambda_1,den))[2];
  reg=gabs(gmul(reg,det(gmul(sublambda_1,pgen1))),PRECREG);
  c_1=gmul2n(gdiv(reg,gdiv(gmul(z,(GEN)zu[1]),clh)),1);
  if(DEBUGLEVEL) {fprintf(errfile,"temps bestappr/regulateur: ");fprintf(errfile,"%ld\n",timer2());fflush(errfile);}
  if(gcmpgs(gmul2n(c_1,1),3)<0) err(talker,"bug check in buchgen, PLEASE REPORT!!!");
  if(gcmpgs(c_1,3)>0) 
    {
      nrelsup++;
      if(nrelsup>10) 
	{
	  for(i=1;i<lg(primsubfactorbase);i++) free(vp[i]);free(vp);
	  free(numprim);free(prim);free(numideal);free(idealbase);
	  free(ex);for(i=1;i<=KCCO;i++) free(matinit[i]);free(matinit);free(mat);
	  free(vperm);free(subvperm);
	  if(cbach>11.99) 
	    fprintf(errfile,"\n  ***   Warning: check is greater than 1.5, suggest increasing extra relations\n");
	  else
	    {
	      avma=av0;cbach=min(2*cbach,12);cbach2=max(cbach2,cbach);
	      goto increasegen;
	    }
	}
      else 
	{
	  nlze=1;goto morerelgen;
	}
    }
  xarch=cleancol(gmul(xarch,pgen1),N,RU,PRECREG);
  lo=lg(met)-1;
  c=0;for(i=1;i<=lo;i++) if(!gcmp1(gcoeff(met,i,i))) c++;
  if(DEBUGLEVEL) {fprintf(errfile,"temps trivialites cleancol: ");fprintf(errfile,"%ld\n",timer2());fflush(errfile);}
  basecl=cgetg(c+1,17);
  for(j=1;j<=c;j++)
    {
      p3=(GEN)idealpowprime(nf,(GEN)vectbase[vperm[1]],pgen1=gcoeff(u1,1,j),PRECREG)[1];
      if(signe(pgen1)<0) p3=numer(p3);
      for(i=2;i<=lo;i++)
	{
	  pgen1=gcoeff(u1,i,j);s=signe(pgen1);
	  if(s)
	    {
	      if(s>0)
		p3=idealmulh(nf,p3,(GEN)idealpowprime(nf,(GEN)vectbase[vperm[i]],pgen1,PRECREGINT)[1]);
	      else
		p3=idealmulh(nf,p3,numer((GEN)idealpowprime(nf,(GEN)vectbase[vperm[i]],pgen1,PRECREGINT)[1]));
	      p3=ideallllredall(nf,p3,gzero,PRECREGINT,PRECREGINT);
	    }
	}
      basecl[j]=(long)p3;
    }
  mot=cgetg(c+1,17);for(i=1;i<=c;i++) mot[i]=coeff(met,i,i);
  pgen1=cgetg(4,17);RES[5]=(long)pgen1;pgen1[1]=(long)clh;pgen1[2]=(long)mot;
  pgen1[3]=(long)basecl;
  if(DEBUGLEVEL) {fprintf(errfile,"temps generateurs du groupe de classes: ");fprintf(errfile,"%ld\n",timer2());fflush(errfile);}
  RES[6]=(long)reg;
  z=gdiv(gmul(reg,clh),gmul((GEN)zu[1],z));
  RES[7]=(long)z;
  if(flun)
    {
      pgen1=cgetg(3,17);RES[8]=(long)pgen1;
      pgen1[1]=zu[1];pgen1[2]=lmul((GEN)nf[7],(GEN)zu[2]);
    }
  if(abs(flun)>1)
    {
      fu=getfu(nf,xarch,reg,&c,PRECREG);
      if(lg(fu)>1)
	{
	  pgen1=gmul((GEN)nf[7],fu);
	  for(j=1;j<lg(fu);j++) 
	    if(!gcmp1(gabs(gnorm(gmodulcp((GEN)pgen1[j],(GEN)nf[1])))))
	      {
		fprintf(errfile,"\n  ***   Warning: fundamental units too large, not given\n");
		c=0;fu=cgetg(1,17);
	      }
	}
      else c=0;
      RES[9]=c?lmul((GEN)nf[7],fu):(long)fu;
      RES[10]=lstoi(c);
      if(DEBUGLEVEL) {fprintf(errfile,"temps getfu: ");fprintf(errfile,"%ld\n",timer2());fflush(errfile);}
    }
  tetpil=avma;
  if(flun>=0) {z=cgetg(2,19);z[1]=lcopy(RES);}
  else
    {
      z=cgetg(9,17);z[1]=lcopy(mit);z[2]=lcopy(matalpha);
      z[3]=lcopy(xarch);
      colnew=lg(matarch)-1-sreg;pgen1=cgetg(colnew+1,19);z[4]=(long)pgen1;
      for(j=1;j<=colnew;j++) 
	pgen1[j]=(long)cleancol((GEN)matarch[j+sreg],N,RU,PRECREG);
      z[5]=lcopy(vectbase);pgen1=cgetg(KC+1,18);z[6]=(long)pgen1;
      for(i=1;i<=KC;i++) pgen1[i]=lstoi(vperm[i]);
      z[7]=lcopy(nf);
      ncz=(flun==-1)?4:6;
      pgen1=cgetg(ncz+1,17);z[8]=(long)pgen1;
      for(i=1;i<=ncz;i++) pgen1[i]=lcopy((GEN)RES[i+4]);
    }
  for(i=1;i<lg(primsubfactorbase);i++) free(vp[i]);free(vp);
  free(numprim);free(prim);free(numideal);free(idealbase);
  free(ex);for(i=1;i<=KCCO;i++) free(matinit[i]);free(matinit);free(mat);
  free(vperm);free(subvperm);
  return gerepile(av,tetpil,z);
  if(DEBUGLEVEL) {fprintf(errfile,"temps free finaux: ");fprintf(errfile,"%ld\n",timer2());fflush(errfile);}
}

GEN buchgen(GEN P, GEN gcbach, GEN gcbach2, long prec)
{
  return buchall(P,gcbach,gcbach2,stoi(5),0,prec);
}

GEN buchgenfu(GEN P, GEN gcbach, GEN gcbach2, long prec)
{
  return buchall(P,gcbach,gcbach2,stoi(5),2,prec);
}

GEN buchinit(GEN P, GEN gcbach, GEN gcbach2, long prec)
{
  return buchall(P,gcbach,gcbach2,stoi(5),-1,prec);
}

GEN buchinitfu(GEN P, GEN gcbach, GEN gcbach2, long prec)
{
  return buchall(P,gcbach,gcbach2,stoi(5),-2,prec);
}

/* Calcul de la base des sous-facteurs pour generer les relations entre ideaux. */

GEN subfactorbasegen(long N, long m, GEN vectbase, long *vperm, long* ptss)
{
  long av=avma,tetpil,i,j,k,s,s1,s2,e,nbidp,ss=0;
  GEN y,y1,y2,y3,p1,perm,perm1;
  double prod;

  k=lg(vectbase)-1;
  y=cgetg(k+1,18);y1=cgetg(k+1,18);y3=cgetg(k+1,18);
  s=0;s1=0;
  for(i=1;i<=k;i++) 
    {
      p1=(GEN)vectbase[i];y3[i]=(long)gpui((GEN)p1[1],(GEN)p1[4]);
      if((i>1)&&cmpii((GEN)p1[1],(GEN)((GEN)vectbase[i-1])[1]))
	{
	  if(s==N) y1[i-1]=zero;
	  s=0;
	  if(s1==N) ss++;
	  s1=0;
	}
      s2=(e=itos((GEN)p1[3]))*itos((GEN)p1[4]);
      s+=s2;s1+=s2;
      if(e>1) {y1[i]=zero;s=0;}
      else y1[i]=y3[i];
    }
  if(s==N) y1[k]=zero;
  if(s1==N) ss++;
  perm=indexsort(y1);
  i=1;while((i<=k)&&(!signe((GEN)y1[itos((GEN)(perm[i]))]))) i++;
  if(i>k) {*ptss= -1;avma=av;return gzero;}
  nbidp=0;prod=1.0;
  while((nbidp<=(k-i))&&(prod<m+0.5))
    {nbidp++;prod*=gtodouble((GEN)y1[itos((GEN)perm[nbidp+i-1])]);}
  if(prod<m) {*ptss= -1;avma=av;return gzero;}
  for(j=1;j<=nbidp;j++) y3[itos((GEN)(perm[j+i-1]))]=zero;
  perm1=indexsort(y3);
  for(j=1;j<=nbidp;j++) vperm[j]=itos((GEN)(perm[j+i-1]));
  for(j=nbidp+1;j<=k;j++) vperm[j]=itos((GEN)(perm1[j]));
  tetpil=avma;y2=cgetg(nbidp+1,18);
  for(j=1;j<=nbidp;j++) y2[j]=lcopy((GEN)vectbase[itos((GEN)perm[j+i-1])]);
  *ptss=ss;return gerepile(av,tetpil,y2);
}

/*Calcul la table des puissances des ideaux premiers de la base des sous-facteurs ;
w est la base des sous-facteurs, a est l'exposant maximum calcule : 
renvoie une matrice x de GEN , x[j][i] contenant
l'ideal (P_i)^j, sous forme HNF */

GEN **powsubfactgen(GEN nf, GEN w, long a, long PRECREG,long PRECREGINT)
{
  long i,j,n=lg(w)-1,N=lgef((GEN)nf[1])-3,R1,R2,RU;
  GEN **x,pgen1,id;

  R1=itos((GEN)((GEN)nf[2])[1]);R2=(N-R1)/2;RU=R1+R2;
  x=(GEN**)malloc(sizeof(GEN*)*(n+1));
  for(i=1;i<=n;i++) x[i]=(GEN*)malloc(sizeof(GEN)*(a+1));
  id=cgetg(3,17);id[1]=(long)idmat(N);pgen1=cgetg(RU+1,17);
  id[2]=(long)pgen1;for(i=1;i<=RU;i++) pgen1[i]=zero;
  for(i=1;i<=n;i++) 
    {
      x[i][0]=id;
      for(j=1;j<=a;j++) 
	{
	  if(DEBUGLEVEL>=2) {fprintf(errfile," %ld",j);fflush(errfile);}
	  x[i][j]=(j==1)?idealmulprime(nf,id,(GEN)w[i]):idealmulprimeredall(nf,x[i][j-1],(GEN)w[i],PRECREG,PRECREGINT);
	}
      if(DEBUGLEVEL>=2) {fprintf(errfile,"\n");fflush(errfile);}
    }
  return x;
}

GEN **newpowsubfactgen(GEN nf, GEN w, long a, long PRECREG)
{
  long i,j,n=lg(w)-1,N=lgef((GEN)nf[1])-3,R1,R2,RU;
  GEN **x,pgen1,pgen2,pgen3,pgen4,pgen5,pgen6,id;

  R1=itos((GEN)((GEN)nf[2])[1]);R2=(N-R1)/2;RU=R1+R2;
  x=(GEN**)malloc(sizeof(GEN*)*(n+1));
  for(i=1;i<=n;i++) x[i]=(GEN*)malloc(sizeof(GEN)*(a+1));
  id=cgetg(3,17);id[1]=(long)idmat(N);pgen1=cgetg(RU+1,17);
  id[2]=(long)pgen1;for(i=1;i<=RU;i++) pgen1[i]=zero;
  for(i=1;i<=n;i++) 
    {
      x[i][0]=id;
      pgen1=(GEN)w[i];x[i][1]=idealmulprime(nf,id,pgen1);
      pgen2=cgetg(6,17);pgen2[1]=pgen1[1];pgen2[2]=pgen1[5];pgen2[5]=pgen1[2];
      pgen2[3]=pgen2[4]=un;pgen4=x[i][1];
      pgen5=idealmulprime(nf,x[i][0],pgen2);
      pgen6=(GEN)pgen1[1];
      for(j=2;j<=a;j++)
	{
	  pgen4=idealmulprime(nf,pgen4,pgen1);
	  pgen5=idealmulprime(nf,pgen5,pgen2);
	  pgen6=mulii(pgen6,(GEN)pgen1[1]);
	  pgen3=minideal(nf,pgen5,gzero,2*PRECREG);
	  x[i][j]=idealmul(nf,pgen3,pgen4);x[i][j][1]=ldiv((GEN)(x[i][j][1]),pgen6);
	}
    }
  return x;
}

/*Calcul de la base de facteurs : n2 est la borne pour les nombres premiers
qui vont etre testes pour obtenir toutes les relations et la norme des ideaux
choisis, n est la borne des nombres premiers qui vont etre testes pour les 
relations primaires (constante de Bach).
Cette fonction cree et affecte un certain nombre de variables :
numprimfactorbase[i] est l'indice k tel que primfactorbase[k]=i (si i n'est
pas premier, numprimfactorbase[i]=0), ; primfactorbase[i] contient le i-eme
nombre premier utilise pour construire la base de facteurs ; numideal[i] est
l'indice k tel que idealbase[k]=i ; idealbase[i] contient les ideaux premiers
de norme convenable et au dessus du nombre premier numero i ;
KCZ contient le nombre de nombres premiers utilises pour construire la base 
de facteurs jusqu'a la constante de Bach, KC contient le nombre d'ideaux 
premiers jusqu'a la constante de Bach ; KCZ2 contient le nombre de nombres 
premiers de la base de facteurs au total ; enfin, la fonction renvoie le 
nombre d'ideaux premiers utilises au total. On n'utilise que des nombres
premiers ne divisant pas l'index F, et non inertes.*/

long factorbasegen(GEN nf, long n2, long n, long **ptnumprim, long **ptprim, long **ptnum, GEN **ptideal, long *ptkc, long *ptkcz, long *ptkcz2, GEN *ptlfun)
{
  byteptr delta=diffptr;
  long av1,tetpil,i,j,k,pp,fl,lon,ip,*numprimfactorbase,*primfactorbase;
  long *numideal,ip0;
  GEN prim,pgen1,pgen2,pgen3,pgen4,*idealbase,lfun;

  numprimfactorbase=(long*)malloc(sizeof(long)*(n2+1));
  primfactorbase=(long*)malloc(sizeof(long)*(n2+1));
  numideal=(long*)malloc(sizeof(long)*(n2+1));
  idealbase=(GEN*)malloc(sizeof(GEN)*(n2+1));
  lfun=cgetr(5);affsr(1,lfun);*ptlfun=lfun;
  i=0;pp=*delta++;fl=1;ip=0;*ptkc=0;
  while(pp<=n2)
    {
      av1=avma;
      prim=stoi(pp);
      if(DEBUGLEVEL>=2) {fprintf(errfile," %ld",pp);fflush(errfile);}
      pgen1=primedec(nf,prim);
      lon=lg(pgen1);divriz(mulir(subis(prim,1),lfun),prim,lfun);
      if((lon>2)||cmpis((GEN)((GEN)pgen1[1])[3],1)>0)
	{
	  ip0=ip;
	  pgen2=cgetg(lon,18);
	  for(j=0,k=1;k<lon;k++)
	    {
	      pgen3=(GEN)pgen1[k];pgen4=gpui(prim,(GEN)pgen3[4]);
	      if(cmpis(pgen4,n2)<=0)
		{
		  ip++;j++;pgen2[j]=(long)pgen3;
		  divriz(mulir(pgen4,lfun),subis(pgen4,1),lfun);
		}
	    }
	  i++;numprimfactorbase[pp]=i;primfactorbase[i]=pp;
	  numideal[pp]=ip0;tetpil=avma;pgen3=cgetg(j+1,17);
	  for(k=1;k<=j;k++) pgen3[k]=lcopy((GEN)pgen2[k]);
	  idealbase[i]=gerepile(av1,tetpil,pgen3);
	}
      else
	{
	  pgen4=gpui(prim,(GEN)((GEN)pgen1[1])[4]);
	  if(cmpis(pgen4,n2)<=0) 
	    divriz(mulir(pgen4,lfun),subis(pgen4,1),lfun);
	}
      pp+=*delta++;
      if((pp>n)&&fl) {*ptkc=ip;fl=0;*ptkcz=i;}
    }
  *ptkcz2=i;
  *ptnumprim=numprimfactorbase;*ptprim=primfactorbase;*ptnum=numideal;*ptideal=idealbase;
  if(DEBUGLEVEL>=2) fprintf(errfile,"\n");
  return ip;
}
/*
long factorisegen(GEN nf, GEN ideal, long kcz, long limp, long *primfact, long *expoprimfact, long *primfactorbase, GEN *idealbase, long *numideal, long *numprimfactorbase, long limhash)
{
  long sr,i,j,n1,ip,v,p,k,fl=1,av1,q1,lo,ifinal;
  GEN x,q,r,pg1,listexpo;

  av1=avma;lo=0;
  for(x=gun,i=1;i<lg(ideal);i++) x=mulii(x,gcoeff(ideal,i,i));
  if(gcmp1(x)) {avma=av1;primfact[0]=0;return 1;}
  listexpo=cgeti(kcz+1);
  for(i=1;(i<=kcz)&&fl;i++)
    {
      p=primfactorbase[i];q=dvmdis(x,p,&r);k=0;
      if(sr=(!signe(r)))
	{
	  x=q;while(sr) {k++;q=dvmdis(x,p,&r);if(sr=(!signe(r))) x=q;}
	}
      listexpo[i]=k;
      fl=(cmpis(q,p)>0);
    }
  if(fl) {avma=av1;return 0;}
  else
    {
      ifinal=i-1;
      if(cmpis(x,limp)>0) {avma=av1;return 0;}
      for(i=1;i<=ifinal;i++)
	{
	  p=primfactorbase[i];
	  if(k=listexpo[i])
	    {
	      pg1=idealbase[numprimfactorbase[p]];
	      n1=lg(pg1);ip=numideal[p];
	      for(j=1;(j<n1)&&k;j++)
		{
		  v=idealval(nf,ideal,(GEN)pg1[j]);
		  if(v) 
		    {
		      primfact[++lo]=ip+j;expoprimfact[lo]=v;
		      k-=v*itos((GEN)((GEN)pg1[j])[4]);
		    }
		}
	      if(k) {avma=av1;return 0;}
	    }
	}
      if(gcmp1(x)) {avma=av1;primfact[0]=lo;return 1;}
      else
	{
	  p=itos(x);
	  pg1=idealbase[numprimfactorbase[p]];
	  n1=lg(pg1);ip=numideal[p];k=1;
	  for(j=1;(j<n1)&&k;j++)
	    {
	      v=idealval(nf,ideal,(GEN)pg1[j]);
	      if(v) 
		{
		  primfact[++lo]=ip+j;expoprimfact[lo]=v;
		  k-=v*itos((GEN)((GEN)pg1[j])[4]);
		}
	    }
	  if(k) {avma=av1;return 0;}
	  avma=av1;primfact[0]=lo;return 1;
	}
    }
}
*/

long factorisegen(GEN nf, GEN idealvec, long kcz, long limp, long *primfact, long *expoprimfact, long *primfactorbase, GEN *idealbase, long *numideal, long *numprimfactorbase, long limhash)
{
  long sr,i,j,n1,ip,v,p,k,fl=1,av1,q1,lo,ifinal;
  GEN x,q,r,pg1,p1,listexpo,ideal=(GEN)idealvec[1];

  av1=avma;lo=0;
  for(x=gun,i=1;i<lg(ideal);i++)
    {p1=gcoeff(ideal,i,i);if(!gcmp1(p1)) x=mulii(x,p1);}
  x=gdiv((GEN)idealvec[3],x);
  if(gcmp1(x)) {avma=av1;primfact[0]=0;return 1;}
  listexpo=cgeti(kcz+1);
  for(i=1;(i<=kcz)&&fl;i++)
    {
      p=primfactorbase[i];q=dvmdis(x,p,&r);k=0;
      if(sr=(!signe(r)))
	{
	  x=q;while(sr) {k++;q=dvmdis(x,p,&r);if(sr=(!signe(r))) x=q;}
	}
      listexpo[i]=k;
      fl=(cmpis(q,p)>0);
    }
  if(fl) {avma=av1;return 0;}
  else
    {
      ifinal=i-1;
      if(cmpis(x,limp)>0) {avma=av1;return 0;}
      for(i=1;i<=ifinal;i++)
	{
	  p=primfactorbase[i];
	  if(k=listexpo[i])
	    {
	      pg1=idealbase[numprimfactorbase[p]];
	      n1=lg(pg1);ip=numideal[p];
	      for(j=1;(j<n1)&&k;j++)
		{
		  v=idealval(nf,ideal,(GEN)pg1[j])-elementval2(nf,(GEN)idealvec[2],(GEN)idealvec[3],(GEN)pg1[j]);
		  if(v) 
		    {
		      primfact[++lo]=ip+j;expoprimfact[lo]=v;
		      k+=v*itos((GEN)((GEN)pg1[j])[4]);
		    }
		}
	      if(k) {avma=av1;return 0;}
	    }
	}
      if(gcmp1(x)) {avma=av1;primfact[0]=lo;return 1;}
      else
	{
	  p=itos(x);
	  pg1=idealbase[numprimfactorbase[p]];
	  n1=lg(pg1);ip=numideal[p];k=1;
	  for(j=1;(j<n1)&&k;j++)
	    {
	      v=idealval(nf,ideal,(GEN)pg1[j])-elementval2(nf,(GEN)idealvec[2],(GEN)idealvec[3],(GEN)pg1[j]);
	      if(v) 
		{
		  primfact[++lo]=ip+j;expoprimfact[lo]=v;
		  k+=v*itos((GEN)((GEN)pg1[j])[4]);
		}
	    }
	  if(k) {avma=av1;return 0;}
	  avma=av1;primfact[0]=lo;return 1;
	}
    }
}

long factorisealpha(GEN nf, GEN alpha, long kcz, long limp, long *primfact, long *expoprimfact, long *primfactorbase, GEN *idealbase, long *numideal, long *numprimfactorbase, long limhash)
{
  long sr,i,j,n1,ip,v,p,k,fl=1,av1,q1,lo,ifinal;
  GEN d,x,q,r,pg1,listexpo;

  av1=avma;lo=0;
  d=x=absi(subres(gmul((GEN)nf[7],alpha),(GEN)nf[1]));
  if(gcmp1(x)) {avma=av1;primfact[0]=0;return 1;}
  listexpo=cgeti(kcz+1);
  for(i=1;(i<=kcz)&&fl;i++)
    {
      p=primfactorbase[i];q=dvmdis(x,p,&r);k=0;
      if(sr=(!signe(r)))
	{
	  x=q;while(sr) {k++;q=dvmdis(x,p,&r);if(sr=(!signe(r))) x=q;}
	}
      listexpo[i]=k;
      fl=(cmpis(q,p)>0);
    }
  if(fl) {avma=av1;return 0;}
  else
    {
      ifinal=i-1;
      if(cmpis(x,limp)>0) {avma=av1;return 0;}
      for(i=1;i<=ifinal;i++)
	{
	  p=primfactorbase[i];
	  if(k=listexpo[i])
	    {
	      pg1=idealbase[numprimfactorbase[p]];
	      n1=lg(pg1);ip=numideal[p];
	      for(j=1;(j<n1)&&k;j++)
		{
		  v=elementval2(nf,alpha,d,(GEN)pg1[j]);
		  if(v) 
		    {
		      primfact[++lo]=ip+j;expoprimfact[lo]=v;
		      k-=v*itos((GEN)((GEN)pg1[j])[4]);
		    }
		}
	      if(k) {avma=av1;return 0;}
	    }
	}
      if(gcmp1(x)) {avma=av1;primfact[0]=lo;return 1;}
      else
	{
	  p=itos(x);
	  pg1=idealbase[numprimfactorbase[p]];
	  n1=lg(pg1);ip=numideal[p];k=1;
	  for(j=1;(j<n1)&&k;j++)
	    {
	      v=elementval2(nf,alpha,d,(GEN)pg1[j]);
	      if(v) 
		{
		  primfact[++lo]=ip+j;expoprimfact[lo]=v;
		  k-=v*itos((GEN)((GEN)pg1[j])[4]);
		}
	    }
	  if(k) {avma=av1;return 0;}
	  avma=av1;primfact[0]=lo;return 1;
	}
    }
}

GEN idealmulprimeredall(GEN nf, GEN x, GEN vp, long PRECREG,long PRECREGINT)
{
  long av=avma,tetpil;
  GEN y;
  
  y=idealmulprime(nf,x,vp);tetpil=avma;
  return gerepile(av,tetpil,ideallllredall(nf,y,gzero,PRECREG,PRECREGINT));
}

GEN idealmulprimered(GEN nf, GEN x, GEN vp, long PRECREG)
{
  long av=avma,tetpil;
  GEN y;
  
  y=idealmulprime(nf,x,vp);tetpil=avma;
  return gerepile(av,tetpil,ideallllredall(nf,y,gzero,PRECREG,PRECREG));
}

GEN cleancol(GEN x,long N,long RU,long PRECREG)
{
  long i,j,av=avma,tetpil,tx=typ(x),R1=RU+RU-N;
  GEN s,s2,p1,p2,p3,p4,y;

  if(tx<17) err(talker,"not a vector/matrix in cleancol");
  if(tx==19) 
    {
      y=cgetg(lg(x),tx);
      for(j=1;j<lg(x);j++) y[j]=(long)cleancol((GEN)x[j],N,RU,PRECREG);
      return y;
    }
  p1=greal(x);s=gzero;for(i=1;i<=RU;i++) s=gadd(s,(GEN)p1[i]);
  s=gdivgs(s,-N);if(N>R1) s2=gmul2n(s,1);
  p2=gmul2n(mppi(PRECREG),2);p3=gimag(x);
  tetpil=avma;y=cgetg(RU+1,tx);
  for(i=1;i<=RU;i++)
    {
      p4=cgetg(3,6);y[i]=(long)p4;
      p4[1]=(i<=R1)?ladd((GEN)p1[i],s):ladd((GEN)p1[i],s2);
      p4[2]=(long)gmod((GEN)p3[i],p2);
    }
  return gerepile(av,tetpil,y);
}

GEN getfu(GEN nf, GEN xarch, GEN reg, long *pte, long PRECREG)
{
  long av=avma,av1,tetpil,i,j,RU,N=lgef((GEN)nf[1])-3,e,R1,R2;
  GEN p1,p2,p3,y,matep,s;

  R1=itos((GEN)((GEN)nf[2])[1]);R2=(N-R1)>>1;RU=R1+R2;
  if(RU==1) return cgetg(1,19);
  if(gexpo(reg)<-8) 
    {
      fprintf(errfile,"\n  ***   Warning: not enough relations for fundamental units, not given\n");
      return cgetg(1,19);
    }
  matep=cgetg(RU,19);
  for(j=1;j<RU;j++)
    {
      s=gzero;for(i=1;i<=RU;i++) s=gadd(s,greal(gcoeff(xarch,i,j)));
      s=gdivgs(s,N);
      p1=cgetg(N+1,18);matep[j]=(long)p1;
      for(i=1;i<=R1;i++)
	p1[i]=lsub(gcoeff(xarch,i,j),s);
      for(i=R1+1;i<=RU;i++)
	{
	  p1[i]=lsub(gmul2n(gcoeff(xarch,i,j),-1),s);
	  p1[i+R2]=lconj((GEN)p1[i]);
	}
    }
  matep=gexp(gmul(matep,p1=lll(greal(matep),PRECREG)),PRECREG);
  gmulz(xarch,p1,xarch);
  p1=(GEN)((GEN)nf[5])[1];
  p2=cgetg(N+1,19);
  for(j=1;j<=N;j++)
    {
      p3=cgetg(N+1,18);p2[j]=(long)p3;
      for(i=1;i<=R1;i++) p3[i]=(long)coeff(p1,i,j);
      for(i=R1+1;i<=RU;i++)
	{
	  p3[i]=(long)coeff(p1,i,j);p3[i+R2]=lconj((GEN)p3[i]);
	}
    }
  y=greal(grndtoi(invmulmatreel(p2,matep),&e));
  if(e>=0)
    {
      fprintf(errfile,"\n  ***   Warning: insufficient precision for fundamental units, not given\n");
      avma=av;return cgetg(1,19);
    }
  *pte= -e;
  for(j=1;j<lg(y);j++)
    {
      p1=(GEN)y[j];p2=(GEN)(ginv(gmodulcp(gmul((GEN)nf[7],p1),(GEN)nf[1]))[2]);
      p3=cgetg(N+1,18);for(i=1;i<lgef(p2)-1;i++) p3[i]=p2[i+1];
      for(;i<=N;i++) p3[i]=zero;
      p2=gmul((GEN)nf[8],p3);
      if(gcmp(gnorml2(p2),gnorml2(p1))<0) 
	{
	  p1=p2;gaffect(gneg((GEN)xarch[j]),(GEN)xarch[j]);
	}
      for(i=N;(i>=1)&&gcmp0((GEN)p1[i]);i--);
      if(gsigne((GEN)p1[i])<0)
	{
	  y[j]=lneg(p1);
	  av1=avma;p1=cgetg(RU+1,18);p2=gmul(gi,mppi(PRECREG));
	  for(i=1;i<=R1;i++) p1[i]=(long)p2;
	  for(;i<=RU;i++) p1[i]=lmul2n(p2,1);
	  gaddz((GEN)xarch[j],p1,(GEN)xarch[j]);avma=av1;
	}
      else y[j]=(long)p1;
    }
  tetpil=avma;return gerepile(av,tetpil,gcopy(y));
}

GEN gcdrealnoer(GEN a, GEN b)
{
  long av,tetpil,e;
  GEN k1,r;

  if(typ(a)==1)
    {
      if(typ(b)==1) return mppgcd(a,b);
      k1=cgetr(lg(b));affir(a,k1);a=k1;
    }
  else if(typ(b)==1) {k1=cgetr(lg(a));affir(b,k1);b=k1;}
  if(expo(a)<-5) return gabs(b);
  if(expo(b)<-5) return gabs(a);
  av=avma;a=absr(a);b=absr(b);
  while((expo(b)>=(-5))&&(signe(b)))
    {k1=gcvtoi(divrr(a,b),&e);r=subrr(a,mulir(k1,b));a=b;b=r;}
  tetpil=avma;return gerepile(av,tetpil,gabs(a));
}

long factorisegensimple(GEN nf, GEN ideal, long *primfact, long *expoprimfact, GEN vectbase)
{
  long i,p,v,fl=1,av1,av2,lo;
  GEN x,p1;

  av1=avma;lo=0;
  for(x=gun,i=1;i<lg(ideal);i++) x=mulii(x,gcoeff(ideal,i,i));
  if(gcmp1(x)) {avma=av1;primfact[0]=0;return 1;}
  av2=avma;fl=1;
  for(i=1;(i<lg(vectbase))&&fl;i++)
    {
      p1=(GEN)vectbase[i];p=itos((GEN)p1[1]);
      if(!signe(modis(x,p)))
	{
	  v=idealval(nf,ideal,p1);
	  if(v) 
	    {
	      primfact[++lo]=i;expoprimfact[lo]=v;
	      x=divii(x,gpui((GEN)p1[1],mulsi(v,(GEN)p1[4])));
	      if(gcmp1(x)) fl=0;
	    }
	}
    }
  primfact[0]=lo;return 1-fl;
}

GEN isprincipal(GEN bignf, GEN x)
{
  long expoprimfact[500],primfact[500],av=avma,tetpil,*vinvperm;
  long i,j,fpc,colmit,colnew,k,N,R1,R2,RU,e,pr,c,ss,nbtest,bou,q,tx=typ(x);
  GEN xalpha,yalpha,mit,matalpha,matunit,matalphac,vectbase,vperm,nf,a,RES;
  GEN u1u2,u1,u2,y,p1,p2,p3,p4,p5,s,s1,s2,om1,om2,om3,xar,pgen1,ideal,vdir,exu;
  long *ex;

  if((typ(bignf)!=17)||(lg(bignf)!=9))
    err(talker,"not a big number field vector in isprincipal");
  mit=(GEN)bignf[1];matalpha=(GEN)bignf[2];matunit=(GEN)bignf[3];
  matalphac=gcopy((GEN)bignf[4]);vectbase=(GEN)bignf[5];vperm=(GEN)bignf[6];
  nf=(GEN)bignf[7];N=lgef((GEN)nf[1])-3;
  RES=(GEN)bignf[8];
  if((tx==9)||(tx==10))
    {
      if(tx==9)
	{
	  if(!gegal((GEN)x[1],(GEN)nf[1]))
	    err(talker,"not the same number field in isprincipal");
	  x=(GEN)x[2];
	}
      avma=av;y=cgetg(4,17);c=lg((GEN)((GEN)RES[1])[2])-1;
      p1=cgetg(c+1,18);y[1]=(long)p1;for(i=1;i<=c;i++) p1[i]=zero;
      y[2]=lmul((GEN)nf[8],x);y[3]=lstoi(BIGINT);
      return y;
    }
  if((tx==17)&&(lg(x)==3)) x=(GEN)x[1];
  if((typ(x)==17)&&(lg(x)==6)) x=idealmulprime(nf,idmat(N),x);
  if((typ(x)!=19)||(lg(x)!=(N+1))||(lg((GEN)x[1])!=(N+1)))
    err(talker,"not an ideal in principalideal");
  R1=itos((GEN)((GEN)nf[2])[1]);R2=(N-R1)>>1;RU=R1+R2;
  pr=precision((GEN)matalphac[1]);if(!pr) pr=8;
  colmit=lg(mit)-1;colnew=lg(matalpha)-1;
  a=content(x);x=gdiv(x,a);x=hnfmod(x,detint(x));vinvperm=(long*)malloc(lg(vectbase)<<TWOPOTBYTES_IN_LONG);
  for(i=1;i<lg(vectbase);i++) vinvperm[itos((GEN)vperm[i])]=i;
  s=gun;for(i=1;i<=N;i++) s=mulii(s,gcoeff(x,i,i));
  xar=cgetg(RU+1,17);
  for(i=1;i<=RU;i++)
    {
      p2=cgetg(3,6);xar[i]=(long)p2;
      p3=cgetr(pr);p2[1]=(long)p3;
      p3=cgetr(pr);p2[2]=(long)p3;
      affsr(0,(GEN)p2[1]);affsr(0,(GEN)p2[2]);
    }
  fpc=factorisegensimple(nf,x,primfact,expoprimfact,vectbase);
  nbtest=0;ex=(long*)malloc(3*sizeof(long));exu=cgetg(RU+1,17);
  q=BITS_IN_LONG-1-(long)ceil(log((double)CBUCHG)/log(2.0));
  if(!fpc) 
    {
      p3=cgetg(3,17);p3[1]=(long)x;p3[2]=(long)xar;
      p1=ideallllred(nf,p3,gzero,pr);
      x=(GEN)p1[1];xar=cleancol((GEN)p1[2],N,RU,pr);
      fpc=factorisegensimple(nf,x,primfact,expoprimfact,vectbase);
      if(!fpc) 
	{
	  vdir=cgetg(RU+1,17);for(i=1;i<=RU;i++) vdir[i]=zero;
	  for(i=1;(i<=RU)&&(!fpc);i++)
	    {
	      vdir[i]=lstoi(10);if(i>1) vdir[i-1]=zero;
	      p1=ideallllred(nf,p3,vdir,pr);
	      x=(GEN)p1[1];xar=cleancol((GEN)p1[2],N,RU,pr);
	      fpc=factorisegensimple(nf,x,primfact,expoprimfact,vectbase);
	    }
	  while((!fpc)&&(nbtest<100))
	    {
	      for(i=1;i<=2;i++) ex[i]=mymyrand()>>q;
	      ideal=idealpowprime(nf,(GEN)vectbase[itos((GEN)vperm[1])],stoi(ex[1]),pr);
	      pgen1=idealpowprime(nf,(GEN)vectbase[itos((GEN)vperm[2])],stoi(ex[2]),pr);
	      ideal=idealmulh(nf,idealmul(nf,p3,ideal),pgen1);
	      for(bou=1;(bou<=RU)&&(!fpc);bou++)
		{
		  if(bou==1) 
		    {for(i=1;i<=RU;i++) exu[i]=lstoi((mymyrand()>>q)+1);}
		  else
		    {
		      for(i=1;i<=RU;i++) exu[i]=zero;
		      exu[bou]=lstoi(10);
		    }
		  p1=ideallllred(nf,ideal,exu,pr);nbtest++;
		  if(DEBUGLEVEL>2)
		    {
		      fprintf(errfile,"nbtest = %ld, ideal = ",nbtest);
		      output((GEN)p1[1]);
		    }
		  x=(GEN)p1[1];xar=cleancol((GEN)p1[2],N,RU,pr);
		  fpc=factorisegensimple(nf,x,primfact,expoprimfact,vectbase);
		}
	    }
	  if(nbtest>=100) 
	    {free(ex);err(talker,"too many iterations in isprincipal");}
	}
    }
  xalpha=cgetg(colmit+1,18);yalpha=cgetg(colnew+1,18);
  for(i=1;i<=colmit;i++) xalpha[i]=zero;
  for(i=1;i<=colnew;i++) yalpha[i]=zero;
  if(nbtest)
    {
      for(i=1;i<=2;i++) 
	{
	  if(i<=colmit) xalpha[i]=lstoi(-ex[i]);
	  else yalpha[i-colmit]=lstoi(-ex[i]);
	}
    }
  free(ex);
  for(i=1;i<=primfact[0];i++)
    {
      k=vinvperm[primfact[i]];
      if(k<=colmit) xalpha[k]=laddsg(expoprimfact[i],(GEN)xalpha[k]);
      else yalpha[k-colmit]=laddsg(expoprimfact[i],(GEN)yalpha[k-colmit]);
    }
  u1u2=smith2(mit);u1=(GEN)u1u2[1];u2=(GEN)u1u2[2];
  p3=(GEN)((GEN)RES[1])[2];
  p1=gmul(u1,gsub(xalpha,gmul(matalpha,yalpha)));
  c=lg(p3)-1;u1=ginv(u1);
  p4=cgetg(colmit+colnew+1,18);p2=cgetg(c+1,18);
  for(i=1;i<=c;i++) 
    {
      p4[i]=(long)dvmdii((GEN)p1[i],(GEN)p3[i],(GEN*)(p2+i));
      if(signe((GEN)p2[i])<0) 
	{
	  p2[i]=ladd((GEN)p2[i],(GEN)p3[i]);
	  p4[i]=(long)gaddgs((GEN)p4[i],-1);
	}
    }
  for(;i<=colmit;i++) p4[i]=p1[i];
  for(;i<=colmit+colnew;i++) p4[i]=yalpha[i-colmit];
  p5=cgetg(colmit+1,19);for(i=1;i<=colmit;i++) p5[i]=matalphac[i];
  p3=gmul(p5,u2);for(i=1;i<=colmit;i++) matalphac[i]=p3[i];
  settyp(xar,18);p1=gsub(gmul(matalphac,p4),xar);
  p4=cgetg(c+1,19);
  for(j=1;j<=c;j++)
    {
      p5=(GEN)idealpowprime(nf,(GEN)vectbase[itos((GEN)(vperm[1]))],pgen1=gcoeff(u1,1,j),pr);
      if(signe(pgen1)<0) p5[1]=(long)numer((GEN)p5[1]);
      for(i=2;i<=colmit;i++)
	{
	  pgen1=gcoeff(u1,i,j);ss=signe(pgen1);
	  if(ss)
	    {
	      if(ss>0)
		p5=idealmulh(nf,p5,(GEN)idealpowprime(nf,(GEN)vectbase[itos((GEN)(vperm[i]))],pgen1,pr));
	      else
		p5=idealmulh(nf,p5,numer((GEN)idealpowprime(nf,(GEN)vectbase[itos((GEN)(vperm[i]))],pgen1,pr)));
	      p5=ideallllred(nf,p5,gzero,pr);
	    }
	}
      if(!gegal((GEN)p5[1],(GEN)((GEN)((GEN)RES[1])[3])[j]))
	err(talker,"bug1 in isprincipal");
      p4[j]=lneg((GEN)p5[2]);settyp((GEN)p4[j],18);
    }
  p1=cleancol(c?gadd(p1,gmul(p4,p2)):p1,N,RU,pr);
  if(RU>1)
    {
      s2=gzero;
      p4=cgetg(RU+1,19);
      for(j=1;j<RU;j++)
	{
	  p5=cgetg(RU+1,18);p4[j]=(long)p5;
	  s1=gzero;
	  for(i=1;i<RU;i++) 
	    {
	      p5[i]=(long)greal(gcoeff(matunit,i,j));s1=gadd(s1,gmul((GEN)p5[i],(GEN)p5[i]));
	    }
	  p5[RU]=zero;if(gcmp(s1,s2)>0) s2=s1;
	}
      p5=cgetg(RU+1,18);p4[RU]=(long)p5;
      for(i=1;i<RU;i++) p5[i]=(long)greal((GEN)p1[i]);
      s2=gsqrt(gmul2n(s2,RU+1),pr);if(gcmpgs(s2,100000000)<0) s2=stoi(100000000);
      p5[RU]=(long)s2;
      p4=(GEN)lll(p4,pr)[RU];
      if(signe((GEN)p4[RU])<0) p4=gneg(p4);
      if(!gcmp1((GEN)p4[RU])) err(talker,"bug2 in isprincipal");
      setlg(p4,RU);
      p1=gadd(p1,gmul(matunit,p4));
      setlg(p4,RU+1);
    }
  s2=gun;
  for(j=1;j<=c;j++) 
    {
      p5=(GEN)((GEN)((GEN)RES[1])[3])[j];
      s1=gun;for(i=1;i<=N;i++) s1=mulii(s1,gcoeff(p5,i,i));
      if(signe((GEN)p2[j])) s2=mulii(s2,gpui(s1,(GEN)p2[j]));
    }
  s=gdivgs(glog(gdiv(s,s2),pr),N);
  p4=cgetg(N+1,18);
  for(i=1;i<=R1;i++) p4[i]=(long)gexp(gadd(s,(GEN)p1[i]),pr);
  for(i=R1+1;i<=RU;i++)
    {
      p4[i]=(long)gexp(gadd(s,gmul2n((GEN)p1[i],-1)),pr);;
      p4[i+R2]=lconj((GEN)p4[i]);
    }
  om1=(GEN)((GEN)nf[5])[1];
  om2=cgetg(N+1,19);
  for(j=1;j<=N;j++)
    {
      om3=cgetg(N+1,18);om2[j]=(long)om3;
      for(i=1;i<=R1;i++) om3[i]=(long)coeff(om1,i,j);
      for(i=R1+1;i<=RU;i++)
	{
	  om3[i]=(long)coeff(om1,i,j);om3[i+R2]=lconj((GEN)om3[i]);
	}
    }
  p1=gdiv(grndtoi(gmul(s2,greal(gauss(om2,p4))),&e),s2);
  if(e>=0)
    {
      fprintf(errfile,"\n  ***   Warning: insufficient precision for generators, not given\n");
      p1=cgetg(1,18);
    }
  tetpil=avma;y=cgetg(4,17);y[1]=lcopy(p2);y[2]=lmul(a,p1);y[3]=lstoi(-e);
  return gerepile(av,tetpil,y);
}

GEN isunit(GEN bignf, GEN x)
{
  long av=avma,tetpil,tx,i,R1,R2,RU,nru;
  GEN RES,matunit,y,p1,p2,p3,nf,ro1;

  if((typ(bignf)!=17)||(lg(bignf)!=9))
    err(talker,"not a big number field vector in isunit");
  tx=typ(x);
  if((tx==4)||(tx==5)) return cgetg(1,17);
  matunit=(GEN)bignf[3];RU=lg(matunit);
  RES=(GEN)bignf[8];nf=(GEN)bignf[7];
  ro1=(GEN)RES[4];nru=itos((GEN)ro1[1]);
  if(tx==1)
    {
      if(!gcmp1(absi(x))) return cgetg(1,17);
      y=cgetg(RU+1,17);
      for(i=1;i<RU;i++) y[i]=zero;
      y[RU]=(signe(x)>0)?lmodulcp(gzero,(GEN)ro1[1]):lmodulcp(stoi(nru>>1),(GEN)ro1[1]);
      return y;
    }
  if(tx!=10) 
    {
      if(tx!=9) err(talker,"not an algebraic number in isunit");
      if(!gegal((GEN)nf[1],(GEN)x[1])) err(talker,"not the same number field in isunit");
    }
  p1=(GEN)nf[2];R1=itos((GEN)p1[1]);R2=itos((GEN)p1[2]);
  p1=cgetg(RU+1,18);for(i=1;i<=R1;i++) p1[i]=un;for(;i<=RU;i++) p1[i]=deux;
  p3=(GEN)principalidele(nf,x)[2];
  p1=concat(matunit,p1);p2=ground(gauss(greal(p1),greal(p3)));
  if(!gcmp0((GEN)p2[RU])) err(talker,"insufficient precision (1) in isunit");
  p1=gsub(p3,gmul(p1,p2));
  p3=(GEN)principalidele(nf,(GEN)ro1[2])[2];
  p1=gdiv((GEN)p1[1],(GEN)p3[1]);
  p1=gmod(ground(greal(p1)),(GEN)ro1[1]);
  tetpil=avma;y=cgetg(RU+1,17);for(i=1;i<RU;i++) y[i]=lcopy((GEN)p2[i]);
  y[RU]=lmodulcp(p1,(GEN)ro1[1]);
  return gerepile(av,tetpil,y);
}

GEN signunit(GEN bignf)
{
  long av,i,j,r,R1,R2,RU;
  GEN matunit,y,p1,p2,nf;

  if((typ(bignf)!=17)||(lg(bignf)!=9))
    err(talker,"not a big number field vector in isunit");
  matunit=(GEN)bignf[3];RU=lg(matunit);nf=(GEN)bignf[7];
  p1=(GEN)nf[2];R1=itos((GEN)p1[1]);constpi(5);
  y=cgetg(RU,19);
  for(j=1;j<RU;j++)
    {
      p1=cgetg(R1+1,18);y[j]=(long)p1;
      for(i=1;i<=R1;i++)
	{
	  av=avma;p2=ground(gdiv(gimag(gcoeff(matunit,i,j)),gpi));
	  r=signe(p2)?p2[lgef(p2)-1]&1:0;avma=av;
	  p1[i]=r?lneg(gun):un;
	}
    }
  return y;
}

	  
