/*
 *  SecuDE Release 4.3 (GMD)
 */
/********************************************************************
 * Copyright (C) 1994, GMD. All rights reserved.                    *
 *                                                                  *
 *                                                                  *
 *                         NOTICE                                   *
 *                                                                  *
 *    Acquisition, use, and distribution of this module             *
 *    and related materials are subject to restrictions             *
 *    mentioned in each volume of the documentation.                *
 *                                                                  *
 ********************************************************************/

#include "af.h"
#include <stdio.h>
#include <errno.h>

#ifdef __STDC__
	static void clrpin(char *pin);
	static void deletepin(char **pin);
#else
	static void clrpin();
	static void deletepin();
#endif

static Certificate *SignCert_cache;
static Certificate *EncCert_cache;
static Certificate *Cert_cache;
static SET_OF_Certificate *SignCSet_cache;
static SET_OF_Certificate *EncCSet_cache;
static SET_OF_Certificate *CSet_cache;
static KeyInfo *SignSK_cache;
static KeyInfo *DecSKnew_cache;
static KeyInfo *DecSKold_cache;
static KeyInfo *SKnew_cache;
static KeyInfo *SKold_cache;
static FCPath *FCPath_cache;
static PKRoot *PKRoot_cache;
static PKList *PKList_cache;
static PKList *EKList_cache;
static PKList *PCAList_cache;
static SET_OF_CertificatePair *CrossCSet_cache;
static CrlSet *CrlSet_cache;
static SerialNumber *SerialNumber_cache;
static AliasList *AliasList_cache;
static QuipuPWD QuipuPWD_cache;
static AlgId *DHparam_cache;

static Boolean SignCert_read;
static Boolean EncCert_read;
static Boolean Cert_read;
static Boolean SignCSet_read;
static Boolean EncCSet_read;
static Boolean CSet_read;
static Boolean SignSK_read;
static Boolean DecSKnew_read;
static Boolean DecSKold_read;
static Boolean SKnew_read;
static Boolean SKold_read;
static Boolean FCPath_read;
static Boolean PKRoot_read;
static Boolean PKList_read;
static Boolean EKList_read;
static Boolean PCAList_read;
static Boolean CrossCSet_read;
static Boolean CrlSet_read;
static Boolean SerialNumber_read;
static Boolean AliasList_read;
static Boolean QuipuPWD_read;
static Boolean DHparam_read;

/****************************************************************************/

/***************************************************************
 *
 * Procedure af_pse_open
 *
 ***************************************************************/
#ifdef __STDC__

PSESel *af_pse_open(
	ObjId	 *af_object,
	Boolean	  create
)

#else

PSESel *af_pse_open(
	af_object,
	create
)
ObjId	 *af_object;
Boolean	  create;

#endif

{
	PSESel     * pse;
	int	     i, obj_index;
	PSELocation  pse_location;
	char	   * proc = "af_pse_open";

	if (!(pse = (PSESel * )calloc(1, sizeof(PSESel)))) {
		aux_add_error(EMALLOC, "pse", CNULL, 0, proc);
		return ( (PSESel *) 0);
	}

	if (af_object) {
		for (obj_index = 0; obj_index < PSE_MAXOBJ; obj_index++) {
			if (aux_cmp_ObjId(af_object, AF_pse.object[obj_index].oid) == 0) 
				break;
		}
		if (obj_index == PSE_MAXOBJ) {
			aux_add_error(EINVALID, "Invalid ObjId", (char *) af_object, ObjId_n, proc);
			aux_free_PSESel(&pse);
			return ( (PSESel *) 0);
		}
	}

	if(!(pse->app_name = aux_cpy_String(AF_pse.app_name))) {
		aux_add_error(EINVALID, "No PSE name given", CNULL, 0, proc);
		aux_free_PSESel(&pse);
		return ( (PSESel *) 0);
	}
	pse->app_id   = AF_pse.app_id;
	pse->pin      = aux_cpy_String(AF_pse.pin);
	if (!af_object) {
		pse->object.name  = CNULL;
		pse->object.pin  = CNULL;
	} 
	else {
		pse->object.name = aux_cpy_String(AF_pse.object[obj_index].name);
		pse->object.pin = aux_cpy_String(AF_pse.object[obj_index].pin);
/*		if (!pse->object.pin) {
			pse->object.pin = aux_cpy_String(pse->pin);
		} */
	}


	/* check if already opened */
	pse_location = sec_psetest(pse->app_name);
#ifdef SCA
	if (pse_location == ERR_in_psetest) {
		if (aux_last_error() == EDEVLOCK) 
			aux_add_error(EDEVLOCK, "Cannot open device for SCT (No such device or device busy).", CNULL, 0, proc);
		else	AUX_ADD_ERROR;
		aux_free_PSESel(&pse);
		return ((PSESel *) 0);
	}
#endif

	if ((af_object && AF_pse.open && AF_pse.object[obj_index].open)
/*	   || (af_object && AF_pse.open && (pse_location == SCpse)) */
           || (!af_object && AF_pse.open) ) {
		return (pse);
	}

	/* open PSE */

	if (create) sec_create(pse);
	if (sec_open(pse) < 0) {
		AUX_ADD_ERROR;

		/* save PINs */

		AF_pse.app_id = pse->app_id;
		if (pse->pin) {
			deletepin(&AF_pse.pin);
			AF_pse.pin = aux_cpy_String(pse->pin);
		}
		if (af_object && pse->object.pin) {
			deletepin(&(AF_pse.object[obj_index].pin));
			AF_pse.object[obj_index].pin = aux_cpy_String(pse->object.pin);
		}

		aux_free_PSESel(&pse);
		return ( (PSESel *) 0);
	}

	if(af_object) AF_pse.object[obj_index].open = TRUE;
	AF_pse.open = TRUE;


	/* set PSE descriptor */

	AF_pse.app_id = pse->app_id;

	deletepin(&AF_pse.pin);
	if (af_object)  deletepin(&(AF_pse.object[obj_index].pin));

	if (pse->pin) AF_pse.pin = aux_cpy_String(pse->pin);
	if (af_object && pse->object.pin) AF_pse.object[obj_index].pin = aux_cpy_String(pse->object.pin);

	return(pse);
}


/****************************************************************************/

/***************************************************************
 *
 * Procedure af_pse_create
 *
 ***************************************************************/
#ifdef __STDC__

PSESel *af_pse_create(
	ObjId	 *af_object
)

#else

PSESel *af_pse_create(
	af_object
)
ObjId	 *af_object;

#endif

{
	PSESel  * pse;
	int	  i, obj_index;
	char	* proc = "af_pse_create";


	if (!(pse = (PSESel * )calloc(1, sizeof(PSESel)))) {
		aux_add_error(EMALLOC, "pse", CNULL, 0, proc);
		return ( (PSESel *) 0);
	}

	if (af_object) {
		for (obj_index = 0; obj_index < PSE_MAXOBJ; obj_index++) {
			if (aux_cmp_ObjId(af_object, AF_pse.object[obj_index].oid) == 0) 
				break;
		}
		if (obj_index == PSE_MAXOBJ) {
			aux_add_error(EINVALID, "Can't create object: Invalid ObjId", (char *) af_object, ObjId_n, proc);
			free(pse);
			return ( (PSESel *) 0);
		}
	}

	pse->app_name = aux_cpy_String(AF_pse.app_name);
	pse->app_id   = AF_pse.app_id;
	pse->pin = aux_cpy_String(AF_pse.pin);
	if (!af_object) {
		pse->object.name  = CNULL;
		pse->object.pin  = CNULL;
	}
	else {
		pse->object.name = aux_cpy_String(AF_pse.object[obj_index].name);
		pse->object.pin = aux_cpy_String(AF_pse.object[obj_index].pin);
		if (!pse->object.pin) {
			pse->object.pin = aux_cpy_String(pse->pin);
			AF_pse.object[obj_index].pin = aux_cpy_String(pse->pin);
		}
	}

	/* create PSE */

	if (sec_create(pse) < 0) {    /* inherit error descriptor */
		aux_add_error(LASTERROR, "Can't create object", (char *) pse, PSESel_n, proc);
		aux_free_PSESel(& pse);
		return ( (PSESel *) 0);
	}

	/* set PSE descriptor */
	deletepin(&AF_pse.pin);
	AF_pse.pin = aux_cpy_String(pse->pin);
	AF_pse.app_id = pse->app_id;
	if (af_object) {
		deletepin(&(AF_pse.object[obj_index].pin));
		AF_pse.object[obj_index].pin = aux_cpy_String(pse->object.pin);
	}

	return(pse);
}


/****************************************************************************/

/***************************************************************
 *
 * Procedure af_pse_close
 *
 ***************************************************************/
#ifdef __STDC__

RC af_pse_close(
	ObjId	 *af_object
)

#else

RC af_pse_close(
	af_object
)
ObjId	 *af_object;

#endif

{
	PSESel         pse;
	int	       i, obj_index;
	char	     * proc = "af_pse_close";

	if (af_object) {
		for (obj_index = 0; obj_index < PSE_MAXOBJ; obj_index++) {
			if (aux_cmp_ObjId(af_object, AF_pse.object[obj_index].oid) == 0) 
				break;
		}
		if (obj_index == PSE_MAXOBJ) {
			aux_add_error(EINVALID, "Can't close object: Invalid ObjId", (char *) af_object, ObjId_n, proc);
			return (- 1);
		}
	}

	/* Check if already opened: If PSE or object "af_object" is not open, return error */
	if ((af_object && (!AF_pse.pin || !AF_pse.object[obj_index].pin)) || (!af_object && !AF_pse.pin)) {
		aux_add_error(EINVALID, "Can't close object: Is not open", CNULL, 0, proc);
		return - 1;
	}

	if (!af_object) {  /* clear all PIN's */
		for (i = 0; i < PSE_MAXOBJ; i++) {
			deletepin(&(AF_pse.object[i].pin));
			AF_pse.object[i].open = FALSE;
		} 
		deletepin(&AF_pse.pin);

	        AF_pse.open = FALSE;

		/* Free the internal object cache of each object */

		af_pse_reset(CNULL);

		pse.object.name  = CNULL;

	}
	else {  /* clear PIN of object identified by "af_object" */

		deletepin(&(AF_pse.object[obj_index].pin));
		AF_pse.object[obj_index].open = FALSE;

		pse.object.name  = AF_pse.object[obj_index].name;
	}

	/* close PSE */
	pse.app_name = AF_pse.app_name;
	pse.pin      = CNULL;
	pse.app_id   = AF_pse.app_id;
	pse.object.pin   = CNULL;

	if (sec_close(&pse) < 0) {    /* inherit error descriptor */
		aux_add_error(LASTERROR, "Can't close object", (char *) &pse, PSESel_n, proc);
		return - 1;
	}

	return 0;
}


/***************************************************************
 *
 * Procedure af_pse_rename
 *
 ***************************************************************/
#ifdef __STDC__

RC af_pse_rename(
	char	 *oldname,
        char     *newname
)

#else

RC af_pse_rename(
	oldname,
        newname
)
char	 *oldname, *newname;

#endif

{
	PSESel         pse;
	int	       i, obj_index;
	char	     * proc = "af_pse_rename";

	/* rename object */
	pse.app_name = AF_pse.app_name;
	pse.pin      = AF_pse.pin;
	pse.app_id   = AF_pse.app_id;
	pse.object.name   = oldname;
	pse.object.pin   = CNULL;

	af_pse_reset(oldname);
	if (sec_rename(&pse, newname) < 0) {    /* inherit error descriptor */
		aux_add_error(LASTERROR, "Can't rename object", (char *) &pse, PSESel_n, proc);
		return - 1;
	}
	af_pse_reset(newname);

	return 0;
}


/***************************************************************
 *
 * Procedure af_pse_get
 *
 * Return (void *) to object with given name, a valid type is
 *   returned in parameter (ObjId *)objtype, 
 *   else object is the decoded OctetString, objtype is Uid_OID
 * Return (void *)NULL on error 
 *
 ***************************************************************/
#ifdef __STDC__

void *af_pse_get(
	char	 *objname,
	ObjId	 *objtype
)

#else

void *af_pse_get(
	objname,
	objtype
)
char	 *objname;
ObjId	 *objtype;

#endif

{
	char			*proc = "af_pse_get";
	PSESel			*pse_sel;
	OctetString		content;
	ObjId	 		objid;
	ObjId	 		*type;
	void			*opaque;

	
	if (!objname || !strcmp(objname, "") || !objtype) {
		aux_add_error(EINVALID, "Invalid parameter objname or objtype", CNULL, 0, proc);
		return((void *)NULL);
	}

	/* Cached standard objects */
	type = af_get_objoid(objname);
	aux_cpy2_ObjId(objtype, type);
	aux_free_ObjId(&type);
	
	if (!strcmp(objname, SignCert_name))
		return((void *)af_pse_get_Certificate(SIGNATURE, NULLDNAME, 0));
	else if (!strcmp(objname, EncCert_name)) 
		return((void *)af_pse_get_Certificate(ENCRYPTION, NULLDNAME, 0));
	else if (!strcmp(objname, Cert_name)) 
		return((void *)af_pse_get_Certificate(SIGNATURE, NULLDNAME, 0));
	else if (!strcmp(objname, SignCSet_name)) 
		return((void *)af_pse_get_CertificateSet(SIGNATURE));
	else if (!strcmp(objname, EncCSet_name)) 
		return((void *)af_pse_get_CertificateSet(ENCRYPTION));
	else if (!strcmp(objname, CSet_name)) 
		return((void *)af_pse_get_CertificateSet(SIGNATURE));
	else if (!strcmp(objname, CrossCSet_name)) 
		return((void *)af_pse_get_CertificatePairSet(SIGNATURE, NULLDNAME, 0));
	else if (!strcmp(objname, FCPath_name)) 
		return((void *)af_pse_get_FCPath(NULLDNAME));
	else if (!strcmp(objname, PKRoot_name)) 
		return((void *)af_pse_get_PKRoot());
	else if (!strcmp(objname, PKList_name)) 
		return((void *)af_pse_get_PKList(SIGNATURE));
	else if (!strcmp(objname, EKList_name)) 
		return((void *)af_pse_get_PKList(ENCRYPTION));
	else if (!strcmp(objname, PCAList_name)) 
		return((void *)af_pse_get_PCAList());
	else if (!strcmp(objname, CrlSet_name)) 
		return((void *)af_pse_get_CrlSet());
	else if (!strcmp(objname, SerialNumber_name)) 
		return((void *)af_pse_get_SerialNumber());
	else if (!strcmp(objname, AliasList_name)) 
		return((void *)af_pse_get_AliasList());
	else if (!strcmp(objname, QuipuPWD_name)) 
		return((void *)af_pse_get_QuipuPWD());
	else if (!strcmp(objname, DHparam_name)) 
		return((void *)af_pse_get_DHparam());
	else if (!strcmp(objname, Random_name))		;	/* ??? treated as OctetString */

	aux_free2_ObjId(objtype);
	opaque = (void *)NULL;

	/* Get non-standard object */
	if (!(pse_sel = af_pse_open(NULL, FALSE))) {
		aux_add_error(LASTERROR, "Can't get pse_sel", CNULL, 0, proc);
		return((void *)NULL);
	}
	pse_sel->object.name = aux_cpy_String(objname);
	pse_sel->object.pin = aux_cpy_String(AF_pse.pin);
	
        if(sec_read_PSE(pse_sel, &objid, &content) < 0) {
		aux_add_error(EREADPSE, "Can't read object", CNULL, 0, proc);
		goto readerror;
	}
	if(aux_cmp_ObjId(&objid, SignSK_OID) == 0
	   || aux_cmp_ObjId(&objid, DecSKnew_OID) == 0 
	   || aux_cmp_ObjId(&objid, DecSKold_OID) == 0
	   || aux_cmp_ObjId(&objid, SKnew_OID) == 0 
	   || aux_cmp_ObjId(&objid, RSA_SK_OID) == 0 
	   || aux_cmp_ObjId(&objid, DSA_SK_OID) == 0 
	   || aux_cmp_ObjId(&objid, SKold_OID) == 0) {
		if(!(opaque = d_KeyInfo(&content))) goto decodeerr;
	}
	else if(aux_cmp_ObjId(&objid, RSA_PK_OID) == 0
	        || aux_cmp_ObjId(&objid, DSA_PK_OID) == 0) {
		if(!(opaque = d_KeyInfo(&content))) goto decodeerr;
	}
	else if(aux_cmp_ObjId(&objid, DHkey_OID) == 0) {
		if(!(opaque = d_BitString(&content))) goto decodeerr;
	}
	else if(aux_cmp_ObjId(&objid, DHprivate_OID) == 0) {
		if(!(opaque = d_KeyInfo(&content))) goto decodeerr;
	}
	else {
		aux_free2_ObjId(&objid);
		if((opaque = d_Certificate(&content)))			aux_cpy2_ObjId(&objid, Cert_OID);
		else if((opaque = d_CertificateSet(&content)))		aux_cpy2_ObjId(&objid, CSet_OID);
		else if((opaque = d_CertificatePairSet(&content)))	aux_cpy2_ObjId(&objid, CrossCSet_OID);
		else if((opaque = d_CrlSet(&content)))			aux_cpy2_ObjId(&objid, CrlSet_OID);
		else if((opaque = d_DName(&content)))			aux_cpy2_ObjId(&objid, Name_OID);
		else if((opaque = d_FCPath(&content)))			aux_cpy2_ObjId(&objid, FCPath_OID);
		else if((opaque = d_PKRoot(&content)))			aux_cpy2_ObjId(&objid, PKRoot_OID);
		else if((opaque = d_PKList(&content)))			aux_cpy2_ObjId(&objid, PKList_OID);
		else if((opaque = d_KeyInfo(&content)))			aux_cpy2_ObjId(&objid, KeyInfo_OID);
		else if((opaque = d_AliasList(&content)))		aux_cpy2_ObjId(&objid, AliasList_OID);
		else if((opaque = d_GRAPHICString(&content)))		aux_cpy2_ObjId(&objid, QuipuPWD_OID);
		else {
			opaque = aux_cpy_OctetString(&content);
			aux_cpy2_ObjId(&objid, Uid_OID);
		}
	}
	
	aux_cpy2_ObjId(objtype, &objid);
	goto noerror;

decodeerr:
	aux_add_error(EDECODE, "Can't decode object", CNULL, 0, proc);
noerror:
	aux_free2_ObjId(&objid);
readerror:
	if(content.octets) free(content.octets);
        aux_free_PSESel(&pse_sel);
        
        return((void *)opaque);
}


/***************************************************************
 *
 * Procedure af_pse_update
 *
 * Update / Create object in PSE
 * opaque/opaque_type can either contain an object of a standard
 *   type, or an OctetString containing an encoded object
 *   (opaque_type is Uid_OID).
 * If a standard object is selected, opaque_type has to be the
 *   fitting standard OID;
 *   an OctetString will be decoded to the fitting type to check
 *   its validity.
 * Return -1 on error 
 *
 ***************************************************************/
#ifdef __STDC__

RC af_pse_update(
	char	 *objname,
	void	 *opaque,
	ObjId	 *opaque_type
)

#else

RC af_pse_update(
	objname,
	opaque,
	opaque_type
)
char	 *objname;
void     *opaque;
ObjId	 *opaque_type;

#endif

