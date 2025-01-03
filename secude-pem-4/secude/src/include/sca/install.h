/*
 *  STARMOD Release 1.1 (GMD)
 */
/*-------------------------------------------------------+-----*/
/*							 | GMD */
/*   SYSTEM   STARMOD  -  Version 1.0			 +-----*/
/*							       */
/*-------------------------------------------------------------*/
/*							       */
/*    PACKAGE	INSTALL			VERSION 1.0	       */
/*					   DATE November 1991  */
/*					     BY Levona Eckstein*/
/*							       */
/*    FILENAME			                 	       */
/*      install.h 		         		       */
/*							       */
/*    DESCRIPTION					       */
/*      This file contains all local define's and structures   */
/*	for the install-programm			       */
/*-------------------------------------------------------------*/
#ifndef _INSTALL_
#define _INSTALL_

extern char *getenv();
struct s_record {
        char      port_name[24];
        int       bwt;
        int       cwt;
        int       baud;
        int       databits;
        int       stopbits;
        int       parity;
        int       dataformat;
        int       tpdu_size;
        int       apdu_size;
        int       edc;
        };

#endif

  
