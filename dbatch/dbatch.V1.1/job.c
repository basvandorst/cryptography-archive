#include <signal.h>
#include <stdio.h>
#include <string.h>

#include "dbatch.h"

/*--------------------------------------------- SYSTEM CALLS -----------------*/
int read(int fd, char *buf, int nbyte);
int write(int fd, char *buf, int nbyte);

/*----------------------------------------------------------------------------*/
struct Job {
  int next;
  int state;
  int userId;
  int hostId;
  int procGroupId;
  int hostSet[HOSTSETLEN];
  char userName[MAXUSERNAMELEN];
  struct Package *package;
};

static struct Job job[MAXNOFJOB + 1];
static int jobPtr;
static int first;
static int last;

/*----------------------------------------------------------------------------*/
void Job_Init ()

{ int jobId;

  Prolog;
  for (jobId = 0; jobId <= MAXNOFJOB; jobId++)
    job[jobId].state = NONE;
  jobPtr = 0;
  first = 0;
  last = 0;
} /* Job_Init */

/*----------------------------------------------------------------------------*/
void Job_SaveState (int fd)

{ int jobId;
  struct Package *package;

  Prolog;
  if (write(fd, (char *)job, sizeof job) != sizeof job) E(6000);
  if (write(fd, (char *)&jobPtr, sizeof jobPtr) != sizeof jobPtr) E(6001);
  if (write(fd, (char *)&first, sizeof first) != sizeof first) E(6002);
  if (write(fd, (char *)&last, sizeof last) != sizeof last) E(6003);
  for (jobId = first; jobId != 0; jobId = job[jobId].next) {
    package = job[jobId].package;
    if (write(fd, (char *)&package, sizeof package) != sizeof package) E(6004);
    while (package != NULL) {
      if (write(fd, (char *)package, sizeof *package) != sizeof *package)
        E(6005);
      package = package->next;
    }
  }
} /* Job_SaveState */

/*----------------------------------------------------------------------------*/
void Job_LoadState (int fd)

{ int jobId;
  struct Package *package;

  Prolog;
  if (read(fd, (char *)job, sizeof job) != sizeof job) E(6006);
  if (read(fd, (char *)&jobPtr, sizeof jobPtr) != sizeof jobPtr) E(6007);
  if (read(fd, (char *)&first, sizeof first) != sizeof first) E(6008);
  if (read(fd, (char *)&last, sizeof last) != sizeof last) E(6009);
  for (jobId = first; jobId != 0; jobId = job[jobId].next) {
    if (read(fd, (char *)&package, sizeof package) != sizeof package) E(6010);
    if (package != NULL && (package = Base_NewPackage()) == NULL) E(6011);
    job[jobId].package = package;
    while (package != NULL) {
      if (read(fd, (char *)package, sizeof *package) != sizeof *package)
        E(6012);
      if (package->next != NULL && (package->next = Base_NewPackage()) == NULL)
        E(6013);
      package = package->next;
    }
  }
} /* Job_LoadState */

/*----------------------------------------------------------------------------*/
void Job_SendState (int connFD, int userId, struct Package *textBuffer)

{ int jobId;
  int i;
  char str[1024];
  char hostList[1024];
  
  Prolog;
  if (first == 0) {
    strcpy(str, "no entries in job queue\n");
    IPC_WriteText(connFD, textBuffer, str);
  }
  else {
    strcpy(str, "Job Owner    State   Host     Hoststate     PID Command\n");
    IPC_WriteText(connFD, textBuffer, str);
    for (jobId = first; jobId != 0; jobId = job[jobId].next) {
      sprintf(str, "%3d %-9s", jobId, job[jobId].userName);
      switch (job[jobId].state) {
        case WAITING: strcat(str, "waiting"); break;
        case PENDING: strcat(str, "pending"); break;
        case RUNNING: strcat(str, "running"); break;
        case STOPPED: strcat(str, "stopped"); break;
        case DELETED: strcat(str, "deleted"); break;
        case FINISHED:strcat(str, "finish "); break;
        default:      strcat(str, "?      ");
      }
      switch (job[jobId].state) {
        case PENDING: case RUNNING: case STOPPED:
          Base_HostIdToName(job[jobId].hostId, hostList);
          sprintf(str, "%s %-9s", str, hostList);
          Host_StateToString(job[jobId].hostId, &str[strlen(str)]);
          sprintf(str, "%s%6d ", str, job[jobId].procGroupId);
          break;
        default:
          if (userId != ROOT)
            Base_HostSetToShortString(job[jobId].hostSet, hostList);
          else {
            hostList[0] = '\0';
            for (i = HOSTSETLEN - 1; i >= 0; i--)
              sprintf(hostList, "%s%08X ", hostList, job[jobId].hostSet[i]);
          }
          hostList[26] = '\0';
          sprintf(str, "%s %-26s ", str, hostList);
      }
      strncat(str, job[jobId].package->buf, 80 - strlen(str));
      str[80] = '\0';
      strcat(str, "\n");
      IPC_WriteText(connFD, textBuffer, str);
    }
  }
  Epilog(6014);
} /* Job_SendState */

