/********************************************************************/
/********************************************************************/
/**                                                                **/
/**                +++++++++++++++++++++++++++++++                 **/
/**                +                             +                 **/
/**                +    OPERATIONS GENERIQUES    +                 **/
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

/********************************************************************/
/********************************************************************/
/**                                                                **/
/**                        TYPE D'UN GEN                           **/
/**                                                                **/
/**                    (POUR GP UNIQUEMENT)                        **/
/**                                                                **/
/********************************************************************/
/********************************************************************/

GEN gtype(GEN x)
{
  return stoi(typ(x));
}

GEN gsettype(GEN x,long t)
{
  GEN y;

  y=gcopy(x);settyp(y,t);return y;
}

/********************************************************************/
/********************************************************************/
/**                                                                **/
/**                 NUMERO DE LA VARIABLE PRINCIPALE               **/
/**                                                                **/
/********************************************************************/
/********************************************************************/

int    gvar(GEN x)
{
  long tx=typ(x),i,v,w;
  if((tx>>1)==5) return varn(x);
  if((tx<9)||(tx==15)||(tx==16)) return BIGINT;
  if(tx==9) return varn((GEN)x[1]);
  v=BIGINT;
  for(i=1;i<lg(x);i++) {w=gvar((GEN)x[i]);if(w<v) v=w;}
  return v;
}

