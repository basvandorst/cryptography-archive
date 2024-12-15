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
#include "NewsTreeList.h"

nt_nodeListNode Nilnt_nodeListNode;

class init_Nilnt_nodeListNode
{
public:
  init_Nilnt_nodeListNode() 
  {
    Nilnt_nodeListNode.tl = &Nilnt_nodeListNode;
    Nilnt_nodeListNode.ref = -1;
  }
};

static init_Nilnt_nodeListNode Nilnt_nodeListNode_initializer;

nt_nodeList& nt_nodeList::operator = (nt_nodeList& a)
{
  reference(a.P);
  dereference(P);
  P = a.P;
  return *this;
}

nt_node nt_nodeList::pop()
{
  nt_node res = P->hd;
  nt_nodeListNode* tail = P->tl;
  reference(tail);
  dereference(P);
  P = tail;
  return res;
}

void nt_nodeList::set_tail(nt_nodeList& a)
{
  reference(a.P);
  dereference(P->tl);
  P->tl = a.P;
}

nt_nodeList nt_nodeList::nth(int n)
{
  for (nt_nodeListNode* p = P; n-- > 0; p = p->tl);
  reference(p);
  return nt_nodeList(p);
}

nt_nodeList nt_nodeList::last()
{
  nt_nodeListNode* p = P;
  if (p != &Nilnt_nodeListNode) while (p->tl != &Nilnt_nodeListNode) p = p->tl;
  reference(p);
  return nt_nodeList(p);
}

void nt_nodeList::append(nt_nodeList& l)
{
  nt_nodeListNode* p = P;
  nt_nodeListNode* a = l.P;
  reference(a);
  if (p != &Nilnt_nodeListNode)
  {
    while (p->tl != &Nilnt_nodeListNode) p = p->tl;
    p->tl = a;
  }
  else
    P = a;
}

int nt_nodeList::length()
{
  int l = 0;
  for (nt_nodeListNode* p = P; p != &Nilnt_nodeListNode; p = p->tl) ++l;
  return l;
}

nt_node&  nt_nodeList::operator [] (int n)
{
  for (nt_nodeListNode* p = P; n-- > 0; p = p->tl);
  return (p->hd);
}

int operator == (nt_nodeList& x, nt_nodeList& y)
{
  nt_nodeListNode* a = x.P;
  nt_nodeListNode* b = y.P;
  
  for (;;)
  {
    if (a == &Nilnt_nodeListNode)
      return b == &Nilnt_nodeListNode;
    else if (b == &Nilnt_nodeListNode)
      return 0;
    else if (nt_nodeEQ(a->hd, b->hd))
    {
      a = a->tl;
      b = b->tl;
    }
    else
      return 0;
  }
}


void nt_nodeList::apply(nt_nodeProcedure f)
{
  for(nt_nodeListNode* p = P; p != &Nilnt_nodeListNode; p = p->tl)
    (*f)((p->hd));
}

void nt_nodeList::subst(nt_node  old, nt_node  repl)
{
  for(nt_nodeListNode* p = P; p != &Nilnt_nodeListNode; p = p->tl)
    if (nt_nodeEQ(p->hd, old))
      p->hd = repl;
}

nt_node nt_nodeList::reduce(nt_nodeCombiner f, nt_node  base)
{
  nt_node r = base;
  for(nt_nodeListNode* p = P; p != &Nilnt_nodeListNode; p = p->tl)
    r = (*f)(r, (p->hd));
  return r;
}

int nt_nodeList::position(nt_node  targ)
{
  int l = 0;
  nt_nodeListNode* p = P;
  for (;;)
  {
    if (p == &Nilnt_nodeListNode)
      return -1;
    else if (nt_nodeEQ(p->hd, targ))
      return l;
    else
    {
      ++l;
      p = p->tl;
    }
  }
}

int nt_nodeList::contains(nt_node  targ)
{
  nt_nodeListNode* p = P;
  for (;;)
  {
    if (p == &Nilnt_nodeListNode)
      return 0;
    else if (nt_nodeEQ(p->hd, targ))
      return 1;
    else
      p = p->tl;
  }
}

nt_nodeList nt_nodeList::find(nt_node  targ)
{
  nt_nodeListNode* p = P;
  while (p != &Nilnt_nodeListNode && !nt_nodeEQ(p->hd, targ))
    p=p->tl;
  reference(p);
  return nt_nodeList(p);
}

