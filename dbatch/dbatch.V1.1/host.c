#include <stdio.h>
#include <string.h>
#include <limits.h>

#include <sys/param.h>
#include "dbatch.h"

/*--------------------------------------------- SYSTEM CALLS -----------------*/
int read(int fd, char *buf, int nbyte);
int write(int fd, char *buf, int nbyte);

/*----------------------------------------------------------------------------*/
struct Host {
  int state;
  int silent;
  int nofJob;
  int nofProcessor;
  int nofInquireState;
  int load;
  int exeLevel;
  unsigned int nofSlot;
  unsigned int nofIdleSlot;
  long cpuTime;
};

static struct Host host[NOFHOST + 1];

/*----------------------------------------------------------------------------*/
static void Host_InitStruct (int hostId)

{ host[hostId].state = NONE;
  host[hostId].silent = 0;
  host[hostId].nofJob = 0;
  host[hostId].nofSlot = 0;
  host[hostId].nofIdleSlot = 0;
  host[hostId].nofInquireState = 0;
  host[hostId].load = 0;
  host[hostId].exeLevel = 0;
  host[hostId].cpuTime = 0;
} /* Host_InitStruct */

/*----------------------------------------------------------------------------*/
void Host_Init ()

{ int hostId;
  char *nofProcessor;
  char str[10000];

  Prolog;
  strcpy(str, HOSTNAMELIST);
  nofProcessor = strtok(str, " ");
  for (hostId = 1; hostId <= NOFHOST; hostId++) {
    Host_InitStruct(hostId);
    host[hostId].state = MAYBEDOWN;
    host[hostId].nofInquireState = 1;
    nofProcessor = strtok(NULL, " ");
    if (nofProcessor == NULL) E(4000);
    sscanf(nofProcessor, "%d", &host[hostId].nofProcessor);
    nofProcessor = strtok(NULL, " ");
  }
  Epilog(4001);
} /* Host_Init */

/*----------------------------------------------------------------------------*/
static void Host_AdjustExeLevel ()

{ int hostId;
  long maxCpuTime = 0;
  long minCpuTime = LONG_MAX;
  long dCpuTime;

  Prolog;
  for (hostId = 1; hostId <= NOFHOST; hostId++) {
    if (host[hostId].cpuTime > maxCpuTime) maxCpuTime = host[hostId].cpuTime;
    if (host[hostId].cpuTime < minCpuTime) minCpuTime = host[hostId].cpuTime;
  }
  dCpuTime = maxCpuTime - minCpuTime;
  if (!dCpuTime) dCpuTime = 1;
  for (hostId = 1; hostId <= NOFHOST; hostId++) {
    host[hostId].exeLevel = NOFEXELEVEL;
    if (host[hostId].nofJob < host[hostId].nofProcessor)
      if (host[hostId].state == OCCUPIED && host[hostId].load < MAXLOADOCCUPIED
          && !host[hostId].silent)
        host[hostId].exeLevel = (NOFEXELEVEL / 3 * host[hostId].load +
                                 MAXLOADOCCUPIED / 2) / MAXLOADOCCUPIED +
                                (NOFEXELEVEL / 3 * (host[hostId].cpuTime - 
                                 minCpuTime) + dCpuTime / 2) / dCpuTime +
                                 NOFEXELEVEL / 3;
      else if (host[hostId].state == FREE && host[hostId].load < MAXLOADFREE)
        host[hostId].exeLevel = (NOFEXELEVEL / 3 * host[hostId].load +
                                 MAXLOADFREE / 2) / MAXLOADFREE +
                                (NOFEXELEVEL / 3 * (host[hostId].cpuTime - 
                                 minCpuTime) + dCpuTime / 2) / dCpuTime;
    if (host[hostId].exeLevel > NOFEXELEVEL) host[hostId].exeLevel= NOFEXELEVEL;
  }
} /* Host_AdjustExeLevel */

/*----------------------------------------------------------------------------*/
void Host_SaveState (int fileDesc)

{ Prolog;
  if (write(fileDesc, (char *)host, sizeof host) != sizeof host) E(4002);
} /* Host_SaveState */

/*----------------------------------------------------------------------------*/
void Host_LoadState (int fileDesc)

{ Prolog;
  if (read(fileDesc, (char *)host, sizeof host) != sizeof host) E(4003);
} /* Host_LoadState */

/*----------------------------------------------------------------------------*/
void Host_SendState (int connFD, int userId, struct Package *textBuffer)

