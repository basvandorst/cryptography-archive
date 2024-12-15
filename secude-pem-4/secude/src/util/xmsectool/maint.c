/*
 *  SecuDE Release 4.2 (GMD)
 */
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

/*-----------------------maint.c------------------------------------*/
/*                                                                  */
/*------------------------------------------------------------------*/
/* GMD Darmstadt Institute for System Technic (I2)                  */
/* Rheinstr. 75 / Dolivostr. 15                                     */
/* 6100 Darmstadt                                                   */
/* Project ``Secure DFN'' 1990 / "SecuDE" 1991/92/93                */
/* Grimm/Nausester/Schneider/Viebeg/Vollmer 	                    */
/* Luehe/Surkau/Reichelt/Kolletzki		                    */
/*------------------------------------------------------------------*/
/* PACKAGE   util            VERSION   4.2                          */
/*                              DATE   01.11.1993                   */
/*                                BY   ws                           */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* PROGRAM   xmst            VERSION   1.0                          */
/*                              DATE   01.11.1993                   */
/*                                BY   Kolletzki                    */
/*                                                                  */
/*                            REVIEW                                */
/*                              DATE                                */
/*                                BY                                */
/*                                                                  */
/* MODULE    maint                                                  */
/*                                                                  */
/* INCLUDES  xmst.h                                                 */
/*                                                                  */
/* DESCRIPTION                                                      */
/*	sectool routines taken from psemaint.c                      */
/*                                                                  */
/*------------------------------------------------------------------*/



/*
 ************************
 *	INCLUDES	*
 ************************
 */

#include "xmst.h"


/*
 ************************
 *	STATICS		*
 ************************
 */




/* ------------------------------
 *	Build key info for object
 */