int gvar2(GEN x)
{
  long tx=typ(x),i,v,w;
  if((tx<9)||(tx==15)||(tx==16)) return BIGINT;
  if(tx==9) {v=gvar2((GEN)x[1]);w=gvar2((GEN)x[2]);return min(v,w);}
  v=BIGINT;
  if((tx==11)&&(!signe(x))) return v;
  if(tx<12) 
    {
      for(i=2;i<((tx==11)?lg(x):lgef(x));i++)
	{w=gvar((GEN)x[i]);if(w<v) v=w;} 
      return v;
    }
  else {for(i=1;i<lg(x);i++) {w=gvar2((GEN)x[i]);if(w<v) v=w;} return v;}
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*                                                                 */
/*            PRECISION D'UN SCALAIRE                              */
/*                                                                 */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

int precision(GEN x)
{
  long    l1,l2,tx=typ(x);
  
  if (tx==2) return max(lg(x),2-(expo(x)>>TWOPOTBITS_IN_LONG));
  if (tx==6)
  {
    l1=precision((GEN)x[1]);l2=precision((GEN)x[2]);
    if(!l1) return l2;
    if(!l2) return l1;
    return (l1>l2) ? l2 : l1;
  }
  return 0;
}

/* degre de x par rapport a la variable principale */
/* et 0 si x est nul */

int tdeg(GEN x)
{
  long tx=typ(x);
  
  if(gcmp0(x)) return 0;
  if(tx<10) return 0;
  switch(tx)
  {
    case 10: return lgef(x)-3;
    case 13:
    case 14: return tdeg((GEN)x[1])-tdeg((GEN)x[2]);
    default: err(tdeger);return 0;
  }
}

/********************************************************************/
/********************************************************************/
/**                                                                **/
/**                     MULTIPLICATION SIMPLE                      **/
/**                                                                **/
/********************************************************************/
/********************************************************************/

GEN   gmulsg(long s, GEN y)
{
  long ty=typ(y),ly=lg(y),i,l,tetpil;
  GEN  z,p1;

  switch(ty)
  {
  case 1 : z=mulsi(s,y);break;
  
  case 2 : z=mulsr(s,y);break;
  
  case 3 : z=cgetg(ly,ty);z[1]=copyifstack((GEN)y[1]);
    l=avma;
    p1=mulsi(s,(GEN)y[2]);
    tetpil=avma;
    z[2]=lpile(l,tetpil,modii(p1,(GEN)y[1]));
    break;
  
  case 4 :
  
  case 5 : z=cgetg(ly,ty);
    z[1]=lmulsi(s,(GEN)y[1]);
    z[2]=lcopy((GEN)y[2]);
    if (ty==4) gredsp(&z);
    break;
  
  case 6 : z=cgetg(ly,ty);
    z[1]=lmulsg(s,(GEN)y[1]);
    z[2]=lmulsg(s,(GEN)y[2]);
    break;
  
  case 7 : if(s)
    {
      l=avma;p1=cgetp(y);gaffsg(s,p1);
      tetpil=avma;z=gerepile(l,tetpil,gmul(p1,y));
    }
    else z=gzero;
    break;
  
  case 8 : z=cgetg(ly,ty);
    z[2]=lmulsg(s,(GEN)y[2]);
    z[3]=lmulsg(s,(GEN)y[3]);
    z[1]=copyifstack((GEN)y[1]);
    break;

  case 9 : z=cgetg(ly,ty);z[2]=lmulsg(s,(GEN)y[2]);
    z[1]=copyifstack((GEN)y[1]);
    break;
  
  case 10: 
    if ((!s)||gcmp0(y)) {z=cgetg(2,10);z[1]=evallgef(2)+evalvarn(varn(y));}
    else
    {
      ly=lgef(y);z=cgetg(ly,ty);
      for (i=2;i<ly;i++)
      z[i]=lmulsg(s,(GEN)y[i]);
      z[1]=y[1];normalizepol(&z);
    }
    break;
  
  case 11: if (!s)
    {
    z=cgetg(3,10);z[1]=evallgef(2)+evalvarn(varn(y));
    }
  else
    {
    if (gcmp0(y)) z=gcopy(y);
    else
      {
      z=cgetg(ly,ty);
      for (i=2;i<ly;i++)
        z[i]=lmulsg(s,(GEN)y[i]);
      z[1]=y[1];
      normalize(&z);
      }
    }
    break;
  
  case 13:
    if(s) 
      {
	l=avma;z=cgetg(ly,ty);z[1]=lmulsg(s,(GEN)y[1]);z[2]=y[2];
	tetpil=avma;z=gerepile(l,tetpil,gred(z));
      }
    else {z=cgetg(2,10);z[1]=evallgef(2)+evalvarn(gvar(y));}
    break;
  case 14: 
    if(s) {z=cgetg(ly,ty);z[1]=lmulsg(s,(GEN)y[1]);z[2]=lcopy((GEN)y[2]);}
    else {z=cgetg(2,10);z[1]=evallgef(2)+evalvarn(gvar(y));}
    break;
  
  case 17:
  case 18:
  case 19: z=cgetg(ly,ty);
    for (i=1;i<ly;i++)
    z[i]=lmulsg(s,(GEN)y[i]);
    break;
  
  default: err(gmuler1);
  
  }
  return z;
}

/********************************************************************/
/********************************************************************/
/**                                                                **/
/**                       DIVISION SIMPLE                          **/
/**                                                                **/
/********************************************************************/
/********************************************************************/

GEN   gdivgs(GEN x, long s)
{
  long  tx=typ(x),lx=lg(x),i,l,tetpil,court[3];
  GEN z,p1;

  if(!s) err(gdiver2);
  switch(tx)
  {
    case 1 : l=avma;z=dvmdis(x,s,&p1);
      if(!signe(p1)) cgiv(p1);
      else
      {
        avma=l;
        z=cgetg(3,4);z[1]=lcopy(x);
        z[2]=lstoi(s);
        if(s>=0)
        {
          z[1]=lcopy(x);z[2]=lstoi(s);
        }
        else
        {
          z[1]=lnegi(x);z[2]=lstoi(-s);
        }
        gredsp(&z);
      }
      break;
    
    case 2 : z=divrs(x,s);break;
    
    case 4 :
    case 5 : z=cgetg(lx,tx);z[1]=lcopy((GEN)x[1]);
      z[2]=lmulsg(s,(GEN)x[2]);
      if(signe((GEN)z[2])<0)
      {
        mpnegz((GEN)z[1],(GEN)z[1]);
        mpnegz((GEN)z[2],(GEN)z[2]);
      }
      if (tx==4) gredsp(&z);
      break;
    
    case 6 : z=cgetg(lx,tx);
      z[1]=ldivgs((GEN)x[1],s);
      z[2]=ldivgs((GEN)x[2],s);
      break;
    
    case 8 : z=cgetg(lx,tx);
      z[1]=copyifstack((GEN)x[1]);
      for (i=2;i<4;i++)
      z[i]=ldivgs((GEN)x[i],s);
      break;
    
    case 9 : z=cgetg(lx,tx);z[2]=ldivgs((GEN)x[2],s);
      z[1]=copyifstack((GEN)x[1]);
      break;
    
    case 10: lx=lgef(x);z=cgetg(lx,tx);
      for (i=2;i<lx;i++)
      z[i]=ldivgs((GEN)x[i],s);
      z[1]=x[1];
      break;
    
    case 11:
      if (gcmp0(x)) z=gcopy(x);
      else
      {
        z=cgetg(lx,tx);
        for (i=2;i<lx;i++)
        z[i]=ldivgs((GEN)x[i],s);
        z[1]=x[1];
        normalize(&z);
      }
      break;
    
    case 13: l=avma;z=cgetg(lx,tx);z[1]=x[1];z[2]=lmulsg(s,(GEN)x[2]);
      tetpil=avma;z=gerepile(l,tetpil,gred(z));
      break;
    case 14: z=cgetg(lx,tx);z[1]=lcopy((GEN)x[1]);z[2]=lmulsg(s,(GEN)x[2]);
      break;
    
    case 17:
    case 18:
    case 19: z=cgetg(lx,tx);
      for (i=1;i<lx;i++)
      z[i]=ldivgs((GEN)x[i],s);
      break;
    
    default: court[0] = evaltyp(1)+evalpere(1)+evallg(3); affsi(s,court);z=gdiv(x,court);
  }
  return z;
}

GEN   gaddpex(GEN x, GEN y)
  
   /* addition d'un type entier ou rationnel avec un p-adique */
   /* x doit etre entier ou rationnel et y   p-adique     */
   /* a usage interne donc aucune verification de type.   */
  
             
  
{
  GEN   z,p,p1,p2,p3;
  long  e1,e2,e3,av,tetpil;

  if(gcmp0(x)) z=gcopy(y);
  else
  {
    av=avma;z=cgetg(5,7);e1=valp(y);
    p=(GEN)y[2];
    if(typ(x)>=4)
    {
      e3=pvaluation((GEN)x[1],p,&p2);
      e3-=pvaluation((GEN)x[2],p,&p3);
      p2=gdiv(p2,p3);
    }
    else e3=pvaluation(x,p,&p2);
    e2=signe((GEN)y[4])?e1+precp(y)-e3:e1-e3;
    z[2]=(long)p;
    if(e2<=0) {z[3]=un;z[1]=evalprecp(0)+evalvalp(e3);}
    else
    {
      z[1]=evalprecp(e2)+evalvalp(e3);
      if(e1-e3)
      {
        p1=gpuigs(p,e1-e3);
        z[3]=lmul((GEN)y[3],p1);
      }
      else z[3]=lcopy((GEN)y[3]);
    }
    z[4]=lmod(p2,(GEN)z[3]);
    tetpil=avma;z=gerepile(av,tetpil,gadd(z,y));
  }
  return z;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*                                                                 */
/*                    MODULO GENERAL                               */
/*                                                                 */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


GEN     gmod(GEN x, GEN y)
{
  long  tx,ty,l,av,tetpil,lx,i;
  GEN   z,p1,p2;
  
  ty=typ(y);tx=typ(x);
  switch(ty)
    {
    case 1:
      switch(tx)
	{
	case 1 : z=modii(x,y);break;
	case 2 : err(gmoder1);break;
	case 3 : z=cgetg(3,3);z[1]=lmppgcd((GEN)x[1],y);
	  z[2]=lmodii((GEN)x[2],(GEN)z[1]);
	  break;
	case 4 :
	case 5 : l=avma;if(tx==5) p1=gred(x);else p1=x;
	  p1=mulii((GEN)p1[1],mpinvmod((GEN)p1[2],y));
	  tetpil=avma;
	  z=gerepile(l,tetpil,modii(p1,y));
	  break;
	case 8 : z=cgetg(4,8);z[1]=copyifstack((GEN)x[1]);
	  z[2]=lmod((GEN)x[2],y);
	  z[3]=lmod((GEN)x[3],y);
	  break;
	case 10: z=gzero;break;
	case 17:
	case 18:
	case 19: lx=lg(x);z=cgetg(lx,tx);
	  for(i=1;i<lx;i++) z[i]=lmod((GEN)x[i],y);
	  break;
	default: err(gmoder1);
	} break;
    case 2: case 4: case 5:
      switch(tx)
	{
	case 1: case 2: case 4: case 5:
	  av=avma;p1=gfloor(gdiv(x,y));p2=gmul(p1,y);
	  tetpil=avma;z=gerepile(av,tetpil,gsub(x,p2));break;
	case 10: z=gzero;break;
	case 17:
	case 18:
	case 19: lx=lg(x);z=cgetg(lx,tx);
	  for(i=1;i<lx;i++) z[i]=lmod((GEN)x[i],y);
	  break;
	default: err(gmoder1);
	} break;
    case 10:
      if(tx<9) z=gcopy(x);
      else
	{
	  switch(tx)
	    {
	    case 9 : z=cgetg(3,9);z[1]=lgcd((GEN)x[1],y);
	      z[2]=lres((GEN)x[2],(GEN)z[1]);
	      break;
	    case 10: z=gres(x,y);break;
	    case 11: err(gmoder3);break;
	    case 13:
	    case 14: l=avma;if(tx==14) p1=gred(x);else p1=x;
	      p1=gmul((GEN)p1[1],ginvmod((GEN)p1[2],y));
            tetpil=avma;z=gerepile(l,tetpil,gres(p1,y));
	      break;
	    case 17:
	    case 18:
	    case 19: lx=lg(x);z=cgetg(lx,tx);
	      for(i=1;i<lx;i++) z[i]=lmod((GEN)x[i],y);
	      break;
	    default: err(gmoder3);
	    } break;
	} break;
    default: err(gmoder5);
    }
  return z;
}

GEN     gmodulo(GEN x,GEN y)
     
{
  long  tx=typ(x),ty=typ(y),l,i;
  GEN   z;
  
  if(tx>=17)
    {
      l=lg(x);z=cgetg(l,tx);
      for(i=1;i<l;i++) z[i]=(long)gmodulo((GEN)x[i],y);
      return z;
    }
  if(ty==1)
  {
    if((tx>5)||(tx==2)||(tx==3)) err(gmoder1);
    z=cgetg(3,3);z[1] = lclone(y);
    z[2]=lmod(x,y);
  }
  else
    if(ty==10)
    {
      z=cgetg(3,9);z[1] = lclone(y);
      if(tx>=10)
	{if((tx==10)||(tx==13)||(tx==14)) z[2]=lmod(x,y);else err(gmoder1);}
      else z[2]=lcopy(x);
    }
    else err(gmoder1);
  return z;
}

GEN     gmodulcp(GEN x,GEN y)
     
{
  long  tx=typ(x),ty=typ(y),l,i;
  GEN   z;
  
  if(tx>=17)
    {
      l=lg(x);z=cgetg(l,tx);
      for(i=1;i<l;i++) z[i]=(long)gmodulcp((GEN)x[i],y);
      return z;
    }
  if(ty==1)
  {
    if((tx>5)||(tx==2)||(tx==3)) err(gmoder1);
    z=cgetg(3,3);z[1]=lcopy(y);
    z[2]=lmod(x,y);
  }
  else
    if(ty==10)
    {
      z=cgetg(3,9);z[1]=lcopy(y);
      if(ty>=10)
	{if((ty==10)||(tx==13)||(tx==14)) z[2]=lmod(x,y);else err(gmoder1);}
      else z[2]=lcopy(x);
    }
    else err(gmoder1);
  return z;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*                                                                 */
/*                 DIVISION ENTIERE GENERALE                       */
/*            DIVISION ENTIERE AVEC RESTE GENERALE                 */
/*                                                                 */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

GEN     gdivent(GEN x, GEN y)
{
  long    tx=typ(x),ty=typ(y),av,tetpil,i;
  GEN     z,p1;
  
  if (tx==1)
  {
    if(ty==1)
    {
      av=avma;z=dvmdii(x,y,&p1);
      i=signe(p1);cgiv(p1);
      if(i<0)
      {
        tetpil=avma;z=gerepile(av,tetpil,gaddgs(z,-signe(y)));
      }
    }
    else
    {
      if(ty!=10) err(gdiventer);
      z=gzero;
    }
    return z;
  }
  if((ty!=10)||(tx>10)) err(gdiventer);
  if(tx==10) return gdeuc(x,y);
  else return gzero;
}

GEN     gdiventres(GEN x, GEN y)
{
  long    tx=typ(x),ty=typ(y);
  GEN     z;
  
  z=cgetg(3,18);
  if (tx==1)
  {
    if(ty==1) z[1]=(long)dvmdii(x,y,(GEN *)(z+2));
    else
    {
      if(ty!=10) err(gdiventer);
      z[1]=zero;z[2]=lcopy(x);
    }
  }
  else
  {
    if((ty!=10)||(tx>10)) err(gdiventer);
    if(tx==10) z[1]=ldivres(x,y,(GEN *)(z+2));
    else {z[1]=zero;z[2]=lcopy(y);}
  }
  return z;
}

GEN     gdivmod(GEN x, GEN y, GEN *pr)
{
  long    tx=typ(x),ty=typ(y);
  
  if(tx==1)
  {
    if(ty==1) return dvmdii(x,y,pr);
    if(ty==10) {*pr=gcopy(x);return gzero;}
    else err(gdivmoder);
  }
  if (tx==10) return poldivres(x,y,pr);
  else {err(gdivmoder);return gnil;}
}


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*                                                                 */
/*                       SHIFT D'UN GEN                            */
/*                                                                 */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/*      SHIFT TRONQUE SI n<0 (MULTIPLICATION TRONQUEE PAR 2**n)  */

GEN     gshift(GEN x, long n)
{
  long  tx,lx,i,l;
  GEN   y;
  lx=lg(x);tx=typ(x);
  if(gcmp0(x)) y=gcopy(x);
  else
    switch(tx)
    {
      case 1 :
      case 2 : y=mpshift(x,n);break;
      case 17:
      case 18:
      case 19: y=cgetg(lx,tx);l=lontyp[tx];
        for(i=1;i<l;y[i]=x[i],i++);
        for(i=l;i<lx;i++)
          y[i]=lshift((GEN)x[i],n);
        break;
      default: y=gmul2n(x,n);
    }
  return y;
}

/*      SHIFT VRAI (MULTIPLICATION EXACTE PAR 2**n)     */

GEN     gmul2n(GEN x, long n)
{
  long  tx,lx,i,l,tetpil;
  GEN   y,p1;
  lx=lg(x);tx=typ(x);
  if(gcmp0(x)) y=gcopy(x);
  else
    switch(tx)
    {
      case 1 : if(n>=0) y=mpshift(x,n);
        else
        {
          y=cgetg(3,4);y[1]=lcopy(x);
          y[2]=lmpshift(gun,-n);gredsp(&y);
        }
        break;
        
      case 2 : y=mpshift(x,n);break;
      case 4 :
      case 5 : y=cgetg(lx,tx);
        if(n>=0)
        {
          y[1]=lmpshift((GEN)x[1],n);
          y[2]=lcopy((GEN)x[2]);
        }
        else
        {
          y[2]=lmpshift((GEN)x[2],-n);
          y[1]=lcopy((GEN)x[1]);
        }
        if(tx==4) gredsp(&y);
        break;
      case 8 : y=cgetg(lx,tx);
	y[1]=copyifstack((GEN)x[1]);
        for(i=2;i<lx;i++)
          y[i]=lmul2n((GEN)x[i],n);
        break;
      case 9 : y=cgetg(lx,tx);
	y[1]=copyifstack((GEN)x[1]);
	y[2]=lmul2n((GEN)x[2],n);
	break;

      case 10: lx=lgef(x);
      case 6 :
      case 11:
      case 17:
      case 18:
      case 19: y=cgetg(lx,tx);l=lontyp[tx];
        for(i=1;i<l;y[i]=x[i],i++);
        for(i=l;i<lx;i++)
          y[i]=lmul2n((GEN)x[i],n);
        break;
      case 13: l=avma;y=cgetg(lx,tx);
        if(n>=0) {y[1]=lmul2n((GEN)x[1],n);y[2]=x[2];}
        else {y[2]=lmul2n((GEN)x[2],-n);y[1]=x[1];}
	tetpil=avma;y=gerepile(l,tetpil,gred(y));
        break;
      case 14: y=cgetg(lx,tx);
	if(n>=0) {y[1]=lmul2n((GEN)x[1],n);y[2]=lcopy((GEN)x[2]);}
        else {y[2]=lmul2n((GEN)x[2],-n);y[1]=lcopy((GEN)x[1]);}
        break;
      case 3 :
      case 7 : l=avma;p1=gmul2n(gun,n);tetpil=avma;
        y=gerepile(l,tetpil,gmul(p1,x));
        break;
      default: err(gmul2ner1);
    }
  return y;
}


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*                                                                 */
/*                    INVERSE D' UN GEN                            */
/*                                                                 */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

GEN     ginv(GEN x)
{
  long    tx=typ(x),av,tetpil;
  GEN     y;

  if(tx==16) 
    {
      y=gcopy(x);
      if(cmpii((GEN)x[1],(GEN)x[2])&&cmpii((GEN)x[1],(GEN)x[3])) setsigne((GEN)y[2],-signe((GEN)y[2]));
      return y;
    }
  if(tx==15) 
    {
      av=avma;y=gcopy(x);setsigne((GEN)y[2],-signe((GEN)y[2]));
      setsigne((GEN)y[4],-signe((GEN)y[4]));tetpil=avma;
      return gerepile(av,tetpil,redreal(y));
    }
  if (tx<15) return gdivsg(1,x);
  if (tx==19) return invmat(x);
  err(ginver);return gnil;
}


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*                                                                 */
/*           SUBSTITUTION DANS UN POLYNOME OU UNE SERIE            */
/*                                                                 */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

GEN     gsubst(GEN x, long v, GEN y)
{
  long  tx,ty,l,lx,ly,vx,vy,e,ex,ey,tetpil;
  long  av,av1,av2,av3,av4,av5,av6,av7,i,j,k,jb,decal;
  GEN   t,p1,p2,z;
  
  tx=typ(x);ty=typ(y);lx=lg(x);ly=lg(y);
  if ((ty>=15)&&(ty<=18)) err(gsubser2);
  if ((ty==19)&&(ly!=lg((GEN)y[1]))) err(gsubser3);
  if ((tx<9)||((tx==9)&&(v<=varn((GEN)x[1]))))
    if(ty==19) z=gscalmat(x,ly-1);else z=gcopy(x);
  else switch(tx)
  {
    case 10:
      if(!signe(x))
      {
        if(ty==19) z=gscalmat(gzero,ly-1);else z=gzero;
      }
      else
      {
	vx=varn(x);
        if((ty==6)&&(v==vx)) z=poleval(x,y);
        else
        {
          l=lgef(x);
          if(ty!=19)
          {
            if(vx>v) z=gcopy(x);
            else
            {
              if(vx==v)
              {
                if(l==3) z=gcopy((GEN)x[2]);
                else
                {
                  av=avma;z=(GEN)x[l-1];
                  for (i=l-1;i>3;i--)
                    z=gadd(gmul(z,y),(GEN)x[i-1]);
                  z=gmul(z,y);tetpil=avma;
                  z=gerepile(av,tetpil,gadd(z,(GEN)x[2]));
                }
              }
              else
              {
                if(l==3) z=gsubst((GEN)x[2],v,y);
                else
                {
                  av=avma;p1=polx[vx];
                  z= gsubst((GEN)x[l-1],v,y);
                  for (i=l-1;i>3;i--)
                    z=gadd(gmul(z,p1),gsubst((GEN)x[i-1],v,y));
                  z=gmul(z,p1);p2=gsubst((GEN)x[2],v,y);
                  tetpil=avma;
                  z=gerepile(av,tetpil,gadd(z,p2));
                }
              }
            }
          }
          else
          {
            if(ly!=lg((GEN)y[1])) err(gsubser3);
            if(vx>v) z=gscalmat(x,ly-1);
            else
            {
              if(vx==v)
              {
                if(l==3) z=gscalmat((GEN)x[2],ly-1);
                else
                {
                  av=avma;z=(GEN)x[l-1];
                  for (i=l-1;i>3;i--)
                    z=gaddmat((GEN)x[i-1],gmul(z,y));
                  z=gmul(z,y);tetpil=avma;
                  z=gerepile(av,tetpil,gaddmat((GEN)x[2],z));
                }
              }
              else
              {
                if(l==3) z=gsubst((GEN)x[2],v,y);
                else
                {
                  av=avma;p1=polx[vx];
                  z=gsubst((GEN)x[l-1],v,y);
                  for(i=l-1;i>3;i--)
                    z=gadd(gmul(z,p1),gsubst((GEN)x[i-1],v,y));
                  z=gmul(z,p1);p2=gsubst((GEN)x[2],v,y);
                  tetpil=avma;
                  z=gerepile(av,tetpil,gadd(p2,z));
                }
              }
            }
          }
        }
      }
      break;
      
    case 11:
      ex=valp(x);vx=varn(x);
      if(vx>v)
      {
        if(ty==19) z=gscalmat(x,ly-1);
        else z=gcopy(x);
        return z;
      }
      if(!signe(x))
      {
        if(vx<v) z=gcopy(x);
        else
        {
          z=cgetg(3,11);z[1]=evalvalp(ex*gval(y,v))+evalvarn(vx);z[2]=zero;
        }
        return z;
      }
      if(vx<v)
      {
        /* a ameliorer */
        av1=avma;setvalp(x,0);p1=gconvsp(x);setvalp(x,ex);
        p2=gsubst(p1,v,y);av2=avma;
        z=tayl(p2,vx,lx-2);
        if(ex)
        {
          p1=gpuigs(polx[vx],ex);
          av2=avma;z=gerepile(av1,av2,gmul(z,p1));
        }
        else z=gerepile(av1,av2,z);
        return z;
      }
      switch(ty)
      {
        case 11:
	ey=valp(y);vy=varn(y);
	if (ey<1)
	  {
	    z=cgetg(3,11);z[1]=evalvalp(ey*(ex+lx-2))+evalvarn(vy);z[2]=zero;
	  }
	else
	  {
	    l=(lx-2)*ey+2;
	    if (ex)
	      {if (l>ly) l=ly;}
	    else 
	      {
		if (gcmp0(y)) l=ey+2;
		else
		  {if (l>ly) l=ly+ey;}
	      }
	    if(vy!=vx)
	      {
		av=avma;z=cgetg(2,11);z[1]=HIGHVALPBIT+evalvarn(vy);
		for(i=lx-1;i>=2;i--)
		  {p1=gmul(y,z);av2=avma;z=gadd((GEN)x[i],p1);}
		if (ex)
		  {
		    p1=gpuigs(y,ex);av2=avma;
		    z=gerepile(av,av2,gmul(z,p1));
		  }
		else z=gerepile(av,av2,z);
	      }
	    else
	      {
		av1=avma;t=cgetg(ly,11);
		av2=avma;z=cgetg(l,11);
		z[2] =lcopy((GEN)x[2]);
		for (i=3;i<l;i++) z[i]=zero;
		for (i=2;i<ly;i++) t[i]=y[i];
		
		for (i=3,jb=ey;jb<=l-2;i++,jb+=ey)
		  {
		    for (j=jb+2;j<l;j++)
		      {
			av4=avma;
			p1=gmul((GEN)x[i],(GEN)t[j-jb]);
			av5=avma;
			z[j]=lpile(av4,av5,gadd((GEN)z[j],p1));
			if (j==jb+ey+1) av=avma;
		      }
		    for (j=l-1-jb-ey;j>1;j--)
		      {
			av4=avma;p1=gzero;
			for (k=2;k<j;k++)
			  p1=gadd(p1,gmul((GEN)t[j-k+2],(GEN)y[k]));
			p2=gmul((GEN)t[2],(GEN)y[j]);
			av5=avma;
			t[j]=lpile(av4,av5,gadd(p1,p2));
		      }
		    if (i>3)
		      {
			av7=avma;decal=lpile(av3,av6,0);
			for (k=jb+2;k<l;k++)
			  if (adecaler((GEN)z[k],av6,av7)) z[k]+=decal;
			for (k=2;k<l-jb-ey+2;k++)
			  if (adecaler((GEN)t[k],av6,av7)) t[k]+=decal;
			av3=av+decal;
		      }
		    else av3=av;
		    av6=avma;
		  }
		z[1]=evalsigne(1)+HIGHVALPBIT+evalvarn(varn(y));
		if (ex)
		  {
		    if (l<ly) setlg(y,l);
		    p1=gpuigs(y,ex);
		    av2=avma;
		    z=gerepile(av1,av2,gmul(z,p1));
		    if (l<ly) setlg(y,ly);
		  }
		else z=gerepile(av1,av2,z);
	      }
	  }
          break;
          
        case 10:
        case 13:
        case 14:
          if(gcmp0(y))
          {
            z=cgetg(lx,tx);z[1]=evalsigne(1)+HIGHVALPBIT+evalvarn(v);
            z[2]=lcopy((GEN)x[2]);for(i=3;i<lx;i++) z[i]=zero;
          }
          else
          {
            e=gval(y,vy=gvar(y));if(e<=0) err(gsubser5);
	    av=avma;p1=gconvsp(x);p2=gsubst(p1,v,y);tetpil=avma;
	    z=gerepile(av,tetpil,tayl(p2,vy,e*(lx-2+ex)));
          }
          break;
        default: err(gsubser4);
        }
      break;
    case 9:
      z=cgetg(lx,tx);z[1]=lsubst((GEN)x[1],v,y);av=avma;
      p1=gsubst((GEN)x[2],v,y);tetpil=avma;
      z[2]=lpile(av,tetpil,gmod(p1,(GEN)z[1]));
      break;
/*  case 9: err(gsubser1); */
    case 13:
    case 14:
      av=avma;p1=gsubst((GEN)x[1],v,y);p2=gsubst((GEN)x[2],v,y);
      tetpil=avma;z=gerepile(av,tetpil,gdiv(p1,p2));
      break;
    case 17:
    case 18:
    case 19:
      z=cgetg(lx,tx);
      for(i=1;i<lx;i++)
      z[i]=lsubst((GEN)x[i],v,y);
  }
  return z;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*                                                                 */
/*                SERIE RECIPROQUE D'UNE SERIE                     */
/*                                                                 */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

GEN     recip(GEN x)
{
  
  long  lx,av1,av2,av3,av4,av5,av6,av7,av8,i,j,k,v,decal;
  GEN   a,y,u,p1,p2;
  
  if ((typ(x)!=11) || (valp(x)!=1)) err(reciper);
  /* attention au coeff directeur */
  a=(GEN)x[2];v=varn(x);
  if (gcmp1(a))
  {
    lx=lg(x);av1=avma;u=cgetg(lx,11);
    av2=avma;y=cgetg(lx,11);
    u[2]=un;y[2]=un;
    av5=avma;u[3]=lmulsg(-2,(GEN)x[3]);
    av6=avma;y[3]=lneg((GEN)x[3]);
    av7=avma;i=2;
    while (++i<lx-1)
    {
      for (j=3;j<i+1;j++)
      {
        av3=avma;p1=(GEN)u[j];
        for (k=j-1;k>2;k--)
          p1=gsub(p1,gmul((GEN)u[k],(GEN)x[j-k+2]));
        av4=avma;
        u[j]=lpile(av3,av4,gsub(p1,(GEN)x[j]));
      }
      av3=avma;p1=gmulsg(i,(GEN)x[i+1]);
      for (k=2;k<i;k++)
      {
        p2=gmul((GEN)x[k+1],(GEN)u[i-k+2]);
        p1=gadd(p1,gmulsg(k,p2) );
      }
      av4=avma;u[i+1]=lpile(av3,av4,gneg(p1));
      av8=avma;decal=lpile(av5,av6,0);
      for (k=3;k<i+2;k++)
        if (adecaler((GEN)u[k],av6,av8)) u[k]+=decal;
      if(adecaler((GEN)y[i],av6,av8)) y[i]+=decal;
      av6=avma;av5=av7+decal;
      y[i+1]=ldivgs((GEN)u[i+1],i);av7=avma;
    }
    y[lx-1]=lpile(av5,av6,(GEN)y[lx-1]);
    y[1]=evalsigne(1)+evalvalp(1)+evalvarn(v);y=gerepile(av1,av2,y);
  }
  else
  {
    p1=(GEN)x[2];av1=avma;y=gdiv(x,p1);
    y=recip(y);p2=gdiv(polx[v],p1);av2=avma;
    y=gerepile(av1,av2,gsubst(y,v,p2));
  }
  return y;
}


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*                                                                 */
/*                    DERIVATION ET INTEGRATION                    */
/*                                                                 */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

GEN     deriv(GEN x, long v)
{
  long  lx,ly,vx,tx,e,i,j,tetpil,l,l1,f;
  GEN   y,p1,p2;
  
  tx=typ(x);if(tx<9) return gzero;
  switch(tx)
    {
    case 9: if(v<=varn((GEN)x[1])) return gzero;
      y=cgetg(3,9);y[1]=copyifstack((GEN)x[1]);y[2]=lderiv((GEN)x[2],v);
      return y;break;
    case 10: vx=varn(x);
      if(vx>v) return gzero;
      lx=lgef(x)-1;
      if(vx<v)
      {
        l=avma;
        for(i=lx;(i>=2)&&(isexactzero(p1=deriv((GEN)x[i],v)));avma=l,i--);
        y=cgetg(i+1,10);
        if(i==1) y[1]=evallgef(2)+evalvarn(vx);
        else
        {
	  y[i]=(long)p1;f=gcmp0(p1);
          for(j=2;j<i;j++) {y[j]=lderiv((GEN)x[j],v);if(f) f=gcmp0((GEN)y[j]);}
	  y[1]=f?evallgef(i+1)+evalvarn(vx):evalsigne(1)+evallgef(i+1)+evalvarn(vx);
        }
        return y;
      }
      if(lx<3) y=gzero;
      else
      {
        l=avma;
        for(i=lx-1;(i>=2)&&isexactzero(p1=gmulsg(i-1,(GEN)x[i+1]));avma=l,i--);
        y=cgetg(i+1,tx);
        if(i==1) y[1]=evallgef(2)+evalvarn(v);
        else
        {
	  y[i]=(long)p1;f=gcmp0(p1);
          for(j=i-1;j>=2;j--) 
	    {y[j]=lmulsg(j-1,(GEN)x[j+1]);if(f) f=gcmp0((GEN)y[j]);}
          y[1]=f?evallgef(i+1)+evalvarn(v):evalsigne(1)+evallgef(i+1)+evalvarn(v);
        }
      }
      break;
    case 11: vx=varn(x);
      if(vx>v) return gzero;
      lx=lg(x);e=valp(x);
      if(vx<v)
      {
        if(!signe(x)) y=gcopy(x);
        else
        {
          l=avma;
          for(i=2;(i<lx)&&(gcmp0(p1=deriv((GEN)x[i],v)));avma=l,i++);
          if(i==lx) y=ggrando(polx[vx],e+lx-2);
          else
          {
            y=cgetg(lx-i+2,11);y[1]=evalvalp(e+i-2)+evalvarn(vx);
            y[2]=(long)p1;
            for(j=3;j<=lx-i+1;j++)
              y[j]=lderiv((GEN)x[i+j-2],v);
          }
        }
        return y;
      }
      ly=lx-1;if(ly<3) ly=3;
      if(gcmp0(x))
        {y=cgetg(3,tx);y[1]=evalvalp(e-1)+evalvarn(vx);}
      else
      {
        if(e)
        {
          y=cgetg(lx,tx);y[1]=evalsigne(1)+evalvalp(e-1)+evalvarn(vx);
          for(i=2;i<lx;i++) y[i]=lmulsg(i+e-2,(GEN)x[i]);
        }
        else
        {
          i=3;while((i<lx)&&gcmp0((GEN)x[i])) i++;
          if(i==lx) {y=cgetg(ly,tx);y[1]=evalvalp(lx-3)+evalvarn(vx);}
          else
          {
            ly=ly-i+3;y=cgetg(ly,tx);
            y[1]=evalsigne(1)+evalvalp(i-3)+evalvarn(vx);
            for(j=2;j<ly;j++) y[j]=lmulsg(j+i-4,(GEN)x[i+j-2]);
          }
        }
      }
      break;
    case 13:
    case 14: l=avma;y=cgetg(3,tx);y[2]=lmul((GEN)x[2],(GEN)x[2]);
      l1=avma;p1=gmul((GEN)x[2],deriv((GEN)x[1],v));p2=gmul((GEN)x[1],deriv((GEN)x[2],v));
      tetpil=avma;p1=gsub(p1,p2);y[1]=lpile(l1,tetpil,p1);
      if(tx==13) {tetpil=avma;y=gerepile(l,tetpil,gred(y));}
      break;
    case 17:
    case 18:
    case 19: lx=lg(x);y=cgetg(lx,tx);
      for(i=1;i<lx;i++)
        y[i]=lderiv((GEN)x[i],v);
      break;
    default: break;
    }
  return y;
}


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*                                                                 */
/*                    INTEGRATION FORMELLE                         */
/*                                                                 */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

GEN     integ(GEN x, long v)
{
  long  lx,tx,e,i,j,vx;
  GEN   y;
  
  tx=typ(x);
  if((tx<9)||((tx==9)&&(v<=varn((GEN)x[1]))))
  {
    if(gcmp0(x)) y=gzero;
    else
    {
      y=cgetg(4,10);y[1]=evalsigne(1)+evallgef(4)+evalvarn(v);
      y[2]=zero;y[3]=lcopy(x);
    }
    return y;
  }
  switch(tx)
    {
    case 9: y=cgetg(3,9);y[1]=copyifstack((GEN)x[1]);y[2]=linteg((GEN)x[2],v);
      return y;break;
    case 10: vx=varn(x);lx=lgef(x);
      if(lx==2)
      {
        y=cgetg(3,10);y[1]=evallgef(2)+evalvarn(min(v,vx));return y;
      }
      if(vx>v)
      {
        y=cgetg(4,10);
	y[1]=(signe(x))?evalsigne(1)+evallgef(4)+evalvarn(v):evallgef(4)+evalvarn(v);
        y[2]=zero;y[3]=lcopy(x);
        return y;
      }
      if(vx<v)
      {
        y=cgetg(lx,10);y[1]=x[1];
        for(i=2;i<lx;i++)
          y[i]=linteg((GEN)x[i],v);
        return y;
      }
      y=cgetg(lx+1,tx);y[2]=zero;
      for(i=3;i<=lx;i++)
        y[i]=ldivgs((GEN)x[i-1],i-2);
      y[1]=signe(x)?evalsigne(1)+evallgef(lx+1)+evalvarn(v):evallgef(lx+1)+evalvarn(v);
      break;
    case 11: lx=lg(x);e=valp(x);vx=varn(x);
      if(!signe(x))
      {
        y=cgetg(3,tx);
        if(vx==v) y[1]=evalvalp(e+1);
        else y[1]=x[1];
        if(vx>v) setvarn(y,v);
        return y;
      }
      if(vx>v)
      {
        y=cgetg(4,10);y[1]=evalsigne(1)+evallgef(4)+evalvarn(v);
        y[2]=zero;y[3]=lcopy(x);
        return y;
      }
      if(vx<v)
      {
        y=cgetg(lx,tx);y[1]=x[1];
        for(i=2;i<lx;i++)
          y[i]=linteg((GEN)x[i],v);
        return y;
      }
      y=cgetg(lx,tx);
      for(i=2;i<lx;i++)
      {
        if((!(j=i+e-1))&&(!gcmp0((GEN)x[i]))) err(inter2);
        y[i]=j?ldivgs((GEN)x[i],j):zero;
      }
      y[1]=x[1]+1;
      break;
    case 13:
    case 14: err(impl,"integration of a rational function");
    case 17:
    case 18:
    case 19: lx=lg(x);y=cgetg(lx,tx);
      for(i=1;i<lx;i++)
        y[i]=linteg((GEN)x[i],v);
      break;
    default: err(inter1);
  }
  return y;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*                                                                 */
/*                    PARTIES ENTIERES                             */
/*                                                                 */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

GEN     gfloor(GEN x)
{
  GEN   y,p1;
  long  i,lx,tx,av,tetpil;
  
  switch(tx=typ(x))
  {
    case 1 :
    case 10: y=gcopy(x);break;
    case 2 : y=mpent(x);break;
    case 4 :
    case 5 : av=avma;y=dvmdii((GEN)x[1],(GEN)x[2],&p1);
      i=!gcmp0(p1);cgiv(p1);
      if(i&&(gsigne(x)<0))
      {
        tetpil=avma;y=gerepile(av,tetpil,gsubgs(y,1));
      }
      break;
    case 13:
    case 14: y=gdeuc((GEN)x[1],(GEN)x[2]);
      break;
    case 17:
    case 18:
    case 19: lx=lg(x);y=cgetg(lx,tx);
      for(i=1;i<lx;i++)
        y[i]=lfloor((GEN)x[i]);
      break;
    default: err(flooer);
  }
  return y;
}

GEN     gfrac(GEN x)
{
  long  av,tetpil;
  GEN   p1;
  
  av=avma;p1=gfloor(x);tetpil=avma;
  return gerepile(av,tetpil,gsub(x,p1));
}

GEN     gceil(GEN x)
{
  GEN     y,p1;
  long  i,lx,tx=typ(x),av,tetpil;
  
  switch(tx)
  {
    case 1 :
    case 10: y=gcopy(x);break;
    case 2 : av=avma;y=mpent(x);
      if(!gegal(x,y))
      {
        tetpil=avma;y=gerepile(av,tetpil,gaddsg(1,y));
      }
      break;
    case 4 :
    case 5 : av=avma;y=dvmdii((GEN)x[1],(GEN)x[2],&p1);
      i=!gcmp0(p1);cgiv(p1);
      if(i&&(gsigne(x)>0))
      {
        tetpil=avma;y=gerepile(av,tetpil,gaddsg(1,y));
      }
      break;
    case 13:
    case 14: y=gdeuc((GEN)x[1],(GEN)x[2]);
      break;
    case 17:
    case 18:
    case 19: lx=lg(x);y=cgetg(lx,tx);
      for(i=1;i<lx;i++)
        y[i]=lceil((GEN)x[i]);
      break;
    default: err(ceiler);
  }
  return y;
}

GEN     ground(GEN x)
{
  GEN     y,p1;
  long  i,lx=lg(x),tx=typ(x),av,tetpil;
  
  switch(tx)
  {
    case 1 :
    case 3 :
    case 8 : y=gcopy(x);break;
    case 2 : 
    case 4 :
    case 5 : av=avma;p1=gadd(ghalf,x);tetpil=avma;
      y=gerepile(av,tetpil,gfloor(p1));break;
    case 9 : y=cgetg(3,9);y[1]=copyifstack((GEN)x[1]);y[2]=lround((GEN)x[2]);
      break;
    case 10: lx=lgef(x);
    case 6 :
    case 11:
    case 13:
    case 14: 
    case 17:
    case 18:
    case 19: lx=lg(x);y=cgetg(lx,tx);
      for(i=1;i<lontyp[tx];i++) y[i]=x[i];
      for(i=lontyp[tx];i<lx;i++) y[i]=lround((GEN)x[i]);
      break;
    default: err(rounder);
  }
  return y;
}

GEN     grndtoi(GEN x, long *e)
{
  GEN     y,p1;
  long  i,lx=lg(x),tx=typ(x),av,tetpil,ex,e1;
  
  *e= -((long)HIGHEXPOBIT);
  switch(tx)
  {
    case 1 :
    case 3 :
    case 8 : y=gcopy(x);break;
    case 2 : av=avma;p1=gadd(ghalf,x);
      ex=expo(p1);
      if(ex<0)
      {
	if(signe(p1)>=0) {*e=expo(x);avma=av;y=gzero;}
	else 
	  {
	    tetpil=avma;y=gerepile(av,tetpil,gneg(gun));
	    av=avma;*e=expo(addsr(1,x));avma=av;
	  }
      }
      else
      {
        e1=ex-((lx-2)<<TWOPOTBITS_IN_LONG)+1;settyp(p1,1);setlgef(p1,lx);
        if(signe(x)>=0) {tetpil=avma;y=gerepile(av,tetpil,shifti(p1,e1));}
        else {y=shifti(p1,e1);tetpil=avma;y=gerepile(av,tetpil,addsi(-1,y));}
	if(e1<=0) {av=avma;*e=expo(subri(x,y));avma=av;}
	else *e=e1;
      }
      break;
    case 4 :
    case 5 : av=avma;p1=gadd(ghalf,x);tetpil=avma;
      y=gerepile(av,tetpil,gfloor(p1));break;
    case 9 : y=cgetg(3,9);y[1]=copyifstack((GEN)x[1]);y[2]=lrndtoi((GEN)x[2],e);
      break;
    case 10: lx=lgef(x);
    case 6 :
    case 11:
    case 13:
    case 14: 
    case 17:
    case 18:
    case 19: lx=lg(x);y=cgetg(lx,tx);
      for(i=1;i<lontyp[tx];i++)
        y[i]=x[i];
      for(i=lontyp[tx];i<lx;i++)
      {
        y[i]=lrndtoi((GEN)x[i],&e1);
        if(e1>*e) *e=e1;
      }
      break;
    default: err(rndtoier);
  }
  return y;
}

long rounderror(GEN x)
{
  long e,av=avma;

  grndtoi(x,&e);avma=av;return (long)(e*L2SL10);
}

GEN     gcvtoi(GEN x, long *e)
     
     /* la variable formelle e,representant le nombre de bits d'erreur */
     /* sur la partie entiere,n'est utilisee que dans le cas 2 (reel) */
     
{
  GEN     y,p1;
  long  tx=typ(x),lx=lg(x),i,ex,av,tetpil,e1;
  
  *e= -1048576;
  switch(tx)
  {
    case 1 :
    case 10: y=gcopy(x);break;
    case 2 : ex=expo(x);
      if(ex<0) {*e=ex;y=gzero;}
      else
      {
	e1=ex-((lx-2)<<TWOPOTBITS_IN_LONG)+1;
        av=avma;p1=gcopy(x);settyp(p1,1);setlgef(p1,lx);
        tetpil=avma;y=gerepile(av,tetpil,shifti(p1,e1));
	if(e1<=0) {av=avma;*e=expo(subri(x,y));avma=av;}
	else *e=e1;
      }
      break;
    case 4 :
    case 5 : y=divii((GEN)x[1],(GEN)x[2]);  break;
    case 7 : y=gconvpe(x);break;
    case 13:
    case 14: y=gdeuc((GEN)x[1],(GEN)x[2]);  break;
    case 11: y=gconvsp(x);break;
    case 17:
    case 18:
    case 19: y=cgetg(lx,tx);
      for(i=1;i<lx;i++)
      {
        y[i]=lcvtoi((GEN)x[i],&e1);
        if(e1>*e) *e=e1;
      }
      break;
    default: err(cvtoier);
  }
  return y;
}

GEN     gtrunc(GEN x)
{
  GEN     y;
  long  tx=typ(x),lx=lg(x),i;
  switch(tx)
  {
    case 1 :
    case 10: y=gcopy(x);break;
    case 2 : y=mptrunc(x);break;
    case 4 :
    case 5 : y=divii((GEN)x[1],(GEN)x[2]); break;
    case 7 : y=gconvpe(x);break;
    case 13:
    case 14: y=gdeuc((GEN)x[1],(GEN)x[2]); break;
    case 11: y=gconvsp(x);break;
    case 17:
    case 18:
    case 19: y=cgetg(lx,tx);
      for(i=1;i<lx;i++)
        y[i]=ltrunc((GEN)x[i]);
      break;
    default: err(truncer);
  }
  return y;
}

GEN gtopoly(GEN x, long v)
{
  long tx=typ(x),lx,i,j;
  GEN y;
  
  if(gcmp0(x)) {y=cgetg(2,10);y[1]=evallgef(2)+evalvarn(v);return y;}
  if(tx<10)
    {
      y=cgetg(3,10);y[1]=evalsigne(1)+evallgef(3)+evalvarn(v);y[2]=lcopy(x);
      return y;
    }
  switch(tx)
    {
    case 10: y=gcopy(x);break;
    case 11: y=gconvsp(x);break;
    case 13:
    case 14: y=gdeuc((GEN)x[1],(GEN)x[2]);break;
    case 15:
    case 16:
    case 17:
    case 18:
    case 19: lx=lg(x);i=1;while((i<lx)&&gcmp0((GEN)x[i])) i++;
      y=cgetg(lx-i+2,10);y[1]=evalsigne(1)+evallgef(2+lx-i);
      for(j=2;j<=lx-i+1;j++) y[j]=lcopy((GEN)x[lx+1-j]);
    }
  setvarn(y,v);return y;
}

GEN gtopolyrev(GEN x, long v)
{
  long tx=typ(x),lx,i,j;
  GEN y;
  
  if(gcmp0(x)) {y=cgetg(2,10);y[1]=evallgef(2)+evalvarn(v);return y;}
  if(tx<10)
    {
      y=cgetg(3,10);y[1]=evalsigne(1)+evallgef(3)+evalvarn(v);y[2]=lcopy(x);
      return y;
    }
  switch(tx)
    {
    case 10: y=gcopy(x);break;
    case 11: y=gconvsp(x);break;
    case 13:
    case 14: y=gdeuc((GEN)x[1],(GEN)x[2]);break;
    case 15:
    case 16:
    case 17:
    case 18:
    case 19: lx=lg(x);i=1;while((i<lx)&&gcmp0((GEN)x[lx-i])) i++;
      y=cgetg(lx-i+2,10);y[1]=evalsigne(1)+evallgef(2+lx-i);
      for(j=2;j<=lx-i+1;j++) y[j]=lcopy((GEN)x[j-1]);
    }
  setvarn(y,v);return y;
}

GEN gtoser(GEN x, long v)
{
  long tx=typ(x),lx,i,j,l,av,tetpil;
  GEN y,p1,p2;
  
  if(tx==11) {y=gcopy(x);setvarn(y,v);return y;}
  if(gcmp0(x)) {y=cgetg(2,11);y[1]=evalvalp(precdl)+evalvarn(v);return y;}
  if(tx<10)
    {
      y=cgetg(precdl+2,11);y[1]=evalsigne(1)+HIGHVALPBIT+evalvarn(v);
      y[2]=lcopy(x);for(i=3;i<=precdl+1;i++) y[i]=zero;
      return y;
    }
  switch(tx)
    {
    case 10: lx=lgef(x);i=2;while((i<lx)&&gcmp0((GEN)x[i])) i++;
      l=lx-i;if(precdl>l) l=precdl;
      y=cgetg(l+2,11);y[1]=evalsigne(1)+HIGHVALPBIT-2+i;
      for(j=2;j<=lx-i+1;j++) y[j]=lcopy((GEN)x[j+i-2]);
      for(j=lx-i+2;j<=l+1;j++) y[j]=zero;
      break;
    case 13:
    case 14: av=avma;p1=gtoser((GEN)x[1],v);p2=gtoser((GEN)x[2],v);
      tetpil=avma;y=gerepile(av,tetpil,gdiv(p1,p2));break;
    case 15:
    case 16:
    case 17:
    case 18:
    case 19: lx=lg(x);i=1;while((i<lx)&&gcmp0((GEN)x[i])) i++;
      y=cgetg(lx-i+2,11);y[1]=evalsigne(1)+HIGHVALPBIT-1+i;
      for(j=2;j<=lx-i+1;j++) y[j]=lcopy((GEN)x[j+i-2]);
    }
  setvarn(y,v);return y;
}

GEN gtovec(GEN x)
{
  long tx=typ(x),lx,i;
  GEN y;
  
  if((tx<10)||(tx==13)||(tx==14))
    {y=cgetg(2,17);y[1]=lcopy(x);return y;}
  if(tx>=15)
    {
      lx=lg(x);y=cgetg(lx,17);
      for(i=1;i<lx;i++) y[i]=lcopy((GEN)x[i]);
      return y;
    }
  if(tx==10)
    {
      lx=lgef(x);y=cgetg(lx-1,17);
      for(i=1;i<=lx-2;i++) y[i]=lcopy((GEN)x[lx-i]);
      return y;
    }
  if(!signe(x)) {y=cgetg(2,17);y[1]=zero;return y;}
  lx=lg(x);y=cgetg(lx-1,17);
  for(i=1;i<=lx-2;i++) y[i]=lcopy((GEN)x[i+1]);
  return y;
}
      
GEN     compo(GEN x, long n)
{
  long l,lx=lg(x),tx=typ(x);

  if((tx==10)&&((n+1)>=lgef(x))) return gzero;
  if((tx==11)&&(!signe(x))) return gzero;
  l=lontyp[tx]+n-1;
  if((l>=lx) || (n<1)) err(compoer1);
  return gcopy((GEN)x[l]);
}

GEN truecoeff(GEN x, long n)
{
  long tx=typ(x),lx,ex;

  if(tx<10)
    {if(n) return gzero;else return gcopy(x);}
  switch(tx)
    {
    case 15: case 16: case 17: case 18: case 19:
      if((n<=0)||(n>=lg(x))) err(compoer1);
      return gcopy((GEN)x[n]);break;
    case 10:
      if((n<0)||(n>=lgef(x)-2)) return gzero;
      return gcopy((GEN)x[n+2]);break;
    case 11:
      if(!signe(x)) return gzero;
      lx=lg(x);ex=valp(x);
      if(n<ex) return gzero;
      if(n>=ex+lx-2) err(compoer1);
      return gcopy((GEN)x[n-ex+2]);break;
    default: err(compoer1);return gnil;
    }
}

GEN  denom(GEN x)
{
  long i,av,tetpil;
  GEN s,t;

  switch(typ(x))
    {
    case 1: case 2: case 3: case 7: case 11: return gun;
    case 4: case 5: return absi((GEN)x[2]);
    case 6: av=avma;t=denom((GEN)x[1]);s=denom((GEN)x[2]);tetpil=avma;
      return gerepile(av,tetpil,glcm(s,t));
    case 8: av=avma;t=denom((GEN)x[2]);s=denom((GEN)x[3]);tetpil=avma;
      return gerepile(av,tetpil,glcm(s,t));
    case 9: return denom((GEN)x[2]);
    case 13: case 14: return gcopy((GEN)x[2]);
    case 10: return polun[varn(x)];
    case 17: case 18: case 19: av=tetpil=avma;
      if(lg(x)==1) return gun;
      s=denom((GEN)x[1]);
      for(i=2;(i<lg(x));i++) {t=denom((GEN)x[i]);tetpil=avma;s=glcm(s,t);}
      return gerepile(av,tetpil,s);
    default: err(denomer1);return gnil;
    }
}

GEN  numer(GEN x)
{
  long av,tetpil;
  GEN s;

  switch(typ(x))
    {
    case 1: case 10: case 2: case 3: case 7: case 11: return gcopy(x);
    case 4: case 5: return (signe((GEN)x[2])>0) ? gcopy((GEN)x[1]) : gneg((GEN)x[1]);
    case 9: av=avma;s=numer((GEN)x[2]);tetpil=avma;return gerepile(av,tetpil,gmodulcp(s,(GEN)x[1]));
    case 13: case 14: return gcopy((GEN)x[1]);
    case 6: case 8: case 17: case 18: case 19: av=avma;s=denom(x);tetpil=avma;
      return gerepile(av,tetpil,gmul(s,x));
    default: err(numerer1);return gnil;
    }
}

GEN lift(GEN x)
{
  long lx,tx=typ(x),i;
  GEN y;

  switch(tx)
    {
    case 1: return gcopy(x);
    case 3:
    case 9: return gcopy((GEN)x[2]);
    case 11: if(!signe(x)) return gcopy(x);
      y=cgetg(lx=lg(x),tx);y[1]=x[1];
      for(i=2;i<lx;i++) y[i]=(long)lift((GEN)x[i]);break;
    case 4:
    case 5:
    case 6:
    case 13:
    case 14:
    case 17:
    case 18:
    case 19: y=cgetg(lx=lg(x),tx);
      for(i=1;i<lx;i++) y[i]=(long)lift((GEN)x[i]);
      break;
    case 10: y=cgetg(lx=lgef(x),tx);y[1]=x[1];
      for(i=2;i<lx;i++) y[i]=(long)lift((GEN)x[i]);break;
    case 8: y=cgetg(4,tx);y[1]=copyifstack((GEN)x[1]);
      for(i=2;i<4;i++) y[i]=(long)lift((GEN)x[i]);break;
    default: err(lifter1);
    }
  return y;
}

GEN centerlift(GEN x)
{
  long lx,tx=typ(x),i,av;
  GEN y;

  switch(tx)
    {
    case 1: return gcopy(x);
    case 3: av=avma;i=cmpii(shifti((GEN)x[2],1),(GEN)x[1]);avma=av;
      y=(i>0)?subii((GEN)x[2],(GEN)x[1]):gcopy((GEN)x[2]);break;
    case 9: y=gcopy((GEN)x[2]);break;
    case 11: if(!signe(x)) return gcopy(x);
      y=cgetg(lx=lg(x),tx);y[1]=x[1];
      for(i=2;i<lx;i++) y[i]=(long)centerlift((GEN)x[i]);break;
    case 4:
    case 5:
    case 6:
    case 13:
    case 14:
    case 17:
    case 18:
    case 19: y=cgetg(lx=lg(x),tx);
      for(i=1;i<lx;i++) y[i]=(long)centerlift((GEN)x[i]);
      break;
    case 10: y=cgetg(lx=lgef(x),tx);y[1]=x[1];
      for(i=2;i<lx;i++) y[i]=(long)centerlift((GEN)x[i]);break;
    case 8: y=cgetg(4,tx);y[1]=copyifstack((GEN)x[1]);
      for(i=2;i<4;i++) y[i]=(long)centerlift((GEN)x[i]);break;
    default: err(lifter1);
    }
  return y;
}

GEN glt(GEN x, GEN y) { return gcmp(x,y)<0 ? gun : gzero;}
GEN gle(GEN x, GEN y) { return gcmp(x,y)<=0 ? gun : gzero;}
GEN gge(GEN x, GEN y) { return gcmp(x,y)>=0 ? gun : gzero;}
GEN ggt(GEN x, GEN y) { return gcmp(x,y)>0 ? gun : gzero;}
GEN geq(GEN x, GEN y) { return gegal(x,y) ? gun : gzero;}
GEN gne(GEN x, GEN y) { return gegal(x,y) ? gzero : gun;}
GEN gand(GEN x, GEN y) { return gcmp0(x)||gcmp0(y) ? gzero : gun;}
GEN gor(GEN x, GEN y) { return gcmp0(x)&&gcmp0(y) ? gzero : gun;}

GEN glength(GEN x)
{
  switch(typ(x))
  {
    case 1:
    case 10: return stoi(lgef(x)-2);
    case 2: return stoi(lg(x)-2);
    case 11: return signe(x) ? stoi(lg(x)-2) : gzero;
    default: return stoi(lg(x)-lontyp[typ(x)]);
  }
}

GEN matsize(GEN x)
{
  GEN y;

  switch(typ(x))
    {
    case 17: y=cgetg(3,17);y[1]=un;y[2]=lstoi(lg(x)-1);break;
    case 18: y=cgetg(3,17);y[1]=lstoi(lg(x)-1);y[2]=un;break;
    case 19: y=cgetg(3,17);y[2]=lstoi(lg(x)-1);
      y[1]=(lg(x)>1)?lstoi(lg((GEN)x[1])-1):zero;break;
    default: err(matler1);
    }
  return y;
}

GEN geval(GEN x)
{
  long av, tetpil, tx = typ(x), lx, i;
  GEN y, z;
  if (tx < 9) return gcopy(x);
  if (tx > 13)
  {
    lx = lg(x);
    y = cgetg(lx, tx);
    for(i = 1; i < lx; i++) y[i] = (long)geval((GEN)x[i]);
    return y;
  }
  switch(tx)
  {
    case 9:
      y=cgetg(3,9);y[1]=(long)geval((GEN)x[1]);av=avma;
      z=geval((GEN)x[2]);tetpil=avma;y[2]=lpile(av,tetpil,gmod(z,(GEN)y[1]));
      return y;
    case 10:
      lx = lgef(x); if (lx == 2) return gzero;
      y = gzero; z = (GEN)varentries[varn(x)]->value;
      av = avma;
      for(i = lx-1; i > 1; i--)
      {
        tetpil = avma;
        y = gadd(geval((GEN)x[i]), gmul(z, y));
      }
      return gerepile(av, tetpil, y);
    case 11: err(impl, "evaluation of a power series");
    case 13: return gdiv(geval((GEN)x[1]),geval((GEN)x[2]));
  }
  return gnil;
}

int isexactzero(GEN g)
{
  long i;
  switch (typ(g))
  {
    case 1: return !signe(g);
    case 2:
    case 7:
    case 11: return 0;
    case 3:
    case 9: return isexactzero((GEN)g[2]);
    case 4:
    case 5:
    case 13:
    case 14: return isexactzero((GEN)g[1]);
    case 6: return isexactzero((GEN)g[1])&&isexactzero((GEN)g[2]);
    case 8: return isexactzero((GEN)g[2])&&isexactzero((GEN)g[3]);
    case 10: for (i=lgef(g)-1;i>1;i--) if (!isexactzero((GEN)g[i])) return 0;
      return 1;
    case 17:
    case 18:
    case 19: for(i=1;i<lg(g);i++) if(!isexactzero((GEN)g[i])) return 0;
      return 1;
    default: return 0;
  }
}

GEN simplify(GEN x)
{
  long tx=typ(x),av,tetpil,i,lx;
  GEN p1,y;

  switch(tx)
    {
    case 1:
    case 2:
    case 3:
    case 4:
    case 7:
    case 15:
    case 16: return gcopy(x);
    case 5: return gred(x);
    case 6: if(isexactzero((GEN)x[2])) return simplify((GEN)x[1]);
      else 
	{
	  y=cgetg(lg(x),tx);y[1]=(long)simplify((GEN)x[1]);
	  y[2]=(long)simplify((GEN)x[2]);return y;
	}
    case 8: if(isexactzero((GEN)x[3])) return simplify((GEN)x[2]);
      else 
	{
	  y=cgetg(lg(x),tx);
	  y[1]=lcopy((GEN)x[1]);y[2]=(long)simplify((GEN)x[2]);
	  y[3]=(long)simplify((GEN)x[3]);return y;
	}
    case 9: y=cgetg(3,9);y[1]=(long)simplify((GEN)x[1]);
      av=avma;p1=gmod((GEN)x[2],(GEN)y[1]);tetpil=avma;
      y[2]=lpile(av,tetpil,simplify(p1));return y;
    case 10: lx=lgef(x);if(lx==2) return gzero;
      if(lx==3) return simplify((GEN)x[2]);
      y=cgetg(lx,tx);y[1]=x[1];for(i=2;i<lx;i++) y[i]=(long)simplify((GEN)x[i]);
      return y;
    case 11: if (!signe(x)) return gcopy(x);
      lx=lg(x);
      y=cgetg(lx,tx);y[1]=x[1];for(i=2;i<lx;i++) y[i]=(long)simplify((GEN)x[i]);
      return y;
    case 13: y=cgetg(3,13);y[1]=(long)simplify((GEN)x[1]);
      y[2]=(long)simplify((GEN)x[2]);return y;
    case 14: av=avma;y=gred(x);tetpil=avma;
      return gerepile(av,tetpil,simplify(y));
    case 17:
    case 18:
    case 19: lx=lg(x);y=cgetg(lx,tx);
      for(i=1;i<lx;i++) y[i]=(long)simplify((GEN)x[i]);
      return y;
    default: err(simplifyer1);return gnil;
    }
}

/* Karatsuba pour les polynomes */

GEN gmulxn(GEN p, long n, GEN *r)
              
            

/* interne: pas de verifications et objects non connexes */

{
  long i,j,lx;
  GEN y;

  if((!signe(p))||(!n)) {*r=gzero;return gcopy(p);}
  lx=lgef(p);
  if(n>0)
    {
      y=cgetg(lx+n,10);y[1]=p[1]+n;
      for(i=2;i<=n+1;i++) y[i]=zero;
      for(i=n+2;i<lx+n;i++) y[i]=p[i-n];
      return y;
    }
  else
    {
      n= -n;if(n>lx-3) {*r=p;return gzero;}
      else
	{
	  i=n+1;while((i>=2)&&gcmp0((GEN)p[i])) i--;
	  if(i==1) *r=gzero;
	  else 
	    {
	      y=cgetg(i+1,10);y[1]=p[1];setlgef(y,i+1);
	      for(j=2;j<=i;j++) y[j]=p[j];*r=y;
	    }
	  y=cgetg(lx-n,10);y[1]=p[1]-n;
	  for(i=2;i<lx-n;i++) y[i]=p[i+n];
	  return y;
	}
    }
}

GEN karamul(GEN x, GEN y, long k)
{
  long av=avma,tetpil,nx,ny,n;
  GEN p1,p2,p3,p4,p5,p6,p7;

  if((typ(x)!=10)||(typ(y)!=10)) err(karaer1);
  if(varn(x)!=varn(y)) return gmul(x,y);
  if((!signe(x))||(!signe(y))) {p1=cgetg(2,10);p1[1]=2;return p1;}
  if(k<=0) return gmul(x,y);
  nx=lgef(x)-3;ny=lgef(y)-3;n=(max(nx,ny)+1)>>1;
  p1=gmulxn(x,-n,&p2);p3=gmulxn(y,-n,&p4);
  p5=karamul(p1,p3,k-1);p6=karamul(p2,p4,k-1);
  p7=karamul(gadd(p1,p2),gadd(p3,p4),k-1);
  p1=gadd(gmulxn(p5,n<<1,&p2),gmulxn(gsub(p7,gadd(p5,p6)),n,&p2));
  tetpil=avma;return gerepile(av,tetpil,gadd(p1,p6));
}
  
/* karatsuba entier */

GEN mulxn(GEN x, long n, GEN *r)
{
  long i,j,lx;
  GEN p1,p2;

  if((!n)||(!signe(x))) {*r=gzero;return x;}
  lx=lgef(x);
  if(n>0)
    {
      *r=gzero;p1=cgeti(lx+n);
      p1[1]=x[1]+n;for(i=2;i<lx;i++) p1[i]=x[i];
      for(i=lx;i<lx+n;i++) p1[i]=0;
      return p1;
    }
  else
    {
      n= -n;
      if(lx<=n+2) {*r=x;return gzero;}
      else
	{
	  p1=cgeti(lx-n);p1[1]=x[1]-n;
	  for(i=2;i<lx-n;i++) p1[i]=x[i];
	  while((i<lx)&&(!x[i])) i++;
	  if(i==lx) *r=gzero;
	  else 
	    {
	      p2=cgeti(lx-i+2);p2[1]=evalsigne(1)+evallgef(2+lx-i);
	      for(j=2;j<=lx-i+1;j++) p2[j]=x[j+i-2];
	      *r=p2;
	    }
	  return p1;
	}
    }
}

GEN mpkaramul(GEN x, GEN y, long k)
{
  long av=avma,tetpil,lx,ly,n;
  GEN x1,x2,y1,y2,p1,p2,p3;

  if((typ(x)!=1)||(typ(y)!=1)) err(karaer2);
  lx=lgef(x)-2;ly=lgef(y)-2;
  if((lx<=2)||(ly<=2)||(k<=0)) return mulii(x,y);
  n=(max(lx,ly)+1)>>1;
  x1=mulxn(x,-n,&x2);y1=mulxn(y,-n,&y2);
  p1=mpkaramul(x1,y1,k-1);p2=mpkaramul(x2,y2,k-1);
  p3=subii(mpkaramul(addii(x1,x2),addii(y1,y2),k-1),addii(p1,p2));
  p1=addii(mulxn(p1,n<<1,&x2),mulxn(p3,n,&x2));tetpil=avma;
  return gerepile(av,tetpil,addii(p1,p2));
}

  
  
