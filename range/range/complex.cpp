#include "range.h"
#include "complex.h"

complex::complex(rvar& x, rvar& y) {real = x; imag = y;}

complex::complex(long x, long y) {real = rvar(x); imag = rvar(y);}

complex::complex() {;}

complex::complex(complex& a) {real = a.real; imag = a.imag;}

complex operator+(complex& a, complex& b)
{
complex c;
c.real = a.real + b.real;
c.imag = a.imag + b.imag;
return c;
}
     
complex operator-(complex& a)
{
complex b;
b.real = -a.real;
b.imag = -a.imag;
return b;
}
     
complex operator-(complex& a, complex& b)
{
complex c;
c.real = a.real - b.real;
c.imag = a.imag - b.imag;
return c;
}

complex operator*(complex& a, complex& b)
{
complex c;
c.real = a.real * b.real - a.imag * b.imag;
c.imag = a.real * b.imag + a.imag * b.real;
return c;
}

complex operator*(rvar& a, complex& b)
{
complex c;
c.real = a * b.real;
c.imag = a * b.imag;
return c;
}

complex operator/(complex& a, complex& b)
{
complex c;
if (is_zero(b.imag)) {
     c.real = a.real / b.real; c.imag = a.imag / b.real; return c;}
if (is_zero(b.real)) {
     c.real = a.imag / b.imag; c.imag = - a.real / b.imag; return c;}
c.real = abs_square(b);
if (c.real == zero) { // try to obtain a nonzero abs_square in c.real
     complex d = mid(b), e;
     d.real = abs(d.real); d.imag = abs(d.imag);
     e.real = range(b.real); e.imag = range(b.imag);
     c.real = d.real * d.real + d.imag * d.imag;
     c.imag = d.real * e.real + d.imag * e.imag;
     c.imag = c.imag + c.imag + (e.real * e.real) + (e.imag * e.imag);
     c.real = c.real % c.imag;}               
c.imag = (a.imag * b.real - a.real * b.imag) / c.real;
c.real = (a.real * b.real + a.imag * b.imag) / c.real; return c;
}

complex operator/(complex& a, rvar& b) {
complex c;
c.real = a.real / b;
c.imag = a.imag / b;
return c;
}

complex operator^(complex& a, rvar& b)
{
complex c = c_one, d; rvar b1, t;
if (!is_int(b)) {
     error("complex a ^ b with b not an integer");}
else if (a == c_zero && b <= zero) {
     error("complex a ^ b with a == c_zero and b not a positive integer");}
else {
     if (b < zero) {
          temporary_top_precision(); b1 = -b; restore_precision();
          d = c / a;}
     else {b1 = b; d = a;}
     if (!is_zero(b1)) {
          do {
               temporary_top_precision();
               t = trunc(div2(b1)); b1 = b1 - t - t; restore_precision();
               if (!is_zero(b1)) {c = c * d;}
               if (!is_zero(t)) {d = d * d; b1 = t;} else break;}
          while (1);}}
return c;
}

complex operator~(complex& a) // complex conjugate
{
complex b;
b.real = a.real;
b.imag = -a.imag;
return b;
}

complex operator%(complex& a, complex& b)
{
complex c;
c.real = a.real % b.real;
c.imag = a.imag % b.imag;
return c;
}

complex& complex::operator=(complex& a)
{
real = a.real; imag = a.imag; return *this;
}

void complex::operator+=(complex& a)
{
*this = *this + a;
}

void complex::operator-=(complex& a)
{
*this = *this - a;
}

void complex::operator*=(complex& a)
{
*this = *this * a;
}

void complex::operator/=(complex& a)
{
*this = *this / a;
}

int operator==(complex& a, complex& b)
{
if (a.real == b.real && a.imag == b.imag) return TRUE; else return FALSE;
}

int operator!=(complex& a, complex& b)
{
if (a.real != b.real || a.imag != b.imag) return TRUE; else return FALSE;
}

rvar abs(complex& a)
{
rvar b;
if (is_zero(a.imag)) b = abs(a.real);
else if (is_zero(a.real)) b = abs(a.imag);
else b = sqrt(a.real * a.real + a.imag * a.imag);
return b;
}

rvar abs_square(complex& a)
{
rvar b =  a.real * a.real + a.imag * a.imag; return b;
}

