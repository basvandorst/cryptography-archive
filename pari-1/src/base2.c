/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*                                                                 */
/*                       BASE D'ENTIERS                            */
/*                                                                 */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

# include "genpari.h"

#define TRUE 1
#define FALSE 0

FILE *temp;
GEN pradical(GEN nf,GEN p),pol_min(GEN alpha,GEN nf,GEN p,GEN algebre,GEN algebre1);
GEN eval_pol(GEN nf,GEN pol,GEN alpha,GEN p,GEN algebre,GEN algebre1);
GEN lens(GEN nf,GEN p,GEN a),two_elt(GEN nf,GEN p,GEN ideal);
GEN two_elt(GEN nf, GEN p, GEN ideal),fasthnf(GEN x, GEN detmat),differente(GEN nf);
GEN element_mulid(GEN nf, GEN x, long i);
long idealvalint(GEN nf, GEN x, GEN vp);

GEN maxord(GEN p,GEN f,long mf);
GEN nilord(GEN p,GEN fx,long mf,GEN gx);
GEN Decomp(GEN p,GEN f,long mf,GEN theta,GEN chi,GEN nu);
GEN respm(GEN f1,GEN f2,GEN pm);
GEN hnfpm(GEN a,GEN pm);
GEN nbasis(GEN ibas,GEN pd);
GEN pbasis(GEN p,GEN f,long mf,GEN alpha);
GEN eltppm(GEN f,GEN pd,GEN theta,GEN k);
GEN testb(GEN p,GEN fa,long Da,GEN theta,long Dt);
GEN testc(GEN p, GEN fa, long c, GEN alph2, long Ma, GEN thet2, long Mt);
GEN testd(GEN p,GEN fa,long c,long Da,GEN alph2,long Ma,GEN theta);
GEN csrch(GEN p,GEN fa,GEN gamma);
GEN dedek(GEN f,GEN p,GEN g);
long cbezout(long a,long b,long *u,long *v);
long clcm(long a,long b);


/*******************************************************************

Operations sur les elements et ideaux de corps de nombres.
Un element sera represente par un vecteur colonne dans la base d'entiers nf[7].
Un ideal premier est represente par [p,a,e,f,b] ou l'ideal est p.Z_K+a.Z_K, ou
a est un element de Z_K, e l'indice de ramification, f le degre residuel et b
l'element de Z_K "constante de Lenstra".
Un ideal sera represente par un couple [M,V] ou M est une HNF de l'ideal dans
la base d'entiers, et V un vecteur ligne a r1+r2 composantes complexes
representant la "partie archimedienne" de l'ideal (considere alors comme
idele). Par exemple, si l'ideal est principal
engendre par a, V contiendra le vecteur des log complexes des r1+r2 premiers
conjugues de a (ceci depend bien sur de a et pas seulement de l'ideal).
Les programmes marchent aussi si seulement M est fourni. 

********************************************************************/

GEN rootsof1(GEN nf)
{
  long av=avma,tetpil,N,ld,fl,k,i,j;
  GEN algun,p1,y,R1,d,list,w;

  if((typ(nf)!=17)||(lg(nf)<10)) err(rootofer1);
  N=lgef((GEN)nf[1])-3;R1=(GEN)((GEN)nf[2])[1];
  algun=(GEN)((GEN)nf[8])[1];
  if(signe(R1)) 
    {
      avma=av;y=cgetg(3,17);y[1]=deux;y[2]=lneg(algun);
      return y;
    }
  y=minim((GEN)((GEN)nf[5])[3],N,1000);
  if(itos((GEN)y[2])!=N) err(rootofer2);
  w=(GEN)y[1];
  if(!cmpii(w,gdeux)) 
    {
      avma=av;y=cgetg(3,17);y[1]=deux;y[2]=lneg(algun);
      return y;
    }
  d=divisors(w);ld=lg(d)-1;
  list=(GEN)y[3];k=lg(list);fl=1;
  for(i=1;(i<k)&&fl;i++)
    {
      p1=(GEN)list[i];j=1;
      while((j<ld-1)&&(!gegal(element_pow(nf,p1,(GEN)d[j]),algun))) j++;
      if(j<ld-1) p1=gneg(p1);j=1;
      while((j<ld-1)&&(!gegal(element_pow(nf,p1,(GEN)d[j]),algun))) j++;
      fl=(j<ld-1);
    }
  if(fl) err(rootsof1er);
  tetpil=avma;y=cgetg(3,17);y[1]=lcopy(w);y[2]=lcopy(p1);
  return gerepile(av,tetpil,y);
}

GEN primedec(GEN nf, GEN p)
              
/* Recoit un corps de nombres nf et un premier p,ressort une liste 
 des ideaux premiers au dessus de p dans le format vu plus haut, dans
l'ordre croissant des degres residuels */

{
  long av=avma,tetpil,i,j,k,v,kbar,l3,np,c,i1,i2,indice,N;
  GEN f,list,list2,ip,elementh,hensemble;
  GEN algebre,algebre1,b,b2,mat1,mat2;
  GEN alpha,beta1,p1,p2,p3,p4,unmodp,zmodp,vecteur,pol;
  GEN T=(GEN)nf[1],pidmat,ppuin;

  if(DEBUGLEVEL>=3) timer2();
  if((typ(nf)!=17)||(lg(nf)<10)) err(idealer1);
  N=lgef(T)-3;
  if(signe(modii((GEN)nf[4],p)))
    {
      f=centerlift(factmod(T,p));np=lg((GEN)f[1]);
      if(DEBUGLEVEL>=3) {fprintf(errfile,"temps factmod: ");fprintf(errfile,"%ld\n",timer2());fflush(errfile);}
      list=cgetg(np,17);
      for(i=1;i<np;i++)
	{
	  p1=(GEN)(list[i]=lgetg(6,17));
	  p1[1]=(long)p;p3=gcoeff(f,i,1);l3=lgef(p3)-1;
	  p2=cgetg(N+1,18);
	  if(l3==(N+2)) 
	    {
	      p1[2]=(long)p2;p1[3]=un;p1[4]=lstoi(N);
	      p3=cgetg(N+1,18);p1[5]=(long)p3;p3[1]=un;
	      p2[1]=(long)p;for(j=2;j<=N;j++) p3[j]=p2[j]=zero;
	    }
	  else
	    {
	      v=ggval(subres(p3,T),p)/(l3-2);if(v>1) p3[2]=ladd((GEN)p3[2],p);
	      for(j=1;j<l3;j++) p2[j]=p3[j+1];
	      for(j=l3;j<=N;j++) p2[j]=zero;
	      p1[2]=lmul((GEN)nf[8],p2);
	      p1[3]=(long)coeff(f,i,2);
	      p1[4]=lstoi(l3-2);p1[5]=(long)lens(nf,p,(GEN)p1[2]);
	    }
	}
      if(DEBUGLEVEL>=3) {fprintf(errfile,"temps lens etc...: ");fprintf(errfile,"%ld\n",timer2());fflush(errfile);}      
      p1=stoi(4);tetpil=avma;return gerepile(av,tetpil,vecsort(list,p1));
    }
  else 
    {
      list=cgetg(N+1,17);for(i=1;i<=N;i++) list[i]=lgetg(6,17);
      ip=pradical(nf,p);
      if(DEBUGLEVEL>=3) {fprintf(errfile,"temps pradical: ");fprintf(errfile,"%ld\n",timer2());fflush(errfile);}
      unmodp=gmodulcp(gun,p);zmodp=gmodulcp(gzero,p);
      pidmat=cgetg(N+1,18);vecteur=cgetg(N+1,18);
      for(i1=2;i1<=N;i1++) {pidmat[i1]=zero;vecteur[i1]=(long)zmodp;}
      pidmat[1]=(long)p;vecteur[1]=(long)unmodp;
      ppuin=gpuigs(p,N);c=0;indice=0;
      hensemble=cgetg(N+1,17); 
      if(lg(ip)<N+1) {c=1;hensemble[1]=(long)ip;}
      while(c)
        {  
           elementh=(GEN)(hensemble[c]);k=lg(elementh)-1;kbar=N-k;
           algebre=gmul(unmodp,suppl(concat(elementh,vecteur)));
           algebre1=cgetg(kbar+1,19);
           for(i1=1;i1<=kbar;i1++) algebre1[i1]=algebre[i1+k];
           b=cgetg(kbar+1,19);
           for(i1=1;i1<=kbar;i1++)
             b[i1]=lsub(element_pow(nf,(GEN)algebre1[i1],p),(GEN)algebre1[i1]);
           b2=inverseimage(algebre,b);
           mat1=cgetg(kbar+1,19);
           for(i1=1;i1<=kbar;i1++) mat1[i1]=lgetg(kbar+1,18);
           for(i1=1;i1<=kbar;i1++)
             for(i2=1;i2<=kbar;i2++)
               coeff(mat1,i2,i1)=coeff(b2,k+i2,i1);
	   mat2=cgetg(k+N+1,19);
	   for(i1=1;i1<=k;i1++) mat2[i1]=elementh[i1];
           mat1=gmul(unmodp,ker(mat1));
           if(lg(mat1)>2)
             {
                alpha=gmul(algebre1,(GEN)mat1[2]);
                pol=pol_min(alpha,nf,p,algebre,algebre1);
                setvarn(pol,0);
		p1=(GEN)factmod(pol,p)[1];
		for(i=1;i<lg(p1);i++)
		  {
		    beta1=eval_pol(nf,(GEN)p1[i],alpha,p,algebre,algebre1);
		    for(i1=1;i1<=N;i1++)
		      mat2[k+i1]=(long)element_mulid(nf,beta1,i1);
		    hensemble[c]=(long)(image(mat2));c++;
		  }
		c--;
		if(DEBUGLEVEL>=3) {fprintf(errfile,"temps hensemble[%ld]: ",c);fprintf(errfile,"%ld\n",timer2());fflush(errfile);}
             }
           else
             {
                indice++;p1=(GEN)list[indice];
                p1[1]=(long)p;p1[4]=lstoi(kbar);
                p1[2]=(long)two_elt(nf,p,elementh);
                p1[5]=(long)lens(nf,p,(GEN)p1[2]);
		p1[3]=lstoi(elementval2(nf,pidmat,ppuin,p1));
                c--;
		if(DEBUGLEVEL>=3) {fprintf(errfile,"temps hensemble[%ld]: ",c);fprintf(errfile,"%ld\n",timer2());fflush(errfile);}
	     }
        }
      list2=cgetg(indice+1,17);
      for(i=1;i<=indice;i++) list2[i]=list[i];
      p1=stoi(4);tetpil=avma;
      return gerepile(av,tetpil,vecsort(list2,p1));
    }
}

GEN element_mul(GEN nf, GEN x, GEN y)
                
/* Recoit deux vecteurs de longueur N representant x et y dans la base 
 d'entiers (peut etre modulo p) et ressort leur produit sous forme d'un
 vecteur */

{
  long av=avma,tetpil,i,j,k,N=lgef((GEN)nf[1])-3;
  GEN s,v,c;

  v=cgetg(N+1,18);
  for(k=1;k<=N;k++)
    {
      s=gzero;
      for(i=1;i<=N;i++)
	{
	  c=(GEN)coeff((GEN)nf[9],k,(i-1)*N+i);
	  if(signe(c))
	    {
	      if(gcmp1(c)) s=gadd(s,gmul((GEN)x[i],(GEN)y[i]));
	      else s=gadd(s,gmul(gmul((GEN)x[i],(GEN)y[i]),c));
	    }
	  for(j=i+1;j<=N;j++)
	    s=gadd(s,gmul(gadd(gmul((GEN)x[i],(GEN)y[j]),gmul((GEN)x[j],(GEN)y[i])),gcoeff((GEN)nf[9],k,(i-1)*N+j)));
	}
      v[k]=(long)s;
    }
  tetpil=avma;return gerepile(av,tetpil,gcopy(v));
}

GEN element_muli(GEN nf, GEN x, GEN y)
                
/* Recoit deux vecteurs de longueur N representant x et y dans la base 
 d'entiers (uniquement coeff entiers) et ressort leur produit sous forme d'un
 vecteur */

{
  long av=avma,tetpil,i,j,k,N=lgef((GEN)nf[1])-3;
  GEN s,v,c;

  v=cgetg(N+1,18);
  for(k=1;k<=N;k++)
    {
      s=gzero;
      for(i=1;i<=N;i++)
	{
	  c=(GEN)coeff((GEN)nf[9],k,(i-1)*N+i);
	  if(signe(c))
	    {
	      if(gcmp1(c)) s=addii(s,mulii((GEN)x[i],(GEN)y[i]));
	      else s=addii(s,mulii(mulii((GEN)x[i],(GEN)y[i]),c));
	    }
	  for(j=i+1;j<=N;j++)
	    {
	      c=(GEN)coeff((GEN)nf[9],k,(i-1)*N+j);
	      if(signe(c))
		{
		  if(gcmp1(c)) s=addii(s,addii(mulii((GEN)x[i],(GEN)y[j]),mulii((GEN)x[j],(GEN)y[i])));
		  else s=addii(s,mulii(addii(mulii((GEN)x[i],(GEN)y[j]),mulii((GEN)x[j],(GEN)y[i])),c));
		}
	    }
	}
      v[k]=(long)s;
    }
  tetpil=avma;return gerepile(av,tetpil,gcopy(v));
}

GEN element_mulh(GEN nf, long limi, long limj, GEN x, GEN y)

