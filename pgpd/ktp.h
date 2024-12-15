#ifndef KTP_H
#define KTP_H

/*
** ktp.h -- for Key Transfer Protocol (KTP)
**
** Ben Cox <thoth@netcom.com>, 19 Nov 1993
*/

/*
** This is the list of commands the server understands.
**
** It is loosely based on the commands supported by the email keyservers
** listed in the KEYSERV.DOC file that is distributed with PGP.
**
** More could be added later.
*/

#define N_KTP_COMMANDS 7

#define ADD      0
#define DBG      1
#define GET      2
#define QUIT     3 
#define INDEX    4
#define VINDEX   5
#define HELP     6

/* must be in same order as above */
extern char *KTPcommands[] = {
    "add",
    "debug",
    "get",
    "quit",
    "index",
    "vindex",
    "help"
};

/*
** This is a list of result codes returned by the server.
*/

#define N_OK     0
#define S_OK     "Ok."
#define N_BYE    1
#define S_BYE    "Goodbye."
#define N_ERR    2
#define S_ERR    "Error in %s: %s (%d)."
#define N_SYNTAX 3
#define S_SYNTAX "Command not understood.  Type 'help' for help."
#define N_NOISE  50
#define N_KEY    100
#define S_KEY    "Key data follows, followed by '.' on a line by itself."
#define N_NOKEY  101
#define S_NOKEY  "Key not found in database."
#define N_INDEX  102
#define S_INDEX  "Key index follows, followed by '.' on a line by itself."
#define N_PGPERR 103
#define S_PGPERR "Unknown PGP error code %d."
#define N_GIMME  200
#define S_GIMME  "Send your key data, followed by '.' on a line by itself."

#endif
