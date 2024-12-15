inline void swap(int& p, int& q) {int t = p; p = q; q = t;}
enum comp_return_value {OVERLAP, HI , LO };

int compare (rvar& a, rvar& b)
{
/* Three different return values distinquish the cases
     X  >  Y     :       HI 
     X  <  Y,    :       LO
 and X  == Y     :       OVERLAP

 lo(X) equals mid(X) - range(X), and hi(X) equals mid(X) + range(X). 
 Let A(X, Y) equal lo(X) - hi(Y). Then X > Y if A(X, Y) is positive,
 X < Y if A(Y, X) is positive, and X == Y if neither A(X, Y) nor A(Y, X)
 is positive. The method used here is to form as much of
 A(X, Y) as is needed to determine whether it is positive,
 and if it becomes clear that this quantity is not positive, then to switch
 to forming A(Y, X) if that quantity could be positive. The switch to 
 A(Y, X) is made by simply switching the arguments X and Y in the
 subroutine that forms A, and adjusting intermediate results.
 
 A(X,Y) equals mid(X) - mid(Y) - range(X) - range(Y). The quantity
 mid(X) - mid(Y) is an approximation to A(X, Y), and if the two leading
 mantissa units of this quantity can be formed (with the leading unit non-zero),
 then A(X, Y) is definitely positive because formation of the rest of A(X, Y)
 could lead to reducing these leading units by at most 3 (one negative carry 
 from the rest of the mid(X) - mid(Y) formation, and one negative carry from 
 each range subtraction). The ranges of X or Y are taken into account only
 when the computation of mid(X) - mid(Y) reaches the least significant
 mantissa unit of X or Y.  This is the procedure followed when the signs of
 both X and Y are plus.
 
 If the sign of X is plus and the sign of Y is minus, the procedure is similar
 except that the formation of mid(X) - mid(Y) entails an addition instead of
 a subtraction. Again, if the leading two mantissa units can be formed (with
 the leading unit non-zero), then A(X, Y) is definitely positive.
 
 If the signs of X and Y are both minus, or the sign of X is minus and the
 sign of Y is plus, we proceed as in the cases above after swapping
 the return values for the X > Y and X < Y cases.
        
 If the sign of X is plus and Y is an exact zero, the same procedure is
 followed as when both signs are plus, except that no Y input is used,
 and mid(X) - range(X) is tested to determine if it is positive. If the
 sign of X is minus and Y is an exact zero, we do the same thing,
 after swapping the return values for the X > Y and X < Y cases. */
 
rvar op1 = a, op2 = b;
int outcome_hi = HI, outcome_lo = LO;
int exp_dif; int signs_alike;

if (op1.rv->len == 0) {
     if (op2.rv->len == 0) return OVERLAP;
     exp_dif = 0; signs_alike = TRUE;
     swap(op1, op2); swap(outcome_hi, outcome_lo);}
else if (op2.rv->len == 0) { 
     exp_dif = 0; signs_alike = TRUE;}
else {
     exp_dif = op1.rv->exp - op2.rv->exp;
     if (exp_dif < 0) {
          exp_dif = -exp_dif;
          swap(op1, op2); swap(outcome_hi, outcome_lo);}
     if (op1.rv->sgn == op2.rv->sgn) signs_alike = TRUE;
     else signs_alike = FALSE;}
if (op1.rv->sgn) swap(outcome_hi, outcome_lo);

int i1 = 0, i2 = 0, carry, temp = 0, positive_op1_rnge_subtracted = FALSE;

while (exp_dif-- > 0) {
     carry = temp;
     if (i1 < op1.rv->len) temp = op1.rv->m[i1]; else temp = 0;
     if (++i1 == op1.rv->len) {
          if (op1.rv->rnge) {
               positive_op1_rnge_subtracted = TRUE;
               // op1, op2 exchange not possible after above integer set TRUE;
               temp -= op1.rv->rnge;
               if (temp < 0) {
                    if (carry--) temp += m_base; else return OVERLAP;}
               if (carry == 0 && temp == 0) break;}}
     if (carry) return outcome_hi;}

do {   // carry = 0 or is uninitialized on entry here
     carry = temp;
     if (i1 < op1.rv->len) temp = op1.rv->m[i1]; else temp = 0;
     if (i2 < op2.rv->len) {
          if (signs_alike) {
               temp -= op2.rv->m[i2];
               if (temp < 0) { 
                    if (carry) {
                         --carry; temp += m_base;}
                    else if (positive_op1_rnge_subtracted) return OVERLAP;
                    else {
                         swap(op1, op2); swap(i1, i2);
                         swap(outcome_hi, outcome_lo);
                         temp = -temp;}}}
          else temp += op2.rv->m[i2];}
     if (++i1 == op1.rv->len) {
          if (op1.rv->rnge) {
               positive_op1_rnge_subtracted = TRUE;
               temp -= op1.rv->rnge;
               if (temp < 0) {
                    if (carry--) temp += m_base; else return OVERLAP;}}}
     if (++i2 == op2.rv->len) {
          temp -= op2.rv->rnge;
          if (temp < 0) {
               if (carry--) temp += m_base; else return OVERLAP;}}
     if (carry) return outcome_hi;}
while (i1 < op1.rv->len || i2 < op2.rv->len);

if (temp) return outcome_hi; else return OVERLAP;
}
               
