#include "range.h"
#include "interval.h"
#include "series9.h"

// order of file is: first, constructors and destructor, then operators,
// and last, functions in alphabetical order

series :: series (ser*& p)
{
if (p == 0) make_series();
se = p; p = se->next;
se->count = 1; 
}

series :: series (rvar& r)
{
if (sstack == 0) make_series();
se = sstack; sstack = se->next;
se->count = 1;
term* p = se->comp;
(*p).xyz = 0;
(*p).mw.mid = r; (*p).mw.wid = zero;
(*p).tnext = 0;
}

series :: series ()
{
static int switch1 = FALSE;
if (switch1) {
     se = zero_series.se;
     se->count++;}
else {
     switch1 = TRUE;
     // construct zero_series
     ser* v; term* p; rvar::rva** q;
     v = (ser*) get_memory(sizeof(ser));
     v->count = 1; p = v->comp;
     q = (rvar::rva**) &p[0].mw.mid; *q = &zero_init;
     q = (rvar::rva**) &p[0].mw.wid; *q = &zero_init;
     zero_init.count += 2;
     p[0].xyz = 0; p[0].tnext = 0;
     se = v;}     
}

series :: series (series& a)
{
se = a.se;
se->count++;
}

series :: ~series()
{
if (--se->count == 0) {
     se->next = sstack;
     sstack = se;}
}

series& series::operator= (series& a)
{
a.se->count++;
if (--se->count == 0) {
     se->next = sstack;
     sstack = se;}
se = a.se; return *this;
}

series operator+ (series& a, series& b)
{
series c(series::sstack);
series::term *p1 = a.se->comp, *p2 = b.se->comp, *p3 = c.se->comp,
     *q1 = p1, *q2 = p2, *q3 = p3;
int i1 = (*q1).tnext, i2 = (*q2).tnext, k = 0;
long s1 = (*q1).xyz, s2 = (*q2).xyz, max = series::max_deg;
do {
     if (s1 == s2) {
          (*q3).xyz = s1;
          (*q3).mw = (*q1).mw + (*q2).mw;
          if (i1 == 0) s1 = deg_high;
          else {q1 = p1 + i1; i1 = (*q1).tnext; s1 = (*q1).xyz;}
          if (i2 == 0) s2 = deg_high;
          else {q2 = p2 + i2; i2 = (*q2).tnext; s2 = (*q2).xyz;}}
     else if (s1 < s2) {
          (*q3).xyz = s1;
          (*q3).mw = (*q1).mw;
          if (i1 == 0) s1 = deg_high;
          else {q1 = p1 + i1; i1 = (*q1).tnext; s1 = (*q1).xyz;}}
     else {
          (*q3).xyz = s2;
          (*q3).mw = (*q2).mw;
          if (i2 == 0) s2 = deg_high;
          else {q2 = p2 + i2; i2 = (*q2).tnext; s2 = (*q2).xyz;}}
     (*q3++).tnext = ++k;}
while (s1 < max || s2 < max);
(*--q3).tnext = 0;
return c;
}

series operator+ (rvar& a, series& b)
{
series c(series::sstack);
series::term *p1 = b.se->comp, *p2 = c.se->comp, *q1 = p1, *q2 = p2;
int t = (*q1).tnext;
(*q2).xyz = 0;
(*q2).mw = a + (*q1).mw;
(*q2).tnext = t;
while (t) {
     q2 = p2 + t;
     q1 = p1 + t;
     (*q2).xyz = (*q1).xyz;
     (*q2).mw = (*q1).mw;
     t = (*q1).tnext;
     (*q2).tnext = t;}
return c;
}

series operator- (series& a)
{
series b(series::sstack);
series::term *p1 = a.se->comp, *p2 = b.se->comp, *q1 = p1, *q2 = p2;
int t;
do {
     (*q2).xyz = (*q1).xyz;
     (*q2).mw = -(*q1).mw;
     t = (*q1).tnext;
     (*q2).tnext = t;
     q2 = p2 + t;
     q1 = p1 + t;}
while (t);
return b;
}

