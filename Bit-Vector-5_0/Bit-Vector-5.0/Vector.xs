

/*****************************************************************************/
/*                                                                           */
/*    Copyright (c) 1995, 1996, 1997, 1998 by Steffen Beyer.                 */
/*    All rights reserved.                                                   */
/*                                                                           */
/*    This piece of software is "Non-Profit-Ware" ("NP-ware").               */
/*                                                                           */
/*    You may use, copy, modify and redistribute it under                    */
/*    the terms of the "Non-Profit-License" (NPL).                           */
/*                                                                           */
/*    Please refer to the file "NONPROFIT" in this distribution              */
/*    for details!                                                           */
/*                                                                           */
/*****************************************************************************/


#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"


#include "BitVector.h"


static    char *BitVector_Class = "Bit::Vector";

typedef     SV *BitVector_Object;
typedef     SV *BitVector_Handle;
typedef N_word *BitVector_Address;
typedef     SV *BitVector_Scalar;


#define BIT_VECTOR_OBJECT(ref,hdl,adr) \
    ( ref && SvROK(ref) && \
    (hdl = (BitVector_Handle)SvRV(ref)) && \
    SvOBJECT(hdl) && (SvTYPE(hdl) == SVt_PVMG) && \
    (strEQ(HvNAME(SvSTASH(hdl)),BitVector_Class)) && \
    SvREADONLY(hdl) && (adr = (BitVector_Address)SvIV(hdl)) )

#define BIT_VECTOR_SCALAR(ref,typ,var) \
    ( ref && !(SvROK(ref)) && ((var = (typ)SvIV(ref)) | 1) )

#define BIT_VECTOR_STRING(ref,var) \
    ( ref && !(SvROK(ref)) && (var = (charptr)SvPV(ref,na)) )

#define BIT_VECTOR_BUFFER(ref,var,len) \
    ( ref && !(SvROK(ref)) && SvPOK(ref) && \
    (var = (charptr)SvPV(ref,na)) && \
    ((len = (N_int)SvCUR(ref)) | 1) )


#define BIT_VECTOR_ERROR(name,error) \
    croak("Bit::Vector::" name "(): " error)

#define BIT_VECTOR_MEMORY_ERROR(name) \
    BIT_VECTOR_ERROR(name,"unable to allocate memory")

#define BIT_VECTOR_OBJECT_ERROR(name) \
    BIT_VECTOR_ERROR(name,"item is not a 'Bit::Vector' object")

#define BIT_VECTOR_SCALAR_ERROR(name) \
    BIT_VECTOR_ERROR(name,"item is not a scalar")

#define BIT_VECTOR_STRING_ERROR(name) \
    BIT_VECTOR_ERROR(name,"item is not a string")

#define BIT_VECTOR_INDEX_ERROR(name) \
    BIT_VECTOR_ERROR(name,"index out of range")

#define BIT_VECTOR_MIN_ERROR(name) \
    BIT_VECTOR_ERROR(name,"minimum index out of range")

#define BIT_VECTOR_MAX_ERROR(name) \
    BIT_VECTOR_ERROR(name,"maximum index out of range")

#define BIT_VECTOR_ORDER_ERROR(name) \
    BIT_VECTOR_ERROR(name,"minimum > maximum index")

#define BIT_VECTOR_START_ERROR(name) \
    BIT_VECTOR_ERROR(name,"start index out of range")

#define BIT_VECTOR_OFFSET_ERROR(name) \
    BIT_VECTOR_ERROR(name,"offset out of range")

#define BIT_VECTOR_CHUNK_ERROR(name) \
    BIT_VECTOR_ERROR(name,"chunk size out of range")

#define BIT_VECTOR_SIZE_ERROR(name) \
    BIT_VECTOR_ERROR(name,"bit vector size mismatch")

#define BIT_VECTOR_SET_ERROR(name) \
    BIT_VECTOR_ERROR(name,"set size mismatch")

#define BIT_VECTOR_MATRIX_ERROR(name) \
    BIT_VECTOR_ERROR(name,"matrix size mismatch")

#define BIT_VECTOR_SHAPE_ERROR(name) \
    BIT_VECTOR_ERROR(name,"matrix is not quadratic")

#define BIT_VECTOR_SYNTAX_ERROR(name) \
    BIT_VECTOR_ERROR(name,"input string syntax error")

#define BIT_VECTOR_OVERFLOW_ERROR(name) \
    BIT_VECTOR_ERROR(name,"numeric overflow error")

#define BIT_VECTOR_DISTINCT_ERROR(name) \
    BIT_VECTOR_ERROR(name,"Q and R must be distinct")

#define BIT_VECTOR_ZERO_ERROR(name) \
    BIT_VECTOR_ERROR(name,"division by zero error")

#define BIT_VECTOR_INTERNAL_ERROR(name) \
    BIT_VECTOR_ERROR(name,"unexpected internal error - please contact author")

#define BIT_VECTOR_EXCEPTION(code,name) \
    { switch (code) { case ErrCode_Ok: break; \
    case ErrCode_Null: BIT_VECTOR_MEMORY_ERROR(name); break; \
    case ErrCode_Indx: BIT_VECTOR_INDEX_ERROR(name); break; \
    case ErrCode_Ordr: BIT_VECTOR_ORDER_ERROR(name); break; \
    case ErrCode_Size: BIT_VECTOR_SIZE_ERROR(name); break; \
    case ErrCode_Pars: BIT_VECTOR_SYNTAX_ERROR(name); break; \
    case ErrCode_Ovfl: BIT_VECTOR_OVERFLOW_ERROR(name); break; \
    case ErrCode_Same: BIT_VECTOR_DISTINCT_ERROR(name); break; \
    case ErrCode_Zero: BIT_VECTOR_ZERO_ERROR(name); break; \
    default: BIT_VECTOR_INTERNAL_ERROR(name); break; } }


MODULE = Bit::Vector		PACKAGE = Bit::Vector		PREFIX = BitVector_


PROTOTYPES: DISABLE


BOOT:
{
    ErrCode rc;

    if (rc = BitVector_Boot())
    {
        switch (rc)
        {
            case ErrCode_Type:
                BIT_VECTOR_ERROR("Boot","sizeof(word) > sizeof(size_t)");
                break;
            case ErrCode_Bits:
                BIT_VECTOR_ERROR("Boot","bits(word) != sizeof(word)*8");
                break;
            case ErrCode_Word:
                BIT_VECTOR_ERROR("Boot","bits(word) < 16");
                break;
            case ErrCode_Long:
                BIT_VECTOR_ERROR("Boot","bits(word) > bits(long)");
                break;
            case ErrCode_Powr:
                BIT_VECTOR_ERROR("Boot","bits(word) != 2^x");
                break;
            case ErrCode_Loga:
                BIT_VECTOR_ERROR("Boot","bits(word) != 2^ld(bits(word))");
                break;
            case ErrCode_Null:
                BIT_VECTOR_MEMORY_ERROR("Boot");
                break;
            default:
                BIT_VECTOR_INTERNAL_ERROR("Boot");
                break;
        }
        exit(rc);
    }
}


void
BitVector_Version(...)
PPCODE:
{
    charptr string;

    if ((items >= 0) and (items <= 1))
    {
        string = BitVector_Version();
        if (string != NULL)
        {
            EXTEND(sp,1);
            PUSHs(sv_2mortal(newSVpv((char *)string,0)));
        }
        else BIT_VECTOR_MEMORY_ERROR("Version");
    }
    else croak("Usage: Bit::Vector->Version()");
}


N_int
BitVector_Word_Bits(...)
CODE:
{
    if ((items >= 0) and (items <= 1))
    {
        RETVAL = BitVector_Word_Bits();
    }
    else croak("Usage: Bit::Vector->Word_Bits()");
}
OUTPUT:
RETVAL


N_int
BitVector_Long_Bits(...)
CODE:
{
    if ((items >= 0) and (items <= 1))
    {
        RETVAL = BitVector_Long_Bits();
    }
    else croak("Usage: Bit::Vector->Long_Bits()");
}
OUTPUT:
RETVAL


void
BitVector_Create(class,bits)
BitVector_Object	class
BitVector_Scalar	bits
ALIAS:
  new = 2
PPCODE:
{
    BitVector_Address address;
    BitVector_Handle  handle;
    BitVector_Object  reference;
    N_int size;

    if ( BIT_VECTOR_SCALAR(bits,N_int,size) )
    {
        if ((address = BitVector_Create(size,true)) != NULL)
        {
            handle = newSViv((IV)address);
            reference = sv_bless(sv_2mortal(newRV(handle)),
                gv_stashpv(BitVector_Class,1));
            SvREFCNT_dec(handle);
            SvREADONLY_on(handle);
            PUSHs(reference);
        }
        else BIT_VECTOR_MEMORY_ERROR("Create");
    }
    else BIT_VECTOR_SCALAR_ERROR("Create");
}


void
BitVector_Shadow(reference)
BitVector_Object        reference
PPCODE:
{
    BitVector_Handle  handle;
    BitVector_Address address;

    if ( BIT_VECTOR_OBJECT(reference,handle,address) )
    {
        if ((address = BitVector_Shadow(address)) != NULL)
        {
            handle = newSViv((IV)address);
            reference = sv_bless(sv_2mortal(newRV(handle)),
                gv_stashpv(BitVector_Class,1));
            SvREFCNT_dec(handle);
            SvREADONLY_on(handle);
            PUSHs(reference);
        }
        else BIT_VECTOR_MEMORY_ERROR("Shadow");
    }
    else BIT_VECTOR_OBJECT_ERROR("Shadow");
}


void
BitVector_Clone(reference)
BitVector_Object        reference
PPCODE:
{
    BitVector_Handle  handle;
    BitVector_Address address;

    if ( BIT_VECTOR_OBJECT(reference,handle,address) )
    {
        if ((address = BitVector_Clone(address)) != NULL)
        {
            handle = newSViv((IV)address);
            reference = sv_bless(sv_2mortal(newRV(handle)),
                gv_stashpv(BitVector_Class,1));
            SvREFCNT_dec(handle);
            SvREADONLY_on(handle);
            PUSHs(reference);
        }
        else BIT_VECTOR_MEMORY_ERROR("Clone");
    }
    else BIT_VECTOR_OBJECT_ERROR("Clone");
}