Key *
build_key_object(object_name, command, open_flag) 
	char		*object_name;
	commands	command;
	int 		open_flag;  
{
	char		*proc = "build_key_object";
        char 		*cc;
	char 		answ[256];
	int 		i;
        KeyInfo 	zwkey;
        OctetString 	ostring;
	char 		newstring[256];
	Key 		*newkey;



	if (sectool_verbose) fprintf(stderr, "-->%s\n", proc);


	newkey = (Key *)calloc(1, sizeof(Key));

/*******
        if(command == GENKEY) {
                newkey->key = (KeyInfo *)calloc(1, sizeof(KeyInfo));
	        newkey->key->subjectAI = aux_Name2AlgId(algname);
                if(!newkey->key->subjectAI) {
			free(newkey->key);
			free(newkey);
                        return((Key *)0);
                }

        }
*******/

/*******
        if(num(object_name)) {

                if(strlen(object_name) == 0) {
			newkey->keyref = -1;
		}
                else sscanf(object_name, "%d", &(newkey->keyref));
		if(newkey->keyref == 0) newkey->keyref = -1;

		if(command == GENKEY && publickey && newkey->keyref != -1) if(newkey->keyref == publickey->keyref) {
                       	fprintf(stderr, "Public key and secret key must have different keyrefs\n");
			aux_free_Key(&newkey);
			return((Key *)0);
		}
		if(newkey->keyref > 0 && command == GENKEY && replace == FALSE) {
			if(sec_get_key(&zwkey, newkey->keyref, (Key *)0) == 0) {
				aux_free2_KeyInfo(&zwkey);
                               	fprintf(stderr, "Keyref %d exists already. Replace? (y/n): ", newkey->keyref);
                               	gets(answ);
                               	if(answ[0] == 'y') {
					sec_del_key(newkey->keyref);
					if(strcmp(s, "generated secret ") == 0) replace = TRUE;
				}
				else {
					aux_free_Key(&newkey);
					return((Key *)0);
				}
			} 
		}
        }
        else
*******/
		{

                newkey->pse_sel = aux_cpy_PSESel(&std_pse);
		strrep(&(newkey->pse_sel->object.name), object_name);

		for (i = 0; i < PSE_MAXOBJ; i++) 
			if(strcmp(AF_pse.object[i].name, newkey->pse_sel->object.name) == 0) {
				strrep(&(newkey->pse_sel->object.pin), AF_pse.object[i].pin);
				break;
               		}



/*******
		if(command == GENKEY) {
			if(publickey) if(strcmp(newkey->pse_sel->object.name, publickey->pse_sel->object.name) == 0) {
	                       	fprintf(stderr, "Public key and secret key must be stored in different objects\n");
				aux_free_Key(&newkey);
				return((Key *)0);
			}
#ifdef SCA
			if(sec_psetest(newkey->pse_sel->app_name) == SCpse && handle_in_SCTSC(newkey) == TRUE) {
				if(strcmp(s, "generated secret ") == 0) {
                                	fprintf(stderr, "Replace existing secret key? (y/n): ");
                                	gets(answ);
                                	if(answ[0] == 'y') replace = TRUE;    
				}
				return(newkey);
                        }
#endif
		}
*******/


                if(sec_open(newkey->pse_sel) < 0) {

			/* READ only */
                        if(open_flag == OPEN_TO_READ) {
                                fprintf(stderr, "Can't open Object %s\n", newkey->pse_sel->object.name);
				aux_add_error(EINVALID, "sec_open failed", CNULL, 0, proc);
				aux_free_Key(&newkey);

				/*
				 *	Error: Quit/Continue
				 */
				sprintf(dialog_message, "Can't open Object\n%s", object_name);
				cont_quit_dialog_open(dialog_message);

                                return((Key *)0);
                        }

			/* Create object */
                        if (verbose) fprintf(stderr, "Create object %s\n", newkey->pse_sel->object.name);

                        strrep(&(newkey->pse_sel->object.pin), newkey->pse_sel->pin);
                        if(sec_create(newkey->pse_sel) < 0) {
                                fprintf(stderr, "Can't create Object %s\n", newkey->pse_sel->object.name);
				aux_add_error(EINVALID, "sec_open failed", CNULL, 0, proc);
				aux_free_Key(&newkey);

				/*
				 *	Error: Quit/Continue
				 */
				sprintf(dialog_message, "Can't create Object\n%s", object_name);
				cont_quit_dialog_open(dialog_message);

                                return((Key *)0);
                        }

/*******		if(strcmp(s, "generated public ")) replace = TRUE;
*******/
			for (i = 0; i < PSE_MAXOBJ; i++) {
				if (!strcmp(AF_pse.object[i].name, newkey->pse_sel->object.name)) {
					strrep(&(AF_pse.object[i].pin), newkey->pse_sel->object.pin);
					break;
                      	        }
			}
                        return(newkey);
                }
 
		for (i = 0; i < PSE_MAXOBJ; i++) {
			if (!strcmp(AF_pse.object[i].name, newkey->pse_sel->object.name)) {
				strrep(&(AF_pse.object[i].pin), newkey->pse_sel->object.pin);
				break;
                        }
                        else if(open_flag == OPEN_TO_WRITE) {
				if(command == GENKEY && replace == TRUE) return(newkey);

				/*
				 *	Overwrite: Confirm/Cancel
				 */
/***				if (!notice_confcancel(item, "Overwrite existing object ?"))  {

					aux_free_Key(&newkey);
					return((Key *)0);

				}
***/

/*******			if(strcmp(s, "generated public ")) replace = TRUE;
*******/

                                return(newkey);
                        }
                }
        }


        return(newkey);
}


/*
 *	stores object pin returned from "pin_check()" in AF_pse
 */
void
store_objpin()
{
	int    		i;
	char 		*proc = "store_objpin";


	for (i = 0; i < PSE_MAXOBJ; i++) 
		if (strcmp(AF_pse.object[i].name, std_pse.object.name) == 0) {
			strrep(&(AF_pse.object[i].pin), std_pse.object.pin);
		}
}



