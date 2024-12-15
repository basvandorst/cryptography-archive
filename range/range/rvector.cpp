#include "range.h"
#include "rational.h"
#include "vector.h"
#include "rvector.h"

// rvector constructors, operators, and destructor

rvector::rvector()
{
ve = null_rvector.ve;
ve->count++;
}

rvector::rvector(rvector& a)
{
ve = a.ve;
ve->count++;
}

rvector::rvector(int n)
{
int i, t;
rvec* v = (rvec*) get_memory(sizeof(rvec) + (n-1) * sizeof(rational));
v->count = 1; v->no_comp = n;
rvar::rva** p = (rvar::rva**) v->comp;
for (i=0; i<n; i++) {
     t = i + i;
     p[t] = &zero_init; p[++t] = &one_init;}
zero_init.count += n; one_init.count +=n;
ve = v;
}

rvector& rvector::operator=(rvector& a)
{
a.ve->count++;
if (--ve->count == 0) {
     rational* p = ve->comp;
     for (int i=0; i<ve->no_comp; i++) (p+i)->rational::~rational();
     return_memory(ve);}
ve = a.ve; return *this;
}

rvector operator+(rvector& a, rvector& b)
{
int i, n = a.ve->no_comp;
if (n != b.ve->no_comp) {
     error("operand mismatch on rvector +"); return null_rvector;}
rvector c(n);
for (i=0; i<n; i++) c.ve->comp[i] = a.ve->comp[i] + b.ve->comp[i];
return c;
}

rvector operator-(rvector& a, rvector& b)
{
int i, n = a.ve->no_comp;
if (n != b.ve->no_comp) {
     error("operand mismatch on rvector -"); return null_rvector;}
rvector c(n);
for (i=0; i<n; i++) c.ve->comp[i] = a.ve->comp[i] - b.ve->comp[i];
return c;
}

rvector operator-(rvector& a)
{
int i, n = a.ve->no_comp;
rvector c(n);
for (i=0; i<n; i++) c.ve->comp[i] = -a.ve->comp[i];
return c;
}

rational operator*(rvector& a, rvector& b)
{
int i, n = a.ve->no_comp;
if (n != b.ve->no_comp) {
     error("operand mismatch on rvector *"); return r_zero;}
rational c = r_zero;
for (i=0; i<n; i++) c = c + a.ve->comp[i] * b.ve->comp[i];
return c;
}

rvector copy(rvector& a)
{
int i, n = a.ve->no_comp;
rvector b(n);
for (i=0; i<n; i++) b.ve->comp[i] = a.ve->comp[i];
return b;
}

void entry(char* message, rvector& a, svector& a1)
{
int i, n = a.ve->no_comp; a1 = svector(n);
svar start_part(message), mess, end_part(")   ");
start_part += svar("(");
for (i=0; i<n; i++) {
     mess = start_part + ltos(i+1) + end_part;
     entry(!mess, a(i), a1(i));}
cout << "\n";
}

rvar length(rvector& a)
{
rational b = a * a;
rvar r = b; r = sqrt(r);
return r;
}

void rvector::no_comps_reduced_to(int q)
{
if (q < 0) q = 0;
int n = ve->no_comp;
if (q > n) q = n;
ve->no_comp = q;
rational* p = ve->comp;
for (int i=q; i<n; i++) (p+i)->rational::~rational();
}

svector rtos(rvector& a)
{
int k, n = a.ve->no_comp;
svector b(n);
for (k=0; k<n; k++) b(k) = rtos(a(k));
return b;
}

rvector:: ~rvector()
{
if (--ve->count == 0) {
     rational* p = ve->comp;
     for (int i=0; i<ve->no_comp; i++) (p+i)->rational::~rational();
     return_memory(ve);}
}

// rmatrix constructors, operators, and destructor

rmatrix::rmatrix()
{
ma = null_rmatrix.ma;
ma->count++;
}

rmatrix::rmatrix(rmatrix& a)
{
ma = a.ma;
ma->count++;
}

