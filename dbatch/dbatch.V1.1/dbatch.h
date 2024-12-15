/*----------------------------------------------------------------------------*/
/* Syntax for Command 'dbatch'                                                */
/*----------------------------------------------------------------------------*/
/*
dbatch [ -jnu ]                  show state of jobs, network and users
dbatch [ -h hostSet ] ansProg jobDesc  add job in queue
dbatch -d { jobId } | ALL        delete job from queue
dbatch -s numHours               stop execution of jobs on your machine
dbatch -login
dbatch -logout

dbatch -fullhoststate	               daemon: report state of host
dbatch -startjob jobId                 daemon: start job on client
dbatch -jobstate jobId procGroupId     daemon: report state of job
dbatch -reboot		               daemon: host rebooted
dbatch -checkstate                     daemon: check state of hosts and jobs
dbatch -savestate                      daemon: save state informations
dbatch -shutdown                       daemon: shutdown daemon
*/
/*----------------------------------------------------------------------------*/
/* Communication Protocol in EBNF                                             */
/*----------------------------------------------------------------------------*/
/*
................................................................................
User / Client				Server
................................................................................
SHOWSTATE userId what
					TextPackageList
DONE
................................................................................
ADDJOB userId userName hostSet JobPackageList
					( OK | NOTOK )
DONE
................................................................................
DELJOB jobId userId
					( OK | NOTOK )
DONE
................................................................................
HOSTSILENT hostId silent
................................................................................
HOSTSTATE hostId hostState
................................................................................
FULLHOSTSTATE hostId hostState nofSlot nofIdleSlot
................................................................................
SENDJOB jobId hostId
					( OK JobPackageList
JOBSTARTED procGroupId
					| NOTOK
DONE )

JOBDONE jobId hostId userId cpuTime nofSlot nofIdleSlot load
................................................................................
JOBSTATE jobId hostId jobState==FINISHED nofSlot nofIdleSlot load
JOBSTATE jobId hostId jobState!=FINISHED
................................................................................
HOSTREBOOTED hostId nofSlot nofIdleSlot
................................................................................
CHECKSTATE
................................................................................
SAVESTATE
................................................................................
SHUTDOWN
................................................................................
*/
/*----------------------------------------------------------------------------*/
/* CONSTANTS                                                                  */
/*----------------------------------------------------------------------------*/

#define HOSTNAMELIST    "isibee26 1\
                         isibee25 1 isibee24 1 isibee23 1 isibee22 1 isibee21 1\
                         isibee20 1 isibee19 1 isibee18 1 isibee17 1 isibee16 1\
                         isibee15 1 isibee14 1 isibee13 1 isibee12 1 isibee11 1\
                         isibee10 1 isibee9  1 isibee8  1 isibee7  1 isibee6  1\
                         isibee5  1 isibee4  1 isibee3  1 isibee2  1 isibee1  1\
                         isidor   1"
#define NOFHOST          27

#define DBATCHADMIN       "demoline"           /* user responsible for dbatch */

#ifdef DEBUG
#define PROGDBATCH        "/home/isi/research/demoline/batch/dbatch"
#define ADDRESSFILE       "/home/isi/research/demoline/batch/dbatchd.adr"
#define SAVEFILE          "/home/isi/research/demoline/batch/dbatchd.sav"
#else
#define PROGDBATCH        "/usr/local/bin/dbatch"
#define ADDRESSFILE       "/usr/local/bin/dbatchd.adr"
#define SAVEFILE          "/usr/local/bin/dbatchd.sav"
#endif

#define AUTOMOUNTDIR      "/tmp_mnt/"
#define MAXNOFJOB       256
#define MAXNOFJOBPERUSER 32
#define MAXNOFUSER       32
#define MAXNOFINQUIRE     3
#define MAXNOFRETRY       3
#define MAXNOFARG       128
#define MAXNOFENV       256
#define MAXNOFLISTEN     64
#define MAXUSERNAMELEN   16
#define NICEVALUE        19
#define CPUTIMEREDUCTION 139 /* half life: (35->1d, 70->2d, 104->3d, 139->4d) */
#define LOADREDUCTION     4
#define MAXLOAD         100
#define MAXLOADFREE      11
#define MAXLOADOCCUPIED   6
#define NOFEXELEVEL      15
#define MAXSILENT        10

