#include <stdio.h>
#include "af.h"

#ifdef __STDC__

KeyInfo *sec_DH_phase1(Key *key_pg, Key *key_x, Boolean  with_pg);
BitString *sec_DH_phase2(Key *key_pg, Key *key_x, KeyInfo *peer_y);

#else

KeyInfo *sec_DH_phase1();
BitString *sec_DH_phase2();

#endif

/***************************************************************
 *
 * Procedure main
 *
 ***************************************************************/
int main()
{
	KeyInfo *public1 = 0, *public2 = 0;
	BitString	*bstr1, *bstr2;
	PSESel std_pse;
	PSESel std_pse_x1;
	PSESel std_pse_x2;
	char *psename=0, *cadir=0, *psepath, *ppin;

	Key key_x1, key_x2, key_pg;




	if (!psename) {
		if(cadir) {
			psename = getenv("CAPSE");
			if(!psename) psename = DEF_CAPSE;
		}
		else {
			psename = getenv("PSE");
			if(!psename) psename = DEF_PSE;
		}
	}

        if(cadir) {
                psepath = (char *)malloc(strlen(cadir)+strlen(psename)+2);
		if( !psepath ) {
			aux_fprint_error(stderr, sec_verbose);
			exit(-1);
		}
                strcpy(psepath, cadir);
                if(psepath[strlen(psepath) - 1] != '/') strcat(psepath, "/");
                strcat(psepath, psename);
        }
        else {
                psepath = (char *)malloc(strlen(psename)+2);
		if( !psepath ) {
			aux_fprint_error(stderr, sec_verbose);
			exit(-1);
		}
                strcpy(psepath, psename);
        }


	if(cadir)
		ppin = getenv("CAPIN");
	else
		ppin = getenv("USERPIN");

	if ( aux_create_AFPSESel(psepath, ppin) < 0 ) {
		aux_fprint_error(stderr, sec_verbose);
		exit(-1);
	}


        std_pse.app_name = aux_cpy_String(AF_pse.app_name);
        std_pse.object.name = "DHa";
        std_pse.object.pin = aux_cpy_String(AF_pse.pin);
        std_pse.pin = aux_cpy_String(AF_pse.pin);
	std_pse.app_id = AF_pse.app_id;

        std_pse_x1.app_name = aux_cpy_String(AF_pse.app_name);
        std_pse_x1.object.name = "DHx1";
        std_pse_x1.object.pin = aux_cpy_String(AF_pse.pin);
        std_pse_x1.pin = aux_cpy_String(AF_pse.pin);
	std_pse_x1.app_id = AF_pse.app_id;

        std_pse_x2.app_name = aux_cpy_String(AF_pse.app_name);
        std_pse_x2.object.name = "DHx2";
        std_pse_x2.object.pin = aux_cpy_String(AF_pse.pin);
        std_pse_x2.pin = aux_cpy_String(AF_pse.pin);
	std_pse_x2.app_id = AF_pse.app_id;




	key_pg.pse_sel = &std_pse;
	key_x1.pse_sel = &std_pse_x1;
	key_x2.pse_sel = &std_pse_x2;




	sec_gen_verbose = 1;
	sec_verbose = 1;




	sec_DH_init(&key_pg, 1, 0);
	aux_fprint_error(stderr, 2);fprintf(stderr,"\n");aux_free_error();

	public1 = sec_DH_phase1(&key_pg, &key_x1, 1);
	aux_fprint_error(stderr, 2);fprintf(stderr,"\n");aux_free_error();


	public2 = sec_DH_phase1(&key_pg, &key_x2, 1);
	aux_fprint_error(stderr, 2);fprintf(stderr,"\n");aux_free_error();
	bstr2 =  sec_DH_phase2(&key_pg, &key_x2, public1);
	aux_fprint_error(stderr, 2);fprintf(stderr,"\n");aux_free_error();


	bstr1 =  sec_DH_phase2(&key_pg, &key_x1, public2);
	aux_fprint_error(stderr, 2);fprintf(stderr,"\n");aux_free_error();

	aux_fprint_BitString(stderr, bstr1);
	aux_fprint_BitString(stderr, bstr2);

	aux_fprint_error(stderr, 2);fprintf(stderr,"\n");aux_free_error();




	key_pg.pse_sel = 0;
	key_x1.pse_sel = 0;
	key_x2.pse_sel = 0;

	key_pg.keyref = -1;
	key_x1.keyref = -1;
	key_x2.keyref = -1;

	sec_DH_init(&key_pg, 2, 0);
	aux_fprint_error(stderr, 2);fprintf(stderr,"\n");aux_free_error();

	public1 = sec_DH_phase1(&key_pg, &key_x1, 1);
	aux_fprint_error(stderr, 2);fprintf(stderr,"\n");aux_free_error();


	public2 = sec_DH_phase1(&key_pg, &key_x2, 1);
	aux_fprint_error(stderr, 2);fprintf(stderr,"\n");aux_free_error();
	bstr2 =  sec_DH_phase2(&key_pg, &key_x2, public1);
	aux_fprint_error(stderr, 2);fprintf(stderr,"\n");aux_free_error();


	bstr1 =  sec_DH_phase2(&key_pg, &key_x1, public2);
	aux_fprint_error(stderr, 2);fprintf(stderr,"\n");aux_free_error();

	aux_fprint_BitString(stderr, bstr1);
	aux_fprint_BitString(stderr, bstr2);

	aux_fprint_error(stderr, 2);fprintf(stderr,"\n");aux_free_error();


	key_pg.keyref = 0;
	key_x1.keyref = 0;
	key_x2.keyref = 0;

	sec_DH_init(&key_pg, 30, 10);
	aux_fprint_error(stderr, 2);fprintf(stderr,"\n");aux_free_error();

	public1 = sec_DH_phase1(&key_pg, &key_x1, 1);
	aux_fprint_error(stderr, 2);fprintf(stderr,"\n");aux_free_error();


	public2 = sec_DH_phase1(&key_pg, &key_x2, 1);
	aux_fprint_error(stderr, 2);fprintf(stderr,"\n");aux_free_error();
	bstr2 =  sec_DH_phase2(&key_pg, &key_x2, public1);
	aux_fprint_error(stderr, 2);fprintf(stderr,"\n");aux_free_error();


	bstr1 =  sec_DH_phase2(&key_pg, &key_x1, public2);
	aux_fprint_error(stderr, 2);fprintf(stderr,"\n");aux_free_error();

	aux_fprint_BitString(stderr, bstr1);
	aux_fprint_BitString(stderr, bstr2);

	aux_fprint_error(stderr, 2);fprintf(stderr,"\n");aux_free_error();

}
