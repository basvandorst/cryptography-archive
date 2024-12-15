/********************************************************************/
/********************************************************************/
/**                                                                **/
/**                +++++++++++++++++++++++++++++++                 **/
/**                +                             +                 **/
/**                +    OPERATIONS GENERIQUES    +                 **/
/**                +      (premiere partie)      +                 **/
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
/**                       ADDITION GENERALE                        **/
/**                                                                **/
/********************************************************************/
/********************************************************************/

GEN   gadd(GEN x, GEN y)
{
  long  lx,ly,tx,ty,i,j,k,lz,e,l,f,tz,vx,vy;
  long  tetpil,l1,a1,a2,r1,r2,d,r,l2,co;
  GEN z,p1,p2,p3,p4,p;
  
  tx=typ(x);ty=typ(y);

  if((tx<9)&&(ty<9))
    {
      if(tx>ty) {p1=x;x=y;y=p1;tz=tx;tx=ty;ty=tz;}
    }
  else
    {
      vx=gvar(x);vy=gvar(y);
      if ((vx<vy)||((vx==vy)&&(tx>ty)))
	{
	  p1=x;x=y;y=p1;tz=tx;tx=ty;ty=tz;
	  tz=vx;vx=vy;vy=tz;
	}
    }
  lx=lg(x);ly=lg(y);
  
  if (ty<10)
    {
      switch(tx)
	{
	case 1 : switch(ty)
	  {
	  case 1 :
	    
	  case 2 : z=mpadd(x,y);break;
	    
	  case 3 : z=cgetg(ly,ty);z[1]=copyifstack((GEN)y[1]);
	    l=avma;
	    p1=addii(x,(GEN)y[2]);
	    tetpil=avma;
	    p2=modii(p1,(GEN)y[1]);
	    z[2]=lpile(l,tetpil,p2);
	    break;
	    
	  case 4 :
	    
	  case 5 : z=cgetg(ly,ty);l=avma;
	    p1=mulii((GEN)y[2],x);
	    tetpil=avma;
	    p2=addii(p1,(GEN)y[1]);
	    z[1]=lpile(l,tetpil,p2);
	    z[2]=lcopy((GEN)y[2]);
	    break;
	    
	  case 6 : z=cgetg(ly,ty);
	    z[1]=ladd(x,(GEN)y[1]);
	    z[2]=lcopy((GEN)y[2]);
	    break;
	    
	  case 7 : z=gaddpex(x,y);break;
	    
	  case 8 : z=cgetg(ly,ty);
	    z[2]=ladd(x,(GEN)y[2]);
	    z[3]=lcopy((GEN)y[3]);
	    z[1]=copyifstack((GEN)y[1]);
	    break;
	    
	  case 9 : z=cgetg(ly,ty);z[2]=ladd(x,(GEN)y[2]);
	    z[1]=copyifstack((GEN)y[1]);
	    break;
	    
	  default: err(gadder1);
	    
	  } break;
	  
	case 2 : switch(ty)
	  {
	  case 2 : z=mpadd(x,y);break;
	    
	  case 3 : err(gadder2);
	    
	  case 4 :
	    
	  case 5 :
	    if(signe((GEN)y[1]))
	      {
		if(gcmp0(x))
		  {
		    lx=(expi((GEN)y[1])-expi((GEN)y[2])-expo(x))>>TWOPOTBITS_IN_LONG;
		    if(lx<0) lx=0;
		    lx+=3;z=cgetr(lx);diviiz((GEN)y[1],(GEN)y[2],z);
		  }
		else
		  {
		    l=avma;z=addir((GEN)y[1],mulir((GEN)y[2],x));tetpil=avma;
		    z=gerepile(l,tetpil,divri(z,(GEN)y[2]));
		  }
              }    
	    else z=gcopy(x);
	    break;
	    
	  case 6 : z=cgetg(ly,ty);
	    z[1]=ladd(x,(GEN)y[1]);
	    z[2]=lcopy((GEN)y[2]);
	    break;
	    
	  case 7 : err(gadder2);
          case 8 : if(gcmp0(y)) z=gcopy(x);
	  else
	    {
	      l=avma;e=gexpo(y)-expo(x);
	      if(e<0) e=0;
	      p1=co8(y,lx+(e>>TWOPOTBITS_IN_LONG));tetpil=avma;
	      z=gerepile(l,tetpil,gadd(p1,x));
	    }
	    break;
	    
	  case 9 : z=cgetg(ly,ty);z[2]=ladd(x,(GEN)y[2]);
	    z[1]=copyifstack((GEN)y[1]);
	    break;
	    
	  default: err(gadder1);
	    
	  } break;
	  
	case 3 : switch(ty)
	  {
	  case 3 : z=cgetg(ly,ty);k=x[1];l=y[1];
	    if((k==l)||gegal((GEN)k,(GEN)l))
	      z[1]=copyifstack((GEN)k);
	    else z[1]=lmppgcd((GEN)k,(GEN)l);
	    l=avma;p1=addii((GEN)x[2],(GEN)y[2]);tetpil=avma;
	    z[2]=lpile(l,tetpil,modii(p1,(GEN)z[1]));
	    break;
	    
	  case 4 :
	    
	  case 5 : z=cgetg(3,3);
	    z[1]=copyifstack((GEN)x[1]);
	    z[2]=lgeti(lgef((GEN)x[1]));
	    gaffect(y,z);gaddz(z,x,z);
	    break;
	    
	  case 6 : z=cgetg(ly,ty);z[2]=lcopy((GEN)y[2]);z[1]=ladd(x,(GEN)y[1]);break;
	    
	  case 7 : l=avma;p1=cgetg(3,3);p1[1]=x[1];p1[2]=lgeti(lgef((GEN)x[1]));
	    gaffect(y,p1);tetpil=avma;z=gerepile(l,tetpil,gadd(p1,x));
	    break;
	    
	  case 8 : z=cgetg(ly,ty);z[1]=copyifstack((GEN)y[1]);z[3]=lcopy((GEN)y[3]);
	    z[2]=ladd(x,(GEN)y[2]);break;
	    
	  case 9 : z=cgetg(ly,ty);z[2]=ladd(x,(GEN)y[2]);
	    z[1]=copyifstack((GEN)y[1]);
	    break;
	    
	  default: err(gadder1);
	    
	  } break;
	  
	case 4 :
	  
	case 5 : switch (ty)
	  {
	  case 4 :
	  case 5 : if ((tx+ty)==8) tz=4;else tz=5;
	    z=cgetg(ly,tz);l=avma;
	    p1=mulii((GEN)x[1],(GEN)y[2]);
	    p2=mulii((GEN)x[2],(GEN)y[1]);
	    tetpil=avma;
	    p3=addii(p1,p2);
	    z[1]=lpile(l,tetpil,p3);
	    z[2]=lmulii((GEN)x[2],(GEN)y[2]);
	    if (tz==4) gredsp(&z);
	    break;
	  case 6 : z=cgetg(ly,ty);
	    z[1]=ladd((GEN)y[1],x);
	    z[2]=lcopy((GEN)y[2]);
	    break;
	  case 7 : z=gaddpex(x,y);break;
	  case 8 : z=cgetg(ly,ty);
	    z[2]=ladd((GEN)y[2],x);
	    z[3]=lcopy((GEN)y[3]);
	    z[1]=lcopy((GEN)y[1]);
	    break;
	  case 9 : z=cgetg(ly,ty);z[2]=ladd(x,(GEN)y[2]);
	    z[1]=copyifstack((GEN)y[1]);
	    break;
	    
	  default: err(gadder1);
	  }
	  break;
	  
	case 6 : switch(ty)
	  {
	  case 6 : z=cgetg(ly,ty);
	    z[1]=ladd((GEN)x[1],(GEN)y[1]);
	    z[2]=ladd((GEN)x[2],(GEN)y[2]);
	    break;
	  case 7 :
	    if(krosg(-1,(GEN)y[2])== -1)
	      {
		z=cgetg(3,6);z[1]=ladd((GEN)x[1],y);
		z[2]=lcopy((GEN)x[2]);
	      }
	    else
	      {
		l=avma;
		p1=cvtop(x,(GEN)y[2],signe((GEN)y[4])?(valp(y)+precp(y)):valp(y)+1);
		tetpil=avma;z=gerepile(l,tetpil,gadd(p1,y));
	      }
	    break;
	  case 8 : lx=precision(x);if(!lx) err(gadder12);
	    if(gcmp0(y)) z=gcopy(x);
	    else
	      {
		l=avma;e=gexpo(y)-gexpo(x);
		if(e<0) e=0;
		p1=co8(y,lx+(e>>TWOPOTBITS_IN_LONG));tetpil=avma;
		z=gerepile(l,tetpil,gadd(p1,x));
	      }
	    break;
	  case 9 : z=cgetg(ly,ty);z[2]=ladd(x,(GEN)y[2]);
	    z[1]=copyifstack((GEN)y[1]);
	    break;
	    
	  default: err(gadder12);
	  }
	  break;
	  
	case 7 : switch(ty)
	  {
	  case 7 :
	    p=(GEN)x[2];
	    if(cmpii(p,(GEN)y[2])) err(gadder15);
	    a1=valp(x);a2=valp(y);e=a1;d=a2-a1;
	    if(a2<a1)
	      {
		p1=x;x=y;y=p1;e=a2;d= -d;
	      }
	    r1=precp(x);r2=precp(y);
	    if(d)
	      {
		l=avma;p1=gpuigs(p,d);tetpil=avma;
		z=cgetg(5,7);z[2]=(long)copyifstack(p);r=d+r2;
		if(r1<=r)
		  {
		    r=r1;z[3]=lcopy((GEN)x[3]);
		  }
		else z[3]=lmul(p1,(GEN)y[3]);
		z[4]=lgeti(lgef((GEN)z[3]));
		l2=avma;p1=mulii(p1,(GEN)y[4]);
		modiiz(addii(p1,(GEN)x[4]),(GEN)z[3],(GEN)z[4]);avma=l2;
		z[1]=evalprecp(r)+evalvalp(e);
		z=gerepile(l,tetpil,z);
	      }
	    else
	      {
		z=cgetg(5,7);z[2]=(long)copyifstack(p);
		l=avma;p1=addii((GEN)x[4],(GEN)y[4]);
		r=r1;if(r2<r1) {r=r2;p2=x;x=y;y=p2;}
		if(gcmp0(p1)||((co=pvaluation(p1,p,&p2))>=r))
		  {
		    avma=l;z[1]=evalvalp(e+r);z[3]=un;
		    z[4]=lgeti(lgef((GEN)x[3]));
		    affsi(0,(GEN)z[4]);
		  }
		else
		  {
		    if(co)
		      {
			p1=gpuigs(p,co);tetpil=avma;
			z[3]=ldivii((GEN)x[3],p1);
			r-=co;z[1]=evalprecp(r)+evalvalp(e+co);
			l2=lgef((GEN)z[3]);z[4]=lgeti(l2);
			modiiz(p2,(GEN)z[3],(GEN)z[4]);
			gerepile(l,tetpil,(GEN)1);
		      }
		    else
		      {
			tetpil=avma;z[4]=lgeti(lgef((GEN)x[3]));
			modiiz(p1,(GEN)x[3],(GEN)z[4]);
			z[4]=lpile(l,tetpil,(GEN)z[4]);
			z[3]=lcopy((GEN)x[3]);
			z[1]=evalprecp(r)+evalvalp(e);
		      }
		  }
	      }
	    break;
	    
	  case 8 :
	    if(kro8(y,(GEN)x[2])== -1)
	      {
		z=cgetg(4,8);z[1]=copyifstack((GEN)y[1]);
		z[2]=ladd((GEN)y[2],x);
		z[3]=lcopy((GEN)y[3]);
	      }
	    else
	      {
		l=avma;
		p1=cvtop(y,(GEN)x[2],signe((GEN)x[4])?(valp(x)+precp(x)):valp(x)+1);
		tetpil=avma;
		z=gerepile(l,tetpil,gadd(p1,x));
	      } break;
	  case 9 : z=cgetg(ly,ty);z[2]=ladd(x,(GEN)y[2]);
	    z[1]=copyifstack((GEN)y[1]);
	    break;
	  default: err(gadder1);
	  }
	  break;
	case 8 : switch(ty)
	  {
	  case 8 :k=x[1];l=y[1];
	    if ((k!=l)&&(!gegal((GEN)k,(GEN)l))) err(gadder13);
	    z=cgetg(ly,ty);z[2]=ladd((GEN)x[2],(GEN)y[2]);z[3]=ladd((GEN)x[3],(GEN)y[3]);
	    z[1]=copyifstack((GEN)l);break;
	  case 9 : z=cgetg(ly,ty);z[2]=ladd(x,(GEN)y[2]);
	    z[1]=copyifstack((GEN)y[1]);
	    break;
	  default: err(gadder1);
	  }
	  break;
        case 9 : z=cgetg(lx,tx);k=x[1];l=y[1];
	  if((k==l)||gegal((GEN)k,(GEN)l))
	    z[1]=copyifstack((GEN)k);
	  else z[1]=lgcd((GEN)k,(GEN)l);
	  l=avma;p1=gadd((GEN)x[2],(GEN)y[2]);tetpil=avma;
	  z[2]=lpile(l,tetpil,gmod(p1,(GEN)z[1]));
	  break;
        default: if(ty!=9) err(gaddbug1);
	  z=cgetg(ly,ty);z[1]=copyifstack((GEN)y[1]);
	  l=avma;p1=gadd(x,(GEN)y[2]);tetpil=avma;
	  z[2]=lpile(l,tetpil,gmod(p1,(GEN)z[1]));
	  break;
	}
    }
  else /* ici ty>=10 */
    {
      if(((vx>vy)&&((tx<17)||(ty<17)))||((vx==vy)&&(tx<10)))
	{
	  switch(ty)
	    {
	    case 10: e=lgef(y);
	      if(e==2)
		{
		  if(isexactzero(x)) 
		    {
		      z=cgetg(2,ty);z[1]=evallgef(2)+evalvarn(vy);
		    }
		  else
		    {
		      z=cgetg(3,ty);z[2]=lcopy(x);
		      z[1]=(gcmp0(x))?evallgef(3)+evalvarn(vy):evalsigne(1)+evallgef(3)+evalvarn(vy);
		    }
		}
	      else
		{
		  z=cgetg(e,ty);
		  z[2]=ladd(x,(GEN)y[2]);
		  if((e==3)&&(gcmp0((GEN)z[2])))
		    {
		      z[1]=isexactzero((GEN)z[2])?evallgef(2)+evalvarn(vy):
			evallgef(3)+evalvarn(vy);
		    }
		  else
		    {
		      z[1]=y[1];for(i=3;i<e;i++) z[i]=lcopy((GEN)y[i]);
		      normalizepol(&z);
		    }
		}
	      break;
	      
	    case 11: e=valp(y);
	      if (e<3-ly) z=gcopy(y);
	      else
		{
		  if (e<0)
		    {
		      z=cgetg(ly,ty);
		      z[2-e]=ladd(x,(GEN)y[2-e]);
		      z[1]=y[1];
		      for (i=2;i<=1-e;i++)
			z[i]=lcopy((GEN)y[i]);
		      for (i=3-e;i<ly;i++)
			z[i]=lcopy((GEN)y[i]);
		    }
		  else
		    {
		      if (e>0)
			{
			  if (gcmp0(x)) z=gcopy(y);
			  else
			    {
			      if(gcmp0(y)) lz=e+2;else lz=ly+e;
			      z=cgetg(lz,ty);
			      z[1]=evalsigne(1)+HIGHVALPBIT+evalvarn(vy);
			      z[2]=lcopy(x);
			      for (i=3;i<=e+1;i++) z[i]=zero;
			      for (i=e+2;i<lz;i++)
				z[i]=lcopy((GEN)y[i-e]);
			    }
			}
		      else
			{
			  z=cgetg(ly,ty);l=avma;
			  p1=(signe(y))?gadd(x,(GEN)y[2]):x;
			  if(isexactzero(p1))
			    {
			      avma=l;
			      if(ly==3)
				{z[2]=zero;z[1]=HIGHVALPBIT+1+evalvarn(vy);}
			      else
				{
				  i=3;
				  while ((i<ly)&&(gcmp0((GEN)y[i]))) i++;
				  if (i==ly)
				    {
				      cgiv(z);z=cgetg(3,ty);z[2]=zero;
				      z[1]=HIGHVALPBIT-2+i+evalvarn(vy);
				    }
				  else
				    {
				      cgiv(z);z=cgetg(ly-i+2,ty);
				      z[1]=evalvalp(i-2)+evalsigne(1)+evalvarn(vy);
				      for (j=2;j<=ly-i+1;j++)
					z[j]=lcopy((GEN)y[j+i-2]);
				    }
				}
			    }
			  else
			    {
			      if(!signe(y)) z[1]=HIGHVALPBIT+evalvarn(vy);
			      else
				{
				  z[1]=evalsigne(1)+HIGHVALPBIT+evalvarn(vy);
				  z[2]=(long)p1;
				  for (i=3;i<ly;i++)
				    z[i]=lcopy((GEN)y[i]);
				}
			    }
			}
		    }
		}
	      break;
	      
	    case 13: l=avma;z=cgetg(ly,ty);
	      z[1]=ladd(gmul(x,(GEN)y[2]),(GEN)y[1]);z[2]=y[2];
	      tetpil=avma;z=gerepile(l,tetpil,gred(z));
	      break;
	    case 14: z=cgetg(ly,ty);l=avma;p1=gmul(x,(GEN)y[2]);tetpil=avma;
	      z[1]=lpile(l,tetpil,gadd(p1,(GEN)y[1]));z[2]=lcopy((GEN)y[2]);
	      break;
	      
	    case 15:
	    case 16: err(gadder5);
	    case 17:
	    case 18: if(isexactzero(x)) z=gcopy(y);else err(gadder5);break;
	    case 19:
	      if(isexactzero(x)) z=gcopy(y);
	      else
		{
		  if((ly>=2)&&(lg((GEN)y[1])==ly))
		    {
		      l=avma;p1=gscalmat(x,ly-1);tetpil=avma;
		      z=gerepile(l,tetpil,gadd(p1,y));
		    }
		  else err(gadder5);
		}
	      break;
	    default: err(gadder1);
	    }
	}
      else /* ici ty>=10 et tx>=10 et vx=vy */
	{
	  if(tx>ty)
	    {p1=x;x=y;y=p1;tz=tx;tx=ty;ty=tz;}
	  switch(tx)
	    {
	    case 10: switch (ty)
	      {
	      case 10:
		lx=lgef(x);ly=lgef(y);
		if (ly>lx)
		  {p1=x;x=y;y=p1;k=lx;lx=ly;ly=k;}
		z=cgetg(lx,ty);
		for (i=2;i<ly;i++)
		  z[i]=ladd((GEN)x[i],(GEN)y[i]);
		for (i=ly;i<lx;i++)
		  z[i]=lcopy((GEN)x[i]);
		z[1]=x[1];normalizepol(&z);break;      
	      case 11:
		if (gcmp0(x)) z=gcopy(y);
		else
		  {
		    i=signe(y) ? valp(y)+ly-gval(x,vx) : valp(y)+3-gval(x,vx);
		    if(i<3) z=gcopy(y);
		    else
		      {
			l=avma;p1=greffe(x,i);
			tetpil=avma;
			p2=gadd(p1,y);z=gerepile(l,tetpil,p2);
		      }
		  }
		break;
		
	      case 13: l=avma;z=cgetg(ly,ty);
		z[1]=ladd(gmul(x,(GEN)y[2]),(GEN)y[1]);z[2]=y[2];
		tetpil=avma;z=gerepile(l,tetpil,gred(z));
		break;
	      case 14: z=cgetg(ly,ty);l=avma;p1=gmul(x,(GEN)y[2]);tetpil=avma;
		z[1]=lpile(l,tetpil,gadd(p1,(GEN)y[1]));z[2]=lcopy((GEN)y[2]);
		break;
	      case 15:
	      case 16:
	      case 17:
	      case 18:
	      case 19: err(gadder6);
	      default: err(gadder1);
	      } break;
	      
	    case 11: switch(ty)
	      {
	      case 11: e=valp(y)-valp(x);
		if(e<0)
		  {
		    e= -e;p1=x;x=y;y=p1;
		    lz=lx;lx=ly;ly=lz;
		  }
		if(gcmp0(x)) z=gcopy(x);
		else
		  {
		    if(gcmp0(y)) ly=2;
		    lz=e+ly;
		    if (lx<lz) lz=lx;
		    if(e)
		      {
			z=cgetg(lz,ty);
			z[1]=evalvalp(valp(x))+evalsigne(1)+evalvarn(vx);
			if(e<lz-2)
			  {
			    for (i=2;i<=e+1;i++) z[i]=lcopy((GEN)x[i]);
			    for(i=e+2;i<lz;i++) z[i]=ladd((GEN)x[i],(GEN)y[i-e]);
			  }
			else
			  for (i=2;i<lz;i++) z[i]=lcopy((GEN)x[i]);
		      }
		    else
		      {
			i=2;l=avma;f=1;
			while(f&&(i<lz))
			  {
			    avma=l; p1=gadd((GEN)x[i],(GEN)y[i]);
			    f=isexactzero(p1);i++;
			  }
			if(f)
			  {
			    avma=l;z=cgetg(lz,ty);
			    z[1]=evalvalp(lz-2+valp(y))+evalvarn(vx);
			  }
			else
			  {
			    z=cgetg(lz-i+3,ty);
			    z[1]=evalvalp(valp(x)+i-3)+evalsigne(1)+evalvarn(vx);
			    z[2]=(long)p1;
			    for(j=i;j<lz;j++)
			      z[j-i+3]=ladd((GEN)x[j],(GEN)y[j]);
			  }
		      }
		  }
		break;
		
	      case 15:
	      case 16:
	      case 17:
	      case 18:
	      case 19: err(gadder7);
	      case 13:
	      case 14: if(gcmp0(y)) z=gcopy(x);
	      else
		{
		  e=gval(y,vy);
		  if (gcmp0(x)) lz=valp(x)+3-e;
		  else lz=lx+valp(x)-e;
		  if(lz<3) z=gcopy(x);
		  else
		    {
		      l=avma;
		      if(typ((GEN)y[2])<10)
			p3=gdiv((GEN)y[1],(GEN)y[2]);
		      else
			{
			  p2=greffe((GEN)y[2],lz);
			  p3=gdiv((GEN)y[1],p2);
			}
		      tetpil=avma;
		      p4=gadd(p3,x);z=gerepile(l,tetpil,p4);
		    }
		} break;
		
	      default: err(gadder1);
		
	      } break;
	      
	    case 13:
	    case 14: if(ty>14) err(gadder10);
	    else
	      {
		if((tx+ty)==26) tz=13;else tz=14;
		l1=avma;z=cgetg(ly,tz);l=avma;
		p1=gmul((GEN)x[1],(GEN)y[2]);
		p2=gmul((GEN)x[2],(GEN)y[1]);
		tetpil=avma;
		p3=gadd(p1,p2);z[1]=lpile(l,tetpil,p3);
		z[2]=lmul((GEN)x[2],(GEN)y[2]);
		if(tz==13)
		  {
		    tetpil=avma;p1=gred(z);z=gerepile(l1,tetpil,p1);
		  }
	      }
	      break;
	      
	    case 15:
	    case 16: err(gadder9);
	    case 17:
	    case 18:
	    case 19: if((lx!=ly)||(tx!=ty)) err(gadder11);
	    else
	      {
		z=cgetg(ly,ty);
		for(i=1;i<ly;i++)
		  z[i]=ladd((GEN)x[i],(GEN)y[i]);
	      }
	      break;
	      
	    default: err(gadder1);
	    }
	}
    }
  return z;
}

