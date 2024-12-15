#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <sys/time.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/param.h>
#include <rpcsvc/rstat.h>
#include <unistd.h>
#include <utmp.h>
#include <pwd.h>
#include "dbatch.h"

/*--------------------------------------------- SYSTEM CALLS -----------------*/
int gethostname(char *name, int namelen);
int setregid(int rgid, int egid);
int setreuid(int ruid, int euid);
int getgroups(int gidsetlen, int *gidset);
int setgroups(int ngroups, int *gidset);
int wait3(union wait *statusp, int options, struct rusage *rusage);

/*--------------------------------------------- C LIBRARY FUNCTIONS ----------*/
char *getwd(char pathname[MAXPATHLEN]);
int nice(int incr);

/*--------------------------------------------- RPC SERVICES LIBRARY ---------*/
int rstat(char *host, struct statstime *statp);

/*----------------------------------------------------------------------------*/
static char workingDirectory[MAXPATHLEN];

/*----------------------------------------------------------------------------*/
void OS_Init ()

{ char workingDir[MAXPATHLEN];
  int pos = 0;

  Prolog;
  if (getwd(workingDir) == NULL) E(7000);
  strcat(workingDir, "/");
  if (!strncmp(workingDir, AUTOMOUNTDIR, strlen(AUTOMOUNTDIR)))
    pos = strlen(AUTOMOUNTDIR) - 1;
  strcpy(workingDirectory, &(workingDir[pos]));
} /* OS_Init */

/*----------------------------------------------------------------------------*/
void OS_Mail(char *userName, char *subject, char *body)

{ char cmd[4096];
  char fileName[1024];
  int len;
  int fd;

  Prolog;
  sprintf(fileName, "/tmp/dbatch.mail.%s", userName);
  remove(fileName);
  if ((fd = creat(fileName, 0666)) < 0) E(7001);
  len = strlen(body);
  if (write(fd, body, len) != len) E(7002);
  close(fd);
  sprintf(cmd, "/usr/ucb/Mail -s \"%s\" %s < %s", subject, userName, fileName);
  system(cmd);
  remove(fileName);
} /* OS_Mail */

/*----------------------------------------------------------------------------*/
void OS_UserIdToName (int userId, char *userName)

{ struct passwd *password;

  Prolog;
  if (userId == ROOT)
    strcpy(userName, "root");
  else if (userId == ALL)
    strcpy(userName, "ALL");
  else {
    password = getpwuid(userId);
    if (password != NULL )
      strcpy(userName, password->pw_name);
    else
      userName[0] = '\0';
  }
} /* OS_UserIdToName */

/*----------------------------------------------------------------------------*/
void OS_GetExecutable (char *progName, char *fileName)

{ struct stat buf;
  char *path;
  int i, k;

  Prolog;
  if (*progName == '/') {
    strcpy(fileName, progName);
    if (!stat(fileName, &buf) && buf.st_mode & 0111
        && (buf.st_mode & S_IFMT) == S_IFREG) return;
  }
  else if ((path = getenv("PATH")) != NULL) {
    while (*path != '\0') {
      i = k = 0;
      while (*path != ':' && *path != '\0') fileName[i++] = *path++;
      if (*path == ':') path++;
      fileName[i++] = '/';
      while ((fileName[i++] = progName[k++]) != '\0');
      if (!stat(fileName, &buf) && buf.st_mode & 0111
          && (buf.st_mode & S_IFMT) == S_IFREG) return;
    }
  }
  strcpy(fileName, "");
} /* OS_GetExecutable */

/*----------------------------------------------------------------------------*/
void OS_GetSlotCnt (unsigned int *nofSlot, unsigned int *nofIdleSlot)

{ char hostName[MAXHOSTNAMELEN];
  struct statstime stat;
  int i;

  Prolog;
  if (gethostname(hostName, MAXHOSTNAMELEN)) E(7003);
  if (rstat(hostName, &stat)) {
    sleep(60);                                /* wait until rstatd is running */
    if (rstat(hostName, &stat)) E(7004);
  }
  *nofSlot = 0;
  for (i = 0; i < 4; i++) *nofSlot += (unsigned int)stat.cp_time[i];
  *nofIdleSlot = (unsigned int)stat.cp_time[3];
} /* OS_GetSlotCnt */

/*----------------------------------------------------------------------------*/
void OS_GetCpuUsage (unsigned int *nofSlot, unsigned int *nofIdleSlot, 
                     int *load)