series operator- (series& a, series& b)
{
series c(series::sstack);
series::term *p1 = a.se->comp, *p2 = b.se->comp, *p3 = c.se->comp,
     *q1 = p1, *q2 = p2, *q3 = p3;
int i1 = (*q1).tnext, i2 = (*q2).tnext, k = 0;
long s1 = (*q1).xyz, s2 = (*q2).xyz, max = series::max_deg;
do {
     if (s1 == s2) {
          (*q3).xyz = s1;
          (*q3).mw = (*q1).mw - (*q2).mw;
          if (i1 == 0) s1 = deg_high;
          else {q1 = p1 + i1; i1 = (*q1).tnext; s1 = (*q1).xyz;}
          if (i2 == 0) s2 = deg_high;
          else {q2 = p2 + i2; i2 = (*q2).tnext; s2 = (*q2).xyz;}}
     else if (s1 < s2) {
          (*q3).xyz = s1;
          (*q3).mw = (*q1).mw;
          if (i1 == 0) s1 = deg_high;
          else {q1 = p1 + i1; i1 = (*q1).tnext; s1 = (*q1).xyz;}}
     else {
          (*q3).xyz = s2;
          (*q3).mw = -(*q2).mw;
          if (i2 == 0) s2 = deg_high;
          else {q2 = p2 + i2; i2 = (*q2).tnext; s2 = (*q2).xyz;}}
     (*q3++).tnext = ++k;}
while (s1 < max || s2 < max);
(*--q3).tnext = 0;
return c;
}

series operator* (series& a, series& b)
{
long d_max = series::max_deg, xyz1, t;
series c(series::sstack);
series::term *p1 = a.se->comp, *p2 = b.se->comp, *p3 = c.se->comp,
     *q1 = p1, *q2 = p2, *q22, *q3 = p3, *q33, *q333;
int k = 0, k1;

(*q3).xyz = 0; (*q3).tnext = 0;
(*q3).mw = I_zero;
while (1) {
     xyz1 = (*q1).xyz;
     q22 = q2; k1 = (*q3).tnext;
     while ((*q3).xyz < xyz1 && k1 != 0) {
          q33 = q3; q3 = p3 + k1; k1 = (*q3).tnext;}
     if ((*q3).xyz > xyz1) q3 = q33;
     q33 = q3;
     while (1) {
          t = xyz1 + (*q22).xyz;
          if (t > d_max) break; k1 = (*q33).tnext;
          while ((*q33).xyz < t && k1 != 0) {
               q333 = q33; q33 = p3 + k1; k1 = (*q33).tnext;}
          if ((*q33).xyz > t) q33 = q333;
          if ((*q33).xyz == t) (*q33).mw = (*q33).mw + (*q1).mw * (*q22).mw;
          else {
               k1 = (*q33).tnext;
               (*q33).tnext = ++k;
               q33 = p3 + k;
               (*q33).xyz = t;
               (*q33).mw = (*q1).mw * (*q22).mw;
               (*q33).tnext = k1;}
          if ((*q22).tnext == 0) break;
          else q22 = p2 + (*q22).tnext;}
     if ((*q1).tnext == 0) break;
     else q1 = p1 + (*q1).tnext;}
return c;
}

series operator* (rvar& a, series& b)
{
series c(series::sstack); rvar a1 = abs(a); int t;
series::term *p1 = b.se->comp, *p2 = c.se->comp, *q1 = p1, *q2 = p2;
do {
     (*q2).xyz = (*q1).xyz;
     (*q2).mw.mid = a * (*q1).mw.mid;
     (*q2).mw.wid = a1 * (*q1).mw.wid;
     t = (*q1).tnext;
     (*q2).tnext = t;
     q2 = p2 + t;
     q1 = p1 + t;}
while (t);
return c;
}

series operator/ (series& a, series& b)
{
long d_max = series::max_deg, xyz1 = 0, xyz3 = 0, t;
series c(series::sstack);
series::term *p1 = a.se->comp, *p2 = b.se->comp, *p3 = c.se->comp,
     *q1 = p1, *q2 = p2, *q22, *q3 = p3, *q33, *q333;
int k = 0, k1; interval d = (*p2).mw;

if ((*q2).tnext == 0) {// fast division when quotient is just constant
     do { 
          (*q3).xyz = (*q1).xyz;
          (*q3).mw = (*q1).mw / d;
          t = (*q1).tnext;
          (*q3).tnext = t;
          q3 = p3 + t;
          q1 = p1 + t;}
     while (t);
     return c;}
q2 = p2 + (*q2).tnext;
(*q3).xyz = 0; (*q3).mw = (*q1).mw; (*q3).tnext = 0;
while (1) {
     (*q3).mw = (*q3).mw / d;
     q33 = q3;
     q22 = q2;
     while (1) {
          t = xyz3 + (*q22).xyz;
          if (t > d_max) break; k1 = (*q33).tnext;
          while ((*q33).xyz < t && k1 != 0) {
               q333 = q33; q33 = p3 + k1; k1 = (*q33).tnext;}
          if ((*q33).xyz > t) q33 = q333;
          if ((*q33).xyz == t) (*q33).mw = (*q33).mw + (*q3).mw * (*q22).mw;
          else {
               k1 = (*q33).tnext;
               (*q33).tnext = ++k;
               q33 = p3 + k;
               (*q33).xyz = t;
               (*q33).mw = (*q3).mw * (*q22).mw;
               (*q33).tnext = k1;}
          if ((*q22).tnext == 0) break;
          q22 = p2 + (*q22).tnext;}
     if (xyz1 == xyz3) {
          if ((*q1).tnext) {q1 = p1 + (*q1).tnext; xyz1 = (*q1).xyz;}
          else xyz1 = deg_high;}
     k1 = (*q3).tnext; q33 = q3;
     if (k1) {q3 = p3 + k1; xyz3 = (*q3).xyz;}
     else {
          if (xyz1 == deg_high) break;
          else xyz3 = deg_high;}
     if (xyz1 < xyz3) {
          (*q33).tnext = ++k;
          q3 = p3 + k;
          (*q3).xyz = xyz1;
          xyz3 = xyz1;
          (*q3).mw = (*q1).mw;
          (*q3).tnext = k1;}
     else if (xyz1 == xyz3) (*q3).mw = (*q1).mw - (*q3).mw;
     else (*q3).mw = -(*q3).mw;}
return c;
}