rmatrix::rmatrix(int m, int n)
{
int i, t, comp_no = m * n;
rmat* v = (rmat*) get_memory(sizeof(rmat) + (comp_no-1) * sizeof(rational));
v->count = 1; v->no_rows = m; v->no_cols = n; v->no_comp = comp_no;
rvar::rva** p = (rvar::rva**) v->comp;
for (i=0; i<comp_no; i++) {
     t = i + i;
     p[t] = &zero_init; p[++t] = &one_init;}
zero_init.count += comp_no; one_init.count += comp_no;
ma = v;
}

rmatrix& rmatrix::operator=(rmatrix& a)
{
a.ma->count++;
if (--ma->count == 0) {
     rational* p = ma->comp;
     for (int i=0; i<ma->no_comp; i++) (p+i)->rational::~rational();
     return_memory(ma);}
ma = a.ma; return *this;
}

rmatrix operator+(rmatrix& a, rmatrix& b)
{
int i, m = a.ma->no_rows, n = a.ma->no_cols, t = a.ma->no_comp;
if (m != b.ma->no_rows || n != b.ma->no_cols) {
     error("operand mismatch on rmatrix operation +"); return null_rmatrix;}
rmatrix c(m, n);
for (i=0; i<t; i++) c.ma->comp[i] = a.ma->comp[i] + b.ma->comp[i];
return c;
}

rmatrix operator-(rmatrix& a, rmatrix& b)
{
int i, m = a.ma->no_rows, n = a.ma->no_cols, t = a.ma->no_comp;
if (m != b.ma->no_comp || n != b.ma->no_cols) {
     error("operand mismatch on rmatrix operation -"); return null_rmatrix;}
rmatrix c(m, n);
for (i=0; i<t; i++) c.ma->comp[i] = a.ma->comp[i] - b.ma->comp[i];
return c;
}

rmatrix operator-(rmatrix& a)
{
rmatrix b(a.ma->no_rows, a.ma->no_cols);
int i, t = a.ma->no_comp;
for (i=0; i<t; i++) b.ma->comp[i] = -a.ma->comp[i];
return b;
}

rmatrix operator*(rmatrix& a, rmatrix& b)
{
int i, j, k, m = a.ma->no_rows, n = b.ma->no_cols, t = a.ma->no_cols;
if (t != b.ma->no_rows) {
     error("operand mismatch on rmatrix operation *"); return null_rmatrix;}
rmatrix c(m, n); rational sum;
for (i=0; i<m; i++) {
     for (j=0; j<n; j++) {
          sum = r_zero;
          for (k=0; k<t; k++) sum = sum + a(i,k) * b(k,j);
          c(i,j) = sum;}}
return c;
}

rvector operator*(rmatrix& a, rvector& b)
{
int i, j, m = a.ma->no_rows, n = a.ma->no_cols, t = b.no_comps();
if (t != n) {
     error("operand mismatch on matrix - vector operation *");
     return null_rvector;}
rvector c(m); rational sum;
for (i=0; i<m; i++) {
     sum = r_zero;
     for (j=0; j<n; j++) {
          sum = sum + a(i,j) * b(j);
          c(i) = sum;}}
return c;
}

