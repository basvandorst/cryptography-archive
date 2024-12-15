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


#ifndef _nt_nodeList_h
#ifdef __GNUG__
#pragma interface
#endif
#define _nt_nodeList_h 1

#include <Pix.h>
#include "NewsTreedefs.h"

#ifndef _nt_node_typedefs
#define _nt_node_typedefs 1
typedef void (*nt_nodeProcedure)(nt_node );
typedef nt_node  (*nt_nodeMapper)(nt_node );
typedef nt_node  (*nt_nodeCombiner)(nt_node , nt_node );
typedef int  (*nt_nodePredicate)(nt_node );
typedef int  (*nt_nodeComparator)(nt_node , nt_node );
#endif

struct nt_nodeListNode
{
  nt_nodeListNode*          tl;
  short                 ref;
  nt_node                   hd;
};

extern nt_nodeListNode Nilnt_nodeListNode;

class nt_nodeList
{
protected:
  nt_nodeListNode*          P;

                        nt_nodeList(nt_nodeListNode* p);
public:
                        nt_nodeList();
                        nt_nodeList(nt_node  head);
                        nt_nodeList(nt_node  head, nt_nodeList& tl);
                        nt_nodeList(nt_nodeList& a);
                        nt_nodeList(Pix p);
                        ~nt_nodeList();

  nt_nodeList&              operator = (nt_nodeList& a);

  int                   null();
  int                   valid();
                        operator const void* ();
  int                   operator ! ();

  int                   length();
  int                   list_length();

  nt_node&                  get();
  nt_node&                  head();
  nt_node&                  operator [] (int n);

  nt_nodeList               nth(int n);
  nt_nodeList               tail();
  nt_nodeList               last();

  nt_nodeList               find(nt_node  targ);
  nt_nodeList               find(nt_nodeList& targ);
  int                   contains(nt_node  targ);
  int                   contains(nt_nodeList& targ);
  int                   position(nt_node  targ);

  friend nt_nodeList        copy(nt_nodeList& a);
  friend nt_nodeList        concat(nt_nodeList& a, nt_nodeList& b);
  friend nt_nodeList        append(nt_nodeList& a, nt_nodeList& b);
  friend nt_nodeList        map(nt_nodeMapper f, nt_nodeList& a);
  friend nt_nodeList        merge(nt_nodeList& a, nt_nodeList& b, nt_nodeComparator f);
  friend nt_nodeList        combine(nt_nodeCombiner f, nt_nodeList& a, nt_nodeList& b);
  friend nt_nodeList        reverse(nt_nodeList& a);
  friend nt_nodeList        select(nt_nodePredicate f, nt_nodeList& a);        
#undef remove
  friend nt_nodeList        remove(nt_node  targ, nt_nodeList& a);
  friend nt_nodeList        remove(nt_nodePredicate f, nt_nodeList& a);
  friend nt_nodeList        subst(nt_node  old, nt_node  repl, nt_nodeList& a);

  void                  push(nt_node  x);
  nt_node                   pop();

  void                  set_tail(nt_nodeList& p);
  void                  append(nt_nodeList& p);
  void                  prepend(nt_nodeList& p);
  void                  del(nt_node  targ);
  void                  del(nt_nodePredicate f);
  void                  select(nt_nodePredicate f);
  void                  subst(nt_node  old, nt_node  repl);
  void                  reverse();
  void                  sort(nt_nodeComparator f);

  void                  apply(nt_nodeProcedure f);
  nt_node                   reduce(nt_nodeCombiner f, nt_node  base);

  friend int            operator == (nt_nodeList& a, nt_nodeList& b);
  friend int            operator != (nt_nodeList& a, nt_nodeList& b);

  Pix                   first();
  void                  next(Pix& p);
  Pix                   seek(nt_node  item);
  nt_node&                  operator () (Pix p);
  int                   owns(Pix p);

  void                  error(const char*);
  int                   OK();
};


inline void reference(nt_nodeListNode* p)
{
  if (p->ref >= 0) ++p->ref;
}

inline void dereference(nt_nodeListNode* p)
{
  while (p->ref > 0 && --p->ref == 0)
  {
    nt_nodeListNode* n = p->tl;
    delete(p);
    p = n;
  }
}


inline nt_nodeListNode* newnt_nodeListNode(nt_node  h)
{
  nt_nodeListNode* p = new nt_nodeListNode;
  p->ref = 1;
  p->hd = h;
  return p;
}

inline nt_nodeListNode* newnt_nodeListNode(nt_node  h, nt_nodeListNode* t)
{
  nt_nodeListNode* p = new nt_nodeListNode;
  p->ref = 1;
  p->hd = h;
  p->tl = t;
  return p;
}


inline nt_nodeList::~nt_nodeList()
{
  dereference(P);
}

inline nt_nodeList::nt_nodeList()
{
  P = &Nilnt_nodeListNode;
}

inline nt_nodeList::nt_nodeList(nt_nodeListNode* p)
{
  P = p;
}

inline nt_nodeList::nt_nodeList(nt_node  head)
{
  P = newnt_nodeListNode(head);
  P->tl = &Nilnt_nodeListNode;
}

inline nt_nodeList::nt_nodeList(nt_node  head, nt_nodeList& tl)
{
  P = newnt_nodeListNode(head, tl.P);
  reference(P->tl);
}

inline nt_nodeList::nt_nodeList(nt_nodeList& a)
{
  reference(a.P);
  P = a.P;
}


inline nt_node& nt_nodeList::get()
{
  return P->hd;
}

inline nt_node& nt_nodeList::head()
{
  return P->hd;
}


inline nt_nodeList nt_nodeList::tail()
{
  reference(P->tl);
  return nt_nodeList(P->tl);
}



inline int nt_nodeList::null()
{
  return P == &Nilnt_nodeListNode;
}

inline int nt_nodeList::valid()
{
  return P != &Nilnt_nodeListNode;
}

inline nt_nodeList::operator const void* ()
{
  return (P == &Nilnt_nodeListNode)? 0 : this;
}

inline int nt_nodeList::operator ! ()
{
  return (P == &Nilnt_nodeListNode);
}


inline void nt_nodeList::push(nt_node  head)
{
  nt_nodeListNode* oldp = P;
  P = newnt_nodeListNode(head, oldp);
}


inline int operator != (nt_nodeList& x, nt_nodeList& y)
{
  return !(x == y);
}

inline Pix nt_nodeList::first()
{
  return (P == &Nilnt_nodeListNode)? 0 : Pix(P);
}

inline nt_node& nt_nodeList::operator () (Pix p)
{
  return ((nt_nodeListNode*)p)->hd;
}

inline void nt_nodeList::next(Pix& p)
{
  if (p != 0)
  {
    p = Pix(((nt_nodeListNode*)p)->tl);
    if (p == &Nilnt_nodeListNode) p = 0;
  }
}

inline nt_nodeList::nt_nodeList(Pix p)
{
  P = (nt_nodeListNode*)p;
  reference(P);
}

#endif
