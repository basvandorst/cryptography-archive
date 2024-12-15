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
#include "NewsTree.List.h"

NewsTreeListNode NilNewsTreeListNode;

class init_NilNewsTreeListNode
{
public:
  init_NilNewsTreeListNode() 
  {
    NilNewsTreeListNode.tl = &NilNewsTreeListNode;
    NilNewsTreeListNode.ref = -1;
  }
};

static init_NilNewsTreeListNode NilNewsTreeListNode_initializer;

NewsTreeList& NewsTreeList::operator = (NewsTreeList& a)
{
  reference(a.P);
  dereference(P);
  P = a.P;
  return *this;
}

NewsTree NewsTreeList::pop()
{
  NewsTree res = P->hd;
  NewsTreeListNode* tail = P->tl;
  reference(tail);
  dereference(P);
  P = tail;
  return res;
}

void NewsTreeList::set_tail(NewsTreeList& a)
{
  reference(a.P);
  dereference(P->tl);
  P->tl = a.P;
}

NewsTreeList NewsTreeList::nth(int n)
{
  for (NewsTreeListNode* p = P; n-- > 0; p = p->tl);
  reference(p);
  return NewsTreeList(p);
}

NewsTreeList NewsTreeList::last()
{
  NewsTreeListNode* p = P;
  if (p != &NilNewsTreeListNode) while (p->tl != &NilNewsTreeListNode) p = p->tl;
  reference(p);
  return NewsTreeList(p);
}

void NewsTreeList::append(NewsTreeList& l)
{
  NewsTreeListNode* p = P;
  NewsTreeListNode* a = l.P;
  reference(a);
  if (p != &NilNewsTreeListNode)
  {
    while (p->tl != &NilNewsTreeListNode) p = p->tl;
    p->tl = a;
  }
  else
    P = a;
}

int NewsTreeList::length()
{
  int l = 0;
  for (NewsTreeListNode* p = P; p != &NilNewsTreeListNode; p = p->tl) ++l;
  return l;
}

NewsTree&  NewsTreeList::operator [] (int n)
{
  for (NewsTreeListNode* p = P; n-- > 0; p = p->tl);
  return (p->hd);
}

int operator == (NewsTreeList& x, NewsTreeList& y)
{
  NewsTreeListNode* a = x.P;
  NewsTreeListNode* b = y.P;
  
  for (;;)
  {
    if (a == &NilNewsTreeListNode)
      return b == &NilNewsTreeListNode;
    else if (b == &NilNewsTreeListNode)
      return 0;
    else if (NewsTreeEQ(a->hd, b->hd))
    {
      a = a->tl;
      b = b->tl;
    }
    else
      return 0;
  }
}


void NewsTreeList::apply(NewsTreeProcedure f)
{
  for(NewsTreeListNode* p = P; p != &NilNewsTreeListNode; p = p->tl)
    (*f)((p->hd));
}

void NewsTreeList::subst(NewsTree  old, NewsTree  repl)
{
  for(NewsTreeListNode* p = P; p != &NilNewsTreeListNode; p = p->tl)
    if (NewsTreeEQ(p->hd, old))
      p->hd = repl;
}

NewsTree NewsTreeList::reduce(NewsTreeCombiner f, NewsTree  base)
{
  NewsTree r = base;
  for(NewsTreeListNode* p = P; p != &NilNewsTreeListNode; p = p->tl)
    r = (*f)(r, (p->hd));
  return r;
}

int NewsTreeList::position(NewsTree  targ)
{
  int l = 0;
  NewsTreeListNode* p = P;
  for (;;)
  {
    if (p == &NilNewsTreeListNode)
      return -1;
    else if (NewsTreeEQ(p->hd, targ))
      return l;
    else
    {
      ++l;
      p = p->tl;
    }
  }
}

int NewsTreeList::contains(NewsTree  targ)
{
  NewsTreeListNode* p = P;
  for (;;)
  {
    if (p == &NilNewsTreeListNode)
      return 0;
    else if (NewsTreeEQ(p->hd, targ))
      return 1;
    else
      p = p->tl;
  }
}

NewsTreeList NewsTreeList::find(NewsTree  targ)
{
  NewsTreeListNode* p = P;
  while (p != &NilNewsTreeListNode && !NewsTreeEQ(p->hd, targ))
    p=p->tl;
  reference(p);
  return NewsTreeList(p);
}

