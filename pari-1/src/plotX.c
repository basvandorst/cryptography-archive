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

#ifdef HPPA
#ifndef __GNUC__
typedef char *caddr_t;
#endif
#endif

#ifdef __cplusplus
extern "C" {
#endif
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#ifdef __cplusplus
}
#endif


#define IDEC 90
#define JDEC 5

#ifdef SONY

typedef int (*XErrorHandler) (
#if NeedFunctionPrototypes
    Display*	
    XErrorEvent*
#endif
);

extern XErrorHandler XSetErrorHandler (
#if NeedFunctionPrototypes
    XErrorHandler	
#endif
);


typedef int (*XIOErrorHandler) ( 
#if NeedFunctionPrototypes
    Display*	
#endif
);

extern XIOErrorHandler XSetIOErrorHandler (
#if NeedFunctionPrototypes
    XIOErrorHandler
#endif
);
#endif

int xerror()
{
  err(ploter1);return 0;
}

GEN  ploth(entree *ep, GEN a, GEN b, char *ch, long prec)
{
  long av = avma,av2,jz,i,sig,is,js,iz;
  GEN p1,ysml,ybig,x,diff,dyj,dx,y[2000];
  char c1[20], c2[20], c3[20], c4[20];
  XPoint points[2000];

  Display *display;
  Window win;
  XSizeHints size_hints;
  XEvent report;
  GC gc;
  XFontStruct *font_info;
  int screen, width, height;
  
  sig=gcmp(b,a); if(!sig) return gnil;
  if(sig<0) {x=a;a=b;b=x;}
  
  if (!(display = XOpenDisplay(NULL))) err(ploter2);
  if (!(font_info = XLoadQueryFont(display, "9x15"))) err(ploter3);

  XSetErrorHandler((XErrorHandler)xerror);
  XSetIOErrorHandler((XIOErrorHandler)xerror);

  screen = DefaultScreen(display);
  width = DisplayWidth(display, screen) - 40;
  height = DisplayHeight(display, screen) - 60;
  is = width - IDEC - 5;
  js = height - 25;
  
  for(i=0;i<=is;i++) y[i]=cgetr(3);
  newvalue(ep,cgetr(prec)); x=ep->value;
  gaffect(a,x);
  dx=gdivgs(gsub(b,a),is); ysml = ybig = gzero;
  av2=avma;
  for(i=0;i<=is;i++)
    {
      gaffect(lisexpr(ch), y[i]);
      if(gcmp(y[i], ysml) < 0) ysml = y[i]; else if(gcmp(y[i], ybig) > 0) ybig = y[i];
      gaddz(x,dx,x);avma=av2;
    }
  diff = gsub(ybig, ysml);
  if(gcmp0(diff)) {ybig=gaddsg(1,ybig);diff=gun;}
  dyj = gdivsg(js, diff); av2 = avma;
  for(i = 0; i <= is; i++)
    {
      points[i].x = IDEC + i;
      points[i].y = JDEC + itos(ground(gmul(gsub(ybig, y[i]),dyj)));
      avma=av2;
    }
  jz = JDEC + itos(ground(gmul(ybig,dyj)));
  if(gsigne(a)*gsigne(b)<0) iz = IDEC + itos(ground(gmulsg(is, gdiv(a, gsub(a, b)))));

  p1=cgetr(4); gaffect(ybig,p1); sprintf(c1,"%9.3lf",rtodbl(p1));
  gaffect(ysml,p1); sprintf(c2,"%9.3lf",rtodbl(p1));
  gaffect(a,p1); sprintf(c3,"%9.3lf",rtodbl(p1));
  gaffect(b,p1); sprintf(c4,"%9.3lf",rtodbl(p1));

  win = XCreateSimpleWindow(display, RootWindow(display, screen), 0, 0, width, height, 4, BlackPixel(display, screen), WhitePixel(display, screen));

  size_hints.flags = PPosition | PSize;
  size_hints.x = 0;
  size_hints.y = 0;
  size_hints.width = width;
  size_hints.height = height;

  XSetStandardProperties(display, win, "ploth", NULL, NULL, NULL, 0, &size_hints);

  XSelectInput(display, win, ExposureMask | ButtonPressMask);

  gc = XCreateGC(display, win, 0, NULL);
  XSetFont(display, gc, font_info->fid);
  XSetForeground(display, gc, BlackPixel(display, screen)); 

  XMapWindow(display, win);

  for(;;)
    {
      XNextEvent(display, &report);
      if (report.type != Expose) break;
      while (XCheckTypedEvent(display, Expose, &report));
      XDrawRectangle(display, win, gc, IDEC, JDEC, is, js);
      XDrawLine(display, win, gc, IDEC, jz, IDEC + is, jz);
      if (iz) XDrawLine(display, win, gc, iz, JDEC, iz, js + JDEC);
      XDrawString(display, win, gc, 0, 15, c1, strlen(c1));
      XDrawString(display, win, gc, 0, height - 30, c2, strlen(c2));
      XDrawString(display, win, gc, IDEC - 20, height - 5, c3, strlen(c3));
      XDrawString(display, win, gc, width - 90, height - 5, c4, strlen(c4));
      XDrawLines(display, win, gc, points, is+1, 0);
    }
  XUnloadFont(display, font_info->fid);
  XFreeGC(display, gc);
  XCloseDisplay(display);
  avma = av;
  killvalue(ep);
  return gnil;
}

