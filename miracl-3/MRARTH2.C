/*
 *   MIRACL arithmetic routines 2 - multiplying and dividing BIG NUMBERS.
 *   mrarth2.c
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

/* Access global variables */

extern small MR_MSBIT;   /* msb of small    */
extern small MR_OBITS;   /* other bits      */
extern small mr_base;    /* number base     */
extern int  mr_nib;      /* length of bigs  */
extern int  mr_depth;    /* error tracing ..*/
extern int  mr_trace[];  /* .. mechanism    */
extern big  mr_w0;       /* workspace big   */
extern bool mr_check;    /* error checking  */

small normalise(x,y)
big x,y;
{ /* normalise divisor */
    small norm,r;
    int len;
    mr_depth++;
    mr_trace[mr_depth]=4;
    if (TRACER) track();
    if (x!=y) copy(x,y);
    len=(int)(y[0]&MR_OBITS);
    if (mr_base==0)
    {
        if ((r=y[len]+1)==0) norm=1;
        else norm=muldvm((small)1,(small)0,r,&r);
        if (norm!=1) pmul(y,norm,y);
    }
    else
    {
        norm=mr_base/(y[len]+1);   
        if (norm!=1) pmul(y,norm,y);
    }
    mr_depth--;
    return norm;
}

void multiply(x,y,z)
big x;
big y;
big z;
{  /*  multiply two big numbers: z=x.y  */
    int i,xl,yl,j,ti;
    small carry,sz,dig;
    if (MR_ERNUM) return;
    if (y[0]==0 || x[0]==0) 
    {
        zero(z);
        return;
    }
    mr_depth++;
    mr_trace[mr_depth]=5;
    if (TRACER) track();
    if (notint(x) || notint(y))
    {
        berror(12);
        mr_depth--;
        return;
    }
    sz=((x[0]&MR_MSBIT)^(y[0]&MR_MSBIT));
    xl=(int)(x[0]&MR_OBITS);
    yl=(int)(y[0]&MR_OBITS);
    zero(mr_w0);
    if (mr_check && xl+yl>mr_nib)
    {
        berror(3);
        mr_depth--;
        return;
    }
    if (mr_base==0)
    {
        if (x==y)
        { /* fast squaring */
            for (i=1;i<xl;i++)
            {  /* long multiplication */
           /* inline - substitutes for loop below */
#if INLINE_ASM == 86
                ASM cld
                ASM mov si,i
                ASM mov cx,xl
                ASM sub cx,si
                ASM shl si,1
                ASM mov dx,x
                ASM mov bx,dx
                ASM add bx,si
                ASM mov di,[bx]
                ASM mov bx,mr_w0
                ASM add bx,si
                ASM add si,dx
                ASM sub bx,dx
                ASM inc si
                ASM inc si
                ASM sub bx,4
                ASM push bp
                ASM xor bp,bp
              tcl4:
                ASM lodsw
                ASM mul di
                ASM add ax,bp
                ASM adc dx,0
                ASM add [si+bx],ax
                ASM adc dx,0
                ASM mov bp,dx
                ASM loop tcl4

                ASM mov [si+bx+2],bp
                ASM pop bp
#endif
#if INLINE_ASM == 386
                ASM cld
                ASM mov si,i
                ASM mov cx,xl
                ASM sub cx,si
                ASM shl si,2
                ASM mov dx, x
                ASM mov bx,dx
                ASM add bx,si
                ASM mov edi,[bx]
                ASM mov bx,mr_w0
                ASM add bx,si
                ASM add si,dx
                ASM sub bx,dx
                ASM add si,4
                ASM sub bx,8
                ASM push ebp
                ASM xor ebp,ebp
              tcl4:
                ASM lodsd
                ASM mul edi
                ASM add eax,ebp
                ASM adc edx,0
                ASM add [si+bx],eax
                ASM adc edx,0
                ASM mov ebp,edx
                ASM loop tcl4

                ASM mov [si+bx+4],ebp
                ASM pop ebp
#endif
#ifndef INLINE_ASM
                carry=0;
                for (j=i+1;j<=xl;j++)
                { /* Only do above the diagonal */
                    carry=muldvd(x[i],x[j],carry,&dig);
                    mr_w0[i+j-1]+=dig;
                    if (mr_w0[i+j-1]<dig) carry++;
                }
                mr_w0[xl+i]=carry;
#endif
            }
#if INLINE_ASM == 86
            ASM mov cx,xl
            ASM shl cx,1
            ASM mov di,mr_w0
          tcl5:
            ASM inc di
            ASM inc di
            ASM rcl WORD PTR [di],1
            ASM loop tcl5

            ASM cld
            ASM mov cx,xl
            ASM mov si,x
            ASM mov di,mr_w0
            ASM inc si
            ASM inc si
            ASM inc di
            ASM inc di
            ASM xor bx,bx
          tcl7:
            ASM lodsw
            ASM mul ax
            ASM add ax,bx
            ASM adc dx,0
            ASM add [di],ax
            ASM adc dx,0
            ASM xor bx,bx
            ASM inc di
            ASM inc di
            ASM add [di],dx
            ASM adc bx,0
            ASM inc di
            ASM inc di
            ASM loop tcl7
#endif
#if INLINE_ASM == 386
            ASM mov cx,xl
            ASM shl cx,1
            ASM mov di,mr_w0
          tcl5:
            ASM inc di
            ASM inc di
            ASM inc di
            ASM inc di
            ASM rcl DWORD PTR [di],1
            ASM loop tcl5

            ASM cld
            ASM mov cx,xl
            ASM mov si,x
            ASM mov di,mr_w0
            ASM add si,4
            ASM add di,4
            ASM xor ebx,ebx
          tcl7:
            ASM lodsd
            ASM mul eax
            ASM add eax,ebx
            ASM adc edx,0
            ASM add [di],eax
            ASM adc edx,0
            ASM xor ebx,ebx
            ASM add di,4
            ASM add [di],edx
            ASM adc ebx,0
            ASM add di,4
            ASM loop tcl7
#endif
#ifndef INLINE_ASM
            mr_w0[0]=xl+xl-1;
            padd(mr_w0,mr_w0,mr_w0);     /* double it */
            carry=0;
            for (i=1;i<=xl;i++)
            { /* add in squared elements */
                ti=i+i;
                carry=muldvd(x[i],x[i],carry,&dig);
                mr_w0[ti-1]+=dig;
                if (mr_w0[ti-1]<dig) carry++;
                mr_w0[ti]+=carry;
                if (mr_w0[ti]<carry) carry=1;
                else              carry=0;
            }
#endif
        }
        else for (i=1;i<=xl;i++)
        { /* long multiplication */
       /* inline - substitutes for loop below */
#if INLINE_ASM == 86
            ASM cld
            ASM mov cx,yl
            ASM mov si,i
            ASM shl si,1
            ASM mov bx,x
            ASM add bx,si
            ASM mov di,[bx]
            ASM mov bx,mr_w0
            ASM add bx,si
            ASM mov si,y
            ASM inc si
            ASM inc si
            ASM sub bx,si
            ASM dec bx
            ASM dec bx
            ASM push bp
            ASM xor bp,bp
          tcl6:
            ASM lodsw
            ASM mul di
            ASM add ax,bp
            ASM adc dx,0
            ASM add [si+bx],ax
            ASM adc dx,0
            ASM mov bp,dx
            ASM loop tcl6

            ASM mov [si+bx+2],bp
            ASM pop bp
#endif
#if INLINE_ASM == 386
            ASM cld
            ASM mov cx,yl
            ASM mov si,i
            ASM shl si,2
            ASM mov bx,x
            ASM add bx,si
            ASM mov edi,[bx]
            ASM mov bx,mr_w0
            ASM add bx,si
            ASM mov si,y
            ASM add si,4
            ASM sub bx,si
            ASM sub bx,4
            ASM push ebp
            ASM xor ebp,ebp
          tcl6:
            ASM lodsd
            ASM mul edi
            ASM add eax,ebp
            ASM adc edx,0
            ASM add [si+bx],eax
            ASM adc edx,0
            ASM mov ebp,edx
            ASM loop tcl6

            ASM mov [si+bx+4],ebp
            ASM pop ebp
#endif
#ifndef INLINE_ASM
            carry=0;
            for (j=1;j<=yl;j++)
            { /* multiply each digit of y by x[i] */
                carry=muldvd(x[i],y[j],carry,&dig);
                mr_w0[i+j-1]+=dig;
                if (mr_w0[i+j-1]<dig) carry++;
            }
            mr_w0[yl+i]=carry;
#endif
        }
    }
    else
    {
        if (x==y)
        { /* squaring can be done nearly twice as fast */
            for (i=1;i<xl;i++)
            { /* long multiplication */
                carry=0;
                for (j=i+1;j<=xl;j++)
                { /* Only do above the diagonal */
                    carry=muldiv(x[i],x[j],mr_w0[i+j-1]+carry,mr_base,&mr_w0[i+j-1]);
                }
                mr_w0[xl+i]=carry;
            }
            mr_w0[0]=xl+xl-1;
            padd(mr_w0,mr_w0,mr_w0);     /* double it */
            carry=0;
            for (i=1;i<=xl;i++)
            { /* add in squared elements */
                ti=i+i;
                carry=muldiv(x[i],x[i],mr_w0[ti-1]+carry,mr_base,&mr_w0[ti-1]);
                mr_w0[ti]+=carry;
                carry=0;
                if (mr_w0[ti]>=mr_base)
                {
                    carry=1;
                    mr_w0[ti]-=mr_base;
                }
            }
        }
        else for (i=1;i<=xl;i++)
        { /* long multiplication */
            carry=0; 
            for (j=1;j<=yl;j++)
            { /* multiply each digit of y by x[i] */
                carry=muldiv(x[i],y[j],mr_w0[i+j-1]+carry,mr_base,&mr_w0[i+j-1]);
            }
            mr_w0[yl+i]=carry;
        }
    }
    mr_w0[0]=(sz|(xl+yl)); /* set length and sign of result */
    lzero(mr_w0);
    copy(mr_w0,z);
    mr_depth--;
}

