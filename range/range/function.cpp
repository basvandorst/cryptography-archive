rvar abs (rvar& a)
{
rvar b; int a_len = a.rv->len;
if (a_len == 0) b = a;
else if (a_len > 1 || a.rv->rnge <= a.rv->m[0]) {
     if (a.rv->sgn) b = -a; else b = a;}
else {
     // a contains zero point and is not an exact zero
     // return interval [0, c] with c >= max distance of a from 0
     b = rvar(1, '\0');
     b.rv->m[0] = b.rv->rnge = a.rv->rnge;
     b.rv->exp = a.rv->exp;}
return b;
}

rvar acos (rvar& a)
{
rvar b = div2(pi) - asin(a); return b;
}

void add_precision (unsigned a)
{
a += test_failure;
unsigned i = a / m_digits; if (a % m_digits) ++i;
i += rvar::precision;
if (i > m_max) i = m_max;
rvar::precision = i; rvar::old_precision = i;
test_failure = 0;
}

rvar asin (rvar& a)
{
rvar b;
if (a < zero) b = -a; else b = a;
if (b > one) { 
     error("asin or acos argument out of bounds"); return zero;}
if (b == one && rvar::strict_ambiguity_treatment == TRUE) {
     if (mid(b) + range(b) > one) {
          error("asin or acos argument an approximation to +1 or -1");
          return zero;}}
b = atan( a / (one + sqrt(one - a * a)));
// Use the identity tan(x/2) = sin(x) / (1 + cos(x)).
b += b;
return b;
}

rvar atan (rvar& a)
{
int nonzero_rnge; rvar r, a1;
if (a.rv->rnge) {
     nonzero_rnge = TRUE;
     r = a * a; r = mid(r) - range(r);
     if (r <= zero) r = zero;
     r = range(a) / (one + r); a1 = mid(a);}
else {
     nonzero_rnge = FALSE; a1 = a;}
int sign_change;
if (a1.rv->sgn) {
     sign_change = TRUE; a1 = -a1;}
else sign_change = FALSE;
int a_recip;
if (a1 > one) {
     a_recip = TRUE; a1 = one / a1;}
else a_recip = FALSE;
int doubles = 0;
while (a1 > tenth) {
     doubles++; a1 = a1 / (one + sqrt(one + a1 * a1));}
//  Use the identity tan(x) / (1 + sec(x) = tan(x/2).
//  At most three doublings will be needed.
rvar term, factor = a1, multiplier = -(a1 * a1), sum = a1;
rvar abs_term, range_sum; long denom = 3;
do {
     factor = factor * multiplier; term = factor / rvar(denom);
     sum = sum + term; denom += 2;
     abs_term = abs(term); range_sum = range(sum);}
while (abs_term > range_sum);
sum = sum % term;
while (doubles--) sum = sum + sum;
if (a_recip) sum = div2(pi) - sum;
if (sign_change) sum = - sum;
if (nonzero_rnge) sum = sum % r;
return sum;
}

rvar cos (rvar& a)
{
// series used is 1- x^2/2! + x^4/4! - ....
int sign_change, i; rvar a1, b, c, r, sum;
r = range(a); a1 = mid(abs(a));
if (a1.rv->len == 0) {
     r = one % r; return r;}
if (a1 > one) {
     b = a1 / pi; c = trunc(b); b = b -  c; i = c.rv->len;
     if (i == 0 || c.rv->exp != i || (c.rv->m[--i] % 2) == 0) {
          sign_change = FALSE;}
     else sign_change = TRUE;
     c = one - b;
     if (c < b) {
          sign_change = ! sign_change; b = c;}
     a1 = b * pi;}
else sign_change = FALSE;
int divisions_by_2 = 0;
while (a1 > tenth) {divisions_by_2++; a1 = div2(a1);}
c = - a1 * a1; b = div2(c); sum = one + b;
long denom, n = 2; rvar abs_b, range_sum;
do {
     n += 2; denom = n * (n-1); b = b * c /rvar(denom); sum = sum + b;
     abs_b = abs(b); range_sum = range(sum);}
while (abs_b > range_sum);
sum = sum % b; // series truncation error is first term not summed
while (divisions_by_2--) sum = sum * (sum + sum) - one;
if (sign_change) sum = - sum;
sum = sum % r;
return sum;
}

rvar cosh (rvar& a)
{
rvar b = exp(a); b = div2(b + one/b);
return b;
}

