/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*                                                                 */
/*            Analyseur syntactique pour la calculette             */
/*                                                                 */
/*                       copyright Babe Cool                       */
/*                                                                 */
/*                                                                 */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include "genpari.h"

GEN seq(void), expr(void), facteur(void), truc(void), identifier(void), constante();
void skipseq(void), skipexpr(void), skipfacteur(void), skiptruc(void), skipconstante(void), skipidentifier(void);
entree *findentry(void), *skipentry(void);

static char *analyseurs,*labellist[100];
static long analyseurtetpil;

GEN lisexpr(char *t)
{
  GEN res;
  long av, oldtetpil = analyseurtetpil;
  char *olds = analyseurs;
  analyseurs = t; analyseurtetpil = av = avma;
  res = expr();
  res = gerepile(av, analyseurtetpil, res);      
  analyseurtetpil = oldtetpil; analyseurs = olds;
  return res;
}

GEN readexpr(char **c)
{
  char *olds = analyseurs, *oldc = *c;
  analyseurs = oldc; skipexpr();
  if ((*analyseurs) && !separe(*analyseurs)) err(caracer1, analyseurs);
  *c = analyseurs; analyseurs = olds;
  return lisexpr(oldc);
}

GEN lisseq(char *t)
{
  GEN res;
  long av, oldtetpil = analyseurtetpil;
  char *olds = analyseurs;
  analyseurs = t; analyseurtetpil = av = avma;
  res = seq();
  res = gerepile(av, analyseurtetpil, res);      
  analyseurtetpil = oldtetpil; analyseurs = olds;
  return res;
}

GEN readseq(char **c)
{
  long i;
  char *olds = analyseurs, *oldc = *c;
  for(i=0;i<100;i++) labellist[i]=(char*)0;
  analyseurs = oldc; skipseq();
  *c = analyseurs; analyseurs = olds;
  return lisseq(oldc);
}

