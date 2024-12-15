#include "range.h"
#include "vector.h"
#include "interval.h"
#include "series1.h"

series :: series (int n)
{
// When n == 0, a series is constructed with only the constant term of zero
int term_count;
if (n) term_count = (max_x_deg + 1) * (num_u_variables + 1);
else term_count = 1;
ser* v = (ser*) get_memory(sizeof(ser) + (term_count - 1) * sizeof(interval));
v->count = 1; v->num_terms = term_count;
interval* p = v->term; rvar::rva** q;
for (int i=0; i<term_count; i++) {
     q = (rvar::rva**) &p[i].mid; *q = &zero_init;
     q = (rvar::rva**) &p[i].wid; *q = &zero_init;}
zero_init.count += term_count + term_count;
se = v;
}

series :: series ()
{
se = zero_series.se;
se->count++;
}

series :: series (series& a)
{
se = a.se;
se->count++;
}

series :: ~series()
{
if (--se->count == 0) {
     interval* p = se->term;
     int i, n = se->num_terms;
     for (i=0; i<n; i++) (p+i)->interval::~interval();
     return_memory(se);}
}

interval& series::operator() (int i)
{
return se->term[i];
}

interval& series::operator() (int i, int j) 
{
return se->term[i + x_len * (j+1)];
}

series& series::operator= (series& a)
{
a.se->count++;
if (--se->count == 0) {
     interval* p = se->term;
     int i, n = se->num_terms;
     for (i=0; i<n; i++) (p+i)->interval::~interval();
     return_memory(se);}
se = a.se; return *this;
}

series new_series(int n, rvar& r)
{
// When n == 0, a series is returned with only the constant term of r
series a(n);
a(0).mid = r;
return a;
}

enum {one_minus_square_ = 'U', one_plus_square_ = 'V', times_const_ = 'W',
     div_const_ = 'X', copy_ = 'Y', power_coefficients_ = 'Z',
     plus_const_ = '1', minus_const_ = '2', const_minus_ = '3',
     const_div_ = '4', transfer_ = '5'};