/* Recoit deux vecteurs de longueur N representant x et y dans la base 
 d'entiers (uniquement coeff entiers) et ressort leur produit sous forme d'un
 vecteur */

{
  long av=avma,tetpil,i,j,k,N=lgef((GEN)nf[1])-3;
  GEN s,v,c;

  if(limi<limj) {i=limi;limi=limj;limj=i;s=x;x=y;y=s;}
  v=cgetg(N+1,18);
  for(k=1;k<=N;k++)
    {
      s=gzero;
      for(i=1;i<=limj;i++)
	{
	  c=(GEN)coeff((GEN)nf[9],k,(i-1)*N+i);
	  if(signe(c))
	    {
	      if(gcmp1(c)) s=addii(s,mulii((GEN)x[i],(GEN)y[i]));
	      else s=addii(s,mulii(mulii((GEN)x[i],(GEN)y[i]),c));
	    }
	  for(j=i+1;j<=limj;j++)
	    {
	      c=(GEN)coeff((GEN)nf[9],k,(i-1)*N+j);
	      if(signe(c))
		{
		  if(gcmp1(c)) s=addii(s,addii(mulii((GEN)x[i],(GEN)y[j]),mulii((GEN)x[j],(GEN)y[i])));
		  else s=addii(s,mulii(addii(mulii((GEN)x[i],(GEN)y[j]),mulii((GEN)x[j],(GEN)y[i])),c));
		}
	    }
	}
      for(i=limj+1;i<=limi;i++)
	{
	  for(j=1;j<=limj;j++)
	    {
	      c=(GEN)coeff((GEN)nf[9],k,(i-1)*N+j);
	      if(signe(c))
		{
		  if(gcmp1(c)) s=addii(s,mulii((GEN)x[i],(GEN)y[j]));
		  else s=addii(s,mulii(mulii((GEN)x[i],(GEN)y[j]),c));
		}
	    }
	}
      v[k]=(long)s;
    }
  tetpil=avma;return gerepile(av,tetpil,gcopy(v));
}

GEN element_sqr(GEN nf, GEN x)
              
/* Recoit un vecteurs de longueur N representant x dans la base 
 d'entiers et ressort son carre sous forme d'un
 vecteur */

{
  long av=avma,tetpil,i,j,k,N=lgef((GEN)nf[1])-3;
  GEN s,v,c;

  v=cgetg(N+1,18);
  for(k=1;k<=N;k++)
    {
      s=gzero;
      for(i=1;i<=N;i++)
	{
	  c=(GEN)coeff((GEN)nf[9],k,(i-1)*N+i);
	  if(signe(c))
	    {
	      if(gcmp1(c)) s=gadd(s,gmul((GEN)x[i],(GEN)x[i]));
	      else s=gadd(s,gmul(gmul((GEN)x[i],(GEN)x[i]),c));
	    }
	  for(j=i+1;j<=N;j++)
	    {
	      c=(GEN)coeff((GEN)nf[9],k,(i-1)*N+j);
	      if(signe(c))
		{
		  if(gcmp1(c)) s=gadd(s,gmul2n(gmul((GEN)x[i],(GEN)x[j]),1));
		  else s=gadd(s,gmul(gmul((GEN)x[i],(GEN)x[j]),shifti(c,1)));
		}
	    }
	}
      v[k]=(long)s;
    }
  tetpil=avma;return gerepile(av,tetpil,gcopy(v));
}

GEN element_pow(GEN nf, GEN x, GEN k)
                
/* Calcule x^k dans le corps de nombres nf */

{
  long i,f,av=avma,tetpil,N=lgef((GEN)nf[1])-3;
  GEN k1,y,z;

  k1=k;z=x;f=1;y=cgetg(N+1,18);
  for(i=2;i<=N;i++) y[i]=zero;
  y[1]=un;
  while(f)
    {
      if(mpodd(k1)) y=element_mul(nf,z,y);
      k1=shifti(k1,-1);f=signe(k1);
      if(f) z=element_sqr(nf,z);
    }
  tetpil=avma;return gerepile(av,tetpil,gcopy(y));
}

GEN element_mulid(GEN nf, GEN x, long i)
                
/* Recoit un vecteur x de longueur N representant x dans la base d'entiers
(peut etre modulo p) et ressort x.w_i sous forme d'un
 vecteur */

{
  long av=avma,tetpil,j,k,N=lgef((GEN)nf[1])-3;
  GEN s,v,c,p1;

  v=cgetg(N+1,18);
  for(k=1;k<=N;k++)
    {
      s=gzero;
      for(j=1;j<=N;j++)
	{
	  c=gcoeff((GEN)nf[9],k,(i-1)*N+j);p1=(GEN)x[j];
	  if((signe(c))&&(!gcmp0(p1)))
	    s=gcmp1(c)?gadd(s,p1):gadd(s,gmul(p1,c));
	}
      v[k]=(long)s;
    }
  tetpil=avma;return gerepile(av,tetpil,gcopy(v));
}

GEN pradical(GEN nf, GEN p)
              
/* Calcule une F_p base du p-radical de Z_K,i.e une
 matrice N fois r (r=dimension sur F_p du p-radical) */

{
  long av=avma,tetpil,j,k,N=lgef((GEN)nf[1])-3;
  GEN p1,unmodp,zmodp,x,m;

  p1=p;while(cmpis(p1,N)<0) p1=mulii(p1,p);
  m=cgetg(N+1,19);
  unmodp=gmodulcp(gun,p);zmodp=gmul(gzero,unmodp);
  for(k=1;k<=N;k++)
    {
      x=cgetg(N+1,18);
      for(j=1;j<=N;j++) x[j]=(long)zmodp;
      x[k]=(long)unmodp;
      m[k]=(long)element_pow(nf,x,p1);
    }
  tetpil=avma;return gerepile(av,tetpil,gmul(unmodp,ker(m)));
}

      
GEN pol_min(GEN alpha, GEN nf, GEN p, GEN algebre, GEN algebre1)
                                     
/* Calcule le polynome minimal de alpha dans algebre (polynome a
 coefficients dans F_p) */

{
   long av=avma,tetpil,i,j,N=lgef((GEN)nf[1])-3,k=N-lg(algebre1)+1;
   GEN puiss,puiss2,noyau,unmodp,zmodp,vecteur;

   unmodp=gmodulcp(gun,p);zmodp=gmul(unmodp,gzero);
   vecteur=gmul(unmodp,(GEN)(idmat(N)[k+1]));
   puiss=cgetg(N+2,19);puiss[1]=(long)vecteur;
   for(i=1;i<=N;i++)
     puiss[i+1]=(long)(inverseimage(algebre,element_pow(nf,alpha,stoi(i))));
   puiss2=cgetg(N+2,19);
   for(i=1;i<=N+1;i++)
     {
       puiss2[i]=lgetg(N-k+1,18);
       for(j=1;j<=N-k;j++)
         coeff(puiss2,j,i)=coeff(puiss,k+j,i);
     }
   noyau=gmul(ker(puiss2),unmodp);
   tetpil=avma;return gerepile(av,tetpil,gtopolyrev((GEN)noyau[1],0));
}

GEN eval_pol(GEN nf, GEN pol, GEN alpha, GEN p, GEN algebre, GEN algebre1)
                                         
/* Evalue le polynome pol en alpha,element de nf */

{
   long av=avma,tetpil,i;
   long N=lgef((GEN)nf[1])-3,k=N-lg(algebre1)+1,lx=lgef(pol)-3;
   GEN res,valeur,unmodp,zmodp;

   res=cgetg(N+1,18);
   unmodp=gmodulcp(gun,p);zmodp=gmul(unmodp,gzero);
   for(i=2;i<=N;i++) res[i]=(long)zmodp;
/*
   res[1]=(long)unmodp;
   res=gmul(res,(GEN)pol[2]);
   for(i=1;i<=lgef(pol)-3;i++)
     res=gadd(res,gmul((GEN)pol[i+2],element_pow(nf,alpha,stoi(i))));
  la methode de Horner est beaucoup plus efficace!!!!:
*/
   res[1]=lmul(unmodp,(GEN)pol[lx+2]);
   for(i=lx+1;i>=2;i--)
     {
       res=element_mul(nf,alpha,res);res[1]=ladd((GEN)res[1],(GEN)pol[i]);
     }
   res=inverseimage(algebre,res);
   valeur=cgetg(N-k+1,18);
   for(i=1;i<=N-k;i++) valeur[i]=res[k+i];
   tetpil=avma;return gerepile(av,tetpil,gmul(algebre1,valeur));
}

GEN kerlens2(GEN x, GEN pgen)
{
  long i,j,k,t,nbc,nbl,av,av1;
  GEN a,c,l,d,y,q;

  av=avma;
  a=gmul(x,gmodulcp(gun,pgen));
  nbl=nbc=lg(x)-1;
  c=cgetg(nbl+1,17);l=cgetg(nbc+1,17);
  d=cgetg(nbc+1,17);
  for(i=1;i<=nbl;i++) c[i]=0;
  k=1;t=1;
  while((t<=nbl)&&(k<=nbc))
    {
      for(j=1;j<k;j++)
	for(i=1;i<=nbl;i++)
	  if(i!=l[j])
	    coeff(a,i,k)=lsub(gmul((GEN)d[j],gcoeff(a,i,k)),gmul(gcoeff(a,l[j],k),gcoeff(a,i,j)));
      t=1;while((t<=nbl)&&((c[t])||gcmp0(gcoeff(a,t,k)))) t++;
      if (t<=nbl) {d[k]=coeff(a,t,k);c[t]=k;l[k++]=t;}
    }
  if(k>nbc) err(kerlenser);
  y=cgetg(nbc+1,18);
  y[1]=(k>1)?(long)coeff(a,l[1],k):un;
  for(q=gun,j=2;j<k;j++)
    {
      q=gmul(q,(GEN)d[j-1]);
      y[j]=lmul(gcoeff(a,l[j],k),q);
    }
  if(k>1) y[k]=lneg(gmul(q,(GEN)d[k-1]));
  for(j=k+1;j<=nbc;j++) y[j]=zero;
  av1=avma;
  return gerepile(av,av1,gcopy(lift(y))); 
}

GEN kerlens(GEN x, GEN pgen)
{
  long i,j,k,t,nbc,nbl,p,q,*c,*l,*d,**a;
  GEN y;

  if(cmpis(pgen,(MAXHALFULONG>>1))>0)
    return kerlens2(x,pgen);
/* ici p<=(MAXHALFULONG>>1) ==> simple precision (long de C) */
  p=itos(pgen);
  nbl=nbc=lg(x)-1;
  a=(long**)malloc((nbc+1)*sizeof(long*));
  for(j=1;j<=nbc;j++)
    {
      c=a[j]=(long*)malloc((nbl+1)<<TWOPOTBYTES_IN_LONG);
      for(i=1;i<=nbl;i++) c[i]=itos(modis(gcoeff(x,i,j),p));
    }
  c=(long*)malloc((nbl+1)<<TWOPOTBYTES_IN_LONG);
  l=(long*)malloc((nbc+1)<<TWOPOTBYTES_IN_LONG);
  d=(long*)malloc((nbc+1)<<TWOPOTBYTES_IN_LONG);
  for(i=1;i<=nbl;i++) c[i]=0;
  k=1;t=1;
  while((t<=nbl)&&(k<=nbc))
    {
      for(j=1;j<k;j++)
	for(i=1;i<=nbl;i++)
	  if(i!=l[j]) a[k][i]=(d[j]*a[k][i]-a[j][i]*a[k][l[j]]) % p;
      t=1;while((t<=nbl)&&((c[t])||(!a[k][t]))) t++;
      if (t<=nbl) {d[k]=a[k][t];c[t]=k;l[k++]=t;}
    }
  if(k>nbc) err(kerlenser);
  y=cgetg(nbc+1,18);
  t=(k>1) ? a[k][l[1]]:1;
  y[1]=(t>0)? lstoi(t):lstoi(t+p);
  for(q=1,j=2;j<k;j++)
    {
      q=(q*d[j-1])%p;
      t=(a[k][l[j]]*q)%p;
      y[j]=(t>0)? lstoi(t):lstoi(t+p);
    }
  if(k>1)
    {
      t=(q*d[k-1])%p;
      y[k]=(t>0)? lstoi(p-t):lstoi(-t);
    }
  for(j=k+1;j<=nbc;j++) y[j]=zero;
  free(c);free(l);free(d);
  for(j=1;j<=nbc;j++) free(a[j]);free(a);
  return y;
}

  
GEN lens(GEN nf, GEN p, GEN a)
                
/* Calcule la constante de lenstra de l'ideal p.Z_K+a.Z_K ou a est un
vecteur sur la base d'entiers */

{
   long av=avma,tetpil,N=lgef((GEN)nf[1])-3,j;
   GEN mat;

   mat=cgetg(N+1,19);for(j=1;j<=N;j++) mat[j]=(long)element_mulid(nf,a,j);
   tetpil=avma;return gerepile(av,tetpil,kerlens(mat,p));
}

GEN two_elt(GEN nf, GEN p, GEN ideal)
                    
/* Recoit un ideal (mod p) et calcule une representation a deux
 elements (ideal non egal a Z_K) */