entree fonctions[]={
  {"O",50,0,7,0},
  {"abs",1,(void *)gabs,3,0},
  {"acos",1,(void *)gacos,3,0},
  {"acosh",1,(void *)gach,3,0},
  {"addell",3,(void *)addell,5,0},
  {"adj",1,(void *)adj,8,0},
  {"agm",2,(void *)agm,3,0},
  {"akell",2,(void *)akell,5,0},
  {"algdep",23,(void *)algdep,8,0},
  {"algdep2",33,(void *)algdep2,8,0},
  {"allocatemem",11,(void *)allocatemem,2,0},
  {"anell",23,(void *)anell,5,0},
  {"apell",2,(void *)apell,5,0},
  {"apell2",2,(void *)apell2,5,0},
  {"apprpadic",2,(void *)apprgen9,7,0},
  {"arg",1,(void *)garg,3,0},
  {"asin",1,(void *)gasin,3,0},
  {"asinh",1,(void *)gash,3,0},
  {"assmat",1,(void *)assmat,8,0},
  {"atan",1,(void *)gatan,3,0},
  {"atanh",1,(void *)gath,3,0},
  {"base",13,(void *)base,6,0},
  {"base2",13,(void *)base2,6,0},
  {"bernreal",11,(void *)bernreal,3,0},
  {"bernvec",11,(void *)bernvec,3,0},
  {"bestappr",2,(void *)bestappr,4,0},
  {"bezout",2,(void *)vecbezout,4,0},
  {"bigomega",1,(void *)gbigomega,4,0},
  {"bilhell",3,(void *)bilhell,5,0},
  {"bin",21,(void *)binome,4,0},
  {"binary",1,(void *)binaire,2,0},
  {"bittest",2,(void *)gbittest,2,0},
  {"boundcf",21,(void *)gboundcf,4,0},
  {"boundfact",21,(void *)boundfact,4,0},
  {"box",35,(void *)rectbox,10,0},
  {"buchgen",3,(void *)buchgen,6,0},
  {"buchgenfu",3,(void *)buchgenfu,6,0},
  {"buchimag",3,(void *)buchimag,4,0},
  {"buchinit",3,(void *)buchinit,6,0},
  {"buchinitfu",3,(void *)buchinitfu,6,0},
  {"buchreal",4,(void *)buchreal,4,0},
  {"bytesize",10,(void *)taille2,2,0},
  {"ceil",1,(void *)gceil,2,0},
  {"centerlift",1,(void *)centerlift,2,0},
  {"cf",1,(void *)gcf,4,0},
  {"cf2",2,(void *)gcf2,4,0},
  {"changevar",2,(void *)changevar,2,0},
  {"char",14,(void *)caradj0,8,0},
  {"char1",14,(void *)caract,8,0},
  {"char2",14,(void *)carhess,8,0},
  {"chell",2,(void *)coordch,5,0},
  {"chinese",2,(void *)chinois,4,0},
  {"chptell",2,(void *)pointch,5,0},
  {"classno",1,(void *)classno,4,0},
  {"classno2",1,(void *)classno2,4,0},
  {"coeff",21,(void *)truecoeff,2,0},
  {"compimag",2,(void *)compimag,4,0},
  {"compo",21,(void *)compo,2,0},
  {"compositum",2,(void *)compositum,6,0},
  {"comprealraw",2,(void *)comprealraw,4,0},
  {"concat",2,(void *)concat,8,0},
  {"conj",1,(void *)gconj,2,0},
  {"conjvec",1,(void *)conjvec,2,0},
  {"content",1,(void *)content,4,0},
  {"convol",2,(void *)convol,7,0},
  {"cos",1,(void *)gcos,3,0},
  {"cosh",1,(void *)gch,3,0},
  {"cursor",11,(void*)rectcursor,10,0},
  {"cvtoi",13,(void *)gcvtoi,2,0},
  {"cyclo",11,(void *)cyclo,7,0},
  {"denom",1,(void *)denom,2,0},
  {"deplin",1,(void *)deplin,8,0},
  {"deriv",14,(void *)deriv,7,0},
  {"det",1,(void *)det,8,0},
  {"det2",1,(void *)det2,8,0},
  {"detint",1,(void *)detint,8,0},
  {"detr",1,(void *)detreel,8,0},
  {"dilog",1,(void *)dilog,3,0},
  {"disc",1,(void *)discsr,7,0},
  {"discf",1,(void *)discf,6,0},
  {"discf2",1,(void *)discf2,6,0},
  {"divisors",1,(void *)divisors,4,0},
  {"divres",2,(void *)gdiventres,1,0},
  {"divsum",22,(void *)divsomme,9,0},
  {"draw",1,(void*)rectdraw,10,0},
  {"eigen",1,(void *)eigen,8,0},
  {"eint1",1,(void *)eint1,3,0},
  {"erfc",1,(void *)gerfc,3,0},
  {"eta",1,(void *)eta,3,0},
  {"euler",0,(void *)mpeuler,3,0},
  {"eval",1,(void *)geval,7,0},
  {"exp",1,(void *)gexp,3,0},
  {"extract",2,(void *)extract,8,0},
  {"fact",11,(void *)mpfactr,4,0},
  {"factfq",3,(void *)factmod9,4,0},
  {"factmod",2,(void *)factmod,4,0},
  {"factor",1,(void *)factor,4,0},
  {"factoredbase",28,(void *)factoredbase,6,0},
  {"factoreddiscf",2,(void *)factoreddiscf,6,0},
  {"factoredpolred",2,(void *)factoredpolred,6,0},
  {"factoredpolred2",2,(void *)factoredpolred2,6,0},
  {"factornf",2,(void *)polfnf,6,0},
  {"factorpadic",32,(void *)factorpadic4,7,0},
  {"factorpadic2",32,(void *)factorpadic2,7,0},
  {"factpol",21,(void *)factpol,7,0},
  {"factpol2",21,(void *)factpol2,7,0},
  {"fibo",11,(void *)fibo,4,0},
  {"floor",1,(void *)gfloor,2,0},
  {"for",83,(void *)forpari,10,0},
  {"fordiv",84,(void *)fordiv,10,0},
  {"forprime",83,(void *)forprime,10,0},
  {"forstep",86,(void *)forstep,10,0},
  {"frac",1,(void *)gfrac,2,0},
  {"galois",1,(void *)galois,6,0},
  {"galoisconj",1,(void *)galoisconj,6,0},
  {"gamh",1,(void *)ggamd,3,0},
  {"gamma",1,(void *)ggamma,3,0},
  {"gauss",2,(void *)gauss,8,0},
  {"gcd",2,(void *)ggcd,4,0},
  {"getrand",0,(void *)getrand,2,0},
  {"getstack",0,(void *)getstack,2,0},
  {"gettime",0,(void *)gettime,2,0},
  {"globalred",1,(void *)globalreduction,5,0},
  {"goto",61,0,11,0},
  {"hclassno",1,(void *)classno3,4,0},
  {"hell",2,(void *)ghell,5,0},
  {"hell2",2,(void *)ghell2,5,0},
  {"hermite",1,(void *)hnf,8,0},
  {"hermitehavas",1,(void *)hnfhavas,8,0},
  {"hermitemod",2,(void *)hnfmod,8,0},
  {"hermitemodid",2,(void *)hnfmodid,8,0},
  {"hess",1,(void *)hess,8,0},
  {"hilb",30,(void *) hil,4,0},
  {"hilbert",11,(void *)hilb,8,0},
  {"hilbp",20,(void *) hil,4,0},
  {"hvector",22,(void *)vecteur,9,0},
  {"hyperu",3,(void *)hyperu,3,0},
  {"i",0,(void *)geni,2,0},
  {"idealadd",3,(void *)idealadd,6,0},
  {"idealdiv",3,(void *)idealdiv,6,0},
  {"idealfactor",2,(void *)idealfactor,6,0},
  {"idealintersect",3,(void *)idealintersect,6,0},
  {"idealinv",2,(void *)idealinv,6,0},
  {"ideallllred",3,(void *)ideallllred,6,0},
  {"idealmul",3,(void *)idealmul,6,0},
  {"idealmulred",3,(void *)idealmulred,6,0},
  {"idealpow",3,(void *)idealpow,6,0},
  {"idealpowred",3,(void *)idealpowred,6,0},
  {"idealval",30,(void *)idealval,6,0},
  {"idmat",11,(void *)idmat,8,0},
  {"if",80,0,11,0},
  {"imag",1,(void *)gimag,2,0},
  {"image",1,(void *)image,8,0},
  {"image2",1,(void *)image2,8,0},
  {"imagecompl",1,(void *)imagecompl,8,0},
  {"imager",1,(void *)imagereel,8,0},
  {"incgam",2,(void *)incgam,3,0},
  {"incgam1",2,(void *)incgam1,3,0},
  {"incgam2",2,(void *)incgam2,3,0},
  {"incgam3",2,(void *)incgam3,3,0},
  {"incgam4",3,(void *)incgam4,3,0},
  {"indexrank",1,(void *)indexrank,8,0},
  {"indsort",1,(void *)indexsort,8,0},
  {"initalg",1,(void *)initalg,6,0},
  {"initalg0",1,(void *)initalg0,6,0},
  {"initalg2",1,(void *)initalg2,6,0},
  {"initell",1,(void *)initell,5,0},
  {"initell2",1,(void *)initell2,5,0},
  {"initrect",34,(void*)initrect,10,0},
  {"integ",14,(void *)integ,7,0},
  {"intersect",2,(void *)intersect,8,0},
  {"intgen",37,(void *)rombint,9,0},
  {"intinf",37,(void *)qromi,9,0},
  {"intnum",37,(void *)qromb,9,0},
  {"intopen",37,(void *)qromo,9,0},
  {"inverseimage",2,(void *)inverseimage,8,0},
  {"isfund",1,(void *)gisfundamental,4,0},
  {"isideal",20,(void *)isideal,6,0},
  {"isincl",2,(void *)nfincl,6,0},
  {"isisom",2,(void *)nfiso,6,0},
  {"isoncurve",20,(void *)oncurve,5,0},
  {"isprime",1,(void *)gisprime,4,0},
  {"isprincipal",2,(void *)isprincipal,6,0},
  {"ispsp",1,(void *)gispsp,4,0},
  {"isqrt",1,(void *)racine,4,0},
  {"issqfree",1,(void *)gissquarefree,4,0},
  {"issquare",1,(void *)gcarreparfait,4,0},
  {"isunit",2,(void *)isunit,6,0},
  {"jacobi",1,(void *)jacobi,8,0},
  {"jbesselh",2,(void *)jbesselh,3,0},
  {"jell",1,(void *)jell,3,0},
  {"karamul",32,(void *)karamul,7,0},
  {"kbessel",2,(void *)kbessel,3,0},
  {"kbessel2",2,(void *)kbessel2,3,0},
  {"ker",1,(void *)ker,8,0},
  {"keri",1,(void *)keri,8,0},
  {"kerint",1,(void *)kerint,8,0},
  {"kerint1",1,(void *)kerint1,8,0},
  {"kerint2",1,(void *)kerint2,8,0},
  {"kerr",1,(void *)kerreel,8,0},
  {"kill",85,0,11,0},
  {"killrect",11,(void *)killrect,10,0},
  {"kro",2,(void *)gkronecker,4,0},
  {"label",60,0,11,0},
  {"laplace",1,(void *)laplace,7,0},
  {"lcm",2,(void *)glcm,4,0},
  {"legendre",11,(void *)legendre,7,0},
  {"length",1,(void *)glength,2,0},
  {"lex",20,(void *)lexcmp,2,0},
  {"lexsort",1,(void *)lexsort,8,0},
  {"lift",1,(void *)lift,2,0},
  {"lindep",1,(void *)lindep,8,0},
  {"lindep2",23,(void *)lindep2,8,0},
  {"line",35,(void *)rectline,10,0},
  {"lines",35,(void *)rectlines,10,0},
  {"lll",1,(void *)lll,8,0},
  {"lll1",1,(void *)lll1,8,0},
  {"lllgen",1,(void *)lllgen,8,0},
  {"lllgram",1,(void *)lllgram,8,0},
  {"lllgram1",1,(void *)lllgram1,8,0},
  {"lllgramgen",1,(void *)lllgramgen,8,0},
  {"lllgramint",1,(void *)lllgramint,8,0},
  {"lllgramkerim",1,(void *)lllgramkerim,8,0},
  {"lllgramkerimgen",1,(void *)lllgramkerimgen,8,0},
  {"lllint",1,(void *)lllint,8,0},
  {"lllkerim",1,(void *)lllkerim,8,0},
  {"lllkerimgen",1,(void *)lllkerimgen,8,0},
  {"lllrat",1,(void *)lllrat,8,0},
  {"ln",1,(void *)glog,3,0},
  {"lngamma",1,(void *)glngamma,3,0},
  {"localred",2,(void *)localreduction,5,0},
  {"log",1,(void *)glog,3,0},
  {"logagm",1,(void *)glogagm,3,0},
  {"lseriesell",4,(void *)lseriesell,5,0},
  {"mat",1,(void *)gtomat,8,0},
  {"matextract",3,(void *)matextract,8,0},
  {"mathell",2,(void *)mathell,5,0},
  {"matinvr",1,(void *)invmatreel,8,0},
  {"matsize",1,(void *)matsize,2,0},
  {"matrix",49,(void *)matrice,9,0},
  {"matrixqz",2,(void *)matrixqz,8,0},
  {"matrixqz2",1,(void *)matrixqz2,8,0},
  {"matrixqz3",1,(void *)matrixqz3,8,0},
  {"max",2,(void *)gmax,1,0},
  {"min",2,(void *)gmin,1,0},
  {"minideal",3,(void *)minideal,6,0},
  {"minim",33,(void *)minim,8,0},
  {"mod",25,(void *)gmodulcp,2,0},
  {"modp",25,(void *)gmodulo,2,0},
  {"modreverse",1,(void *)polymodrecip,6,0},
  {"move",35,(void*)rectmove,10,0},
  {"mu",1,(void *)gmu,4,0},
  {"newtonpoly",2,(void *)newtonpoly,6,0},
  {"nextprime",1,(void *)bigprem,4,0},
  {"norm",1,(void *)gnorm,2,0},
  {"norml2",1,(void *)gnorml2,2,0},
  {"nucomp",3,(void *)nucomp,4,0},
  {"numdiv",1,(void *)numbdiv,4,0},
  {"numer",1,(void *)numer,2,0},
  {"nupow",2,(void *)nupow,4,0},
  {"o",50,0,7,0},
  {"omega",1,(void *)gomega,4,0},
  {"ordell",2,(void *)ordell,5,0},
  {"order",1,(void *)order,4,0},
  {"orderell",2,(void *)orderell,5,0},
  {"ordred",1,(void *)ordred,6,0},
  {"pascal",11,(void *)pasc,8,0},
  {"perf",10,(void *)perf,8,0},
  {"permutation",24,(void *)permute,2,0},
  {"pf",2,(void *)primeform,4,0},
  {"phi",1,(void *)phi,4,0},
  {"pi",0,(void *)mppi,3,0},
  {"plot",37,(void *)plot,10,0},
  {"ploth",37,(void *)ploth,10,0},
  {"ploth2",37,(void *)ploth2,10,0},
  {"plothraw",2,(void *)plothraw,10,0},
  {"pnqn",1,(void *)pnqn,4,0},
  {"point",35,(void *)rectpoint,10,0},
  {"pointell",2,(void *)pointell,5,0},
  {"points",35,(void *)rectpoints,10,0},
  {"polint",31,(void *)polint,7,0},
  {"polred",1,(void *)polred,6,0},
  {"polred2",1,(void *)polred2,6,0},
  {"polsym",21,(void *)polsym,7,0},
  {"poly",14,(void *)gtopoly,2,0},
  {"polylog",24,(void *)gpolylog,3,0},
  {"polylogd",24,(void *)polylogd,3,0},
  {"polylogdold",24,(void *)polylogdold,3,0},
  {"polylogp",24,(void *)polylogp,3,0},
  {"polyrev",14,(void *)gtopolyrev,2,0},
  {"postdraw",1,(void *)postdraw,10,0},
  {"postploth",37,(void *)postploth,10,0},
  {"postploth2",37,(void *)postploth2,10,0},
  {"postplothraw",2,(void *)postplothraw,10,0},
  {"powell",3,(void *)powell,5,0},
  {"powrealraw",23,(void *)powrealraw,4,0},
  {"pprint",54,0,11,0},
  {"pprint1",52,0,11,0},
  {"prec",21,(void *)gprec,2,0},
  {"prime",11,(void *)prime,4,0},
  {"primedec",2,(void *)primedec,6,0},
  {"primes",11,(void *)primes,4,0},
  {"primroot",1,(void *)gener,4,0},
  {"principalideal",2,(void *)principalideal,6,0},
  {"principalidele",2,(void *)principalidele,6,0},
  {"print",53,0,11,0},
  {"print1",51,0,11,0},
  {"prod",48,(void *)produit,9,0},
  {"prodeuler",37,(void *)prodeuler,9,0},
  {"prodinf",27,(void *)prodinf,9,0},
  {"prodinf1",27,(void *)prodinf1,9,0},
  {"psi",1,(void *)gpsi,3,0},
  {"qfi",3,(void *)qfi,4,0},
  {"qfr",4,(void *)qfr,4,0},
  {"quaddisc",1,(void *)quaddisc,4,0},   
  {"quadgen",1,(void *)quadgen,2,0},   
  {"quadpoly",1,(void *)quadpoly,2,0},   
  {"random",0,(void *)genrand,2,0},
  {"rank",10,(void *)rank,8,0},
  {"rbox",35,(void *)rectrbox,10,0},
  {"read",56,0,11,0},
  {"real",1,(void *)greal,2,0},
  {"recip",1,(void *)polrecip,7,0},
  {"redimag",1,(void *)redimag,4,0},      
  {"redreal",1,(void *)redreal,4,0},      
  {"redrealnod",2,(void *)redrealnod,4,0},      
  {"regula",1,(void *)regula,4,0}, 
  {"reorder",1,(void *)reorder,11,0}, 
  {"resultant",2,(void *)subres,7,0},    
  {"resultant2",2,(void *)resultant2,7,0},    
  {"reverse",1,(void *)recip,7,0}, 
  {"rhoreal",1,(void *)rhoreal,4,0},      
  {"rhorealnod",2,(void *)rhorealnod,4,0},      
  {"rline",35,(void *)rectrline,10,0},
  {"rmove",35,(void *)rectrmove,10,0},      
  {"rndtoi",13,(void *)grndtoi,2,0},
  {"rootmod",2,(void *)rootmod,7,0},
  {"rootmod2",2,(void *)rootmod2,7,0},
  {"rootpadic",32,(void *)rootpadic,7,0},
  {"roots",1,(void *)roots,7,0},
  {"rootslong",1,(void *)rootslong,7,0},
  {"rootsof1",1,(void *)rootsof1,6,0},
  {"round",1,(void *)ground,2,0},
  {"rounderror",10,(void *)rounderror,2,0},
  {"rpoint",35,(void *)rectrpoint,10,0},      
  {"scale",59,(void *)rectscale,10,0},
  {"series",14,(void *)gtoser,2,0},
  {"setprecision",15,(void *)setprecr,2,0},
  {"setrand",11,(void *)setrand,2,0},
  {"setserieslength",15,(void *)setserieslength,2,0},
  {"settype",21,(void *)gsettype,2,0},
  {"shift",21,(void *)gshift,1,0},
  {"shiftmul",21,(void *)gmul2n,1,0},
  {"sigma",1,(void *)sumdiv,4,0},
  {"sigmak",24,(void *)sumdivk,4,0},
  {"sign",10,(void *)gsigne,1,0},
  {"signat",1,(void *)signat,8,0},
  {"signunit",1,(void *)signunit,6,0},
  {"simplefactmod",2,(void *)simplefactmod,4,0},
  {"simplify",1,(void *)simplify,2,0},
  {"sin",1,(void *)gsin,3,0},
  {"sinh",1,(void *)gsh,3,0},
  {"size",10,(void *)gsize,2,0},
  {"smallbase",13,(void *)smallbase,6,0},
  {"smalldiscf",1,(void *)smalldiscf,6,0},
  {"smallfact",1,(void *)smallfact,4,0},
  {"smallinitell",1,(void *)smallinitell,5,0},
  {"smallpolred",1,(void *)smallpolred,6,0},
  {"smallpolred2",1,(void *)smallpolred2,6,0},
  {"smith",1,(void *)smith,8,0},
  {"smith2",1,(void *)smith2,8,0},
  {"solve",37,(void *)zbrent,9,0},
  {"sort",1,(void *)sort,8,0},
  {"sqr",1,(void *)gsqr,3,0},
  {"sqred",1,(void *)sqred,8,0},
  {"sqrt",1,(void *)gsqrt,3,0},
  {"srgcd",2,(void *)srgcd,7,0},
  {"string",57,(void*)rectstring,10,0},
  {"sturm",10,(void *)sturm,7,0},
  {"sturmpart",30,(void *)sturmpart,7,0},
  {"subell",3,(void *)subell,5,0},
  {"subst",26,(void *)gsubst,7,0},
  {"sum",48,(void *)somme,9,0},
  {"sumalt",27,(void *)sumalt,9,0},
  {"suminf",27,(void *)suminf,9,0},
  {"sumpos",27,(void *)sumpos,9,0},
  {"supplement",1,(void *)suppl,8,0},
  {"tan",1,(void *)gtan,3,0},
  {"tanh",1,(void *)gth,3,0},
  {"taniyama",1,(void *)taniyama,5,0},
  {"taylor",12,(void *)tayl,7,0},
  {"tchebi",11,(void *)tchebi,7,0},
  {"teich",1,(void *)teich,3,0},
  {"texprint",55,0,11,0},
  {"theta",2,(void *)theta,3,0},
  {"thetanullk",21,(void *)thetanullk,3,0},
  {"torsell",1,(void *)torsell,5,0},
  {"trace",1,(void *)trace,8,0},
  {"trans",1,(void *)gtrans,8,0},
  {"trunc",1,(void *)gtrunc,2,0},
  {"tschirnhaus",1,(void *)tschirnhaus,6,0},
  {"type",1,(void *)gtype,2,0},
  {"unit",1,(void *)fundunit,4,0},
  {"until",82,0,11,0},
  {"valuation",20,(void *)ggval,2,0},
  {"vec",1,(void *)gtovec,2,0},
  {"vecmax",1,(void *)vecmax,2,0},
  {"vecmin",1,(void *)vecmin,2,0},
  {"vecsort",2,(void *)vecsort,8,0},
  {"vector",22,(void *)vecteur,9,0},
  {"vvector",22,(void *)vvecteur,9,0},
  {"wf",1,(void *)wf,3,0},
  {"wf2",1,(void *)wf2,3,0},
  {"while",81,0,11,0},
  {"zell",2,(void *)zell,5,0},
  {"zeta",1,(void *)gzeta,3,0},
  {"zzzz",1,(void *)deplin,8,0}
};

