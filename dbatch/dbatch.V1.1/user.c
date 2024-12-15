#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#include "dbatch.h"

/*--------------------------------------------- SYSTEM CALLS -----------------*/
int read(int fd, char *buf, int nbyte);
int write(int fd, char *buf, int nbyte);

/*----------------------------------------------------------------------------*/
struct User {
  int userId;
  int nofWaitingJob;
  int nofJob;
  long cpuTime;
  char userName[MAXUSERNAMELEN];
};

static struct User user[MAXNOFUSER + 1];
static int index = 0;

/*----------------------------------------------------------------------------*/
void User_Init ()

{ int i;

  Prolog;
  for (i = 1; i <= MAXNOFUSER; i++)
    user[i].userId = NONE;
  index = NONE;
} /* User_Init */

/*----------------------------------------------------------------------------*/
void User_SaveState (int fileDesc)

{ Prolog;
  if (write(fileDesc, (char *)user, sizeof user) != sizeof user) E(9000);
} /* User_SaveState */

/*----------------------------------------------------------------------------*/
void User_LoadState (int fileDesc)

{ Prolog;
  if (read(fileDesc, (char *)user, sizeof user) != sizeof user) E(9001);
} /* User_LoadState */

/*----------------------------------------------------------------------------*/
void User_SendState (int connFD, int userId, struct Package *textBuffer)

{ int i;
  char str[1024];
  
  Prolog;
  if (userId == ROOT) str[0] = '\0'; /* only for alint */
  if (user[1].userId == NONE) {
    strcpy(str, "no entries in user list\n");
    IPC_WriteText(connFD, textBuffer, str);
  }
  else {
    strcpy(str, "User      Time [sec]  #Waiting  #Job\n");
    IPC_WriteText(connFD, textBuffer, str);
    for (i = 1; i <= MAXNOFUSER && user[i].userId != NONE; i++) {
      sprintf(str, "%-8s %11d %9d %5d\n", user[i].userName, user[i].cpuTime,
              user[i].nofWaitingJob, user[i].nofJob);
      IPC_WriteText(connFD, textBuffer, str);
    }
  }
  Epilog(9002);
} /* User_SendState */

/*----------------------------------------------------------------------------*/
static int User_GetIndex (int userId)

{ int i;

  Prolog NONE;
  for (i = 1; i <= MAXNOFUSER && user[i].userId != NONE; i++)
    if (user[i].userId == userId) return i;
  if (i > MAXNOFUSER) ER(9003, NONE);
  if (i < MAXNOFUSER)
    user[i + 1].userId = NONE;
  user[i].userId = userId;
  user[i].nofWaitingJob = 0;
  user[i].nofJob = 0;
  user[i].cpuTime = 0;
  user[i].userName[0] = '\0';
  return i;
} /* User_GetIndex */

/*----------------------------------------------------------------------------*/
static void User_Remove (int i)

{ Prolog;
  if (user[i].nofJob != 0 || user[i].cpuTime != 0) E(9004);
  for (; i < MAXNOFUSER && user[i].userId != NONE; i++)
    user[i] = user[i + 1];
  user[MAXNOFUSER].userId = NONE;
} /* User_Remove */

/*----------------------------------------------------------------------------*/
void User_IncJobCount (int userId)

{ int i;

  Prolog;
  if ((i = User_GetIndex(userId)) != NONE)
    user[i].nofJob++;
} /* User_IncJobCount */

/*----------------------------------------------------------------------------*/
void User_DecJobCount (int userId)

{ int i;

  Prolog;
  if ((i = User_GetIndex(userId)) != NONE)
    if (user[i].nofJob > 0) {
      user[i].nofJob--;
      if (user[i].nofJob == 0 && user[i].cpuTime == 0)
        User_Remove(i);
    }
  Epilog(9005);
} /* User_DecJobCount */

/*----------------------------------------------------------------------------*/
void User_IncWaitCount (int userId)

{ int i;

  Prolog;
  if ((i = User_GetIndex(userId)) != NONE)
    user[i].nofWaitingJob++;
} /* User_IncWaitCount */

/*----------------------------------------------------------------------------*/
void User_DecWaitCount (int userId)

{ int i;

  Prolog;
  if ((i = User_GetIndex(userId)) != NONE)
    if (user[i].nofWaitingJob > 0)
      user[i].nofWaitingJob--;
} /* User_DecWaitCount */

/*----------------------------------------------------------------------------*/
void User_AddCpuTime (int userId, long cpuTime)

{ int i;

  Prolog;
  if ((i = User_GetIndex(userId)) != NONE)
    user[i].cpuTime += cpuTime;
} /* User_AddCpuTime */

/*----------------------------------------------------------------------------*/
void User_SetUserName (int userId, char *userName)

{ int i;
 
  Prolog;
  if ((i = User_GetIndex(userId)) != NONE && user[i].userName[0] == '\0')
    strcpy(user[i].userName, userName);
} /* User_SetUserName */

/*----------------------------------------------------------------------------*/
void User_ReduceCpuTime ()

{ int i;

  Prolog;
  for (i = 1; i <= MAXNOFUSER && user[i].userId != NONE; i++) {
    user[i].cpuTime -= user[i].cpuTime / CPUTIMEREDUCTION + 1;
    if (user[i].cpuTime <= 0) {
      user[i].cpuTime = 0;
      if (user[i].nofJob == 0) {
        User_Remove(i); T(9006);
        i--;
      }
    }
  }
} /* User_ReduceCpuTime */

/*----------------------------------------------------------------------------*/
int User_JobCount (int userId)

{ int i;
 
  Prolog NONE;
  if ((i = User_GetIndex(userId)) != NONE)
    return user[i].nofJob;
  else
    return NONE;
} /* User_JobCount */

/*----------------------------------------------------------------------------*/
int User_First ()

{ long min = LONG_MAX;
  int i;

  Prolog NONE;
  index = NONE;
  for (i = 1; i <= MAXNOFUSER && user[i].userId != NONE; i++)
    if (user[i].nofWaitingJob > 0 && user[i].cpuTime < min)
      min = user[index = i].cpuTime;
  return (index == NONE) ? NONE : user[index].userId;
} /* User_First */

/*----------------------------------------------------------------------------*/
int User_Next ()

{ long min = LONG_MAX;
  long oldMin;
  int i;

  Prolog NONE;
  if (index == NONE) return NONE;
  oldMin = user[index].cpuTime;
  for (i = index + 1; i <= MAXNOFUSER && user[i].userId != NONE; i++)
    if (user[i].nofWaitingJob > 0 && user[i].cpuTime == oldMin)
      return user[index = i].userId;
  index = NONE;
  for (i = 1; i <= MAXNOFUSER && user[i].userId != NONE; i++)
    if (user[i].nofWaitingJob > 0 && user[i].cpuTime < min &&
                                     user[i].cpuTime > oldMin)
      min = user[index = i].cpuTime;
  return (index == NONE) ? NONE : user[index].userId;
} /* User_Next */

/*----------------------------------------------------------------------------*/
