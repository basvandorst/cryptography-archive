#include <sys/param.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include "dbatch.h"

/*--------------------------------------------- SYSTEM CALLS -----------------*/
int  gethostname(char *name, int namelen);
/*----------------------------------------------------------------------------*/

static char hostNameList[NOFHOST][MAXHOSTNAMELEN];

/*----------------------------------------------------------------------------*/
void Base_Init ()

{ char *hostName;
  char str[10000];
  int hostId;

  Prolog;
  hostId = 1;
  strcpy(str, HOSTNAMELIST);
  hostName = strtok(str, " ");
  while (hostName != NULL && hostId <= NOFHOST) {
    strcpy(hostNameList[hostId++], hostName);
    hostName = strtok(NULL, " ");
    hostName = strtok(NULL, " ");
  }
  if (hostName != NULL || hostId != (NOFHOST + 1)) E(1000);
} /* Base_Init */

/*----------------------------------------------------------------------------*/
void Base_Error (int num)

{ char cmd[1024];
  char hostName[MAXHOSTNAMELEN];

  fprintf(stderr, "*** error %d\n", num);
  ok = FALSE;
  if (gethostname(hostName, MAXHOSTNAMELEN) != 0) strcpy(hostName, "?");
  sprintf(cmd, "/usr/ucb/Mail -s '%8s error %4d' %s < /dev/null",
          hostName, num, DBATCHADMIN);
  system(cmd);
} /* Base_Error */

/*----------------------------------------------------------------------------*/
struct Package *Base_NewPackage ()

{ struct Package *package;

  Prolog NULL;
  if ((package = (struct Package *)malloc(PACKAGESIZE)) != NULL)
    package->next = NULL;
  return package;
} /* Base_NewPackage */

/*----------------------------------------------------------------------------*/
void Base_DisposePackageList  (struct Package *package)

{ struct Package *p, *q;

  Prolog;
  p = package;
  while (p != NULL) {
    q = p->next;
    free(p);
    p = q;
  }
} /* Base_DisposePackageList */

/*----------------------------------------------------------------------------*/
void Base_HostIdToName (int hostId, char *hostName)

{ Prolog;
  if (hostId <= 0 || NOFHOST < hostId) E(1001);
  strcpy(hostName, hostNameList[hostId]);
} /* Base_HostIdToName */

/*----------------------------------------------------------------------------*/
void Base_HostNameToId (char *hostName, int *hostId)

{ Prolog;
  for (*hostId = 1; *hostId <= NOFHOST; (*hostId)++) {
    if (strcmp(hostName, hostNameList[*hostId]) == 0) return;
  }
  *hostId = NONE;
} /* Base_HostNameToId */

/*----------------------------------------------------------------------------*/
void Base_HostSetToShortString (int hostSet[], char *str)

{ int hostId;

  Prolog;
  str[0] = '\0';
  for (hostId = 1; hostId <= NOFHOST; hostId++)
    if (Base_InHostSet(hostId, hostSet))
      str[hostId - 1] = hostNameList[hostId][strlen(hostNameList[hostId]) - 1];
    else
      str[hostId - 1] = '_';
  str[NOFHOST] = '\0';
  Epilog(1002);
} /* Base_HostSetToShortString */

/*----------------------------------------------------------------------------*/
void Base_HostSetToString (int hostSet[], char *str)

{ int hostId;

  Prolog;
  str[0] = '\0';
  for (hostId = 1; hostId <= NOFHOST; hostId++) {
    if (Base_InHostSet(hostId, hostSet)) {
      strcpy(&str[strlen(str)], hostNameList[hostId]);
      strcpy(&str[strlen(str)], " ");
    }
  }
  Epilog(1003);
} /* Base_HostSetToString */

/*----------------------------------------------------------------------------*/
void Base_HostStringToSet (char *str, int hostSet[])

{ char *hostName;
  int hostId;
  int lastHostId;

  Prolog;
  Base_EmptyHostSet(hostSet);
  hostName = strtok(str, " ");
  hostId = NONE;
  while (ok && hostName != NULL) {
    if (hostId != NONE && !strcmp(hostName, "..")) {
      hostName = strtok(NULL, " ");
      if (hostName == NULL) E(1004);
      Base_HostNameToId(hostName, &lastHostId);
      if (lastHostId == NONE) E(1005);
      if (hostId < lastHostId)
        for (; hostId <= lastHostId; hostId++)
          Base_InclHostSet(hostId, hostSet);
      else
        for (; hostId >= lastHostId; hostId--)
          Base_InclHostSet(hostId, hostSet);
    }
    else {
      Base_HostNameToId(hostName, &hostId);
      if (hostId == NONE) E(1006);
      Base_InclHostSet(hostId, hostSet);
    }
    hostName = strtok(NULL, " ");
  }
  Epilog(1007);
} /* Base_HostStringToSet */

/*----------------------------------------------------------------------------*/
void Base_EmptyHostSet (int hostSet[])

{ int i;

  Prolog;
  for (i = 0; i < HOSTSETLEN; i++) hostSet[i] = 0;
} /* Base_EmptyHostSet */

/*----------------------------------------------------------------------------*/
void Base_FullHostSet (int hostSet[])

{ int i;

  Prolog;
  for (i = 0; i < HOSTSETLEN; i++) hostSet[i] = ~0;
} /* Base_FullHostSet */

/*----------------------------------------------------------------------------*/
int  Base_FirstInHostSet (int hostSet[])

{ int i, h;

  Prolog NONE;
  i = 0;
  while (i < HOSTSETLEN && hostSet[i] == 0) i++;
  if (i >= HOSTSETLEN)
    return NONE;
  else {
    h = hostSet[i];
    i = i * HOSTSPERINT + 1;
    while ((h & 1) == 0) { h >>= 1; i++; }
    return (i > NOFHOST) ? NONE : i;
  }
} /* Base_FirstInHostSet */

/*----------------------------------------------------------------------------*/
int  Base_InHostSet (int hostId, int hostSet[])

{ Prolog FALSE;
  if (hostId <= 0 || NOFHOST < hostId) ER(1008, FALSE);
  hostId--;
  return (hostSet[hostId / HOSTSPERINT] & 1 << hostId % HOSTSPERINT) != 0;
} /* Base_InHostSet */

/*----------------------------------------------------------------------------*/
void Base_InclHostSet (int hostId, int hostSet[])

{ Prolog;
  if (hostId <= 0 || NOFHOST < hostId) E(1009);
  hostId--;
  hostSet[hostId / HOSTSPERINT] |= 1 << hostId % HOSTSPERINT;
} /* Base_InclHostSet */

/*----------------------------------------------------------------------------*/
void Base_ExclHostSet (int hostId, int hostSet[])

{ Prolog;
  if (hostId <= 0 || NOFHOST < hostId) E(1010);
  hostId--;
  hostSet[hostId / HOSTSPERINT] &= ~(1 << hostId % HOSTSPERINT);
} /* Base_ExclHostSet */

/*----------------------------------------------------------------------------*/
void Base_JoinHostSet (int a[], int b[], int c[])

{ int i;

  Prolog;
  for (i = 0; i < HOSTSETLEN; i++) c[i] = a[i] & b[i];
} /* Base_JoinHostSet */

/*----------------------------------------------------------------------------*/