void
BitVector_Concat(Xref,Yref)
BitVector_Object        Xref
BitVector_Object        Yref
PPCODE:
{
    BitVector_Handle  Xhdl;
    BitVector_Address Xadr;
    BitVector_Handle  Yhdl;
    BitVector_Address Yadr;
    BitVector_Object  reference;
    BitVector_Handle  handle;
    BitVector_Address address;

    if ( BIT_VECTOR_OBJECT(Xref,Xhdl,Xadr) &&
         BIT_VECTOR_OBJECT(Yref,Yhdl,Yadr) )
    {
        if ((address = BitVector_Concat(Xadr,Yadr)) != NULL)
        {
            handle = newSViv((IV)address);
            reference = sv_bless(sv_2mortal(newRV(handle)),
                gv_stashpv(BitVector_Class,1));
            SvREFCNT_dec(handle);
            SvREADONLY_on(handle);
            PUSHs(reference);
        }
        else BIT_VECTOR_MEMORY_ERROR("Concat");
    }
    else BIT_VECTOR_OBJECT_ERROR("Concat");
}


void
BitVector_Concat_List(...)
PPCODE:
{
    BitVector_Object  Xref;
    BitVector_Handle  Xhdl;
    BitVector_Address Xadr;
    BitVector_Object  reference;
    BitVector_Handle  handle;
    BitVector_Address address;
    N_int offset;
    N_int bits;
    I32 index;

    bits = 0;
    index = items;
    while (index-- > 0)
    {
        Xref = ST(index);
        if ( BIT_VECTOR_OBJECT(Xref,Xhdl,Xadr) )
        {
            bits += bits_(Xadr);
        }
        else if ((index != 0) or SvROK(Xref))
          BIT_VECTOR_OBJECT_ERROR("Concat_List");
    }
    if ((address = BitVector_Create(bits,false)) != NULL)
    {
        offset = 0;
        index = items;
        while (index-- > 0)
        {
            Xref = ST(index);
            if ( BIT_VECTOR_OBJECT(Xref,Xhdl,Xadr) )
            {
                if ((bits = bits_(Xadr)) > 0)
                {
                    BitVector_Interval_Copy(address,Xadr,offset,0,bits);
                    offset += bits;
                }
            }
            else if ((index != 0) or SvROK(Xref))
              BIT_VECTOR_OBJECT_ERROR("Concat_List");
        }
        handle = newSViv((IV)address);
        reference = sv_bless(sv_2mortal(newRV(handle)),
            gv_stashpv(BitVector_Class,1));
        SvREFCNT_dec(handle);
        SvREADONLY_on(handle);
        PUSHs(reference);
    }
    else BIT_VECTOR_MEMORY_ERROR("Concat_List");
}


N_int
BitVector_Size(reference)
BitVector_Object	reference
CODE:
{
    BitVector_Handle  handle;
    BitVector_Address address;

    if ( BIT_VECTOR_OBJECT(reference,handle,address) )
    {
        RETVAL = bits_(address);
    }
    else BIT_VECTOR_OBJECT_ERROR("Size");
}
OUTPUT:
RETVAL


void
BitVector_Resize(reference,bits)
BitVector_Object	reference
BitVector_Scalar	bits
CODE:
{
    BitVector_Handle  handle;
    BitVector_Address address;
    N_int size;

    if ( BIT_VECTOR_OBJECT(reference,handle,address) )
    {
        if ( BIT_VECTOR_SCALAR(bits,N_int,size) )
        {
            address = BitVector_Resize(address,size);
            SvREADONLY_off(handle);
            sv_setiv(handle,(IV)address);
            SvREADONLY_on(handle);
            if (address == NULL) BIT_VECTOR_MEMORY_ERROR("Resize");
        }
        else BIT_VECTOR_SCALAR_ERROR("Resize");
    }
    else BIT_VECTOR_OBJECT_ERROR("Resize");
}


void
BitVector_DESTROY(reference)
BitVector_Object	reference
CODE:
{
    BitVector_Handle  handle;
    BitVector_Address address;

    if ( BIT_VECTOR_OBJECT(reference,handle,address) )
    {
        BitVector_Destroy(address);
        SvREADONLY_off(handle);
        sv_setiv(handle,(IV)NULL);
        SvREADONLY_on(handle);
    }
    else BIT_VECTOR_OBJECT_ERROR("DESTROY");
}


void
BitVector_Copy(Xref,Yref)
BitVector_Object        Xref
BitVector_Object        Yref
CODE:
{
    BitVector_Handle  Xhdl;
    BitVector_Address Xadr;
    BitVector_Handle  Yhdl;
    BitVector_Address Yadr;

    if ( BIT_VECTOR_OBJECT(Xref,Xhdl,Xadr) &&
         BIT_VECTOR_OBJECT(Yref,Yhdl,Yadr) )
    {
        if (bits_(Xadr) == bits_(Yadr))
        {
            BitVector_Copy(Xadr,Yadr);
        }
        else BIT_VECTOR_SIZE_ERROR("Copy");
    }
    else BIT_VECTOR_OBJECT_ERROR("Copy");
}


void
BitVector_Empty(reference)
BitVector_Object	reference
CODE:
{
    BitVector_Handle  handle;
    BitVector_Address address;

    if ( BIT_VECTOR_OBJECT(reference,handle,address) )
    {
        BitVector_Empty(address);
    }
    else BIT_VECTOR_OBJECT_ERROR("Empty");
}


void
BitVector_Fill(reference)
BitVector_Object	reference
CODE:
{
    BitVector_Handle  handle;
    BitVector_Address address;

    if ( BIT_VECTOR_OBJECT(reference,handle,address) )
    {
        BitVector_Fill(address);
    }
    else BIT_VECTOR_OBJECT_ERROR("Fill");
}


void
BitVector_Flip(reference)
BitVector_Object	reference
CODE:
{
    BitVector_Handle  handle;
    BitVector_Address address;

    if ( BIT_VECTOR_OBJECT(reference,handle,address) )
    {
        BitVector_Flip(address);
    }
    else BIT_VECTOR_OBJECT_ERROR("Flip");
}


void
BitVector_Primes(reference)
BitVector_Object	reference
CODE:
{
    BitVector_Handle  handle;
    BitVector_Address address;

    if ( BIT_VECTOR_OBJECT(reference,handle,address) )
    {
        BitVector_Primes(address);
    }
    else BIT_VECTOR_OBJECT_ERROR("Primes");
}


void
BitVector_Reverse(Xref,Yref)
BitVector_Object	Xref
BitVector_Object	Yref
CODE:
{
    BitVector_Handle  Xhdl;
    BitVector_Address Xadr;
    BitVector_Handle  Yhdl;
    BitVector_Address Yadr;

    if ( BIT_VECTOR_OBJECT(Xref,Xhdl,Xadr) &&
         BIT_VECTOR_OBJECT(Yref,Yhdl,Yadr) )
    {
        if (bits_(Xadr) == bits_(Yadr))
        {
            BitVector_Reverse(Xadr,Yadr);
        }
        else BIT_VECTOR_SIZE_ERROR("Reverse");
    }
    else BIT_VECTOR_OBJECT_ERROR("Reverse");
}


void
BitVector_Interval_Empty(reference,min,max)
BitVector_Object	reference
BitVector_Scalar	min
BitVector_Scalar	max
ALIAS:
  Empty_Interval = 1
CODE:
{
    BitVector_Handle  handle;
    BitVector_Address address;
    N_int lower;
    N_int upper;

    if ( BIT_VECTOR_OBJECT(reference,handle,address) )
    {
        if ( BIT_VECTOR_SCALAR(min,N_int,lower) &&
             BIT_VECTOR_SCALAR(max,N_int,upper) )
        {
            if      (lower >= bits_(address)) BIT_VECTOR_MIN_ERROR("Interval_Empty");
            else if (upper >= bits_(address)) BIT_VECTOR_MAX_ERROR("Interval_Empty");
            else if (lower > upper)         BIT_VECTOR_ORDER_ERROR("Interval_Empty");
            else                       BitVector_Interval_Empty(address,lower,upper);
        }
        else BIT_VECTOR_SCALAR_ERROR("Interval_Empty");
    }
    else BIT_VECTOR_OBJECT_ERROR("Interval_Empty");
}


void
BitVector_Interval_Fill(reference,min,max)
BitVector_Object	reference
BitVector_Scalar	min
BitVector_Scalar	max
ALIAS:
  Fill_Interval = 1
CODE:
{
    BitVector_Handle  handle;
    BitVector_Address address;
    N_int lower;
    N_int upper;

    if ( BIT_VECTOR_OBJECT(reference,handle,address) )
    {
        if ( BIT_VECTOR_SCALAR(min,N_int,lower) &&
             BIT_VECTOR_SCALAR(max,N_int,upper) )
        {
            if      (lower >= bits_(address)) BIT_VECTOR_MIN_ERROR("Interval_Fill");
            else if (upper >= bits_(address)) BIT_VECTOR_MAX_ERROR("Interval_Fill");
            else if (lower > upper)         BIT_VECTOR_ORDER_ERROR("Interval_Fill");
            else                       BitVector_Interval_Fill(address,lower,upper);
        }
        else BIT_VECTOR_SCALAR_ERROR("Interval_Fill");
    }
    else BIT_VECTOR_OBJECT_ERROR("Interval_Fill");
}


void
BitVector_Interval_Flip(reference,min,max)
BitVector_Object	reference
BitVector_Scalar	min
BitVector_Scalar	max
ALIAS:
  Flip_Interval = 1
CODE:
{
    BitVector_Handle  handle;
    BitVector_Address address;
    N_int lower;
    N_int upper;

    if ( BIT_VECTOR_OBJECT(reference,handle,address) )
    {
        if ( BIT_VECTOR_SCALAR(min,N_int,lower) &&
             BIT_VECTOR_SCALAR(max,N_int,upper) )
        {
            if      (lower >= bits_(address)) BIT_VECTOR_MIN_ERROR("Interval_Flip");
            else if (upper >= bits_(address)) BIT_VECTOR_MAX_ERROR("Interval_Flip");
            else if (lower > upper)         BIT_VECTOR_ORDER_ERROR("Interval_Flip");
            else                       BitVector_Interval_Flip(address,lower,upper);
        }
        else BIT_VECTOR_SCALAR_ERROR("Interval_Flip");
    }
    else BIT_VECTOR_OBJECT_ERROR("Interval_Flip");
}


void
BitVector_Interval_Reverse(reference,min,max)
BitVector_Object	reference
BitVector_Scalar	min
BitVector_Scalar	max
ALIAS:
  Reverse_Interval = 1
