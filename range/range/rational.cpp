#include "range.h"
#include "rational.h"

#ifdef four_digits

const int m_digits = 4;
const int m_base = 10000;
const long m_base_squared_div10 = 10000000;

#endif

#ifdef six_digits

const int m_digits = 6;
const int m_base = 1000000;
const long m_base_squared_div10 = 100000000000;

#endif

#ifdef eight_digits

const int m_digits = 8;
const int m_base = 100000000;
const long m_base_squared_div10 = 1000000000000000;

#endif

rational::rational(rvar& a, rvar& b)
{
if ((a.rv->len && a.rv->rnge) || (b.rv->len && b.rv->rnge)) {
     error("rational creation with non-exact rvars");
     numer = one; denom = one;}
else {
     numer = a; denom = b; temporary_top_precision();
     while( !is_int(numer) || !is_int(denom) ) {
          numer = ten * numer; denom = ten * denom;}
     reduce(); restore_precision();}
}

rational::rational(long a, long b)
{
numer = rvar(a); denom = rvar(b); reduce();
}

rational::rational() {denom = one;}

rational::rational(rational& a) {numer = a.numer; denom = a.denom;}

rational operator+(rational& a, rational& b)
{
rational c; temporary_top_precision();
c.numer = a.numer * b.denom + a.denom * b.numer;
c.denom = a.denom * b.denom;
c.reduce(); restore_precision(); return c;
}

rational operator-(rational& a)
{
rational b; temporary_top_precision(); b.numer = -a.numer;
restore_precision(); b.denom = a.denom; return b;
}

rational operator-(rational& a, rational& b)
{
rational c; temporary_top_precision();
c.numer = a.numer * b.denom - a.denom * b.numer;
c.denom = a.denom * b.denom;
c.reduce(); restore_precision(); return c;
}

rational operator*(rational& a, rational& b)
{
rational c; temporary_top_precision();
c.numer = a.numer * b.numer; c.denom = a.denom * b.denom; 
c.reduce(); restore_precision(); return c;
}

rational operator/(rational& a, rational& b)
{
rational c; temporary_top_precision();
c.numer = a.numer * b.denom; c.denom = a.denom * b.numer;
c.reduce(); restore_precision(); return c;
}

rational operator^(rational& a, rvar& b)
{
rational c = r_one, d; rvar b1, t;
if (!is_int(b)) {
     error("rational a ^ b with b not an integer");}
else if (is_zero(a.numer) && (b <= zero)) {
     error("rational a ^ b with a zero and b not a positive integer");}
else {
     if (b < zero) {
          temporary_top_precision(); b1 = -b; restore_precision();
          d = c / a;}
     else {
          b1 = b; d = a;}
     if (!is_zero(b1)) {
          do {
               temporary_top_precision();
               t = trunc(div2(b1)); b1 = b1 - t - t; restore_precision();
               if (!is_zero(b1)) {
                    c = c * d;}
               if (!is_zero(t)) {
                    d = d * d; b1 = t;}
               else break;}
          while (1);}}
return c;
}

rational& rational::operator=(rational& a)
{
numer = a.numer; denom = a.denom; return *this;
}

void rational::operator+=(rational& a) {*this = *this + a;}

void rational::operator-=(rational& a) {*this = *this - a;}

void rational::operator*=(rational& a) {*this = *this * a;}

void rational::operator/=(rational& a) {*this = *this / a;}

int operator==(rational& a, rational& b)
{
if (a.numer == b.numer && a.denom == b.denom) return TRUE;
else return FALSE;
}

int operator!=(rational& a, rational& b)
{
if (a.numer != b.numer || a.denom != b.denom) return TRUE;
else return FALSE;
}

int operator>(rational& a, rational& b)
{
temporary_top_precision();
rvar c = a.numer * b.denom, d = a.denom * b.numer; restore_precision();
if (c > d) return TRUE; else return FALSE;
}

int operator>=(rational& a, rational& b)
{
temporary_top_precision();
rvar c = a.numer * b.denom, d = a.denom * b.numer; restore_precision();
if (c >= d) return TRUE; else return FALSE;
}