{
	char			*proc = "af_pse_update";
	PSESel			*pse_sel;
	ObjId	 		*type;
	ObjId	 		decoded_type;
	OctetString		*content = NULLOCTETSTRING;
	void			*euqapo = NULL;
	int			rcode;

	
	if (!objname || !strcmp(objname, "") || !opaque || !opaque_type) {
		aux_add_error(EINVALID, "Invalid parameter objname, opaque, or opaque_type", CNULL, 0, proc);
		return(-1);
	}

	type = af_get_objoid(objname);

	/* standard type */
	if (aux_cmp_ObjId(opaque_type, Uid_OID)) {
	
		/* standard object */
		if (aux_cmp_ObjId(type, Uid_OID)) {
		
			/* Test: standard OID fits */
			if (aux_cmp_ObjId(opaque_type, type))
				euqapo = af_pse_cpy_opaque(objname, opaque, opaque_type);
			else {
				aux_add_error(EINVALID, "Invalid OID for standard object", CNULL, 0, proc);
				aux_free_ObjId(&type);
				return(-1);
			}
				
		/* non-standard object */
		}
		else content = af_pse_encode(CNULL, opaque, opaque_type);
			
	/* opaque is OctetString */
	} else {	
	
		/* standard object */
		if (aux_cmp_ObjId(type, Uid_OID)) {
		
			/* Test: standard OID fits */
			euqapo = af_pse_decode(objname, opaque, &decoded_type);
			aux_free2_ObjId(&decoded_type);
			if (!euqapo) {
				aux_add_error(EDECODE, "Can't decode opaque to standard object", CNULL, 0, proc);
				aux_free_ObjId(&type);
				return(-1);
			}
			
		/* non-standard object */
		}
		else content = aux_cpy_OctetString((OctetString *)opaque);
	}
	/* either euqapo or content contains object data */

	/* cached standard object */
	if (euqapo) {
	
		if (!strcmp(objname, SignCert_name))
			rcode = af_pse_update_Certificate(SIGNATURE, (Certificate *)euqapo, TRUE);
		else if (!strcmp(objname, EncCert_name)) 
			rcode = af_pse_update_Certificate(ENCRYPTION, (Certificate *)euqapo, TRUE);
		else if (!strcmp(objname, Cert_name)) 
			rcode = af_pse_update_Certificate(SIGNATURE, (Certificate *)euqapo, TRUE);
		else if (!strcmp(objname, SignCSet_name)) 
			rcode = af_pse_update_Certificate(SIGNATURE, (Certificate *)euqapo, FALSE);
		else if (!strcmp(objname, EncCSet_name)) 
			rcode = af_pse_update_Certificate(ENCRYPTION, (Certificate *)euqapo, FALSE);
		else if (!strcmp(objname, CSet_name)) 
			rcode = af_pse_update_Certificate(SIGNATURE, (Certificate *)euqapo, FALSE);
		else if (!strcmp(objname, CrossCSet_name)) 
			rcode = af_pse_update_CertificatePairSet((SET_OF_CertificatePair *)euqapo);
		else if (!strcmp(objname, FCPath_name)) 
			rcode = af_pse_update_FCPath((FCPath *)euqapo);
		else if (!strcmp(objname, PKRoot_name)) 
			rcode = af_pse_update_PKRoot((PKRoot *)euqapo);
		else if (!strcmp(objname, PKList_name)) 
			rcode = af_pse_update_PKList(SIGNATURE, (PKList *)euqapo);
		else if (!strcmp(objname, EKList_name)) 
			rcode = af_pse_update_PKList(ENCRYPTION, (PKList *)euqapo);
		else if (!strcmp(objname, PCAList_name)) 
			rcode = af_pse_update_PCAList((PKList *)euqapo);
		else if (!strcmp(objname, CrlSet_name)) 
			rcode = af_pse_update_CrlSet((CrlSet *)euqapo);
		else if (!strcmp(objname, SerialNumber_name)) 
			rcode = af_pse_update_SerialNumber((SerialNumber *)euqapo);
		else if (!strcmp(objname, AliasList_name)) 
			rcode = af_pse_update_AliasList((AliasList *)euqapo);
		else if (!strcmp(objname, QuipuPWD_name)) 
			rcode = af_pse_update_QuipuPWD((QuipuPWD)euqapo);
		else if (!strcmp(objname, DHparam_name)) 
			rcode = af_pse_update_DHparam((AlgId *)euqapo);
			
		if (rcode >= 0) {
			af_pse_free(euqapo, type);
		}
			
	} else {
	
		/* write uncached object */
		if (aux_cmp_ObjId(type, Uid_OID)) {
			if (!(pse_sel = af_pse_open(type, TRUE))) {
				aux_add_error(LASTERROR, "Can't open object", CNULL, 0, proc);
				aux_free_ObjId(&type);	
				return(-1);
			}
		} else {
			if (!(pse_sel = af_pse_open((ObjId *)NULL, TRUE))) {
				aux_add_error(LASTERROR, "Can't open object", CNULL, 0, proc);
				aux_free_ObjId(&type);	
				return(-1);
			}
			pse_sel->object.name = aux_cpy_String(objname);
			pse_sel->object.pin = aux_cpy_String(AF_pse.pin);
		}
                if (sec_open(pse_sel)) rcode = sec_create(pse_sel);

                if (!rcode) {
                	if (sec_write_PSE(pse_sel, opaque_type, content)) {
				aux_add_error(EREADPSE, "Can't write object", CNULL, 0, proc);
				aux_free_OctetString(&content);
		        	aux_free_PSESel(&pse_sel);
				aux_free_ObjId(&type);	
		        	return(-1);
		        }
		} else {
			aux_add_error(EREADPSE, "Can't create object", CNULL, 0, proc);
			aux_free_OctetString(&content);
	        	aux_free_PSESel(&pse_sel);
			aux_free_ObjId(&type);	
	        	return(-1);
		}
		
		aux_free_OctetString(&content);
	        aux_free_PSESel(&pse_sel);
	}
        
	aux_free_ObjId(&type);	
	
	
        return(0);
}

/***************************************************************
 *
 * Procedure af_pse_encode
 *
 * encode opaque data to OctetString
 * If a standard PSE object name is given, the fitting encoding
 *   routine will be used, opaque_type is ignored.
 * Elsewhere, opaque_type selects the encoding routine.
 * If objname is omitted, opaque_type is needed, too.
 * Return NULLOCTETSTRING on error 
 *
 ***************************************************************/
#ifdef __STDC__

OctetString *af_pse_encode(
	char	 *objname,
	void	 *opaque,
	ObjId	 *opaque_type
)

#else

OctetString *af_pse_encode(
	objname,
	opaque,
	opaque_type
)
char	 *objname;
void     *opaque;
ObjId	 *opaque_type;

#endif

{
	char			*proc = "af_pse_encode";
	ObjId	 		*type;	

	
	if (!opaque || !opaque_type) {
		aux_add_error(EINVALID, "Invalid parameter objname, opaque, or opaque_type", CNULL, 0, proc);
		return(NULLOCTETSTRING);
	}

	if (!objname) type = aux_cpy_ObjId(Uid_OID);
	else type = af_get_objoid(objname);

	if (!aux_cmp_ObjId(type, SignCert_OID)
	    || (!aux_cmp_ObjId(type, Uid_OID) && !aux_cmp_ObjId(opaque_type, SignCert_OID))) {
	    	aux_free_ObjId(&type);
		return(e_Certificate((Certificate *)opaque));
	}
	else if (!aux_cmp_ObjId(type, EncCert_OID)
	    || (!aux_cmp_ObjId(type, Uid_OID) && !aux_cmp_ObjId(opaque_type, EncCert_OID))) { 
	    	aux_free_ObjId(&type);
		return(e_Certificate((Certificate *)opaque));
	}
	else if (!aux_cmp_ObjId(type, Cert_OID)
	    || (!aux_cmp_ObjId(type, Uid_OID) && !aux_cmp_ObjId(opaque_type, Cert_OID))) { 
	    	aux_free_ObjId(&type);
		return(e_Certificate((Certificate *)opaque));
	}
	else if (!aux_cmp_ObjId(type, SignCSet_OID)
	    || (!aux_cmp_ObjId(type, Uid_OID) && !aux_cmp_ObjId(opaque_type, SignCSet_OID))) { 
	    	aux_free_ObjId(&type);
		return(e_CertificateSet((SET_OF_Certificate *)opaque));
	}
	else if (!aux_cmp_ObjId(type, EncCSet_OID)
	    || (!aux_cmp_ObjId(type, Uid_OID) && !aux_cmp_ObjId(opaque_type, EncCSet_OID))) { 
	    	aux_free_ObjId(&type);
		return(e_CertificateSet((SET_OF_Certificate *)opaque));
	}
	else if (!aux_cmp_ObjId(type, CSet_OID)
	    || (!aux_cmp_ObjId(type, Uid_OID) && !aux_cmp_ObjId(opaque_type, CSet_OID))) { 
	    	aux_free_ObjId(&type);
		return(e_CertificateSet((SET_OF_Certificate *)opaque));
	}
	else if (!aux_cmp_ObjId(type, CrossCSet_OID)
	    || (!aux_cmp_ObjId(type, Uid_OID) && !aux_cmp_ObjId(opaque_type, CrossCSet_OID))) { 
	    	aux_free_ObjId(&type);
		return(e_CertificatePairSet((SET_OF_CertificatePair *)opaque));
	}
	else if (!aux_cmp_ObjId(type, FCPath_OID)
	    || (!aux_cmp_ObjId(type, Uid_OID) && !aux_cmp_ObjId(opaque_type, FCPath_OID))) { 
	    	aux_free_ObjId(&type);
		return(e_FCPath((FCPath *)opaque));
	}
	else if (!aux_cmp_ObjId(type, PKRoot_OID)
	    || (!aux_cmp_ObjId(type, Uid_OID) && !aux_cmp_ObjId(opaque_type, PKRoot_OID))) { 
	    	aux_free_ObjId(&type);
		return(e_PKRoot((PKRoot *)opaque));
	}
	else if (!aux_cmp_ObjId(type, PKList_OID)
	    || (!aux_cmp_ObjId(type, Uid_OID) && !aux_cmp_ObjId(opaque_type, PKList_OID))) { 
	    	aux_free_ObjId(&type);
		return(e_PKList((PKList *)opaque));
	}
	else if (!aux_cmp_ObjId(type, EKList_OID)
	    || (!aux_cmp_ObjId(type, Uid_OID) && !aux_cmp_ObjId(opaque_type, EKList_OID))) { 
	    	aux_free_ObjId(&type);
		return(e_PKList((PKList *)opaque));
	}
	else if (!aux_cmp_ObjId(type, PCAList_OID)
	    || (!aux_cmp_ObjId(type, Uid_OID) && !aux_cmp_ObjId(opaque_type, PCAList_OID))) { 
	    	aux_free_ObjId(&type);
		return(e_PKList((PKList *)opaque));
	}
	else if (!aux_cmp_ObjId(type, CrlSet_OID)
	    || (!aux_cmp_ObjId(type, Uid_OID) && !aux_cmp_ObjId(opaque_type, CrlSet_OID))) { 
	    	aux_free_ObjId(&type);
		return(e_CrlSet((CrlSet *)opaque));
	}
	else if (!aux_cmp_ObjId(type, Name_OID)
	    || (!aux_cmp_ObjId(type, Uid_OID) && !aux_cmp_ObjId(opaque_type, Name_OID))) { 
	    	aux_free_ObjId(&type);
		return(e_DName((DName *)opaque));
	}
	else if (!aux_cmp_ObjId(type, SerialNumber_OID)
	    || (!aux_cmp_ObjId(type, Uid_OID) && !aux_cmp_ObjId(opaque_type, SerialNumber_OID))) { 
	    	aux_free_ObjId(&type);
		return(e_OctetString((OctetString *)opaque));
	}
	else if (!aux_cmp_ObjId(type, AliasList_OID)
	    || (!aux_cmp_ObjId(type, Uid_OID) && !aux_cmp_ObjId(opaque_type, AliasList_OID))) { 
	    	aux_free_ObjId(&type);
		return(e_AliasList((AliasList *)opaque));
	}
	else if (!aux_cmp_ObjId(type, QuipuPWD_OID)
	    || (!aux_cmp_ObjId(type, Uid_OID) && !aux_cmp_ObjId(opaque_type, QuipuPWD_OID))) { 
	    	aux_free_ObjId(&type);
		return(e_GRAPHICString((QuipuPWD)opaque));
	}
	else if (!aux_cmp_ObjId(type, KeyInfo_OID)
	    || (!aux_cmp_ObjId(type, Uid_OID) && !aux_cmp_ObjId(opaque_type, KeyInfo_OID))) {
	    	aux_free_ObjId(&type);
		return(e_KeyInfo((KeyInfo *)opaque));
	}
	else if (!aux_cmp_ObjId(type, SignSK_OID)
	    || (!aux_cmp_ObjId(type, Uid_OID) && !aux_cmp_ObjId(opaque_type, SignSK_OID))) {
	    	aux_free_ObjId(&type);
		return(e_KeyInfo((KeyInfo *)opaque));
	}
	else if (!aux_cmp_ObjId(type, DecSKnew_OID)
	    || (!aux_cmp_ObjId(type, Uid_OID) && !aux_cmp_ObjId(opaque_type, DecSKnew_OID))) {
	    	aux_free_ObjId(&type);
		return(e_KeyInfo((KeyInfo *)opaque));
	}
	else if (!aux_cmp_ObjId(type, DecSKold_OID)
	    || (!aux_cmp_ObjId(type, Uid_OID) && !aux_cmp_ObjId(opaque_type, DecSKold_OID))) {
	    	aux_free_ObjId(&type);
		return(e_KeyInfo((KeyInfo *)opaque));
	}
	else if (!aux_cmp_ObjId(type, SKnew_OID)
	    || (!aux_cmp_ObjId(type, Uid_OID) && !aux_cmp_ObjId(opaque_type, SKnew_OID))) {
	    	aux_free_ObjId(&type);
		return(e_KeyInfo((KeyInfo *)opaque));
	}
	else if (!aux_cmp_ObjId(type, SKold_OID)
	    || (!aux_cmp_ObjId(type, Uid_OID) && !aux_cmp_ObjId(opaque_type, SKold_OID))) {
	    	aux_free_ObjId(&type);
		return(e_KeyInfo((KeyInfo *)opaque));
	}
	else if (!aux_cmp_ObjId(type, RSA_SK_OID)
	    || (!aux_cmp_ObjId(type, Uid_OID) && !aux_cmp_ObjId(opaque_type, RSA_SK_OID))) {
	    	aux_free_ObjId(&type);
		return(e_KeyInfo((KeyInfo *)opaque));
	}
	else if (!aux_cmp_ObjId(type, RSA_PK_OID)
	    || (!aux_cmp_ObjId(type, Uid_OID) && !aux_cmp_ObjId(opaque_type, RSA_PK_OID))) {
	    	aux_free_ObjId(&type);
		return(e_KeyInfo((KeyInfo *)opaque));
	}
	else if (!aux_cmp_ObjId(type, DSA_SK_OID)
	    || (!aux_cmp_ObjId(type, Uid_OID) && !aux_cmp_ObjId(opaque_type, DSA_SK_OID))) {
	    	aux_free_ObjId(&type);
		return(e_KeyInfo((KeyInfo *)opaque));
	}
	else if (!aux_cmp_ObjId(type, DSA_PK_OID)
	    || (!aux_cmp_ObjId(type, Uid_OID) && !aux_cmp_ObjId(opaque_type, DSA_PK_OID))) {
	    	aux_free_ObjId(&type);
		return(e_KeyInfo((KeyInfo *)opaque));
	}
	else if (!aux_cmp_ObjId(type, EDBKey_OID)
	    || (!aux_cmp_ObjId(type, Uid_OID) && !aux_cmp_ObjId(opaque_type, EDBKey_OID))) {
	    	aux_free_ObjId(&type);
		return(e_KeyInfo((KeyInfo *)opaque));
	}
	else if (!aux_cmp_ObjId(type, DES_OID)
	    || (!aux_cmp_ObjId(type, Uid_OID) && !aux_cmp_ObjId(opaque_type, DES_OID))) {
	    	aux_free_ObjId(&type);
		return(e_KeyInfo((KeyInfo *)opaque));
	}
	else if (!aux_cmp_ObjId(type, DES3_OID)
	    || (!aux_cmp_ObjId(type, Uid_OID) && !aux_cmp_ObjId(opaque_type, DES3_OID))) {
	    	aux_free_ObjId(&type);
		return(e_KeyInfo((KeyInfo *)opaque));
	}
	else if (!aux_cmp_ObjId(type, DHparam_OID)
	    || (!aux_cmp_ObjId(type, Uid_OID) && !aux_cmp_ObjId(opaque_type, DHparam_OID))) { 
	    	aux_free_ObjId(&type);
		return(e_AlgId((AlgId *)opaque));
	}
	else if (!aux_cmp_ObjId(type, DHkey_OID)
	    || (!aux_cmp_ObjId(type, Uid_OID) && !aux_cmp_ObjId(opaque_type, DHkey_OID))) {
	    	aux_free_ObjId(&type);
		return(e_BitString((BitString *)opaque));
	}
	else if (!aux_cmp_ObjId(type, DHprivate_OID)
	    || (!aux_cmp_ObjId(type, Uid_OID) && !aux_cmp_ObjId(opaque_type, DHprivate_OID))) {
	    	aux_free_ObjId(&type);
		return(e_KeyInfo((KeyInfo *)opaque));
	}
	else if (!aux_cmp_ObjId(type, Random_OID)
	    || (!aux_cmp_ObjId(type, Uid_OID) && !aux_cmp_ObjId(opaque_type, Random_OID))) {
	    	aux_free_ObjId(&type);
		return(e_OctetString((OctetString *)opaque));
	}
	else {
		aux_add_error(EOBJ, "Unknown object identifier", CNULL, 0, proc);
	    	aux_free_ObjId(&type);
		return(NULLOCTETSTRING);
	}
}


/***************************************************************
 *
 * Procedure af_pse_decode
 *
 * decode OctetString to object
 * If a standard PSE object name is given, the fitting decoding
 *   routine will be used.
 * Elsewhere, the first decoding routine that returns data
 *   wins.
 * If objname is omitted, the first hit is returned, too. 
 * opaque_type returns the decoded OID.
 * Return (void *)NULL on error 
 *
 ***************************************************************/
#ifdef __STDC__

void *af_pse_decode(
	char	 	*objname,
	OctetString	*ostr,
	ObjId	 	*opaque_type
)

#else

void *af_pse_decode(
		objname,
		ostr,
		opaque_type
)
char	 	*objname;
OctetString     *ostr;
ObjId	 	*opaque_type;

#endif

