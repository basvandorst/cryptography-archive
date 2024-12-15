#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fcntl.h>
#include <sys/param.h>
#include "dbatch.h"

/*--------------------------------------------- SYSTEM CALLS -----------------*/
int gethostname(char *name, int namelen);
int read(int fd, char *buf, int nbyte);
int write(int fd, char *buf, int nbyte);
int close(int des);

/*--------------------------------------------- C LIBRARY FUNCTIONS ----------*/
void sleep(unsigned seconds);

/*--------------------------------------------- NETWORK FUNCTIONS ------------*/
#include <tiuser.h>
#include <netdb.h>

int   t_accept(int fd, int resfd, struct t_call *call);
char *t_alloc(int fd, int struct_type, int fields);
int   t_bind(int fd, struct t_bind *req, struct t_bind *ret);
int   t_close(int fd);
int   t_connect(int fd, struct t_call *sndcall, struct t_call *rcvcall);
int   t_errno;
int   t_free(char *ptr, int struct_type);
int   t_listen(int fd, struct t_call *call);
int   t_look(int fd);
int   t_open(char *path, int oflag, struct t_info *info);
int   t_rcv(int fd, char *buf, unsigned nbytes, int *flags);
int   t_rcvdis(int fd, struct t_discon *discon);
int   t_snd(int fd, char *buf, unsigned nbytes, int flags);
int   t_snddis(int fd, struct t_call *call);
int   t_unbind(int fd);

/*----------------------------------------------------------------------------*/
void IPC_PutMessage (int connFD, struct Message *message)

{ Prolog;
  if (t_snd(connFD, (char *)message, MESSAGESIZE, 0) != MESSAGESIZE) E(5000);
} /* IPC_PutMessage */

/*----------------------------------------------------------------------------*/
void IPC_GetMessage (int connFD, struct Message *message)

{ int flags = 0;

  Prolog;
  if (t_rcv(connFD, (char *)message, MESSAGESIZE, &flags) != MESSAGESIZE) 
    E(5001);
} /* IPC_GetMessage */

/*----------------------------------------------------------------------------*/
void IPC_PutPackage (int connFD, struct Package *package)

{ Prolog;
  if (t_snd(connFD, (char *)package, PACKAGESIZE, 0) != PACKAGESIZE) E(5002);
} /* IPC_PutPackage */

/*----------------------------------------------------------------------------*/
void IPC_GetPackage (int connFD, struct Package *package)

{ int flags = 0;

  Prolog;
  if (t_rcv(connFD, (char *)package, PACKAGESIZE, &flags) != PACKAGESIZE)
    E(5003);
} /* IPC_GetPackage */

/*----------------------------------------------------------------------------*/
void IPC_PutPackageList (int connFD, struct Package *package)

{ Prolog;
  if (package == NULL) E(5004);
  do {
    IPC_PutPackage(connFD, package);
    package = package->next;
  } while(package != NULL);
  Epilog(5005);
} /* IPC_PutPackageList */

/*----------------------------------------------------------------------------*/
void IPC_GetPackageList (int connFD, struct Package **package)

{ struct Package *curr;

  Prolog;
  if ((curr = *package = Base_NewPackage()) == NULL) E(5006);
  do {
    IPC_GetPackage(connFD, curr);
    if (curr->next != NULL)
      if ((curr->next = Base_NewPackage()) == NULL) E(5007);
    curr = curr->next;
  } while (curr != NULL);
  Epilog(5008);
} /* IPC_GetPackageList */

/*----------------------------------------------------------------------------*/
void IPC_OpenText (struct Package *textBuffer)

{ Prolog;
  textBuffer->next = (struct Package *)(0); /* current text length */
} /* IPC_OpenText */

/*----------------------------------------------------------------------------*/
void IPC_WriteText (int connFD, struct Package *textBuffer, char *text)

{ int last;
  int newLast;

  Prolog;
  last = (int)(textBuffer->next);
  for (newLast = strlen(text) + last; newLast >= PACKAGEBUFSIZE - 1;
                                      newLast -= PACKAGEBUFSIZE - 1) {
    strncpy(&(textBuffer->buf[last]), text, PACKAGEBUFSIZE - 1 - last);
    text = &(text[PACKAGEBUFSIZE - 1 - last]);
    textBuffer->buf[PACKAGEBUFSIZE - 1] = '\0';
    textBuffer->next = (struct Package *)(~NULL);
    IPC_PutPackage(connFD, textBuffer);
    last = 0;
  }
  if (newLast > 0) strcpy(&(textBuffer->buf[last]), text);
  textBuffer->next = (struct Package *)newLast;
  Epilog(5009);
} /* IPC_WriteText */


