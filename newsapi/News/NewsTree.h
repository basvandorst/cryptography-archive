// -*- C++ -*-

#ifndef _NEWSTREE_H
#define _NEWSTREE_H

#include "NewsGroup.h"

class NewsTreeList;

class NewsTree
{
 private:
  char _tree_name[16];	/* I think this assumption is ok (death to sysV paths!) */
  NewsGroup * _ng;
  NewsTreeList * _nt;
  
 public:
  const char * tree_name(void)		{ return _tree_name; }
  const NewsGroup * ng(void)		{ return _ng; }
  const NewsTreeList * children(void) 	{ return _nt; }
  NewsTree (NewsGroup & ng);
};

typedef NewsTree * nt_node;

#endif // _NEWSTREE_H
