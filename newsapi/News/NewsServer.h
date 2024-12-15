
#ifndef _NEWSSERVER_H
#define _NEWSSERVER_H

#include "newsapi.h"
#include "NewsGroup.List.h"
// #include "NewsTreeList.h"

// wrapper classes for newsapi

//---------------------------------------------------------------------------
//
// class NewsServer
//

class NewsServer
{
 private: 
  news_server_type * _server;
  NewsGroupList groups;
//  NewsTreeList tree;

 public:
  int connect (int server_type, void * connect_data);
  char * name (void);
  void dump(void);
  void close (void);
  int ready(void);
  int list (void);
  ~NewsServer(void);
};


//---------------------------------------------------------------------------
//
// NNTP server
//


class Nntp_NewsServer : public NewsServer
{
 private:
  nntp_connect_type connect_data;

 public:
  Nntp_NewsServer (char * hostname,
		    char * username = NULL,
		    char * password = NULL);

  ~Nntp_NewsServer();

};



#endif // _NEWSSERVER_H