CODE:
{
    BitVector_Handle  handle;
    BitVector_Address address;
    N_int lower;
    N_int upper;

    if ( BIT_VECTOR_OBJECT(reference,handle,address) )
    {
        if ( BIT_VECTOR_SCALAR(min,N_int,lower) &&
             BIT_VECTOR_SCALAR(max,N_int,upper) )
        {
            if      (lower >= bits_(address)) BIT_VECTOR_MIN_ERROR("Interval_Reverse");
            else if (upper >= bits_(address)) BIT_VECTOR_MAX_ERROR("Interval_Reverse");
            else if (lower > upper)         BIT_VECTOR_ORDER_ERROR("Interval_Reverse");
            else                       BitVector_Interval_Reverse(address,lower,upper);
        }
        else BIT_VECTOR_SCALAR_ERROR("Interval_Reverse");
    }
    else BIT_VECTOR_OBJECT_ERROR("Interval_Reverse");
}


void
BitVector_Interval_Scan_inc(reference,start)
BitVector_Object	reference
BitVector_Scalar	start
PPCODE:
{
    BitVector_Handle  handle;
    BitVector_Address address;
    N_int first;
    N_int min;
    N_int max;

    if ( BIT_VECTOR_OBJECT(reference,handle,address) )
    {
        if ( BIT_VECTOR_SCALAR(start,N_int,first) )
        {
            if (first < bits_(address))
            {
                if ( BitVector_interval_scan_inc(address,first,&min,&max) )
                {
                    EXTEND(sp,2);
                    PUSHs(sv_2mortal(newSViv((IV)min)));
                    PUSHs(sv_2mortal(newSViv((IV)max)));
                }
                /* else return empty list */
            }
            else BIT_VECTOR_START_ERROR("Interval_Scan_inc");
        }
        else BIT_VECTOR_SCALAR_ERROR("Interval_Scan_inc");
    }
    else BIT_VECTOR_OBJECT_ERROR("Interval_Scan_inc");
}


void
BitVector_Interval_Scan_dec(reference,start)
BitVector_Object	reference
BitVector_Scalar	start
PPCODE:
{
    BitVector_Handle  handle;
    BitVector_Address address;
    N_int first;
    N_int min;
    N_int max;

    if ( BIT_VECTOR_OBJECT(reference,handle,address) )
    {
        if ( BIT_VECTOR_SCALAR(start,N_int,first) )
        {
            if (first < bits_(address))
            {
                if ( BitVector_interval_scan_dec(address,first,&min,&max) )
                {
                    EXTEND(sp,2);
                    PUSHs(sv_2mortal(newSViv((IV)min)));
                    PUSHs(sv_2mortal(newSViv((IV)max)));
                }
                /* else return empty list */
            }
            else BIT_VECTOR_START_ERROR("Interval_Scan_dec");
        }
        else BIT_VECTOR_SCALAR_ERROR("Interval_Scan_dec");
    }
    else BIT_VECTOR_OBJECT_ERROR("Interval_Scan_dec");
}


void
BitVector_Interval_Copy(Xref,Yref,Xoffset,Yoffset,length)
BitVector_Object	Xref
BitVector_Object	Yref
BitVector_Scalar	Xoffset
BitVector_Scalar	Yoffset
BitVector_Scalar	length
CODE:
{
    BitVector_Handle  Xhdl;
    BitVector_Address Xadr;
    BitVector_Handle  Yhdl;
    BitVector_Address Yadr;
    N_int Xoff;
    N_int Yoff;
    N_int len;

    if ( BIT_VECTOR_OBJECT(Xref,Xhdl,Xadr) &&
         BIT_VECTOR_OBJECT(Yref,Yhdl,Yadr) )
    {
        if ( BIT_VECTOR_SCALAR(Xoffset,N_int,Xoff) &&
             BIT_VECTOR_SCALAR(Yoffset,N_int,Yoff) &&
             BIT_VECTOR_SCALAR(length, N_int,len) )
        {
            if ((Xoff < bits_(Xadr)) and (Yoff < bits_(Yadr)))
            {
                if (len > 0) BitVector_Interval_Copy(Xadr,Yadr,Xoff,Yoff,len);
            }
            else BIT_VECTOR_OFFSET_ERROR("Interval_Copy");
        }
        else BIT_VECTOR_SCALAR_ERROR("Interval_Copy");
    }
    else BIT_VECTOR_OBJECT_ERROR("Interval_Copy");
}


void
BitVector_Interval_Substitute(Xref,Yref,Xoffset,Xlength,Yoffset,Ylength)
BitVector_Object	Xref
BitVector_Object	Yref
BitVector_Scalar	Xoffset
BitVector_Scalar	Xlength
BitVector_Scalar	Yoffset
BitVector_Scalar	Ylength
CODE:
{
    BitVector_Handle  Xhdl;
    BitVector_Address Xadr;
    BitVector_Handle  Yhdl;
    BitVector_Address Yadr;
    N_int Xoff;
    N_int Xlen;
    N_int Yoff;
    N_int Ylen;

    if ( BIT_VECTOR_OBJECT(Xref,Xhdl,Xadr) &&
         BIT_VECTOR_OBJECT(Yref,Yhdl,Yadr) )
    {
        if ( BIT_VECTOR_SCALAR(Xoffset,N_int,Xoff) &&
             BIT_VECTOR_SCALAR(Xlength,N_int,Xlen) &&
             BIT_VECTOR_SCALAR(Yoffset,N_int,Yoff) &&
             BIT_VECTOR_SCALAR(Ylength,N_int,Ylen) )
        {
            if ((Xoff <= bits_(Xadr)) and (Yoff <= bits_(Yadr)))
            {
                Xadr = BitVector_Interval_Substitute(Xadr,Yadr,Xoff,Xlen,Yoff,Ylen);
                SvREADONLY_off(Xhdl);
                sv_setiv(Xhdl,(IV)Xadr);
                SvREADONLY_on(Xhdl);
                if (Xadr == NULL) BIT_VECTOR_MEMORY_ERROR("Interval_Substitute");
            }
            else BIT_VECTOR_OFFSET_ERROR("Interval_Substitute");
        }
        else BIT_VECTOR_SCALAR_ERROR("Interval_Substitute");
    }
    else BIT_VECTOR_OBJECT_ERROR("Interval_Substitute");
}


boolean
BitVector_is_empty(reference)
BitVector_Object	reference
CODE:
{
    BitVector_Handle  handle;
    BitVector_Address address;

    if ( BIT_VECTOR_OBJECT(reference,handle,address) )
    {
        RETVAL = BitVector_is_empty(address);
    }
    else BIT_VECTOR_OBJECT_ERROR("is_empty");
}
OUTPUT:
RETVAL


boolean
BitVector_is_full(reference)
BitVector_Object	reference
CODE:
{
    BitVector_Handle  handle;
    BitVector_Address address;

    if ( BIT_VECTOR_OBJECT(reference,handle,address) )
    {
        RETVAL = BitVector_is_full(address);
    }
    else BIT_VECTOR_OBJECT_ERROR("is_full");
}
OUTPUT:
RETVAL


boolean
BitVector_equal(Xref,Yref)
BitVector_Object	Xref
BitVector_Object	Yref
CODE:
{
    BitVector_Handle  Xhdl;
    BitVector_Address Xadr;
    BitVector_Handle  Yhdl;
    BitVector_Address Yadr;

    if ( BIT_VECTOR_OBJECT(Xref,Xhdl,Xadr) &&
         BIT_VECTOR_OBJECT(Yref,Yhdl,Yadr) )
    {
        if (bits_(Xadr) == bits_(Yadr))
        {
            RETVAL = BitVector_equal(Xadr,Yadr);
        }
        else BIT_VECTOR_SIZE_ERROR("equal");
    }
    else BIT_VECTOR_OBJECT_ERROR("equal");
}
OUTPUT:
RETVAL


Z_int
BitVector_Lexicompare(Xref,Yref)
BitVector_Object	Xref
BitVector_Object	Yref
CODE:
{
    BitVector_Handle  Xhdl;
    BitVector_Address Xadr;
    BitVector_Handle  Yhdl;
    BitVector_Address Yadr;

    if ( BIT_VECTOR_OBJECT(Xref,Xhdl,Xadr) &&
         BIT_VECTOR_OBJECT(Yref,Yhdl,Yadr) )
    {
        if (bits_(Xadr) == bits_(Yadr))
        {
            RETVAL = BitVector_Lexicompare(Xadr,Yadr);
        }
        else BIT_VECTOR_SIZE_ERROR("Lexicompare");
    }
    else BIT_VECTOR_OBJECT_ERROR("Lexicompare");
}
OUTPUT:
RETVAL


Z_int
BitVector_Compare(Xref,Yref)
BitVector_Object	Xref
BitVector_Object	Yref
CODE:
{
    BitVector_Handle  Xhdl;
    BitVector_Address Xadr;
    BitVector_Handle  Yhdl;
    BitVector_Address Yadr;

    if ( BIT_VECTOR_OBJECT(Xref,Xhdl,Xadr) &&
         BIT_VECTOR_OBJECT(Yref,Yhdl,Yadr) )
    {
        if (bits_(Xadr) == bits_(Yadr))
        {
            RETVAL = BitVector_Compare(Xadr,Yadr);
        }
        else BIT_VECTOR_SIZE_ERROR("Compare");
    }
    else BIT_VECTOR_OBJECT_ERROR("Compare");
}
OUTPUT:
RETVAL


void
BitVector_to_Hex(reference)
BitVector_Object	reference
ALIAS:
  to_String = 1
PPCODE:
{
    BitVector_Handle  handle;
    BitVector_Address address;
    charptr string;

    if ( BIT_VECTOR_OBJECT(reference,handle,address) )
    {
        string = BitVector_to_Hex(address);
        if (string != NULL)
        {
            EXTEND(sp,1);
            PUSHs(sv_2mortal(newSVpv((char *)string,0)));
            BitVector_Dispose(string);
        }
        else BIT_VECTOR_MEMORY_ERROR("to_Hex");
    }
    else BIT_VECTOR_OBJECT_ERROR("to_Hex");
}


void
BitVector_from_Hex(reference,string)
BitVector_Object	reference
BitVector_Scalar	string
ALIAS:
  from_string = 1
CODE:
{
    BitVector_Handle  handle;
    BitVector_Address address;
    charptr pointer;
    ErrCode code;

    if ( BIT_VECTOR_OBJECT(reference,handle,address) )
    {
        if ( BIT_VECTOR_STRING(string,pointer) )
        {
            if (code = BitVector_from_Hex(address,pointer))
            {
                BIT_VECTOR_EXCEPTION(code,"from_Hex");
            }
        }
        else BIT_VECTOR_STRING_ERROR("from_Hex");
    }
    else BIT_VECTOR_OBJECT_ERROR("from_Hex");
}


