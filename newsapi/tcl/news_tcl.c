#include <stdio.h>
#include <stdlib.h>
#include <tcl.h>
#include "newsapi.h"

#define STR_EQ(s1,s2)  (!strcmp ((s1),(s2)))

/* NEWS SERVER OBJECTS */

/* could put other stuff in here */
typedef struct tcl_news_server {
  server_info_type * server;
} tcl_news_server_type;

void
delete_tcl_news_server (ClientData clientData)
{
  tcl_news_server_type * tcl_server = (tcl_news_server_type *) clientData;
  
  if (tcl_server->server)
    news_close_server (tcl_server->server);

  free (tcl_server);
}

/* NEWS OVERVIEW OBJECTS */

typedef struct tcl_news_overview {
  server_info_type * server;
  header_info_type * overview;
  unsigned long length;
} tcl_news_overview_type;

void
delete_tcl_news_overview (ClientData clientData)
{
  tcl_news_overview_type * tcl_overview =
    (tcl_news_overview_type *) clientData;
  
  if (tcl_overview->overview)
    free (tcl_overview->overview);
  
  free (tcl_overview);
}

/*
 * ----------------------------------------------------------------------
 *  News Overview Objects
 * ----------------------------------------------------------------------
 */

char * header_name_table[] = {
#define SUBJECT 	0
  "subject",
#define FROM 		1
  "from",
#define MESSAGE_ID 	2
  "message_id",
#define REFERENCES	3
  "references",
#define XREF		4
  "xref",
#define DATE		5
  "date",
#define NUMBER		6
  "number",
#define LINES		7
  "lines",
#define BYTES		8
  "bytes"
};
#define NUM_HEADERS	9

int match_header_name (char * header) 
{
  int i = -1;

  for (i = 0; i < NUM_HEADERS; i++) {
    if (STR_EQ(header_name_table[i], header))
      return (i);
  }
  return(i);
}

/* syntax: 'news_overview_0 HEADER ARTINDEX'  	*/
/*         'news_overview_0 length' 		*/

int
news_overview_cmd (ClientData clientData,
		   Tcl_Interp *interp,
		   int argc,
		   char *argv[])
{
  tcl_news_overview_type * tcl_overview =
    (tcl_news_overview_type *) clientData ;
  long article_index;
  int header_name_type;
  char * header;

  if (tcl_overview) {
    if (tcl_overview->overview) {
      switch (argc) {
      default:
	interp->result = "wrong # of args"; 	
	return (TCL_ERROR);
	break;
      case 2:
	if (STR_EQ(argv[1], "length")) {
	  sprintf(interp->result, "%ld", tcl_overview->length);
	  return(TCL_OK);
	}
      case 3:
	if (Tcl_ExprLong (interp, argv[2], &article_index) != TCL_OK) {
	  Tcl_SetResult(interp, "error parsing article index", TCL_STATIC);
	  return (TCL_ERROR);
	}
	if ((article_index < 0) ||
	    (article_index > tcl_overview->length)) {
	  Tcl_SetResult(interp, "article index out of range", TCL_STATIC);
	  return (TCL_ERROR);
	}
	if ((header_name_type = match_header_name(argv[1])) >= 0) {
	  header_info_type * ov = &tcl_overview->overview[article_index];

	  switch (header_name_type) {
	  default:
	    header = (char *) NULL;
	    break;
	  case SUBJECT:
	    header = ov->subject;
	    break;
	  case FROM:
	    header = ov->from;
	    break;
	  case MESSAGE_ID:
	    header = ov->message_id;
	    break;
	  case REFERENCES:
	    header = ov->references;
	    break;
	  case XREF:
	    header = ov->xref;
	    break;
	  }
	  if (header != (char *) NULL) {
	    Tcl_SetResult(interp, header, TCL_STATIC); /* sorta STATIC */
	    return (TCL_OK);
	  }
	  else {
	    switch (header_name_type) {
	    case DATE:
	      sprintf (interp->result, "%ld", ov->date);
	      break;
	    case NUMBER:
	      sprintf (interp->result, "%ld", ov->number);
	      break;
	    case LINES:
	      sprintf (interp->result, "%ld", ov->lines);
	      break;
	    case BYTES:
	      sprintf (interp->result, "%ld", ov->bytes);
	      break;
	    }
	    return (TCL_OK);
	  }
	}
	else {
	  Tcl_SetResult(interp, "unknown header type", TCL_STATIC);
	  return (TCL_ERROR);
	}
	break;	/* case 3: */
      }
    }
    else {
      interp->result = "null overview data in overview object";
      return (TCL_ERROR);
    }
  } else {
    interp->result = "null overview object";
    return (TCL_ERROR);
  }
}


