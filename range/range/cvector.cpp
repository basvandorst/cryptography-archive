#include "range.h"
#include "complex.h"
#include "vector.h"
#include "cvector.h"

// cvector constructors, operators, and destructor

cvector::cvector()
{
ve = null_cvector.ve;
ve->count++;
}

cvector::cvector(cvector& a)
{
ve = a.ve;
ve->count++;
}

cvector::cvector(int n)
{
int i, t;
cvec* v = (cvec*) get_memory(sizeof(cvec) + (n-1) * sizeof(complex));
v->count = 1; v->no_comp = n;
rvar::rva** p = (rvar::rva**) v->comp;
for (i=0; i<n; i++) {
     t = i + i;
     p[t] = &zero_init; p[++t] = &zero_init;}
zero_init.count += n + n;
ve = v;
}

cvector& cvector::operator=(cvector& a)
{
a.ve->count++;
if (--ve->count == 0) {
     complex* p = ve->comp;
     for (int i=0; i<ve->no_comp; i++) (p+i)->complex::~complex();
     return_memory(ve);}
ve = a.ve; return *this;
}

cvector operator+(cvector& a, cvector& b)
{
int i, n = a.ve->no_comp;
if (n != b.ve->no_comp) {
     error("operand mismatch on cvector +"); return null_cvector;}
cvector c(n);
for (i=0; i<n; i++) c.ve->comp[i] = a.ve->comp[i] + b.ve->comp[i];
return c;
}

cvector operator-(cvector& a, cvector& b)
{
int i, n = a.ve->no_comp;
if (n != b.ve->no_comp) {
     error("operand mismatch on cvector -"); return null_cvector;}
cvector c(n);
for (i=0; i<n; i++) c.ve->comp[i] = a.ve->comp[i] - b.ve->comp[i];
return c;
}

cvector operator-(cvector& a)
{
int i, n = a.ve->no_comp;
cvector c(n);
for (i=0; i<n; i++) c.ve->comp[i] = -a.ve->comp[i];
return c;
}

complex operator*(cvector& a, cvector& b)
{
int i, n = a.ve->no_comp;
if (n != b.ve->no_comp) {
     error("operand mismatch on cvector *"); return c_zero;}
complex c = c_zero;
for (i=0; i<n; i++) c = c + a.ve->comp[i] * b.ve->comp[i];
return c;
}

cvector copy(cvector& a)
{
int i, n = a.ve->no_comp;
cvector b(n);
for (i=0; i<n; i++) b.ve->comp[i] = a.ve->comp[i];
return b;
}

void entry(char* message, cvector& a, svector& a1)
{
int i, n = a.ve->no_comp; a1 = svector(n);
svar start_part(message), mess, end_part(")   ");
start_part += svar("(");
for (i=0; i<n; i++) {
     mess = start_part + ltos(i+1) + end_part;
     entry(!mess, a(i), a1(i));}
cout << "\n";
}

void evaluate(cvector& a, svector& a1)
{
int i, n = a.ve->no_comp;
for (i=0; i<n; i++) evaluate(a(i), a1(i));
}

rvar length(cvector& a)
{
complex b = a * a;
rvar c = sqrt(b.real * b.real + b.imag * b.imag);
return c;
}

void cvector::no_comps_reduced_to(int q)
{
if (q < 0) q = 0;
int n = ve->no_comp;
if (q > n) q = n;
ve->no_comp = q;
complex* p = ve->comp;
for (int i=q; i<n; i++) (p+i)->complex::~complex();
}

svector rtos(cvector& a, int i, int j)
{
int k, n = a.ve->no_comp;
svector b(n);
for (k=0; k<n; k++) b(k) = rtos(a(k), i, j);
return b;
}

void test(cvector& a, int i, int j)
{
int k, n = a.ve->no_comp;
for (k=0; k<n; k++) test(a.ve->comp[k], i, j);
}

cvector:: ~cvector()
{
if (--ve->count == 0) {
     complex* p = ve->comp;
     for (int i=0; i<ve->no_comp; i++) (p+i)->complex::~complex();
     return_memory(ve);}
}

// cmatrix constructors, operators, and destructor

cmatrix::cmatrix()
{
ma = null_cmatrix.ma;
ma->count++;
}

cmatrix::cmatrix(cmatrix& a)
{
ma = a.ma;
ma->count++;
}

cmatrix::cmatrix(int m, int n)
{
int i, t, comp_no = m * n;
cmat* v = (cmat*) get_memory(sizeof(cmat) + (comp_no-1) * sizeof(complex));
v->count = 1; v->no_rows = m; v->no_cols = n; v->no_comp = comp_no;
rvar::rva** p = (rvar::rva**) v->comp;
for (i=0; i<comp_no; i++) {
     t = i + i;
     p[t] = &zero_init; p[++t] = &zero_init;}
zero_init.count += comp_no + comp_no;
ma = v;
}