int operator==(rvar& op1, rvar& op2)
{
if (compare( op1, op2) == OVERLAP) return TRUE; else return FALSE;
}
     
int operator!=(rvar& op1, rvar& op2)
{
if (compare( op1, op2) != OVERLAP) return TRUE; else return FALSE;
}     
     
int operator>(rvar& op1, rvar& op2)
{
if (compare( op1, op2) == HI) return TRUE; else return FALSE;
}
     
int operator<(rvar& op1, rvar& op2)
{
if (compare( op1, op2) == LO) return TRUE; else return FALSE;
}

int operator>=(rvar& op1, rvar& op2)
{
if (compare( op1, op2) != LO) return TRUE; else return FALSE;
}
     
int operator<=(rvar& op1, rvar& op2)
{
if (compare( op1, op2) != HI) return TRUE; else return FALSE;
}
     
rvar& rvar::operator=(rvar& a)
{
a.rv->count++;
if (--rv->count == 0) return_memory(rv);
rv = a.rv; return *this;
}

void rvar :: operator+=(rvar& a) {*this = *this + a;}

void rvar :: operator-=(rvar& a) {*this = *this - a;}

void rvar :: operator*=(rvar& a) {*this = *this * a;}

void rvar :: operator/=(rvar& a) {*this = *this / a;}

rvar operator- (rvar& a)
{
int a_len = a.rv->len, a_rnge;
if (a_len == 0) return zero;
if (a_len > rvar::precision &&
(a.rv->rnge != 0 || rvar::exact_arithmetic == FALSE) ) {
     a_len = rvar::precision; a_rnge = 2;}
else a_rnge = a.rv->rnge;
rvar b(a_len, (a.rv->sgn)^ '-');
b.rv->exp = a.rv->exp; b.rv->rnge = a_rnge;
for (int i = 0; i < a_len; i++) b.rv->m[i] = a.rv->m[i];
return b;
}

enum add_or_sub {SUB = 0, ADD = 1};

rvar operator+ (rvar& a, rvar& b)
{
rvar c;
if (a.rv->len == 0) c = b;
else if (b.rv->len == 0) c = a;
else if (a.rv->sgn == b.rv->sgn) c = add_sub(a, b, ADD);
else c = add_sub(a, b, SUB);
return c;
}

rvar operator- (rvar& a, rvar& b)
{
rvar c;
if (b.rv->len == 0) c = a;
else if (a.rv->len == 0) c = -b;
else if (a.rv->sgn == b.rv->sgn) c = add_sub(a, b, SUB);
else c = add_sub(a, b, ADD);
return c;
}