{ unsigned int dSlot, dIdleSlot;

  Prolog;
  OS_GetSlotCnt(&dSlot, &dIdleSlot); T(7005);
  sleep(60);
  OS_GetSlotCnt(nofSlot, nofIdleSlot); T(7006);
  if (!(dSlot = *nofSlot - dSlot)) E(7007);
  dIdleSlot = *nofIdleSlot - dIdleSlot;
  *load = ((dSlot - dIdleSlot) * MAXLOAD + dSlot / 2) / dSlot;
  if (*load < 0 || MAXLOAD < *load) E(7008);
} /* OS_GetCpuUsage */

/*----------------------------------------------------------------------------*/
char *OS_WorkingDirectory ()

{ return workingDirectory;
} /* OS_WorkingDirectory */

/*----------------------------------------------------------------------------*/
int  OS_IsInFile (char *fileName)

{ struct stat buf;

  Prolog FALSE;
  if (!strcmp(fileName, "/dev/null"))
    return TRUE;
  else if (!stat(fileName, &buf) && (buf.st_mode & S_IFMT) == S_IFREG)
    return TRUE;
  else
    return FALSE;
} /* OS_IsInFile */

/*----------------------------------------------------------------------------*/
int  OS_IsOutFile (char *fileName)

{ struct stat buf;

  Prolog FALSE;
  if (!strcmp(fileName, "/dev/null"))
    return TRUE;
  else if (!stat(fileName, &buf))
    return ((buf.st_mode & S_IFMT) == S_IFREG) ? TRUE : FALSE;
  else
    return (errno == ENOENT) ? TRUE : FALSE;
} /* OS_IsOutFile */

/*----------------------------------------------------------------------------*/
int OS_HostId()

{ char hostName[MAXHOSTNAMELEN];
  int hostId;

  Prolog NONE;
  if (gethostname(hostName, MAXHOSTNAMELEN) != 0) ER(7009, NONE);
  Base_HostNameToId(hostName, &hostId); TR(7010, NONE);
  return hostId;
} /* OS_HostId */

/*----------------------------------------------------------------------------*/
int OS_UserId()

{ int userId;

  Prolog NONE;
  userId = geteuid();
  return (userId == 0) ? ROOT : userId;
} /* OS_UserId */

/*----------------------------------------------------------------------------*/
int OS_GroupId()

{ Prolog NONE;
  return getegid();
} /* OS_GroupId */

/*----------------------------------------------------------------------------*/
void OS_GetGroupList (char *groupList)

{ int groupSet[NGROUPS];
  int groupSetLen, i;

  Prolog;
  if ((groupSetLen = getgroups(NGROUPS, groupSet)) < 0) E(7011);
  sprintf(groupList, "%d", groupSetLen);
  for (i = 0; i < groupSetLen; i++)
    sprintf(groupList, "%s %d", groupList, groupSet[i]);
} /* OS_GetGroupList */

/*----------------------------------------------------------------------------*/
void OS_SetGroupList (char *groupList)

{ int groupSet[NGROUPS];
  int groupSetLen, i;
  char *number;

  Prolog;
  if ((number = strtok(groupList, " ")) == NULL) E(7012);
  if (sscanf(number, "%d", &groupSetLen) != 1) E(7013);
  for (i = 0; i < groupSetLen; i++) {
    if ((number = strtok(NULL, " ")) == NULL) E(7014);
    if (sscanf(number, "%d", &groupSet[i]) != 1) E(7015);
  }
  if (setgroups(groupSetLen, groupSet)) E(7016);
} /* OS_SetGroupList */

/*----------------------------------------------------------------------------*/
static void OS_GetConsoleUserName (char *userName)

{ int fd;
  struct utmp user;

  Prolog;
  strcpy(userName, "");
  if ((fd = open("/etc/utmp", O_RDONLY)) < 0) E(7017);
  while (read(fd, (char *)&user, sizeof user) == sizeof user)
    if (!strcmp(user.ut_line, "console")) {
      strcpy(userName, user.ut_name);
      break;
    }
  close(fd);
} /* OS_GetConsoleUserName */

/*----------------------------------------------------------------------------*/
int OS_HostState ()

{ char userName[MAXUSERNAMELEN];

  Prolog FREE;
  OS_GetConsoleUserName(userName);
  return (!strcmp(userName, "")) ? FREE : OCCUPIED;
} /* OS_HostState */

/*----------------------------------------------------------------------------*/
int  OS_UserOnConsole ()

{ char userName1[MAXUSERNAMELEN];
  char userName2[MAXUSERNAMELEN];

  Prolog FALSE;
  OS_GetConsoleUserName(userName1);
  OS_UserIdToName(OS_UserId(), userName2);
  return (!strcmp(userName1, userName2)) ? TRUE : FALSE;
} /* OS_UserOnConsole */
  
/*----------------------------------------------------------------------------*/
int OS_ProcessId()