void
BitVector_to_Bin(reference)
BitVector_Object	reference
PPCODE:
{
    BitVector_Handle  handle;
    BitVector_Address address;
    charptr string;

    if ( BIT_VECTOR_OBJECT(reference,handle,address) )
    {
        string = BitVector_to_Bin(address);
        if (string != NULL)
        {
            EXTEND(sp,1);
            PUSHs(sv_2mortal(newSVpv((char *)string,0)));
            BitVector_Dispose(string);
        }
        else BIT_VECTOR_MEMORY_ERROR("to_Bin");
    }
    else BIT_VECTOR_OBJECT_ERROR("to_Bin");
}


void
BitVector_from_Bin(reference,string)
BitVector_Object	reference
BitVector_Scalar	string
CODE:
{
    BitVector_Handle  handle;
    BitVector_Address address;
    charptr pointer;
    ErrCode code;

    if ( BIT_VECTOR_OBJECT(reference,handle,address) )
    {
        if ( BIT_VECTOR_STRING(string,pointer) )
        {
            if (code = BitVector_from_Bin(address,pointer))
            {
                BIT_VECTOR_EXCEPTION(code,"from_Bin");
            }
        }
        else BIT_VECTOR_STRING_ERROR("from_Bin");
    }
    else BIT_VECTOR_OBJECT_ERROR("from_Bin");
}


void
BitVector_to_Dec(reference)
BitVector_Object	reference
PPCODE:
{
    BitVector_Handle  handle;
    BitVector_Address address;
    charptr string;

    if ( BIT_VECTOR_OBJECT(reference,handle,address) )
    {
        string = BitVector_to_Dec(address);
        if (string != NULL)
        {
            EXTEND(sp,1);
            PUSHs(sv_2mortal(newSVpv((char *)string,0)));
            BitVector_Dispose(string);
        }
        else BIT_VECTOR_MEMORY_ERROR("to_Dec");
    }
    else BIT_VECTOR_OBJECT_ERROR("to_Dec");
}


void
BitVector_from_Dec(reference,string)
BitVector_Object	reference
BitVector_Scalar	string
CODE:
{
    BitVector_Handle  handle;
    BitVector_Address address;
    charptr pointer;
    ErrCode code;

    if ( BIT_VECTOR_OBJECT(reference,handle,address) )
    {
        if ( BIT_VECTOR_STRING(string,pointer) )
        {
            if (code = BitVector_from_Dec(address,pointer))
            {
                BIT_VECTOR_EXCEPTION(code,"from_Dec");
            }
        }
        else BIT_VECTOR_STRING_ERROR("from_Dec");
    }
    else BIT_VECTOR_OBJECT_ERROR("from_Dec");
}


void
BitVector_to_Enum(reference)
BitVector_Object	reference
ALIAS:
  to_ASCII = 1
PPCODE:
{
    BitVector_Handle  handle;
    BitVector_Address address;
    charptr string;

    if ( BIT_VECTOR_OBJECT(reference,handle,address) )
    {
        string = BitVector_to_Enum(address);
        if (string != NULL)
        {
            EXTEND(sp,1);
            PUSHs(sv_2mortal(newSVpv((char *)string,0)));
            BitVector_Dispose(string);
        }
        else BIT_VECTOR_MEMORY_ERROR("to_Enum");
    }
    else BIT_VECTOR_OBJECT_ERROR("to_Enum");
}


void
BitVector_from_Enum(reference,string)
BitVector_Object	reference
BitVector_Scalar	string
ALIAS:
  from_ASCII = 1
CODE:
{
    BitVector_Handle  handle;
    BitVector_Address address;
    charptr pointer;
    ErrCode code;

    if ( BIT_VECTOR_OBJECT(reference,handle,address) )
    {
        if ( BIT_VECTOR_STRING(string,pointer) )
        {
            if (code = BitVector_from_Enum(address,pointer))
            {
                BIT_VECTOR_EXCEPTION(code,"from_Enum");
            }
        }
        else BIT_VECTOR_STRING_ERROR("from_Enum");
    }
    else BIT_VECTOR_OBJECT_ERROR("from_Enum");
}


void
BitVector_Bit_Off(reference,index)
BitVector_Object	reference
BitVector_Scalar	index
CODE:
{
    BitVector_Handle  handle;
    BitVector_Address address;
    N_int idx;

    if ( BIT_VECTOR_OBJECT(reference,handle,address) )
    {
        if ( BIT_VECTOR_SCALAR(index,N_int,idx) )
        {
            if (idx < bits_(address))
            {
                BitVector_Bit_Off(address,idx);
            }
            else BIT_VECTOR_INDEX_ERROR("Bit_Off");
        }
        else BIT_VECTOR_SCALAR_ERROR("Bit_Off");
    }
    else BIT_VECTOR_OBJECT_ERROR("Bit_Off");
}


void
BitVector_Bit_On(reference,index)
BitVector_Object	reference
BitVector_Scalar	index
CODE:
{
    BitVector_Handle  handle;
    BitVector_Address address;
    N_int idx;

    if ( BIT_VECTOR_OBJECT(reference,handle,address) )
    {
        if ( BIT_VECTOR_SCALAR(index,N_int,idx) )
        {
            if (idx < bits_(address))
            {
                BitVector_Bit_On(address,idx);
            }
            else BIT_VECTOR_INDEX_ERROR("Bit_On");
        }
        else BIT_VECTOR_SCALAR_ERROR("Bit_On");
    }
    else BIT_VECTOR_OBJECT_ERROR("Bit_On");
}


boolean
BitVector_bit_flip(reference,index)
BitVector_Object	reference
BitVector_Scalar	index
ALIAS:
  flip = 1
CODE:
{
    BitVector_Handle  handle;
    BitVector_Address address;
    N_int idx;

    if ( BIT_VECTOR_OBJECT(reference,handle,address) )
    {
        if ( BIT_VECTOR_SCALAR(index,N_int,idx) )
        {
            if (idx < bits_(address))
            {
                RETVAL = BitVector_bit_flip(address,idx);
            }
            else BIT_VECTOR_INDEX_ERROR("bit_flip");
        }
        else BIT_VECTOR_SCALAR_ERROR("bit_flip");
    }
    else BIT_VECTOR_OBJECT_ERROR("bit_flip");
}
OUTPUT:
RETVAL


boolean
BitVector_bit_test(reference,index)
BitVector_Object	reference
BitVector_Scalar	index
ALIAS:
  in = 1
  contains = 2
CODE:
{
    BitVector_Handle  handle;
    BitVector_Address address;
    N_int idx;

    if ( BIT_VECTOR_OBJECT(reference,handle,address) )
    {
        if ( BIT_VECTOR_SCALAR(index,N_int,idx) )
        {
            if (idx < bits_(address))
            {
                RETVAL = BitVector_bit_test(address,idx);
            }
            else BIT_VECTOR_INDEX_ERROR("bit_test");
        }
        else BIT_VECTOR_SCALAR_ERROR("bit_test");
    }
    else BIT_VECTOR_OBJECT_ERROR("bit_test");
}
OUTPUT:
RETVAL


void
BitVector_Bit_Copy(reference,index,bit)
BitVector_Object	reference
BitVector_Scalar	index
BitVector_Scalar	bit
CODE:
{
    BitVector_Handle  handle;
    BitVector_Address address;
    N_int idx;
    boolean b;

    if ( BIT_VECTOR_OBJECT(reference,handle,address) )
    {
        if ( BIT_VECTOR_SCALAR(index,N_int,idx) &&
             BIT_VECTOR_SCALAR(bit,boolean,b) )
        {
            if (idx < bits_(address))
            {
                BitVector_Bit_Copy(address,idx,b);
            }
            else BIT_VECTOR_INDEX_ERROR("Bit_Copy");
        }
        else BIT_VECTOR_SCALAR_ERROR("Bit_Copy");
    }
    else BIT_VECTOR_OBJECT_ERROR("Bit_Copy");
}


void
BitVector_LSB(reference,bit)
BitVector_Object	reference
BitVector_Scalar	bit
CODE:
{
    BitVector_Handle  handle;
    BitVector_Address address;
    boolean b;

    if ( BIT_VECTOR_OBJECT(reference,handle,address) )
    {
        if ( BIT_VECTOR_SCALAR(bit,boolean,b) )
        {
            BitVector_LSB(address,b);
        }
        else BIT_VECTOR_SCALAR_ERROR("LSB");
    }
    else BIT_VECTOR_OBJECT_ERROR("LSB");
}


void
BitVector_MSB(reference,bit)
BitVector_Object	reference
BitVector_Scalar	bit
CODE:
{
    BitVector_Handle  handle;
    BitVector_Address address;
    boolean b;

    if ( BIT_VECTOR_OBJECT(reference,handle,address) )
    {
        if ( BIT_VECTOR_SCALAR(bit,boolean,b) )
        {
            BitVector_MSB(address,b);
        }
        else BIT_VECTOR_SCALAR_ERROR("MSB");
    }
    else BIT_VECTOR_OBJECT_ERROR("MSB");
}


boolean
BitVector_lsb(reference)
BitVector_Object	reference
CODE:
{
    BitVector_Handle  handle;
    BitVector_Address address;

    if ( BIT_VECTOR_OBJECT(reference,handle,address) )
    {
        RETVAL = BitVector_lsb(address);
    }
    else BIT_VECTOR_OBJECT_ERROR("lsb");
}
OUTPUT:
RETVAL


boolean
BitVector_msb(reference)
BitVector_Object	reference
CODE:
{
    BitVector_Handle  handle;
    BitVector_Address address;

    if ( BIT_VECTOR_OBJECT(reference,handle,address) )
    {
        RETVAL = BitVector_msb(address);
    }
    else BIT_VECTOR_OBJECT_ERROR("msb");
}
OUTPUT:
RETVAL


boolean
BitVector_rotate_left(reference)
BitVector_Object	reference
CODE:
{
    BitVector_Handle  handle;
    BitVector_Address address;

    if ( BIT_VECTOR_OBJECT(reference,handle,address) )
    {
        RETVAL = BitVector_rotate_left(address);
    }
    else BIT_VECTOR_OBJECT_ERROR("rotate_left");
}
OUTPUT:
RETVAL