Pix nt_nodeList::seek(nt_node  targ)
{
  nt_nodeListNode* p = P; 
  for (;;)
  {
    if (p == &Nilnt_nodeListNode)
      return 0;
    else if (nt_nodeEQ(p->hd, targ))
      return Pix(p);
    else
      p = p->tl;
  }
}

int nt_nodeList::owns(Pix i)
{
  nt_nodeListNode* p = P; 
  for (;;)
  {
    if (p == &Nilnt_nodeListNode)
      return 0;
    else if (Pix(p) == i)
      return 1;
    else
      p = p->tl;
  }
}

nt_nodeList nt_nodeList::find(nt_nodeList& target)
{
  nt_nodeListNode* targ = target.P;
  if (targ == &Nilnt_nodeListNode)
    return nt_nodeList(targ);

  nt_nodeListNode* p = P;
  while (p != &Nilnt_nodeListNode)
  {
    if (nt_nodeEQ(p->hd, targ->hd))
    {
      nt_nodeListNode* a = p->tl;
      nt_nodeListNode* t = targ->tl;
      for(;;)
      {
        if (t == &Nilnt_nodeListNode)
        {
          reference(p);
          return nt_nodeList(p);
        }
        else if (a == &Nilnt_nodeListNode || !nt_nodeEQ(a->hd, t->hd))
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
  return nt_nodeList(&Nilnt_nodeListNode);
}

int nt_nodeList::contains(nt_nodeList& target)
{
  nt_nodeListNode* targ = target.P;
  if (targ == &Nilnt_nodeListNode)
    return 0;

  nt_nodeListNode* p = P;
  while (p != &Nilnt_nodeListNode)
  {
    if (nt_nodeEQ(p->hd, targ->hd))
    {
      nt_nodeListNode* a = p->tl;
      nt_nodeListNode* t = targ->tl;
      for(;;)
      {
        if (t == &Nilnt_nodeListNode)
          return 1;
        else if (a == &Nilnt_nodeListNode || !nt_nodeEQ(a->hd, t->hd))
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

void nt_nodeList::del(nt_node  targ)
{
  nt_nodeListNode* h = P;

  for (;;)
  {
    if (h == &Nilnt_nodeListNode)
    {
      P = h;
      return;
    }
    else if (nt_nodeEQ(h->hd, targ))
    {
      nt_nodeListNode* nxt = h->tl;
      reference(nxt);
      dereference(h);
      h = nxt;
    }
    else
      break;
  }

  nt_nodeListNode* trail = h;
  nt_nodeListNode* p = h->tl;
  while (p != &Nilnt_nodeListNode)
  {
    if (nt_nodeEQ(p->hd, targ))
    {
      nt_nodeListNode* nxt = p->tl;
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

void nt_nodeList::del(nt_nodePredicate f)
{
  nt_nodeListNode* h = P;
  for (;;)
  {
    if (h == &Nilnt_nodeListNode)
    {
      P = h;
      return;
    }
    else if ((*f)(h->hd))
    {
      nt_nodeListNode* nxt = h->tl;
      reference(nxt);
      dereference(h);
      h = nxt;
    }
    else
      break;
  }

  nt_nodeListNode* trail = h;
  nt_nodeListNode* p = h->tl;
  while (p != &Nilnt_nodeListNode)
  {
    if ((*f)(p->hd))
    {
      nt_nodeListNode* nxt = p->tl;
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

void nt_nodeList::select(nt_nodePredicate f)
{
  nt_nodeListNode* h = P;
  for (;;)
  {
    if (h == &Nilnt_nodeListNode)
    {
      P = h;
      return;
    }
    else if (!(*f)(h->hd))
    {
      nt_nodeListNode* nxt = h->tl;
      reference(nxt);
      dereference(h);
      h = nxt;
    }
    else
      break;
  }
  nt_nodeListNode* trail = h;
  nt_nodeListNode* p = h->tl;
  while (p != &Nilnt_nodeListNode)
  {
    if (!(*f)(p->hd))
    {
      nt_nodeListNode* nxt = p->tl;
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

void nt_nodeList::reverse()
{
  nt_nodeListNode* l = &Nilnt_nodeListNode;
  nt_nodeListNode* p = P; 
  while (p != &Nilnt_nodeListNode)
  {
    nt_nodeListNode* nxt = p->tl;
    p->tl = l;
    l = p;
    p = nxt;
  }
  P = l;
}


nt_nodeList copy(nt_nodeList& x)
{
  nt_nodeListNode* a = x.P;
  if (a == &Nilnt_nodeListNode)
    return nt_nodeList(a);
  else
  {
    nt_nodeListNode* h = newnt_nodeListNode(a->hd);
    nt_nodeListNode* trail = h;
    for(a = a->tl; a != &Nilnt_nodeListNode; a = a->tl)
    {
      nt_nodeListNode* n = newnt_nodeListNode(a->hd);
      trail->tl = n;
      trail = n;
    }
    trail->tl = &Nilnt_nodeListNode;
    return nt_nodeList(h);
  }
}


nt_nodeList subst(nt_node  old, nt_node  repl, nt_nodeList& x)
{
  nt_nodeListNode* a = x.P;
  if (a == &Nilnt_nodeListNode)
    return nt_nodeList(a);
  else
  {
    nt_nodeListNode* h = new nt_nodeListNode;
    h->ref = 1;
    if (nt_nodeEQ(a->hd, old))
      h->hd = repl;
    else
      h->hd = a->hd;
    nt_nodeListNode* trail = h;
    for(a = a->tl; a != &Nilnt_nodeListNode; a = a->tl)
    {
      nt_nodeListNode* n = new nt_nodeListNode;
      n->ref = 1;
      if (nt_nodeEQ(a->hd, old))
        n->hd = repl;
      else
        n->hd = a->hd;
      trail->tl = n;
      trail = n;
    }
    trail->tl = &Nilnt_nodeListNode;
    return nt_nodeList(h);
  }
}

nt_nodeList combine(nt_nodeCombiner f, nt_nodeList& x, nt_nodeList& y)
{
  nt_nodeListNode* a = x.P;
  nt_nodeListNode* b = y.P;
  if (a == &Nilnt_nodeListNode || b == &Nilnt_nodeListNode)
    return nt_nodeList(&Nilnt_nodeListNode);
  else
  {
    nt_nodeListNode* h = newnt_nodeListNode((*f)(a->hd, b->hd));
    nt_nodeListNode* trail = h;
    a = a->tl;
    b = b->tl;
    while (a != &Nilnt_nodeListNode && b != &Nilnt_nodeListNode)
    {
      nt_nodeListNode* n = newnt_nodeListNode((*f)(a->hd, b->hd));
      trail->tl = n;
      trail = n;
      a = a->tl;
      b = b->tl;
    }
    trail->tl = &Nilnt_nodeListNode;
    return nt_nodeList(h);
  }
}

nt_nodeList reverse(nt_nodeList& x)
{
  nt_nodeListNode* a = x.P;
  if (a == &Nilnt_nodeListNode)
    return nt_nodeList(a);
  else
  {
    nt_nodeListNode* l = newnt_nodeListNode(a->hd);
    l->tl = &Nilnt_nodeListNode;
    for(a = a->tl; a != &Nilnt_nodeListNode; a = a->tl)
    {
      nt_nodeListNode* n = newnt_nodeListNode(a->hd);
      n->tl = l;
      l = n;
    }
    return nt_nodeList(l);
  }
}

nt_nodeList append(nt_nodeList& x, nt_nodeList& y)
{
  nt_nodeListNode* a = x.P;
  nt_nodeListNode* b = y.P;
  reference(b);
  if (a != &Nilnt_nodeListNode)
  {
    nt_nodeListNode* h = newnt_nodeListNode(a->hd);
    nt_nodeListNode* trail = h;
    for(a = a->tl; a != &Nilnt_nodeListNode; a = a->tl)
    {
      nt_nodeListNode* n = newnt_nodeListNode(a->hd);
      trail->tl = n;
      trail = n;
    }
    trail->tl = b;
    return nt_nodeList(h);
  }
  else
    return nt_nodeList(b);
}

void nt_nodeList::prepend(nt_nodeList& y)
{
  nt_nodeListNode* b = y.P;
  if (b != &Nilnt_nodeListNode)
  {
    nt_nodeListNode* h = newnt_nodeListNode(b->hd);
    nt_nodeListNode* trail = h;
    for(b = b->tl; b != &Nilnt_nodeListNode; b = b->tl)
    {
      nt_nodeListNode* n = newnt_nodeListNode(b->hd);
      trail->tl = n;
      trail = n;
    }
    trail->tl = P;
    P = h;
  }
}

nt_nodeList concat(nt_nodeList& x, nt_nodeList& y)
{
  nt_nodeListNode* a = x.P;
  nt_nodeListNode* b = y.P;
  if (a != &Nilnt_nodeListNode)
  {
    nt_nodeListNode* h = newnt_nodeListNode(a->hd);
    nt_nodeListNode* trail = h;
    for(a = a->tl; a != &Nilnt_nodeListNode; a = a->tl)
    {
      nt_nodeListNode* n = newnt_nodeListNode(a->hd);
      trail->tl = n;
      trail = n;
    };
    for(;b != &Nilnt_nodeListNode; b = b->tl)
    {
      nt_nodeListNode* n = newnt_nodeListNode(b->hd);
      trail->tl = n;
      trail = n;
    }
    trail->tl = &Nilnt_nodeListNode;
    return nt_nodeList(h);
  }
  else if (b != &Nilnt_nodeListNode)
  {
    nt_nodeListNode* h = newnt_nodeListNode(b->hd);
    nt_nodeListNode* trail = h;
    for(b = b->tl; b != &Nilnt_nodeListNode; b = b->tl)
    {
      nt_nodeListNode* n = newnt_nodeListNode(b->hd);
      trail->tl = n;
      trail = n;
    }
    trail->tl = &Nilnt_nodeListNode;
    return nt_nodeList(h);
  }
  else
    return nt_nodeList(&Nilnt_nodeListNode);
}

nt_nodeList select(nt_nodePredicate f, nt_nodeList& x)
{
  nt_nodeListNode* a = x.P;
  nt_nodeListNode* h = &Nilnt_nodeListNode;
  while (a != &Nilnt_nodeListNode)
  {
    if ((*f)(a->hd))
    {
      h = newnt_nodeListNode(a->hd);
      nt_nodeListNode* trail = h;
      for(a = a->tl; a != &Nilnt_nodeListNode; a = a->tl)
      {
        if ((*f)(a->hd))
        {
          nt_nodeListNode* n = newnt_nodeListNode(a->hd);
          trail->tl = n;
          trail = n;
        }
      }
      trail->tl = &Nilnt_nodeListNode;
      break;
    }
    else
      a = a->tl;
  }
  return nt_nodeList(h);
}

nt_nodeList remove(nt_nodePredicate f, nt_nodeList& x)
{
  nt_nodeListNode* a = x.P;
  nt_nodeListNode* h = &Nilnt_nodeListNode;
  while (a != &Nilnt_nodeListNode)
  {
    if (!(*f)(a->hd))
    {
      h = newnt_nodeListNode(a->hd);
      nt_nodeListNode* trail = h;
      for(a = a->tl; a != &Nilnt_nodeListNode; a = a->tl)
      {
        if (!(*f)(a->hd))
        {
          nt_nodeListNode* n = newnt_nodeListNode(a->hd);
          trail->tl = n;
          trail = n;
        }
      }
      trail->tl = &Nilnt_nodeListNode;
      break;
    }
    else
      a = a->tl;
  }
  return nt_nodeList(h);
}

nt_nodeList remove(nt_node  targ, nt_nodeList& x)
{
  nt_nodeListNode* a = x.P;
  nt_nodeListNode* h = &Nilnt_nodeListNode;
  while (a != &Nilnt_nodeListNode)
  {
    if (!(nt_nodeEQ(a->hd, targ)))
    {
      h = newnt_nodeListNode(a->hd);
      nt_nodeListNode* trail = h;
      for(a = a->tl; a != &Nilnt_nodeListNode; a = a->tl)
      {
        if (!nt_nodeEQ(a->hd, targ))
        {
          nt_nodeListNode* n = newnt_nodeListNode(a->hd);
          trail->tl = n;
          trail = n;
        }
      }
      trail->tl = &Nilnt_nodeListNode;
      break;
    }
    else
      a = a->tl;
  }
  return nt_nodeList(h);
}

nt_nodeList map(nt_nodeMapper f, nt_nodeList& x)
{
  nt_nodeListNode* a = x.P;
  nt_nodeListNode* h = &Nilnt_nodeListNode;
  if (a != &Nilnt_nodeListNode)
  {
    h = newnt_nodeListNode((*f)(a->hd));
    nt_nodeListNode* trail = h;
    for(a = a->tl; a != &Nilnt_nodeListNode; a = a->tl)
    {
      nt_nodeListNode* n = newnt_nodeListNode((*f)(a->hd));
      trail->tl = n;
      trail = n;
    }
    trail->tl = &Nilnt_nodeListNode;
  }
  return nt_nodeList(h);
}


nt_nodeList merge(nt_nodeList& x, nt_nodeList& y, nt_nodeComparator f)
{
  nt_nodeListNode* a = x.P;
  nt_nodeListNode* b = y.P;

  if (a == &Nilnt_nodeListNode)
  {
    if (b == &Nilnt_nodeListNode)
      return nt_nodeList(&Nilnt_nodeListNode);
    else
      return copy(y);
  }
  else if (b == &Nilnt_nodeListNode)
    return copy(x);

  nt_nodeListNode* h = new nt_nodeListNode;
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

  nt_nodeListNode* r = h;

  for(;;)
  {
    if (a == &Nilnt_nodeListNode)
    {
      while (b != &Nilnt_nodeListNode)
      {
        nt_nodeListNode* n = new nt_nodeListNode;
        n->ref = 1;
        n->hd = b->hd;
        r->tl = n;
        r = n;
        b = b->tl;
      }
      r->tl = &Nilnt_nodeListNode;
      return nt_nodeList(h);
    }
    else if (b == &Nilnt_nodeListNode)
    {
      while (a != &Nilnt_nodeListNode)
      {
        nt_nodeListNode* n = new nt_nodeListNode;
        n->ref = 1;
        n->hd = a->hd;
        r->tl = n;
        r = n;
        a = a->tl;
      }
      r->tl = &Nilnt_nodeListNode;
      return nt_nodeList(h);
    }
    else if ((*f)(a->hd, b->hd) <= 0)
    {
      nt_nodeListNode* n = new nt_nodeListNode;
      n->ref = 1;
      n->hd = a->hd;
      r->tl = n;
      r = n;
      a = a->tl;
    }
    else
    {
      nt_nodeListNode* n = new nt_nodeListNode;
      n->ref = 1;
      n->hd = b->hd;
      r->tl = n;
      r = n;
      b = b->tl;
    }
  }
}

void nt_nodeList::sort(nt_nodeComparator f)
{
  // strategy: place runs in queue, merge runs until done
  // This is often very fast

  if (P == &Nilnt_nodeListNode || P->tl == &Nilnt_nodeListNode)
    return;

  int qlen = 250;   // guess a good queue size, realloc if necessary

  nt_nodeListNode** queue = (nt_nodeListNode**)malloc(qlen * sizeof(nt_nodeListNode*));

  nt_nodeListNode* h = P;
  nt_nodeListNode* a = h;
  nt_nodeListNode* b = a->tl;
  int qin = 0;

  while (b != &Nilnt_nodeListNode)
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
          queue = (nt_nodeListNode**)realloc(queue, qlen * sizeof(nt_nodeListNode*));
        }
        queue[qin++] = h;
        a->tl = &Nilnt_nodeListNode;
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
      if (a == &Nilnt_nodeListNode)
      {
        h->tl = b;
        break;
      }
      else if (b == &Nilnt_nodeListNode)
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
    
int nt_nodeList::list_length()
{
  nt_nodeListNode* fast = P;
  if (fast == &Nilnt_nodeListNode)
    return 0;

  nt_nodeListNode* slow = fast->tl;
  if (slow == &Nilnt_nodeListNode)
    return 1;

  fast = slow->tl;
  int n = 2;

  for (;;)
  {
    if (fast == &Nilnt_nodeListNode)
      return n;
    else if (fast->tl == &Nilnt_nodeListNode)
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

void nt_nodeList::error(const char* msg)
{
  (*lib_error_handler)("List", msg);
}

int nt_nodeList::OK()
{
  int v = P != 0;               // have a node
  // check that all nodes OK, even if circular:

  nt_nodeListNode* fast = P;
  if (fast != &Nilnt_nodeListNode)
  {
    v &= fast->ref != 0;
    nt_nodeListNode* slow = fast->tl;
    v &= slow->ref != 0;
    if (v && slow != &Nilnt_nodeListNode)
    {
      fast = slow->tl;
      v &= fast->ref != 0;
      while (v)
      {
        if (fast == &Nilnt_nodeListNode)
          break;
        else if (fast->tl == &Nilnt_nodeListNode)
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
