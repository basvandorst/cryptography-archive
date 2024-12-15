/*********************************************************************
 * Filename:      verify.c
 * Description:   
 * Author:        Tage Stabell-Kulo <tage@acm.org>
 * Modified at:   Mon Jan 19 10:11:59 1998
 * Modified by:   Tage Stabell-Kuloe <tage@acm.org>
 * 	
 * 	Copyright (c) University of Tromsø, Norway
 * 	See the file COPYING for details
 ********************************************************************/

#include <assert.h>
#include <stdlib.h>

#include <pgplib.h>
#include "shell.h"

void
verify (int argc, char *argv[])
{

    PGPKEYID	key_id;
    keyent_t	**contents;
    PGPpubkey_t	*keyp, *tmpkey;
    PGPuserid_t	*uid = NULL;
    PGPsig_t	*sig = NULL;
    PGPuserid_t	**tmpuidp;
    int		index;
    int		have_key, sig_ok;
    /* scratch */
    int		i;
    char *what;
    PGPKEYID	key_id2;

    if ( argc != 2 ) {
	fprintf(stderr, "VERIFY KeyID 0x1234567890123456\n");
	return;
    }

    if ( ! PGPKEYID_FromAscii(argv[1],&key_id)) 
    {
	fprintf(stderr, "VERIFY 0x1234567890123456\n");
	return;
    }

    i = get_keycontents(key_id, &contents);
    switch (i) {
    case  0: break;
    case  1: fprintf(stderr, "No such key\n"); 
	return;
    case -1: fprintf(stderr, "Some error getting key contents\n"); 
	return;
    default: assert(0);
    }

    /* **************************************************************
     * We walk through the contents, we pick up the key on the way (it
     * is supposted to be the first item).
     * ************************************************************** */

    keyp = NULL;
    have_key = 0;
    for(index=0; contents[index] != NULL; index++) {
	switch (contents[index]->what) {
	default :	/* ignore unknown packets.*/
	    break;
	case TRUSTBYTE:
	    fprintf(stdout,"\t  trust level: ");
	    switch(contents[index]->u.trust->byte & 3)
	    {
	    case 0:
		fprintf(stdout, "undefined\n");
		break;
	    case 1:
		fprintf(stdout, "untrusted\n");
		break;
	    case 2:
		fprintf(stdout, "marginal\n");
		break;
	    case 3:
		fprintf(stdout, "complete\n");
		break;
	    }
	    

	    if(!uid) {
		if(contents[index]->u.trust->byte & (1 << 5))
		    fprintf(stdout, "\t DISABLED.\n");
		
		if(contents[index]->u.trust->byte & (1 << 7))
		    fprintf(stdout, "\t own key\n");
	    }
	    break;

		
	case PUBKEY:
	    if ( have_key == 1 ) {
		PGPKEYID_FromBN(&key_id2, keyp->N);
		fprintf(stderr, "Found key %s, have %s\n",
			PGPKEYID_ToStr(&(contents[index]->u.sig->key_id)), 
			PGPKEYID_ToStr(&key_id2));
	    }
	    have_key = 1;
	    keyp = contents[index]->u.key;
	    uid = NULL;
	    sig = NULL;
	    break;

	case UID:
	    uid = contents[index]->u.uid;
	    sig = NULL;
	    fprintf(stdout, "\tUID: %s\n", uid->name);
	    break;

	case SIG: 
	    
	    sig = contents[index]->u.sig;

	    if((i = get_only_pubkey(sig->key_id, &tmpkey)) == -1)
	    {
		fprintf(stderr, "Error getting pubkey: %s\n",
			pgplib_strerror(pgplib_errno));
		break;
	    }
	    if ( i == 1 ) {
		fprintf(stdout, "\t\tSig: Unknown key, can not verify\n");
		break;
	    }
	    sig_ok = 0; 
	    
	    i = verify_keysig(keyp, uid, sig, tmpkey);

	    free_pubkey(tmpkey);
	    free(tmpkey);

	    if ( i == -1 ) {
		    fprintf(stderr, "Error veryfiyng signature: %s\n",
			    pgplib_strerror(pgplib_errno));

	    } else if ( i == 1 ) {
		fprintf(stdout, "Didn't verify OK\n");
		continue;
	    }

	    switch(sig->classification)
	    {
	    case K0_SIGNATURE_BYTE:
	    case K1_SIGNATURE_BYTE:
	    case K2_SIGNATURE_BYTE:
	    case K3_SIGNATURE_BYTE:
		what = "Sig";
		break;
	    case KC_SIGNATURE_BYTE:
		what = "Compromise";
		break;
	    case KR_SIGNATURE_BYTE:
		what = "ID/signature revocation";
		break;
	    default:
		what = "Unkn.";
	    }
		

	    fprintf(stdout, "\t\t%s with %s OK\n",
		    what, PGPKEYID_ToStr(&sig->key_id));

	    if(sig->classification != KC_SIGNATURE_BYTE 
	       && sig->classification != KR_SIGNATURE_BYTE 
	       && is_compromised(sig->key_id))
		fprintf(stdout, "\t\t*** KEY IS COMPROMISED ***\n");

	    i = uids_from_keyid(sig->key_id, 
				&tmpuidp);
	    if ( i == -1 ) {
		fprintf(stderr, "error:%s\n", pgplib_strerror(pgplib_errno));
		return;
	    }
	    if ( i == 1 )
		fprintf(stdout, "\t\t\tNo matching UID\n");
	    else {
		assert(i==0);
		for(i=0; tmpuidp[i] != NULL; i++) {
		    fprintf(stdout, "\t\t\t%s\n", tmpuidp[i]->name);
		    free_uid(tmpuidp[i]);
		}
		free(tmpuidp);
	    }

	    break;
	} /* switch */
    } /* for (we have gone through the entire thing */
    free_keyrecords(contents);
}