series operator^(series& a, series& b)
{
series c;
if (b.se->comp[0].tnext) {c = exp(b*log(a)); return c;}
rvar r = b.se->comp[0].mw.mid;
if (is_int(r) && r <= ten && r > zero) c = integer_power(a, r); 
else c = power(a, r, '^');
return c;
}

interval& series::operator() (int i, int j, int k)
{
// i is order of derivative and may be 0, 1, or 2.
// If i is 0, constant term of series supplied.
// If i is 1, first partial with respect to variable j supplied.
// If i is 2, second partial with respect to variables j and k supplied.
if (i == 0) return se->comp[0].mw;
series::term *p = se->comp, *q = p;
long xyz1 = i, deg1 = i * deg_one;
if ((i == 1) || ((i == 2) && (j == k))) xyz1 = deg1 + (xyz1 << (j*2));
else {
     xyz1 = deg1 + ((long) 1 << (j*2)) + ((long) 1 << (k*2));}
while(1) {
     if ((*q).xyz == xyz1) return (*q).mw;
     else if ((*q).tnext == 0) return I_zero;
     else q = p + (*q).tnext;}
}

series abs(series& a)
{
interval I = a.se->comp[0].mw;
if (abs(I.mid) > I.wid) {
     series b;
     if (I.mid > zero) b = a; else b = -a; return b;}
else if (series::max_deg != 0 && a.se->comp[0].tnext != 0) {
     error("abs series is not defined"); return zero_series;}
else {
     series b(series::sstack);
     series::term *p1 = a.se->comp, *p2 = b.se->comp;
     (*p2).xyz = (*p1).xyz;
     (*p2).mw = abs((*p1).mw);
     (*p2).tnext = 0;
     return b;}
}

series atrig(series& a, int code)
{
long d_max = series::max_deg, xyz1, xyz3, t;
series b(series::sstack), c; interval multiplier;
series::term *p1 = a.se->comp, *p2 = b.se->comp, *p3,
     *q1, *q2, *q22, *q3, *q33, *q333;
int k = 0, k1; interval d;

(*p2).xyz = 0;
if ((*p1).tnext == 0 || d_max == 0) {
     if (code == atan_) (*p2).mw = atan((*p1).mw);
     else if (code == asin_) (*p2).mw = asin((*p1).mw);
     else (*p2).mw = acos((*p1).mw);
     (*p2).tnext = 0; return b;}
if (code == atan_) {
     (*p2).mw = atan((*p1).mw);
     c = a*a; p3 = c.se->comp;
     d = (*p3).mw; d.mid = d.mid + one;}
else {
     (*p2).mw = asin((*p1).mw);
     c = -(a*a);
     p3 = c.se->comp; (*p3).mw.mid = (*p3).mw.mid + one;
     c = power(c, half, sqrt_); p3 = c.se->comp; d = (*p3).mw;
     if (errorstatus == error_reported) return zero_series;}
q2 = p3 + (*p3).tnext;
q1 = p1 + (*p1).tnext;
q3 = p2;
(*q3++).tnext = ++k;
xyz1 = (*q1).xyz;
xyz3 = xyz1;
(*q3).xyz = xyz3;
(*q3).mw = (*q1).mw;
(*q3).tnext = 0;
while (1) {
     (*q3).mw = (*q3).mw / d;
     t = (xyz3 & deg_part) >> 28;
     multiplier = rvar_int[t] * (*q3).mw;
     q33 = q3;
     q22 = q2;
     while (1) {
          t = xyz3 + (*q22).xyz;
          if (t > d_max) break; k1 = (*q33).tnext;
          while ((*q33).xyz < t && k1 != 0) {
               q333 = q33; q33 = p2 + k1; k1 = (*q33).tnext;}
          if ((*q33).xyz > t) q33 = q333;
          if ((*q33).xyz == t) (*q33).mw = (*q33).mw + multiplier * (*q22).mw;
          else {
               k1 = (*q33).tnext;
               (*q33).tnext = ++k;
               q33 = p2 + k;
               (*q33).xyz = t;
               (*q33).mw = multiplier * (*q22).mw;
               (*q33).tnext = k1;}
          if ((*q22).tnext == 0) break;
          q22 = p3 + (*q22).tnext;}
     if (xyz1 == xyz3) {
          if ((*q1).tnext) {q1 = p1 + (*q1).tnext; xyz1 = (*q1).xyz;}
          else xyz1 = deg_high;}
     k1 = (*q3).tnext; q33 = q3;
     if (k1) {q3 = p2 + k1; xyz3 = (*q3).xyz;}
     else {
          if (xyz1 == deg_high) break;
          else xyz3 = deg_high;}
     if (xyz1 < xyz3) {
          (*q33).tnext = ++k;
          q3 = p2 + k;
          (*q3).xyz = xyz1;
          xyz3 = xyz1;
          (*q3).mw = (*q1).mw;
          (*q3).tnext = k1;}
     else {
          t = (xyz3 & deg_part) >> 28;
          if (xyz1 == xyz3) (*q3).mw = (*q1).mw - (*q3).mw / rvar_int[t];
          else (*q3).mw = -(*q3).mw / rvar_int[t];}}
if (code == acos_) {
     b = -b; p2 = b.se->comp; (*p2).mw.mid = (*p2).mw.mid + div2(pi);}
return b;
}

