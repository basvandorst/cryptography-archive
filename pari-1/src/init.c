/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*                                                                 */
/*                                                                 */
/*              PROGRAMME D'INITIALISATION DU SYSTEME              */
/*                                                                 */
/*                    ET TRAITEMENT DES ERREURS                    */
/*                                                                 */
/*                       copyright Babe Cool                       */
/*                                                                 */
/*                                                                 */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


#include        "genpari.h"

const int STACKSIZE = 5000;  /* nombre de gn possibles */
const int TBLSZ = 135;  /* taille de la table de hashcodes */
const int MAXBLOC = 5000;  /* nombre de blocs autorises dans le tas */

const double K = 9.632959862*(BYTES_IN_LONG/4);  /* 32*log(2)/log(10)    */
const double K1 = 0.103810253/(BYTES_IN_LONG/4); /* log(10)/(32*log(2))  */
const double K2 = 1.1239968;               /* 1/(1-(log(2)/(2*pi)))*/
const double K4 = 0.5337333889;            /* e*pi/16              */
const double LOG2 = 0.693147180559945;       /* log(2)               */
const double L2SL10 = 0.301029995663981;     /* log(2)/log(10)       */
#ifndef  PI
const double PI = 3.141592653589;          /* pi                   */
#endif
const double rac5 = 2.23606797749;           /* racine de 5          */
const double C1 = 0.9189385332;            /* log(2*pi)/2          */
const double C2 = 22.18070978*(BYTES_IN_LONG/4);  /* 32*log(2)            */
const double C3 = 0.0216950598/(BYTES_IN_LONG/4); /* log((1+sqrt(5))/2)/(32*log(2)) */
#ifdef LONG_IS_64BIT
const double C31 = 9223372036854775808.0;  /* 2^63 */
#else
const double C31 = 2147483648.0;            /* 2^31                 */
#endif

#ifdef LONG_IS_32BIT
const long BIGINT = 32767;                  /* 2^15-1               */
const long EXP220 = 1048576;               /* 2^20                 */
const long VERYBIGINT = 2147483647;        /* 2^31-1               */
#endif

#ifdef LONG_IS_64BIT
const long BIGINT = 2147483647;                  /* 2^31-1               */
const long EXP220 = 1099511627776;              /* 2^40                 */
const long VERYBIGINT = 9223372036854775807;    /* 2^63-1               */
#endif

/*      Variables statiques communes :          */

unsigned long top,bot,avma;
long    avloc;
long    prec=5, precdl=16, defaultpadicprecision=16;
long    tglobal,paribuffsize=30000,pariecho=0;
jmp_buf environnement;
FILE    *outfile;
FILE    *errfile;
FILE    *logfile;
FILE    *infile;
long    nvar = 0;
GEN     gnil,gzero,gun,gdeux,ghalf,polvar,gi,RAVYZARC;
GEN     gpi=(GEN)0;
GEN     geuler=(GEN)0;
GEN     bernzone=(GEN)0;
entree  **varentries, **hashtable;
GEN     *blocliste, *polun, *polx, *g;
long    *ordvar,varchanged=0;
long    nextbloc = 0;
#ifdef LONG_IS_64BIT
long    glbfmt[]={'g',0,38};
#else
long    glbfmt[]={'g',0,28};
#endif
long    **rectgraph;
long    pari_randseed;
long    DEBUGLEVEL = 0;

byteptr diffptr;
#ifdef LONG_IS_64BIT
long    lontyp[30]={0,0x100000000,0x100000000,1,1,1,1,2,1,1,2,2,0,1,1,1,1,1,1,1};
long    lontyp2[30]={0,0x100000000,0x100000000,2,1,1,1,3,2,2,2,2,0,1,1,1,1,1,1,1};     
#else
long    lontyp[30]={0,0x10000,0x10000,1,1,1,1,2,1,1,2,2,0,1,1,1,1,1,1,1};
long    lontyp2[30]={0,0x10000,0x10000,2,1,1,1,3,2,2,2,2,0,1,1,1,1,1,1,1};     
#endif
void    (*printvariable)(long);

     
     /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
     /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
     /*                                                                 */
     /*                      INITIALISATION DU SYSTEME                  */
     /*                                                                 */
     /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
     /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