Pix NewsTreeList::seek(NewsTree  targ)
{
  NewsTreeListNode* p = P; 
  for (;;)
  {
    if (p == &NilNewsTreeListNode)
      return 0;
    else if (NewsTreeEQ(p->hd, targ))
      return Pix(p);
    else
      p = p->tl;
  }
}

int NewsTreeList::owns(Pix i)
{
  NewsTreeListNode* p = P; 
  for (;;)
  {
    if (p == &NilNewsTreeListNode)
      return 0;
    else if (Pix(p) == i)
      return 1;
    else
      p = p->tl;
  }
}

NewsTreeList NewsTreeList::find(NewsTreeList& target)
{
  NewsTreeListNode* targ = target.P;
  if (targ == &NilNewsTreeListNode)
    return NewsTreeList(targ);

  NewsTreeListNode* p = P;
  while (p != &NilNewsTreeListNode)
  {
    if (NewsTreeEQ(p->hd, targ->hd))
    {
      NewsTreeListNode* a = p->tl;
      NewsTreeListNode* t = targ->tl;
      for(;;)
      {
        if (t == &NilNewsTreeListNode)
        {
          reference(p);
          return NewsTreeList(p);
        }
        else if (a == &NilNewsTreeListNode || !NewsTreeEQ(a->hd, t->hd))
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
  return NewsTreeList(&NilNewsTreeListNode);
}

int NewsTreeList::contains(NewsTreeList& target)
{
  NewsTreeListNode* targ = target.P;
  if (targ == &NilNewsTreeListNode)
    return 0;

  NewsTreeListNode* p = P;
  while (p != &NilNewsTreeListNode)
  {
    if (NewsTreeEQ(p->hd, targ->hd))
    {
      NewsTreeListNode* a = p->tl;
      NewsTreeListNode* t = targ->tl;
      for(;;)
      {
        if (t == &NilNewsTreeListNode)
          return 1;
        else if (a == &NilNewsTreeListNode || !NewsTreeEQ(a->hd, t->hd))
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

void NewsTreeList::del(NewsTree  targ)
{
  NewsTreeListNode* h = P;

  for (;;)
  {
    if (h == &NilNewsTreeListNode)
    {
      P = h;
      return;
    }
    else if (NewsTreeEQ(h->hd, targ))
    {
      NewsTreeListNode* nxt = h->tl;
      reference(nxt);
      dereference(h);
      h = nxt;
    }
    else
      break;
  }

  NewsTreeListNode* trail = h;
  NewsTreeListNode* p = h->tl;
  while (p != &NilNewsTreeListNode)
  {
    if (NewsTreeEQ(p->hd, targ))
    {
      NewsTreeListNode* nxt = p->tl;
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

void NewsTreeList::del(NewsTreePredicate f)
{
  NewsTreeListNode* h = P;
  for (;;)
  {
    if (h == &NilNewsTreeListNode)
    {
      P = h;
      return;
    }
    else if ((*f)(h->hd))
    {
      NewsTreeListNode* nxt = h->tl;
      reference(nxt);
      dereference(h);
      h = nxt;
    }
    else
      break;
  }

  NewsTreeListNode* trail = h;
  NewsTreeListNode* p = h->tl;
  while (p != &NilNewsTreeListNode)
  {
    if ((*f)(p->hd))
    {
      NewsTreeListNode* nxt = p->tl;
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

void NewsTreeList::select(NewsTreePredicate f)
{
  NewsTreeListNode* h = P;
  for (;;)
  {
    if (h == &NilNewsTreeListNode)
    {
      P = h;
      return;
    }
    else if (!(*f)(h->hd))
    {
      NewsTreeListNode* nxt = h->tl;
      reference(nxt);
      dereference(h);
      h = nxt;
    }
    else
      break;
  }
  NewsTreeListNode* trail = h;
  NewsTreeListNode* p = h->tl;
  while (p != &NilNewsTreeListNode)
  {
    if (!(*f)(p->hd))
    {
      NewsTreeListNode* nxt = p->tl;
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

void NewsTreeList::reverse()
{
  NewsTreeListNode* l = &NilNewsTreeListNode;
  NewsTreeListNode* p = P; 
  while (p != &NilNewsTreeListNode)
  {
    NewsTreeListNode* nxt = p->tl;
    p->tl = l;
    l = p;
    p = nxt;
  }
  P = l;
}


NewsTreeList copy(NewsTreeList& x)
{
  NewsTreeListNode* a = x.P;
  if (a == &NilNewsTreeListNode)
    return NewsTreeList(a);
  else
  {
    NewsTreeListNode* h = newNewsTreeListNode(a->hd);
    NewsTreeListNode* trail = h;
    for(a = a->tl; a != &NilNewsTreeListNode; a = a->tl)
    {
      NewsTreeListNode* n = newNewsTreeListNode(a->hd);
      trail->tl = n;
      trail = n;
    }
    trail->tl = &NilNewsTreeListNode;
    return NewsTreeList(h);
  }
}


NewsTreeList subst(NewsTree  old, NewsTree  repl, NewsTreeList& x)
{
  NewsTreeListNode* a = x.P;
  if (a == &NilNewsTreeListNode)
    return NewsTreeList(a);
  else
  {
    NewsTreeListNode* h = new NewsTreeListNode;
    h->ref = 1;
    if (NewsTreeEQ(a->hd, old))
      h->hd = repl;
    else
      h->hd = a->hd;
    NewsTreeListNode* trail = h;
    for(a = a->tl; a != &NilNewsTreeListNode; a = a->tl)
    {
      NewsTreeListNode* n = new NewsTreeListNode;
      n->ref = 1;
      if (NewsTreeEQ(a->hd, old))
        n->hd = repl;
      else
        n->hd = a->hd;
      trail->tl = n;
      trail = n;
    }
    trail->tl = &NilNewsTreeListNode;
    return NewsTreeList(h);
  }
}

NewsTreeList combine(NewsTreeCombiner f, NewsTreeList& x, NewsTreeList& y)
{
  NewsTreeListNode* a = x.P;
  NewsTreeListNode* b = y.P;
  if (a == &NilNewsTreeListNode || b == &NilNewsTreeListNode)
    return NewsTreeList(&NilNewsTreeListNode);
  else
  {
    NewsTreeListNode* h = newNewsTreeListNode((*f)(a->hd, b->hd));
    NewsTreeListNode* trail = h;
    a = a->tl;
    b = b->tl;
    while (a != &NilNewsTreeListNode && b != &NilNewsTreeListNode)
    {
      NewsTreeListNode* n = newNewsTreeListNode((*f)(a->hd, b->hd));
      trail->tl = n;
      trail = n;
      a = a->tl;
      b = b->tl;
    }
    trail->tl = &NilNewsTreeListNode;
    return NewsTreeList(h);
  }
}

NewsTreeList reverse(NewsTreeList& x)
{
  NewsTreeListNode* a = x.P;
  if (a == &NilNewsTreeListNode)
    return NewsTreeList(a);
  else
  {
    NewsTreeListNode* l = newNewsTreeListNode(a->hd);
    l->tl = &NilNewsTreeListNode;
    for(a = a->tl; a != &NilNewsTreeListNode; a = a->tl)
    {
      NewsTreeListNode* n = newNewsTreeListNode(a->hd);
      n->tl = l;
      l = n;
    }
    return NewsTreeList(l);
  }
}

NewsTreeList append(NewsTreeList& x, NewsTreeList& y)
{
  NewsTreeListNode* a = x.P;
  NewsTreeListNode* b = y.P;
  reference(b);
  if (a != &NilNewsTreeListNode)
  {
    NewsTreeListNode* h = newNewsTreeListNode(a->hd);
    NewsTreeListNode* trail = h;
    for(a = a->tl; a != &NilNewsTreeListNode; a = a->tl)
    {
      NewsTreeListNode* n = newNewsTreeListNode(a->hd);
      trail->tl = n;
      trail = n;
    }
    trail->tl = b;
    return NewsTreeList(h);
  }
  else
    return NewsTreeList(b);
}

void NewsTreeList::prepend(NewsTreeList& y)
{
  NewsTreeListNode* b = y.P;
  if (b != &NilNewsTreeListNode)
  {
    NewsTreeListNode* h = newNewsTreeListNode(b->hd);
    NewsTreeListNode* trail = h;
    for(b = b->tl; b != &NilNewsTreeListNode; b = b->tl)
    {
      NewsTreeListNode* n = newNewsTreeListNode(b->hd);
      trail->tl = n;
      trail = n;
    }
    trail->tl = P;
    P = h;
  }
}

NewsTreeList concat(NewsTreeList& x, NewsTreeList& y)
{
  NewsTreeListNode* a = x.P;
  NewsTreeListNode* b = y.P;
  if (a != &NilNewsTreeListNode)
  {
    NewsTreeListNode* h = newNewsTreeListNode(a->hd);
    NewsTreeListNode* trail = h;
    for(a = a->tl; a != &NilNewsTreeListNode; a = a->tl)
    {
      NewsTreeListNode* n = newNewsTreeListNode(a->hd);
      trail->tl = n;
      trail = n;
    };
    for(;b != &NilNewsTreeListNode; b = b->tl)
    {
      NewsTreeListNode* n = newNewsTreeListNode(b->hd);
      trail->tl = n;
      trail = n;
    }
    trail->tl = &NilNewsTreeListNode;
    return NewsTreeList(h);
  }
  else if (b != &NilNewsTreeListNode)
  {
    NewsTreeListNode* h = newNewsTreeListNode(b->hd);
    NewsTreeListNode* trail = h;
    for(b = b->tl; b != &NilNewsTreeListNode; b = b->tl)
    {
      NewsTreeListNode* n = newNewsTreeListNode(b->hd);
      trail->tl = n;
      trail = n;
    }
    trail->tl = &NilNewsTreeListNode;
    return NewsTreeList(h);
  }
  else
    return NewsTreeList(&NilNewsTreeListNode);
}

NewsTreeList select(NewsTreePredicate f, NewsTreeList& x)
{
  NewsTreeListNode* a = x.P;
  NewsTreeListNode* h = &NilNewsTreeListNode;
  while (a != &NilNewsTreeListNode)
  {
    if ((*f)(a->hd))
    {
      h = newNewsTreeListNode(a->hd);
      NewsTreeListNode* trail = h;
      for(a = a->tl; a != &NilNewsTreeListNode; a = a->tl)
      {
        if ((*f)(a->hd))
        {
          NewsTreeListNode* n = newNewsTreeListNode(a->hd);
          trail->tl = n;
          trail = n;
        }
      }
      trail->tl = &NilNewsTreeListNode;
      break;
    }
    else
      a = a->tl;
  }
  return NewsTreeList(h);
}

NewsTreeList remove(NewsTreePredicate f, NewsTreeList& x)
{
  NewsTreeListNode* a = x.P;
  NewsTreeListNode* h = &NilNewsTreeListNode;
  while (a != &NilNewsTreeListNode)
  {
    if (!(*f)(a->hd))
    {
      h = newNewsTreeListNode(a->hd);
      NewsTreeListNode* trail = h;
      for(a = a->tl; a != &NilNewsTreeListNode; a = a->tl)
      {
        if (!(*f)(a->hd))
        {
          NewsTreeListNode* n = newNewsTreeListNode(a->hd);
          trail->tl = n;
          trail = n;
        }
      }
      trail->tl = &NilNewsTreeListNode;
      break;
    }
    else
      a = a->tl;
  }
  return NewsTreeList(h);
}

NewsTreeList remove(NewsTree  targ, NewsTreeList& x)
{
  NewsTreeListNode* a = x.P;
  NewsTreeListNode* h = &NilNewsTreeListNode;
  while (a != &NilNewsTreeListNode)
  {
    if (!(NewsTreeEQ(a->hd, targ)))
    {
      h = newNewsTreeListNode(a->hd);
      NewsTreeListNode* trail = h;
      for(a = a->tl; a != &NilNewsTreeListNode; a = a->tl)
      {
        if (!NewsTreeEQ(a->hd, targ))
        {
          NewsTreeListNode* n = newNewsTreeListNode(a->hd);
          trail->tl = n;
          trail = n;
        }
      }
      trail->tl = &NilNewsTreeListNode;
      break;
    }
    else
      a = a->tl;
  }
  return NewsTreeList(h);
}

NewsTreeList map(NewsTreeMapper f, NewsTreeList& x)
{
  NewsTreeListNode* a = x.P;
  NewsTreeListNode* h = &NilNewsTreeListNode;
  if (a != &NilNewsTreeListNode)
  {
    h = newNewsTreeListNode((*f)(a->hd));
    NewsTreeListNode* trail = h;
    for(a = a->tl; a != &NilNewsTreeListNode; a = a->tl)
    {
      NewsTreeListNode* n = newNewsTreeListNode((*f)(a->hd));
      trail->tl = n;
      trail = n;
    }
    trail->tl = &NilNewsTreeListNode;
  }
  return NewsTreeList(h);
}


NewsTreeList merge(NewsTreeList& x, NewsTreeList& y, NewsTreeComparator f)
{
  NewsTreeListNode* a = x.P;
  NewsTreeListNode* b = y.P;

  if (a == &NilNewsTreeListNode)
  {
    if (b == &NilNewsTreeListNode)
      return NewsTreeList(&NilNewsTreeListNode);
    else
      return copy(y);
  }
  else if (b == &NilNewsTreeListNode)
    return copy(x);

  NewsTreeListNode* h = new NewsTreeListNode;
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

  NewsTreeListNode* r = h;

  for(;;)
  {
    if (a == &NilNewsTreeListNode)
    {
      while (b != &NilNewsTreeListNode)
      {
        NewsTreeListNode* n = new NewsTreeListNode;
        n->ref = 1;
        n->hd = b->hd;
        r->tl = n;
        r = n;
        b = b->tl;
      }
      r->tl = &NilNewsTreeListNode;
      return NewsTreeList(h);
    }
    else if (b == &NilNewsTreeListNode)
    {
      while (a != &NilNewsTreeListNode)
      {
        NewsTreeListNode* n = new NewsTreeListNode;
        n->ref = 1;
        n->hd = a->hd;
        r->tl = n;
        r = n;
        a = a->tl;
      }
      r->tl = &NilNewsTreeListNode;
      return NewsTreeList(h);
    }
    else if ((*f)(a->hd, b->hd) <= 0)
    {
      NewsTreeListNode* n = new NewsTreeListNode;
      n->ref = 1;
      n->hd = a->hd;
      r->tl = n;
      r = n;
      a = a->tl;
    }
    else
    {
      NewsTreeListNode* n = new NewsTreeListNode;
      n->ref = 1;
      n->hd = b->hd;
      r->tl = n;
      r = n;
      b = b->tl;
    }
  }
}

void NewsTreeList::sort(NewsTreeComparator f)
{
  // strategy: place runs in queue, merge runs until done
  // This is often very fast

  if (P == &NilNewsTreeListNode || P->tl == &NilNewsTreeListNode)
    return;

  int qlen = 250;   // guess a good queue size, realloc if necessary

  NewsTreeListNode** queue = (NewsTreeListNode**)malloc(qlen * sizeof(NewsTreeListNode*));

  NewsTreeListNode* h = P;
  NewsTreeListNode* a = h;
  NewsTreeListNode* b = a->tl;
  int qin = 0;

  while (b != &NilNewsTreeListNode)
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
          queue = (NewsTreeListNode**)realloc(queue, qlen * sizeof(NewsTreeListNode*));
        }
        queue[qin++] = h;
        a->tl = &NilNewsTreeListNode;
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
      if (a == &NilNewsTreeListNode)
      {
        h->tl = b;
        break;
      }
      else if (b == &NilNewsTreeListNode)
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
    
int NewsTreeList::list_length()
{
  NewsTreeListNode* fast = P;
  if (fast == &NilNewsTreeListNode)
    return 0;

  NewsTreeListNode* slow = fast->tl;
  if (slow == &NilNewsTreeListNode)
    return 1;

  fast = slow->tl;
  int n = 2;

  for (;;)
  {
    if (fast == &NilNewsTreeListNode)
      return n;
    else if (fast->tl == &NilNewsTreeListNode)
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

void NewsTreeList::error(const char* msg)
{
  (*lib_error_handler)("List", msg);
}

int NewsTreeList::OK()
{
  int v = P != 0;               // have a node
  // check that all nodes OK, even if circular:

  NewsTreeListNode* fast = P;
  if (fast != &NilNewsTreeListNode)
  {
    v &= fast->ref != 0;
    NewsTreeListNode* slow = fast->tl;
    v &= slow->ref != 0;
    if (v && slow != &NilNewsTreeListNode)
    {
      fast = slow->tl;
      v &= fast->ref != 0;
      while (v)
      {
        if (fast == &NilNewsTreeListNode)
          break;
        else if (fast->tl == &NilNewsTreeListNode)
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