{
	char			*proc = "af_pse_decode";
	ObjId			*type;
	void 			*opaque;

	
	if (!ostr || !opaque_type) {
		aux_add_error(EINVALID, "Invalid parameter objname, ostr, or opaque_type", CNULL, 0, proc);
		return((void *)NULL);
	}

	if (objname) {
	
		type = af_get_objoid(objname);
		aux_cpy2_ObjId(opaque_type, type);
		aux_free_ObjId(&type);
	
		/* standard object */
		if (!aux_cmp_ObjId(opaque_type, SignCert_OID)) {
			return((void *)d_Certificate(ostr));
		}
		else if (!aux_cmp_ObjId(opaque_type, EncCert_OID)) { 
			return((void *)d_Certificate(ostr));
		}
		else if (!aux_cmp_ObjId(opaque_type, Cert_OID)) { 
			return((void *)d_Certificate(ostr));
		}
		else if (!aux_cmp_ObjId(opaque_type, SignCSet_OID)) { 
			return((void *)d_CertificateSet(ostr));
		}
		else if (!aux_cmp_ObjId(opaque_type, EncCSet_OID)) { 
			return((void *)d_CertificateSet(ostr));
		}
		else if (!aux_cmp_ObjId(opaque_type, CSet_OID)) { 
			return((void *)d_CertificateSet(ostr));
		}
		else if (!aux_cmp_ObjId(opaque_type, CrossCSet_OID)) { 
			return((void *)d_CertificatePairSet(ostr));
		}
		else if (!aux_cmp_ObjId(opaque_type, FCPath_OID)) { 
			return((void *)d_FCPath(ostr));
		}
		else if (!aux_cmp_ObjId(opaque_type, PKRoot_OID)) { 
			return((void *)d_PKRoot(ostr));
		}
		else if (!aux_cmp_ObjId(opaque_type, PKList_OID)) { 
			return((void *)d_PKList(ostr));
		}
		else if (!aux_cmp_ObjId(opaque_type, EKList_OID)) { 
			return((void *)d_PKList(ostr));
		}
		else if (!aux_cmp_ObjId(opaque_type, PCAList_OID)) { 
			return((void *)d_PKList(ostr));
		}
		else if (!aux_cmp_ObjId(opaque_type, CrlSet_OID)) { 
			return((void *)d_CrlSet(ostr));
		}
		else if (!aux_cmp_ObjId(opaque_type, Name_OID)) { 
			return((void *)d_DName(ostr));
		}
		else if (!aux_cmp_ObjId(opaque_type, SerialNumber_OID)) { 
			return((void *)d_OctetString(ostr));
		}
		else if (!aux_cmp_ObjId(opaque_type, AliasList_OID)) { 
			return((void *)d_AliasList(ostr));
		}
		else if (!aux_cmp_ObjId(opaque_type, QuipuPWD_OID)) { 
			return((void *)d_GRAPHICString(ostr));
		}
		else if (!aux_cmp_ObjId(opaque_type, KeyInfo_OID)) {
			return((void *)d_KeyInfo(ostr));
		}
		else if (!aux_cmp_ObjId(opaque_type, SignSK_OID)) {
			return((void *)d_KeyInfo(ostr));
		}
		else if (!aux_cmp_ObjId(opaque_type, DecSKnew_OID)) {
			return((void *)d_KeyInfo(ostr));
		}
		else if (!aux_cmp_ObjId(opaque_type, DecSKold_OID)) {
			return((void *)d_KeyInfo(ostr));
		}
		else if (!aux_cmp_ObjId(opaque_type, SKnew_OID)) {
			return((void *)d_KeyInfo(ostr));
		}
		else if (!aux_cmp_ObjId(opaque_type, SKold_OID)) {
			return((void *)d_KeyInfo(ostr));
		}
		else if (!aux_cmp_ObjId(opaque_type, RSA_SK_OID)) {
			return((void *)d_KeyInfo(ostr));
		}
		else if (!aux_cmp_ObjId(opaque_type, RSA_PK_OID)) {
			return((void *)d_KeyInfo(ostr));
		}
		else if (!aux_cmp_ObjId(opaque_type, DSA_SK_OID)) {
			return((void *)d_KeyInfo(ostr));
		}
		else if (!aux_cmp_ObjId(opaque_type, DSA_PK_OID)) {
			return((void *)d_KeyInfo(ostr));
		}
		else if (!aux_cmp_ObjId(opaque_type, EDBKey_OID)) {
			return((void *)d_KeyInfo(ostr));
		}
		else if (!aux_cmp_ObjId(opaque_type, DES_OID)) {
			return((void *)d_KeyInfo(ostr));
		}
		else if (!aux_cmp_ObjId(opaque_type, DES3_OID)) {
			return((void *)d_KeyInfo(ostr));
		}
		else if (!aux_cmp_ObjId(opaque_type, DHparam_OID)) { 
			return((void *)d_AlgId(ostr));
		}
		else if (!aux_cmp_ObjId(opaque_type, DHkey_OID)) {
			return((void *)d_BitString(ostr));
		}
		else if (!aux_cmp_ObjId(opaque_type, DHprivate_OID)) {
			return((void *)d_KeyInfo(ostr));
		}
		else if (!aux_cmp_ObjId(opaque_type, Random_OID)) {
			return((void *)d_OctetString(ostr));
		}
		
		aux_free2_ObjId(opaque_type);
	}
		
	/* non-standard object */
	if((opaque = d_Certificate(ostr)))			aux_cpy2_ObjId(opaque_type, Cert_OID);
	else if((opaque = d_CertificateSet(ostr)))		aux_cpy2_ObjId(opaque_type, CSet_OID);
	else if((opaque = d_CertificatePairSet(ostr)))		aux_cpy2_ObjId(opaque_type, CrossCSet_OID);
	else if((opaque = d_CrlSet(ostr)))			aux_cpy2_ObjId(opaque_type, CrlSet_OID);
	else if((opaque = d_DName(ostr)))			aux_cpy2_ObjId(opaque_type, Name_OID);
	else if((opaque = d_FCPath(ostr)))			aux_cpy2_ObjId(opaque_type, FCPath_OID);
	else if((opaque = d_PKRoot(ostr)))			aux_cpy2_ObjId(opaque_type, PKRoot_OID);
	else if((opaque = d_PKList(ostr)))			aux_cpy2_ObjId(opaque_type, PKList_OID);
	else if((opaque = d_KeyInfo(ostr)))			aux_cpy2_ObjId(opaque_type, KeyInfo_OID);
	else if((opaque = d_AliasList(ostr)))			aux_cpy2_ObjId(opaque_type, AliasList_OID);
	else if((opaque = d_GRAPHICString(ostr)))		aux_cpy2_ObjId(opaque_type, QuipuPWD_OID);
	else {
		aux_add_error(EDECODE, "Can't decode OctetString", CNULL, 0, proc);
		return((void *)NULL);
	}
	
	return((void *)opaque);
}

/***************************************************************
 *
 * Procedure af_pse_cpy_opaque
 *
 * copy opaque object using af_pse_encode()/af_pse_decode()
 *
 ***************************************************************/
#ifdef __STDC__

void *af_pse_cpy_opaque(
	char		*objname,
	void	 	*opaque,
	ObjId	 	*opaque_type
)

#else

void *af_pse_cpy_opaque(
		objname,
		opaque,
		opaque_type
)
char		*objname;
void	 	*opaque;
ObjId	 	*opaque_type;

#endif

{
	char			*proc = "af_pse_cpy_opaque";


	return(	af_pse_decode(	objname,
				af_pse_encode(	objname, opaque, opaque_type),
				opaque_type					) );
				
}










/***************************************************************
 *
 * Procedure af_pse_free
 *
 * Free object opaque of assumed type objtype
 * (opaque is null pointer after all!)
 *
 ***************************************************************/
#ifdef __STDC__
RC af_pse_free(
	void	 *opaque,
	ObjId	 *opaque_type
)

#else

RC af_pse_free(
	opaque,
	opaque_type
)
void	 *opaque;
ObjId	 *opaque_type;

#endif

{
	char			*proc = "af_pse_free";

	
	if (!opaque || !opaque_type) {
		aux_add_error(EINVALID, "Invalid parameter opaque or opaque_type", CNULL, 0, proc);
		return(-1);
	}

	if (!aux_cmp_ObjId(opaque_type, SignCert_OID)) {
		aux_free_Certificate((Certificate **)&opaque);
	}
	else if (!aux_cmp_ObjId(opaque_type, EncCert_OID)) { 
		aux_free_Certificate((Certificate **)&opaque);
	}
	else if (!aux_cmp_ObjId(opaque_type, Cert_OID)) { 
		aux_free_Certificate((Certificate **)&opaque);
	}
	else if (!aux_cmp_ObjId(opaque_type, SignCSet_OID)) { 
		aux_free_CertificateSet((SET_OF_Certificate **)&opaque);
	}
	else if (!aux_cmp_ObjId(opaque_type, EncCSet_OID)) { 
		aux_free_CertificateSet((SET_OF_Certificate **)&opaque);
	}
	else if (!aux_cmp_ObjId(opaque_type, CSet_OID)) { 
		aux_free_CertificateSet((SET_OF_Certificate **)&opaque);
	}
	else if (!aux_cmp_ObjId(opaque_type, CrossCSet_OID)) { 
		aux_free_CertificatePairSet((SET_OF_CertificatePair **)&opaque);
	}
	else if (!aux_cmp_ObjId(opaque_type, FCPath_OID)) { 
		aux_free_FCPath((FCPath **)&opaque);
	}
	else if (!aux_cmp_ObjId(opaque_type, PKRoot_OID)) { 
		aux_free_PKRoot((PKRoot **)&opaque);
	}
	else if (!aux_cmp_ObjId(opaque_type, PKList_OID)) { 
		aux_free_PKList((PKList **)&opaque);
	}
	else if (!aux_cmp_ObjId(opaque_type, EKList_OID)) { 
		aux_free_PKList((PKList **)&opaque);
	}
	else if (!aux_cmp_ObjId(opaque_type, PCAList_OID)) { 
		aux_free_PKList((PKList **)&opaque);
	}
	else if (!aux_cmp_ObjId(opaque_type, CrlSet_OID)) { 
		aux_free_CrlSet((CrlSet **)&opaque);
	}
	else if (!aux_cmp_ObjId(opaque_type, Name_OID)) { 
		aux_free_DName((DName **)&opaque);
	}
	else if (!aux_cmp_ObjId(opaque_type, SerialNumber_OID)) { 
		aux_free_OctetString((OctetString **)&opaque);
	}
	else if (!aux_cmp_ObjId(opaque_type, AliasList_OID)) { 
		aux_free_AliasList((AliasList **)&opaque);
	}
	else if (!aux_cmp_ObjId(opaque_type, QuipuPWD_OID)) { 
		free(opaque);
	}
	else if (!aux_cmp_ObjId(opaque_type, KeyInfo_OID)) {
		aux_free_KeyInfo((KeyInfo **)&opaque);
	}
	else if (!aux_cmp_ObjId(opaque_type, SignSK_OID)) {
		aux_free_KeyInfo((KeyInfo **)&opaque);
	}
	else if (!aux_cmp_ObjId(opaque_type, DecSKnew_OID)) {
		aux_free_KeyInfo((KeyInfo **)&opaque);
	}
	else if (!aux_cmp_ObjId(opaque_type, DecSKold_OID)) {
		aux_free_KeyInfo((KeyInfo **)&opaque);
	}
	else if (!aux_cmp_ObjId(opaque_type, SKnew_OID)) {
		aux_free_KeyInfo((KeyInfo **)&opaque);
	}
	else if (!aux_cmp_ObjId(opaque_type, SKold_OID)) {
		aux_free_KeyInfo((KeyInfo **)&opaque);
	}
	else if (!aux_cmp_ObjId(opaque_type, RSA_SK_OID)) {
		aux_free_KeyInfo((KeyInfo **)&opaque);
	}
	else if (!aux_cmp_ObjId(opaque_type, RSA_PK_OID)) {
		aux_free_KeyInfo((KeyInfo **)&opaque);
	}
	else if (!aux_cmp_ObjId(opaque_type, DSA_SK_OID)) {
		aux_free_KeyInfo((KeyInfo **)&opaque);
	}
	else if (!aux_cmp_ObjId(opaque_type, DSA_PK_OID)) {
		aux_free_KeyInfo((KeyInfo **)&opaque);
	}
	else if (!aux_cmp_ObjId(opaque_type, EDBKey_OID)) {
		aux_free_KeyInfo((KeyInfo **)&opaque);
	}
	else if (!aux_cmp_ObjId(opaque_type, DES_OID)) {
		aux_free_KeyInfo((KeyInfo **)&opaque);
	}
	else if (!aux_cmp_ObjId(opaque_type, DES3_OID)) {
		aux_free_KeyInfo((KeyInfo **)&opaque);
	}
	else if (!aux_cmp_ObjId(opaque_type, DHparam_OID)) { 
		aux_free_AlgId((AlgId **)&opaque);
	}
	else if (!aux_cmp_ObjId(opaque_type, DHkey_OID)) {
		aux_free_BitString((BitString **)&opaque);
	}
	else if (!aux_cmp_ObjId(opaque_type, DHprivate_OID)) {
		aux_free_KeyInfo((KeyInfo **)&opaque);
	}
	else if (!aux_cmp_ObjId(opaque_type, Random_OID)) {
		aux_free_OctetString((OctetString **)&opaque);
	}
	else if (!aux_cmp_ObjId(opaque_type, Uid_OID)) {
		aux_free_OctetString((OctetString **)&opaque);
	}
	else {
		aux_add_error(EOBJ, "Unknown object identifier", CNULL, 0, proc);
		return(-1);
	}

        return(0);
}


/***************************************************************
 *
 * Procedure af_pse_print
 *
 * Print object with given name to file, calls af_pse_fprint()
 *
 ***************************************************************/
#ifdef __STDC__

RC af_pse_print(
	char	 *objname
)

#else

RC af_pse_print(
	objname
)
char	 *objname;

#endif

{
	char			*proc = "af_pse_print";


	return(af_pse_fprint(stderr, objname));
}




/***************************************************************
 *
 * Procedure af_pse_fprint
 *
 * Print object with given name to file, calls af_pse_sprint()
 *
 ***************************************************************/
#ifdef __STDC__

RC af_pse_fprint(
	FILE	 *ff,
	char	 *objname
)

#else

RC af_pse_fprint(
	ff,
	objname
)
FILE	 *ff;
char	 *objname;

#endif

{
	char			*proc = "af_pse_fprint";
	char 			*string = af_pse_sprint(CNULL, objname);

	if (string) {
		fprintf(ff, string);
		free(string);
		return(0);
	}
	else return(-1);
}


/***************************************************************
 *
 * Procedure af_pse_sprint
 *
 * Print object with given name to file, calls af_pse_get()
 *
 ***************************************************************/
#ifdef __STDC__

char *af_pse_sprint(
	char	 *string,
	char	 *objname
)

#else

char *af_pse_sprint(
	string,
	objname
)
char	 *string;
char	 *objname;

#endif

{
	char			*proc = "af_pse_sprint";
	ObjId	 		objtype;
	void			*opaque;

	
	if (!objname || !strcmp(objname, "")) {
		aux_add_error(EINVALID, "Invalid parameter objname", CNULL, 0, proc);
		return(CNULL);
	}

	if (! (opaque = af_pse_get(objname, &objtype))) {
		AUX_ADD_ERROR;
		return(CNULL);
	}
	
	if (!aux_cmp_ObjId(&objtype, SignCert_OID)) {
		string = CATSPRINTF(CNULL, "*** Object <%s> is of the type <SignCert_OID> *** \n\n", objname);
		string = aux_sprint_Certificate(string, (Certificate *)opaque);
	}
	else if (!aux_cmp_ObjId(&objtype, EncCert_OID)) { 
		string = CATSPRINTF(CNULL, "*** Object <%s> is of the type <EncCert_OID> *** \n\n", objname);
		string = aux_sprint_Certificate(string, (Certificate *)opaque);
	}
	else if (!aux_cmp_ObjId(&objtype, Cert_OID)) { 
		string = CATSPRINTF(CNULL, "*** Object <%s> is of the type <Cert_OID> *** \n\n", objname);
		string = aux_sprint_Certificate(string, (Certificate *)opaque);
	}
	else if (!aux_cmp_ObjId(&objtype, SignCSet_OID)) { 
		string = CATSPRINTF(CNULL, "*** Object <%s> is of the type <SignCSet_OID> *** \n\n", objname);
		string = aux_sprint_CertificateSet(string, (SET_OF_Certificate *)opaque);
	}
	else if (!aux_cmp_ObjId(&objtype, EncCSet_OID)) { 
		string = CATSPRINTF(CNULL, "*** Object <%s> is of the type <EncCSet_OID> *** \n\n", objname);
		string = aux_sprint_CertificateSet(string, (SET_OF_Certificate *)opaque);
	}
	else if (!aux_cmp_ObjId(&objtype, CSet_OID)) { 
		string = CATSPRINTF(CNULL, "*** Object <%s> is of the type <CSet_OID> *** \n\n", objname);
		string = aux_sprint_CertificateSet(string, (SET_OF_Certificate *)opaque);
	}
	else if (!aux_cmp_ObjId(&objtype, CrossCSet_OID)) { 
		string = CATSPRINTF(CNULL, "*** Object <%s> is of the type <CrossCSet_OID> *** \n\n", objname);
		string = aux_sprint_CertificatePairSet(string, (SET_OF_CertificatePair *)opaque);
	}
	else if (!aux_cmp_ObjId(&objtype, FCPath_OID)) { 
		string = CATSPRINTF(CNULL, "*** Object <%s> is of the type <FCPath_OID> *** \n\n", objname);
		string = aux_sprint_FCPath(string, (FCPath *)opaque);
	}
	else if (!aux_cmp_ObjId(&objtype, PKRoot_OID)) { 
		string = CATSPRINTF(CNULL, "*** Object <%s> is of the type <PKRoot_OID> *** \n\n", objname);
		string = aux_sprint_PKRoot(string, (PKRoot *)opaque);
	}
	else if (!aux_cmp_ObjId(&objtype, PKList_OID)) { 
		string = CATSPRINTF(CNULL, "*** Object <%s> is of the type <PKList_OID> *** \n\n", objname);
		string = aux_sprint_PKList(string, (PKList *)opaque);
	}
	else if (!aux_cmp_ObjId(&objtype, EKList_OID)) { 
		string = CATSPRINTF(CNULL, "*** Object <%s> is of the type <EKList_OID> *** \n\n", objname);
		string = aux_sprint_PKList(string, (PKList *)opaque);
	}
	else if (!aux_cmp_ObjId(&objtype, PCAList_OID)) { 
		string = CATSPRINTF(CNULL, "*** Object <%s> is of the type <PCAList_OID> *** \n\n", objname);
		string = aux_sprint_PKList(string, (PKList *)opaque);
	}
	else if (!aux_cmp_ObjId(&objtype, CrlSet_OID)) { 
		string = CATSPRINTF(CNULL, "*** Object <%s> is of the type <CrlSet_OID> *** \n\n", objname);
		string = aux_sprint_CrlSet(string, (CrlSet *)opaque);
	}
	else if (!aux_cmp_ObjId(&objtype, Name_OID)) { 
		string = CATSPRINTF(CNULL, "*** Object <%s> is of the type <Name_OID> *** \n\n", objname);
		string = aux_sprint_DNameOrAlias(string, (DName *)opaque);
	}
	else if (!aux_cmp_ObjId(&objtype, SerialNumber_OID)) { 
		string = CATSPRINTF(CNULL, "*** Object <%s> is of the type <SerialNumber_OID> *** \n\n", objname);
		string = aux_sprint_Serialnumber(string, (SerialNumber *)opaque);
	}
	else if (!aux_cmp_ObjId(&objtype, AliasList_OID)) { 
		string = CATSPRINTF(CNULL, "*** Object <%s> is of the type <AliasList_OID> *** \n\n", objname);
		string = aux_sprint_AliasList(string, (AliasList *)opaque);
	}
	else if (!aux_cmp_ObjId(&objtype, QuipuPWD_OID)) { 
		string = CATSPRINTF(CNULL, "*** Object <%s> is of the type <QuipuPWD_OID> *** \n\n", objname);
		string = aux_sprint_GRAPHICString(string, (QuipuPWD)opaque);
	}
	else if (!aux_cmp_ObjId(&objtype, KeyInfo_OID)) {
		string = CATSPRINTF(CNULL, "*** Object <%s> is of the type <KeyInfo_OID> *** \n\n", objname);
		string = aux_sprint_KeyInfo(string, (KeyInfo *)opaque);
	}
	else if (!aux_cmp_ObjId(&objtype, SignSK_OID)) {
		string = CATSPRINTF(CNULL, "*** Object <%s> is of the type <SignSK_OID> *** \n\n", objname);
		string = aux_sprint_KeyInfo(string, (KeyInfo *)opaque);
	}
	else if (!aux_cmp_ObjId(&objtype, DecSKnew_OID)) {
		string = CATSPRINTF(CNULL, "*** Object <%s> is of the type <DecSKnew_OID> *** \n\n", objname);
		string = aux_sprint_KeyInfo(string, (KeyInfo *)opaque);
	}
	else if (!aux_cmp_ObjId(&objtype, DecSKold_OID)) {
		string = CATSPRINTF(CNULL, "*** Object <%s> is of the type <DecSKold_OID> *** \n\n", objname);
		string = aux_sprint_KeyInfo(string, (KeyInfo *)opaque);
	}
	else if (!aux_cmp_ObjId(&objtype, SKnew_OID)) {
		string = CATSPRINTF(CNULL, "*** Object <%s> is of the type <SKnew_OID> *** \n\n", objname);
		string = aux_sprint_KeyInfo(string, (KeyInfo *)opaque);
	}
	else if (!aux_cmp_ObjId(&objtype, SKold_OID)) {
		string = CATSPRINTF(CNULL, "*** Object <%s> is of the type <SKold_OID> *** \n\n", objname);
		string = aux_sprint_KeyInfo(string, (KeyInfo *)opaque);
	}
	else if (!aux_cmp_ObjId(&objtype, RSA_SK_OID)) {
		string = CATSPRINTF(CNULL, "*** Object <%s> is of the type <RSA_SK_OID> *** \n\n", objname);
		string = aux_sprint_KeyInfo(string, (KeyInfo *)opaque);
	}
	else if (!aux_cmp_ObjId(&objtype, RSA_PK_OID)) {
		string = CATSPRINTF(CNULL, "*** Object <%s> is of the type <RSA_PK_OID> *** \n\n", objname);
		string = aux_sprint_KeyInfo(string, (KeyInfo *)opaque);
	}
	else if (!aux_cmp_ObjId(&objtype, DSA_SK_OID)) {
		string = CATSPRINTF(CNULL, "*** Object <%s> is of the type <DSA_SK_OID> *** \n\n", objname);
		string = aux_sprint_KeyInfo(string, (KeyInfo *)opaque);
	}
	else if (!aux_cmp_ObjId(&objtype, DSA_PK_OID)) {
		string = CATSPRINTF(CNULL, "*** Object <%s> is of the type <DSA_PK_OID> *** \n\n", objname);
		string = aux_sprint_KeyInfo(string, (KeyInfo *)opaque);
	}
	else if (!aux_cmp_ObjId(&objtype, EDBKey_OID)) {
		string = CATSPRINTF(CNULL, "*** Object <%s> is of the type <EDBKey_OID> *** \n\n", objname);
		string = aux_sprint_KeyInfo(string, (KeyInfo *)opaque);
	}
	else if (!aux_cmp_ObjId(&objtype, DES_OID)) {
		string = CATSPRINTF(CNULL, "*** Object <%s> is of the type <DES_OID> *** \n\n", objname);
		string = aux_sprint_KeyInfo(string, (KeyInfo *)opaque);
	}
	else if (!aux_cmp_ObjId(&objtype, DES3_OID)) {
		string = CATSPRINTF(CNULL, "*** Object <%s> is of the type <DES3_OID> *** \n\n", objname);
		string = aux_sprint_KeyInfo(string, (KeyInfo *)opaque);
	}
	else if (!aux_cmp_ObjId(&objtype, DHparam_OID)) { 
		string = CATSPRINTF(CNULL, "*** Object <%s> is of the type <DHparam_OID> *** \n\n", objname);
		string = aux_sprint_AlgId(string, (AlgId *)opaque);
	}
	else if (!aux_cmp_ObjId(&objtype, DHkey_OID)) {
		string = CATSPRINTF(CNULL, "*** Object <%s> is of the type <DHkey_OID> *** \n\n", objname);
		string = aux_sprint_BitString(string, (BitString *)opaque);
	}
	else if (!aux_cmp_ObjId(&objtype, DHprivate_OID)) {
		string = CATSPRINTF(CNULL, "*** Object <%s> is of the type <DHprivate_OID> *** \n\n", objname);
		string = aux_sprint_KeyInfo(string, (KeyInfo *)opaque);
	}
	else if (!aux_cmp_ObjId(&objtype, Random_OID)) {
		string = CATSPRINTF(CNULL, "*** Object <%s> is of the type <Random_OID> *** \n\n", objname);
		string = aux_sprint_OctetString(string, (OctetString *)opaque);
	}
	else if (!aux_cmp_ObjId(&objtype, Uid_OID)) {
		string = CATSPRINTF(CNULL, "*** Object <%s> is of an unknown type <Uid_OID> *** \n\n", objname);
		string = aux_sprint_OctetString(string, (OctetString *)opaque);
	}
	else {
		string = CNULL;
		aux_add_error(EOBJ, "Unknown object identifier", CNULL, 0, proc);
	}

	if (string) af_pse_free(opaque, &objtype);
	aux_free2_ObjId(&objtype);

        return(string);
}

