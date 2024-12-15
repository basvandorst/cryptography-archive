/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
/*@                                                               @*/
/*@                        PARI CALCULATOR                        @*/
/*@                                                               @*/
/*@                      copyright Babe Cool                      @*/
/*@                                                               @*/
/*@                                                               @*/
/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/

#include "genpari.h"

long    champ, dec, nbchi, avglob;
extern  long avloc;
long    prettyp = 1, chrono = 0, simplifyflag = 1;
char    prompt[79], format;

#define NUMGLOB sizeof(globales)/sizeof(char*)
  
  static char *globales[] = {"precision", "serieslength", "format", "prompt"};

void escape(char *tch, long parisize, long primelimit);
void commands(int n), menucommands(void), slashcommands(void), gentypes(void), aide(char *s), globs(long parisize, long primelimit);

char* findsep(char **t)
{
  char *s1;
  static char s2[80];
  int i;
  
  for(s1 = *t, i = 0; (i < 79) && (*s1) && (!separe(*s1)); i++) s2[i] = *s1++;
  while ((*s1) && (!separe(*s1))) s1++;
  s2[i] = 0; *t = s1;
  return s2;
}

void checkok(char *t)
{
  char c = *t;
  if((c) && (!separe(c))) err(caracer1,t);
}

long getint(char *ch, long n)
{
  long av = avma;
  filtre(ch);
  if (*ch) n = itos(readexpr(&ch));
  avma = av; return n;
}
void usage(char *s)
{
  pariputs("   ### usage: ");
  pariputs(s);
  pariputs("[-s stacksize] [-p primelimit] [-b buffersize]\n");
  exit(0);
}

