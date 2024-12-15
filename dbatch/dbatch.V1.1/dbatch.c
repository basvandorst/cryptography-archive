#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <sys/param.h>
#include "dbatch.h"

/*--------------------------------------------- SYSTEM CALLS -----------------*/
int fork();

/*--------------------------------------------- C LIBRARY FUNCTIONS ----------*/
void sleep(unsigned seconds);

/*----------------------------------------------------------------------------*/
#define SYMNAME        0
#define SYMSTRING      1
#define SYMINREDIR     2
#define SYMOUTREDIR    3
#define SYMERRREDIR    4
#define SYMOUTERRREDIR 5
#define SYMEND         6

#define EOFPACKAGE     0x02
#define EOFSTRINGARRAY 0x01

struct JobDesc {
  int userId;
  int groupId;
  char *groupList;
  char *workDir;
  char *stdIn;
  char *stdOut;
  char *stdErr;
  char **env;
  char **arg;
  char **resArg;
  char *envTab[MAXNOFENV];
  char *argTab[MAXNOFARG];
  char *resArgTab[MAXNOFARG];
};

char *symStr;
int sym;
int ok;

/*----------------------------------------------------------------------------*/
void UsageError ()

{ printf(
"\nusage:\n\
  dbatch [ -jun ]                    show job, user and network informations\n\
  dbatch -s numHours                 interrupt execution of jobs on your host\n\
  dbatch -d { jobId } | ALL                              delete jobs\n\
  dbatch [ -h \"hostList\" ] [ ansProgram ] \"jobDesc\"      add a new job\n\n\
  hostList    = { hostName [ .. hostName ] }\n\
  jobDesc     = program { parameter } { redirection }\n\
  redirection = < inFile | > outFile | 2> errFile | >& outErrFile\n\n");
  exit(-1);
} /* UsageError */

/*----------------------------------------------------------------------------*/
void ShowPackageList(struct Package *package)

{ int i, j;
  char ch;

  Prolog;
  while (package != NULL) {
    printf("\n#####################################\n");
    for (i = 0; i < PACKAGEBUFSIZE; i++) {
      printf("%2X ", (*package).buf[i]);
      if ((i % 20) == 19) { 
        for (j = 19; j >=0; j--) {
          ch = (*package).buf[i-j];
          if ((ch < ' ') || (ch > '~')) ch = '.';
          printf("%c", ch);
        }
        printf("\n");
      }
    }
    package = (*package).next;
  }
  Epilog(2000);
} /* ShowPackageList */

/*----------------------------------------------------------------------------*/
void ShowJobDesc (struct JobDesc job)

{ int i;

  Prolog;
  printf("userId = %d, groupId = %d\n", job.userId, job.groupId);
  printf("groupList = %s\n", job.groupList);
  printf("workDir= %s\n", job.workDir);
  printf("stdin  = %s\n", job.stdIn);
  printf("stdout = %s\n", job.stdOut);
  printf("stderr = %s\n", job.stdErr);
  for (i = 0; job.env[i] != NULL; i++)
    printf("env[%d] = %s\n", i, job.env[i]);
  for (i = 0; job.arg[i] != NULL; i++)
    printf("arg[%d] = %s\n", i, job.arg[i]);
  for (i = 0; job.resArg[i] != NULL; i++)
    printf("resArg[%d] = %s\n", i, job.resArg[i]);
  Epilog(2001);
} /* ShowJobDesc */

/*----------------------------------------------------------------------------*/
void AddString (char *str, struct Package **package, int *currLen)

{ int len;

  Prolog;
  len = strlen(str) + 2;
  if (len > PACKAGEBUFSIZE) E(2002);
  if ((*currLen + len) > PACKAGEBUFSIZE) { /* *currLen > 0 */
    (*package)->buf[*currLen - 1] |= EOFPACKAGE;
    if (((*package)->next = Base_NewPackage()) == NULL) E(2003);
    *package = (*package)->next;
    *currLen = 0;
  }
  strcpy(&((*package)->buf[*currLen]), str);
  *currLen += len;
  (*package)->buf[*currLen - 1] = 0;
} /* AddString */

/*----------------------------------------------------------------------------*/
void AddStringArray (char **strArray, struct Package **package, int *currLen)