/****************************************************************************/



/***************************************************************
 *
 * Procedure af_pse_reset
 * Free cache of given object,
 *   or of all objects if no name is specified
 *
 ***************************************************************/

#ifdef __STDC__

RC af_pse_reset(
	char	 *objname
)

#else

RC af_pse_reset(
	objname
)
char	 *objname;

#endif

{
	char			*proc = "af_pse_reset";
	int			rcode = -1;

	if(!objname || !strcmp(objname, SignCert_name)) {
		if (SignCert_cache) aux_free_Certificate(&SignCert_cache);
	        SignCert_read = FALSE;
		rcode = 0;
	}        
	if(!objname || !strcmp(objname, EncCert_name)) {
		if (EncCert_cache) aux_free_Certificate(&EncCert_cache); 
	        EncCert_read = FALSE;        
		rcode = 0;
	}        
	if(!objname || !strcmp(objname, Cert_name)) {
		if (Cert_cache) aux_free_Certificate(&Cert_cache); 
	        Cert_read = FALSE;        
		rcode = 0;
	}        
	if(!objname || !strcmp(objname, SignCSet_name)) {
		if (SignCSet_cache) aux_free_CertificateSet(&SignCSet_cache); 
	        SignCSet_read = FALSE;        
		rcode = 0;
	}        
	if(!objname || !strcmp(objname, EncCSet_name)) {
		if (EncCSet_cache) aux_free_CertificateSet(&EncCSet_cache);
	        EncCSet_read = FALSE;        
		rcode = 0;
	}        
	if(!objname || !strcmp(objname, CSet_name)) {
		if (CSet_cache) aux_free_CertificateSet(&CSet_cache);
	        CSet_read = FALSE;        
		rcode = 0;
	}        
	if(!objname || !strcmp(objname, SignSK_name)) {
		if (SignSK_cache) aux_free_KeyInfo(&SignSK_cache); 
	        SignSK_read = FALSE;        
		rcode = 0;
	}        
	if(!objname || !strcmp(objname, DecSKnew_name)) {
		if (DecSKnew_cache) aux_free_KeyInfo(&DecSKnew_cache); 
	        DecSKnew_read = FALSE;        
		rcode = 0;
	}        
	if(!objname || !strcmp(objname, DecSKold_name)) {
		if (DecSKold_cache) aux_free_KeyInfo(&DecSKold_cache); 
                DecSKold_read = FALSE;        
		rcode = 0;
	}        
	if(!objname || !strcmp(objname, SKnew_name)) {
		if (SKnew_cache) aux_free_KeyInfo(&SKnew_cache); 
                SKnew_read = FALSE;        
		rcode = 0;
	}        
	if(!objname || !strcmp(objname, SKold_name)) {
		if (SKold_cache) aux_free_KeyInfo(&SKold_cache); 
                SKold_read = FALSE;        
		rcode = 0;
	}        
	if(!objname || !strcmp(objname, FCPath_name)) {
		if (FCPath_cache) aux_free_FCPath(&FCPath_cache); 
                FCPath_read = FALSE;        
		rcode = 0;
	}        
	if(!objname || !strcmp(objname, PKRoot_name)) {
		if (PKRoot_cache) aux_free_PKRoot(&PKRoot_cache); 
                PKRoot_read = FALSE;        
		rcode = 0;
	}        
	if(!objname || !strcmp(objname, PKList_name)) {
		if (PKList_cache) aux_free_PKList(&PKList_cache); 
                PKList_read = FALSE;        
		rcode = 0;
	}        
	if(!objname || !strcmp(objname, EKList_name)) {
		if (EKList_cache) aux_free_PKList(&EKList_cache); 
                EKList_read = FALSE;        
		rcode = 0;
	}        
	if(!objname || !strcmp(objname, PCAList_name)) {
		if (PCAList_cache) aux_free_PKList(&PCAList_cache); 
                PCAList_read = FALSE;        
		rcode = 0;
	}        
	if(!objname || !strcmp(objname, CrossCSet_name)) {
		if (CrossCSet_cache) aux_free_CertificatePairSet(&CrossCSet_cache); 
                CrossCSet_read = FALSE;        
		rcode = 0;
	}        
	if(!objname || !strcmp(objname, CrlSet_name)) {
		if (CrlSet_cache) aux_free_CrlSet(&CrlSet_cache); 
                CrlSet_read = FALSE;        
		rcode = 0;
	}        
	if(!objname || !strcmp(objname, SerialNumber_name)) {
		if (SerialNumber_cache) aux_free_OctetString((SerialNumber **)&SerialNumber_cache); 
                SerialNumber_read = FALSE;        
		rcode = 0;
	}        
	if(!objname || !strcmp(objname, AliasList_name)) {
		if (AliasList_cache) aux_free_AliasList(&AliasList_cache); 
                AliasList_read = FALSE;        
		rcode = 0;
	}        
	if(!objname || !strcmp(objname, QuipuPWD_name)) {
		if (QuipuPWD_cache) {
			free(QuipuPWD_cache);
			QuipuPWD_cache = CNULL;
		}
                QuipuPWD_read = FALSE;        
		rcode = 0;
	}        
	if(!objname || !strcmp(objname, DHparam_name)) {
		if (DHparam_cache) aux_free_AlgId(&DHparam_cache); 
                DHparam_read = FALSE; 
		rcode = 0;
	}        
	
	return(rcode);
}       


/***************************************************************
 *
 * Procedure clrpin
 *
 ***************************************************************/
#ifdef __STDC__

static void clrpin(
	char	 *pin
)

#else

static void clrpin(
	pin
)
char	 *pin;

#endif

{
	char	* proc = "clrpin";

	if(pin) while(*pin) *pin++ = '\0';
	return;
}


/***************************************************************
 *
 * Procedure deletepin
 *
 ***************************************************************/
#ifdef __STDC__

static void deletepin(
	char	 **pin
)

#else

static void deletepin(
	pin
)
char	 **pin;

#endif

{
	char	* proc = "deletepin";

	clrpin(*pin);
	if(*pin) {
		free(*pin);
		*pin = CNULL;
	}

	return;
}



/****************************************************************************/

/***************************************************************
 *
 * Procedure af_pse_get_Name
 *
 ***************************************************************/
#ifdef __STDC__

DName *af_pse_get_Name(
)

#else

DName *af_pse_get_Name(
)

#endif

{
	Certificate * cert;
	DName	    * ret;
	char	    * proc = "af_pse_get_Name";

	cert = af_pse_get_Certificate(SIGNATURE, NULLDNAME, 0);
	if(!cert){
		if(aux_last_error() != EOBJNAME) AUX_ADD_ERROR;
		else 	aux_add_error(EREADPSE, "Can't read certificate to get Name", CNULL, 0, proc);
		return (NULLDNAME);
	}

	ret = aux_cpy_DName(cert->tbs->subject);
	aux_free_Certificate(&cert);

	return(ret);
}


/****************************************************************************/


/***************************************************************
 *
 * Procedure af_pse_get_AliasList
 *
 ***************************************************************/
#ifdef __STDC__

AliasList *af_pse_get_AliasList(
)

#else

AliasList *af_pse_get_AliasList(
)

#endif

{
	PSESel      * pse_sel;
	OctetString   content;
	AliasList   * alist;
	ObjId         objid;
	char	    * proc = "af_pse_get_AliasList";

	if(AliasList_read) {
		if(!AliasList_cache) aux_add_error(EOBJNAME, "Object AliasList doesn't exist", CNULL, 0, proc);
		return(aux_cpy_AliasList(AliasList_cache));
	}
	AliasList_read = TRUE;

	if (!(pse_sel = af_pse_open(AliasList_OID, FALSE))) {
		if(aux_last_error() != EOBJNAME) AUX_ADD_ERROR;
		else aux_add_error(LASTERROR, "Can't open object AliasList", CNULL, 0, proc);
		return ((AliasList * ) 0);
	}

	if (sec_read_PSE(pse_sel, &objid, &content) < 0 ) {
		aux_add_error(EREADPSE, "Can't read object AliasList", (char *) pse_sel, PSESel_n, proc);
        	aux_free_PSESel(&pse_sel);
		return ((AliasList * ) 0);
	}
       	aux_free_PSESel(&pse_sel);
        if(aux_cmp_ObjId(&objid, AliasList_OID)) {
		aux_add_error(EDAMAGE, "Can't get object AliasList: has the wrong objid", CNULL, 0, proc);
		if(objid.oid_elements) free(objid.oid_elements);
		return ((AliasList * ) 0);
	}
	if(objid.oid_elements) free(objid.oid_elements);

	alist = d_AliasList(&content);
	free(content.octets);

	if (! alist) {
		aux_add_error(EDECODE, "Can't decode object AliasList", CNULL, 0, proc);
		return ((AliasList * ) 0);
	}
	AliasList_cache = alist;

	return(aux_cpy_AliasList(AliasList_cache));
}


/****************************************************************************/


/***************************************************************
 *
 * Procedure af_pse_get_QuipuPWD
 *
 ***************************************************************/
#ifdef __STDC__

QuipuPWD af_pse_get_QuipuPWD(
)

#else

QuipuPWD af_pse_get_QuipuPWD(
)

#endif

{
PSESel      * pse_sel;
OctetString   content;
QuipuPWD      pwd;
ObjId         objid;
char	    * proc = "af_pse_get_QuipuPWD";

	if(QuipuPWD_read) {
		if(!QuipuPWD_cache) aux_add_error(EOBJNAME, "Object QuipuPWD doesn't exist", CNULL, 0, proc);
		return(QuipuPWD_cache);
	}
	QuipuPWD_read = TRUE;

	if (!(pse_sel = af_pse_open(QuipuPWD_OID, FALSE))) {
		if(aux_last_error() != EOBJNAME) AUX_ADD_ERROR;
		else aux_add_error(LASTERROR, "Can't open object QuipuPWD", CNULL, 0, proc);
		return (CNULL);
	}

	if (sec_read_PSE(pse_sel, &objid, &content) < 0 ) {
		aux_add_error(EREADPSE, "Can't read object QuipuPWD", (char *) pse_sel, PSESel_n, proc);
        	aux_free_PSESel(&pse_sel);
		return (CNULL);
	}
       	aux_free_PSESel(&pse_sel);
        if(aux_cmp_ObjId(&objid, QuipuPWD_OID)) {
		aux_add_error(EDAMAGE, "Can't get object QuipuPWD: has the wrong objid", CNULL, 0, proc);
		if(objid.oid_elements) free(objid.oid_elements);
		return (CNULL);
	}

	pwd = (QuipuPWD)d_GRAPHICString(&content);
	if(objid.oid_elements) free(objid.oid_elements);
	free(content.octets);

	if (! pwd) {
		aux_add_error(EDECODE, "Can't decode object QuipuPWD", CNULL, 0, proc);
		return (CNULL);
	}
	QuipuPWD_cache = pwd;

	return((QuipuPWD)aux_cpy_String(QuipuPWD_cache));
}


/***************************************************************
 *
 * Procedure af_pse_get_DHparam
 *
 ***************************************************************/
#ifdef __STDC__

AlgId *af_pse_get_DHparam(
)

#else

AlgId *af_pse_get_DHparam(
)

#endif

{
PSESel      * pse_sel;
OctetString   content;
AlgId       * dhparam;
ObjId         objid;
char	    * proc = "af_pse_get_DHparam";

	if(DHparam_read) {
		if(!DHparam_cache) aux_add_error(EOBJNAME, "Object DHparam doesn't exist", CNULL, 0, proc);
		return(aux_cpy_AlgId(DHparam_cache));
	}
	DHparam_read = TRUE;

	if (!(pse_sel = af_pse_open(DHparam_OID, FALSE))) {
		if(aux_last_error() != EOBJNAME) AUX_ADD_ERROR;
		else aux_add_error(LASTERROR, "Can't open PSE object DHparam", CNULL, 0, proc);
		return ((AlgId *)0);
	}

	if (sec_read_PSE(pse_sel, &objid, &content) < 0 ) {
		aux_add_error(EREADPSE, "Can't read PSE object DHparam", (char *) pse_sel, PSESel_n, proc);
        	aux_free_PSESel(&pse_sel);
		return ((AlgId *)0);
	}
       	aux_free_PSESel(&pse_sel);
        if(aux_cmp_ObjId(&objid, DHparam_OID)) {
		aux_add_error(EDAMAGE, "Can't get PSE object DHparam: has the wrong objid", CNULL, 0, proc);
		return ((AlgId *)0);
	}

	dhparam = d_AlgId(&content);
	free(content.octets);

	if (! dhparam) {
		aux_add_error(EDECODE, "Can't decode object DHparam", CNULL, 0, proc);
		return ((AlgId *)0);
	}

	DHparam_cache = dhparam;
	return(aux_cpy_AlgId(dhparam));
}

/****************************************************************************/


/***************************************************************
 *
 * Procedure af_pse_get_SerialNumber
 *
 ***************************************************************/
#ifdef __STDC__

SerialNumber *af_pse_get_SerialNumber(
)

#else

SerialNumber *af_pse_get_SerialNumber(
)

#endif

{
	PSESel      	     * pse_sel;
	OctetString   	       content;
	SerialNumber         * serial;
	ObjId         	       objid;
	char	             * proc = "af_pse_get_SerialNumber";


	if(SerialNumber_read) {
		if(!SerialNumber_cache) aux_add_error(EOBJNAME, "Object SerialNumber doesn't exist", CNULL, 0, proc);
		return(aux_cpy_OctetString(SerialNumber_cache));
	}
	SerialNumber_read = TRUE;

	if (!(pse_sel = af_pse_open(SerialNumber_OID, FALSE))) {
		if(aux_last_error() != EOBJNAME) AUX_ADD_ERROR;
		else aux_add_error(LASTERROR, "Can't open object SerialNumber", CNULL, 0, proc);
		return (NULLSERIALNUMBER);
	}

	if (sec_read_PSE(pse_sel, &objid, &content) < 0 ) {
		aux_add_error(EREADPSE, "Can't read object SerialNumber", (char *) pse_sel, PSESel_n, proc);
        	aux_free_PSESel(&pse_sel);
		return (NULLSERIALNUMBER);
	}
       	aux_free_PSESel(&pse_sel);
        if(aux_cmp_ObjId(&objid, SerialNumber_OID)) {
		aux_add_error(EDAMAGE, "Can't get object SerialNumber: has the wrong objid", CNULL, 0, proc);
		return (NULLSERIALNUMBER);
	}

	serial = d_SerialNumber(&content);
	free(content.octets);

	if (! serial) {
		aux_add_error(EDECODE, "Can't decode object SerialNumber", CNULL, 0, proc);
		return (NULLSERIALNUMBER);
	}

	SerialNumber_cache = serial;
	return((SerialNumber *)aux_cpy_OctetString(SerialNumber_cache));

}    /*af_pse_get_SerialNumber()*/


/****************************************************************************/


/***************************************************************
 *
 * Procedure af_pse_get_Certificate
 *
 ***************************************************************/
#ifdef __STDC__

Certificate *af_pse_get_Certificate(
	KeyType	     type,
	DName	    *issuer,
	OctetString *serial
)

#else

Certificate *af_pse_get_Certificate(
	type,
	issuer,
	serial
)
KeyType	     type;
DName	    *issuer;
OctetString *serial;

#endif