int main(long argc, char **argv)
{
  
  long  i,typy, parisize, primelimit, silent, avs, tets;
  static long tloc,listloc;
  char  *buffer, *tch, *tch2, thestring[100];
  GEN  z;
  long tmpparibuffsize=0;

#ifdef macintosh
  strcpy(prompt,"?\n"); parisize = 1000000; primelimit = 200000;
#else
  strcpy(prompt,"? "); parisize = 4000000; primelimit = 500000;
#endif

  for(i = 1; i < argc; i++)
    {
      tch = argv[i++];
      if ((i == argc) || (*tch++ != '-')) usage(argv[0]);
      if (*tch == 's') parisize = atoi(argv[i]);
      else if (*tch == 'p') primelimit = atoi(argv[i]);
      else if (*tch == 'b') tmpparibuffsize = atoi(argv[i]);
      else usage(argv[0]);
    }
  
  init(parisize, primelimit);
  printversion();
  pariputs("\nCopyright 1989,1993 by C. Batut, D. Bernardi, H. Cohen and M. Olivier\n\n");
  if(tmpparibuffsize) paribuffsize=tmpparibuffsize;
  buffer = (char *)malloc(paribuffsize);

  avglob = avloc = avma;
  tglobal=chrono=0;simplifyflag=1;
#ifdef LONG_IS_64BIT
  prec=4;dec=38;nbchi=38;
#else
  prec=5;dec=28;nbchi=28;
#endif
  champ=0;format='g';
  
  pariputs("Type ? for help\n\n");
  globs(top-bot,primelimit);
  
  for(;;)
    {
      avloc = avma; tloc = tglobal; listloc = marklist();
      if (setjmp(environnement)) {avma = avloc; tglobal = tloc; recover(listloc);}
      if(infile==stdin) pariputs(prompt);
      if (!fgets(buffer, paribuffsize, infile)) {switchin(NULL); continue;}
      if (pariecho) pariputs(buffer); else if (logfile) fputs(buffer, logfile);
      tch = buffer + 1;
      switch(buffer[0])
	{
	case '#':
	  checkok(tch);
	  pariputs((chrono = !chrono) ? "    timer on\n" : "    timer off\n");
	  continue;
	case '?': aide(findsep(&tch)); pariputc('\n'); continue;
	case '\\': escape(tch,parisize,primelimit); continue;
	case '{':
	  for(;;)
	    {
	      tch2 = buffer + strlen(buffer) - 1;
	      if (*tch2 == '\n') tch2--;
	      if (*tch2 == '}') {*tch2-- = 0; break;}
	      if (*tch2 != '\\') tch2++;
	      if(!fgets(tch2, paribuffsize - (tch2 - buffer), infile)) break;
	      if((*tch2=='\\')&&(tch2[1]=='\\')) *tch2=0;
	      if(pariecho) pariputs(tch2); else if (logfile) fputs(tch2, logfile);
	    }
	  break;
	default:
	  for(tch--;;)
	    {
	      tch2 = buffer + strlen(buffer) - 1;
	      if (*tch2 == '\n') tch2--;
	      if (*tch2 != '\\') {tch2[1] = 0; break;}
	      if(!fgets(tch2, paribuffsize - (tch2 - buffer), infile)) break;
	      if(pariecho) pariputs(tch2); else if (logfile) fputs(tch2, logfile);
	    }							
	  break;
	}
      silent = separe(*tch2);
      filtre(tch);
      fflush(outfile); if (logfile) fflush(logfile);
      if(!tglobal) timer2();
      if (chrono) timer();
      avs=avma;z = readseq(&tch);
      nbchi=dec=glbfmt[2];
      if (*tch) {pariputs("  unused characters: "); pariputs(tch); pariputc('\n');}
      if (chrono)
	{
	  long delay = timer();
	  pariputs("time = ");
	  if (delay >= 3600000)
	    {
	      sprintf(thestring, "%dh, ", delay / 3600000);
	      delay %= 3600000;
	      pariputs(thestring);
	    }
	  if (delay >= 60000)
	    {
	      sprintf(thestring, "%dmn, ", delay / 60000);
	      delay %= 60000;
	      pariputs(thestring);
	    }
	  if (delay >= 1000)
	    {
	      sprintf(thestring, "%d,", delay / 1000);
	      delay %= 1000;
	      pariputs(thestring);
              if (delay < 100) pariputc('0');
              if (delay < 10) pariputc('0');
	    }
	  sprintf(thestring, "%d ms\n", delay);
	  pariputs(thestring);
	}
      if (z == gnil) continue;
      if(simplifyflag) 
	{
	  if(isonstack(z))
	    {
	      tets = avma;
	      z = g[0] = g[++tglobal] = gerepile(avs,tets,simplify(z));
	    }
	  else {z = g[0] = g[++tglobal] = simplify(z);}
	}
      else {g[0] = g[++tglobal] = isonstack(z) ? z : gcopy(z);}
      typy=typ(z);
      if (!separe(*tch2))
	{
	  sprintf(thestring, "%%%d = ",tglobal);
	  pariputs(thestring);
	  if ((typy > 16) && (prettyp==2)) pariputc('\n');;
	  if(nbchi < 0)
	    if(prettyp==2) sor(z, format, -1, champ);
	    else if(prettyp) matbrute(z, format, -1);
	    else brute(z, format, -1);
	  else
	    if (typy < 3) ecrire(z, format, nbchi, 0);
	    else 
	      if(prettyp==2) sor(z, format, nbchi, champ);
	      else if(prettyp) matbrute(z, format, nbchi);
	      else brute(z, format, nbchi);
	  pariputc('\n'); 
	}
    } /* for(;;) */
  return 0;
} /* main */

/********************************************************************/
/********************************************************************/
/**                                                                **/
/**                    COMMANDES COMMENCANT PAR \                  **/
/**                                                                **/
/**                     ET ANALOGUES DANS ANAL.C                   **/
/**                                                                **/
/********************************************************************/
/********************************************************************/