/********************************************************************/
/********************************************************************/
/**                                                                **/
/**                   SOUSTRACTION GENERALE                        **/
/**                                                                **/
/********************************************************************/
/********************************************************************/

GEN   gsub(GEN x, GEN y)
{
  long    l,tetpil;
  GEN   z,p1,p2;
  
  l=avma;p1=gneg(y);
  tetpil=avma;
  p2=gadd(x,p1);
  z=gerepile(l,tetpil,p2);
  return z;
}

/********************************************************************/
/********************************************************************/
/**                                                                **/
/**                 MULTIPLICATION GENERALE                        **/
/**                                                                **/
/********************************************************************/
/********************************************************************/

GEN   gmul(GEN x, GEN y)
{
  long  tx,ty,tz,lx,ly,dx,dy,i,j;
  long  k,l,l1,l2,tetpil,vx,vy,vfl;
  GEN   z,p1,p2,p3,p4,p5,yy;
  
  tx=typ(x);ty=typ(y);
  if((tx<9)&&(ty<9))
    {
      if(tx>ty) {p1=x;x=y;y=p1;tz=tx;tx=ty;ty=tz;}
    }
  else
    {
      vx=gvar(x);vy=gvar(y);vfl=0;
      if(ty<17)
	{
	  if(tx>=17) vfl=1;
	  else {if((vx<vy)||((vx==vy)&&(tx>ty))) vfl=1;}
	} 
      if(vfl)
	{
	  p1=x;x=y;y=p1;tz=tx;tx=ty;ty=tz;
	  tz=vx;vx=vy;vy=tz;
	}
    }
  lx=lg(x);ly=lg(y);
  
  if (ty<10)
    {
      switch(tx)
	{
	case 1 : switch(ty)
	  {
	  case 1 :
	  case 2 : z=mpmul(x,y);break;
	  case 3 : z=cgetg(ly,ty);
	    z[1]=copyifstack((GEN)y[1]);
	    l=avma;
	    p1=mulii(x,(GEN)y[2]);
	    tetpil=avma;
	    p2=modii(p1,(GEN)y[1]);
	    z[2]=lpile(l,tetpil,p2);
	    break;
	  case 4 :
	  case 5 : z=cgetg(ly,ty);
	    z[1]=lmulii(x,(GEN)y[1]);
	    z[2]=lcopy((GEN)y[2]);
	    if (ty==4) gredsp(&z);
	    break;
	    
	  case 6 : z=cgetg(ly,ty);
	    z[1]=lmul(x,(GEN)y[1]);
	    z[2]=lmul(x,(GEN)y[2]);
	    break;
	    
	  case 7 : if(signe(x))
	    {
	      l=avma;p1=cgetp(y);gaffect(x,p1); tetpil=avma;
	      z=gerepile(l,tetpil,gmul(p1,y));
	    }
	  else z=gzero;
	    break; 
	    
	  case 8 : z=cgetg(ly,ty);
	    z[2]=lmul(x,(GEN)y[2]);
	    z[3]=lmul(x,(GEN)y[3]);
	    z[1]=copyifstack((GEN)y[1]);
	    break;
	    
	  case 9 : z=cgetg(ly,ty);z[2]=lmul(x,(GEN)y[2]);
	    z[1]=copyifstack((GEN)y[1]);
	    break;
	    
	  default: err(gmuler1);
	  } break;
	  
	case 2 : switch(ty)
	  {
	  case 2 : z=mulrr(x,y);break;
	    
	  case 3 : err(gmuler2);
	    
	  case 4 :
	    
	  case 5 : l=avma;p1=cgetr(lx);
	    tetpil=avma;gaffect(y,p1);
	    p2=mulrr(p1,x);z=gerepile(l,tetpil,p2);
	    break;
	    
	  case 6 : z=cgetg(ly,ty);
	    z[1]=lmul(x,(GEN)y[1]);
	    z[2]=lmul(x,(GEN)y[2]);
	    break;
	    
	  case 8 : l=avma;p1=co8(y,lx);tetpil=avma;
	    z=gerepile(l,tetpil,gmul(p1,x));
	    break;
	    
	  case 9 : z=cgetg(ly,ty);z[2]=lmul(x,(GEN)y[2]);
	    z[1]=copyifstack((GEN)y[1]);
	    break;
	    
	  default: err(gmuler1);
	    
	  } break;
	  
	case 3 : switch(ty)
	  {
	  case 3 : z=cgetg(ly,ty);k=x[1];l=y[1];
	    if((k==l)||gegal((GEN)k,(GEN)l))
	      z[1]=copyifstack((GEN)k);
	    else z[1]=lmppgcd((GEN)k,(GEN)l);
	    l=avma;p1=mulii((GEN)x[2],(GEN)y[2]);tetpil=avma;
	    z[2]=lpile(l,tetpil,modii(p1,(GEN)z[1]));
	    break;
	    
	  case 4 :
	    
	  case 5 : z=cgetg(3,3);
	    z[1]=copyifstack((GEN)x[1]);
	    z[2]=lgeti(lgef((GEN)x[1]));
	    gaffect(y,z);gmulz(z,x,z);
	    break;
	    
	  case 7 : l=avma;p1=cgetg(3,3);p1[1]=x[1];p1[2]=lgeti(lg((GEN)x[1]));
	    gaffect(y,p1);tetpil=avma;z=gerepile(l,tetpil,gmul(x,p1));
	    break;
	    
	  case 6 : z=cgetg(ly,ty);z[1]=lmul(x,(GEN)y[1]);z[2]=lmul(x,(GEN)y[2]);break;
	  case 8 : z=cgetg(ly,ty);z[1]=copyifstack((GEN)y[1]);
	    z[2]=lmul(x,(GEN)y[2]);z[3]=lmul(x,(GEN)y[3]);
	    break;
	    
	  case 9 : z=cgetg(ly,ty);z[2]=lmul(x,(GEN)y[2]);
	    z[1]=copyifstack((GEN)y[1]);
	    break;
	    
	  default: err(gmuler1);
	    
	  } break;
	  
	case 4 :
	  
	case 5 : switch(ty)
	  {
	  case 4 :
	    
	  case 5 : if ((tx+ty)==8) tz=4;else tz=5;
	    z=cgetg(ly,tz);
	    z[1]=lmulii((GEN)x[1],(GEN)y[1]);
	    z[2]=lmulii((GEN)x[2],(GEN)y[2]);
	    if (tz==4) gredsp(&z);
	    break;
	    
	  case 6 : z=cgetg(ly,ty);
	    z[1]=lmul((GEN)y[1],x);
	    z[2]=lmul((GEN)y[2],x);
	    break;
	    
	  case 7 : if(signe((GEN)x[1]))
	    {
	      l=avma;p1=cgetp(y);gaffect(x,p1);
	      tetpil=avma;z=gerepile(l,tetpil,gmul(p1,y));
	    }
	  else z=gzero;
	    break;
	    
	  case 8 : z=cgetg(ly,ty);
	    z[2]=lmul((GEN)y[2],x);
	    z[3]=lmul((GEN)y[3],x);
	    z[1]=copyifstack((GEN)y[1]);
	    break;
	    
	  case 9 : z=cgetg(ly,ty);z[2]=lmul(x,(GEN)y[2]);
	    z[1]=copyifstack((GEN)y[1]);
	    break;
	    
	  default: err(gmuler1);
	    
	  } break;
	  
	case 6 : switch(ty)
	  {
	  case 6 : z=cgetg(ly,ty);l=avma;
	    p1=gmul((GEN)x[1],(GEN)y[1]);
	    p2=gmul((GEN)x[2],(GEN)y[2]);
	    p3=gadd((GEN)x[1],(GEN)x[2]);
	    p4=gadd((GEN)y[1],(GEN)y[2]);
	    p5=gmul(p3,p4);
	    p3=gadd(p1,p2);
	    tetpil=avma;
	    z[2]=lsub(p5,p3);
	    z[1]=lsub(p1,p2);
	    z[1]=lpile(l,tetpil,(GEN)z[1]);
	    break;
	    
	  case 7 :
	    if(krosg(-1,(GEN)y[2])== -1)
	      {
		z=cgetg(3,6);
		z[1]=lmul((GEN)x[1],y);
		z[2]=lmul((GEN)x[2],y);
	      }
	    else
	      {
		l=avma;
		if(signe((GEN)y[4])) p1=cvtop(x,(GEN)y[2],precp(y));
		else p1=cvtop(x,(GEN)y[2],(valp(y)>0)?valp(y)+1:1);
		tetpil=avma;z=gerepile(l,tetpil,gmul(p1,y));
	      }
	    break;
	    
	  case 8 : lx=precision(x);if(!lx) err(gmuler11);
	    l=avma;p1=co8(y,lx);tetpil=avma;
	    z=gerepile(l,tetpil,gmul(p1,x));
	    break;
	    
	  case 9 : z=cgetg(ly,ty);z[2]=lmul(x,(GEN)y[2]);
	    z[1]=copyifstack((GEN)y[1]);
	    break;
	    
	  default: err(gmuler11);
	    
	  } break;
	  
	case 7 : switch(ty)
	  {
	  case 7 :
	    if(cmpii((GEN)x[2],(GEN)y[2])) err(gmuler14);
	    if(!signe((GEN)x[4])) {z=gcopy(x);setvalp(z,valp(x)+valp(y));}
	    else
	      {
		if(!signe((GEN)y[4]))
		  {
		    z=gcopy(y);setvalp(z,valp(x)+valp(y));
		  }
		else
		  {
		    p1=(precp(x)>precp(y)) ? y : x;
		    z=cgetp(p1);l=avma;
		    setvalp(z,valp(x)+valp(y));
		    modiiz(mulii((GEN)x[4],(GEN)y[4]),(GEN)p1[3],(GEN)z[4]);
		    avma=l;
		  }
	      }
	    break;
	    
	  case 8 :
	    if(kro8(y,(GEN)x[2])== -1)
	      {
		z=cgetg(4,8);z[1]=copyifstack((GEN)y[1]);
		z[2]=lmul((GEN)y[2],x);
		z[3]=lmul((GEN)y[3],x);
	      }
	    else
	      {
		l=avma;p1=cvtop(y,(GEN)x[2],signe((GEN)x[4])?precp(x):valp(x)+1);
		tetpil=avma;z=gerepile(l,tetpil,gmul(p1,x));
	      }
	    break;
	    
	  case 9 : z=cgetg(ly,ty);z[2]=lmul(x,(GEN)y[2]);
	    z[1]=copyifstack((GEN)y[1]);
	    break;
	    
	  default: err(gmuler1);
	  }
	  break;
	  
	case 8 : switch(ty)
	  {
	  case 8 : p1=(GEN)x[1];yy=(GEN)y[1];
	    if ((p1!=yy)&&(!gegal(p1,yy))) err(gmuler12);
	    z=cgetg(ly,ty);
	    if (gcmp0((GEN)p1[3]))
	      {
		l=avma;
		p2=gmul((GEN)x[2],(GEN)y[2]);
		p3=gmul((GEN)x[3],(GEN)y[3]);
		p4=gmul(gneg((GEN)p1[2]),p3);tetpil=avma;
		z[2]=lpile(l,tetpil,gadd(p4,p2));
		l=avma;
		p2=gmul((GEN)x[2],(GEN)y[3]);
		p3=gmul((GEN)x[3],(GEN)y[2]);
		tetpil=avma;
		z[3]=lpile(l,tetpil,gadd(p2,p3));
		z[1]=copyifstack(yy);
	      }
	    else
	      {
		l1=avma;p2=gmul((GEN)x[3],(GEN)y[3]);
		l2=avma;
		p3=gmul((GEN)x[2],(GEN)y[2]);
		p4=gmul(gneg((GEN)p1[2]),p2);tetpil=avma;
		z[2]=lpile(l2,tetpil,gadd(p3,p4));
		l=avma;p3=gmul((GEN)x[2],(GEN)y[3]);
		p4=gmul((GEN)x[3],(GEN)y[2]);
		p5=gadd(p4,p3);tetpil=avma;
		z[3]=lpile(l,tetpil,gadd(p5,p2));
		z[1]=copyifstack(yy);
		gerepile(l1,l2,z);
	      }
	    break;
	  case 9 : z=cgetg(ly,ty);z[2]=lmul(x,(GEN)y[2]);
	    z[1]=copyifstack((GEN)y[1]);
	    break;
	    
	  default: err(gmuler1);
	  }	  
	  break;
	  
	case 9 : z=cgetg(lx,tx);k=x[1];l=y[1];
	  if((k==l)||gegal((GEN)k,(GEN)l))
	    z[1]=copyifstack((GEN)k);
	  else z[1]=lgcd((GEN)k,(GEN)l);
	  l=avma;p1=gmul((GEN)x[2],(GEN)y[2]);tetpil=avma;
	  z[2]=lpile(l,tetpil,gmod(p1,(GEN)z[1]));
	  break;
	  
	default: if(ty!=9) err(gmulbug1);
	  z=cgetg(ly,ty);z[1]=copyifstack((GEN)y[1]);
	  l=avma;p1=gmul(x,(GEN)y[2]);tetpil=avma;
	  z[2]=lpile(l,tetpil,gmod(p1,(GEN)z[1]));
	  break;
	}
    }
  else  /* ici ty>=10 */
    {
      if(((vx>vy)&&((tx<17)||(ty<17)))||((vx==vy)&&(tx<10))||((ty>=17)&&(tx<17)))
	{
	  switch(ty)
	    {
	    case 10: z=cgetg(ly,ty);
	      if (isexactzero(x)||isexactzero(y))
		{z[1]=evallgef(2)+evalvarn(vy);}
	      else
		{
		  for (i=2;i<lgef(y);i++)
		    z[i]=lmul(x,(GEN)y[i]);
		  z[1]=y[1];
		  normalizepol(&z);
		}
	      break;
	      
	    case 11: if (gcmp0(x))
	      {
		z=cgetg(2,10);z[1]=evallgef(2)+evalvarn(vy);
	      }
	    else
	      {
		if (gcmp0(y)) z=gcopy(y);
		else
		  {
		    z=cgetg(ly,ty);
		    for (i=2;i<ly;i++)
		      z[i]=lmul(x,(GEN)y[i]);
		    z[1]=y[1];
		    normalize(&z);
		  }
	      }
	      break;
	      
	    case 13:
	      if(isexactzero(x)) {z=cgetg(2,10);z[1]=evallgef(2)+evalvarn(vy);}
	      else 
		{
		  l=avma;z=cgetg(ly,ty);z[1]=lmul(x,(GEN)y[1]);z[2]=y[2];
		  tetpil=avma;z=gerepile(l,tetpil,gred(z));
		}
	      break;
	    case 14: 
	      if(isexactzero(x)) {z=cgetg(2,10);z[1]=evallgef(2)+evalvarn(vy);}
	      else {z=cgetg(ly,ty);z[1]=lmul(x,(GEN)y[1]);z[2]=lcopy((GEN)y[2]);}
	      break;
	      
	    case 17:
	    case 18:
	    case 19: z=cgetg(ly,ty);
	      for (i=1;i<ly;i++) z[i]=lmul(x,(GEN)y[i]);
	      break;
	      
	    default: err(gmuler1);
	      
	    }
	}
      else /* ici ty>=10 et tx>=10 et vx=vy ou tx et ty>=17 */
	{
	  if((tx>ty)&&((tx<17)||(ty<17)))
	    {
	      p1=x;x=y;y=p1;tz=tx;tx=ty;ty=tz;
	      tz=lx;lx=ly;ly=tz;
	    }
	  switch(tx)
	    {
	    case 10: switch (ty)
	      {
	      case 10: if (isexactzero(x)||isexactzero(y))
		{
		  z=cgetg(2,ty);z[1]=evallgef(2)+evalvarn(vx);
		}
	      else
		{
		  dx=lgef(x);dy=lgef(y);
		  if (dx<dy)
		    {
		      p1=x;x=y;y=p1;k=lx;lx=ly;ly=k;
		      k=dx;dx=dy;dy=k;
		    }
		  k=dx+dy-3;z=cgetg(k,ty);
		  for (i=2;i<dy;i++)
		    {
		      p1=gzero;l=avma;
		      for (j=2;j<=i;j++)
			{
			  p2=gmul((GEN)y[j],(GEN)x[i-j+2]);
			  tetpil=avma;
			  p1=gadd(p1,p2);
			}
		      z[i]=lpile(l,tetpil,p1);
		    }
		  for (i=dy;i<dx;i++)
		    {
		      p1=gzero;l=avma;
		      for (j=2;j<dy;j++)
			{
			  p2=gmul((GEN)y[j],(GEN)x[i-j+2]);
			  tetpil=avma;
			  p1=gadd(p1,p2);
			}
		      z[i]=lpile(l,tetpil,p1);
		    }
		  for (i=dx;i<=dx+dy-4;i++)
		    {
		      p1=gzero;l=avma;
		      for (j=i-dx+3;j<dy;j++)
			{
			  p2=gmul((GEN)y[j],(GEN)x[i-j+2]);
			  tetpil=avma;
			  p1=gadd(p1,p2);
			}
		      z[i]=lpile(l,tetpil,p1);
		    }
		  z[1]=evalsigne(1)+evallgef(k)+evalvarn(vx);
		}
		normalizepol(&z);
		break;
		
	      case 11: if (gcmp0(x))
		{
		  z=cgetg(2,10);z[1]=evallgef(2)+evalvarn(vx);
		}
	      else
		{
		  if (gcmp0(y))
		    {
		      z=cgetg(3,11);
		      z[1]=evalvalp(valp(y)+gval(x,vx))+evalvarn(vx);
		    }
		  else
		    {
		      l=avma;p1=greffe(x,ly);
		      tetpil=avma;p2=gmul(p1,y);
		      z=gerepile(l,tetpil,p2);
		    }
		}
		break;
		
	      case 13:
	      case 14: l=avma;z=cgetg(ly,ty);z[1]=lmul(x,(GEN)y[1]);
		z[2]=lcopy((GEN)y[2]);
		if (ty==13)
		  {
		    tetpil=avma;p1=gred(z);z=gerepile(l,tetpil,p1);
		  }
		break;
		
	      case 17:
	      case 18:
	      case 19: z=cgetg(ly,ty);
		for (i=1;i<ly;i++) z[i]=lmul(x,(GEN)y[i]);
		break;
		
	      default: err(gmuler1);
		
	      } break;
	      
	    case 11: switch (ty)
	      {
	      case 11: if(lx>ly) {k=ly;ly=lx;lx=k;p1=y;y=x;x=p1;}
		if (gcmp0(x)||gcmp0(y))
		  {
		    z=cgetg(3,11);z[1]=evalvalp(valp(x)+valp(y))+evalvarn(vx);
		  }
		else
		  {
		    l=avma;p1=cgeti(lx);z=cgetg(lx,ty);
		    z[1]=evalvalp(valp(x)+valp(y))+evalvarn(vx)+evalsigne(1);
		    for (i=2;i<lx;i++)
		      {
			p1[i]=!isexactzero((GEN)y[i]);
			z[i]=p1[i]?lmul((GEN)x[2],(GEN)y[i]):zero;
		      }
		    for(i=3;i<lx;i++)
		      if(!isexactzero((GEN)x[i]))
			for(j=2;j<=lx+1-i;j++)
			  if(p1[j]) z[i+j-2]=ladd((GEN)z[i+j-2],gmul((GEN)x[i],(GEN)y[j]));
		    tetpil=avma;z=gerepile(l,tetpil,gcopy(z));
		    normalize(&z);
		  }
		break;
		
	      case 13:
	      case 14: if (gcmp0(y))
		{
		  z=cgetg(2,10);z[1]=evallgef(2)+evalvarn(vx);
		}
	      else
		{
		  if (gcmp0(x))
		    {
		      z=cgetg(3,11);
		      z[1]=evalvalp(valp(x)+gval(y,vx))+evalvarn(vx);
		    }
		  else
		    {
		      l=avma;p1=gmul((GEN)y[1],x);
		      tetpil=avma;z=gerepile(l,tetpil,gdiv(p1,(GEN)y[2]));
		    }
		}
		break;
		
	      case 17:
	      case 18:
	      case 19: z=cgetg(ly,ty);
		for (i=1;i<ly;i++)
		  z[i]=lmul(x,(GEN)y[i]);
		break;
		
	      default: err(gmuler1);
		
	      } break;
	      
	    case 13:
	    case 14: if (ty<=14)
	      {
		if (tx+ty==26) tz=13;else tz=14;
		l=avma;z=cgetg(ly,tz);
		z[1]=lmul((GEN)x[1],(GEN)y[1]);
		z[2]=lmul((GEN)x[2],(GEN)y[2]);
		if (tz==13)
		  {
		    tetpil=avma;p1=gred(z);z=gerepile(l,tetpil,p1);
		  }
	      }
	    else
	      {
		z=cgetg(ly,ty);for(i=1;i<ly;i++)
		  z[i]=lmul(x,(GEN)y[i]);
	      }
	      break;
	    case 15: if(ty!=15) err(gmuler1);
	      z=compreal(x,y);break;
	      
	    case 16: if(ty!=16) err(gmuler1);
	      z=compimag(x,y);break;
	      
	    case 17: switch(ty)
	      {
	      case 17: err(gmuler7);
		
	      case 18: if (lx!=ly) err(gmuler7);
	      else
		{
		  z=gzero;l=avma;
		  for (i=1;i<lx;i++)
		    {
		      p1=gmul((GEN)x[i],(GEN)y[i]);
		      tetpil=avma;
		      z=gadd(z,p1);
		    }
		  z=gerepile(l,tetpil,z);
		}
		break;
		
	      case 19: if(ly==1) z=cgetg(1,19);
		else
		  {
		    dy=lg((GEN)y[1]);
		    if (lx!=dy) err(gmuler8);
		    else
		      {
			z=cgetg(ly,tx);
			for (i=1;i<ly;i++)
			  {
			    p1=gzero;l=avma;
			    for (j=1;j<lx;j++)
			      {
				p2=gmul((GEN)x[j],gcoeff(y,j,i));
				tetpil=avma;
				p1=gadd(p1,p2);
			      }
			    z[i]=lpile(l,tetpil,p1);
			  }
		      }
		  }
		break;
		
	      default: err(gmuler1);
		
	      } break;
	      
	    case 18: switch(ty)
	      {
	      case 17: z=cgetg(ly,19);
		for(i=1;i<ly;i++)
		  z[i]=lmul((GEN)y[i],x);break;
		
	      case 18: err(gmuler7);
		
	      case 19: if((ly!=1)&&(lg((GEN)y[1])!=2)) err(gmuler7);
	      else
		{
		  z=cgetg(ly,19);
		  for(i=1;i<ly;i++)
		    z[i]=lmul(gcoeff(y,1,i),x);
		}
		break;
		
	      default: err(gmuler1);
		
	      } break;
	      
	    case 19: switch(ty)
	      {
	      case 17: if(lx!=2) err(gmuler7);
		z=cgetg(ly,19);
		for(i=1;i<ly;i++)
		  z[i]=lmul((GEN)y[i],(GEN)x[1]);
		break;
		
	      case 18: if (lx!=ly) err(gmuler9);
		if(lx==1) z=gcopy(x);
		else
		  {
		    dx=lg((GEN)x[1]);
		    z=cgetg(dx,ty);
		    for (i=1;i<dx;i++)
		      {
			p1=gzero;l=avma;
			for (j=1;j<ly;j++)
			  {
			    p2=gmul(gcoeff(x,i,j),(GEN)y[j]);
			    tetpil=avma;
			    p1=gadd(p1,p2);
			  }
			z[i]=lpile(l,tetpil,p1);
		      }
		  }
		break;
		
	      case 19: 
		if(lx==1)
		  {
		    if((ly!=1)&&(lg((GEN)y[1])!=1)) err(gmuler10);
		    else ly=1;
		  }
		if(ly==1) z=cgetg(ly,tx);
		else
		  {
		    dx=lg((GEN)x[1]);dy=lg((GEN)y[1]);
		    if (lx!=dy) err(gmuler10);
		    z=cgetg(ly,tx);
		    for (i=1;i<ly;i++) z[i]=lgetg(dx,18);
		    for (i=1;i<dx;i++)
		      {
			for (j=1;j<ly;j++)
			  {
			    p1=gzero;l=avma;
			    for (k=1;k<dy;k++)
			      {
				p2=gmul(gcoeff(x,i,k),gcoeff(y,k,j));
				tetpil=avma;
				p1=gadd(p1,p2);
			      }
			    coeff(z,i,j)=lpile(l,tetpil,p1);
			  }
		      }
		  }
		break;
		
	      default: err(gmuler1);
		
	      } break;
	      
	    default: err(gmuler1);
	      
	    }     
	}
    }
  return z;
}

