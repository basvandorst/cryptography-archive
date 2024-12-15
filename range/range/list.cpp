#include "range.h"
#include "list.h"
// all constructors, destructors, operators, members of classes region and list

void remove(region* p)
{
p->next = region::free;
region::free = p;
}

void region::initiate(int n)
{
region :: dim = n;
region :: size = sizeof(region) + (n-1) * sizeof(dat);
region :: free = 0;
}

void get(region*& p)
{
if (region::free) {
     p = region::free; region::free = p->next;}
else {
     p = (region*) get_memory(region::size);
     int n = region::dim; region::dat* q = p->data; rvar::rva** t;
     for (int i=0; i<n; i++) {
          t = (rvar::rva**) &q[i].left;
          *t = &zero_init;
          t = (rvar::rva**) &q[i].right;
          *t = &zero_init;}
     zero_init.count += n + n;}
p->next = 0;
}

region& region :: operator=(region& a)
{
int i, n = region::dim;
for (i=0; i<n; i++) {
     data[i].type = a.data[i].type;
     data[i].left = a.data[i].left;
     data[i].right = a.data[i].right;}
return *this;
}

list :: list()
{
base = 0;
}

list :: ~list()
{
clear();
}

void list :: append(region *p)
{
region *q, *q1;
p->next = 0;
if (base) {
     q1 = base; q = q1->next;
     while (q) {
          q1 = q; q = q->next;}
     q1->next = p;}
else base = p;
}

void list :: clear()
{
if (base) {
     region *p = base, *q;
     do {
          q = p; p = p->next;}
     while (p);
     q->next = region::free;
     region::free = base;
     base = 0;}

}     

void list :: initiate(region *p)
{
clear();
region* q;
get(q);
*q = *p; // get sets *q.next to 0
base = q;
}

void list :: insert(region* p)
{
p->next = base;
base = p;
}

void list :: remove_first()
{
region *p = base;
base = p->next;
p->next = region::free;
region::free = p;
}
