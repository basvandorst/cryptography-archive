#include "range.h"
#include "vector.h"

// svector constructors, operators, and destructor

svector::svector()
{
ve = null_svector.ve;
ve->count++;
}

svector::svector(svector& a)
{
ve = a.ve;
ve->count++;
}

svector::svector(int n)
{
svec* v = (svec*) get_memory(sizeof(svec) + (n-1) * sizeof(svar));
v->count = 1; v->no_comp = n;
svar::sva** p = (svar::sva**) v->comp;
for (int t=0; t<n; t++) p[t] = &empty_init;
empty_init.count += n;
ve = v;
}

svector& svector::operator=(svector& a)
{
a.ve->count++;
if (--ve->count == 0) {
     svar* p = ve->comp;
     for (int i=0; i<ve->no_comp; i++) (p+i)->svar::~svar();
     return_memory(ve);}
ve = a.ve; return *this;
}

svector copy(svector& a)
{
int i, n = a.ve->no_comp;
svector b(n);
for (i=0; i<n; i++) b.ve->comp[i] = a.ve->comp[i];
return b;
}

void svector::no_comps_reduced_to(int q)
{
if (q < 0) q = 0;
int n = ve->no_comp;
if (q > n) q = n;
ve->no_comp = q;
svar* p = ve->comp;
for (int i=q; i<n; i++) (p+i)->svar::~svar();
}

svector:: ~svector()
{
if (--ve->count == 0) {
     svar* p = ve->comp;
     for (int i=0; i<ve->no_comp; i++) (p+i)->svar::~svar();
     return_memory(ve);}
}

// smatrix constructors, operators, and destructor

smatrix::smatrix()
{
ma = null_smatrix.ma;
ma->count++;
}

smatrix::smatrix(smatrix& a)
{
ma = a.ma;
ma->count++;
}

smatrix::smatrix(int m, int n)
{
int comp_no = m * n;
smat* v = (smat*) get_memory(sizeof(smat) + (comp_no-1) * sizeof(svar));
v->count = 1; v->no_rows = m; v->no_cols = n; v->no_comp = comp_no;
svar::sva** p = (svar::sva**) v->comp;
for (int t=0; t<comp_no; t++) p[t] = &empty_init;
empty_init.count += comp_no;
ma = v;
}

smatrix& smatrix::operator=(smatrix& a)
{
a.ma->count++;
if (--ma->count == 0) {
     svar* p = ma->comp;
     for (int i=0; i<ma->no_comp; i++) (p+i)->svar::~svar();
     return_memory(ma);}
ma = a.ma; return *this;
}

int smatrix::dim(int i)
{
if (i == 1) return ma->no_rows; else return ma->no_cols;
}

smatrix copy(smatrix& a)
{
int i, m = a.ma->no_rows, n = a.ma->no_cols, t = a.ma->no_comp;
smatrix b(m, n);
for (i=0; i<t; i++) b.ma->comp[i] = a.ma->comp[i];
return b;
}

smatrix:: ~smatrix()
{
if (--ma->count == 0) {
     svar* p = ma->comp;
     for (int i=0; i<ma->no_comp; i++) (p+i)->svar::~svar();
     return_memory(ma);}
}

// vector constructors, operators, and destructor

vector::vector()
{
ve = null_vector.ve;
ve->count++;
}

vector::vector(vector& a)
{
ve = a.ve;
ve->count++;
}

vector::vector(int n)
{
vec* v = (vec*) get_memory(sizeof(vec) + (n-1) * sizeof(rvar));
v->count = 1; v->no_comp = n;
rvar::rva** p = (rvar::rva**) v->comp;
for (int t=0; t<n; t++) p[t] = &zero_init;
zero_init.count += n;
ve = v;
}

vector& vector::operator=(vector& a)
{
a.ve->count++;
if (--ve->count == 0) {
     rvar* p = ve->comp;
     for (int i=0; i<ve->no_comp; i++) (p+i)->rvar::~rvar();
     return_memory(ve);}
ve = a.ve; return *this;
}

vector operator+(vector& a, vector& b)
{
int i, n = a.ve->no_comp;
if (n != b.ve->no_comp) {
     error("operand mismatch on vector +"); return null_vector;}
vector c(n);
for (i=0; i<n; i++) c.ve->comp[i] = a.ve->comp[i] + b.ve->comp[i];
return c;
}

vector operator-(vector& a, vector& b)
{
int i, n = a.ve->no_comp;
if (n != b.ve->no_comp) {
     error("operand mismatch on vector -"); return null_vector;}
vector c(n);
for (i=0; i<n; i++) c.ve->comp[i] = a.ve->comp[i] - b.ve->comp[i];
return c;
}

vector operator-(vector& a)
{
int i, n = a.ve->no_comp;
vector c(n);
for (i=0; i<n; i++) c.ve->comp[i] = -a.ve->comp[i];
return c;
}

rvar operator*(vector& a, vector& b)
{
int i, n = a.ve->no_comp;
if (n != b.ve->no_comp) {
     error("operand mismatch on vector *"); return zero;}
rvar c = zero;
for (i=0; i<n; i++) c = c + a.ve->comp[i] * b.ve->comp[i];
return c;
}