{ Prolog;
  AddString("", package, currLen); /* write null-string */
  while (*strArray != NULL) {
    AddString(*strArray, package, currLen);
    strArray++;
  }
  (*package)->buf[*currLen - 1] |= EOFSTRINGARRAY;
} /* AddStringArray */

/*----------------------------------------------------------------------------*/
void PackJob (struct JobDesc *jobDesc, struct Package *package)

{ char str[1024];
  int currLen = 0;
  int i;

  Prolog;
  i = strlen(jobDesc->arg[0]);
  while ((i > 0) && (jobDesc->arg[0][i - 1] != '/')) i--;
  strcpy(str, &jobDesc->arg[0][i]);
  for (i = 1; jobDesc->arg[i] != NULL; i++){
    strcat(str, " ");
    strcat(str, jobDesc->arg[i]);
  }
  i = strlen(jobDesc->stdIn);
  while ((i > 0) && (jobDesc->stdIn[i - 1] != '/')) i--;
  strcat(str, " <");
  strcat(str, &jobDesc->stdIn[i]);
  AddString(str, &package, &currLen);
  sprintf(str, "%d", jobDesc->userId);
  AddString(str, &package, &currLen);
  sprintf(str, "%d", jobDesc->groupId);
  AddString(str, &package, &currLen);
  AddString(jobDesc->groupList, &package, &currLen);
  AddString(jobDesc->workDir, &package, &currLen);
  AddString(jobDesc->stdIn, &package, &currLen);
  AddString(jobDesc->stdOut, &package, &currLen);
  AddString(jobDesc->stdErr, &package, &currLen);
  AddStringArray(jobDesc->env, &package, &currLen);
  AddStringArray(jobDesc->arg, &package, &currLen);
  AddStringArray(jobDesc->resArg, &package, &currLen);
  package->next = NULL;
  Epilog(2004);
} /* PackJob */

/*----------------------------------------------------------------------------*/
void NextString (struct Package **package, int *currLen, char **str)

{ Prolog;
  if ((*currLen > 0) && ((*package)->buf[*currLen - 1] & EOFPACKAGE)) {
    *package = (*package)->next;
    *currLen = 0;
  }
  *str = &((*package)->buf[*currLen]);
  *currLen += strlen(*str) + 2;
} /* NextString */

/*----------------------------------------------------------------------------*/
void NextStringArray (struct Package **package, int *currLen, char **str)

{ Prolog;
  NextString(package, currLen, str); /* read null-string */
  while (!((*package)->buf[*currLen - 1] & EOFSTRINGARRAY)) {
    NextString (package, currLen, str);
    str++;
  }
  *str = NULL;
} /* NextStringArray */

/*----------------------------------------------------------------------------*/
void UnpackJob (struct Package *package, struct JobDesc *jobDesc)

{ int currLen = 0;
  char *str;

  Prolog;
  NextString(&package, &currLen, &str); /* read command description */
  NextString(&package, &currLen, &str);
  sscanf(str, "%d", &jobDesc->userId);
  NextString(&package, &currLen, &str);
  sscanf(str, "%d", &jobDesc->groupId);
  NextString(&package, &currLen, &jobDesc->groupList);
  NextString(&package, &currLen, &jobDesc->workDir);
  NextString(&package, &currLen, &jobDesc->stdIn);
  NextString(&package, &currLen, &jobDesc->stdOut);
  NextString(&package, &currLen, &jobDesc->stdErr);
  NextStringArray(&package, &currLen, jobDesc->envTab);
  NextStringArray(&package, &currLen, jobDesc->argTab);
  NextStringArray(&package, &currLen, jobDesc->resArgTab);
  jobDesc->env = jobDesc->envTab;
  jobDesc->arg = jobDesc->argTab;
  jobDesc->resArg = jobDesc->resArgTab;
  Epilog(2005);
} /* UnpackJob */

/*----------------------------------------------------------------------------*/
void GetSym(char **in)

