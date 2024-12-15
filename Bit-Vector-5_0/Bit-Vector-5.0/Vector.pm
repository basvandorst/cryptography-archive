
###############################################################################
##                                                                           ##
##    Copyright (c) 1995, 1996, 1997, 1998 by Steffen Beyer.                 ##
##    All rights reserved.                                                   ##
##                                                                           ##
##    This piece of software is "Non-Profit-Ware" ("NP-ware").               ##
##                                                                           ##
##    You may use, copy, modify and redistribute it under                    ##
##    the terms of the "Non-Profit-License" (NPL).                           ##
##                                                                           ##
##    Please refer to the file "NONPROFIT" in this distribution              ##
##    for details!                                                           ##
##                                                                           ##
###############################################################################

package Bit::Vector;

use strict;
use vars qw(@ISA @EXPORT @EXPORT_OK $VERSION @CONFIG);

require Exporter;
require DynaLoader;

@ISA = qw(Exporter DynaLoader);

@EXPORT = qw();

@EXPORT_OK = qw();

$VERSION = '5.0';

$CONFIG[0] = 0;
$CONFIG[1] = 0;
$CONFIG[2] = 0;

#  Configuration:
#
#  0 = Scalar Input:        0 = Bit Index  (default)
#                           1 = from_Hex
#                           2 = from_Bin
#                           3 = from_Dec
#                           4 = from_Enum
#
#  1 = Operator Semantics:  0 = Set Ops    (default)
#                           1 = Arithmetic Ops
#
#      Affected Operators:  "+"  "-"  "*"
#                           "<"  "<="  ">"  ">="
#                           "abs"
#
#  2 = String Output:       0 = to_Hex()   (default)
#                           1 = to_Bin()
#                           2 = to_Dec()
#                           3 = to_Enum()

bootstrap Bit::Vector $VERSION;

use Carp;

use overload
      '""' => '_scalarize',
    'bool' => '_boolean',
       '!' => '_not_boolean',
       '~' => '_complement',
     'neg' => '_negate',
     'abs' => '_absolute',
       '.' => '_concat',
       'x' => '_xerox',
      '<<' => '_shift_left',
      '>>' => '_shift_right',
       '|' => '_union',
       '&' => '_intersection',
       '^' => '_exclusive_or',
       '+' => '_add',
       '-' => '_sub',
       '*' => '_mul',
       '/' => '_div',
       '%' => '_mod',
      '.=' => '_assign_concat',
      'x=' => '_assign_xerox',
     '<<=' => '_assign_shift_left',
     '>>=' => '_assign_shift_right',
      '|=' => '_assign_union',
      '&=' => '_assign_intersection',
      '^=' => '_assign_exclusive_or',
      '+=' => '_assign_add',
      '-=' => '_assign_sub',
      '*=' => '_assign_mul',
      '/=' => '_assign_div',
      '%=' => '_assign_mod',
      '++' => '_increment',
      '--' => '_decrement',
     'cmp' => '_lexicompare',  #  also enables lt, le, gt, ge, eq, ne
     '<=>' => '_compare',
      '==' => '_equal',
      '!=' => '_not_equal',
       '<' => '_less_than',
      '<=' => '_less_equal',
       '>' => '_greater_than',
      '>=' => '_greater_equal',
       '=' => '_clone',
'fallback' =>   undef;

sub Configuration
{
    my(@commands);
    my($assignment);
    my($which,$value);
    my($m0,$m1,$m2,$m3,$m4);
    my($result);
    my($ok);

    croak
'Usage: $configuration = Bit::Vector->Configuration( [ $configuration ] );'
    if (@_ > 2);

    $result  =   "Scalar Input       = ";
    if    ($CONFIG[0] == 4) { $result .= "Enumeration"; }
    elsif ($CONFIG[0] == 3) { $result .= "Decimal"; }
    elsif ($CONFIG[0] == 2) { $result .= "Binary"; }
    elsif ($CONFIG[0] == 1) { $result .= "Hexadecimal"; }
    else                    { $result .= "Bit Index"; }
    $result .= "\nOperator Semantics = ";
    if    ($CONFIG[1] == 1) { $result .= "Arithmetic Operators"; }
    else                    { $result .= "Set Operators"; }
    $result .= "\nString Output      = ";
    if    ($CONFIG[2] == 3) { $result .= "Enumeration"; }
    elsif ($CONFIG[2] == 2) { $result .= "Decimal"; }
    elsif ($CONFIG[2] == 1) { $result .= "Binary"; }
    else                    { $result .= "Hexadecimal"; }
    shift if (@_ > 0);
    if (@_ > 0)
    {
        $ok = 1;
        @commands = split(/[,;:|\/\n&+-]/, $_[0]);
        foreach $assignment (@commands)
        {
            if    ($assignment =~ /^\s*$/) { }  #  ignore empty lines
            elsif ($assignment =~ /^([A-Za-z\s]+)=([A-Za-z\s]+)$/)
            {
                $which = $1;
                $value = $2;
                $m0 = 0;
                $m1 = 0;
                $m2 = 0;
                if ($which =~ /\bscalar|\binput|\bin\b/i)       { $m0 = 1; }
                if ($which =~ /\boperator|\bsemantic|\bops\b/i) { $m1 = 1; }
                if ($which =~ /\bstring|\boutput|\bout\b/i)     { $m2 = 1; }
                if    ($m0 && !$m1 && !$m2)
                {
                    $m0 = 0;
                    $m1 = 0;
                    $m2 = 0;
                    $m3 = 0;
                    $m4 = 0;
                    if ($value =~ /\bbit\b|\bindex|\bindice/i) { $m0 = 1; }
                    if ($value =~ /\bhex/i)                    { $m1 = 1; }
                    if ($value =~ /\bbin/i)                    { $m2 = 1; }
                    if ($value =~ /\bdec/i)                    { $m3 = 1; }
                    if ($value =~ /\benum/i)                   { $m4 = 1; }
                    if    ($m0 && !$m1 && !$m2 && !$m3 && !$m4) { $CONFIG[0] = 0; }
                    elsif (!$m0 && $m1 && !$m2 && !$m3 && !$m4) { $CONFIG[0] = 1; }
                    elsif (!$m0 && !$m1 && $m2 && !$m3 && !$m4) { $CONFIG[0] = 2; }
                    elsif (!$m0 && !$m1 && !$m2 && $m3 && !$m4) { $CONFIG[0] = 3; }
                    elsif (!$m0 && !$m1 && !$m2 && !$m3 && $m4) { $CONFIG[0] = 4; }
                    else                                        { $ok = 0; last; }
                }
                elsif (!$m0 && $m1 && !$m2)
                {
                    $m0 = 0;
                    $m1 = 0;
                    if ($value =~ /\bset\b/i)      { $m0 = 1; }
                    if ($value =~ /\barithmetic/i) { $m1 = 1; }
                    if    ($m0 && !$m1) { $CONFIG[1] = 0; }
                    elsif (!$m0 && $m1) { $CONFIG[1] = 1; }
                    else                { $ok = 0; last; }
                }
                elsif (!$m0 && !$m1 && $m2)
                {
                    $m0 = 0;
                    $m1 = 0;
                    $m2 = 0;
                    $m3 = 0;
                    if ($value =~ /\bhex/i)  { $m0 = 1; }
                    if ($value =~ /\bbin/i)  { $m1 = 1; }
                    if ($value =~ /\bdec/i)  { $m2 = 1; }
                    if ($value =~ /\benum/i) { $m3 = 1; }
                    if    ($m0 && !$m1 && !$m2 && !$m3) { $CONFIG[2] = 0; }
                    elsif (!$m0 && $m1 && !$m2 && !$m3) { $CONFIG[2] = 1; }
                    elsif (!$m0 && !$m1 && $m2 && !$m3) { $CONFIG[2] = 2; }
                    elsif (!$m0 && !$m1 && !$m2 && $m3) { $CONFIG[2] = 3; }
                    else                                { $ok = 0; last; }
                }
                else { $ok = 0; last; }
            }
            else { $ok = 0; last; }
        }
        croak
'Bit::Vector::Configuration(): configuration string syntax error'
        unless ($ok);
    }
    return($result);
}

sub _vectorize
{
    my($vector,$scalar,$name) = @_;

    if    ($CONFIG[0] == 4) { eval { $vector->from_Enum($scalar); }; }
    elsif ($CONFIG[0] == 3) { eval { $vector->from_Dec ($scalar); }; }
    elsif ($CONFIG[0] == 2) { eval { $vector->from_Bin ($scalar); }; }
    elsif ($CONFIG[0] == 1) { eval { $vector->from_Hex ($scalar); }; }
    else                    { eval { $vector->Bit_On   ($scalar); }; }
    if ($@)
    {
        if ($@ =~ m/^[a-zA-Z0-9_:]+\(\):\s+(.+?)\s+at\s/)
        {
            croak "Bit::Vector \"$name\": $1";
        }
        else
        {
            croak "Bit::Vector \"$name\": scalar operand conversion error";
        }
    }
}

sub _scalarize
{
    my($vector) = @_;

    if    ($CONFIG[2] == 3) { return( $vector->to_Enum() ); }
    elsif ($CONFIG[2] == 2) { return( $vector->to_Dec () ); }
    elsif ($CONFIG[2] == 1) { return( $vector->to_Bin () ); }
    else                    { return( $vector->to_Hex () ); }
}

sub _fetch_operand
{
    my($name,$object,$argument,$flag,$build) = @_;
    my($operand);

    $name .= '=' unless (defined $flag);
    if ((defined $argument) && ref($argument) && (ref($argument) !~ /^[A-Z]+$/))
    {
        if ($build) { $operand = $argument->Clone(); }
        else        { $operand = $argument; }
    }
    elsif ((defined $argument) && !(ref($argument)))
    {
        $operand = $object->Shadow();
        _vectorize($operand,$argument,$name);
    }
    else { croak "Bit::Vector \"$name\": illegal operand type error"; }
    return($operand);
}

sub _check_operand
{
    my($name,$argument,$flag) = @_;

    $name .= '=' unless (defined $flag);
    if ((defined $argument) && !(ref($argument)))
    {
        if ((defined $flag) && $flag)
        {
            croak "Bit::Vector \"$name\": reversed operands error";
        }
    }
    else { croak "Bit::Vector \"$name\": illegal operand type error"; }
}

sub _boolean
{
    my($object) = @_;

    return( ! $object->is_empty() );
}

sub _not_boolean
{
    my($object) = @_;

    return( $object->is_empty() );
}

sub _complement
{
    my($object) = @_;
    my($result);

    $result = $object->Shadow();
    $result->Complement($object);
    return($result);
}

sub _negate
{
    my($object) = @_;
    my($result);

    $result = $object->Shadow();
    $result->Negate($object);
    return($result);
}

sub _absolute
{
    my($object) = @_;
    my($result);

    if ($CONFIG[1] == 1)
    {
        $result = $object->Shadow();
        $result->Absolute($object);
        return($result);
    }
    else
    {
        return( $object->Norm() );
    }
}

sub _concat
{
    my($object,$argument,$flag) = @_;
    my($result);
    my($name);

    $name = '.';
    $name .= '=' unless (defined $flag);
    if ((defined $argument) && ref($argument) && (ref($argument) !~ /^[A-Z]+$/))
    {
        if (defined $flag)
        {
            if ($flag) { $result = $argument->Concat($object); }
            else       { $result = $object->Concat($argument); }
            return($result);
        }
        else
        {
            $object->Interval_Substitute($argument,0,0,0,$argument->Size());
            return($object);
        }
    }
    elsif ((defined $argument) && !(ref($argument)))
    {
        if (defined $flag)
        {
            if ($flag) { $result = $argument . _scalarize($object); }
            else       { $result = _scalarize($object) . $argument; }
            return($result);
        }
        else
        {
            if    ($CONFIG[0] == 2) { $result = $object->new( length($argument) ); }
            elsif ($CONFIG[0] == 1) { $result = $object->new( length($argument) << 2 ); }
            else                    { $result = $object->Shadow(); }
            _vectorize($result,$argument,$name);
            $object->Interval_Substitute($result,0,0,0,$result->Size());
            return($object);
        }
    }
    else { croak "Bit::Vector \"$name\": illegal operand type error"; }
}

sub _xerox  #  (in Brazil, a photocopy is called a "xerox")
{
    my($object,$argument,$flag) = @_;
    my($result);
    my($offset);
    my($index);
    my($size);

    &_check_operand('x',$argument,$flag);
    $size = $object->Size();
    if (defined $flag)
    {
        $result = $object->new($size * $argument);
        $offset = 0;
        $index = 0;
    }
    else
    {
        $result = $object;
        $result->Resize($size * $argument);
        $offset = $size;
        $index = 1;
    }
    for ( ; $index < $argument; $index++, $offset += $size )
    {
        $result->Interval_Copy($object,$offset,0,$size);
    }
    return($result);
}

sub _shift_left
{
    my($object,$argument,$flag) = @_;
    my($result);

    &_check_operand('<<',$argument,$flag);
    if (defined $flag)
    {
        $result = $object->Clone();
        $result->Move_Left($argument);
        return($result);
    }
    else
    {
        $object->Move_Left($argument);
        return($object);
    }
}

sub _shift_right
{
    my($object,$argument,$flag) = @_;
    my($result);

    &_check_operand('>>',$argument,$flag);
    if (defined $flag)
    {
        $result = $object->Clone();
        $result->Move_Right($argument);
        return($result);
    }
    else
    {
        $object->Move_Right($argument);
        return($object);
    }
}

sub _union_
{
    my($object,$operand,$flag) = @_;

    if (defined $flag)
    {
        $operand->Union($object,$operand);
        return($operand);
    }
    else
    {
        $object->Union($object,$operand);
        return($object);
    }
}

sub _union
{
    my($object,$argument,$flag) = @_;
    my($operand) = &_fetch_operand('|',$object,$argument,$flag,1);

    return ( &_union_($object,$operand,$flag) );
}

sub _intersection_
{
    my($object,$operand,$flag) = @_;

    if (defined $flag)
    {
        $operand->Intersection($object,$operand);
        return($operand);
    }
    else
    {
        $object->Intersection($object,$operand);
        return($object);
    }
}

sub _intersection
{
    my($object,$argument,$flag) = @_;
    my($operand) = &_fetch_operand('&',$object,$argument,$flag,1);

    return ( &_intersection_($object,$operand,$flag) );
}

sub _exclusive_or
{
    my($object,$argument,$flag) = @_;
    my($operand) = &_fetch_operand('^',$object,$argument,$flag,1);

    if (defined $flag)
    {
        $operand->ExclusiveOr($object,$operand);
        return($operand);
    }
    else
    {
        $object->ExclusiveOr($object,$operand);
        return($object);
    }
}

sub _add
{
    my($object,$argument,$flag) = @_;
    my($operand) = &_fetch_operand('+',$object,$argument,$flag,1);

    if ($CONFIG[1] == 1)
    {
        if (defined $flag)
        {
            $operand->add($object,$operand,0);
            return($operand);
        }
        else
        {
            $object->add($object,$operand,0);
            return($object);
        }
    }
    else
    {
        return( &_union_($object,$operand,$flag) );
    }
}

sub _sub
{
    my($object,$argument,$flag) = @_;
    my($operand) = &_fetch_operand('-',$object,$argument,$flag,1);

    if ($CONFIG[1] == 1)
    {
        if (defined $flag)
        {
            if ($flag) { $operand->subtract($operand,$object,0); }
            else       { $operand->subtract($object,$operand,0); }
            return($operand);
        }
        else
        {
            $object->subtract($object,$operand,0);
            return($object);
        }
    }
    else
    {
        if (defined $flag)
        {
            if ($flag) { $operand->Difference($operand,$object); }
            else       { $operand->Difference($object,$operand); }
            return($operand);
        }
        else
        {
            $object->Difference($object,$operand);
            return($object);
        }
    }
}

sub _mul
{
    my($object,$argument,$flag) = @_;
    my($operand) = &_fetch_operand('*',$object,$argument,$flag,1);

    if ($CONFIG[1] == 1)
    {
        if (defined $flag)
        {
            $operand->Multiply($object,$operand);
            return($operand);
        }
        else
        {
            $object->Multiply($object,$operand);
            return($object);
        }
    }
    else
    {
        return( &_intersection_($object,$operand,$flag) );
    }
}

sub _div
{
    my($object,$argument,$flag) = @_;
    my($operand) = &_fetch_operand('/',$object,$argument,$flag,1);
    my($temp) = $object->Shadow();

    if (defined $flag)
    {
        if ($flag) { $operand->Divide($operand,$object,$temp); }
        else       { $operand->Divide($object,$operand,$temp); }
        return($operand);
    }
    else
    {
        $object->Divide($object,$operand,$temp);
        return($object);
    }
}

sub _mod
{
    my($object,$argument,$flag) = @_;
    my($operand) = &_fetch_operand('%',$object,$argument,$flag,1);
    my($temp) = $object->Shadow();

    if (defined $flag)
    {
        if ($flag) { $temp->Divide($operand,$object,$operand); }
        else       { $temp->Divide($object,$operand,$operand); }
        return($operand);
    }
    else
    {
        $temp->Divide($object,$operand,$object);
        return($object);
    }
}

sub _assign_concat
{
    my($object,$argument) = @_;

    return( &_concat($object,$argument,undef) );
}

sub _assign_xerox
{
    my($object,$argument) = @_;

    return( &_xerox($object,$argument,undef) );
}

sub _assign_shift_left
{
    my($object,$argument) = @_;

    return( &_shift_left($object,$argument,undef) );
}

sub _assign_shift_right
{
    my($object,$argument) = @_;

    return( &_shift_right($object,$argument,undef) );
}

sub _assign_union
{
    my($object,$argument) = @_;

    return( &_union($object,$argument,undef) );
}

