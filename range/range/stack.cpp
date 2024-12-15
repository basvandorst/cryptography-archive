#include "range.h"
#include "stack.h"

stack :: stack(int n)
{
no_comps = n;
no_chars = (n-1) * sizeof(rvar) + sizeof(level);
sta = (level*) get_memory(no_chars); // construction of starting lowest level
sta->forward = 0;
sta->back = 0;
rvar::rva** q = (rvar::rva**) sta->comp;
for (int t=0; t<no_comps; t++) q[t] = &zero_init;
zero_init.count += no_comps;
not_empty = FALSE;
}

void stack::pop()
{
// Note that a pop never results in destruction of a level; as a stack
// gets used, it generally gradually increases in size.
if (not_empty) { 
     sta = sta->back;
     if (sta->back == 0) not_empty = FALSE;}
}

void stack::push()
{
if (sta->forward == 0) { // construction of new lowest level when necessary
     level* p = (level*) get_memory(no_chars);
     p->back = sta;
     p->forward = 0;
     sta->forward = p;
     rvar::rva** q = (rvar::rva**) p->comp;
     for (int t=0; t<no_comps; t++) q[t] = &zero_init;
     zero_init.count += no_comps;}
sta = sta->forward;
not_empty = TRUE;
}

stack:: ~stack()
{ // destructor needed if stack used temporarily by some function
level* q; rvar* p;
while (sta->back != 0) sta = sta->back;
do {
     p = sta->comp;
     for (int i=0; i< no_comps; i++) (p+i)->rvar::~rvar();
     q = sta;
     sta = sta->forward;
     return_memory(q);}
while (sta != 0);
}