void series::const_term(rvar& M, rvar& W)
{
term* p = se->comp;
(*p).mw.mid = M; (*p).mw.wid = W;
}

series exp(series& a)
{
long d_max = series::max_deg, xyz1, xyz3, t;
series b(series::sstack), c(series::sstack); interval multiplier;
series::term *p1 = a.se->comp, *p2 = b.se->comp, *p3 = c.se->comp,
     *q1 = p1, *q2 = p3, *q22, *q3 = p2, *q33, *q333;
int k = 0, k1;

(*q3).mw = exp((*q1).mw);
(*q3).xyz = 0; (*q3).tnext = 0; 
if ((*q1).tnext == 0) return b;
q1 = p1 + (*q1).tnext;
do {
     t = (*q1).xyz; (*q2).xyz = t;
     t = (t & deg_part) >> 28;
     (*q2).mw = rvar_int[t] * (*q1).mw;
     (*q2++).tnext = ++k;
     k1 = (*q1).tnext;
     q1 = p1 + k1;}
while (k1);
(*--q2).tnext = 0;
q2 = p3; k = 0; xyz3 = 0;
while (1) {
     multiplier = (*q3).mw;
     q33 = q3;
     q22 = q2;
     while (1) {
          t = xyz3 + (*q22).xyz;
          if (t > d_max) break; k1 = (*q33).tnext;
          while ((*q33).xyz < t && k1 != 0) {
               q333 = q33; q33 = p2 + k1; k1 = (*q33).tnext;}
          if ((*q33).xyz > t) q33 = q333;
          if ((*q33).xyz == t) (*q33).mw = (*q33).mw + multiplier * (*q22).mw;
          else {
               k1 = (*q33).tnext;
               (*q33).tnext = ++k;
               q33 = p2 + k;
               (*q33).xyz = t;
               (*q33).mw = multiplier * (*q22).mw;
               (*q33).tnext = k1;}
          if ((*q22++).tnext == 0) break;}
     k1 = (*q3).tnext;
     if (k1) {
          q3 = p2 + k1; xyz3 = (*q3).xyz;
          t = (xyz3 & deg_part) >> 28;
          (*q3).mw = (*q3).mw / rvar_int[t];}
     else break;}
return b;
}

