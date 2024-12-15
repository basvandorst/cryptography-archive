// This may look like C code, but it is really -*- C++ -*-
/* 
Copyright (C) 1988 Free Software Foundation
    written by Doug Lea (dl@rocky.oswego.edu)

This file is part of the GNU C++ Library.  This library is free
software; you can redistribute it and/or modify it under the terms of
the GNU Library General Public License as published by the Free
Software Foundation; either version 2 of the License, or (at your
option) any later version.  This library is distributed in the hope
that it will be useful, but WITHOUT ANY WARRANTY; without even the
implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the GNU Library General Public License for more details.
You should have received a copy of the GNU Library General Public
License along with this library; if not, write to the Free Software
Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifdef __GNUG__
#pragma implementation
#endif
#include <builtin.h>
#include "NewsGroup.List.h"

ng_nodeListNode Nilng_nodeListNode;

class init_Nilng_nodeListNode
{
public:
  init_Nilng_nodeListNode() 
  {
    Nilng_nodeListNode.tl = &Nilng_nodeListNode;
    Nilng_nodeListNode.ref = -1;
  }
};

static init_Nilng_nodeListNode Nilng_nodeListNode_initializer;

ng_nodeList& ng_nodeList::operator = (ng_nodeList& a)
{
  reference(a.P);
  dereference(P);
  P = a.P;
  return *this;
}

ng_node ng_nodeList::pop()
{
  ng_node res = P->hd;
  ng_nodeListNode* tail = P->tl;
  reference(tail);
  dereference(P);
  P = tail;
  return res;
}

void ng_nodeList::set_tail(ng_nodeList& a)
{
  reference(a.P);
  dereference(P->tl);
  P->tl = a.P;
}

ng_nodeList ng_nodeList::nth(int n)
{
  for (ng_nodeListNode* p = P; n-- > 0; p = p->tl);
  reference(p);
  return ng_nodeList(p);
}

ng_nodeList ng_nodeList::last()
{
  ng_nodeListNode* p = P;
  if (p != &Nilng_nodeListNode) while (p->tl != &Nilng_nodeListNode) p = p->tl;
  reference(p);
  return ng_nodeList(p);
}

void ng_nodeList::append(ng_nodeList& l)
{
  ng_nodeListNode* p = P;
  ng_nodeListNode* a = l.P;
  reference(a);
  if (p != &Nilng_nodeListNode)
  {
    while (p->tl != &Nilng_nodeListNode) p = p->tl;
    p->tl = a;
  }
  else
    P = a;
}

int ng_nodeList::length()
{
  int l = 0;
  for (ng_nodeListNode* p = P; p != &Nilng_nodeListNode; p = p->tl) ++l;
  return l;
}

ng_node&  ng_nodeList::operator [] (int n)
{
  for (ng_nodeListNode* p = P; n-- > 0; p = p->tl);
  return (p->hd);
}

int operator == (ng_nodeList& x, ng_nodeList& y)
{
  ng_nodeListNode* a = x.P;
  ng_nodeListNode* b = y.P;
  
  for (;;)
  {
    if (a == &Nilng_nodeListNode)
      return b == &Nilng_nodeListNode;
    else if (b == &Nilng_nodeListNode)
      return 0;
    else if (ng_nodeEQ(a->hd, b->hd))
    {
      a = a->tl;
      b = b->tl;
    }
    else
      return 0;
  }
}


void ng_nodeList::apply(ng_nodeProcedure f)
{
  for(ng_nodeListNode* p = P; p != &Nilng_nodeListNode; p = p->tl)
    (*f)((p->hd));
}

void ng_nodeList::subst(ng_node  old, ng_node  repl)
{
  for(ng_nodeListNode* p = P; p != &Nilng_nodeListNode; p = p->tl)
    if (ng_nodeEQ(p->hd, old))
      p->hd = repl;
}

ng_node ng_nodeList::reduce(ng_nodeCombiner f, ng_node  base)
{
  ng_node r = base;
  for(ng_nodeListNode* p = P; p != &Nilng_nodeListNode; p = p->tl)
    r = (*f)(r, (p->hd));
  return r;
}

int ng_nodeList::position(ng_node  targ)
{
  int l = 0;
  ng_nodeListNode* p = P;
  for (;;)
  {
    if (p == &Nilng_nodeListNode)
      return -1;
    else if (ng_nodeEQ(p->hd, targ))
      return l;
    else
    {
      ++l;
      p = p->tl;
    }
  }
}

int ng_nodeList::contains(ng_node  targ)
{
  ng_nodeListNode* p = P;
  for (;;)
  {
    if (p == &Nilng_nodeListNode)
      return 0;
    else if (ng_nodeEQ(p->hd, targ))
      return 1;
    else
      p = p->tl;
  }
}

ng_nodeList ng_nodeList::find(ng_node  targ)
{
  ng_nodeListNode* p = P;
  while (p != &Nilng_nodeListNode && !ng_nodeEQ(p->hd, targ))
    p=p->tl;
  reference(p);
  return ng_nodeList(p);
}

Pix ng_nodeList::seek(ng_node  targ)
{
  ng_nodeListNode* p = P; 
  for (;;)
  {
    if (p == &Nilng_nodeListNode)
      return 0;
    else if (ng_nodeEQ(p->hd, targ))
      return Pix(p);
    else
      p = p->tl;
  }
}

int ng_nodeList::owns(Pix i)
{
  ng_nodeListNode* p = P; 
  for (;;)
  {
    if (p == &Nilng_nodeListNode)
      return 0;
    else if (Pix(p) == i)
      return 1;
    else
      p = p->tl;
  }
}

ng_nodeList ng_nodeList::find(ng_nodeList& target)
{
  ng_nodeListNode* targ = target.P;
  if (targ == &Nilng_nodeListNode)
    return ng_nodeList(targ);

  ng_nodeListNode* p = P;
  while (p != &Nilng_nodeListNode)
  {
    if (ng_nodeEQ(p->hd, targ->hd))
    {
      ng_nodeListNode* a = p->tl;
      ng_nodeListNode* t = targ->tl;
      for(;;)
      {
        if (t == &Nilng_nodeListNode)
        {
          reference(p);
          return ng_nodeList(p);
        }
        else if (a == &Nilng_nodeListNode || !ng_nodeEQ(a->hd, t->hd))
          break;
        else
        {
          a = a->tl;
          t = t->tl;
        }
      }
    }
    p = p->tl;
  }
  return ng_nodeList(&Nilng_nodeListNode);
}

int ng_nodeList::contains(ng_nodeList& target)
{
  ng_nodeListNode* targ = target.P;
  if (targ == &Nilng_nodeListNode)
    return 0;

  ng_nodeListNode* p = P;
  while (p != &Nilng_nodeListNode)
  {
    if (ng_nodeEQ(p->hd, targ->hd))
    {
      ng_nodeListNode* a = p->tl;
      ng_nodeListNode* t = targ->tl;
      for(;;)
      {
        if (t == &Nilng_nodeListNode)
          return 1;
        else if (a == &Nilng_nodeListNode || !ng_nodeEQ(a->hd, t->hd))
          break;
        else
        {
          a = a->tl;
          t = t->tl;
        }
      }
    }
    p = p->tl;
  }
  return 0;
}

void ng_nodeList::del(ng_node  targ)
{
  ng_nodeListNode* h = P;

  for (;;)
  {
    if (h == &Nilng_nodeListNode)
    {
      P = h;
      return;
    }
    else if (ng_nodeEQ(h->hd, targ))
    {
      ng_nodeListNode* nxt = h->tl;
      reference(nxt);
      dereference(h);
      h = nxt;
    }
    else
      break;
  }

  ng_nodeListNode* trail = h;
  ng_nodeListNode* p = h->tl;
  while (p != &Nilng_nodeListNode)
  {
    if (ng_nodeEQ(p->hd, targ))
    {
      ng_nodeListNode* nxt = p->tl;
      reference(nxt);
      dereference(p);
      trail->tl = nxt;
      p = nxt;
    }
    else
    {
      trail = p;
      p = p->tl;
    }
  }
  P = h;
}

void ng_nodeList::del(ng_nodePredicate f)
{
  ng_nodeListNode* h = P;
  for (;;)
  {
    if (h == &Nilng_nodeListNode)
    {
      P = h;
      return;
    }
    else if ((*f)(h->hd))
    {
      ng_nodeListNode* nxt = h->tl;
      reference(nxt);
      dereference(h);
      h = nxt;
    }
    else
      break;
  }

  ng_nodeListNode* trail = h;
  ng_nodeListNode* p = h->tl;
  while (p != &Nilng_nodeListNode)
  {
    if ((*f)(p->hd))
    {
      ng_nodeListNode* nxt = p->tl;
      reference(nxt);
      dereference(p);
      trail->tl = nxt;
      p = nxt;
    }
    else
    {
      trail = p;
      p = p->tl;
    }
  }
  P = h;
}

void ng_nodeList::select(ng_nodePredicate f)
{
  ng_nodeListNode* h = P;
  for (;;)
  {
    if (h == &Nilng_nodeListNode)
    {
      P = h;
      return;
    }
    else if (!(*f)(h->hd))
    {
      ng_nodeListNode* nxt = h->tl;
      reference(nxt);
      dereference(h);
      h = nxt;
    }
    else
      break;
  }
  ng_nodeListNode* trail = h;
  ng_nodeListNode* p = h->tl;
  while (p != &Nilng_nodeListNode)
  {
    if (!(*f)(p->hd))
    {
      ng_nodeListNode* nxt = p->tl;
      reference(nxt);
      dereference(p);
      trail->tl = nxt;
      p = nxt;
    }
    else
    {
      trail = p;
      p = p->tl;
    }
  }
  P = h;
}

void ng_nodeList::reverse()
{
  ng_nodeListNode* l = &Nilng_nodeListNode;
  ng_nodeListNode* p = P; 
  while (p != &Nilng_nodeListNode)
  {
    ng_nodeListNode* nxt = p->tl;
    p->tl = l;
    l = p;
    p = nxt;
  }
  P = l;
}


ng_nodeList copy(ng_nodeList& x)
{
  ng_nodeListNode* a = x.P;
  if (a == &Nilng_nodeListNode)
    return ng_nodeList(a);
  else
  {
    ng_nodeListNode* h = newng_nodeListNode(a->hd);
    ng_nodeListNode* trail = h;
    for(a = a->tl; a != &Nilng_nodeListNode; a = a->tl)
    {
      ng_nodeListNode* n = newng_nodeListNode(a->hd);
      trail->tl = n;
      trail = n;
    }
    trail->tl = &Nilng_nodeListNode;
    return ng_nodeList(h);
  }
}


ng_nodeList subst(ng_node  old, ng_node  repl, ng_nodeList& x)
{
  ng_nodeListNode* a = x.P;
  if (a == &Nilng_nodeListNode)
    return ng_nodeList(a);
  else
  {
    ng_nodeListNode* h = new ng_nodeListNode;
    h->ref = 1;
    if (ng_nodeEQ(a->hd, old))
      h->hd = repl;
    else
      h->hd = a->hd;
    ng_nodeListNode* trail = h;
    for(a = a->tl; a != &Nilng_nodeListNode; a = a->tl)
    {
      ng_nodeListNode* n = new ng_nodeListNode;
      n->ref = 1;
      if (ng_nodeEQ(a->hd, old))
        n->hd = repl;
      else
        n->hd = a->hd;
      trail->tl = n;
      trail = n;
    }
    trail->tl = &Nilng_nodeListNode;
    return ng_nodeList(h);
  }
}

ng_nodeList combine(ng_nodeCombiner f, ng_nodeList& x, ng_nodeList& y)
{
  ng_nodeListNode* a = x.P;
  ng_nodeListNode* b = y.P;
  if (a == &Nilng_nodeListNode || b == &Nilng_nodeListNode)
    return ng_nodeList(&Nilng_nodeListNode);
  else
  {
    ng_nodeListNode* h = newng_nodeListNode((*f)(a->hd, b->hd));
    ng_nodeListNode* trail = h;
    a = a->tl;
    b = b->tl;
    while (a != &Nilng_nodeListNode && b != &Nilng_nodeListNode)
    {
      ng_nodeListNode* n = newng_nodeListNode((*f)(a->hd, b->hd));
      trail->tl = n;
      trail = n;
      a = a->tl;
      b = b->tl;
    }
    trail->tl = &Nilng_nodeListNode;
    return ng_nodeList(h);
  }
}

ng_nodeList reverse(ng_nodeList& x)
{
  ng_nodeListNode* a = x.P;
  if (a == &Nilng_nodeListNode)
    return ng_nodeList(a);
  else
  {
    ng_nodeListNode* l = newng_nodeListNode(a->hd);
    l->tl = &Nilng_nodeListNode;
    for(a = a->tl; a != &Nilng_nodeListNode; a = a->tl)
    {
      ng_nodeListNode* n = newng_nodeListNode(a->hd);
      n->tl = l;
      l = n;
    }
    return ng_nodeList(l);
  }
}

ng_nodeList append(ng_nodeList& x, ng_nodeList& y)
{
  ng_nodeListNode* a = x.P;
  ng_nodeListNode* b = y.P;
  reference(b);
  if (a != &Nilng_nodeListNode)
  {
    ng_nodeListNode* h = newng_nodeListNode(a->hd);
    ng_nodeListNode* trail = h;
    for(a = a->tl; a != &Nilng_nodeListNode; a = a->tl)
    {
      ng_nodeListNode* n = newng_nodeListNode(a->hd);
      trail->tl = n;
      trail = n;
    }
    trail->tl = b;
    return ng_nodeList(h);
  }
  else
    return ng_nodeList(b);
}

void ng_nodeList::prepend(ng_nodeList& y)
{
  ng_nodeListNode* b = y.P;
  if (b != &Nilng_nodeListNode)
  {
    ng_nodeListNode* h = newng_nodeListNode(b->hd);
    ng_nodeListNode* trail = h;
    for(b = b->tl; b != &Nilng_nodeListNode; b = b->tl)
    {
      ng_nodeListNode* n = newng_nodeListNode(b->hd);
      trail->tl = n;
      trail = n;
    }
    trail->tl = P;
    P = h;
  }
}

ng_nodeList concat(ng_nodeList& x, ng_nodeList& y)
{
  ng_nodeListNode* a = x.P;
  ng_nodeListNode* b = y.P;
  if (a != &Nilng_nodeListNode)
  {
    ng_nodeListNode* h = newng_nodeListNode(a->hd);
    ng_nodeListNode* trail = h;
    for(a = a->tl; a != &Nilng_nodeListNode; a = a->tl)
    {
      ng_nodeListNode* n = newng_nodeListNode(a->hd);
      trail->tl = n;
      trail = n;
    };
    for(;b != &Nilng_nodeListNode; b = b->tl)
    {
      ng_nodeListNode* n = newng_nodeListNode(b->hd);
      trail->tl = n;
      trail = n;
    }
    trail->tl = &Nilng_nodeListNode;
    return ng_nodeList(h);
  }
  else if (b != &Nilng_nodeListNode)
  {
    ng_nodeListNode* h = newng_nodeListNode(b->hd);
    ng_nodeListNode* trail = h;
    for(b = b->tl; b != &Nilng_nodeListNode; b = b->tl)
    {
      ng_nodeListNode* n = newng_nodeListNode(b->hd);
      trail->tl = n;
      trail = n;
    }
    trail->tl = &Nilng_nodeListNode;
    return ng_nodeList(h);
  }
  else
    return ng_nodeList(&Nilng_nodeListNode);
}

ng_nodeList select(ng_nodePredicate f, ng_nodeList& x)
{
  ng_nodeListNode* a = x.P;
  ng_nodeListNode* h = &Nilng_nodeListNode;
  while (a != &Nilng_nodeListNode)
  {
    if ((*f)(a->hd))
    {
      h = newng_nodeListNode(a->hd);
      ng_nodeListNode* trail = h;
      for(a = a->tl; a != &Nilng_nodeListNode; a = a->tl)
      {
        if ((*f)(a->hd))
        {
          ng_nodeListNode* n = newng_nodeListNode(a->hd);
          trail->tl = n;
          trail = n;
        }
      }
      trail->tl = &Nilng_nodeListNode;
      break;
    }
    else
      a = a->tl;
  }
  return ng_nodeList(h);
}

ng_nodeList remove(ng_nodePredicate f, ng_nodeList& x)
{
  ng_nodeListNode* a = x.P;
  ng_nodeListNode* h = &Nilng_nodeListNode;
  while (a != &Nilng_nodeListNode)
  {
    if (!(*f)(a->hd))
    {
      h = newng_nodeListNode(a->hd);
      ng_nodeListNode* trail = h;
      for(a = a->tl; a != &Nilng_nodeListNode; a = a->tl)
      {
        if (!(*f)(a->hd))
        {
          ng_nodeListNode* n = newng_nodeListNode(a->hd);
          trail->tl = n;
          trail = n;
        }
      }
      trail->tl = &Nilng_nodeListNode;
      break;
    }
    else
      a = a->tl;
  }
  return ng_nodeList(h);
}

ng_nodeList remove(ng_node  targ, ng_nodeList& x)
{
  ng_nodeListNode* a = x.P;
  ng_nodeListNode* h = &Nilng_nodeListNode;
  while (a != &Nilng_nodeListNode)
  {
    if (!(ng_nodeEQ(a->hd, targ)))
    {
      h = newng_nodeListNode(a->hd);
      ng_nodeListNode* trail = h;
      for(a = a->tl; a != &Nilng_nodeListNode; a = a->tl)
      {
        if (!ng_nodeEQ(a->hd, targ))
        {
          ng_nodeListNode* n = newng_nodeListNode(a->hd);
          trail->tl = n;
          trail = n;
        }
      }
      trail->tl = &Nilng_nodeListNode;
      break;
    }
    else
      a = a->tl;
  }
  return ng_nodeList(h);
}

ng_nodeList map(ng_nodeMapper f, ng_nodeList& x)
{
  ng_nodeListNode* a = x.P;
  ng_nodeListNode* h = &Nilng_nodeListNode;
  if (a != &Nilng_nodeListNode)
  {
    h = newng_nodeListNode((*f)(a->hd));
    ng_nodeListNode* trail = h;
    for(a = a->tl; a != &Nilng_nodeListNode; a = a->tl)
    {
      ng_nodeListNode* n = newng_nodeListNode((*f)(a->hd));
      trail->tl = n;
      trail = n;
    }
    trail->tl = &Nilng_nodeListNode;
  }
  return ng_nodeList(h);
}


ng_nodeList merge(ng_nodeList& x, ng_nodeList& y, ng_nodeComparator f)
{
  ng_nodeListNode* a = x.P;
  ng_nodeListNode* b = y.P;

  if (a == &Nilng_nodeListNode)
  {
    if (b == &Nilng_nodeListNode)
      return ng_nodeList(&Nilng_nodeListNode);
    else
      return copy(y);
  }
  else if (b == &Nilng_nodeListNode)
    return copy(x);

  ng_nodeListNode* h = new ng_nodeListNode;
  h->ref = 1;
  if ((*f)(a->hd, b->hd) <= 0)
  {
    h->hd = a->hd;
    a = a->tl;
  }
  else
  {
    h->hd = b->hd;
    b = b->tl;
  }

  ng_nodeListNode* r = h;

  for(;;)
  {
    if (a == &Nilng_nodeListNode)
    {
      while (b != &Nilng_nodeListNode)
      {
        ng_nodeListNode* n = new ng_nodeListNode;
        n->ref = 1;
        n->hd = b->hd;
        r->tl = n;
        r = n;
        b = b->tl;
      }
      r->tl = &Nilng_nodeListNode;
      return ng_nodeList(h);
    }
    else if (b == &Nilng_nodeListNode)
    {
      while (a != &Nilng_nodeListNode)
      {
        ng_nodeListNode* n = new ng_nodeListNode;
        n->ref = 1;
        n->hd = a->hd;
        r->tl = n;
        r = n;
        a = a->tl;
      }
      r->tl = &Nilng_nodeListNode;
      return ng_nodeList(h);
    }
    else if ((*f)(a->hd, b->hd) <= 0)
    {
      ng_nodeListNode* n = new ng_nodeListNode;
      n->ref = 1;
      n->hd = a->hd;
      r->tl = n;
      r = n;
      a = a->tl;
    }
    else
    {
      ng_nodeListNode* n = new ng_nodeListNode;
      n->ref = 1;
      n->hd = b->hd;
      r->tl = n;
      r = n;
      b = b->tl;
    }
  }
}

void ng_nodeList::sort(ng_nodeComparator f)
{
  // strategy: place runs in queue, merge runs until done
  // This is often very fast

  if (P == &Nilng_nodeListNode || P->tl == &Nilng_nodeListNode)
    return;

  int qlen = 250;   // guess a good queue size, realloc if necessary

  ng_nodeListNode** queue = (ng_nodeListNode**)malloc(qlen * sizeof(ng_nodeListNode*));

  ng_nodeListNode* h = P;
  ng_nodeListNode* a = h;
  ng_nodeListNode* b = a->tl;
  int qin = 0;

  while (b != &Nilng_nodeListNode)
  {
    if ((*f)(a->hd, b->hd) > 0)
    {
      if (h == a)               // minor optimization: ensure runlen >= 2
      {
        h = b;
        a->tl = b->tl;
        b->tl = a;
        b = a->tl;
      }
      else
      {
        if (qin >= qlen)
        {
          qlen *= 2;
          queue = (ng_nodeListNode**)realloc(queue, qlen * sizeof(ng_nodeListNode*));
        }
        queue[qin++] = h;
        a->tl = &Nilng_nodeListNode;
        h = a = b;
        b = b->tl;
      }
    }
    else
    {
      a = b;
      b = b->tl;
    }
  }

  int count = qin;
  queue[qin] = h;
  if (++qin >= qlen) qin = 0;
  int qout = 0;

  while (count-- > 0)
  {
    a = queue[qout];
    if (++qout >= qlen) qout = 0;
    b = queue[qout];
    if (++qout >= qlen) qout = 0;

    if ((*f)(a->hd, b->hd) <= 0)
    {
      h = a;
      a = a->tl;
    }
    else
    {
      h = b;
      b = b->tl;
    }
    queue[qin] = h;
    if (++qin >= qlen) qin = 0;

    for (;;)
    {
      if (a == &Nilng_nodeListNode)
      {
        h->tl = b;
        break;
      }
      else if (b == &Nilng_nodeListNode)
      {
        h->tl = a;
        break;
      }
      else if ((*f)(a->hd, b->hd) <= 0)
      {
        h->tl = a;
        h = a;
        a = a->tl;
      }
      else
      {
        h->tl = b;
        h = b;
        b = b->tl;
      }
    }
  }
  P = queue[qout];
  free(queue);
}
    
int ng_nodeList::list_length()
{
  ng_nodeListNode* fast = P;
  if (fast == &Nilng_nodeListNode)
    return 0;

  ng_nodeListNode* slow = fast->tl;
  if (slow == &Nilng_nodeListNode)
    return 1;

  fast = slow->tl;
  int n = 2;

  for (;;)
  {
    if (fast == &Nilng_nodeListNode)
      return n;
    else if (fast->tl == &Nilng_nodeListNode)
      return n+1;
    else if (fast == slow)
      return -1;
    else
    {
      n += 2;
      fast = fast->tl->tl;
      slow = slow->tl;
    }
  }
}

void ng_nodeList::error(const char* msg)
{
  (*lib_error_handler)("List", msg);
}

int ng_nodeList::OK()
{
  int v = P != 0;               // have a node
  // check that all nodes OK, even if circular:

  ng_nodeListNode* fast = P;
  if (fast != &Nilng_nodeListNode)
  {
    v &= fast->ref != 0;
    ng_nodeListNode* slow = fast->tl;
    v &= slow->ref != 0;
    if (v && slow != &Nilng_nodeListNode)
    {
      fast = slow->tl;
      v &= fast->ref != 0;
      while (v)
      {
        if (fast == &Nilng_nodeListNode)
          break;
        else if (fast->tl == &Nilng_nodeListNode)
          break;
        else if (fast == slow)
          break;
        else
        {
          v &= fast->ref != 0 && slow->ref != 0;
          fast = fast->tl->tl;
          slow = slow->tl;
        }
      }
    }
  }
  if (!v) error ("invariant failure");
  return v;
}