{
	PSESel             * pse_sel;
	Certificate        * cert;
	OctetString          content;
        ObjId                objid;
	ObjId              * obj_type = NULLOBJID;
	SET_OF_Certificate * setofcert, *savesoc = (SET_OF_Certificate *) 0;
	Boolean		     onekeypaironly = FALSE;

	char	           * proc = "af_pse_get_Certificate";

	if ((type != SIGNATURE) && (type != ENCRYPTION)) {
		aux_add_error(EINVALID, "Invalid parameter", CNULL, 0, proc);
		return ( (Certificate * )0);
	}

	if(af_check_if_onekeypaironly(&onekeypaironly)){
		AUX_ADD_ERROR;
		return ( (Certificate * )0);
	}

	if (issuer) { /* extract one certificate from CSet/SignCSet/EncCSet */
		setofcert = af_pse_get_CertificateSet(type);
		if(!setofcert) return((Certificate *)0);
		while (setofcert) {
			savesoc = setofcert;
			if (!aux_cmp_OctetString(setofcert->element->tbs->serialnumber, serial)
			     && !aux_cmp_DName(setofcert->element->tbs->issuer, issuer)) {
				cert = setofcert->element;
				return(aux_cpy_Certificate(cert));
			}

			setofcert = setofcert->next;
		}
		return((Certificate *)0);
	} 
	else { /* read certificate from Cert/SignCert/EncCert */
		if(onekeypaironly == TRUE) {
			if(Cert_read) {
				if(!Cert_cache) aux_add_error(EOBJNAME, "Object Cert doesn't exist", CNULL, 0, proc);
				return(aux_cpy_Certificate(Cert_cache));
			}
			Cert_read = TRUE;
			obj_type = Cert_OID;
		}
		else {
			if (type == SIGNATURE) {
				if(SignCert_read) {
					if(!SignCert_cache) aux_add_error(EOBJNAME, "Object SignCert doesn't exist", CNULL, 0, proc);
					return(aux_cpy_Certificate(SignCert_cache));
				}

				SignCert_read = TRUE;
				obj_type = SignCert_OID;
			}
			else {
				if(EncCert_cache) {
					if(!EncCert_cache) aux_add_error(EOBJNAME, "Object EncCert doesn't exist", CNULL, 0, proc);
					return(aux_cpy_Certificate(EncCert_cache));
				}

				EncCert_read = TRUE;
				obj_type = EncCert_OID;
			}
		}

		if (!(pse_sel = af_pse_open(obj_type, FALSE))) {
			if(aux_last_error() != EOBJNAME) AUX_ADD_ERROR;
			else aux_add_error(LASTERROR, "Can't get certificate from PSE", CNULL, 0, proc);
			return ( (Certificate * )0);
		}
	
		if (sec_read_PSE(pse_sel, &objid, &content) < 0) {
			aux_add_error(EREADPSE, "Can't get certificate because cannot read object", (char *) pse_sel, PSESel_n, proc);
			aux_free_PSESel(&pse_sel);
			return ( (Certificate * )0);
		}
		aux_free_PSESel(&pse_sel);
		if(aux_cmp_ObjId(&objid, obj_type)) {
			aux_add_error(EDAMAGE, "Can't get certificate because object has the wrong objid", (char *)pse_sel, PSESel_n, proc);
			free(content.octets);
			return ( (Certificate * )0);
		}
		if (!(cert = d_Certificate(&content))) {
			free(content.octets);
			aux_add_error(EDECODE, "Can't decode certificate", CNULL, 0, proc);
			return ( (Certificate * )0);
		}
		free(content.octets);
		if(onekeypaironly == TRUE) Cert_cache = cert;
		else {
			if (type == SIGNATURE) SignCert_cache = cert;
			else EncCert_cache = cert;
		}
		return(aux_cpy_Certificate(cert));
	}


}   /* af_pse_get_Certificate() */


/****************************************************************************/

/***************************************************************
 *
 * Procedure af_pse_get_CertificateSet
 *
 ***************************************************************/
#ifdef __STDC__

SET_OF_Certificate *af_pse_get_CertificateSet(
	KeyType	  type
)

#else

SET_OF_Certificate *af_pse_get_CertificateSet(
	type
)
KeyType	  type;

#endif

{
	PSESel             * pse_sel;
	OctetString          content;
        ObjId                objid;
	ObjId              * obj_type = NULLOBJID;
	SET_OF_Certificate * cset;
	Boolean 	     onekeypaironly = FALSE;

	char	           * proc = "af_pse_get_CertificateSet";

	if((type != SIGNATURE) && (type != ENCRYPTION)){
		aux_add_error(EINVALID, "Invalid parameter", CNULL, 0, proc);
		return ( (SET_OF_Certificate * )0);
	}

	if(af_check_if_onekeypaironly(&onekeypaironly)){
		AUX_ADD_ERROR;
		return ( (SET_OF_Certificate * )0);
	}

	if(onekeypaironly == TRUE) {
		if(CSet_read) {
			if(!CSet_cache) aux_add_error(EOBJNAME, "Object CSet doesn't exist", CNULL, 0, proc);
			return(aux_cpy_SET_OF_Certificate(CSet_cache));
		}
		CSet_read = TRUE;
		obj_type = CSet_OID;
	}
	else {
		switch (type) {
		case SIGNATURE:
			if(SignCSet_read) {
				if(!SignCSet_cache) aux_add_error(EOBJNAME, "Object SignCSet doesn't exist", CNULL, 0, proc);
				return(aux_cpy_SET_OF_Certificate(SignCSet_cache));
			}
			SignCSet_read = TRUE;
			obj_type = SignCSet_OID;
			break;
		case ENCRYPTION:
			if(EncCSet_read) {
				if(!EncCSet_cache) aux_add_error(EOBJNAME, "Object EncCSet doesn't exist", CNULL, 0, proc);
				return(aux_cpy_SET_OF_Certificate(EncCSet_cache));
			}
			EncCSet_read = TRUE;
			obj_type = EncCSet_OID;
			break;
		default:
			aux_add_error(EINVALID, "Invalid keytype", CNULL, 0, proc);
			return ( (SET_OF_Certificate * ) 0);
		}  /* switch */
	}

	if (!(pse_sel = af_pse_open(obj_type, FALSE))) {
		if(aux_last_error() != EOBJNAME) AUX_ADD_ERROR;
		else aux_add_error(LASTERROR, "Can't get certificate set because cannot open object", CNULL, 0, proc);
		return ( (SET_OF_Certificate * ) 0);
	}

	if (sec_read_PSE(pse_sel, &objid, &content) < 0 ) {
		aux_add_error(EREADPSE, "Can't get certificate set because cannot read object", (char *) pse_sel, PSESel_n, proc);
		aux_free_PSESel(&pse_sel);
		return ( (SET_OF_Certificate * ) 0);
	}
	aux_free_PSESel(&pse_sel);
        if(aux_cmp_ObjId(&objid, obj_type)) {
		aux_add_error(EDAMAGE, "Can't get certificate set because reed object has the wrong objid", CNULL, 0, proc);
		return ( (SET_OF_Certificate * ) 0);
	}

	if (!(cset = d_CertificateSet(&content))) {
		free(content.octets);
		aux_add_error(EDECODE, "d_CertificateSet failed", CNULL, 0, proc);
		return ( (SET_OF_Certificate * ) 0);
	}

	free(content.octets);

	if(onekeypaironly == TRUE) CSet_cache = cset;
	else {
		if (type == SIGNATURE) SignCSet_cache = cset;
		else EncCSet_cache = cset;
	}
	return(aux_cpy_SET_OF_Certificate(cset));
}


/****************************************************************************/

/***************************************************************
 *
 * Procedure af_pse_get_CertificatePairSet
 *
 ***************************************************************/
#ifdef __STDC__

SET_OF_CertificatePair *af_pse_get_CertificatePairSet(
)

#else

SET_OF_CertificatePair *af_pse_get_CertificatePairSet(
)

#endif

{
	PSESel                 * pse_sel;
	OctetString              content;
        ObjId                    objid;
	SET_OF_CertificatePair * cpairset;
	char		       * proc = "af_pse_get_CertificatePairSet";

	if(CrossCSet_read) {
		if(!CrossCSet_cache) aux_add_error(EOBJNAME, "Object CrossCSet doesn't exist", CNULL, 0, proc);
		return(aux_cpy_SET_OF_CertificatePair(CrossCSet_cache));
	}
	CrossCSet_read = TRUE;

	if (!(pse_sel = af_pse_open(CrossCSet_OID, FALSE))) {
		if(aux_last_error() != EOBJNAME) AUX_ADD_ERROR;
		else aux_add_error(LASTERROR, "Can't open object CrossCSet", CNULL, 0, proc);
		return ( (SET_OF_CertificatePair * ) 0);
	}

	if (sec_read_PSE(pse_sel, &objid, &content) < 0 ) {
		aux_add_error(EREADPSE, "Can't read object CrossCSet", (char *) pse_sel, PSESel_n, proc);
        	aux_free_PSESel(&pse_sel);
		return ( (SET_OF_CertificatePair * ) 0);
	}
	aux_free_PSESel(&pse_sel);
        if(aux_cmp_ObjId(&objid, CrossCSet_OID)) {
		aux_add_error(EDAMAGE, "Can't get object CrossCSet: has the wrong objid", CNULL, 0, proc);
		return ( (SET_OF_CertificatePair * ) 0);
	}

	cpairset = d_CertificatePairSet(&content);
	free(content.octets);

	if (!cpairset) {
		aux_add_error(EDECODE, "Can't decode object CrossCSet", CNULL, 0, proc);
		return ( (SET_OF_CertificatePair * ) 0);
	}

	CrossCSet_cache = cpairset;
	return(aux_cpy_SET_OF_CertificatePair(CrossCSet_cache));
}


/****************************************************************************/

/***************************************************************
 *
 * Procedure af_pse_get_FCPath
 *
 ***************************************************************/
#ifdef __STDC__

FCPath *af_pse_get_FCPath(
	DName	 *name
)

#else

FCPath *af_pse_get_FCPath(
	name
)
DName	 *name;

#endif

{
	PSESel            * pse_sel;
	FCPath            * fcpath;
        ObjId 		    objid;
	OctetString 	    content;
	CrossCertificates * crosscert;
	FCPath 		  * current_level_ref;
	int		    found = 0;
	char		  * proc = "af_pse_get_FCPath";

	if(FCPath_read) {
		if(!FCPath_cache) aux_add_error(EOBJNAME, "Object FCPath doesn't exist", CNULL, 0, proc);
		return(aux_cpy_FCPath(FCPath_cache));
	}
	FCPath_read = TRUE;	

	if (!(pse_sel = af_pse_open(FCPath_OID, FALSE))) {
		if(aux_last_error() != EOBJNAME) AUX_ADD_ERROR;
		else aux_add_error(LASTERROR, "Can't open object FCPath", CNULL, 0, proc);
		return ( (FCPath * ) 0);
	}

	if (sec_read_PSE(pse_sel, &objid, &content) < 0 ) {
		aux_add_error(EREADPSE, "Can't read object FCPath", (char *) pse_sel, PSESel_n, proc);
		aux_free_PSESel(&pse_sel);
		return ( (FCPath * ) 0);
	}
	aux_free_PSESel(&pse_sel);
        if(aux_cmp_ObjId(&objid, FCPath_OID)) {
		aux_add_error(EDAMAGE, "Can't get object FCPath: has the wrong objid", CNULL, 0, proc);
		free(content.octets);
		return ( (FCPath * ) 0);
	}

	if (!(fcpath = d_FCPath(&content)) ) {
		free(content.octets);
		aux_add_error(EDECODE, "Can't decode object FCPath", CNULL, 0, proc);
		return ( (FCPath * ) 0);
	}

	free(content.octets);

	if (name)       /*return reduced forward certification path*/ {
		if (!aux_cmp_DName(name, fcpath->liste->element->tbs->issuer)) 
			aux_free_FCPath( &(fcpath->next_forwardpath) );
		else {
			current_level_ref = fcpath->next_forwardpath;
			while (current_level_ref) {
				crosscert = current_level_ref->liste;
				while (crosscert) {
					if (!found) {
						if (aux_cmp_DName(name, crosscert->element->tbs->issuer) > 0) {
							if (current_level_ref->next_forwardpath) {
								if (aux_cmp_DName(current_level_ref->next_forwardpath->liste->element->tbs->subject,
								     								     crosscert->element->tbs->issuer) > 0 )
/* code folded from here */
	aux_free2_Certificate( crosscert->element );
/* unfolding */
								else /*crosscert ist Hierarchie-Zertifikat*/			     {
/* code folded from here */
	if ( !( current_level_ref->liste->element = aux_cpy_Certificate(crosscert->element) ) ) {
		aux_add_error(EMALLOC, "aux_cpy_Certificate", CNULL, 0, proc);
		return ( (FCPath * ) 0);
	}
	current_level_ref->liste->next = (CrossCertificates * )0;
	aux_free_Certificate( &(crosscert->element) );
/* unfolding */
								}
							} else 
								aux_free_Certificate( &(crosscert->element) );
						} else {
							found = 1;
							if (!(current_level_ref->liste->element = aux_cpy_Certificate(crosscert->element))) {
								aux_add_error(EMALLOC, "aux_cpy_Certificate", CNULL, 0, proc);
								return ( (FCPath * ) 0);
							}
							current_level_ref->liste->next = (CrossCertificates * )0;
							aux_free_Certificate(&(crosscert->element));
						}
					}   /*if*/ else /*found = 1*/
						aux_free_Certificate( &(crosscert->element) );
					crosscert = crosscert->next;
				}  /*while*/

				current_level_ref = current_level_ref->next_forwardpath;
			}   /*while*/
		}  /*else*/
	}  /*if*/

	if (name && !found) {
		aux_add_error(EINVALID, "Can't get the reduced FCPath for the specified name: not found", (char *) name, DName_n, proc);
		return ( (FCPath * ) 0);
	}
	FCPath_cache = fcpath;
	return(aux_cpy_FCPath(fcpath));

}  /*af_pse_get_FCPath()*/


/****************************************************************************/

/***************************************************************
 *
 * Procedure af_pse_get_Certificates
 *
 ***************************************************************/
#ifdef __STDC__

Certificates *af_pse_get_Certificates(
	KeyType	  type,
	DName	 *name
)

#else

Certificates *af_pse_get_Certificates(
	type,
	name
)
KeyType	  type;
DName	 *name;

#endif

{
	Certificates * certs;
	Certificate  * cert;
	FCPath	     * fcpath;
	char	     * proc = "af_pse_get_Certificates";

	if (!(cert = af_pse_get_Certificate(type, NULLDNAME, 0)))  {
		AUX_ADD_ERROR;
		return ( (Certificates * ) 0);
	}


	fcpath = af_pse_get_FCPath(name);

	if(!fcpath) {
		if(aux_last_error() != EOBJNAME) {
			AUX_ADD_ERROR;
			return( (Certificates * ) 0);
		} else aux_free_error();
	}

        if(!(certs = aux_create_Certificates(cert, fcpath))) {
		aux_add_error(EINVALID, "Can't create the certificates structure", CNULL, 0, proc);
	}

        aux_free_Certificate(&cert);
	if(fcpath) aux_free_FCPath(&fcpath);

	return(certs);

}  /*af_pse_get_Certificates()*/


/****************************************************************************/

/***************************************************************
 *
 * Procedure af_pse_get_PKRoot
 *
 ***************************************************************/
#ifdef __STDC__

PKRoot *af_pse_get_PKRoot(
)

#else

PKRoot *af_pse_get_PKRoot(
)

#endif

{
	PSESel      * pse_sel;
	PKRoot      * pkroot;
        ObjId         objid;
	OctetString   content;
	char	    * proc = "af_pse_get_PKRoot";


	if(PKRoot_read) {
		if(!PKRoot_cache) aux_add_error(EOBJNAME, "Object PKRoot doesn't exist", CNULL, 0, proc);
		return(aux_cpy_PKRoot(PKRoot_cache));
	}
	PKRoot_read = TRUE;

	if (!(pse_sel = af_pse_open(PKRoot_OID, FALSE))) {
		if(aux_last_error() != EOBJNAME) AUX_ADD_ERROR;
		else aux_add_error(EROOTKEY, "Can't open object PKRoot", CNULL, 0, proc);
		return ( (PKRoot * ) 0);
	}

	if (sec_read_PSE(pse_sel, &objid, &content) < 0) {
		aux_add_error(EREADPSE, "Can't read object PKRoot", (char *) pse_sel, PSESel_n, proc);
        	aux_free_PSESel(&pse_sel);
		return ( (PKRoot * ) 0);
	}
	aux_free_PSESel(&pse_sel);
        if(aux_cmp_ObjId(&objid, PKRoot_OID)) {
		aux_add_error(EDAMAGE, "Can't get object PKRoot: has the wrong objid", CNULL, 0, proc);
		free(content.octets);
		return ( (PKRoot * ) 0);
	}

	if (!(pkroot = d_PKRoot(&content))) {
		free(content.octets);
		aux_add_error(EDECODE, "Can't decode object PKRoot", CNULL, 0, proc);
		return ( (PKRoot * ) 0);
	}

	free(content.octets);

	PKRoot_cache = pkroot;
	return(aux_cpy_PKRoot(pkroot));

}  /*af_pse_get_PKRoot()*/


/****************************************************************************/

/***************************************************************
 *
 * Procedure af_pse_get_PKList
 *
 ***************************************************************/
#ifdef __STDC__

PKList *af_pse_get_PKList(
	KeyType	  type
)

#else

PKList *af_pse_get_PKList(
	type
)
KeyType	  type;

#endif

{
	PSESel      * pse_sel;
	PKList      * list;
	OctetString   content;
        ObjId	      objid;
	ObjId 	    * obj_type = NULLOBJID;
	int	      i;
	Boolean       onekeypaironly = FALSE;

	char	    * proc = "af_pse_get_PKList";


	if ((type != ENCRYPTION) && (type != SIGNATURE)) {
		aux_add_error(EINVALID, "Invalid parameter", CNULL, 0, proc);
		return ( (PKList * ) 0);
	}

	if(af_check_if_onekeypaironly(&onekeypaironly)){
		AUX_ADD_ERROR;
		return ( (PKList * )0);
	}

	if ((type == SIGNATURE) || (onekeypaironly == TRUE)) { 
		if(PKList_read) {
			if(!PKList_cache) aux_add_error(EOBJNAME, "Object PKList doesn't exist", CNULL, 0, proc);
			return(aux_cpy_PKList(PKList_cache));
		}
		PKList_read = TRUE;
		obj_type = PKList_OID;
	}
	else {
		if(EKList_read) {
			if(!EKList_cache) aux_add_error(EOBJNAME, "Object EKList doesn't exist", CNULL, 0, proc);
			return(aux_cpy_PKList(EKList_cache));
		}
		EKList_read = TRUE;
		obj_type = EKList_OID;
	}

	if (!(pse_sel = af_pse_open(obj_type, FALSE))) {
		if(aux_last_error() != EOBJNAME) AUX_ADD_ERROR;
		else aux_add_error(LASTERROR, "Can't get PKList because cannot open object", CNULL, 0, proc);
		return ( (PKList * ) 0);
	}

	if (sec_read_PSE(pse_sel, &objid, &content) < 0 ) {
		aux_add_error(EREADPSE, "Can't get PKList because cannot read object", (char *) pse_sel, PSESel_n, proc);
        	aux_free_PSESel(&pse_sel);
		return ( (PKList * ) 0);
	}
	aux_free_PSESel(&pse_sel);
        if(aux_cmp_ObjId(&objid, obj_type)) {
		aux_add_error(EDAMAGE, "Can't get PKList/EKList because reed object has the wrong objid", CNULL, 0, proc);
		free(content.octets);
		return ( (PKList * ) 0);
	}

	if (!(list = d_PKList(&content))) {
		free(content.octets);
		aux_add_error(EDECODE, "Can't decode PKList", CNULL, 0, proc);
		return ( (PKList * ) 0);
	}

	free(content.octets);

	if ((type == SIGNATURE) || (onekeypaironly == TRUE)) PKList_cache = list;
	else EKList_cache = list;

	return(aux_cpy_PKList(list));

}   /*af_pse_get_PKList()*/


/****************************************************************************/


/***************************************************************
 *
 * Procedure af_pse_get_PCAList
 *
 ***************************************************************/

PKList *af_pse_get_PCAList()