{
   long av=avma,tetpil,N=lgef((GEN)nf[1])-3,m,r,i,j,k;
   GEN beta,alpha,lambda,norme,pf;
      
   k=lg(ideal)-1;pf=gpuigs(p,N-k);r=1;
   alpha=cgetg(N+1,18);
   beta=concat(gscalmat(p,N),lift(ideal));
   m=lg(beta)-1;lambda=cgeti(m+1);
   for(i=1;i<=m;i++) lambda[i]=r;
   do
     {
        for(i=1;i<=N;i++) alpha[i]=zero;
        for(i=1;i<=m;i++) alpha=gadd(alpha,gmulsg(lambda[i],(GEN)beta[i]));
        norme=gnorm(gmodulcp(gmul((GEN)nf[7],alpha),(GEN)nf[1]));
        if(gcmp0(gmod(gdiv(norme,pf),p)))
          {
             for(j=m;(lambda[j]+r)==0;j--);
             lambda[j]--;
             for(i=j+1;i<=m;i++) lambda[i]=r;
             for(j=1;(j<m)&&(!lambda[j]);j++);
             if(!lambda[j]) {r++;for(i=1;i<=m;i++) lambda[i]=r;}
	  }
     }
   while(gcmp0(gmod(gdiv(norme,pf),p)));
   tetpil=avma;return gerepile(av,tetpil,gcopy(alpha));
}

#define isideletype(x) ((typ(x)==17)&&(lg(x)==3))
#define principalid(nf,x,y) (((typ(y)==17)&&(lg(y)==3))?principalidele(nf,x):principalideal(nf,x))

GEN idealmul(GEN nf, GEN ix, GEN iy)
                  
/* recoit deux ideaux ix et iy comme ci-dessus avec ou sans leur composante
archimedienne et ressort leur produit sans le reduire */

{
  long av=avma,f,tetpil,rx,ry,i,j,tx=typ(ix),ty=typ(iy);
  GEN m,x,y,dp,dx,dy,dz,y1;

  if(tx<=9) {ix=principalid(nf,ix,iy);tx=typ(ix);}
  if(ty<=9) {iy=principalid(nf,iy,ix);ty=typ(iy);}
  if((tx==17)&&(lg(ix)==6)) 
    {
      if((ty==17)&&(lg(iy)==6))
	{
	  x=idealmulprime(nf,idmat(lgef((GEN)nf[1])-3),ix);
	  tetpil=avma;
	  return gerepile(av,tetpil,idealmulprime(nf,x,iy));
	}
      else return idealmulprime(nf,iy,ix);
    }
  if((ty==17)&&(lg(iy)==6)) return idealmulprime(nf,ix,iy);
  if((typ(nf)!=17)||(lg(nf)<10)) err(idealer1);
  f=1;
  if(tx==17) {if(lg(ix)==3) x=(GEN)ix[1];else err(idealer2);}
  else {f=0;x=ix;}
  if(ty==17) {if(lg(iy)==3) y=(GEN)iy[1];else err(idealer2);}
  else {f=0;y=iy;}
  if((typ(x)!=19)||(typ(y)!=19)) err(idealer2);
  rx=lg(x)-1;ry=lg(y)-1;dx=denom(x);dy=denom(y);x=gmul(dx,x);y=gmul(dy,y);
  dz=mulii(dx,dy);
  m=cgetg(rx*ry+1,19);
  for(i=1;i<=rx;i++)
    for(j=1;j<=ry;j++)
      m[(i-1)*ry+j]=(long)element_muli(nf,(GEN)x[i],(GEN)y[j]);
  if((rx==lg((GEN)x[1])-1)&&(ry==(lg((GEN)y[1])-1)))
    {
      dp=mulii(det(x),det(y));tetpil=avma;y=hnfmod(m,dp);
    }
  else {dp=detint(m);tetpil=avma;y=gcmp0(dp)?hnf(m):hnfmod(m,dp);}
  if(!gcmp1(dz)) {tetpil=avma;y=gdiv(y,dz);}
  if(f) 
    {
      y1=cgetg(3,17);y1[1]=(long)y;
      y1[2]=ladd((GEN)ix[2],(GEN)iy[2]);
      return gerepile(av,tetpil,y1);
    }
  else return gerepile(av,tetpil,y);
}

GEN idealmulred(GEN nf, GEN ix, GEN iy, long prec)
{
  long av=avma,tetpil;
  GEN p1;

  p1=idealmul(nf,ix,iy);tetpil=avma;
  return gerepile(av,tetpil,ideallllred(nf,p1,gzero,prec));
}

GEN idealmulh(GEN nf, GEN ix, GEN iy)
                  
/* recoit deux ideaux ix et iy comme ci-dessus avec ou sans leur composante
archimedienne et ressort leur produit sans le reduire. On suppose les ideaux
sous forme HNF et de meme taille. A usage interne donc aucune verification. */

{
  long av=avma,f,tetpil,N,i,j;
  GEN m,x,y,dx,dy,dz;
  
  f=1;
  if(typ(ix)==17) x=(GEN)ix[1];else {f=0;x=ix;}
  if(typ(iy)==17) y=(GEN)iy[1];else {f=0;y=iy;}
  N=lg(x)-1;m=cgetg(N*N+1,19);dx=gcoeff(x,1,1);dy=gcoeff(y,1,1);
  for(i=2;i<=N;i++) {dx=mulii(dx,gcoeff(x,i,i));dy=mulii(dy,gcoeff(y,i,i));}
  dz=mulii(dx,dy);
  for(i=1;i<=N;i++)
    for(j=1;j<=N;j++)
      m[(N-i)*N+j]=(long)element_mulh(nf,i,j,(GEN)x[i],(GEN)y[j]);
  tetpil=avma;
  if(f) {y=cgetg(3,17);y[1]=(long)fasthnf(m,dz);y[2]=ladd((GEN)ix[2],(GEN)iy[2]);}
  else y=fasthnf(m,dz);
  return gerepile(av,tetpil,y);
}

GEN idealmulprime(GEN nf, GEN ix, GEN vp)
                  
/* recoit un ideal ix et un ideal premier vp en format
primedec et ressort leur produit */

{
  long av=avma,tetpil,i,f,N;
  GEN m,x,y,dx,denx,p1;

  if((typ(nf)!=17)||(lg(nf)<10)) err(idealer1);
  N=lgef((GEN)nf[1])-3;
  if((typ(vp)!=17)||(lg(vp)!=6)) err(idealer3);
  if(typ(ix)<=9) ix=principalideal(nf,ix);
  if((typ(ix)==17)&&(lg(ix)==3)) {f=1;x=(GEN)ix[1];} else {f=0;x=ix;}
  if(typ(x)!=19) err(idealer2);
  if(N!=(lg((GEN)x[1])-1)) err(idealer4);
  denx=denom(x);if(!gcmp1(denx)) x=gmul(denx,x);
  if(lg(x)!=(N+1)) x=idealmul(nf,x,idmat(N));
  dx=gpui((GEN)vp[1],(GEN)vp[4]);for(i=1;i<=N;i++) dx=mulii(dx,gcoeff(x,i,i));
  m=cgetg((N<<1)+1,19);
  for(i=1;i<=N;i++) m[i]=(long)element_muli(nf,(GEN)vp[2],(GEN)x[i]);
  for(i=N+1;i<=(N<<1);i++) m[i]=lmul((GEN)vp[1],(GEN)x[i-N]);
  tetpil=avma;p1=fasthnf(m,dx);
  if(gcmp1(denx))
    {
      if(f) {y=cgetg(3,17);y[1]=(long)p1;y[2]=lcopy((GEN)ix[2]);}
      else y=p1;
    }
  else
    {
      tetpil=avma;
      if(f) {y=cgetg(3,17);y[1]=ldiv(p1,denx);y[2]=lcopy((GEN)ix[2]);}
      else y=gdiv(p1,denx);
    }
  return gerepile(av,tetpil,y);
}

GEN idealmulspec(GEN nf, GEN x, GEN spec)
/* a usage interne, aucune verif. ix est un ideal entier sans partie 
archimedienne en HNF, spec=[a,alpha,n] representant un ideal aZ_K+alpha Z_K
de norme n. Calcule le produit */
{
  long av=avma,tetpil,i,N=lgef((GEN)nf[1])-3;
  GEN dx,m;

  dx=(GEN)spec[3];for(i=1;i<=N;i++) dx=mulii(dx,gcoeff(x,i,i));
  m=cgetg((N<<1)+1,19);
  for(i=1;i<=N;i++) m[i]=(long)element_muli(nf,(GEN)spec[2],(GEN)x[i]);
  for(i=N+1;i<=(N<<1);i++) m[i]=lmul((GEN)spec[1],(GEN)x[i-N]);
  tetpil=avma;return gerepile(av,tetpil,fasthnf(m,dx));
}

long idealvalint(GEN nf, GEN x, GEN vp)
                  
/* recoit un ideal entier x et un ideal premier vp dans le format
donne par primedec et calcule la valuation de ix en vp. A usage interne, pas de verifs.
*/

{
  long N,v,w,av=avma,i,j,bo;
  GEN mat,d,bp,p=(GEN)vp[1],p1,r;

  N=lg((GEN)x[1])-1;for(d=gun,i=1;i<=N;i++) d=mulii(d,(GEN)coeff(x,i,i));
  v=ggval(d,p);if(!v) return 0;
  bo=0;w=0;bp=(GEN)vp[5];
  do
    {
      if(w) {for(i=1;i<=N;i++) mat[i]=(long)element_muli(nf,(GEN)mat[i],bp);}
      else 
	{
	  mat=cgetg(N+1,19);
	  for(i=1;i<=N;i++) mat[i]=(long)element_mulh(nf,i,N,(GEN)x[i],bp);
	}
      if(divise(gcoeff(mat,N,N),p))
	{
	  for(j=1;j<=N;j++)
	    for(i=1;i<=N;i++)
	      {
		p1=dvmdii(gcoeff(mat,i,j),p,&r);
		if(signe(r)) goto labelivint; else coeff(mat,i,j)=(long)p1;
	      }
	  w++;
	}
      else bo=1;
    }
  while((bo==0)&&(w<v));
 labelivint:
  avma=av;return w;
}

long elementval(GEN nf, GEN x, GEN vp)
/* a usage interne, pas de verifs. */
{
  long av=avma,tetpil,N=lgef((GEN)nf[1])-3,w,bo,i,vd,v,e=itos((GEN)vp[3]);
  GEN denx,p1,p=(GEN)vp[1],bp,d,r;

  switch(typ(x))
    {
    case 1: case 4: case 5:
      return ggval(x,p)*e;
    case 10: d=gabs(subres(x,(GEN)nf[1]));
      x=(GEN)principalideal(nf,x)[1];break;
    case 9: d=gabs(subres((GEN)x[2],(GEN)nf[1]));
      x=(GEN)principalideal(nf,x)[1];break;
    default: err(talker,"incorrect type in elementval");
    }
  denx=denom(x);if(!gcmp1(denx)) x=gmul(denx,x);
  vd=ggval(denx,p);v=ggval(d,p)+N*vd;
  if(!v) return -vd*e;
  bo=0;w=0;bp=(GEN)vp[5];
  do
    {
      x=element_muli(nf,x,bp);
      if(divise((GEN)x[N],p))
	{
	  for(i=1;i<=N;i++)
	    {
	      p1=dvmdii((GEN)x[i],p,&r);
	      if(signe(r)) goto labelelv; else x[i]=(long)p1;
	    }
	  w++;
	}
      else bo=1;
    }
  while((bo==0)&&(w<v));
 labelelv:
  avma=av;return w-vd*e;
}

long elementval2(GEN nf, GEN x, GEN d, GEN vp)
/* a usage interne, pas de verifs. */
{
  long av=avma,tetpil,N=lgef((GEN)nf[1])-3,w,bo,i,v;
  GEN denx,p1,p=(GEN)vp[1],bp,r;

  v=ggval(d,p);
  if(!v) return 0;
  bo=0;w=0;bp=(GEN)vp[5];
  do
    {
      x=element_muli(nf,x,bp);
      if(divise((GEN)x[N],p))
	{
	  for(i=1;i<=N;i++)
	    {
	      p1=dvmdii((GEN)x[i],p,&r);
	      if(signe(r)) goto labelelv2; else x[i]=(long)p1;
	    }
	  w++;
	}
      else bo=1;
    }
  while((bo==0)&&(w<v));
 labelelv2:
  avma=av;return w;
}

long idealval(GEN nf, GEN ix, GEN vp)
                  
/* recoit un ideal ix et un ideal premier vp dans le format
donne par primedec et calcule la valuation de ix en vp */

{
  long N,v,vd,w,av=avma,i,j,bo;
  GEN mat,x,d,bp,p,p1,r,denx;

  if((typ(nf)!=17)||(lg(nf)<10)) err(idealer1);
  if((typ(vp)!=17)||(lg(vp)!=6)) err(idealer3);
  if(typ(ix)<=9) return elementval(nf,ix,vp);
  p=(GEN)vp[1];
  if((typ(ix)==17)&&(lg(ix)==3)) x=(GEN)ix[1]; else x=ix;
  if(typ(x)!=19) err(idealer2);
  denx=denom(x);
  if(!gcmp1(denx)) x=gmul(denx,x);
  N=lgef((GEN)nf[1])-3;if(lg((GEN)x[1])!=(N+1)) err(idealer4);
  if(lg(x)!=(N+1)) x=idealmul(nf,x,idmat(N));
  for(d=gun,i=1;i<=N;i++) d=mulii(d,(GEN)coeff(x,i,i));
  v=ggval(d,p);vd=ggval(denx,p);
  if(!v) return -vd*itos((GEN)vp[3]);
  bo=0;w=0;bp=(GEN)vp[5];
  do
    {
      if(w) {for(i=1;i<=N;i++) mat[i]=(long)element_muli(nf,(GEN)mat[i],bp);}
      else 
	{
	  mat=cgetg(N+1,19);
	  for(i=1;i<=N;i++) mat[i]=(long)element_mulh(nf,i,N,(GEN)x[i],bp);
	}
      if(divise(gcoeff(mat,N,N),p))
	{
	  for(j=1;j<=N;j++)
	    for(i=1;i<=N;i++)
	      {
		p1=dvmdii(gcoeff(mat,i,j),p,&r);
		if(signe(r)) goto labeliv; else coeff(mat,i,j)=(long)p1;
	      }
	  w++;
	}
      else bo=1;
    }
  while((bo==0)&&(w<v));
 labeliv:
  avma=av;return w-vd*itos((GEN)vp[3]);
}