unsigned current_precision()
{
return rvar::precision * m_digits;
}

rvar div2 (rvar& a)
{
// fast division by 2 
int a_len = a.rv->len; if (a_len == 0) return a;
int a_rnge, b_exp = a.rv->exp;
if (a_len > rvar::precision &&
(a.rv->rnge != 0 || rvar::exact_arithmetic == FALSE) ) {
     a_len = rvar::precision; a_rnge = 2;}
else a_rnge = a.rv->rnge;
rvar b(a_len+1, a.rv->sgn); int i = 0, j = 0, carry = 0, temp, temp1;
do {
     temp = a.rv->m[i++]; temp1 = temp >> 1; b.rv->m[j++] = temp1 + carry;
     if ( (temp1 << 1) < temp) carry = m_base_half; else carry = 0;}
while (i < a_len);
if (a_rnge == 1) {
     b.rv->m[j++] = carry; 
     a_rnge = m_base_half;}
else if (a_rnge) {
     temp = a_rnge >> 1; 
     if ( (temp << 1) < a_rnge || carry > 0) temp++; 
     a_rnge = temp;}
else if (carry) b.rv->m[j++] = carry;
if (b.rv->m[0] == 0 && j > 1) {// units must be moved
     for (i=1; i<j; i++) b.rv->m[i-1] = b.rv->m[i];
     if (--b_exp < m_min) {
          error ("exp underflow on div2"); return zero;}
     j--;}
b.rv->len = j;
b.rv->exp = b_exp; b.rv->rnge = a_rnge;
return b;
}

void error (char* error_message)
{
if (errorstatus == exit_on_error || errorstatus == display_error) {
     cout << "Error: " << error_message << "  \n";
     if (errorstatus == exit_on_error) exit(1);
     errorstatus = error_reported;}
else if (errorstatus == signal_error) errorstatus = error_reported;
}

rvar exp (rvar& a)
{
// series used is 1 + x/1! + x^2/2! + x^3/3! + ...
int a_rnge_nonzero; rvar a1, r;
if (a.rv->rnge) {
     a_rnge_nonzero = TRUE; r = range(a); a1 = mid(a) + r;}
else {
     a_rnge_nonzero = FALSE; a1 = a;}
if (a1.rv->len == 0) {
     if (a_rnge_nonzero) {r = one % r; return r;}
     else return one;}
int a_negative;
if (a1.rv->sgn) {
     a_negative = TRUE; a1 = -a1;}
else a_negative = FALSE;
if (a1.rv->exp > 2) {
     error ("exp argument out of bounds"); return zero;}
int divisions_by_2 = 0;
while (a1 > tenth) {
     ++divisions_by_2; a1 = div2(a1);}
long denom = 1;
rvar sum = one + a1, term = a1, range_sum;
do {
     term = term * a1 / rvar(++denom); sum = sum + term;
     range_sum = range(sum);}
while (term > range_sum);
sum = sum % term;    // series truncation error < 2 * next term < term
while (divisions_by_2--) sum = sum * sum;
if (a_negative) sum = one / sum;
if (a_rnge_nonzero) sum = sum % (r * sum);
return sum;
}

rvar expon_e (rvar& a, rvar& b)
{
// a ^ b with b = p / q with p even, q odd
rvar c;
if ( (a > zero) || is_int(b) ) c = a ^ b; else c = abs(a) ^ b;
return c;
}
     
rvar expon_o (rvar& a, rvar& b)
{
// a ^ b with b = p / q with p odd, q odd
rvar c;
if ( (a > zero) || is_int(b) ) c = a ^ b;
else if (a < zero) c = - ((-a) ^ b);
else {
     // a has 0 point in interval and b is not an integer
     c = abs(a) ^ b;}
return c;
}
     