void prepare_series(
svector& u_function_entry, series* u_input, series* u_output, series* x_input)
{
// Constructs the u and x series, and two lists of series operations.
// One list, op, with series variables v1, v2, v3, when executed by
// series_computation(int t) yields the x^t term of each u series.
// This list, when executed by series_computation1(int t) yields the
// x^t, u const, and x^t, u partial, terms of each u series.
// The other list, op1, with series variables w1, w2, w3, when executed
// by compute_series_constants() before beginning series computation and
// after every precision increase, computes all the constants needed for
// series computation.
int i, n = u_function_entry.dim();
series::num_u_variables = n;
x_input[0] = new_series(1); x_input[0](1).mid = one; 
for (i=0; i<n; i++) {
     u_input[i] = new_series(1);
     u_input[i](0,i).mid = one;} 
for (i=0; i<=max_x_deg; i++) integ[i] = rvar(i);

char c, *p, *q, *q1;
int k, v_last = -1, done, const_v_last, const_v_nextlast, empty;
char type[evaluation_stack_size];
series v_stack[evaluation_stack_size]; rvar r, r1;
series s1;
op_count = -1; op1_count = -1;

for (i=0; i<n; i++) {
     p = u_function_entry(i).tokens() - 3; q = !u_function_entry(i);
     do {
          p += 3;
          if (v_last >= 0 && type[v_last] == const_) {
               const_v_last = TRUE; r = v_stack[v_last](0).mid;}
          else const_v_last = FALSE;
          if (v_last >= 1 && type[v_last-1] == const_) {
               const_v_nextlast = TRUE; r1 = v_stack[v_last-1](0).mid;}
          else const_v_nextlast = FALSE;
          c = *p; done = FALSE;
          if (const_v_last & const_v_nextlast) {
               switch (c) {
                    case '+': case '-': case '*': case '/': case '^':
                    case power_even_p_odd_q: case power_odd_p_odd_q:
                    op1[++op1_count] = c;
                    w1[op1_count] = v_stack[v_last-1];
                    w2[op1_count] = v_stack[v_last];
                    w3[op1_count] = new_series(0);
                    v_stack[--v_last] = w3[op1_count];
                    done = TRUE;}}
          if (!done & const_v_last) {
               switch (c) {
                    case abs_:  case acos_: case asin_: case atan_:
                    case cos_:  case cosh_: case exp_:  case log_:
                    case sin_:  case sinh_: case sqrt_: case tan_:
                    case tanh_: case unary_minus_:
                    op1[++op1_count] = c;
                    w1[op1_count] = v_stack[v_last];
                    w2[op1_count] = new_series(0);
                    v_stack[v_last] = w2[op1_count];
                    done = TRUE;}}
          if (done) continue;
          switch (c) {
               case end_:
               if (v_last != 0) error("series preparation 1");
               if (const_v_last) {
                    u_output[i] = new_series(1);
                    op1[++op1_count] = transfer_;
                    w1[op1_count] = v_stack[v_last];
                    w2[op1_count] = u_output[i];}
               else u_output[i] = v_stack[v_last];
               v_last--; op[++op_count] = end_;
               break;
               
               case const_: q1 = q + (*(p+1) << 7) + *(p+2);
               r = rvar(q1);
               if (!is_zero(range(r))) error("series preparation 2");
               v_stack[++v_last] = new_series(0, r);
               type[v_last] = const_; break;
               
               case pi_const_: r = pi;
               v_stack[++v_last] = new_series(0, r);
               type[v_last] = const_; break;
               
               case var_: k = *(p+2); 
               if (k < n) s1 = u_input[k];
               else       s1 = x_input[0];
               v_stack[++v_last] = s1;
               type[v_last] = var_; break;
               
               case acos_:
               op[++op_count] = unary_minus_;
               v1[op_count] = v_stack[v_last];
               v2[op_count] = new_series(1);
               v_stack[v_last] = v2[op_count]; // asin below completes acos
               
               case asin_:
               op[++op_count] = one_minus_square_;
               v1[op_count] = v_stack[v_last];
               v2[op_count] = new_series(1);
               op[++op_count] = sqrt_;
               v1[op_count] = v2[op_count-1];
               v2[op_count] = new_series(1);
               op1[++op1_count] = power_coefficients_;
               w1[op1_count] = new_series(0, half);
               w2[op1_count] = v2[op_count];
               v3[op_count] = new_series(1);
               op[++op_count] = c; // c = asin_ or acos_
               v1[op_count] = v_stack[v_last];
               v2[op_count] = v3[op_count-1];
               v3[op_count] = new_series(1);
               v_stack[v_last] = v3[op_count]; break;
               
               case atan_:
               op[++op_count] = one_plus_square_;
               v1[op_count] = v_stack[v_last];
               v2[op_count] = new_series(1);
               op[++op_count] = atan_;
               v1[op_count] = v_stack[v_last];
               v2[op_count] = v2[op_count-1];
               v3[op_count] = new_series(1);
               v_stack[v_last] = v3[op_count]; break;
               
               case cosh_ : case sinh_ : case tanh_ :
               case cos_ :  case sin_ :  case tan_ : 
               if (c == sin_ || c == cos_ || c == tan_) op[++op_count] = cos_;
               else op[++op_count] = cosh_;
               v1[op_count] = v_stack[v_last];
               v2[op_count] = new_series(1);
               v3[op_count] = new_series(1);
               if (c == sin_ || c == sinh_) {
                    v_stack[v_last] = v2[op_count]; break;}
               if (c == cos_ || c == cosh_) {
                    v_stack[v_last] = v3[op_count]; break;}
               op[++op_count] = '/';
               v1[op_count] = v2[op_count-1];
               v2[op_count] = v3[op_count-1];
               v3[op_count] = new_series(1);
               v_stack[v_last] = v3[op_count]; break;
               
               case unary_minus_: case abs_ : case log_ : case exp_ :
               op[++op_count] = c;
               v1[op_count] = v_stack[v_last];
               v2[op_count] = new_series(1);
               v_stack[v_last] = v2[op_count]; break;
               
               case sqrt_:
               op[++op_count] = sqrt_;
               v1[op_count] = v_stack[v_last];
               v2[op_count] = new_series(1);
               op1[++op1_count] = power_coefficients_;
               w1[op1_count] = new_series(0, half);
               w2[op1_count] = v2[op_count];
               v3[op_count] = new_series(1);
               v_stack[v_last] = v3[op_count]; break;
               
               case '+':
               if (const_v_last | const_v_nextlast) c = plus_const_;
               op[++op_count] = c;
               if (const_v_last) {
                    v2[op_count] = v_stack[v_last--];
                    v1[op_count] = v_stack[v_last];}
               else {
                    v1[op_count] = v_stack[v_last--];
                    v2[op_count] = v_stack[v_last];}
               v3[op_count] = new_series(1);
               v_stack[v_last] = v3[op_count];
               type[v_last] = var_; break;
               
               case '-':
               if (const_v_last) c = minus_const_;
               if (const_v_nextlast) c = const_minus_;
               op[++op_count] = c;
               v2[op_count] = v_stack[v_last--];
               v1[op_count] = v_stack[v_last];
               v3[op_count] = new_series(1);
               v_stack[v_last] = v3[op_count];
               type[v_last] = var_; break;
               
               case '*':
               if (const_v_last | const_v_nextlast) c = times_const_;
               op[++op_count] = c;
               if (const_v_last) {
                    v1[op_count] = v_stack[v_last--];
                    v2[op_count] = v_stack[v_last];}
               else {
                    v2[op_count] = v_stack[v_last--];
                    v1[op_count] = v_stack[v_last];}
               v3[op_count] = new_series(1);
               v_stack[v_last] = v3[op_count];
               type[v_last] = var_; break;
               
               case '/':
               if (const_v_last) c = div_const_;
               if (const_v_nextlast) c = const_div_;
               op[++op_count] = c;
               v2[op_count] = v_stack[v_last--];
               v1[op_count] = v_stack[v_last];
               v3[op_count] = new_series(1);
               v_stack[v_last] = v3[op_count];
               type[v_last] = var_; break;
               
               case '^':
               if (!const_v_last) {
                    if (!const_v_nextlast) {
                         op[++op_count] = log_;
                         v1[op_count] = v_stack[v_last-1];
                         v2[op_count] = new_series(1);
                         op[++op_count] = '*';
                         v1[op_count] = v2[op_count-1];}
                    else {
                         op1[++op1_count] = log_;
                         w1[op1_count] = v_stack[v_last-1];
                         w2[op1_count] = new_series(1);
                         v_stack[v_last-1] = w2[op1_count];
                         op[++op_count] = times_const_;
                         v1[op_count] = w2[op1_count];}
                    v2[op_count] = v_stack[v_last];
                    v3[op_count] = new_series(1);
                    op[++op_count] = exp_;
                    v1[op_count] = v3[op_count-1];
                    v2[op_count] = new_series(1);
                    v_stack[--v_last] = v2[op_count];
                    type[v_last] = var_; break;}
               // ^ combines with cases below if not taken care of already
               // these cases all have constant r equal to v_stack[v_last] 
               case power_odd_p_odd_q: case power_even_p_odd_q:
               if (is_int(r) && (r > zero)) {
                    empty = TRUE; v_stack[v_last] = new_series(1);
                    while (1) { 
                         r1 = r;
                         temporary_top_precision();
                         r = trunc(div2(r1)); r1 -= r + r;
                         restore_precision();
                         if (r1 > zero) {
                              if (empty) {
                                   empty = FALSE;
                                   op[++op_count] = copy_;
                                   v1[op_count] = v_stack[v_last-1];
                                   v2[op_count] = v_stack[v_last];}
                              else {
                                   op[++op_count] = '*';
                                   v1[op_count] = v_stack[v_last-1];
                                   v2[op_count] = v_stack[v_last];
                                   v3[op_count] = new_series(1);
                                   v_stack[v_last] = v3[op_count];}}
                         if (r > zero) {
                              op[++op_count] = '*';
                              v1[op_count] = v_stack[v_last-1];
                              v2[op_count] = v1[op_count];
                              v3[op_count] = new_series(1);
                              v_stack[v_last-1] = v3[op_count];}
                         else break;}
                    v_stack[v_last-1] = v_stack[v_last]; v_last--;}
               else {
                    op1[++op1_count] = power_coefficients_;
                    w1[op1_count] = v_stack[v_last--];
                    w2[op1_count] = new_series(1);
                    op[++op_count] = c;
                    v1[op_count] = v_stack[v_last];
                    v2[op_count] = w2[op1_count];
                    v3[op_count] = new_series(1);
                    v_stack[v_last] = v3[op_count];}
               type[v_last] = var_; break; 
               
               default: error("series preparation 3");}}
     while (c != end_);}
if (op_count >= operation_max || op1_count >= operation_max) {
     error("too many series operations");}
}
void compute_series_constants()
{
int i, j; char c; rvar r, r1; series s1, s2, s3;
for (i=0; i<=op1_count; i++) {
     c = op1[i]; s2 = w2[i]; s3 = w3[i];
     r = w1[i](0).mid; r1 = s2(0).mid;
     switch (c) {
          case abs_:  s2(0).mid = abs(r);  break;
          case acos_: s2(0).mid = acos(r); break;
          case asin_: s2(0).mid = asin(r); break;
          case atan_: s2(0).mid = atan(r); break;
          case cos_:  s2(0).mid = cos(r);  break;
          case cosh_: s2(0).mid = cosh(r); break;
          case exp_:  s2(0).mid = exp(r);  break;
          case log_:  s2(0).mid = log(r);  break;
          case sin_:  s2(0).mid = sin(r);  break;
          case sinh_: s2(0).mid = sinh(r); break;
          case sqrt_: s2(0).mid = sqrt(r); break;
          case tan_:  s2(0).mid = tan(r);  break;
          case tanh_: s2(0).mid = tanh(r); break;
          case unary_minus_: s2(0).mid = -r; break;
          case '+': s3(0).mid = r + r1; break;
          case '-': s3(0).mid = r - r1; break;
          case '*': s3(0).mid = r * r1; break;
          case '/': s3(0).mid = r / r1; break;
          case '^': s3(0).mid = r ^ r1; break;
          case power_even_p_odd_q: s3(0).mid = expon_e(r, r1); break;
          case power_odd_p_odd_q:  s3(0).mid = expon_o(r, r1); break;
          case transfer_: s2(0).mid = r; break;
          case power_coefficients_: s2(0).mid = r; r += one;
          for (j=1; j<=max_x_deg; j++) s2(j).mid = r * integ[j];
          break;
          default: error("compute_series_constants");}}
}

