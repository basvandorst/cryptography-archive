/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*                                                                 */
/*                     PLOT EN HAUTE RESOLUTION                    */
/*                                                                 */
/*                       copyright Babe Cool                       */
/*                                                                 */
/*                                                                 */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

# include "genpari.h"

#include        <suntool/sunview.h>
#include        <suntool/canvas.h>
#include        <suntool/textsw.h>
#include        <suntool/panel.h>

typedef struct spoint {
  int x,y;} SPoint; 
typedef struct ssegment {
  int x1,y1,x2,y2;} SSegment;
typedef struct srectangle {
  int x,y,width,height;} SRectangle;

#define ISCR 1120 /* 1400 en haute resolution */     
#define JSCR 800  /* 1120 en haute resolution */     
#define DECI  95
#define DECJ  30 

GEN  ploth(ep,a,b,ch,prec)
     entree *ep;
     GEN a,b;
     char *ch;
     long prec;

{
  long av,av2,jz,j,j1,i,sig,is,is2,js,js2;
  GEN p1,ysml,ybig,x,diff,dyj,dx,y[ISCR+1];
  char c1[20];
  Frame ecran;
  Canvas canevas;
  Pixwin *pw;
  Pixfont *font;

  ecran=window_create(NULL,FRAME,FRAME_LABEL,"ploth",
                      WIN_ERROR_MSG,"you must be in suntools",0);
  canevas=window_create(ecran,CANVAS,WIN_HEIGHT,JSCR,
                        WIN_WIDTH,ISCR,0);
  window_fit(ecran);pw=canvas_pixwin(canevas);
  is=ISCR-DECI;js=JSCR-DECJ;is2=is-DECI;js2=js-DECJ;
  pw_vector(pw,DECI,DECJ,DECI,js,PIX_SRC,1);
  pw_vector(pw,DECI,DECJ,is,DECJ,PIX_SRC,1);
  pw_vector(pw,is,DECJ,is,js,PIX_SRC,1);
  pw_vector(pw,DECI,js,is,js,PIX_SRC,1);

  sig=gcmp(b,a); if(!sig) return gnil;
  av=avma;
  if(sig<0) {x=a;a=b;b=x;}
  for(i=1;i<=is2;i++) y[i]=cgetr(3);
  newvalue(ep,cgetr(prec)); x=(GEN)ep->value; gaffect(a,x);
  dx=gdivgs(gsub(b,a),is2-1);ysml=gzero;ybig=gzero;
  av2=avma;
  for(i=1;i<=is2;i++)
  {
    gaffect(lisexpr(ch),y[i]);
    if(gcmp(y[i],ysml)<0) ysml=y[i];
    if(gcmp(y[i],ybig)>0) ybig=y[i];
    gaddz(x,dx,x);avma=av2;
  }
  diff=gsub(ybig,ysml);
  if(gcmp0(diff)) {ybig=gaddsg(1,ybig);diff=gun;}
  dyj=gdivsg(js2-1,diff);jz=js+itos(ground(gmul(ysml,dyj)));
  pw_vector(pw,DECI,jz,is,jz,PIX_SRC,1);
  if(gsigne(a)*gsigne(b)<0)
  {
    jz=1-itos(ground(gdiv(a,dx)))+DECI;
    pw_vector(pw,jz,DECJ,jz,js,PIX_SRC,1);
  }
  av2=avma;
  for(i=1;i<=is2;i++)
  {
    j1=js-itos(ground(gmul(gsub(y[i],ysml),dyj)));
    if(i==1) j=j1;
    else
    {
      pw_vector(pw,i-2+DECI,j,i-1+DECI,j1,PIX_SRC,1);j=j1;
    }
    avma=av2;
  }
  font=pw_pfsysopen();
  p1=cgetr(4);gaffect(ysml,p1);sprintf(c1,"%9.3lf",rtodbl(p1));
  for(i=1;c1[i];i++) pw_char(pw,-4+9*i,js,PIX_SRC|PIX_DST,font,c1[i]);
  gaffect(ybig,p1);sprintf(c1,"%9.3lf",rtodbl(p1));
  for(i=1;c1[i];i++) pw_char(pw,-4+9*i,DECJ,PIX_SRC|PIX_DST,font,c1[i]);
  gaffect(a,p1);sprintf(c1,"%9.3lf",rtodbl(p1));
  for(i=1;c1[i];i++) pw_char(pw,DECI-45+9*i,js+20,PIX_SRC|PIX_DST,font,c1[i]);
  gaffect(b,p1);sprintf(c1,"%9.3lf",rtodbl(p1));
  for(i=1;c1[i];i++) pw_char(pw,is-45+9*i,js+20,PIX_SRC|PIX_DST,font,c1[i]);
  avma = av;
  window_main_loop(ecran);
  killvalue(ep);
  return gnil;
}