long    NUMFUNC=sizeof(fonctions)/sizeof(entree);

static void matcherr(char c)
{
  static char reste[100];
  char *p;
  long i;
  
  for(analyseurs--, p=reste, *p++=c, i=0; i<97; i++) *p++ = *analyseurs++;
  *p = 0;err(matcher1,reste);
}

#define match(c)  if(*analyseurs++ != c) matcherr(c)

GEN seq(void)
{
  GEN res=gnil;
  for(;;)
    {
      while(separe(*analyseurs)) analyseurs++;
      if ((!*analyseurs) || (*analyseurs == ')') || (*analyseurs == ',')) return res;
      res = expr();
      if(!separe(*analyseurs)) return res;
    }
}

GEN expr(void)
{
  typedef GEN (*PFGEN)();
  PFGEN func[4];
  GEN aux,e,e1,e2,e3;
  long niveau;
  
  for(niveau=0;niveau<4;niveau++) func[niveau]=NULL;
  e1=e2=e3=(GEN)0;
  niveau=3;
  for(;;)
    switch(niveau)
      {
      case 3: aux=facteur();
        if(func[3]) {analyseurtetpil=avma;e3=((GEN (*)(GEN,GEN))func[3])(e3,aux);}
        else e3=aux;
        switch(*analyseurs)
	  {
          case '*': analyseurs++;func[3]=&gmul;break;
          case '/': analyseurs++;func[3]=&gdiv;break;
          case '\\': analyseurs++;func[3]=&gdivent;break;
          case '%': analyseurs++;func[3]=&gmod;break;
          default: niveau--;func[3]=NULL; 

	  }
        break;
      case 2: 
        if(!e3) {niveau++;break;}
        if(func[2]) {analyseurtetpil=avma;e2=((GEN (*)(GEN,GEN))func[2])(e2,e3);}
        else e2=e3;
        e3=(GEN)0;
        switch(*analyseurs)
	  {
          case '+': analyseurs++;func[2]=&gadd;niveau++;break;
          case '-': analyseurs++;func[2]=&gsub;niveau++;break;
          default: niveau--;func[2]=NULL;
	  }
        break;
      case 1: 
        if(!e2) {niveau++;break;}
        if(func[1]) {analyseurtetpil=avma;e1=((GEN (*)(GEN,GEN))func[1])(e1,e2);}
        else e1=e2;
        e2=(GEN)0;
        switch(*analyseurs)
	  {
          case '<': analyseurs++;
            switch(*analyseurs)
	      {
              case '=': analyseurs++;func[1]=&gle;break;
              case '>': analyseurs++;func[1]=&gne;break;
		default : func[1]=&glt;
	      }
            niveau++;break;
          case '>': analyseurs++;
            if((*analyseurs)=='=') {analyseurs++;func[1]=&gge;}
            else func[1]=&ggt;
            niveau++;break;
          case '=': 
            if((analyseurs[1])=='=') {analyseurs+=2;func[1]=&geq;niveau++;}
            break;
          case '!': 
            if((analyseurs[1])=='=') {analyseurs+=2;func[1]=&gne;niveau++;}
            break;
          default: niveau--;func[1]=NULL;
	  }
        break;
      case 0: 
        if(!e1) {niveau++;break;}
        if(func[0]) {analyseurtetpil=avma;e=((GEN (*)(GEN,GEN))func[0])(e,e1);}
        else e=e1;
        e1=(GEN)0;
        switch(*analyseurs)
	  {
          case '&': analyseurs++;if(*analyseurs=='&') analyseurs++;func[0]=&gand;niveau++;break;
          case '|': analyseurs++;if(*analyseurs=='|') analyseurs++;func[0]=&gor;niveau++;break;
          default: return e;
	  }
      }
}

