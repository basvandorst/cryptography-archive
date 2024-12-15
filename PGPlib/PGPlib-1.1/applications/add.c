/*********************************************************************
 * Filename:      add.c
 * Description:
 * Author:        Tage Stabell-Kulo <tage@acm.org>
 * Modified at:   Thu Dec 11 19:53:42 1997
 * Modified by:   Tage Stabell-Kuloe <tage@acm.org>
 *
 * 	Copyright University of Tromsø, Norway
 * 	See the file COPYING for details
 ********************************************************************/

#include <assert.h>
#include <ctype.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <pgplib.h>
#include "shell.h"


#define STATISTICS

void
goodbye(void)
{
    keydb_end();
    exit(1);
}

void
add(int argc, char *argv[])
{
    int		fid;
    char	*p;
    int		armor;
    FILE	*file;
    u_char	CTB;
    u_char	*packet;
    int		p_len = 0;

    PGPsig_t	sig;
    PGPpubkey_t	key;
    PGPuserid_t	uid;
    PGPliteral_t literal;
    PGPtrust_t	trust;
    int		how_much;
    int		have_key;
    int		have_uid;
    int		have_sig;

    PGPKEYID    key_id;

#ifdef STATISTICS
    int		packet_sofar, sig_sofar, key_sofar, uid_sofar;
#endif
    /* scratch */
    int		i;

    if ( argc > 3 || argc < 2) {
	fprintf(stderr, "ADD file [armor] .  Will add things to a database\n");
	return;
    }
    if ( dbase_open == 0 ) {
	fprintf(stderr, "No database is open.\n");
	return;
    }
    armor = 0;
    if ( argc == 3 ) {
	for(p=argv[2]; *p; p++)
	    *p = tolower(*p);
	if ( strcmp(argv[2], "armor")) {
	    fprintf(stderr, "ADD: arg 3 MUST be \"armor\" (or \"ARMOR\") or nothing (not \"%s\")\n",
		    argv[2]);
	    return;
	}
	else {
	    armor = 1;
	}
    }

    if ( armor ) {

	FILE	*tmp1, *tmp2;

	tmp1 = fopen(argv[1], "r");
	if ( tmp1 == NULL ) {
	    fprintf(stderr, "Can't open %s\n", argv[1]);
	    return;
	}
	fid = mkstemp("/tmp/temp.XXXXXX");
	tmp2 = fdopen(fid, "w+");
	if ( tmp2 == NULL ) {
	    fclose(tmp1);
	    fprintf(stderr, "Error in creating tmp-file\n");
	    return;
	}
	fprintf(stdout, "DeArmoring....\n");
	i = dearmor_file(tmp1, tmp2);
	if ( i != 0 ) {
	    fclose(tmp1);
	    fclose(tmp2);
	    fprintf(stderr, "Could not dearmor file %s\n", argv[1]);
	    return;
	}
	fclose(tmp2);
	fprintf(stdout, "....Done.\n");

    }
    else { /* not armored */
	fid = open(argv[1], O_RDONLY);
    }
    if ( fid < 0 ) {
	perror("open");
	return;
    }
    file = fdopen(fid, "r");
    if ( file == NULL ) {
	fprintf(stderr, "Can't fdopen file\n");
	return;
    }

    /* Turn it into a file we can handle */
more:
    i  = fgetc(file);
    if ( i == EOF ) {
	fprintf(stderr, "EOF reached\n");
	fclose(file);
	return;
    }
    CTB = (u_char) i;

    switch ( (CTB & CTB_TYPE_MASK) >> 2) {
    case CTB_SKE_TYPE:
    case CTB_CERT_PUBKEY_TYPE:
    case CTB_KEYCTRL_TYPE:
    case CTB_USERID_TYPE:
    case CTB_COMMENT_TYPE:
	ungetc(CTB, file);
	break;
    case CTB_COMPRESSED_TYPE: {
	FILE    *tmp1;

	fprintf(stdout, "File is compressed.  Decompressing....");
	tmp1 = tmpfile();
        i = decompress_file(file, tmp1);
        if ( i != 0 ) {
            fprintf(stderr, "Compressed, but it wouldn't decompress\n");
            fclose(tmp1);
            return;
        }

	fprintf(stdout, "...Done\n");
	goto more;
    }
    case CTB_LITERAL_TYPE:
	fprintf(stdout, "File is literal.  Unpacking....\n");
	i = file2literal(file, &literal);
	if ( i == -1 ) {
	    fprintf(stderr, "First package was literal, but would't unpack\n");
	    fclose(file);
	    return;
	}
	fprintf(stdout, "Literal info:\n");
	fprintf(stdout, "\tMode:\t\t%d\n", literal.mode);
	fprintf(stdout, "\tFilename:\t%s\n", literal.filename);
	fprintf(stdout, "\tTimestamp:\t%ld\n", literal.timestamp);
	fprintf(stdout, "\tLength:\t\t%d\n", literal.datalen);

	goto more;

    case CTB_CKE_TYPE:
	fprintf(stdout, "Encrypted data.\n");
	fclose(file);
	return;
    default:
	fprintf(stderr, "File contains packet of unknown type\n");
	fclose(file);
	return;
    }

#ifdef STATISTICS
    packet_sofar = sig_sofar = uid_sofar = key_sofar = 0;
#endif

    how_much = 0;
    packet = NULL;
    have_key = 0;
    have_uid = 0;
    have_sig = 0;

    while ( 1 ) {

#ifdef STATISTICS
	if ( packet_sofar % 500 == 0 )
	    fprintf(stderr, "%d: key=%d sig=%d uid=%d\n",
		    packet_sofar, key_sofar, sig_sofar, uid_sofar);
	packet_sofar += 1;

	/*if ( packet_sofar % 1000 == 0 ) {
	    keydb_sync();
	}*/
#endif

	i = read_packet(file, &packet);
	switch( i ) {
	case -1:
	    fprintf(stderr, "Some error in read packet\n");
	    keydb_end();
	    return;
	case 1:
	    if ( feof(file) ) {
		fclose(file);
	    }
	    else
		fprintf(stderr, "Syntax error in the file\n");
	    return;
	case 0:
	    /* We found a package.  We consumed p_len bytes */
	    p_len = packet_length(packet);
	    how_much += p_len;
	    break;
	default:
	    assert(0);
	}

 	CTB = (packet[0] & CTB_TYPE_MASK) >> 2;

	switch ( CTB ) {

	case CTB_COMMENT_TYPE:
	case CTB_CKE_TYPE:
	case CTB_COMPRESSED_TYPE:
	    fprintf(stderr, "CTB = %x\n", packet[0]);
	    assert(0);

	case CTB_CERT_SECKEY_TYPE:
	    fprintf(stdout, "%d Secret key !! \n", how_much);
	    free(packet);
	    continue;

 	case CTB_KEYCTRL_TYPE:
	    fprintf(stdout, "Key Control Packet\n");
	    buf2trust(packet, packet_length(packet), &trust);
	    PGPKEYID_FromBN(&key_id,key.N);
	    if(have_key && !have_sig)
		add_trust(key_id, have_uid ? &uid : NULL, &trust);
	    free(packet);
	    continue;

	case CTB_CERT_PUBKEY_TYPE:
#ifdef STATISTICS
	    key_sofar += 1;
#endif
	    if (have_key == 1)
		free_pubkey(&key);
	    else
		have_key = 1;

	    if(have_uid) {
		free_uid(&uid);
		have_uid = 0;
	    }

	    have_sig = 0;

	    i = buf2pubkey(packet, p_len, &key);
	    PGPKEYID_FromBN(&key_id,key.N);
	    fprintf(stdout, "Storing Key: %s\n",PGPKEYID_ToStr(&key_id));
	    i = store_pubkey(&key, INSERT);
	    switch (i) {
	    case -1:
		fprintf(stderr, "\tSome error storing key %s\n",
			PGPKEYID_ToStr(&key_id));
		goodbye();
	    case 1:
		fprintf(stdout, "\tKey exists, can-not store %s\n",
			PGPKEYID_ToStr(&key_id));
		break;
	    case 0:
		break;
	    default:
		assert(0);
	    }
	    free(packet);
	    continue;

 	case CTB_USERID_TYPE:
#ifdef STATISTICS
	    uid_sofar += 1;
#endif

	    have_sig = 0;

	    if(have_uid)
		free_uid(&uid);

	    memset(&uid, 0, sizeof(uid));
	    i = buf2uid(packet, p_len, &uid);
	    if ( i != 0 ) {
		fprintf(stderr, "%s\n", pgplib_errlist[pgplib_errno]);
		exit(1);
	    }
	    fprintf(stdout, "Adding UserID: %s\n", uid.name);
	    PGPKEYID_FromBN(&key_id,key.N);
	    i = add_uid(key_id,&uid);
	    if ( i != 0 ) {
		if ( i == 1 )
		    fprintf(stdout,
			    "Found an identical ID to %s on %s\n",
			    uid.name, PGPKEYID_ToStr(&key_id));
		else {
		    fprintf(stderr,
			    "\tError while adding userID \"%s\" to key %s\n",
			    	uid.name, PGPKEYID_ToStr(&key_id));
		    goodbye();
		}
	    }
	    free(packet);
	    have_uid = 1;
	    continue;

	case CTB_SKE_TYPE: {
	    /* a local variable */
	    PGPuserid_t	**uidp;

#ifdef STATISTICS
	    sig_sofar += 1;
#endif
	    
	    have_sig = 1;
	    i = buf2signature(packet, p_len, &sig);
	    if ( i == -1 ) {
		fprintf(stderr, "Could not ad sig\n");
		goodbye();
	    }
	    fprintf(stdout, "Adding Signature: %s\n", 
	    			PGPKEYID_ToStr(&sig.key_id));
	    i = uids_from_keyid(sig.key_id, &uidp);
	    if ( i == -1 )
	    {
		fprintf(stderr, "Got -1 from uids_from_keyid\n");
		goodbye();
	    }
	    if ( i == 1)
		fprintf(stdout, "\t\tUnknown\n");
	    else {
		for(i = 0; uidp[i] != NULL; i++) {
		    fprintf(stdout, "\t\t%s\n", uidp[i]->name);
		    free_uid(uidp[i]);
		}
		free(uidp);
	    }
	    PGPKEYID_FromBN(&key_id,key.N);
	    if(have_uid)
		i = add_sig(key_id, &uid, &sig);
	    else
		i = add_sig(key_id, NULL, &sig);
	    if ( i != 0 ) {
		fprintf(stdout, "\tCouldn't add sig %s to key %s\n",
			PGPKEYID_ToStr(&sig.key_id), 
			PGPKEYID_ToStr(&key_id));
		if ( i == 1 ) {
		    fprintf(stdout, "\t\tSignature already there\n");
		} else {
		    fprintf(stderr, "\t\tError\n");
		    goodbye();
		}
	    }
	    (void) free_signature(&sig);
	    free(packet);
	    continue;
	}
	default:
	    fprintf(stderr, "%d Unknown CTB=%d\n", how_much, CTB);
	    assert(0);
	}
    } /* while */

    return;
}