sub _assign_intersection
{
    my($object,$argument) = @_;

    return( &_intersection($object,$argument,undef) );
}

sub _assign_exclusive_or
{
    my($object,$argument) = @_;

    return( &_exclusive_or($object,$argument,undef) );
}

sub _assign_add
{
    my($object,$argument) = @_;

    return( &_add($object,$argument,undef) );
}

sub _assign_sub
{
    my($object,$argument) = @_;

    return( &_sub($object,$argument,undef) );
}

sub _assign_mul
{
    my($object,$argument) = @_;

    return( &_mul($object,$argument,undef) );
}

sub _assign_div
{
    my($object,$argument) = @_;

    return( &_div($object,$argument,undef) );
}

sub _assign_mod
{
    my($object,$argument) = @_;

    return( &_mod($object,$argument,undef) );
}

sub _increment
{
    my($object) = @_;

    return( $object->increment() );
}

sub _decrement
{
    my($object) = @_;

    return( $object->decrement() );
}

sub _lexicompare
{
    my($object,$argument,$flag) = @_;
    my($operand) = &_fetch_operand('cmp',$object,$argument,$flag,0);

    if ((defined $flag) && $flag)
    {
        return( $operand->Lexicompare($object) );
    }
    else
    {
        return( $object->Lexicompare($operand) );
    }
}

sub _compare
{
    my($object,$argument,$flag) = @_;
    my($operand) = &_fetch_operand('<=>',$object,$argument,$flag,0);

    if ((defined $flag) && $flag)
    {
        return( $operand->Compare($object) );
    }
    else
    {
        return( $object->Compare($operand) );
    }
}

sub _equal
{
    my($object,$argument,$flag) = @_;
    my($operand) = &_fetch_operand('==',$object,$argument,$flag,0);

    return( $object->equal($operand) );
}

sub _not_equal
{
    my($object,$argument,$flag) = @_;
    my($operand) = &_fetch_operand('!=',$object,$argument,$flag,0);

    return( !$object->equal($operand) );
}

sub _less_than
{
    my($object,$argument,$flag) = @_;
    my($operand) = &_fetch_operand('<',$object,$argument,$flag,0);

    if ($CONFIG[1] == 1)
    {
        if ((defined $flag) && $flag)
        {
            return( $operand->Compare($object) < 0 );
        }
        else
        {
            return( $object->Compare($operand) < 0 );
        }
    }
    else
    {
        if ((defined $flag) && $flag)
        {
            return( !($operand->equal($object)) &&
                     ($operand->subset($object)) );
        }
        else
        {
            return( !($object->equal($operand)) &&
                     ($object->subset($operand)) );
        }
    }
}

sub _less_equal
{
    my($object,$argument,$flag) = @_;
    my($operand) = &_fetch_operand('<=',$object,$argument,$flag,0);

    if ($CONFIG[1] == 1)
    {
        if ((defined $flag) && $flag)
        {
            return( $operand->Compare($object) <= 0 );
        }
        else
        {
            return( $object->Compare($operand) <= 0 );
        }
    }
    else
    {
        if ((defined $flag) && $flag)
        {
            return( $operand->subset($object) );
        }
        else
        {
            return( $object->subset($operand) );
        }
    }
}

sub _greater_than
{
    my($object,$argument,$flag) = @_;
    my($operand) = &_fetch_operand('>',$object,$argument,$flag,0);

    if ($CONFIG[1] == 1)
    {
        if ((defined $flag) && $flag)
        {
            return( $operand->Compare($object) > 0 );
        }
        else
        {
            return( $object->Compare($operand) > 0 );
        }
    }
    else
    {
        if ((defined $flag) && $flag)
        {
            return( !($object->equal($operand)) &&
                     ($object->subset($operand)) );
        }
        else
        {
            return( !($operand->equal($object)) &&
                     ($operand->subset($object)) );
        }
    }
}

sub _greater_equal
{
    my($object,$argument,$flag) = @_;
    my($operand) = &_fetch_operand('>=',$object,$argument,$flag,0);

    if ($CONFIG[1] == 1)
    {
        if ((defined $flag) && $flag)
        {
            return( $operand->Compare($object) >= 0 );
        }
        else
        {
            return( $object->Compare($operand) >= 0 );
        }
    }
    else
    {
        if ((defined $flag) && $flag)
        {
            return( $object->subset($operand) );
        }
        else
        {
            return( $operand->subset($object) );
        }
    }
}

sub _clone
{
    my($object) = @_;

    return( $object->Clone() );
}

1;

__END__

=head1 NAME

Bit::Vector - efficient base class implementing bit vectors

This module implements bit vectors of arbitrary size
and provides efficient methods for handling them.

This goes far beyond the built-in capabilities of Perl for
handling bit vectors (compare with the method list below).

Moreover, the C core of this module can be used "stand-alone"
in other C applications; Perl is not necessarily required.

The module is intended to serve as a base class for other applications
or application classes, such as implementing sets or performing big
integer arithmetic.

All methods are implemented in C internally for maximum performance.

The module also provides overloaded arithmetic and relational operators
for maximum ease of use (Perl only).

Note that there is (of course) a little speed penalty to pay for
overloaded operators. If speed is crucial, use the methods of this
module directly instead of their corresponding overloaded operators!

=head1 SYNOPSIS

=head2 CLASS METHODS (implemented in C)

  Version
      $version = Bit::Vector->Version();

  Word_Bits
      $bits = Bit::Vector->Word_Bits();  #  bits in a machine word

  Long_Bits
      $bits = Bit::Vector->Long_Bits();  #  bits in an unsigned long

  new
      $vector = Bit::Vector->new($bits);  #  bit vector constructor

  Concat_List
      $vector = Bit::Vector->Concat_List(@vectors);

=head2 OBJECT METHODS (implemented in C)

  new
      $vec2 = $vec1->new($bits);  #  alternative call of constructor

  Shadow
      $vec2 = $vec1->Shadow();  #  new vector, same size but empty

  Clone
      $vec2 = $vec1->Clone();  #  new vector, exact duplicate

  Concat
      $vector = $vec1->Concat($vec2);

  Concat_List
      $vector = $vec1->Concat_List($vec2,$vec3,...);

  Size
      $bits = $vector->Size();

  Resize
      $vector->Resize($bits);
      $vector->Resize($vector->Size()+5);
      $vector->Resize($vector->Size()-5);

  Copy
      $vec2->Copy($vec1);

  Empty
      $vector->Empty();

  Fill
      $vector->Fill();

  Flip
      $vector->Flip();

  Primes
      $vector->Primes();  #  Sieve of Erathostenes

  Reverse
      $vec2->Reverse($vec1);

  Interval_Empty
      $vector->Interval_Empty($min,$max);

  Interval_Fill
      $vector->Interval_Fill($min,$max);

  Interval_Flip
      $vector->Interval_Flip($min,$max);

  Interval_Reverse
      $vector->Interval_Reverse($min,$max);

  Interval_Scan_inc
      if (($min,$max) = $vector->Interval_Scan_inc($start))

  Interval_Scan_dec
      if (($min,$max) = $vector->Interval_Scan_dec($start))

  Interval_Copy
      $vec2->Interval_Copy($vec1,$offset2,$offset1,$length);

  Interval_Substitute
      $vec2->Interval_Substitute($vec1,$off2,$len2,$off1,$len1);

  is_empty
      if ($vector->is_empty())

  is_full
      if ($vector->is_full())

  equal
      if ($vec1->equal($vec2))

  Lexicompare (unsigned)
      if ($vec1->Lexicompare($vec2) == 0)
      if ($vec1->Lexicompare($vec2) != 0)
      if ($vec1->Lexicompare($vec2) <  0)
      if ($vec1->Lexicompare($vec2) <= 0)
      if ($vec1->Lexicompare($vec2) >  0)
      if ($vec1->Lexicompare($vec2) >= 0)

  Compare (signed)
      if ($vec1->Compare($vec2) == 0)
      if ($vec1->Compare($vec2) != 0)
      if ($vec1->Compare($vec2) <  0)
      if ($vec1->Compare($vec2) <= 0)
      if ($vec1->Compare($vec2) >  0)
      if ($vec1->Compare($vec2) >= 0)

  to_Hex
      $string = $vector->to_Hex();

  from_Hex
      $vector->from_Hex($string);

  to_Bin
      $string = $vector->to_Bin();

  from_Bin
      $vector->from_Bin($string);

  to_Dec
      $string = $vector->to_Dec();

  from_Dec
      $vector->from_Dec($string);

  to_Enum
      $string = $vector->to_Enum();  #  e.g. "2,3,5-7,11,13-19"

  from_Enum
      $vector->from_Enum($string);

  Bit_Off
      $vector->Bit_Off($index);

  Bit_On
      $vector->Bit_On($index);

  bit_flip
      $bit = $vector->bit_flip($index);

  bit_test, contains
      $bit = $vector->bit_test($index);
      $bit = $vector->contains($index);
      if ($vector->bit_test($index))
      if ($vector->contains($index))

  Bit_Copy
      $vector->Bit_Copy($index,$bit);

  LSB (least significant bit)
      $vector->LSB($bit);

  MSB (most significant bit)
      $vector->MSB($bit);

  lsb (least significant bit)
      $bit = $vector->lsb();

  msb (most significant bit)
      $bit = $vector->msb();

  rotate_left
      $carry = $vector->rotate_left();

  rotate_right
      $carry = $vector->rotate_right();

  shift_left
      $carry = $vector->shift_left($carry);

  shift_right
      $carry = $vector->shift_right($carry);

  Move_Left
      $vector->Move_Left($bits);  #  shift left "$bits" positions

  Move_Right
      $vector->Move_Right($bits);  #  shift right "$bits" positions

  Insert
      $vector->Insert($offset,$bits);

  Delete
      $vector->Delete($offset,$bits);

  increment
      $carry = $vector->increment();

  decrement
      $carry = $vector->decrement();

  add
      $carry = $vec3->add($vec1,$vec2,$carry);

  subtract
      $carry = $vec3->subtract($vec1,$vec2,$carry);

  Negate
      $vec2->Negate($vec1);

  Absolute
      $vec2->Absolute($vec1);

  Sign
      if ($vector->Sign() == 0)
      if ($vector->Sign() != 0)
      if ($vector->Sign() <  0)
      if ($vector->Sign() <= 0)
      if ($vector->Sign() >  0)
      if ($vector->Sign() >= 0)

  Multiply
      $vec3->Multiply($vec1,$vec2);

  Divide
      $quot->Divide($vec1,$vec2,$rest);

  GCD (Greatest Common Divisor)
      $vec3->GCD($vec1,$vec2);

  Block_Store
      $vector->Block_Store($buffer);

  Block_Read
      $buffer = $vector->Block_Read();

  Word_Size
      $size = $vector->Word_Size();  #  number of words in "$vector"

  Word_Store
      $vector->Word_Store($offset,$word);

  Word_Read
      $word = $vector->Word_Read($offset);

  Word_List_Store
      $vector->Word_List_Store(@words);

  Word_List_Read
      @words = $vector->Word_List_Read();

  Word_Insert
      $vector->Word_Insert($offset,$count);

  Word_Delete
      $vector->Word_Delete($offset,$count);

  Chunk_Store
      $vector->Chunk_Store($chunksize,$offset,$chunk);

  Chunk_Read
      $chunk = $vector->Chunk_Read($chunksize,$offset);

  Chunk_List_Store
      $vector->Chunk_List_Store($chunksize,@chunks);

  Chunk_List_Read
      @chunks = $vector->Chunk_List_Read($chunksize);

  Index_List_Remove
      $vector->Index_List_Remove(@indices);

  Index_List_Store
      $vector->Index_List_Store(@indices);

  Index_List_Read
      @indices = $vector->Index_List_Read();

  Union
      $set3->Union($set1,$set2);

  Intersection
      $set3->Intersection($set1,$set2);

  Difference
      $set3->Difference($set1,$set2);

  ExclusiveOr
      $set3->ExclusiveOr($set1,$set2);

  Complement
      $set2->Complement($set1);

  subset
      if ($set1->subset($set2))  #  true if $set1 is subset of $set2

  Norm
      $norm = $set->Norm();

  Min
      $min = $set->Min();

  Max
      $max = $set->Max();

  Multiplication
      $matrix3->Multiplication($rows3,$cols3,
                      $matrix1,$rows1,$cols1,
                      $matrix2,$rows2,$cols2);

  Closure
      $matrix->Closure($rows,$cols);

  Transpose
      $matrix2->Transpose($rows2,$cols2,$matrix1,$rows1,$cols1);

=head2 CLASS METHODS (implemented in Perl)

  Configuration
      $config = Bit::Vector->Configuration();
      Bit::Vector->Configuration($config);
      $oldconfig = Bit::Vector->Configuration($newconfig);