GEN ideallllredall(GEN nf, GEN ix, GEN vdir, long prec, long precint)
{
  long N=lgef((GEN)nf[1])-3,av=avma,tetpil,i,j,f,r1,r2,ru;
  GEN T,p1,p2,p3,p4,y,alpha,beta,x,x2,v,z,detmat;

  if((typ(nf)!=17)||(lg(nf)<10)) err(idealer1);
  if((!gcmp0(vdir))&&(typ(vdir)!=17)) err(idealer5);
  if(typ(ix)<=9) ix=principalidele(nf,ix);
  if((typ(ix)==17)&&(lg(ix)==6)) {f=0;ix=idealmulprime(nf,idmat(N),ix);}
  if((typ(ix)==17)&&(lg(ix)==3)) {f=1;x=(GEN)ix[1];} else {f=0;x=ix;}
  if(typ(x)!=19) err(idealer2);
  p1=content(x);if(!gcmp1(p1)) x=gdiv(x,p1);
  p1=(GEN)nf[2];r2=itos((GEN)p1[2]);ru=N-r2;r1=ru-r2;p1=(GEN)nf[5];
  if(!gcmp0(vdir))
    {
      if(lg(vdir)!=(ru+1)) err(idealer5);
      p3=(GEN)p1[2];p2=cgetg(ru+1,19);
      for(j=1;j<=ru;j++) 
	{
	  if(typ((GEN)vdir[j])==1) 
	    p2[j]=lmul2n((GEN)p3[j],itos((GEN)vdir[j])<<1);
	  else
	    p2[j]=lmul((GEN)p3[j],gpui(stoi(4),(GEN)vdir[j]));
	}
      p1=greal(gmul(p2,(GEN)p1[1]));
    }
  else p1=(GEN)p1[3];
  p4=gmul(x,lllint(x));
  y=gmul(p4,(GEN)(lllgram(gmul(gtrans(p4),gmul(p1,p4)),2*precint)[1]));
  for(i=2;(i<=N)&&gcmp0((GEN)y[i]);i++);
  if(i>N) 
    {
      tetpil=avma;
      if(f) {y=cgetg(3,17);y[1]=lcopy(x);y[2]=lcopy((GEN)ix[2]);}
      else y=gcopy(x);
      return gerepile(av,tetpil,y);
    }
  T=(GEN)nf[1];alpha=gmodulcp(gmul((GEN)nf[7],y),T);
  beta=lift(gdiv(gnorm(alpha),alpha));
  z=gmul((GEN)((GEN)(nf[5]))[1],y);
  p1=cgetg(N+1,18);for(i=1;i<=N;i++) p1[i]=(long)truecoeff(beta,i-1);
  p1=gmul((GEN)nf[8],p1);p2=cgetg(N+1,19);
  for(i=1;i<=N;i++) p2[i]=(long)element_muli(nf,p1,(GEN)x[i]);
  p1=content(p2);if(!gcmp1(p1)) p2=gdiv(p2,p1);
  detmat=detint(p2);
  if(f)
    {
      x2=(GEN)ix[2];v=cgetg(ru+1,17);
      for(i=1;i<=r1;i++) v[i]=(long)glog((GEN)z[i],prec);
      for(i=r1+1;i<=ru;i++) v[i]=lmul2n(glog((GEN)z[i],prec),1);
      tetpil=avma;y=cgetg(3,17);y[1]=(long)hnfmod(p2,detmat);p1=cgetg(ru+1,17);
      y[2]=(long)p1;for(i=1;i<=ru;i++) p1[i]=lsub((GEN)x2[i],(GEN)v[i]);
    }
  else {tetpil=avma;y=hnfmod(p2,detmat);}
  return gerepile(av,tetpil,y);
}

GEN ideallllred(GEN nf, GEN ix, GEN vdir, long prec)
{
  return ideallllredall(nf,ix,vdir,prec,prec);
}

GEN ideallllredpart1(GEN nf, GEN x, GEN vdir, long prec)
/* a usage interne, pas de verifs */
{
  long N=lgef((GEN)nf[1])-3,av=avma,tetpil,i,j,f,r1,r2,ru;
  GEN T,p1,p2,p3,p4,p5,y,alpha,beta,x2,v,z,detmat;

  p1=content(x);if(!gcmp1(p1)) x=gdiv(x,p1);
  p1=(GEN)nf[2];r2=itos((GEN)p1[2]);ru=N-r2;p1=(GEN)nf[5];
  if(!gcmp0(vdir))
    {
      p3=(GEN)p1[2];p2=cgetg(ru+1,19);
      for(j=1;j<=ru;j++) 
	{
	  if(typ((GEN)vdir[j])==1) 
	    p2[j]=lmul2n((GEN)p3[j],itos((GEN)vdir[j])<<1);
	  else
	    p2[j]=lmul((GEN)p3[j],gpui(stoi(4),(GEN)vdir[j]));
	}
      p1=greal(gmul(p2,(GEN)p1[1]));
    }
  else p1=(GEN)p1[3];
/*
  p4=gmul(x,lllint(x));
*/
  p4=x;
  p5=lllgram(gmul(gtrans(p4),gmul(p1,p4)),2*prec);
  y=gmul(p4,(GEN)p5[1]);
  for(i=2;(i<=N)&&gcmp0((GEN)y[i]);i++);
  if(i>N) y=gmul(p4,(GEN)p5[2]);
  T=(GEN)nf[1];alpha=gmul((GEN)nf[7],y);p1=subres(alpha,(GEN)nf[1]);
  tetpil=avma;z=cgetg(4,17);z[1]=lcopy(x);z[2]=lcopy(y);z[3]=(long)gabs(p1);
  return gerepile(av,tetpil,z);
}

GEN ideallllredpart2(GEN nf, GEN arch, GEN gamma, long prec)
{
  long N=lgef((GEN)nf[1])-3,av=avma,tetpil,i,r1,r2,ru;
  GEN p1,r,v,z;
  
  z=gmul((GEN)((GEN)(nf[5]))[1],gamma);
  p1=(GEN)nf[2];r2=itos((GEN)p1[2]);ru=N-r2;r1=ru-r2;
  v=cgetg(ru+1,17);
  for(i=1;i<=r1;i++) v[i]=(long)glog((GEN)z[i],prec);
  for(i=r1+1;i<=ru;i++) v[i]=lmul2n(glog((GEN)z[i],prec),1);
  tetpil=avma;p1=cgetg(ru+1,17);
  for(i=1;i<=ru;i++) p1[i]=lsub((GEN)arch[i],(GEN)v[i]);
  return gerepile(av,tetpil,p1);
}

GEN minideal(GEN nf, GEN ix, GEN vdir, long prec)
{
  long N=lgef((GEN)nf[1])-3,av=avma,tetpil,i,j,f,r1,r2,ru;
  GEN p1,p2,p3,y,x,v,z;

  if((typ(nf)!=17)||(lg(nf)<10)) err(idealer1);
  if((!gcmp0(vdir))&&(typ(vdir)!=17)) err(idealer5);
  if((typ(ix)==17)&&(lg(ix)==3)) {f=1;x=(GEN)ix[1];} else {f=0;x=ix;}
  if(typ(x)!=19) err(idealer2);
  p1=(GEN)nf[2];r2=itos((GEN)p1[2]);ru=N-r2;r1=ru-r2;p1=(GEN)nf[5];
  if(!gcmp0(vdir))
    {
      p3=(GEN)p1[2];p2=cgetg(ru+1,19);
      for(j=1;j<=ru;j++) p2[j]=lmul2n((GEN)p3[j],itos((GEN)vdir[j])<<1);
      p1=greal(gmul(p2,(GEN)p1[1]));
    }
  else p1=(GEN)p1[3];
  y=gmul(x,(GEN)(lllgram(gmul(gtrans(x),gmul(p1,x)),prec)[1]));
  z=gmul((GEN)((GEN)(nf[5]))[1],y);
  tetpil=avma;p2=cgetg(3,17);p2[1]=(long)gtomat(y);v=cgetg(ru+1,17);
  for(i=1;i<=r1;i++) v[i]=(long)glog((GEN)z[i],prec);
  for(i=r1+1;i<=ru;i++) v[i]=lmul2n(glog((GEN)z[i],prec),1);
  p2[2]=(long)v;
  return gerepile(av,tetpil,p2);
}

/****************************************/
/* Calcul de la differente d'un corps K */
/****************************************/

GEN differente(GEN nf)
            
/* Calcule la differente de nf */

{
  long av=avma,tetpil,i,j,k,vi,ei,v,nb_p,N,vpc,a;
  GEN ideal,premiers,mat_diff,liste_id,p1,p2,pcon,pr,pol;

  pol=(GEN)nf[1];N=lgef(pol)-3;
  if(DEBUGLEVEL) fprintf(errfile,"calcul de la differente\n");
  if(gcmp1((GEN)nf[4]))
    {
      p1=gmodulcp(deriv(pol,varn(pol)),pol);p2=idmat(N);
      tetpil=avma;return gerepile(av,tetpil,idealmul(nf,p1,p2));
    }
  ideal=gmul((GEN)nf[3],ginv((GEN)((GEN)nf[5])[4]));
  ideal=gdiv(ideal,pcon=content(ideal));
  if(DEBUGLEVEL) {fprintf(errfile,"temps D*delta^-1: ");fprintf(errfile,"%ld\n",timer2());fflush(errfile);}
  ideal=hnfmodid(ideal,divii((GEN)nf[3],pcon));
  if(DEBUGLEVEL) {fprintf(errfile,"temps hnf(D*delta^-1): ");fprintf(errfile,"%ld\n",timer2());fflush(errfile);}
  mat_diff=idmat(lgef((GEN)nf[1])-3);premiers=factor(gabs((GEN)nf[3]));
  nb_p=lg((GEN)premiers[1]);
  if(DEBUGLEVEL) {fprintf(errfile,"temps factor(D): ");fprintf(errfile,"%ld\n",timer2());fflush(errfile);}
  for(i=1;i<nb_p;i++)
    {
      liste_id=primedec(nf,pr=gcoeff(premiers,i,1));vi=itos(gcoeff(premiers,i,2));
      vpc=ggval(pcon,pr);
      for(j=1;j<lg(liste_id);j++)
	{
	  p1=(GEN)liste_id[j];ei=itos((GEN)p1[3]);
	  if(ei>1)
	    {
	      if(DEBUGLEVEL) 
		{
		  temp=outfile;outfile=errfile;
		  fprintf(errfile,"traitement de ");output(p1);
		  fflush(errfile);outfile=temp;
		}
	      if(signe(ressi(ei,pr))) v=ei-1;
	      else
		{
		  v=ei*(vi-vpc)-idealval(nf,ideal,p1);
		}
	      a=1+(v-1)/ei;p2=cgetg(4,17);p2[1]=(long)gpuigs(pr,a);
	      p2[2]=(long)element_pow(nf,(GEN)p1[2],stoi(v));
	      p2[3]=(long)gpui(pr,mulsi(v,(GEN)p1[4]));
	      mat_diff=idealmulspec(nf,mat_diff,p2);
	    }
	}
    }
  tetpil=avma;return gerepile(av,tetpil,gcopy(mat_diff));
}

/****************************************/
/* Calcul de l'inverse d'un ideal       */
/****************************************/

GEN idealinv(GEN nf, GEN ix)
               
/* Calcule le dual de mat_id pour la forme trace */

{
  long av=avma,tetpil,f,tx=typ(ix),N;
  GEN mat_dual,di,x,y,p1;

  if((typ(nf)!=17)||(lg(nf)<10)) err(idealer1);
  if(tx<=9)
    {
      x=principalideal(nf,ginv(ix));y=idmat(lgef((GEN)nf[1])-3);
      tetpil=avma;return gerepile(av,tetpil,idealmul(nf,x,y));
    }
  if(tx==17)
    {
      if(lg(ix)==6) 
	{
	  p1=cgetg(6,17);p1[1]=ix[1];p1[2]=ix[5];N=lgef((GEN)nf[1])-3;
	  p1[3]=p1[5]=zero;p1[4]=(long)subsi(N,(GEN)ix[4]);
	  p1=idealmulprime(nf,idmat(N),p1);tetpil=avma;
	  return gerepile(av,tetpil,gdiv(p1,(GEN)ix[1]));
	}
      if(lg(ix)!=3) err(idealer2);
      else {f=1;x=(GEN)ix[1];}
    }
  else {f=0;x=ix;}
  if(typ(x)!=19) err(idealer2);
  mat_dual=ginv(gmul(gtrans(x),(GEN)((GEN)nf[5])[4]));
  di=denom(mat_dual);mat_dual=gmul(di,mat_dual);
  mat_dual=idealmul(nf,(GEN)((GEN)nf[5])[5],mat_dual);
  tetpil=avma;
  if(f) {y=cgetg(3,17);y[1]=ldiv(mat_dual,di);y[2]=lneg((GEN)ix[2]);}
  else y=gdiv(mat_dual,di);
  return gerepile(av,tetpil,y);
}