{
	PSESel      * pse_sel;
	PKList      * list;
	OctetString   content;
        ObjId	      objid;
	ObjId 	    * obj_type = NULLOBJID;
	int	      i;

	char	    * proc = "af_pse_get_PCAList";


	if(PCAList_read) {
		if(!PCAList_cache) aux_add_error(EOBJNAME, "Object PCAList doesn't exist", CNULL, 0, proc);
		return(aux_cpy_PKList(PCAList_cache));
	}
	PCAList_read = TRUE;

	obj_type = PCAList_OID;

	if (!(pse_sel = af_pse_open(obj_type, FALSE))) {
		if(aux_last_error() != EOBJNAME) AUX_ADD_ERROR;
		else aux_add_error(LASTERROR, "Can't get PCAList because cannot open object", CNULL, 0, proc);
		return ( (PKList * ) 0);
	}

	if (sec_read_PSE(pse_sel, &objid, &content) < 0 ) {
		aux_add_error(EREADPSE, "Can't get PCAList because cannot read object", (char *) pse_sel, PSESel_n, proc);
        	aux_free_PSESel(&pse_sel);
		return ( (PKList * ) 0);
	}
	aux_free_PSESel(&pse_sel);
        if(aux_cmp_ObjId(&objid, obj_type)) {
		aux_add_error(EDAMAGE, "Can't get PCAList because reed object has the wrong objid", CNULL, 0, proc);
		free(content.octets);
		return ( (PKList * ) 0);
	}

	if (!(list = d_PKList(&content))) {
		free(content.octets);
		aux_add_error(EDECODE, "Can't decode PCAList", CNULL, 0, proc);
		return ( (PKList * ) 0);
	}

	free(content.octets);

	PCAList_cache = list;

	return(aux_cpy_PKList(list));

}   /*af_pse_get_PCAList()*/


/****************************************************************************/
/***************************************************************
 *
 * Procedure af_pse_get_CrlSet
 *
 ***************************************************************/
#ifdef __STDC__

CrlSet *af_pse_get_CrlSet(
)

#else

CrlSet *af_pse_get_CrlSet(
)

#endif

{
	PSESel       * pse_sel;
	OctetString    content;
        ObjId 	       objid;
	CrlSet       * crlset;
	char	     * proc = "af_pse_get_CrlSet";



	if(CrlSet_read) {
		if(!CrlSet_cache) aux_add_error(EOBJNAME, "Object CrlSet doesn't exist", CNULL, 0, proc);
		return(aux_cpy_CrlSet(CrlSet_cache));
	}
	CrlSet_read = TRUE;

	if (!(pse_sel = af_pse_open(CrlSet_OID, FALSE))) {
		if(aux_last_error() != EOBJNAME) AUX_ADD_ERROR;
		else aux_add_error(LASTERROR, "Can't open object CrlSet", CNULL, 0, proc);
		return ( (CrlSet * ) 0);
	}

	if (sec_read_PSE(pse_sel, &objid, &content) < 0 ) {
		aux_add_error(EREADPSE, "Can't read object CrlSet", (char *) pse_sel, PSESel_n, proc);
        	aux_free_PSESel(&pse_sel);
		return ( (CrlSet * ) 0);
	}
	aux_free_PSESel(&pse_sel);
        if(aux_cmp_ObjId(&objid, CrlSet_OID)) {
		aux_add_error(EDAMAGE, "Can't get object CrlSet: has the wrong objid", CNULL, 0, proc);
		free(content.octets);
		return ( (CrlSet * ) 0);
	}

	crlset = d_CrlSet(&content);
	free(content.octets);

	if (!crlset) {
		aux_add_error(EDECODE, "Can't decode object CrlSet", CNULL, 0, proc);
		return ( (CrlSet * ) 0);
	}

	CrlSet_cache = crlset;
	return(aux_cpy_CrlSet(CrlSet_cache));
}


/****************************************************************************/


/***************************************************************
 *
 * Procedure af_pse_update_Certificate
 *
 ***************************************************************/
#ifdef __STDC__

RC af_pse_update_Certificate(
	KeyType		  type,
	Certificate	 *cert,
	Boolean		  hierarchy
)

#else

RC af_pse_update_Certificate(
	type,
	cert,
	hierarchy
)
KeyType		  type;
Certificate	 *cert;
Boolean		  hierarchy;

#endif

{

	/* hierarchy = TRUE  (hierarchy certifiacte) updates SignCert Or EncCert 
                     = FALSE (crosscertificate) updates SignCSet or EncCSet      */

	PSESel 		   * pse_sel;
	OctetString 	   * content;
	ObjId 		   * obj_type = NULLOBJID;
	SET_OF_Certificate * setofcert, * savesoc = (SET_OF_Certificate * ) 0;
	Boolean		     onekeypaironly = FALSE;

	char		   * proc = "af_pse_update_Certificate";


	if (((type != SIGNATURE) && (type != ENCRYPTION)) || !cert ) {
		aux_add_error(EINVALID, "Invalid parameter", CNULL, 0, proc);
		return (-1);
	}

	if(af_check_if_onekeypaironly(&onekeypaironly)){
		AUX_ADD_ERROR;
		return (- 1);
	}

	if (hierarchy) {
		if(onekeypaironly == TRUE)
			obj_type = Cert_OID;
		else{
			if (type == SIGNATURE) 
				obj_type = SignCert_OID;
			else 
				obj_type = EncCert_OID;
		}
	} 
	else {
		if(onekeypaironly == TRUE)
			obj_type = CSet_OID;
		else{
			if (type == SIGNATURE) 
				obj_type = SignCSet_OID;
			else 
				obj_type = EncCSet_OID;
		}
	}

	if (!(pse_sel = af_pse_open(obj_type, TRUE))) {
		aux_add_error(LASTERROR, "Can't update user certficate because cannot open object", CNULL, 0, proc);
		return (-1);
	}

	if (hierarchy) {

		/* Hierarchy Certificate */

		if (!(content = e_Certificate(cert))) {
			aux_free_PSESel(&pse_sel);
			aux_add_error(EENCODE, "Can't encode certificate", CNULL, 0, proc);
			return (-1);
		}

		if (sec_write_PSE(pse_sel, obj_type, content) < 0) {
			aux_add_error(EWRITEPSE, "Can't update user certficate because cannot write object", (char *) pse_sel, PSESel_n, proc);
			aux_free_PSESel(&pse_sel);
			aux_free_OctetString(&content);
			return (-1);
		}
		aux_free_PSESel(&pse_sel);
		aux_free_OctetString(&content);
		if(onekeypaironly == TRUE) {
			if(Cert_cache) aux_free_Certificate(&Cert_cache);
			Cert_cache = aux_cpy_Certificate(cert);
			Cert_read = TRUE;
		}
		else {
			if (type == SIGNATURE) { 
				if(SignCert_cache) aux_free_Certificate(&SignCert_cache);
				SignCert_cache = aux_cpy_Certificate(cert);
				SignCert_read = TRUE;
			}
			else {
				if(EncCert_cache) aux_free_Certificate(&EncCert_cache);
				EncCert_cache = aux_cpy_Certificate(cert);
				EncCert_read = TRUE;
			}
		}
	} 
        else {

		/* CertificateSet */

		if (!(setofcert = af_pse_get_CertificateSet(type))) {

			if(aux_last_error() != EOBJNAME) {
				AUX_ADD_ERROR;
				return(-1);
			} else aux_free_error();

			if (!(setofcert = (SET_OF_Certificate * )malloc(sizeof(SET_OF_Certificate)))) {
				aux_add_error(EMALLOC, "setofcert", CNULL, 0, proc);
				aux_free_PSESel(&pse_sel);
				return( -1);
			}
			setofcert->element = aux_cpy_Certificate(cert);
			setofcert->next = (SET_OF_Certificate * )0;
		} 
		else {
			savesoc = setofcert;
			while (setofcert) {
				if (!aux_cmp_DName(setofcert->element->tbs->issuer, cert->tbs->issuer)
				     && !aux_cmp_OctetString(setofcert->element->tbs->serialnumber, cert->tbs->serialnumber)) {

					/* update existing member */

					aux_free_Certificate(&setofcert->element);

					setofcert->element = aux_cpy_Certificate(cert);
					break;
				}
				if (!setofcert->next) {

					/* add new member */

					if (!(setofcert->next = (SET_OF_Certificate * ) malloc(sizeof(SET_OF_Certificate)))) {
						aux_free_PSESel(&pse_sel);
						aux_add_error(EMALLOC, "setofcert->next", CNULL, 0, proc);
						aux_free_CertificateSet(&savesoc);
						return( -1);
					}
					setofcert = setofcert->next;
					setofcert->element = aux_cpy_Certificate(cert);
					setofcert->next = (SET_OF_Certificate * )0;
					break;
				}
				setofcert = setofcert->next;
			}

			setofcert = savesoc;
		}

		if (!(content = e_CertificateSet(setofcert)) ) {
			aux_free_PSESel(&pse_sel);
			aux_free_CertificateSet(&setofcert);
			aux_add_error(EENCODE, "Can't encode certificate set", CNULL, 0, proc);
			return (-1);
		}

		if (sec_write_PSE(pse_sel, obj_type, content) < 0 ) {
			aux_add_error(EWRITEPSE, "Can't update certificate because cannot write object", (char *) pse_sel, PSESel_n, proc);
			aux_free_PSESel(&pse_sel);
			aux_free_OctetString(&content);
			aux_free_CertificateSet(&setofcert);
			return (-1);
		}
		aux_free_PSESel(&pse_sel);
		aux_free_OctetString(&content);
		if(onekeypaironly == TRUE) {
			if(CSet_cache) aux_free_CertificateSet(&CSet_cache);
			CSet_cache = aux_cpy_SET_OF_Certificate(setofcert);
			CSet_read = TRUE;
		}
		else {
			if (type == SIGNATURE) { 
				if(SignCSet_cache) aux_free_CertificateSet(&SignCSet_cache);
				SignCSet_cache = aux_cpy_SET_OF_Certificate(setofcert);
				SignCSet_read = TRUE;
			}
			else {
				if(EncCSet_cache) aux_free_CertificateSet(&EncCSet_cache);
				EncCSet_cache = aux_cpy_SET_OF_Certificate(setofcert);
				EncCSet_read = TRUE;
			}
		}
		aux_free_CertificateSet(&setofcert);
	}

	return(0);

}   /*af_pse_update_Certificate()*/


/****************************************************************************/

/***************************************************************
 *
 * Procedure af_pse_update_FCPath
 *
 ***************************************************************/
#ifdef __STDC__

RC af_pse_update_FCPath(
	FCPath	 *fcpath
)

#else

RC af_pse_update_FCPath(
	fcpath
)
FCPath	 *fcpath;

#endif

{
	PSESel 		* pse_sel;
	OctetString 	* content;
	char		* proc = "af_pse_update_FCPath";

	if (!fcpath) {
		aux_add_error(EINVALID, "Invalid parameter", CNULL, 0, proc);
		return (-1);
	}

	if (!(pse_sel = af_pse_open(FCPath_OID, TRUE))) {
		aux_add_error(LASTERROR, "Can't open object FCPath", CNULL, 0, proc);
		return (-1);
	}

	if (!(content = e_FCPath(fcpath)) ) {
		aux_free_PSESel(&pse_sel);
		aux_add_error(EENCODE, "Can't encode FCPath", CNULL, 0, proc);
		return (-1);
	}

	if (sec_write_PSE(pse_sel, FCPath_OID, content) < 0 ) {
		aux_add_error(EWRITEPSE, "Can't write object FCPath", (char *) pse_sel, PSESel_n, proc);
		aux_free_PSESel(&pse_sel);
		aux_free_OctetString(&content);
		return (-1);
	}
	aux_free_PSESel(&pse_sel);
	aux_free_OctetString(&content);

	if(FCPath_cache) aux_free_FCPath(&FCPath_cache);
	FCPath_cache = aux_cpy_FCPath(fcpath);
	FCPath_read = TRUE;

	return(0);

}  /*af_pse_update_FCPath()*/


/****************************************************************************/

/***************************************************************
 *
 * Procedure af_pse_update_CertificatePairSet
 *
 ***************************************************************/
#ifdef __STDC__

RC af_pse_update_CertificatePairSet(
	SET_OF_CertificatePair	 *cpairset
)

#else

RC af_pse_update_CertificatePairSet(
	cpairset
)
SET_OF_CertificatePair	 *cpairset;

#endif

{
	PSESel 		* pse_sel;
	OctetString	* content;
	char		* proc = "af_pse_update_CertificatePairSet";

	if (!cpairset) {
		aux_add_error(EINVALID, "Invalid parameter", CNULL, 0, proc);
		return (-1);
	}

	if (!(pse_sel = af_pse_open(CrossCSet_OID, TRUE))) {
		aux_add_error(LASTERROR, "Can't open object CrossCSet", CNULL, 0, proc);
		return (-1);
	}

	if (!(content = e_CertificatePairSet(cpairset)) ) {
		aux_free_PSESel(&pse_sel);
		aux_add_error(EENCODE, "Can't encode cross certificate set", CNULL, 0, proc);
		return (-1);
	}

	if (sec_write_PSE(pse_sel, CrossCSet_OID, content) < 0 ) {
		aux_add_error(EWRITEPSE, "Can't write object CrossCSet", (char *) pse_sel, PSESel_n, proc);
		aux_free_PSESel(&pse_sel);
		aux_free_OctetString(&content);
		return (-1);
	}
	aux_free_PSESel(&pse_sel);
	aux_free_OctetString(&content);

	if(CrossCSet_cache) aux_free_CertificatePairSet(&CrossCSet_cache);
	CrossCSet_cache = aux_cpy_SET_OF_CertificatePair(cpairset);
	CrossCSet_read = TRUE;

	return(0);

}  /*af_pse_update_CertificatePairSet()*/


/****************************************************************************/


/***************************************************************
 *
 * Procedure af_pse_update_AliasList
 *
 ***************************************************************/
#ifdef __STDC__

RC af_pse_update_AliasList(
	AliasList	 *alist
)

#else

RC af_pse_update_AliasList(
	alist
)
AliasList	 *alist;

#endif

{
	PSESel 		* pse_sel;
	OctetString 	* content;
	char		* proc = "af_pse_update_AliasList";

	if (!alist) {
		aux_add_error(EINVALID, "Invalid parameter", CNULL, 0, proc);
		return (-1);
	}

	if (!(pse_sel = af_pse_open(AliasList_OID, TRUE))) {
		aux_add_error(LASTERROR, "Can't open object AliasList", CNULL, 0, proc);
		return (-1);
	}

	if (!(content = e_AliasList(alist)) ) {
		aux_free_PSESel(&pse_sel);
		aux_add_error(EENCODE, "Can't encode AliasList", CNULL, 0, proc);
		return (-1);
	}

	if (sec_write_PSE(pse_sel, AliasList_OID, content) < 0 ) {
		aux_add_error(EWRITEPSE, "Can't write object AliasList", (char *) pse_sel, PSESel_n, proc);
		aux_free_PSESel(&pse_sel);
		aux_free_OctetString(&content);
		return (-1);
	}
	aux_free_PSESel(&pse_sel);
	aux_free_OctetString(&content);

	if(AliasList_cache) af_pse_reset(AliasList_name);
	AliasList_cache = aux_cpy_AliasList(alist);
	AliasList_read = TRUE;

	return(0);

}  /*af_pse_update_AliasList()*/




/****************************************************************************/


/***************************************************************
 *
 * Procedure af_pse_update_QuipuPWD
 *
 ***************************************************************/
#ifdef __STDC__

RC af_pse_update_QuipuPWD(
	QuipuPWD	 pwd
)

#else

RC af_pse_update_QuipuPWD(
	pwd
)
QuipuPWD	 pwd;

#endif

{
	PSESel 		* pse_sel;
	OctetString 	* content;
	char		* proc = "af_pse_update_QuipuPWD";

	if (!pwd) {
		aux_add_error(EINVALID, "Invalid parameter", CNULL, 0, proc);
		return (-1);
	}

	if (!(pse_sel = af_pse_open(QuipuPWD_OID, TRUE))) {
		aux_add_error(LASTERROR, "Can't open object QuipuPWD", CNULL, 0, proc);
		return (-1);
	}

	if (!(content = e_GRAPHICString((char *)pwd)) ) {
		aux_free_PSESel(&pse_sel);
		aux_add_error(EENCODE, "Can't encode QuipuPWD", CNULL, 0, proc);
		return (-1);
	}

	if (sec_write_PSE(pse_sel, QuipuPWD_OID, content) < 0 ) {
		aux_add_error(EWRITEPSE, "Can't write object FCPath", (char *) pse_sel, PSESel_n, proc);
		aux_free_PSESel(&pse_sel);
		aux_free_OctetString(&content);
		return (-1);
	}
	aux_free_PSESel(&pse_sel);
	aux_free_OctetString(&content);

	if(QuipuPWD_cache) free(QuipuPWD_cache);
	QuipuPWD_cache = (QuipuPWD)aux_cpy_String((char *)pwd);
	QuipuPWD_read = TRUE;

	return(0);

}  /*af_pse_update_QuipuPWD()*/


/****************************************************************************/


/***************************************************************
 *
 * Procedure af_pse_update_SerialNumber
 *
 ***************************************************************/
#ifdef __STDC__

RC af_pse_update_SerialNumber(
	SerialNumber	 *serial
)

#else

RC af_pse_update_SerialNumber(
	serial
)
SerialNumber	 *serial;

#endif

{
	PSESel 		* pse_sel;
	OctetString 	* content;
	char		* proc = "af_pse_update_SerialNumber";

	if (! serial) {
		aux_add_error(EINVALID, "Invalid parameter", CNULL, 0, proc);
		return (-1);
	}

	if (!(pse_sel = af_pse_open(SerialNumber_OID, TRUE))) {
		aux_add_error(LASTERROR, "Can't open object SerialNumber", CNULL, 0, proc);
		return (-1);
	}

	if (!(content = e_SerialNumber(serial)) ) {
		aux_free_PSESel(&pse_sel);
		aux_add_error(EENCODE, "Can't encode SerialNumber", CNULL, 0, proc);
		return (-1);
	}

	if (sec_write_PSE(pse_sel, SerialNumber_OID, content) < 0 ) {
		aux_add_error(EWRITEPSE, "Can't write object SerialNumber", (char *) pse_sel, PSESel_n, proc);
		aux_free_PSESel(&pse_sel);
		aux_free_OctetString(&content);
		return (-1);
	}
	aux_free_PSESel(&pse_sel);
	aux_free_OctetString(&content);

	if(SerialNumber_cache) aux_free_OctetString((SerialNumber **)&SerialNumber_cache);
	SerialNumber_cache = (SerialNumber *)aux_cpy_OctetString(serial);
	SerialNumber_read = TRUE;

	return(0);

}  /*af_pse_update_SerialNumber()*/


/****************************************************************************/


/***************************************************************
 *
 * Procedure af_pse_update_PKRoot
 *
 ***************************************************************/
#ifdef __STDC__

RC af_pse_update_PKRoot(
	PKRoot	 *pkroot
)

#else

RC af_pse_update_PKRoot(
	pkroot
)
PKRoot	 *pkroot;

#endif

{
	PSESel		* pse_sel;
	OctetString 	* content;
	char		* proc = "af_pse_update_PKRoot";

	if (!pkroot) {
		aux_add_error(EINVALID, "Invalid parameter", CNULL, 0, proc);
		return (-1);
	}

	if (!(pse_sel = af_pse_open(PKRoot_OID, TRUE))) {
		aux_add_error(LASTERROR, "Can't open object PKRoot", CNULL, 0, proc);
		return (-1);
	}

	if (!(content = e_PKRoot(pkroot))) {
		aux_free_PSESel(&pse_sel);
		aux_add_error(EENCODE, "Can't encode PKRoot", CNULL, 0, proc);
		return (-1);
	}

	if ( sec_write_PSE(pse_sel, PKRoot_OID, content) < 0 ) {
		aux_add_error(EWRITEPSE, "Can't write object PKRoot", (char *) pse_sel, PSESel_n, proc);
		aux_free_OctetString(&content);
		aux_free_PSESel(&pse_sel);
		return (-1);
	}
	aux_free_PSESel(&pse_sel);
	aux_free_OctetString(&content);

	if(PKRoot_cache) aux_free_PKRoot(&PKRoot_cache);
	PKRoot_cache = aux_cpy_PKRoot(pkroot);
	PKRoot_read = TRUE;

	return(0);

}  /*af_pse_update_PKRoot()*/