vector copy(vector& a)
{
int i, n = a.ve->no_comp;
vector b(n);
for (i=0; i<n; i++) b.ve->comp[i] = a.ve->comp[i];
return b;
}

void entry(char* message, vector& a, svector& a1)
{
int i, n = a.ve->no_comp; a1 = svector(n);
svar start_part(message), mess, end_part(")   ");
start_part += svar("(");
for (i=0; i<n; i++) {
     mess = start_part + ltos(i+1) + end_part;
     entry(!mess, a(i), a1(i));}
cout << "\n";
}

void evaluate(vector& a, svector& a1)
{
int i, n = a.ve->no_comp;
for (i=0; i<n; i++) evaluate(a(i), a1(i));
}

rvar length(vector& a)
{
rvar b = a * a;
b = sqrt(b);
return b;
}

vector mid(vector& a)
{
int i, n = a.ve->no_comp;
vector b(n);
for (i=0; i<n; i++) b.ve->comp[i] = mid(a.ve->comp[i]);
return b;
}

void vector::no_comps_reduced_to(int q)
{
if (q < 0) q = 0;
int n = ve->no_comp;
if (q > n) q = n;
ve->no_comp = q;
rvar* p = ve->comp;
for (int i=q; i<n; i++) (p+i)->rvar::~rvar();
}

svector rtos(vector& a, int i, int j)
{
int k, n = a.ve->no_comp;
svector b(n);
for (k=0; k<n; k++) b(k) = rtos(a(k), i, j);
return b;
}

void test(vector& a, int i, int j)
{
int k, n = a.ve->no_comp;
for (k=0; k<n; k++) test(a.ve->comp[k], i, j);
}

vector:: ~vector()
{
if (--ve->count == 0) {
     rvar* p = ve->comp;
     for (int i=0; i<ve->no_comp; i++) (p+i)->rvar::~rvar();
     return_memory(ve);}
}

// matrix constructors, destructor, and operators

matrix::matrix()
{
ma = null_matrix.ma;
ma->count++;
}

matrix::matrix(matrix& a)
{
ma = a.ma;
ma->count++;
}

matrix::matrix(int m, int n)
{
int comp_no = m * n;
mat* v = (mat*) get_memory(sizeof(mat) + (comp_no-1) * sizeof(rvar));
v->count = 1; v->no_rows = m; v->no_cols = n; v->no_comp = comp_no;
rvar::rva** p = (rvar::rva**) v->comp;
for (int t=0; t<comp_no; t++) p[t] = &zero_init;
zero_init.count += comp_no;
ma = v;
}

matrix:: ~matrix()
{
if (--ma->count == 0) {
     rvar* p = ma->comp;
     for (int i=0; i<ma->no_comp; i++) (p+i)->rvar::~rvar();
     return_memory(ma);}
}

matrix& matrix::operator=(matrix& a)
{
a.ma->count++;
if (--ma->count == 0) {
     rvar* p = ma->comp;
     for (int i=0; i<ma->no_comp; i++) (p+i)->rvar::~rvar();
     return_memory(ma);}
ma = a.ma; return *this;
}

matrix operator+(matrix& a, matrix& b)
{
int i, m = a.ma->no_rows, n = a.ma->no_cols, t = a.ma->no_comp;
if (m != b.ma->no_rows || n != b.ma->no_cols) {
     error("operand mismatch on matrix operation +"); return null_matrix;}
matrix c(m, n);
for (i=0; i<t; i++) c.ma->comp[i] = a.ma->comp[i] + b.ma->comp[i];
return c;
}

matrix operator-(matrix& a, matrix& b)
{
int i, m = a.ma->no_rows, n = a.ma->no_cols, t = a.ma->no_comp;
if (m != b.ma->no_comp || n != b.ma->no_cols) {
     error("operand mismatch on matrix operation -"); return null_matrix;}
matrix c(m, n);
for (i=0; i<t; i++) c.ma->comp[i] = a.ma->comp[i] - b.ma->comp[i];
return c;
}

matrix operator-(matrix& a)
{
matrix b(a.ma->no_rows, a.ma->no_cols);
int i, n = a.ma->no_comp;
for (i=0; i<n; i++) b.ma->comp[i] = -a.ma->comp[i];
return b;
}

matrix operator*(matrix& a, matrix& b)
{
int i, j, k, m = a.ma->no_rows, n = b.ma->no_cols, t = a.ma->no_cols;
if (t != b.ma->no_rows) {
     error("operand mismatch on matrix operation *"); return null_matrix;}
matrix c(m, n); rvar sum;
for (i=0; i<m; i++) {
     for (j=0; j<n; j++) {
          sum = zero;
          for (k=0; k<t; k++) sum = sum + a(i,k) * b(k,j);
          c(i,j) = sum;}}
return c;
}