{ char ch;
  char last;

  Prolog;
  ch = **in;
  while ((ch != '\0') && (ch <= ' ') || ('~' < ch)) ch = *++*in;
  if (ch == '\0')
    sym = SYMEND;
  else if ((ch == '\"') || (ch == '\'')) {
    sym = SYMSTRING;
    last = ch;
    ch = *++*in;
    symStr = *in;
    while ((ch != last) && (ch != '\0')) ch = *++*in;
    if (ch != '\0') *(*in)++ = '\0';
    else { printf("*** missing %c in: '%s'\n", last, symStr); ok = FALSE; }
  }
  else if (ch == '2' && (*in)[1] == '>') { ++*in; ++*in; sym = SYMERRREDIR; }
  else if (ch == '>' && (*in)[1] == '&') {++*in; ++*in; sym = SYMOUTERRREDIR; }
  else if (ch == '<') { ++*in; sym = SYMINREDIR; }
  else if (ch == '>') { ++*in; sym = SYMOUTREDIR; }
  else {
    sym = SYMNAME;
    symStr = *in;
    while ((' ' < ch) && (ch <= '~')) {
      if ((ch == '<') || (ch == '>')) {
        printf("*** wrong character '%c' in: '%s'\n", ch, symStr);
        ok = FALSE;
        return;
      }
      ch = *++*in;
    }
    if (ch != '\0') *(*in)++ = '\0';
  }
} /* GetSym */
  
/*----------------------------------------------------------------------------*/
void CompleteFilename(char *in, char *out)

{ Prolog;
  if (*in != '/') strcpy(out, OS_WorkingDirectory());
  else *out = '\0';
  strcat(out, in);
} /* CompleteFilename */

/*----------------------------------------------------------------------------*/
void AddJob(char *hostList, char *resultProc, char *cmd, char **env)

{ int i;
  int connFD;
  int done = FALSE;
  int retry = 0;
  char fullCmd[1024];
  char fullRes[1024];
  char fullStdIn[1024];
  char fullStdOut[1024];
  char fullStdErr[1024];
  char fullGroupList[1024];
  struct JobDesc job;
  struct Message message;
  struct Package package;

  Prolog;
  strcpy(fullStdIn, "/dev/null");
  strcpy(fullStdOut, "/dev/null");
  strcpy(fullStdErr, "/dev/null");
  if (!strcmp(resultProc, ""))
    strcpy(fullRes, "");
  else {
    OS_GetExecutable(resultProc, fullRes);
    if (!strcmp(fullRes, "")) {
      printf("*** in $PATH is no executable file called '%s'\n", resultProc);
      exit(-1);
    }
  }

  GetSym(&cmd); if (!ok) UsageError();  /* scanner for command     */
  if (sym != SYMNAME) UsageError();
  OS_GetExecutable(symStr, fullCmd);
  if (!strcmp(fullCmd, "")) {
    printf("*** in $PATH is no executable file called '%s'\n", symStr);
    exit(-1);
  }
  GetSym(&cmd);
  for (i = 1; ok && ((sym == SYMSTRING) || (sym == SYMNAME)); GetSym(&cmd))
    job.argTab[i++] = symStr;
  job.argTab[i] = NULL;
  while (ok && (sym != SYMEND)) {
    if (sym == SYMINREDIR) {
      GetSym(&cmd);
      if (sym != SYMNAME) UsageError();
      else CompleteFilename(symStr, fullStdIn);
    }
    else if (sym == SYMOUTREDIR) {
      GetSym(&cmd);
      if (sym != SYMNAME) UsageError();
      else CompleteFilename(symStr, fullStdOut);
    }
    else if (sym == SYMERRREDIR) {
      GetSym(&cmd);
      if (sym != SYMNAME) UsageError();
      else CompleteFilename(symStr, fullStdErr);
    }
    else if (sym == SYMOUTERRREDIR) {
      GetSym(&cmd);
      if (sym != SYMNAME) UsageError();
      else {
        CompleteFilename(symStr, fullStdOut);
        CompleteFilename(symStr, fullStdErr);
      }
    }
    else UsageError();
    GetSym(&cmd);
  }
  if (!ok) UsageError();
  if (!OS_IsInFile(fullStdIn)) {
    printf("*** wrong input file '%s'\n", fullStdIn);
    exit(-1);
  }
  if (!OS_IsOutFile(fullStdOut)) {
    printf("*** wrong output file '%s'\n", fullStdOut);
    exit(-1);
  }
  if (!OS_IsOutFile(fullStdErr)) {
    printf("*** wrong error file '%s'\n", fullStdErr);
    exit(-1);
  }
  OS_GetGroupList(fullGroupList);

  job.userId = OS_UserId();
  job.groupId = OS_GroupId();
  job.groupList = fullGroupList;
  job.arg = job.argTab;
  job.argTab[0] = fullCmd;
  job.resArg = job.resArgTab;
  job.resArgTab[0] = fullRes;
  job.resArgTab[1] = NULL;
  job.env = env;
  job.workDir = OS_WorkingDirectory();
  job.stdIn = fullStdIn;
  job.stdOut = fullStdOut;
  job.stdErr = fullStdErr;
  PackJob(&job, &package);

  OS_UserIdToName(job.userId, message.userName); T(2006);
  if (!strcmp(hostList, "ALL"))
    Base_FullHostSet(message.hostSet);
  else
    Base_HostStringToSet(hostList, message.hostSet);
  if (!ok) { printf("*** wrong hostname in: '%s'\n", hostList); return; }

  do {
    if (retry) { printf("rerun of sending job to daemon.\n"); sleep(10); }
    ok = TRUE;
    message.type = ADDJOB;
    message.userId = job.userId;
    IPC_OpenConnection(&connFD);
    IPC_PutMessage(connFD, &message);
    IPC_PutPackageList(connFD, &package);
    IPC_GetMessage(connFD, &message);
    if (ok) retry = MAXNOFRETRY;
    done = ok && (message.type == OK);
    message.type = DONE;
    IPC_PutMessage(connFD, &message);
    IPC_CloseConnection(connFD);
  } while (++retry < MAXNOFRETRY);

  if (!done)
    printf("*** could not add job!\n");
  Epilog(2007);
  ok = ok && done;
} /* AddJob */