boolean
BitVector_rotate_right(reference)
BitVector_Object	reference
CODE:
{
    BitVector_Handle  handle;
    BitVector_Address address;

    if ( BIT_VECTOR_OBJECT(reference,handle,address) )
    {
        RETVAL = BitVector_rotate_right(address);
    }
    else BIT_VECTOR_OBJECT_ERROR("rotate_right");
}
OUTPUT:
RETVAL


boolean
BitVector_shift_left(reference,carry)
BitVector_Object	reference
BitVector_Scalar	carry
CODE:
{
    BitVector_Handle  handle;
    BitVector_Address address;
    boolean c;

    if ( BIT_VECTOR_OBJECT(reference,handle,address) )
    {
        if ( BIT_VECTOR_SCALAR(carry,boolean,c) )
        {
            RETVAL = BitVector_shift_left(address,c);
        }
        else BIT_VECTOR_SCALAR_ERROR("shift_left");
    }
    else BIT_VECTOR_OBJECT_ERROR("shift_left");
}
OUTPUT:
RETVAL


boolean
BitVector_shift_right(reference,carry)
BitVector_Object	reference
BitVector_Scalar	carry
CODE:
{
    BitVector_Handle  handle;
    BitVector_Address address;
    boolean c;

    if ( BIT_VECTOR_OBJECT(reference,handle,address) )
    {
        if ( BIT_VECTOR_SCALAR(carry,boolean,c) )
        {
            RETVAL = BitVector_shift_right(address,c);
        }
        else BIT_VECTOR_SCALAR_ERROR("shift_right");
    }
    else BIT_VECTOR_OBJECT_ERROR("shift_right");
}
OUTPUT:
RETVAL


void
BitVector_Move_Left(reference,bits)
BitVector_Object	reference
BitVector_Scalar	bits
CODE:
{
    BitVector_Handle  handle;
    BitVector_Address address;
    N_int cnt;

    if ( BIT_VECTOR_OBJECT(reference,handle,address) )
    {
        if ( BIT_VECTOR_SCALAR(bits,N_int,cnt) )
        {
            BitVector_Move_Left(address,cnt);
        }
        else BIT_VECTOR_SCALAR_ERROR("Move_Left");
    }
    else BIT_VECTOR_OBJECT_ERROR("Move_Left");
}


void
BitVector_Move_Right(reference,bits)
BitVector_Object	reference
BitVector_Scalar	bits
CODE:
{
    BitVector_Handle  handle;
    BitVector_Address address;
    N_int cnt;

    if ( BIT_VECTOR_OBJECT(reference,handle,address) )
    {
        if ( BIT_VECTOR_SCALAR(bits,N_int,cnt) )
        {
            BitVector_Move_Right(address,cnt);
        }
        else BIT_VECTOR_SCALAR_ERROR("Move_Right");
    }
    else BIT_VECTOR_OBJECT_ERROR("Move_Right");
}


void
BitVector_Insert(reference,offset,count)
BitVector_Object	reference
BitVector_Scalar	offset
BitVector_Scalar	count
CODE:
{
    BitVector_Handle  handle;
    BitVector_Address address;
    N_int off;
    N_int cnt;

    if ( BIT_VECTOR_OBJECT(reference,handle,address) )
    {
        if ( BIT_VECTOR_SCALAR(offset,N_int,off) &&
             BIT_VECTOR_SCALAR(count,N_int,cnt) )
        {
            if (off < bits_(address))
            {
                BitVector_Insert(address,off,cnt,true);
            }
            else BIT_VECTOR_OFFSET_ERROR("Insert");
        }
        else BIT_VECTOR_SCALAR_ERROR("Insert");
    }
    else BIT_VECTOR_OBJECT_ERROR("Insert");
}


void
BitVector_Delete(reference,offset,count)
BitVector_Object	reference
BitVector_Scalar	offset
BitVector_Scalar	count
CODE:
{
    BitVector_Handle  handle;
    BitVector_Address address;
    N_int off;
    N_int cnt;

    if ( BIT_VECTOR_OBJECT(reference,handle,address) )
    {
        if ( BIT_VECTOR_SCALAR(offset,N_int,off) &&
             BIT_VECTOR_SCALAR(count,N_int,cnt) )
        {
            if (off < bits_(address))
            {
                BitVector_Delete(address,off,cnt,true);
            }
            else BIT_VECTOR_OFFSET_ERROR("Delete");
        }
        else BIT_VECTOR_SCALAR_ERROR("Delete");
    }
    else BIT_VECTOR_OBJECT_ERROR("Delete");
}


boolean
BitVector_increment(reference)
BitVector_Object	reference
CODE:
{
    BitVector_Handle  handle;
    BitVector_Address address;

    if ( BIT_VECTOR_OBJECT(reference,handle,address) )
    {
        RETVAL = BitVector_increment(address);
    }
    else BIT_VECTOR_OBJECT_ERROR("increment");
}
OUTPUT:
RETVAL


boolean
BitVector_decrement(reference)
BitVector_Object	reference
CODE:
{
    BitVector_Handle  handle;
    BitVector_Address address;

    if ( BIT_VECTOR_OBJECT(reference,handle,address) )
    {
        RETVAL = BitVector_decrement(address);
    }
    else BIT_VECTOR_OBJECT_ERROR("decrement");
}
OUTPUT:
RETVAL


boolean
BitVector_add(Xref,Yref,Zref,carry)
BitVector_Object	Xref
BitVector_Object	Yref
BitVector_Object	Zref
BitVector_Scalar	carry
CODE:
{
    BitVector_Handle  Xhdl;
    BitVector_Address Xadr;
    BitVector_Handle  Yhdl;
    BitVector_Address Yadr;
    BitVector_Handle  Zhdl;
    BitVector_Address Zadr;
    boolean c;

    if ( BIT_VECTOR_OBJECT(Xref,Xhdl,Xadr) &&
         BIT_VECTOR_OBJECT(Yref,Yhdl,Yadr) &&
         BIT_VECTOR_OBJECT(Zref,Zhdl,Zadr) )
    {
        if ( BIT_VECTOR_SCALAR(carry,boolean,c) )
        {
            if ((bits_(Xadr) == bits_(Yadr)) and (bits_(Xadr) == bits_(Zadr)))
            {
                RETVAL = BitVector_add(Xadr,Yadr,Zadr,c);
            }
            else BIT_VECTOR_SIZE_ERROR("add");
        }
        else BIT_VECTOR_SCALAR_ERROR("add");
    }
    else BIT_VECTOR_OBJECT_ERROR("add");
}
OUTPUT:
RETVAL


boolean
BitVector_subtract(Xref,Yref,Zref,carry)
BitVector_Object	Xref
BitVector_Object	Yref
BitVector_Object	Zref
BitVector_Scalar	carry
CODE:
{
    BitVector_Handle  Xhdl;
    BitVector_Address Xadr;
    BitVector_Handle  Yhdl;
    BitVector_Address Yadr;
    BitVector_Handle  Zhdl;
    BitVector_Address Zadr;
    boolean c;

    if ( BIT_VECTOR_OBJECT(Xref,Xhdl,Xadr) &&
         BIT_VECTOR_OBJECT(Yref,Yhdl,Yadr) &&
         BIT_VECTOR_OBJECT(Zref,Zhdl,Zadr) )
    {
        if ( BIT_VECTOR_SCALAR(carry,boolean,c) )
        {
            if ((bits_(Xadr) == bits_(Yadr)) and (bits_(Xadr) == bits_(Zadr)))
            {
                RETVAL = BitVector_subtract(Xadr,Yadr,Zadr,c);
            }
            else BIT_VECTOR_SIZE_ERROR("subtract");
        }
        else BIT_VECTOR_SCALAR_ERROR("subtract");
    }
    else BIT_VECTOR_OBJECT_ERROR("subtract");
}
OUTPUT:
RETVAL


void
BitVector_Negate(Xref,Yref)
BitVector_Object	Xref
BitVector_Object	Yref
ALIAS:
  Neg = 2
CODE:
{
    BitVector_Handle  Xhdl;
    BitVector_Address Xadr;
    BitVector_Handle  Yhdl;
    BitVector_Address Yadr;

    if ( BIT_VECTOR_OBJECT(Xref,Xhdl,Xadr) &&
         BIT_VECTOR_OBJECT(Yref,Yhdl,Yadr) )
    {
        if (bits_(Xadr) == bits_(Yadr))
        {
            BitVector_Negate(Xadr,Yadr);
        }
        else BIT_VECTOR_SIZE_ERROR("Negate");
    }
    else BIT_VECTOR_OBJECT_ERROR("Negate");
}


void
BitVector_Absolute(Xref,Yref)
BitVector_Object	Xref
BitVector_Object	Yref
CODE:
{
    BitVector_Handle  Xhdl;
    BitVector_Address Xadr;
    BitVector_Handle  Yhdl;
    BitVector_Address Yadr;

    if ( BIT_VECTOR_OBJECT(Xref,Xhdl,Xadr) &&
         BIT_VECTOR_OBJECT(Yref,Yhdl,Yadr) )
    {
        if (bits_(Xadr) == bits_(Yadr))
        {
            BitVector_Absolute(Xadr,Yadr);
        }
        else BIT_VECTOR_SIZE_ERROR("Absolute");
    }
    else BIT_VECTOR_OBJECT_ERROR("Absolute");
}


Z_int
BitVector_Sign(reference)
BitVector_Object	reference
CODE:
{
    BitVector_Handle  handle;
    BitVector_Address address;

    if ( BIT_VECTOR_OBJECT(reference,handle,address) )
    {
        RETVAL = BitVector_Sign(address);
    }
    else BIT_VECTOR_OBJECT_ERROR("Sign");
}
OUTPUT:
RETVAL


void
BitVector_Multiply(Xref,Yref,Zref)
BitVector_Object	Xref
BitVector_Object	Yref
BitVector_Object	Zref
CODE:
{
    BitVector_Handle  Xhdl;
    BitVector_Address Xadr;
    BitVector_Handle  Yhdl;
    BitVector_Address Yadr;
    BitVector_Handle  Zhdl;
    BitVector_Address Zadr;
    ErrCode           code;

    if ( BIT_VECTOR_OBJECT(Xref,Xhdl,Xadr) &&
         BIT_VECTOR_OBJECT(Yref,Yhdl,Yadr) &&
         BIT_VECTOR_OBJECT(Zref,Zhdl,Zadr) )
    {
        if ((bits_(Xadr) >= bits_(Yadr)) and (bits_(Yadr) == bits_(Zadr)))
        {
            if (code = BitVector_Multiply(Xadr,Yadr,Zadr))
                BIT_VECTOR_EXCEPTION(code,"Multiply");
        }
        else BIT_VECTOR_SIZE_ERROR("Multiply");
    }
    else BIT_VECTOR_OBJECT_ERROR("Multiply");
}