void catchinterrupt(int truc)
{
  truc = truc; /* Only way to keep quiet all ANSI compilers... */
  signal(SIGINT,catchinterrupt);
  err(interrupter);
}

void catchbus(int truc)
{
  truc = truc; 
  signal(SIGBUS,catchbus);
  err(talker,"bus error: bug in GP or in calling program");
}

void catchsegv(int truc)
{
  truc = truc; 
  signal(SIGSEGV,catchsegv);
  err(talker,"segmentation fault: bug in GP or in calling program");
}

#ifdef __cplusplus
void init(long parisize, long maxprime, void (*printvar)(long)=printvargp)
#else
void init(long parisize, long maxprime)
#endif
{
  long v, n, *e;
  char *p;
  GEN p1;
  
#ifdef __cplusplus
  printvariable=printvar;
#else
  printvariable=printvargp;
#endif

  outfile = stdout;errfile = stderr;logfile = NULL;infile = stdin;
  if (setjmp(environnement))
  {
    fprintf(errfile, "\n  ***   Error in the PARI system. End of the program.\n");
    exit(1);
  }
  signal(SIGINT,catchinterrupt);
  signal(SIGBUS,catchbus);
  signal(SIGSEGV,catchsegv);
  pari_randseed=1;
  if (!(diffptr=initprimes(maxprime))) err(memer);
  if (!(bot=(long)malloc(parisize))) err(memer);
  top=avma=bot+parisize;
  if (!(varentries=(entree **)malloc(sizeof(entree*)*MAXVAR))) err(memer);
  if (!(hashtable=(entree **)malloc(sizeof(entree*)*TBLSZ))) err(memer);
  if (!(blocliste=(GEN *)malloc(sizeof(GEN)*MAXBLOC))) err(memer);
  if (!(ordvar=(long *)malloc(sizeof(long)*MAXVAR))) err(memer);
  if (!(polun=(GEN *)malloc(sizeof(GEN)<<MAXSHIFTVAR))) err(memer);
  if (!(polx=(GEN *)malloc(sizeof(GEN)<<MAXSHIFTVAR))) err(memer);
  if (!(g=(GEN *)malloc(sizeof(GEN)*STACKSIZE))) err(memer);
  if (!(rectgraph=(long**)malloc(sizeof(long*)*16))) err(memer);
  for(n=0;n<16;n++) 
    {
      if(!(e=rectgraph[n]=(long*)malloc(sizeof(long)*10))) err(memer);
      e[4]=lgetr(3);e[5]=lgetr(3);e[6]=lgetr(3);e[7]=lgetr(3);
      e[8]=lgetr(3);e[9]=lgetr(3);
    }
  for(n = 0; n < TBLSZ; n++) hashtable[n] = NULL;
  for(v = 0; v < NUMFUNC; v++)
  {
    for(n = 0, p = fonctions[v].name; *p; p++) n = n << 1 ^ *p;
    if (n < 0) n = -n; n %= TBLSZ;
    fonctions[v].next = hashtable[n];
    hashtable[n] = fonctions + v;
  }
  gnil = cgeti(2);gnil[1]=2; setpere(gnil,MAXUBYTE);
  gzero = cgeti(2);gzero[1]=2; setpere(gzero, MAXUBYTE);
  gun = stoi(1); setpere(gun, MAXUBYTE);
  gdeux = stoi(2); setpere(gdeux, MAXUBYTE);
  ghalf = cgetg(3,4);ghalf[1]=un;ghalf[2]=deux; setpere(ghalf, MAXUBYTE);
  gi = cgetg(3,6); gi[1] = zero; gi[2] = un; setpere(gi, MAXUBYTE);
  p1=cgetg(4,10);p1[1]=evalsigne(1)+evalvarn(MAXVARN)+evallgef(4);
  p1[2]=zero;p1[3]=un;polx[MAXVARN]=p1;
  p1=cgetg(3,10);p1[1]=evalsigne(1)+evalvarn(MAXVARN)+evallgef(3);
  p1[2]=un;polun[MAXVARN]=p1;
  for(v=0; v < MAXVAR; v++) ordvar[v] = v;
  polvar = cgetg(MAXVAR + 1,17); setlg(polvar,1); setpere(polvar, MAXUBYTE);
  for(v=1;v<=MAXVAR;v++) polvar[v]=evaltyp(17)+evalpere(MAXUBYTE)+evallg(1);
  for(v = 0; v < MAXBLOC; v++) blocliste[v] = (GEN)0;
  for(v = 0; v < STACKSIZE; v++) g[v] = gzero;
  lisseq("x");avloc=avma;
}

