/*
 *   MIRACL arithmetic routines 1 - multiplying and dividing BIG NUMBERS by  
 *   integer numbers.
 *   mrarth1.c
 *
 *   Copyright (c) 1988-1993 Shamus Software Ltd.
 */

#include <stdio.h>
#include <miracl.h>

#ifdef __TURBOC__ 
#define ASM asm
#if defined(__SMALL__) || defined(__MEDIUM__) 

#ifdef MIRACL_16
#define INLINE_ASM 86
#endif

#if __TURBOC__>=0x410
#ifdef MIRACL_PC
#define INLINE_ASM 386
#endif
#endif

#endif
#endif

#ifdef _MSC_VER
#if _MSC_VER>=600
#define ASM _asm
#if defined(M_I86SM) || defined(M_I86MM)

#ifdef MIRACL_16
#define INLINE_ASM 86
#endif

#endif
#endif
#endif

/* access global variables */

extern small MR_MSBIT;  /* msb of small    */
extern small MR_OBITS;  /* other bits      */
extern small mr_base;   /* number base     */
extern int mr_nib;      /* length of bigs  */
extern int mr_depth;    /* error tracing ..*/
extern int mr_trace[];  /* .. mechanism    */
extern bool mr_check;   /* overflow check  */

void pmul(x,sn,z)
big x,z;
small sn;
{ 
    int m,xl;
    small carry,sx;
    if (x!=z) zero(z);
    m=0;
    carry=0;
    sx=x[0]&MR_MSBIT;
    xl=(int)(x[0]&MR_OBITS);
    if (mr_base==0) 
    {
/* inline 8086 assembly - substitutes for loop below */
#if INLINE_ASM == 86
        ASM cld
        ASM mov cx,xl
        ASM or cx,cx
        ASM je out1
        ASM mov ax,ds
        ASM mov es,ax
        ASM mov bx,sn
        ASM mov di,z
        ASM mov si,x
        ASM inc si
        ASM inc si
        ASM inc di
        ASM inc di
        ASM push bp
        ASM xor bp,bp
    tcl1:
        ASM lodsw
        ASM mul bx
        ASM add ax,bp
        ASM adc dx,0
        ASM stosw
        ASM mov bp,dx
        ASM loop tcl1

        ASM mov ax,bp
        ASM pop bp
        ASM mov carry,ax
     out1:  
#endif
#if INLINE_ASM == 386
        ASM cld
        ASM mov cx,xl
        ASM or cx,cx
        ASM je out1
        ASM mov ax,ds
        ASM mov es,ax
        ASM mov ebx,sn
        ASM mov di,z
        ASM mov si,x
        ASM add si,4
        ASM add di,4
        ASM push ebp
        ASM xor ebp,ebp
    tcl1:
        ASM lodsd
        ASM mul ebx
        ASM add eax,ebp
        ASM adc edx,0
        ASM stosd
        ASM mov ebp,edx
        ASM loop tcl1

        ASM mov eax,ebp
        ASM pop ebp
        ASM mov carry,eax
     out1:  
#endif
#ifndef INLINE_ASM
        for (m=1;m<=xl;m++)
            carry=muldvd(x[m],sn,carry,&z[m]);
#endif
        
        if (carry>0)
        {
            m=xl+1;
            if (m>mr_nib && mr_check)
            {
                berror(3);
                return;
            }
            z[m]=carry;
            z[0]=m;
        }
        else z[0]=xl;
    }
    else while (m<xl || carry>0)
    { /* multiply each digit of x by n */ 
        m++;
        if (m>mr_nib && mr_check)
        {
            berror(3);
            return;
        }
        carry=muldiv(x[m],sn,carry,mr_base,&z[m]);
        z[0]=m;
    }
    if (z[0]!=0) z[0]|=sx;
}

void premult(x,n,z)
big x;
big z;
int n;
{ /* premultiply a big number by an int z=x.n */
    if (MR_ERNUM) return;
    mr_depth++;
    mr_trace[mr_depth]=9;
    if (TRACER) track();
    if (notint(x))
    {
        berror(12);
        mr_depth--;
        return;
    }
    if (n==0)  /* test for some special cases  */
    {
        zero(z);
        mr_depth--;
        return;
    }
    if (n==1)
    {
        copy(x,z);
        mr_depth--;
        return;
    }
    if (n<0)
    {
        n=(-n);
        pmul(x,(small)n,z);
        if (z[0]!=0) z[0]^=MR_MSBIT;
    }
    else pmul(x,(small)n,z);
    mr_depth--;  
}

small sdiv(x,sn,z)
big x,z;
small sn;
{
    int i,xl;
    small sr;
    sr=0;
    xl=(int)(x[0]&MR_OBITS);
    if (x!=z) zero(z);
    if (mr_base==0) 
    {
/* inline - substitutes for loop below */
#if INLINE_ASM == 86
        ASM std
        ASM mov cx,xl
        ASM or cx,cx
        ASM je out2
        ASM mov ax,ds
        ASM mov es,ax
        ASM mov bx,cx
        ASM shl bx,1
        ASM mov si,x
        ASM add si,bx
        ASM mov di,z
        ASM add di,bx
        ASM mov bx,sn
        ASM push bp
        ASM xor bp,bp
    tcl2:
        ASM mov dx,bp
        ASM lodsw
        ASM div bx
        ASM mov bp,dx
        ASM stosw
        ASM loop tcl2

        ASM mov ax,bp
        ASM pop bp
        ASM mov sr,ax
     out2: 
        ASM cld
#endif
#if INLINE_ASM == 386
        ASM std
        ASM mov cx,xl
        ASM or cx,cx
        ASM je out2
        ASM mov ax,ds
        ASM mov es,ax
        ASM mov bx,cx
        ASM shl bx,2
        ASM mov si, x
        ASM add si,bx
        ASM mov di, z
        ASM add di,bx
        ASM mov ebx,sn
        ASM push ebp
        ASM xor ebp,ebp
    tcl2:
        ASM mov edx,ebp
        ASM lodsd
        ASM div ebx
        ASM mov ebp,edx
        ASM stosd
        ASM loop tcl2

        ASM mov eax,ebp
        ASM pop ebp
        ASM mov sr,eax
     out2: 
        ASM cld
#endif
#ifndef INLINE_ASM
        for (i=xl;i>0;i--)
        {
            z[i]=muldvm(sr,x[i],sn,&sr);
        }
#endif
    }
    else for (i=xl;i>0;i--)
    { /* divide each digit of x by n */
        z[i]=muldiv(sr,mr_base,x[i],sn,&sr);
    }
    z[0]=x[0];
    lzero(z);
    return sr;
}
         
int subdiv(x,n,z)
big x;
big z;
int n;
{  /*  subdivide a big number by an int   z=x/n  *
    *  returns int remainder                     */ 
    small sx;
    int r;
    if (MR_ERNUM) return 0;
    mr_depth++;
    mr_trace[mr_depth]=10;
    if (TRACER) track();
    if (notint(x)) berror(12);
    if (n==0) berror(2);
    if (MR_ERNUM) 
    {
        mr_depth--;
        return 0;
    }
    if (n==1) /* special case */
    {
        copy(x,z);
        mr_depth--;
        return 0;
    }
    sx=(x[0]&MR_MSBIT);
    if (n<0)
    {
        n=(-n);
        r=(int)sdiv(x,(small)n,z);
        if (z[0]!=0) z[0]^=MR_MSBIT;
    }
    else r=(int)sdiv(x,(small)n,z);
    mr_depth--;
    if (sx==0) return r;
    else       return (-r);
}

