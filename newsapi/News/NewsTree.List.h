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


#ifndef _NewsTreeList_h
#ifdef __GNUG__
#pragma interface
#endif
#define _NewsTreeList_h 1

#include <Pix.h>
#include "NewsTree.defs.h"

#ifndef _NewsTree_typedefs
#define _NewsTree_typedefs 1
typedef void (*NewsTreeProcedure)(NewsTree );
typedef NewsTree  (*NewsTreeMapper)(NewsTree );
typedef NewsTree  (*NewsTreeCombiner)(NewsTree , NewsTree );
typedef int  (*NewsTreePredicate)(NewsTree );
typedef int  (*NewsTreeComparator)(NewsTree , NewsTree );
#endif

struct NewsTreeListNode
{
  NewsTreeListNode*          tl;
  short                 ref;
  NewsTree                   hd;
};

extern NewsTreeListNode NilNewsTreeListNode;

class NewsTreeList
{
protected:
  NewsTreeListNode*          P;

                        NewsTreeList(NewsTreeListNode* p);
public:
                        NewsTreeList();
                        NewsTreeList(NewsTree  head);
                        NewsTreeList(NewsTree  head, NewsTreeList& tl);
                        NewsTreeList(NewsTreeList& a);
                        NewsTreeList(Pix p);
                        ~NewsTreeList();

  NewsTreeList&              operator = (NewsTreeList& a);

  int                   null();
  int                   valid();
                        operator const void* ();
  int                   operator ! ();

  int                   length();
  int                   list_length();

  NewsTree&                  get();
  NewsTree&                  head();
  NewsTree&                  operator [] (int n);

  NewsTreeList               nth(int n);
  NewsTreeList               tail();
  NewsTreeList               last();

  NewsTreeList               find(NewsTree  targ);
  NewsTreeList               find(NewsTreeList& targ);
  int                   contains(NewsTree  targ);
  int                   contains(NewsTreeList& targ);
  int                   position(NewsTree  targ);

  friend NewsTreeList        copy(NewsTreeList& a);
  friend NewsTreeList        concat(NewsTreeList& a, NewsTreeList& b);
  friend NewsTreeList        append(NewsTreeList& a, NewsTreeList& b);
  friend NewsTreeList        map(NewsTreeMapper f, NewsTreeList& a);
  friend NewsTreeList        merge(NewsTreeList& a, NewsTreeList& b, NewsTreeComparator f);
  friend NewsTreeList        combine(NewsTreeCombiner f, NewsTreeList& a, NewsTreeList& b);
  friend NewsTreeList        reverse(NewsTreeList& a);
  friend NewsTreeList        select(NewsTreePredicate f, NewsTreeList& a);        
#undef remove
  friend NewsTreeList        remove(NewsTree  targ, NewsTreeList& a);
  friend NewsTreeList        remove(NewsTreePredicate f, NewsTreeList& a);
  friend NewsTreeList        subst(NewsTree  old, NewsTree  repl, NewsTreeList& a);

  void                  push(NewsTree  x);
  NewsTree                   pop();

  void                  set_tail(NewsTreeList& p);
  void                  append(NewsTreeList& p);
  void                  prepend(NewsTreeList& p);
  void                  del(NewsTree  targ);
  void                  del(NewsTreePredicate f);
  void                  select(NewsTreePredicate f);
  void                  subst(NewsTree  old, NewsTree  repl);
  void                  reverse();
  void                  sort(NewsTreeComparator f);

  void                  apply(NewsTreeProcedure f);
  NewsTree                   reduce(NewsTreeCombiner f, NewsTree  base);

  friend int            operator == (NewsTreeList& a, NewsTreeList& b);
  friend int            operator != (NewsTreeList& a, NewsTreeList& b);

  Pix                   first();
  void                  next(Pix& p);
  Pix                   seek(NewsTree  item);
  NewsTree&                  operator () (Pix p);
  int                   owns(Pix p);

  void                  error(const char*);
  int                   OK();
};


inline void reference(NewsTreeListNode* p)
{
  if (p->ref >= 0) ++p->ref;
}

inline void dereference(NewsTreeListNode* p)
{
  while (p->ref > 0 && --p->ref == 0)
  {
    NewsTreeListNode* n = p->tl;
    delete(p);
    p = n;
  }
}


inline NewsTreeListNode* newNewsTreeListNode(NewsTree  h)
{
  NewsTreeListNode* p = new NewsTreeListNode;
  p->ref = 1;
  p->hd = h;
  return p;
}

inline NewsTreeListNode* newNewsTreeListNode(NewsTree  h, NewsTreeListNode* t)
{
  NewsTreeListNode* p = new NewsTreeListNode;
  p->ref = 1;
  p->hd = h;
  p->tl = t;
  return p;
}


inline NewsTreeList::~NewsTreeList()
{
  dereference(P);
}

inline NewsTreeList::NewsTreeList()
{
  P = &NilNewsTreeListNode;
}

inline NewsTreeList::NewsTreeList(NewsTreeListNode* p)
{
  P = p;
}

inline NewsTreeList::NewsTreeList(NewsTree  head)
{
  P = newNewsTreeListNode(head);
  P->tl = &NilNewsTreeListNode;
}

inline NewsTreeList::NewsTreeList(NewsTree  head, NewsTreeList& tl)
{
  P = newNewsTreeListNode(head, tl.P);
  reference(P->tl);
}

inline NewsTreeList::NewsTreeList(NewsTreeList& a)
{
  reference(a.P);
  P = a.P;
}


inline NewsTree& NewsTreeList::get()
{
  return P->hd;
}

inline NewsTree& NewsTreeList::head()
{
  return P->hd;
}


inline NewsTreeList NewsTreeList::tail()
{
  reference(P->tl);
  return NewsTreeList(P->tl);
}



inline int NewsTreeList::null()
{
  return P == &NilNewsTreeListNode;
}

inline int NewsTreeList::valid()
{
  return P != &NilNewsTreeListNode;
}

inline NewsTreeList::operator const void* ()
{
  return (P == &NilNewsTreeListNode)? 0 : this;
}

inline int NewsTreeList::operator ! ()
{
  return (P == &NilNewsTreeListNode);
}


inline void NewsTreeList::push(NewsTree  head)
{
  NewsTreeListNode* oldp = P;
  P = newNewsTreeListNode(head, oldp);
}


inline int operator != (NewsTreeList& x, NewsTreeList& y)
{
  return !(x == y);
}

inline Pix NewsTreeList::first()
{
  return (P == &NilNewsTreeListNode)? 0 : Pix(P);
}

inline NewsTree& NewsTreeList::operator () (Pix p)
{
  return ((NewsTreeListNode*)p)->hd;
}

inline void NewsTreeList::next(Pix& p)
{
  if (p != 0)
  {
    p = Pix(((NewsTreeListNode*)p)->tl);
    if (p == &NilNewsTreeListNode) p = 0;
  }
}

inline NewsTreeList::NewsTreeList(Pix p)
{
  P = (NewsTreeListNode*)p;
  reference(P);
}

#endif