cmatrix& cmatrix::operator=(cmatrix& a)
{
a.ma->count++;
if (--ma->count == 0) {
     complex* p = ma->comp;
     for (int i=0; i<ma->no_comp; i++) (p+i)->complex::~complex();
     return_memory(ma);}
ma = a.ma; return *this;
}

cmatrix operator+(cmatrix& a, cmatrix& b)
{
int i, m = a.ma->no_rows, n = a.ma->no_cols, t = a.ma->no_comp;
if (m != b.ma->no_rows || n != b.ma->no_cols) {
     error("operand mismatch on cmatrix operation +"); return null_cmatrix;}
cmatrix c(m, n);
for (i=0; i<t; i++) c.ma->comp[i] = a.ma->comp[i] + b.ma->comp[i];
return c;
}

cmatrix operator-(cmatrix& a, cmatrix& b)
{
int i, m = a.ma->no_rows, n = a.ma->no_cols, t = a.ma->no_comp;
if (m != b.ma->no_comp || n != b.ma->no_cols) {
     error("operand mismatch on cmatrix operation -"); return null_cmatrix;}
cmatrix c(m, n);
for (i=0; i<t; i++) c.ma->comp[i] = a.ma->comp[i] - b.ma->comp[i];
return c;
}

cmatrix operator-(cmatrix& a)
{
cmatrix b(a.ma->no_rows, a.ma->no_cols);
int i, t = a.ma->no_comp;
for (i=0; i<t; i++) b.ma->comp[i] = -a.ma->comp[i];
return b;
}

cmatrix operator*(cmatrix& a, cmatrix& b)
{
int i, j, k, m = a.ma->no_rows, n = b.ma->no_cols, t = a.ma->no_cols;
if (t != b.ma->no_rows) {
     error("operand mismatch on cmatrix operation *"); return null_cmatrix;}
cmatrix c(m, n); complex sum;
for (i=0; i<m; i++) {
     for (j=0; j<n; j++) {
          sum = c_zero;
          for (k=0; k<t; k++) sum = sum + a(i,k) * b(k,j);
          c(i,j) = sum;}}
return c;
}

cvector operator*(cmatrix& a, cvector& b)
{
int i, j, m = a.ma->no_rows, n = a.ma->no_cols, t = b.no_comps();
if (t != n) {
     error("operand mismatch on cmatrix - cvector operation *");
     return null_cvector;}
cvector c(m); complex sum;
for (i=0; i<m; i++) {
     sum = c_zero;
     for (j=0; j<n; j++) {
          sum = sum + a(i,j) * b(j);
          c(i) = sum;}}
return c;
}

cmatrix operator^(cmatrix& a, int q)
{
int i, m = a.ma->no_rows, n = a.ma->no_cols, t;
if (m != n) {
     error("non-square cmatrix on cmatrix operation ^"); return null_cmatrix;}
cmatrix c, d;
if (q < 0)      {d = inverse(a); q = -q;}
else if (q > 0)  d = a;
else {
     c = cmatrix(n,n); // result is I if q = 0
     for (i=0; i<n; i++) c(i,i) = c_one;
     return c;}
int c_empty = TRUE;
do {
     t = q >> 1; q = q - (t << 1);
     if (q > 0) {
          if (c_empty) {c = d; c_empty = FALSE;}
          else c = c * d;}
     if (t > 0) {d = d * d; q = t;} else break;}
while (1);
return c;
}

cvector operator/(cvector& a, cmatrix& b)
{
int i, i1, j, k, m = b.ma->no_rows, n = b.ma->no_cols;
if (m != n) {
     error("non-square cmatrix on cmatrix operation /"); return null_cvector;}
if (n != a.no_comps()) {
     error("operand mismatch on cmatrix operation /"); return null_cvector;}
cvector c = copy(a); cmatrix d = copy(b);
complex divisor, multiplier, sum; rvar max, temp;

for (i=0; i<n; i++) { 
     i1 = i+1;
     max = abs_square(d(i,i)); j = i;
     for (k=i1; k<n; k++) {
          temp = abs_square(d(k,i));
          if (temp > max) {
               max = temp; j = k;}}
     if (j != i) {
              for (k=i; k<n; k++) swap(d(i,k), d(j,k));
          swap(c(i), c(j));}
     if (max == zero) {
          if (cmatrix::failure_report_wanted) cmatrix::failure = TRUE;
          else error("cvector / cmatrix with possibly singular cmatrix");
          return c;}
     divisor = d(i,i);
     for (k=i1; k<n; k++) {
          multiplier = d(k,i) / divisor;
          for (j=i1; j<n; j++) d(k,j) -= multiplier * d(i,j);
          c(k) -= multiplier * c(i);}}

for (i=n-1; i>=0; i--) {
     sum = c(i);
     for (k=i+1; k<n; k++) sum -= d(i,k) * c(k);
     c(i) = sum / d(i,i);}
if (cmatrix::failure_report_wanted) cmatrix::failure = FALSE;
return c;
}

