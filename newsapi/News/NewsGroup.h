
#ifndef _NEWSGROUP_H
#define _NEWSGROUP_H

#include "newsapi.h"
#include <iostream.h>

class NewsGroup
{
  friend class NewsServer;

 private:
  news_group_type * _group;
  NewsGroup (news_group_type * ng);

 public:
  friend ostream& operator<< (ostream& os, const NewsGroup& ng);

  const char * name() const		{ return _group->group_name; }
  const unsigned long low() 		{ return _group->low; }
  const unsigned long estnum()		{ return _group->estnum; } 
  const unsigned long high()		{ return _group->high; }
  // FIXME:
  const char flags()			{ return _group->flags; }

  int compare (NewsGroup& a, NewsGroup& b);

  NewsGroup();
  ~NewsGroup();
};

typedef NewsGroup * NewsGroupP;

inline ostream & operator<< (ostream& os, const NewsGroup& ng)
{
  return (os << ng.name() << '\n');
}

#endif // _NEWSGROUP_H