/*----------------------------------------------------------------------------*/
void DeleteJob(int jobId, int userId)

{ int connFD;
  int done;
  struct Message message;

  Prolog;
  message.type = DELJOB;
  message.jobId = jobId;
  message.userId = userId;
  IPC_OpenConnection(&connFD);
  IPC_PutMessage(connFD, &message);
  IPC_GetMessage(connFD, &message);
  done = message.type == OK;
  message.type = DONE;
  IPC_PutMessage(connFD, &message);
  IPC_CloseConnection(connFD);
  if (jobId == ALL)
    if (done) printf("all your jobs are deleted\n");
    else printf("*** could not delete all your jobs!\n");
  else
    if (done) printf("job %d is deleted\n", jobId);
    else printf("*** could not delete job %d!\n", jobId);
  Epilog(2008);
  ok = ok && done;
} /* DeleteJob */

/*----------------------------------------------------------------------------*/
void ShowState(int userId, int what)

{ int connFD;
  struct Message message;

  Prolog;
  message.type = SHOWSTATE;
  message.what = what;
  message.userId = userId;
  IPC_OpenConnection(&connFD);
  IPC_PutMessage(connFD, &message);
  IPC_ReadText(connFD);
  message.type = DONE;
  IPC_PutMessage(connFD, &message);
  IPC_CloseConnection(connFD);
  Epilog(2009);
} /* ShowState */

/*----------------------------------------------------------------------------*/
void StartJob(int jobId, int hostId)

