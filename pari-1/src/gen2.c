/********************************************************************/
/********************************************************************/
/**                                                                **/
/**                +++++++++++++++++++++++++++++++                 **/
/**                +                             +                 **/
/**                +    OPERATIONS GENERIQUES    +                 **/
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

/*******************************************************************/
/*******************************************************************/
/*                                                                 */
/*                 LISTE DES TYPES GENERIQUES                      */
/*                 ~~~~~~~~~~~~~~~~~~~~~~~~~~                      */
/*                                                                 */
/*  1  :entier long     [ cod1 ] [ cod2 ] [ man1 ] ... [ manl ]    */
/*  2  :reel            [ cod1 ] [ cod2 ] [ man1 ] ... [ manl ]    */
/*  3  :entier modulo   [ code ] [ mod  ] [ entier modulo ]        */
/*  4  :fraction        [ code ] [ num. ] [ den. ]                 */
/*  5  :nfraction       [ code ] [ num. ] [ den. ]                 */
/*  6  :complexe        [ code ] [ reel ] [ imag ]                 */
/*  7  :p-adique        [ cod1 ] [ cod2 ] [ p ] [ p^r ] [ entier]  */
/*  8  :quadrat         [ cod1 ] [ mod  ] [ reel ] [ imag ]        */
/*  9  :poly mod        [ code ] [ mod  ] [ polynome  mod ]        */
/* --------------------------------------------------------------- */
/*  10 :polynome        [ cod1 ] [ cod2 ] [ man1 ] ... [ manl ]    */
/*  11 :serie           [ cod1 ] [ cod2 ] [ man1 ] ... [ manl ]    */
/*  13 :fr.rat          [ code ] [ num. ] [ den. ]                 */
/*  14 :n.fr.rat        [ code ] [ num. ] [ den. ]                 */
/*  15 :formqre         [ code ] [  a  ] [  b  ] [  c  ] [ del ]   */
/*  16 :formqim         [ code ] [  a   ] [  b   ] [  c   ]        */
/*  17 :vecteur ligne   [ code ] [  x1  ] ... [  xl  ]             */
/*  18 :vecteur colonne [ code ] [  x1  ] ... [  xl  ]             */
/*  19 :matrice         [ code ] [ col1 ] ... [ coll ]             */
/*                                                                 */
/*******************************************************************/
/*******************************************************************/

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*                                                                 */
/*                    OPERATIONS PAR VALEUR                        */
/*                                                                 */
/*      parametres : f pointe sur la fonction appelee;             */
/*                 x,y,... ,z pointent sur des GEN;                */
/*                 le dernier parametre recoit le resultat de      */
/*                 l'operation .                                   */
/*                                                                 */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifdef __cplusplus
void gop0z(GEN f (void), GEN x)
#else
void gop0z(GEN (*f) (void), GEN x)
#endif
{
  long    avmacourant;
  GEN   p1;
  
  avmacourant=avma;
  p1=(*f)();gaffect(p1,x);
  avma=avmacourant;
}


/* operation a un parametre   */

#ifdef __cplusplus
void gop1z(GEN f (GEN), GEN x, GEN y)
#else
void gop1z(GEN (*f) (GEN), GEN x, GEN y)
#endif
{
  long    avmacourant;
  GEN     p1;
  
  avmacourant=avma;p1=(*f)(x);gaffect(p1,y);
  avma=avmacourant;
}


/* operation a deux parametres */

#ifdef __cplusplus
void gop2z(GEN f (GEN, GEN), GEN x, GEN y, GEN z)
#else
void gop2z(GEN (*f) (GEN, GEN), GEN x, GEN y, GEN z)
#endif
{
  long    avmacourant;
  GEN     p1;
  
  avmacourant=avma;p1=(*f)(x,y);gaffect(p1,z);
  avma=avmacourant;
}

#ifdef __cplusplus
void gops2gsz(GEN f (GEN, long), GEN x, long s, GEN z)
#else
void gops2gsz(GEN (*f) (GEN, long), GEN x, long s, GEN z)
#endif
{
  long    avmacourant;
  GEN     p1;
  
  avmacourant=avma;p1=(*f)(x,s);gaffect(p1,z);
  avma=avmacourant;
}


#ifdef __cplusplus
void gops2sgz(GEN f (long, GEN), long s, GEN y, GEN z)
#else
void gops2sgz(GEN (*f) (long, GEN), long s, GEN y, GEN z)
#endif
{
  long    avmacourant;
  GEN     p1;
  
  avmacourant=avma;p1=(*f)(s,y);gaffect(p1,z);
  avma=avmacourant;
}


#ifdef __cplusplus
void gops2ssz(GEN f (long, long), long s, long y, GEN z)
#else
void gops2ssz(GEN (*f) (long, long), long s, long y, GEN z)
#endif
{
  long    avmacourant;
  GEN     p1;
  
  avmacourant=avma;p1=(*f)(s,y);gaffect(p1,z);
  avma=avmacourant;
}

/* operation a trois parametres */

#ifdef __cplusplus
void gop3z(GEN f (GEN, GEN, GEN), GEN x, GEN y, GEN z, GEN t)
#else
void gop3z(GEN (*f) (GEN, GEN, GEN), GEN x, GEN y, GEN z, GEN t)
#endif
{
  long    avmacourant;
  GEN     p1;
  
  avmacourant=avma;p1=(*f)(x,y,z);gaffect(p1,t);
  avma=avmacourant;
}


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*                                                                 */
/*            OPERATIONS AVEC DES ENTIERS COURTS                   */
/*                                                                 */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


#ifdef __cplusplus
GEN     gopsg2(GEN f (GEN, GEN), long s, GEN y)
#else
GEN     gopsg2(GEN (*f) (GEN, GEN), long s, GEN y)
#endif
{
  long court[3];
  court[0] = evaltyp(1)+evalpere(1)+evallg(3);
  affsi(s,court);return (*f)((GEN)court,y);
}

#ifdef __cplusplus
GEN     gopgs2(GEN f (GEN, GEN), GEN y, long s)
#else
GEN     gopgs2(GEN (*f) (GEN, GEN), GEN y, long s)
#endif
{
  long court[3];
  court[0] = evaltyp(1)+evalpere(1)+evallg(3);
  affsi(s,court);return (*f)(y,(GEN)court);
}

#ifdef __cplusplus
long opgs2(int f (GEN, GEN), GEN y, long s)
#else
long opgs2(int (*f) (GEN, GEN), GEN y, long s)
#endif
{
  long court[3];
  court[0] = evaltyp(1)+evalpere(1)+evallg(3);
  affsi(s,court);return (*f)(y,(GEN)court);
}

#ifdef __cplusplus
void gops1z(GEN f (long), long s, GEN y)
#else
void gops1z(GEN (*f) (long), long s, GEN y)
#endif
{
  long av=avma; gaffect((*f)(s),y); avma=av;
}

#ifdef __cplusplus
void     gopsg2z(GEN f (GEN, GEN), long s, GEN y, GEN z)
#else
void     gopsg2z(GEN (*f) (GEN, GEN), long s, GEN y, GEN z)
#endif
{
  long av, court[3];
  court[0] = evaltyp(1)+evalpere(1)+evallg(3);
  affsi(s,court);av=avma;
  gaffect((*f)((GEN)court,y),z);avma=av;
}