{ int pid;

  Prolog NONE;
  if ((pid = getpid()) <= 1) ER(7018, NONE);
  return pid;
} /* OS_ProcessId */

/*----------------------------------------------------------------------------*/
int  OS_ProcessState (int procId)

{ char state;
  char fileName[128];
  char cmd[128];
  int fd;

  Prolog NONE;
  sprintf(fileName, "/tmp/dbatch.%d", procId);
  remove(fileName);
  sprintf(cmd, "ps %d > %s 2>&1", procId, fileName);
  system(cmd);
  if ((fd = open(fileName, O_RDONLY)) < 0) ER(7019, NONE);
  if (lseek(fd, 37, L_SET) != 37 || read(fd, &state, 1) != 1) state = '@';
  close(fd);
  remove(fileName);
  switch (state) {
    case 'R': case 'P': case 'D': case 'S': case 'I': 
      return RUNNING;
    case 'T':
      return STOPPED;
    default:
      return FINISHED;
  }
} /* OS_ProcessState */

/*----------------------------------------------------------------------------*/
void OS_SetSessionLeader (int procGroupId)

{ Prolog;
  if ((int)setsid() != procGroupId) E(7020);
} /* OS_SetSessionLeader */

/*----------------------------------------------------------------------------*/
static void OS_StartJobError (char *msg, char **argv, char *stdIn,
                              char *stdOut, char *stdErr, int userId)

{ char userName[MAXUSERNAMELEN];
  char head[128];
  char str[4096];
  int i;

  sprintf(str, "%s:\n\n ", msg);
  for (i = 0; argv[i] != NULL; i++)
    sprintf(str, "%s %s", str, argv[i]);
  sprintf(str, "%s\n  <  %s\n  >  %s\n  2> %s\n", str, stdIn, stdOut, stdErr);
  OS_UserIdToName(userId, userName);
  sprintf(head, "dbatch: %s", msg);
  OS_Mail(userName, head, str);
  exit(-1);
} /* OS_StartJobError */

/*----------------------------------------------------------------------------*/
int OS_SetInOutFiles (char *stdIn, char *stdOut, char *stdErr)

{ int fd;

  if ((fd = open(stdIn, O_RDONLY)) < 0) return FALSE;  /* initialize stdin  */
  if (dup2(fd, 0) < 0) ER(7021, FALSE);
  if (close(fd)) ER(7022, FALSE);

  if ((fd = creat(stdOut, 0600)) < 0) return FALSE;    /* initialize stdout */
  if (dup2(fd, 1) < 0) ER(7023, FALSE);
  if (close(fd)) ER(7024, FALSE);

  if (!strcmp(stdOut, stdErr)) {
    if (dup2(1, 2) < 0) ER(7025, FALSE);
  }
  else {
    if ((fd = creat(stdErr, 0600)) < 0) return FALSE;  /* initialize stderr */
    if (dup2(fd, 2) < 0) ER(7026, FALSE);
    if (close(fd)) ER(7027, FALSE);
  }
  return TRUE;
} /* OS_SetInOutFiles */

/*----------------------------------------------------------------------------*/
void OS_StartJob (char *workDir, int userId, int groupId, char *groupList,
                  char **argv, char **envp, char *stdIn, char *stdOut, 
                  char *stdErr)

{ int pid;

  Prolog;
  if (userId == ROOT) userId = 0;
  else if (userId <= 0) E(7028);
  if (groupId < 0) E(7029);
  if ((pid = fork()) < 0) E(7030);
  if (pid == 0) {                                        /* child process     */
    if (nice(NICEVALUE)) E(7031);
    if (setregid(groupId, groupId)) E(7032);
    OS_SetGroupList(groupList); T(7033);
    if (setreuid(userId, userId)) E(7034);
    if (chdir(workDir)) E(7035);
    if (! OS_SetInOutFiles(stdIn, stdOut, stdErr))
      OS_StartJobError("wrong input, output or error file", argv, stdIn, stdOut,
                        stdErr, userId);
    execve(*argv, argv, envp);
    OS_StartJobError("command not found", argv, stdIn, stdOut, stdErr, userId);
    exit(-1);
  }
} /* OS_StartJob */

/*----------------------------------------------------------------------------*/
void OS_WaitForChildren (int *exitValue, long *cpuTime)


{ union wait status;
  struct rusage rusage;

  Prolog;
  *exitValue = -1;
  while (wait3(&status, 0, &rusage) != -1) {
    if (status.__w_termsig == 0)
      *exitValue = status.__w_retcode;
    *cpuTime += rusage.ru_utime.tv_sec;
  }
} /* OS_WaitForChildren */

/*----------------------------------------------------------------------------*/