void series_computation(int t)
{
// Computes u series coefficients of x^t terms.
// For this run, x interval is h, u intervals are `sufficiently large'
// as found by determine_x_step_h() function in main program
int i, k; char c; interval temp; rvar r; series s1, s2, s3;
series::error = FALSE; series::function_count = 0;
for (i=0; i<op_count; i++) {
     c = op[i];
     switch (c) {
          case end_: series::function_count++; break;

          case unary_minus_: v2[i](t) = - v1[i](t); break;

          case '+': v3[i](t) = v1[i](t) + v2[i](t); break;

          case plus_const_:
          if (t == 0) v3[i](t) = v1[i](t) + v2[i](t);
          else v3[i](t) = v1[i](t);
          break;

          case '-': v3[i](t) = v1[i](t) - v2[i](t); break;

          case minus_const_:
          if (t == 0) v3[i](t) = v1[i](t) - v2[i](t);
          else v3[i](t) = v1[i](t);
          break;

          case const_minus_:
          if (t == 0) v3[i](t) = v1[i](t) - v2[i](t);
          else v3[i](t) = -v2[i](t);
          break;

          case '*': s1 = v1[i]; s2 = v2[i];
          temp = s1(0) * s2(t);
          for (k=1; k<=t; k++) {
               temp = temp + s1(k) * s2(t-k);}
          v3[i](t) = temp; break;
               
          case times_const_: v3[i](t) = v1[i](0).mid * v2[i](t); break;

          case div_const_: v3[i](t) = v1[i](t) / v2[i](0).mid; break;
          
          case copy_: v2[i](t) = v1[i](t); break;
          
          case one_plus_square_: s1 = v1[i];
          temp = s1(0) * s1(t);
          for (k=1; k<=t; k++) {
               temp = temp + s1(k) * s1(t-k);}
          if (t == 0) {
               temp.mid += one;
               if (temp.has_zero()) {
                    series::error = TRUE; return;}}
          v2[i](t) = temp; break;
               
          case one_minus_square_: s1 = v1[i];
          temp = -s1(0) * s1(t);
          for (k=1; k<=t; k++) {
               temp = temp - s1(k) * s1(t-k);}
          if (t == 0) {
               temp.mid += one;
               if (temp.has_zero()) {
                    series::error = TRUE; return;}}
          v2[i](t) = temp; break;
               
          case '/': s2 = v2[i];
          if (t == 0) {
               if (s2(0).has_zero()) {
                    series::error = TRUE; return;}
               v3[i](0) = v1[i](0) / s2(0);}
          else {
               temp = v1[i](t); s3 = v3[i]; 
               for (k=0; k<t; k++) temp = temp - s3(k) * s2(t-k);
               s3(t) = temp / s2(0);}
          break;
               
          case const_div_: s2 = v2[i];
          if (t == 0) {
               if (s2(0).has_zero()) {
                    series::error = TRUE; return;}
               v3[i](0) = v1[i](0) / s2(0);}
          else {
               temp = I_zero; s3 = v3[i]; 
               for (k=0; k<t; k++) temp = temp - s3(k) * s2(t-k);
               s3(t) = temp / s2(0);}
          break;
               
          case sqrt_: case '^':
          case power_even_p_odd_q: case power_odd_p_odd_q:
          s1 = v1[i];
          if (t == 0) {
               if (c == sqrt_ || c == '^') {
                    if (s1(0).not_positive()) {
                         series::error = TRUE; return;}
                    if (c == sqrt_) v3[i](0) = sqrt(s1(0));
                    else v3[i](0) = s1(0) ^ v2[i](0).mid;}
               else {
                    if (s1(0).has_zero()) {
                         series::error = TRUE; return;}
                    if (c == power_even_p_odd_q) {
                         v3[i](0) = s1(0) << v2[i](0).mid;}
                    else v3[i](0) = s1(0) >> v2[i](0).mid;}}
          else {
               temp = I_zero; r = integ[t]; s2 = v2[i]; s3 = v3[i];
               for (k=1; k<=t; k++) {
                    temp = temp + (s2(k).mid - r) * s3(t-k) * s1(k);}
               s3(t) = (temp / r) / s1(0);}
          break;
               
          case abs_: s1 = v1[i];
          if (s1(0).positive()) v2[i](t) = s1(t);
          else if (s1(0).negative()) v2[i](t) = - s1(t);
          else {
               series::error = TRUE; return;}
          break;
               
          case acos_: case asin_: case atan_:
          if (t == 0) {
               if (c == atan_) v3[i](0) = atan(v1[i](0));
               else if (c == asin_) v3[i](0) = asin(v1[i](0));
               else v3[i](0) = acos(-v1[i](0));}
          else {
               temp = I_zero; s2 = v2[i]; s3 = v3[i];
               for (k=1; k<t; k++) {
                    temp = temp + integ[k] * s3(k) * s2(t-k);}
               s3(t) = (v1[i](t) - temp / integ[t]) / s2(0);}
          break;
               
          case cos_: s1 = v1[i];
          if (t == 0)  {
               v2[i](0) = sin(s1(0));
               v3[i](0) = cos(s1(0));}
          else {
               s2 = v2[i]; s3 = v3[i];
               temp = s1(1) * s3(t-1);
               for (k=2; k<=t; k++) {
                    temp = temp + integ[k] * s1(k) * s3(t-k);}
               s2(t) = temp / integ[t];
               temp = s1(1) * s2(t-1);
               for (k=2; k<=t; k++) {
                    temp = temp + integ[k] * s1(k) * s2(t-k);}
               s3(t) = -temp / integ[t];}
          break;

          case cosh_: s1 = v1[i];
          if (t == 0)  {
               v2[i](0) = sinh(s1(0));
               v3[i](0) = cosh(s1(0));}
          else {
               s2 = v2[i]; s3 = v3[i];
               temp = s1(1) * s3(t-1);
               for (k=2; k<=t; k++) {
                    temp = temp + integ[k] * s1(k) * s3(t-k);}
               s2(t) = temp / integ[t];
               temp = s1(1) * s2(t-1);
               for (k=2; k<=t; k++) {
                    temp = temp + integ[k] * s1(k) * s2(t-k);}
               s3(t) = temp / integ[t];}
          break;
               
          case exp_:
          if (t == 0) v2[i](0) = exp(v1[i](0));
          else {
               s1 = v1[i]; s2 = v2[i];
               temp = s1(1) * s2(t-1);
               for (k=2; k<=t; k++) {
                    temp = temp + integ[k] * s1(k) * s2(t-k);}
               s2(t) = temp / integ[t];}
          break;

          case log_: s1 = v1[i];
          if (t == 0) {
               if (s1(0).not_positive()) {
                     series::error = TRUE; return;}
               v2[i](0) = log(s1(0));}
          else {
               s2 = v2[i];
               if (t > 1) temp = s2(1) * s1(t-1); else temp = I_zero;
               for (k=2; k<t; k++) {
                    temp = temp + integ[k] * s2(k) * s1(t-k);}
               s2(t) = (s1(t) - (temp / integ[t])) / s1(0);}
          break;

          default: error("series computation");}}
}