complex cos(complex& a)
{
rvar c = cos(a.real), s = sin(a.real), r = exp(a.imag), r_recip = one/r; 
c = div2(r + r_recip) * c; s = div2(r_recip - r) * s;
complex q(c, s); return q;
}

complex cosh(complex& a)
{
rvar c = cos(a.imag), s = sin(a.imag), r = exp(a.real), r_recip = one/r; 
c = div2(r + r_recip) * c; s = div2(r - r_recip) * s;
complex q(c, s); return q;
}

int no_complex_error(svar& entry_line)
{
char *p = !entry_line, c;
int i = entry_line.len() - 3, j = (p[i] << 7) + p[i+1], type[140];
p = p + j; i = -1;
errorstatus = display_error;
while (1) {
     c = *p;
     switch (c) {
          case end_: errorstatus = no_error; return TRUE;
          
          case const_: case pi_const_: type[++i] = 'R'; break;
          
          case 'i': case var_: type[++i] = 'C'; break;
          
          case abs_: type[i] = 'R'; break;
          
          case acos_: case asin_: case atan_: case log_: case sqrt_:
          if (type[i] == 'C') {
               error("function argument not a real constant");}
          break;
          
          case cos_: case cosh_: case exp_: case sin_: case sinh_:
          case tan_: case tanh_: case unary_minus_: break;
          
          case '+': case '-': case '*': case '/':
          if (type[i--] == 'C') type[i] = 'C'; break;
          
          case '^': case power_even_p_odd_q: case power_odd_p_odd_q:
          if (type[i--] == 'C') {
               error("complex a ^ b with b not real");}
          break;
          
          default: error("complex specification faulty");}
     if (errorstatus == error_reported) {
          show_error_pos (entry_line, (*(p+1) << 7) + *(p+2));
          errorstatus = exit_on_error; return FALSE;}
     p += 3;}
}

void entry(char* message, complex& cc, svar& entry_line, svar& variables)
{
int i = complex::evaluate_error_display, j = complex::evaluate_exit_on_error;
// save the evaluate settings; entry needs to impose own conditions
complex::evaluate_error_display = TRUE;
complex::evaluate_exit_on_error = FALSE;

int programmed_entry_line, first_cycle = TRUE;
if (*message) programmed_entry_line = FALSE; else programmed_entry_line = TRUE;
     
while (1) { 
     if (programmed_entry_line) {
          if (first_cycle) first_cycle = FALSE;
          else exit(1);}
     else svar_entry(message, entry_line); 
     compile(entry_line, variables);
     if (errorstatus == no_error) {
          if (no_complex_error(entry_line)) {
               if (variables == empty) { // constant entry
                    evaluate (cc, entry_line); 
                    if (complex::evaluate_error == FALSE) break;}
               else break;}}}
complex::evaluate_error_display = i;
complex::evaluate_exit_on_error = j;
}