/*
 * ----------------------------------------------------------------------
 *  News Server Objects
 * ----------------------------------------------------------------------
 */


int
process_news_server_cmd (ClientData clientData,
			 Tcl_Interp *interp,
			 int argc,
			 char *argv[])
{
  tcl_news_server_type * tcl_server = (tcl_news_server_type *) clientData ;  

  if (argc == 1) {
    return (TCL_OK);
  }

  /*
   * Re-order these, most likely to least 
   */

  /*
   * ----------------------------------------------------------------------
   * the GROUP command
   * ----------------------------------------------------------------------
   */

  if (STR_EQ (argv[1], "group")) {
    group_info_type * temp_group;

    /* low n high n estnum n */
    char group_template[50];	/* assuming no more than 10 chars for 'n' */

    if (argc != 3) {
      Tcl_SetResult (interp, "wrong # args", TCL_STATIC);
      return (TCL_ERROR);
    }
    /* use this to construct the response */
    temp_group = (group_info_type *) malloc (sizeof (group_info_type));
    
    /* TODO error checking */
    news_group(tcl_server->server, argv[2], temp_group);

    sprintf (group_template,
	     "low %ld high %ld estnum %ld",
	     temp_group->low, temp_group->high, temp_group->estnum);

    Tcl_SetResult (interp, group_template, TCL_VOLATILE);
    return (TCL_OK);
  }

  /*
   * ----------------------------------------------------------------------
   * The ARTICLE command
   * ----------------------------------------------------------------------
   */
  
  if (STR_EQ (argv[1], "article")) {
    char * article_data;
    long article_number;

    if (argc != 4) {
      Tcl_SetResult(interp, "wrong # args", TCL_STATIC);
      return (TCL_ERROR);
    }
    
    if (Tcl_ExprLong (interp, argv[3], &article_number) != TCL_OK) {
      Tcl_SetResult(interp, "error parsing article number", TCL_STATIC);
      return (TCL_ERROR);
    }

    /* TODO error checking */
    news_article (tcl_server->server,
		  argv[2],	/* group name */
		  (unsigned long) article_number,
		  &article_data);

    Tcl_SetResult(interp, article_data, TCL_DYNAMIC);
    return (TCL_OK);
  }

  /*
   * ----------------------------------------------------------------------
   * The OVERVIEW command
   * ----------------------------------------------------------------------
   */

  /* news_server_0 overview group_name low high */

  if (STR_EQ (argv[1], "overview")) {
    unsigned long low, high;
    header_info_type * header_data;
    tcl_news_overview_type * tcl_overview;
    static overview_id = 0;

    if (argc != 5) {
      Tcl_SetResult(interp, "wrong # args", TCL_STATIC);
      return (TCL_ERROR);
    }
    if (Tcl_ExprLong (interp, argv[3], &low) != TCL_OK) {
      Tcl_SetResult(interp, "error parsing low range index", TCL_STATIC);
      return (TCL_ERROR);
    }
    if (Tcl_ExprLong (interp, argv[4], &high) != TCL_OK) {
      Tcl_SetResult(interp, "error parsing high range index", TCL_STATIC);
      return (TCL_ERROR);
    }
    if ((tcl_overview =
	 (tcl_news_overview_type *) malloc (sizeof (tcl_news_overview_type)))
	== NULL) {
      Tcl_SetResult(interp,
		    "couldn't allocate new overview object",
		    TCL_STATIC);
      return (TCL_ERROR);
    }

    /* TODO error checking */
    news_get_headers (tcl_server->server,
		      argv[2],			/* groupname */
		      low, high, 		/* range */
		      &header_data); 		/* storage */
    tcl_overview->length = high - low;
    tcl_overview->overview = header_data;
    sprintf (interp->result, "news_overview_%d", overview_id);
    Tcl_CreateCommand (interp, 
		       interp->result,
		       news_overview_cmd,
		       (ClientData) tcl_overview,
		       delete_tcl_news_overview);
    return(TCL_OK);
  }

  /*
   * ----------------------------------------------------------------------
   * The CLOSE command
   * ----------------------------------------------------------------------
   */

  if (STR_EQ (argv[1], "close")) {
    if (argc != 2) {
      Tcl_SetResult(interp, "wrong # args", TCL_STATIC);
      return (TCL_ERROR);
    }
    /* TODO error checking */
    news_close_server (tcl_server->server);
    
    tcl_server->server = (server_info_type *) NULL;
    Tcl_DeleteCommand (interp, argv[0]);

    Tcl_SetResult(interp, "Ok", TCL_STATIC);
    return (TCL_OK);
  }
  
  /*
   * ----------------------------------------------------------------------
   *  Unknown command
   * ----------------------------------------------------------------------
   */

  Tcl_SetResult(interp, "unknown server command", TCL_STATIC);
  return (TCL_ERROR);

}