#define NBPOINTS 1500

GEN  ploth2(entree *ep, GEN a, GEN b, char *ch, long prec)
{
  long av = avma,av2,jz,i,sig,is,js,iz;
  GEN p1,xsml,xbig,ysml,ybig,t,diffx,diffy,dy,dx,dt,x[NBPOINTS],y[NBPOINTS];
  char c1[20], c2[20], c3[20], c4[20];
  XPoint points[NBPOINTS];

  Display *display;
  Window win;
  XSizeHints size_hints;
  XEvent report;
  GC gc;
  XFontStruct *font_info;
  int screen, width, height;
  
  sig=gcmp(b,a); if(!sig) return gnil;
  if(sig<0) {t=a;a=b;b=t;}
  
  if (!(display = XOpenDisplay(NULL))) err(ploter2);
  if (!(font_info = XLoadQueryFont(display, "9x15"))) err(ploter3);

  XSetErrorHandler((XErrorHandler)xerror);
  XSetIOErrorHandler((XIOErrorHandler)xerror);

  screen = DefaultScreen(display);
  width = DisplayWidth(display, screen) - 40;
  height = DisplayHeight(display, screen) - 60;
  is = width - IDEC - 5;
  js = height - 25;
  
  for(i=0;i<NBPOINTS;i++) {x[i] = cgetr(3); y[i]=cgetr(3);}
  newvalue(ep,cgetr(prec)); t=ep->value;
  gaffect(a,t);
  dt=gdivgs(gsub(b,a),NBPOINTS); xsml = xbig = ysml = ybig = gzero;
  av2=avma;
  for(i=0;i<NBPOINTS;i++)
    {
      p1 = lisexpr(ch);
      gaffect((GEN)p1[1], x[i]); gaffect((GEN)p1[2], y[i]);
      if(gcmp(x[i], xsml) < 0) xsml = x[i]; else if(gcmp(x[i], xbig) > 0) xbig = x[i];
      if(gcmp(y[i], ysml) < 0) ysml = y[i]; else if(gcmp(y[i], ybig) > 0) ybig = y[i];
      gaddz(t,dt,t);
      avma=av2;
    }
  diffx = gsub(xbig, xsml);
  if(gcmp0(diffx)) {xbig=gaddsg(1,xbig);diffx=gun;}
  diffy = gsub(ybig, ysml);
  if(gcmp0(diffy)) {ybig=gaddsg(1,ybig);diffy=gun;}
  dy = gdivsg(js, diffy);
  dx = gdivsg(is, diffx);
  av2 = avma;
  for(i = 0; i < NBPOINTS; i++)
    {
      points[i].x = IDEC + itos(ground(gmul(gsub(x[i],xsml),dx)));
      points[i].y = JDEC + itos(ground(gmul(gsub(ybig, y[i]),dy)));
      avma=av2;
    }

  iz = IDEC - itos(ground(gmul(xsml, dx)));
  jz = JDEC + itos(ground(gmul(ybig, dy)));

  p1=cgetr(4); gaffect(ybig,p1); sprintf(c1,"%9.3lf",rtodbl(p1));
  gaffect(ysml,p1); sprintf(c2,"%9.3lf",rtodbl(p1));
  gaffect(xsml,p1); sprintf(c3,"%9.3lf",rtodbl(p1));
  gaffect(xbig,p1); sprintf(c4,"%9.3lf",rtodbl(p1));

  win = XCreateSimpleWindow(display, RootWindow(display, screen), 0, 0, width, height, 4,
                            BlackPixel(display, screen), WhitePixel(display, screen));

  size_hints.flags = PPosition | PSize;
  size_hints.x = 0;
  size_hints.y = 0;
  size_hints.width = width;
  size_hints.height = height;

  XSetStandardProperties(display, win, "ploth2", NULL, NULL, NULL, 0, &size_hints);

  XSelectInput(display, win, ExposureMask | ButtonPressMask);

  gc = XCreateGC(display, win, 0, NULL);
  XSetFont(display, gc, font_info->fid);
  XSetForeground(display, gc, BlackPixel(display, screen));

  XMapWindow(display, win);

  for(;;)
    {
      XNextEvent(display, &report);
      if (report.type != Expose) break;
      while (XCheckTypedEvent(display, Expose, &report));
      XDrawRectangle(display, win, gc, IDEC, JDEC, width - IDEC - 5, js);
      XDrawLine(display, win, gc, iz, JDEC, iz, js + JDEC);
      XDrawLine(display, win, gc, IDEC, jz, width - 5, jz);
      XDrawString(display, win, gc, 0, 15, c1, strlen(c1));
      XDrawString(display, win, gc, 0, height - 30, c2, strlen(c2));
      XDrawString(display, win, gc, IDEC - 20, height - 5, c3, strlen(c3));
      XDrawString(display, win, gc, width - 90, height - 5, c4, strlen(c4));
      XDrawLines(display, win, gc, points, NBPOINTS, 0);
    }
  XUnloadFont(display, font_info->fid);
  XFreeGC(display, gc);
  XCloseDisplay(display);
  avma = av;
  killvalue(ep);
  return gnil;
}
	