GEN geni(void)
{
  return gi;
}

long marklist(void)
{
  long i;
  GEN x, *p = blocliste;
  for (i = 0; i < MAXBLOC; i++)
    if(x = blocliste[i])
    {
      x[-2] = (long)p;
      *p++ = x;
    }
  for (nextbloc = i = p - blocliste; i < MAXBLOC; i++)
    blocliste[i] = 0;
  return nextbloc;
}

GEN newbloc(long n)
{
  long i, *x;
  for(i = nextbloc; i < MAXBLOC; i++) if (!blocliste[i]) break;
  if (i == MAXBLOC)
  {
    for (i = 0; i < nextbloc; i++) if (!blocliste[i]) break;
    if (i == nextbloc) err(newblocer1);
  }
  x = (long *)malloc((n << TWOPOTBYTES_IN_LONG) + 2*BYTES_IN_LONG);
  if (!x) err(memer);
  x += 2;
  x[-2] = (long)(blocliste + i);
  x[-1] = 0;
  blocliste[i] = x;
  nextbloc = i + 1;
  return x;
}

void killbloc(GEN x)
{
  if (!x || isonstack(x)) return;
  *(long *)x[-2] = 0;
  free(x-2);
}

void newvalue(entree *ep, GEN val)
{
  GEN y = gclone(val);
  y[-1] = (long) ep->value;
  ep->value = (void *)y;
}

void changevalue(entree *ep, GEN val)
{
  GEN y = gclone(val);
  GEN x = (GEN)ep->value;
  ep->value = (void *)y;
  if ((long)x - (long)ep == sizeof(entree)) 
  {
    y[-1] = (long)x;
    return;
  }
  y[-1] = x[-1];
  killbloc(x);
}

void killvalue(entree *ep)
{
  GEN x = (GEN)ep->value;
  if ((long)x - (long)ep == sizeof(entree)) return;
  ep->value = (void *)x[-1];
  killbloc(x);
}


void install(GEN f (/* ??? */), char *name, int valence)
{
  int n;
  entree *ep;
  char *p;
  
  if ((valence < 0) || (valence > 3)) err(valencer1);
  for(n = 0, p = name; *p; p++) n = n << 1 ^ *p;
  if (n < 0) n = -n; n %= TBLSZ;
  for(ep = hashtable[n]; ep; ep = ep->next)
    if (!strcmp(name, ep->name)) err(nomer1);
  ep = (entree *)malloc(sizeof(entree) + strlen(name) + 1);
  ep->name = (char *)ep + sizeof(entree); strcpy(ep->name, name);
  ep->value = (void *)f;
  ep->valence = valence;
  ep->menu = 0;
  ep->next = hashtable[n];
  hashtable[n] = ep;
}