GEN facteur(void)
{
  GEN tru,p1,arg,arg1;
  long tx,c,e,av2,flcol,flrow;
  long plus = (*analyseurs =='+')||(*analyseurs =='-')?(*analyseurs++=='+'):1;
  tru=truc();
  for (;;) switch(*analyseurs)
    {
    case '^': analyseurs++;p1=facteur();analyseurtetpil=avma; tru=gpui(tru,p1,prec);break;
    case '~': analyseurs++;analyseurtetpil=avma;tru=gtrans(tru);break;
    case '_': analyseurs++;analyseurtetpil=avma;tru=gconj(tru);break;
    case '\'': analyseurs++;analyseurtetpil=avma;tru=deriv(tru,gvar9(tru));break;
    case '[': 
      tx=typ(p1=tru);
      if((tx<17)||(tx>19)) err(caracer1,analyseurs);
      analyseurs++;av2=avma;flcol=flrow=0;
      if(tx<19)
	{
	  arg=expr();if(typ(arg)!=1) err(caseer);
	  c=itos(arg);if((c<1)||(c>=lg(p1))) err(arrayer1);
	}
      else
	{
	  if(lg(p1)==1) err(arrayer1);
	  if(*analyseurs==',')
	    {
	      analyseurs++;arg=expr();if(typ(arg)!=1) err(caseer);
	      c=itos(arg);if((c<1)||(c>=lg(p1))) err(arrayer1);
	      flcol=1;
	    }
	  else
	    {
	      arg=expr();if(typ(arg)!=1) err(caseer);
	      e=itos(arg);if((e<1)||(e>=lg((GEN)p1[1]))) err(arrayer1);
	      match(',');
	      if(*analyseurs==']') flrow=1;
	      else
		{
		  arg1=expr();if(typ(arg1)!=1) err(caseer);
		  c=itos(arg1);
		  if((c<1)||(c>=lg(p1))) err(arrayer1);
		}
	    }
	}
      match(']'); analyseurtetpil=avma=av2;
      if((tx<19)||flcol) tru=gcopy((GEN)p1[c]);
      else
	{
	  if(flrow)
	    {
	      tru=cgetg(lg(p1),17);
	      for(c=1;c<lg(p1);c++) tru[c]=lcopy((GEN)((GEN)p1[c])[e]);
	    }
	  else tru = gcopy((GEN)((GEN)p1[c])[e]);
	}
      break;
    case '!': analyseurs++;if((*analyseurs)!='=') {analyseurtetpil=avma;tru=mpfact(itos(tru));break;} else analyseurs--;
    default: if(plus) return tru; else {analyseurtetpil=avma;return gneg(tru);}
    }
}

GEN truc(void)
{
  long i,n=0,j,p=0,m=1;
  GEN *table,p1;
  
  if (isalpha(*analyseurs)) return identifier();
  if (isdigit(*analyseurs) || (*analyseurs=='.')) return constante();
  switch(*analyseurs++)
    {
    case '(': p1=expr();match(')');return p1;
    case '[':
      table = (GEN *)newbloc(paribuffsize>>TWOPOTBYTES_IN_LONG);
      if (*analyseurs!=']') 
	{do table[++n]=expr();while (*analyseurs++==',');analyseurs--;}
      switch (*analyseurs++)
	{
        case ']': analyseurtetpil=avma;p1=cgetg(n+1,17);
          for (i=1;i<=n;i++) p1[i]=lcopy(table[i]);
          break;
        case ';': m=n;do table[++n]=expr();while (*analyseurs++!=']');
          if (n % m) err(recter1);
          p=n/m;analyseurtetpil=avma;p1=cgetg(m+1,19);
          for (i=1;i<=m;i++) p1[i]=(long)cgetg(p+1,18);
          for (j=1;j<=m;j++)
            for(i=1;i<=p;i++)
              ((GEN)p1[j])[i]=lcopy(table[(i-1)*m+j]);
          break;
        default: err(vectmater1);
	}
      killbloc((GEN)table);
      return p1;
    case '%':
      p=0;while((*analyseurs)=='`') {analyseurs++;p++;}
      if(p>tglobal) err(referer1);
      if(p) return g[tglobal-p];
      while (isdigit(*analyseurs)) p = 10*p + *analyseurs++ - '0';
      if(p>tglobal) err(referer2);
      return g[p];
    }
  err(caracer1,analyseurs-1);return gnil;
}

int numvar(GEN x)
{
  if(typ(x)!=10) err(numvarer);
  if(lgef(x)!=4) err(numvarer);
  if((!gcmp0((GEN)x[2])) || (!gcmp1((GEN)x[3]))) err(numvarer);
  return varn(x);
}


