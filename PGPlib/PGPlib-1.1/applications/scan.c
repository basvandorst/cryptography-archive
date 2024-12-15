/*********************************************************************
 * Filename:      scan.c
 * Description:
 * Author:        Tage Stabell-Kulo <tage@acm.org>
 * Modified at:   Mon Jan 19 10:10:40 1998
 * Modified by:   Tage Stabell-Kuloe <tage@acm.org>
 *
 * 	Copyright (c) University of Tromsø, Norway
 * 	See the file COPYING for details
 ********************************************************************/

#include <assert.h>
#include <ctype.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>

#include <pgplib.h>

#include "shell.h"

void
scan(int argc, char *argv[])
{

    int		fid;
    char	*p;
    int		armor;
    u_char	*packet;
    int		p_len;
    FILE	*file;

    int		long_keys, short_keys, long_name;

    PGPsig_t	sig;
    PGPpubkey_t	key;
    PGPuserid_t	uid;
    PGPliteral_t literal;
    int		how_much;
    int		have_key;
    u_char	CTB;

    /* scratch */
    int		i;
    PGPKEYID 	key_id;


    if ( argc > 3 || argc < 2) {
	fprintf(stderr, "SCAN file [armor] .  Will scan the file\n");
	return;
    }

    armor = 0;
    if ( argc == 3 ) {
	for(p=argv[2]; *p; p++)
	    *p = tolower(*p);
	if ( strcmp(argv[2], "armor")) {
	    fprintf(stderr, "SCAN: arg 3 MUST be \"armor\" (or \"ARMOR\") or nothing (not \"%s\")\n",
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
	fprintf(stderr, "Canr fdopen file\n");
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
	/* We have to read from here and until EOF storing the
	 * result in a new file (to get rid of the CTB).
	 * Then we uncompreee and reopen.
	 */
	FILE    *tmp1;

        fprintf(stdout, "File is compressed.  Decompressing....\n");
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

    how_much = 0;
    packet = NULL;
    long_keys = short_keys = long_name = 0;
    while ( 1 ) {

	/* malloc_dump(2);*/

	free(packet);
	packet = NULL;
	i = read_packet(file, &packet);
	switch( i ) {
	case -1:
	    free(packet);
	    fprintf(stderr, "Some error\n");
	    return;
	case 1:
	    free(packet);
	    if ( feof(file) ) {
		fclose(file);
	    }
	    else
		fprintf(stderr, "Syntax error in the file\n");
	    fprintf(stderr, "Lange=%d korte=%d\n", long_keys, short_keys);
	    fprintf(stderr, "%d har navn > 50 bytes\n", long_name);
	    return;
	case 0:
	    break;
	default:
	    assert(0);
	}
	/* We found a package.  We consumed j bytes */
	p_len = packet_length(packet);
	how_much += p_len;

 	CTB = (packet[0] & CTB_TYPE_MASK) >> 2;

	switch ( CTB ) {

	case CTB_COMMENT_TYPE:
	case CTB_CKE_TYPE:
	case CTB_COMPRESSED_TYPE:
	    fprintf(stderr, "CTB = %x\n", packet[0]);
	    assert(0);

	case CTB_CERT_SECKEY_TYPE: {
	    u_short	us;

	    memcpy(&us, packet+11, 2);
	    us = ntohs(us);
	    fprintf(stdout, "%d Secret key (length=%d)!! \n", how_much, us);
	    continue;
	}
 	case CTB_KEYCTRL_TYPE:
	    fprintf(stdout, "%d Key Control Packet\n", how_much);
	    continue;
	case CTB_CERT_PUBKEY_TYPE:
	    have_key = 1;
	    i = buf2pubkey(packet, p_len, &key);
	    PGPKEYID_FromBN(&key_id, key.N);
	    fprintf(stdout, "%d Key: %s\n", how_much,
		    PGPKEYID_ToStr(&key_id));
	    free_pubkey(&key);
	    continue;
 	case CTB_USERID_TYPE:
	    i = buf2uid(packet, p_len, &uid);
	    fprintf(stdout, "%d UserID: %s\n", how_much, uid.name);
	    if ( strlen(uid.name) > 50)
		long_name += 1;
	    free_uid(&uid);
	    continue;

	case CTB_SKE_TYPE: {
	    /* a local variable */
	    PGPuserid_t	**uidp;

	    (void)buf2signature(packet, p_len, &sig);
	    fprintf(stdout, "%d Signature KeyID:\t%s\n",
		    how_much, PGPKEYID_ToStr(&sig.key_id));
	    i = uids_from_keyid(sig.key_id, &uidp);
	    if ( i == 1 || i == -1 )
		fprintf(stdout, "\tUnknown\n");
	    else {
		for(i = 0; uidp[i] != 0; i++) {
		    fprintf(stdout, "\t%s\n", uidp[i]->name);
		    free_uid(uidp[i]);
		}
		free(uidp);
	    }
	    (void) free_signature(&sig);
	    continue;
	}
	default:
	    fprintf(stderr, "%d Unknown CTB=%d\n", how_much, CTB);
	    assert(0);
	}
    } /* while */

    return;
}

