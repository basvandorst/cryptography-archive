/*--- function strerror ---------------------------------
 *
 *  Return a zero-terminated string describing an error.
 *
 *  Entry:	myerrno	is an integer usually taken from "errno".
 *
 *	 Exit:	Returns a pointer to a system error message.
 *
 *  By Mark Riordan, due to strerror being omitted from some Unixes.
 */
char *
strerror(myerrno)
int myerrno;
{
	extern char *sys_errlist[];
	extern int sys_nerr;
	
	if(myerrno >= sys_nerr) {
		return "";
	} else {
		return sys_errlist[myerrno];
	}
} 