/* Eleve un ideal premier vp a la puissance n ou n est dans Z */

GEN idealpowprime(GEN nf, GEN vp, GEN n, long prec)
{
  long N=lgef((GEN)nf[1])-3,RU,av=avma,tetpil,s,i,m,ns;
  unsigned long j;
  GEN x,p1,p2,p3,vpinv;

  if((typ(nf)!=17)||(lg(nf)<10)) err(idealer1);
  if((typ(vp)!=17)||(lg(vp)!=6)) err(idealer3);
  p1=(GEN)nf[2];RU=itos((GEN)p1[1])+itos((GEN)p1[2]);
  x=cgetg(3,17);x[1]=(long)idmat(N);
  p1=cgetg(RU+1,17);
  for(i=1;i<=RU;i++)
    {
      p2=cgetg(3,6);p1[i]=(long)p2;
      p3=cgetr(prec);p2[1]=(long)p3;
      p3=cgetr(prec);p2[2]=(long)p3;
      affsr(0,(GEN)p2[1]);affsr(0,(GEN)p2[2]);
    }
  x[2]=(long)p1;
  s=signe(n);if(!s) return x;
  if(s<0) 
    {
      n=negi(n);vpinv=cgetg(6,17);vpinv[1]=vp[1];vpinv[2]=vp[5];
      vpinv[3]=vp[3];vpinv[4]=(long)subsi(N,(GEN)vp[4]);vpinv[5]=vp[2];
      vp=vpinv;
    }
  if(gcmpgs(n,16)<0)
    {
      ns=n[2];
      for(j=1;j<ns;j++) x=ideallllred(nf,idealmulprime(nf,x,vp),gzero,prec);
      tetpil=avma;x=ideallllred(nf,idealmulprime(nf,x,vp),gzero,prec);
    }
  else
    {
      m=n[lgef(n)-1];j=HIGHBIT;
      while((m&j)==0) 
	j>>=1;
      x=idealmulprime(nf,x,vp);j>>=1;
      if(gcmp1(n)) {tetpil=avma;x=ideallllred(nf,x,gzero,prec);}
      for(;j;j>>=1)
	{
	  x=idealmulh(nf,x,x);
	  if(m&j) x=idealmulprime(nf,x,vp);
	  tetpil=avma;x=ideallllred(nf,x,gzero,prec);
	}
      for (i=lgef(n)-2;i>=2;i--)
	{
	  for (m=n[i],j=HIGHBIT;j;j>>=1)
	    {
	      x=idealmulh(nf,x,x);
	      if (m&j) x=idealmulprime(nf,x,vp);
	      tetpil=avma;x=ideallllred(nf,x,gzero,prec);
	    }
	}
    }
  return gerepile(av,tetpil,x);
}

/* Eleve un ideal ix a la puissance n ou n est dans Z */

GEN idealpow(GEN nf, GEN ix, GEN n)
{
  long N,av=avma,tetpil,s,i,j,m,ns,f;
  GEN iy,iz,y,x;

  if((typ(nf)!=17)||(lg(nf)<10)) err(idealer1);
  N=lgef((GEN)nf[1])-3;iy=idmat(N);
  if(typ(ix)<=9)
    {
      x=principalideal(nf,gpui(ix,n));tetpil=avma;
      return gerepile(av,tetpil,idealmul(nf,x,iy));
    }
  if((typ(ix)==17)&&(lg(ix)==6)) ix=idealmulprime(nf,iy,ix);
  if((typ(ix)==17)&&(lg(ix)==3)) {f=1;x=(GEN)ix[1];} else {f=0;x=ix;}
  if(typ(x)!=19) err(idealer2);
  if((N!=lg(x)-1)||(N!=lg((GEN)x[1])-1)) err(idealer4);
  s=signe(n);
  if(!s) 
    {
      if(f) {y=cgetg(3,17);y[1]=(long)iy;y[2]=lmul(gzero,(GEN)ix[2]);}
      else y=iy;
      return y;
    }
  if(s<0) n=negi(n);
  iz=x;
  for (i=lgef(n)-1;i>2;i--)
    {
      for (m=n[i],j=0;j<BITS_IN_LONG;j++,m>>=1)
        {
          if (m&1) iy=idealmulh(nf,iy,iz);
          iz=idealmulh(nf,iz,iz);
        }
    }
  for (m=n[2];m>1;m>>=1)
    {
      if (m&1) iy=idealmulh(nf,iy,iz);
      iz=idealmulh(nf,iz,iz);
    }
  tetpil=avma;iy=idealmulh(nf,iy,iz);
  if(s<0) {n=negi(n);tetpil=avma;iy=idealinv(nf,iy);}
  if(f) {y=cgetg(3,17);y[1]=(long)iy;y[2]=lmul(n,(GEN)ix[2]);}
  else y=iy;
  return gerepile(av,tetpil,y);
}

GEN idealpowred(GEN nf, GEN ix, GEN n, long prec)
{
  long N,av=avma,tetpil,s,i,j,m,ns,f;
  GEN iy,iz,y,x;

  if((typ(nf)!=17)||(lg(nf)<10)) err(idealer1);
  N=lgef((GEN)nf[1])-3;iy=idmat(N);
  if(typ(ix)<=9) ix=idealmul(nf,principalideal(nf,ix),iy);
  if((typ(ix)==17)&&(lg(ix)==6)) return idealpowprime(nf,ix,n,prec);
  if((typ(ix)==17)&&(lg(ix)==3)) {f=1;x=(GEN)ix[1];} else {f=0;x=ix;}
  if(typ(x)!=19) err(idealer2);
  if((N!=lg(x)-1)||(N!=lg((GEN)x[1])-1)) err(idealer4);
  s=signe(n);
  if(!s) 
    {
      if(f) {y=cgetg(3,17);y[1]=(long)iy;y[2]=lmul(gzero,(GEN)ix[2]);}
      else y=iy;
      return y;
    }
  if(s<0) n=negi(n);
  iz=x;
  for (i=lgef(n)-1;i>2;i--)
    {
      for (m=n[i],j=0;j<BITS_IN_LONG;j++,m>>=1)
        {
          if (m&1) iy=ideallllred(nf,idealmulh(nf,iy,iz),gzero,prec);
          iz=ideallllred(nf,idealmulh(nf,iz,iz),gzero,prec);
        }
    }
  for (m=n[2];m>1;m>>=1)
    {
      if (m&1) iy=ideallllred(nf,idealmulh(nf,iy,iz),gzero,prec);
      iz=ideallllred(nf,idealmulh(nf,iz,iz),gzero,prec);
    }
  iy=idealmulh(nf,iy,iz);if(s<0) {n=negi(n);iy=idealinv(nf,iy);}
  tetpil=avma;iy=ideallllred(nf,iy,gzero,prec);
  if(f) {y=cgetg(3,17);y[1]=(long)iy;y[2]=lmul(n,(GEN)ix[2]);}
  else y=iy;
  return gerepile(av,tetpil,y);
}
  
