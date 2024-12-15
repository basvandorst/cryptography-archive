#ifndef getopt_h_DEFINED
#define getopt_h_DEFINED

/**************************************************************************
   This INCLUDE file contains the external definitions for the GETOPT(3)
   function and its global variables.
**************************************************************************/

extern   int   getopt () ;       /* Function to get comnand line options. */

extern   char  *optarg ;         /* Set by GETOPT for options expecting
				      arguments. */
extern   int   optind ;           /* set by GETOPT: index of next ARGV to
				      be processed. */
extern   int   opterr;           /* Disable (== 0) or enable (== 1) error
				      messages written to standard error. */

#define   NONOPT  (-1)            /* Non-Option - returned by GETOPT when
				      it encounters a non-option argument. */

#endif getopt_h_DEFINED
/* function prototype */
int  getopt (int argc, char **argv, char *optstring);