void evaluate (complex& result, svar& entry_line, complex* x)
{
char c, *p = entry_line.tokens(), *q = !entry_line, *q1;
int i = -1; complex z; rvar r;
if (complex::evaluate_error_display == TRUE) errorstatus = display_error;
else if (complex::evaluate_exit_on_error == FALSE) errorstatus = signal_error;
else errorstatus = exit_on_error;
while (1) {
     c = *p;
     switch (c) {
          case end_: complex::evaluate_error = FALSE; result = c_num[0];
          errorstatus = exit_on_error; return;
          
          case const_: q1 = q + ((*(p+1) << 7) + *(p+2)); r = rvar(q1);
          c_num[++i] = complex(r); break;

          case 'i': c_num[++i] = complex(zero, one); break;
          case var_: c_num[++i] = x[*(p+2)]; break;
          case pi_const_: c_num[++i] = complex(pi); break;
          case unary_minus_: c_num[i] = - c_num[i]; break;
          case abs_: r = abs(c_num[i]); c_num[i] = complex(r); break;
          case exp_: c_num[i] = exp(c_num[i]); break;
          case cos_: c_num[i] = cos(c_num[i]); break;
          case sin_: c_num[i] = sin(c_num[i]); break;
          case tan_: c_num[i] = tan(c_num[i]); break;
          case cosh_: c_num[i] = cosh(c_num[i]); break;
          case sinh_: c_num[i] = sinh(c_num[i]); break;
          case tanh_: c_num[i] = tanh(c_num[i]); break;
          case acos_: case asin_: case atan_: case log_: case sqrt_:
          if (! is_zero(c_num[i].imag)) {
               error("function argument not pure real"); break;}
          switch (c) {
               case acos_: c_num[i].real = acos(c_num[i].real); break;
               case asin_: c_num[i].real = asin(c_num[i].real); break;
               case atan_: c_num[i].real = atan(c_num[i].real); break;
               case log_:  c_num[i].real =  log(c_num[i].real); break;
               case sqrt_: c_num[i].real = sqrt(c_num[i].real);} break;
          case '+': z = c_num[i--]; c_num[i] += z; break;
          case '-': z = c_num[i--]; c_num[i] -= z; break;
          case '*': z = c_num[i--]; c_num[i] *= z; break;
          case '/': z = c_num[i--]; c_num[i] /= z; break;
          case '^': case power_even_p_odd_q: case power_odd_p_odd_q:
          r = c_num[i--].real;
          if (is_zero(c_num[i].imag)) {
               switch (c) {
                    case '^': c_num[i].real = c_num[i].real ^ r; break;
                    case power_odd_p_odd_q: 
                    c_num[i].real = expon_o(c_num[i].real, r); break;
                    case power_even_p_odd_q:
                    c_num[i].real = expon_e(c_num[i].real, r); break;}}
          else {
                if (is_int(r)) c_num[i] = c_num[i] ^ r;
                else error("complex a ^ b with b not an integer");}
          break;
          default: error("complex specification Faulty");}
     if (errorstatus == error_reported) {
          complex::evaluate_error = TRUE;
          if (complex::evaluate_error_display == TRUE) {
               show_error_pos(entry_line, (*(p+1) << 7) + *(p+2));}
          if (complex::evaluate_exit_on_error == TRUE) exit(1);
          errorstatus = exit_on_error;
          return;}
     else p += 3;} 
}
complex exp(complex& a)
{
rvar r = exp(a.real), c = r * cos(a.imag), s = r * sin(a.imag);
complex q(c, s); return q;
}

void list_tokens(complex& dummy)
{
complex r = dummy;
cout << "\
                           SYMBOLS RECOGNIZED\n\
   (     )       abs()   exp()  sqrt(R)     Operations             Evaluation\n\
 ln(R) = log(R) asin(R) acos(R) atan(R)    low rank: + -          left to right\n\
    i     pi     sin()   cos()   tan()     med rank: * /          left to right\n\
    constants    sinh()  cosh()  tanh()     hi rank: ^ -(prefix) right to left\n\
  function(R) denotes a real-valued function that must have a real argument\n\n";
}

complex mid(complex& a)
{
complex b; b.real = mid(a.real); b.imag = mid(a.imag); return b;
}

svar rtos(complex& a, int i, int j)
{
svar s1 = rtos(a.real, i, j), s2, s3, s4 = " * i";
if (a.imag < zero) {
     s3 = rtos(-a.imag, i, j); s2 = svar(" - ");}
else {
     s3 = rtos(a.imag, i, j); s2 = svar(" + ");}
s1 = s1 + s2 + s3 + s4; return s1;
}

complex sin(complex& a)
{
rvar c = cos(a.real), s = sin(a.real), r = exp(a.imag), r_recip = one/r; 
s = div2(r + r_recip) * s; c = div2(r - r_recip) * c;
complex q(s, c); return q;
}

complex sinh(complex& a)
{
rvar c = cos(a.imag), s = sin(a.imag), r = exp(a.real), r_recip = one/r; 
s = div2(r + r_recip) * s; c = div2(r - r_recip) * c;
complex q(c, s); return q;
}

void swap(complex& a, complex& b)
{
swap(a.real, b.real); swap(a.imag, b.imag);
}

complex tan(complex& a)
{
complex t = sin(a) / cos(a);
return t;
}

complex tanh(complex& a)
{
complex t = sinh(a) / cosh(a);
return t;
}

void test(complex& a, int d1, int d2)
{
test(a.real, d1, d2); test(a.imag, d1, d2);
}














