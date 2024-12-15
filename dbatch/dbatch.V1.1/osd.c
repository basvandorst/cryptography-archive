#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/param.h>
#include "dbatch.h"

/*----------------------------------------------------------------------------*/
void OSD_SendSignal (int hostId, int procGroupId, int signal)

{ char cmd[1024];
  char hostName[MAXHOSTNAMELEN];

  Prolog;
  if (procGroupId <= 1) E(8000);
  Base_HostIdToName(hostId, hostName); T(8001);
  sprintf(cmd, "rsh %s kill -%d %d < /dev/null >/dev/null 2>&1 &",
          hostName, signal, -procGroupId);
  system(cmd);
} /* OSD_SendSignal */

/*----------------------------------------------------------------------------*/
void OSD_InquireStartJob (int jobId, int hostId)

{ char cmd[1024];
  char hostName[MAXHOSTNAMELEN];

  Prolog;
  Base_HostIdToName(hostId, hostName); T(8002);
  sprintf(cmd, "rsh %s %s -startjob %d < /dev/null >/dev/null 2>&1 &",
          hostName, PROGDBATCH, jobId);
  system(cmd);
} /* OSD_InquireStartJob */

/*----------------------------------------------------------------------------*/
void OSD_InquireJobState (int jobId, int hostId, int procGroupId)

{ char cmd[1024];
  char hostName[MAXHOSTNAMELEN];

  Prolog;
  Base_HostIdToName(hostId, hostName); T(8003);
  sprintf(cmd, "rsh %s %s -jobstate %d %d < /dev/null >/dev/null 2>&1 &",
          hostName, PROGDBATCH, jobId, procGroupId);
  system(cmd);
} /* OSD_InquireJobState */

/*----------------------------------------------------------------------------*/
void OSD_InquireHostState (int hostId)

{ char cmd[1024];
  char hostName[MAXHOSTNAMELEN];

  Prolog;
  Base_HostIdToName(hostId, hostName); T(8004);
  sprintf(cmd, "rsh %s %s -fullhoststate < /dev/null >/dev/null 2>&1 &",
          hostName, PROGDBATCH);
  system(cmd);
} /* OSD_InquireHostState */

/*----------------------------------------------------------------------------*/