rvar operator* (rvar& a1, rvar& b1)
{
if (a1.rv->len == 0) return zero; if (b1.rv->len == 0) return zero;
rvar a = a1, b = b1;
if (a.rv->len > b.rv->len) swap(a, b);
int a_len = a.rv->len, b_len = b.rv->len, r_len;
long r_rnge_hi = 0, r_rnge_lo = 0;
if (a.rv->rnge) {
     r_len = a_len + 1;
     r_rnge_hi = ((long) a.rv->rnge) * b.rv->m[0];
     if (b_len == 2) r_rnge_lo = ((long) a.rv->rnge) * (b.rv->m[1]);
     else if (b_len > 2) {
          r_rnge_lo = ((long) a.rv->rnge) * (b.rv->m[1] + 1);}
     if (b.rv->rnge) {
          if (a_len == b_len) {
               r_rnge_hi += ((long) a.rv->m[0]) * b.rv->rnge;
               if (a_len == 1) {
                    r_rnge_hi += ((long) a.rv->rnge) * b.rv->rnge;}
               else if (a_len == 2) {
                    r_rnge_lo += (a.rv->m[1] + a.rv->rnge)
                    * ((long) b.rv->rnge);}
               else r_rnge_lo += (a.rv->m[1] + 2) * ((long) b.rv->rnge);}
          else if (a_len + 1 == b_len) {
               if (a_len == 1) {
                    r_rnge_lo += (a.rv->m[0] + a.rv->rnge)
                    * ((long) b.rv->rnge);}
               else r_rnge_lo += (a.rv->m[0] + 2) * ((long) b.rv->rnge);}
          else {
               if (a_len == 1) r_rnge_lo += b.rv->rnge + b.rv->rnge;
               else r_rnge_lo += b.rv->rnge + 1;}}}
else if (b.rv->rnge) {
     r_len = b_len + 1;
     r_rnge_hi = ((long) a.rv->m[0]) * b.rv->rnge;
     if (a_len == 2) r_rnge_lo = ((long) a.rv->m[1]) * b.rv->rnge;
     else if (a_len > 2) {
          r_rnge_lo = (a.rv->m[1] + 1) * ((long) b.rv->rnge);}}
else r_len = a_len + b_len;
     
long temp, temp1;
if (r_rnge_lo > m_max) {
     temp = r_rnge_lo / m_base; r_rnge_hi += temp;
     r_rnge_lo = r_rnge_lo - temp * m_base;}
while (r_rnge_hi > m_max) {
     --r_len;
     temp1 = r_rnge_hi / m_base;
     temp = r_rnge_hi - temp1 * m_base; r_rnge_hi = temp1;
     if (r_rnge_lo > 0) r_rnge_lo = ++temp; else r_rnge_lo = temp;}
// Note that it is possible for r_len to be 0 after above routine finishes.
// This can happen if a and b both have len = 1 and large rnge.          

int t = r_len - rvar::precision - 1;
if (t > 0 &&
(r_rnge_lo > 0 || r_rnge_hi > 0 || rvar::exact_arithmetic == FALSE) ) {
     r_len -= t;
     if (t == 1) {
          if (r_rnge_lo > 0) r_rnge_lo = r_rnge_hi + 1;
          else r_rnge_lo = r_rnge_hi;}
     else {
          if (r_rnge_hi > 0 || r_rnge_lo > 0) r_rnge_lo = 1;}
     r_rnge_hi = 0;}

int a_index, b_index, b_start, r_index, r_start, carry;
int a_max = a_len -1, b_max = b_len - 1;
t = a_len - r_len; 

// The indices of the mantissa units of r range from 0 for the most significant
// to r_len - 1 for the least significant. For accuracy, an extra r mantissa
// unit beyond the least sigificant one is formed. The index position of this
// unit corresponds to the position of r_rnge_lo.  t measures the difference
// in indices between this unit and the unit that would be generated by
// multiplying the least sig. unit of a by the most sig. unit of b.
// If the above quantity is positive, then multiplication starts with
// a more significant mantissa unit of a than the last one.

if (t > 0) a_index = a_max - t; else a_index = a_max;
b_start = r_len - 1 - a_index; r_start = r_len;
if (b_start > b_max) {b_start--; r_start--;} 

// When b_start > b_max, an extra r mantissa unit is not possible.
// In this case b_start = b_len and both a and b have a rnge of zero.
// Note that b_start = 0 if a_index is not a_max.

// Now prepare r to receive the product.

int leng = r_start + 1;
rvar r(leng, a.rv->sgn ^ b.rv->sgn);
r.rv->exp = a.rv->exp + b.rv->exp;
     
// Clear enough of the r mantissa units for first multiplier cycle.
t = r_start - b_start;
for (int i = r_start; i >= t; i--) r.rv->m[i] = 0; 
     
// Multiply cycle     
long multiplier;
if (a_index < a_max) r_rnge_lo += a.rv->m[a_index+1] + 1;
while (a_index >= 0) {
     multiplier = a.rv->m[a_index--]; carry = 0;
     if (b_start < b_max) r_rnge_lo += multiplier;
     b_index = b_start++; r_index = r_start;
     while (b_index >= 0) {
          temp = multiplier * b.rv->m[b_index--] + carry + r.rv->m[r_index];
          carry = temp / m_base_long; temp -= m_base_long * carry;
          r.rv->m[r_index--] = temp;}
     r.rv->m[r_index] = carry; 
     if (b_start > b_max) {
          b_start = b_max; r_start--;}} 
          
// Formation of r mantissa units now complete; next is computation of r rnge.
if (r_len < leng) r_rnge_lo += r.rv->m[r_len];
temp = r_rnge_lo / m_base_long; temp1 = r_rnge_lo - temp * m_base_long;
r_rnge_hi += temp; if (temp1 > 0) r_rnge_hi++;
r_index = r_len - 1;
while (r_rnge_hi >= m_base_long) {
     r_len--;
     if (r_index >= 0) r_rnge_hi += r.rv->m[r_index--];
     temp = r_rnge_hi / m_base; temp1 = r_rnge_hi - temp * m_base;
     if (temp1 > 0) temp++; r_rnge_hi = temp;}
          
// The r rnge calculation above or earlier may have set r_len to zero. 
while (r_len <= 0) {r_len++; r.rv->exp++; r.rv->m[0] = 0;}
     
// If the leading r mantissa unit is zero, and r_len > 1, then all the units 
// have to be moved.
if (r.rv->m[0] == 0 && r_len > 1) {
     for (t = 1; t < r_len; t++) r.rv->m[t-1] = r.rv->m[t];
     r_len--; r.rv->exp--;}
          
// Discard terminal r mantissa units that are zero if r rnge is zero also.
if (r_rnge_hi == 0) {
     while (r.rv->m[r_len-1] == 0) --r_len;}
          
// r formation can now be completed.
r.rv->len = r_len; r.rv->rnge = r_rnge_hi;
if (r.rv->exp > m_max) {
     error("exp overflow on multiply"); return zero;}
if (r.rv->exp < m_min) {
     error("exp underflow on multiply"); return zero;}
return r;
}

