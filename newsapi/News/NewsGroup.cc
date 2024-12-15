
#include "NewsGroup.h"

NewsGroup::NewsGroup (news_group_type * ng)
{
  _group = ng;
}

NewsGroup::NewsGroup (void)
{
  _group = 0;
}

inline int
NewsGroup::compare (NewsGroup& a, NewsGroup& b)
{
  return strcmp (a.name(), b.name());
}

NewsGroup::~NewsGroup()
{
  // this is done by newsapi.
  //  delete _group;
}