void divide(x,y,z)
big x;
big y;
big z;
{  /*  divide two big numbers  z=x/y : x=x mod y  *
    *  returns quotient only if  divide(x,y,x)    *
    *  returns remainder only if divide(x,y,y)    */
    small carry,borrow,try,ldy,sdy,ra,r,d,tst,dig,psum,sx,sy,sz;
    int i,k,m,x0,y0,w00;
    bool check;
    if (MR_ERNUM) return;
    mr_depth++;
    mr_trace[mr_depth]=6;
    if (TRACER) track();
    if (x==y) berror(7);
    if (notint(x) || notint(y)) berror(12);
    if (y[0]==0) berror(2);
    if (MR_ERNUM)
    {
        mr_depth--;
        return;
    }
    sx=(x[0]&MR_MSBIT);   /*  extract signs ... */
    sy=(y[0]&MR_MSBIT);
    sz=(sx^sy);
    x[0]&=MR_OBITS;   /* ... and force operands to positive  */
    y[0]&=MR_OBITS;
    x0=(int)x[0];
    y0=(int)y[0];
    copy(x,mr_w0);
    w00=(int)mr_w0[0];
    if (mr_check && (w00-y0+1>mr_nib))
    {
        berror(3);
        mr_depth--;
        return;
    }
    d=0;
    if (x0==y0)
    {
        if (x0==1) /* special case - x and y are both smalls */
        { 
            d=mr_w0[1]/y[1];
            mr_w0[1]=(mr_w0[1]%y[1]);
            lzero(mr_w0);
        }
        else if (mr_w0[x0]/4<y[x0])
        while (compare(mr_w0,y)>=0)
        {  /* small quotient - so do up to four subtracts instead */
            psub(mr_w0,y,mr_w0);
            d++;
        }
    }
    if (compare(mr_w0,y)<0)
    {  /*  x less than y - so x becomes remainder */
        if (x!=z)  /* testing parameters */
        {
            copy(mr_w0,x);
            x[0]|=sx;
        }
        if (y!=z)
        {
            zero(z);
            z[1]=d;
            if (d>0) z[0]=(sz|1);
        }
        y[0]|=sy;
        mr_depth--;
        return;
    }
    if (y0==1)
    {  /* y is int - so use subdiv instead */
        r=sdiv(mr_w0,y[1],mr_w0);
        if (y!=z)
        {
            copy(mr_w0,z);
            z[0]|=sz;
        }
        if (x!=z)
        {
            zero(x);
            x[1]=r;
            if (r>0) x[0]=(sx|1);
        }
        y[0]|=sy;
        mr_depth--;
        return;
    }
    if (y!=z) zero(z);
    d=normalise(y,y);
    check=mr_check;
    mr_check=OFF;
    if (mr_base==0)
    {
        if (d!=1) pmul(mr_w0,d,mr_w0);
        ldy=y[y0];
        sdy=y[y0-1];
        for (k=w00;k>=y0;k--)
        {  /* long division */
#if INLINE_ASM == 86
            ASM mov bx,mr_w0
            ASM mov si,k
            ASM shl si,1
            ASM add bx,si
            ASM mov dx,[bx+2]
            ASM mov ax,[bx]
            ASM cmp dx,ldy
            ASM jne tcl8
            ASM mov di,0xffff
            ASM mov si,ax
            ASM add si,ldy
            ASM jc tcl12
            ASM jmp tcl10
          tcl8:
            ASM div WORD PTR ldy
            ASM mov di,ax
            ASM mov si,dx
          tcl10:
            ASM mov ax,sdy
            ASM mul di
            ASM cmp dx,si
            ASM jb tcl12
            ASM jne tcl11
            ASM cmp ax,[bx-2]
            ASM jbe tcl12
          tcl11:
            ASM dec di
            ASM add si,ldy
            ASM jnc tcl10
          tcl12:
            ASM mov try,di
#endif
/* NOTE push and pop of esi/edi should not be necessary - Borland C bug *
 * These pushes are needed here even if register variables are disabled */
#if INLINE_ASM == 386
            ASM push esi
            ASM push edi
            ASM mov bx,mr_w0
            ASM mov si,k
            ASM shl si,2
            ASM add bx,si
            ASM mov edx,[bx+4]
            ASM mov eax,[bx]
            ASM cmp edx,ldy
            ASM jne tcl8
            ASM mov edi,0xffffffff
            ASM mov esi,eax
            ASM add esi,ldy
            ASM jc tcl12
            ASM jmp tcl10
          tcl8:
            ASM div DWORD PTR ldy
            ASM mov edi,eax
            ASM mov esi,edx
          tcl10:
            ASM mov eax,sdy
            ASM mul edi
            ASM cmp edx,esi
            ASM jb tcl12
            ASM jne tcl11
            ASM cmp eax,[bx-4]
            ASM jbe tcl12
          tcl11:
            ASM dec edi
            ASM add esi,ldy
            ASM jnc tcl10
          tcl12:
            ASM mov try,edi
            ASM pop edi
            ASM pop esi
#endif  
#ifndef INLINE_ASM
            carry=0;
            if (mr_w0[k+1]==ldy) /* guess next quotient digit */
            {
                try=(small)(-1);
                ra=ldy+mr_w0[k];
                if (ra<ldy) carry=1;
            }
            else try=muldvm(mr_w0[k+1],mr_w0[k],ldy,&ra);
            while (carry==0)
            {
                tst=muldvd(sdy,try,(small)0,&r); 
                if (tst< ra || (tst==ra && r<=mr_w0[k-1])) break;
                try--;  /* refine guess */
                ra+=ldy;
                if (ra<ldy) carry=1;
            }
#endif    
            m=k-y0;
            if (try>0)
            { /* do partial subtraction */
                borrow=0;
    /*  inline - substitutes for loop below */
#if INLINE_ASM == 86
                ASM cld
                ASM mov cx,y0
                ASM mov si,m
                ASM shl si,1
                ASM mov di,try
                ASM mov bx,mr_w0
                ASM add bx,si
                ASM mov si,y
                ASM inc si
                ASM inc si
                ASM sub bx,si
                ASM push bp
                ASM xor bp,bp

             tcl3:
                ASM lodsw
                ASM mul di
                ASM add ax,bp
                ASM adc dx,0
                ASM sub [si+bx],ax
                ASM adc dx,0
                ASM mov bp,dx
                ASM loop tcl3

                ASM mov ax,bp
                ASM pop bp
                ASM mov borrow,ax
#endif
/* NOTE push and pop of esi/edi should not be necessary - Borland C bug *
 * These pushes are needed here even if register variables are disabled */
#if INLINE_ASM == 386
                ASM push esi
                ASM push edi
                ASM cld
                ASM mov cx,y0
                ASM mov si,m
                ASM shl si,2
                ASM mov edi,try
                ASM mov bx,mr_w0
                ASM add bx,si
                ASM mov si,y
                ASM add si,4
                ASM sub bx,si
                ASM push ebp
                ASM xor ebp,ebp

             tcl3:
                ASM lodsd
                ASM mul edi
                ASM add eax,ebp
                ASM adc edx,0
                ASM sub [si+bx],eax
                ASM adc edx,0
                ASM mov ebp,edx
                ASM loop tcl3

                ASM mov eax,ebp
                ASM pop ebp
                ASM mov borrow,eax
                ASM pop edi
                ASM pop esi
#endif
#ifndef INLINE_ASM
                for (i=1;i<=y0;i++)
                {
                  borrow=muldvd(try,y[i],borrow,&dig);
                  if (mr_w0[m+i]<dig) borrow++;
                  mr_w0[m+i]-=dig;
                }
#endif
                if (mr_w0[k+1]<borrow)
                {  /* whoops! - over did it */
                    mr_w0[k+1]=0;
                    carry=0;
                    for (i=1;i<=y0;i++)
                    {  /* compensate for error ... */
                        psum=mr_w0[m+i]+y[i]+carry;
                        if (psum>y[i]) carry=0;
                        if (psum<y[i]) carry=1;
                        mr_w0[m+i]=psum;
                    }
                    try--;  /* ... and adjust guess */
                }
                else mr_w0[k+1]-=borrow;
            }
            if (k==w00 && try==0) w00--;
            else if (y!=z) z[m+1]=try;
        }
    }
    else
    {   /* have to do it the hard way */
        if (d!=1) pmul(mr_w0,d,mr_w0);
        ldy=y[y0];
        sdy=y[y0-1];
        for (k=w00;k>=y0;k--)
        {  /* long division */
            if (mr_w0[k+1]==ldy) /* guess next quotient digit */
            {
                try=mr_base-1;
                ra=ldy+mr_w0[k];
            }
            else try=muldiv(mr_w0[k+1],mr_base,mr_w0[k],ldy,&ra);
            while (ra<mr_base)
            {
                tst=muldiv(sdy,try,(small)0,mr_base,&r); 
                if (tst< ra || (tst==ra && r<=mr_w0[k-1])) break;
                try--;  /* refine guess */
                ra+=ldy;
            }    
            m=k-y0;
            if (try>0)
            { /* do partial subtraction */
                borrow=0;
                for (i=1;i<=y0;i++)
                {
                  borrow=muldiv(try,y[i],borrow,mr_base,&dig);
                  if (mr_w0[m+i]<dig)
                  { /* set borrow */
                      borrow++;
                      mr_w0[m+i]+=(mr_base-dig);
                  }
                  else mr_w0[m+i]-=dig;
                }
                if (mr_w0[k+1]<borrow)
                {  /* whoops! - over did it */
                    mr_w0[k+1]=0;
                    carry=0;
                    for (i=1;i<=y0;i++)
                    {  /* compensate for error ... */
                        psum=mr_w0[m+i]+y[i]+carry;
                        carry=0;
                        if (psum>=mr_base)
                        {
                            carry=1;
                            psum-=mr_base;
                        }
                        mr_w0[m+i]=psum;
                    }
                    try--;  /* ... and adjust guess */
                }
                else mr_w0[k+1]-=borrow;
            }
            if (k==w00 && try==0) w00--;
            else if (y!=z) z[m+1]=try;
        }
    }
    if (y!=z) z[0]=((w00-y0+1)|sz); /* set sign and length of result */
    mr_w0[0]=y0;
    lzero(y);
    lzero(z);
    if (x!=z)
    {
        lzero(mr_w0);
        if (d!=1) sdiv(mr_w0,d,x);
        else copy(mr_w0,x);
        if (x[0]!=0) x[0]|=sx;
    }
    if (d!=1) sdiv(y,d,y);
    y[0]|=sy;
    mr_check=check;
    mr_depth--;
}

void mad(x,y,z,w,q,r)
big x,y,z,w,q,r;
{ /* Multiply, Add and Divide; q=(x*y+z)/w remainder r   *
   * returns remainder only if w=q, quotient only if q=r *
   * add done only if x, y and z are distinct.           */
    if (MR_ERNUM) return;
    mr_depth++;
    mr_trace[mr_depth]=24;
    if (TRACER) track();
    mr_check=OFF;           /* turn off some error checks */
    if (w==r)
    {
        berror(7);
        mr_depth--;
        return;
    }
    multiply(x,y,mr_w0);
    if (x!=z && y!=z)add(mr_w0,z,mr_w0);
    divide(mr_w0,w,q);
    if (q!=r) copy(mr_w0,r);
    mr_check=ON;
    mr_depth--;
}

