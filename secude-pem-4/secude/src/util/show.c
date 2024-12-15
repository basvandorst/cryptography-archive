/*
 *  SecuDE Release 4.3 (GMD)
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

#include <stdio.h>
#include "af.h"

int             verbose = 0;
static void     usage();


/***************************************************************
 *
 * Procedure main
 *
 ***************************************************************/
#ifdef __STDC__

int main(
	int	  cnt,
	char	**parm
)

#else

int main(
	cnt,
	parm
)
int	  cnt;
char	**parm;

#endif

{
        ObjId 	      		  object_oid;
        DName 	     		* dname;
        Name         		* name;
        OctetString  		* ostr = NULLOCTETSTRING, *object, * tmp_ostr, * serial;
        Certificate  		* certificate;
	Certificates 		* certificates;
        KeyInfo      		* keyinfo;
	KeyBits      		* keybits;
	BitString     		  bitstring, *bstr;
        SET_OF_Certificate 	* certset;
        FCPath       		* fcpath;
        PKRoot       		* pkroot;
        PKList       		* pklist;
	CRL      		* crl;
	AliasList   		* aliaslist;
	CrlSet			* crlset;
        SET_OF_CertificatePair  * cpairset;
        int 			  i;
	extern char		* optarg;
	extern int		  optind, opterr;
	char	        	* cmd = *parm, opt;
	char                    * signatureTimeDate, * file = CNULL;
	Name		        * signer;
	Boolean			  authenticated;

	char 			* proc = "main (show)";



        print_cert_flag = TBS | ALG | SIGNAT;
        print_keyinfo_flag = ALGID | BSTR | KEYBITS;
	af_use_alias = FALSE;

	optind = 1;
	opterr = 0;

	MF_check = FALSE;

nextopt:
	while ( (opt = getopt(cnt, parm, "hvzVW")) != -1 ) { 
		switch(opt) {
		case 'v':
			verbose = 1;
			continue;
		case 'V':
			verbose = 2;
			continue;
		case 'W':
			verbose = 2;
			af_verbose = TRUE;
			sec_verbose = TRUE;
			continue;
		case 'z':
			MF_check = TRUE;
			continue;
		case 'h':
			usage(LONG_HELP);
			continue;
		default:
		case '?':
			usage(SHORT_HELP);
		}
	}


        if(optind < cnt) {
		if(ostr){
			fprintf(stderr, "Input-file already specified!\n");
			exit(- 1);
		}
		file = parm[optind];
		ostr = aux_file2OctetString(file);
		optind++;
		goto nextopt;
	}

	if(! ostr) ostr = aux_file2OctetString(0);

        if(!ostr) {
        	fprintf(stderr, "Can't read file\n");
		aux_add_error(EINVALID, "Can't read", 0, 0, proc);
		aux_fprint_error(stderr, verbose);
		exit(-1);
	}


	bitstring.nbits = ostr->noctets * 8;
	bitstring.bits = ostr->octets;
	bstr = &bitstring;

        if((object = d_PSEObject(&object_oid, ostr))) { 
                /*
                 * PSE object of the form SEQUENCE { 
                 *                            objectType  OBJECT IDENTIFIER, 
                 *                            objectValue ANY DEFINED BY objectType
                 *                        }
                 */

                if(aux_cmp_ObjId(&object_oid, SignSK_OID) == 0
                   || aux_cmp_ObjId(&object_oid, DecSKnew_OID) == 0 
                   || aux_cmp_ObjId(&object_oid, DecSKold_OID) == 0
                   || aux_cmp_ObjId(&object_oid, SKnew_OID) == 0 
                   || aux_cmp_ObjId(&object_oid, SKold_OID) == 0) {
                        fprintf(stderr, "PSE object %s:\n", aux_ObjId2PSEObjectName(&object_oid));
                        if(!(keyinfo = d_KeyInfo(object))) goto decodeerr;
                        print_keyinfo_flag |= SK;
                        aux_fprint_KeyInfo(stderr, keyinfo);                                      
                }
                else if(aux_cmp_ObjId(&object_oid, Name_OID) == 0) {
                        fprintf(stderr, "PSE object %s:\n", aux_ObjId2PSEObjectName(&object_oid));
                        if(!(dname = d_DName(object))) goto decodeerr;
        		if(!(name = aux_DName2Name(dname))) {
  			      	fprintf(stderr, "Can't build printable repr. of dname\n");
 				aux_add_error(EINVALID, "Can't build printable repr. of dname", 0, 0, proc);
				aux_fprint_error(stderr, verbose);
        			exit(-1);
        		}
                        fprintf(stderr, "%s\n", name);
                }
                else if(aux_cmp_ObjId(&object_oid, SignCert_OID) == 0
                   || aux_cmp_ObjId(&object_oid, EncCert_OID) == 0
		   || aux_cmp_ObjId(&object_oid, Cert_OID) == 0) { 
                        fprintf(stderr, "PSE object %s:\n", aux_ObjId2PSEObjectName(&object_oid));
                        if(!(certificate = d_Certificate(object))) goto decodeerr;
                        print_keyinfo_flag |= PK;
                        aux_fprint_Certificate(stderr, certificate);
                }
                else if(aux_cmp_ObjId(&object_oid, SignCSet_OID) == 0
                   || aux_cmp_ObjId(&object_oid, EncCSet_OID) == 0
                   || aux_cmp_ObjId(&object_oid, CSet_OID) == 0) { 
                        fprintf(stderr, "PSE object %s:\n", aux_ObjId2PSEObjectName(&object_oid));
                        if(!(certset = d_CertificateSet(object))) goto decodeerr;
                        print_keyinfo_flag |= PK;
                        aux_fprint_CertificateSet(stderr, certset);
                }
                else if(aux_cmp_ObjId(&object_oid, FCPath_OID) == 0) {
                        fprintf(stderr, "PSE object %s:\n", aux_ObjId2PSEObjectName(&object_oid));
                        if(!(fcpath = d_FCPath(object))) goto decodeerr;
                        print_keyinfo_flag |= PK;
                        aux_fprint_FCPath(stderr, fcpath);
                }
                else if(aux_cmp_ObjId(&object_oid, PKRoot_OID) == 0) {
                        fprintf(stderr, "PSE object %s:\n", aux_ObjId2PSEObjectName(&object_oid));
                        if(!(pkroot = d_PKRoot(object))) goto decodeerr;
                        print_keyinfo_flag |= PK;
                        aux_fprint_PKRoot(stderr, pkroot);
                }
                else if(aux_cmp_ObjId(&object_oid, SerialNumber_OID) == 0) {
                        fprintf(stderr, "PSE object %s:\n", aux_ObjId2PSEObjectName(&object_oid));
                        if(!(serial = d_OctetString(object))) goto decodeerr;
                        aux_fprint_Serialnumber(stderr, serial);
                }
                else if(aux_cmp_ObjId(&object_oid, PKList_OID) == 0
                   || aux_cmp_ObjId(&object_oid, EKList_OID) == 0) { 
                        fprintf(stderr, "PSE object %s:\n", aux_ObjId2PSEObjectName(&object_oid));
                        if(!(pklist = d_PKList(object))) goto decodeerr;
                        print_keyinfo_flag |= PK;
                        aux_fprint_PKList(stderr, pklist);
                }
                else if(aux_cmp_ObjId(&object_oid, CrossCSet_OID) == 0) {
                        fprintf(stderr, "PSE object %s:\n", aux_ObjId2PSEObjectName(&object_oid));
                        if(!(cpairset = d_CertificatePairSet(object))) goto decodeerr;
                        print_keyinfo_flag |= PK;
                        aux_fprint_CertificatePairSet(stderr, cpairset);
                }
                else if(aux_cmp_ObjId(&object_oid, CrlSet_OID) == 0) {
                        fprintf(stderr, "PSE object %s:\n", aux_ObjId2PSEObjectName(&object_oid));
                        if(!(crlset = d_CrlSet(object))) goto decodeerr;
                        aux_fprint_CrlSet(stderr, crlset);
                }
                else {
                        printf("Object OID { ");
                        for(i = 0; i < object_oid.oid_nelem; i++) {
             	                printf("%d ", object_oid.oid_elements[i]);
                        }
                        printf(" }\n");
                        aux_xdump(object->octets, object->noctets, 0);
                }
        }
        else {
                if((keyinfo = d_KeyInfo(ostr))) {
                        fprintf(stderr, "KeyInfo:\n");
                        aux_fprint_KeyInfo(stderr, keyinfo);
                }
                else if((keybits = d_KeyBits(bstr))) {
                        fprintf(stderr, "KeyBits:\n");
                        aux_fprint_KeyBits(stderr, keybits);
                }
                else if((dname = d_DName(ostr))) {
        		if(!(name = aux_DName2Name(dname)))  {
  			      	fprintf(stderr, "Can't build printable repr. of dname\n");
 				aux_add_error(EINVALID, "Can't build printable repr. of dname", 0, 0, proc);
				aux_fprint_error(stderr, verbose);
        			exit(-1);
        		}
                        fprintf(stderr, "DName: ");
                        fprintf(stderr, "%s\n", name);
                }
                else if((certificate = d_Certificate(ostr))) {
                        fprintf(stderr, "Certificate:\n");
                        aux_fprint_Certificate(stderr, certificate);
                }
                else if((certificates = d_Certificates(ostr))) {
                        fprintf(stderr, "Certificates:\n");
                        aux_fprint_Certificates(stderr, certificates);
                }
                else if((certset = d_CertificateSet(ostr))) {
                        fprintf(stderr, "SET_OF_Certificate:\n");
                        aux_fprint_CertificateSet(stderr, certset);
                }
                else if((fcpath = d_FCPath(ostr))) {
                        fprintf(stderr, "FCPath:\n");
                        aux_fprint_FCPath(stderr, fcpath);
                }
                else if((pkroot = d_PKRoot(ostr))) {
                        fprintf(stderr, "PKRoot:\n");
                        aux_fprint_PKRoot(stderr, pkroot);
                }
                else if((serial = d_OctetString(ostr))) {
                        fprintf(stderr, "SerialNumber:\n");
                        aux_fprint_Serialnumber(stderr, serial);
                }
                else if((pklist = d_PKList(ostr))) {
                        fprintf(stderr, "PKList or EKList:\n");
                        aux_fprint_PKList(stderr, pklist);
                }
                else if((cpairset = d_CertificatePairSet(ostr))) {
                        fprintf(stderr, "SET_OF_CertificatePair:\n");
                        aux_fprint_CertificatePairSet(stderr, cpairset);
                }
                else if((crl = d_CRL(ostr))) {
                        fprintf(stderr, "Revocation List (PEM syntax):\n");
                        aux_fprint_CRL(stderr, crl);
                }
                else if((crlset = d_CrlSet(ostr))) {
                        fprintf(stderr, "Set of locally stored Revocation Lists:\n");
                        aux_fprint_CrlSet(stderr, crlset);
                }
                else if((aliaslist = d_AliasList(ostr))) {
        		if (file) {
				tmp_ostr = af_SignedFile2OctetString(file);
				if(! tmp_ostr && aux_last_error() != EVERIFICATION){
					aux_fprint_error(stderr, verbose);
					fprintf(stderr, "WARNING: The following AliasList has not been proven authentic!!!");
				}
				else{
					if(verbose) aux_fprint_VerificationResult(stderr, verifresult);
					signer = aux_determine_Signer(verifresult, &authenticated);
					if(! signer){
						  fprintf(stderr, "Cannot determine entity which signed AliasList.\n");
						exit(-1);
					}
					if(authenticated == TRUE){
						fprintf(stderr, "AliasList signed by <%s>", signer);
						if(sec_SignatureTimeDate) {
							signatureTimeDate = aux_readable_UTCTime(sec_SignatureTimeDate);
							fprintf(stderr, " at %s", signatureTimeDate);
							free(sec_SignatureTimeDate);
							free(signatureTimeDate);
						}
					}
					else{
						fprintf(stderr, "WARNING: Signature applied to AliasList has not been proven authentic\n");
						fprintf(stderr, "(its purported author is <%s>)", signer);
					}
				}
			}
			else fprintf(stderr, "WARNING: AliasList NOT SIGNED!");
			fprintf(stderr, "\n\n");
			fprintf(stderr, "AliasList:\n");
                        aux_fprint_AliasList(stderr, aliaslist);
                }
                else {
			for(i = 0; i < ostr->noctets; i++) {
				if(!isascii(ostr->octets[i])) {
                        		fprintf(stderr, "Unknown object:\n");
                        		aux_xdump(ostr->octets, ostr->noctets, 0);
					exit(0);
				}
			}
                	fprintf(stderr, "ASCII Text:\n");
			ostr->octets[ostr->noctets] = '\0';
			fprintf(stderr, "%s\n", ostr->octets);
                }
        }
        exit(0);
decodeerr:
	fprintf(stderr, "Can't decode objectValue\n");
  	aux_add_error(EINVALID, "Can't decode objectValue", 0, 0, proc);
	aux_fprint_error(stderr, verbose);
        exit(-1);
}



/***************************************************************
 *
 * Procedure usage
 *
 ***************************************************************/
#ifdef __STDC__

static void usage(
	int	  help
)

#else

static void usage(
	help
)
int	  help;

#endif

{
	aux_fprint_version(stderr);

        fprintf(stderr, "show  Show ASN.1-coded SecuDE Object in Suitable Form\n\n");
        fprintf(stderr, "usage:\n\n");
	fprintf(stderr,"show [-hvzVW] [file (containing ASN.1 code)]\n\n");
 

        if(help == LONG_HELP) {
        	fprintf(stderr, "with:\n\n");
        	fprintf(stderr, "-h               write this help text\n");
        	fprintf(stderr, "-v               verbose\n");
        	fprintf(stderr, "-V               Verbose\n");
        	fprintf(stderr, "-W               Grand Verbose (for testing only)\n");
		fprintf(stderr, "-z               control malloc/free behaviour\n");
        }


        exit(-1);                                /* IRREGULAR EXIT FROM SHOW */
}