=head2 OVERLOADED OPERATORS (implemented in Perl)

  String Conversion
      $string = "$vector";             #  depending on configuration
      print "\$vector = '$vector'\n";

  Emptyness
      if ($vector)  #  if not empty (non-zero)
      if (! $vector)  #  if empty (zero)
      unless ($vector)  #  if empty (zero)

  Complement (one's complement)
      $vector2 = ~$vector1;
      $vector = ~$vector;

  Negation (two's complement)
      $vector2 = -$vector1;
      $vector = -$vector;

  Norm
      $norm = abs($vector);  #  depending on configuration

  Absolute
      $vector2 = abs($vector1);  #  depending on configuration

  Concatenation
      $vector3 = $vector1 . $vector2;
      $vector1 .= $vector2;
      $vector1 = $vector2 . $vector1;
      $vector2 = $vector1 . $scalar;  #  depending on configuration
      $vector2 = $scalar . $vector1;
      $vector .= $scalar;

  Duplication
      $vector2 = $vector1 x $factor;
      $vector x= $factor;

  Shift Left
      $vector2 = $vector1 << $bits;
      $vector <<= $bits;

  Shift Right
      $vector2 = $vector1 >> $bits;
      $vector >>= $bits;

  Union
      $vector3 = $vector1 | $vector2;
      $vector1 |= $vector2;
      $vector2 = $vector1 | $scalar;
      $vector |= $scalar;

      $vector3 = $vector1 + $vector2;  #  depending on configuration
      $vector1 += $vector2;
      $vector2 = $vector1 + $scalar;
      $vector += $scalar;

  Intersection
      $vector3 = $vector1 & $vector2;
      $vector1 &= $vector2;
      $vector2 = $vector1 & $scalar;
      $vector &= $scalar;

      $vector3 = $vector1 * $vector2;  #  depending on configuration
      $vector1 *= $vector2;
      $vector2 = $vector1 * $scalar;
      $vector *= $scalar;

  ExclusiveOr
      $vector3 = $vector1 ^ $vector2;
      $vector1 ^= $vector2;
      $vector2 = $vector1 ^ $scalar;
      $vector ^= $scalar;

  Set Difference
      $vector3 = $vector1 - $vector2;  #  depending on configuration
      $vector1 -= $vector2;
      $vector1 = $vector2 - $vector1;
      $vector2 = $vector1 - $scalar;
      $vector2 = $scalar - $vector1;
      $vector -= $scalar;

  Addition
      $vector3 = $vector1 + $vector2;  #  depending on configuration
      $vector1 += $vector2;
      $vector2 = $vector1 + $scalar;
      $vector += $scalar;

  Subtraction
      $vector3 = $vector1 - $vector2;  #  depending on configuration
      $vector1 -= $vector2;
      $vector1 = $vector2 - $vector1;
      $vector2 = $vector1 - $scalar;
      $vector2 = $scalar - $vector1;
      $vector -= $scalar;

  Multiplication
      $vector3 = $vector1 * $vector2;  #  depending on configuration
      $vector1 *= $vector2;
      $vector2 = $vector1 * $scalar;
      $vector *= $scalar;

  Division
      $vector3 = $vector1 / $vector2;
      $vector1 /= $vector2;
      $vector1 = $vector2 / $vector1;
      $vector2 = $vector1 / $scalar;
      $vector2 = $scalar / $vector1;
      $vector /= $scalar;

  Modulo
      $vector3 = $vector1 % $vector2;
      $vector1 %= $vector2;
      $vector1 = $vector2 % $vector1;
      $vector2 = $vector1 % $scalar;
      $vector2 = $scalar % $vector1;
      $vector %= $scalar;

  Increment
      ++$vector;
      $vector++;

  Decrement
      --$vector;
      $vector--;

  Lexical Comparison (unsigned)
      $cmp = $vector1 cmp $vector2;
      if ($vector1 lt $vector2)
      if ($vector1 le $vector2)
      if ($vector1 gt $vector2)
      if ($vector1 ge $vector2)

      $cmp = $vector cmp $scalar;
      if ($vector lt $scalar)
      if ($vector le $scalar)
      if ($vector gt $scalar)
      if ($vector ge $scalar)

  Comparison (signed)
      $cmp = $vector1 <=> $vector2;
      if ($vector1 < $vector2)  #  depending on configuration
      if ($vector1 <= $vector2)
      if ($vector1 > $vector2)
      if ($vector1 >= $vector2)

      $cmp = $vector <=> $scalar;
      if ($vector < $scalar)  #  depending on configuration
      if ($vector <= $scalar)
      if ($vector > $scalar)
      if ($vector >= $scalar)

  Equality
      if ($vector1 eq $vector2)
      if ($vector1 ne $vector2)
      if ($vector eq $scalar)
      if ($vector ne $scalar)

      if ($vector1 == $vector2)
      if ($vector1 != $vector2)
      if ($vector == $scalar)
      if ($vector != $scalar)

  Subset Relationship
      if ($vector1 <= $vector2)  #  depending on configuration

  True Subset Relationship
      if ($vector1 < $vector2)  #  depending on configuration

  Superset Relationship
      if ($vector1 >= $vector2)  #  depending on configuration

  True Superset Relationship
      if ($vector1 > $vector2)  #  depending on configuration

=head1 IMPORTANT NOTES

=over 2

=item *

Method naming conventions

Method names completely in lower case indicate a boolean return value.

(Except for the bit vector constructor method "C<new()>", of course.)

=item *

Boolean values

Boolean values in this module are always a numeric zero ("C<0>") for
"false" and a numeric one ("C<1>") for "true".

=item *

Negative numbers

All numeric input parameters passed to any of the methods of this module
or (in general, but see below) to overloaded operators are regarded as
being B<UNSIGNED>.

This affects overloaded operators only where numeric factors (as with
"C<E<lt>E<lt>>", "C<E<gt>E<gt>>" and "C<x>") are concerned or when the
configuration (see also the section "Overloaded operators configuration"
immediately below) states that numeric input (which comes in place of one
of the two bit vector object operands the overloaded operator expects) is
to be regarded as bit indices (which is the default, however).

As a consequence, whenever you pass a negative number as an argument to
some method of this module (or an overloaded operator - under the conditions
explained above), it will be treated as a (usually very large) positive
number due to its internal two's complement binary representation, usually
resulting in an "index out of range" error message and program abortion.

Note that this does not apply to "big integer" decimal numbers, which
are (usually) passed as strings, and which may of course be negative
(see also the section "Big integers" a little further below).

=item *

Overloaded operators configuration

Note that the behaviour of certain overloaded operators can be changed
in various ways by means of the "C<Configuration()>" method (for more
details, see the description of this method further below).

For instance, scalars (i.e., numbers and strings) provided as operands
to overloaded operators are automatically converted to bit vectors,
internally.

These scalars are thereby automatically assumed to be indices or to be
in hexadecimal, binary, decimal or enumeration format, depending on the
configuration.

Similarly, when converting bit vectors to strings using double quotes
(""), the output format will also depend on the previously chosen
configuration.

Finally, some overloaded operators may have different semantics depending
on the proper configuration; for instance, the operator "+" can be the
"union" operator from set theory or the arithmetic "add" operator.

In all cases (input, output and operator semantics), the defaults have
been chosen in such a way so that the behaviour of the module is backward
compatible with previous versions.

=item *

"Big integers"

As long as "big integers" (for "big integer" arithmetic) are small enough
so that Perl doesn't need scientific notation (exponents) to be able to
represent them internally, you can provide these "big integer" constants
to the overloaded operators of this module (or to the method "C<from_Dec()>")
in numeric form (i.e., either as a numeric constant or expression or as a Perl
variable containing a numeric value).

Note that you will get an error message (resulting in program abortion)
if your "big integer" numbers exceed that limit.

Because this limit is machine-dependent and not obvious to find out,
it is strongly recommended that you enclose B<ALL> your "big integer"
constants in your programs in (double or single) quotes.

Examples:

    $vector /= 10;  #  ok because number is small

    $vector /= -10;  #  ok for same reason

    $vector /= "10";  #  always correct

    $vector += "1152921504606846976";  #  quotes probably required here

All examples assume

    Bit::Vector->Configuration("input=decimal");

having been set beforehand.

Note also that this module does not support scientific notation (exponents)
for "big integer" decimal numbers because you can always make the bit vector
large enough for the whole number to fit without loss of precision (as it
would occur if scientific notation were used).

Finally, note that the only characters allowed in "big integer" constant
strings are the digits C<0..9> and an optional leading sign ("C<+>" or "C<->").

All other characters produce a syntax error.

=item *

Valid operands for overloaded operators

All overloaded operators expect at least one bit vector operand,
in order for the operator to "know" that not the usual operation
is to be carried out, but rather the overloaded variant.

This is especially true for all unary operators:

                    "$vector"
                    if ($vector)
                    if (!$vector)
                    ~$vector
                    -$vector
                    abs($vector)
                    ++$vector
                    $vector++
                    --$vector
                    $vector--

For obvious reasons the left operand (the "lvalue") of all
assignment operators is also required to be a bit vector:

                        .=
                        x=
                        <<=
                        >>=
                        |=
                        &=
                        ^=
                        +=
                        -=
                        *=
                        /=
                        %=

In the case of three special operators, namely "C<E<lt>E<lt>>", "C<E<gt>E<gt>>" and "C<x>",
as well as their related assignment variants, "C<E<lt>E<lt>=>", "C<E<gt>E<gt>=>" and "C<x=>",
the left operand is B<ALWAYS> a bit vector and the right operand
is B<ALWAYS> a number (which is the factor indicating how many times
the operator is to be applied).

In all truly binary operators, i.e.,

                        .
                        |
                        &
                        ^
                        +
                        -
                        *
                        /
                        %
                    <=>   cmp
                     ==    eq
                     !=    ne
                     <     lt
                     <=    le
                     >     gt
                     >=    ge

one of either operands may be replaced by a Perl scalar, i.e.,
a number or a string, either as a Perl constant, a Perl expression
or a Perl variable yielding a number or a string.

The same applies to the right side operand (the "rvalue") of the
remaining assignment operators, i.e.,

                        .=
                        |=
                        &=
                        ^=
                        +=
                        -=
                        *=
                        /=
                        %=

Note that this Perl scalar should be of the correct type, i.e.,
numeric or string, for the chosen configuration, because otherwise
a warning message will occur if your program runs under the "C<-w>"
switch of Perl.

The acceptable scalar types for each possible configuration are
the following:

    input = bit indices    (default)  :    numeric
    input = hexadecimal               :    string
    input = binary                    :    string
    input = decimal                   :    string     (in general)
    input = decimal                   :    numeric    (if small enough)
    input = enumeration               :    string

NOTE ALSO THAT THESE SCALAR OPERANDS ARE CONVERTED TO BIT VECTORS OF
THE SAME SIZE AS THE BIT VECTOR WHICH IS THE OTHER OPERAND.

The only exception from this rule is the concatenation operator
("C<.>") and its assignment variant ("C<.=>"):

If one of the two operands of the concatenation operator ("C<.>") is
not a bit vector object but a Perl scalar, the contents of the remaining
bit vector operand are converted into a string (the format of which
depends on the configuration set with the "C<Configuration()>" method),
which is then concatenated in the proper order (i.e., as indicated by the
order of the two operands) with the Perl scalar (in other words, a string
is returned in such a case instead of a bit vector object!).

If the right side operand (the "rvalue") of the assignment variant
("C<.=>") of the concatenation operator is a Perl scalar, it is converted
internally to a bit vector of the same size as the left side operand provided
that the configuration states that scalars are to be regarded as indices,
decimal strings or enumerations.

If the configuration states that scalars are to be regarded as hexadecimal
or boolean strings, however, these strings are converted to bit vectors of
a size matching the length of the input string, i.e., four times the length
for hexadecimal strings (because each hexadecimal digit is worth 4 bits) and
once the length for binary strings.

If a decimal number ("big integer") is too large to be stored in a
bit vector of the given size, a "numeric overflow error" occurs.

If a bit index is out of range for the given bit vector, an "index
out of range" error occurs.

If a scalar operand cannot be converted successfully due to invalid
syntax, a fatal "input string syntax error" is issued.

If the two operands of the operator "C<E<lt>E<lt>>", "C<E<gt>E<gt>>"
or "C<x>" are reversed, a fatal "reversed operands error" occurs.

If an operand is neither a bit vector nor a scalar, then a fatal
"illegal operand type error" occurs.

=item *

Bit order

Note that bit vectors are stored least order bit and least order word first
internally.

I.e., bit #0 of any given bit vector corresponds to bit #0 of word #0 in the
array of machine words representing the bit vector.

(Where word #0 comes first in memory, i.e., it is stored at the least memory
address in the allocated block of memory holding the given bit vector.)

Note however that machine words can be stored least order byte first or last,
depending on your system's implementation.

When you are exporting or importing a whole bit vector at once using the
methods "C<Block_Read()>" and "C<Block_Store()>" (the only time in this
module where this could make any difference), however, a conversion to and
from "least order byte first" order is automatically supplied.

In other words, what "C<Block_Read()>" provides and what "C<Block_Store()>"
expects is always in "least order byte first" order, regardless of the order
in which words are stored internally on your machine.

This is to make sure that what you export on one machine using "C<Block_Read()>"
can always be read in correctly with "C<Block_Store()>" on a different machine.

Note further that whenever bit vectors are converted to and from (binary or
hexadecimal) strings, the B<RIGHTMOST> bit is always the B<LEAST SIGNIFICANT> one,
and the B<LEFTMOST> bit is always the B<MOST SIGNIFICANT> bit.

This is because in our western culture, numbers are always represented in this
way (least significant to most significant digits go from right to left).

Of course this requires an internal reversion of order, which the corresponding
conversion methods perform automatically (without any additional overhead).

=item *

"Word" related methods

Note that all methods whose names begin with "C<Word_>" are B<MACHINE-DEPENDENT>!

They depend on the size (number of bits) of an "unsigned int" (C type) on
your machine.

Therefore, you should only use these methods if you are B<ABSOLUTELY CERTAIN>
that portability of your code is not an issue!

Note that you can use arbitrarily large chunks (i.e., fragments of bit vectors)
of up to 32 bits B<IN A PORTABLE WAY> using the methods whose names begin with
"C<Chunk_>".

=item *

Chunk sizes

Note that legal chunk sizes for all methods whose names begin with "C<Chunk_>"
range from "C<1>" to "C<Bit::Vector-E<gt>Long_Bits();>" bits ("C<0>" is B<NOT>
allowed!).

In order to make your programs portable, however, you shouldn't use chunk sizes
larger than 32 bits, since this is the minimum size of an "unsigned long" (C type)
on all systems, as prescribed by S<ANSI C>.

=item *

Matching sizes

In general, for methods involving several bit vectors at the same time, all
bit vector arguments must have identical sizes (number of bits), or a fatal
"size mismatch" error will occur.

Exceptions from this rule are the methods "C<Concat()>", "C<Concat_List()>",
"C<Interval_Copy()>" and "C<Interval_Substitute()>", where no conditions at
all are imposed on the size of their bit vector arguments, and the method
"C<Multiply()>", where all three bit vector arguments must in principle
obey the rule of matching sizes, but where the bit vector in which the
result of the multiplication is to be stored may be larger than the two
bit vector arguments containing the factors for the multiplication.

=item *

Index ranges

All indices for any given bits must lie between "C<0>" and "C<$vector-E<gt>Size()-1>",
or a fatal "index out of range" error will occur.

=back

=head1 DESCRIPTION

=head2 CLASS METHODS (implemented in C)

=over 2

=item *

C<$version = Bit::Vector-E<gt>Version();>

Returns the current version number of this module.

=item *

C<$bits = Bit::Vector-E<gt>Word_Bits();>

Returns the number of bits of an "unsigned int" (C type)
on your machine.

(An "unsigned int" is also called a "machine word",
hence the name of this method.)

=item *

C<$bits = Bit::Vector-E<gt>Long_Bits();>

Returns the number of bits of an "unsigned long" (C type)
on your machine.

=item *

C<$vector = Bit::Vector-E<gt>new($bits);>

This is the bit vector constructor method.

Call this method to create a new bit vector containing "C<$bits>"
bits (with indices ranging from "C<0>" to "C<$bits-1>").

Note that - in contrast to previous versions - bit vectors
of length zero (i.e., with C<$bits = 0>) are permitted now.

The method returns a reference to the newly created bit vector.

A new bit vector is always initialized so that all bits are cleared
(turned off).

An exception will be raised if the method is unable to allocate the
necessary memory.

Note that if you specify a negative number for "C<$bits>" it will be
interpreted as a large positive number due to its internal two's
complement binary representation.

In such a case, the bit vector constructor method will obediently attempt
to create a bit vector of that size, probably resulting in an exception,
as explained above.

=item *

C<$vector = Bit::Vector-E<gt>Concat_List(@vectors);>

This method creates a new vector containing all bit vectors from the
argument list in concatenated form.

The argument list may contain any number of arguments (including
zero); the only condition is that all arguments must be bit vectors.

There is no condition concerning the length (in number of bits) of
these arguments.

The vectors from the argument list are not changed in any way.

If the argument list is empty or if all arguments have length zero,
the resulting bit vector will also have length zero.

Note that the B<RIGHTMOST> bit vector from the argument list will
become the B<LEAST> significant part of the resulting bit vector,
and the B<LEFTMOST> bit vector from the argument list will
become the B<MOST> significant part of the resulting bit vector.

=back

=head2 OBJECT METHODS (implemented in C)

=over 2

=item *

C<$vec2 = $vec1-E<gt>new($bits);>

This is an alternative way of calling the bit vector constructor method.

Vector "C<$vec1>" is not affected by this, it just serves as an anchor
for the method invocation mechanism.

In fact B<ALL> class methods in this module can be called this way,
even though this is probably considered to be "politically incorrect"
by OO ("object-orientation") aficionados. ;-)

So even if you are too lazy to type "C<Bit::Vector-E<gt>>" instead of
"C<$vec1-E<gt>>" (and even though laziness is - allegedly - a programmer's
virtue C<:-)>), maybe it is better not to use this feature if you don't
want to get booed at. ;-)

=item *

C<$vec2 = $vec1-E<gt>Shadow();>

Creates a B<NEW> bit vector "C<$vec2>" of the B<SAME SIZE> as "C<$vec1>"
but which is B<EMPTY>.

Just like a shadow that has the same shape as the object it
originates from, but is flat and has no volume, i.e., contains
nothing.

=item *

C<$vec2 = $vec1-E<gt>Clone();>

Creates a B<NEW> bit vector "C<$vec2>" of the B<SAME SIZE> as "C<$vec1>"
which is an B<EXACT COPY> of "C<$vec1>".

=item *

C<$vector = $vec1-E<gt>Concat($vec2);>

This method returns a new bit vector object which is the result of the
concatenation of the contents of "C<$vec1>" and "C<$vec2>".

Note that the contents of "C<$vec1>" become the B<MOST> significant part
of the resulting bit vector, and "C<$vec2>" the B<LEAST> significant part.

If both bit vector arguments have length zero, the resulting bit vector
will also have length zero.

=item *

C<$vector = $vec1-E<gt>Concat_List($vec2,$vec3,...);>

This is an alternative way of calling this (class) method as an
object method.

The method returns a new bit vector object which is the result of
the concatenation of the contents of C<$vec1 . $vec2 . $vec3 . ...>

See the section "class methods" above for a detailed description of
this method.

Note that the argument list may be empty and that all arguments
must be bit vectors if it isn't.

=item *

C<$bits = $vector-E<gt>Size();>

Returns the size (number of bits) the given vector was created with
(or "C<Resize()>"d to).

=item *

C<$vector-E<gt>Resize($bits);>

Changes the size of the given vector to the specified number of bits.

This method allows you to change the size of an existing bit vector,
preserving as many bits from the old vector as will fit into the
new one (i.e., all bits with indices smaller than the minimum of the
sizes of both vectors, old and new).

If the number of machine words needed to store the new vector is smaller
than or equal to the number of words needed to store the old vector, the
memory allocated for the old vector is reused for the new one, and only
the relevant book-keeping information is adjusted accordingly.

This means that even if the number of bits increases, new memory is not
necessarily being allocated (i.e., if the old and the new number of bits
fit into the same number of machine words).

If the number of machine words needed to store the new vector is greater
than the number of words needed to store the old vector, new memory is
allocated for the new vector, the old vector is copied to the new one,
the remaining bits in the new vector are cleared (turned off) and the old
vector is deleted, i.e., the memory that was allocated for it is released.

(An exception will be raised if the method is unable to allocate the
necessary memory for the new vector.)

As a consequence, if you decrease the size of a given vector so that
it will use fewer machine words, and increase it again later so that it
will use more words than immediately before but still less than the
original vector, new memory will be allocated anyway because the
information about the size of the original vector is lost whenever
you resize it.

Note also that if you specify a negative number for "C<$bits>" it will
be interpreted as a large positive number due to its internal two's
complement binary representation.

In such a case, "Resize()" will obediently attempt to create a bit
vector of that size, probably resulting in an exception, as explained
above.

Finally, note that - in contrast to previous versions - resizing a bit
vector to a size of zero bits (length zero) is now permitted.

=item *

C<$vec2-E<gt>Copy($vec1);>

Copies the contents of bit vector "C<$vec1>" to
bit vector "C<$vec2>".

The previous contents of bit vector "C<$vec2>"
get overwritten, i.e., are lost.

Both vectors must exist beforehand, i.e., this method
does not B<CREATE> any new bit vector object.

=item *

C<$vector-E<gt>Empty();>

Clears all bits in the given vector.

=item *

C<$vector-E<gt>Fill();>

Sets all bits in the given vector.

=item *

C<$vector-E<gt>Flip();>

Flips (i.e., complements) all bits in the given vector.

=item *

C<$vector-E<gt>Primes();>

Clears the given bit vector and sets all bits whose
indices are prime numbers.

This method uses the algorithm known as the "Sieve of
Erathostenes" internally.

=item *

C<$vec2-E<gt>Reverse($vec1);>

This method copies the given vector "C<$vec1>" to
the vector "C<$vec2>", thereby reversing the order
of all bits.

I.e., the least significant bit of "C<$vec1>" becomes the
most significant bit of "C<$vec2>", whereas the most
significant bit of "C<$vec1>" becomes the least
significant bit of "C<$vec2>", and so forth
for all bits in between.

Note that in-place processing is also possible, i.e.,
"C<$vec1>" and "C<$vec2>" may be identical.

(Internally, this is the same as
C<$vec1-E<gt>Interval_Reverse(0,$vec1-E<gt>Size()-1);>.)

=item *

C<$vector-E<gt>Interval_Empty($min,$max);>