void escape(char *tch, long parisize, long primelimit)
{
  int i, d;
  char c, *s1, *s2, thestring[50];
  
  for (i=0;i<NUMGLOB;i++)
    {
      s1 = tch;
      s2 = globales[i];
      while ((*s2) && (*s1 == *s2)) {s1++; s2++;}
      while (isspace(*s1)) s1++;
      if (!*s2 && (*s1++ == '=')) 
	switch (i) 
	  {
	  case 0: 
	    glbfmt[2] = nbchi = dec = getint(s1, dec);
	    prec = (long)(dec*K1+3);
	    sprintf(thestring, "   precision = %d significant digits\n",dec);
	    pariputs(thestring);
	    return;
	  case 1:
	    precdl = getint(s1, precdl);
	    sprintf(thestring, "   series precision = %d significant terms\n",precdl);
	    pariputs(thestring);
	    return;
	  case 2:
	    format = *s1++;
	    if(isdigit(*s1))
	      for(champ = 0; isdigit(*s1); s1++)
		champ = 10 * champ + *s1 - '0';
	    if(*s1++ == '.')
	      if(*s1 == '-')
		nbchi = -1;
	      else
		if(isdigit(*s1))
		  for(nbchi = 0; isdigit(*s1); s1++)
		    nbchi = 10 * nbchi + *s1 - '0';
	    sprintf(thestring, "   real format = %c%d.%d\n", format, champ, nbchi);
	    pariputs(thestring);
	    glbfmt[0] = format; glbfmt[1] = champ; glbfmt[2] = nbchi;
	    return;
	  case 3:
	    strcpy(prompt, findsep(&s1));
#ifdef macintosh
	    strcat(prompt,"\n");
#else
	    strcat(prompt," ");
#endif
	    return;
	  }
    }
  c = *tch++;
  switch (isupper(c) ? tolower(c) : c)
    {
    case 'a': brute(g[getint(tch, tglobal)], format, -1);pariputc('\n');break;
    case 'b': sor(g[getint(tch, tglobal)], format, -1, champ);pariputc('\n');
      break;
    case 'c': checkok(tch); commands(-1); break;
    case 'd': checkok(tch); globs(top-bot,primelimit); break;
    case 'e': checkok(tch); pariecho = !pariecho; break;
    case 'g': while(isspace(*tch)) tch++;
      for (d = 0; isdigit(*tch);) d = 10 * d + *tch++ - '0';
      sprintf(thestring, "    debug level = %ld\n", d);DEBUGLEVEL = d;
      pariputs(thestring);break;
    case 'k': checkok(tch);
      avma = avloc = avglob;
      tglobal = chrono = 0; simplifyflag = 1;
      gpi = geuler = bernzone = (GEN)0;
      pari_randseed = 1; DEBUGLEVEL = 0;
#ifdef LONG_IS_64BIT
      prec=4;dec=38;nbchi=38;
#else
      prec=5;dec=28;nbchi=28;
#endif
      champ=0;format='g';

#ifdef macintosh
      strcpy(prompt,"?\n");
#else
      strcpy(prompt,"? ");
#endif
      for (i = 0; i < STACKSIZE; i++) g[i] = gzero;
      globs(top-bot,primelimit);
      break;
    case 'l': checkok(tch); fliplog(); break;
    case 'm': matbrute(g[getint(tch, tglobal)], format, -1);pariputc('\n');
      break;
    case 'p': checkok(tch); prettyp = (prettyp==2)?0:prettyp+1;
      if(prettyp==2) sprintf(thestring, "   default format: prettyprint\n");
      else if(prettyp) sprintf(thestring, "   default format: prettymatrix\n");
      else sprintf(thestring, "   default format: raw\n");
      pariputs(thestring);break;
    case 'q': exit(0);
    case 'r': while(isspace(*tch)) tch++; switchin(findsep(&tch)); break;
    case 's': etatpile(getint(tch, 0)); break;
    case 't': checkok(tch); gentypes(); break;
    case 'v': checkok(tch); printversion(); break;
    case 'w':
      while(isspace(*tch)) tch++;
      for (d = 0; isdigit(*tch);) d = 10 * d + *tch++ - '0';
      while(isspace(*tch)) tch++;
      switchout(findsep(&tch));
      brute(g[d ? d : tglobal], format, -1);
      pariputc('\n'); switchout(NULL); break;
    case 'x': voir(g[tglobal], getint(tch, -1)); break;
    case 'y': pariputs("    automatic simplification ");
      pariputs((simplifyflag = !simplifyflag) ? "on\n" : "off\n"); break;
    case '\\': break;
    default: err(caracer1,tch+1);
    }
}