GEN identifier(void)
{
  long c,e,va,m,nparam,i,av,av2,tx,flrow,flcol;
  static long yatileugoto;
#ifdef __cplusplus
  GEN arg,arg1,arg2,arg3,arg4,res=gnil,(*f)(...),p1;
#else
  GEN arg,arg1,arg2,arg3,arg4,res=gnil,(*f)(),p1;
#endif
  char *ch1, *ch2, *readbuffer;
  entree *ep, *ep1, **p;
  
  ep = findentry();
  if (ep->valence < 100) /* fonctions predefinies */
    {
      f = (GEN (*)())ep->value;
      if (!ep->valence && (*analyseurs != '(')) return (*f)(prec);
      match('(');
      switch(ep->valence)
	{
	case 0: res=(*f)(prec);break;
	case 1: arg=expr();analyseurtetpil=avma;
	  res=(*f)(arg,prec);break;
	case 2: arg=expr();match(',');arg1=expr();
	  analyseurtetpil=avma;res=(*f)(arg,arg1,prec);break;
	case 3: arg=expr();match(',');arg1=expr();
	  match(',');arg2=expr();analyseurtetpil=avma;
	  res=(*f)(arg,arg1,arg2,prec);break;
	case 4: arg=expr();match(',');arg1=expr();
	  match(',');arg2=expr();match(',');arg3=expr();analyseurtetpil=avma;
	  res=(*f)(arg,arg1,arg2,arg3,prec);break;
	case 5: arg=expr();match(',');arg1=expr();match(',');arg2=expr();
	  match(',');arg3=expr();match(',');arg4=expr();analyseurtetpil=avma;
	  res=(*f)(arg,arg1,arg2,arg3,arg4,prec);break;
	case 10: p1=(*f)(expr());analyseurtetpil=avma;
	  res=stoi((long)p1);break;
	case 11: arg=expr();if(typ(arg)!=1) err(caseer);
	  analyseurtetpil=avma;res=(*f)(itos(arg),prec);break;
	case 12: arg=expr();match(',');arg1=expr();va=numvar(arg1);
	  analyseurtetpil=avma;res=(*f)(arg,va,precdl);break;
	case 13: arg=expr();analyseurtetpil=avma;
	  res=(*f)(arg,&e);break;
	case 14: arg=expr();match(',');arg1=expr();va=numvar(arg1);
	  analyseurtetpil=avma;res=(*f)(arg,va);break;
	case 15: arg=expr();if(typ(arg)!=1) err(caseer);
	  analyseurtetpil=avma;res=stoi((long)(*f)(itos(arg)));break;
	case 20: arg=expr();match(',');
	  p1=(*f)(arg,expr());analyseurtetpil=avma;
	  res=stoi((long)p1);break;
	case 21: arg=expr();match(',');arg1=expr();if(typ(arg1)!=1) err(caseer);
	  analyseurtetpil=avma;res=(*f)(arg,itos(arg1));break;
	case 22: arg=expr();match(',');
	  if(!isalpha(*analyseurs)) err(varer1,analyseurs);
	  ep = findentry();if (ep->valence!=200) err(varer1,analyseurs);match(',');
	  analyseurtetpil=avma;
	  res=(*f)(ep,arg,analyseurs); skipexpr(); break;
	case 23: arg=expr();match(',');arg1=expr();if(typ(arg1)!=1) err(caseer);
	  analyseurtetpil=avma;res=(*f)(arg,itos(arg1),prec);break;
	case 24: arg=expr();if(typ(arg)!=1) err(caseer);
	  match(',');arg1=expr();analyseurtetpil=avma;
	  res=(*f)(itos(arg),arg1,prec);break;
	case 25: arg=expr();match(',');arg1=expr();analyseurtetpil=avma;
	  res=(*f)(arg,arg1);break;
	case 26: arg=expr();match(',');arg1=expr();
	  va=numvar(arg1);match(',');arg2=expr();
	  analyseurtetpil=avma;res=(*f)(arg,va,arg2);break;
	case 27: if(!isalpha(*analyseurs)) err(varer1,analyseurs);
	  ep = findentry();match('='); if (ep->valence!=200) err(varer1,analyseurs);
	  arg=expr(); match(','); analyseurtetpil=avma;
	  res=(*f)(ep,arg,analyseurs,prec); skipexpr(); break;
	case 28: arg=expr();match(',');arg1=expr();
	  analyseurtetpil=avma;res=(*f)(arg,arg1,&e);break;
	case 29: arg=expr();match(',');arg1=expr();if(typ(arg1)!=1) err(caseer);
	  p1=(*f)(arg,itos(arg1));analyseurtetpil=avma;
	  res=stoi((long)p1);break;
	case 30: arg=expr();match(',');arg1=expr();match(',');
	  p1=(*f)(arg,arg1,expr());analyseurtetpil=avma;
	  res=stoi((long)p1);break;
	case 31: arg=expr();match(',');arg1=expr();match(',');
	  analyseurtetpil=avma;res=(*f)(arg,arg1,expr(),&arg2);cgiv(arg2);
	  break;
	case 32: arg=expr();match(',');arg1=expr();match(',');arg2=expr();
	  if(typ(arg2)!=1) err(caseer);
	  analyseurtetpil=avma;res=(*f)(arg,arg1,itos(arg2));break;
	case 33: arg=expr();match(',');arg1=expr();match(',');arg2=expr();
	  if((typ(arg2)!=1)||(typ(arg1)!=1)) err(caseer);
	  analyseurtetpil=avma;res=(*f)(arg,itos(arg1),itos(arg2),prec);break;
	case 34: arg1=expr();match(',');arg2=expr();match(',');arg3=expr();
	  analyseurtetpil=avma;
	  res=(*f)(gtolong(arg1),gtolong(arg2),gtolong(arg3));
	  break;
	case 35: arg=expr();match(',');arg1=expr();match(',');arg2=expr();
	  if(typ(arg)!=1) err(caseer);
	  analyseurtetpil=avma;res=(*f)(itos(arg),arg1,arg2);break;
	case 37: if(!isalpha(*analyseurs)) err(varer1,analyseurs);
	  ep = findentry();match('='); if (ep->valence!=200) err(varer1,analyseurs);
	  arg=expr(); match(','); arg1=expr(); match(','); 
	  analyseurtetpil=avma;
	  res=(*f)(ep,arg,arg1,analyseurs,prec); skipexpr(); break;
	case 48: arg=expr(); match(',');
	  if(!isalpha(*analyseurs)) err(varer1,analyseurs);
	  ep = findentry();match('='); if (ep->valence!=200) err(varer1,analyseurs);
	  arg1=expr(); match(','); arg2=expr(); match(',');
	  analyseurtetpil=avma;
	  res=(*f)(ep,arg,arg1,arg2,analyseurs,prec); skipexpr(); break;
	case 49: arg=expr();match(','); arg1=expr(); match(',');
	  if(!isalpha(*analyseurs)) err(varer1,analyseurs);
	  ep = findentry();if (ep->valence!=200) err(varer1,analyseurs);match(',');
	  if(!isalpha(*analyseurs)) err(varer1,analyseurs);
	  ep1 = findentry();if (ep1->valence!=200) err(varer1,analyseurs);match(',');
	  analyseurtetpil=avma;
	  res=(*f)(ep,ep1,arg,arg1,analyseurs); skipexpr(); break;
	case 50: p1=truc();
	  if (*analyseurs++=='^') 
	    {
	      arg=facteur();if(typ(arg)!=1) err(caseer);
	      e=itos(arg);
	    }
	  else {e = 1; analyseurs--;}
	  analyseurtetpil=avma; res = ggrando(p1,e); break;
	case 51: case 52: case 53: case 54: case 55:
	  if (*analyseurs != ')') for(;;)
	    {
	      if (*analyseurs == '"')
		{
		  analyseurs++;
		  while ((*analyseurs)&&(*analyseurs!='"')) pariputc(*analyseurs++);
		  match('"');
		}
	      else
		{
		  analyseurtetpil=avma;
		  if (ep->valence == 55) f = (GEN (*)()) texe;
		  else if(ep->valence&1) f = (GEN (*)()) brute;else f = (GEN (*)()) sor;
		  (*f)(res=expr(),glbfmt[0],glbfmt[2],glbfmt[1]);
		}
	      if (*analyseurs == ')') break;
	      match(',');
	    }
	  if (ep->valence>52) pariputc('\n'); 
	  fflush(outfile); if (logfile) fflush(logfile); break;
	case 56: 
	  readbuffer = (char *)newbloc(paribuffsize>>TWOPOTBYTES_IN_LONG);
	  while(!fgets(readbuffer, paribuffsize, infile)) switchin(NULL);
	  if (pariecho) pariputs(readbuffer);
	  else if (logfile) fputs(readbuffer, logfile);
	  res=lisseq(readbuffer);killbloc((GEN)readbuffer);break;
	case 57:
	  arg=expr();if(typ(arg)!=1) err(caseer);
	  match(',');
	  if(*analyseurs!='"') arg1=expr();
	  else
	    {
	      match('"');ch1=(char*)malloc(256);m=0;
	      while ((m<256)&&(*analyseurs)&&(*analyseurs!='"'))
		ch1[m++]=*analyseurs++;
	      match('"');arg1=cgetg(m+1,17);
	      for(i=1;i<=m;i++) arg1[i]=lstoi((long)ch1[i-1]);
	      free(ch1);
	    }
	  analyseurtetpil=avma;res=(*f)(itos(arg),arg1);
	  break;
	case 59: arg=expr();match(',');arg1=expr();match(',');
	  arg2=expr();match(',');arg3=expr();match(',');arg4=expr();
	  if(typ(arg)!=1) err(caseer);
	  analyseurtetpil=avma;res=(*f)(itos(arg),arg1,arg2,arg3,arg4);
	  break;	  
	case 60: arg=expr();if(typ(arg)!=1) err(caseer);
	  m=itos(arg);if((m>=100)||(m<0)) err(labeler);
	  labellist[m]=analyseurs;break;
	case 61: arg=expr();if(typ(arg)!=1) err(caseer);
	  m=itos(arg);if((m>=100)||(m<0)||(!labellist[m])) err(labeler);
	  analyseurs=labellist[m];yatileugoto=1;break;
	case 80: av = avma; c=gcmp0(expr()); analyseurtetpil = avma = av; match(',');
	  if (c) {skipseq();match(',');res = seq();}
	  else 
	    {
	      yatileugoto=0;res = seq();
	      if(!yatileugoto) {match(',');skipseq();}
	    }
	  break;
	case 81: analyseurtetpil = av = avma; ch1 = analyseurs;
	  while (!gcmp0(expr()))
	    {
	      analyseurtetpil = avma = av; match(',');
	      yatileugoto=0;seq();
	      if(!yatileugoto) analyseurs = ch1;else break;
	    }
	  if(!yatileugoto) {match(','); skipseq();}
	  break;
	case 82: av = avma; ch1 = analyseurs;
	  skipexpr();
	  do 
	    {
	      analyseurtetpil = avma = av; match(','); 
	      yatileugoto=0;seq();
	      if(!yatileugoto) analyseurs = ch1;else break;
	    }
	  while (gcmp0(expr()));
	  if(!yatileugoto) {match(','); skipseq();}
	  break;
	case 83: if(!isalpha(*analyseurs)) err(varer1,analyseurs);
	  ep = findentry();match('='); if (ep->valence!=200) err(varer1,analyseurs);
	  arg=expr(); match(','); arg1=expr(); match(','); 
	  analyseurtetpil=avma;
	  res=(*f)(ep,arg,arg1,analyseurs); skipseq(); break;
	case 84: arg=expr();match(',');
	  if(!isalpha(*analyseurs)) err(varer1,analyseurs);
	  ep = findentry();if (ep->valence!=200) err(varer1,analyseurs);match(',');
	  analyseurtetpil=avma;
	  res=(*f)(ep,arg,analyseurs); skipseq(); break;
	case 85: if(!isalpha(*analyseurs)) err(killer1);
	  ep = findentry(); if (ep->valence<100) err(killer1);
	  killvalue(ep);
	  if (ep->valence == 200) res = (GEN)ep->value;
	  else
	    {
	      for(i = 0; i<TBLSZ; i++)
		if (hashtable[i] == ep) {hashtable[i] = ep->next; free(ep); break;}
		else
		  for(ep1 = hashtable[i]; ep1; ep1 = ep1->next)
		    if (ep1->next == ep) {ep1->next = ep->next; free(ep); break;}
	    }
	  break;
	case 86: if(!isalpha(*analyseurs)) err(varer1,analyseurs);
	  ep = findentry();match('='); if (ep->valence!=200) err(varer1,analyseurs);
	  arg=expr();match(',');arg1=expr();match(',');arg2=expr();match(',');
	  analyseurtetpil=avma;
	  res=(*f)(ep,arg,arg1,arg2,analyseurs); skipseq(); break;
	  
	default: err(valencer1);
	}
      match(')');return res;
    }
  switch (ep->valence)
    {
    case 200: /* variables */
      if((*analyseurs)=='[')
	{
	  tx=typ(p1=(GEN)ep->value);
	  if((tx<17)||(tx>19)) err(caracer1,analyseurs);
	  analyseurs++;av2=avma;flcol=flrow=0;
	  if(tx<19)
	    {
	      arg=expr();if(typ(arg)!=1) err(caseer);
	      c=itos(arg);if((c<1)||(c>=lg(p1))) err(arrayer1);
	    }
	  else
	    {
	      if(lg(p1)==1) err(arrayer1);
	      if(*analyseurs==',')
		{
		  analyseurs++;arg=expr();if(typ(arg)!=1) err(caseer);
		  c=itos(arg);if((c<1)||(c>=lg(p1))) err(arrayer1);
		  flcol=1;
		}
	      else
		{
		  arg=expr();if(typ(arg)!=1) err(caseer);
		  e=itos(arg);if((e<1)||(e>=lg((GEN)p1[1]))) err(arrayer1);
		  match(',');
		  if(*analyseurs==']') flrow=1;
		  else
		    {
		      arg1=expr();if(typ(arg1)!=1) err(caseer);
		      c=itos(arg1);
		      if((c<1)||(c>=lg(p1))) err(arrayer1);
		    }
		}
	    }
	  match(']'); avma=av2;
	  if(((*analyseurs)=='=')&&(*(analyseurs+1)!='=')) 
	    {
	      analyseurs++;res=expr();
	      if((tx==19)&&(!flcol))
		{
		  if(flrow)
		    {
		      if((typ(res)!=17)||(lg(res)!=lg(p1))) err(caseer2);
		      for(c=1;c<lg(p1);c++) ((GEN)p1[c])[e]=(long)res[c];
/* maybe lcopy(res[c]) instead ? */
		    }
		  else ((GEN)p1[c])[e]=(long)res;
		}
	      else 
		{
		  if(flcol)
		    {
		      if((typ(res)!=18)||(lg(res)!=lg((GEN)p1[1]))) err(caseer2);
		    }
		  p1[c]=(long)res;
		}
	      changevalue(ep, p1);p1=(GEN)ep->value;
	    }
	  analyseurtetpil=avma;
	  if((tx<19)||flcol) return (GEN)p1[c];
	  else
	    {
	      if(flrow) 
		{
		  res=cgetg(lg(p1),17);
		  for(c=1;c<lg(p1);c++) res[c]=((GEN)p1[c])[e];
/* maybe lcopy() instead */
		  return res;
		}
	      else return (GEN)((GEN)p1[c])[e];
	    }
	}
      if(((*analyseurs)=='=')&&(*(analyseurs+1)!='=')) 
	{
	  analyseurs++;changevalue(ep, expr()); 
	}
      analyseurtetpil=avma;return (GEN)ep->value;
      
    case 100: /* fonctions utilisateur */
      ch1 = analyseurs;
      match('(');
      p = (entree **)ep->value;
      nparam = (long)*p++;
      arg1 = arg = cgetg(nparam+1, 17);
      for(i = 0; (i < nparam) && (*analyseurs != ')'); i++)
	{
	  if (i) match(',');
	  *++arg = (long)expr();
	}
      if ((*analyseurs==')') && ((analyseurs[1] != '=') || (analyseurs[2] == '=')))
	{
	  analyseurs++;
	  while(i++ < nparam) *++arg = zero;
	  analyseurtetpil = avma;
	  for(i=0; i<nparam; i++) newvalue(*p++,(GEN)(*++arg1));
	  res = lisseq((char *)p);
          res = forcecopy(res);
	  for(i = 0; i < nparam; i++) 
            killvalue(*--p);
	  return res;
	}
      while (*analyseurs == ',') {analyseurs++; skipexpr();}
      match(')');
      if ((*analyseurs != '=') || (analyseurs[1] == '=')) err(nparamer1);
      analyseurs = ch1;
      killbloc((GEN)ep->value);
      
    case 101: /* nouvelle fonction */
      
      match('(');
      ch1 = analyseurs;
      for(nparam = 0; *analyseurs != ')'; nparam++)
	{
	  if (nparam) match(',');
	  if (!isalpha(*analyseurs)) err(paramer1);
	  if (skipentry()->valence != 200) err(paramer1);
	}
      match(')'); match('='); ch2 = analyseurs; skipseq(); 
      p = (entree **)newbloc(nparam + ((analyseurs - ch2)>>TWOPOTBYTES_IN_LONG) + 2);
      p[-1] = (entree *)ep->value;
      ep->value = (void *)p;
      *p++ = (entree *)nparam;
      ch2 = analyseurs; analyseurs = ch1;
      for(i = 0; i < nparam; i++)
	{
	  if (i) match(',');
	  *p++ = ep1 = findentry();
	  if (ep1->valence != 200) err(paramer1);
	}      
      match(')'); match('=');
      strncpy((char *)p, analyseurs, ch2 - analyseurs);
      *((char *)p + (ch2 - analyseurs)) = 0;
      ep->valence = 100;
      ep->menu = 0;
      analyseurs = ch2;
      return gnil;
      
    default: err(valencer1);return gnil;
    }
}