rvar operator/ (rvar& a, rvar& b)
{
// a will be copied to c; subtractions of b from a take place in c.
// The divide proceeds by subtracting multiples of b from c to obtain
// as close to a zero result for c as possible.  The multiple count is held
// in r, and the subtractions are ranged subtractions.  After this process 
// has gone to completion, any c remainder is added to the c rnge that 
// has been accumulating, and then this rnge is divided by the minimum
// absolute value of b to obtain the r rnge.

// The successive r mantissa units are computed by dividing c_trial by b_trial
// to form r_trial. Here b_trial is fixed and formed from the first two
// mantissa units of b, shifted right by dividing by a high enough power of 10
// so that four significant digits remain. c_trial is formed repeatedly 
// from the leading two mantissa units of c, shifted appropriately to match
// b_trial. The quotient r_trial may be too large, so after this multiple
// of b is subtracted from c, if a final carry occurs, then b is added back
// as many times as needed to correct this.

long b_trial, rnge_div;
if (b.rv->len > 2) {
     b_trial = b.rv->m[0] * m_base_long + b.rv->m[1];
     rnge_div = b_trial - 1;}
else if (b.rv->len == 2) {
     b_trial = b.rv->m[0] * m_base_long + b.rv->m[1];
     rnge_div = b_trial - b.rv->rnge;}
else if (b.rv->len == 1 && b.rv->m[0] > b.rv->rnge) {
     b_trial = b.rv->m[0] * m_base_long;
     rnge_div = b_trial - m_base_long * b.rv->rnge;}
else if (b.rv->len == 0) {
     error("division by zero"); return zero;}
else {
     // b len = 1 and b interval contains zero point
     error("division by zero approximation");
     return zero;}
if (a.rv->len == 0) return zero;
b_trial /= 10;
long shifter = m_base_div10;
while (b_trial > m_max) {
     b_trial /= 10; shifter /= 10;}

long c_rnge_hi, c_rnge_lo;
int i, j, t, c_len = rvar::precision;
if (a.rv->rnge > 0 && a.rv->len < c_len) c_len = a.rv->len;
if (b.rv->rnge > 0 && b.rv->len < c_len) c_len = b.rv->len;
i = a.rv->len - c_len;
if (i == 0) {
     c_rnge_hi = a.rv->rnge; c_rnge_lo = 0;}
else if (i == 1) {
     c_rnge_hi = 0; c_rnge_lo = a.rv->rnge;}
else if (i > 1) {
     c_rnge_hi = 0; c_rnge_lo = 2;}
else {
     c_rnge_hi = 0; c_rnge_lo = 0;}

int c_max = c_len, c_max_minus1 = c_max - 1, c_data_end = a.rv->len - 1;
if (c_data_end > c_max) c_data_end = c_max;
rvar c(++c_len, '\0'); rvar r(c_len, a.rv->sgn ^ b.rv->sgn);
r.rv->exp = a.rv->exp - b.rv->exp + 1;
for (i = 0; i <= c_data_end; i++) c.rv->m[i] = a.rv->m[i];

// The division loop starts here. Termination occurs when either the leading
// c mantissa unit being examined goes past c_max (last c unit), or goes past
// c_data_end and result range is zero. The second case permits
// a division of one exact number by another to end whenever c becomes zero.

int r_index = 0, c_index = 0, c_hi = 0, carry;
long temp, temp1, r_trial, c_trial = c.rv->m[0] * shifter;
int b_max = b.rv->len - 1, b_start = b_max, c_start = c_index + b_start;
long b_rnge;
if (c_start > c_max) {
     i = c_start - c_max; c_start -= i; b_start -= i; b_max -= i;
     b_rnge = 2;}
else b_rnge = b.rv->rnge;
do {
     r_trial = c_trial / b_trial;
     if (r_trial) {
          while (c_data_end < c_start) c.rv->m[++c_data_end] = 0;
          j = c_start; carry = 0;
          for (i = b_start; i >= 0; i--) {
               temp = b.rv->m[i] * r_trial + carry;
               carry = temp / m_base_long;
               temp =  c.rv->m[j] + carry * m_base_long - temp;
               if (temp < 0) {
                    c.rv->m[j--] = m_base_long + temp; carry++;}
               else c.rv->m[j--] = temp;}
          c_hi -= carry;
          while (c_hi < 0) {
               r_trial--; j = c_start; carry = 0;
               for (i = b_start; i >= 0; i--) {
                    temp = c.rv->m[j] + b.rv->m[i] + carry;
                    if (temp >= m_base_long) {
                         c.rv->m[j--] = temp - m_base_long; carry = 1;}
                    else {
                         c.rv->m[j--] = temp; carry = 0;}}
               c_hi += carry;}
          if (c_start >= c_max_minus1) {
               if (c_start == c_max) {
                    if (b_start-- == b_max) c_rnge_lo += b_rnge * r_trial;
                    else c_rnge_lo += r_trial + r_trial;}
               else {
                    c_rnge_hi += b_rnge * r_trial; c_start++;}}
          else c_start++;}
     else {
          if (c_start == c_max) b_start--;
          else c_start++;}
     if (r_trial > m_max) {
          r_trial -= m_base; j = r_index - 1;
          while (++r.rv->m[j] == m_base) r.rv->m[j--] = 0;}
     r.rv->m[r_index++] = r_trial;
     c_hi = c.rv->m[c_index++];
     c_trial = c_hi * m_base_long;
     if (c_index <= c_data_end) c_trial += c.rv->m[c_index];
     else if (c_data_end == c_max) break;
     else if (c_hi == 0 && c_rnge_hi == 0 && c_rnge_lo == 0) break;
     else c.rv->m[++c_data_end] = 0;
     c_trial *= shifter;} 
while (1);

// Formation of r mantissa units complete; now calculate r rnge.
// First get c_rnge_hi and c_rnge_lo both less than m_base.

int r_discard = 0;
c_rnge_lo += c.rv->m[c_data_end];
if (c_rnge_lo >= m_base_long) {
     temp = c_rnge_lo / m_base_long;
     c_rnge_lo -= temp * m_base_long; c_rnge_hi += temp;}
while (c_rnge_hi >= m_base_long) {
     temp = c_rnge_hi / m_base_long;
     temp1 = c_rnge_hi - temp * m_base_long;
     if (c_rnge_lo) c_rnge_lo = ++temp1; else c_rnge_lo = temp1;
     c_rnge_hi = temp; r_discard++;}

// Next get r_rnge by dividing by rnge_div.

c_rnge_hi = c_rnge_hi * m_base_long + c_rnge_lo;
if (c_rnge_hi) {
     if (c_rnge_hi < rnge_div) {
          do {
               if (c_rnge_hi < m_base_long) {
                    c_rnge_hi *= m_base_long;
                    if (c_rnge_hi < rnge_div && r_index < c_len) {
                         r.rv->m[r_index++] = 0;}
                    else break;}
               for (i = m_digits; i > 0; --i) {
                    if (c_rnge_hi < m_base_squared_div10) c_rnge_hi *= 10;
                    else rnge_div /= 10;}
               break;}
          while (1);}
     else r_discard++;
     temp = c_rnge_hi / rnge_div;
     if (c_rnge_hi - temp * rnge_div > 0) ++temp;
     if (r_discard) ++temp;
     if (temp >= m_base_long) {
          temp1 = temp / m_base_long;
          if (temp - temp1 * m_base_long > 0) ++temp1;
          r_discard++; temp = ++temp1;}
     r.rv->rnge = temp;}

// If r rnge is zero, discard terminal r mantissa units that are zero.

else {
     r.rv->rnge = 0;
     while (r.rv->m[r_index-1] == 0) --r_index;}

     
r_index -= r_discard;
// r_index now equals r len.
// If the rnge computation has made r_index <= 0, there are no significant 
// r mantissa units left.
while (r_index <= 0) {r_index++; r.rv->m[0] = 0; r.rv->exp++;}
     
// If the leading r mantissa unit is zero, then all the units have to be moved.
if (r.rv->m[0] == 0 && r_index > 1) {
     for (i = 1; i < r_index; ++i) r.rv->m[i-1] = r.rv->m[i];
     r_index--; r.rv->exp--;}
 
r.rv->len = r_index;
if (r.rv->exp > m_max) {
     error("exp overflow on divide"); return zero;}
if (r.rv->exp < m_min) {
     error("exp underflow on divide"); return zero;}
return r;
}