#define OK                1   /* message type */
#define NOTOK             2
#define DONE              3
#define SHOWSTATE         4
#define ADDJOB            5
#define DELJOB            6
#define SENDJOB           7
#define JOBSTARTED        8
#define JOBDONE           9
#define JOBSTATE         10
#define HOSTSTATE        11
#define FULLHOSTSTATE    12
#define HOSTSILENT       13
#define HOSTREBOOTED     14
#define CHECKSTATE       15
#define SAVESTATE        16
#define SHUTDOWN         17

#define SHOWJOBSTATE      1   /* what to display */
#define SHOWHOSTSTATE     2
#define SHOWUSERSTATE     4

#define WAITING           1   /* job state */
#define PENDING           2
#define RUNNING           4
#define STOPPED           8
#define DELETED          16
#define FINISHED         32

#define FREE              1   /* host state */
#define OCCUPIED          2
#define MAYBEDOWN         4
#define DOWN              8

#define ROOT             -2   /* special user ID */
#define ALL              -1
#define NONE              0
#ifndef TRUE
#define TRUE              (1)
#endif
#ifndef FALSE
#define FALSE             (0)
#endif

#define MESSAGESIZE       sizeof(struct Message)
#define PACKAGESIZE       sizeof(struct Package) /* 1460 bytes is maximal size*/
#define PACKAGEBUFSIZE    (1024 - 4)
#define HOSTSPERINT       (8 * sizeof(int))
#define HOSTSETLEN        ((NOFHOST + HOSTSPERINT - 1) / HOSTSPERINT)

/*----------------------------------------------------------------------------*/
/* DATA TYPES                                                                 */
/*----------------------------------------------------------------------------*/
struct Message {
  int type;
  int jobId;
  int userId;
  int hostId;
  int state;
  int silent;
  int load;
  int hostSet[HOSTSETLEN];
  int procGroupId;
  int what;
  long cpuTime;
  unsigned int nofSlot;
  unsigned int nofIdleSlot;
  char userName[MAXUSERNAMELEN];
};

struct Package {
  struct Package *next;
  char buf[PACKAGEBUFSIZE];
};

/*----------------------------------------------------------------------------*/
/* GLOBAL VARIABLES                                                           */
/*----------------------------------------------------------------------------*/
extern int ok;

/*----------------------------------------------------------------------------*/
/* MODULE job                                                                 */
/*----------------------------------------------------------------------------*/
void Job_Init           ();
void Job_SaveState      (int fd);
void Job_LoadState      (int fd);
void Job_SendState      (int connFD, int userId, struct Package *textBuffer);
void Job_SendJob        (int connFD, int jobId);
void Job_CheckState     ();
void Job_Insert         (int userId, char *userName, int hostSet[],
                         struct Package *jobPackage, int *inserted);
void Job_Delete         (int userId, int jobId, int *deleted);
void Job_StartNext      ();
void Job_ChangeState    (int jobId, int hostId, int srcStateSet, int destState);
void Job_SetHostId      (int jobId, int hostId);
void Job_SetProcGroupId (int jobId, int procGroupId);
int  Job_State          (int jobId);

/*----------------------------------------------------------------------------*/
/* MODULE host                                                                */
/*----------------------------------------------------------------------------*/
void Host_Init          ();
void Host_SaveState     (int fileDesc);
void Host_LoadState     (int fileDesc);
void Host_SendState     (int connFD, int userId, struct Package *textBuffer);
void Host_CheckState    ();
void Host_IncJobCount   (int hostId);
void Host_DecJobCount   (int hostId);
void Host_Rebooted      (int hostId);
void Host_SetLoad       (int hostId, int load);
void Host_SetState      (int hostId, int state);
void Host_SetSlotCnt    (int hostId, unsigned int nofSlot, 
                         unsigned int nofIdleSlot);
void Host_SetSilent     (int hostId, int silent);
void Host_ReduceSilent  ();
void Host_AddCpuTime    (int hostId, long cpuTime);
void Host_ReduceCpuTime ();
void Host_GetFreeList   (int hostSetList[NOFEXELEVEL][HOSTSETLEN],
                         int *level);
void Host_StateToString (int hostId, char *str);
int  Host_State         (int hostId);

/*----------------------------------------------------------------------------*/
/* MODULE user                                                                */
/*----------------------------------------------------------------------------*/
void User_Init          ();
void User_SaveState     (int fileDesc);
void User_LoadState     (int fileDesc);
void User_SendState     (int connFD, int userId, struct Package *textBuffer);
void User_IncJobCount   (int userId);
void User_DecJobCount   (int userId);
void User_IncWaitCount  (int userId);
void User_DecWaitCount  (int userId);
void User_AddCpuTime    (int userId, long cpuTime);
void User_SetUserName   (int userId, char *userName);
void User_ReduceCpuTime ();
int  User_JobCount      (int userId);
int  User_First         ();
int  User_Next          ();