GEN  plothraw(GEN listx, GEN listy)
{
  long av = avma,av2,i,lx,is,js;
  char c1[20],c2[20],c3[20],c4[20];
  XPoint *points;
  GEN p1,xsml,xbig,ysml,ybig,dx,dy,scal,scaly;

  Display *display;
  Window win;
  XSizeHints size_hints;
  XEvent report;
  GC gc;
  XFontStruct *font_info;
  int screen, width, height;
  
  if((typ(listx)<17)||(typ(listx)>18)||(typ(listy)<17)||(typ(listy)>18))
    err(ploter4);
  lx=lg(listx);
  if(lg(listy)!=lx) err(ploter5);
  if(lx==1) return gnil;
  points = (XPoint*)malloc(lx*sizeof(XPoint));
  if(!points) err(ploter6);
  if (!(display = XOpenDisplay(NULL))) err(ploter2);
  if (!(font_info = XLoadQueryFont(display, "9x15"))) err(ploter3);

  XSetErrorHandler((XErrorHandler)xerror);
  XSetIOErrorHandler((XIOErrorHandler)xerror);

  screen = DefaultScreen(display);
  width = DisplayWidth(display, screen) - 40;
  height = DisplayHeight(display, screen) - 60;
  is = width - IDEC - 5;
  js = height - 25;
  
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
	  scal=gun;dx=gsub(divis(stoi(is),2),xsml);
	  dy=gsub(divis(stoi(js),2),ysml);
	}
      else
	{
	  scal=gdivsg(js,dy);
	  dx=gneg(gmul(scal,xsml));dy=gneg(gmul(scal,ysml));
	}
    }
  else
    {
      scal=gdivsg(is,dx);
      if(!gcmp0(dy))
	{
	  scaly=gdivsg(js,dy);if(gcmp(scaly,scal)<0) scal=scaly;
	}
      dx=gneg(gmul(scal,xsml));dy=gneg(gmul(scal,ysml));
    }
  for(i = 0; i < lx-1; i++)
    {
      av2=avma;
      points[i].x = IDEC + itos(ground(gadd(gmul((GEN)listx[i+1],scal),dx)));
      points[i].y = js + JDEC - itos(ground(gadd(gmul((GEN)listy[i+1],scal),dy)));
      avma=av2;
    }
  xsml=gneg(gdiv(dx,scal));xbig=gdiv(gsubsg(is,dx),scal);
  ysml=gneg(gdiv(dy,scal));ybig=gdiv(gsubsg(js,dy),scal);
  p1=cgetr(4); gaffect(ybig,p1); sprintf(c1,"%9.3lf",rtodbl(p1));
  gaffect(ysml,p1); sprintf(c2,"%9.3lf",rtodbl(p1));
  gaffect(xsml,p1); sprintf(c3,"%9.3lf",rtodbl(p1));
  gaffect(xbig,p1); sprintf(c4,"%9.3lf",rtodbl(p1));

  win = XCreateSimpleWindow(display, RootWindow(display, screen), 0, 0, width, height, 4, BlackPixel(display, screen), WhitePixel(display, screen));

  size_hints.flags = PPosition | PSize;
  size_hints.x = 0;
  size_hints.y = 0;
  size_hints.width = width;
  size_hints.height = height;

  XSetStandardProperties(display, win, "plothraw", NULL, NULL, NULL, 0, &size_hints);

  XSelectInput(display, win, ExposureMask | ButtonPressMask);

  gc = XCreateGC(display, win, 0, NULL);
  XSetFont(display, gc, font_info->fid);
  XSetForeground(display, gc, BlackPixel(display, screen)); 

  XMapWindow(display, win);

  for(;;)
    {
      XNextEvent(display, &report);
      if (report.type != Expose) break;
      while (XCheckTypedEvent(display, Expose, &report));
      XDrawRectangle(display, win, gc, IDEC, JDEC, is, js);
      XDrawString(display, win, gc, 0, 15, c1, strlen(c1));
      XDrawString(display, win, gc, 0, height - 30, c2, strlen(c2));
      XDrawString(display, win, gc, IDEC - 20, height - 5, c3, strlen(c3));
      XDrawString(display, win, gc, width - 90, height - 5, c4, strlen(c4));
      XDrawPoints(display, win, gc, points, lx-1, 0);
    }
  XUnloadFont(display, font_info->fid);
  XFreeGC(display, gc);
  XCloseDisplay(display);
  free(points);avma = av;
  return gnil;
}