rvar operator% (rvar& a, rvar& b)
{
int i, b_len = b.rv->len; rvar c;
if (b_len == 0) c = a;
else {
     c = rvar(1, '\0');
     c.rv->exp = b.rv->exp; c.rv->m[0] = 0;
     int temp = b.rv->m[0];
     if (b_len == 1) temp += b.rv->rnge;
     else {
          temp++;
          i = 1; b_len--;
          while (i < b_len && b.rv->m[i] == m_max) i++;
          if (i == b_len && (b.rv->m[i] + b.rv->rnge) > m_max) temp++;}
     if (temp > m_max) {
          if (temp == m_base) temp = 1; else temp = 2;
          if (++c.rv->exp > m_max) {
               error("exp overflow on rvar % operation"); return zero;}}
     c.rv->rnge = temp;
     c = a + c;}
return c;
}

rvar operator^ (rvar& a, rvar& b)
{     
if (a.rv->len == 0) { // exact zero a
     if (b.rv->len == 0) error("a ^ b with zero a and zero b");
     else if (b.rv->len == 1 && b.rv->m[0] <= b.rv->rnge) {
          error("a ^ b with zero a and zero approximation b");}
     else if (b.rv->sgn) error("a ^ b with zero a and negative b");
     return zero;}
if (b.rv->len == 0) { // exact zero b
     if (a.rv->len == 1 && a.rv->m[0] <= a.rv->rnge) {
          error("a ^ b with zero approximation a and zero b");}
     return one;}
if (is_int(b)) {
     int c_empty = TRUE, t; rvar a1, c, power = b;
     if (b.rv->sgn) a1 = one / a; else a1 = a; // possible divide error
     do {
          t = power.rv->len;
          if (power.rv->exp == t && power.rv->m[t-1] % 2) { 
               // above test determines whether power is an odd integer
               if (c_empty) {
                    c = a1; c_empty = FALSE;}
               else c = c * a1;}
          temporary_top_precision();
          power = trunc(div2(power));
          restore_precision();
          if (!is_zero(power)) a1 = a1 * a1; else break;}
     while (1);
     if (c == zero) { // need to correct c if b is even
          t = b.rv->len;
          if (!(b.rv->exp == t && b.rv->m[t-1] % 2)) c = abs(c);} 
     return c;}

// If this point is reached, a is not an exact zero and b is not an integer
rvar temp;
if (a.rv->len > 1 || a.rv->m[0] > a.rv->rnge) { // a interval excludes 0 point
     if (a.rv->sgn) {
          error("a ^ b with negative a and non-integer b"); return zero;}
     temp = exp(b * log(a)); return temp;}

// If this point is reached, a len is 1 and a interval contains zero point
// and other numbers, and b is not an integer.
if (b < zero) {
     error("a ^ b with zero approximation a and negative b");
     return zero;}
else if (b > zero) {
     if (!(a.rv->sgn == '\0' && a.rv->m[0] == a.rv->rnge)) {
          // a interval contains negative numbers and zero
          if (rvar::strict_ambiguity_treatment) {
               error("a ^ b with zero approximation a and b not an integer");
               return zero;}}
     temp = abs(a); temp = mid(temp) + range(temp);
     temp = zero % exp(log(temp) * b);
     return temp;}
else {
     error("a ^ b with zero approximation a and zero approximation b");
     return zero;}
}               

