/********************************************************************
 * Copyright (C) 1991, GMD. All rights reserved.                    *
 *                                                                  *
 *                                                                  *
 *                         NOTICE                                   *
 *                                                                  *
 *    Acquisition, use, and distribution of this module             *
 *    and related materials are subject to restrictions             *
 *    mentioned in each volume of the documentation.                *
 *                                                                  *
 ********************************************************************/

/*-----------------------------------------------------------------------*/
/* INCLUDE FILE  secsc_config.h                                          */
/* Default values for the configuration file.				 */
/* The configuration file contains the PSE objects stored on the SC.     */
/*-----------------------------------------------------------------------*/
/*
 *
 *   filename.h defines:
 *           Names of PSE Objects (File-Names in the PSE)
 *
 *   secsc.h defines:
 *           SCObjEntry		(typedef struct SCObjEntry)
 *           SCAppEntry		(typedef struct SCAppEntry)
 *
 */

#ifndef _SCCONFIG_
#define _SCCONFIG_


#include "filename.h"
#include "secsc.h"


/*
 *  Initialization of the smartcard application list with default values
 */

  char		*default_sc_app_list[] =
  {
    "polikom",
    "password",
    0
  };


  SCObjEntry	default_sc_obj_list[] =

/* test values without secure messaging */

/* keys: */
/*
  Name,		 Type,         key-   not     sm_SCT,       sm_SC_read         sm_SC_write   
                               no,    used,               command,  response,  cdo.,   resp., 
*/
  {
  SignSK_name,   SC_KEY_TYPE,   1,      0,    SCT_NORM,   SC_NORM,  SC_NORM,  SC_NORM,  SC_NORM,
  DecSKnew_name, SC_KEY_TYPE,   2,      0,    SCT_NORM,   SC_NORM,  SC_NORM,  SC_NORM,  SC_NORM,
  DecSKnew_name, SC_KEY_TYPE,   3,      0,    SCT_NORM,   SC_NORM,  SC_NORM,  SC_NORM,  SC_NORM,
  SKnew_name,    SC_KEY_TYPE,   1,      0,    SCT_NORM,   SC_NORM,  SC_NORM,  SC_NORM,  SC_NORM,
  SKold_name,    SC_KEY_TYPE,   2,      0,    SCT_NORM,   SC_NORM,  SC_NORM,  SC_NORM,  SC_NORM,

  AUTH_name,     SC_KEY_TYPE,  27,      0,    SCT_NORM,   SC_NORM,  SC_NORM,  SC_NORM,  SC_NORM,
  SM_AUTH_name,  SC_KEY_TYPE,  28,      0,    SCT_NORM,   SC_NORM,  SC_NORM,  SC_NORM,  SC_NORM,
  SM_CONC_name,  SC_KEY_TYPE,  29,      0,    SCT_NORM,   SC_NORM,  SC_NORM,  SC_NORM,  SC_NORM,
  SC_PIN_name,   SC_KEY_TYPE,  30,      0,    SCT_NORM,   SC_NORM,  SC_NORM,  SC_NORM,  SC_NORM,
  SC_PUK_name,   SC_KEY_TYPE,  31,      0,    SCT_NORM,   SC_NORM,  SC_NORM,  SC_NORM,  SC_NORM,

/* files: */
/*
  Name,		 Type,        short-  file-   sm_SCT,       sm_SC_read         sm_SC_write   
                              index,  size,               command,  response,  cdo.,  resp., 
*/
  SCToc_name,    SC_FILE_TYPE,  1,    480,    SCT_NORM,   SC_NORM,  SC_NORM,  SC_NORM,  SC_NORM,
  SignCert_name, SC_FILE_TYPE,  2,    384,    SCT_NORM,   SC_NORM,  SC_NORM,  SC_NORM,  SC_NORM,
  EncCert_name,  SC_FILE_TYPE,  3,    384,    SCT_NORM,   SC_NORM,  SC_NORM,  SC_NORM,  SC_NORM,
  Cert_name,     SC_FILE_TYPE,  3,    384,    SCT_NORM,   SC_NORM,  SC_NORM,  SC_NORM,  SC_NORM,
  PKRoot_name,   SC_FILE_TYPE,  4,    192,    SCT_NORM,   SC_NORM,  SC_NORM,  SC_NORM,  SC_NORM,
  FCPath_name,   SC_FILE_TYPE,  5,    384,    SCT_NORM,   SC_NORM,  SC_NORM,  SC_NORM,  SC_NORM, 
  PKList_name,   SC_FILE_TYPE,  6,    384,    SCT_NORM,   SC_NORM,  SC_NORM,  SC_NORM,  SC_NORM, 
  PSE_PIN_name,  SC_FILE_TYPE, 15,      8,    SCT_NORM,   SC_NORM,  SC_NORM,  SC_NORM,  SC_NORM,

  0
  };