{ int connFD;
  int i;
  int k;
  struct Message message;
  struct Package *package;
  struct JobDesc jd;
  int exitValue;
  long cpuTime = 0;
  char strExitValue[128];
  char strCpuTime[128];
  char stdInRedir[8];
  char stdOutRedir[8];
  char stdErrRedir[8];
  char hostName[MAXHOSTNAMELEN];
  char userName[MAXUSERNAMELEN];
  char str[4096];

  Prolog;
  IPC_OpenConnection(&connFD); T(2010);
  message.type = SENDJOB;
  message.jobId = jobId;
  message.hostId = hostId;
  IPC_PutMessage(connFD, &message);
  IPC_GetMessage(connFD, &message);
  if (message.type == OK) {
    IPC_GetPackageList(connFD, &package);
    message.type = JOBSTARTED;
    message.procGroupId = OS_ProcessId();
    IPC_PutMessage(connFD, &message);
    IPC_CloseConnection(connFD);

    UnpackJob(package, &jd);
    OS_SetSessionLeader(message.procGroupId);
    OS_StartJob(jd.workDir, jd.userId, jd.groupId, jd.groupList, jd.arg, jd.env,
                jd.stdIn, jd.stdOut, jd.stdErr);
    OS_WaitForChildren(&exitValue, &cpuTime);
    Base_HostIdToName(hostId, hostName);
    sprintf(strExitValue, "%d", exitValue);
    sprintf(strCpuTime, "%d", cpuTime);
    strcpy(stdInRedir, "<");
    strcpy(stdOutRedir, ">");
    strcpy(stdErrRedir, "2>");
    i = 1;
    k = 0;
    jd.resArg[i++] = strExitValue;
    jd.resArg[i++] = strCpuTime;
    jd.resArg[i++] = hostName;
    while (jd.arg[k] != NULL)
      jd.resArg[i++] = jd.arg[k++];
    jd.resArg[i++] = stdInRedir;
    jd.resArg[i++] = jd.stdIn;
    jd.resArg[i++] = stdOutRedir;
    jd.resArg[i++] = jd.stdOut;
    jd.resArg[i++] = stdErrRedir;
    jd.resArg[i++] = jd.stdErr;
    jd.resArg[i] = NULL;
    if (!strcmp(jd.resArg[0], "")) {
      sprintf(str, "exit value = %8d\ncpu time   = %6d s\nhost name  = %8s\ncommand    =", exitValue, cpuTime, hostName);
      for (i = 4; jd.resArg[i] != NULL; i++) {
        strcat(str, " ");
        strcat(str, jd.resArg[i]);
      }
      strcat(str, "\n");
      OS_UserIdToName(jd.userId, userName);
      OS_Mail(userName, "dbatch: job done", str);
    }
    else {
      OS_StartJob(jd.workDir, jd.userId, jd.groupId, jd.groupList, jd.resArg,
                  jd.env, "/dev/null", "/dev/null", "/dev/null");
      OS_WaitForChildren(&exitValue, &cpuTime);
    }
    message.type = JOBDONE;
    message.jobId = jobId;
    message.hostId = hostId;
    message.userId = jd.userId;
    message.cpuTime = cpuTime;
    OS_GetCpuUsage(&message.nofSlot, &message.nofIdleSlot, &message.load);
    IPC_OpenConnection(&connFD);
    IPC_PutMessage(connFD, &message);
    IPC_CloseConnection(connFD);
  }
  else {
    message.type = DONE;
    IPC_PutMessage(connFD, &message);
    IPC_CloseConnection(connFD);
  }
  Epilog(2011);
} /* StartJob */

/*----------------------------------------------------------------------------*/
void SendMessage (struct Message *message)

{ int connFD;

  Prolog;
  IPC_OpenConnection(&connFD);
  IPC_PutMessage(connFD, message);
  IPC_CloseConnection(connFD);
  Epilog(2012);
} /* SendMessage */

/*----------------------------------------------------------------------------*/
main (int argc, char *argv[], char **env)

