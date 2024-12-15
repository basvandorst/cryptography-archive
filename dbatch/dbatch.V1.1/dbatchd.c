#include <stdio.h>
#include <stdlib.h>

#include <fcntl.h>
#include "dbatch.h"

/*--------------------------------------------- SYSTEM CALLS -----------------*/
int close (int des);

/*----------------------------------------------------------------------------*/
int ok;

/*----------------------------------------------------------------------------*/
void SaveState ()

{ int fd;

  Prolog;
  if ((fd = creat(SAVEFILE, 0644)) < 0) E(3000);
  User_SaveState(fd);
  Host_SaveState(fd);
  Job_SaveState(fd);
  close(fd);
  Epilog(3001);
} /* SaveState */

/*----------------------------------------------------------------------------*/
void LoadState ()

{ int fd;

  Prolog;
  if ((fd = open(SAVEFILE, O_RDONLY)) < 0) E(3002);
  User_LoadState(fd);
  Host_LoadState(fd);
  Job_LoadState(fd);
  close(fd);
  Epilog(3003);
} /* LoadState */

/*----------------------------------------------------------------------------*/
void HandleMessage (int connFD)

{ struct Message message;
  struct Package *package;
  struct Package textBuffer;
  int hostId;
  int i;
  int inserted;
  int deleted;
  char line[124];

  Prolog;
  IPC_GetMessage(connFD, &message); T(3004);
  switch (message.type) {

  case SHOWSTATE:
#ifdef DEBUG
printf("SHOWSTATE: userId = %d, what = %d\n", message.userId, message.what);
#endif
    IPC_OpenText(&textBuffer);
    if (message.what == SHOWUSERSTATE)
      User_SendState(connFD, message.userId, &textBuffer);
    else if (message.what == SHOWJOBSTATE)
      Job_SendState(connFD, message.userId, &textBuffer);
    else if (message.what == SHOWHOSTSTATE)
      Host_SendState(connFD, message.userId, &textBuffer);
    else {
      for (i = 0; i < 80; i++) line[i] = '-';
      line[i++] = '\n';
      line[i] = '\0';
      IPC_WriteText(connFD, &textBuffer, line);
      if (message.what & SHOWUSERSTATE) {
        User_SendState(connFD, message.userId, &textBuffer);
        IPC_WriteText(connFD, &textBuffer, line);
      }
      if (message.what & SHOWJOBSTATE) {
        Job_SendState(connFD, message.userId, &textBuffer);
        IPC_WriteText(connFD, &textBuffer, line);
      }
      if (message.what & SHOWHOSTSTATE) {
        Host_SendState(connFD, message.userId, &textBuffer);
        IPC_WriteText(connFD, &textBuffer, line);
      }
    }
    IPC_CloseText(connFD, &textBuffer);
    IPC_GetMessage(connFD, &message); T(3005);
    if (message.type != DONE) E(3006);
    break;

  case ADDJOB:
#ifdef DEBUG
printf("ADDJOB: userId = %d, name = %s, hostSet[0] = %x\n", message.userId,
message.userName, message.hostSet[0]);
#endif
    IPC_GetPackageList(connFD, &package); T(3007);
    User_SetUserName(message.userId, message.userName);
    Job_Insert(message.userId, message.userName, message.hostSet, package, 
               &inserted);
    message.type = (ok && inserted) ? OK : NOTOK;
    ok = TRUE;
    IPC_PutMessage(connFD, &message);
    IPC_GetMessage(connFD, &message); T(3008);
    if (message.type != DONE) E(3009);
    break;

  case DELJOB:
#ifdef DEBUG
printf("DELJOB: jobId = %d, userId = %d\n", message.jobId, message.userId);
#endif
    Job_Delete(message.userId, message.jobId, &deleted);
    message.type = (ok && deleted) ? OK : NOTOK;
    ok = TRUE;
    IPC_PutMessage(connFD, &message);
    IPC_GetMessage(connFD, &message); T(3010);
    if (message.type != DONE) E(3011);
    break;

  case SENDJOB:
#ifdef DEBUG
printf("SENDJOB: jobId = %d, hostId = %d\n", message.jobId, message.hostId);
#endif
    message.type = (Job_State(message.jobId) == PENDING) ? OK : NOTOK;
    IPC_PutMessage(connFD, &message);
    if (message.type == OK) {
      Job_SendJob(connFD, message.jobId);
      IPC_GetMessage(connFD, &message); T(3012);
      if (message.type != JOBSTARTED) E(3013);
      Job_SetHostId(message.jobId, message.hostId);
      Job_SetProcGroupId(message.jobId, message.procGroupId);
      Job_ChangeState(message.jobId, message.hostId, PENDING, RUNNING); T(3014);
      if (message.procGroupId <= 1) E(3015);
    }
    else {
      IPC_GetMessage(connFD, &message); T(3016);
      if (message.type != DONE) E(3017);
    }
    break;

  case JOBDONE:
#ifdef DEBUG
printf("JOBDONE: jobId = %d, hostId = %d, userId = %d, cpuTime = %ld, nofSlot = %u, nofIdleSlot = %u, load = %d\n", message.jobId, message.hostId, message.userId, message.cpuTime, message.nofSlot, message.nofIdleSlot, message.load);
#endif
    User_AddCpuTime(message.userId, message.cpuTime);
    Host_AddCpuTime(message.hostId, message.cpuTime);
    Host_SetSlotCnt(message.hostId, message.nofSlot, message.nofIdleSlot);
    Host_SetLoad(message.hostId, message.load);
    Job_ChangeState(message.jobId, message.hostId, ALL, FINISHED);
    break;

  case JOBSTATE:
#ifdef DEBUG
printf("JOBSTATE: jobId = %d, hostId = %d, jobState = %d",message.jobId, message.hostId, message.state);
if (message.state == FINISHED)
  printf("nofSlot = %u, nofIdleSlot = %u, load = %d", message.nofSlot, message.nofIdleSlot, message.load);
printf("\n");
#endif
    if (message.state == FINISHED) {
      Host_SetSlotCnt(message.hostId, message.nofSlot, message.nofIdleSlot);
      Host_SetLoad(message.hostId, message.load);
    }
    Job_ChangeState(message.jobId,message.hostId,RUNNING|STOPPED,message.state);
    if (message.state == STOPPED && Host_State(message.hostId) == FREE)
      Job_ChangeState(message.jobId, message.hostId, STOPPED, RUNNING);
    else if (message.state == RUNNING && Host_State(message.hostId) == OCCUPIED)
      Job_ChangeState(message.jobId, message.hostId, RUNNING, STOPPED);
    break;

  case HOSTSTATE:
#ifdef DEBUG
printf("HOSTSTATE: hostId = %d, hostState = %d\n",message.hostId,message.state);
#endif
    Host_SetState(message.hostId, message.state);
    break;

  case FULLHOSTSTATE:
#ifdef DEBUG
printf("FULLHOSTSTATE: hostId = %d, hostState = %d, nofSlot = %u, nofIdleSlot = %u\n", message.hostId, message.state, message.nofSlot, message.nofIdleSlot);
#endif
    Host_SetState(message.hostId, message.state);
    Host_SetSlotCnt(message.hostId, message.nofSlot, message.nofIdleSlot);
    break;

  case HOSTSILENT:
#ifdef DEBUG
printf("HOSTSILENT: hostId = %d, silent = %d\n", message.hostId,message.silent);
#endif
    Host_SetState(message.hostId, OCCUPIED);
    Host_SetSilent(message.hostId, message.silent);
    switch (Host_State(message.hostId)) {
      case OCCUPIED:
        Job_ChangeState(ALL, message.hostId, RUNNING, STOPPED);
        break;
      case FREE:
        Job_ChangeState(ALL, message.hostId, STOPPED, RUNNING);
    }
    break;

  case HOSTREBOOTED:
#ifdef DEBUG
printf("HOSTREBOOTED: hostId = %d, nofSlot = %u, nofIdleSlot = %u\n", message.hostId, message.nofSlot, message.nofIdleSlot);
#endif
    Host_Rebooted(message.hostId);
    Host_SetSlotCnt(message.hostId, message.nofSlot, message.nofIdleSlot);
    Host_SetLoad(message.hostId, MAXLOAD);
    Job_ChangeState(ALL, message.hostId, ALL, WAITING);
    break;

  case CHECKSTATE:
#ifdef DEBUG
printf("CHECKSTATE\n");
#endif
    User_ReduceCpuTime();
    Host_ReduceCpuTime();
    Host_ReduceSilent();
    Host_CheckState();
    for (hostId = 1; hostId <= NOFHOST; hostId++) {
      switch (Host_State(hostId)) {
        case OCCUPIED:
          Job_ChangeState(ALL, hostId, RUNNING, STOPPED);
          break;
        case FREE:
          Job_ChangeState(ALL, hostId, STOPPED, RUNNING);
          break;
        case DOWN:
          Job_ChangeState(ALL, hostId, ALL, WAITING);
      }
    }
    Job_CheckState();
    break;

  case SAVESTATE:
#ifdef DEBUG
printf("SAVESTATE\n");
#endif
    SaveState();
    break;

  case SHUTDOWN:
#ifdef DEBUG
printf("SHUTDOWN\n");
#endif
    SaveState();
    exit(0);

  default:
#ifdef DEBUG
printf("WRONG MESSAGE: %d\n", message.type);
#endif
    E(3018);
    break;
  }
  Epilog(3019);
} /* HandleMessage */

/*----------------------------------------------------------------------------*/
int main (int argc)

{ int listenFD;

  if (argc != 1) ER(3020, -1);
  ok = TRUE;
  Base_Init();
  User_Init();
  Host_Init();
  Job_Init();
  LoadState();
  if (!ok) { 
    ok = TRUE;
    User_Init();
    Host_Init();
    Job_Init();
  }
  IPC_OpenListen(&listenFD); TR(3021, -1);
  Host_CheckState();
  Job_CheckState(); TR(3022, -1);
  for (;;) IPC_Listen(listenFD, HandleMessage, Job_StartNext);
}

/*----------------------------------------------------------------------------*/