rmatrix operator^(rmatrix& a, int q)
{
int i, m = a.ma->no_rows, n = a.ma->no_cols, q1, t;
if (m != n) {
     error("non-square rmatrix on rmatrix operation ^"); return null_rmatrix;}
rmatrix c, d;
if (q < 0)      {d = inverse(a); q = -q;}
else if (q > 0)  d = a;
else {
     c = rmatrix(n,n); // result is I if q = 0
     for (i=0; i<n; i++) c(i,i) = r_one;
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

rvector operator/(rvector& a, rmatrix& b)
{
int i, i1, j, k, m = b.ma->no_rows, n = b.ma->no_cols;
if (m != n) {
     error("non-square rmatrix on rmatrix operation /"); return null_rvector;}
if (n != a.no_comps()) {
     error("operand mismatch on rmatrix operation /"); return null_rvector;}
rvector c = copy(a); rmatrix d = copy(b);
rational divisor, multiplier, sum, temp;

for (i=0; i<n; i++) { 
     i1 = i+1;
     if (d(i,i) == r_zero) {
          for (k=i1; k<n; k++) {
               if (d(k,i) != r_zero) {
                     for (j=i; j<n; j++) {
                          temp = d(i,j); d(i,j) = d(k,j); d(k,j) = temp;}
                     temp = c(i); c(i) = c(k); c(k) = temp;
                    break;}}
          if (k == n) {
               error("rvector / rmatrix with singular rmatrix");
               return c;}}
     divisor = d(i,i);
     for (k=i1; k<n; k++) {
          multiplier = d(k,i) / divisor;
          for (j=i1; j<n; j++) d(k,j) = d(k,j) - multiplier * d(i,j);
          c(k) = c(k) - multiplier * c(i);}}

for (i=n-1; i>=0; i--) {
     sum = c(i);
     for (k=i+1; k<n; k++) sum = sum - d(i,k) * c(k);
     c(i) = sum / d(i,i);}
return c;
}

rmatrix copy(rmatrix& a)
{
int i, m = a.ma->no_rows, n = a.ma->no_cols, t = a.ma->no_comp;
rmatrix b(m, n);
for (i=0; i<t; i++) b.ma->comp[i] = a.ma->comp[i];
return b;
}

rational det(rmatrix& a)
{
int i, i1, j, k, n = a.ma->no_rows;
if (n != a.ma->no_cols) {
     error("det of non_square rmatrix"); return r_zero;}
rmatrix b = copy(a);
rational d, divisor, multiplier, temp;

d = r_one;
for (i=0; i<n; i++) { 
     i1 = i+1;
     if (b(i,i) == r_zero) {
          for (k=i1; k<n; k++) {
               if (b(k,i) != r_zero) {
                     for (j=i; j<n; j++) {
                          temp = b(i,j); b(i,j) = b(k,j); b(k,j) = temp;}
                    break;}}
          if (k == n) {d = r_zero; break;}}
     divisor = b(i,i); d = d * divisor;
     for (k=i1; k<n; k++) {
          multiplier = b(k,i) / divisor;
          for (j=i1; j<n; j++) b(k,j) = b(k,j) - multiplier * b(i,j);}}
return d;
}

void entry(char* message, rmatrix& a, smatrix& a1)
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

rmatrix inverse(rmatrix& a)
{
int i, i1, j, k, n = a.ma->no_rows;
if (n != a.ma->no_cols) {
     error("inverse of non_square rmatrix"); return null_rmatrix;}
rmatrix b = copy(a), c(n, n);
rational divisor, multiplier, sum, temp;

for (i=0; i<n; i++) c(i,i) = r_one;                  // c begins as I
for (i=0; i<n; i++) { 
     i1 = i+1;
     if (b(i,i) == r_zero) {
          for (k=i1; k<n; k++) {
               if (b(k,i) != r_zero) {
                     for (j=i; j<n; j++) {
                          temp = b(i,j); b(i,j) = b(k,j); b(k,j) = temp;}
                     for (j=0; j<n; j++) {
                          temp = c(i,j); c(i,j) = c(k,j); c(k,j) = temp;}
                    break;}}
          if (k == n) {
               error("inverse of singular rmatrix"); return c;}}
     divisor = b(i,i);
     for (k=i1; k<n; k++) {
          multiplier = b(k,i) / divisor;
          for (j=i1; j<n; j++) b(k,j) = b(k,j) - multiplier * b(i,j);
          for (j=0; j<n; j++)  c(k,j) = c(k,j) - multiplier * c(i,j);}}

for (i=n-1; i>=0; i--) {
     i1 = i+1; divisor = b(i,i);
     for (j=0; j<n; j++) {
          sum = c(i,j);
          for (k=i1; k<n; k++) sum = sum - b(i,k) * c(k,j);
          c(i,j) = sum / divisor;}}
return c;
}

smatrix rtos(rmatrix& a)
{
int k, l, m = a.ma->no_rows, n = a.ma->no_cols;
smatrix b(m,n);
for (k=0; k<m; k++) {
     for (l=0; l<n; l++) {
           b(k,l) = rtos(a(k,l));}}
return b;
}

rmatrix:: ~rmatrix()
{
if (--ma->count == 0) {
     rational* p = ma->comp;
     for (int i=0; i<ma->no_comp; i++) (p+i)->rational::~rational();
     return_memory(ma);}
}