Clears all bits in the interval C<[$min..$max]> (including both limits)
in the given vector.

"C<$min>" and "C<$max>" may have the same value; this is the same
as clearing a single bit with "C<Bit_Off()>" (but less efficient).

Note that C<$vector-E<gt>Interval_Empty(0,$vector-E<gt>Size()-1);>
is the same as C<$vector-E<gt>Empty();> (but less efficient).

=item *

C<$vector-E<gt>Interval_Fill($min,$max);>

Sets all bits in the interval C<[$min..$max]> (including both limits)
in the given vector.

"C<$min>" and "C<$max>" may have the same value; this is the same
as setting a single bit with "C<Bit_On()>" (but less efficient).

Note that C<$vector-E<gt>Interval_Fill(0,$vector-E<gt>Size()-1);>
is the same as C<$vector-E<gt>Fill();> (but less efficient).

=item *

C<$vector-E<gt>Interval_Flip($min,$max);>

Flips (i.e., complements) all bits in the interval C<[$min..$max]>
(including both limits) in the given vector.

"C<$min>" and "C<$max>" may have the same value; this is the same
as flipping a single bit with "C<bit_flip()>" (but less efficient).

Note that C<$vector-E<gt>Interval_Flip(0,$vector-E<gt>Size()-1);>
is the same as C<$vector-E<gt>Flip();> and
C<$vector-E<gt>Complement($vector);>
(but less efficient).

=item *

C<$vector-E<gt>Interval_Reverse($min,$max);>

Reverses the order of all bits in the interval C<[$min..$max]>
(including both limits) in the given vector.

I.e., bits "C<$min>" and "C<$max>" swap places, and so forth
for all bits in between.

"C<$min>" and "C<$max>" may have the same value; this has no
effect whatsoever, though.

=item *

C<if (($min,$max) = $vector-E<gt>Interval_Scan_inc($start))>

Returns the minimum and maximum indices of the next contiguous block
of set bits (i.e., bits in the "on" state).

The search starts at index "C<$start>" (i.e., C<"$min" E<gt>= "$start">)
and proceeds upwards (i.e., C<"$max" E<gt>= "$min">), thus repeatedly
increments the search pointer "C<$start>" (internally).

Note though that the contents of the variable (or scalar literal value)
"C<$start>" is B<NOT> altered. I.e., you have to set it to the desired
value yourself prior to each call to "C<Interval_Scan_inc()>" (see also
the example given below).

Actually, the bit vector is not searched bit by bit, but one machine
word at a time, in order to speed up execution (which means that this
method is quite efficient).

An empty list is returned if no such block can be found.

Note that a single set bit (surrounded by cleared bits) is a valid
block by this definition. In that case the return values for "C<$min>"
and "C<$max>" are the same.

Typical use:

    $start = 0;
    while (($start < $vector->Size()) &&
        (($min,$max) = $vector->Interval_Scan_inc($start)))
    {
        $start = $max + 2;

        # do something with $min and $max
    }

=item *

C<if (($min,$max) = $vector-E<gt>Interval_Scan_dec($start))>

Returns the minimum and maximum indices of the next contiguous block
of set bits (i.e., bits in the "on" state).

The search starts at index "C<$start>" (i.e., C<"$max" E<lt>= "$start">)
and proceeds downwards (i.e., C<"$min" E<lt>= "$max">), thus repeatedly
decrements the search pointer "C<$start>" (internally).

Note though that the contents of the variable (or scalar literal value)
"C<$start>" is B<NOT> altered. I.e., you have to set it to the desired
value yourself prior to each call to "C<Interval_Scan_dec()>" (see also
the example given below).

Actually, the bit vector is not searched bit by bit, but one machine
word at a time, in order to speed up execution (which means that this
method is quite efficient).

An empty list is returned if no such block can be found.

Note that a single set bit (surrounded by cleared bits) is a valid
block by this definition. In that case the return values for "C<$min>"
and "C<$max>" are the same.

Typical use:

    $start = $vector->Size() - 1;
    while (($start >= 0) &&
        (($min,$max) = $vector->Interval_Scan_dec($start)))
    {
        $start = $min - 2;

        # do something with $min and $max
    }

=item *

C<$vec2-E<gt>Interval_Copy($vec1,$offset2,$offset1,$length);>

This method allows you to copy a stretch of contiguous bits (starting
at any position "C<$offset1>" you choose, with a length of "C<$length>"
bits) from a given "source" bit vector "C<$vec1>" to another position
"C<$offset2>" in a "target" bit vector "C<$vec2>".

Note that the two bit vectors "C<$vec1>" and "C<$vec2>" do B<NOT>
need to have the same (matching) size!

Consequently, any of the two terms "C<$offset1 + $length>" and
"C<$offset2 + $length>" (or both) may exceed the actual length
of its corresponding bit vector ("C<$vec1-E<gt>Size()>" and
"C<$vec2-E<gt>Size()>", respectively).

In such a case, the "C<$length>" parameter is automatically reduced
internally so that both terms above are bounded by the number of bits
of their corresponding bit vector.

This may even result in a length of zero, in which case nothing is
copied at all.

(Of course the value of the "C<$length>" parameter, supplied by you
in the initial method call, may also be zero right from the start!)

Note also that "C<$offset1>" and "C<$offset2>" must lie within the
range "C<0>" and, respectively, "C<$vec1-E<gt>Size()-1>" or
"C<$vec2-E<gt>Size()-1>", or a fatal "offset out of range" error
will occur.

Note further that "C<$vec1>" and "C<$vec2>" may be identical, i.e.,
you may copy a stretch of contiguous bits from one part of a given
bit vector to another part.

The source and the target interval may even overlap, in which case
the copying is automatically performed in ascending or descending
order (depending on the direction of the copy - downwards or upwards
in the bit vector, respectively) to handle this situation correctly,
i.e., so that no bits are being overwritten before they have been
copied themselves.

=item *

C<$vec2-E<gt>Interval_Substitute($vec1,$off2,$len2,$off1,$len1);>

This method is (roughly) the same for bit vectors (i.e., arrays
of booleans) as what the "splice" function in Perl is for lists
(i.e., arrays of scalars).

(See L<perlfunc/splice> for more details about this function.)

The method allows you to substitute a stretch of contiguous bits
(defined by a position (offset) "C<$off1>" and a length of "C<$len1>"
bits) from a given "source" bit vector "C<$vec1>" for a different
stretch of contiguous bits (defined by a position (offset) "C<$off2>"
and a length of "C<$len2>" bits) in another, "target" bit vector
"C<$vec2>".

Note that the two bit vectors "C<$vec1>" and "C<$vec2>" do B<NOT>
need to have the same (matching) size!

Note further that "C<$off1>" and "C<$off2>" must lie within the
range "C<0>" and, respectively, "C<$vec1-E<gt>Size()>" or
"C<$vec2-E<gt>Size()>", or a fatal "offset out of range" error
will occur.

Alert readers will have noticed that these upper limits are B<NOT>
"C<$vec1-E<gt>Size()-1>" and "C<$vec2-E<gt>Size()-1>", as they would
be for any other method in this module, but that these offsets may
actually point to one position B<PAST THE END> of the corresponding
bit vector.

This is necessary in order to make it possible to B<APPEND> a given
stretch of bits to the target bit vector instead of B<REPLACING>
something in it.

For reasons of symmetry and generality, the same applies to the offset
in the source bit vector, even though such an offset (one position past
the end of the bit vector) does not serve any practical purpose there
(but does not cause any harm either).

(Actually this saves you from the need of testing for this special case,
in certain circumstances.)

Note that whenever the term "C<$off1 + $len1>" exceeds the size
"C<$vec1-E<gt>Size()>" of bit vector "C<$vec1>" (or if "C<$off2 + $len2>"
exceeds "C<$vec2-E<gt>Size()>"), the corresponding length ("C<$len1>"
or "C<$len2>", respectively) is automatically reduced internally
so that "C<$off1 + $len1 E<lt>= $vec1-E<gt>Size()>" (and
"C<$off2 + $len2 E<lt>= $vec2-E<gt>Size()>") holds.

(Note that this does B<NOT> alter the intended result, even though
this may seem counter-intuitive at first!)

This may even result in a length ("C<$len1>" or "C<$len2>") of zero.

A length of zero for the interval in the B<SOURCE> bit vector
("C<$len1 == 0>") means that the indicated stretch of bits in
the target bit vector (starting at position "C<$off2>") is to
be replaced by B<NOTHING>, i.e., is to be B<DELETED>.

A length of zero for the interval in the B<TARGET> bit vector
("C<$len2> == 0") means that B<NOTHING> is replaced, and that the
stretch of bits from the source bit vector is simply B<INSERTED>
into the target bit vector at the indicated position ("C<$off2>").

If both length parameters are zero, nothing is done at all.

Note that in contrast to any other method in this module (especially
"C<Interval_Copy()>", "C<Insert()>" and "C<Delete()>"), this method
B<IMPLICITLY> and B<AUTOMATICALLY> adapts the length of the resulting
bit vector as needed, as given by

        $size = $vec2->Size();   #  before
        $size += $len1 - $len2;  #  after

(The only other method in this module that changes the size of a bit
vector is the method "C<Resize()>".)

In other words, replacing a given interval of bits in the target bit
vector with a longer or shorter stretch of bits from the source bit
vector, or simply inserting ("C<$len2 == 0>") a stretch of bits into
or deleting ("C<$len1 == 0>") an interval of bits from the target bit
vector will automatically increase or decrease, respectively, the size
of the target bit vector accordingly.

For the sake of generality, this may even result in a bit vector with
a size of zero (containing no bits at all).

This is also the reason why bit vectors of length zero are permitted
in this module in the first place, starting with version 5.0.

Finally, note that "C<$vec1>" and "C<$vec2>" may be identical, i.e.,
in-place processing is possible.

(If you think about that for a while or if you look at the code,
you will see that this is far from trivial!)

=item *

C<if ($vector-E<gt>is_empty())>

Tests wether the given bit vector is empty, i.e., wether B<ALL> of
its bits are cleared (in the "off" state).

In "big integer" arithmetic, this is equivalent to testing wether
the number stored in the bit vector is zero ("C<0>").

Returns "true" ("C<1>") if the bit vector is empty and "false" ("C<0>")
otherwise.

Note that this method also returns "true" ("C<1>") if the given bit
vector has a length of zero, i.e., if it contains no bits at all.

=item *

C<if ($vector-E<gt>is_full())>

Tests wether the given bit vector is full, i.e., wether B<ALL> of
its bits are set (in the "on" state).

In "big integer" arithmetic, this is equivalent to testing wether
the number stored in the bit vector is minus one ("-1").

Returns "true" ("C<1>") if the bit vector is full and "false" ("C<0>")
otherwise.

If the given bit vector has a length of zero (i.e., if it contains
no bits at all), this method returns "false" ("C<0>").

=item *

C<if ($vec1-E<gt>equal($vec2))>

Tests the two given bit vectors for equality.

Returns "true" ("C<1>") if the two bit vectors are exact
copies of one another and "false" ("C<0>") otherwise.

=item *

C<$cmp = $vec1-E<gt>Lexicompare($vec2);>

Compares the two given bit vectors, which are
regarded as B<UNSIGNED> numbers in binary representation.

The method returns "C<-1>" if the first bit vector is smaller
than the second bit vector, "C<0>" if the two bit vectors are
exact copies of one another and "C<1>" if the first bit vector
is greater than the second bit vector.

=item *

C<$cmp = $vec1-E<gt>Compare($vec2);>

Compares the two given bit vectors, which are
regarded as B<SIGNED> numbers in binary representation.

The method returns "C<-1>" if the first bit vector is smaller
than the second bit vector, "C<0>" if the two bit vectors are
exact copies of one another and "C<1>" if the first bit vector
is greater than the second bit vector.

=item *

C<$string = $vector-E<gt>to_Hex();>

Returns a hexadecimal string representing the given bit vector.

Note that this representation is quite compact, in that it only
needs twice the number of bytes needed to store the bit vector
itself, internally.

Note also that since a hexadecimal digit is always worth four bits,
the length of the resulting string is always a multiple of four bits,
regardless of the true length (in bits) of the given bit vector.

Moreover, in order to simplify the conversion, the unused bits in
the bit vector (if any) are also converted, which may produce some
extra (but innocuous) leading hexadecimal zeros.

Finally, note that the B<LEAST> significant hexadecimal digit is
located at the B<RIGHT> end of the resulting string, and the B<MOST>
significant digit at the B<LEFT> end.

=item *

C<$vector-E<gt>from_Hex($string);>

Allows to read in the contents of a bit vector from a hexadecimal
string, such as returned by the method "C<to_Hex()>" (see above).

Remember that the least significant bits are always to the right of a
hexadecimal string, and the most significant bits to the left. Therefore,
the string is actually read in from right to left while the bit vector
is filled accordingly, 4 bits at a time, starting with the least significant
bits and going upward to the most significant bits.

If the given string contains less hexadecimal digits than are needed
to completely fill the given bit vector, the remaining (most significant)
bits are all cleared.

This also means that, even if the given string does not contain enough digits
to completely fill the given bit vector, the previous contents of the
bit vector are erased completely.

If the given string is longer than it needs to fill the given bit vector,
the superfluous characters are simply ignored.

(In fact they are ignored completely - they are not even checked for
proper syntax. See also below for more about that.)

This behaviour is intentional so that you may read in the string
representing one bit vector into another bit vector of different
size, i.e., as much of it as will fit.

If during the process of reading the given string any character is
encountered which is not a hexadecimal digit, a fatal syntax error
ensues ("input string syntax error").

=item *

C<$string = $vector-E<gt>to_Bin();>

Returns a binary string representing the given bit vector.

Example:

  $vector = Bit::Vector->new(8);
  $vector->Primes();
  $string = $vector->to_Bin();
  print "'$string'\n";

This prints:

  '10101100'