/********************************************************************/
/********************************************************************/
/**                                                                **/
/**           AFFICHAGE TYPES, COMMANDES AIDES ET GLOBALES         **/
/**                                                                **/
/********************************************************************/
/********************************************************************/

void gentypes(void)
{
  pariputs("\n      List of the PARI types :");
  pariputs("\n     -------------------------\n\n");
  pariputs("  1  :long integers     [ cod1 ] [ cod2 ] [ man1 ] ... [ manl ]\n");
  pariputs("  2  :long real numbers [ cod1 ] [ cod2 ] [ man1 ] ... [ manl ]\n");
  pariputs("  3  :integermods       [ code ] [ mod  ] [ integer ]\n");
  pariputs("  4  :irred. rationals  [ code ] [ num. ] [ den. ] \n");
  pariputs("  5  :rational numbers  [ code ] [ num. ] [ den. ] \n");
  pariputs("  6  :complex numbers   [ code ] [ real ] [ imag ] \n");
  pariputs("  7  :p-adic numbers    [ cod1 ] [ cod2 ] [ p ] [ p^r ] [ integer]\n");
  pariputs("  8  :quadratic numbers [ cod1 ] [ mod  ] [ real ] [ imag ]\n");
  pariputs("  9  :polymods          [ code ] [ mod  ] [ polynomial ]\n");
  pariputs(" -------------------------------------------------------------\n");
  pariputs("  10 :polynomials       [ cod1 ] [ cod2 ] [ man1 ] ... [ manl ]\n");
  pariputs("  11 :power series      [ cod1 ] [ cod2 ] [ man1 ] ... [ manl ]\n");
  pariputs("  13 :irred. rat. func. [ code ] [ num. ] [ den. ]\n");
  pariputs("  14 :rational function [ code ] [ num. ] [ den. ]\n");
  pariputs("  17 :row vector        [ code ] [  x1  ] ... [  xl  ]  \n");
  pariputs("  18 :column vector     [ code ] [  x1  ] ... [  xl  ]  \n");
  pariputs("  19 :matrix            [ code ] [ col1 ] ... [ coll ]\n");
}

void commands(int n)
{
  int i, w, lig = 0, col = 0;
  
  pariputc('\n');
  for (i = 0; i < NUMFUNC; i++)
    {
      if((n<0)||(fonctions[i].menu==n))
	{
	  w = strlen(fonctions[i].name);
	  if ((col == 72) || (col + w >= 80))
	    {
	      pariputc('\n'); col = 0;
	      if(n<0)
		{
		  if (!(++lig % 10)) pariputc('\n');
		  if (!(lig % 20))
		    {pariputs("---- (type return to continue) ----\n");getchar();}
		}
	    }
	  pariputs(fonctions[i].name);
	  col += w;
	  do {pariputc(' '); col++;} while (col % 12);
	}
    }
  pariputc('\n');
}