void series_computation1(int t)
{
// Computes u series coefficients of x^t term to obtain constant component
// and u partials. For this run, x interval width is set to 0
// and u intervals are set to global errors.
int i, j, J = series::num_u_variables, k, k1;
char c; interval temp; rvar r; series s1, s2, s3;
for (i=0; i<op_count; i++) {
     c = op[i]; s1 = v1[i]; s2 = v2[i]; s3 = v3[i];
     switch (c) {
          case end_: break;
               
          case unary_minus_:
          for (j=-1; j<J; j++) s2(t,j) = - s1(t,j); 
          break;
               
          case '+':
          for (j=-1; j<J; j++) s3(t,j) = s1(t,j) + s2(t,j);
          break;
               
          case '-':
          for (j=-1; j<J; j++) s3(t,j) = s1(t,j) - s2(t,j);
          break;
               
          case '*':
          temp = s1(0) * s2(t);
          for (k=1; k<=t; k++) {
               temp = temp + s1(k) * s2(t-k);}
          s3(t) = temp;
          for (j=0; j<J; j++) {
               temp = I_zero;
               for (k=0; k<=t; k++) {
                    temp = temp + s1(k,j) * s2(t-k) + s1(k) * s2(t-k,j);}
               s3(t,j) = temp;}
          break;
               
          case plus_const_:
          if (t == 0) {
               s3(0) = s1(0) + s2(0);
               for (j=0; j<J; j++) s3(t,j) = s1(t,j);}
          else {
               for (j=-1; j<J; j++) s3(t,j) = s1(t,j);}
          break;
               
          case minus_const_:
          if (t == 0) {
               s3(0) = s1(0) - s2(0);
               for (j=0; j<J; j++) s3(t,j) = s1(t,j);}
          else {
               for (j=-1; j<J; j++) s3(t,j) = s1(t,j);}
          break;
               
          case const_minus_:
          if (t == 0) {
               s3(0) = s1(0) - s2(0);
               for (j=0; j<J; j++) s3(t,j) = -s2(t,j);}
          else {
               for (j=-1; j<J; j++) s3(t,j) = -s2(t,j);}
          break;
               
          case times_const_:
          r = s1(0).mid;
          for (j=-1; j<J; j++) s3(t,j) = r * s2(t,j);
          break;
               
          case div_const_:
          r = s2(0).mid;
          for (j=-1; j<J; j++) s3(t,j) = s1(t,j) / r;
          break;
               
          case copy_:
          for (j=-1; j<J; j++) s2(t,j) = s1(t,j);
          break;

          case one_plus_square_:
          temp = s1(0) * s1(t);
          for (k=1; k<=t; k++) {
               temp = temp + s1(k) * s1(t-k);}
          if (t == 0) temp.mid += one;
          s2(t) = temp;
          for (j=0; j<J; j++) {
               temp = I_zero;
               for (k=0; k<=t; k++) {
                    temp = temp + s1(k,j) * s1(t-k) + s1(k) * s1(t-k,j);}
               s2(t,j) = temp;}
          break;
               
          case one_minus_square_:
          temp = -s1(0) * s1(t);
          for (k=1; k<=t; k++) {
               temp = temp - s1(k) * s1(t-k);}
          if (t == 0) temp.mid += one;
          s2(t) = temp;
          for (j=0; j<J; j++) {
               temp = I_zero;
               for (k=0; k<=t; k++) {
                    temp = temp - s1(k,j) * s1(t-k) - s1(k) * s1(t-k,j);}
               s2(t,j) = temp;}
          break;
               
          case '/':
          if (t == 0) {
               s3(0) = s1(0) / s2(0);
               for (j=0; j<J; j++) {
                    s3(0,j) = (s1(0,j) - s3(0) * s2(0,j)) / s2(0);}}
          else {
               temp = s1(t);
               for (k=0; k<t; k++) temp = temp - s3(k) * s2(t-k);
               s3(t) = temp / s2(0);
               for (j=0; j<J; j++) {
                    temp = s1(t,j); 
                    for (k=0; k<t; k++) {
                         temp = temp - s3(k,j) * s2(t-k) - s3(k) * s2(t-k,j);}
                    s3(t,j) = temp / s2(0);}}
          break;
               
          case const_div_:
          if (t == 0) {
               s3(0) = s1(0) / s2(0);
               for (j=0; j<J; j++) {
                    s3(0,j) = - (s3(0) * s2(0,j) / s2(0));}}
          else {
               temp = I_zero;
               for (k=0; k<t; k++) temp = temp - s3(k) * s2(t-k);
               s3(t) = temp / s2(0);
               for (j=0; j<J; j++) {
                    temp = I_zero; 
                    for (k=0; k<t; k++) {
                         temp = temp - s3(k,j) * s2(t-k) - s3(k) * s2(t-k,j);}
                    s3(t,j) = temp / s2(0);}}
          break;
               
          case sqrt_: case '^':
          case power_even_p_odd_q: case power_odd_p_odd_q:
          if (t == 0) {
               if (c == sqrt_) s3(0) = sqrt(s1(0));
               else if (c == '^') s3(0) = s1(0) ^ s2(0).mid;
               else if (c == power_even_p_odd_q) s3(0) = s1(0) << s2(0).mid;
               else s3(0) = s1(0) >> s2(0).mid;
               for (j=0; j<J; j++) {
                    s3(0,j) = (s2(1).mid - one) * s3(0) * s1(0,j) / s1(0);}}
          else {
               r = integ[t]; temp = I_zero; 
               for (k=1; k<=t; k++) {
                    temp = temp + (s2(k).mid - r) * s3(t-k) * s1(k);}
               s3(t) = (temp / r) / s1(0);
               r = integ[t+1];
               for (j=0; j<J; j++) {
                    temp = I_zero;
                    for (k=1; k<=t; k++) {
                         k1 = k - 1;
                         temp = temp + (s2(k).mid - r) * 
                         (s3(t-k1) * s1(k1,j) + s3(t-k,j) * s1(k));}
                    temp = temp + (s2(t+1).mid - r) * s3(0) * s1(t,j);
                    s3(t,j) = (temp / r) / s1(0);}}
          break;
               
          case abs_:
          if (s1(0).positive()) for (j=-1; j<J; j++) s2(t,j) =  s1(t,j);
          else                  for (j=-1; j<J; j++) s2(t,j) = -s1(t,j);
          break;
               
          case acos_: case asin_: case atan_:
          if (t == 0) {
               if (c == atan_)      s3(0) = atan( s1(0));
               else if (c == asin_) s3(0) = asin( s1(0));
               else                 s3(0) = acos(-s1(0));
               for (j=0; j<J; j++) s3(0,j) = s1(0,j) / s2(0);}
          else {
               if (t > 1) temp = s3(1) * s2(t-1); else temp = I_zero;
               for (k=2; k<t; k++) {
                    temp = temp + integ[k] * s3(k) * s2(t-k);}
               s3(t) = (s1(t) - temp / integ[t]) / s2(0);
               for (j=0; j<J; j++) {
                    temp = s3(1) * s2(t-1,j) + s3(0,j) * s2(t);
                    for (k=2; k<t; k++) {
                         k1 = k - 1;
                         temp = integ[k] * 
                         (s3(k) * s2(t-k,j) + s3(k1,j) * s2(t-k1));}
                    temp = temp + integ[t+1] * s3(t,j) * s2(0);
                    s3(t,j) = (s1(t,j) - temp / integ[t+1]) / s2(0);}}
          break;  
               
          case cos_:
          if (t == 0)  {
               s2(0) = sin(s1(0));
               s3(0) = cos(s1(0));
               for (j=0; j<J; j++) s2(0,j) =  s1(0,j) * s3(0);
               for (j=0; j<J; j++) s3(0,j) = -s1(0,j) * s2(0);}
          else {
               temp = s1(1) * s3(t-1);
               for (k=2; k<=t; k++) {
                    temp = temp + integ[k] * s1(k) * s3(t-k);}
               s2(t) = temp / integ[t];
               temp = s1(1) * s2(t-1);
               for (k=2; k<=t; k++) {
                    temp = temp + integ[k] * s1(k) * s2(t-k);}
               s3(t) = -temp / integ[t];
               for (j=0; j<J; j++) {
                    temp = s1(1) * s3(t-1,j) + s1(0,j) * s3(t);
                    for (k=2; k<=t; k++) {
                         k1 = k - 1;
                         temp = temp + integ[k] * 
                         (s1(k) * s3(t-k,j) + s1(k1,j) * s3(t-k1));}
                    temp = temp + integ[t+1] * s1(t,j) * s3(0);
                    s2(t,j) = temp / integ[t+1];
                    temp = s1(1) * s2(t-1,j) + s1(0,j) * s2(t);
                    for (k=2; k<=t; k++) {
                         k1 = k - 1;
                         temp = temp + integ[k] * 
                         (s1(k) * s2(t-k,j) + s1(k1,j) * s2(t-k1));}
                    temp = temp + integ[t+1] * s1(t,j) * s2(0);
                    s3(t,j) = -temp / integ[t+1];}}
          break;

          case cosh_:
          if (t == 0)  {
               s2(0) = sinh(s1(0));
               s3(0) = cosh(s1(0));
               for (j=0; j<J; j++) s2(0,j) = s1(0,j) * s3(0);
               for (j=0; j<J; j++) s3(0,j) = s1(0,j) * s2(0);}
          else {
               temp = s1(1) * s3(t-1);
               for (k=2; k<=t; k++) {
                    temp = temp + integ[k] * s1(k) * s3(t-k);}
               s2(t) = temp / integ[t];
               temp = s1(1) * s2(t-1);
               for (k=2; k<=t; k++) {
                    temp = temp + integ[k] * s1(k) * s2(t-k);}
               s3(t) = temp / integ[t];
               for (j=0; j<J; j++) {
                    temp = s1(1) * s3(t-1,j) + s1(0,j) * s3(t);
                    for (k=2; k<=t; k++) {
                         k1 = k - 1;
                         temp = temp + integ[k] * 
                         (s1(k) * s3(t-k,j) + s1(k1,j) * s3(t-k1));}
                    temp = temp + integ[t+1] * s1(t,j) * s3(0);
                    s2(t,j) = temp / integ[t+1];
                    temp = s1(1) * s2(t-1,j) + s1(0,j) * s2(t);
                    for (k=2; k<=t; k++) {
                         k1 = k - 1;
                         temp = temp + integ[k] * 
                         (s1(k) * s2(t-k,j) + s1(k1,j) * s2(t-k1));}
                    temp = temp + integ[t+1] * s1(t,j) * s2(0);
                    s3(t,j) = temp / integ[t+1];}}
          break;

          case exp_:
          if (t == 0) {
               s2(0) = exp(s1(0));
               for (j=0; j<J; j++) s2(0,j) = s1(0,j) * s2(0);}
          else {
               temp = s1(1) * s2(t-1);
               for (k=2; k<=t; k++) {
                    temp = temp + integ[k] * s1(k) * s2(t-k);}
               s2(t) = temp / integ[t];
               for (j=0; j<J; j++) {
                    temp = s1(1) * s2(t-1,j) + s1(0,j) * s2(t);
                    for (k=2; k<=t; k++) {
                         k1 = k - 1;
                         temp = temp + integ[k] *
                         (s1(k) * s2(t-k,j) + s1(k1,j) * s2(t-k1));}
                    temp = temp + integ[t+1] * s1(t,j) * s2(0);
                    s2(t,j) = temp / integ[t+1];}}
          break;

          case log_:
          if (t == 0) {
               s2(0) = log(s1(0));
               for (j=0; j<J; j++) s2(0,j) = s1(0,j) / s1(0);}
          else {
               if (t > 1) temp = s2(1) * s1(t-1); else temp = I_zero;
               for (k=2; k<t; k++) {
                    temp = temp + integ[k] * s2(k) * s1(t-k);}
               s2(t) = (s1(t) - temp / integ[t]) / s1(0);
               for (j=0; j<J; j++) {
                    temp = s2(1) * s1(t-1,j) + s2(0,j) * s1(t);
                    for (k=2; k<t; k++) {
                         k1 = k - 1;
                         temp = temp + integ[k] *
                         (s2(k) * s1(t-k,j) + s2(k1,j) * s1(t-k1));}
                    temp = temp + integ[t+1] * s2(t,j) * s1(0);
                    s2(t,j) = (s1(t,j) - temp / integ[t+1]) / s1(0);}}
          break;
          default: error("series computation1");}}
}