void evaluate (series& result, svar& entry_line, series* var) 
{
char c, *p = entry_line.tokens(), *q = !entry_line, *q1;
int i = -1; series s;
if (series::evaluate_error_display == TRUE) errorstatus = display_error;
else if (series::evaluate_exit_on_error == FALSE) errorstatus = signal_error;
else errorstatus = exit_on_error;
while (1) {
     c = *p;
     switch (c) {
          case end_: series::evaluate_error = FALSE; result = sri[0];
          errorstatus = exit_on_error; return;
          
          case const_: q1 = q + ((*(p+1) << 7) + *(p+2));
          sri[++i] = series(rvar(q1)); break;
          
          case pi_const_: sri[++i] = series(pi); break;
          case var_: sri[++i] = var[*(p+2)]; break;
          case unary_minus_: sri[i] = - sri[i]; break;
          case abs_ : sri[i] = abs(sri[i]); break;
          case acos_: case asin_: case atan_:
          sri[i] = atrig(sri[i], c); break;
          
          case cos_ : case sin_ : case tan_ :
          sri[i] = sin_cos(sri[i], c); break;
          
          case cosh_: case sinh_: case tanh_:
          sri[i] = sinh_cosh(sri[i], c); break;
          
          case exp_ : sri[i] = exp(sri[i]); break;
          case log_ : sri[i] = log(sri[i]); break;
          case sqrt_: sri[i] = power(sri[i], half, sqrt_); break;
          case '+': s = sri[i--]; sri[i] = sri[i] + s; break;
          case '-': s = sri[i--]; sri[i] = sri[i] - s; break;
          case '*': s = sri[i--]; sri[i] = sri[i] * s; break;
          case '/': s = sri[i--]; sri[i] = sri[i] / s; break;
          case '^': s = sri[i--]; sri[i] = sri[i] ^ s; break;
          case power_odd_p_odd_q: case_power_even_p_odd_q: s = sri[i--];
          sri[i] = power(sri[i], s.se->comp[0].mw.mid, c);
          break;
                    
          default: error("series evaluation");}
     if (errorstatus == error_reported) {
          series::evaluate_error = TRUE;
          if (series::evaluate_error_display == TRUE) {
               show_error_pos(entry_line, (*(p+1) << 7) + *(p+2));}
          if (series::evaluate_exit_on_error == TRUE) exit(1);
          errorstatus = exit_on_error;
          return;}
     else p += 3;} 
}

void series::initiate(int no_var, int max_degree, series* var)
{
// this function must be called before series are used
if (no_var < 1) no_var = 1;
if (no_var > 14) no_var = 14;
if (max_degree < 0) max_degree = 0;
if (max_degree > 3) max_degree = 3;
long t = no_var + 1, n;
if (max_degree == 1) n = t;
else if (max_degree == 2) n = t * (t+1) / 2;
else n = t * (t + 1) * (t + 2) / 6;
series::max_no_terms = n;
series::max_deg_bound = max_degree;
series::max_deg = max_degree * max_factor;
series::max_size = (sizeof(ser) + (n-1) * sizeof(series::term));
series::sstack = 0;
for (int i=1; i<=3; i++) rvar_int[i] = rvar(i);
ser* v; series::term* p; rvar::rva** q;
// construct basic variables as needed
long xyz1 = 1, deg_part = 0X10000000; int size1, size2;
if (max_degree == 0) {
     size1 = 1; size2 = sizeof(ser);}
else {
     size1 = 2; size2 = sizeof(ser) + sizeof(series::term);}
for (int j=0; j<no_var; j++) {
     v = (ser*) get_memory(size2);
     v->count = 1; p = v->comp;
     for (i=0; i<size1; i++) {
          q = (rvar::rva**) &p[i].mw.mid; *q = &zero_init;
          q = (rvar::rva**) &p[i].mw.wid; *q = &zero_init;}
     zero_init.count += size1 + size1;
     p[0].xyz = 0;
     if (size1 == 1) p[0].tnext = 0;
     else {
          p[0].tnext = 1;
          p[1].xyz = deg_part + xyz1; xyz1 <<= 2;
          p[1].mw.mid = one; p[1].tnext = 0;}
     var[j].se = v;}
}

series integer_power(series& a, rvar& r)
{// generate c by forming int power r of a
series c, t = a; int c_empty = TRUE; rvar r1 = r, r2;
do {
     r2 = trunc(div2(r1)); r1 -= r2 + r2;
     if (r1 > zero) {
          if (c_empty) {
               c = t; c_empty = FALSE;}
          else c = c * t;}
     if (r2 > zero) t = t * t; else return c;
     r1 = r2;}
while (1);
}