/****************************************************************************/

/***************************************************************
 *
 * Procedure af_pse_update_PKList
 *
 ***************************************************************/
#ifdef __STDC__

RC af_pse_update_PKList(
	KeyType	  type,
	PKList	 *list
)

#else

RC af_pse_update_PKList(
	type,
	list
)
KeyType	  type;
PKList	 *list;

#endif

{
	PSESel      * pse_sel;
	OctetString * content;
	ObjId       * obj_type = NULLOBJID;
	Boolean       onekeypaironly = FALSE;

	char	    * proc = "af_pse_update_PKList";

	if (((type != ENCRYPTION) && (type != SIGNATURE)) || !list ) {
		aux_add_error(EINVALID, "Invalid parameter", CNULL, 0, proc);
		return (-1);
	}

	if(af_check_if_onekeypaironly(&onekeypaironly)){
		AUX_ADD_ERROR;
		return(- 1);
	}

	if ((type == SIGNATURE) || (onekeypaironly == TRUE)) 
		obj_type = PKList_OID;
	else 
		obj_type = EKList_OID;

	if (!(pse_sel = af_pse_open(obj_type, TRUE))) {
		aux_add_error(LASTERROR, "Can't update PKList because cannot open object", CNULL, 0, proc);
		return (-1);
	}

	if ( !(content = e_PKList(list)) ) {
		aux_free_PSESel(&pse_sel);
		aux_add_error(EENCODE, "Can't encode PKList", CNULL, 0, proc);
		return (-1);
	}

	if ( sec_write_PSE(pse_sel, obj_type, content) < 0 ) {
		aux_add_error(EWRITEPSE, "Can't update PKList because cannot write object", (char *) pse_sel, PSESel_n, proc);
		aux_free_OctetString(&content);
		aux_free_PSESel(&pse_sel);
		return (-1);
	}
	aux_free_PSESel(&pse_sel);
	aux_free_OctetString(&content);

	if ((type == SIGNATURE) || (onekeypaironly == TRUE)) {
		if(PKList_cache) aux_free_PKList(&PKList_cache);
		PKList_cache = aux_cpy_PKList(list);
		PKList_read = TRUE;
	}
	else {
		if(EKList_cache) aux_free_PKList(&EKList_cache);
		EKList_cache = aux_cpy_PKList(list);
		EKList_read = TRUE;
	}
	return(0);

}   /*af_pse_update_PKList()*/


/****************************************************************************/

/***************************************************************
 *
 * Procedure af_pse_update_PCAList
 *
 ***************************************************************/
#ifdef __STDC__

RC af_pse_update_PCAList(
	PKList	 *list
)

#else

RC af_pse_update_PCAList(
	list
)
PKList	 *list;

#endif

{
	PSESel      * pse_sel;
	OctetString * content;
	ObjId       * obj_type = NULLOBJID;

	char	    * proc = "af_pse_update_PCAList";

	obj_type = PCAList_OID;

	if (!(pse_sel = af_pse_open(obj_type, TRUE))) {
		aux_add_error(LASTERROR, "Can't update PCAList because cannot open object", CNULL, 0, proc);
		return (-1);
	}

	if ( !(content = e_PKList(list)) ) {
		aux_free_PSESel(&pse_sel);
		aux_add_error(EENCODE, "Can't encode PCAList", CNULL, 0, proc);
		return (-1);
	}

	if ( sec_write_PSE(pse_sel, obj_type, content) < 0 ) {
		aux_add_error(EWRITEPSE, "Can't update PCAList because cannot write object", (char *) pse_sel, PSESel_n, proc);
		aux_free_OctetString(&content);
		aux_free_PSESel(&pse_sel);
		return (-1);
	}
	aux_free_PSESel(&pse_sel);
	aux_free_OctetString(&content);

	if(PCAList_cache) aux_free_PKList(&PCAList_cache);
	PCAList_cache = aux_cpy_PKList(list);
	PCAList_read = TRUE;

	return(0);

}   /*af_pse_update_PCAList()*/


/****************************************************************************/

/***************************************************************
 *
 * Procedure af_pse_update_CrlSet
 *
 ***************************************************************/
#ifdef __STDC__

RC af_pse_update_CrlSet(
	CrlSet	 *crlset
)

#else

RC af_pse_update_CrlSet(
	crlset
)
CrlSet	 *crlset;

#endif

{
	PSESel		 * pse_sel;
	OctetString	 * content;
	char		 * proc = "af_pse_update_CrlSet";

	if (!crlset) {
		aux_add_error(EINVALID, "Invalid parameter", CNULL, 0, proc);
		return (-1);
	}

	if (!(pse_sel = af_pse_open(CrlSet_OID, TRUE))) {
		aux_add_error(LASTERROR, "Can't open object CrlSet", CNULL, 0, proc);
		return (-1);
	}

	if (!(content = e_CrlSet(crlset)) ) {
		aux_free_PSESel(&pse_sel);
		aux_add_error(EENCODE, "Can't encode CrlSet", CNULL, 0, proc);
		return (-1);
	}

	if (sec_write_PSE(pse_sel, CrlSet_OID, content) < 0 ) {
		aux_add_error(EWRITEPSE, "Can't write object CrlSet", (char *) pse_sel, PSESel_n, proc);
		aux_free_PSESel(&pse_sel);
		aux_free_OctetString(&content);
		return (-1);
	}
	aux_free_PSESel(&pse_sel);
	aux_free_OctetString(&content);

	if(CrlSet_cache) aux_free_CrlSet(&CrlSet_cache);
	CrlSet_cache = aux_cpy_CrlSet(crlset);
	CrlSet_read = TRUE;

	return (0);

}  /*af_pse_update_CrlSet()*/


/****************************************************************************/

/***************************************************************
 *
 * Procedure af_pse_update_DHparam
 *
 ***************************************************************/
#ifdef __STDC__

RC af_pse_update_DHparam(
	AlgId	 *dhparam
)

#else

RC af_pse_update_DHparam(
	dhparam
)
AlgId	 *dhparam;

#endif

{
	PSESel		* pse_sel;
	OctetString 	* content;
	char		* proc = "af_pse_update_DHparam";

	if (!dhparam) {
		aux_add_error(EINVALID, "Invalid parameter", CNULL, 0, proc);
		return (-1);
	}

	if (!(pse_sel = af_pse_open(DHparam_OID, TRUE))) {
		aux_add_error(LASTERROR, "Can't open PSE object DHparam", CNULL, 0, proc);
		return (-1);
	}

	if (!(content = e_AlgId(dhparam))) {
		aux_free_PSESel(&pse_sel);
		aux_add_error(EENCODE, "Can't encode DHparam", CNULL, 0, proc);
		return (-1);
	}

	if ( sec_write_PSE(pse_sel, DHparam_OID, content) < 0 ) {
		aux_add_error(EWRITEPSE, "Can't write PSE object DHparam", (char *) pse_sel, PSESel_n, proc);
		aux_free_OctetString(&content);
		aux_free_PSESel(&pse_sel);
		return (-1);
	}
	aux_free_PSESel(&pse_sel);
	aux_free_OctetString(&content);

	if(DHparam_cache) aux_free_AlgId(&DHparam_cache);
	DHparam_cache = aux_cpy_AlgId(dhparam);
	DHparam_read = TRUE;

	return(0);

}  /* af_pse_update_DHparam() */


/***************************************************************
 *
 * Procedure af_pse_add_PK
 *
 ***************************************************************/
#ifdef __STDC__

RC af_pse_add_PK(
	KeyType		  type,
	ToBeSigned	 *tbs
)

#else

RC af_pse_add_PK(
	type,
	tbs
)
KeyType		  type;
ToBeSigned	 *tbs;

#endif

{
	PKList      * newlist,  * list, * np, * ahead_np;
	ToBeSigned  * found_tbs;
	ObjId       * obj_type = NULLOBJID;
	RC            rc;
	int	      error;
	char	    * proc = "af_pse_add_PK";

	if (((type != ENCRYPTION) && (type != SIGNATURE)) || !tbs) {
		aux_add_error(EINVALID, "Invalid parameter", CNULL, 0, proc);
		return (-1);
	}

	if (!(newlist = (PKList * )malloc(sizeof(PKList))) ) {
		aux_add_error(EMALLOC, "newlist", CNULL, 0, proc);
		return ( - 1);
	}
	list = af_pse_get_PKList(type);

	if(!list) {

		if(aux_last_error() != EOBJNAME) {
			AUX_ADD_ERROR;
			return(-1);
		} else aux_free_error();
	}

	for (np = list; np; np = np->next) {
		if (!aux_cmp_DName(np->element->issuer, tbs->issuer)
		     && !aux_cmp_OctetString(np->element->serialnumber, tbs->serialnumber)) {
			error = EPK;
			break;
		}
		if (aux_cmp_KeyInfo(np->element->subjectPK, tbs->subjectPK) == 0) {
			error = EPKCROSS;
			break;
		}
	}

	if (np) {         /* tbs with same issuer and serialnumber or with same subjectPK
	                     already exists in PKList */
		found_tbs = aux_cpy_ToBeSigned(np->element);
		aux_free_PKList(&list);
		free(newlist);
		aux_add_error(error, "Don't add key to PKList because tbs with same issuer and serialnumber or with same subjectPK exists already", (char *) found_tbs, ToBeSigned_n, proc);  /* error is either EPK or EPKCROSS */
		return (-1);
	} 

	/* add new entry */
	newlist->element = aux_cpy_ToBeSigned(tbs);
	newlist->next    = list; /* NULL or existing list */

	/* update PKList/EKList on PSE */
	rc = af_pse_update_PKList(type, newlist);
	aux_free_PKList(&newlist);
	return(rc);                            

} /*af_pse_add_PK()*/


/****************************************************************************/

/***************************************************************
 *
 * Procedure af_pse_add_PCA
 *
 ***************************************************************/
#ifdef __STDC__

RC af_pse_add_PCA(
	ToBeSigned	 *tbs
)

#else

RC af_pse_add_PCA(
	tbs
)
ToBeSigned	 *tbs;

#endif

{
	PKList      * newlist,  * list, * np, * ahead_np;
	ToBeSigned  * found_tbs;
	ObjId       * obj_type = NULLOBJID;
	RC            rc;
	int	      error;
	char	    * proc = "af_pse_add_PCA";


	if (!(newlist = (PKList * )malloc(sizeof(PKList))) ) {
		aux_add_error(EMALLOC, "newlist", CNULL, 0, proc);
		return ( - 1);
	}
	list = af_pse_get_PCAList();

	if(!list) {

		if(aux_last_error() != EOBJNAME) {
			AUX_ADD_ERROR;
			return(-1);
		} else aux_free_error();
	}

	for (np = list; np; np = np->next) {
		if (!aux_cmp_DName(np->element->issuer, tbs->issuer)
		     && !aux_cmp_OctetString(np->element->serialnumber, tbs->serialnumber)) {
			error = EPK;
			break;
		}
		if (aux_cmp_KeyInfo(np->element->subjectPK, tbs->subjectPK) == 0) {
			error = EPKCROSS;
			break;
		}
	}

	if (np) {         /* tbs with same issuer and serialnumber or with same subjectPK
	                     already exists in PCAList */
		found_tbs = aux_cpy_ToBeSigned(np->element);
		aux_free_PKList(&list);
		free(newlist);
		aux_add_error(error, "Don't add key to PCAList because tbs with same issuer and serialnumber or with same subjectPK exists already", (char *) found_tbs, ToBeSigned_n, proc);  /* error is either EPK or EPKCROSS */
		return (-1);
	} 

	/* add new entry */
	newlist->element = aux_cpy_ToBeSigned(tbs);
	newlist->next    = list; /* NULL or existing list */

	/* update PCAList on PSE */
	rc = af_pse_update_PCAList(newlist);
	aux_free_PKList(&newlist);
	return(rc);                            

} /*af_pse_add_PCA()*/


/****************************************************************************/

/***************************************************************
 *
 * Procedure af_pse_delete_PK
 *
 ***************************************************************/
#ifdef __STDC__

RC af_pse_delete_PK(
	KeyType	     type,
	DName	    *name,
	DName	    *issuer,
	OctetString *serial
)

#else

RC af_pse_delete_PK(
	type,
	name,
	issuer,
	serial
)
KeyType	     type;
DName	    *name;
DName	    *issuer;
OctetString *serial;

#endif

{
	PKList  * list, * np, * ahead_np, * tmp_np;
	int	  found = 0;
	PSESel    pse;
	RC        rc;
	char	* proc = "af_pse_delete_PK";

	if ( ((type != ENCRYPTION) && (type != SIGNATURE)) || ((!serial || !issuer) && !name) ) {
		aux_add_error(EINVALID, "Invalid parameter", CNULL, 0, proc);
		return (-1);
	}

	if (!(list = af_pse_get_PKList(type)) ) {      /* List not found */

		aux_add_error(LASTERROR, "Can't delete key: PK/EK-List not found", CNULL, 0, proc);
		return (-1);
	}

	if (issuer) {
		for (np = list, ahead_np = (PKList *) 0; np; ahead_np = np, np = np->next) {
			if (!aux_cmp_DName(np->element->issuer, issuer) && !aux_cmp_OctetString(np->element->serialnumber, serial))
				break;
		}
		if (np) {      /* PK (to be deleted) found */
			if (!ahead_np) 
				list = np->next;     /* firstelement */
			else 
				ahead_np->next = np->next;    /* not first */
			np->next = (PKList *) 0;
			aux_free_PKList(&np);

			if ( !list ) {       /* last element deleted from list */
				pse.app_name = AF_pse.app_name;
				pse.pin      = AF_pse.pin;
				if ( type == ENCRYPTION )
					pse.object.name = EKList_name;
				else
					pse.object.name = PKList_name;
				pse.object.pin = getobjectpin(pse.object.name);
				rc = sec_delete(&pse);
			}
			else {
				rc = af_pse_update_PKList(type, list);
				aux_free_PKList(&list);
			}
			return(rc);
		} 
		else {      /* PK (to be deleted) not found */
			aux_free_PKList(&list);
			aux_add_error(ENOPK, "PK (to be deleted) not found in PK/EK-List", CNULL, 0, proc);
			return (-1);
		}
	} 
	else {   /*name is set*/
		np = list;
		ahead_np = (PKList *) 0;
		while (np) {
			if (!aux_cmp_DName(np->element->subject, name)) {
				if (!found) 
					found = 1;
				if (!ahead_np) 
					list = np->next;     /* firstelement */
				else 
					ahead_np->next = np->next;    /* not first */
				tmp_np = np->next;
				np->next = (PKList *) 0;
				aux_free_PKList(&np);
				np = tmp_np;
			} else {
				ahead_np = np;
				np = np->next;
			}
		}
		if (found) {
			if ( !list ) {       /* last element deleted from list */
				pse.app_name = AF_pse.app_name;
				pse.pin      = AF_pse.pin;
				if ( type == ENCRYPTION )
					pse.object.name = EKList_name;
				else
					pse.object.name = PKList_name;
				pse.object.pin = getobjectpin(pse.object.name);
				rc = sec_delete(&pse);
			}
			else {
				rc = af_pse_update_PKList(type, list);
				aux_free_PKList(&list);
			}
			return(rc);
		} else {
			aux_free_PKList(&list);
			aux_add_error(ENOPK, "PK (to be deleted) with this name not found in PK/EK-List", (char *) name, DName_n, proc);
			return (-1);
		}
	}

}  /*af_pse_delete_PK()*/


/****************************************************************************/

/***************************************************************
 *
 * Procedure af_pse_delete_PCA
 *
 ***************************************************************/
#ifdef __STDC__

RC af_pse_delete_PCA(
	DName	    *name
)

#else

RC af_pse_delete_PCA(
	name
)
DName	    *name;

#endif

{
	PKList  * list, * np, * ahead_np, * tmp_np;
	int	  found = 0;
	PSESel    pse;
	RC        rc;
	char	* proc = "af_pse_delete_PCA";

	if (!(list = af_pse_get_PCAList()) ) {      /* List not found */

		aux_add_error(LASTERROR, "Can't delete PCA: PCAList not found", CNULL, 0, proc);
		return (-1);
	}

	np = list;
	ahead_np = (PKList *) 0;
	while (np) {
		if (!aux_cmp_DName(np->element->subject, name)) {
			if (!found) 
				found = 1;
			if (!ahead_np) 
				list = np->next;     /* firstelement */
			else 
				ahead_np->next = np->next;    /* not first */
			tmp_np = np->next;
			np->next = (PKList *) 0;
			aux_free_PKList(&np);
			np = tmp_np;
		} else {
			ahead_np = np;
			np = np->next;
		}
	}
	if (found) {
		if ( !list ) {       /* last element deleted from list */
			pse.app_name = AF_pse.app_name;
			pse.pin      = AF_pse.pin;
			pse.object.name = PCAList_name;
			pse.object.pin = getobjectpin(pse.object.name);
			rc = sec_delete(&pse);
		}
		else {
			rc = af_pse_update_PCAList(list);
			aux_free_PKList(&list);
		}
		return(rc);
	} else {
		aux_free_PKList(&list);
		aux_add_error(ENOPK, "PCA (to be deleted) with this name not found in PCAList", (char *) name, DName_n, proc);
		return (-1);
	}


}  /*af_pse_delete_PCA()*/


/****************************************************************************/

/***************************************************************
 *
 * Procedure af_pse_add_CertificatePairSet
 *
 ***************************************************************/
#ifdef __STDC__

RC af_pse_add_CertificatePairSet(
	SET_OF_CertificatePair	 *cpairset
)

#else

RC af_pse_add_CertificatePairSet(
	cpairset
)
SET_OF_CertificatePair	 *cpairset;

#endif

{
	SET_OF_CertificatePair * newset,  * set, * np_arg, * np_pse, * newset_tmp;
	RC     			 rc;
	int			 error;
	char		       * proc = "af_pse_add_CertificatePairSet";

	rc = 0;
	newset = (SET_OF_CertificatePair * ) 0;
	newset_tmp = (SET_OF_CertificatePair * ) 0;
	set = af_pse_get_CertificatePairSet();

	if (!set) {
		if(aux_last_error() != EOBJNAME) {
			AUX_ADD_ERROR;
			return(-1);
		} else aux_free_error();
		rc = af_pse_update_CertificatePairSet(cpairset);
		return(rc);
	}

	for (np_arg = cpairset; np_arg; np_arg = np_arg->next) {
		for (np_pse = set; np_pse; np_pse = np_pse->next) {
			if (!aux_cmp_CertificatePair(np_arg->element, np_pse->element))
				break;
		}
		if (!np_pse) {   /* add new entry */
			if (!(newset = (SET_OF_CertificatePair * )malloc(sizeof(SET_OF_CertificatePair))) ) {
				aux_add_error(EMALLOC, "newset", CNULL, 0, proc);
				return( -1);
			}
			newset->element = np_arg->element;
			if (!newset_tmp)
				newset->next = set;
			else
				newset->next = newset_tmp;
			newset_tmp = newset;
		}
	}

	if (newset)
		rc = af_pse_update_CertificatePairSet(newset);
	aux_free_CertificatePairSet(&set);
	return(rc);

} /*af_pse_add_CertificatePairSet()*/


/****************************************************************************/


/***************************************************************
 *
 * Procedure af_pse_add_CRL
 *
 ***************************************************************/
#ifdef __STDC__

RC af_pse_add_CRL(
	Crl	 *crlpse
)

#else

RC af_pse_add_CRL(
	crlpse
)
Crl	 *crlpse;

#endif

