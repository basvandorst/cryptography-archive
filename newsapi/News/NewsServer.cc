// a -*- C++ -*- wrapper for newsapi.lib
//

#include "newsapi.h"
#include "NewsGroup.h"
#include "NewsServer.h"
#include "iostream.h"

//---------------------------------------------------------------------------
//
// Generic news server members
//

int
NewsServer::ready (void)
{
  if (_server)
    return (1);
  else
    return (0);
}

char *
NewsServer::name(void)
{
  return (_server->hostname);
}

int
NewsServer::connect(int server_type, void * connect_data)
{
  int retval;

  if ((retval = news_add_server (server_type, connect_data, &_server))) {
    _server = 0;
  }

  return (retval);
}    

int
compare_ng (NewsGroup& a, NewsGroup& b)
{
  return strcmp (a.name(), b.name());
}

int
NewsServer::list(void)
{
  news_group_list_type * group_list;
  unsigned long i;

  // this will update _server's group_list 
  news_list_groups(_server);
  
  // now, we construct the NewsGroupList from it

  group_list = _server->group_list;

  for (i = 0; i < group_list->length; i++) {
    NewsGroup * ng = new NewsGroup(&(group_list->groups[i]));
    groups.push (ng);
  }

  groups.sort (&compare_ng);

  // now, construct the tree of newsgroups

}

void NewsServer::dump(void)
{
  for (Pix i = groups.first(); i != 0; groups.next(i))
    cout << groups(i);
}


void
NewsServer::close (void)
{
  if (_server)
    news_close_server (_server);

}

NewsServer::~NewsServer (void)
{
  close();
}

#if 0
int
group_length (NewsGroup & nga)
{
  const char * a = nga.name();


  int count = 0;
  while (*a) {
    if (*a == '.')
      count++;
    a++;
  }
  return (count+1);
}

int
compare_name_depth (NewsGroup & nga, NewsGroup & ngb)
{
  int count = 0;
  const char * a = nga.name();
  const char * b = nga.name();  

  while (*a && *b && (*a == *b)) {
    if (*a == '.')
      count++;
    a++; b++;
  }
  if (*b && !*a && *b == '.')
    count++;
  if (*a && !*b && *a == '.')
    count++;

  return count;
}

NewsGroup * _current_group_ = NULL;
NewsGroupList * _current_list_ = NULL;
int _rising_ = 0;

NewsTreeList *
construct_group_tree (int depth)
{
  if (depth == (group_length (*_current_group_))) {

    // this computes how far back up the tree to go

    _rising_ =
      depth -
	compare_name_depth
	  (*_current_group_,
	   _current_list_->head());
    
    _current_group_ = & (_current_list_->pop());

    NewsTree * nt = new NewsTree (*_current_group_);
    NewsTreeList * ntl = new NewsTreeList;
    return ntl;
  }
}

#endif

//---------------------------------------------------------------------------
//
// NNTP server members
//

Nntp_NewsServer::Nntp_NewsServer (char * hostname,
				    char * username = NULL,
				    char * password = NULL)
{
  // we make copies just to be sure 
  connect_data.hostname = new char[strlen(hostname)+1];
  strcpy (connect_data.hostname, hostname);
  
  if (username) {
    connect_data.username = new char[strlen(username)+1];
    strcpy (connect_data.username, username);
  }
  else 
    connect_data.username = NULL;
  
  if (password) {
    connect_data.password = new char[strlen(password)+1];
    strcpy (connect_data.password, password);
  }
  else 
    connect_data.password = NULL;
  
  connect (NNTP_SERVER_TYPE, &connect_data);
}

Nntp_NewsServer::~Nntp_NewsServer()
{
  close();
  delete connect_data.hostname;
  delete connect_data.username;
  delete connect_data.password;
}