/*----------------------------------------------------------------------------*/
void Job_SendJob (int connFD, int jobId)

{ Prolog;
  if (jobId <= 0 || MAXNOFJOB < jobId) E(6015);
  IPC_PutPackageList(connFD, job[jobId].package);
  Epilog(6016);
} /* Job_SendJob */

/*----------------------------------------------------------------------------*/
void Job_CheckState ()

{ int jobId;

  Prolog;
  for (jobId = first; jobId != 0; jobId = job[jobId].next) {
    switch (job[jobId].state) {
      case PENDING:
        Job_ChangeState(jobId, job[jobId].hostId, PENDING, WAITING);
        break;

      case RUNNING: case STOPPED:
        if (job[jobId].procGroupId <= 1) E(6017);
        OSD_InquireJobState(jobId, job[jobId].hostId, job[jobId].procGroupId);
        break;
    }
  }
  Epilog(6018);
} /* Job_CheckState */

/*----------------------------------------------------------------------------*/
void Job_Insert (int userId, char *userName, int hostSet[],
                 struct Package *package, int *inserted)

{ int i, oldJobPtr;

  Prolog;
  *inserted = FALSE;
  if (User_JobCount(userId) >= MAXNOFJOBPERUSER) return;
  oldJobPtr = jobPtr;
  jobPtr = (jobPtr >= MAXNOFJOB) ? 1 : (jobPtr + 1);
  while ((jobPtr != oldJobPtr) && (job[jobPtr].state != NONE))
    jobPtr = (jobPtr >= MAXNOFJOB) ? 1 : (jobPtr + 1);
  if (job[jobPtr].state != NONE) return;
  job[jobPtr].next = 0;
  if (last != 0) job[last].next = jobPtr;
  last = jobPtr;
  if (first == 0) first = jobPtr;
  job[jobPtr].state = WAITING;
  job[jobPtr].userId = userId;
  User_IncJobCount(userId);
  User_IncWaitCount(userId);
  strcpy(job[jobPtr].userName, userName);
  for (i = 0; i < HOSTSETLEN; i++) job[jobPtr].hostSet[i] = hostSet[i];
  job[jobPtr].package = package;
  Job_SetHostId(jobPtr, NONE);
  Job_SetProcGroupId(jobPtr, NONE);
  *inserted = ok;
  Epilog(6019);
} /* Job_Insert */

/*----------------------------------------------------------------------------*/
void Job_Delete (int userId, int jobId, int *deleted)

{ Prolog;
  *deleted = TRUE;
  if (userId == ROOT)
    Job_ChangeState(jobId, ALL, ALL, DELETED);
  else if (jobId == ALL) {
    for (jobId = 1; jobId <= MAXNOFJOB; jobId++)
      if (job[jobId].state != NONE && job[jobId].userId == userId)
        Job_ChangeState(jobId, ALL, ALL, DELETED);
  }
  else if (*deleted = userId == job[jobId].userId)
    Job_ChangeState(jobId, ALL, ALL, DELETED);
  Epilog(6020);
} /* Job_Delete */

/*----------------------------------------------------------------------------*/
static void Job_GetNextJob (int *jobId, int *hostId)

{ int joinSet[HOSTSETLEN];
  int hostSetList[NOFEXELEVEL][HOSTSETLEN];
  int nofLevel;
  int userId;
  int level;

  Prolog;
  *hostId = *jobId = NONE;
  if ((userId = User_First()) == NONE) return;
  Host_GetFreeList(hostSetList, &nofLevel);
  if (!nofLevel) return;
  for (; userId != NONE; userId = User_Next())
    for (level = 0; level < nofLevel; level++)
      for (*jobId = first; *jobId != 0; *jobId = job[*jobId].next)
        if (job[*jobId].userId == userId && job[*jobId].state == WAITING) {
          Base_JoinHostSet(hostSetList[level], job[*jobId].hostSet, joinSet);
          *hostId = Base_FirstInHostSet(joinSet);
          if (*hostId != NONE) return;
        }
  *hostId = *jobId = NONE;
} /* Job_GetNextJob */

/*----------------------------------------------------------------------------*/
void Job_StartNext ()

{ int jobId;
  int hostId;

  Prolog;
  Job_GetNextJob(&jobId, &hostId);
  if (jobId != NONE) {
    Job_SetHostId(jobId, hostId);
    Job_ChangeState(jobId, hostId, WAITING, PENDING);
  }
  Epilog(6021);
} /* Job_StartNext */

/*----------------------------------------------------------------------------*/
static void Job_RemoveFromQueue (int jobId)