int operator<(rational& a, rational& b)
{
temporary_top_precision();
rvar c = a.numer * b.denom, d = a.denom * b.numer; restore_precision();
if (c < d) return TRUE; else return FALSE;
}

int operator<=(rational& a, rational& b)
{
temporary_top_precision();
rvar c = a.numer * b.denom, d = a.denom * b.numer; restore_precision();
if (c <= d) return TRUE; else return FALSE;
}

rational abs(rational& a)
{
if (a.numer >= zero) return a;
else {
     rational b = -a; return b;}
}

void list_tokens(rational& dummy)
{
rational r = dummy;
cout << "\
SYMBOLS RECOGNIZED:    constants         Operations             Evaluation\n\
                    (    )   abs()     low rank: + -          left to right\n\
 Constants may use decimal point       med rank: * /          left to right\n\
  and e notation for powers of 10     high rank: ^ -(prefix) right to left\n\n";
}

int no_rational_error(svar& entry_line)
{
char *p = !entry_line, c;
int i = entry_line.len() - 3, j = (p[i] << 7) + p[i+1];
p = p + j;
while (1) {
     c = *p;
     switch (c) {
          case pi_const_: case acos_: case asin_: case atan_: case cos_:
          case cosh_: case exp_: case log_: case sin_: case sinh_: 
          case sqrt_: case tan_: case tanh_: case 'i':
          errorstatus = display_error;
          error("rational syntax");
          show_error_pos (entry_line, (*(p+1) << 7) + *(p+2));
          errorstatus = exit_on_error;
          return FALSE;
          case end_: return TRUE;}
     p += 3;}
}

void entry(char* message, rational& cc, svar& entry_line, svar& variables)
{
int i = rational::evaluate_error_display, j = rational::evaluate_exit_on_error;
// save the evaluate settings; entry needs to impose own conditions
rational::evaluate_error_display = TRUE;
rational::evaluate_exit_on_error = FALSE;

int programmed_entry_line, first_cycle = TRUE;
if (*message) programmed_entry_line = FALSE; else programmed_entry_line = TRUE;
     
while (1) { 
     if (programmed_entry_line) {
          if (first_cycle) first_cycle = FALSE;
          else exit(1);}
     else svar_entry(message, entry_line);
     compile(entry_line, variables);
     if (errorstatus == no_error) {
          if (no_rational_error(entry_line) ) {
               if (variables == empty) { // constant entry
                    evaluate (cc, entry_line); 
                    if (rational::evaluate_error == FALSE) break;}
               else break;}}}
rational::evaluate_error_display = i;
rational::evaluate_exit_on_error = j;
}

void evaluate (rational& result, svar& entry_line, rational* x)
{
char *p = entry_line.tokens(), *q = !entry_line, *q1, c;
int i = -1; rational r; rvar a;
if (rational::evaluate_error_display == TRUE) errorstatus = display_error;
else if (rational::evaluate_exit_on_error == FALSE) errorstatus = signal_error;
else errorstatus = exit_on_error;
while (1) {
     c = *p;
     switch (c) {
          case end_: rational::evaluate_error = FALSE; result = r_num[0];
          errorstatus = exit_on_error; return;
          
          case const_: q1 = q + ((*(p+1) << 7) + *(p+2)); a = rvar(q1);
          r_num[++i] = rational(a); break;
          
          case var_: r_num[++i] = x[*(p+2)]; break;
          case unary_minus_: r_num[i] = - r_num[i]; break;
          case abs_: r_num[i] = abs(r_num[i]); break;
          case '+': r = r_num[i--]; r_num[i] = r_num[i] + r; break;
          case '-': r = r_num[i--]; r_num[i] = r_num[i] - r; break;
          case '*': r = r_num[i--]; r_num[i] = r_num[i] * r; break;
          case '/': r = r_num[i--]; r_num[i] = r_num[i] / r; break;
          case '^': case power_even_p_odd_q: case power_odd_p_odd_q:
          r = r_num[i--]; 
          if (r.denom != one) {
               error("rational a ^ b with b not an integer");
               result = r_one; break;}
          r_num[i] = r_num[i] ^ r.numer; break;
          
          default: error("rational specification faulty");}
     if (errorstatus == error_reported) {
          rational::evaluate_error = TRUE;
          if (rational::evaluate_error_display == TRUE) {
               show_error_pos(entry_line, (*(p+1) << 7) + *(p+2));}
          if (rational::evaluate_exit_on_error == TRUE) exit(1);
          errorstatus = exit_on_error;
          return;}
     else p += 3;} 
}