vector operator*(matrix& a, vector& b)
{
int i, j, m = a.ma->no_rows, n = a.ma->no_cols, t = b.no_comps();
if (t != n) {
     error("operand mismatch on matrix - vector operation *");
     return null_vector;}
vector c(m); rvar sum;
for (i=0; i<m; i++) {
     sum = zero;
     for (j=0; j<n; j++) {
          sum = sum + a(i,j) * b(j);
          c(i) = sum;}}
return c;
}

matrix operator^(matrix& a, int q)
{
int i, m = a.ma->no_rows, n = a.ma->no_cols, q1, t;
if (m != n) {
     error("non-square matrix on matrix operation ^"); return null_matrix;}
matrix c, d;
if (q < 0)      {d = inverse(a); q = -q;}
else if (q > 0)  d = a;
else {
     c = matrix(n,n); // result is I if q = 0
     for (i=0; i<n; i++) c(i,i) = one;
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

vector operator/(vector& a, matrix& b)
{
int i, i1, j, k, m = b.ma->no_rows, n = b.ma->no_cols;
if (m != n) {
     error("non-square matrix on matrix operation /"); return null_vector;}
if (n != a.no_comps()) {
     error("operand mismatch on matrix operation /"); return null_vector;}
vector c = copy(a); matrix d = copy(b);
rvar divisor, max, multiplier, sum, temp;

for (i=0; i<n; i++) { 
     i1 = i+1;
     max = abs(d(i,i)); j = i;
     for (k=i1; k<n; k++) {
          temp = abs(d(k,i));
          if (temp > max) {
               max = temp; j = k;}}
     if (j != i) {
              for (k=i; k<n; k++) swap(d(i,k), d(j,k));
          swap(c(i), c(j));}
     if (max == zero) {
          if (matrix::failure_report_wanted) matrix::failure = TRUE;
          else error("vector / matrix with matrix singular or near-singular");
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
if (matrix::failure_report_wanted) matrix::failure = FALSE;
return c;
}

// matrix functions in alphabetical order

matrix copy(matrix& a)
{
int i, m = a.ma->no_rows, n = a.ma->no_cols, t = a.ma->no_comp;
matrix b(m, n);
for (i=0; i<t; i++) b.ma->comp[i] = a.ma->comp[i];
return b;
}

rvar det(matrix& a)
{
int i, i1, j, k, n = a.ma->no_rows;
if (n != a.ma->no_cols) {
     error("det of non_square matrix"); return zero;}
matrix b = copy(a);
rvar d, divisor, multiplier;

d = one;
for (i=0; i<n; i++) { 
     i1 = i+1;
     if (b(i,i) == zero) {
          for (k=i1; k<n; k++) {
               if (b(k,i) != zero) {
                     for (j=i; j<n; j++) swap(b(i,j), b(k,j));
                    break;}}
          if (k == n) {
               i1 = 1;
               for (j=i; j<n; j++) {
                    multiplier = zero;
                    for (k=i; k<n; k++) {
                         multiplier = max(abs(b(k,j)), multiplier);} 
                    d = d * multiplier * rvar(i1++);}
               d = zero % d; return d;}}
     divisor = b(i,i); d = d * divisor;
     for (k=i1; k<n; k++) {
          multiplier = b(k,i) / divisor;
          for (j=i1; j<n; j++) b(k,j) -= multiplier * b(i,j);}}
return d;
}

void entry(char* message, matrix& a, smatrix& a1)
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

void evaluate(matrix& a, smatrix& a1)
{
int i, j, m = a.ma->no_rows, n = a.ma->no_cols;
for (i=0; i<m; i++) {
     for (j=0; j<n; j++) evaluate(a(i,j), a1(i,j));}
}

matrix inverse(matrix& a)
{
int i, i1, j, k, n = a.ma->no_rows;
if (n != a.ma->no_cols) {
     error("inverse of non_square matrix"); return null_matrix;}
matrix b = copy(a), c(n,n);
rvar divisor, max, multiplier, sum, temp;

for (i=0; i<n; i++) c(i,i) = one;   // c begins as I
for (i=0; i<n; i++) { 
     i1 = i+1;
     max = abs(b(i,i)); j = i;
     for (k=i1; k<n; k++) {
          temp = abs(b(k,i));
          if (temp > max) {
               max = temp; j = k;}}
     if (j != i) {
          for (k=i; k<n; k++) swap(b(i,k), b(j,k));
          for (k=0; k<n; k++) swap(c(i,k), c(j,k));}
     if (max == zero) {
          if (matrix::failure_report_wanted) matrix::failure = TRUE;
          else error("inverse of possibly singular matrix");
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
if (matrix::failure_report_wanted) matrix::failure = FALSE;
return c;
}

smatrix rtos(matrix& a, int i, int j)
{
int k, l, m = a.ma->no_rows, n = a.ma->no_cols;
smatrix b(m,n);
for (k=0; k<m; k++) {
     for (l=0; l<n; l++) {
          b(k,l) = rtos(a(k,l), i, j);}}
return b;
}

void test(matrix& a, int i, int j)
{
int k, n = a.ma->no_comp;
for (k=0; k<n; k++) test(a.ma->comp[k], i, j);
}