void menucommands(void)
{
  pariputc('\n');
  pariputs("1: Standard monadic or dyadic OPERATORS\n");
  pariputs("2: CONVERSIONS and similar elementary functions\n");
  pariputs("3: TRANSCENDENTAL functions\n");
  pariputs("4: NUMBER THEORETICAL functions\n");
  pariputs("5: Functions related to ELLIPTIC CURVES\n");
  pariputs("6: Functions related to general NUMBER FIELDS\n");
  pariputs("7: POLYNOMIALS and power series\n");
  pariputs("8: Vectors, matrices and LINEAR ALGEBRA\n");
  pariputs("9: SUMS, products, integrals and similar functions\n");
  pariputs("10: GRAPHIC functions\n");
  pariputs("11: PROGRAMMING under GP\n");
  pariputc('\n');
  pariputs("Further help: ?n (1<=n<=11), ?functionname, or ?\\ (keyboard commands)\n");
}

void slashcommands(void)
{
  pariputc('\n');
  pariputs("# : enable/disable timer\n");
  pariputs("\\\\ : start a comment line\n");
  pariputs("\\a : print result in raw format (readable by PARI)\n");
  pariputs("\\b : print result in beautified format\n");
  pariputs("\\c : list all commands (same effect as ?*)\n");
  pariputs("\\d : give current defaults and syntax to change them\n");
  pariputs("\\e : enable/disable echo\n");
  pariputs("\\g : set debugging level\n");
  pariputs("\\k : kill current GP session but do not exit\n");
  pariputs("\\l : enable/disable logfile\n");
  pariputs("\\m : print result in prettymatrix format\n");
  pariputs("\\p : change default output format (3-way switch)\n");
  pariputs("\\q : quit completely this GP session\n");
  pariputs("\\r : read a file\n");
  pariputs("\\s : print stack information\n");
  pariputs("\\t : print the list of PARI types\n");
  pariputs("\\v : print current version of GP\n");
  pariputs("\\w : write to a file\n");
  pariputs("\\x : print complete inner structure of last result\n");
  pariputs("\\y : disable/enable automatic simplification\n");
}

void globs(long parisize, long primelimit)
{
  int i, j;
  char thestring[70];
  
  for (i = 0; i < NUMGLOB; i++)
    {
      pariputc('\\'); pariputs(globales[i]);
      for(j = strlen(globales[i]); j < 15; j++) pariputc(' ');
      pariputs("= ");
      switch (i)
	{
	case 0: sprintf(thestring, "%d",dec);break;
	case 1: sprintf(thestring, "%d",precdl);break;
	case 2: sprintf(thestring, "%c%d.%d",format,champ,nbchi);break;
	case 3: sprintf(thestring, "%s",prompt);break;
	}
      pariputs(thestring); pariputc('\n');
    }
  sprintf(thestring, "stacksize = %ld, prime limit = %ld, buffersize = %ld",parisize, primelimit, paribuffsize);pariputs(thestring);
  pariputc('\n');
}

void aide(char *s)
{
  long  i, n, m, nparam;
  char  *u = s;
  entree *ep, **q;
  
  if (!*s) {menucommands(); return;}
  if (*s=='\\') {slashcommands(); return;}
  if (*s=='*') {commands(-1);return;}
  if ((*s>='1')&&(*s<='9'))
    {
      m=*s-'0';if ((s[1]>='0')&&(s[1]<='9')) m=10*m+s[1]-'0';
      commands(m);return;
    }
  for (n=0;n<NUMFUNC;n++)
    if(!strcmp(fonctions[n].name,s))
      {pariputs(helpmessage[n]); pariputc('.'); return;}
  for(n = 0; isalnum(*u); u++) n = n << 1 ^ *u;
  if (n < 0) n = -n; n %= TBLSZ;
  for(ep = hashtable[n]; ep; ep = ep->next)
    if(!strcmp(ep->name,s))
      {
	if (ep->valence != 100) break;
	q = (entree **)(ep->value);
	nparam = (long)*q++;
	pariputs(ep->name);
	pariputc('(');
	for(i = 0; i < nparam; i++)
	  {
	    if(i) pariputc(',');
	    pariputs((*q++)->name);
	  }
	pariputs(")= ");
	pariputs((char*)q);
	return;
      }
  pariputs("Unknown function\n");
}
