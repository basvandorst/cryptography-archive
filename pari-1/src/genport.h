/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*                                                                 */
/*                        Fichier Include PARI                     */
/*                                                                 */
/*                 declarations specifiques portables              */
/*                                                                 */
/*                        copyright  Babecool                      */
/*                                                                 */
/*                                                                 */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/* mp.c */

#define signe(x)          (((long)((GEN)(x))[1])>>SIGNSHIFT)

#define setsigne(x,s)     (((GEN)(x))[1]=(((GEN)(x))[1]&(~SIGNBITS))+(((long)(s))<<SIGNSHIFT))
#define typ(x)            (((ulong)((GEN)(x))[0])>>TYPSHIFT)
#define settyp(x,s)       (((GEN)(x))[0]=(((GEN)(x))[0]&(~TYPBITS))+(((ulong)(s))<<TYPSHIFT))
#define pere(x)           ((ulong)(((GEN)(x))[0]&PEREBITS)>>PERESHIFT)
#define setpere(x,s)      (((GEN)(x))[0]=(((GEN)(x))[0]&(~PEREBITS))+(((ulong)(s))<<PERESHIFT))
#define lg(x)             (((GEN)(x))[0]&LGBITS)
#define setlg(x,s)        (((GEN)(x))[0]=(((GEN)(x))[0]&(~LGBITS))+(s))
#define lgef(x)           (((GEN)(x))[1]&LGEFBITS)
#define setlgef(x,s)      (((GEN)(x))[1]=(((GEN)(x))[1]&(~LGEFBITS))+(s))
#define expo(x)           ((((GEN)(x))[1]&EXPOBITS)-HIGHEXPOBIT)
#define setexpo(x,s)      (((GEN)(x))[1]=(((GEN)(x))[1]&(~EXPOBITS))+(HIGHEXPOBIT+(s)))
#define valp(x)           ((((GEN)(x))[1]&VALPBITS)-HIGHVALPBIT)
#define setvalp(x,s)      (((GEN)(x))[1]=(((GEN)(x))[1]&(~VALPBITS))+(HIGHVALPBIT+(s)))
#define precp(x)          (((ulong)((GEN)(x))[1])>>PRECPSHIFT)
#define setprecp(x,s)     (((GEN)(x))[1]=(((GEN)(x))[1]&(~PRECPBITS))+(((long)(s))<<PRECPSHIFT))
#define varn(x)           ((((GEN)(x))[1]&VARNBITS)>>VARNSHIFT)
#define setvarn(x,s)      (((GEN)(x))[1]=(((GEN)(x))[1]&(~VARNBITS))+(((ulong)(s))<<VARNSHIFT))
#define mant(x,i)         ((((GEN)(x))[1]&SIGNBITS)?((GEN)(x))[i+1]:0)
#define setmant(x,i,s)    (((GEN)(x))[i+1]=s)
#define affrs(x,s)        (err(affer4))
#define affri(x,y)        (err(affer5))
#define negs(s)           (stoi(-(s)))
#define mpshift(x,s)      ((typ(x)==1)?shifti(x,s):shiftr(x,s))
#define cmpis(x,y)        (-cmpsi(y,x))
#define cmprs(x,y)        (-cmpsr(y,x))
#define cmpri(x,y)        (-cmpir(y,x))
#define subis(x,y)        (addsi(-(y),x))
#define subrs(x,y)        (addsr(-(y),x))

#define divii(a,b)        (dvmdii(a,b,0))
#define resii(a,b)        (dvmdii(a,b,(GEN *)-1))

#define affsz(s,x)        ((typ(x)==1)?affsi(s,x):affsr(s,x))
#define mpneg(x)          ((typ(x)==1)?negi(x):negr(x))
#define mpabs(x)          ((typ(x)==1)?absi(x):absr(x))
#define mpinvz(x,y)       ((typ(x)==1)?divsiz(1,x,y):divsrz(1,x,y))

#define mpnegz(x,y)       ((typ(x)==1)?gop1z(negi,x,y):gop1z(negr,x,y))
#define mpabsz(x,y)       ((typ(x)==1)?gop1z(absi,x,y):gop1z(absr,x,y))
#define mpshiftz(x,s,y)   ((typ(x)==1)?gops2gsz(shifti,x,s,y):gops2gsz(shiftr,x,s,y))
#define mptruncz(x,y)     (gop1z(mptrunc,x,y))
#define mpentz(x,y)       (gop1z(mpent,x,y))
#define mpaddz(x,y,z)     (gop2z(mpadd,x,y,z))
#define addsiz(s,y,z)     (gops2sgz(addsi,s,y,z))
#define addsrz(s,y,z)     (gops2sgz(addsr,s,y,z))
#define addiiz(x,y,z)     (gop2z(addii,x,y,z))
#define addirz(x,y,z)     (gop2z(addir,x,y,z))
#define addriz(x,y,z)     (gop2z(addir,y,x,z))
#define addrrz(x,y,z)     (gop2z(addrr,x,y,z))
#define mpsubz(x,y,z)     (gop2z(mpsub,x,y,z))
#define subsiz(s,y,z)     (gops2sgz(subsi,s,y,z))
#define subsrz(s,y,z)     (gops2sgz(subsr,s,y,z))
#define subisz(y,s,z)     (gops2sgz(addsi,-(s),y,z))
#define subrsz(y,s,z)     (gops2sgz(addsr,-(s),y,z))
#define subiiz(x,y,z)     (gop2z(subii,x,y,z))
#define subirz(x,y,z)     (gop2z(subir,x,y,z))
#define subriz(x,y,z)     (gop2z(subri,x,y,z))
#define subrrz(x,y,z)     (gop2z(subrr,x,y,z))
#define mpmulz(x,y,z)     (gop2z(mpmul,x,y,z))
#define mulsiz(s,y,z)     (gops2sgz(mulsi,s,y,z))
#define mulsrz(s,y,z)     (gops2sgz(mulsr,s,y,z))
#define muliiz(x,y,z)     (gop2z(mulii,x,y,z))
#define mulirz(x,y,z)     (gop2z(mulir,x,y,z))
#define mulriz(x,y,z)     (gop2z(mulir,y,x,z))
#define mulrrz(x,y,z)     (gop2z(mulrr,x,y,z))
#define mpinvsr(s,y)      (divssz(1,s,y))
#define mpinvir(x,y)      (divsiz(1,x,y))
#define mpinvrr(x,y)      (divsrz(1,x,y))
#define mpdvmdz(x,y,z,t)  (dvmdiiz(x,y,z,t))
#define modssz(s,y,z)     (gops2ssz(modss,s,y,z))
#define modsiz(s,y,z)     (gops2sgz(modsi,s,y,z))
#define modisz(y,s,z)     (gops2gsz(modis,y,s,z))
#define ressiz(s,y,z)     (gops2sgz(ressi,s,y,z))
#define resisz(y,s,z)     (gops2gsz(resis,y,s,z))
#define resssz(s,y,z)     (gops2ssz(resss,s,y,z))
#define divirz(x,y,z)     (gop2z(divir,x,y,z))
#define divriz(x,y,z)     (gop2z(divri,x,y,z))
#define divsrz(s,y,z)     (gops2sgz(divsr,s,y,z))
#define divrsz(y,s,z)     (gops2gsz(divrs,y,s,z))