svar rtos(rational& a)
{
svar s1 = rtos(a.numer, 3), s2(" / "), s3 = rtos(a.denom, 3);
s1 = s1 + s2 + s3; return s1;
}

void swap(rational& a, rational& b)
{
swap(a.numer, b.numer); swap(a.denom, b.denom);
}

void rational::reduce()
{
// If the sign of denom is negative, then the signs of both numer and denom
// are changed. The greatest common divisor d of abs(numer) and denom is found
// by the Euclidean Algorithm. If d > 1 then numer and denom are divided by d.

if (denom.rv->len == 0) {
     error ("rational number with zero denom");
     numer = one; denom = one; return;}
if (denom.rv->sgn) {
     numer = -numer; denom = -denom;}
if ((numer.rv->len && numer.rv->rnge) || denom.rv->rnge) {
     error("rational numer or denom with nonzero range");
     numer = one; denom = one; return;}
if (numer.rv->len == 0) {
     denom = one; return;}

int a_len_inc, b_len_inc, gcd_exp_inc;
a_len_inc = numer.rv->exp - numer.rv->len;
b_len_inc = denom.rv->exp - denom.rv->len;
gcd_exp_inc = a_len_inc;
if (b_len_inc < gcd_exp_inc) gcd_exp_inc = b_len_inc;
a_len_inc = a_len_inc - gcd_exp_inc; b_len_inc = b_len_inc - gcd_exp_inc;
int a_len = numer.rv->len + a_len_inc, b_len = denom.rv->len + b_len_inc;
rvar a1(a_len, '\0'); a1.rv->exp = 0;
rvar b1(b_len, '\0'); b1.rv->exp = 0;
int i, j, k;
for (i = numer.rv->len; i < a_len; i++) a1.rv->m[i] = 0;
for (i = numer.rv->len - 1; i >= 0; i--) a1.rv->m[i] = numer.rv->m[i];
for (i = denom.rv->len; i < b_len; i++) b1.rv->m[i] = 0;
for (i = denom.rv->len - 1; i >= 0; i--) b1.rv->m[i] = denom.rv->m[i];
//a1, b1 exp is used for the index of the first nonzero mantissa unit;
rvar *hi, *lo, *t;
if (a_len >= b_len) {
     hi = &a1; lo = &b1;}
else {
     hi = &b1; lo = &a1;}

long hi_trial, lo_trial, lo_trial1, r_trial, base = m_base, 
     base1 = m_base_squared_div10, temp;
int carry, lo_end, lo_start, hi_end, hi_start, hi_head, hi_len,
     count = m_digits;
// beginning of Euclidean cycle
do {
     lo_end = (*lo).rv->exp; lo_start = (*lo).rv->len - 1;
     if (lo_end < lo_start) {
          lo_trial1 = (*lo).rv->m[lo_end] * base + (*lo).rv->m[lo_end+1];}
     else lo_trial1 = (*lo).rv->m[lo_end] * base;
     hi_end = (*hi).rv->exp; hi_start = hi_end + lo_start - lo_end;
     hi_len = (*hi).rv->len; hi_head = 0;
     do {
          hi_trial = hi_head * base + (*hi).rv->m[hi_end];
          lo_trial = lo_trial1;
          for (k = 0; k < count; k++) {
               if (hi_trial < base1) hi_trial = hi_trial * 10 + 9;
               else lo_trial /= 10;}
          r_trial = hi_trial / lo_trial;
          if (r_trial) {
               j = hi_start; carry = 0;
               for (i = lo_start; i >= lo_end; i--) {
                    temp = (*lo).rv->m[i] * r_trial + carry;
                    carry = temp / base;
                    temp = (*hi).rv->m[j] + carry * base - temp;
                    if (temp < 0) {
                         (*hi).rv->m[j--] = base + temp; carry++;}
                    else (*hi).rv->m[j--] = temp;}
               hi_head -= carry;
               while (hi_head < 0) {
                    j = hi_start; carry = 0;
                    for (i = lo_start; i>= lo_end; i--) {
                         temp = (*hi).rv->m[j] + (*lo).rv->m[i] + carry;
                         if (temp >= base) {
                              (*hi).rv->m[j--] = temp - base; carry = 1;}
                         else {
                              (*hi).rv->m[j--] = temp; carry = 0;}}
                    hi_head += carry;}}
          hi_head = (*hi).rv->m[hi_end++];}
     while (++hi_start < hi_len);
     while (hi_head == 0 && hi_end < hi_len) {
          hi_head = (*hi).rv->m[hi_end++];}
     (*hi).rv->exp = --hi_end;
     t = hi; hi = lo; lo = t;}
while (hi_head);

lo = hi;     
// At this point lo has gcd.
lo_end = (*lo).rv->exp; lo_start = (*lo).rv->len - 1;
if (gcd_exp_inc == 0 && lo_end == lo_start && (*lo).rv->m[lo_end] == 1) {
     return;} // gcd = 1
// numer and denom now are divided by lo  
rvar a2(a_len, numer.rv->sgn); a2.rv->rnge = 0;
rvar b2(b_len, '\0'); b2.rv->rnge = 0;
for (i = numer.rv->len; i < a_len; i++) a2.rv->m[i] = 0;
for (i = numer.rv->len - 1; i >= 0; i--) a2.rv->m[i] = numer.rv->m[i];
for (i = denom.rv->len; i < b_len; i++) b2.rv->m[i] = 0;
for (i = denom.rv->len - 1; i >= 0; i--) b2.rv->m[i] = denom.rv->m[i];
if (lo_end < lo_start) {
     lo_trial1 = (*lo).rv->m[lo_end] * base + (*lo).rv->m[lo_end+1];}
else lo_trial1 = (*lo).rv->m[lo_end] * base;
for (int q = 1; q <= 2; q++) {
     if (q == 1) {
          hi = &a2; hi_len = a_len;}
     else {
          hi = &b2; hi_len = b_len;}
     hi_end = 0; hi_start = lo_start - lo_end;
     hi_head = 0;
     do {
          hi_trial = hi_head * base + (*hi).rv->m[hi_end];
          lo_trial = lo_trial1;
          for (k = 0; k < count; k++) {
               if (hi_trial < base1) hi_trial = hi_trial * 10 + 9;
               else lo_trial /= 10;}
          r_trial = hi_trial / lo_trial;
          if (r_trial) {
               j = hi_start; carry = 0;
               for (i = lo_start; i >= lo_end; i--) {
                    temp = (*lo).rv->m[i] * r_trial + carry;
                    carry = temp / base;
                    temp = (*hi).rv->m[j] + carry * base - temp;
                    if (temp < 0) {
                         (*hi).rv->m[j--] = base + temp; carry++;}
                    else (*hi).rv->m[j--] = temp;}
               hi_head -= carry;
               while (hi_head < 0) {
                    j = hi_start; carry = 0; r_trial--;
                    for (i = lo_start; i>= lo_end; i--) {
                         temp = (*hi).rv->m[j] + (*lo).rv->m[i] + carry;
                         if (temp >= base) {
                              (*hi).rv->m[j--] = temp - base; carry = 1;}
                         else {
                              (*hi).rv->m[j--] = temp; carry = 0;}}
                    hi_head += carry;}}
          hi_head = (*hi).rv->m[hi_end];
          if (r_trial >= base) {
               r_trial -= base; j = hi_end - 1;
               while (++((*hi).rv->m[j]) == base) (*hi).rv->m[j--] = 0;}
          (*hi).rv->m[hi_end++] = r_trial;}
     while (++hi_start < hi_len);
     hi_len = hi_end; (*hi).rv->exp = hi_len;
     while ((*hi).rv->m[--hi_end] == 0) hi_len--;
     while ((*hi).rv->m[0] == 0) {
          hi_len--; (*hi).rv->exp--;
          for (j=0; j<hi_len; j++) (*hi).rv->m[j] = (*hi).rv->m[j+1];}
     (*hi).rv->len = hi_len;}
numer = a2; denom = b2; return;
}