rvar log (rvar& a)
{
//series used is log(x) = 2[u + u^3/3 + u^5/5 + ...] where u = (x-1)/(x+1)
int a_len = a.rv->len;
if (a_len == 0) {
       error("log(a) with zero a"); return zero;}
else if (a.rv->sgn == '-' && (a_len > 1 || a.rv->m[0] >= a.rv->rnge) ) {
     error("log(a) with non-positive a"); return zero;}
else if (a_len == 1 && a.rv->m[0] <= a.rv->rnge) {
     // a interval contains 0 point and positive numbers.
     error("log(a) with zero approximation a");
     return zero;}
int nonzero_rnge; rvar a1 = mid(a), r;
if (a.rv->rnge) {
     nonzero_rnge = TRUE; r = range(a);
     r = r / (a1 - r);}
else nonzero_rnge = FALSE;
long power10 = (a1.rv->exp - 1) * (long) m_digits;
a1.rv->exp = 1;
while (a1 > ten) {
     power10++; a1 = a1 / ten;}
rvar test_value = "1.01";
int power2 = 2;
while (a1 > test_value) {
     a1 = sqrt(a1); power2 *= 2;} // max number of sqrts is 8
long denom = 3;
rvar power  = (a1 - one) / (a1 + one), multiplier = power * power;
rvar term, sum = power;
if (!is_zero(power)) {
     do {
          power *= multiplier;
          term =  power / rvar(denom); 
          denom += 2;
          if (term  >= range(sum)) sum += term;
          else {
               // series truncation error < 2 * term
               sum = sum % (term + term); 
               break;}}
     while (1);}
sum  = sum * rvar(power2) + rvar(power10) * logten;
if (nonzero_rnge) sum = sum % r;
return sum;
}

rvar max (rvar& a, rvar& b)
{
rvar c;
int i = compare(a, b);
if (i == HI) c = a;
else if (i == LO) c = b;
else {
     rvar ra = range(a), rb = range(b);
     temporary_top_precision();
     i = compare (mid(a) + ra, mid(b) + rb);
     restore_precision();
     if (i == HI) c = a;
     else if (i== LO) c = b;
     else {
          i = compare (ra, rb);
          if (i == LO) c = a;
          else c = b;}}
return c;
}
     
rvar mid (rvar& a)
{
int a_len = a.rv->len;
if (a_len == 0)  return zero;
while (a.rv->m[--a_len] == 0) ; // to remove any terminal 0 elements
a_len++;
rvar b(a_len, a.rv->sgn);
b.rv->exp = a.rv->exp; b.rv->rnge = 0;
for (int i=0; i<a_len; i++) b.rv->m[i] = a.rv->m[i];
return b;
}
     
rvar min (rvar& a, rvar& b)
{
rvar c;
int i = compare(a, b);
if (i == LO) c = a;
else if (i == HI) c = b;
else {
     rvar ra = range(a), rb = range(b);     
     temporary_top_precision();
     i = compare (mid(a) - ra, mid(b) - rb);
     restore_precision();
     if (i == LO) c = a;
     else if (i== HI) c = b;
     else {
          i = compare (ra, rb);
          if (i == LO) c = a;
          else c = b;}}
return c;
}

rvar range (rvar& a)
{
if (a.rv->len == 0 || a.rv->rnge == 0) return zero;
rvar b(1, '\0');
b.rv->rnge = 0;
b.rv->exp = a.rv->exp - a.rv->len + 1;
if (b.rv->exp < m_min) {
     error("exp underflow on range operation"); return zero;}
b.rv->m[0] = a.rv->rnge;
return b;
}

void set_precision (unsigned a)
{
unsigned i = a / m_digits; if (a % m_digits) ++i;
if (i < 3) i = 3; 
if (i > m_max) i = m_max;
rvar::precision = i; rvar::old_precision = i;
return;
}

rvar sin (rvar& a)
{
rvar b = cos(a - div2(pi));
return b;
}

rvar sinh (rvar& a)
{
rvar b = exp(a);
b = div2(b - one/b);
return b;
}

rvar sqrt_rnge(rvar& a)
{
// Result returned is 0 with a range r such that
// r^2 >= right endpoint of the a interval.
// This function is never called if rvar a right end point < 0.
long rnge_squared;
if (a.rv->len > 1) rnge_squared = a.rv->m[0] + 2;
else {
     rnge_squared = a.rv->rnge;
     if (a.rv->sgn) rnge_squared -= a.rv->m[0];
     else rnge_squared += a.rv->m[0];
     if (rnge_squared == 0) return zero;}
rvar b(1, '\0');
b.rv->m[0] = 0;
b.rv->exp = a.rv->exp / 2;
if (b.rv->exp * 2 - a.rv->exp) {
     if (a.rv->exp > 0) b.rv->exp++;}
else rnge_squared *= m_base;
long b_test = 1; int b_rnge = 1;
while (b_test < rnge_squared) {
     b_test = b_test << 2; b_rnge = b_rnge << 1;}
if (b_rnge > m_max) {
     b.rv->exp++;
     b_rnge = b_rnge / m_base + 1;}
b.rv->rnge = b_rnge;
return b;
}