void
BitVector_Divide(Qref,Xref,Yref,Rref)
BitVector_Object	Qref
BitVector_Object	Xref
BitVector_Object	Yref
BitVector_Object	Rref
CODE:
{
    BitVector_Handle  Qhdl;
    BitVector_Address Qadr;
    BitVector_Handle  Xhdl;
    BitVector_Address Xadr;
    BitVector_Handle  Yhdl;
    BitVector_Address Yadr;
    BitVector_Handle  Rhdl;
    BitVector_Address Radr;
    ErrCode           code;

    if ( BIT_VECTOR_OBJECT(Qref,Qhdl,Qadr) &&
         BIT_VECTOR_OBJECT(Xref,Xhdl,Xadr) &&
         BIT_VECTOR_OBJECT(Yref,Yhdl,Yadr) &&
         BIT_VECTOR_OBJECT(Rref,Rhdl,Radr) )
    {
        if ((bits_(Qadr) == bits_(Xadr)) and
            (bits_(Qadr) == bits_(Yadr)) and (bits_(Qadr) == bits_(Radr)))
        {
            if (Qadr != Radr)
            {
                if (not BitVector_is_empty(Yadr))
                {
                    if (code = BitVector_Divide(Qadr,Xadr,Yadr,Radr))
                        BIT_VECTOR_EXCEPTION(code,"Divide");
                }
                else BIT_VECTOR_ZERO_ERROR("Divide");
            }
            else BIT_VECTOR_DISTINCT_ERROR("Divide");
        }
        else BIT_VECTOR_SIZE_ERROR("Divide");
    }
    else BIT_VECTOR_OBJECT_ERROR("Divide");
}


void
BitVector_GCD(Xref,Yref,Zref)
BitVector_Object	Xref
BitVector_Object	Yref
BitVector_Object	Zref
CODE:
{
    BitVector_Handle  Xhdl;
    BitVector_Address Xadr;
    BitVector_Handle  Yhdl;
    BitVector_Address Yadr;
    BitVector_Handle  Zhdl;
    BitVector_Address Zadr;
    ErrCode           code;

    if ( BIT_VECTOR_OBJECT(Xref,Xhdl,Xadr) &&
         BIT_VECTOR_OBJECT(Yref,Yhdl,Yadr) &&
         BIT_VECTOR_OBJECT(Zref,Zhdl,Zadr) )
    {
        if ((bits_(Xadr) == bits_(Yadr)) and (bits_(Xadr) == bits_(Zadr)))
        {
            if ((not BitVector_is_empty(Yadr)) and
                (not BitVector_is_empty(Zadr)))
            {
                if (code = BitVector_GCD(Xadr,Yadr,Zadr))
                    BIT_VECTOR_EXCEPTION(code,"GCD");
            }
            else BIT_VECTOR_ZERO_ERROR("GCD");
        }
        else BIT_VECTOR_SIZE_ERROR("GCD");
    }
    else BIT_VECTOR_OBJECT_ERROR("GCD");
}


void
BitVector_Block_Store(reference,buffer)
BitVector_Object	reference
BitVector_Scalar	buffer
CODE:
{
    BitVector_Handle  handle;
    BitVector_Address address;
    charptr string;
    N_int length;

    if ( BIT_VECTOR_OBJECT(reference,handle,address) )
    {
        if ( BIT_VECTOR_BUFFER(buffer,string,length) )
        {
            BitVector_Block_Store(address,string,length);
        }
        else BIT_VECTOR_STRING_ERROR("Block_Store");
    }
    else BIT_VECTOR_OBJECT_ERROR("Block_Store");
}


void
BitVector_Block_Read(reference)
BitVector_Object	reference
PPCODE:
{
    BitVector_Handle  handle;
    BitVector_Address address;
    charptr string;
    N_int length;

    if ( BIT_VECTOR_OBJECT(reference,handle,address) )
    {
        string = BitVector_Block_Read(address,&length);
        if (string != NULL)
        {
            EXTEND(sp,1);
            PUSHs(sv_2mortal(newSVpv((char *)string,(int)length)));
            BitVector_Dispose(string);
        }
        else BIT_VECTOR_MEMORY_ERROR("Block_Read");
    }
    else BIT_VECTOR_OBJECT_ERROR("Block_Read");
}


N_int
BitVector_Word_Size(reference)
BitVector_Object	reference
CODE:
{
    BitVector_Handle  handle;
    BitVector_Address address;

    if ( BIT_VECTOR_OBJECT(reference,handle,address) )
    {
        RETVAL = size_(address);
    }
    else BIT_VECTOR_OBJECT_ERROR("Word_Size");
}
OUTPUT:
RETVAL


void
BitVector_Word_Store(reference,offset,value)
BitVector_Object	reference
BitVector_Scalar	offset
BitVector_Scalar	value
CODE:
{
    BitVector_Handle  handle;
    BitVector_Address address;
    N_int off;
    N_int val;

    if ( BIT_VECTOR_OBJECT(reference,handle,address) )
    {
        if ( BIT_VECTOR_SCALAR(offset,N_int,off) &&
             BIT_VECTOR_SCALAR(value,N_int,val) )
        {
            if (off < size_(address))
            {
                BitVector_Word_Store(address,off,val);
            }
            else BIT_VECTOR_OFFSET_ERROR("Word_Store");
        }
        else BIT_VECTOR_SCALAR_ERROR("Word_Store");
    }
    else BIT_VECTOR_OBJECT_ERROR("Word_Store");
}


N_int
BitVector_Word_Read(reference,offset)
BitVector_Object	reference
BitVector_Scalar	offset
CODE:
{
    BitVector_Handle  handle;
    BitVector_Address address;
    N_int off;

    if ( BIT_VECTOR_OBJECT(reference,handle,address) )
    {
        if ( BIT_VECTOR_SCALAR(offset,N_int,off) )
        {
            if (off < size_(address))
            {
                RETVAL = BitVector_Word_Read(address,off);
            }
            else BIT_VECTOR_OFFSET_ERROR("Word_Read");
        }
        else BIT_VECTOR_SCALAR_ERROR("Word_Read");
    }
    else BIT_VECTOR_OBJECT_ERROR("Word_Read");
}
OUTPUT:
RETVAL


void
BitVector_Word_List_Store(reference,...)
BitVector_Object	reference
CODE:
{
    BitVector_Handle  handle;
    BitVector_Address address;
    BitVector_Scalar  scalar;
    N_int offset;
    N_int value;
    N_int size;
    I32 index;

    if ( BIT_VECTOR_OBJECT(reference,handle,address) )
    {
        size = size_(address);
        for ( offset = 0, index = 1;
            ((offset < size) and (index < items)); offset++, index++ )
        {
            scalar = ST(index);
            if ( BIT_VECTOR_SCALAR(scalar,N_int,value) )
            {
                BitVector_Word_Store(address,offset,value);
            }
            else BIT_VECTOR_SCALAR_ERROR("Word_List_Store");
        }
        for ( ; (offset < size); offset++ )
        {
            BitVector_Word_Store(address,offset,0);
        }
    }
    else BIT_VECTOR_OBJECT_ERROR("Word_List_Store");
}


void
BitVector_Word_List_Read(reference)
BitVector_Object	reference
PPCODE:
{
    BitVector_Handle  handle;
    BitVector_Address address;
    N_int offset;
    N_int value;
    N_int size;

    if ( BIT_VECTOR_OBJECT(reference,handle,address) )
    {
        size = size_(address);
        EXTEND(sp,size);
        for ( offset = 0; (offset < size); offset++ )
        {
            value = BitVector_Word_Read(address,offset);
            PUSHs(sv_2mortal(newSViv((IV)value)));
        }
    }
    else BIT_VECTOR_OBJECT_ERROR("Word_List_Read");
}


void
BitVector_Word_Insert(reference,offset,count)
BitVector_Object	reference
BitVector_Scalar	offset
BitVector_Scalar	count
CODE:
{
    BitVector_Handle  handle;
    BitVector_Address address;
    N_int off;
    N_int cnt;

    if ( BIT_VECTOR_OBJECT(reference,handle,address) )
    {
        if ( BIT_VECTOR_SCALAR(offset,N_int,off) &&
             BIT_VECTOR_SCALAR(count,N_int,cnt) )
        {
            if (off < size_(address))
            {
                BitVector_Word_Insert(address,off,cnt,true);
            }
            else BIT_VECTOR_OFFSET_ERROR("Word_Insert");
        }
        else BIT_VECTOR_SCALAR_ERROR("Word_Insert");
    }
    else BIT_VECTOR_OBJECT_ERROR("Word_Insert");
}


void
BitVector_Word_Delete(reference,offset,count)
BitVector_Object	reference
BitVector_Scalar	offset
BitVector_Scalar	count
CODE:
{
    BitVector_Handle  handle;
    BitVector_Address address;
    N_int off;
    N_int cnt;

    if ( BIT_VECTOR_OBJECT(reference,handle,address) )
    {
        if ( BIT_VECTOR_SCALAR(offset,N_int,off) &&
             BIT_VECTOR_SCALAR(count,N_int,cnt) )
        {
            if (off < size_(address))
            {
                BitVector_Word_Delete(address,off,cnt,true);
            }
            else BIT_VECTOR_OFFSET_ERROR("Word_Delete");
        }
        else BIT_VECTOR_SCALAR_ERROR("Word_Delete");
    }
    else BIT_VECTOR_OBJECT_ERROR("Word_Delete");
}


void
BitVector_Chunk_Store(reference,chunksize,offset,value)
BitVector_Object	reference
BitVector_Scalar	chunksize
BitVector_Scalar	offset
BitVector_Scalar	value
CODE:
{
    BitVector_Handle  handle;
    BitVector_Address address;
    N_int bits;
    N_int off;
    N_long val;

    if ( BIT_VECTOR_OBJECT(reference,handle,address) )
    {
        if ( BIT_VECTOR_SCALAR(chunksize,N_int,bits) &&
             BIT_VECTOR_SCALAR(offset,N_int,off) &&
             BIT_VECTOR_SCALAR(value,N_long,val) )
        {
            if ((bits > 0) and (bits <= BitVector_Long_Bits()))
            {
                if (off < bits_(address))
                {
                    BitVector_Chunk_Store(address,bits,off,val);
                }
                else BIT_VECTOR_OFFSET_ERROR("Chunk_Store");
            }
            else BIT_VECTOR_CHUNK_ERROR("Chunk_Store");
        }
        else BIT_VECTOR_SCALAR_ERROR("Chunk_Store");
    }
    else BIT_VECTOR_OBJECT_ERROR("Chunk_Store");
}