/*  real values     (not used)

  {
  SignSK_name,   SC_KEY_TYPE,   1,     0,     SCT_NORM,   SC_NORM,  SC_NORM,  SC_NORM,  SC_NORM,
  DecSKnew_name, SC_KEY_TYPE,   2,     0,     SCT_NORM,   SC_NORM,  SC_NORM,  SC_NORM,  SC_NORM,
  DecSKold_name, SC_KEY_TYPE,   3,     0,     SCT_NORM,   SC_NORM,  SC_NORM,  SC_NORM,  SC_NORM,
  SKnew_name,    SC_KEY_TYPE,   1,     0,     SCT_NORM,   SC_NORM,  SC_NORM,  SC_NORM,  SC_NORM,
  SKold_name,    SC_KEY_TYPE,   2,     0,     SCT_NORM,   SC_NORM,  SC_NORM,  SC_NORM,  SC_NORM,

  AUTH_name,     SC_KEY_TYPE,  27,     0,     SCT_NORM,   SC_NORM,  SC_NORM,  SC_NORM,  SC_NORM,
  SM_AUTH_name,  SC_KEY_TYPE,  28,     0,     SCT_NORM,   SC_NORM,  SC_NORM,  SC_NORM,  SC_NORM,
  SM_CONC_name,  SC_KEY_TYPE,  29,     0,     SCT_NORM,   SC_NORM,  SC_NORM,  SC_NORM,  SC_NORM,
  SC_PUK_name,   SC_KEY_TYPE,  30,     0,     SCT_NORM,   SC_NORM,  SC_NORM,  SC_CONC,  SC_NORM,
  SC_PIN_name,   SC_KEY_TYPE,  31,     0,     SCT_NORM,   SC_NORM,  SC_NORM,  SC_CONC,  SC_NORM,

  SCToc_name,    SC_FILE_TYPE,  1,   480,     SCT_NORM,   SC_NORM,  SC_NORM,  SC_NORM,  SC_NORM,
  SignCert_name, SC_FILE_TYPE,  2,   384,     SCT_NORM,   SC_NORM,  SC_AUTH,  SC_AUTH,  SC_NORM,
  EncCert_name,  SC_FILE_TYPE,  3,   384,     SCT_NORM,   SC_NORM,  SC_AUTH,  SC_AUTH,  SC_NORM,
  Cert_name,     SC_FILE_TYPE,  3,   384,     SCT_NORM,   SC_NORM,  SC_NORM,  SC_NORM,  SC_NORM,
  PKRoot_name,   SC_FILE_TYPE,  4,   192,     SCT_NORM,   SC_NORM,  SC_AUTH,  SC_AUTH,  SC_NORM,
  FCPath_name,   SC_FILE_TYPE,  5,   384,     SCT_NORM,   SC_NORM,  SC_AUTH,  SC_AUTH,  SC_NORM, 
  PKList_name,   SC_FILE_TYPE,  6,   384,     SCT_NORM,   SC_NORM,  SC_NORM,  SC_NORM,  SC_NORM,  
  PSE_PIN_name,  SC_FILE_TYPE, 15,     8,     SCT_CONC,   SC_CONC,  SC_CONC,  SC_NORM,  SC_NORM,
  0
  };  */





/*
 *  List of the mandatory objects belonging to one application on the SC:
 */
   
  char	*man_sc_obj_list[MAX_SCOBJ] =
  {
    SCToc_name,
    0
  };




/*
 *  List of the predefined objects belonging to one application on the SC:
 *
 *    These objects and their default values are added automatically
 *    to the object list belonging to one application,
 *    These objects should not be mentioned with the file ".scinit"
 */
   
  char	*predef_sc_obj_list[MAX_SCOBJ] =
  {
    AUTH_name,
    SM_CONC_name,
    SM_AUTH_name,
    PSE_PIN_name,
    SC_PUK_name,
    SC_PIN_name,
    0
  };



/*
 *  List of the mandatory objects belonging to an SC-application with one key pair:
 */
   
  char	*onekeypair_sc_obj_list[MAX_SCOBJ] =
  {
    SKnew_name,
    SKold_name,
    Cert_name,
    PKRoot_name,
    0
  };


/*
 *  List of the mandatory objects belonging to an SC-application with two key pairs:
 */
   
  char	*twokeypairs_sc_obj_list[MAX_SCOBJ] =
  {
    SignSK_name,
    DecSKnew_name,
    DecSKold_name,
    SignCert_name,
    EncCert_name,
    PKRoot_name,
   0
  };



#endif