{
	CrlSet     * newset, * set, * np, * ahead_np;
	RC           rc;
	char	   * proc = "af_pse_add_CRL";

	if (!(newset = (CrlSet * )malloc(sizeof(CrlSet))) ) {
		aux_add_error(EMALLOC, "newset", CNULL, 0, proc);
		return( -1);
	}
	newset->element = aux_cpy_Crl(crlpse);

	set = af_pse_get_CrlSet();

	if (!set) {
		if(aux_last_error() != EOBJNAME) {
			AUX_ADD_ERROR;
			return(-1);
		} else aux_free_error();
		newset->next = (CrlSet *)0;
		rc = af_pse_update_CrlSet(newset);
		aux_free_CrlSet(&newset);
		return(rc);
	}

	for (np = set, ahead_np = (CrlSet *) 0; np; ahead_np = np, np = np->next) {
		if (!aux_cmp_DName(crlpse->issuer, np->element->issuer))
			break;
	}

	if (np) {      /* overwrite obsolete entry */
		newset->next = np->next;  /* may be NULL pointer */
		np->next = (CrlSet *) 0;
		aux_free_CrlSet(&np);   /* delete obsolete entry */
		if (ahead_np) {     /* not first */
			ahead_np->next = newset;    
			rc = af_pse_update_CrlSet(set);
			aux_free_CrlSet(&set);
		}
		else {     /* first element to be overwritten */
			rc = af_pse_update_CrlSet(newset);
			aux_free_CrlSet(&newset);
		}
	} 
	else {   /* add new entry */
		newset->next = (CrlSet *)0;
		ahead_np->next = newset;
		rc = af_pse_update_CrlSet(set);
		aux_free_CrlSet(&set);
	}

	return(rc);

} /*af_pse_add_CRL()*/


/****************************************************************************/


/***************************************************************
 *
 * Procedure af_pse_delete_CRL
 *
 ***************************************************************/
#ifdef __STDC__

RC af_pse_delete_CRL(
	DName	 *issuer
)

#else

RC af_pse_delete_CRL(
	issuer
)
DName	 *issuer;

#endif

{
	CrlSet  * set, * np, * ahead_np;
	int	  found = 0;
	PSESel    pse_sel;
	RC        rc;
	char	* proc = "af_pse_delete_CRL";

	if (! issuer){
		aux_add_error(EINVALID, "Invalid parameter", CNULL, 0, proc);
		return (-1);
	}

	set = af_pse_get_CrlSet();
	if(! set) {
		if(aux_last_error() != EOBJNAME) AUX_ADD_ERROR;
		else aux_add_error(LASTERROR, "No set of revocation lists stored locally", CNULL, 0, proc);
		return (-1);
	}

	for (np = set, ahead_np = (CrlSet *) 0; np; ahead_np = np, np = np->next) {
		if (!aux_cmp_DName(np->element->issuer, issuer))
			break;
	}

	if (np) {      /* Revocation list (to be deleted) found */
		if (! ahead_np) 
			set = np->next;     /* firstelement */
		else 
			ahead_np->next = np->next;    /* not first */

		np->next = (CrlSet *) 0;
		aux_free_CrlSet(&np);

		if (! set) {       /* last element deleted from set */
			pse_sel.app_name = AF_pse.app_name;
			pse_sel.pin      = AF_pse.pin;
			pse_sel.object.name = CrlSet_name;
			pse_sel.object.pin = getobjectpin(pse_sel.object.name);
			rc = sec_delete(&pse_sel);
		}
		else {
			rc = af_pse_update_CrlSet(set);
			aux_free_CrlSet(&set);
		}
		return(rc);
	} 
	else {      /* Revocation list (to be deleted) not found */
		aux_free_CrlSet(&set);
		aux_add_error(ENOCRL, "Revocation list (to be deleted) not found", CNULL, 0, proc);
		return (-1);
	}


}  /*af_pse_delete_CRL()*/


/****************************************************************************/


/***************************************************************
 *
 * Procedure af_pse_exchange_PK
 *
 ***************************************************************/
#ifdef __STDC__

RC af_pse_exchange_PK(
	KeyType		  type,
	ToBeSigned	 *tbs
)

#else

RC af_pse_exchange_PK(
	type,
	tbs
)
KeyType		  type;
ToBeSigned	 *tbs;

#endif

{
	PKList  * list, *np;
	RC        rc;
	char	* proc = "af_pse_exchange_PK";

	if (((type != ENCRYPTION) && (type != SIGNATURE)) || !tbs) {
		/* public key identified by name, exchange by the new key pk */
		aux_add_error(EINVALID, "Invalid parameter", CNULL, 0, proc);
		return (-1);
	}

	if (!(list = af_pse_get_PKList(type))) {
		aux_add_error(LASTERROR, "Can't read EK/PK list to change key", CNULL, 0, proc);
		return (-1);
	}

	for (np = list; np; np = np->next) {
		if (!aux_cmp_DName(np->element->subject, tbs->subject) && !aux_cmp_DName(np->element->issuer, tbs->issuer)) 
			break;
	}

	if (np) {
		np->element = aux_cpy_ToBeSigned(tbs);
		rc = af_pse_update_PKList(type, list);
		aux_free_PKList(&list);
		return(rc);
	} else {         /* PK (to be changed) does not exist */
		aux_free_PKList(&list);
		aux_add_error(ENONAME, "PK (to be changed) does not exist", (char *) tbs->subject, DName_n, proc);
		return (-1);
	}

}  /*af_pse_exchange_PK()*/


/****************************************************************************/

/***************************************************************
 *
 * Procedure af_pse_get_owner
 *
 ***************************************************************/
#ifdef __STDC__

DName *af_pse_get_owner(
	KeyType	  type,
	KeyInfo	 *pk
)

#else

DName *af_pse_get_owner(
	type,
	pk
)
KeyType	  type;
KeyInfo	 *pk;

#endif

{
	PKList  * list;
	PKList  * np;
	char	* proc = "af_pse_get_owner";

	if ( ((type != ENCRYPTION) && (type != SIGNATURE)) || !pk) {
		aux_add_error(EINVALID, "Invalid parameter", CNULL, 0, proc);
		return (NULLDNAME);
	}

	if ( !(list = af_pse_get_PKList(type)) ) {
		aux_add_error(LASTERROR, "Can't read EK/PK list to get owner of key", CNULL, 0, proc);
		return (NULLDNAME);
	}

	for ( np = list; np; np = np->next ) {
		if ( aux_cmp_KeyInfo(np->element->subjectPK, pk) == 0 )
			break;
	}

	if (np)
		return(np->element->subject);

	aux_add_error(ENOPK, "Can't get owner of key: PK not found in list", CNULL, 0, proc);

	return (NULLDNAME);

}   /*af_pse_get_owner()*/


/****************************************************************************/

/***************************************************************
 *
 * Procedure af_pse_get_PK
 *
 ***************************************************************/
#ifdef __STDC__

KeyInfo *af_pse_get_PK(
	KeyType	     type,
	DName	    *subject,
	DName	    *issuer,
	OctetString *serial
)

#else

KeyInfo *af_pse_get_PK(
	type,
	subject,
	issuer,
	serial
)
KeyType	     type;
DName	    *subject;
DName	    *issuer;
OctetString *serial;

#endif

{
	PKList  * list;
	PKList  * np;
	char	* proc = "af_pse_get_PK";

	if ((type != ENCRYPTION && type != SIGNATURE) || (!subject && !issuer)) {
		aux_add_error(EINVALID, "Invalid parameter", CNULL, 0, proc);
		return ((KeyInfo * )0);
	}

	if ( !(list = af_pse_get_PKList(type)) ) {
		if(aux_last_error() != EOBJNAME) {
			AUX_ADD_ERROR;
		}
		else aux_free_error();

		return ((KeyInfo * )0);
	}

	for ( np = list; 
	    np; 
	    np = np->next
	    ) {
		if(issuer && !aux_cmp_DName(np->element->issuer, issuer)
		     && !aux_cmp_OctetString(np->element->serialnumber, serial) && !subject) break;

		if(issuer && !aux_cmp_DName(np->element->issuer, issuer)
		     && !aux_cmp_OctetString(np->element->serialnumber, serial) && subject && !aux_cmp_DName(np->element->subject, subject)) break;

		 if(!issuer && subject && !aux_cmp_DName(np->element->subject, subject))
			break;
	}

	if (np) return(np->element->subjectPK);

	aux_add_error(ENONAME, "Requested TBS not found in PKList/EKList", CNULL, 0, proc);

	return ((KeyInfo * )0);

}   /*af_pse_get_PK()*/


/****************************************************************************/

/***************************************************************
 *
 * Procedure af_pse_get_TBS
 *
 ***************************************************************/
#ifdef __STDC__

ToBeSigned *af_pse_get_TBS(
	KeyType	     type,
	DName	    *subject,
	DName	    *issuer,
	OctetString *serial
)

#else

ToBeSigned *af_pse_get_TBS(
	type,
	subject,
	issuer,
	serial
)
KeyType	     type;
DName	    *subject;
DName	    *issuer;
OctetString *serial;

#endif

{
	PKList  * list;
	PKList  * np;
	char	* proc = "af_pse_get_TBS";

	if ((type != ENCRYPTION && type != SIGNATURE) || (!subject && !issuer)) {
		aux_add_error(EINVALID, "Invalid parameter", CNULL, 0, proc);
		return ((ToBeSigned * )0);
	}

	if ( !(list = af_pse_get_PKList(type)) ) {
		if(aux_last_error() != EOBJNAME) AUX_ADD_ERROR;
		else aux_add_error(ENONAME, "Requested PK not found: there is no PKList/EKList", CNULL, 0, proc);

		return ((ToBeSigned * )0);
	}

	for ( np = list; 
	    np; 
	    np = np->next
	    ) {
		if(issuer && !aux_cmp_DName(np->element->issuer, issuer)
		     && !aux_cmp_OctetString(np->element->serialnumber, serial) && !subject) break;

		if(issuer && !aux_cmp_DName(np->element->issuer, issuer)
		     && !aux_cmp_OctetString(np->element->serialnumber, serial) && subject && !aux_cmp_DName(np->element->subject, subject)) break;

		 if(!issuer && subject && !aux_cmp_DName(np->element->subject, subject))
			break;
	}

	if (np)	return(np->element);

	aux_add_error(ENONAME, "Requested PK not found in PKList/EKList", CNULL, 0, proc);

	return ((ToBeSigned * )0);

}   /*af_pse_get_TBS()*/


/****************************************************************************/

/***************************************************************
 *
 * Procedure af_gen_key
 *
 ***************************************************************/
#ifdef __STDC__

RC af_gen_key(
	Key	 *key,
	KeyType	  ktype,
	Boolean	  replace
)

#else

RC af_gen_key(
	key,
	ktype,
	replace
)
Key	 *key;
KeyType	  ktype;
Boolean	  replace;

#endif

{
	KeyType          keytype;
	PSESel         * pse;
	Boolean          onekeypaironly = FALSE;

	int	          rc;
	Certificate     * newcert;
	FCPath          * fcpath;
	PKRoot          * pkroot;

	char	        * proc = "af_gen_key";


	if (!key || !key->key || (ktype != SIGNATURE && ktype != ENCRYPTION)) {
		aux_add_error(EINVALID, "Invalid parameter", CNULL, 0, proc);
		return - 1;
	}

	if ( (key->keyref != 0) || (key->pse_sel != (PSESel *) 0) ) {
		if (sec_gen_key(key, replace) < 0) {
			aux_add_error(LASTERROR, "Can't generate key", CNULL, 0, proc);
			return - 1;
		} else return 0;
	}


	/* generate permanent key */

	pse = (PSESel * )calloc( 1, sizeof(PSESel));
	if (!pse) {
		aux_add_error(EMALLOC, "pse", CNULL, 0, proc);
		return - 1;
	}
	pse->app_name = aux_cpy_String(AF_pse.app_name);
	pse->pin      = aux_cpy_String(AF_pse.pin);
	pse->app_id   = AF_pse.app_id;

	if(af_check_if_onekeypaironly(&onekeypaironly)){
		AUX_ADD_ERROR;
		return (- 1);
	}

	if(onekeypaironly == TRUE){
		pse->object.name = aux_cpy_String(SKnew_name);
		pse->object.pin = aux_cpy_String(getobjectpin(SKnew_name));
	}
	else{
		switch (ktype) {
		case ENCRYPTION:
			pse->object.name = aux_cpy_String(DecSKnew_name);
			pse->object.pin = aux_cpy_String(getobjectpin(DecSKnew_name));
			keytype = ENCRYPTION;
			break;
		case SIGNATURE:
			pse->object.name = aux_cpy_String(SignSK_name);
			pse->object.pin = aux_cpy_String(getobjectpin(SignSK_name));
			keytype = SIGNATURE;
			break;
		default:
			aux_add_error(EALGID, "Can't generate key because algid is unknown", CNULL, 0, proc);
                	aux_free_PSESel(&pse);
			return - 1;
		} /* switch */
	}

	key->pse_sel = pse;
	if (sec_gen_key(key, replace) < 0) {
		aux_add_error(LASTERROR, "Can't generate key", (char *) key->pse_sel, PSESel_n, proc);
                aux_free_PSESel(&pse);
		return - 1;
	}
        aux_free_PSESel(&pse);


	return (0);
}



/***************************************************************
 *
 * Procedure af_pse_get_signAI
 *
 ***************************************************************/
#ifdef __STDC__

AlgId *af_pse_get_signAI(
)

#else

AlgId *af_pse_get_signAI(
)

#endif

{
	PSESel	  pse;
	Key       skey;
        KeyInfo * keyinfo;
        KeyInfo * get_keyinfo_from_key();
        AlgId   * algid;
	Boolean   onekeypaironly = FALSE;

	char	* proc = "af_pse_get_signAI";

	skey.key = (KeyInfo *) 0;
	skey.keyref = 0;
	skey.pse_sel = &pse;
	pse.app_name = AF_pse.app_name;
	pse.pin      = AF_pse.pin;

	if(af_check_if_onekeypaironly(&onekeypaironly)){
		AUX_ADD_ERROR;
		return (NULLALGID);
	}

	if(onekeypaironly == TRUE){
		pse.object.name = SKnew_name;
		pse.object.pin = getobjectpin(SKnew_name);
	}
	else{
		pse.object.name = SignSK_name;
		pse.object.pin = getobjectpin(SignSK_name);
	}

        algid = NULLALGID;
        if((keyinfo = get_keyinfo_from_key(&skey))) {
                algid = aux_cpy_AlgId(keyinfo->subjectAI);
                aux_free_KeyInfo(&keyinfo);
        }

	return(algid);
}


/***************************************************************************************
 *                                     getobjectpin                                    *
 ***************************************************************************************/

/*
 *  given: object name from AF_pse
 *  returns: object pin from AF_pse
 */

/***************************************************************
 *
 * Procedure getobjectpin
 *
 ***************************************************************/
#ifdef __STDC__

char *getobjectpin(
	char	 *objectname
)

#else

char *getobjectpin(
	objectname
)
char	 *objectname;

#endif

{
	PSESel         * pse_sel;
	ObjId          * oid;
	register int	 i;
	char	       * proc = "getobjectpin";

	for (i = 0; i < PSE_MAXOBJ; i++)
		if (strcmp(objectname, AF_pse.object[i].name) == 0) {
			if (!AF_pse.object[i].pin) {
				oid = af_get_objoid(objectname);
				if(!(pse_sel = af_pse_open(oid, FALSE))) {
					aux_add_error(LASTERROR, "Can't get PIN of object: cannot open", CNULL, 0, proc);
					return (CNULL);
				}
				if (!pse_sel->object.pin) {
					aux_free_PSESel(&pse_sel);
					return (CNULL);
				}
				aux_free_PSESel(&pse_sel);
				return (AF_pse.object[i].pin);
			}
			else return(AF_pse.object[i].pin);
		}

	return (CNULL);
}


/***************************************************************************************
 *                                     setobjectpin                                    *
 ***************************************************************************************/

/*
 *  given: object name from AF_pse and new pin
    action: set new pin for object in AF_pse
 *  returns: 0 or 1
 */

/***************************************************************
 *
 * Procedure setobjectpin
 *
 ***************************************************************/
#ifdef __STDC__

RC setobjectpin(
	char	 *objectname,
	char	 *newpin
)

#else

RC setobjectpin(
	objectname,
	newpin
)
char	 *objectname;
char	 *newpin;

#endif

{
	register  int	i;
	char	* proc = "setobjectpin";

	for (i = 0; i < PSE_MAXOBJ; i++) {
		if (strcmp(objectname, AF_pse.object[i].name) == 0) {
			AF_pse.object[i].pin = (char *)malloc(strlen(newpin) + 1);
			if (!AF_pse.object[i].pin ) {
				aux_add_error(EMALLOC, "AF_pse.object[i].pin", CNULL, 0, proc);
				return(1);
			}
			strcpy(AF_pse.object[i].pin, newpin);
			return(0);
		}
	}
	return(1);
}



/*******************************************************************************
 *                         af_check_if_onekeypaironly                          *
 *******************************************************************************/

/***************************************************************
 *
 * Procedure af_check_if_onekeypaironly
 *
 ***************************************************************/
#ifdef __STDC__

RC af_check_if_onekeypaironly(
	Boolean	 *onekeypaironly
)

#else

RC af_check_if_onekeypaironly(
	onekeypaironly
)
Boolean	 *onekeypaironly;

#endif

{
	PSEToc  	* psetoc = (PSEToc * )0;
	PSESel          * psesel;
	PSELocation       pse_location;
	char            * proc = "af_check_if_onekeypaironly";


	if(! onekeypaironly){
		aux_add_error(EINVALID, "No parameter provided", CNULL, 0, proc);
		return(- 1);
	}

	if(!(psesel = af_pse_open(NULLOBJID, FALSE))) {
		AUX_ADD_ERROR;
		return(- 1);
	}

#ifdef SCA
	/* If SC available, try to read SC Toc */

	if((pse_location = sec_psetest(psesel->app_name)) == ERR_in_psetest) {
		if (aux_last_error() == EDEVLOCK) 
			aux_add_error(EDEVLOCK, "Cannot open device for SCT (No such device or device busy).", CNULL, 0, proc);
		else
			AUX_ADD_ERROR;
		aux_free_PSESel(&psesel);
		return (-1);
	}

	if(pse_location == SCpse) {
		psetoc = chk_SCToc(psesel);
	}
#endif

	/* If no SCToc available, try to read SW-PSE Toc  */

	if (!(psetoc)) {
		if (!(psetoc = chk_toc(psesel, FALSE))) {
		        aux_add_error(ETOC, "Can't read Toc of PSE", (char *) psesel, PSESel_n, proc);
			aux_free_PSESel(&psesel);
			return(- 1);
		}
	}
	aux_free_PSESel(&psesel);


	if(psetoc->status & ONEKEYPAIRONLY) 
		* onekeypaironly = TRUE;
	else 
		* onekeypaironly = FALSE;

	return(0);
}

/***************************************************************
 *
 * Procedure af_pse_incr_serial
 *
 ***************************************************************/
#ifdef __STDC__

OctetString * af_pse_incr_serial(
)

#else

OctetString * af_pse_incr_serial(
)

#endif

{
	OctetString   * serial, * tmp_ostr;
	int 		i, a;
	RC		rc;
	char          * proc = "af_pse_incr_serial";

	/* increments serial number and returns new value */

	serial = af_pse_get_SerialNumber();
	if(!serial){
		if(aux_last_error() != EOBJNAME) AUX_ADD_ERROR;
		else aux_add_error(LASTERROR, "Can't get Serial Number", CNULL, 0, proc);
		return(NULLOCTETSTRING);
	}
	if(!((a = ++(serial->octets[serial->noctets - 1])) % 256)){
		for(i = serial->noctets - 2; i>=0 && !((a = ++(serial->octets[i])) % 256); i--)
		;
		if(i < 0){
			tmp_ostr = aux_new_OctetString(1);
			tmp_ostr->octets[0] = 1;
			rc = aux_append_OctetString(tmp_ostr, serial);
			if(rc) aux_add_error(LASTERROR, "Can't append OctetString", (char *) tmp_ostr, OctetString_n, proc);
			if(serial) aux_free_OctetString(&serial);
			serial = tmp_ostr;
		}
	}
	if(af_pse_update_SerialNumber(serial) < 0){
		aux_add_error(LASTERROR, "Can't update Serial Number", CNULL, 0, proc);
		return(NULLOCTETSTRING);
	}

	return(serial);
}