static long  word(long *nb)
{
  int m=0;
  for(*nb = 0; (*nb < 9) && isdigit(*analyseurs); (*nb)++)
    m = 10 * m + *analyseurs++-'0';
   return m;
}

GEN constante()
{
  static long pw10[] = {1, 10, 100, 1000, 10000, 100000,
                          1000000, 10000000, 100000000, 1000000000};
  long l,m=0,n=0,plus=1,nb, av = avma, limite=(avma + bot)/2;
  GEN z,y;
  
  analyseurtetpil=avma;
  y = stoi(word(&nb));
  while (isdigit(*analyseurs))
    {
      m = word(&nb); y = mulsi(pw10[nb], y);
      analyseurtetpil = avma;
      y = addsi(m, y);
      if (avma < limite)
        {
          y = gerepile(av, analyseurtetpil, y);
          analyseurtetpil = av;
        }
    }
  if ((*analyseurs!='.')&&(*analyseurs!='e')&&(*analyseurs!='E')) return y;
  if (*analyseurs=='.') 
    {
      analyseurs++;
      while (isdigit(*analyseurs))
        {
          m = word(&nb); y = mulsi(pw10[nb], y);
          analyseurtetpil = avma;
          y = addsi(m, y);
          if (avma < limite)
            {
              y = gerepile(av, analyseurtetpil, y);
              analyseurtetpil = av;
            }
          n -= nb;
        }
    }
  l=lgef(y);if(l<prec) l=prec;
  analyseurtetpil=avma;
  z=cgetr(l);affir(y,z);
  if ((*analyseurs=='e') || (*analyseurs=='E'))
    {
      analyseurs++;
      if (((*analyseurs)=='+') || ((*analyseurs)=='-')) plus=(*analyseurs++=='+');
      m = word(&nb);
      if(isdigit(*analyseurs)) err(expter1);
      if (plus) n += m;else n -= m;
    }
  if (n)
    {
      affsr(10, y = cgetr(l));
      y = gpuigs(y, labs(n));
      analyseurtetpil=avma;
      z = n > 0 ?  mulrr(z, y) : divrr(z, y);
    }
  return z;
}