{ int hostId;
  int row;
  int nofRow;
  int col;
  int nofCol = 3;
  char str[1024];
  char hostName[MAXHOSTNAMELEN];

  Prolog;
  if (userId == ROOT) {
    Host_AdjustExeLevel();
    strcpy(str, 
"Host     State     Silent Usage ExeLevel Load     #Slot #IdleSlot   cpuTime\n"
          );
    IPC_WriteText(connFD, textBuffer, str);
    for (hostId = 1; hostId <= NOFHOST; hostId++) {
      Base_HostIdToName(hostId, hostName);
      sprintf(str, "%-9s", hostName);
      Host_StateToString(hostId, &str[strlen(str)]);
      sprintf(str, "%s %4d %d/%d/%d %8d %4d %9u %9u %9d\n", str,
              host[hostId].silent,
              host[hostId].nofJob,
              host[hostId].nofProcessor,
              host[hostId].nofInquireState,
              host[hostId].exeLevel,
              host[hostId].load,
              host[hostId].nofSlot,
              host[hostId].nofIdleSlot,
              host[hostId].cpuTime);
      IPC_WriteText(connFD, textBuffer, str);
    }
  }
  else {
    str[0] = '\0';
    for (col = 1; col <= nofCol; col++) {
      strcat(str, "Host     State      Usage");
      if (col != nofCol) strcat(str, "  ");
    }
    sprintf(str, "%s\n", str);
    IPC_WriteText(connFD, textBuffer, str);
    nofRow = (NOFHOST + nofCol - 1) / nofCol;
    for (row = 1; row <= nofRow; row++) {
      str[0] = '\0';
      for (col = 1; col <= nofCol; col++) {
        hostId = (col - 1) * nofRow + row;
        if (hostId <= NOFHOST) {
          Base_HostIdToName(hostId, hostName);
          sprintf(str, "%s%-8s%c", str, hostName, host[hostId].nofJob?'*':' ');
          Host_StateToString(hostId, &str[strlen(str)]);
          if (host[hostId].nofJob)
            sprintf(str, "%s %3dJ", str, host[hostId].nofJob);
          else if (host[hostId].state & (DOWN | MAYBEDOWN))
            strcat(str, "    -");
          else
            sprintf(str, "%s %3d%%", str, host[hostId].load);
          if (col != nofCol) strcat(str, "  ");
        }
      }
      strcat(str, "\n");
      IPC_WriteText(connFD, textBuffer, str);
    }
  }
  Epilog(4004);
} /* Host_SendState */

/*----------------------------------------------------------------------------*/
void Host_CheckState ()

{ int nofIS;
  int hostId;

  Prolog;
  for (hostId = 1; hostId <= NOFHOST; hostId++) {
    nofIS = host[hostId].nofInquireState++;
    if (nofIS >= MAXNOFINQUIRE)
      host[hostId].state = DOWN;
    else if (nofIS >= 1 && host[hostId].state != DOWN)
      host[hostId].state = MAYBEDOWN;
    OSD_InquireHostState(hostId);
  }
  Epilog(4005);
} /* Host_CheckState */

/*----------------------------------------------------------------------------*/
void Host_IncJobCount (int hostId)

{ Prolog;
  if (hostId <= 0 || NOFHOST < hostId) E(4006);
  host[hostId].nofJob++;
  Epilog(4007);
} /* Host_IncJobCount */

/*----------------------------------------------------------------------------*/
void Host_DecJobCount (int hostId)

{ Prolog;
  if (hostId <= 0 || NOFHOST < hostId) E(4008);
  if (host[hostId].nofJob > 0) host[hostId].nofJob--;
  Epilog(4009);
} /* Host_DecJobCount */

/*----------------------------------------------------------------------------*/
void Host_Rebooted (int hostId)

{ Prolog;
  if (hostId <= 0 || NOFHOST < hostId) E(4010);
  Host_InitStruct(hostId);
  host[hostId].state = FREE;
  OSD_InquireHostState(hostId);
  Epilog(4011);
} /* Host_Rebooted */

/*----------------------------------------------------------------------------*/
void Host_SetState (int hostId, int state)

{ Prolog;
  if (hostId <= 0 || NOFHOST < hostId) E(4012);
  if (state != FREE && state != OCCUPIED) E(4013);
  host[hostId].state = state;
  if (state == FREE)
    host[hostId].silent = 0;
} /* Host_SetState */