GEN  ploth2(ep,a,b,ch,prec)
     entree *ep;
     GEN a,b;
     char *ch;
     long prec;

{
  long av,av2,jz,iz,k1,k,j,j1,i,sig,is,is2,js,js2;
  GEN p1,ysml,ybig,xsml,xbig,diffx,diffy,dxj,t,dyj,dt,y[ISCR+1],x[ISCR+1];
  char c1[20];
  Frame ecran;
  Canvas canevas;
  Pixwin *pw;
  Pixfont *font;

  ecran=window_create(NULL,FRAME,FRAME_LABEL,"ploth2",
                      WIN_ERROR_MSG,"you must be in suntools",0);
  canevas=window_create(ecran,CANVAS,WIN_HEIGHT,JSCR,
                        WIN_WIDTH,ISCR,0);
  window_fit(ecran);pw=canvas_pixwin(canevas);
  is=ISCR-DECI;js=JSCR-DECJ;is2=is-DECI;js2=js-DECJ;
  pw_vector(pw,DECI,DECJ,DECI,js,PIX_SRC,1);
  pw_vector(pw,DECI,DECJ,is,DECJ,PIX_SRC,1);
  pw_vector(pw,is,DECJ,is,js,PIX_SRC,1);
  pw_vector(pw,DECI,js,is,js,PIX_SRC,1);

  sig=gcmp(b,a); if(!sig) return gnil;
  av=avma;
  if(sig<0) {p1=a;a=b;b=p1;}
  for(i=1;i<=is2;i++) {x[i]=cgetr(3);y[i]=cgetr(3);}
  newvalue(ep,cgetr(prec)); t=(GEN)ep->value; gaffect(a,t);
  dt=gdivgs(gsub(b,a),is2-1);ysml=ybig=xsml=xbig=gzero;
  av2=avma;
  for(i=1;i<=is2;i++)
  {
    p1=lisexpr(ch);gaffect(p1[1],x[i]);gaffect(p1[2],y[i]);
    if(gcmp(y[i],ysml)<0) ysml=y[i];
    if(gcmp(y[i],ybig)>0) ybig=y[i];
    if(gcmp(x[i],xsml)<0) xsml=x[i];
    if(gcmp(x[i],xbig)>0) xbig=x[i];
    gaddz(t,dt,t);avma=av2;
  }
  diffy=gsub(ybig,ysml);
  if(gcmp0(diffy)) {ybig=gaddsg(1,ybig);diffy=gun;}
  diffx=gsub(xbig,xsml);
  if(gcmp0(diffx)) {xbig=gaddsg(1,xbig);diffx=gun;}
  dyj=gdivsg(js2-1,diffy);jz=js+itos(ground(gmul(ysml,dyj)));
  dxj=gdivsg(is2-1,diffx);iz=DECI-itos(ground(gmul(xsml,dxj)));
  if(gsigne(ysml)*gsigne(ybig)<0)
    pw_vector(pw,DECI,jz,is,jz,PIX_SRC,1);
  if(gsigne(xsml)*gsigne(xbig)<0)
    pw_vector(pw,iz,DECJ,iz,js,PIX_SRC,1);
  av2=avma;
  for(i=1;i<=is2;i++)
  {
    j1=js-itos(ground(gmul(gsub(y[i],ysml),dyj)));
    k1=DECI+itos(ground(gmul(gsub(x[i],xsml),dxj)));
    if(i==1) {j=j1;k=k1;}
    else
    {
      pw_vector(pw,k,j,k1,j1,PIX_SRC,1);j=j1;k=k1;
    }
    avma=av2;
  }
  font=pw_pfsysopen();
  p1=cgetr(4);gaffect(ysml,p1);sprintf(c1,"%9.3lf",rtodbl(p1));
  for(i=1;c1[i];i++) pw_char(pw,-4+9*i,js,PIX_SRC|PIX_DST,font,c1[i]);
  gaffect(ybig,p1);sprintf(c1,"%9.3lf",rtodbl(p1));
  for(i=1;c1[i];i++) pw_char(pw,-4+9*i,DECJ,PIX_SRC|PIX_DST,font,c1[i]);
  gaffect(xsml,p1);sprintf(c1,"%9.3lf",rtodbl(p1));
  for(i=1;c1[i];i++) pw_char(pw,DECI-45+9*i,js+20,PIX_SRC|PIX_DST,font,c1[i]);
  gaffect(xbig,p1);sprintf(c1,"%9.3lf",rtodbl(p1));
  for(i=1;c1[i];i++) pw_char(pw,is-45+9*i,js+20,PIX_SRC|PIX_DST,font,c1[i]);
  avma = av;
  window_main_loop(ecran);
  killvalue(ep);
  return gnil;
}

