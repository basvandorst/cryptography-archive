#include "range.h"
#include "interval.h"

interval::interval(rvar& a, rvar& b) {mid = a; wid = b;}

interval::interval() {;}

interval::interval(interval& a) {mid = a.mid; wid = a.wid;}

interval& interval::operator=(interval& a)
{
mid = a.mid; wid = a.wid; return *this;
}

interval operator+(interval& a, interval& b)
{
interval c;
c.mid = a.mid + b.mid;
c.wid = a.wid + b.wid;
return c;
}

interval operator+(rvar& a, interval& b)
{
interval c;
c.mid = a + b.mid;
c.wid = b.wid;
return c;
}

interval operator-(interval& a)
{
interval c;
c.mid = -a.mid;
c.wid = a.wid;
return c;
}

interval operator-(interval& a, interval& b)
{
interval c;
c.mid = a.mid - b.mid;
c.wid = a.wid + b.wid;
return c;
}

interval operator*(interval& a, interval& b)
{
interval c;
c.mid = a.mid * b.mid;
c.wid = (abs(a.mid) + a.wid) * b.wid + a.wid * abs(b.mid);
return c;
}

interval operator*(rvar& a, interval& b)
{
interval c;
c.mid = a * b.mid;
c.wid = abs(a) * b.wid;
return c;
}
     
interval operator/(interval& a, interval& b)
{
interval c;
c.wid = abs(b.mid) - b.wid;
if (c.wid <= zero) {
     error ("interval a / b with b containing zero"); return I_zero;}
c.mid = a.mid / b.mid;
c.wid = (a.wid + abs(c.mid) * b.wid) / c.wid;
return c;
}

interval operator/(interval& a, rvar& b)
{
interval c;
c.mid = a.mid / b;
c.wid = a.wid / abs(b);
return c;
}

interval operator^(interval& a, rvar& b)
{
interval c; rvar high = a.mid + a.wid, low = a.mid - a.wid;
int zero_check_needed = FALSE;
c.mid = a.mid ^ b;
if (is_int(b)) {
     if ((high > zero && a.mid < zero) || (low  < zero && a.mid > zero)) {
          zero_check_needed = TRUE;}
     high = high ^ b; low = low ^ b;
     c.wid = max(abs(high - c.mid), abs(c.mid - low));
     if (zero_check_needed) {
          high = zero ^ b; // error message if b is a negative integer
          c.wid = max(c.wid, abs(high - c.mid));}
     return c;}
if (low < zero) low = zero; else low = low ^ b;
high = high ^ b;
c.wid = max(abs(high - c.mid), abs(c.mid - low));
return c;
}

interval operator<<(interval& a, rvar& b)
{
// a ^ b with b rational with even numer and odd denom
interval c; rvar high = a.mid + a.wid, low = a.mid - a.wid;
int zero_check_needed = FALSE;
c.mid = expon_e(a.mid, b);
if ((high > zero && a.mid < zero) || (low  < zero && a.mid > zero)) {
     zero_check_needed = TRUE;}
high = expon_e(high, b); low = expon_e(low, b);
c.wid = max(abs(high - c.mid), abs(c.mid - low));
if (zero_check_needed) {
     high = expon_e(zero, b); // zero result but error message if b negative
     c.wid = max(c.wid, abs(high - c.mid));}
return c;
}

interval operator>>(interval& a, rvar& b)
{
// a ^ b with b rational with odd numer and odd denom
interval c; rvar high = a.mid + a.wid, low = a.mid - a.wid;
int zero_check_needed = FALSE;
c.mid = expon_o(a.mid, b);
if ((high > zero && a.mid < zero) || (low  < zero && a.mid > zero)) {
     zero_check_needed = TRUE;}
high = expon_o(high, b); low = expon_o(low, b);
c.wid = max(abs(high - c.mid), abs(c.mid - low));
if (zero_check_needed) {
     high = expon_o(zero, b); // zero result but error message if b negative
     c.wid = max(c.wid, abs(high - c.mid));}
return c;
}

int interval::has_zero()
{
if (mid - wid > zero) return FALSE;
if (mid + wid < zero) return FALSE;
return TRUE;
}

int interval::positive()
{
if (mid - wid > zero) return TRUE;
return FALSE;
}

int interval::not_positive()
{
if (mid - wid <= zero) return TRUE;
return FALSE;
}

int interval::negative()
{
if (mid + wid < zero) return TRUE;
return FALSE;
}

interval abs(interval& a)
{
interval b = a;
b.mid = abs(b.mid);
return b;
}

interval asin(interval& a)
{
interval c; rvar low, high;
c.mid = asin(a.mid); // error message if a.mid outside [-1,1]
c.wid = abs(a.mid) + a.wid;
if (c.wid < one) {
     c.wid = a.wid / sqrt(one - c.wid * c.wid);}
else {
     low  = a.mid - a.wid, high = a.mid + a.wid;
     if (low <= -one) low = -div2(pi); else low = asin(low);
     if (high >= one) high = div2(pi); else high = asin(high);
     c.wid = max(c.mid - low, high - c.mid);}
return c;
}

interval acos(interval& a)
{
interval c(div2(pi));
c = c - asin(a);
return c;
}

interval atan(interval& a)
{
interval c;
c.mid = atan(a.mid);
c.wid = abs(a.mid) - a.wid;
if (c.wid <= zero) c.wid = a.wid;
else c.wid = a.wid / ( one + c.wid * c.wid);
return c;
}

interval cos(interval& a)
{
interval c(cos(a.mid), sin(a.mid));
c.wid = min(abs(c.wid) + a.wid, one) * a.wid;
return c;
}

interval cosh(interval& a)
{
interval c;
c.mid = cosh(a.mid);
c.wid = c.mid * (exp(a.wid) - one);
return c;
}

interval exp(interval& a)
{
interval c;
c.mid = exp(a.mid);
c.wid = c.mid * (exp(a.wid) - one);
return c;
}

interval log(interval& a)
{
interval c;
c.mid = log(a.mid);
c.wid = a.mid - a.wid;
if (c.wid <= zero) {
     error("interval log(a) with a out of bounds"); return I_zero;}
c.wid = a.wid / c.wid;
return c;
}

interval sin(interval& a)
{
interval c(sin(a.mid), cos(a.mid));
c.wid = min(abs(c.wid) + a.wid, one) * a.wid;
return c;
}

interval sinh(interval& a)
{
interval c;
c.mid = sinh(a.mid); c.wid = cosh(a.mid);
c.wid = c.wid * (exp(a.wid) - one);
return c;
}

interval sqrt(interval& a)
{
interval c; rvar low = a.mid - a.wid, high = a.mid + a.wid;
c.mid = sqrt(a.mid); // error message if a.mid negative
if (low <= zero) low = zero; else low = sqrt(low);
high = sqrt(high);
c.wid = max(c.mid - low, high - c.mid);
return c;
}

void swap(interval& a, interval& b)
{
swap(a.mid, b.mid); swap(a.wid, b.wid);
}
     