{ int prev;
  int curr;

  Prolog;
  if (jobId <= 0 || MAXNOFJOB < jobId) E(6022);
  prev = 0;
  curr = first;
  while (curr != 0 && curr != jobId) {
    prev = curr;
    curr = job[curr].next;
  }
  if (curr != jobId) E(6023);
  if (prev == 0) {
    if (first == last) first = last = 0;
    else first = job[first].next;
  }
  else {
    job[prev].next = job[curr].next;
    if (last == curr) last = prev;
  }
  if (job[jobId].state == WAITING) User_DecWaitCount(job[jobId].userId);
  User_DecJobCount(job[jobId].userId);
  job[jobId].state = NONE;
  Job_SetHostId(jobId, NONE);
  Job_SetProcGroupId(jobId, NONE);
  job[jobId].userId = NONE;
  Base_DisposePackageList(job[jobId].package);
  Epilog(6024);
} /* Job_RemoveFromQueue */

/*----------------------------------------------------------------------------*/
void Job_ChangeState (int jobId, int hostId, int srcStateSet, int destState)

{ Prolog;
  if (jobId == ALL) {
    for (jobId = 1; jobId <= MAXNOFJOB; jobId++)
      if (job[jobId].state != NONE)
        Job_ChangeState(jobId, hostId, srcStateSet, destState);
  }
  else {
    if (jobId <= 0 || MAXNOFJOB < jobId) E(6025);
    if (!(job[jobId].state & srcStateSet) ||
        hostId != ALL && hostId != job[jobId].hostId) return;
    if (job[jobId].state == destState) return;

    switch (destState) {
      case WAITING:
        switch (job[jobId].state) {
          case PENDING: case RUNNING: case STOPPED:
            job[jobId].state = WAITING;
            User_IncWaitCount(job[jobId].userId);
            Job_SetHostId(jobId, NONE);
            break;

          default: E(6026);
        }
        break;

      case PENDING:
        switch (job[jobId].state) {
          case WAITING:
            job[jobId].state = PENDING;
            User_DecWaitCount(job[jobId].userId);
            OSD_InquireStartJob(jobId, job[jobId].hostId);
            break;

          default: E(6027);
        }
        break;

      case RUNNING:
        switch (job[jobId].state) {
          case PENDING:
            if (job[jobId].procGroupId <= 1) E(6028);
            job[jobId].state = RUNNING;
            if (Host_State(job[jobId].hostId) == OCCUPIED)
              Job_ChangeState(jobId, job[jobId].hostId, RUNNING, STOPPED);
            break;

          case STOPPED:
            if (job[jobId].procGroupId <= 1) E(6029);
            OSD_SendSignal(job[jobId].hostId, job[jobId].procGroupId, SIGCONT);
            job[jobId].state = RUNNING;
            break;

          default: E(6030);
        }
        break;

      case STOPPED:
        switch (job[jobId].state) {
          case RUNNING:
            if (job[jobId].procGroupId <= 1) E(6031);
            OSD_SendSignal(job[jobId].hostId, job[jobId].procGroupId, SIGSTOP);
            job[jobId].state = STOPPED;
            break;

          default: E(6032);
        }
        break;

      case DELETED:
        switch (job[jobId].state) {
          case RUNNING: case STOPPED:
            if (job[jobId].procGroupId > 1)
              OSD_SendSignal(job[jobId].hostId, job[jobId].procGroupId,SIGKILL);

          case WAITING: case PENDING:
            Job_RemoveFromQueue(jobId);
            break;

          default: E(6033);
        }
        break;

      case FINISHED:
        switch (job[jobId].state) {
          case WAITING: case RUNNING: case STOPPED:
            Job_RemoveFromQueue(jobId);
            break;

          default: E(6034);
        }
        break;

      default: E(6035);
    }
  }
  Epilog(6036);
} /* Job_ChangeState */

/*----------------------------------------------------------------------------*/
void Job_SetHostId (int jobId, int hostId)

{ Prolog;
  if (jobId <= 0 || MAXNOFJOB < jobId) E(6037);
  if (job[jobId].hostId != hostId) {
    if (job[jobId].hostId != NONE)
      Host_DecJobCount(job[jobId].hostId);
    if (hostId != NONE)
      Host_IncJobCount(hostId);
    job[jobId].hostId = hostId;
  }
} /* Job_SetHostId */

/*----------------------------------------------------------------------------*/
void Job_SetProcGroupId (int jobId, int procGroupId)

{ Prolog;
  if (jobId <= 0 || MAXNOFJOB < jobId) E(6038);
  if (job[jobId].state != NONE && job[jobId].state != WAITING && procGroupId <= 1) E(6039);
  job[jobId].procGroupId = procGroupId;
} /* Job_SetProcGroupId */

/*----------------------------------------------------------------------------*/
int Job_State (int jobId)

{ Prolog NONE;
  if (jobId <= 0 || MAXNOFJOB < jobId) ER(6040, NONE);
  return job[jobId].state;
} /* Job_State */

/*----------------------------------------------------------------------------*/