entree *findentry(void)
{
  char *olds = analyseurs, *u, *v;
  long sv, n;
  GEN p1;
  entree *ep;
  
  for (n = 0; isalnum(*analyseurs); analyseurs++) n = n << 1 ^ *analyseurs;
  if (n < 0) n = -n; n %= TBLSZ;
  for(ep = hashtable[n]; ep; ep = ep->next)
    {
      for(u = ep->name, v = olds; (*u) && *u == *v; u++, v++);
      if (!*u && (v == analyseurs)) return ep;
    }
  sv = (*analyseurs == '(') ? 0 : 7*BYTES_IN_LONG;
  ep = (entree *)malloc(sizeof(entree) + sv + analyseurs - olds + 1);
  ep->name = (char *)ep + sizeof(entree) + sv;
  for (u = ep->name, v = olds; v < analyseurs;) *u++ = *v++; *u = 0;
  ep->value = (void *)((char *)ep + sizeof(entree));
  ep->next = hashtable[n];
  hashtable[n] = ep;
  p1 = (GEN)ep->value;
  if (*analyseurs == '(') ep->valence = 101;
  else
    {
      if (nvar == MAXVAR) err(trucer1);
      ep->valence = 200;
      p1[0] = evaltyp(10)+evalpere(1)+evallg(4);
      p1[1] = evalsigne(1)+evallgef(4)+evalvarn(nvar);
      p1[2] = zero; p1[3] = un;
      polx[nvar] = p1;
      polvar[nvar+1] = (long)p1;
      p1 += 4;
      p1[0] = evaltyp(10)+evalpere(1)+evallg(3);
      p1[1] = evalsigne(1)+evallgef(3)+evalvarn(nvar); p1[2] = un;
      polun[nvar] = p1;
      varentries[nvar++] = ep;
      setlg(polvar, nvar+1);
    }
  return ep;
}



void skipseq(void)
{
  for(;;)
    {
      while(separe(*analyseurs)) analyseurs++;
      if ((!*analyseurs) || (*analyseurs == ')') || (*analyseurs == ',')) return;
      skipexpr();
      if(!separe(*analyseurs)) return;
    }
}

void skipexpr(void)
{
  long niveau=3,e1,e2,e3;
  
  e1=e2=e3=0;
  for(;;)
    switch(niveau)
      {
      case 3: e3=1;skipfacteur();
        switch(*analyseurs)
	  {
          case '*': 
          case '/': 
          case '\\':
          case '%': analyseurs++;break;
          default: niveau--;
	  }
        break;
      case 2:
        if(!e3) {niveau++;break;}
        e3=0;e2=1;
        switch(*analyseurs)
	  {
          case '+':
          case '-': analyseurs++;niveau++;break;
          default: niveau--;
	  }
        break;
      case 1: 
        if(!e2) {niveau++;break;}
        e2=0;e1=1;
        switch(*analyseurs)
	  {
          case '<': analyseurs++;
            switch(*analyseurs)
	      {
              case '=':
              case '>': analyseurs++;niveau++;break;
		default : niveau++;break;
	      }
            break;
          case '>': analyseurs++;
            if((*analyseurs)=='=') analyseurs++;
            niveau++; break;
          case '=': 
          case '!': 
            if((analyseurs[1])=='=') {analyseurs+=2;niveau++;}
            break;
          default: niveau--;
	  }
        break;
      case 0: 
        if(!e1) {niveau++;break;}
        e1=0;
        switch(*analyseurs)
	  {
          case '&': analyseurs++;if(*analyseurs=='&') analyseurs++;niveau++;break;
          case '|': analyseurs++;if(*analyseurs=='|') analyseurs++;niveau++;break;
          default: return;
	  }
      }
}

void skipfacteur(void)
{
  if (((*analyseurs)=='+') || ((*analyseurs)=='-')) analyseurs++;
  skiptruc();
  for (;;) switch(*analyseurs)
    {
    case '^': analyseurs++;skipfacteur(); break;
    case '~': 
    case '_':
    case '\'': analyseurs++;break;
    case '[': 
      analyseurs++;
      if(*analyseurs == ',') {analyseurs++;skipexpr();}
      else
	{
	  skipexpr();
	  if(*analyseurs==',')
	    {
	      analyseurs++;if(*analyseurs != ']') skipexpr();
	    }
	}
      match(']');break;
    case '!': analyseurs++;if((*analyseurs)!='=') break; else analyseurs--;
    default: return;
    }
}