GEN  plothraw(listx,listy)
     GEN listx,listy;
{
  long av = avma,av2,i,lx,is,js,is2,js2;
  char c1[20];
  SPoint *points;
  GEN p1,xsml,xbig,ysml,ybig,dx,dy,scal,scaly;

  Frame ecran;
  Canvas canevas;
  Pixwin *pw;
  Pixfont *font;
  
  if((typ(listx)<17)||(typ(listx)>18)||(typ(listy)<17)||(typ(listy)>18))
    err(ploter4);
  lx=lg(listx);
  if(lg(listy)!=lx) err(ploter5);
  if(lx==1) return gnil;
  points = (SPoint*)malloc(lx*sizeof(SPoint));
  if(!points) err(ploter6);
  is=ISCR-DECI;js=JSCR-DECJ;is2=is-DECI;js2=js-DECJ;
  av=avma;xsml=xbig=(GEN)listx[1];ysml=ybig=(GEN)listy[1];
  for(i = 0; i < lx-1; i++)
    {
      p1=(GEN)listx[i+1];
      if(gcmp(p1,xsml)<0) xsml=p1;if(gcmp(p1,xbig)>0) xbig=p1;
      p1=(GEN)listy[i+1];
      if(gcmp(p1,ysml)<0) ysml=p1;if(gcmp(p1,ybig)>0) ybig=p1;
    }
  dx=gsub(xbig,xsml);dy=gsub(ybig,ysml);
  if(gcmp0(dx))
    {
      if(gcmp0(dy)) 
	{
	  scal=gun;dx=gsubsg(is2>>1,xsml);
	  dy=gsubsg(js2>>1,ysml);
	}
      else
	{
	  scal=gdivsg(js2,dy);
	  dx=gneg(gmul(scal,xsml));dy=gneg(gmul(scal,ysml));
	}
    }
  else
    {
      scal=gdivsg(is2,dx);
      if(!gcmp0(dy))
	{
	  scaly=gdivsg(js2,dy);if(gcmp(scaly,scal)<0) scal=scaly;
	}
      dx=gneg(gmul(scal,xsml));dy=gneg(gmul(scal,ysml));
    }
  for(i = 0; i < lx-1; i++)
    {
      av2=avma;
      points[i].x = DECI + itos(ground(gadd(gmul(listx[i+1],scal),dx)));
      points[i].y = js - itos(ground(gadd(gmul(listy[i+1],scal),dy)));
      avma=av2;
    }
  xsml=gneg(gdiv(dx,scal));xbig=gdiv(gsubsg(is2,dx),scal);
  ysml=gneg(gdiv(dy,scal));ybig=gdiv(gsubsg(js2,dy),scal);

  ecran=window_create(NULL,FRAME,FRAME_LABEL,"plothraw",
                      WIN_ERROR_MSG,"you must be in suntools",0);
  canevas=window_create(ecran,CANVAS,WIN_HEIGHT,JSCR,
                        WIN_WIDTH,ISCR,0);
  window_fit(ecran);pw=canvas_pixwin(canevas);
  pw_vector(pw,DECI,DECJ,DECI,js,PIX_SRC,1);
  pw_vector(pw,DECI,DECJ,is,DECJ,PIX_SRC,1);
  pw_vector(pw,is,DECJ,is,js,PIX_SRC,1);
  pw_vector(pw,DECI,js,is,js,PIX_SRC,1);
  font=pw_pfsysopen();
  p1=cgetr(4);gaffect(ysml,p1);sprintf(c1,"%9.3lf",rtodbl(p1));
  for(i=1;c1[i];i++) pw_char(pw,-4+9*i,js,PIX_SRC|PIX_DST,font,c1[i]);
  gaffect(ybig,p1);sprintf(c1,"%9.3lf",rtodbl(p1));
  for(i=1;c1[i];i++) pw_char(pw,-4+9*i,DECJ,PIX_SRC|PIX_DST,font,c1[i]);
  gaffect(xsml,p1);sprintf(c1,"%9.3lf",rtodbl(p1));
  for(i=1;c1[i];i++) pw_char(pw,DECI-45+9*i,js+20,PIX_SRC|PIX_DST,font,c1[i]);
  gaffect(xbig,p1);sprintf(c1,"%9.3lf",rtodbl(p1));
  for(i=1;c1[i];i++) pw_char(pw,is-45+9*i,js+20,PIX_SRC|PIX_DST,font,c1[i]);

  for(i = 0; i < lx-1; i++)
    pw_put(pw,points[i].x,points[i].y,1);
/* Should be:
  pw_polypoint(pw,0,0,lx-1,points,PIX_SRC);
  Why doesn't it work???
 */
  window_main_loop(ecran);
  free(points);avma = av;
  return gnil;
}