N_long
BitVector_Chunk_Read(reference,chunksize,offset)
BitVector_Object	reference
BitVector_Scalar	chunksize
BitVector_Scalar	offset
CODE:
{
    BitVector_Handle  handle;
    BitVector_Address address;
    N_int bits;
    N_int off;

    if ( BIT_VECTOR_OBJECT(reference,handle,address) )
    {
        if ( BIT_VECTOR_SCALAR(chunksize,N_int,bits) &&
             BIT_VECTOR_SCALAR(offset,N_int,off) )
        {
            if ((bits > 0) and (bits <= BitVector_Long_Bits()))
            {
                if (off < bits_(address))
                {
                    RETVAL = BitVector_Chunk_Read(address,bits,off);
                }
                else BIT_VECTOR_OFFSET_ERROR("Chunk_Read");
            }
            else BIT_VECTOR_CHUNK_ERROR("Chunk_Read");
        }
        else BIT_VECTOR_SCALAR_ERROR("Chunk_Read");
    }
    else BIT_VECTOR_OBJECT_ERROR("Chunk_Read");
}
OUTPUT:
RETVAL


void
BitVector_Chunk_List_Store(reference,chunksize,...)
BitVector_Object	reference
BitVector_Scalar	chunksize
CODE:
{
    BitVector_Handle  handle;
    BitVector_Address address;
    BitVector_Scalar  scalar;
    N_int chunkspan;
    N_long chunkmask;
    N_long mask;
    N_long chunk;
    N_long value;
    N_int chunkbits;
    N_int wordbits;
    N_int wordsize;
    N_int offset;
    N_int size;
    N_int bits;
    I32 index;

    if ( BIT_VECTOR_OBJECT(reference,handle,address) )
    {
        if ( BIT_VECTOR_SCALAR(chunksize,N_int,chunkspan) )
        {
            if ((chunkspan > 0) and (chunkspan <= BitVector_Long_Bits()))
            {
                wordsize = BitVector_Word_Bits();
                size = size_(address);
                chunkmask = ~((~0L << (chunkspan-1)) << 1); /* C bug work-around */
                chunk = 0L;
                value = 0L;
                index = 2;
                offset = 0;
                wordbits = 0;
                chunkbits = 0;
                while (offset < size)
                {
                    if ((chunkbits == 0) and (index < items))
                    {
                        scalar = ST(index);
                        if ( BIT_VECTOR_SCALAR(scalar,N_long,chunk) )
                        {
                            chunk &= chunkmask;
                            chunkbits = chunkspan;
                            index++;
                        }
                        else BIT_VECTOR_SCALAR_ERROR("Chunk_List_Store");
                    }
                    bits = wordsize - wordbits;
                    if (chunkbits <= bits)
                    {
                        chunk <<= wordbits;
                        value |= chunk;
                        wordbits += chunkbits;
                        chunk = 0L;
                        chunkbits = 0;
                    }
                    else
                    {
                        mask = ~(~0L << bits);
                        mask &= chunk;
                        mask <<= wordbits;
                        value |= mask;
                        wordbits += bits;
                        chunk >>= bits;
                        chunkbits -= bits;
                    }
                    if ((wordbits >= wordsize) or (index >= items))
                    {
                        BitVector_Word_Store(address,offset,(N_int)value);
                        value = 0L;
                        wordbits = 0;
                        offset++;
                    }
                }
            }
            else BIT_VECTOR_CHUNK_ERROR("Chunk_List_Store");
        }
        else BIT_VECTOR_SCALAR_ERROR("Chunk_List_Store");
    }
    else BIT_VECTOR_OBJECT_ERROR("Chunk_List_Store");
}


void
BitVector_Chunk_List_Read(reference,chunksize)
BitVector_Object	reference
BitVector_Scalar	chunksize
PPCODE:
{
    BitVector_Handle  handle;
    BitVector_Address address;
    N_int chunkspan;
    N_long chunk;
    N_long value;
    N_long mask;
    N_int chunkbits;
    N_int wordbits;
    N_int wordsize;
    N_int length;
    N_int index;
    N_int offset;
    N_int size;
    N_int bits;

    if ( BIT_VECTOR_OBJECT(reference,handle,address) )
    {
        if ( BIT_VECTOR_SCALAR(chunksize,N_int,chunkspan) )
        {
            if ((chunkspan > 0) and (chunkspan <= BitVector_Long_Bits()))
            {
                wordsize = BitVector_Word_Bits();
                bits = bits_(address);
                size = size_(address);
                length = (N_int) (bits / chunkspan);
                if ((length * chunkspan) < bits) length++;
                EXTEND(sp,length);
                chunk = 0L;
                value = 0L;
                index = 0;
                offset = 0;
                wordbits = 0;
                chunkbits = 0;
                while (index < length)
                {
                    if ((wordbits == 0) and (offset < size))
                    {
                        value = (N_long) BitVector_Word_Read(address,offset);
                        wordbits = wordsize;
                        offset++;
                    }
                    bits = chunkspan - chunkbits;
                    if (wordbits <= bits)
                    {
                        value <<= chunkbits;
                        chunk |= value;
                        chunkbits += wordbits;
                        value = 0L;
                        wordbits = 0;
                    }
                    else
                    {
                        mask = ~(~0L << bits);
                        mask &= value;
                        mask <<= chunkbits;
                        chunk |= mask;
                        chunkbits += bits;
                        value >>= bits;
                        wordbits -= bits;
                    }
                    if ((chunkbits >= chunkspan) or
                        ((offset >= size) and (chunkbits > 0)))
                    {
                        PUSHs(sv_2mortal(newSViv((IV)chunk)));
                        chunk = 0L;
                        chunkbits = 0;
                        index++;
                    }
                }
            }
            else BIT_VECTOR_CHUNK_ERROR("Chunk_List_Read");
        }
        else BIT_VECTOR_SCALAR_ERROR("Chunk_List_Read");
    }
    else BIT_VECTOR_OBJECT_ERROR("Chunk_List_Read");
}


void
BitVector_Index_List_Remove(reference,...)
BitVector_Object	reference
CODE:
{
    BitVector_Handle  handle;
    BitVector_Address address;
    BitVector_Scalar  scalar;
    N_int value;
    N_int bits;
    I32 index;

    if ( BIT_VECTOR_OBJECT(reference,handle,address) )
    {
        bits = bits_(address);
        for ( index = 1; index < items; index++ )
        {
            scalar = ST(index);
            if ( BIT_VECTOR_SCALAR(scalar,N_int,value) )
            {
                if (value < bits)
                {
                    BitVector_Bit_Off(address,value);
                }
                else BIT_VECTOR_INDEX_ERROR("Index_List_Remove");
            }
            else BIT_VECTOR_SCALAR_ERROR("Index_List_Remove");
        }
    }
    else BIT_VECTOR_OBJECT_ERROR("Index_List_Remove");
}


void
BitVector_Index_List_Store(reference,...)
BitVector_Object	reference
CODE:
{
    BitVector_Handle  handle;
    BitVector_Address address;
    BitVector_Scalar  scalar;
    N_int value;
    N_int bits;
    I32 index;

    if ( BIT_VECTOR_OBJECT(reference,handle,address) )
    {
        bits = bits_(address);
        for ( index = 1; index < items; index++ )
        {
            scalar = ST(index);
            if ( BIT_VECTOR_SCALAR(scalar,N_int,value) )
            {
                if (value < bits)
                {
                    BitVector_Bit_On(address,value);
                }
                else BIT_VECTOR_INDEX_ERROR("Index_List_Store");
            }
            else BIT_VECTOR_SCALAR_ERROR("Index_List_Store");
        }
    }
    else BIT_VECTOR_OBJECT_ERROR("Index_List_Store");
}


void
BitVector_Index_List_Read(reference)
BitVector_Object	reference
PPCODE:
{
    BitVector_Handle  handle;
    BitVector_Address address;
    N_int size;
    N_int bits;
    N_int norm;
    N_int base;
    N_int word;
    N_int index;
    N_int value;

    if ( BIT_VECTOR_OBJECT(reference,handle,address) )
    {
        size = size_(address);
        bits = BitVector_Word_Bits();
        norm = Set_Norm(address);
        if (norm > 0)
        {
            EXTEND(sp,norm);
            for ( base = word = 0; word < size; word++, base += bits )
            {
                index = base;
                value = BitVector_Word_Read(address,word);
                while (value)
                {
                    if (value AND 0x0001)
                      PUSHs(sv_2mortal(newSViv((IV)index)));
                    value >>= 1;
                    index++;
                }
            }
        }
    }
    else BIT_VECTOR_OBJECT_ERROR("Index_List_Read");
}


MODULE = Bit::Vector		PACKAGE = Bit::Vector		PREFIX = Set_


void
Set_Union(Xref,Yref,Zref)
BitVector_Object	Xref
BitVector_Object	Yref
BitVector_Object	Zref
ALIAS:
  Or = 2
CODE:
{
    BitVector_Handle  Xhdl;
    BitVector_Address Xadr;
    BitVector_Handle  Yhdl;
    BitVector_Address Yadr;
    BitVector_Handle  Zhdl;
    BitVector_Address Zadr;

    if ( BIT_VECTOR_OBJECT(Xref,Xhdl,Xadr) &&
         BIT_VECTOR_OBJECT(Yref,Yhdl,Yadr) &&
         BIT_VECTOR_OBJECT(Zref,Zhdl,Zadr) )
    {
        if ((bits_(Xadr) == bits_(Yadr)) and (bits_(Xadr) == bits_(Zadr)))
        {
            Set_Union(Xadr,Yadr,Zadr);
        }
        else BIT_VECTOR_SET_ERROR("Union");
    }
    else BIT_VECTOR_OBJECT_ERROR("Union");
}


void
Set_Intersection(Xref,Yref,Zref)
BitVector_Object	Xref
BitVector_Object	Yref
BitVector_Object	Zref
ALIAS:
  And = 2
