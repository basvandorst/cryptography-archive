#include "range.h"
#include "vector.h"
#include "ivector.h"

// ivector constructors, operators, and destructor

ivector::ivector()
{
ve = null_ivector.ve;
ve->count++;
}

ivector::ivector(ivector& a)
{
ve = a.ve;
ve->count++;
}

ivector::ivector(int n)
{
ivec* v = (ivec*) get_memory(sizeof(ivec) + (n-1) * sizeof(int));
v->count = 1; v->no_comp = n;
int* p = v->comp;
for (int t=0; t<n; t++) p[t] = 0;
ve = v;
}

ivector& ivector::operator=(ivector& a)
{
a.ve->count++;
if (--ve->count == 0) return_memory(ve);
ve = a.ve; return *this;
}

ivector operator+(ivector& a, ivector& b)
{
int i, n = a.ve->no_comp;
if (n != b.ve->no_comp) {
     error("operand mismatch on ivector +"); return null_ivector;}
ivector c(n);
for (i=0; i<n; i++) c.ve->comp[i] = a.ve->comp[i] + b.ve->comp[i];
return c;
}

ivector operator-(ivector& a, ivector& b)
{
int i, n = a.ve->no_comp;
if (n != b.ve->no_comp) {
     error("operand mismatch on ivector -"); return null_ivector;}
ivector c(n);
for (i=0; i<n; i++) c.ve->comp[i] = a.ve->comp[i] - b.ve->comp[i];
return c;
}

ivector operator-(ivector& a)
{
int i, n = a.ve->no_comp;
ivector c(n);
for (i=0; i<n; i++) c.ve->comp[i] = -a.ve->comp[i];
return c;
}

int operator*(ivector& a, ivector& b)
{
int i, n = a.ve->no_comp;
if (n != b.ve->no_comp) {
     error("operand mismatch on ivector *"); return 0;}
int c = 0;
for (i=0; i<n; i++) c = c + a.ve->comp[i] * b.ve->comp[i];
return c;
}

ivector copy(ivector& a)
{
int i, n = a.ve->no_comp;
ivector b(n);
for (i=0; i<n; i++) b.ve->comp[i] = a.ve->comp[i];
return b;
}

void ivector::no_comps_reduced_to(int q)
{
if (q < 0) q = 0;
int n = ve->no_comp;
if (q > n) q = n;
ve->no_comp = q;
}

ivector:: ~ivector()
{
if (--ve->count == 0) return_memory(ve);
}

// imatrix constructors, operators, and destructor

imatrix::imatrix()
{
ma = null_imatrix.ma;
ma->count++;
}

imatrix::imatrix(imatrix& a)
{
ma = a.ma;
ma->count++;
}

imatrix::imatrix(int m, int n)
{
int comp_no = m * n;
imat* v = (imat*) get_memory(sizeof(imat) + (comp_no-1) * sizeof(int));
v->count = 1; v->no_rows = m; v->no_cols = n; v->no_comp = comp_no;
int* p = v->comp;
for (int t=0; t<comp_no; t++) p[t] = 0;
ma = v;
}

imatrix& imatrix::operator=(imatrix& a)
{
a.ma->count++;
if (--ma->count == 0) return_memory(ma);
ma = a.ma; return *this;
}

imatrix operator+(imatrix& a, imatrix& b)
{
int i, m = a.ma->no_rows, n = a.ma->no_cols, t = a.ma->no_comp;
if (m != b.ma->no_rows || n != b.ma->no_cols) {
     error("operand mismatch on imatrix operation +"); return null_imatrix;}
imatrix c(m, n);
for (i=0; i<t; i++) c.ma->comp[i] = a.ma->comp[i] + b.ma->comp[i];
return c;
}

imatrix operator-(imatrix& a, imatrix& b)
{
int i, m = a.ma->no_rows, n = a.ma->no_cols, t = a.ma->no_comp;
if (m != b.ma->no_comp || n != b.ma->no_cols) {
     error("operand mismatch on imatrix operation -"); return null_imatrix;}
imatrix c(m, n);
for (i=0; i<t; i++) c.ma->comp[i] = a.ma->comp[i] - b.ma->comp[i];
return c;
}

imatrix operator-(imatrix& a)
{
imatrix b(a.ma->no_rows, a.ma->no_cols);
int i, t = a.ma->no_comp;
for (i=0; i<t; i++) b.ma->comp[i] = -a.ma->comp[i];
return b;
}

imatrix operator*(imatrix& a, imatrix& b)
{
int i, j, k, m = a.ma->no_rows, n = b.ma->no_cols, t = a.ma->no_cols;
if (t != b.ma->no_rows) {
     error("operand mismatch on imatrix operation *"); return null_imatrix;}
imatrix c(m, n); int sum;
for (i=0; i<m; i++) {
     for (j=0; j<n; j++) {
          sum = 0;
          for (k=0; k<t; k++) sum = sum + a(i,k) * b(k,j);
          c(i,j) = sum;}}
return c;
}

ivector operator*(imatrix& a, ivector& b)
{
int i, j, m = a.ma->no_rows, n = a.ma->no_cols, t = b.no_comps();
if (t != n) {
     error("operand mismatch on imatrix - ivector operation *");
     return null_ivector;}
ivector c(m); int sum;
for (i=0; i<m; i++) {
     sum = 0;
     for (j=0; j<n; j++) {
          sum = sum + a(i,j) * b(j);
          c(i) = sum;}}
return c;
}

imatrix copy(imatrix& a)
{
int i, m = a.ma->no_rows, n = a.ma->no_cols, t = a.ma->no_comp;
imatrix b(m, n);
for (i=0; i<t; i++) b.ma->comp[i] = a.ma->comp[i];
return b;
}

imatrix:: ~imatrix()
{
if (--ma->count == 0) return_memory(ma);
}







