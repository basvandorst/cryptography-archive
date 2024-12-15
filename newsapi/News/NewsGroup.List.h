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


#ifndef _ng_nodeList_h
#ifdef __GNUG__
#pragma interface
#endif
#define _ng_nodeList_h 1

#include <Pix.h>
#include "NewsGroup.defs.h"

#ifndef _ng_node_typedefs
#define _ng_node_typedefs 1
typedef void (*ng_nodeProcedure)(ng_node );
typedef ng_node  (*ng_nodeMapper)(ng_node );
typedef ng_node  (*ng_nodeCombiner)(ng_node , ng_node );
typedef int  (*ng_nodePredicate)(ng_node );
typedef int  (*ng_nodeComparator)(ng_node , ng_node );
#endif

struct ng_nodeListNode
{
  ng_nodeListNode*          tl;
  short                 ref;
  ng_node                   hd;
};

extern ng_nodeListNode Nilng_nodeListNode;

class ng_nodeList
{
protected:
  ng_nodeListNode*          P;

                        ng_nodeList(ng_nodeListNode* p);
public:
                        ng_nodeList();
                        ng_nodeList(ng_node  head);
                        ng_nodeList(ng_node  head, ng_nodeList& tl);
                        ng_nodeList(ng_nodeList& a);
                        ng_nodeList(Pix p);
                        ~ng_nodeList();

  ng_nodeList&              operator = (ng_nodeList& a);

  int                   null();
  int                   valid();
                        operator const void* ();
  int                   operator ! ();

  int                   length();
  int                   list_length();

  ng_node&                  get();
  ng_node&                  head();
  ng_node&                  operator [] (int n);

  ng_nodeList               nth(int n);
  ng_nodeList               tail();
  ng_nodeList               last();

  ng_nodeList               find(ng_node  targ);
  ng_nodeList               find(ng_nodeList& targ);
  int                   contains(ng_node  targ);
  int                   contains(ng_nodeList& targ);
  int                   position(ng_node  targ);

  friend ng_nodeList        copy(ng_nodeList& a);
  friend ng_nodeList        concat(ng_nodeList& a, ng_nodeList& b);
  friend ng_nodeList        append(ng_nodeList& a, ng_nodeList& b);
  friend ng_nodeList        map(ng_nodeMapper f, ng_nodeList& a);
  friend ng_nodeList        merge(ng_nodeList& a, ng_nodeList& b, ng_nodeComparator f);
  friend ng_nodeList        combine(ng_nodeCombiner f, ng_nodeList& a, ng_nodeList& b);
  friend ng_nodeList        reverse(ng_nodeList& a);
  friend ng_nodeList        select(ng_nodePredicate f, ng_nodeList& a);        
#undef remove
  friend ng_nodeList        remove(ng_node  targ, ng_nodeList& a);
  friend ng_nodeList        remove(ng_nodePredicate f, ng_nodeList& a);
  friend ng_nodeList        subst(ng_node  old, ng_node  repl, ng_nodeList& a);

  void                  push(ng_node  x);
  ng_node                   pop();

  void                  set_tail(ng_nodeList& p);
  void                  append(ng_nodeList& p);
  void                  prepend(ng_nodeList& p);
  void                  del(ng_node  targ);
  void                  del(ng_nodePredicate f);
  void                  select(ng_nodePredicate f);
  void                  subst(ng_node  old, ng_node  repl);
  void                  reverse();
  void                  sort(ng_nodeComparator f);

  void                  apply(ng_nodeProcedure f);
  ng_node                   reduce(ng_nodeCombiner f, ng_node  base);

  friend int            operator == (ng_nodeList& a, ng_nodeList& b);
  friend int            operator != (ng_nodeList& a, ng_nodeList& b);

  Pix                   first();
  void                  next(Pix& p);
  Pix                   seek(ng_node  item);
  ng_node&                  operator () (Pix p);
  int                   owns(Pix p);

  void                  error(const char*);
  int                   OK();
};


inline void reference(ng_nodeListNode* p)
{
  if (p->ref >= 0) ++p->ref;
}

inline void dereference(ng_nodeListNode* p)
{
  while (p->ref > 0 && --p->ref == 0)
  {
    ng_nodeListNode* n = p->tl;
    delete(p);
    p = n;
  }
}


inline ng_nodeListNode* newng_nodeListNode(ng_node  h)
{
  ng_nodeListNode* p = new ng_nodeListNode;
  p->ref = 1;
  p->hd = h;
  return p;
}

inline ng_nodeListNode* newng_nodeListNode(ng_node  h, ng_nodeListNode* t)
{
  ng_nodeListNode* p = new ng_nodeListNode;
  p->ref = 1;
  p->hd = h;
  p->tl = t;
  return p;
}


inline ng_nodeList::~ng_nodeList()
{
  dereference(P);
}

inline ng_nodeList::ng_nodeList()
{
  P = &Nilng_nodeListNode;
}

inline ng_nodeList::ng_nodeList(ng_nodeListNode* p)
{
  P = p;
}

inline ng_nodeList::ng_nodeList(ng_node  head)
{
  P = newng_nodeListNode(head);
  P->tl = &Nilng_nodeListNode;
}

inline ng_nodeList::ng_nodeList(ng_node  head, ng_nodeList& tl)
{
  P = newng_nodeListNode(head, tl.P);
  reference(P->tl);
}

inline ng_nodeList::ng_nodeList(ng_nodeList& a)
{
  reference(a.P);
  P = a.P;
}


inline ng_node& ng_nodeList::get()
{
  return P->hd;
}

inline ng_node& ng_nodeList::head()
{
  return P->hd;
}


inline ng_nodeList ng_nodeList::tail()
{
  reference(P->tl);
  return ng_nodeList(P->tl);
}



inline int ng_nodeList::null()
{
  return P == &Nilng_nodeListNode;
}

inline int ng_nodeList::valid()
{
  return P != &Nilng_nodeListNode;
}

inline ng_nodeList::operator const void* ()
{
  return (P == &Nilng_nodeListNode)? 0 : this;
}

inline int ng_nodeList::operator ! ()
{
  return (P == &Nilng_nodeListNode);
}


inline void ng_nodeList::push(ng_node  head)
{
  ng_nodeListNode* oldp = P;
  P = newng_nodeListNode(head, oldp);
}


inline int operator != (ng_nodeList& x, ng_nodeList& y)
{
  return !(x == y);
}

inline Pix ng_nodeList::first()
{
  return (P == &Nilng_nodeListNode)? 0 : Pix(P);
}

inline ng_node& ng_nodeList::operator () (Pix p)
{
  return ((ng_nodeListNode*)p)->hd;
}

inline void ng_nodeList::next(Pix& p)
{
  if (p != 0)
  {
    p = Pix(((ng_nodeListNode*)p)->tl);
    if (p == &Nilng_nodeListNode) p = 0;
  }
}

inline ng_nodeList::ng_nodeList(Pix p)
{
  P = (ng_nodeListNode*)p;
  reference(P);
}

#endif