CODE:
{
    BitVector_Handle  Xhdl;
    BitVector_Address Xadr;
    BitVector_Handle  Yhdl;
    BitVector_Address Yadr;
    BitVector_Handle  Zhdl;
    BitVector_Address Zadr;

    if ( BIT_VECTOR_OBJECT(Xref,Xhdl,Xadr) &&
         BIT_VECTOR_OBJECT(Yref,Yhdl,Yadr) &&
         BIT_VECTOR_OBJECT(Zref,Zhdl,Zadr) )
    {
        if ((bits_(Xadr) == bits_(Yadr)) and (bits_(Xadr) == bits_(Zadr)))
        {
            Set_Intersection(Xadr,Yadr,Zadr);
        }
        else BIT_VECTOR_SET_ERROR("Intersection");
    }
    else BIT_VECTOR_OBJECT_ERROR("Intersection");
}


void
Set_Difference(Xref,Yref,Zref)
BitVector_Object	Xref
BitVector_Object	Yref
BitVector_Object	Zref
ALIAS:
  AndNot = 2
CODE:
{
    BitVector_Handle  Xhdl;
    BitVector_Address Xadr;
    BitVector_Handle  Yhdl;
    BitVector_Address Yadr;
    BitVector_Handle  Zhdl;
    BitVector_Address Zadr;

    if ( BIT_VECTOR_OBJECT(Xref,Xhdl,Xadr) &&
         BIT_VECTOR_OBJECT(Yref,Yhdl,Yadr) &&
         BIT_VECTOR_OBJECT(Zref,Zhdl,Zadr) )
    {
        if ((bits_(Xadr) == bits_(Yadr)) and (bits_(Xadr) == bits_(Zadr)))
        {
            Set_Difference(Xadr,Yadr,Zadr);
        }
        else BIT_VECTOR_SET_ERROR("Difference");
    }
    else BIT_VECTOR_OBJECT_ERROR("Difference");
}


void
Set_ExclusiveOr(Xref,Yref,Zref)
BitVector_Object	Xref
BitVector_Object	Yref
BitVector_Object	Zref
ALIAS:
  Xor = 2
CODE:
{
    BitVector_Handle  Xhdl;
    BitVector_Address Xadr;
    BitVector_Handle  Yhdl;
    BitVector_Address Yadr;
    BitVector_Handle  Zhdl;
    BitVector_Address Zadr;

    if ( BIT_VECTOR_OBJECT(Xref,Xhdl,Xadr) &&
         BIT_VECTOR_OBJECT(Yref,Yhdl,Yadr) &&
         BIT_VECTOR_OBJECT(Zref,Zhdl,Zadr) )
    {
        if ((bits_(Xadr) == bits_(Yadr)) and (bits_(Xadr) == bits_(Zadr)))
        {
            Set_ExclusiveOr(Xadr,Yadr,Zadr);
        }
        else BIT_VECTOR_SET_ERROR("ExclusiveOr");
    }
    else BIT_VECTOR_OBJECT_ERROR("ExclusiveOr");
}


void
Set_Complement(Xref,Yref)
BitVector_Object	Xref
BitVector_Object	Yref
ALIAS:
  Not = 2
CODE:
{
    BitVector_Handle  Xhdl;
    BitVector_Address Xadr;
    BitVector_Handle  Yhdl;
    BitVector_Address Yadr;

    if ( BIT_VECTOR_OBJECT(Xref,Xhdl,Xadr) &&
         BIT_VECTOR_OBJECT(Yref,Yhdl,Yadr) )
    {
        if (bits_(Xadr) == bits_(Yadr))
        {
            Set_Complement(Xadr,Yadr);
        }
        else BIT_VECTOR_SET_ERROR("Complement");
    }
    else BIT_VECTOR_OBJECT_ERROR("Complement");
}


boolean
Set_subset(Xref,Yref)
BitVector_Object	Xref
BitVector_Object	Yref
ALIAS:
  inclusion = 1
CODE:
{
    BitVector_Handle  Xhdl;
    BitVector_Address Xadr;
    BitVector_Handle  Yhdl;
    BitVector_Address Yadr;

    if ( BIT_VECTOR_OBJECT(Xref,Xhdl,Xadr) &&
         BIT_VECTOR_OBJECT(Yref,Yhdl,Yadr) )
    {
        if (bits_(Xadr) == bits_(Yadr))
        {
            RETVAL = Set_subset(Xadr,Yadr);
        }
        else BIT_VECTOR_SET_ERROR("subset");
    }
    else BIT_VECTOR_OBJECT_ERROR("subset");
}
OUTPUT:
RETVAL


N_int
Set_Norm(reference)
BitVector_Object	reference
CODE:
{
    BitVector_Handle  handle;
    BitVector_Address address;

    if ( BIT_VECTOR_OBJECT(reference,handle,address) )
    {
        RETVAL = Set_Norm(address);
    }
    else BIT_VECTOR_OBJECT_ERROR("Norm");
}
OUTPUT:
RETVAL


Z_long
Set_Min(reference)
BitVector_Object	reference
CODE:
{
    BitVector_Handle  handle;
    BitVector_Address address;

    if ( BIT_VECTOR_OBJECT(reference,handle,address) )
    {
        RETVAL = Set_Min(address);
    }
    else BIT_VECTOR_OBJECT_ERROR("Min");
}
OUTPUT:
RETVAL


Z_long
Set_Max(reference)
BitVector_Object	reference
CODE:
{
    BitVector_Handle  handle;
    BitVector_Address address;

    if ( BIT_VECTOR_OBJECT(reference,handle,address) )
    {
        RETVAL = Set_Max(address);
    }
    else BIT_VECTOR_OBJECT_ERROR("Max");
}
OUTPUT:
RETVAL


MODULE = Bit::Vector		PACKAGE = Bit::Vector		PREFIX = Matrix_


void
Matrix_Multiplication(Xref,Xrows,Xcols,Yref,Yrows,Ycols,Zref,Zrows,Zcols)
BitVector_Object	Xref
BitVector_Scalar	Xrows
BitVector_Scalar	Xcols
BitVector_Object	Yref
BitVector_Scalar	Yrows
BitVector_Scalar	Ycols
BitVector_Object	Zref
BitVector_Scalar	Zrows
BitVector_Scalar	Zcols
CODE:
{
    BitVector_Handle  Xhdl;
    BitVector_Address Xadr;
    BitVector_Handle  Yhdl;
    BitVector_Address Yadr;
    BitVector_Handle  Zhdl;
    BitVector_Address Zadr;
    N_int rowsX;
    N_int colsX;
    N_int rowsY;
    N_int colsY;
    N_int rowsZ;
    N_int colsZ;

    if ( BIT_VECTOR_OBJECT(Xref,Xhdl,Xadr) &&
         BIT_VECTOR_OBJECT(Yref,Yhdl,Yadr) &&
         BIT_VECTOR_OBJECT(Zref,Zhdl,Zadr) )
    {
        if ( BIT_VECTOR_SCALAR(Xrows,N_int,rowsX) &&
             BIT_VECTOR_SCALAR(Xcols,N_int,colsX) &&
             BIT_VECTOR_SCALAR(Yrows,N_int,rowsY) &&
             BIT_VECTOR_SCALAR(Ycols,N_int,colsY) &&
             BIT_VECTOR_SCALAR(Zrows,N_int,rowsZ) &&
             BIT_VECTOR_SCALAR(Zcols,N_int,colsZ) )
        {
            if ((colsY == rowsZ) and (rowsX == rowsY) and (colsX == colsZ) and
                (bits_(Xadr) == rowsX*colsX) and
                (bits_(Yadr) == rowsY*colsY) and
                (bits_(Zadr) == rowsZ*colsZ))
            {
                Matrix_Multiplication(Xadr,rowsX,colsX,
                                      Yadr,rowsY,colsY,
                                      Zadr,rowsZ,colsZ);
            }
            else BIT_VECTOR_MATRIX_ERROR("Multiplication");
        }
        else BIT_VECTOR_SCALAR_ERROR("Multiplication");
    }
    else BIT_VECTOR_OBJECT_ERROR("Multiplication");
}


void
Matrix_Closure(reference,rows,cols)
BitVector_Object	reference
BitVector_Scalar	rows
BitVector_Scalar	cols
CODE:
{
    BitVector_Handle  handle;
    BitVector_Address address;
    N_int r;
    N_int c;

    if ( BIT_VECTOR_OBJECT(reference,handle,address) )
    {
        if ( BIT_VECTOR_SCALAR(rows,N_int,r) &&
             BIT_VECTOR_SCALAR(cols,N_int,c) )
        {
            if (bits_(address) == r*c)
            {
                if (r == c)
                {
                    Matrix_Closure(address,r,c);
                }
                else BIT_VECTOR_SHAPE_ERROR("Closure");
            }
            else BIT_VECTOR_MATRIX_ERROR("Closure");
        }
        else BIT_VECTOR_SCALAR_ERROR("Closure");
    }
    else BIT_VECTOR_OBJECT_ERROR("Closure");
}


void
Matrix_Transpose(Xref,Xrows,Xcols,Yref,Yrows,Ycols)
BitVector_Object	Xref
BitVector_Scalar	Xrows
BitVector_Scalar	Xcols
BitVector_Object	Yref
BitVector_Scalar	Yrows
BitVector_Scalar	Ycols
CODE:
{
    BitVector_Handle  Xhdl;
    BitVector_Address Xadr;
    BitVector_Handle  Yhdl;
    BitVector_Address Yadr;
    N_int rowsX;
    N_int colsX;
    N_int rowsY;
    N_int colsY;

    if ( BIT_VECTOR_OBJECT(Xref,Xhdl,Xadr) &&
         BIT_VECTOR_OBJECT(Yref,Yhdl,Yadr) )
    {
        if ( BIT_VECTOR_SCALAR(Xrows,N_int,rowsX) &&
             BIT_VECTOR_SCALAR(Xcols,N_int,colsX) &&
             BIT_VECTOR_SCALAR(Yrows,N_int,rowsY) &&
             BIT_VECTOR_SCALAR(Ycols,N_int,colsY) )
        {
            if ((rowsX == colsY) and (colsX == rowsY) and
                (bits_(Xadr) == rowsX*colsX) and
                (bits_(Yadr) == rowsY*colsY))
            {
                if ((Xadr != Yadr) or (rowsY == colsY))
                {
                    Matrix_Transpose(Xadr,rowsX,colsX,
                                     Yadr,rowsY,colsY);
                }
                else BIT_VECTOR_SHAPE_ERROR("Transpose");
            }
            else BIT_VECTOR_MATRIX_ERROR("Transpose");
        }
        else BIT_VECTOR_SCALAR_ERROR("Transpose");
    }
    else BIT_VECTOR_OBJECT_ERROR("Transpose");
}