/*----------------------------------------------------------------------------*/
void Host_SetLoad (int hostId, int load)

{ Prolog;
  if (hostId <= 0 || NOFHOST < hostId) E(4014);
  host[hostId].load = load;
} /* Host_SetLoad */

/*----------------------------------------------------------------------------*/
void Host_SetSlotCnt (int hostId, unsigned int nofSlot, 
                      unsigned int nofIdleSlot)

{ unsigned int dSlot, dIdleSlot;

  Prolog;
  if (hostId <= 0 || NOFHOST < hostId) E(4015);
  dSlot = nofSlot - host[hostId].nofSlot;
  dIdleSlot = nofIdleSlot - host[hostId].nofIdleSlot;
  host[hostId].nofSlot = nofSlot;
  host[hostId].nofIdleSlot = nofIdleSlot;
  host[hostId].nofInquireState = 0;
  if (dSlot) {
    host[hostId].load /= LOADREDUCTION;
    host[hostId].load += ((dSlot - dIdleSlot) * MAXLOAD + dSlot / 2) / dSlot;
    if (host[hostId].load > MAXLOAD) host[hostId].load = MAXLOAD;
    else if (host[hostId].load < 0)  host[hostId].load = 0;
  }
  Epilog(4016);
} /* Host_SetSlotCnt */

/*----------------------------------------------------------------------------*/
void Host_SetSilent (int hostId, int silent)

{ Prolog;
  if (hostId <= 0 || NOFHOST < hostId) E(4017);
  host[hostId].silent = silent;
} /* Host_SetSilent */

/*----------------------------------------------------------------------------*/
void Host_ReduceSilent ()

{ int hostId;

  Prolog;
  for (hostId = 1; hostId <= NOFHOST; hostId++)
    if (host[hostId].silent > 0)
      host[hostId].silent--;
} /* Host_ReduceSilent */

/*----------------------------------------------------------------------------*/
void Host_AddCpuTime (int hostId, long cpuTime)

{ Prolog;
  if (hostId <= 0 || NOFHOST < hostId) E(4018);
  host[hostId].cpuTime += cpuTime;
} /* Host_AddCpuTime */

/*----------------------------------------------------------------------------*/
void Host_ReduceCpuTime ()

{ int hostId;

  Prolog;
  for (hostId = 1; hostId <= NOFHOST; hostId++) {
    host[hostId].cpuTime -= host[hostId].cpuTime / CPUTIMEREDUCTION / 2 + 1;
    if (host[hostId].cpuTime < 0)
      host[hostId].cpuTime = 0;
  }
} /* Host_ReduceCpuTime */

/*----------------------------------------------------------------------------*/
void Host_GetFreeList   (int hostSetList[NOFEXELEVEL][HOSTSETLEN],
                         int *nofLevel)

{ int exeLevel;
  int hostId;
  int empty;
  int i;

  Host_AdjustExeLevel();
  i = 0;
  for (exeLevel = 0; exeLevel < NOFEXELEVEL; exeLevel++) {
    Base_EmptyHostSet(hostSetList[i]);
    empty = TRUE;
    for (hostId = 1; hostId <= NOFHOST; hostId ++)
      if (host[hostId].exeLevel == exeLevel) {
        Base_InclHostSet(hostId, hostSetList[i]);
        empty = FALSE;
      }
    if (!empty) i++;
  }
  *nofLevel = i;
} /* Host_GetFreeList */

/*----------------------------------------------------------------------------*/
void Host_StateToString (int hostId, char *str)

{ Prolog;
  if (hostId <= 0 || NOFHOST < hostId) E(4019);
  switch (host[hostId].state) {
    case DOWN:      strcpy(str, "down       "); break;
    case MAYBEDOWN: strcpy(str, "maybe down "); break;
    case FREE:      strcpy(str, "free       "); break;
    case OCCUPIED:
      if (host[hostId].silent)
                    sprintf(str,"silent %2dh ", host[hostId].silent);
      else          strcpy(str, "interactive");
      break;
    default:        strcpy(str, "?          ");
  }
} /* Host_StateToString */

/*----------------------------------------------------------------------------*/
int Host_State (int hostId)

{ Prolog NONE;
  if (hostId <= 0 || NOFHOST < hostId) ER(4020, NONE);
  if (host[hostId].state == OCCUPIED && !host[hostId].silent)
    return FREE;
  else
    return host[hostId].state;
} /* Host_State */

/*----------------------------------------------------------------------------*/