GEN rectdraw(list)
     GEN list;
{
  long *e,*p1,*ptx,*pty,*numpoints,*numtexts,*xtexts,*ytexts;
  long n,i,j,x0,y0,av=avma;
  long a,b,c,d,nd[10],ne;
  char **texts;

  Frame ecran;
  Canvas canevas;
  Pixwin *pw;
  Pixfont *font;
  SPoint *points, **lines, *SLine;
  SSegment *segments; 
  SRectangle *rectangles, SRec;

  if(typ(list)!=17) err(rploter3);
  n=lg(list)-1;if(n%3) err(rploter4);
  n=n/3;if(!n) return gnil;
  nd[0]=nd[1]=nd[2]=nd[3]=nd[4]=nd[5]=nd[6]=0;
  for(i=0;i<n;i++)
    {
      if(typ(list[3*i+1])!=1) err(rploter5);
      ne=itos(list[3*i+1]);if((ne<0)||(ne>15)) err(rploter2);
      e=rectgraph[ne];
      p1=(long*)e[0];while((long)p1) 
	{
	  if(p1[1]!=4) nd[p1[1]]++;
	  else nd[1]+=p1[2];
	  p1=(long*)p1[0];
	}
    }
  points=(SPoint*)malloc(nd[1]*sizeof(SPoint));
  segments=(SSegment*)malloc(nd[2]*sizeof(SSegment));
  rectangles=(SRectangle*)malloc(nd[3]*sizeof(SRectangle));
  lines=(SPoint**)malloc(nd[5]*sizeof(SPoint*));
  numpoints=(long*)malloc(nd[5]*sizeof(long));
  texts=(char**)malloc(nd[6]*sizeof(char*));
  numtexts=(long*)malloc(nd[6]*sizeof(long));
  xtexts=(long*)malloc(nd[6]*sizeof(long));
  ytexts=(long*)malloc(nd[6]*sizeof(long));
  nd[1]=nd[2]=nd[3]=nd[5]=nd[6]=0;
  for(i=0;i<n;i++)
    {
      e=rectgraph[itos(list[3*i+1])];x0=list[3*i+2];y0=list[3*i+3];
      if((typ(x0)!=1)||(typ(y0)!=1)) err(rploter5);
      x0=itos(x0);y0=itos(y0);
      p1=(long*)e[0];
      while((long)p1)
	{
	  switch(p1[1])
	    {
	    case 1: 
	      points[nd[1]].x=p1[2]+x0;
	      points[nd[1]].y=p1[3]+y0;
	      nd[1]++;break;
	    case 2:
	      segments[nd[2]].x1=p1[2]+x0;
	      segments[nd[2]].y1=p1[3]+y0;
	      segments[nd[2]].x2=p1[4]+x0;
	      segments[nd[2]].y2=p1[5]+y0;
	      nd[2]++;break;
	    case 3:
	      a=rectangles[nd[3]].x=p1[2]+x0;
	      b=rectangles[nd[3]].y=p1[3]+y0;
	      rectangles[nd[3]].width=p1[4]+x0-a;
	      rectangles[nd[3]].height=p1[5]+y0-b;
	      nd[3]++;break;
	    case 4:
	      ptx=(long*)p1[3];pty=(long*)p1[4];
	      for(j=0;j<p1[2];j++)
		{
		  points[nd[1]+j].x=ptx[j]+x0;
		  points[nd[1]+j].y=pty[j]+y0;
		}
	      nd[1]+=p1[2];break;
	    case 5:
	      ptx=(long*)p1[3];pty=(long*)p1[4];
	      numpoints[nd[5]]=p1[2];
	      lines[nd[5]]=(SPoint*)malloc(p1[2]*sizeof(SPoint));
	      for(j=0;j<p1[2];j++)
		{
		  lines[nd[5]][j].x=ptx[j]+x0;
		  lines[nd[5]][j].y=pty[j]+y0;
		}
	      nd[5]++;break;
	    case 6: 
	      texts[nd[6]]=(char*)p1[3];numtexts[nd[6]]=p1[2];
	      xtexts[nd[6]]=p1[4]+x0;ytexts[nd[6]]=p1[5]+y0;
	      nd[6]++;break;
	    default: break;
	    }
	  p1=(long*)p1[0];
	}
    }
  ecran=window_create(NULL,FRAME,FRAME_LABEL,"rectplot",
                      WIN_ERROR_MSG,"you must be in suntools",0);
  canevas=window_create(ecran,CANVAS,WIN_HEIGHT,JSCR,
                        WIN_WIDTH,ISCR,0);
  window_fit(ecran);pw=canvas_pixwin(canevas);

  font=pw_pfsysopen();
  for(i=0;i<nd[1];i++) pw_put(pw,points[i].x,points[i].y,1);
  for(i=0;i<nd[2];i++) pw_vector(pw,segments[i].x1,segments[i].y1,segments[i].x2,segments[i].y2,PIX_SRC,1);
  for(i=0;i<nd[3];i++) 
    {
      SRec=rectangles[i];a=SRec.x;b=SRec.y;c=a+SRec.width;
      d=b+SRec.height;
      pw_vector(pw,a,b,c,b,PIX_SRC,1);
      pw_vector(pw,c,b,c,d,PIX_SRC,1);
      pw_vector(pw,a,d,c,d,PIX_SRC,1);
      pw_vector(pw,a,b,a,d,PIX_SRC,1);
    }
  for(i=0;i<nd[5];i++) 
    {
      SLine=lines[i];
      for(j=1;j<numpoints[i];j++)
	pw_vector(pw,SLine[j-1].x,SLine[j-1].y,SLine[j].x,SLine[j].y,PIX_SRC,1);
    }
  for(i=0;i<nd[6];i++) 
    for(j=0;texts[i][j];j++)
      pw_char(pw,xtexts[i]+9*j,ytexts[i],PIX_SRC|PIX_DST,font,texts[i][j]);
  window_main_loop(ecran);
  free(points);free(segments);free(rectangles);
  free(numpoints);for(i=0;i<nd[5];i++) free(lines[i]);
  free(lines);free(texts);free(numtexts);free(xtexts);free(ytexts);
  avma = av;return gnil;
}