rvar add_sub(rvar& a1, rvar& b1, int add_call) 
{
// Here sgn of b is taken equal to sgn of a, so initial result sgn is sgn of a.

// a len and b len are always positive when add_sub is called.

// When we take into account the exp's of operand a and operand b, their 
// mantissa units may line up in various forms:
//        A A . . .  A A A . . .  A A A . . .  A
//        <- count1 -> B B . . .  B <- count3 ->  If b.rnge is positive then
//                     <- count2 ->               count3 part of a is ignored.
//
//        A A . . .  A A A . . .  A
//        <- count1 -> B B . . .  B B B . . .  B
//                     <- count2 -> <- count3 ->  If a.rnge is positive then
//                                                count3 part of b is ignored.
//   
//        A A . . .  A  
//        <- count1 -> <- count2 -> B B . . .  B  If a.rnge is positive then
//                                  <- count3 ->  count3 part of b is ignored.
//
//        A A . . .  A A A . . .  A
//        <- count1 -> B B . . .  B
//                     <- count2 ->
//

rvar a = a1, b = b1;
char r_sgn = a.rv->sgn; int r_rnge, r_len, r_exp;
int count2_uses_zeros = FALSE, count3_uses_a = TRUE; // usual case
int count1, count2, count3, carry, temp, i, a_index, b_index, r_index;
int exp_dif = a.rv->exp - b.rv->exp;
if (exp_dif < 0) { 
     swap(a, b); exp_dif = -exp_dif;
     if (!add_call) r_sgn = r_sgn ^ '-';} // change sign if SUB
r_exp = a.rv->exp;
a_index = a.rv->len - 1; b_index = b.rv->len - 1;
// a_index and b_index set for least sig. mantissa units
if (exp_dif >= a.rv->len) {
     count1 = a.rv->len;
     if (a.rv->rnge) {
          r_rnge = a.rv->rnge + 1; if (b.rv->rnge) r_rnge++;
          count2 = 0; count3 = 0;
          if (r_rnge > m_max) {
               r_rnge = 2; --count1; --a_index;}}
     else {
          r_rnge = b.rv->rnge;
          count2 = exp_dif - count1; count2_uses_zeros = TRUE;
          count3 = b.rv->len; count3_uses_a = FALSE;}}
else {
     count1 = exp_dif; temp = a.rv->len - exp_dif;
     if (temp > b.rv->len) {
          count2 = b.rv->len;
          if (b.rv->rnge) {
               count3 = 0; a_index -= (temp - count2);
               r_rnge = b.rv->rnge + 1;
               if (a.rv->rnge) r_rnge++;
               if (r_rnge > m_max) {
                    r_rnge = 3;
                    --count2; --a_index; --b_index;}}
          else {
               count3 = temp - count2; r_rnge = a.rv->rnge;}}
     else if (temp == b.rv->len) {
          count2 = temp; count3 = 0;
          r_rnge = a.rv->rnge + b.rv->rnge;
          if (r_rnge > m_max) {
               --count2; --a_index; --b_index; r_rnge = 4;}}
     else {
          count2 = temp; count3 = b.rv->len - temp;
          if (a.rv->rnge) {
               b_index -= count3; count3 = 0;
               r_rnge = a.rv->rnge + 1; if (b.rv->rnge) r_rnge++;
               if (r_rnge > m_max) {
                    r_rnge = 3;
                    --count2; --a_index; --b_index;}}
          else {
               count3_uses_a = FALSE; r_rnge = b.rv->rnge;}}}
temp = count1 + count2 + count3; r_len = temp; 
temp -= rvar::precision; 
if (temp > 0 && (r_rnge > 0 || rvar::exact_arithmetic == FALSE) ) {
     // temp now holds r len excess over precision
     r_len -= temp; r_rnge = 3;
     if (temp <= count3) {
          if (count3_uses_a) a_index -= temp; else b_index -= temp;
          count3 -= temp;}
     else {
          if (count3_uses_a) a_index -= count3; else b_index -= count3;
          temp -= count3; count3 = 0;
          if (temp <= count2) {
               if (!count2_uses_zeros) {a_index -= temp; b_index -= temp;}
               count2 -= temp;}
          else {
               if (!count2_uses_zeros) {
                    a_index -= count2; b_index -= count2;}
               temp -= count2; count2 = 0;
               count1 -= temp; a_index -= temp;}}}
                    
carry = 0; r_index = r_len - 1;
rvar r(r_len + 1, r_sgn); // length 1 extra in case of add carry

// r_len is 0 if a and b have 1 mantissa unit with same exp and high rnge.
if (r_len == 0) {
     r.rv->len = 1; r.rv->m[0] = 0; r.rv->rnge = r_rnge;
     r.rv->exp = ++r_exp;
     if (r_exp > m_max) {
          error("exponent overflow on addition or subtraction");
          return zero;}
     else return r;}

if (add_call) { // addition starts here
     if (count3_uses_a) {
          while (count3--) r.rv->m[r_index--] = a.rv->m[a_index--];}
     else {
          while (count3--) r.rv->m[r_index--] = b.rv->m[b_index--];}
     if (count2_uses_zeros) {
          while (count2--) r.rv->m[r_index--] = 0;}
     else {
          while (count2--) {
               temp = a.rv->m[a_index--] + b.rv->m[b_index--] + carry;
               if (temp > m_max) {
                    carry = 1; r.rv->m[r_index--] = temp - m_base;}
               else {
                    carry = 0; r.rv->m[r_index--] = temp;}}}
     if (carry) {
          while (count1--) {
               temp  = a.rv->m[a_index--];
               if (temp == m_max) r.rv->m[r_index--] = 0;
               else {
                    carry = 0; r.rv->m[r_index--] = ++temp; break;}}}
     while (count1-- > 0) {
          r.rv->m[r_index--] = a.rv->m[a_index--];}
               
     // all r mantissa units now formed
     if (carry) { // r mantissa units adjusted for final carry
          ++r_exp; ++r_len;
          i = 0;
          while (i < r_len) { 
               temp = r.rv->m[i]; r.rv->m[i++] = carry; carry = temp;}}
          
     // When r rnge is 0, terminal zero r mantissa units are discarded.
     if (r_rnge == 0) {
          while (r_len > 0 && r.rv->m[r_len - 1] == 0)  r_len--;}
     if (r_exp > m_max) {
          error("exponent overflow on addition or subtraction");
          return zero;}
               
// Finish forming r.
     r.rv->len = r_len; r.rv->exp = r_exp; 
     r.rv->rnge = r_rnge;
     
     return r;}    // end of addition

// beginning of subtraction

if (count3_uses_a) {
     while (count3--) r.rv->m[r_index--] = a.rv->m[a_index--];}
else {
     while (count3--) {
          if (carry) {
               r.rv->m[r_index--] = m_max - b.rv->m[b_index--];}
          else {
               temp = m_base - b.rv->m[b_index--];
               if (temp == m_base) r.rv->m[r_index--] = 0;
               else {
                    r.rv->m[r_index--] = temp; carry = 1;}}}}
if (count2_uses_zeros) {
     if (carry) temp = m_max; else temp = 0;
     while (count2--) r.rv->m[r_index--] = temp;}
else {
     while (count2--) {
          temp = a.rv->m[a_index--] - b.rv->m[b_index--] - carry;
          if (temp < 0) {
               carry = 1; r.rv->m[r_index--] = m_base + temp;}
          else {
               carry = 0; r.rv->m[r_index--] = temp;}}}
if (carry) {
     while (count1--) {
          temp = a.rv->m[a_index--];
          if (temp == 0) r.rv->m[r_index--] = m_max;
          else {
               r.rv->m[r_index--] = --temp; carry = 0; break;}}}
while (count1-- > 0) {
     r.rv->m[r_index--] = a.rv->m[a_index--];}
          
// all r mantissa units now formed               

if (carry) {// correction of r mantissa units required if final carry = 1
     carry = 0; r.rv->sgn = r.rv->sgn ^ '-';  // change sign of r
     for (i = r_len -1; i >= 0; i--) {
          if (carry) r.rv->m[i] = m_max - r.rv->m[i];
          else {
               temp = m_base - r.rv->m[i];
               if (temp < m_base) {
                    carry = 1; r.rv->m[i] = temp;}}}}
                         
// find number of leading zero mantissa units of r and remove these
temp = r_len - 1;
for (i = 0; i < temp && r.rv->m[i] == 0; ++i) {;}
if (i) {
     r_exp -= i; r_len -= i;
     for (int j = 0; j < r_len; ++j) r.rv->m[j] = r.rv->m[j+i];}
          
// When r rnge is zero, terminal zero mantissa units are discarded. r may
// become a pure zero.
if (r_rnge == 0) {
     while (r_len > 0 && r.rv->m[r_len-1] == 0) --r_len;}
          
// Finish forming r.
r.rv->len = r_len; r.rv->exp = r_exp; r.rv->rnge = r_rnge;

if (r_exp < m_min) {
     error("exp underflow on addition or subtraction"); return zero;}

return r;    // end of subtraction               
}     // end of add_sub function