series log(series& a)
{
long d_max = series::max_deg, xyz1, xyz3, t;
series b(series::sstack); interval multiplier;
series::term *p1 = a.se->comp, *p2 = b.se->comp,
     *q1 = p1, *q2 = p1, *q22, *q3 = p2, *q33, *q333;
int k = 0, k1; interval d = (*p1).mw;

(*q3).mw = log(d); (*q3).xyz = 0;
if ((*q2).tnext == 0) {(*q3).tnext = 0; return b;}
q2 = p1 + (*q2).tnext;
(*q3++).tnext = ++k;
q1 = q2;
xyz1 = (*q1).xyz;
xyz3 = xyz1;
(*q3).xyz = xyz3;
(*q3).mw = (*q1).mw;
(*q3).tnext = 0;
while (1) {
     (*q3).mw = (*q3).mw / d;
     t = (xyz3 & deg_part) >> 28;
     multiplier = rvar_int[t] * (*q3).mw;
     q33 = q3;
     q22 = q2;
     while (1) {
          t = xyz3 + (*q22).xyz;
          if (t > d_max) break; k1 = (*q33).tnext;
          while ((*q33).xyz < t && k1 != 0) {
               q333 = q33; q33 = p2 + k1; k1 = (*q33).tnext;}
          if ((*q33).xyz > t) q33 = q333;
          if ((*q33).xyz == t) (*q33).mw = (*q33).mw + multiplier * (*q22).mw;
          else {
               k1 = (*q33).tnext;
               (*q33).tnext = ++k;
               q33 = p2 + k;
               (*q33).xyz = t;
               (*q33).mw = multiplier * (*q22).mw;
               (*q33).tnext = k1;}
          if ((*q22).tnext == 0) break;
          q22 = p1 + (*q22).tnext;}
     if (xyz1 == xyz3) {
          if ((*q1).tnext) {q1 = p1 + (*q1).tnext; xyz1 = (*q1).xyz;}
          else xyz1 = deg_high;}
     k1 = (*q3).tnext; q33 = q3;
     if (k1) {q3 = p2 + k1; xyz3 = (*q3).xyz;}
     else {
          if (xyz1 == deg_high) break;
          else xyz3 = deg_high;}
     if (xyz1 < xyz3) {
          (*q33).tnext = ++k;
          q3 = p2 + k;
          (*q3).xyz = xyz1;
          xyz3 = xyz1;
          (*q3).mw = (*q1).mw;
          (*q3).tnext = k1;}
     else {
          t = (xyz3 & deg_part) >> 28;
          if (xyz1 == xyz3) (*q3).mw = (*q1).mw - (*q3).mw / rvar_int[t];
          else (*q3).mw = -(*q3).mw / rvar_int[t];}}
return b;
}

void make_series()
{
series::ser* v = (series::ser*) get_memory(series::max_size);
series::term* p = v->comp; rvar::rva** q;
int i, n = series::max_no_terms;
for (i=0; i<n; i++) {
     q = (rvar::rva**) &p[i].mw.mid; *q = &zero_init;
     q = (rvar::rva**) &p[i].mw.wid; *q = &zero_init;}
zero_init.count += n+n;
v->next = series::sstack; series::sstack = v;
}