int
news_server_cmd (ClientData clientData,
		 Tcl_Interp *interp,
		 int argc,
		 char *argv[])
{
  tcl_news_server_type * tcl_server = (tcl_news_server_type *) clientData ;

  if (tcl_server) {
    if (tcl_server->server) {
      if (tcl_server->server->hostname) {
	/* process the command */
	return (process_news_server_cmd (clientData,interp,argc,argv));
      }
      else {
	interp->result = "no hostname";
	return (TCL_ERROR);
      }
    }
    else {
      interp->result = "no server object";
      return (TCL_ERROR);
    }
  }
  else {
    interp->result = "no tcl server object";
    return (TCL_ERROR);
  }
}

int
news_connect_cmd (ClientData clientData,
		  Tcl_Interp *interp,
		  int argc,
		  char* argv[])
{
  
  tcl_news_server_type * tcl_server;
  static int id = 0;
  
  if (argc != 2) {
    interp->result = "wrong # of args";
    return TCL_ERROR;
  }

  /* make a new tcl_server object */

  tcl_server = (tcl_news_server_type *)
    malloc (sizeof (tcl_news_server_type *));
  
  if (!tcl_server) {
    interp->result = "couldn't allocate new tcl news server object";
    return TCL_ERROR;
  }
  
  /* connect to the specified server */
  /* error checking here ! */

  news_add_server(argv[1], &(tcl_server->server));

  sprintf (interp->result, "news_server_%d", id++);
  
  Tcl_CreateCommand(interp, interp->result,
		    news_server_cmd,
		    (ClientData) tcl_server,
		    delete_tcl_news_server);
  
  return (TCL_OK);
}


#ifdef LIBEDIT
char * readline(char * prompt);
void add_history(char * line);
#endif

int
main()
{
  Tcl_Interp *interp;
  int code = TCL_OK;
#ifdef LIBEDIT
  char * command;
#else
  char command[512];
#endif

  interp = Tcl_CreateInterp();

  Tcl_CreateCommand(interp, "news_connect", news_connect_cmd,
		    (ClientData) 0,
		    (Tcl_CmdDeleteProc *) NULL);

  while (1) {
#ifdef LIBEDIT
    command = readline("news_tcl> ");
#else
    printf ("news_tcl> ");
    fflush (stdout);
    fgets (command, 511, stdin);
#endif

    if (command) {
      code = Tcl_Eval (interp, command, 0, NULL);
    
#ifdef LIBEDIT
    add_history(command);
    free(command);
#endif

      if (*interp->result != 0) {
	printf ("%s\n", interp->result);
      }
    }
  }
  exit(0);
}