void preserve(long av, long nb)
{
  GEN q,**s;
  long i,tetpil=avma;
  for(s=(GEN**)&nb,i=1; i<nb; i++) {s++; **s = gcopy(**s);}
  q=cgetg(nb+1,17);
  for(s=(GEN**)&nb,i=1; i<nb; i++) q[i]=(long)**++s;
  q=gerepile(av, tetpil,q);
  for(s=(GEN**)&nb,i=1; i<nb; i++) **++s=(GEN)q[i];
  avma+=(nb+1)*sizeof(long);
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*                                                                 */
/*              TRAITEMENT DES ERREURS                             */
/*                                                                 */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

void err(long numerr, ...)
{
  char c;
  FILE *temp;
  va_list poer;
  char *ch;
  GEN noninv;

  va_start(poer,numerr);
  fprintf(errfile, "  ***   %s",errmessage[numerr]);
  switch (numerr)
  {
    case matcher1:
      ch=va_arg(poer, char*);c = *ch++;
      fprintf(errfile, "'%c'\n  ***   instead of: '%s'", c, ch); break;
    case impl: ch=va_arg(poer, char*);
      fprintf(errfile, " %s is not yet implemented.",ch); break;
    case talker: ch=va_arg(poer, char*);
      fprintf(errfile, "%s.",ch); break;
    case varer1:
    case unknowner1:
    case caracer1: ch=va_arg(poer, char*);fprintf(errfile, "'%s'",ch);break;
    case invmoder: ch=va_arg(poer, char*);noninv=va_arg(poer, GEN);
      temp=outfile;outfile=errfile;fprintf(errfile,": ");
      brute(noninv,'g',-1);outfile=temp;break;
    case errpile: putc('\n', errfile);allocatemoremem(0);break;
  }
  putc('\n', errfile);
  va_end(poer);
  longjmp(environnement, numerr);
}

void recover(long listloc)
{
  long i, m, n;
  GEN x;
  entree *ep, *ep2;

  for (n = 0; n < TBLSZ; n++)
    for (ep = hashtable[n]; ep;)
      if (ep->valence >= 100)
      {
        x = (GEN)ep->value;
        if ((long)x - (long)ep == sizeof(entree))
        {
          if (ep->valence == 200) ep = ep->next;
          else
            if (ep == hashtable[n])
            {
              hashtable[n] = ep->next;
              free(ep);
              ep = hashtable[n];
            }
            else
            {
              for(ep2 = hashtable[n]; ep2->next != ep; ep2 = ep2->next);
              ep2->next = ep->next;
              free(ep); ep = ep2->next;
            }
          continue;
        }
        m = (long *)x[-2] - (long *)blocliste;
        if ((m < listloc) || (m >= MAXBLOC)) ep=ep->next;
        else killvalue(ep);
      }
      else ep = ep->next;
  for (i = listloc; i < MAXBLOC; i++)
    if ((x = blocliste[i]) && (x != gpi) && (x != geuler))
      killbloc(x);
}

void allocatemoremem(unsigned long newsize)
{
  long av,declg,declg2,tl,parisize,v;
  GEN ll,pp,l1,l2,l3;
  unsigned long topold,avmaold,botold;

  avmaold=avloc;topold=top;botold=bot;
  if(newsize<3) parisize=(topold-botold)<<1;
  else 
    {
      if(newsize<(topold-avmaold)) 
	err(talker,"required stack memory too small");
      else parisize=newsize+16-((newsize-1)&15+1);
    }
  if (!(bot=(long)malloc(parisize))) err(nomer2);
  if(!newsize)
    {
      fprintf(errfile, "  *** Warning: doubling the stack size; new stack = %d\n",parisize);
      fprintf(errfile, "  *** Please reissue the same command if you are under GP\n");
    }
  top=avma=bot+parisize;
  declg=(long)top-(long)topold;declg2=declg>>TWOPOTBYTES_IN_LONG;
  for(ll=(GEN)top,pp=(GEN)topold;pp>(GEN)avmaold;) *--ll= *--pp;
  av=(long)ll;
  while(ll<(GEN)top)
  {
    l2=ll+lontyp[tl=typ(ll)];
    if(tl==10) {l3=ll+lgef(ll);ll+=lg(ll);if(l3>ll) l3=l2;}
    else {ll+=lg(ll);l3=ll;} 
    for(;l2<l3;l2++) 
      {
	l1=(GEN)(*l2);
	if((l1<(GEN)topold)&&(l1>=(GEN)avmaold)) *l2+=declg;
      }
  }
  gnil+=declg2;gzero+=declg2;gun+=declg2;gdeux+=declg2;ghalf+=declg2;
  gi+=declg2;polx[MAXUBYTE]+=declg2;polun[MAXUBYTE]+=declg2;polvar+=declg2;
  for(v=0;v<=tglobal;v++)
    if((g[v]<(GEN)topold)&&(g[v]>=(GEN)avmaold)) g[v]+=declg2;
  free((void *)botold);avloc=avma=av;
}

GEN allocatemem(unsigned long newsize)
{
  allocatemoremem(newsize);
  longjmp(environnement,errpile);
  return gnil; /*inutile mais ca fait plaisir a des compilos */
}