series power(series& a, rvar& r, int code)
{
long d_max = series::max_deg, i, xyz1, xyz2, xyz3, t, t0, t1;
series b(series::sstack);
series::term *p1 = a.se->comp, *p2 = b.se->comp,
     *q1 = p1, *q2, *q22, *q3 = p2, *q33, *q333;
interval d = (*q1).mw, multiplier; rvar r1, r2;
int k, k1;

(*q3).tnext = 0; (*q3).xyz = 0;
if (! d.has_zero()) {
     if (code == sqrt_) (*q3).mw = sqrt(d);
     else if (code == power_even_p_odd_q) (*q3).mw = d << r;
     else if (code == power_odd_p_odd_q) (*q3).mw = d >> r;
     else (*q3).mw = d ^ r;
     if ((*q1).tnext == 0 || errorstatus == error_reported) return b;}
else { // d overlaps zero; non-standard generation of b if possible
     if ((*q1).tnext == 0) {
          r1 = r; t0 = 0;}
     else {
          t0 = d_max & deg_part; r1 = r - rvar_int[t0 >> 28];}
     if (r1 <= zero) {
          error("series raised to a power is not defined");
          return zero_series;}
     if (code == sqrt_) (*q3).mw = sqrt(d);
     else if (code == power_even_p_odd_q) (*q3).mw = d << r1;
     else if (code == power_odd_p_odd_q) (*q3).mw = d >> r1;
     else (*q3).mw = d ^ r1;
     if (t0 == 0 || errorstatus == error_reported) return b;
     series c(series::sstack), e(series::sstack);
     series::term *p3 = c.se->comp, *p4 = e.se->comp;
     q1 = p1 + (*q1).tnext; q2 = p3; k = 0;
     do {
          t = (*q1).xyz; (*q2).xyz = t;
          t = (t & deg_part) >> 28;
          (*q2).mw = rvar_int[t] * (*q1).mw;
          (*q2++).tnext = ++k;
          k1 = (*q1).tnext;
          q1 = p1 + k1;}
     while (k1);
     (*--q2).tnext = 0;
     k = 0; q2 = p3;
     for (i=deg_one; i<=t0; i+=deg_one) {
          r1 = r1 + one; q1 = p2; q3 = p4; k = 0;
          (*q3).mw = (*q1).mw * d; (*q3).tnext = 0; (*q3).xyz = 0;
          do {
               xyz1 = (*q1).xyz; q22 = q2; q33 = q3;
               do {
                    t = xyz1 + (*q22).xyz;
                    if ((t & deg_part) > i) break;
                    k1 = (*q33).tnext;
                    while ((*q33).xyz < t && k1 != 0) {
                         q333 = q33; q33 = p4 + k1; k1 = (*q33).tnext;}
                    if ((*q33).xyz > t) q33 = q333;
                    if ((*q33).xyz == t) {
                         (*q33).mw = (*q33).mw + (*q1).mw * (*q22).mw;}
                    else {
                         k1 = (*q33).tnext; (*q33).tnext = ++k;
                         q33 = p4 + k; (*q33).xyz = t;
                         (*q33).mw = (*q1).mw * (*q22).mw;
                         (*q33).tnext = k1;}}
               while ((*q22++).tnext);}
          while ((*q1++).tnext);
          q1 = p4; q3 = p2; k = 0;
          (*q3).mw = (*q1).mw; (*q3++).tnext = ++k;
          q1 = p4 + (*q1).tnext; t1 = 0;
          do {
               t = (*q1).xyz; (*q3).xyz = t; 
               if ((t & deg_part) != t1) {
                    t1 = t & deg_part;
                    multiplier = interval(r1 / rvar_int[t1 >> 28]);}
               (*q3).mw = multiplier * (*q1).mw;
               (*q3++).tnext = ++k;
               k1 = (*q1).tnext;
               q1 = p4 + k1;}
          while (k1);
          (*--q3).tnext = 0;}
     return b;}

// standard generation of series
q2 = p1 + (*q1).tnext; q3 = p2; k = 0; xyz3 = 0; r1 = r + one;
while (1) {
     multiplier = (*q3).mw;
     q33 = q3;
     q22 = q2;
     while (1) {
          xyz2 = (*q22).xyz;
          t = xyz3 + xyz2;
          if (t > d_max) break;
          r2 = r1 * rvar_int[xyz2 >> 28] / rvar_int[t >> 28] - one;
          k1 = (*q33).tnext;
          while ((*q33).xyz < t && k1 != 0) {
               q333 = q33; q33 = p2 + k1; k1 = (*q33).tnext;}
          if ((*q33).xyz > t) q33 = q333;
          if ((*q33).xyz == t)
               (*q33).mw = (*q33).mw + r2 * multiplier * (*q22).mw;
          else {
               k1 = (*q33).tnext;
               (*q33).tnext = ++k;
               q33 = p2 + k;
               (*q33).xyz = t;
               (*q33).mw = r2 * multiplier * (*q22).mw;
               (*q33).tnext = k1;}
          k1 = (*q22).tnext;
          if (k1 == 0) break; else q22 = p1 + k1;}
     k1 = (*q3).tnext;
     if (k1) {
          q3 = p2 + k1; xyz3 = (*q3).xyz;
          (*q3).mw = (*q3).mw / d;}
     else break;}
return b;
}

void series::set_max_deg(int i)
{
if (i < 0) i = 0;
if (i > series::max_deg_bound) i = series::max_deg_bound;
series::max_deg = i * max_factor;
}

series sin_cos(series& a, int code)
{
long d_max = series::max_deg, xyz1, xyz3, t;
series s(series::sstack), c(series::sstack), g(series::sstack);
interval multiplier_s, multiplier_c;
series::term *p1 = a.se->comp, *p2s = s.se->comp, *p2c = c.se->comp,
     *p3 = g.se->comp, *q1 = p1, *q2 = p3, *q22, *q3s = p2s, *q3c = p2c,
     *q33s, *q33c, *q333s, *q333c;
int k, k1;
(*q3s).mw = sin((*q1).mw);
(*q3c).mw = cos((*q1).mw);
(*q3s).xyz = 0; (*q3s).tnext = 0;
(*q3c).xyz = 0; (*q3c).tnext = 0;
if ((*q1).tnext == 0) {
     if (code == cos_) return c;
     else if (code == sin_) return s;
     else {(*q3s).mw = (*q3s).mw / (*q3c).mw; return s;}}
q1 = p1 + (*q1).tnext; k = 0;
do {
     t = (*q1).xyz; (*q2).xyz = t;
     t = (t & deg_part) >> 28;
     (*q2).mw = rvar_int[t] * (*q1).mw;
     (*q2++).tnext = ++k;
     k1 = (*q1).tnext;
     q1 = p1 + k1;}
while (k1);
(*--q2).tnext = 0;
q2 = p3; k = 0; xyz3 = 0;
while (1) {
     multiplier_s = (*q3s).mw;
     multiplier_c = (*q3c).mw;
     q33s = q3s; q33c = q3c;
     q22 = q2;
     while (1) {
          t = xyz3 + (*q22).xyz;
          if (t > d_max) break; k1 = (*q33s).tnext;
          while ((*q33s).xyz < t && k1 != 0) {
               q333s = q33s; q333c = q33c; q33s = p2s + k1; q33c = p2c + k1;
               k1 = (*q33s).tnext;}
          if ((*q33s).xyz > t) {q33s = q333s; q33c = q333c;}
          if ((*q33s).xyz == t) {
               (*q33s).mw = (*q33s).mw + multiplier_c * (*q22).mw;
               (*q33c).mw = (*q33c).mw + multiplier_s * (*q22).mw;}
          else {
               k1 = (*q33s).tnext;
               (*q33s).tnext = ++k; (*q33c).tnext = k;
               q33s = p2s + k; q33c = p2c + k;
               (*q33s).xyz = t; (*q33c).xyz = t;
               (*q33s).mw = multiplier_c * (*q22).mw;
               (*q33c).mw = multiplier_s * (*q22).mw;
               (*q33s).tnext = k1; (*q33c).tnext = k1;}
          if ((*q22++).tnext == 0) break;}
     k1 = (*q3s).tnext;
     if (k1) {
          q3s = p2s + k1; q3c = p2c + k1; xyz3 = (*q3s).xyz;
          t = (xyz3 & deg_part) >> 28;
          (*q3s).mw = (*q3s).mw / rvar_int[t];
          (*q3c).mw = -(*q3c).mw / rvar_int[t];}
     else break;}
if (code == cos_) return c;
else if (code == sin_) return s;
else {s = s / c; return s;}
}