GEN rectdraw(GEN list)
{
  long *e,*p1,*ptx,*pty,*numpoints,*numtexts,*xtexts,*ytexts;
  long n,i,j,x0,y0,av=avma;
  long a,b,nd[10],ne;
  char **texts;

  Display *display;
  Window win;
  XSizeHints size_hints;
  XEvent report;
  GC gc;
  XFontStruct *font_info;
  int screen, width, height;
  XPoint *points,**lines;
  XSegment *segments;
  XRectangle *rectangles;

  if(typ(list)!=17) err(rploter3);
  n=lg(list)-1;if(n%3) err(rploter4);
  n=n/3;if(!n) return gnil;
  if (!(display = XOpenDisplay(NULL))) err(ploter2);
  if (!(font_info = XLoadQueryFont(display, "9x15"))) err(ploter3);
  XSetErrorHandler((XErrorHandler)xerror);
  XSetIOErrorHandler((XIOErrorHandler)xerror);
  screen = DefaultScreen(display);
  width = DisplayWidth(display, screen) - 40;
  height = DisplayHeight(display, screen) - 60; 
  nd[0]=nd[1]=nd[2]=nd[3]=nd[4]=nd[5]=nd[6]=0;
  for(i=0;i<n;i++)
    {
      if(typ((GEN)list[3*i+1])!=1) err(rploter5);
      ne=itos((GEN)list[3*i+1]);if((ne<0)||(ne>15)) err(rploter2);
      e=rectgraph[ne];
      p1=(long*)e[0];while((long)p1) 
	{
	  if(p1[1]!=4) nd[p1[1]]++;
	  else nd[1]+=p1[2];
	  p1=(long*)p1[0];
	}
    }
  points=(XPoint*)malloc(nd[1]*sizeof(XPoint));
  segments=(XSegment*)malloc(nd[2]*sizeof(XSegment));
  rectangles=(XRectangle*)malloc(nd[3]*sizeof(XRectangle));
  lines=(XPoint**)malloc(nd[5]*sizeof(XPoint*));
  numpoints=(long*)malloc(nd[5]*sizeof(long));
  texts=(char**)malloc(nd[6]*sizeof(char*));
  numtexts=(long*)malloc(nd[6]*sizeof(long));
  xtexts=(long*)malloc(nd[6]*sizeof(long));
  ytexts=(long*)malloc(nd[6]*sizeof(long));
  nd[1]=nd[2]=nd[3]=nd[5]=nd[6]=0;
  for(i=0;i<n;i++)
    {
      e=rectgraph[itos((GEN)list[3*i+1])];x0=list[3*i+2];y0=list[3*i+3];
      if((typ((GEN)x0)!=1)||(typ((GEN)y0)!=1)) err(rploter5);
      x0=itos((GEN)x0);y0=itos((GEN)y0);
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
	      lines[nd[5]]=(XPoint*)malloc(p1[2]*sizeof(XPoint));
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
  win= XCreateSimpleWindow(display, RootWindow(display, screen), 0, 0, width, height, 4, BlackPixel(display, screen), WhitePixel(display, screen));
  size_hints.flags = PPosition | PSize;
  size_hints.x = 0;
  size_hints.y = 0;
  size_hints.width = width;
  size_hints.height = height;
  
  XSetStandardProperties(display, win, "rectplot", NULL, NULL, NULL, 0, &size_hints);
  XSelectInput(display, win, ExposureMask | ButtonPressMask);
  gc = XCreateGC(display, win, 0, NULL);
  XSetFont(display, gc, font_info->fid);
  XSetForeground(display, gc, BlackPixel(display, screen)); 
  XMapWindow(display, win);
  for(;;)
    {
      XNextEvent(display, &report);
      if (report.type != Expose) break;
      while (XCheckTypedEvent(display, Expose, &report));
      if(nd[1]) XDrawPoints(display, win, gc, points, nd[1], 0);
      if(nd[2]) XDrawSegments(display, win, gc, segments, nd[2]);
      if(nd[3]) XDrawRectangles(display, win, gc, rectangles, nd[3]);
      for(i=0;i<nd[5];i++) XDrawLines(display,win,gc,lines[i],numpoints[i],0);
      for(i=0;i<nd[6];i++) XDrawString(display,win,gc,xtexts[i],ytexts[i],texts[i],numtexts[i]);
    }
  XUnloadFont(display, font_info->fid);
  XFreeGC(display, gc);
  free(points);free(segments);free(rectangles);
  free(numpoints);for(i=0;i<nd[5];i++) free(lines[i]);
  free(lines);free(texts);free(numtexts);free(xtexts);free(ytexts);
  XCloseDisplay(display);
  avma = av;return gnil;
}