long isideal(GEN nf,GEN x)  
{
  long N,av,f,i,j,k,tx=typ(x);
  GEN p1,minv,be;

  if((typ(nf)!=17)||(lg(nf)<10)) err(idealer1);
  if(tx<=9)
    return ((tx==1)||(tx==4)||(tx==5)||((tx==9)&&gegal((GEN)nf[1],(GEN)x[1])));
  if((typ(x)==17)&&(lg(x)==3)) x=(GEN)x[1];
  if(typ(x)!=19) err(idealer2);
  N=lgef((GEN)nf[1])-3;if(lg((GEN)x[1])!=(N+1)) err(idealer4);
  av=avma;be=idmat(N);
  if(lg(x)!=(N+1)) x=idealmul(nf,x,idmat(N));
  x=gdiv(x,content(x));minv=ginv(x);f=1;
  for(i=1;(i<=N)&&f;i++)
    for(j=1;(j<=N)&&f;j++)
      {
	p1=gmul(minv,element_muli(nf,(GEN)x[i],(GEN)be[j]));
	for(k=1;(k<=N)&&f;k++) if(typ((GEN)p1[k])!=1) f=0;
      }
  avma=av;return f;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*                                                                 */
/*                    BASE D'ENTIERS (ROUND 4)                     */
/*                                                                 */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

int fnz(GEN x,long j)
{
  long i;
  i=1;while(!signe((GEN)x[i])) i++;
  return i==j;
}

GEN allbase4(GEN f,long code, GEN *y )
{
  GEN w,a,da,b,db,bas,q,bdiag,ab;
  long v,n,mf,h,templevel;
  long i,j,k,av=avma,tetpil,pro,first;
  
  if(typ(f)!=10) err(allbaser1);
  n=lgef(f)-3;if(n<=0) err(allbaser1);
  v=varn(f);
  *y=discsr(f);
  if(DEBUGLEVEL) {timer2();templevel=DEBUGLEVEL;DEBUGLEVEL=5;}
  switch(code)
    {
    case 0: w=auxdecomp(absi(*y),1);h=lg((GEN)w[1])-1;break; /* base */
    case 1: w=auxdecomp(absi(*y),0);h=lg((GEN)w[1])-1;break; /* smallbase */
    default: w=(GEN)code;
      if((typ(w)!=19)||(lg(w)!=3)) err(factoreder1); /* factoredbase */
      h=lg((GEN)w[1])-1;
      q=gun;for(i=1;i<=h;i++) q=gmul(q,gpui((GEN)coeff(w,i,1),(GEN)coeff(w,i,2)));
      if(gcmp(absi(q),absi((GEN)*y))) err(factoreder2);
    }
  if(DEBUGLEVEL) 
    {
      DEBUGLEVEL=templevel;
      fprintf(errfile,"temps factorisation disc: ");fprintf(errfile,"%ld\n",timer2());fflush(errfile);
    }
  a=idmat(n);da=gun;first=TRUE;
  for(i=1;i<=h;i++)
    {
      mf=itos((GEN)coeff(w,i,2));
      if(mf>1)
	{ 
	  if(DEBUGLEVEL)
	    {
	      fprintf(errfile,"On traite le cas p^k = ");
	      temp=outfile;outfile=errfile;
	      brute((GEN)coeff(w,i,1),'g',-1);
              fprintf(errfile,"^%ld\n",mf);
              outfile=temp;
	    }
	  b=(GEN)maxord((GEN)coeff(w,i,1),f,mf);
	  bdiag=cgetg(n+1,17);for(j=1;j<=n;j++) bdiag[j]=coeff(b,j,j);
	  db=denom(bdiag); 
	  if (!(gcmp1(db)))    /* la matrice est identite   */
	    {
	      da=gmul(da,db);
	      if (first!=TRUE)
		{
		  b=gmul(da,b);a=gmul(db,a);
		  for(j=1;(j<=n)&&(fnz((GEN)a[j],j)&&fnz((GEN)b[j],j));j++);
		  k=j-1;ab=cgetg(2*n-k+1,19);
		  for(j=1;j<=k;j++)
		    {
		      ab[j]=a[j];
		      coeff(ab,j,j)=(long)mppgcd(gcoeff(a,j,j),gcoeff(b,j,j));
		    }
		  for(;j<=n;j++) ab[j]=a[j];
		  for(;j<=2*n-k;j++) ab[j]=b[j+k-n];
		  a=hnf(ab);
		}
	      else {a=gmul(b,db);first=FALSE;}
	    }
          if(DEBUGLEVEL>=3)
            {
              fprintf(errfile,"Le resultat pour ce nombre p est : \n ");
              temp=outfile;outfile=errfile;
	      output(b);
              fprintf(errfile,"\n");
	      outfile=temp;
            }
	}
    } 
  for(j=1;j<=n;j++)
    {
      *y=divii(mulii((GEN)coeff(a,j,j),*y),da);
      *y=divii(mulii((GEN)coeff(a,j,j),*y),da);
    }
  tetpil=avma;*y=gcopy(*y);bas=cgetg(n+1,17);
  for(k=1;k<=n;k++)
    {
      q=cgetg(k+2,10);q[1]=evalsigne(1)+evallgef(2+k)+evalvarn(v);bas[k]=(long)q;
      for(j=2;j<=k+1;j++) q[j]=ldiv((GEN)coeff(a,j-1,k),da);
    }
  pro=lpile(av,tetpil,0)>>TWOPOTBYTES_IN_LONG;bas+=pro;(*y)+=pro;
  return bas;
}

                /*     p-maximal order of Af        */
                /*     p^m does not divide Df       */

GEN maxord(GEN p,GEN f,long mf)
{
  GEN w,g,h,res,fmp;
  long j,r,v=varn(f),n=lgef(f)-3,av=avma,tetpil,flw;

  if(flw=(cmpsi(n,p)<0))
    {
      fmp=gmul(gmodulcp(gun,p),f);
      g=gdeuc(fmp,polgcd(fmp,deriv(fmp,v)));
    }
  else 
    {
      w=factmod(f,p);r=lg((GEN)w[1])-1;g=gun;
      for(j=1;j<=r;j++) g=gmul((GEN)coeff(w,j,1),g);
    }
  res=dedek(f,p,g);
  if(itos((GEN)res[1])) {avma=av;return idmat(n);}
  else
    if (2*itos((GEN)res[3])+1>=mf)
      {
	tetpil=avma;
	return gerepile(av,tetpil,gdiv((GEN)res[2],p));
      }
    else
      {
	if(flw) {w=factmod(f,p);r=lg((GEN)w[1])-1;}
	h=bestnu(w);
	if (r==1)
	  { 
	    res=nilord(p,f,mf,h);
	    tetpil=avma;return gerepile(av,tetpil,gcopy(res));
	  }
	else
	  { 
	    tetpil=avma;return gerepile(av,tetpil,Decomp(p,f,mf,polx[v],f,h));
	  }
      }
}

GEN dedek(GEN f,GEN p,GEN g)
{
  long av=avma,tetpil,index,dk,n=lgef(f)-3;
  long dh,c,i,v=varn(f);
  GEN k,h,unmodp,U,res,b,p1;
  
  res=cgetg(4,17);
  if(DEBUGLEVEL>=3)
    {
      fprintf(errfile," On est dans Dedekind ");
      if(DEBUGLEVEL>=4)
	{
	  fprintf(errfile," avec les parametres \n" );
	  temp=outfile;outfile=errfile;
	  fprintf(errfile," p=");brute(p,'g',-1);
	  fprintf(errfile,",  f=");brute(f,'g',-1);
          outfile=temp;
	}
      fprintf(errfile,"\n");
    } 
  unmodp=gmodulcp(gun,p);
  g=gmul(g,unmodp);
 
  h=gdivent(gmul(f,unmodp),g);
  k=gdiv(gsub(lift(f),gmul(lift(g),lift(h))),p);
  k=ggcd(gmul(k,unmodp),ggcd(g,h));
  dk=lgef(k)-3;
  if(DEBUGLEVEL>=4)
   fprintf(errfile," Le pgcd est de degre %ld \n",dk );
  res[1]= (dk==0)?un:zero;
  if (dk!=0)
    {
      U=gdiv(gmul(f,unmodp),k);
      h=lift(U);
      b=cgetg(2*n+1,19);
      for(c=1;c<=n;c++)
	{
	  p1=cgetg(n+1,18);b[c]=(long)p1;
	  for(i=1;i<=n;i++) p1[i]=(c==i)?(long)p:zero;
	}
      for(c=n+1;c<=2*n;c++)
	{
	  dh=lgef(h)-3;
	  p1=cgetg(n+1,18);b[c]=(long)p1;
	  for(i=1;i<=dh+1;i++) p1[i]=(long)lift(gmul((GEN)h[1+i],unmodp));
	  for(i=dh+2;i<=n;i++) p1[i]=zero;
	  if (c!=2*n) {h=gmod(gmul(h,polx[v]),f);} 
	}
    if(DEBUGLEVEL>=4)
	{
	  fprintf(errfile," On construit un nouvel ordre  \n" );
	  if(DEBUGLEVEL>=5)
	    {
	      temp=outfile;outfile=errfile;
	      output(b);
	    }
	  fprintf(errfile," On fait sa HNF \n");
	  outfile=temp;
	}
/*      b=hnfmod(b,gpuigs(p,n)); */
      b=hnfmodid(b,p);
      if(DEBUGLEVEL>=4)
	{
          fprintf(errfile," Sa HNF est finie \n");
	  if(DEBUGLEVEL>=5)
	    {
	      temp=outfile;outfile=errfile;
	      output(b);
	      outfile=temp;
	    }
	}

      res[2]=(long)b;
      
      index=0;
      for(i=1;i<=n;i++)
	index=index+ggval((GEN)coeff(b,i,i),p);
      res[3]=lstoi(n-index);
    }
  else
    {
      res[2]=zero;
      res[3]=un;
    }
  tetpil=avma;
  return gerepile(av,tetpil,gcopy(res));
}

GEN Decomp(GEN p,GEN f,long mf,GEN theta,GEN chi,GEN nu)
{
  long df,n1,n2,j,i,v2,v1,v=varn(f),av=avma,tetpil;
  GEN unmodpdr,unmodp,unmodpdrp,unmodpkpdr,unmodpmr;
  GEN pk,ph,pmr,pdr,pme,pde;
  GEN b1,b2,b3,a2,a1,e,f1,f2;
  GEN ib1,ib2,ibas,h;
  GEN df1,df2;

  if(DEBUGLEVEL>=3)
    {
      fprintf(errfile," On entre dans Decomp ");
      if(DEBUGLEVEL>=4)
	{
	  temp=outfile;outfile=errfile;
	  fprintf(errfile," avec les parametres suivants \n ");
	  fprintf(errfile," p=");brute(p,'g',-1);
	  fprintf(errfile,",  f=");brute(f,'g',-1);
	  fprintf(errfile,",  exposant=%ld ",mf);
	  outfile=temp;
	}
      fprintf(errfile,"\n");
    }
  
  unmodp=gmodulcp(gun,p);

  pdr=(GEN)respm(f,deriv(f,v),gpuigs(p,mf));
  pmr=mulii(pdr,mulii(pdr,p));

  unmodpmr=gmodulcp(gun,pmr);
  unmodpdr=gmodulcp(gun,pdr);
  unmodpdrp=gmodulcp(gun,mulii(pdr,p));

  b1=gmul(chi,unmodp);  a2=gzero;
  b2=unmodp;            a1=gun;
  b3=gmul(nu,unmodp);
  
  while ( lgef(b3)>3 )
    {
      b1=gdivent(b1,b3);
      b2=gmul(b2,b3);
      b3=lift(gbezout(b2,b1,&a1,&a2));
    }
                           
  e=(GEN)eleval(f,lift(gmul(a1,b2)),theta);
  e=gdiv(lift(gmul(gmul(pdr,e),unmodpdrp)),pdr); 

  pk=p;
  ph=mulii(pdr,pmr); 

  /*    E(t)- e(t) belongs to p^k Op, which is contained in p^(k-df)*Zp[xi]  */

  while (cmpii(pk,ph)==-1)
    {
      e=gmod(gmul(e,gmul(e,gsubsg(3,gmulsg(2,e)))),f);
      pk=gmul(pk,pk);
      unmodpkpdr=gmodulcp(gun,mulii(pk,pdr ));
      e=gdiv(lift(gmul(gmul(pdr,e),unmodpkpdr)),pdr);
    }  
  
  f1=(GEN)gcdpm(f,gmul(pdr,gsubsg(1,e)),mulii(pmr,pdr));f1=lift(gmul(gmod(f1,f),unmodpmr));
  f2=gdivent(f,f1);f2=lift(gmul(gmod(f2,f),unmodpmr));

  n1=lgef(f1)-3;v1=ggval(discsr(f1),p); b1=(GEN)maxord(p,f1,v1);

  ib1=cgetg(n1+1,17);
  for(i=1;i<=n1;i++)
    {
      h=gzero;
      for(j=1;j<=i;j++)
	h=gadd(h,gmul((GEN)coeff(b1,j,i),gpuigs(polx[v],j-1)));
      ib1[i]=(long)h;
    }  
  
  n2=lgef(f2)-3; v2=ggval(discsr(f2),p); b2=(GEN)maxord(p,f2,v2);

  ib2=cgetg(n2+1,17);
  for(i=1;i<=n2;i++)
    {
      h=gzero;
      for(j=1;j<=i;j++)
	h=gadd(h,gmul((GEN)coeff(b2,j,i),gpuigs(polx[v],j-1)));
      ib2[i]=(long)h;
    }
  
  ibas=cgetg(n1+n2+1,17);


  for(j=1;j<=n1;j++)
    ibas[j]=(long)lift(gmul(gmod(gmul(gmul(pdr,(GEN)ib1[j]),e),f),unmodpdr));
  for(j=n1+1;j<=n1+n2;j++)
    ibas[j]=(long)lift(gmul(gmod(gmul(gsubsg(1,e),gmul(pdr,(GEN)ib2[j-n1])),f),unmodpdr));
  tetpil=avma;
  return gerepile(av,tetpil,nbasis(ibas,pdr));
 
}


GEN nilord(GEN p,GEN fx,long mf,GEN gx)
{
  long La,Ma,first=TRUE,v=varn(fx),av=avma,tetpil; 
  GEN alpha,chi,nu,eta,w,phi,U;
  GEN res,pm,Dchi,unmodp,unmodpm;
  

  if(DEBUGLEVEL>=3)
    {
      fprintf(errfile," On entre dans Nilord ");
      if(DEBUGLEVEL>=4)
	{
	  temp=outfile;outfile=errfile;
	  fprintf(errfile," avec les parametres suivants \n ");
	  fprintf(errfile," p=");brute(p,'g',-1);
	  fprintf(errfile,",  fx=");brute(fx,'g',-1);
	  fprintf(errfile,",  exposant=%ld,  gx= ",mf);brute(gx,'g',-1);
          outfile=temp;
	}
      fprintf(errfile,"\n");
    } 
  
  pm=gpuigs(p,mf+1);

  alpha=polx[v];     chi=fx;     nu=gx;       Dchi=gpuigs(p,mf);

  unmodpm=gmodulcp(gun,pm);
  unmodp=gmodulcp(gun,p);
 
  res=cgetg(4,17);

  while (TRUE)
    {
      if (gcmp0(Dchi))
	alpha=gadd(alpha,gmul(p,polx[v]));
      else
	{
         if (first!=TRUE)
	  {
	    res=dedek(chi,p,nu);
	  }
	 else { res[1]=zero;first=FALSE;}
	 if (itos((GEN)res[1])==1) 
	   { 
	     tetpil=avma;
	     return gerepile(av,tetpil,pbasis(p,fx,mf,alpha));
	   }
	 else
	   { 
	     if (gcmp(vstar(p,chi),gzero)==1)
	       {
		 alpha=gadd(alpha,gun);
		  chi=gsubst(chi,v,gsub(polx[v],gun));
		  nu=lift(gmul(gsubst(nu,v,gsub(polx[v],gun)),unmodp));
		}
	      w=(GEN)setup(p,chi,polx[v],nu);
	      eta=(GEN)w[2];
	      La=itos((GEN)w[3]);
	      Ma=itos((GEN)w[4]);
	      if (La>1)
		alpha=gadd(alpha,eleval(fx,eta,alpha));
	      else
		{
		  w=(GEN)bsrch(p,chi,ggval(Dchi,p),eta,Ma);
		  phi=(GEN)eleval(fx,(GEN)w[2],alpha);
		  if (gcmp1((GEN)w[1]))
		    {
		      tetpil=avma;
		      return gerepile(av,tetpil,Decomp(p,fx,mf,phi,(GEN)w[3],(GEN)w[4]));
		    }
		  else alpha=phi;
		}
	    }
	  w=(GEN)factcp(p,fx,alpha);
	  chi=(GEN)w[1];nu=(GEN)w[2];
	  if(cmpis((GEN)w[4],1)==1)
	    {
	      tetpil=avma;
	      return gerepile(av,tetpil,Decomp(p,fx,mf,alpha,chi,nu));
	    }
	  Dchi=lift(gmul(discsr(lift(gmul(chi,unmodpm))),unmodpm));
	  if (gcmp0(Dchi))
	    Dchi=discsr(chi);
	}
    }
}


/*****************************************************************************/
/****    returns                                                          ****/
/****        [1,theta,chi,nu]     if theta non-primary                    ****/
/****        [2,phi, * , * ]      if D_phi > D_alpha or M_phi > M_alpha   ****/
/*****************************************************************************/

GEN bsrch(GEN p,GEN fa,long ka,GEN eta,long Ma)
{
  long n=lgef(fa)-3,Da=lgef(eta)-3;
  long c,r,field,j,MaVb,deg,av=avma,tetpil;
  GEN pc,pcc,unmodpcc,Vb;
  GEN beta,b,gamma,delta,pik,w;
  
  pc=respm(fa,deriv(fa,varn(fa)),gpuigs(p,ka));
  c=ggval(pc,p);
  pcc=gmul(pc,pc);
  unmodpcc=gmodulcp(gun,pcc);
  
  r=1+(long)ceil(c/(double)(Da)+gtodouble(gdivsg(c*n-2,mulsi(Da,subis(p,1)))));
  
  b=cgetg(5,17);
  
  beta=gdiv(lift(gpuigs(gmodulcp(eta,fa),Ma)),p);
  
  while(TRUE)
    { 
      beta=gdiv(lift(gmul(gmul(pc,beta),unmodpcc)),pc);
      w=testd(p,fa,c,Da,eta,Ma,beta);
      if(cmpis((GEN)w[1],3)==-1) 
	{ tetpil=avma;
	  return gerepile(av,tetpil,gcopy((GEN)w));
	} 
      
      Vb=vstar(p,(GEN)w[3]);
      
      MaVb=itos(gmulsg(Ma,Vb));
      
      pik=lift(gpuigs(gmodulcp(eta,fa),MaVb));
      
      gamma=gmod(gmul(beta,(GEN)(vecbezout(pik,fa))[1]),fa);
      gamma=gdiv(lift(gmul(gmul(pc,gamma),unmodpcc)),pc);
      w=(GEN)testd(p,fa,c,Da,eta,Ma,gamma);
      if (cmpis((GEN)w[1],3)==-1) 
	{
	  tetpil=avma;
	  return gerepile(av,tetpil,gcopy((GEN)w));
	} 

      delta=eltppm(fa,pc,gamma,gpuigs(p,r*Da));
      delta=gdiv(lift(gmul(gmul(pc,delta),unmodpcc)),pc);
      w=(GEN)testd(p,fa,c,Da,eta,Ma,delta);
      if (cmpis((GEN)w[1],3)==-1)
	{
	  tetpil=avma;
	  return gerepile(av,tetpil,gcopy((GEN)w));
	} 
      
      field=TRUE;
      deg=lgef(delta)-3;
      for(j=0;j<=deg;j++)
	if (!(gcmp0((GEN)delta[j+2])))
	  if (ggval((GEN)delta[j+2],p) < 0)  field=FALSE;
      if (field) 
	beta=gsub(beta,gmod(gmul(pik,delta),fa));
      else
	{ 
	  tetpil=avma;
	  return gerepile(av,tetpil,csrch(p,fa,gamma));
	} 
    }
}

/*****************************************************************************/
/****    returns                                                          ****/
/****        [1,phi,chi,nu]      if theta non-primary                    ****/
/****        [2,phi,chi,nu]      if D_phi > D_aplha or M_phi > M_alpha   ****/
/****        [3,phi,chi,nu]    otherwise                               ****/
/*****************************************************************************/

GEN testd(GEN p,GEN fa,long c,long Da,GEN alph2,long Ma,GEN theta)
{
  long Mt,Dt,v=varn(fa),av=avma,tetpil;
  GEN chit,nut,thet2,b,w;
  
  b=cgetg(5,17);
  
  
  w=factcp(p,fa,theta);
  chit=(GEN)w[1];
  nut=(GEN)w[2];
  Dt=itos((GEN)w[3]);

  if (cmpis((GEN)w[4],1)==1)
    {
      b[1]=un;
      b[2]=(long)theta;
      b[3]=(long)chit;
      b[4]=(long)nut;
      tetpil=avma;
      return gerepile(av,tetpil,gcopy((GEN)b));
    } 

  if (Da< clcm(Da,Dt)) 
    { 
      tetpil=avma;
      return gerepile(av,tetpil,testb(p,fa,Da,theta,Dt));
    }
  
  w=setup(p,fa,theta,nut);
  thet2=(GEN)w[2];
  Mt=itos((GEN)w[4]);
  
  if (Ma < clcm(Ma,Mt))
    {
      tetpil=avma;
      return gerepile(av,tetpil,testc(p,fa,c,alph2,Ma,thet2,Mt));
    }
  else
    {   
      b[1]=(long)stoi(3);
      b[2]=(long)theta;
      b[3]=(long)chit;
      b[4]=(long)nut; 
      tetpil=avma;
      return gerepile(av,tetpil,gcopy((GEN)b));
    }
}


/****************************************************************************/
/*****        Returns [1,phi,chi,nu] if phi non-primary                 *****/
/*****                [2,phi,chi,nu] if D_phi = lcm (D_alpha, D_theta)  *****/
/****************************************************************************/

GEN testc(GEN p, GEN fa, long c, GEN alph2, long Ma, GEN thet2, long Mt)

{
  GEN b,pc,ppc,c1,c2,c3,psi,unmodppc,phi,w;
  long g,r,s,t,v=varn(fa),av=avma,tetpil;

  b=cgetg(5,17);
  pc=gpuigs(p,c);
  ppc=mulii(pc,p);
  unmodppc=gmodulcp(gun,ppc);

  g=cbezout(Ma,Mt,&r,&s);
  t=0;
  while (r<0)
    {
      r=r+Mt;
      t++;
    }
  while (s<0)
    {
      s=s+Ma;
      t++;
    }
  c1=lift(gpuigs(gmodulcp(alph2,fa),s));
  c2=lift(gpuigs(gmodulcp(thet2,fa),r));
  c3=gdiv(gmod(gmul(c1,c2),fa),gpuigs(p,t));
  psi=gdiv(lift(gmul(gmul(pc,c3),unmodppc)),pc);
  phi=gadd(polx[v],psi);

  w=factcp(p,fa,phi);
  if(cmpis((GEN)w[4],1)==1)
    {
      b[1]=un;
      b[2]=(long)phi;
      b[3]=w[1];
      b[4]=w[2];
      tetpil=avma;
      return gerepile(av,tetpil,gcopy((GEN)b));
    }
  else
    {   
      b[1]=deux;
      b[2]=(long)phi;
      b[3]=w[1];
      b[4]=w[2]; 
      tetpil=avma;
      return gerepile(av,tetpil,gcopy((GEN)b));
    }
}


/****************************************************************************/
/*****        Returns [1,phi,chi,nu] if phi non-primary                 *****/
/*****                [2,phi,chi,nu] if D_phi = lcm (D_alpha, D_theta)  *****/
/****************************************************************************/


GEN testb(GEN p,GEN fa,long Da,GEN theta,long Dt)
{
  long Dat,t,j,vf=varn(fa),av=avma,tetpil;
  GEN b,w,r,v;
  GEN phi,h;
  
  
  Dat=clcm(Da,Dt);
  b=cgetg(5,17);
  t=0;
  
  while (TRUE)
    {
      t++;
      v=stoi(t);
      h=gzero;
      j=0;
      while (!(gcmp0(v)))
	{
	  r=gmod(v,p);
	  v=gdivent(v,p);
	  h=gadd(h,gmul(r,gpuigs(polx[vf],j)));
	  j++;
	}
      phi=gadd(theta,gmod(h,fa));
      w=factcp(p,fa,phi);
      if (cmpis((GEN)w[4],1)==1)
	{
	  b[1]=un;
	  b[2]=(long)phi;
	  b[3]=w[1];
	  b[4]=w[2];
	  tetpil=avma;
	  return gerepile(av,tetpil,gcopy((GEN)b));
	}
      if (cmpis((GEN)w[3],Dat)==0)
	{
          b[1]=deux;
          b[2]=(long)phi;
          b[3]=w[1];
          b[4]=w[2];
          tetpil=avma;
          return gerepile(av,tetpil,gcopy((GEN)b));
        }
    }
}



/****************************************************************************/
/*****        Factorize characteristic polynomial of beta mod p         *****/
/****************************************************************************/

GEN factcp(GEN p,GEN f,GEN beta)
{
  GEN chi,nu,b;
  long v,av=avma,tetpil;
  
  v=varn(f);
  chi=lift(caradj0(gmodulcp(beta,f),v));
  nu=lift(factmod(chi,p));
  
  b=cgetg(5,17);
  
  b[1]=(long)chi;
  b[2]=coeff(nu,1,1);
  b[3]=lstoi(lgef((GEN)b[2])-3);
  b[4]=lstoi(lg((GEN)nu[1])-1);
  tetpil=avma;
  return gerepile(av,tetpil,gcopy((GEN)b));
}

/*****************************************************************************/
/*****************          minimum extension valuation     ******************/
/*****************************************************************************/

GEN vstar(GEN p,GEN h)
{
  long m,first,j,av=avma,tetpil;
  GEN g,y,w,v;
  
  m=lgef(h)-3;
  first=TRUE;
  v=gzero;
  for(j=1;j<=m;j++)
    if (!(gcmp0((GEN)h[m-j+2])))
      {
	w=gdiv(stoi(ggval((GEN)h[m-j+2],p)),stoi(j));
	if (first) 
	  v=w;
	else 
	  if(gcmp(w,v)==-1) v=w;
	first=FALSE;
      }
  tetpil=avma;
  return gerepile(av,tetpil,gcopy(v));
}

/****************************************************************************/
/*****  Returns [theta_1,theta_2,L_theta,M_theta] with theta non-primary ****/
/*****            [1]      [2]     [3]     [4]                           ****/
/****************************************************************************/

GEN setup(GEN p,GEN f,GEN theta,GEN nut)
{
  GEN b,t1,t2,v1;
  long Lt,Mt,r,s,c,v,av=avma,tetpil;
  
  v=varn(f);
  b=cgetg(5,17);
  
  t1=eleval(f,nut,theta);
  v1=vstar(p,lift(caradj0(gmodulcp(t1,f),v)));
  
  if (typ(v1)==1) 
    {     
      Lt=itos(v1);
      Mt=1;
    }
  else
    {
      Lt=itos((GEN)v1[1]);
      Mt=itos((GEN)v1[2]);
    }
  
  c=cbezout(Lt,-Mt,&r,&s);
  
  while(r<=0)
    {
      r=r+Mt;
      s=s+Lt;
    }
  t2=gdiv(lift(gpuigs(gmodulcp(t1,f),r)),gpuigs(p,s));
  
  b[1]=(long)t1;
  b[2]=(long)t2;
  b[3]=lstoi(Lt);
  b[4]=lstoi(Mt);
  
  tetpil=avma;
  return gerepile(av,tetpil,gcopy((GEN)b));
}

/*****************************************************************************/
/*****************          evaluate g(a)                   ******************/
/*****************************************************************************/

GEN eleval(GEN f,GEN h,GEN a)
{
  long n,k,v=varn(f),av=avma,tetpil;
  GEN g,y;
  
  g=gmul(h,polun[v]);
  n=lgef(g)-3;
  y=gzero;
  for(k=n;k>=0;k--)
    y=gmod(gadd(gmul(y,a),(GEN)g[k+2]),f);
  tetpil=avma;
  return gerepile(av,tetpil,gcopy(y));
}

/****************************************************************************/
/************** Returns [theta,chi,nu ] with theta non-primary **************/
/****************************************************************************/

GEN csrch(GEN p,GEN fa,GEN gamma)
{
  GEN b,h,theta,w,v,r;
  long t,j,vf=varn(fa),av=avma,tetpil;
  
  b=cgetg(5,17);
  
  t=0;
  while (TRUE)
    {
      t++;
      v=stoi(t);
      h=gzero;
      j=0;
      while (!(gcmp0(v)))
	{
	  r=gmod(v,p);
	  v=gdivent(v,p);
	  h=gadd(h,gmul(r,gpuigs(polx[vf],j)));
	  j++;
	}
      theta=gadd(gamma,gmod(h,fa));
      w=factcp(p,fa,theta);
      if (cmpis((GEN)w[4],1)==1)
	{
	  b[1]=un;
	  b[2]=(long)theta;
	  b[3]=w[1];
	  b[4]=w[2];
	  tetpil=avma;
	  return gerepile(av,tetpil,gcopy((GEN)b));
	}
    }
}

/*****************************************************************************/
/*****************          Modular power of an elment           **** ********/
/*****************************************************************************/

GEN eltppm(GEN f,GEN pd,GEN theta,GEN k)
{
  GEN pdd,phi,psi,unmodpdd,q;
  long r,av=avma,tetpil;
  
  pdd=gmul(pd,pd);
  unmodpdd=gmodulcp(gun,pdd);
  phi=pd;
  psi=gmul(pd,theta);
  q=k;
  
  while (cmpis(q,0)!=0)
    { 
      r=q[lgef(q)-1]&1;
      if (r !=0)
	{ 
	  phi=gmod(gdiv(gmul(phi,psi),pd),f);
	  phi=lift(gmul(phi,unmodpdd));
	}
      q=gshift(q,-1);
      if (cmpis(q,0) != 0)
	{
	  psi=gmod(gdiv(gmul(psi,psi),pd),f);
	  psi=lift(gmul(psi,unmodpdd));
	}
    }
  tetpil=avma;
  return gerepile(av,tetpil,gdiv(phi,pd));
}

/****************************************************************************/
/************** Hermite normal form (n x n ) mod p^m            **************/
/****************************************************************************/
 
GEN hnfpm(GEN a,GEN pm)
{
  GEN b,cb,unmodpm,c00,c11,c12,c22,c21,c13,c23,r,q;
  long m,n,j,k,c,i,l,av=avma,tetpil;
  
  unmodpm=gmodulcp(gun,pm);
  m=lg(a)-1;                  /* col  */
  n=lg((GEN)a[1])-1;          /* ligne */
  
  b=cgetg(m+n+1,19);
  
  for(k=1;k<=m;k++)
    {
      b[k]=lgetg(n+1,18);
      for(j=1;j<=n;j++)
	{
	  coeff(b,j,k)=(long)lift(gmul((GEN)coeff(a,j,k),unmodpm));
	}
    }
  for(k=m+1;k<=m+n;k++)
    {
      b[k]=lgetg(n+1,18);
      for(j=1;j<=n;j++)
	{
	  coeff(b,j,k)=((j+m)==(k))? (long)pm:zero;
	}
    }
  k=m+n;
  for(i=n;i>=1;i--)
    {
      for(j=k-1;j>=1;j--)
	if (!(gcmp0((GEN)coeff(b,i,j))))
	  {
	    c00=gbezout((GEN)coeff(b,i,k),(GEN)coeff(b,i,j),&c11,&c12);
	    c21=gdiv(gneg((GEN)coeff(b,i,j)),c00);
	    c22=gdiv((GEN)coeff(b,i,k),c00);
	    
	    for(l=1;l<=n;l++)
	      { 
		c13=gadd(gmul(c11,(GEN)coeff(b,l,k)),gmul(c12,(GEN)coeff(b,l,j)));
		c23=gadd(gmul(c21,(GEN)coeff(b,l,k)),gmul(c22,(GEN)coeff(b,l,j)));
		if (j<=m)
		  { 
		    coeff(b,l,k)=(long)lift(gmul(c13,unmodpm));
		    coeff(b,l,j)=(long)lift(gmul(c23,unmodpm));
		  }
		else
		  {
		    coeff(b,l,k)=(long)c13;
		    coeff(b,l,j)=(long)c23;
		  }
	      }
	  }
      if (cmpis((GEN)coeff(b,i,k),0)==-1)
	for(l=1;l<=n;l++) coeff(b,l,k)=lneg((GEN)coeff(b,l,k));
      for(c=k+1;c<=m+n;c++)
	{
	  q=gdivent((GEN)coeff(b,i,c),(GEN)coeff(b,i,k));
	  if (!(gcmp0(q)))
	    for(l=1;l<=n;l++)
	      coeff(b,l,c)=lsub((GEN)coeff(b,l,c),gmul(q,(GEN)coeff(b,l,k)));
	}
      k--;
    }
  cb=cgetg(n+1,19);
  for(k=m+1;k<=m+n;k++) cb[k-m]=b[k];
  tetpil=avma;
  return gerepile(av,tetpil,gcopy(cb));
}

/****************************************************************************/
/*******        polynomial gcd mod p^m (assumes f1 monic)           ********/
/****************************************************************************/

GEN gcdpm(GEN f1,GEN f2,GEN pm)
{
  long n,c,deg,k,j,v=varn(f1),av=avma,tetpil;
  GEN a,h,unmodpm,b;
  
  
  n=lgef(f1)-3;
  unmodpm=gmodulcp(gun,pm);
  a=cgetg(n+1,19);
  
  h=lift(gmul(gmod(f2,f1),unmodpm));
  for(k=1;k<=n;k++)
    a[k]=lgetg(n+1,18);
  for(j=1;j<=n;j++)
    { deg=lgef(h)-3;
      for(k=1;k<=deg+1;k++)
	coeff(a,k,j)=h[k+1];
      for(k=deg+2;k<=n;k++)
	coeff(a,k,j)=zero;
      
      if (j<n)
	h=lift(gmul(gmod(gmul(polx[v],h),f1),unmodpm));
    }

  a=hnfpm(a,pm);

  c=0;
  
  for(j=n;j>=1;j--)
    if (!(gcmp0(lift(gmul((GEN)coeff(a,j,j),unmodpm)))))
      c=j;
  b=gmul(gzero,polun[v]);
  for(k=1;k<=c;k++)
    b=gadd(b,gmul(gdiv((GEN)coeff(a,k,c),(GEN)coeff(a,c,c)),gpuigs(polx[v],k-1)));
  tetpil=avma;
  return gerepile(av,tetpil,gcopy((GEN)b));
}

/****************************************************************************/
/*******      reduced resultant mod p^m (assumes f1 monic)           ********/
/****************************************************************************/
GEN respm(GEN f1,GEN f2,GEN pm)
{
  long v=varn(f1),av=avma,tetpil;
  GEN a1,a2,pc,g,unmodpm;
 
  unmodpm=gmodulcp(gun,pm);
  g=gbezout(f1,f2,&a1,&a2);

  a1=lift(gmul(gmul(pm,a1),unmodpm));
  a2=lift(gmul(gmul(pm,a2),unmodpm));

  pc=ggcd(pm,content(a1));
  pc=ggcd(pc,content(a2));

  tetpil=avma;

 return gerepile(av,tetpil,gdiv(pm,pc));
}

/****************************************************************************/
/**************  Normalized integral basis                     **************/
/****************************************************************************/

GEN nbasis(GEN ibas,GEN pd)
{
  long n,j,k,m,av=avma,tetpil;
  GEN a,b,h,unmodpd;
  
  unmodpd=gmodulcp(gun,pd);
  n=lg(ibas)-1;
  
  a=cgetg(n+1,19);
  m=lgef((GEN)ibas[1])-2;
  for(k=1;k<=n;k++)
    {
      m=lgef((GEN)ibas[k])-2;
      a[k]=lgetg(n+1,18);
      for(j=1;j<=m;j++)
	coeff(a,j,k)=(long)((GEN)ibas[k])[j+1];
      for(j=m+1;j<=n;j++)
	coeff(a,j,k)=zero;
    }
  
  a=hnfpm(a,pd);
  tetpil=avma;
  return gerepile(av,tetpil,gdiv(a,pd));
}

/****************************************************************************/
/**************                Power basis                     **************/
/****************************************************************************/

GEN pbasis(GEN p,GEN f,long mf,GEN alpha)
{
  long n,j,k,df,dh,v=varn(f),av=avma,tetpil;
  GEN a,b,h,pd;
  
  df=mf/2;
  pd=gpuigs(p,df);
  
  n=lgef(f)-3;
  a=cgetg(n+1,19);
  h=gmul(pd,polun[v]);
  
  for(k=1;k<=n;k++)
    { 
      dh=lgef(h)-3;
      a[k]=lgetg(n+1,18);
      for(j=1;j<=dh+1;j++)
	coeff(a,j,k)=h[j+1];
      for(j=dh+2;j<=n;j++)
	coeff(a,j,k)=zero;
      
      h=gmod(gmul(alpha,h),f);
    }
  
  a=(GEN)hnfpm(a,pd);
  tetpil=avma;
  return gerepile(av,tetpil,gdiv(a,pd));
}

/****************************************************************************/
/**************        Pick best divisor of chi                **************/
/****************************************************************************/

GEN bestnu(GEN w)
{
  long r,j,av=avma,tetpil;
  GEN g,h;
  
  r=lg((GEN)w[1])-1;
  g=polun[0];
  
  for(j=1;j<=r;j++)
    {
      h=(GEN)coeff(w,j,1);
      if (lgef(h)>lgef(g)) g=h;
    }
  tetpil=avma;
  return gerepile(av,tetpil,lift(g));
}

/*****************************************************************************/
/******                        bezout etendu                           *******/
/******                  Return d=pgcd(a,b)  and  &u,&v                *******/
/*****************************************************************************/

long cbezout(long a,long b,long *u,long *v)
{
  long d,v3,v1,q,r,t;
  
  (*u)=1;
  d=a;
  v1=0;
  v3=b;
  
  while (1)
    {
      if (v3==0) 
	{ 
	  (*v)=(d-a*(*u))/b;
	  return d;
	}
      q=d/v3;
      r=d%v3;
      t=(*u)-v1*q;
      (*u)=v1;
      d=labs(v3);
      v1=t;
      v3=r;
    }
}

long clcm(long a,long b)
{
  long d,r,v1,q;
  
  d=a;
  r=b;

  while (1)
    {
      if (r==0) 
	{ 
	  return (a*b)/d;
	}
      v1=r;
      q=d/r;
      r=d%r;
      d=labs(v1);
    }
}

GEN idealfactor(GEN nf, GEN x)
{
  long av=avma,tetpil,i,j,k,lf,N,ls,v,vd;
  GEN d,f,f1,f2,y1,y2,y,p1,p2,denx;

  if((typ(nf)!=17)||(lg(nf)<10)) err(idealer1);
  if(typ(x)<=9) x=principalideal(nf,x);
  if((typ(x)==17)&&(lg(x)==3)) x=(GEN)x[1];
  if(typ(x)!=19) err(idealer2);
  N=lgef((GEN)nf[1])-3;if(lg(x)!=(N+1)) x=idealmul(nf,x,idmat(N));
  denx=denom(x);if(!gcmp1(denx)) x=gmul(denx,x);
  for(d=gun,i=1;i<=N;i++) d=mulii(d,gcoeff(x,i,i));
  f=factor(d);f1=(GEN)f[1];f2=(GEN)f[2];lf=lg(f1);
  y1=cgetg((lf-1)*N+1,18);y2=cgeti((lf-1)*N+1);k=0;
  for(i=1;i<lf;i++)
    {
      p1=primedec(nf,(GEN)f1[i]);ls=itos((GEN)f2[i]);
      vd=ggval(denx,(GEN)f1[i]);
      for(j=1;j<lg(p1);j++)
	{
	  if(ls)
	    {
	      v=idealval(nf,x,p2=(GEN)p1[j]);
	      ls-=(v*itos((GEN)p2[4]));
	      v-=vd*itos((GEN)p2[3]);
	    }
	  else v=-vd*itos((GEN)p2[3]);
	  if(v) {y1[++k]=(long)p2;y2[k]=v;}
	}
    }
  tetpil=avma;
  y=cgetg(3,19);p1=cgetg(k+1,18);p2=cgetg(k+1,18);y[1]=(long)p1;y[2]=(long)p2;
  for(i=1;i<=k;i++) {p1[i]=lcopy((GEN)y1[i]);p2[i]=lstoi(y2[i]);}
  return gerepile(av,tetpil,y);
}

GEN idealadd(GEN nf, GEN x, GEN y)
{
  long av=avma,tetpil,N=lgef((GEN)nf[1])-3;
  GEN z;

  if((typ(nf)!=17)||(lg(nf)<10)) err(idealer1);
  if(typ(x)<=9) x=principalideal(nf,x);
  if(typ(y)<=9) y=principalideal(nf,y);
  if((typ(x)==17)&&(lg(x)==6)) x=idealmulprime(nf,idmat(N),x);
  if((typ(y)==17)&&(lg(y)==6)) y=idealmulprime(nf,idmat(N),y);
  if((typ(x)==17)&&(lg(x)==3)) x=(GEN)x[1];
  if((typ(y)==17)&&(lg(y)==3)) y=(GEN)y[1];
  if((typ(x)!=19)||(typ(y)!=19)) err(idealer2);
  z=concat(x,y);tetpil=avma;return gerepile(av,tetpil,hnf(z));
}

GEN idealdiv(GEN nf, GEN x, GEN y)
{
  long av=avma,tetpil;
  GEN z;

  z=idealinv(nf,y);tetpil=avma;return gerepile(av,tetpil,idealmul(nf,x,z));
}

GEN idealintersect(GEN nf, GEN x, GEN y)
{
  long av=avma,tetpil,lz,i,j,N;
  GEN z,p1,p2;

  if((typ(nf)!=17)||(lg(nf)<10)) err(idealer1);
  if(typ(x)<=9) x=principalideal(nf,x);
  if(typ(y)<=9) y=principalideal(nf,y);
  N=lgef((GEN)nf[1])-3;
  if((typ(x)==17)&&(lg(x)==6)) x=idealmulprime(nf,idmat(N),x);
  if((typ(y)==17)&&(lg(y)==6)) y=idealmulprime(nf,idmat(N),y);
  if((typ(x)==17)&&(lg(x)==3)) x=(GEN)x[1];
  if((typ(y)==17)&&(lg(y)==3)) y=(GEN)y[1];
  if((typ(x)!=19)||(typ(y)!=19)) err(idealer2);
  z=kerint(concat(x,y));
  if(lg(x)!=(N+1)) err(talker,"ideal not in HNF in ideal-related function");
  lz=lg(z);p1=cgetg(lz,19);
  for(j=1;j<lz;j++)
    {
      p2=cgetg(N+1,18);p1[j]=(long)p2;
      for(i=1;i<=N;i++) p2[i]=coeff(z,i,j);
    }
  p2=gmul(x,p1);tetpil=avma;return gerepile(av,tetpil,hnf(p2));
}

GEN principalideal(GEN nf, GEN a)  
{
  long av,tetpil,N,i;
  GEN y,z;

  if((typ(nf)!=17)||(lg(nf)<10)) err(idealer1);
  N=lgef((GEN)nf[1])-3;
  switch(typ(a))
    {
    case 1: case 4: case 5: z=cgetg(2,19);y=cgetg(N+1,18);z[1]=(long)y;
      y[1]=lcopy(a);for(i=2;i<=N;i++) y[i]=zero;
      return z;
    case 9: if(!gegal((GEN)nf[1],(GEN)a[1])) 
      err(talker,"incompatible number fields in principalideal");
      a=(GEN)a[2];   /* fall through */
    case 10: av=avma;z=cgetg(N+1,18);
      for(i=1;i<=N;i++) z[i]=(long)truecoeff(a,i-1);
      tetpil=avma;y=gmul((GEN)nf[8],z);z=cgetg(2,19);z[1]=(long)y;
      return gerepile(av,tetpil,z);
    default: err(talker,"incorrect type in principalideal");
      return gnil;
    }
}

GEN principalidele(GEN nf, GEN a)  
{
  long av,tetpil,N,RU,R1,R2,i;
  GEN y,z,res,arc,p1;

  if((typ(nf)!=17)||(lg(nf)<10)) err(idealer1);
  N=lgef((GEN)nf[1])-3;y=(GEN)nf[2];R1=itos((GEN)y[1]);R2=itos((GEN)y[2]);
  RU=R1+R2;
  switch(typ(a))
    {
    case 1: case 4: case 5: z=cgetg(2,19);y=cgetg(N+1,18);z[1]=(long)y;
      y[1]=lcopy(a);for(i=2;i<=N;i++) y[i]=zero;
      arc=cgetg(RU+1,18);for(i=1;i<=RU;i++) arc[i]=zero;
      res=cgetg(3,17);res[1]=(long)z;res[2]=(long)arc;return res;
    case 9: if(!gegal((GEN)nf[1],(GEN)a[1])) 
      err(talker,"incompatible number fields in principalidele");
      a=(GEN)a[2];   /* fall through */
    case 10: av=avma;z=cgetg(N+1,18);
      for(i=1;i<=N;i++) z[i]=(long)truecoeff(a,i-1);
      y=gmul((GEN)nf[8],z);p1=gmul((GEN)((GEN)nf[5])[1],y);arc=cgetg(RU+1,18);
      for(i=1;i<=R1;i++) arc[i]=(long)glog((GEN)p1[i],6);
      for(i=R1+1;i<=RU;i++) arc[i]=lmul2n(glog((GEN)p1[i],6),1);
      tetpil=avma;z=cgetg(2,19);z[1]=lcopy(y);
      res=cgetg(3,17);res[1]=(long)z;res[2]=lcopy(arc);
      return gerepile(av,tetpil,res);
    default: err(talker,"incorrect type in principalidele");
      return gnil;
    }
}