void skiptruc(void)
{
  long n=0,p=0,m=1;
  
  if (isalpha(*analyseurs)) {skipidentifier(); return;}
  if (isdigit(*analyseurs) || (*analyseurs=='.')) {skipconstante(); return;}
  switch(*analyseurs++)
    {
    case '(': skipexpr();match(')');return;
    case '[': if (*analyseurs!=']') 
      {do {n++; skipexpr();} while (*analyseurs++==',');analyseurs--;}
      switch (*analyseurs++)
	{
        case ']': return;
        case ';': m=n;do {n++; skipexpr();} while (*analyseurs++!=']');
          if (n % m) err(recter1);
          return;
        default: err(vectmater1);
	}
    case '%':
      p=0;while((*analyseurs)=='`') {analyseurs++;p++;}
      if(p>tglobal) err(referer1);
      if(p) return;
      while (isdigit(*analyseurs)) p = 10*p + *analyseurs++ - '0';
      if(p>tglobal) err(referer1);
      return;
    }
  err(caracer1,analyseurs-1);
}

void skipidentifier(void)
{
  long nparam, i, m;
  entree *ep, **p;
  char *ch1;
  GEN arg;

  ep = skipentry();
  if (ep->valence < 100) /* fonctions predefinies */
    {
      if (!ep->valence && (*analyseurs != '(')) return;
      match('(');
      switch(ep->valence)
	{
	case 0:
	case 56: break;
	case 1:
	case 10:
	case 11:
	case 13:
	case 15:
	case 61: skipexpr(); break;
	case 60: arg=expr();if(typ(arg)!=1) err(caseer);
	  m=itos(arg);if((m>=100)||(m<0)) err(labeler);
	  labellist[m]=analyseurs;break;
	case 51: case 52: case 53: case 54: case 55:
	  if (*analyseurs != ')') for(;;)
	    {
	      if (*analyseurs == '"')
		{
		  analyseurs++;
		  while ((*analyseurs)&&(*analyseurs!='"')) analyseurs++;
		  match('"');
		}
	      else skipexpr();
	      if (*analyseurs == ')') break;
	      match(',');
	    }
	  break;
	case 57:
	  skipexpr();match(',');if(*analyseurs!='"') skipexpr();
	  else
	    {
	      match('"');m=0;
	      while ((m<256)&&(*analyseurs)&&(*analyseurs!='"')) 
		{m++;analyseurs++;}
	      match('"');
	    }
	  break;
	case 2:
	case 12:
	case 14:
	case 20:
	case 21:
	case 23:
	case 24:
	case 25:
	case 28:
	case 29: skipexpr(); match(','); skipexpr(); break;
	case 22: skipexpr(); match(',');
	  if(!isalpha(*analyseurs)) err(varer1,analyseurs);
	  ep = skipentry(); if (ep->valence!=200) err(varer1,analyseurs);
	  match(','); skipexpr(); break;
	case 27: if(!isalpha(*analyseurs)) err(varer1,analyseurs);
	  ep = skipentry(); match('='); if (ep->valence!=200) err(varer1,analyseurs);
	  skipexpr(); match(','); skipexpr(); break;
	case 3:
	case 26: 
	case 30:
	case 31:
	case 32:
	case 33:
	case 34:
	case 35: skipexpr();match(',');skipexpr();match(',');skipexpr();
	  break;
	case 37: if(!isalpha(*analyseurs)) err(varer1,analyseurs);
	  ep = skipentry(); match('='); if (ep->valence!=200) err(varer1,analyseurs);
	  skipexpr(); match(','); skipexpr(); match(','); skipexpr();break;
	case 4: skipexpr();match(',');skipexpr();match(',');skipexpr();
	  match(',');skipexpr();break;
	case 48: skipexpr(); match(',');
	  if(!isalpha(*analyseurs)) err(varer1,analyseurs);
	  ep = skipentry(); match('='); if (ep->valence!=200) err(varer1,analyseurs);
	  skipexpr(); match(','); skipexpr(); match(','); skipexpr();break;
	case 49: skipexpr(); match(','); skipexpr(); match(',');
	  if(!isalpha(*analyseurs)) err(varer1,analyseurs);
	  ep = skipentry(); if (ep->valence!=200) err(varer1,analyseurs);
	  match(',');if(!isalpha(*analyseurs)) err(varer1,analyseurs);
	  ep = skipentry(); if (ep->valence!=200) err(varer1,analyseurs);
	  match(','); skipexpr(); break;
	case 50: skiptruc();
	  if (*analyseurs++=='^') skipfacteur();else analyseurs--;
	  break;
	case 5:
	case 59: skipexpr();match(',');skipexpr();match(',');skipexpr();
	  match(',');skipexpr();match(',');skipexpr();break;
	case 80: skipexpr(); match(','); skipseq(); match(','); skipseq(); break;
	case 81:
	case 82: skipexpr(); match(','); skipseq(); break;
	case 83: if(!isalpha(*analyseurs)) err(varer1,analyseurs);
	  ep = skipentry(); match('='); if (ep->valence!=200) err(varer1,analyseurs);
	  skipexpr(); match(','); skipexpr(); match(','); skipseq(); break;
	case 84: skipexpr(); match(',');
	  if(!isalpha(*analyseurs)) err(varer1,analyseurs);
	  ep = skipentry(); if (ep->valence!=200) err(varer1,analyseurs);
	  match(','); skipseq(); break;
	case 85: if(!isalpha(*analyseurs)) err(killer1);
	  ep = skipentry(); if (ep->valence<100) err(killer1);
	  break;
	case 86: if(!isalpha(*analyseurs)) err(varer1,analyseurs);
	  ep = skipentry(); match('='); if (ep->valence!=200) err(varer1,analyseurs);
	  skipexpr();match(',');skipexpr();match(',');skipexpr();match(',');skipseq();break;
	default: err(valencer1);
	}
      match(')');
      return;
    }
  switch (ep->valence)
    {
    case 200: /* variables */
      if((*analyseurs)=='[')
	{
	  analyseurs++;
	  if(*analyseurs == ',') {analyseurs++;skipexpr();}
	  else
	    {
	      skipexpr();
	      if(*analyseurs == ',')
		{
		  analyseurs++;if(*analyseurs != ']') skipexpr();
		}
	    }
	  match(']');
	}
      if(((*analyseurs)=='=')&&(*(analyseurs+1)!='=')) 
	{
	  analyseurs++;skipexpr(); 
	}
      return;
      
    case 100: /* fonctions utilisateur */
      ch1 = analyseurs;
      match('(');
      p = (entree **)ep->value;
      nparam = (long)*p++;
      i = 0;
      for(i = 0; (i < nparam) && (*analyseurs != ')'); i++)
	{
	  if (i) match(',');
	  skipexpr();
	}
      if ((*analyseurs==')') && ((analyseurs[1] != '=') || (analyseurs[2] == '='))) {analyseurs++; return;}
      while (*analyseurs == ',') {analyseurs++; skipexpr();}
      match(')');
      if ((*analyseurs != '=') || (analyseurs[1] == '=')) err(nparamer1);
      analyseurs = ch1;
      
    case 101: /* nouvelle fonction */
      
      match('(');
      for(nparam = 0; *analyseurs != ')'; nparam++)
	{
	  if (nparam) match(',');
	  skipexpr();
	};
      match(')');
      if (*analyseurs == '=') {analyseurs++; skipseq();}
      return;
      
    default: err(valencer1);
    }
}

void skipconstante(void)
{  
  while (isdigit(*analyseurs)) analyseurs++;
  if ((*analyseurs!='.')&&(*analyseurs!='e')&&(*analyseurs!='E')) return;
  if (*analyseurs=='.') analyseurs++;
  while (isdigit(*analyseurs)) analyseurs++;
  if ((*analyseurs=='e') || (*analyseurs=='E'))
    {
      analyseurs++;
      if (((*analyseurs)=='+') || ((*analyseurs)=='-')) analyseurs++;
      while (isdigit(*analyseurs)) analyseurs++;
    }
}

entree fake101 = {"",101,0,0,0};
entree fake200 = {"",200,0,0,0};

entree *skipentry(void)
{
  char *u, *v, *olds = analyseurs;
  long n;
  entree *ep;
  
  for(n = 0; isalnum(*analyseurs); analyseurs++) n = n << 1 ^ *analyseurs;
  if (n < 0) n = -n; n %= TBLSZ;
  for(ep = hashtable[n]; ep; ep = ep->next)
    {
      for(u = ep->name, v = olds; (*u) && *u == *v; u++, v++);
      if (!*u && (v == analyseurs)) return ep;
    }
  return (*analyseurs == '(') ? &fake101 : &fake200;
}
