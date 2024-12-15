/* Author: Wietse Venema <wietse@wzv.win.tue.nl> */

#include <sys/types.h>
#include <sys/time.h>
#include <utmpx.h>
#include <string.h>

/* utmpx_logout - update utmp and wtmp after logout */

utmpx_logout(line)
char   *line;
{
    struct utmpx utx;
    struct utmpx *ut;

    strncpy(utx.ut_line, line, sizeof(utx.ut_line));

    if (ut = getutxline(&utx)) {
	ut->ut_type = DEAD_PROCESS;
	ut->ut_exit.e_termination = 0;
	ut->ut_exit.e_exit = 0;
#ifdef SOLARIS
	gettimeofday(&(ut->ut_tv), NULL);
#else
	gettimeofday(&(ut->ut_tv));
#endif
	pututxline(ut);
	updwtmpx(WTMPX_FILE, ut);
    }
    endutxent();
}
