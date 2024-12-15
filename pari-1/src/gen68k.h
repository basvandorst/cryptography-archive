/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*                                                                 */
/*                        Fichier Include PARI                     */
/*                                                                 */
/*                   declarations specifiques 680x0                */
/*                                                                 */
/*                        copyright  Babecool                      */
/*                                                                 */
/*                                                                 */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifdef __cplusplus
extern "C" {
#endif
GEN     mpneg(GEN x),mpabs(GEN x),mpshift(GEN x, long y),subis(GEN x, long y),subrs(GEN x, long y),divii(GEN x, GEN y),resii(GEN x, GEN y),negs(long x);
int     typ(GEN x),lg(GEN x),lgef(GEN x),mant(GEN x, long y),signe(GEN x),expo(GEN x),pere(GEN x),valp(GEN x),varn(GEN x);
int     cmpis(GEN x, long y),cmprs(GEN x, long y),cmpri(GEN x, GEN y),precp(GEN x);

void    settyp(GEN x, long y),setlg(GEN x, long y),setlgef(GEN x, long y),setmant(GEN x, long y, long z),setsigne(GEN x, long y),setexpo(GEN x, long y),setpere(GEN x, long y);
void    incpere(GEN x),setvalp(GEN x, long y),setprecp(GEN x, long y),setvarn(GEN x, long y),affsz(long x, GEN y),mpnegz(GEN x, GEN y),mpabsz(GEN x, GEN y);
void    mptruncz(GEN x, GEN y),mpentz(GEN x, GEN y),mpshiftz(GEN x, long y, GEN z),mpaddz(GEN x, GEN y, GEN z),addsiz(long x, GEN y, GEN z),addsrz(long x, GEN y, GEN z);
void    addiiz(GEN x, GEN y, GEN z),addirz(GEN x, GEN y, GEN z),addrrz(GEN x, GEN y, GEN z),mpsubz(GEN x, GEN y, GEN z),subsiz(long x, GEN y, GEN z),subsrz(long x, GEN y, GEN z);
void    subisz(GEN x, long y, GEN z),subiiz(GEN x, GEN y, GEN z),subirz(GEN x, GEN y, GEN z),subrsz(GEN x, long y, GEN z),subriz(GEN x, GEN y, GEN z),subrrz(GEN x, GEN y, GEN z),mpmulz(GEN x, GEN y, GEN z);
void    mulsiz(long x, GEN y, GEN z),mulsrz(long x, GEN y, GEN z),muliiz(GEN x, GEN y, GEN z),mulirz(GEN x, GEN y, GEN z),mulrrz(GEN x, GEN y, GEN z),mpdvmdz(GEN x, GEN y, GEN z, GEN *r);
void    divsrz(long x, GEN y, GEN z),divirz(GEN x, GEN y, GEN z),divrsz(GEN x, long y, GEN z),divriz(GEN x, GEN y, GEN z);
void    mpinvz(GEN x, GEN z),modssz(long x, long y, GEN z),modsiz(long x, GEN y, GEN z),modisz(GEN x, long y, GEN z),resssz(long x, long y, GEN z),ressiz(long x, GEN y, GEN z),resisz(GEN x, long y, GEN z);
void    affrs(GEN x, long y),affri(GEN x, GEN y),addriz(GEN x, GEN y, GEN z),mulriz(GEN x, GEN y, GEN z),mpinvsr(long x, GEN y),mpinvir(GEN x, GEN y),mpinvrr(GEN x, GEN y);

GEN     cgetg(long x, long y),cgetr(long x),cgeti(long x),gerepile(long l, long p, GEN q),stoi(long x);
GEN     negi(GEN x),negr(GEN x),absi(GEN x),absr(GEN x);
GEN     mptrunc(GEN x),mpent(GEN x),shifts(long x, long y),shifti(GEN x, long n),shiftr(GEN x, long n);
GEN     addsi(long x, GEN y),addsr(long x, GEN y),addii(GEN x, GEN y),addir(GEN x, GEN y),addrr(GEN x, GEN y),mpadd(GEN x, GEN y);
GEN     subsi(long x, GEN y),subsr(long x, GEN y),subii(GEN x, GEN y),subir(GEN x, GEN y);
GEN     subri(GEN x, GEN y),subrr(GEN x, GEN y),mpsub(GEN x, GEN y);
GEN     mulss(long x, long y),mulsi(long x, GEN y),mulsr(long x, GEN y),mulii(GEN x, GEN y),mulir(GEN x, GEN y),mulrr(GEN x, GEN y),mpmul(GEN x, GEN y);
GEN     divsi(long x, GEN y),divis(GEN y, long x),divsr(long x, GEN y),divrs(GEN x, long y),divir(GEN x, GEN y);
GEN     divri(GEN x, GEN y),divrr(GEN x, GEN y),mpdiv(GEN x, GEN y),convi(GEN x),confrac(GEN x);
GEN     modss(long x, long y),resss(long x, long y),modsi(long x, GEN y),ressi(long x, GEN y),modis(GEN x, long y),resis(GEN x, long y),modii(GEN x, GEN y);
GEN     dvmdii(GEN x, GEN y, GEN *z),dvmdsi(long x, GEN y, GEN *z),dvmdis(GEN x, long y, GEN *z);
long    itos(GEN x),vals(long x),vali(GEN x),divisii(GEN x, long y, GEN z);
int     expi(GEN x),divise(GEN x, GEN y),mpcmp(GEN x, GEN y),cmpss(long x, long y),cmpsi(long x, GEN y),cmpsr(long x, GEN y),cmpii(GEN x, GEN y),cmpir(GEN x, GEN y),cmprr(GEN x, GEN y), mpdivis(GEN x, GEN y, GEN z);
void    mpaff(GEN x, GEN y),affsi(long s, GEN x),affsr(long s, GEN x),affii(GEN x, GEN y),affir(GEN x, GEN y),affrr(GEN x, GEN y),mulsii(long x, GEN y, GEN z),addsii(long x, GEN y, GEN z);
void    divsiz(long x, GEN y, GEN z),divisz(GEN x, long y, GEN z),divssz(long x, long y, GEN z),diviiz(GEN x, GEN y, GEN z),divrrz(GEN x, GEN y, GEN z),cgiv(GEN x);
void    dvmdssz(long x, long y, GEN z, GEN t),dvmdsiz(long x, GEN y, GEN z, GEN t),dvmdisz(GEN x, long y, GEN z, GEN t),dvmdiiz(GEN x, GEN y, GEN z, GEN t),mpdivz(GEN x, GEN y, GEN z),modiiz(GEN x, GEN y, GEN z);
void    addssz(long x, long y, GEN z),subssz(long x, long y, GEN z),mulssz(long x, long y, GEN z),modiiz(GEN x, GEN y, GEN z),resiiz(GEN x, GEN y, GEN z);
#ifdef __cplusplus
}
#endif