rvar sqrt (rvar& a)
{
// A version of the square root algorithm of arithmetic.
// a is copied to rvar c, and subtractions from c occur to get sqrt a.
// The sqrt is formed by cyclic updating of 2 * sqrt approximation,
// held in rvar b. As a new increment, delta, to sqrt is found, it is
// added to b, then b * delta is subtracted from c, and finally delta
// is added again to b, to form the next 2 * sqrt approximation.
// c always equals a - (b/2)^2. Note that 
//   c - [b + delta] * delta = [a - (b/2)^2] - b * delta - delta^2
//   = a - [b/2 + delta]^2 = a - [(b + 2*delta)/2]^2 = a - (b'/2)^2
// After completion of approximation process, b is divided by 2
// to get sqrt.
int a_len = a.rv->len;
if (a_len == 0) return zero;
else if (a_len > 1 || a.rv->rnge < a.rv->m[0]) {
     // a interval does not contain zero point
     if (a.rv->sgn) { 
          error ("sqrt(a) with negative a"); return zero;}}
else { // a interval contains zero point
     if (a.rv->sgn || (a.rv->rnge > a.rv->m[0]) ) {
          // a interval left endpoint < 0
          if (rvar::strict_ambiguity_treatment == TRUE) {
               error("sqrt(a) with zero approximation a");
               return zero;}}
     rvar b = sqrt_rnge(a); return b;}
int c_len = rvar::precision;
if (a.rv->rnge > 0 && a_len < c_len) c_len = a_len;
int c_max = c_len, c_data_end = a_len - 1;
if (c_data_end > c_max) c_data_end = c_max;
rvar c(++c_len, '\0'); rvar b(c_len, '\0');
for (int i=1; i<=c_data_end; i++) c.rv->m[i] = a.rv->m[i];
long c_rnge_hi = 0, c_rnge_lo = 0;
i = a_len - c_max;
if (i == 0) c_rnge_hi = a.rv->rnge;
else if (i == 1) c_rnge_lo = a.rv->rnge;
else if (i > 1) c_rnge_lo = 2;

long c_lead = a.rv->m[0] * m_base_long;
int c_shift = 1, c_shift1, b_shift = 1, b_shift1;
if (1 <= c_data_end) c_lead += c.rv->m[1];
else {c_data_end++; c.rv->m[1] = 0;}
while (c_lead < m_base_squared_div100) {
     b_shift *= 10;
     c_shift *= 100;
     c_lead *= 100;}
if (c_shift > 1) {
     c_shift1 = m_base / c_shift;
     if (2 <= c_data_end) {
          i = c.rv->m[2] / c_shift1;
          c_lead += i;
          c.rv->m[2] -= i * c_shift1;}
     else {
          if (2 <= c_max) {
               c_data_end = 2;
               c.rv->m[2] = 0;}}}
int a_exp = a.rv->exp, b_exp = a_exp / 2;
if (a_exp - 2 * b_exp) {
     if (a_exp > 0) b_exp++;
     b_shift *= m_base_sqrt;}
b.rv->exp = b_exp;
long b_test = m_base_squared_left_bit;
while (b_test > c_lead) b_test >>= 2;
c_lead -= b_test;
long bit_squared = b_test >> 2;

// Below the general sqrt routine is carried out bitwise
// to compute in b_test the k leading digits of sqrt (k = m_digits).

do {
     if (c_lead > b_test) {
          c_lead  -= b_test;
          if (c_lead >= bit_squared) {
               c_lead -= bit_squared; 
               b_test >>= 1; 
               b_test += bit_squared;}
          else {
               c_lead += b_test;
               b_test >>= 1;}}
     else b_test >>= 1;
     bit_squared >>= 2;}
while (bit_squared);
long b_trial = b_test << 1;
if (b_shift == 1) {
     b.rv->m[0] = b_trial;
     b.rv->m[1] = 0;}
else {
     i = b_test / b_shift;
     b.rv->m[0] = i << 1;
     b_test -= i * b_shift;
     b_shift1 = m_base / b_shift;
     b.rv->m[1] = (b_test * b_shift1) << 1;}
if (c_shift == 1) c.rv->m[1] = c_lead;
else {
     c.rv->m[1] = c_lead / c_shift;
     c_lead -= c.rv->m[1] * c_shift;
     if (2 > c_max) {
          if (c_lead) c_rnge_lo++;}
     else {
          c.rv->m[2] += c_lead * c_shift1;}}

// The regular sqrt loop starts here. Termination occurs when either
// c_index equals c_max (last c unit position),
// or goes past c_data_end with a zero carry-over from the preceding unit,
// and with c_rnge_hi and c_rnge_lo both zero.
// The second case permits the sqrt of an exact perfect square to end 
// as soon as c becomes zero. The subtractions of b from c are ranged
// subtractions, and the accumulated range is held in c_rnge_lo. 
// When the process ends, any c remainder is added to c_rnge_lo.

// The successive b mantissa units are computed by dividing c_trial by b_trial
// to form r_trial. Here b_trial has been formed by the earlier bitwise
// procedure which obtained the leading k digits of sqrt times 2.
// c_trial is formed from the two leading nonzero mantissa units of c. 
// r_trial is added to the end of b at the b_index element,
// then b * r_trial is subtracted from c, and finally r_trial
// added again at the b_index position of b.

// The quotient r_trial may be too large, so after this operation,
// if a final carry occurs on the subtraction, then a correction is
// made by reducing r_trial by 1, and adjusting b. 
// The b adjustment is done by subtracting 1 from the b_index element,
// then adding the changed b to c, and finally subtracting another 1
// from the b_index element. This is done as many times as needed to
// make c non-negative.

int b_index = 1, c_index;
if (b_shift >= m_base_sqrt) {
     c_index = 0; c.rv->m[0] = 0;}
else c_index = 1;
int b_start = b_index, c_start = c_index + 2;
if (c_start > c_max) {
     c_start--; b_start--;}
int c_hi, j; unsigned carry; long c_trial, r_trial, temp, temp1;
while (c_index < c_max) {
     c_hi = c.rv->m[c_index++];
     c_trial = c_hi * m_base_long;
     if (c_index <= c_data_end) c_trial += c.rv->m[c_index];
     else if (c_hi == 0 && c_rnge_hi == 0 && c_rnge_lo == 0) break;
     else c.rv->m[++c_data_end] = 0;
     r_trial = (c_trial * b_shift) / b_trial;
     if (r_trial) {
          while (c_data_end < c_start) c.rv->m[++c_data_end] = 0;
          if (r_trial > m_max && b.rv->m[b_index-1] == m_max_times2) {
               r_trial = m_max;} 
          // r_trial sometimes exceeds m_max, and a carry into preceding
          // b element then occurs after the subtraction of b * r_trial
          // from c, but r_trial can not be allowed to exceed m_max if
          // preceding b element equals m_max times 2, otherwise there
          // is difficulty finding the sqrt of .999999999....
          b.rv->m[b_index] += r_trial; 
          j = c_start; carry = 0;
          for (i=b_start; i>=0; i--) {
               temp = b.rv->m[i] * r_trial + carry;
               carry = temp / m_base_long;
               temp =  c.rv->m[j] + carry * m_base_long - temp;
               if (temp < 0) {
                    c.rv->m[j--] = m_base_long + temp; carry++;}
               else c.rv->m[j--] = temp;}
          c_hi -= carry; b.rv->m[b_index] += r_trial;
          while (c_hi < 0) {
               r_trial--; b.rv->m[b_index]--; j = c_start; carry = 0;
               for (i = b_start; i >= 0; i--) {
                    temp = c.rv->m[j] + b.rv->m[i] + carry;
                    if (temp >= m_base_long) {
                         carry = temp / m_base_long;
                         c.rv->m[j--] = temp - carry * m_base_long;}
                    else {
                         c.rv->m[j--] = temp; carry = 0;}}
               c_hi += carry; b.rv->m[b_index]--;}
          if (c_hi) c.rv->m[c_index] += c_hi * m_base;
          if (b_start < b_index) c_rnge_lo += r_trial + r_trial;
          if ((unsigned) b.rv->m[b_index] >= m_base_times2) {
               b.rv->m[b_index] -= m_base_times2;
               b.rv->m[b_index-1] += 2;}}
     if (++b_index < c_len) b.rv->m[b_index] = 0; else break;
     if (c_start == c_max) b_start--; 
     else {
          c_start += 2;
          if (c_start > c_max) c_start = c_max;
          else b_start++;}}

// b divided by 2 to get sqrt
for (i=0; i<b_index; i++) b.rv->m[i] >>= 1;

// Formation of b mantissa units complete; now calculate b rnge.
// First get c_rnge_hi and c_rnge_lo both less than m_base.

int b_discard = 0;
c_rnge_lo += c.rv->m[c_data_end];
if (c_rnge_lo >= m_base_long) {
     temp = c_rnge_lo / m_base_long;
     c_rnge_lo -= temp * m_base_long; c_rnge_hi += temp;}
while (c_rnge_hi >= m_base_long) {
     temp = c_rnge_hi / m_base_long;
     temp1 = c_rnge_hi - temp * m_base_long;
     if (c_rnge_lo) c_rnge_lo = ++temp1; else c_rnge_lo = temp1;
     c_rnge_hi = temp; b_discard++;}

// sqrt range is calculated as the accumulated c range times the
// maximum sqrt derivative, which is the reciprical of twice
// the minimum value of sqrt. So sqrt range = c range / rnge_div, where
// rnge_div equals 2 * (sqrt - sqrt rnge). That rnge_div involves
// sqrt rnge poses no difficulty if sqrt len > 2,
// for then rnge_div can be safely approximated. For the cases where
// sqrt len = 1 or 2, a tentative sqrt range of 1 is assumed in order to
// calculate sqrt range. If this is greater than the tentative sqrt range,
// then it replaces the tentative range, and the calculation is repeated.
// Otherwise the sqrt range value is accepted.
          
if (c_rnge_lo == 0 && c_rnge_hi == 0) {
     // b range = 0; discard terminal b mantissa units that are zero
     while (b.rv->m[b_index-1] == 0) --b_index;
     b.rv->len = b_index; b.rv->rnge = 0; return b;}

long rnge_div, c_rnge;
int tentative_b_rnge = 1, b_rnge, b_len, reduce_b_len = FALSE;
do {
     if (reduce_b_len) {
          b_discard++;
          if (c_rnge_lo) c_rnge_lo = c_rnge_hi + 1;
          else c_rnge_lo = c_rnge_hi;
          c_rnge_hi = 0;
          tentative_b_rnge = 1;
          reduce_b_len = FALSE;}
     b_len = b_index - b_discard;
     if (b_len > 1) {
          rnge_div = b.rv->m[0] * m_base_long + b.rv->m[1];
          if (b_len == 2) rnge_div -= tentative_b_rnge;
          else rnge_div -= 1;}
     else if (b_len == 1) {
          rnge_div = (b.rv->m[0] - tentative_b_rnge) * m_base_long;}
     else {
          b = sqrt_rnge(a); return b;}
     rnge_div <<= 1;
     c_rnge = c_rnge_hi * m_base_long + c_rnge_lo;
     if (c_rnge >= rnge_div) reduce_b_len = TRUE;
     else {
          for (i = m_digits; i > 0; --i) {
               if (c_rnge < m_base_squared_div10) c_rnge *= 10;
               else rnge_div /= 10;}
          b_rnge = c_rnge / rnge_div;
          if (b_rnge * rnge_div - c_rnge) b_rnge++;
          if (b_discard) b_rnge++;
          if (b_rnge > m_max) reduce_b_len = TRUE;
          else if (b_len > 2) break;
          else if (b_rnge <= tentative_b_rnge) break;
          else tentative_b_rnge = b_rnge;}}
while (1);

b.rv->len = b_len; b.rv->rnge = b_rnge;
return b;
}

void swap (rvar& a, rvar& b)
{
rvar::rva* c = a.rv;
a.rv = b.rv; b.rv = c;
}

rvar tan (rvar& a)
{
rvar b = sin(a) / cos(a);
return b;
}

rvar tanh (rvar& a)
{
rvar b = exp(a), c = one / b;
b = (b - c) / (b + c);
return b;
}

rvar trunc (rvar& a)
{
// trunc(a) = greatest integer <= abs(mid(a))
// with same sign as rvar a.
int i = a.rv->len, e = a.rv->exp;
if (i == 0 || e <= 0) return zero;
if (e < i) i = e;
while (--i >= 0 && a.rv->m[i] == 0) {;}
i++;
// above 2 lines reduce trunc len if trunc terminal element is zero
rvar b(i, a.rv->sgn);
b.rv->exp = e; b.rv->rnge = 0;
for (int j=0; j<i; ++j) b.rv->m[j] = a.rv->m[j];
return b;
}