cmatrix copy(cmatrix& a)
{
int i, m = a.ma->no_rows, n = a.ma->no_cols, t = a.ma->no_comp;
cmatrix b(m, n);
for (i=0; i<t; i++) b.ma->comp[i] = a.ma->comp[i];
return b;
}

complex det(cmatrix& a)
{
int i, i1, j, k, n = a.ma->no_rows;
if (n != a.ma->no_cols) {
     error("det of non_square cmatrix"); return c_zero;}
cmatrix b = copy(a);
complex d, divisor, multiplier, temp;

d = c_one;
for (i=0; i<n; i++) { 
     i1 = i+1;
     if (b(i,i) == c_zero) {
          for (k=i1; k<n; k++) {
               if (b(k,i) != c_zero) {
                     for (j=i; j<n; j++) {
                          temp = b(i,j); b(i,j) = b(k,j); b(k,j) = temp;}
                    break;}}
          if (k == n) {
               i1 = 1; rvar maximum, r = abs_square(d), r_temp;
               for (j=i; j<n; j++) {
                    maximum = zero;
                    for (k=i; k<n; k++) {
                         r_temp = abs_square(b(k,j));
                         maximum = max(r_temp, maximum);} 
                    r = r * maximum * rvar(i1*i1++);}
               r = zero % sqrt(r);
               d = complex(r, r); return d;}}
     divisor = b(i,i); d = d * divisor;
     for (k=i1; k<n; k++) {
          multiplier = b(k,i) / divisor;
          for (j=i1; j<n; j++) b(k,j) = b(k,j) - multiplier * b(i,j);}}
return d;
}

void entry(char* message, cmatrix& a, smatrix& a1)
{
int i, j, m = a.ma->no_rows, n = a.ma->no_cols;
a1 = smatrix(m,n);
svar start_part(message), mess, end_part(")   ");
start_part += svar("(");
for (i=0; i<m; i++) {
     for (j=0; j<n; j++) {
          mess = start_part + ltos(i+1) + svar(",") + ltos(j+1) + end_part;
          entry(!mess, a(i,j), a1(i,j));}
     cout << "\n";}
}

void evaluate(cmatrix& a, smatrix& a1)
{
int i, j, m = a.ma->no_rows, n = a.ma->no_cols;
for (i=0; i<m; i++) {
     for (j=0; j<n; j++) evaluate(a(i,j), a1(i,j));}
}

cmatrix inverse(cmatrix& a)
{
int i, i1, j, k, n = a.ma->no_rows;
if (n != a.ma->no_cols) {
     error("inverse of non_square cmatrix"); return null_cmatrix;}
cmatrix b = copy(a), c(n, n);
complex divisor, multiplier, sum; rvar max, temp;

for (i=0; i<n; i++) c(i,i) = c_one;    // c begins as I
for (i=0; i<n; i++) { 
     i1 = i+1;
     max = abs_square(b(i,i)); j = i;
     for (k=i1; k<n; k++) {
          temp = abs_square(b(k,i));
          if (temp > max) {
               max = temp; j = k;}}
     if (j != i) {
          for (k=i; k<n; k++) swap(b(i,k), b(j,k));
          for (k=0; k<n; k++) swap(c(i,k), c(j,k));}
     if (max == zero) {
          if (cmatrix::failure_report_wanted) cmatrix::failure = TRUE;
          else error("inverse of possibly singular cmatrix");
          return c;}
     divisor = b(i,i);
     for (k=i1; k<n; k++) {
          multiplier = b(k,i) / divisor;
          for (j=i1; j<n; j++) b(k,j) -= multiplier * b(i,j);
          for (j=0; j<n; j++)  c(k,j) -= multiplier * c(i,j);}}

for (i=n-1; i>=0; i--) {
     i1 = i+1; divisor = b(i,i);
     for (j=0; j<n; j++) {
          sum = c(i,j);
          for (k=i1; k<n; k++) sum -= b(i,k) * c(k,j);
          c(i,j) = sum / divisor;}}
if (cmatrix::failure_report_wanted) cmatrix::failure = FALSE;
return c;
}

smatrix rtos(cmatrix& a, int i, int j)
{
int k, l, m = a.ma->no_rows, n = a.ma->no_cols;
smatrix b(m,n);
for (k=0; k<m; k++) {
     for (l=0; l<n; l++) {
          b(k,l) = rtos(a(k,l), i, j);}}
return b;
}

void test(cmatrix& a, int i, int j)
{
int k, n = a.ma->no_comp;
for (k=0; k<n; k++) test(a.ma->comp[k], i, j);
}

cmatrix:: ~cmatrix()
{
if (--ma->count == 0) {
     complex* p = ma->comp;
     for (int i=0; i<ma->no_comp; i++) (p+i)->complex::~complex();
     return_memory(ma);}
}







