GEN cgeti(long x),cgetr(long x),stoi(long x);
GEN cgetg(long x, long y),negi(GEN x),negr(GEN x),absi(GEN x),absr(GEN x);
GEN shiftr(GEN x, long n),mpadd(GEN x, GEN y),subii(GEN x, GEN y);
GEN subrr(GEN x, GEN y),subir(GEN x, GEN y),subri(GEN x, GEN y);
GEN mpsub(GEN x, GEN y),subsi(long x, GEN y),subsr(long x, GEN y);
GEN subss(long x, long y),mpmul(GEN x, GEN y);
GEN divss(long x, long y),mpdiv(GEN x, GEN y),dvmdss(long x, long y, GEN *z);
GEN ressi(long x, GEN y),modis(GEN x, long y),resis(GEN x, long y);
double  gtodouble(GEN x);
long itos(GEN x),divisii(GEN x, long y, GEN z),vali(GEN x);
int expi(GEN x),cmpir(GEN x, GEN y),mpcmp(GEN x, GEN y);
int cmpsr(long x, GEN y),mpdivis(GEN x, GEN y, GEN z),divise(GEN x, GEN y);
void affii(GEN x, GEN y),mpaff(GEN x, GEN y),affsi(long s, GEN x);
void affsr(long s, GEN x),addssz(long x, long y, GEN z);
void mulssz(long x, long y, GEN z),mulsii(long x, GEN y, GEN z);
void addsii(long x, GEN y, GEN z),dvmdssz(long x, long y, GEN z, GEN t);
void dvmdsiz(long x, GEN y, GEN z, GEN t),dvmdisz(GEN x, long y, GEN z, GEN t);
void dvmdiiz(GEN x, GEN y, GEN z, GEN t),divisz(GEN x, long y, GEN z);
void divsiz(long x, GEN y, GEN z),divssz(long x, long y, GEN z);
void divrrz(GEN x, GEN y, GEN z),resiiz(GEN x, GEN y, GEN z);

/* mp.c ou mp.s */

GEN     gerepile(long l, long p, GEN q), icopy(GEN x), rcopy(GEN x);
GEN     mptrunc(GEN x),mpent(GEN x),shifts(long x, long y),shifti(GEN x, long n);
GEN     addsi(long x, GEN y),addsr(long x, GEN y),addii(GEN x, GEN y),addir(GEN x, GEN y),addrr(GEN x, GEN y), addss(long x, long y);
GEN     mulss(long x, long y),mulsi(long x, GEN y),mulsr(long x, GEN y),mulii(GEN x, GEN y),mulir(GEN x, GEN y),mulrr(GEN x, GEN y);
GEN     divsi(long x, GEN y),divis(GEN y, long x),divsr(long x, GEN y),divrs(GEN x, long y),divir(GEN x, GEN y);
GEN     divri(GEN x, GEN y),divrr(GEN x, GEN y),convi(GEN x),confrac(GEN x);
GEN     modss(long x, long y),resss(long x, long y),modsi(long x, GEN y),modii(GEN x, GEN y);
GEN     dvmdii(GEN x, GEN y, GEN *z),dvmdsi(long x, GEN y, GEN *z),dvmdis(GEN x, long y, GEN *z);
long    vals(long x);
int     cmpss(long x, long y),cmpsi(long x, GEN y),cmpii(GEN x, GEN y),cmprr(GEN x, GEN y);
void    affir(GEN x, GEN y),affrr(GEN x, GEN y),diviiz(GEN x, GEN y, GEN z);
void    cgiv(GEN x),mpdivz(GEN x, GEN y, GEN z),modiiz(GEN x, GEN y, GEN z);
void    modiiz(GEN x, GEN y, GEN z);








