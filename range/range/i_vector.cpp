#include "range.h"
#include "vector.h"
#include "interval.h"
#include "i_vector.h"

// Ivector constructors, operators, and destructor

Ivector::Ivector()
{
ve = null_Ivector.ve;
ve->count++;
}

Ivector::Ivector(Ivector& a)
{
ve = a.ve;
ve->count++;
}

Ivector::Ivector(int n)
{
vec* v = (vec*) get_memory(sizeof(vec) + (n-1) * sizeof(interval));
v->count = 1; v->no_comp = n;
interval* p = v->comp; rvar::rva** q;
for (int i=0; i<n; i++) {
     q = (rvar::rva**) &p[i].mid; *q = &zero_init;
     q = (rvar::rva**) &p[i].wid; *q = &zero_init;}
zero_init.count += n + n;
ve = v;
}

Ivector& Ivector::operator=(Ivector& a)
{
a.ve->count++;
if (--ve->count == 0) {
     interval* p = ve->comp;
     for (int i=0; i<ve->no_comp; i++) (p+i)->interval::~interval();
     return_memory(ve);}
ve = a.ve; return *this;
}

Ivector:: ~Ivector()
{
if (--ve->count == 0) {
     interval* p = ve->comp;
     int i, n = ve->no_comp;
     for (i=0; i<n; i++) (p+i)->interval::~interval();
     return_memory(ve);}
}

// Imatrix constructors, destructor, and operators

Imatrix::Imatrix()
{
ma = null_Imatrix.ma;
ma->count++;
}

Imatrix::Imatrix(Imatrix& a)
{
ma = a.ma;
ma->count++;
}

Imatrix::Imatrix(int m, int n)
{
int comp_no = m * n;
mat* v = (mat*) get_memory(sizeof(mat) + (comp_no-1) * sizeof(interval));
v->count = 1; v->no_rows = m; v->no_cols = n; v->no_comp = comp_no;
interval* p = v->comp; rvar::rva** q;
for (int i=0; i<comp_no; i++) {
     q = (rvar::rva**) &p[i].mid; *q = &zero_init;
     q = (rvar::rva**) &p[i].wid; *q = &zero_init;}
zero_init.count += comp_no + comp_no;
ma = v;
}

Imatrix& Imatrix::operator=(Imatrix& a)
{
a.ma->count++;
if (--ma->count == 0) {
     interval* p = ma->comp;
     for (int i=0; i<ma->no_comp; i++) (p+i)->interval::~interval();
     return_memory(ma);}
ma = a.ma; return *this;
}

Imatrix:: ~Imatrix()
{
if (--ma->count == 0) {
     interval* p = ma->comp;
     int i, n = ma->no_comp;
     for (i=0; i<n; i++) (p+i)->interval::~interval();
     return_memory(ma);}
}