{ int jobId = 0;
  int procGroupId = 0;
  int i;
  int what = 0;
  char hostName[MAXHOSTNAMELEN];
  struct Message message;

  ok = TRUE;
  Base_Init();
  OS_Init();
  if (OS_HostId() == NONE) {
    printf("*** 'dbatch' does not work on this machine! Alternatives are:\n");
    for (i = 1; i <= NOFHOST; i++)
      { Base_HostIdToName(i, hostName); printf(" %-9s", hostName); }
    printf("\n");
    exit(-1);
  }
  if (argc < 1)
    UsageError();
  else if (argc == 1)
    ShowState(OS_UserId(), SHOWJOBSTATE | SHOWHOSTSTATE);
  else if (*(argv[1]) == '-') {
    if (argc == 2)
      for (i = 1; argv[1][i] != '\0'; i++)
        if (argv[1][i] == 'j') what |= SHOWJOBSTATE;
        else if (argv[1][i] == 'n') what |= SHOWHOSTSTATE;
        else if (argv[1][i] == 'u') what |= SHOWUSERSTATE;
        else { what = 0; break; }
    if (argc == 2 && what != 0)
      ShowState(OS_UserId(), what);
    else if (argc == 2 && (!strcmp(argv[1], "-login") || 
                           !strcmp(argv[1], "-logout"))) {
      if (fork() <= 0) { /* child process */
        if (!strcmp(argv[1], "-logout")) sleep(30);
        message.type = HOSTSTATE;
        message.hostId = OS_HostId();
        message.state = OS_HostState();
        SendMessage(&message);
      }
    }
    else if (argc == 3 && !strcmp(argv[1], "-s")) {
      if (!OS_UserOnConsole())
        printf("*** you are not sitting in front of this machine\n");
      else {
        message.silent = 0;
        sscanf(argv[2], "%d", &message.silent);
        if (message.silent < 0) message.silent = 0;
        if (MAXSILENT < message.silent) message.silent = MAXSILENT;
        message.type = HOSTSILENT;
        message.hostId = OS_HostId();
        SendMessage(&message);
      }
    }
    else if (argc >= 2 && !strcmp(argv[1], "-d"))
      if (argc == 3 && !strcmp(argv[2], "ALL"))
        DeleteJob(ALL, OS_UserId());
      else
        for (i = 2; argv[i] != NULL; i++) {
          sscanf(argv[i], "%d", &jobId);
          if (0 < jobId && jobId <= MAXNOFJOB)
            DeleteJob(jobId, OS_UserId());
          else {
            printf("*** illegal job-id: '%s'\n", argv[i]);
            ok = FALSE;
          }
        }
    else if (argc == 4 && !strcmp(argv[1], "-h"))
      AddJob(argv[2], "", argv[3], env);
    else if (argc == 5 && !strcmp(argv[1], "-h"))
      AddJob(argv[2], argv[3], argv[4], env);

    else if (OS_UserId() == ROOT) {
      if (argc == 2 && !strcmp(argv[1], "-reboot")) {
        message.type = HOSTREBOOTED;
        message.hostId = OS_HostId();
        OS_GetSlotCnt(&message.nofSlot, &message.nofIdleSlot);
        SendMessage(&message);
      }
      else if (argc == 2 && !strcmp(argv[1], "-fullhoststate")) {
        message.type = FULLHOSTSTATE;
        message.hostId = OS_HostId();
        message.state = OS_HostState();
        OS_GetSlotCnt(&message.nofSlot, &message.nofIdleSlot);
        SendMessage(&message);
      }
      else if (argc == 2 && !strcmp(argv[1], "-checkstate")) {
        message.type = CHECKSTATE;
        SendMessage(&message);
      }
      else if (argc == 2 && !strcmp(argv[1], "-savestate")) {
        message.type = SAVESTATE;
        SendMessage(&message);
      }
      else if (argc == 2 && !strcmp(argv[1], "-shutdown")) {
        message.type = SHUTDOWN;
        SendMessage(&message);
      }
      else if (argc == 3 && !strcmp(argv[1], "-startjob")) {
        sscanf(argv[2], "%d", &jobId);
        if (jobId <= 0 || MAXNOFJOB < jobId) ER(2013, -1);
        if (fork() <= 0) {
          OS_SetInOutFiles("/dev/null", "/dev/null", "/dev/null");
          StartJob(jobId, OS_HostId());
        }
      } 
      else if (argc == 4 && !strcmp(argv[1], "-jobstate")) {
        sscanf(argv[2], "%d", &message.jobId);
        sscanf(argv[3], "%d", &procGroupId);
        if (message.jobId <= 0 || MAXNOFJOB < message.jobId || procGroupId <= 0)
          ER(2014, -1);
        message.type = JOBSTATE;
        message.hostId = OS_HostId();
        message.state = OS_ProcessState(procGroupId);
        if (message.state == FINISHED)
          OS_GetCpuUsage(&message.nofSlot, &message.nofIdleSlot, &message.load);
        SendMessage(&message);
      }
      else
        UsageError();
    }
    else
      UsageError();
  }
  else if (argc == 2)
    AddJob("ALL", "", argv[1], env);
  else if (argc == 3)
    AddJob("ALL", argv[1], argv[2], env);
  else
    UsageError();
  return (ok ? 0 : -1);
} /* main */

/*----------------------------------------------------------------------------*/
