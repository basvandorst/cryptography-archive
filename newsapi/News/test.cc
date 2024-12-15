#include "NewsServer.h"

// ---------------------------------------------------------------------------
//
// testing

main()
{
  Nntp_NewsServer * nntp;
  
  nntp = new Nntp_NewsServer ("algol");

  nntp.list();

  nntp.dump();
  
  delete nntp;
}