(Bits #7, #5, #3 and #2 are set.)

Note that the B<LEAST> significant bit is located at the B<RIGHT>
end of the resulting string, and the B<MOST> significant bit at
the B<LEFT> end.

=item *

C<$vector-E<gt>from_Bin($string);>

This method allows you to read in the contents of a bit vector from a
binary string, such as returned by the method "C<to_Bin()>" (see above).

Note that this method assumes that the B<LEAST> significant bit is located at
the B<RIGHT> end of the binary string, and the B<MOST> significant bit at the
B<LEFT> end. Therefore, the string is actually read in from right to left
while the bit vector is filled accordingly, one bit at a time, starting with
the least significant bit and going upward to the most significant bit.

If the given string contains less binary digits ("C<0>" and "C<1>") than are
needed to completely fill the given bit vector, the remaining (most significant)
bits are all cleared.

This also means that, even if the given string does not contain enough digits
to completely fill the given bit vector, the previous contents of the
bit vector are erased completely.

If the given string is longer than it needs to fill the given bit vector,
the superfluous characters are simply ignored.

(In fact they are ignored completely - they are not even checked for
proper syntax. See also below for more about that.)

This behaviour is intentional so that you may read in the string
representing one bit vector into another bit vector of different
size, i.e., as much of it as will fit.

If during the process of reading the given string any character is
encountered which is not either "C<0>" or "C<1>", a fatal syntax error
ensues ("input string syntax error").

=item *

C<$string = $vector-E<gt>to_Dec();>

This method returns a string representing the contents of the given bit
vector converted to decimal (base C<10>).

Note that this method assumes the given bit vector to be B<SIGNED> (and
to contain a number in two's complement binary representation).

Consequently, whenever the most significant bit of the given bit vector
is set, the number stored in it is regarded as being B<NEGATIVE>.

The resulting string can be fed into "C<from_Dec()>" (see below) in order
to copy the contents of this bit vector to another one (or to restore the
contents of this one). This is not advisable, though, since this would be
very inefficient (there are much more efficient methods for storing and
copying bit vectors in this module).

Note that such conversion from binary to decimal is inherently slow
since the bit vector has to be repeatedly divided by C<10> with remainder
until the quotient becomes C<0> (each remainder in turn represents a single
decimal digit of the resulting string).

This is also true for the implementation of this method in this module,
even though a considerable effort has been made to speed it up: instead of
repeatedly dividing by C<10>, the bit vector is repeatedly divided by the
largest power of C<10> that will fit into a machine word. The remainder is
then repeatedly divided by C<10> using only machine word arithmetics, which
is much faster than dividing the whole bit vector ("divide and rule" principle).

According to my own measurements, this resulted in an 8-fold speed increase
over the straightforward approach.

Still, conversion to decimal should be used only where absolutely necessary.

Keep the resulting string stored in some variable if you need it again,
instead of converting the bit vector all over again.

Beware that if you set the configuration for overloaded operators to
"output=decimal", this method will be called for every bit vector
enclosed in double quotes!

=item *

C<$vector-E<gt>from_Dec($string);>

This method allows you to convert a given decimal number, which may be
positive or negative, into two's complement binary representation, which
is then stored in the given bit vector.

The decimal number should always be provided as a string, to avoid possible
truncation (due to the limited precision of integers in Perl) or formatting
(due to Perl's use of scientific notation for large numbers), which would
lead to errors.

If the binary representation of the given decimal number is too big to fit
into the given bit vector (if the given bit vector does not contain enough
bits to hold it), a fatal "numeric overflow error" occurs.

If the input string contains other characters than decimal digits (C<0-9>)
and an optional leading sign ("C<+>" or "C<->"), a fatal "input string
syntax error" occurs.

If possible program abortion is unwanted or intolerable, use
"C<eval>", like this:

  eval { $vector->from_Dec("1152921504606846976"); };
  if ($@)
  {
      # an error occurred
  }

There are four possible error messages:

  if ($@ =~ /item is not a string/)

  if ($@ =~ /input string syntax error/)

  if ($@ =~ /numeric overflow error/)

  if ($@ =~ /unable to allocate memory/)

Note that the conversion from decimal to binary is costly in terms of
processing time, since a lot of multiplications have to be carried out
(in principle, each decimal digit must be multiplied with the binary
representation of the power of C<10> corresponding to its position in
the decimal number, i.e., 1, 10, 100, 1000, 10000 and so on).

This is not as time consuming as the opposite conversion, from binary
to decimal (where successive divisions have to be carried out, which
are even more expensive than multiplications), but still noticeable.

Again (as in the case of "C<to_Dec()>"), the implementation of this
method in this module uses the principle of "divide and rule" in order
to speed up the conversion, i.e., as many decimal digits as possible
are first accumulated (converted) in a machine word and only then
stored in the given bit vector.

Even so, use this method only where absolutely necessary if speed is
an important consideration in your application.

Beware that if you set the configuration for overloaded operators to
"input=decimal", this method will be called for every scalar operand
you use!

=item *

C<$string = $vector-E<gt>to_Enum();>

Converts the given bit vector or set into an enumeration of single
indices and ranges of indices (".newsrc" style), representing the
bits that are set ("C<1>") in the bit vector.

Example:

  $vector = Bit::Vector->new(20);
  $vector->Bit_On(2);
  $vector->Bit_On(3);
  $vector->Bit_On(11);
  $vector->Interval_Fill(5,7);
  $vector->Interval_Fill(13,19);
  print "'", $vector->to_Enum(), "'\n";

which prints

  '2,3,5-7,11,13-19'

If the given bit vector is empty, the resulting string will
also be empty.

Note, by the way, that the above example can also be written
a little handier, perhaps, as follows:

  Bit::Vector->Configuration("out=enum");
  $vector = Bit::Vector->new(20);
  $vector->Index_List_Store(2,3,5,6,7,11,13,14,15,16,17,18,19);
  print "'$vector'\n";

=item *

C<$vector-E<gt>from_Enum($string);>

This method first empties the given bit vector and then tries to
set the bits and ranges of bits specified in the given string.

The string "C<$string>" must only contain unsigned integers
or ranges of integers (two unsigned integers separated by a
dash "-"), separated by commas (",").

All other characters are disallowed (including white space!)
and will lead to a fatal "input string syntax error".

In each range, the first integer (the lower limit of the range)
must always be less than or equal to the second integer (the
upper limit), or a fatal "minimum > maximum index" error occurs.

All integers must lie in the permitted range for the given
bit vector, i.e., they must lie between "C<0>" and
"C<$vector-E<gt>Size()-1>".

If this condition is not met, a fatal "index out of range"
error occurs.

If possible program abortion is unwanted or intolerable, use
"C<eval>", like this:

  eval { $vector->from_Enum("2,3,5-7,11,13-19"); };
  if ($@)
  {
      # an error occurred
  }

There are four possible error messages:

  if ($@ =~ /item is not a string/)

  if ($@ =~ /input string syntax error/)

  if ($@ =~ /index out of range/)

  if ($@ =~ /minimum > maximum index/)

Note that the order of the indices and ranges is irrelevant,
i.e.,

  eval { $vector->from_Enum("11,5-7,3,13-19,2"); };

results in the same vector as in the example above.

Ranges and indices may also overlap.

This is because each (single) index in the string is passed
to the method "C<Bit_On()>", internally, and each range to
the method "C<Interval_Fill()>".

This means that the resulting bit vector is just the union
of all the indices and ranges specified in the given string.

=item *

C<$vector-E<gt>Bit_Off($index);>

Clears the bit with index "C<$index>" in the given vector.

=item *

C<$vector-E<gt>Bit_On($index);>

Sets the bit with index "C<$index>" in the given vector.

=item *

C<$vector-E<gt>bit_flip($index)>

Flips (i.e., complements) the bit with index "C<$index>"
in the given vector.

Moreover, this method returns the B<NEW> state of the
bit in question, i.e., it returns "C<0>" if the bit is
cleared or "C<1>" if the bit is set (B<AFTER> flipping it).

=item *

C<if ($vector-E<gt>bit_test($index))>

Returns the current state of the bit with index "C<$index>"
in the given vector, i.e., returns "C<0>" if it is cleared
(in the "off" state) or "C<1>" if it is set (in the "on" state).

=item *

C<$vector-E<gt>Bit_Copy($index,$bit);>

Sets the bit with index "C<$index>" in the given vector either
to "C<0>" or "C<1>" depending on the boolean value "C<$bit>".

=item *

C<$vector-E<gt>LSB($bit);>

Allows you to set the least significant bit in the given bit
vector to the value given by the boolean parameter "C<$bit>".

This is a (faster) shortcut for "C<$vector-E<gt>Bit_Copy(0,$bit);>".

=item *

C<$vector-E<gt>MSB($bit);>

Allows you to set the most significant bit in the given bit
vector to the value given by the boolean parameter "C<$bit>".

This is a (faster) shortcut for
"C<$vector-E<gt>Bit_Copy($vector-E<gt>Size()-1,$bit);>".

=item *

C<$bit = $vector-E<gt>lsb();>

Returns the least significant bit of the given bit vector.

This is a (faster) shortcut for "C<$bit = $vector-E<gt>bit_test(0);>".

=item *

C<$bit = $vector-E<gt>msb();>

Returns the most significant bit of the given bit vector.

This is a (faster) shortcut for
"C<$bit = $vector-E<gt>bit_test($vector-E<gt>Size()-1);>".

=item *

C<$carry_out = $vector-E<gt>rotate_left();>

  carry             MSB           vector:           LSB
   out:
  +---+            +---+---+---+---     ---+---+---+---+
  |   |  <---+---  |   |   |   |    ...    |   |   |   |  <---+
  +---+      |     +---+---+---+---     ---+---+---+---+      |
             |                                                |
             +------------------------------------------------+

The least significant bit (LSB) is the bit with index "C<0>", the most
significant bit (MSB) is the bit with index "C<$vector-E<gt>Size()-1>".

=item *

C<$carry_out = $vector-E<gt>rotate_right();>

          MSB           vector:           LSB            carry
                                                          out:
         +---+---+---+---     ---+---+---+---+           +---+
  +--->  |   |   |   |    ...    |   |   |   |  ---+---> |   |
  |      +---+---+---+---     ---+---+---+---+     |     +---+
  |                                                |
  +------------------------------------------------+

The least significant bit (LSB) is the bit with index "C<0>", the most
significant bit (MSB) is the bit with index "C<$vector-E<gt>Size()-1>".

=item *

C<$carry_out = $vector-E<gt>shift_left($carry_in);>

  carry         MSB           vector:           LSB         carry
   out:                                                      in:
  +---+        +---+---+---+---     ---+---+---+---+        +---+
  |   |  <---  |   |   |   |    ...    |   |   |   |  <---  |   |
  +---+        +---+---+---+---     ---+---+---+---+        +---+

The least significant bit (LSB) is the bit with index "C<0>", the most
significant bit (MSB) is the bit with index "C<$vector-E<gt>Size()-1>".

=item *

C<$carry_out = $vector-E<gt>shift_right($carry_in);>

  carry         MSB           vector:           LSB         carry
   in:                                                       out:
  +---+        +---+---+---+---     ---+---+---+---+        +---+
  |   |  --->  |   |   |   |    ...    |   |   |   |  --->  |   |
  +---+        +---+---+---+---     ---+---+---+---+        +---+

The least significant bit (LSB) is the bit with index "C<0>", the most
significant bit (MSB) is the bit with index "C<$vector-E<gt>Size()-1>".

=item *

C<$vector-E<gt>Move_Left($bits);>

Shifts the given bit vector left by "C<$bits>" bits, i.e., inserts "C<$bits>"
new bits at the lower end (least significant bit) of the bit vector,
moving all other bits up by "C<$bits>" places, thereby losing the "C<$bits>"
most significant bits.

The inserted new bits are all cleared (set to the "off" state).

This method does nothing if "C<$bits>" is equal to zero.

Beware that the whole bit vector is cleared B<WITHOUT WARNING> if
"C<$bits>" is greater than or equal to the size of the given bit vector!

In fact this method is equivalent to

  for ( $i = 0; $i < $bits; $i++ ) { $vector->shift_left(0); }

except that it is much more efficient (for "C<$bits>" greater than or
equal to the number of bits in a machine word on your system) than
this straightforward approach.

=item *

C<$vector-E<gt>Move_Right($bits);>

Shifts the given bit vector right by "C<$bits>" bits, i.e., deletes the
"C<$bits>" least significant bits of the bit vector, moving all other bits
down by "C<$bits>" places, thereby creating "C<$bits>" new bits at the upper
end (most significant bit) of the bit vector.

These new bits are all cleared (set to the "off" state).

This method does nothing if "C<$bits>" is equal to zero.

Beware that the whole bit vector is cleared B<WITHOUT WARNING> if
"C<$bits>" is greater than or equal to the size of the given bit vector!

In fact this method is equivalent to

  for ( $i = 0; $i < $bits; $i++ ) { $vector->shift_right(0); }

except that it is much more efficient (for "C<$bits>" greater than or
equal to the number of bits in a machine word on your system) than
this straightforward approach.

=item *

C<$vector-E<gt>Insert($offset,$bits);>

This method inserts "C<$bits>" fresh new bits at position "C<$offset>"
in the given bit vector.

The "C<$bits>" most significant bits are lost, and all bits starting
with bit number "C<$offset>" up to and including bit number
"C<$vector-E<gt>Size()-$bits-1>" are moved up by "C<$bits>" places.

The now vacant "C<$bits>" bits starting at bit number "C<$offset>"
(up to and including bit number "C<$offset+$bits-1>") are then set
to zero (cleared).

Note that this method does B<NOT> increase the size of the given bit
vector, i.e., the bit vector is B<NOT> extended at its upper end to
"rescue" the "C<$bits>" uppermost (most significant) bits - instead,
these bits are lost forever.

If you don't want this to happen, you have to increase the size of the
given bit vector B<EXPLICITLY> and B<BEFORE> you perform the "Insert"
operation, with a statement such as the following:

  $vector->Resize($vector->Size() + $bits);

Or use the method "C<Interval_Substitute()>" instead of "C<Insert()>",
which performs automatic growing and shrinking of its target bit vector.

Note also that "C<$offset>" must lie in the permitted range between
"C<0>" and "C<$vector-E<gt>Size()-1>", or a fatal "offset out of range"
error will occur.

If the term "C<$offset + $bits>" exceeds "C<$vector-E<gt>Size()-1>",
all the bits starting with bit number "C<$offset>" up to bit number
"C<$vector-E<gt>Size()-1>" are simply cleared.

=item *

C<$vector-E<gt>Delete($offset,$bits);>

This method deletes, i.e., removes the bits starting at position
"C<$offset>" up to and including bit number "C<$offset+$bits-1>"
from the given bit vector.

The remaining uppermost bits (starting at position "C<$offset+$bits>"
up to and including bit number "C<$vector-E<gt>Size()-1>") are moved
down by "C<$bits>" places.

The now vacant uppermost (most significant) "C<$bits>" bits are then
set to zero (cleared).

Note that this method does B<NOT> decrease the size of the given bit
vector, i.e., the bit vector is B<NOT> clipped at its upper end to
"get rid of" the vacant "C<$bits>" uppermost bits.

If you don't want this, i.e., if you want the bit vector to shrink
accordingly, you have to do so B<EXPLICITLY> and B<AFTER> the "Delete"
operation, with a couple of statements such as these:

  $size = $vector->Size();
  if ($bits > $size) { $bits = $size; }
  $vector->Resize($size - $bits);

Or use the method "C<Interval_Substitute()>" instead of "C<Delete()>",
which performs automatic growing and shrinking of its target bit vector.

Note also that "C<$offset>" must lie in the permitted range between
"C<0>" and "C<$vector-E<gt>Size()-1>", or a fatal "offset out of range"
error will occur.

If the term "C<$offset + $bits>" exceeds "C<$vector-E<gt>Size()-1>",
all the bits starting with bit number "C<$offset>" up to bit number
"C<$vector-E<gt>Size()-1>" are simply cleared.

=item *

C<$carry = $vector-E<gt>increment();>

This method increments the given bit vector.

Note that this method regards bit vectors as being unsigned,
i.e., the largest possible positive number is directly
followed by the smallest possible (or greatest possible,
speaking in absolute terms) negative number:

  before:  2 ^ (b-1) - 1    (= "0111...1111")
  after:   2 ^ (b-1)        (= "1000...0000")

where "C<b>" is the number of bits of the given bit vector.

The method returns "false" ("C<0>") in all cases except when a
carry-over occurs (in which case it returns "true", i.e., "C<1>"),
which happens when the number "1111...1111" is incremented,
which gives "0000...0000" plus a carry-over to the next higher
(binary) digit.

This can be used for the terminating condition of a "while" loop,
for instance, in order to cycle through all possible values the
bit vector can assume.

=item *

C<$carry = $vector-E<gt>decrement();>

This method decrements the given bit vector.

Note that this method regards bit vectors as being unsigned,
i.e., the smallest possible (or greatest possible, speaking
in absolute terms) negative number is directly followed by
the largest possible positive number:

  before:  2 ^ (b-1)        (= "1000...0000")
  after:   2 ^ (b-1) - 1    (= "0111...1111")

where "C<b>" is the number of bits of the given bit vector.

The method returns "false" ("C<0>") in all cases except when a
carry-over occurs (in which case it returns "true", i.e., "C<1>"),
which happens when the number "0000...0000" is decremented,
which gives "1111...1111" minus a carry-over to the next higher
(binary) digit.

This can be used for the terminating condition of a "while" loop,
for instance, in order to cycle through all possible values the
bit vector can assume.

=item *

C<$carry = $vec3-E<gt>add($vec1,$vec2,$carry);>

This method adds the two numbers contained in bit vector "C<$vec1>"
and "C<$vec2>" with carry "C<$carry>" and stores the result in
bit vector "C<$vec3>".

I.e.,
            $vec3 = $vec1 + $vec2 + $carry

Note that the "C<$carry>" parameter is a boolean value, i.e.,
only its least significant bit is taken into account. (Think of
it as though "C<$carry &= 1;>" was always executed internally.)

The method returns a boolean value which indicates if a carry-over
(to the next higher bit) has occured.

The carry in- and output is needed mainly for cascading, i.e.,
to add numbers that are fragmented into several pieces.

Example:

  # initialize

  for ( $i = 0; $i < $n; $i++ )
  {
      $a[$i] = Bit::Vector->new($bits);
      $b[$i] = Bit::Vector->new($bits);
      $c[$i] = Bit::Vector->new($bits);
  }

  # fill @a and @b

  # $a[  0 ] is low order part,
  # $a[$n-1] is high order part,
  # and same for @b

  # add

  $carry = 0;
  for ( $i = 0; $i < $n; $i++ )
  {
      $carry = $c[$i]->add($a[$i],$b[$i],$carry);
  }

Note that it makes no difference to this method wether the numbers
in "C<$vec1>" and "C<$vec2>" are unsigned or signed (i.e., in two's
complement binary representation).

Note however that the return value (carry-over) is not meaningful
when the numbers are B<SIGNED>.

Moreover, when the numbers are signed, a special type of error can
occur which is commonly called an "overflow error".

An overflow error occurs when the sign of the result (its most
significant bit) is flipped (i.e., falsified) by a carry-over
from the next-lower bit position.

It is your own responsibility to make sure that no overflow error
occurs if the numbers are signed.

To make absolutely sure that no overflow error can occur, make
your bit vectors at least one bit longer than the largest number
you wish to represent needs, right from the start, or proceed as
follows:

    $msb1 = $vec1->msb();
    $msb2 = $vec2->msb();
    $vec1->Resize($vec1->Size()+1);
    $vec2->Resize($vec2->Size()+1);
    $vec3->Resize($vec3->Size()+1);
    $vec1->MSB($msb1);
    $vec2->MSB($msb2);
    $c_o = $vec3->add($vec1,$vec2,$c_i);

=item *

C<$carry = $vec3-E<gt>subtract($vec1,$vec2,$carry);>

This method subtracts the two numbers contained in bit vector
"C<$vec1>" and "C<$vec2>" with carry "C<$carry>" and stores the
result in bit vector "C<$vec3>".

I.e.,
            $vec3 = $vec1 - $vec2 - $carry

Note that the "C<$carry>" parameter is a boolean value, i.e.,
only its least significant bit is taken into account. (Think of
it as though "C<$carry &= 1;>" was always executed internally.)

The method returns a boolean value which indicates if a carry-over
(to the next higher bit) has occured.

The carry in- and output is needed mainly for cascading, i.e.,
to subtract numbers that are fragmented into several pieces.

Example:

  # initialize

  for ( $i = 0; $i < $n; $i++ )
  {
      $a[$i] = Bit::Vector->new($bits);
      $b[$i] = Bit::Vector->new($bits);
      $c[$i] = Bit::Vector->new($bits);
  }

  # fill @a and @b

  # $a[  0 ] is low order part,
  # $a[$n-1] is high order part,
  # and same for @b

  # subtract

  $carry = 0;
  for ( $i = 0; $i < $n; $i++ )
  {
      $carry = $c[$i]->subtract($a[$i],$b[$i],$carry);
  }

Note that it makes no difference to this method wether the numbers
in "C<$vec1>" and "C<$vec2>" are unsigned or signed (i.e., in two's
complement binary representation).

Note however that the return value (carry-over) is not meaningful
when the numbers are B<SIGNED>.

Moreover, when the numbers are signed, a special type of error can
occur which is commonly called an "overflow error".

An overflow error occurs when the sign of the result (its most
significant bit) is flipped (i.e., falsified) by a carry-over
from the next-lower bit position.

It is your own responsibility to make sure that no overflow error
occurs if the numbers are signed.

To make absolutely sure that no overflow error can occur, make
your bit vectors at least one bit longer than the largest number
you wish to represent needs, right from the start, or proceed as
follows:

    $msb1 = $vec1->msb();
    $msb2 = $vec2->msb();
    $vec1->Resize($vec1->Size()+1);
    $vec2->Resize($vec2->Size()+1);
    $vec3->Resize($vec3->Size()+1);
    $vec1->MSB($msb1);
    $vec2->MSB($msb2);
    $c_o = $vec3->subtract($vec1,$vec2,$c_i);

=item *

C<$vec2-E<gt>Negate($vec1);>

This method calculates the two's complement of the number in bit
vector "C<$vec1>" and stores the result in bit vector "C<$vec2>".

Calculating the two's complement of a given number in binary representation
consists of inverting all bits and incrementing the result by one.

This is the same as changing the sign of the given number from "C<+>" to
"C<->" or vice-versa. In other words, applying this method twice on a given
number yields the original number again.

Note that in-place processing is also possible, i.e., "C<$vec1>" and
"C<$vec2>" may be identical.

Most importantly, beware that this method produces a counter-intuitive
result if the number contained in bit vector "C<$vec1>" is C<2 ^ (n-1)>
(i.e., "1000...0000"), where "C<n>" is the number of bits the given bit
vector contains: The negated value of this number is the number itself!

=item *

C<$vec2-E<gt>Absolute($vec1);>

Depending on the sign (i.e., the most significant bit) of the number in
bit vector "C<$vec1>", the contents of bit vector "C<$vec1>" are copied
to bit vector "C<$vec2>" either with the method "C<Copy()>" (if the number
in bit vector "C<$vec1>" is positive), or with "C<Negate()>" (if the number
in bit vector "C<$vec1>" is negative).

In other words, this method calculates the absolute value of the number
in bit vector "C<$vec1>" and stores the result in bit vector "C<$vec2>".

Note that in-place processing is also possible, i.e., "C<$vec1>" and
"C<$vec2>" may be identical.

Most importantly, beware that this method produces a counter-intuitive
result if the number contained in bit vector "C<$vec1>" is C<2 ^ (n-1)>
(i.e., "1000...0000"), where "C<n>" is the number of bits the given bit
vector contains: The absolute value of this number is the number itself,
even though this number is still negative by definition (the most
significant bit is still set)!

=item *

C<$sign = $vector-E<gt>Sign();>

This method returns "C<0>" if all bits in the given bit vector are cleared,
i.e., if the given bit vector contains the number "C<0>", or if the given
bit vector has a length of zero (contains no bits at all).

If not all bits are cleared, this method returns "C<-1>" if the most
significant bit is set (i.e., if the bit vector contains a negative
number), or "C<1>" otherwise (i.e., if the bit vector contains a
positive number).

=item *

C<$vec3-E<gt>Multiply($vec1,$vec2);>

This method multiplies the two numbers contained in bit vector "C<$vec1>"
and "C<$vec2>" and stores the result in bit vector "C<$vec3>".

Note that this method regards its arguments as B<SIGNED>.

If you want to make sure that a large number can never be treated as being
negative by mistake, make your bit vectors at least one bit longer than the
largest number you wish to represent, right from the start, or proceed as
follows:

    $msb1 = $vec1->msb();
    $msb2 = $vec2->msb();
    $vec1->Resize($vec1->Size()+1);
    $vec2->Resize($vec2->Size()+1);
    $vec3->Resize($vec3->Size()+1);
    $vec1->MSB($msb1);
    $vec2->MSB($msb2);
    $vec3->Multiply($vec1,$vec2);

Note also that all three bit vector arguments must in principle obey the
rule of matching sizes, but that the bit vector "C<$vec3>" may be larger
than the two factors "C<$vec1>" and "C<$vec2>".

In fact multiplying two binary numbers with "C<n>" bits may yield a result
which is at most "C<2n>" bits long.

Therefore, it is usually a good idea to let bit vector "C<$vec3>" have
twice the size of bit vector "C<$vec1>" and "C<$vec2>", unless you are
absolutely sure that the result will fit into a bit vector of the same
size as the two factors.

If you are wrong, a fatal "numeric overflow error" will occur.

Finally, note that in-place processing is possible, i.e., "C<$vec3>"
may be identical with "C<$vec1>" or "C<$vec2>", or both.

=item *

C<$quot-E<gt>Divide($vec1,$vec2,$rest);>

This method divides the two numbers contained in bit vector "C<$vec1>"
and "C<$vec2>" and stores the quotient in bit vector "C<$quot>" and
the remainder in bit vector "C<$rest>".

I.e.,
            $quot = $vec1 / $vec2;  #  div
            $rest = $vec1 % $vec2;  #  mod

Therefore, "C<$quot>" and "C<$rest>" must be two B<DISTINCT> bit vectors,
or a fatal "Q and R must be distinct" error will occur.

Note also that a fatal "division by zero error" will occur if "C<$vec2>"
is equal to zero.

Note further that this method regards its arguments as B<SIGNED>.

If you want to make sure that a large number can never be treated as being
negative by mistake, make your bit vectors at least one bit longer than the
largest number you wish to represent, right from the start, or proceed as
follows:

    $msb1 = $vec1->msb();
    $msb2 = $vec2->msb();
    $vec1->Resize($vec1->Size()+1);
    $vec2->Resize($vec2->Size()+1);
    $quot->Resize($quot->Size()+1);
    $rest->Resize($rest->Size()+1);
    $vec1->MSB($msb1);
    $vec2->MSB($msb2);
    $quot->Divide($vec1,$vec2,$rest);

Finally, note that in-place processing is possible, i.e., "C<$quot>"
may be identical with "C<$vec1>" or "C<$vec2>" or both, and "C<$rest>"
may also be identical with "C<$vec1>" or "C<$vec2>" or both, as long
as "C<$quot>" and "C<$rest>" are distinct. (!)

=item *

C<$vec3-E<gt>GCD($vec1,$vec2);>

This method calculates the "Greatest Common Divisor" of the two numbers
contained in bit vector "C<$vec1>" and "C<$vec2>" and stores the result
in bit vector "C<$vec3>".

The method uses Euklid's algorithm internally:

    int GCD(int a, int b)
    {
        int t;

        while (b != 0)
        {
            t = a % b; /* = remainder of (a div b) */
            a = b;
            b = t;
        }
        return(a);
    }

Note that a fatal "division by zero error" will occur if any of the two
numbers is equal to zero.

=item *

C<$vector-E<gt>Block_Store($buffer);>

This method allows you to load the contents of a given bit vector in
one go.

This is useful when you store the contents of a bit vector in a file,
for instance (using method "C<Block_Read()>"), and when you want to
restore the previously saved bit vector.

For this, "C<$buffer>" B<MUST> be a string (B<NO> automatic conversion
from numeric to string is provided here as would normally in Perl!)
containing the bit vector in "low order byte first" order.

If the given string is shorter than what is needed to completely fill
the given bit vector, the remaining (most significant) bytes of the
bit vector are filled with zeros, i.e., the previous contents of the
bit vector are always erased completely.

If the given string is longer than what is needed to completely fill
the given bit vector, the superfluous bytes are simply ignored.

See L<perlfunc/sysread> for how to read in the contents of "C<$buffer>"
from a file prior to passing it to this method.

=item *

C<$buffer = $vector-E<gt>Block_Read();>

This method allows you to export the contents of a given bit vector in
one block.

This is useful when you want to save the contents of a bit vector for
later, for instance in a file.

The advantage of this method is that it allows you to do so in the
compactest possible format, in binary.

The method returns a Perl string which contains an exact copy of the
contents of the given bit vector in "low order byte first" order.

See L<perlfunc/syswrite> for how to write the data from this string
to a file.

=item *

C<$size = $vector-E<gt>Word_Size();>

Each bit vector is internally organized as an array of machine words.

The methods whose names begin with "Word_" allow you to access this
internal array of machine words.

Note that because the size of a machine word may vary from system to
system, these methods are inherently B<MACHINE-DEPENDENT>!

Therefore, B<DO NOT USE> these methods unless you are absolutely certain
that portability of your code is not an issue!

You have been warned!

To be machine-independent, use the methods whose names begin with "C<Chunk_>"
instead, with chunks sizes no greater than 32 bits.

The method "C<Word_Size()>" returns the number of machine words that the
internal array of words of the given bit vector contains.

This is similar in function to the term "C<scalar(@array)>" for a Perl array.

=item *

C<$vector-E<gt>Word_Store($offset,$word);>

This method allows you to store a given value "C<$word>" at a given
position "C<$offset>" in the internal array of words of the given
bit vector.

Note that "C<$offset>" must lie in the permitted range between "C<0>"
and "C<$vector-E<gt>Word_Size()-1>", or a fatal "offset out of range"
error will occur.

This method is similar in function to the expression
"C<$array[$offset] = $word;>" for a Perl array.

=item *

C<$word = $vector-E<gt>Word_Read($offset);>

This method allows you to access the value of a given machine word
at position "C<$offset>" in the internal array of words of the given
bit vector.

Note that "C<$offset>" must lie in the permitted range between "C<0>"
and "C<$vector-E<gt>Word_Size()-1>", or a fatal "offset out of range"
error will occur.

This method is similar in function to the expression
"C<$word = $array[$offset];>" for a Perl array.

=item *

C<$vector-E<gt>Word_List_Store(@words);>

This method allows you to store a list of values "C<@words>" in the
internal array of machine words of the given bit vector.

Thereby the B<LEFTMOST> value in the list ("C<$words[0]>") is stored
in the B<LEAST> significant word of the internal array of words (the
one with offset "C<0>"), the next value from the list ("C<$words[1]>")
is stored in the word with offset "C<1>", and so on, as intuitively
expected.

If the list "C<@words>" contains fewer elements than the internal
array of words of the given bit vector contains machine words,
the remaining (most significant) words are filled with zeros.

If the list "C<@words>" contains more elements than the internal
array of words of the given bit vector contains machine words,
the superfluous values are simply ignored.

This method is comparable in function to the expression
"C<@array = @words;>" for a Perl array.

=item *

C<@words = $vector-E<gt>Word_List_Read();>

This method allows you to retrieve the internal array of machine
words of the given bit vector all at once.

Thereby the B<LEFTMOST> value in the returned list ("C<$words[0]>")
is the B<LEAST> significant word from the given bit vector, and the
B<RIGHTMOST> value in the returned list ("C<$words[$#words]>") is
the B<MOST> significant word of the given bit vector.

This method is similar in function to the expression
"C<@words = @array;>" for a Perl array.

=item *

C<$vector-E<gt>Word_Insert($offset,$count);>

This method inserts "C<$count>" empty new machine words at position
"C<$offset>" in the internal array of words of the given bit vector.

The "C<$count>" most significant words are lost, and all words starting
with word number "C<$offset>" up to and including word number
"C<$vector-E<gt>Word_Size()-$count-1>" are moved up by "C<$count>" places.

The now vacant "C<$count>" words starting at word number "C<$offset>"
(up to and including word number "C<$offset+$count-1>") are then set
to zero (cleared).

Note that this method does B<NOT> increase the size of the given bit
vector, i.e., the bit vector is B<NOT> extended at its upper end to
"rescue" the "C<$count>" uppermost (most significant) words - instead,
these words are lost forever.

If you don't want this to happen, you have to increase the size of the
given bit vector B<EXPLICITLY> and B<BEFORE> you perform the "Insert"
operation, with a statement such as the following:

  $vector->Resize($vector->Size() + $count * Bit::Vector->Word_Bits());

Note also that "C<$offset>" must lie in the permitted range between
"C<0>" and "C<$vector-E<gt>Word_Size()-1>", or a fatal "offset out
of range" error will occur.

If the term "C<$offset + $count>" exceeds "C<$vector-E<gt>Word_Size()-1>",
all the words starting with word number "C<$offset>" up to word number
"C<$vector-E<gt>Word_Size()-1>" are simply cleared.

=item *

C<$vector-E<gt>Word_Delete($offset,$count);>

This method deletes, i.e., removes the words starting at position
"C<$offset>" up to and including word number "C<$offset+$count-1>"
from the internal array of machine words of the given bit vector.

The remaining uppermost words (starting at position "C<$offset+$count>"
up to and including word number "C<$vector-E<gt>Word_Size()-1>") are
moved down by "C<$count>" places.

The now vacant uppermost (most significant) "C<$count>" words are then
set to zero (cleared).

Note that this method does B<NOT> decrease the size of the given bit
vector, i.e., the bit vector is B<NOT> clipped at its upper end to
"get rid of" the vacant "C<$count>" uppermost words.

If you don't want this, i.e., if you want the bit vector to shrink
accordingly, you have to do so B<EXPLICITLY> and B<AFTER> the "Delete"
operation, with a couple of statements such as these:

  $bits = $vector->Size();
  $count *= Bit::Vector->Word_Bits();
  if ($count > $bits) { $count = $bits; }
  $vector->Resize($bits - $count);

Note also that "C<$offset>" must lie in the permitted range between
"C<0>" and "C<$vector-E<gt>Word_Size()-1>", or a fatal "offset out
of range" error will occur.

If the term "C<$offset + $count>" exceeds "C<$vector-E<gt>Word_Size()-1>",
all the words starting with word number "C<$offset>" up to word number
"C<$vector-E<gt>Word_Size()-1>" are simply cleared.

=item *

C<$vector-E<gt>Chunk_Store($chunksize,$offset,$chunk);>

This method allows you to set more than one bit at a time with
different values.

You can access chunks (i.e., ranges of contiguous bits) between
one and at most "C<Bit::Vector-E<gt>Long_Bits()>" bits wide.

In order to be portable, though, you should never use chunk sizes
larger than 32 bits.

If the given "C<$chunksize>" does not lie between "C<1>" and
"C<Bit::Vector-E<gt>Long_Bits()>", a fatal "chunk size out of range"
error will occur.

The method copies the "C<$chunksize>" least significant bits
from the value "C<$chunk>" to the given bit vector, starting at
bit position "C<$offset>" and proceeding upwards until bit number
"C<$offset+$chunksize-1>".

(I.e., bit number "C<0>" of "C<$chunk>" becomes bit number "C<$offset>"
in the given bit vector, and bit number "C<$chunksize-1>" becomes
bit number "C<$offset+$chunksize-1>".)

If the term "C<$offset+$chunksize-1>" exceeds "C<$vector-E<gt>Size()-1>",
the corresponding superfluous (most significant) bits from "C<$chunk>"
are simply ignored.

Note that "C<$offset>" itself must lie in the permitted range between
"C<0>" and "C<$vector-E<gt>Size()-1>", or a fatal "offset out of range"
error will occur.

This method (as well as the other "C<Chunk_>" methods) is useful, for
example, when you are reading in data in chunks of, say, 8 bits, which
you need to access later, say, using 16 bits at a time (like audio CD
wave files, for instance).

=item *

C<$chunk = $vector-E<gt>Chunk_Read($chunksize,$offset);>

This method allows you to read the values of more than one bit at
a time.

You can read chunks (i.e., ranges of contiguous bits) between
one and at most "C<Bit::Vector-E<gt>Long_Bits()>" bits wide.

In order to be portable, though, you should never use chunk sizes
larger than 32 bits.

If the given "C<$chunksize>" does not lie between "C<1>" and
"C<Bit::Vector-E<gt>Long_Bits()>", a fatal "chunk size out of range"
error will occur.

The method returns the "C<$chunksize>" bits from the given bit vector
starting at bit position "C<$offset>" and proceeding upwards until
bit number "C<$offset+$chunksize-1>".

(I.e., bit number "C<$offset>" of the given bit vector becomes bit number
"C<0>" of the returned value, and bit number "C<$offset+$chunksize-1>"
becomes bit number "C<$chunksize-1>".)

If the term "C<$offset+$chunksize-1>" exceeds "C<$vector-E<gt>Size()-1>",
the non-existent bits are simply not returned.

Note that "C<$offset>" itself must lie in the permitted range between
"C<0>" and "C<$vector-E<gt>Size()-1>", or a fatal "offset out of range"
error will occur.

=item *

C<$vector-E<gt>Chunk_List_Store($chunksize,@chunks);>

This method allows you to fill the given bit vector with a list of
data packets ("chunks") of any size ("C<$chunksize>") you like
(within certain limits).

In fact the given "C<$chunksize>" must lie in the range between "C<1>"
and "C<Bit::Vector-E<gt>Long_Bits()>", or a fatal "chunk size out of
range" error will occur.

In order to be portable, though, you should never use chunk sizes
larger than 32 bits.

The given bit vector is thereby filled in ascending order: The first
chunk from the list (i.e., "C<$chunks[0]>") fills the "C<$chunksize>"
least significant bits, the next chunk from the list ("C<$chunks[1]>")
fills the bits number "C<$chunksize>" to number "C<2*$chunksize-1>",
the third chunk ("C<$chunks[2]>") fills the bits number "C<2*$chunksize>",
to number "C<3*$chunksize-1>", and so on.

If there a less chunks in the list than are needed to fill the entire
bit vector, the remaining (most significant) bits are cleared, i.e.,
the previous contents of the given bit vector are always erased completely.

If there are more chunks in the list than are needed to fill the entire
bit vector, and/or if a chunk extends beyond "C<$vector-E<gt>Size()-1>"
(which happens whenever "C<$vector-E<gt>Size()>" is not a multiple of
"C<$chunksize>"), the superfluous chunks and/or bits are simply ignored.

The method is useful, for example (and among many other applications),
for the conversion of packet sizes in a data stream.

This method can also be used to store an octal string in a given
bit vector:

  $vector->Chunk_List_Store(3, split(//, reverse $string));

Note however that unlike the conversion methods "C<from_Hex()>",
"C<from_Bin()>", "C<from_Dec()>" and "C<from_Enum()>",
this statement does not include any syntax checking, i.e.,
it may fail silently, without warning.

To perform syntax checking, add the following statements:

  if ($string =~ /^[0-7]+$/)
  {
      # okay, go ahead with conversion as shown above
  }
  else
  {
      # error, string contains other than octal characters
  }

Another application is to store a repetitive pattern in a given
bit vector:

  $pattern = 0xDEADBEEF;
  $length = 32;            # = length of $pattern in bits
  $size = $vector->Size();
  $factor = int($size / $length);
  if ($size % $length) { $factor++; }
  $vector->Chunk_List_Store($length, ($pattern) x $factor);

=item *

C<@chunks = $vector-E<gt>Chunk_List_Read($chunksize);>

This method allows you to access the contents of the given bit vector in
form of a list of data packets ("chunks") of a size ("C<$chunksize>")
of your choosing (within certain limits).

In fact the given "C<$chunksize>" must lie in the range between "C<1>"
and "C<Bit::Vector-E<gt>Long_Bits()>", or a fatal "chunk size out of
range" error will occur.

In order to be portable, though, you should never use chunk sizes
larger than 32 bits.

The given bit vector is thereby read in ascending order: The
"C<$chunksize>" least significant bits (bits number "C<0>" to
"C<$chunksize-1>") become the first chunk in the returned list
(i.e., "C<$chunks[0]>"). The bits number "C<$chunksize>" to
"C<2*$chunksize-1>" become the next chunk in the list
("C<$chunks[1]>"), and so on.

If "C<$vector-E<gt>Size()>" is not a multiple of "C<$chunksize>",
the last chunk in the list will contain fewer bits than "C<$chunksize>".

B<BEWARE> that for large bit vectors and/or small values of "C<$chunksize>",
the number of returned list elements can be extremely large! B<BE CAREFUL!>

You could blow up your application with lack of memory (each list element
is a full-grown Perl scalar, internally, with an associated memory overhead
for its administration!) or at least cause a noticeable, more or less
long-lasting "freeze" of your application!

Possible applications:

The method is especially useful in the conversion of packet sizes in
a data stream.

This method can also be used to convert a given bit vector to a string
of octal numbers:

  $string = reverse join('', $vector->Chunk_List_Read(3));

=item *

C<$vector-E<gt>Index_List_Remove(@indices);>

This method allows you to specify a list of indices of bits which
should be turned off in the given bit vector.

In fact this method is a shortcut for

    foreach $index (@indices)
    {
        $vector->Bit_Off($index);
    }

In contrast to all other import methods in this module, this method
does B<NOT> clear the given bit vector before processing its list
of arguments.

Instead, this method allows you to accumulate the results of various
consecutive calls.

(The same holds for the method "C<Index_List_Store()>". As a
consequence, you can "wipe out" what you did using the method
"C<Index_List_Remove()>" by passing the identical argument list
to the method "C<Index_List_Store()>".)

=item *

C<$vector-E<gt>Index_List_Store(@indices);>

This method allows you to specify a list of indices of bits which
should be turned on in the given bit vector.

In fact this method is a shortcut for

    foreach $index (@indices)
    {
        $vector->Bit_On($index);
    }

In contrast to all other import methods in this module, this method
does B<NOT> clear the given bit vector before processing its list
of arguments.

Instead, this method allows you to accumulate the results of various
consecutive calls.

(The same holds for the method "C<Index_List_Remove()>". As a
consequence, you can "wipe out" what you did using the method
"C<Index_List_Store()>" by passing the identical argument list
to the method "C<Index_List_Remove()>".)

=item *

C<@indices = $vector-E<gt>Index_List_Read();>

This method returns a list of Perl scalars.

The list contains one scalar for each set bit in the given
bit vector.

B<BEWARE> that for large bit vectors, this can result in a literally
overwhelming number of list elements! B<BE CAREFUL!> You could run
out of memory or slow down your application considerably!

Each scalar contains the number of the index corresponding to
the bit in question.

These indices are always returned in ascending order.

If the given bit vector is empty (contains only cleared bits)
or if it has a length of zero (if it contains no bits at all),
the method returns an empty list.

This method can be useful, for instance, to obtain a list of
prime numbers:

    $limit = 1000; # or whatever
    $vector = Bit::Vector->new($limit+1);
    $vector->Primes();
    @primes = $vector->Index_List_Read();

=item *

C<$set3-E<gt>Union($set1,$set2);>

This method calculates the union of "C<$set1>" and "C<$set2>" and stores
the result in "C<$set3>".

This is usually written as "C<$set3 = $set1 u $set2>" in set theory
(where "u" is the "cup" operator).

(On systems where the "cup" character is unavailable this operator
is often denoted by a plus sign "+".)

In-place calculation is also possible, i.e., "C<$set3>" may be identical
with "C<$set1>" or "C<$set2>" or both.

=item *

C<$set3-E<gt>Intersection($set1,$set2);>

This method calculates the intersection of "C<$set1>" and "C<$set2>" and
stores the result in "C<$set3>".

This is usually written as "C<$set3 = $set1 n $set2>" in set theory
(where "n" is the "cap" operator).

(On systems where the "cap" character is unavailable this operator
is often denoted by an asterisk "*".)

In-place calculation is also possible, i.e., "C<$set3>" may be identical
with "C<$set1>" or "C<$set2>" or both.

=item *

C<$set3-E<gt>Difference($set1,$set2);>

This method calculates the difference of "C<$set1>" less "C<$set2>" and
stores the result in "C<$set3>".

This is usually written as "C<$set3 = $set1 \ $set2>" in set theory
(where "\" is the "less" operator).

In-place calculation is also possible, i.e., "C<$set3>" may be identical
with "C<$set1>" or "C<$set2>" or both.

=item *

C<$set3-E<gt>ExclusiveOr($set1,$set2);>

This method calculates the symmetric difference of "C<$set1>" and "C<$set2>"
and stores the result in "C<$set3>".

This can be written as "C<$set3 = ($set1 u $set2) \ ($set1 n $set2)>" in set
theory (the union of the two sets less their intersection).

When sets are implemented as bit vectors then the above formula is
equivalent to the exclusive-or between corresponding bits of the two
bit vectors (hence the name of this method).

Note that this method is also much more efficient than evaluating the
above formula explicitly since it uses a built-in machine language
instruction internally.

In-place calculation is also possible, i.e., "C<$set3>" may be identical
with "C<$set1>" or "C<$set2>" or both.

=item *

C<$set2-E<gt>Complement($set1);>

This method calculates the complement of "C<$set1>" and stores the result
in "C<$set2>".

In "big integer" arithmetic, this is equivalent to calculating the one's
complement of the number stored in the bit vector "C<$set1>" in binary
representation.

In-place calculation is also possible, i.e., "C<$set2>" may be identical
with "C<$set1>".

=item *

C<if ($set1-E<gt>subset($set2))>

Returns "true" ("C<1>") if "C<$set1>" is a subset of "C<$set2>"
(i.e., completely contained in "C<$set2>") and "false" ("C<0>")
otherwise.

This means that any bit which is set ("C<1>") in "C<$set1>" must
also be set in "C<$set2>", but "C<$set2>" may contain set bits
which are not set in "C<$set1>", in order for the condition
of subset relationship to be true between these two sets.

Note that by definition, if two sets are identical, they are
also subsets (and also supersets) of each other.

=item *

C<$norm = $set-E<gt>Norm();>

Returns the norm (number of bits which are set) of the given vector.

This is equivalent to the number of elements contained in the given
set.

=item *

C<$min = $set-E<gt>Min();>

Returns the minimum of the given set, i.e., the minimum of all
indices of all set bits in the given bit vector "C<$set>".

If the set is empty (no set bits), plus infinity (represented
by the constant "MAX_LONG" on your system) is returned.

(This constant is usually S<2 ^ (n-1) - 1>, where "C<n>" is the
number of bits of an unsigned long on your machine.)

=item *

C<$max = $set-E<gt>Max();>

Returns the maximum of the given set, i.e., the maximum of all
indices of all set bits in the given bit vector "C<$set>".

If the set is empty (no set bits), minus infinity (represented
by the constant "MIN_LONG" on your system) is returned.

(This constant is usually S<-(2 ^ (n-1) - 1)> or S<-(2 ^ (n-1))>,
where "C<n>" is the number of bits of an unsigned long on your machine.)

=item *

C<$m3-E<gt>Multiplication($r3,$c3,$m1,$r1,$c1,$m2,$r2,$c2);>

This method multiplies two boolean matrices (stored as bit vectors)
"C<$m1>" and "C<$m2>" and stores the result in matrix "C<$m3>".

An exception is raised if the product of the number of rows and
columns of any of the three matrices differs from the actual size
of their underlying bit vector.

An exception is also raised if the numbers of rows and columns
of the three matrices do not harmonize in the required manner:

  rows3 == rows1
  cols3 == cols2
  cols1 == rows2

This method is used by the module "Math::MatrixBool".

See L<Math::MatrixBool(3)> for details.

=item *

C<$matrix-E<gt>Closure($rows,$cols);>

This method calculates the reflexive transitive closure of the
given boolean matrix (stored as a bit vector) using Kleene's
algoritm.

(See L<Math::Kleene(3)> for a brief introduction into the
theory behind Kleene's algorithm.)

The reflexive transitive closure answers the question wether
a path exists between any two vertices of a graph whose edges
are given as a matrix:

If a (directed) edge exists going from vertex "i" to vertex "j",
then the element in the matrix with coordinates (i,j) is set to
"C<1>" (otherwise it remains set to "C<0>").

If the edges are undirected, the resulting matrix is symmetric,
i.e., elements (i,j) and (j,i) always contain the same value.

The matrix representing the edges of the graph only answers the
question wether an B<EDGE> exists between any two vertices of
the graph or not, whereas the reflexive transitive closure
answers the question wether a B<PATH> (a series of adjacent
edges) exists between any two vertices of the graph!

Note that the contents of the given matrix are modified by
this method, so make a copy of the initial matrix in time
if you are going to need it again later.

An exception is raised if the given matrix is not quadratic,
i.e., if the number of rows and columns of the given matrix
is not identical.

An exception is also raised if the product of the number of
rows and columns of the given matrix differs from the actual
size of its underlying bit vector.

This method is used by the module "Math::MatrixBool".

See L<Math::MatrixBool(3)> for details.

=item *

C<$matrix2-E<gt>Transpose($rows2,$cols2,$matrix1,$rows1,$cols1);>

This method calculates the transpose of a boolean matrix "C<$matrix1>"
(stored as a bit vector) and stores the result in matrix "C<$matrix2>".

The transpose of a boolean matrix, representing the edges of a graph,
can be used for finding the strongly connected components of that graph.

An exception is raised if the product of the number of rows and
columns of any of the two matrices differs from the actual size
of its underlying bit vector.

An exception is also raised if the following conditions are not
met:

  rows2 == cols1
  cols2 == rows1

Note that in-place processing ("C<$matrix1>" and "C<$matrix2>" are
identical) is only possible if the matrix is quadratic. Otherwise,
a fatal "matrix is not quadratic" error will occur.

This method is used by the module "Math::MatrixBool".

See L<Math::MatrixBool(3)> for details.

=back

=head2 CLASS METHODS (implemented in Perl)

=over 2

=item *

C<$config = Bit::Vector-E<gt>Configuration();>

=item *

C<Bit::Vector-E<gt>Configuration($config);>

=item *

C<$oldconfig = Bit::Vector-E<gt>Configuration($newconfig);>

This method serves to alter the semantics (i.e., behaviour) of certain
overloaded operators (which are all implemented in Perl, by the way).

It does not have any effect whatsoever on anything else. In particular,
it does not affect the methods implemented in C.

The method accepts an (optional) string as input in which certain keywords
are expected, which influence some or almost all of the overloaded operators
in several possible ways.

The method always returns a string (which you do not need to take care of, i.e.,
to store, in case you aren't interested in keeping it) which is a complete
representation of the current configuration (i.e., B<BEFORE> any modifications
are applied) and which can be fed back to this method later in order to restore
the previous configuration.

There are three aspects of the way certain overloaded operators behave which
can be controlled with this method:

  +  the way scalar operands (replacing one of the two
     bit vector object operands) are automatically
     converted internally into a bit vector object of
     their own,

  +  the operation certain overloaded operators perform,
     i.e., an operation with sets or an arithmetic
     operation,

  +  the format to which bit vectors are converted
     automatically when they are enclosed in double
     quotes.

The input string may contain any number of assignments, each of which
controls one of these three aspects.

Each assignment has the form "C<E<lt>whichE<gt>=E<lt>valueE<gt>>".

"C<E<lt>whichE<gt>>" and "C<E<lt>valueE<gt>>" thereby consist of letters
(C<[a-zA-Z]>) and white space.

Multiple assignments have to be separated by one or more comma (","),
semi-colon (";"), colon (":"), vertical bar ("|"), slash ("/"),
newline ("\n"), ampersand ("&"), plus ("+") or dash ("-").

Empty lines or statements (only white space) are allowed but will be
ignored.

"C<E<lt>whichE<gt>>" has to contain one or more keywords from one of
three groups, each group representing one of the three aspects that
the "C<Configuration()>" method controls:

  +  "^scalar", "^input", "^in$"

  +  "^operator", "^semantic", "^ops$"

  +  "^string", "^output", "^out$"

The character "^" thereby denotes the beginning of a word, and "$"
denotes the end. Case is ignored (!).

Using these keywords, you can build any phrase you like to select one
of the three aspects (see also examples given below).

The only condition is that no other keyword from any of the other two
groups may match - otherwise a syntax error will occur (i.e., ambiguities
are forbidden). A syntax error also occurs if none of the keywords
matches.

This same principle applies to "C<E<lt>valueE<gt>>":

Depending on which aspect you specified for "C<E<lt>whichE<gt>>",
there are different groups of keywords that determine the value
the selected aspect will be set to:

  +  "<which>" = "^scalar", "^input", "^in$":

       "<value>" =

       *  "^bit$", "^index", "^indice"
       *  "^hex"
       *  "^bin"
       *  "^dec"
       *  "^enum"

  +  "<which>" = "^operator", "^semantic", "^ops$":

       "<value>" =

       *  "^set$"
       *  "^arithmetic"

  +  "<which>" = "^string", "^output", "^out$":

       "<value>" =

       *  "^hex"
       *  "^bin"
       *  "^dec"
       *  "^enum"

Examples:

  "Any scalar input I provide should be considered to be = a bit index"

  "I want to have operator semantics suitable for = arithmetics"

  "Any bit vector in double quotes is to be output as = an enumeration"

=head2 Scalar input:

In the case of scalar input, "C<^bit$>", "C<^index>", or "C<^indice>"
all cause scalar input to be considered to represent a bit index, i.e.,
"C<$vector ^= 5;>" will flip bit #5 in the given bit vector (this is
essentially the same as "C<$vector-E<gt>bit_flip(5);>").

Note that "bit indices" is the default setting for "scalar input".

The keyword "C<^hex>" will cause scalar input to be considered as being in
hexadecimal, i.e., "C<$vector ^= 5;>" will flip bit #0 and bit #2 (because
hexadecimal "C<5>" is binary "C<0101>").

(Note though that hexadecimal input should always be enclosed in quotes,
otherwise it will be interpreted as a decimal number by Perl! The example
relies on the fact that hexadecimal C<0-9> and decimal C<0-9> are the same.)

The keyword "C<^bin>" will cause scalar input to be considered as being in
binary format. All characters except "C<0>" and "C<1>" are forbidden in
this case (i.e., produce a syntax error).

"C<$vector ^= '0101';>", for instance, will flip bit #0 and bit #2.

The keyword "C<^dec>" causes scalar input to be considered as integers
in decimal format, i.e., "C<$vector ^= 5;>" will flip bit #0 and bit #2
(because decimal "C<5>" is binary "C<0101>").

(Note though that all decimal input should be enclosed in quotes, because
for large numbers, Perl will use scientific notation internally for
representing them, which produces a syntax error because scientific
notation is neither supported by this module nor needed.)

Finally, the keyword "C<^enum>" causes scalar input to be considered
as being a list ("enumeration") of indices and ranges of (contiguous)
indices, i.e., "C<$vector |= '2,3,5,7-13,17-23';>" will cause bits #2,
#3, #5, #7 through #13 and #17 through #23 to be set.

=head2 Operator semantics:

Several overloaded operators can have two distinct functions depending
on this setting.

The affected operators are: "C<+>", "C<->", "C<*>", "C<E<lt>>", "C<E<lt>=>",
"C<E<gt>>" and "C<E<gt>=>".

With the default setting, "set operations", these operators perform:

  +       set union                           ( set1  u   set2 )
  -       set difference                      ( set1  \   set2 )
  *       set intersection                    ( set1  n   set2 )
  <       true subset relationship            ( set1  <   set2 )
  <=      subset relationship                 ( set1  <=  set2 )
  >       true superset relationship          ( set1  >   set2 )
  >=      superset relationship               ( set1  >=  set2 )

With the alternative setting, "arithmetic operations", these operators
perform:

  +       addition                            ( num1  +   num2 )
  -       subtraction                         ( num1  -   num2 )
  *       multiplication                      ( num1  *   num2 )
  <       "less than" comparison              ( num1  <   num2 )
  <=      "less than or equal" comparison     ( num1  <=  num2 )
  >       "greater than" comparison           ( num1  >   num2 )
  >=      "greater than or equal" comparison  ( num1  >=  num2 )

Note that these latter comparison operators ("C<E<lt>>", "C<E<lt>=>",
"C<E<gt>>" and "C<E<gt>=>") regard their operands as being B<SIGNED>.

To perform comparisons with B<UNSIGNED> operands, use the operators
"C<lt>", "C<le>", "C<gt>" and "C<ge>" instead (in contrast to the
operators above, these operators are B<NOT> affected by the
"operator semantics" setting).

=head2 String output:

There are four methods which convert the contents of a given bit vector
into a string: "C<to_Hex()>", "C<to_Bin()>", "C<to_Dec()>" and "C<to_Enum()>"
(not counting "C<Block_Read()>", since this method does not return a
human-readable string).

(For conversion to octal, see the description of the method
"C<Chunk_List_Read()>".)

Therefore, there are four possible formats into which a bit vector can
be converted when it is enclosed in double quotes, for example:

  print "\$vector = '$vector'\n";
  $string = "$vector";

Hence you can set "string output" to four different values: To "hex"
for hexadecimal format (which is the default), to "bin" for binary
format, to "dec" for conversion to decimal numbers and to "enum"
for conversion to enumerations (".newsrc" style sets).

B<BEWARE> that the conversion to decimal numbers is inherently slow;
it can easily take up several seconds for a single large bit vector!

Therefore you should store the decimal strings returned to you
rather than converting a given bit vector again.

=head2 Examples:

The default setting as returned by the method "C<Configuration()>"
is:

        Scalar Input       = Bit Index
        Operator Semantics = Set Operators
        String Output      = Hexadecimal

Performing a statement such as:

  Bit::Vector->Configuration("in=bin,ops=arithmetic,out=bin");
  print Bit::Vector->Configuration(), "\n";

yields the following output:

        Scalar Input       = Binary
        Operator Semantics = Arithmetic Operators
        String Output      = Binary

Note that you can always feed this output back into the "C<Configuration()>"
method to restore that setting later.

This also means that you can enter the same given setting with almost any
degree of verbosity you like (as long as the required keywords appear and
no ambiguities arise).

Note further that any aspect you do not specify is not changed, i.e.,
the statement

  Bit::Vector->Configuration("operators = arithmetic");

leaves all other aspects unchanged.

=back

=head2 OVERLOADED OPERATORS (implemented in Perl)

=over 2

=item *

C<"$vector">

Remember that variables enclosed in double quotes are always
interpolated in Perl.

Whenever a Perl variable containing the reference of a "Bit::Vector"
object is enclosed in double quotes (either alone or together with
other text and/or variables), the contents of the corresponding
bit vector are converted into a printable string.

Since there are several conversion methods available in this module
(see the description of the methods "C<to_Hex()>", "C<to_Bin()>",
"C<to_Dec()>" and "C<to_Enum()>"), it is of course desirable to
be able to choose which of these methods should be applied in this
case.

This can actually be done by changing the configuration of this
module using the method "C<Configure()>" (see the previous chapter,
immediately above).

The default is conversion to hexadecimal.

=item *

C<if ($vector)>

It is possible to use a Perl variable containing the reference of a
"Bit::Vector" object as a boolean expression.

The condition above is true if the corresponding bit vector contains
at least one set bit, and it is false if B<ALL> bits of the corresponding
bit vector are cleared.

=item *

C<if (!$vector)>

Since it is possible to use a Perl variable containing the reference of a
"Bit::Vector" object as a boolean expression, you can of course also negate
this boolean expression.

The condition above is true if B<ALL> bits of the corresponding bit vector
are cleared, and it is false if the corresponding bit vector contains at
least one set bit.

Note that this is B<NOT> the same as using the method "C<is_full()>",
which returns true if B<ALL> bits of the corresponding bit vector are
B<SET>.

=item *

C<~$vector>

This term returns a new bit vector object which is the one's complement
of the given bit vector.

This is equivalent to inverting all bits.

=item *

C<-$vector> (unary minus)

This term returns a new bit vector object which is the two's complement
of the given bit vector.

This is equivalent to inverting all bits and incrementing the result by one.

(This is the same as changing the sign of a number in two's complement
binary representation.)

=item *

C<abs($vector)>

Depending on the configuration (see the description of the method
"C<Configuration()>" for more details), this term either returns
the number of set bits in the given bit vector (this is the same
as calculating the number of elements which are contained in the
given set) - which is the default behaviour, or it returns a new
bit vector object which contains the absolute value of the number
stored in the given bit vector.

=item *

C<$vector1 . $vector2>

This term usually returns a new bit vector object which is the
result of the concatenation of the two bit vector operands.

The left operand becomes the most significant, and the right operand
becomes the least significant part of the new bit vector object.

If one of the two operands is not a bit vector object but a Perl scalar,
however, the contents of the remaining bit vector operand are converted
into a string (the format of which depends on the configuration set with
the "C<Configuration()>" method), which is then concatenated in the proper
order (i.e., as indicated by the order of the two operands) with the Perl
scalar.

In other words, a string is returned in such a case instead of a
bit vector object!

=item *

C<$vector x $factor>

This term returns a new bit vector object which is the concatenation
of as many copies of the given bit vector operand (the left operand)
as the factor (the right operand) specifies.

If the factor is zero, a bit vector object with a length of zero bits
is returned.

If the factor is one, just a new copy of the given bit vector is
returned.

Note that a fatal "reversed operands error" occurs if the two operands
are swapped.

=item *

C<$vector E<lt>E<lt> $bits>

This term returns a new bit vector object which is a copy of the given
bit vector (the left operand), which is then shifted left (towards the
most significant bit) by as many places as the right operand, "C<$bits>",
specifies.

This means that the "C<$bits>" most significant bits are lost, all other
bits move up by "C<$bits>" positions, and the "C<$bits>" least significant
bits that have been left unoccupied by this shift are all set to zero.

If "C<$bits>" is greater than the number of bits of the given bit vector,
this term returns an empty bit vector (i.e., with all bits cleared) of
the same size as the given bit vector.

Note that a fatal "reversed operands error" occurs if the two operands
are swapped.

=item *

C<$vector E<gt>E<gt> $bits>

This term returns a new bit vector object which is a copy of the given
bit vector (the left operand), which is then shifted right (towards the
least significant bit) by as many places as the right operand, "C<$bits>",
specifies.

This means that the "C<$bits>" least significant bits are lost, all other
bits move down by "C<$bits>" positions, and the "C<$bits>" most significant
bits that have been left unoccupied by this shift are all set to zero.

If "C<$bits>" is greater than the number of bits of the given bit vector,
this term returns an empty bit vector (i.e., with all bits cleared) of
the same size as the given bit vector.

Note that a fatal "reversed operands error" occurs if the two operands
are swapped.

=item *

C<$vector1 | $vector2>

This term returns a new bit vector object which is the result of
a bitwise OR operation between the two bit vector operands.

This is the same as calculating the union of two sets.

=item *

C<$vector1 & $vector2>

This term returns a new bit vector object which is the result of
a bitwise AND operation between the two bit vector operands.

This is the same as calculating the intersection of two sets.

=item *

C<$vector1 ^ $vector2>

This term returns a new bit vector object which is the result of
a bitwise XOR (exclusive-or) operation between the two bit vector
operands.

This is the same as calculating the symmetric difference of two sets.

=item *

C<$vector1 + $vector2>

Depending on the configuration (see the description of the method
"C<Configuration()>" for more details), this term either returns
a new bit vector object which is the result of a bitwise OR operation
between the two bit vector operands (this is the same as calculating
the union of two sets) - which is the default behaviour, or it returns
a new bit vector object which contains the sum of the two numbers
stored in the two bit vector operands.

=item *

C<$vector1 - $vector2>

Depending on the configuration (see the description of the method
"C<Configuration()>" for more details), this term either returns
a new bit vector object which is the set difference of the two sets
represented in the two bit vector operands - which is the default
behaviour, or it returns a new bit vector object which contains
the difference of the two numbers stored in the two bit vector
operands.

=item *

C<$vector1 * $vector2>

Depending on the configuration (see the description of the method
"C<Configuration()>" for more details), this term either returns
a new bit vector object which is the result of a bitwise AND operation
between the two bit vector operands (this is the same as calculating
the intersection of two sets) - which is the default behaviour, or it
returns a new bit vector object which contains the product of the two
numbers stored in the two bit vector operands.

=item *

C<$vector1 / $vector2>

This term returns a new bit vector object containing the result of the
division of the two numbers stored in the two bit vector operands.

=item *

C<$vector1 % $vector2>

This term returns a new bit vector object containing the remainder of
the division of the two numbers stored in the two bit vector operands.

=item *

C<$vector1 .= $vector2;>

This statement "appends" the right bit vector operand (the "rvalue")
to the left one (the "lvalue").

The former contents of the left operand become the most significant
part of the resulting bit vector, and the right operand becomes the
least significant part.

Since bit vectors are stored in "least order bit first" order, this
actually requires the left operand to be shifted "up" by the length
of the right operand, which is then copied to the now freed least
significant part of the left operand.

If the right operand is a Perl scalar, it is first converted to a
bit vector of the same size as the left operand, provided that the
configuration states that scalars are to be regarded as indices,
decimal strings or enumerations.

If the configuration states that scalars are to be regarded as hexadecimal
or boolean strings, however, these strings are converted to bit vectors of
a size matching the length of the input string, i.e., four times the length
for hexadecimal strings (because each hexadecimal digit is worth 4 bits) and
once the length for binary strings.

=item *

C<$vector x= $factor;>

This statement replaces the given bit vector by a concatenation of as many
copies of the original contents of the given bit vector as the factor (the
right operand) specifies.

If the factor is zero, the given bit vector is resized to a length of zero
bits.

If the factor is one, the given bit vector is not changed at all.

=item *

C<$vector E<lt>E<lt>= $bits;>

This statement moves the contents of the given bit vector left by "C<$bits>"
positions (towards the most significant bit).

This means that the "C<$bits>" most significant bits are lost, all other
bits move up by "C<$bits>" positions, and the "C<$bits>" least significant
bits that have been left unoccupied by this shift are all set to zero.

If "C<$bits>" is greater than the number of bits of the given bit vector,
the given bit vector is erased completely (i.e., all bits are cleared).

=item *

C<$vector E<gt>E<gt>= $bits;>

This statement moves the contents of the given bit vector right by "C<$bits>"
positions (towards the least significant bit).

This means that the "C<$bits>" least significant bits are lost, all other
bits move down by "C<$bits>" positions, and the "C<$bits>" most significant
bits that have been left unoccupied by this shift are all set to zero.

If "C<$bits>" is greater than the number of bits of the given bit vector,
the given bit vector is erased completely (i.e., all bits are cleared).

=item *

C<$vector1 |= $vector2;>

This statement performs a bitwise OR operation between the two
bit vector operands and stores the result in the left operand.

This is the same as calculating the union of two sets.

=item *

C<$vector1 &= $vector2;>

This statement performs a bitwise AND operation between the two
bit vector operands and stores the result in the left operand.

This is the same as calculating the intersection of two sets.

=item *

C<$vector1 ^= $vector2;>

This statement performs a bitwise XOR (exclusive-or) operation
between the two bit vector operands and stores the result in the
left operand.

This is the same as calculating the symmetric difference of two sets.

=item *

C<$vector1 += $vector2;>

Depending on the configuration (see the description of the method
"C<Configuration()>" for more details), this statement either performs
a bitwise OR operation between the two bit vector operands (this is
the same as calculating the union of two sets) - which is the default
behaviour, or it calculates the sum of the two numbers stored in the
two bit vector operands.

The result of this operation is stored in the left operand.

=item *

C<$vector1 -= $vector2;>

Depending on the configuration (see the description of the method
"C<Configuration()>" for more details), this statement either calculates
the set difference of the two sets represented in the two bit vector
operands - which is the default behaviour, or it calculates the
difference of the two numbers stored in the two bit vector operands.

The result of this operation is stored in the left operand.

=item *

C<$vector1 *= $vector2;>

Depending on the configuration (see the description of the method
"C<Configuration()>" for more details), this statement either performs
a bitwise AND operation between the two bit vector operands (this is
the same as calculating the intersection of two sets) - which is the
default behaviour, or it calculates the product of the two numbers
stored in the two bit vector operands.

The result of this operation is stored in the left operand.

=item *

C<$vector1 /= $vector2;>

This statement puts the result of the division of the two numbers
stored in the two bit vector operands into the left operand.

=item *

C<$vector1 %= $vector2;>

This statement puts the remainder of the division of the two numbers
stored in the two bit vector operands into the left operand.

=item *

C<++$vector>, C<$vector++>

This operator performs pre- and post-incrementation of the
given bit vector.

The value returned by this term is a reference of the given
bit vector object (after or before the incrementation,
respectively).

=item *

C<--$vector>, C<$vector-->

This operator performs pre- and post-decrementation of the
given bit vector.

The value returned by this term is a reference of the given
bit vector object (after or before the decrementation,
respectively).

=item *

C<($vector1 cmp $vector2)>

This term returns "C<-1>" if "C<$vector1>" is less than "C<$vector2>",
"C<0>" if "C<$vector1>" and "C<$vector2>" are the same, and "C<1>"
if "C<$vector1>" is greater than "C<$vector2>".

This comparison assumes B<UNSIGNED> bit vectors.

=item *

C<($vector1 eq $vector2)>

This term returns true ("C<1>") if the contents of the two bit vector
operands are the same and false ("C<0>") otherwise.

=item *

C<($vector1 ne $vector2)>

This term returns true ("C<1>") if the two bit vector operands differ
and false ("C<0>") otherwise.

=item *

C<($vector1 lt $vector2)>

This term returns true ("C<1>") if "C<$vector1>" is less than "C<$vector2>",
and false ("C<0>") otherwise.

This comparison assumes B<UNSIGNED> bit vectors.

=item *

C<($vector1 le $vector2)>

This term returns true ("C<1>") if "C<$vector1>" is less than or equal to
"C<$vector2>", and false ("C<0>") otherwise.

This comparison assumes B<UNSIGNED> bit vectors.

=item *

C<($vector1 gt $vector2)>

This term returns true ("C<1>") if "C<$vector1>" is greater than "C<$vector2>",
and false ("C<0>") otherwise.

This comparison assumes B<UNSIGNED> bit vectors.

=item *

C<($vector1 ge $vector2)>

This term returns true ("C<1>") if "C<$vector1>" is greater than or equal to
"C<$vector2>", and false ("C<0>") otherwise.

This comparison assumes B<UNSIGNED> bit vectors.

=item *

C<($vector1 E<lt>=E<gt> $vector2)>

This term returns "C<-1>" if "C<$vector1>" is less than "C<$vector2>",
"C<0>" if "C<$vector1>" and "C<$vector2>" are the same, and "C<1>"
if "C<$vector1>" is greater than "C<$vector2>".

This comparison assumes B<SIGNED> bit vectors.

=item *

C<($vector1 == $vector2)>

This term returns true ("C<1>") if the contents of the two bit vector
operands are the same and false ("C<0>") otherwise.

=item *

C<($vector1 != $vector2)>

This term returns true ("C<1>") if the two bit vector operands differ
and false ("C<0>") otherwise.

=item *

C<($vector1 E<lt> $vector2)>

Depending on the configuration (see the description of the method
"C<Configuration()>" for more details), this term either returns
true ("C<1>") if "C<$vector1>" is a true subset of "C<$vector2>"
(and false ("C<0>") otherwise) - which is the default behaviour,
or it returns true ("C<1>") if "C<$vector1>" is less than
"C<$vector2>" (and false ("C<0>") otherwise).

The latter comparison assumes B<SIGNED> bit vectors.

=item *

C<($vector1 E<lt>= $vector2)>

Depending on the configuration (see the description of the method
"C<Configuration()>" for more details), this term either returns
true ("C<1>") if "C<$vector1>" is a subset of "C<$vector2>" (and
false ("C<0>") otherwise) - which is the default behaviour, or it
returns true ("C<1>") if "C<$vector1>" is less than or equal to
"C<$vector2>" (and false ("C<0>") otherwise).

The latter comparison assumes B<SIGNED> bit vectors.

=item *

C<($vector1 E<gt> $vector2)>

Depending on the configuration (see the description of the method
"C<Configuration()>" for more details), this term either returns
true ("C<1>") if "C<$vector1>" is a true superset of "C<$vector2>"
(and false ("C<0>") otherwise) - which is the default behaviour,
or it returns true ("C<1>") if "C<$vector1>" is greater than
"C<$vector2>" (and false ("C<0>") otherwise).

The latter comparison assumes B<SIGNED> bit vectors.

=item *

C<($vector1 E<gt>= $vector2)>

Depending on the configuration (see the description of the method
"C<Configuration()>" for more details), this term either returns
true ("C<1>") if "C<$vector1>" is a superset of "C<$vector2>" (and
false ("C<0>") otherwise) - which is the default behaviour, or it
returns true ("C<1>") if "C<$vector1>" is greater than or equal to
"C<$vector2>" (and false ("C<0>") otherwise).

The latter comparison assumes B<SIGNED> bit vectors.

=back

=head1 SEE ALSO

Set::IntegerFast(3), Set::IntegerRange(3), Math::MatrixBool(3),
Math::MatrixReal(3), DFA::Kleene(3), Math::Kleene(3), Graph::Kruskal(3).

perl(1), perlsub(1), perlmod(1), perlref(1), perlobj(1), perlbot(1),
perltoot(1), perlxs(1), perlxstut(1), perlguts(1), overload(3).

=head1 VERSION

This man page documents "Bit::Vector" version 5.0.

=head1 AUTHOR

Steffen Beyer <sb@sdm.de>.

=head1 COPYRIGHT

Copyright (c) 1995, 1996, 1997, 1998 by Steffen Beyer.
All rights reserved.

=head1 LICENSE

This package is "Non-Profit-Ware" ("NP-ware").

You may use, copy, modify and redistribute it
under the terms of the "Non-Profit-License" (NPL).

Please refer to the file "NONPROFIT" in this module's
distribution for details!