/********************************************************************/
/********************************************************************/
/**                                                                **/
/**                       DIVISION GENERALE                        **/
/**                                                                **/
/********************************************************************/
/********************************************************************/

GEN   gdiv(GEN x, GEN y)
{
  long  lx,ly,lz,k,l,l1,i,j,s;
  long  tetpil,tx,ty,tz,vx,vy;
  GEN z,p1,p2,p3,p4,p5;
  
  if (gcmp0(y)) err(gdiver2);
  else
    {
      lx=lg(x);ly=lg(y);tx=typ(x);ty=typ(y);
      if ((tx<10)&&(ty<10))
	{
	  switch(tx)
	    {
	    case 1 : switch(ty)
	      {
	      case 1 : l=avma;z=dvmdii(x,y,&p1);
		if(!signe(p1)) cgiv(p1);
		else
		  {
		    avma=l;
		    z=cgetg(3,4);z[1]=lcopy(x);
		    z[2]=lcopy(y);
		    if(signe((GEN)z[2])<0)
		      {
			mpnegz((GEN)z[1],(GEN)z[1]);
			mpnegz((GEN)z[2],(GEN)z[2]);
		      }
		    gredsp(&z);
		  }
		break;
		
	      case 2 : z=divir(x,y);break;
		
	      case 3 : z=cgetg(ly,ty);l=avma;
		p1= mpinvmod((GEN)y[2],(GEN)y[1]);
		p2=mulii(x,p1);tetpil=avma;
		p2=modii(p2,(GEN)y[1]);
		z[1]=copyifstack((GEN)y[1]);
		z[2]=lpile(l,tetpil,p2);
		break;
		
	      case 4 :
		
	      case 5 : z=cgetg(ly,ty);
		z[1]=lmulii(x,(GEN)y[2]);
		z[2]=lcopy((GEN)y[1]);
		if(signe((GEN)z[2])<0)
		  {
		    mpnegz((GEN)z[1],(GEN)z[1]);
		    mpnegz((GEN)z[2],(GEN)z[2]);
		  }
		if (ty==4) gredsp(&z);
		break;
		
	      case 7 : if(signe(x))
		{
		  l=avma;p1=cgetp(y);gaffect(x,p1);
		  tetpil=avma;z=gerepile(l,tetpil,gdiv(p1,y));
		}
	      else z=gzero;
		break;
		
	      case 6 :
	      case 8 : l=avma;p1=gnorm(y);
		p2=gmul(x,gconj(y));tetpil=avma;
		z=gerepile(l,tetpil,gdiv(p2,p1));
		break;
		
	      case 9 : z=cgetg(ly,ty);l=avma;
		p1=ginvmod((GEN)y[2],(GEN)y[1]);
		tetpil=avma;z[2]=lmul(x,p1);
		gerepile(l,tetpil,(GEN)1);
		z[1]=copyifstack((GEN)y[1]);
		break;
		
	      default: err(gdiver1);
	      } break;
	      
	    case 2 : switch(ty)
	      {
	      case 1 : z=divri(x,y);break;
	      case 2 : z=divrr(x,y);break;
	      case 3 : err(gdiver3);
	      case 4 :
	      case 5 : l=avma;p1=cgetg(lx,tx);gaffect(y,p1);
		p2=divrr(x,p1);z=gerepile(l,(long)p1,p2);
		break;
	      case 6 : z=cgetg(ly,ty);l=avma;p1=gnorm(y);
		p2=gmul(x,(GEN)y[1]);p3=gmul(x,(GEN)y[2]);
		gnegz(p3,p3);tetpil=avma;
		z[1]=ldiv(p2,p1);
		z[2]=ldiv(p3,p1);
		gerepile(l,tetpil,(GEN)1);
		break;
	      case 7 : err(gdiver3);
	      case 8 : l=avma;p1=co8(y,lx);tetpil=avma;
		z=gerepile(l,tetpil,gdiv(x,p1));
		break;
	      case 9 : z=cgetg(ly,ty);l=avma;
		p1=ginvmod((GEN)y[2],(GEN)y[1]);
		tetpil=avma;z[2]=lmul(x,p1);
		gerepile(l,tetpil,(GEN)1);
		z[1]=copyifstack((GEN)y[1]);
		break;
	      default: err(gdiver1);
	      } break;
	    case 3 : switch(ty)
	      {
	      case 1 : z=cgetg(lx,tx);
		z[2]=lmpinvmod(y,(GEN)x[1]);l=avma;
		p1=gmul((GEN)x[2],(GEN)z[2]);
		mpmodz(p1,(GEN)x[1],(GEN)z[2]);avma=l;
		z[1]=copyifstack((GEN)x[1]);
		break;
	      case 2 : err(gdiver4);
	      case 3 : z=cgetg(ly,ty);k=x[1];l=y[1];
		if((k==l)||gegal((GEN)k,(GEN)l))
		  z[1]=copyifstack((GEN)k);
		else z[1]=lmppgcd((GEN)k,(GEN)l);
		l=avma;p1=mpinvmod((GEN)y[2],(GEN)z[1]);
		p2=mulii((GEN)x[2],p1);tetpil=avma;
		z[2]=lpile(l,tetpil,modii(p2,(GEN)z[1]));
		break;
	      case 4 : z=cgetg(lx,tx);
		z[2]=lmpinvmod((GEN)y[1],(GEN)x[1]);
		l=avma;
		p1=mulii((GEN)x[2],(GEN)y[2]);
		mpmodz(p1,(GEN)x[1],p1);
		p2=mulii(p1,(GEN)z[2]);
		mpmodz(p2,(GEN)x[1],(GEN)z[2]);avma=l;
		z[1]=copyifstack((GEN)x[1]);
		break;
	      case 5 : l=avma;
		p1=gred(y);tetpil=avma;
		p2=gdiv(x,p1);z=gerepile(l,tetpil,p2);
		break;
	      case 6 : 
	      case 8 : l=avma;p1=gnorm(y);p2=gmul(x,gconj(y));tetpil=avma;
		z=gerepile(l,tetpil,gdiv(p2,p1));break;
	      case 7 : l=avma;p1=cgetg(3,3);p1[1]=x[1];p1[2]=lgeti(lg((GEN)x[1]));
		gaffect(y,p1);tetpil=avma;z=gerepile(l,tetpil,gdiv(x,p1));
		break;
	      case 9 : z=cgetg(ly,ty);l=avma;
		p1=ginvmod((GEN)y[2],(GEN)y[1]);
		tetpil=avma;z[2]=lmul(x,p1);
		gerepile(l,tetpil,(GEN)1);
		z[1]=copyifstack((GEN)y[1]);
		break;
	      default: err(gdiver1);
	      } break;
	    case 4 :
	    case 5 : switch(ty)
	      {
	      case 1 : z=cgetg(lx,tx);z[1]=lcopy((GEN)x[1]);
		z[2]=lmul((GEN)x[2],y);
		if(signe((GEN)z[2])<0)
		  {
		    mpnegz((GEN)z[1],(GEN)z[1]);
		    mpnegz((GEN)z[2],(GEN)z[2]);
		  }
		if (tx==4) gredsp(&z);
		break;
	      case 2 : l=avma;p1=cgetg(ly,ty);gaffect(x,p1);
		p2=divrr(p1,y);z=gerepile(l,(long)p1,p2);
		break;
	      case 3 : z=cgetg(ly,ty);l=avma;
		p1=mulii((GEN)y[2],(GEN)x[2]);
		p2=mpinvmod(p1,(GEN)y[1]);
		p3=mulii(p2,(GEN)x[1]);tetpil=avma;
		z[2]=lmodii(p3,(GEN)y[1]);
		gerepile(l,tetpil,(GEN)1);
		z[1]=copyifstack((GEN)y[1]);
		break;
	      case 4 :
	      case 5 : if ((tx+ty)==8) tz=4;else tz=5;
		z=cgetg(ly,tz);
		z[1]=lmulii((GEN)x[1],(GEN)y[2]);
		z[2]=lmulii((GEN)x[2],(GEN)y[1]);
		if(signe((GEN)z[2])<0)
		  {
		    mpnegz((GEN)z[1],(GEN)z[1]);
		    mpnegz((GEN)z[2],(GEN)z[2]);
		  }
		if (tz==4) gredsp(&z);
		break;
	      case 6 : z=cgetg(ly,ty);l=avma;p1=gnorm(y);
		p2=gmul(x,(GEN)y[1]);
		p3=gmul(x,(GEN)y[2]);gnegz(p3,p3);
		tetpil=avma;
		z[1]=ldiv(p2,p1);
		z[2]=ldiv(p3,p1);
		gerepile(l,tetpil,(GEN)1);
		break;
	      case 7 : if(signe((GEN)x[1]))
		{
		  l=avma;p1=cgetp(y);gaffect(x,p1);
		  tetpil=avma;z=gerepile(l,tetpil,gdiv(p1,y));
		}
	      else z=gzero;
		break;
	      case 8 : l=avma;
		p1=gnorm(y);
		p2=gmul(x,gconj(y));tetpil=avma;
		z=gerepile(l,tetpil,gdiv(p2,p1));
		break;
	      case 9 : z=cgetg(ly,ty);l=avma;
		p1=ginvmod((GEN)y[2],(GEN)y[1]);
		tetpil=avma;z[2]=lmul(x,p1);
		gerepile(l,tetpil,(GEN)1);
		z[1]=copyifstack((GEN)y[1]);
		break;
	      default: err(gdiver1);
	      } break;
	    case 6 : switch(ty)
	      {
	      case 1 :
	      case 2 :
	      case 3 :
	      case 4 :
	      case 5 : z=cgetg(lx,tx);
		z[1]=ldiv((GEN)x[1],y);
		z[2]=ldiv((GEN)x[2],y);
		break;
	      case 6 : l=avma;p1=gnorm(y);p2=gconj(y);
		p3=gmul(x,p2);tetpil=avma;
		p4=gdiv(p3,p1);
		z=gerepile(l,tetpil,p4);
		break;
	      case 7 :
		if(krosg(-1,(GEN)y[2])== -1)
		  {
		    z=cgetg(3,6);
		    z[1]=ldiv((GEN)x[1],y);
		    z[2]=ldiv((GEN)x[2],y);
		  }
		else
		  {
		    l=avma;p1=cvtop(x,(GEN)y[2],precp(y));
		    tetpil=avma;
		    z=gerepile(l,tetpil,gdiv(p1,y));
		  }
		break;
	      case 8 : lx=precision(x);if(!lx) err(gdiver7);
		l=avma;p1=co8(y,lx);tetpil=avma;
		z=gerepile(l,tetpil,gdiv(x,p1));
		break;
	      case 9 : z=cgetg(ly,ty);l=avma;
		p1=ginvmod((GEN)y[2],(GEN)y[1]);
		tetpil=avma;z[2]=lmul(x,p1);
		gerepile(l,tetpil,(GEN)1);
		z[1]=copyifstack((GEN)y[1]);
		break;
	      default: err(gdiver1);
	      } break;
	    case 7 : switch(ty)
	      {
	      case 3 : l=avma;p1=cgetg(3,3);p1[1]=y[1];p1[2]=lgeti(lg((GEN)y[1]));
		gaffect(x,p1);tetpil=avma;z=gerepile(l,tetpil,gdiv(p1,y));
		break;
	      case 7:
		if(cmpii((GEN)x[2],(GEN)y[2])) err(gdiver19);
		if(!signe((GEN)x[4])) {z=gcopy(x);setvalp(z,valp(x)-valp(y));}
		else
		  {
		    p1=(precp(x)>precp(y)) ? y : x;
		    z=cgetp(p1);l=avma;
		    setvalp(z,valp(x)-valp(y));
		    p2=mpinvmod((GEN)y[4],(GEN)p1[3]);
		    modiiz(mulii((GEN)x[4],p2),(GEN)p1[3],(GEN)z[4]);
		    avma=l;
		  }
		break;
	      case 6:
	      case 8: l=avma;p1=gmul(x,gconj(y));
		p2=gnorm(y);tetpil=avma;
		z=gerepile(l,tetpil,gdiv(p1,p2));
		break;
	      case 9 : z=cgetg(ly,ty);l=avma;
		p1=ginvmod((GEN)y[2],(GEN)y[1]);
		tetpil=avma;z[2]=lmul(x,p1);
		gerepile(l,tetpil,(GEN)1);
		z[1]=copyifstack((GEN)y[1]);
		break;
	      default:
		l=avma;
		if(signe((GEN)x[4])) p1=cvtop(y,(GEN)x[2],precp(x));
		else p1=cvtop(y,(GEN)x[2],(valp(x)>0)?valp(x):1);
		tetpil=avma;
		z=gerepile(l,tetpil,gdiv(x,p1));
	      }
	      break;
	    case 8 : switch (ty)
	      {
	      case 1 :
	      case 3 :
	      case 4 :
	      case 5 : z=cgetg(lx,tx);
		z[1]=copyifstack((GEN)x[1]);
		for (i=2;i<4;i++)
		  z[i]=ldiv((GEN)x[i],y);
		break;
	      case 2 : l=avma;p1=co8(x,ly);tetpil=avma;
		z=gerepile(l,tetpil,gdiv(p1,y));
		break;
	      case 7 : l=avma;p1=cvtop(x,(GEN)y[2],precp(y));
		tetpil=avma;z=gerepile(l,tetpil,gdiv(p1,y));
		break;
	      case 6 : ly=precision(y);if(!ly) err(gdiver6);
		l=avma;p1=co8(x,ly);tetpil=avma;
		z=gerepile(l,tetpil,gdiv(p1,y));
		break;
	      case 8 : k=x[1];l=y[1];
		if ((k!=l)&&(!gegal((GEN)k,(GEN)l))) err(gdiver18);
		l=avma;p1=gnorm(y);
		p3=gmul(x,gconj(y));tetpil=avma;
		z=gerepile(l,tetpil,gdiv(p3,p1));
		break;
	      case 9 : z=cgetg(ly,ty);l=avma;
		p1=ginvmod((GEN)y[2],(GEN)y[1]);
		tetpil=avma;z[2]=lmul(x,p1);
		gerepile(l,tetpil,(GEN)1);
		z[1]=copyifstack((GEN)y[1]);
		break;
	      default: err(gdiver1);
	      } break;
	    case 9 : 
	      if(ty!=9)
		{
		  z=cgetg(lx,tx);
		  z[2]=ldiv((GEN)x[2],y);
		  z[1]=copyifstack((GEN)x[1]);
		}
	      else
		{
		  z=cgetg(ly,ty);k=x[1];l=y[1];
		  if((k==l)||gegal((GEN)k,(GEN)l))
		    z[1]=copyifstack((GEN)k);
		  else z[1]=lgcd((GEN)k,(GEN)l);
		  l=avma;p1=ginvmod((GEN)y[2],(GEN)z[1]);
		  p2=gmul((GEN)x[2],p1);tetpil=avma;
		  z[2]=lpile(l,tetpil,gmod(p2,(GEN)z[1]));
		}
	      break;
	      
	    default: err(gdiver1);
	    }
	}
      else
	{
	  vx=gvar(x);vy=gvar(y);
	  if(((vx<vy)&&((tx<17)||(ty<17)))||((vx==vy)&&(ty<10))||((tx>=17)&&(ty<17)))
	    {
	      l=avma;z=cgetg(lx,tx);
	      switch(tx)
		{
		case 9: z[1]=copyifstack((GEN)x[1]);z[2]=ldiv((GEN)x[2],y);break;
		case 10: l1=lgef(x);
		  for(i=2;i<l1;i++)
		    z[i]=ldiv((GEN)x[i],y);
		  z[1]=x[1];
		  break;
		case 11:
		  if(gcmp0(x)) z=gcopy(x);
		  else
		    {
		      for(i=2;i<lx;i++)
			z[i]=ldiv((GEN)x[i],y);
		      setvalp(z,valp(x));
		      setvarn(z,vx);
		      normalize(&z);
		    }
		  break;
		case 13: z[1]=x[1];z[2]=lmul((GEN)x[2],y);
		  tetpil=avma;z=gerepile(l,tetpil,gred(z));
		  break;
		case 14: z[1]=lcopy((GEN)x[1]);z[2]=lmul((GEN)x[2],y);
		  break;
		case 17:
		case 18:
		case 19: for(i=1;i<lx;i++)
		  z[i]=ldiv((GEN)x[i],y);
		  break;
		default: err(gdiver1);
		}
	    }
	  else
	    {
	      if((vy<vx)||((vy==vx)&&(tx<10)))
		switch(ty)
		  {
		  case 9 : z=cgetg(ly,ty);l=avma;
		    p1=ginvmod((GEN)y[2],(GEN)y[1]);
		    tetpil=avma;z[2]=lmul(x,p1);
		    gerepile(l,tetpil,(GEN)1);
		    z[1]=copyifstack((GEN)y[1]);
		    break;
		  case 10:
		    if(lgef(y)>3)
		      {
			if(isexactzero(x)) 
			  {z=cgetg(2,10);z[1]=evallgef(2)+evalvarn(vy);}
			else
			  {
			    l=avma;z=cgetg(3,13);z[1]=(long)x;
			    z[2]=(long)y;tetpil=avma;
			    z=gerepile(l,tetpil,gred(z));
			  }
		      }
		    else z=gdiv(x,(GEN)y[2]);
		    break;
		  case 11: l=avma;
		    if(gcmp0(x))
		      {
			p1=ginv(y);tetpil=avma; /* a ameliorer !!!! */
			z=gerepile(l,tetpil,gmul(x,p1));
		      }
		    else
		      {
			p1=cgetg(ly,ty);p1[2]=lcopy(x);
			p1[1]=evalsigne(1)+HIGHVALPBIT+evalvarn(vy);
			for(i=3;i<ly;i++) p1[i]=zero;
			tetpil=avma;p2=gdiv(p1,y);
			z=gerepile(l,tetpil,p2);
		      }
		    break;
		  case 13: l=avma;z=cgetg(ly,ty);z[1]=lmul(x,(GEN)y[2]);
		    z[2]=y[1];tetpil=avma;z=gerepile(l,tetpil,gred(z));
		    break;
		  case 14: z=cgetg(ly,ty);z[1]=lmul(x,(GEN)y[2]);z[2]=lcopy((GEN)y[1]);
		    break;
		  case 17:
		  case 18: err(gdiver8);
		  case 19: if((ly==1)||(lg((GEN)y[1])!=ly)) err(gdiver9);
		    l=avma;p1=invmat(y);tetpil=avma;
		    z=gerepile(l,tetpil,gmul(x,p1));
		    break;
		  default: err(gdiver1);
		  }
	      else /* ici vx=vy et tx>=10 et ty>=10*/
		switch(tx)
		  {
		  case 10: switch(ty)
		    {
		    case 10: l=avma;z=cgetg(3,13);
		      z[1]=lcopy(x);z[2]=lcopy(y);tetpil=avma;
		      z=gerepile(l,tetpil,gred(z));
		      break;
		    case 11: if(gcmp0(x))
		      {
			z=cgetg(2,10);z[1]=evallgef(2)+evalvarn(vx);
		      }
		    else
		      {
			l=avma;p1=greffe(x,ly);
			tetpil=avma;p2=gdiv(p1,y);
			z=gerepile(l,tetpil,p2);
		      }
		      break;
		    case 13:
		    case 14: l=avma;z=cgetg(ly,ty);
		      z[1]=lmul(x,(GEN)y[2]);z[2]=(ty==13)?y[1]:lcopy((GEN)y[1]);
		      if(ty==13) {tetpil=avma;z=gerepile(l,tetpil,gred(z));}
		      break;
		    case 17:
		    case 18:
		    case 19: err(gdiver10);
		    default: err(gdiver1);
		    } break;
		  case 11: switch(ty)
		    {
		    case 10: l=avma;p1=greffe(y,lx);tetpil=avma;
		      p2=gdiv(x,p1);z=gerepile(l,tetpil,p2);
		      break;
		    case 11: lz=lx;if(ly<lx) lz=ly;
		      if(gcmp0(x))
			{
			  z=cgetg(3,11);
			  z[1]=evalvalp(valp(x)-valp(y))+evalvarn(vx);
			}
		      else
			{
			  z=cgetg(lz,ty);p3=cgeti(lz);
			  for(i=3;i<lz;i++) p3[i]=!isexactzero((GEN)y[i]);
			  z[1]=evalvalp(valp(x)-valp(y))+evalvarn(vx);
			  z[2]=ldiv((GEN)x[2],(GEN)y[2]);
			  for(i=3;i<lz;i++)
			    {
			      l=avma;p1=(GEN)x[i];
			      for(j=2;j<i;j++)
				if(p3[i-j+2]) p1=gsub(p1,gmul((GEN)z[j],(GEN)y[i-j+2]));
			      tetpil=avma;p5=gdiv(p1,(GEN)y[2]);
			      z[i]=lpile(l,tetpil,p5);
			    } normalize(&z);
			}
		      break;
		    case 13:
		    case 14: l=avma;p2=gmul(x,(GEN)y[2]);tetpil=avma;
		      z=gerepile(l,tetpil,gdiv(p2,(GEN)y[1]));
		      break;
		    case 17:
		    case 18:
		    case 19: err(gdiver12);
		    default: err(gdiver1);
		    } break;
		  case 13:
		  case 14: switch(ty)
		    {
		    case 10: l=avma;z=cgetg(lx,tx);
		      z[1]=(tx==13)?x[1]:lcopy((GEN)x[1]);z[2]=lmul((GEN)x[2],y);
		      if(tx==13) {tetpil=avma;z=gerepile(l,tetpil,gred(z));}
		      break;
		    case 11: l=avma;p2=gmul((GEN)x[2],y);tetpil=avma;
		      p3=gdiv((GEN)x[1],p2);z=gerepile(l,tetpil,p3);
		      break;
		    case 13:
		    case 14: if((tx+ty)==26) tz=13;else tz=14;
		      l=avma;z=cgetg(ly,tz);
		      z[1]=lmul((GEN)x[1],(GEN)y[2]);z[2]=lmul((GEN)x[2],(GEN)y[1]);
		      if(tz==13) {tetpil=avma;z=gerepile(l,tetpil,gred(z));}
		      break;
		    case 17:
		    case 18:
		    case 19: err(gdiver16);
		    default: err(gdiver1);
		    } break;
		  case 15: l=signe((GEN)y[2]);setsigne((GEN)y[2],-l);s=signe((GEN)y[4]);
		    setsigne((GEN)y[4],-s);z=compreal(x,y);setsigne((GEN)y[2],l);setsigne((GEN)y[4],s);
		    break;
		  case 16: l=signe((GEN)y[2]);setsigne((GEN)y[2],-l);
		    z=compimag(x,y);setsigne((GEN)y[2],l);break;
		  case 17:
		  case 18:
		  case 19: if(ty<17)
		    {
		      z=cgetg(lx,tx);
		      for(i=1;i<lx;i++)
			z[i]=ldiv((GEN)x[i],y);
		    }
		  else
		    {
		      if((ty==19)&&(ly!=1)&&(lg((GEN)y[1])==ly))
			{
			  l=avma;
			  p1=invmat(y);tetpil=avma;
			  z=gerepile(l,tetpil,gmul(x,p1));
			}
		      else err(gdiver17);
		    }
		    break;
		  default: err(gdiver1);
		  }
	    }
	}
    }
  return z;
}