/*----------------------------------------------------------------------------*/
void IPC_CloseText (int connFD, struct Package *textBuffer)

{ Prolog;
  textBuffer->buf[(int)(textBuffer->next)] = '\0';
  textBuffer->next = NULL;
  IPC_PutPackage(connFD, textBuffer);
  Epilog(5010);
} /* IPC_CloseText */

/*----------------------------------------------------------------------------*/
void IPC_ReadText (int connFD)

{ struct Package textBuffer;

  Prolog;
  do {
    IPC_GetPackage(connFD, &textBuffer); T(5011);
    printf("%s", textBuffer.buf);
  } while (textBuffer.next != NULL);
  Epilog(5012);
} /* IPC_ReadText */

/*----------------------------------------------------------------------------*/
void IPC_OpenListen (int *listenFD)

{ int i;
  int fd;
  struct t_bind *bind;
  struct hostent *host;
  char hostname[MAXHOSTNAMELEN];

  Prolog;
  if ((*listenFD = t_open("/dev/tcp", O_RDWR, NULL)) < 0) E(5013);
  if ((bind = (struct t_bind *)t_alloc(*listenFD, T_BIND, T_ALL)) == NULL) 
    E(5014);
  bind->qlen = MAXNOFLISTEN;
  bind->addr.len = 0;
  if (t_bind(*listenFD, bind, bind) < 0) E(5015);
  if (gethostname(hostname, MAXHOSTNAMELEN) < 0) E(5016);
  if ((host = gethostbyname(hostname)) == NULL) E(5017);
  for (i = 0; i < host->h_length; i++)
    bind->addr.buf[4 + i] = host->h_addr_list[0][i];
  if ((fd = creat(ADDRESSFILE, 0644)) < 0) E(5018);
  if (write(fd, bind->addr.buf, bind->addr.maxlen) != bind->addr.maxlen)
    E(5019);
  close(fd);
} /* IPC_OpenListen */

/*----------------------------------------------------------------------------*/
void IPC_Listen (int listenFD, void (*HandleMessage)(int),
                               void (*StartNextJob)())

{ int connFD;
  struct t_call *call;

  ok = TRUE;
  if ((call = (struct t_call *)t_alloc(listenFD,T_CALL,T_ALL)) == NULL) E(5020);
  if (t_listen(listenFD, call) < 0) E(5021);
  if ((connFD = t_open("/dev/tcp", O_RDWR, NULL)) < 0) E(5022);
  if (t_bind(connFD, NULL, NULL) < 0) E(5023);
  while (t_accept(listenFD, connFD, call) < 0) {
    if (t_errno != TLOOK) E(5024);
    if (t_look(listenFD) != T_LISTEN) E(5025);
    IPC_Listen(listenFD, HandleMessage, StartNextJob);
  }
  (*HandleMessage)(connFD);
  ok = TRUE;
  if (t_snddis(connFD, NULL) != 0) E(5026);
  if (t_unbind(connFD) != 0) E(5027);
  if (t_close(connFD) != 0) E(5028);
  if (t_free((char *)call, T_CALL) != 0) E(5029);
  (*StartNextJob)();
  ok = TRUE;
} /* IPC_Listen */

/*----------------------------------------------------------------------------*/
void IPC_OpenConnection (int *connFD)

{ int fd;
  struct t_call *sndcall;

  Prolog;
  if ((*connFD = t_open("/dev/tcp", O_RDWR, NULL)) < 0) E(5030);
  if (t_bind(*connFD, NULL, NULL) < 0) E(5031);
  if ((sndcall = (struct t_call *)t_alloc(*connFD, T_CALL, T_ADDR)) == NULL) 
    E(5032);
  if ((fd = open(ADDRESSFILE, O_RDONLY)) < 0) E(5033);
  if ((sndcall->addr.len = read(fd, sndcall->addr.buf, sndcall->addr.maxlen)) !=
      sndcall->addr.maxlen) E(5034);
  close(fd);
  while (t_connect(*connFD, sndcall, NULL) < 0) {
    if (t_look(*connFD) != T_DISCONNECT) E(5035);
    while (t_rcvdis(*connFD, NULL) != 0) {
      if (t_errno != TNODIS) E(5036);
      sleep(1);
    }
  }
} /* IPC_OpenConnection */

/*----------------------------------------------------------------------------*/
void IPC_CloseConnection (int connFD)

{ Prolog;
  while (t_rcvdis(connFD, NULL) != 0) {
    if (t_errno != TNODIS) E(5037);
    sleep(1);
  }
  if (t_unbind(connFD) != 0) E(5038);
  if (t_close(connFD) != 0) E(5039);
} /* IPC_CloseConnection */

/*----------------------------------------------------------------------------*/