#ifdef __cplusplus
void     gopgs2z(GEN f (GEN, GEN), GEN y, long s, GEN z)
#else
void     gopgs2z(GEN (*f) (GEN, GEN), GEN y, long s, GEN z)
#endif
{
  long av, court[3];
  court[0] = evaltyp(1)+evalpere(1)+evallg(3);
  affsi(s,court);av=avma;
  gaffect((*f)(y,(GEN)court),z);avma=av;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*                                                                 */
/*            CREATION D'UN P-ADIQUE                               */
/*                                                                 */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


GEN cgetp(GEN x)
{
  GEN y;
  
  y=cgetg(5,7);y[1]=evalprecp(precp(x));
  y[2]=(long)copyifstack((GEN)x[2]);y[3]=lcopy((GEN)x[3]);
  y[4]=lgeti(lg((GEN)x[3]));return y;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*                                                                 */
/*                       CLONE ET COPIE                            */
/*                                                                 */
/*            Cree la replique exacte d'un GEN existant            */
/*                                                                 */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

GEN     gcopy(GEN x)
{
  long tx=typ(x),lx=lg(x),i;
  GEN  y;
  
  y=cgetg(lx,tx);
  if(tx<=2)
    for(i=1;i<lx;i++) y[i]=x[i];
  else
  {
    if(tx==10) lx=lgef(x);
    if((tx==11)&&gcmp0(x)) lx=2;
    for(i=1;i<lontyp[tx];i++)
      y[i]=x[i];
    for(i=lontyp[tx];i<lontyp2[tx];i++)
      y[i]=copyifstack((GEN)x[i]);
    for(i=lontyp2[tx];i<lx;i++)
      y[i]=lcopy((GEN)x[i]);
  }
  return y;
}

GEN     forcecopy(GEN x)
{
  long tx=typ(x),lx=lg(x),i;
  GEN  y;
  
  y=cgetg(lx,tx);
  if(tx<=2)
    for(i=1;i<lx;i++) y[i]=x[i];
  else
  {
    if(tx==10) lx=lgef(x);
    if((tx==11)&&gcmp0(x)) lx=2;
    for(i=1;i<lontyp[tx];i++)
      y[i]=x[i];
    for(i=lontyp[tx];i<lontyp2[tx];i++)
      y[i]=(long)forcecopy((GEN)x[i]);
    for(i=lontyp2[tx];i<lx;i++)
      y[i]=(long)forcecopy((GEN)x[i]);
  }
  return y;
}

long taille(GEN x)
{
  long i, n, lx = lg(x), tx = typ(x);
  if (tx <= 2) return (tx == 1) ? lgef(x) : lx;
  if ((tx == 11) && gcmp0(x)) return 2;
  if (tx == 10) lx = lgef(x);
  n = lx;
  for(i = lontyp[tx]; i < lx; i++) n += taille((GEN)x[i]);
  return n;
}

long taille2(GEN x) {return (taille(x)<<TWOPOTBYTES_IN_LONG);}

GEN brutcopy(GEN x, GEN y)
{
  long i, lx, tx = typ(x);
  GEN z;
  lx = ((tx == 1) || (tx == 10)) ? lgef(x) : lg(x);
  if (tx <= 2)
    for(i = 0; i < lx; i++) y[i] = x[i];
  else
  {
    if ((tx == 11) && gcmp0(x)) lx = 2;
    z = y + lx;
    for(i = 0; i < lontyp[tx]; i++) y[i] = x[i];
    for(i = lontyp[tx]; i < lx; i++)
    {
      y[i] = (long)brutcopy((GEN)x[i], z);
      z += taille((GEN)x[i]);
    }
  }
  setlg(y,lx);
  return y;
}

GEN gclone(GEN x)
{
  GEN y = newbloc(taille(x));
  return brutcopy(x, y);
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*                                                                 */
/*                            GREFFE                               */
/*                                                                 */
/*            Greffe d'une serie sur un polynome                   */
/*                                                                 */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


GEN     greffe(GEN x, long l)
{
  long    i,e,k,vx;
  GEN     y;
  
  if (typ(x)!=10) err(grefer1);
  else
  {
    y=cgetg(l,11);
    if (gcmp0(x))
    {
      y[1]=evalvalp(l-2)+evalvarn(varn(x));
      for (i=2;i<l;y[i]=x[2],i++);
    }
    else
    {
      e=gval(x,vx=varn(x));y[1]=evalsigne(1)+evalvalp(e)+evalvarn(vx);
      k=lgef(x)-e-1;
      if (k<l)
      {
        for (i=2;i<=k;y[i]=x[i+e],i++);
        for (i=k+1;i<l;y[i]=zero,i++);
      }
      else
        for (i=2;i<l;y[i]=x[i+e],i++);
    }
  }
  return y;
}


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*                                                                 */
/*            CONVERSION GEN-->LONG DU C                           */
/*                                                                 */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

long    gtolong(GEN x)
{
  long    y,tx,av;
  
  tx=typ(x);av=avma;
  
  switch(tx)
    {
    case 1 : y=itos(x);break;
    case 2 : 
    case 4 :
    case 5 : y=itos(ground(x));break;
    case 6 : if (gcmp0((GEN)x[2])) y=gtolong((GEN)x[1]);
    else err(gtolger);
      break;
    case 8 : if (gcmp0((GEN)x[3])) y=gtolong((GEN)x[2]);
    else err(gtolger);
      break;
    default: err(gtolger);
  }
  avma=av;
  return y;
}


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*                                                                 */
/*                    COMPARAISON  A ZERO                          */
/*                                                                 */
/*        x pointe sur un GEN;la fonction renvoie 1 si le          */
/*                   GEN est nul,0 sinon .                         */
/*                                                                 */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


int gcmp0(GEN x)
{
  long  l,i;
  
  switch(typ(x))
  {
    case 1 :
    case 2 :
    case 10:
    case 11: return !signe(x);
      
    case 3 :
    case 9 : return gcmp0((GEN)x[2]);
      
    case 4 :
    case 5 : return !signe((GEN)x[1]);
    case 13:
    case 14: return gcmp0((GEN)x[1]);
      
    case 6 : return gcmp0((GEN)x[1])&&gcmp0((GEN)x[2]);
      
    case 8 : return gcmp0((GEN)x[2])&&gcmp0((GEN)x[3]);
      
    case 7 : return !signe((GEN)x[4]);
      
    case 15:
    case 16: 
    case 17:
    case 18:
    case 19: l=lg(x);i=1;
      while ((i<l)&&gcmp0((GEN)x[i])) i++;
      return i==l;
      
    default: err(gcmper);return 0;
  }
}


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*                                                                 */
/*            COMPARAISONS  A     1  et -1                         */
/*                                                                 */
/*          x pointe sur un GEN;la fonction renvoie 1 si le        */
/*          GEN est l'entier 1 (resp.-1),0 sinon .                 */
/*                                                                 */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


int gcmp1(GEN x)
{
  
  long    typy=typ(x),l,y;
  GEN     p1;
  
  switch(typy)
  {
    case 1 : return (lgef(x)==3)&&(signe(x)==1)&&(mant(x,1)==1);
    case 2 : l=avma;p1=subrs(x,1);
      y=(!signe(p1));avma=l;
      return y;
    case 3 :
    case 9 : return gcmp1((GEN)x[2]);
    case 4 : return gcmp1((GEN)x[1])&&gcmp1((GEN)x[2]);
    case 5 : return !(cmpii((GEN)x[1],(GEN)x[2]));
    case 6 : return gcmp1((GEN)x[1])&&gcmp0((GEN)x[2]);
    case 8 : return gcmp1((GEN)x[2])&&gcmp0((GEN)x[3]);
    case 7 : return !valp(x)&&gcmp1((GEN)x[4]);
    case 10 : return (lgef(x)==3)&&gcmp1((GEN)x[2]);
    default: return 0;
  }
}


int gcmp_1(GEN x)
{
  
  long    typy=typ(x),l,y;
  GEN     p1;
  
  switch(typy)
  {
    case 1 : return (lgef(x)==3)&&(signe(x)== -1)&&(mant(x,1)==1);
    case 2 : l=avma;p1=addsr(1,x);
      y=(!signe(p1));avma=l;
      return y;
    case 3 : l=avma;p1=addsi(1,(GEN)x[2]);
      y=cmpii(p1,(GEN)x[1]);avma=l;
      return !y;
    case 4 :
    case 5 : l=avma;p1=addii((GEN)x[1],(GEN)x[2]);
      y=signe(p1);avma=l;
      return !y;
    case 6 : return gcmp_1((GEN)x[1])&&gcmp0((GEN)x[2]);
    case 8 : return gcmp_1((GEN)x[2])&&gcmp0((GEN)x[3]);
    case 7 : l=avma;p1=addsi(1,(GEN)x[4]);
      y=cmpii(p1,(GEN)x[3]);avma=l;
      return !y;
    case 9 : l=avma;p1=gaddsg(1,(GEN)x[2]);
      y=(!gegal(p1,(GEN)x[1]))&&(signe(p1));avma=l;
      return !y;
    case 10 : return (lgef(x)==3)&&gcmp_1((GEN)x[2]);
    default: return 0;
    }
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*                                                                 */
/*                       COMPARAISON SIGNEE                        */
/*                                                                 */
/*            rend le signe de x-y si ceci a un sens,              */
/*            sinon erreur.                                        */
/*                                                                 */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


int   gcmp(GEN x, GEN y)
{
  long  tx,ty,f,avmacourant;
  
  tx=typ(x);ty=typ(y);
  if ((tx>5)||(tx==3)||(ty>5)||(ty==3)) err(gcmper);
  if((tx<=2)&&(ty<=2)) return mpcmp(x,y);
  else
  {
    avmacourant=avma;f=gsigne(gsub(x,y));
    avma=avmacourant;return f;
  }
}

int lexcmp(GEN x, GEN y)
{
  long tx=typ(x),ty=typ(y),lx,ly,fl,s,i;
  GEN p1;

  if(tx>ty) {p1=x;x=y;y=p1;fl=tx;tx=ty;ty=fl;s= -1;} else s=1;
  ly=lg(y);
  if(tx<17)
    {
      if(ty<17) return s*gcmp(x,y);
      if(ly==1) return s;
      fl=lexcmp(x,(GEN)y[1]);if(fl) return s*fl;
      return (ly>2)?-s:0;
    }
  lx=lg(x);
  if(ly==1) return (lx==1)?0:s;
  if(lx==1) return -s;
  if((ty==19)&&(tx<19))
    {
      fl=lexcmp(x,(GEN)y[1]);if(fl) return s*fl;
      return (ly>2)?-s:0;
    }
  if(lx>ly) {p1=x;x=y;y=p1;fl=lx;lx=ly;ly=fl;s= -s;}
  fl=0;for(i=1;(i<lx)&&(!fl);i++) fl=lexcmp((GEN)x[i],(GEN)y[i]);
  if(fl) return s*fl;
  return (ly>lx)?-s:0;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*                                                                 */
/*                            EGALITE                              */
/*                                                                 */
/*            Renvoie 1 si x=y, 0 sinon                            */
/*                                                                 */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


int gegal(GEN x, GEN y)
{
  long    avmacourant,f,i,tx=typ(x),ty=typ(y),lx;
  ulong   masq=SIGNBITS+LGEFBITS;
  GEN     p1;
  
  if(tx!=ty)
  {
    avmacourant=avma;p1=gsub(x,y);f=gcmp0(p1);avma=avmacourant;
  }
  else
  {
    switch(tx)
    {
      case 1: if((((ulong)x[1])&masq)!=(((ulong)y[1])&masq)) return 0;
        i=2;lx=lgef(x);while((i<lx)&&(x[i]==y[i])) i++;return i==lx;
      case 10: if(x[1]!=y[1]) return 0;
        i=2;lx=lgef(x);while((i<lx)&&(gegal((GEN)x[i],(GEN)y[i]))) i++;return i==lx;
      case 6: return gegal((GEN)x[1],(GEN)y[1])&&gegal((GEN)x[2],(GEN)y[2]);
      case 3:
      case 9: return (x[1]==y[1])?gegal((GEN)x[2],(GEN)y[2]):gegal((GEN)x[1],(GEN)y[1])&&gegal((GEN)x[2],(GEN)y[2]);
      case 8: return gegal((GEN)x[1],(GEN)y[1])&&gegal((GEN)x[2],(GEN)y[2])&&gegal((GEN)x[3],(GEN)y[3]);
      case 4:
      case 5:
      case 13:
      case 14: avmacourant=avma;f=gegal(gmul((GEN)x[1],(GEN)y[2]),gmul((GEN)x[2],(GEN)y[1]));
	avma=avmacourant;return f;
      case 15: return gegal((GEN)x[1],(GEN)y[1])&&gegal((GEN)x[2],(GEN)y[2])&&gegal((GEN)x[3],(GEN)y[3])&&gegal((GEN)x[4],(GEN)y[4]);
      case 16: return gegal((GEN)x[1],(GEN)y[1])&&gegal((GEN)x[2],(GEN)y[2])&&gegal((GEN)x[3],(GEN)y[3]);
      default: avmacourant=avma;p1=gsub(x,y);f=gcmp0(p1);avma=avmacourant;
    }
  }
  return f;
}

int polegal(GEN x, GEN y)
     
                
     
     /* a usage interne donc pas de verification de types */
     
{
  long i;
  
  if (x[1] != y[1]) return 0;
  for(i = lgef(x) - 1; i > 1; i--) if (!gegal((GEN)x[i],(GEN)y[i])) return 0;
  return 1;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*                                                                 */
/*                          VALUATION                              */
/*                                                                 */
/*   renvoie le plus grand exposant de p divisant x quand cela a   */
/*   un sens (erreur pour les types reels, integermod et polymod   */
/*   si le module n'est pas divisible par p, et q-adiques pour     */
/*   q!=p. p doit etre de type entier ou polynome.                 */
/*                                                                 */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

long ggval(GEN x, GEN p)
{
  long  tx=typ(x),tp=typ(p),lx,i,j,vx,v,av=avma,val,limite=(avma+bot)/2,tetpil;
  GEN  p1,p2,reste;
  
  if(isexactzero(x)) err(gvaler2);
  if((tx<9)&&(tp==10)) return 0;
  switch(tx)
    {
    case 1: if(tp!=1) err(gvaler4);
      val=0;
      while (1)
	{
	  p1=dvmdii(x,p,&reste);
	  if (signe(reste)) break;
	  val++;x=p1;
          if (avma<limite) {tetpil=avma;x=gerepile(av,tetpil,gcopy(x));}
	}
      break;
    case 3: p1=cgeti(lgef((GEN)x[1]));
      if((tp!=1)||(!mpdivis((GEN)x[1],p,p1))) err(gvaler4);
      p2=cgeti(lgef((GEN)x[2]));
      if(mpdivis((GEN)x[2],p,p2))
	{val=1;while(mpdivis(p1,p,p1)&&mpdivis(p2,p,p2)) val++;}
      else val=0;
      break;
    case 7: if((tp!=1)||(!gegal(p,(GEN)x[2]))) err(gvaler4);
      val=valp(x);break;
    case 9: if(tp==1) val=ggval((GEN)x[2],p);
      else
	{
	  if((tp!=10)||(!poldivis((GEN)x[1],p,&p1))) err(gvaler4);
	  if(poldivis((GEN)x[2],p,&p2))
	    {val=1;while(poldivis(p1,p,&p1)&&poldivis(p2,p,&p2)) val++;}
	  else val=0;
	}
      break;
    case 10: i=2;while (isexactzero((GEN)x[i])) i++;
      if(tp==10)
	{
	  v=varn(p);vx=varn(x);if(vx>v) return 0;
	  if(vx==v)
	    {
	      if(ismonome(p)) val=i-2;
	      else
		{
		  val=0;
		  while (1)
		    {
		      p1=poldivres(x,p,&reste);
		      if (signe(reste)) break;
		      val++;x=p1;
                      if (avma<limite) {tetpil=avma;x=gerepile(av,tetpil,gcopy(x));}
		    }
		}
	    }
	  else
	    {
	      val=ggval((GEN)x[i],p);
	      for(j=i+1;j<lgef(x);j++)
		if(!gcmp0((GEN)x[j])) val=min(val,ggval((GEN)x[j],p));
	    }
	}
      else 
	{
	  if(tp!=1) err(gvaler4);
	  val=ggval((GEN)x[i],p);
	  for(j=i+1;j<lgef(x);j++)
	    if(!gcmp0((GEN)x[j])) val=min(val,ggval((GEN)x[j],p));
	}
      break;
      
    case 11: if((tp!=10)&&(tp!=11)&&(tp!=1)) err(gvaler4);
      v=gvar(p);vx=varn(x);if(vx>v) return 0;
      if(vx==v) return (long)(valp(x)/ggval(p,polx[v]));
      val=ggval((GEN)x[2],p);
      for(j=3;j<lg(x);j++)
        if(!gcmp0((GEN)x[j])) val=min(val,ggval((GEN)x[j],p));
      break;
    case 4:
    case 5:
    case 13:
    case 14: val=ggval((GEN)x[1],p)-ggval((GEN)x[2],p);break;
      
    case 2:
    case 15:
    case 16: err(gvaler4);
    case 6:
    case 8:
    case 17:
    case 18:
    case 19: val=VERYBIGINT;lx=lg(x);
      for(j=1;j<lx;j++) val=min(val,ggval((GEN)x[j],p));
      break;
    default: err(gvaler4);
  }
  avma=av;return val;
}

long pvaluation(GEN x, GEN p, GEN *py)
{
  long l,tetpil,val,limite = (bot + avma) / 2;
  GEN  p1,reste;
  
  val=0;*py=cgeti(lgef(x));l=avma;
  while (1)
  {
    p1=dvmdii(x,p,&reste);
    if (signe(reste)) break;
    val++;x=p1;
    if (avma < limite)
      {
        tetpil = avma;
        x = gerepile(l, tetpil, gcopy(x));
      }
  }
  affii(x,*py);avma=l;
  return val;
}


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*                                                                 */
/*                            NEGATION                             */
/*                                                                 */
/*            Cree-x,ou x pointe sur un GEN .                      */
/*                                                                 */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


GEN     gneg(GEN x)
{
  long  tx=typ(x),lx,i,sx;
  GEN   y;
  
  if (gcmp0(x))
  {
    y=(tx==1)?gzero:gcopy(x);
  }
  else
  {
    lx=lg(x);
    
    switch(tx)
    {
      case 1 : 
      case 2 : y=gcopy(x);
        sx=signe(x);
        sx= -sx;
        setsigne(y,sx);
        break;
        
      case 3 : y=cgetg(3,tx);y[1]=copyifstack((GEN)x[1]);
        if(gcmp0((GEN)x[2])) y[2]=zero;
        else
          y[2]=lsubii((GEN)y[1],(GEN)x[2]);
        break;
        
      case 9: y=cgetg(3,tx);y[1]=copyifstack((GEN)x[1]);
        y[2]=lneg((GEN)x[2]);
        break;
        
      case 4 :
      case 5 :
      case 13:
      case 14: y=cgetg(3,tx);y[1]=lneg((GEN)x[1]);y[2]=lcopy((GEN)x[2]);
        break;
        
      case 7 : y=cgetp(x);
        if(gcmp0((GEN)x[4])) {affsi(0,(GEN)y[4]);y[1]=x[1];}
        else
          {
            subiiz((GEN)y[3],(GEN)x[4],(GEN)y[4]);
            setvalp(y,valp(x));
          }
        break;

      case 8 : y=cgetg(lx,tx);
        for (i=2;i<lx;i++) y[i]=lneg((GEN)x[i]);
        y[1]=copyifstack((GEN)x[1]);
        break;
        
      case 6 :
      case 17:
      case 18:
      case 19: y=cgetg(lx,tx);
        for (i=1;i<lx;i++) y[i]=lneg((GEN)x[i]);
        break;

      case 15:
      case 16: err(gneger);
        
      case 10: lx=lgef(x);
        y=cgetg(lx,tx);
        for (i=2;i<lx;i++) y[i]=lneg((GEN)x[i]);
        y[1]=x[1];
        break;
        
      case 11: y=cgetg(lx,tx);
        for (i=2;i<lx;i++) y[i]=lneg((GEN)x[i]);
        y[1]=x[1];
    }
  }
  return y;
}


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*                                                                 */
/*                        VALEUR ABSOLUE                           */
/*                                                                 */
/*            Cree un GEN pointant sur la valeur absolue de x      */
/*            a condition que x soit un entier,ou un reel,         */
/*            ou une fraction,ou un complexe;sinon erreur .        */
/*                                                                 */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifdef __cplusplus
GEN     gabs(GEN x, long prec = 0)
#else
GEN     gabs(GEN x, ...)
#endif
{
  long  tx=typ(x),lx=lg(x),i,l,tetpil;
  GEN   y,p1;
  
  switch(tx)
  {
    case 1 :
    case 2 : y=mpabs(x);break;
      
    case 4 :
    case 5 : y=cgetg(lx,tx);
      y[1]=lmpabs((GEN)x[1]);
      y[2]=lmpabs((GEN)x[2]);
      break;
      
    case 6 : l=avma;p1=gnorm(x);tetpil=avma;
      y=gsqrt(p1,prec);y=gerepile(l,tetpil,y);
      break;
      
    case 8 : l=avma;p1=cgetr(prec);affsr(1,p1);
    p1=gmul(x,p1);tetpil=avma;
    y=gerepile(l,tetpil,gabs(p1,prec));break;
        
    case 17:
    case 18:
    case 19: y=cgetg(lx,tx);
      for(i=1;i<lx;i++)
        y[i]=(long)gabs((GEN)x[i],prec);
      break;
      
    default: err(gabser);
  }
  return y;
}

GEN     gmax(GEN x, GEN y)
{
  return (gcmp(x,y)>=0) ? gcopy(x) : gcopy(y);
}

GEN     gmin(GEN x, GEN y)
{
  return (gcmp(x,y)<=0) ? gcopy(x) : gcopy(y);
}

GEN vecmax(GEN x)
{
  long tx=typ(x),lx,lx2,i,j;
  GEN s,p1;

  if(tx<17) return gcopy(x);
  lx=lg(x);s=gzero;
  if(tx<19)
    {
      for(i=1;i<lx;i++) if(gcmp((GEN)x[i],s)>0) s=(GEN)x[i];
     }
  else
    {
      if(lx>1) lx2=lg((GEN)x[1]);
      for(j=1;j<lx;j++)
	{
	  p1=(GEN)x[j];
	  for(i=1;i<lx2;i++) if(gcmp((GEN)p1[i],s)>0) s=(GEN)p1[i];
	}
    }
  return gcopy(s);
}

GEN vecmin(GEN x)
{
  long tx=typ(x),lx,lx2,i,j;
  GEN s,p1;

  if(tx<17) return gcopy(x);
  lx=lg(x);if(lx==1) err(minier1);
  if(tx<19)
    {
      s=(GEN)x[1];for(i=2;i<lx;i++) if(gcmp((GEN)x[i],s)<0) s=(GEN)x[i];
     }
  else
    {
      lx2=lg((GEN)x[1]);
      if(lx2==1) err(minier1);
      s=gcoeff(x,1,1);
      for(j=1;j<lx;j++)
	{
	  p1=(GEN)x[j];
	  for(i=1;i<lx2;i++) if(gcmp((GEN)p1[i],s)<0) s=(GEN)p1[i];
	}
    }
  return gcopy(s);
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*                                                                 */
/*                  AFFECTATION  S--> GEN                          */
/*                                                                 */
/*            Etant donnes un long et un GEN,affecte le long       */
/*            dans le GEN,permettant une initialisation .          */
/*                                                                 */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


void gaffsg(long s, GEN x)
{
  long  tx,l,i,v,val;
  GEN   reste,p1;
  
  tx=typ(x);
  
  switch(tx)
  {
    case 1 : affsi(s,x);break;
    case 2 : affsr(s,x);break;
    case 3 : modsiz(s,(GEN)x[1],(GEN)x[2]);break;
    case 4 :
    case 5 : affsi(s,(GEN)x[1]);
      affsi(1,(GEN)x[2]);
      break;
    case 6 : gaffsg(s,(GEN)x[1]);
      gaffsg(0,(GEN)x[2]);
      break;
    case 7 : if (!s) {setvalp(x,defaultpadicprecision);x[4]=zero;}
    else
    {
      val=0;
      l=avma;
      while (1)
      {
        p1=dvmdsi(s,(GEN)x[2],&reste);
        if (signe(reste)) break;
        val++;s=itos(p1);
      }
      avma=l;
      setvalp(x,val);
      if (s>0) affsi(s,(GEN)x[4]);
      else addsiz(s,(GEN)x[3],(GEN)x[4]);
    }
    break;
      
    case 8 : gaffsg(s,(GEN)x[2]);
      gaffsg(0,(GEN)x[3]);
      break;
      
    case 9 : gaffsg(s,(GEN)x[2]);break;
      
    case 10: v=varn(x);
      if (!s) x[1]=evallgef(2)+evalvarn(v);
      else
      {
        x[1]=evalsigne(1)+evallgef(3)+evalvarn(v);gaffsg(s,(GEN)x[2]);
      }
      break;
      
    case 11: v=varn(x);gaffsg(s,(GEN)x[2]);l=lg(x);
      if (!s) x[1]=evalvalp(l-2)+evalvarn(v);
      else
      {
        x[1]=evalsigne(1)+HIGHVALPBIT+evalvarn(v);
        for (i=3;i<l;gaffsg(0,(GEN)x[i]),i++);
      }
      break;
      
    case 13:
    case 14: gaffsg(s,(GEN)x[1]);gaffsg(1,(GEN)x[2]);
      break;
      
    case 17:
    case 18: if (lg(x)!=2) err(gaffsger1);
    else gaffsg(s,(GEN)x[1]);break;
      
    case 19: if (lg(x)!=2) err(gaffsger2);
    else gaffsg(s,(GEN)x[1]);break;
      
    default: err(gaffer1);
  }
}


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*                                                                 */
/*                    AFFECTATION GENERALE                         */
/*                                                                 */
/*            Etant donnes deux GEN x et y,affecte le contenu      */
/*            de x dans y,si cela est possible .                   */
/*                                                                 */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


void gaffect(GEN x, GEN y)
{
  long  i,j,k,l,v,vy,val,lx,ly,tx,ty,d,avmacourant;
  GEN   p1,num,den;
  
  tx=typ(x);ty=typ(y);lx=lg(x);ly=lg(y);
  
  
  if (tx<10)
  {
    if (ty>=10)
    {
      switch(ty)
      {
        case 10: v=varn(y);
	  if((y==polun[v])||(y==polx[v])) 
	    err(talker,"trying to overwrite universal polynomial, please report!");
	  gaffect(x,(GEN)y[2]);
          for(i=3;i<ly;gaffsg(0,(GEN)y[i]),i++);
          if (gcmp0(x)) y[1]=evallgef(2)+evalvarn(v);
          else y[1]=evalsigne(1)+evallgef(3)+evalvarn(v);
          break;
          
        case 11: v=varn(y);gaffect(x,(GEN)y[2]);
          if (gcmp0(x)) y[1]=evalvalp(ly-2)+evalvarn(v);
          else
          {
            y[1]=evalsigne(1)+HIGHVALPBIT+evalvarn(v);
            for (i=3;i<ly;gaffsg(0,(GEN)y[i]),i++);
          }
          break;
          
        case 13:
        case 14: gaffect(x,(GEN)y[1]);gaffsg(1,(GEN)y[2]); break;
        case 17:
        case 18:
        case 19: err(gaffer2);
          
        default: err(gaffer1);
        }
    }
    else
    {
      switch(tx)
      {
        case 1 :
          switch(ty)
          {
            case 1 : if(((y==gnil)||(y==gzero)||(y==gun)||(y==gdeux))&&(x!=y))
	      err(talker,"trying to overwrite a universal integer, please report!");
	      affii(x,y);break;
            case 2 : if((y==gpi)||(y==geuler))
	      err(talker,"trying to overwrite a universal real, please report!");
	      affir(x,y);break;
            case 3 : modiiz(x,(GEN)y[1],(GEN)y[2]);break;
            case 4 :
            case 5 : if(y==ghalf)
	      err(talker,"trying to overwrite a universal fraction, please report!");
	      affii(x,(GEN)y[1]);affsi(1,(GEN)y[2]); break;
            case 6 : if(y==gi)
	      err(talker,"trying to overwrite the universal i, please report!");
	      gaffect(x,(GEN)y[1]);gaffsg(0,(GEN)y[2]); break;
            case 7 : if (!signe(x)) {setvalp(y,defaultpadicprecision);y[4]=zero;}
              else
              {
                l=avma;
                val=pvaluation(x,(GEN)y[2],&p1);
                setvalp(y,val);modiiz(p1,(GEN)y[3],(GEN)y[4]);
                avma=l;
              }
              break;
            case 8 : gaffect(x,(GEN)y[2]);gaffsg(0,(GEN)y[3]); break;
	    case 9 : gaffect(x,(GEN)y[2]);break;
            default: err(gaffer1);
            }
            break;
          
        case 2 :
          switch(ty)
          {
            case 1 : err(gaffer3);
            case 2 : affrr(x,y);break;
            case 3 :
            case 4 :
            case 5 : err(gaffer3);
            case 6 : gaffect(x,(GEN)y[1]);gaffsg(0,(GEN)y[2]); break;
            case 7 :
            case 8 : err(gaffer3);
	    case 9 : gaffect(x,(GEN)y[2]);break;
            default: err(gaffer1);
          }
          break;
          
        case 3 :
          switch(ty)
          {
            case 1 :
            case 2 : err(gaffer4);
            case 3 : if (!divise((GEN)x[1],(GEN)y[1])) err(gaffer5);
              modiiz((GEN)x[2],(GEN)y[1],(GEN)y[2]); break;
            case 4 :
            case 5 :
            case 6 :
            case 8 : err(gaffer4);
            case 7 : err(gaffer6);
	    case 9 : gaffect(x,(GEN)y[2]);break;
            default: err(gaffer1);
          }
          break;
        case 4 :
          switch(ty)
          {
            case 1 : i=mpdivis((GEN)x[1],(GEN)x[2],y);
              if (!i) err(gaffer7);
              break;
            case 2 : diviiz((GEN)x[1],(GEN)x[2],y);break;
              
            case 3 : avmacourant=avma;
              p1=mpinvmod((GEN)x[2],(GEN)y[1]);
              modiiz(mulii((GEN)x[1],p1),(GEN)y[1],(GEN)y[2]);
              avma=avmacourant;
              break;
            case 4 :
            case 5 : for (i=1;i<=2;affii((GEN)x[i],(GEN)y[i]),i++);break;
            case 6 : gaffect(x,(GEN)y[1]);gaffsg(0,(GEN)y[2]); break;
            case 7 : if(!signe((GEN)x[1]))
              {
                setvalp(y,defaultpadicprecision);y[4]=zero;
              }
              else
              {
                l=avma;num=(GEN)x[1];den=(GEN)x[2];
                if(!(val=pvaluation(num,(GEN)y[2],&num)))
                  val= -pvaluation(den,(GEN)y[2],&den);
                p1=mulii(num,mpinvmod(den,(GEN)y[3]));
                modiiz(p1,(GEN)y[3],(GEN)y[4]);avma=l;
                setvalp(y,val);
              }
              break;
            case 8 : gaffect(x,(GEN)y[2]);gaffsg(0,(GEN)y[3]); break;
	    case 9 : gaffect(x,(GEN)y[2]);break;
            default: err(gaffer1);
          }
          break;
        case 5 :
          switch(ty)
          {
            case 1 : i=mpdivis((GEN)x[1],(GEN)x[2],y);
              if (!i) err(gaffer7);
              break;
            case 2 : diviiz((GEN)x[1],(GEN)x[2],y);break;
              
            case 3 : avmacourant=avma;gaffect(gred(x),y);
              avma=avmacourant;
              break;
            case 4 : gredz(x,y);break;
              
            case 5 : for (i=1;i<=2;affii((GEN)x[i],(GEN)y[i]),i++);break;
            case 6 : gaffect(x,(GEN)y[1]);gaffsg(0,(GEN)y[2]); break;
            case 7 : if(!signe((GEN)x[1]))
              {
                setvalp(y,defaultpadicprecision);y[4]=zero;
              }
              else
              {
                l=avma;num=(GEN)x[1];den=(GEN)x[2];
                val=pvaluation(num,(GEN)y[2],&num)-pvaluation(den,(GEN)y[2],&den);
                p1=mulii(num,mpinvmod(den,(GEN)y[3]));
                modiiz(p1,(GEN)y[3],(GEN)y[4]);avma=l;
                setvalp(y,val);
              }
              break;
            case 8 : gaffect(x,(GEN)y[2]);gaffsg(0,(GEN)y[3]); break;
	    case 9 : gaffect(x,(GEN)y[2]);break;
            default: err(gaffer1);
            }
            break;
            
        case 6 :
          switch(ty)
          {
            case 1 :
            case 2 :
            case 3 :
            case 4 :
            case 5 :
            case 7 :
            case 8 : if (!gcmp0((GEN)x[2])) err(gaffer8);
              gaffect((GEN)x[1],y);
              break;
              
            case 6 : for (i=1;i<=2;gaffect((GEN)x[i],(GEN)y[i]),i++);break;
	    case 9 : gaffect(x,(GEN)y[2]);break;
            default: err(gaffer1);
          }
          break;
          
        case 7 :
          switch(ty)
          {
            case 1 :
            case 2 : err(gaffer10);
            case 3 : if(valp(x)<0) err(gaffer11);
              l=avma;
              val=pvaluation((GEN)y[1],(GEN)x[2],&p1);
              if(!gcmp1(p1)) err(gaffer11);
              if(val>(signe((GEN)x[4])?(precp(x)+valp(x)):valp(x)+1)) err(gaffer11);
              modiiz((GEN)x[4],(GEN)y[1],(GEN)y[2]);
              avma=l;break;
            case 4 :
            case 5 :
            case 6 :
            case 8 : err(gaffer10);
              
            case 7 : if(cmpii((GEN)x[2],(GEN)y[2])) err(gaffer12);
              modiiz((GEN)x[4],(GEN)y[3],(GEN)y[4]);
              setvalp(y,valp (x));
              break;
              
	    case 9 : gaffect(x,(GEN)y[2]);break;
            default: err(gaffer1);
          }
          break;
          
        case 8 :
          switch(ty)
          {
            case 1 :
            case 3 :
            case 4 :
            case 5 :
            case 7 : if(!gcmp0((GEN)x[3])) err(gaffer9);
              gaffect((GEN)x[2],y);
              break;
            case 2 : l=avma;p1=co8(x,ly);gaffect(p1,y);
              avma=l;break;
            case 6 : ly=precision(y);
              if(ly)
              {
                l=avma;p1=co8(x,ly);gaffect(p1,y);
                avma=l;
              }
              else
              {
                if(!gcmp0((GEN)x[3])) err(gaffer9);
                gaffect((GEN)x[2],y);
              }
              break;
            case 8 : if(!gegal((GEN)x[1],(GEN)y[1])) err(gaffer21);
              for (i=2;i<=3;gaffect((GEN)x[i],(GEN)y[i]),i++);
              break;
	    case 9 : gaffect(x,(GEN)y[2]);break;
            default: err(gaffer1);
          }
          break;
	case 9 : if(ty!=9) err(gaffer17);
	  if (gdivise((GEN)x[1],(GEN)y[1]))
	    gmodz((GEN)x[2],(GEN)y[1],(GEN)y[2]);else err(gaffer18);
	  break;
        default: err(gaffer1);
	}
    }
  }
  else
  {
    if (ty<9) err(gaffer13);
    switch(tx)
    {
      case 10: v=varn(x);
        switch(ty)
        {
          case 10: if((vy=varn(y))>v) err(gaffer13);
            if(vy==v)
            {
              d=lgef(x);
              if (d>ly) err(gaffer14);
              y[1]=x[1];
              for (i=2;i<d;gaffect((GEN)x[i],(GEN)y[i]),i++);
            }
            else
            {
              gaffect(x,(GEN)y[2]);
              for(i=3;i<ly;gaffsg(0,(GEN)y[i]),i++);
              if (signe(x)) y[1]=evalsigne(1)+evallgef(3)+evalvarn(vy);
              else y[1]=evallgef(2)+evalvarn(vy);
            }
            break;
            
          case 11: if((vy=varn(y))>v) err(gaffer13);
            if (!signe(x)) gaffsg(0,y);
            else
            {
              if(vy==v)
              {
                i=gval(x,v);y[1]=evalvarn(v)+evalvalp(i)+evalsigne(1);
                k=lgef(x)-i;
                if(k>ly) k=ly;
                for (j=2;j<k;j++)
                  gaffect((GEN)x[i+j],(GEN)y[j]);
                for (j=k;j<ly;j++)
                  gaffsg(0,(GEN)y[j]);
              }
              else
              {
                gaffect(x,(GEN)y[2]);
                if (!signe(x)) y[1]=evalvalp(ly-2)+evalvarn(vy);
                else
                {
                  y[1]=evalsigne(1)+HIGHVALPBIT+evalvarn(vy);
                  for (i=3;i<ly;gaffsg(0,(GEN)y[i]),i++);
                }
              }
            }
            break;
            
          case 9: gmodz(x,(GEN)y[1],(GEN)y[2]);break;
          case 13:
          case 14: gaffect(x,(GEN)y[1]);gaffsg(1,(GEN)y[2]); break;
          case 17:
          case 18:
          case 19: err(gaffer15);
            
          default: err(gaffer1);
        }
        break;
        
      case 11: if (ty!=11) err(gaffer16);
        v=varn(x);if((vy=varn(y))>v) err(gaffer13);
        if(vy==v)
        {
          y[1]=x[1];
          if(!gcmp0(x))
          {
            k=lx;if (k>ly) k=ly;
            for (i=2;i<k;i++) gaffect((GEN)x[i],(GEN)y[i]);
            for (i=k;i<ly;i++) gaffsg(0,(GEN)y[i]);
          }
        }
        else
        {
          gaffect(x,(GEN)y[2]);
          if (!signe(x)) y[1]=evalvalp(ly-2)+evalvarn(vy);
          else
          {
            y[1]=evalsigne(1)+HIGHVALPBIT+evalvarn(vy);
            for (i=3;i<ly;gaffsg(0,(GEN)y[i]),i++);
          }
        }
        break;
        
      case 13:
      case 14: switch(ty)
      {
        case 10:
        case 17:
        case 18:
        case 19: err(gaffer19);
        case 11: gdivz((GEN)x[1],(GEN)x[2],y);break;
        case 9: avmacourant=avma;p1=ginvmod((GEN)x[2],(GEN)y[1]);
          gmodz(gmul((GEN)x[1],p1),(GEN)y[1],(GEN)y[2]);
          avma=avmacourant;
          break;
          
        case 13:
          if (tx==14) gredz(x,y);
          else
            for (i=1;i<=2;gaffect((GEN)x[i],(GEN)y[i]),i++);
          break;
          
        case 14: for (i=1;i<=2;gaffect((GEN)x[i],(GEN)y[i]),i++); break;
          
        default: err(gaffer1);
      }
      break;

      case 15:
      case 16:
      case 17:
      case 18:
      case 19: if (ty!=tx) err(gaffer20);
        if (ly!=lx) err(gaffer20);
        for (i=1;i<lx;gaffect((GEN)x[i],(GEN)y[i]),i++);
        break;
        
      default: err(gaffer1);
    }
  }
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*                                                                 */
/*                 CONVERSION QUAD-->REEL,COMPLEXE                 */
/*                          OU P-ADIQUE                            */
/*                                                                 */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

GEN     co8(GEN x, long l)
{
  long  av,tetpil;
  GEN   y,p1,p2,p3;
  
  p3=(GEN)x[1];
  av=avma;p1=gmul2n((GEN)p3[3],-1);
  p2=gsqrt(gsub(gmul(p1,p1),(GEN)p3[2]),l);
  p1=gmul((GEN)x[3],gsub(p2,p1));
  tetpil=avma;y=gerepile(av,tetpil,gadd(p1,(GEN)x[2]));
  return y;
}

GEN   cvtop(GEN x, GEN p, long l)
{
  GEN z,p1,p2,p3;
  long av,tetpil,n,tx=typ(x);
  
  if(typ(p)!=1) err(cvtoper1);
  if(gcmp0(x)) z=ggrando(p,l);
  else
  {
    switch(tx)
    {
      case 1: z=gadd(x,ggrando(p,ggval(x,p)+l));break;
      case 2: err(cvtoper2);
      case 3: n=ggval((GEN)x[1],p);
        if(n>l) n=l;
        z=gadd((GEN)x[2],ggrando(p,n));break;
      case 4:
      case 5: n=ggval((GEN)x[1],p);
        n-=ggval((GEN)x[2],p);
        z=gadd(x,ggrando(p,n+l));break;
      case 6: av=avma;p1=gsqrt(gaddgs(ggrando(p,l),-1),0);
        p1=gmul(p1,(GEN)x[2]);tetpil=avma;
        z=gerepile(av,tetpil,gadd(p1,(GEN)x[1]));
        break;
      case 7: z=gprec(x,l);break;
      case 8:
        av=avma;p1=(GEN)x[1];p3=gmul2n((GEN)p1[3],-1);
        p2=gsub(gmul(p3,p3),(GEN)p1[2]);
        switch(typ(p2))
          {
          case 1: n=ggval(p2,p);
            p2=gadd(p2,ggrando(p,n+l));break;
          case 3: break;
          case 4:
          case 5: n=ggval((GEN)p2[1],p);
            n-=ggval((GEN)p2[2],p);
            p2=gadd(p2,ggrando(p,n+l));break;
          case 7: break;
          default: err(gadder14);
          }
        p2=gsqrt(p2,0);
        p1=gmul((GEN)x[3],gsub(p2,p3));tetpil=avma;
        z=gerepile(av,tetpil,gadd((GEN)x[2],p1));
        break;
      default: err(cvtoper2);
    }
  }
  return z;
}

GEN gcvtop(GEN x, GEN p, long r)
{
  long i,tx=typ(x),lx;
  GEN y;

  if(tx<9) return cvtop(x,p,r);
  switch(tx)
    {
    case 10: lx=lgef(x);y=cgetg(lx,10);y[1]=x[1];
      for(i=2;i<lx;i++) y[i]=(long)gcvtop((GEN)x[i],p,r);break;
    case 11:
      if(gcmp0(x)) y=gcopy(x);
      else 
	{
	  lx=lg(x);y=cgetg(lx,11);y[1]=x[1];
	  for(i=2;i<lx;i++) y[i]=(long)gcvtop((GEN)x[i],p,r);
	}
      break;
    case 9:
    case 13:
    case 14:
    case 17:
    case 18:
    case 19: lx=lg(x);y=cgetg(lx,tx);
      for(i=1;i<lx;i++) y[i]=(long)gcvtop((GEN)x[i],p,r);break;
    default: err(cvtoper2);
    }
  return y;
}

long   gexpo(GEN x)
{
  long tx=typ(x),lx=lg(x),e,i,y,av;
  GEN  p1;
  
  switch(tx)
  {
    case 1 :if(!signe(x)) err(gexpoer2);
      return expi(x);
    case 4 :
    case 5 : if(!signe((GEN)x[1])) err(gexpoer2);
      return expi((GEN)x[1])-expi((GEN)x[2]);
    case 2 : return expo(x);
    case 6 : av=avma;p1=gnorm(x);y=(gexpo(p1)>>1); avma=av;break;
    case 8 :  if(gcmp0(x)) err(gexpoer2);
      av=avma;p1=cgetg(3,6);p1[1]=lgetr(3);
      p1[2]=lgetr(3);gaffect(x,p1);y=gexpo(p1);
      avma=av;break;
    case 10: lx=lgef(x);
    case 11: 
    case 17:
    case 18:
    case 19: y= -BIGINT;
      for(i=lontyp[tx];i<lx;i++)
      {
        e=gexpo((GEN)x[i]);if(e>y) y=e;
      }
      break;
    default: err(gexpoer1);
  }
  return y;
}

long gsize(GEN x)
{
  long l;
  if(gcmp0(x)) return 0;
  l=(long)(gexpo(x)*L2SL10);
  return l;
}

void normalize(GEN *px)
{
  long    i,j,v,l,tetpil,e,lx,f;
  GEN     p1,x;
  
  if(typ(x= *px)!=11) err(gnormaler);
  i=2;f=1;lx=lg(x);v=varn(x);
  while (f&&(i<lx))
    {f=isexactzero((GEN)x[i]);i++;}
/*    {f=gcmp0((GEN)x[i]);i++;} */
  i--;
  if(i>2)
  {
    l=(long)(x+lx);e=valp(x);
    if (f)
    {
      avma=l;
      p1=cgetg(3,11);p1[1]=evalvalp(lx-2)+evalvarn(v);
      *px=p1;
    }
    else
    {
      tetpil=avma;
      p1=cgetg(lx-i+2,11);
      p1[1]=evalsigne(1)+evalvalp(e+i-2)+evalvarn(v);
      for (j=i;j<lx;j++)
        p1[j-i+2]=lcopy((GEN)x[j]);
      *px=gerepile(l,tetpil,p1);
    }
  }
  else
  {
    if(f) setsigne(x,0);else setsigne(x,1);
  }
}

void normalizepol(GEN *px)
{
  long    i,lx,f;
  GEN     x;
  
  if(typ(x= *px)!=10) err(gnormaler);
  lx=lgef(x);
  if(lx>2)
    {
      f=1;i=lx-1;
      while (f&&(i>1))
	{f=isexactzero((GEN)x[i]);i--;}
      if(f) {setlgef(*px,2);setsigne(*px,0);}
      else
	{
	  i++;f=gcmp0((GEN)x[i]);setlgef(*px,i+1);
	  while(f&&(i>1))
	    {f=gcmp0((GEN)x[i]);i--;}
	  if(f) setsigne(*px,0);else setsigne(*px,1);
	  /* setsigne(*px,f?0:1); ecrit comme ci-dessus pour faire plaisir a alpha */
	}
    }
  else setsigne(*px,0);
}

int gsigne(GEN x)
{
  switch(typ(x))
  {
    case 1:
    case 2: return signe(x);
    case 4:
    case 5: return (signe((GEN)x[2])>0) ? signe((GEN)x[1]) : -signe((GEN)x[1]);
    default: err(gsigner);return 0;
  }
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*                                                                 */
/*                            PUISSANCE                            */
/*                                                                 */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifdef __cplusplus
GEN     gpui(GEN x, GEN n, long prec = 0)
#else
GEN     gpui(GEN x, GEN n, ...)
#endif
{
  long    av1,av2,av3,lx,tx,i,j,tetpil;
  unsigned long m;
  GEN     p1,p2,y,z,alp;
  
  if (typ(n)==1)
  {
    lx=lgef(n);
    if((lx==2)||((lx==3)&&((ulong)n[2]<(ulong)HIGHBIT)))
      y=gpuigs(x,itos(n),prec);
    else
    {
      z=x;av1=avma;
      if((typ(x)!=16)&&(typ(x)!=15)) y=gun;
      else
      {
	if(typ(x)==16)
	  {
	    p1=mulii((GEN)x[1],(GEN)x[3]);p2=shifti(mulii((GEN)x[2],(GEN)x[2]),-2);
	    y=cgetg(4,16);y[1]=un;y[2]=mpodd((GEN)x[2]) ? un : zero;
	    y[3]=lsubii(p1,p2);
	  }
	else
	  {
	    y=cgetg(5,15);y[1]=un;
	    p1=subii(mulii((GEN)x[2],(GEN)x[2]),shifti(mulii((GEN)x[1],(GEN)x[3]),2));
	    p2=racine(p1);if(mpodd(subii(p2,(GEN)x[2]))) p2=addsi(-1,p2);
	    y[2]=(long)p2;y[3]=lshifti(subii(mulii(p2,p2),p1),-2);
	    p1=cgetr(prec);y[4]=(long)p1;p1[1]=HIGHEXPOBIT-((prec-2)<<TWOPOTBITS_IN_LONG);
	    p1[2]=0;tetpil=avma;y=gerepile(av1,tetpil,gcopy(y));
	  }
      }
      for (i=lgef(n)-1;i>2;i--)
      {
        for (m=n[i],j=0;j<BITS_IN_LONG;j++,m>>=1)
        {
          if (m&1) y=gmul(y,z);
          z=gsqr(z);
        }
      }
      for (m=n[2];m>1;m>>=1)
      {
        if (m&1) y=gmul(y,z);
        z=gsqr(z);
      }
      if (signe(n)>0)
      {
        tetpil=avma;y=gmul(y,z);
      }
      else
      {
        y=gmul(y,z);
        tetpil=avma;y=ginv(y);
      }
      y=gerepile(av1,tetpil,y);
    }
  }
  else
  {
    if((tx=typ(x))>=17)
      {
	y=cgetg(lx=lg(x),tx);for(i=1;i<lx;i++) y[i]=lpui((GEN)x[i],n,prec);
      }
    else
      {
	if(tx!=11)
	  {
	    av1=avma;
	    if(gcmp0(x))
	      {
		if(gcmpgs(p1=greal(n),0)<=0) err(gpuier3);
		av2=(long)precision(x);
		if(av2)
		  {
		    p1=ground(gmulsg(gexpo(x),p1));
		    if(lgef(p1)>3) err(gpuier4);
		    av2=itos(p1);if(labs(av2)>=(long)HIGHEXPOBIT) err(gpuier4);
		    avma=av1;y=cgetr(3);y[2]=0;y[1]=HIGHEXPOBIT+av2;
		  }
		else {avma=av1;y=gcopy(x);}
	      }
	    else
	      {
		y=gmul(n,glog(x,prec));av2=avma;
		y=gerepile(av1,av2,gexp(y,prec));
	      }
	  }
	else
	  {
	    if(valp(x)) err(gpuier2);
	    if(gvar9(n)>varn(x))
	      {
		if(gcmp1((GEN)x[2]))
		  {
		    av1=avma;alp=gaddgs(n,1);
		    av2=avma;y=cgetg(lx=lg(x),11);
		    y[1]=evalsigne(1)+HIGHVALPBIT+evalvarn(varn(x));
		    y[2]=un;
		    for(i=3;i<lx;i++)
		      {
			av3=avma;p1=gzero;
			for(j=1;j<i-1;j++)
			  {
			    p2=gsubgs(gmulgs(alp,j),i-2);
			    p1=gadd(p1,gmul(gmul(p2,(GEN)x[j+2]),(GEN)y[i-j]));
			  }
			tetpil=avma;
			y[i]=lpile(av3,tetpil,gdivgs(p1,i-2));
		      }
		    y=gerepile(av1,av2,y);
		  }
		else
		  {
		    av1=avma;p1=gdiv(x,(GEN)x[2]);
		    p1=gpui(p1,n,prec);p2=gpui((GEN)x[2],n,prec);
		    tetpil=avma;y=gerepile(av1,tetpil,gmul(p1,p2));
		  }
	      }
	    else
	      {
		av1=avma;y=gmul(n,glog(x,prec));av2=avma;
		y=gerepile(av1,av2,gexp(y,prec));
	      }
	  }
      }
  }
  return y;
}

#ifdef __cplusplus
GEN     gpuigs(GEN x, long n, long prec = 0)
#else
GEN     gpuigs(GEN x, long n, ...)
#endif
{
  long    lx,av,m,dd,tetpil,i,j;
  GEN     y,z,p1,p2;
  
  
  if (!n)
  {
    lx=lg(x);
    switch(typ(x))
    {
      case 1 :
      case 2 :
      case 4 :
      case 5 :
      case 7 : y=gun;break;
      case 6 : y=cgetg(3,6);y[1]=un;
        y[2]=zero;
        break;
      case 3 : y=cgetg(3,3);y[1]=copyifstack((GEN)x[1]);
        y[2]=un;
        break;
      case 8 : y=cgetg(4,8);y[1]=copyifstack((GEN)x[1]);
        y[2]=un;y[3]=zero;
        break;
      case 10: 
      case 11: y=polun[varn(x)]; break;
      case 13: 
      case 14: y=polun[gvar(x)]; break;
      case 9: y=cgetg(3,9);y[1]=copyifstack((GEN)x[1]);
        y[2]=lpuigs((GEN)x[2],0);
        break;
      case 15 : av=avma;y=cgetg(5,15);y[1]=un;
        p1=subii(mulii((GEN)x[2],(GEN)x[2]),shifti(mulii((GEN)x[1],(GEN)x[3]),2));
	p2=racine(p1);if(mpodd(subii(p2,(GEN)x[2]))) p2=addsi(-1,p2);
	y[2]=(long)p2;y[3]=lshifti(subii(mulii(p2,p2),p1),-2);
        p1=cgetr(prec);y[4]=(long)p1;p1[1]=HIGHEXPOBIT-((prec-2)<<TWOPOTBITS_IN_LONG);
	p1[2]=0;tetpil=avma;y=gerepile(av,tetpil,gcopy(y));break;
      case 16 : y=cgetg(4,16);y[1]=un;y[2]=mpodd((GEN)x[2]) ? un : zero;
        av=avma;p1=mulii((GEN)x[1],(GEN)x[3]);p2=shifti(mulii((GEN)x[2],(GEN)x[2]),-2);tetpil=avma;
        y[3]=lpile(av,tetpil,subii(p1,p2));break;
      case 17:
      case 18: err(gpuier1);
      case 19: if (lx!=(lg((GEN)x[1]))) err(gpuier1);
        else
        {
          y=cgetg(lx,19);
          for (j=1;j<lx;j++)
          {
            y[j]=lgetg(lx,18);
            for (i=1;i<lx;i++)
              coeff(y,i,j)=(i!=j) ? zero :
                lpuigs(gcoeff(x,i,i),0);
          }
        }
    }
  }
  else if (n==1) y=gcopy(x);
  else if (n== -1) y=ginv(x);
  else
  {
    m=abs(n);
    if(ismonome(x))
    {
      j=lgef(x);
      dd=(j-3)*m+3;
      av=avma;
      p1=cgetg(dd,10);p1[1]=evalsigne(1)+evallgef(dd)+evalvarn(varn(x));
      for(i=2;i<=dd-1;i++) p1[i]=zero;
      p1[dd-1]=lpuigs((GEN)x[j-1],m);
      if(n<0)
      {
        tetpil=avma;y=cgetg(3,13);y[1]=(long)denom((GEN)p1[dd-1]);
        y[2]=lmul(p1,(GEN)y[1]);y=gerepile(av,tetpil,y);
      }
      else y=p1;
    }
    else
    {
      z=x;av=avma;
      if((typ(x)!=16)&&(typ(x)!=15)) y=gun;
      else
      {
	if(typ(x)==16)
	  {
	    p1=mulii((GEN)x[1],(GEN)x[3]);p2=shifti(mulii((GEN)x[2],(GEN)x[2]),-2);
	    y=cgetg(4,16);y[1]=un;if(mpodd((GEN)x[2])) y[2]=un;else y[2]=zero;
	    y[3]=lsubii(p1,p2);
	  }
	else
	  {
	    y=cgetg(5,15);y[1]=un;
	    p1=subii(mulii((GEN)x[2],(GEN)x[2]),shifti(mulii((GEN)x[1],(GEN)x[3]),2));
	    p2=racine(p1);if(mpodd(subii(p2,(GEN)x[2]))) p2=addsi(-1,p2);
	    y[2]=(long)p2;y[3]=lshifti(subii(mulii(p2,p2),p1),-2);
	    p1=cgetr(prec);y[4]=(long)p1;p1[1]=HIGHEXPOBIT-((prec-2)<<TWOPOTBITS_IN_LONG);
	    p1[2]=0;tetpil=avma;y=gerepile(av,tetpil,gcopy(y));
	  }
      }
      for(;m>1;m>>=1)
      {
        if (m&1) y=gmul(y,z);
        z=gsqr(z);
      }
      if (n>0)
      {
        tetpil=avma;y=gmul(y,z);
      }
      else
      {
        y=gmul(y,z);
        tetpil=avma;y=ginv(y);
      }
      y=gerepile(av,tetpil,y);
    }
  }
  return y;
}

int iscomplex(GEN x)
{
  long tx=typ(x);
  
  switch(tx)
  {
    case 1:
    case 2:
    case 4:
    case 5: return 0;
    case 6: return !gcmp0((GEN)x[2]);
    case 8: return signe((GEN)((GEN)x[1])[2])>0;
    default: err(iscomplexer1);return 0;
  }
}

int ismonome(GEN x)
{
  long i,l;

  if((typ(x)!=10)||(!signe(x))) return 0;
  l=lgef(x)-1;i=2;
  while((i<=l)&&isexactzero((GEN)x[i])) i++;
  return i==l;
}

GEN     gsqr(GEN x)
{
  long  tx=typ(x),lx,av,i,j;
  GEN   y,p1;
  
  if(tx==7)
  {
    y=cgetg(5,7);
    y[2]=x[2];
    if(!cmpis((GEN)x[2] ,2))
    {
      i=precp(x)+1;av=avma;
      p1=addii((GEN)x[3],shifti((GEN)x[4],1));
      if(!gcmp0(p1))
      {
        j=vali(p1);if(j<i) i=j;
      }
      avma=av;y[3]=lshifti((GEN)x[3],i);
      y[1]=evalprecp(precp(x)+i)+evalvalp(2*valp(x));
    }
    else
    {
      y[3]=lcopy((GEN)x[3]);
      y[1]=evalprecp(precp(x))+evalvalp(2*valp(x));
    }
    y[4]=lgeti(lg((GEN)y[3]));
    av=avma;modiiz(mulii((GEN)x[4],(GEN)x[4]),(GEN)y[3],(GEN)y[4]);avma=av;
  }
  else if(tx==15) y=sqcompreal(x);
  else if(tx==16) y=sqcomp(x);
  else
  {
    if((tx<17)||((tx==19)&&((lg(x)==1)||(lg(x)==lg((GEN)x[1]))))) y=gmul(x,x);
    else
    {
      y=cgetg(lx=lg(x),tx);
      for(i=1;i<lx;i++)
        y[i]=lsqr((GEN)x[i]);
    }
  }
  return y;
}