/*----------------------------------------------------------------------------*/
/* MODULE osd                                                                 */
/*----------------------------------------------------------------------------*/
void OSD_SendSignal     (int hostId, int procGroupId, int signal);
void OSD_InquireStartJob(int jobId, int hostId);
void OSD_InquireJobState(int jobId, int hostId, int procGroupId);
void OSD_InquireHostState(int hostId);

/*----------------------------------------------------------------------------*/
/* MODULE os                                                                  */
/*----------------------------------------------------------------------------*/
void OS_Init            ();
void OS_Mail            (char *userName, char *subject, char *body);
void OS_UserIdToName    (int userId, char *userName);
void OS_GetExecutable   (char *progName, char *fileName);
void OS_GetSlotCnt      (unsigned int *nofSlot, unsigned int *nofIdleSlot);
void OS_GetCpuUsage     (unsigned int *nofSlot, unsigned int *nofIdleSlot, 
                         int *load);
void OS_GetGroupList    (char *groupList);
void OS_SetGroupList    (char *groupList);
char *OS_WorkingDirectory();
int  OS_IsInFile        (char *fileName);
int  OS_IsOutFile       (char *fileName);
int  OS_HostId          ();
int  OS_UserId          ();
int  OS_GroupId         ();
int  OS_HostState       ();
int  OS_UserOnConsole   ();
int  OS_ProcessId       ();
int  OS_ProcessState    (int processId);
int  OS_SetInOutFiles   (char *stdIn, char *stdOut, char *stdErr);
void OS_SetSessionLeader(int procGroupId);
void OS_StartJob        (char *workDir, int userId, int groupId, 
                         char *groupList, char **argv, char **envp, 
                         char *stdIn, char *stdOut, char *stdErr);
void OS_WaitForChildren (int *exitValue, long *cpuTime);

/*----------------------------------------------------------------------------*/
/* MODULE ipc                                                                 */
/*----------------------------------------------------------------------------*/
void IPC_PutMessage     (int connFD, struct Message *message);
void IPC_GetMessage     (int connFD, struct Message *message);
void IPC_PutPackage     (int connFD, struct Package *package);
void IPC_GetPackage     (int connFD, struct Package *package);
void IPC_PutPackageList (int connFD, struct Package *package);
void IPC_GetPackageList (int connFD, struct Package **package);
void IPC_OpenText       (struct Package *textBuffer);
void IPC_WriteText      (int connFD, struct Package *textBuffer, char *text);
void IPC_CloseText      (int connFD, struct Package *textBuffer);
void IPC_ReadText       (int connFD);
void IPC_OpenListen     (int *listenFD);
void IPC_Listen         (int listenFD, void (*HandleMessage)(int),
                                       void (*StartNextJob)());
void IPC_OpenConnection (int *connFD);
void IPC_CloseConnection(int connFD);

/*----------------------------------------------------------------------------*/
/* MODULE base                                                                */
/*----------------------------------------------------------------------------*/
void Base_Init          ();
void Base_Error         (int num);

struct Package 
    *Base_NewPackage    ();
void Base_DisposePackageList (struct Package *package);

void Base_HostIdToName  (int hostId, char *hostName);
void Base_HostNameToId  (char *hostName, int *hostId);
void Base_HostSetToShortString (int hostSet[], char *str);
void Base_HostSetToString      (int hostSet[], char *str);
void Base_HostStringToSet      (char *str, int hostSet[]);

void Base_EmptyHostSet  (int hostSet[]);
void Base_FullHostSet   (int hostSet[]);
int  Base_FirstInHostSet(int hostSet[]);
int  Base_InHostSet     (int hostId, int hostSet[]);
void Base_InclHostSet   (int hostId, int hostSet[]);
void Base_ExclHostSet   (int hostId, int hostSet[]);
void Base_JoinHostSet   (int a[], int b[], int c[]);

/*----------------------------------------------------------------------------*/
/* MACROS                                                                     */
/*----------------------------------------------------------------------------*/
#define E(num)          { Base_Error(num); return; }
#define ER(num, ret)    { Base_Error(num); return (ret); }
#define T(num)          if (! ok) E(num)
#define TR(num, ret)    if (! ok) ER(num, ret)
#define Prolog          if (! ok) return
#define Epilog(num)     if (! ok) Base_Error(num)