series sinh_cosh(series& a, int code)
{
long d_max = series::max_deg, xyz1, xyz3, t;
series s(series::sstack), c(series::sstack), g(series::sstack);
interval multiplier_s, multiplier_c;
series::term *p1 = a.se->comp, *p2s = s.se->comp, *p2c = c.se->comp,
     *p3 = g.se->comp, *q1 = p1, *q2 = p3, *q22, *q3s = p2s, *q3c = p2c,
     *q33s, *q33c, *q333s, *q333c;
int k, k1;

(*q3s).mw = sinh((*q1).mw);
(*q3c).mw = cosh((*q1).mw);
(*q3s).xyz = 0; (*q3s).tnext = 0;
(*q3c).xyz = 0; (*q3c).tnext = 0;
if ((*q1).tnext == 0) {
     if (code == cosh_) return c;
     else if (code == sinh_) return s;
     else {(*q3s).mw = (*q3s).mw / (*q3c).mw; return s;}}
q1 = p1 + (*q1).tnext; k = 0;
do {
     t = (*q1).xyz; (*q2).xyz = t;
     t = (t & deg_part) >> 28;
     (*q2).mw = rvar_int[t] * (*q1).mw;
     (*q2++).tnext = ++k;
     k1 = (*q1).tnext;
     q1 = p1 + k1;}
while (k1);
(*--q2).tnext = 0;
q2 = p3; k = 0; xyz3 = 0;
while (1) {
     multiplier_s = (*q3s).mw;
     multiplier_c = (*q3c).mw;
     q33s = q3s; q33c = q3c;
     q22 = q2;
     while (1) {
          t = xyz3 + (*q22).xyz;
          if (t > d_max) break; k1 = (*q33s).tnext;
          while ((*q33s).xyz < t && k1 != 0) {
               q333s = q33s; q333c = q33c; q33s = p2s + k1; q33c = p2c + k1;
               k1 = (*q33s).tnext;}
          if ((*q33s).xyz > t) {q33s = q333s; q33c = q333c;}
          if ((*q33s).xyz == t) {
               (*q33s).mw = (*q33s).mw + multiplier_c * (*q22).mw;
               (*q33c).mw = (*q33c).mw + multiplier_s * (*q22).mw;}
          else {
               k1 = (*q33s).tnext;
               (*q33s).tnext = ++k; (*q33c).tnext = k;
               q33s = p2s + k; q33c = p2c + k;
               (*q33s).xyz = t; (*q33c).xyz = t;
               (*q33s).mw = multiplier_c * (*q22).mw;
               (*q33c).mw = multiplier_s * (*q22).mw;
               (*q33s).tnext = k1; (*q33c).tnext = k1;}
          if ((*q22++).tnext == 0) break;}
     k1 = (*q3s).tnext;
     if (k1) {
          q3s = p2s + k1; q3c = p2c + k1; xyz3 = (*q3s).xyz;
          t = (xyz3 & deg_part) >> 28;
          (*q3s).mw = (*q3s).mw / rvar_int[t];
          (*q3c).mw = (*q3c).mw / rvar_int[t];}
     else break;}
if (code == cosh_) return c;
else if (code == sinh_) return s;
else {s = s / c; return s;}
}

void test(series& s, int format, int k)
{
series::term *p = s.se->comp, *q = p; int i;
do {
     test((*q).mw.mid, format, k);
     i = (*q).tnext; q = p + i;}
while (i);
}
