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

#define NL '\012'
#define ALL 6
#define ENC 5
#define TIMELEN 40

#include "af.h"
#include "cadb.h"
#ifndef MAC
#include <sys/types.h>
#include <sys/stat.h>
#else
#include <unix.h>
#include <console.h>
#include "Mac.h"
#endif
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>


/*
 *    Be careful when changing  enum {...} commands  and struct {...} cmds[]:
 *    They must be in the same order. commands is the index of cmds[].
 */

enum {
       ADDALIAS, ADDEK, ADDPCA, ADDPK, ALGS, ALIAS2DNAME, ALIASES,
#ifdef X500
       AUTHNONE, AUTHSIMPLE,
#ifdef STRONG
       AUTHSTRONG,
#endif
#endif
       CACRL, CAPRINTLOG, CASERIALNUMBERS, CAUSERS, CERTIFY,
       CERT2KEYINFO, CERT2PKROOT, CHECK, CHPIN, CLOSE, CREATE, DELALIAS, DELEK, DELETE,
       DELKEY, DELPCA, DELCRL, DELPK, DH1, DH2, DHINIT, DNAME2ALIAS,
#ifdef SCA
       EJECT,
#endif
       ENDE, ENTER, ERROR, EXIT, GENKEY, HELPCMD, INITCRL, ISSUEDCERTIFICATE, KEYTOC,
       MFLIST,
       OPEN, OWNER, PROLONG, PROTOTYPE, QM, QUIT, READ, REMOVE,
       RENAME, RESETERROR, RETRIEVE, REVOKE, SETPARM, SETSERIAL, SHOW,
       SPLIT, STRING2KEY, TOC,
#ifdef SCA
       TOGGLE,
#endif
       TRUSTPATH, VERBOSE, VERIFY, WRITE, XDUMP
} commands;

struct {
        char *cmd;
        char *parms;
        char *text;
        char *longtext;
} cmds[] = {
{ "addalias",
        "dirname=<username> aliasfile=<source> alias=<local alias> rfcmail=<RFC mail address>",
        "Add alias entry with distinguished name <username> to alias file",
        "" } ,
{ "addek",
        "<certificate>",
        "The ToBeSigned part of <certificate> is added to EKList",
        "" } ,
{ "addpca",
        "<certificate> ",
        "The ToBeSigned part of <certificate> is added to PCAList",
        "" } ,
{ "addpk",
        "<certificate> ",
        "The ToBeSigned part of <certificate> is added to PKList",
        "" } ,
{ "algs",
        "<algname> or <algtype>",
        "Show parameters of given algorithm or algorithm type",
        "" } ,
{ "alias2dname",
        "<alias>",
        "Search alias and print corresponding DName",
        "" } ,
{ "aliases",
        "<pattern>",
        "Search aliases containing <pattern> and print corresponding DName",
        "" } ,
#ifdef X500
{ "authnone",
        "", 
        "Bind to X.500 Directory without using any authentication", 
        "" } ,
{ "authsimple",
        "", 
        "Use simple DSA authentication ", 
        "" } ,
#ifdef STRONG
{ "authstrong",
        "", 
        "Use strong DSA authentication ", 
        "" } ,
#endif
#endif
{ "cacrl",
        "",
        "CA-cmd: List all CRLs stored in local database",
        "" } ,
{ "calog",
        "",
        "CA-cmd: Show CA log-file",
        "" } ,
{ "caserialnumbers",    
        "<name>", 
        "CA-cmd: Show all serialnumbers and dates of issue of certificates issued for user <name>",
        "" } ,
{ "causers",
        "", 
        "CA-cmd: List all users who have been certified", 
        "" } ,
{ "certify",
        "<certificate>", 
        "CA-cmd: Certify the public key contained in <certificate>", 
        "" } ,
{ "cert2keyinfo",
        "<certificate> <object or keyref>", 
        "Take public key from certificate and store it as KeyInfo in object or under keyref",
        "" } ,
{ "cert2pkroot",
        "<certificate> <object>", 
        "Take public key from certificate and store it as PKRoot in <object>",
        "" } ,
{ "check",
        "", 
        "Check content of PSE for consistency",
        "" } ,
{ "chpin",
        "", 
        "Change all PINs for PSE",
        "" } ,
{ "close",   
        "", 
        "Close PSE",
        "" } ,
{ "create",
        "<object>", 
        "Create <object> on PSE",
        "" } ,
{ "delalias",
        "<alias>", 
        "Remove alias <alias> from alias file",
        "" } ,
{ "delek",
        "<issuer> <serialnumber> or <subject>",
        "The ToBeSigned identified by either <issuer> and <serialnumber> or <subject> is deleted from EKList",
        "" } ,
{ "delete",
        "<object>", 
        "Delete PSE or <object> on PSE",
        "" } ,
{ "delkey",
        "<keyref>", 
        "Remove key with given <keyref>",
        "" } ,
{ "delpca",
        "<issuer> <serialnumber> or <subject>",
        "The ToBeSigned identified by either <issuer> and <serialnumber> or <subject> is deleted from PCAList",
        "" } ,
{ "delcrl",
        "<issuer>",
        "Remove revoction list of <issuer> from set of locally stored revocation lists",
        "" } ,
{ "delpk",
        "<issuer> <serialnumber> or <subject>",
        "The ToBeSigned identified by either <issuer> and <serialnumber> or <subject> is deleted from PKList",
        "" } ,
{ "dh1",
        "(\"temp\" <size> <pr_val_lgth> | \"peer\" <peers_public_y> | \"def\" <with_pg>) <own_private_key> <own_public_key>",
        "Phase 1 of DH key agreement generates <own_private_key> and writes the public key to file <own_public_key>",
        "The DH Parameter could be generated temporary with prime modulus length <size> and <pr_val_lgth>,\nit could be extracted from file <peers_public_y> or it could be got from default PSE object.\nIf <with_pg> is \"common\" the AlgId dhWithCommonModulus is used."
 } ,
{ "dh2",
        "<private_key> <peers_public_key> agreed_key>",
        "Phase 2 of DH key agreement reads <peers_public_key> from file and creates <agreed_key> with <private_key>",
        "" } ,
{ "dhinit",
        "<size> <private_value_length>",
        "The PSE object for DH Parameter is generated with modulus length <size> and <private_value_length>",
        "" } ,
{ "dname2alias",
        "<pattern>",
        "Search DName containing <pattern> and print corresponding alias names",
        "" } ,
#ifdef SCA
{ "eject",
        "<sct-id>",
        "Eject smartcard <sct-id>. If sct-id is 0 or omitted, eject all smartcards",
        "" } ,
#endif
{ "end",
        "",
        "Exit program",
        "" } ,
{ "enter",
        "attrtype=<attrtype> keytype=<keytype>", 
        "Enter security attribute into Directory",
        "" } ,
{ "error",
        "", 
        "print error stack",
        "" } ,
{ "exit",
        "",
        "Exit program",
        "" } ,
{ "genkey",
        "<algname> <object or keyref>", 
        "Generate key and store in <object> or under <keyref>",
        "" } ,
{ "helpcmd",
        "<cmd>",
        "Show helptext for <cmd>",
        "" } ,
{ "initcrl",
        "",
        "CA-cmd: Create an empty CRL",
        "" } ,
{ "issuedcertificate",
        "<serial>",
        "CA-cmd: Show issued certificate with serial number <serial>",
        "" } ,
{ "keytoc",  
        "", 
        "Show table of contents (toc) of all keys (keyref's)",
        "" } ,
{ "mflist",
        "<cmd>",
        "Show list of malloc'd addresses",
        "" } ,
{ "open",    
        "<pse>", 
        "Open PSE",
        "" } ,
{ "owner",    
        "", 
        "Display name of PSE owner",
        "" } ,
{ "prolong",    
        "", 
        "CA-cmd: Prolong the validity of the own CRL",
        "" } ,
{ "prototype",    
        "", 
        "Create a self-signed prototype-certificate of the own public signature key",
        "" } ,
{ "?",
        "<cmd>", 
        "show helptext for cmd",
        "" } ,
{ "quit",
        "",
        "Exit program",
        "" } ,
{ "read",
        "<object> <destination>", 
        "Read <object> into file <destination>",
        "" } ,
{ "remove",
        "attrtype=<attrtype> keytype=<keytype> cert=<serial,issuer> for=<serial,issuer> rev=<serial,issuer> replace=<TRUE/FALSE>", 
        "Remove security attribute from own directory entry",
        "" } ,
{ "rename",
        "<object> <newname>", 
        "Rename <object> to <newname>",
        "" } ,
{ "reseterror",
        "", 
        "free error stack",
        "" } ,
{ "retrieve",
        "dirname=<dirname> attrtype=<attrtype> keytype=<keytype> update=<TRUE/FALSE>", 
        "Retrieve security attribute from directory entry identified by <dirname>",
        "" } ,
{ "revoke",
        "",
        "Revoke one or more certificates",
        "" } ,
{ "setparm",
        "<algname>",
        "Set parameters of algorithm <algname>",
        "" } ,
{ "setserial",
        "",
        "Set SerialNumber to new value (CA only)",
        "" } ,
{ "show",
        "<object or keyref>",
        "Show object or keyref in suitable form",
        "" } ,
{ "split",
        "for=<serial,issuer> rev=<serial,issuer>",
        "Split a Cross Certificate Pair into its components",
        "" } ,
{ "string2key", 
        "<string>", 
        "Generate DES key from string and store in object or under keyref",
        "" } ,
{ "toc",
        "", 
        "Show table of contents (toc) of PSE)", 
        "" } ,
#ifdef SCA
{ "toggle",
        "", 
        "Toggle verification/encryption tool from SC to SW and vice versa", 
        "" } ,
#endif
{ "trustpath",
        "", 
        "Show owner of the PSE and certification path up to the Root-CA", 
        "" } ,
{ "verbose",
        "",
        "Change verbose level",
        "" } ,
{ "verify",
        "certificate=<cert> fcpath=<fcpath> pkroot=<pkroot>",
        "Verify digital signatures",
        "" } ,
{ "write",
        "<object> <source>", 
        "Write object from file", 
        "" } ,
{ "xdump",
        "<object or keyref>", 
        "xdump object or keyref",
        "" } ,
{ CNULL }
};


int     cmd;
Boolean replace;
char    inp[256];
char    * cmdname, * helpname, * filename, * pin, * newpin, * algname, * objtype, * attrname;
CertificateType certtype;
KeyRef  keyref;
PSESel  std_pse;
Key     *key, *publickey, *secretkey;
PSELocation pse_location;


static ack();
static psesel();
static getsize();
static filen();
static keytype();
static printfile();
static new_pin();
static str2key();
static helpcmd();
static store_objpin();
static Key *build_key();
static char *getalgname();
static DName *getdname();
static Name *getname();
static Name *getalias();
static OctetString *getserial();
static char *getattrtype();
static AliasFile getaliasfile();
static Key *object();
static char *nxtpar();
static char *strmtch();
static int check_if_number();
static off_t fsize();
static CertificatePair * specify_CertificatePair();
static incorrectName();
static incorrectSerialNumber();
static CertificatePair *compose_CertificatePair();
static RC store_certificate();

int             verbose = 0;
static void     usage();


time_t time();
char *sec_read_pin();
char *gets(), *getenv();
OctetString *aux_file2OctetString();
UTCTime *get_nextUpdate();
Boolean interactive = TRUE;
char *pname, *ppin;
int pkeyref;

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
	char			* proc = "main (psemaint)";
	char			* newstring;
	extern char		* optarg;
	extern int		  optind, opterr;
        int             	  fd1, fd2, fdin;
        int             	  i, anz, n, k, algtype, found;
        unsigned int              opt;
        time_t          	  atime, etime;
	char	        	  x500 = TRUE;
	char	        	  calogfile[256];
        Boolean         	  update, create;
        char            	* enc_file, *plain_file, *tbs_file, *sign_file, *hash_file;
        char            	* buf1, *buf2, *ii, *xx;
        char            	* par, *dd, *ptr, *cc, *afname, *newname, *attrtype, *number, *objname;
	char	        	* pgm_name = *parm;
	char            	* psename = CNULL, *cadir = CNULL, *home, * notbefore = CNULL, * notafter = CNULL;
        RC              	  rcode_dir, rcode_afdb, rcode;
        OctetString     	  octetstring, *ostr, *objectvalue, *tmp_ostr;
        ObjId    		  objecttype, object_oid, *oid;
	AlgId           	* algid;
        BitString       	  bitstring, *bstr;
        HashInput       	  hashinput;
        KeyInfo         	  tmpkey, * keyinfo = (KeyInfo *)0, * signpk, * encpk;
        FCPath          	* fcpath = (FCPath *)0;
        PKList          	* pklist = (PKList *)0;
        PKRoot          	* pkroot = (PKRoot *)0;
        Certificate     	* certificate = (Certificate *)0, * fcpath_cert;
        Certificates    	* certs = (Certificates *)0;
	ToBeSigned 		* tbs;
        SET_OF_Certificate 	* certset, * soc, * tmp_soc;
	CRLEntry     		* crlentry;
	CertificatePair 	* cpair;
	SET_OF_CertificatePair  * cpairset;
	CRLWithCertificates         * crlwithcerts;
	SET_OF_CRLWithCertificates  * setofcrlwithcerts;
	SEQUENCE_OF_CRLEntry  * crlentryseq;
	Crl			* crlpse;
	CrlSet      		* crlset;
	CRL			* crl;
        Name            	* name, * alias, * issuer, * subject;
	DName			* dname, * issuer_dn = NULLDNAME, * subject_dn = NULLDNAME, * own_dname, * signsubject, * encsubject;
        EncryptedKey    	  encryptedkey;
        rsa_parm_type   	* rsaparm;
	KeyType         	  ktype;
        AlgEnc          	  algenc;
        PSESel          	* pse_sel, *psesel_zw;
	PSEToc          	* psetoc, * sctoc;
	struct PSE_Objects 	* pseobj;
	OctetString		* serial;
	SET_OF_IssuedCertificate *isscertset;
	SET_OF_Name		* nameset;
	UTCTime 		* lastUpdate, * nextUpdate;
	AlgList         	* a;
	AlgId           	* algorithm = DEF_ISSUER_ALGID;
	Name			* printrepr;
	Boolean         	  onekeypaironly = FALSE;
	Boolean			  script = FALSE;
	Boolean			  retvalue;
	char			* outtext;
	char			  puff[1024];
	FILE		        * keyboard;
	AliasFile		  aliasfile;
	AliasList               * aliaslist;
	int			  cmd_count = 1;
	char			  answ[8];
	int			  dh_keysize, dh_privatevalue;
	Boolean			  dh_with_pg;
	Key			 *dh_private;
        KeyInfo         	 *dh_own_public_y, *dh_peer_public_y;
        OctetString         	 *dh_own_public_y_encoded, *dh_peer_public_y_encoded, dh_agreed_key_ostr;
	BitString		 *dh_agreed_key;
	ObjId			 *dh_agreed_key_oid;
	AlgId			 *dh_parm;

#ifdef AFDBFILE
	char		 	  afdb[256];
#endif
#ifdef X500
	char	        * env_af_dir_authlevel;
#endif
	
	logfile = (FILE * )0;

/*
 *      get args
 */

	optind = 1;
	opterr = 0;

	MF_check = FALSE;
	af_access_directory = FALSE;
	chk_PEM_subordination = TRUE;

#ifdef X500
	af_dir_authlevel = DBA_AUTH_SIMPLE;
#endif

#ifdef X500
	while ( (opt = getopt(cnt, parm, "a:i:c:p:y:d:t:f:l:A:hzvCFRDOTVW")) != -1 ) {
#else
	while ( (opt = getopt(cnt, parm, "a:i:c:p:y:f:l:hzvCFRDOTVW")) != -1 ) {
#endif
		switch(opt) {

		case 'z':
			MF_check = TRUE;
			break;
		case 'y':
			sec_debug = atoi(optarg);
			break;
                case 'i':
                        if((fdin = open(optarg, O_RDONLY)) < 0) {
                                fprintf(stderr, "Can't open %s \n", optarg);
                                exit(1);
                        }
                        close(0);
                        dup(fdin);
                        close(fdin);
			script = TRUE;
                        continue;
                case 'c':
                        cadir = optarg;
                        continue;
		case 'a':
                        oid = aux_Name2ObjId(optarg);
                        if (aux_ObjId2AlgType(oid) != SIG) usage(SHORT_HELP);
			algorithm = aux_ObjId2AlgId(oid);
			continue;
		case 'f':
			if (notbefore) usage(SHORT_HELP);
			else notbefore = optarg;
			continue;
		case 'l':
			if (notafter) usage(SHORT_HELP);
			else notafter = optarg;
			continue;
                case 'R':
                        af_chk_crl = TRUE;
                        continue;
                case 'p':
                        psename = optarg;
                        continue;
                case 'F':
                        af_FCPath_is_trusted = TRUE;
                        continue;
                case 'O':
                        chk_PEM_subordination = FALSE;
                        continue;
#ifdef X500
		case 'A':
			if (! strcasecmp(optarg, "STRONG"))
				af_dir_authlevel = DBA_AUTH_STRONG;
			else if (! strcasecmp(optarg, "SIMPLE"))
				af_dir_authlevel = DBA_AUTH_SIMPLE;
			break;
		case 'd':
			af_dir_dsaname = aux_cpy_String(optarg);
			continue;
		case 't':
			af_dir_tailor = aux_cpy_String(optarg);
			continue;
#endif
		case 'D':
			af_access_directory = TRUE;
			continue;
#ifdef SCA
		case 'T':
			SC_verify = TRUE;
			SC_encrypt = TRUE;
			continue;
#endif
                case 'C':
                        strcpy(inp, "helpcmd");
                        continue;
		case 'v':
			verbose = 1;
			continue;
		case 'V':
			af_verbose = TRUE;
			verbose = 2;
			continue;
		case 'W':
			verbose = 2;
			af_verbose = TRUE;
			sec_verbose = TRUE;
			continue;
                case 'X':
 		 	random_from_pse = TRUE;
                        break;
                case 'Y':
 			random_init_from_tty = TRUE;
                        break;
		case 'h':
			usage(LONG_HELP);
			continue;
		default:
		case '?':
			usage(SHORT_HELP);
		}
	}                    

	while (optind < cnt) {
                if(strlen(inp)) strcat(inp, " ");
                strcat(inp, parm[optind++]);
                interactive = FALSE;
        }

	aux_set_pse(psename, cadir);

        if(strncmp(inp, "helpcmd", 4) == 0) {
                par = nxtpar(CNULL);
                helpcmd();
                exit(0);
        }

        ii = inp;


#ifdef X500
	if (af_dir_authlevel == DBA_AUTH_NONE) {
		env_af_dir_authlevel = getenv("AUTHLEVEL");
		if (env_af_dir_authlevel) {
			if (! strcasecmp(env_af_dir_authlevel, "STRONG"))
				af_dir_authlevel = DBA_AUTH_STRONG;
			else if (! strcasecmp(env_af_dir_authlevel, "SIMPLE"))
				af_dir_authlevel = DBA_AUTH_SIMPLE;
		}
	}
#endif

        if(strncmp(inp, "create", 5)) {
        	if(!(pse_sel = af_pse_open(NULLOBJID, FALSE))) {
			aux_fprint_error(stderr, verbose);
			exit(-1);
        	}

		if(af_check_if_onekeypaironly(&onekeypaironly)){
			aux_fprint_error(stderr, verbose);
			exit(- 1);
		}

                aux_free_PSESel(&pse_sel);
        }

        std_pse.app_name = aux_cpy_String(AF_pse.app_name);
        std_pse.object.name = CNULL;
        std_pse.object.pin = aux_cpy_String(AF_pse.pin);
        std_pse.pin = aux_cpy_String(AF_pse.pin);
	std_pse.app_id = AF_pse.app_id;

#ifdef AFDBFILE
	/* Determine whether X.500 directory shall be accessed */
	strcpy(afdb, AFDBFILE);         /* file = .af-db/ */
	strcat(afdb, X500_name);           /* file = .af-db/'X500' */
	if (open(afdb, O_RDONLY) < 0) 
		x500 = FALSE;
#endif

	if (af_access_directory == TRUE) directory_user_dname = af_pse_get_Name();

        if(strlen(inp)) goto entr;

        while(1) {
		if(key) aux_free_Key(&key);
		if(certificate) aux_free_Certificate(&certificate);
		if(pkroot) aux_free_PKRoot(&pkroot);
		if(fcpath) aux_free_FCPath(&fcpath);
		if(certs) aux_free_Certificates(&certs);
		if(keyinfo) aux_free_KeyInfo(&keyinfo);
                if(interactive == FALSE) {
			exit(rcode);
		}
		if(script == FALSE) {
			if(AF_pse.app_name) fprintf(stderr, "PSE %s> ", AF_pse.app_name);
			else fprintf(stderr, "psemaint> ");
		}
                std_pse.object.name = CNULL;
                if(!gets(inp)) exit(0);
entr:
                ii = inp;

		if(script == TRUE && ii && strlen(ii)){
			fprintf(stderr, "%d. cmd: <%s>\n", cmd_count, ii);
			cmd_count++;
		}

                if(!(par = nxtpar(CNULL))) continue;

                anz = 0;
                for(i = 0; cmds[i].cmd; i++) {
                        if(!strncmp(cmds[i].cmd, par, strlen(par))) {
                                cmd = i;
                                cmdname = cmds[i].cmd;
                                anz++;
                        }
                }
		if(par) free(par);
                if(anz > 1) {
                        fprintf(stderr, "Ambiguous cmd\n");
                        rcode = 1;
                        continue;
                }
                if(anz == 0) {
                        fprintf(stderr, "unknown cmd\n");
                        rcode = 1;
                        continue;
                }

                rcode = 0;

                switch(cmd) {
			case ADDALIAS:
				name = getname();

				aliasfile = getaliasfile();

				alias = getalias(name, LOCALNAME);
				dname = aux_Name2DName(name);

				if(aux_add_alias(alias, dname, aliasfile, TRUE, TRUE) < 0) {
					fprintf(stderr, "Couldn't add alias\n");
					break;
				}

				if(alias){
					free(alias);
					alias = CNULL;
				}

				alias = getalias(name, RFCMAIL);
				if(alias) {
					if(aux_add_alias(alias, dname, aliasfile, TRUE, TRUE) < 0)
						fprintf(stderr, "Couldn't add alias\n");
					free(alias);
					alias = CNULL;
				}
				break;
                        case ADDEK:
                                if(!(key = build_key("Certificate from PSE object", 0))) {
					fprintf(stderr,"Can't build key\n");
					break;
				}
                                ostr = &octetstring;
                                if(sec_read_PSE(key->pse_sel, &object_oid, ostr) < 0)  {
					fprintf(stderr, "Can't read object  %s  from PSE\n", key->pse_sel->object.name);
					aux_free2_ObjId(&object_oid);
					break;
				}
				aux_free2_ObjId(&object_oid);
                                if(!(certificate = d_Certificate(ostr))) {
					fprintf(stderr,"Can't decode Certificate\n");
					free(ostr->octets);
					break;
				}
				free(ostr->octets);
				aux_free_Key(&key);
				rcode = af_pse_add_PK(ENCRYPTION, certificate->tbs);
				if(rcode < 0) fprintf(stderr, "Can't add cert to EKList\n");
				aux_free_Certificate(&certificate);
                                break;
                        case ADDPCA:
                                if(!(key = build_key("Certificate from PSE object", 0))) {
					fprintf(stderr,"Can't build key\n");
					break;
				}
                                ostr = &octetstring;
                                if(sec_read_PSE(key->pse_sel, &object_oid, ostr) < 0)  {
					fprintf(stderr,"Can't read object  %s  from PSE\n", key->pse_sel->object.name);
					aux_free2_ObjId(&object_oid);
					break;
				}
				aux_free2_ObjId(&object_oid);
                                if(!(certificate = d_Certificate(ostr))) {
					fprintf(stderr,"Can't decode Certificate\n");
					free(ostr->octets);
					break;
				}
				free(ostr->octets);
				rcode = af_pse_add_PCA(certificate->tbs);
				if(rcode < 0) fprintf(stderr, "Can't add cert to PCAList\n");
				aux_free_Certificate(&certificate);
                                break;
                        case ADDPK:
                                if(!(key = build_key("Certificate from PSE object", 0))) {
					fprintf(stderr,"Can't build key\n");
					break;
				}
                                ostr = &octetstring;
                                if(sec_read_PSE(key->pse_sel, &object_oid, ostr) < 0)  {
					fprintf(stderr,"Can't read object  %s  from PSE\n", key->pse_sel->object.name);
					aux_free2_ObjId(&object_oid);
					break;
				}
				aux_free2_ObjId(&object_oid);
                                if(!(certificate = d_Certificate(ostr))) {
					fprintf(stderr,"Can't decode Certificate\n");
					free(ostr->octets);
					break;
				}
				free(ostr->octets);
				rcode = af_pse_add_PK(SIGNATURE, certificate->tbs);
				if(rcode < 0) fprintf(stderr, "Can't add cert to PKList\n");
				aux_free_Certificate(&certificate);
                                break;
                        case ALGS:
                                strrep(&algname, getalgname());
                                algtype = 0;
                                if(!strcmp(algname, "ASYM_ENC")) algtype = ASYM_ENC;
                                else if(!strcmp(algname, "SYM_ENC")) algtype = SYM_ENC;
                                else if(!strcmp(algname, "HASH")) algtype = HASH;
                                else if(!strcmp(algname, "SIG")) algtype = SIG;
                                else if(!strcmp(algname, "ENC")) algtype = ENC;
                                else if(!strcmp(algname, "ALL")) algtype = ALL;

                                for(i = 0; TRUE; i++) {
                                        if(!alglist[i].name) break;
                                        if(algtype == 0) {
                                                if(!strcmp(alglist[i].name, algname)) {
                                                        aux_fprint_AlgId(stderr, alglist[i].algid);
                                                }
                                        }
                                        else {
                                                if(algtype == ALL || 
                                                   ((algtype == ASYM_ENC || algtype == SYM_ENC || algtype == HASH || algtype == SIG) 
                                                     && algtype == alglist[i].algtype) ||
                                                   ((algtype == ENC) && (alglist[i].algtype == ASYM_ENC || alglist[i].algtype == SYM_ENC))) {
                                                        aux_fprint_AlgId(stderr, alglist[i].algid);
                                                }
                                        }
                                }
                                break;
                        case ALIAS2DNAME:
                                alias = (Name *)nxtpar("pattern");
                                aux_fprint_alias2dname(stderr, (char *)alias);
				if(alias) free(alias);
                                break;
                        case ALIASES:
                                alias = (Name *)nxtpar("pattern");
                                aux_fprint_alias2dnames(stderr, (char *)alias);
				if(alias) free(alias);
                                break;
#ifdef X500
			case AUTHNONE:
				af_dir_authlevel = DBA_AUTH_NONE;
				break;
			case AUTHSIMPLE:
				af_dir_authlevel = DBA_AUTH_SIMPLE;
				break;
#ifdef STRONG
			case AUTHSTRONG:
				af_dir_authlevel = DBA_AUTH_STRONG;
				break;
#endif
#endif
			case CACRL:
				if(!cadir) {
					fprintf(stderr, "%s: This command is for CAs only\n", pgm_name);
					break;
				}
				setofcrlwithcerts = af_cadb_list_CRLWithCertificates(cadir);
                                fprintf(stderr, "The following revocation lists, each accompanied by its issuer's certification path\n");
				fprintf(stderr, "(which is OPTIONAL), have been stored in your CA's local database:\n\n\n\n");
				aux_fprint_SET_OF_CRLWithCertificates(stderr, setofcrlwithcerts);
				aux_free_SET_OF_CRLWithCertificates(&setofcrlwithcerts);
				break;
			case CAPRINTLOG:
				if(!cadir) {
					fprintf(stderr, "%s: This command is for CAs only\n", pgm_name);
					break;
				}
				if(*cadir != PATH_SEPARATION_CHAR) {
					strcpy(calogfile, getenv("HOME"));
					strcat(calogfile, PATH_SEPARATION_STRING);
					strcat(calogfile, cadir);
				}
				else strcpy(calogfile, cadir);
				strcat(calogfile, PATH_SEPARATION_STRING);
				strcat(calogfile, "calog");
				logfile = fopen(calogfile, "r");
				if(logfile == (FILE * ) 0) {
					fprintf(stderr, "%s: Can't open %s\n", pgm_name, CALOG);
					break;
				}
				while ((n = getc(logfile)) != EOF) putchar(n);
				fclose(logfile);
				logfile = (FILE * )0;
				break;
			case CASERIALNUMBERS:
				if(!cadir) {
					fprintf(stderr, "%s: This command is for CAs only\n", pgm_name);
					break;
				}
				name = getname();
                                if(!name) break;
				isscertset = af_cadb_get_user(name, cadir);
				if(isscertset) {
                                        fprintf(stderr, "Certificates issued for <%s>:\n", name);
                                        aux_fprint_SET_OF_IssuedCertificate(stderr, isscertset);
                                }
                                else fprintf(stderr, "No certificates issued for this user\n");
				break;
			case CAUSERS:
				if(!cadir) {
					fprintf(stderr, "%s: This command is for CAs only\n", pgm_name);
					break;
				}
				nameset = af_cadb_list_user(cadir);
                                fprintf(stderr, "The following users are registered:\n");
				aux_fprint_SET_OF_Name(stderr, nameset);
				break;

			case CERTIFY:
				if(!cadir) {
					fprintf(stderr, "%s: This command is for CAs only\n", pgm_name);
					break;
				}
                                if(!(key = build_key("Certificate from", 0))) {
					fprintf(stderr,"Can't build key\n");
					break;
				}
                                ostr = &octetstring;
                                if(sec_read_PSE(key->pse_sel, &object_oid, ostr) < 0)  {
					fprintf(stderr,"Can't read object  %s  from PSE\n", key->pse_sel->object.name);
					break;
				}
                                if(!(certificate = d_Certificate(ostr))) {
					fprintf(stderr,"Can't decode Certificate\n");
					aux_free_Key(&key);
					aux_free2_ObjId(&object_oid);
					free(ostr->octets);
					break;
				}
				aux_free2_ObjId(&object_oid);
				free(ostr->octets);
				aux_free_Key(&key);
                                if(!(key = build_key("Certified public key to", 1)))  {
					fprintf(stderr,"Can't build key\n");
					aux_free_Certificate(&certificate);
					break;
				}
				if(*cadir != PATH_SEPARATION_CHAR) {
					strcpy(calogfile, getenv("HOME"));
					strcat(calogfile, PATH_SEPARATION_STRING);
					strcat(calogfile, cadir);
				}
				else strcpy(calogfile, cadir);
				strcat(calogfile, PATH_SEPARATION_STRING);
				strcat(calogfile, "calog");
				logfile = fopen(calogfile, LOGFLAGS);
				if(logfile == (FILE * ) 0) {
					fprintf(stderr, "%s: Can't open %s\n", pgm_name, CALOG);
					break;
				}
				printrepr = aux_DName2Name(certificate->tbs->subject);
        			if(af_cadb_add_user(printrepr, cadir) < 0) {
                			LOGERR("can't access user db");
					fprintf(stderr, "%s: ",pgm_name);
                			fprintf(stderr, "Can't access user db\n");
                			break;
        			}
				free(printrepr);
				if(certificate->tbs->issuer)
					aux_free_DName(&certificate->tbs->issuer);
				certificate->tbs->issuer = af_pse_get_Name();
				if (certificate->tbs->valid->notbefore) {
					free(certificate->tbs->valid->notbefore);
					certificate->tbs->valid->notbefore = CNULL;
				}
				if (certificate->tbs->valid->notafter) {
					free(certificate->tbs->valid->notafter);
					certificate->tbs->valid->notafter = CNULL;
				}
				if (! notbefore) {
					certificate->tbs->valid->notbefore = aux_current_UTCTime();
					certificate->tbs->valid->notafter = aux_delta_UTCTime(certificate->tbs->valid->notbefore);
				}
				else {
					certificate->tbs->valid->notbefore = (UTCTime *)malloc(TIMELEN);
					strcpy(certificate->tbs->valid->notbefore, notbefore);
					free(notbefore);
					certificate->tbs->valid->notafter = (UTCTime *)malloc(TIMELEN);
					strcpy(certificate->tbs->valid->notafter, notafter);
					free(notafter);
				}
				certificate->tbs->serialnumber = af_pse_incr_serial();
				certificate->tbs->version = 0;           /* default version */
				if(certificate->tbs_DERcode)
					aux_free_OctetString(&certificate->tbs_DERcode);
				if (certificate->sig)
					aux_free_Signature(&certificate->sig);
				certificate->sig = (Signature * )malloc(sizeof(Signature));
 				if(! certificate->sig) {
					fprintf(stderr, "%s: ",pgm_name);
                			fprintf(stderr, "Can't allocate memory\n");
					break;
				}
				certificate->sig->signAI = af_pse_get_signAI();
				if (! certificate->sig->signAI) {
					fprintf(stderr, "%s: ",pgm_name);
                			fprintf(stderr, "Cannot determine the algorithm associated to your own secret signature key\n");
					break;
				}
				if (aux_ObjId2AlgType(certificate->sig->signAI->objid) == ASYM_ENC )
					certificate->sig->signAI = aux_cpy_AlgId(algorithm);
				certificate->tbs->signatureAI = aux_cpy_AlgId(certificate->sig->signAI);
				certificate->tbs_DERcode = e_ToBeSigned(certificate->tbs);
				if (!certificate->tbs_DERcode || (af_sign(certificate->tbs_DERcode, certificate->sig, SEC_END) < 0)) {
					fprintf(stderr, "%s: ",pgm_name);
                			fprintf(stderr, "AF Error with CA Signature\n");
					LOGAFERR;
					break;
				}
				if (af_cadb_add_Certificate(0, certificate, cadir)) {
					LOGERR("Can't access cert db");
					break;
				}
                                if(key->pse_sel) {
                                        if(!(ostr = e_Certificate(certificate)))  {
						fprintf(stderr,"Can't encode new Certificate\n");
						aux_free_Certificate(&certificate);
  						break;
					}
					aux_free_Certificate(&certificate);
					if(onekeypaironly == TRUE)
						oid = af_get_objoid(Cert_name);
					else {
                                        	keytype();
                                        	if(*objtype == 'S') oid = af_get_objoid(SignCert_name);
                                        	else if(*objtype == 'E') oid = af_get_objoid(EncCert_name);
                                        	else {
                                                	fprintf(stderr, "Type must me either 'S' or 'E'\n");
                                                	break;
                                        	}
					}
                                        if(sec_write_PSE(key->pse_sel, oid, ostr) < 0)  {
						fprintf(stderr,"Can't write to PSE\n");
						aux_free_OctetString(&ostr);
 						break;
					}
                                        fprintf(stderr, "New certificate stored in object %s\n", key->pse_sel->object.name);
					aux_free_OctetString(&ostr);
                                }
				fclose(logfile);
				logfile = (FILE * )0;
				break;
                        case CERT2KEYINFO:
                                if(!(key = build_key("Certificate from ", 0))) {
					fprintf(stderr,"Can't build key\n");
					break;
				}
                                ostr = &octetstring;
                                if(sec_read_PSE(key->pse_sel, &object_oid, ostr) < 0)  {
					fprintf(stderr,"Can't read object  %s  from PSE\n", key->pse_sel->object.name);
					aux_free2_ObjId(&object_oid);
					break;
				}
				aux_free2_ObjId(&object_oid);
                                if(!(certificate = d_Certificate(ostr))) {
					fprintf(stderr,"Can't decode Certificate\n");
					free(ostr->octets);
					break;
				}
				free(ostr->octets);
                                if(!(key = build_key("KeyInfo to ", 1)))  {
					fprintf(stderr,"Can't build key\n");
					aux_free_Certificate(&certificate);
					break;
				}
                                keyinfo = aux_cpy_KeyInfo(certificate->tbs->subjectPK);
				aux_free_Certificate(&certificate);
                                if(key->pse_sel) {
                                        if(!(ostr = e_KeyInfo(keyinfo)))  {
						fprintf(stderr,"Can't encode KeyInfo\n");
						aux_free_KeyInfo(&keyinfo);
						break;
					}
					aux_free_KeyInfo(&keyinfo);
					if(onekeypaironly == TRUE){
                                        	if(sec_write_PSE(key->pse_sel, SKnew_OID, ostr) < 0)  {
							fprintf(stderr,"Can't write to PSE\n");
							aux_free_OctetString(&ostr);
							break;
						}
					}
					else{
                                        	if(sec_write_PSE(key->pse_sel, SignSK_OID, ostr) < 0)  {
							fprintf(stderr,"Can't write to PSE\n");
							aux_free_OctetString(&ostr);
							break;
						}
					}
					aux_free_OctetString(&ostr);
                                        fprintf(stderr, "Public Key stored in object %s\n", key->pse_sel->object.name);
                                }
                                else {
                                        keyref = sec_put_key(keyinfo, key->keyref);
                                        fprintf(stderr, "Public Key stored under keyref %d\n", keyref);
					aux_free_KeyInfo(&keyinfo);
                                }
                                break;
                        case CERT2PKROOT:
                                if(!(key = build_key("Certificate from", 0))) {
					fprintf(stderr,"Can't build key\n");
					break;
				}
                                ostr = &octetstring;
                                if(sec_read_PSE(key->pse_sel, &object_oid, ostr) < 0)  {
					fprintf(stderr,"Can't read object  %s  from PSE\n", key->pse_sel->object.name);
					break;
				}
                                if(!(certificate = d_Certificate(ostr))) {
					fprintf(stderr,"Can't decode Certificate\n");
					aux_free2_ObjId(&object_oid);
					free(ostr->octets);
					break;
				}
				aux_free2_ObjId(&object_oid);
				free(ostr->octets);
				if(key) aux_free_Key(&key);
                                if(!(key = build_key("PKRoot to", 1)))  {
					fprintf(stderr,"Can't build key\n");
					aux_free_Certificate(&certificate);
					break;
				}
                                keyinfo = aux_cpy_KeyInfo(certificate->tbs->subjectPK);
                                pkroot = (PKRoot *)calloc(1, sizeof(PKRoot));
				if(!pkroot) {
					fprintf(stderr, "Can't allocate memory");
					aux_free_KeyInfo(&keyinfo);
					aux_free_Certificate(&certificate);
	                                break;
				}
                                pkroot->ca = aux_cpy_DName(certificate->tbs->issuer);
                                pkroot->newkey = (struct Serial *)calloc(1, sizeof(struct Serial));
				if(!pkroot->newkey) {
					fprintf(stderr, "Can't allocate memory");
					aux_free_PKRoot(&pkroot);
	                                break;
				}
                                pkroot->newkey->serial = aux_cpy_OctetString(certificate->tbs->serialnumber);
				pkroot->newkey->version = certificate->tbs->version;
				pkroot->newkey->valid = aux_cpy_Validity(certificate->tbs->valid);
                                pkroot->newkey->key = aux_cpy_KeyInfo(keyinfo);

				if (cadir)
					pkroot->newkey->sig = aux_cpy_Signature(certificate->sig);
				else
					pkroot->newkey->sig = (Signature * )0;

				aux_free_KeyInfo(&keyinfo);
				aux_free_Certificate(&certificate);

                                if(key->pse_sel) {
                                        if(!(ostr = e_PKRoot(pkroot)))  {
						fprintf(stderr,"Can't encode PKRoot\n");
						aux_free_PKRoot(&pkroot);
  						break;
					}
					aux_free_PKRoot(&pkroot);
                                        if(sec_write_PSE(key->pse_sel, PKRoot_OID, ostr) < 0)  {
						fprintf(stderr,"Can't write to PSE\n");
						aux_free_OctetString(&ostr);
 						break;
					}
                                        fprintf(stderr, "PKRoot stored in object %s\n", key->pse_sel->object.name);
					aux_free_OctetString(&ostr);
                                }
				if(pkroot) aux_free_PKRoot(&pkroot);
                                break;
			case CHECK:
			case TRUSTPATH:
				secretkey = (Key *)malloc(sizeof(Key));
				if(!secretkey) {
					fprintf(stderr, "Can't malloc memory for secretkey\n");
					break;
				}
				secretkey->key = (KeyInfo *)0;
				secretkey->keyref = 0;
				secretkey->alg = (AlgId *)0;
				secretkey->pse_sel = &std_pse;
				signpk = (KeyInfo *)0;
				encpk = (KeyInfo *)0;
				signsubject = NULLDNAME;
				encsubject = NULLDNAME;

				certs = af_pse_get_Certificates(SIGNATURE, NULLDNAME);
				if(!certs) {
					if(onekeypaironly == TRUE) {
						fprintf(stderr, "Can't read certificate from PSE\n");
						free(secretkey);
						break;
					}
					else {
						fprintf(stderr, "Can't get SIGNATURE certificate from PSE\n");
						goto enccert;
					}
				}
				signpk = aux_cpy_KeyInfo(certs->usercertificate->tbs->subjectPK);
				signsubject = aux_cpy_DName(certs->usercertificate->tbs->subject);
				if(cmd == CHECK) {
					if(onekeypaironly == TRUE)
						fprintf(stderr, "\nVerifying Cert with FCPath and PKRoot ... ");
					else
						fprintf(stderr, "\nVerifying SignCert with FCPath and PKRoot ... ");
				}
				else af_verbose = sec_verbose = FALSE;
                                rcode = af_verify_Certificates(certs, (UTCTime *)0, (PKRoot *)0);
				aux_free_Certificates(&certs);
				if(verbose && cmd == CHECK) aux_fprint_VerificationResult(stderr, verifresult);
				if(cmd == TRUSTPATH) aux_fprint_TrustPath(stderr, verifresult);
				aux_free_VerificationResult(&verifresult);
				if(cmd == CHECK) {
					if(rcode == 0) fprintf(stderr, "O.K.\n");
					else fprintf(stderr, "failed\n");
					if(onekeypaironly == TRUE)
						fprintf(stderr, "\nChecking whether the keys in Cert and SKnew are an RSA key pair ... ");
					else
						fprintf(stderr, "\nChecking whether the keys in SignCert and SignSK are an RSA key pair ... ");
				}
				if(onekeypaironly == TRUE)
					std_pse.object.name = SKnew_name;
				else
					std_pse.object.name = SignSK_name;
				rcode = sec_checkSK(secretkey, signpk);
				if(rcode < 0) {
					if(onekeypaironly == TRUE)
						fprintf(stderr, "\nRSA keys in SKnew and Cert do not fit\n");
					else
						fprintf(stderr, "\nRSA keys in SignSK and SignCert do not fit\n");
				}
				else if(cmd == CHECK) fprintf(stderr, "O.K.\n");
				if(cmd == TRUSTPATH) {
					if(verbose == 2) af_verbose = TRUE;
					if(verbose == 3) af_verbose = sec_verbose = TRUE;
				}
				if(onekeypaironly == TRUE  || cmd == TRUSTPATH) break;
enccert:
				certs = af_pse_get_Certificates(ENCRYPTION, NULLDNAME);
				if(!certs) {
					fprintf(stderr, "Can't get ENCRYPTION Certificates from PSE (EncCert and/or FCPath missing)\n");
					if(signpk) aux_free_KeyInfo(&signpk);
					if(signsubject) aux_free_DName(&signsubject);
					free(secretkey);
					break;
				}
				encpk = aux_cpy_KeyInfo(certs->usercertificate->tbs->subjectPK);
				encsubject = aux_cpy_DName(certs->usercertificate->tbs->subject);
				if(signsubject) if(aux_cmp_DName(signsubject, encsubject)) {
					fprintf(stderr, "SignCert and EncCert have different subject names\n");
				}
				if(signsubject) aux_free_DName(&signsubject);
				aux_free_DName(&encsubject);
				fprintf(stderr, "\nVerifying EncCert with FCPath and PKRoot ...\n");
                                rcode = af_verify_Certificates(certs, (UTCTime *)0, (PKRoot *)0);
				aux_free_Certificates(&certs);
				if(verbose) aux_fprint_VerificationResult(stderr, verifresult);
				aux_free_VerificationResult(&verifresult);
				fprintf(stderr, "\nChecking whether the keys in EncCert and DecSKnew are an RSA key pair ... ");
				std_pse.object.name = DecSKnew_name;
				rcode = sec_checkSK(secretkey, encpk);
				if(rcode < 0) fprintf(stderr, "\nRSA keys in DecSKnew and EncCert do not fit\n");
				else fprintf(stderr, "O.K.\n");
				aux_free_KeyInfo(&signpk);
				aux_free_KeyInfo(&encpk);
				free(secretkey);
				break;

                        case CHPIN:
#ifdef SCA
				if ((pse_location = sec_psetest(std_pse.app_name)) == ERR_in_psetest) {
					fprintf(stderr, "Error during test for pse: %s\n", std_pse.app_name);
					break;
				}
				if (pse_location == SWpse) new_pin();
else
				new_pin();
#endif
				std_pse.object.name = CNULL;
				/* Re-encrypt PSE-toc and all PSE-objects */
				rcode = sec_chpin(&std_pse, newpin);

				/* Free obsolete pins */
				if(AF_pse.pin) strzfree(&(AF_pse.pin));
				for (i = 0; i < PSE_MAXOBJ; i++){
					if(AF_pse.object[i].pin) strzfree(&(AF_pse.object[i].pin));
				}

				/* Assign new pin-values to AF_pse */

				AF_pse.pin = aux_cpy_String(newpin);
                                break;

                        case CLOSE:	/* whole PSE to be closed */
close:

				if(af_pse_close (NULLOBJID) == 0) ack(&std_pse, "closed");
				else fprintf(stderr, "No PSE open\n");
				AF_pse.app_name = CNULL;
				if(std_pse.app_name) {
					free (std_pse.app_name);
					std_pse.app_name = CNULL;
				}
				if(std_pse.pin) strzfree (&(std_pse.pin));
				std_pse.app_id = 0;
				if ( std_pse.object.name ) {
					free (std_pse.object.name);
					std_pse.object.name = CNULL;
				}
				if(std_pse.object.pin) strzfree (&(std_pse.object.pin));
                                break;
                        case CREATE:
                                if(interactive == TRUE) psesel(1);
                                if(!(rcode = sec_create(&std_pse))) {
                                        ack(&std_pse, "created");
                                        AF_pse.app_name = std_pse.app_name;
                                        AF_pse.pin = std_pse.pin;
                                        AF_pse.app_id = std_pse.app_id;
                                        if (std_pse.object.name) {
                                                for (i = 0; i < PSE_MAXOBJ; i++) 
							if (strcmp(AF_pse.object[i].name, std_pse.object.name) == 0) {
								if(std_pse.object.pin) {
									AF_pse.object[i].pin = (char *) malloc (strlen(std_pse.object.pin) + 1);
									if ( !AF_pse.object[i].pin ) {
										fprintf(stderr, "Can't allocate memory\n");
										break;
									}
									strcpy (AF_pse.object[i].pin, std_pse.object.pin);
								}
								else AF_pse.object[i].pin = (char *)0;
                                                        	break;
                                                        }  
                                        }
                                } 
				else {
					fprintf(stderr, "Can't create object\n");
                                        break;
                                }

                                break;
			case DELALIAS:
				alias = getalias(CNULL, LOCALNAME);
				if(!alias){
					fprintf(stderr, "No alias name specified!\n");
					break;
				}
				fprintf(stderr, "USER/SYSTEM Alias Database ? [U/S] [CR for U]:  ");
				gets(inp);
                		if(!inp || strlen(inp) == 0)
                        		aliasfile = useralias;
				else {
					if (! strcasecmp(inp, "U"))
						aliasfile = useralias;
					else if (! strcasecmp(inp, "S"))
						aliasfile = systemalias;
					else {
						fprintf(stderr,"Answer must be either 'U' or 'S'\n");
						break;
					}
				}
				if(aux_delete_alias(alias, aliasfile, TRUE) < 0) {
					fprintf(stderr, "Could not remove alias\n");
					break;
				}
				break;
			case DELEK:
				subject_dn = getdname("Owner");
				if(! subject_dn){
					issuer_dn = getdname("Issuer");
					if(! issuer_dn){
						fprintf(stderr, "Neither Owner nor Issuer has been specified!\n");
						break;
					}
					if ((serial = getserial()) == NULLOCTETSTRING) {
						fprintf(stderr, "No serial number specified!\n");
						break;
					}
				}
				if(subject_dn){
					subject = aux_DName2Name(subject_dn);
					if(! subject){
						fprintf(stderr, "Cannot transform DName-structure into Name!\n");
						break;
					}
					rcode = af_pse_delete_PK(ENCRYPTION, subject_dn, NULLDNAME, 0);
				}
				else{
					issuer = aux_DName2Name(issuer_dn);
					if(! issuer){
						fprintf(stderr, "Cannot transform DName-structure into Name!\n");
						break;
					}
					rcode = af_pse_delete_PK(ENCRYPTION, NULLDNAME, issuer_dn, serial);
				}
				if ( rcode < 0 ) {
					if (err_stack && (err_stack->e_number == EOBJNAME)) {
		        			fprintf(stderr, "\nThere is no ToBeSigned with\n");
						if (issuer_dn && serial) {
		        				fprintf(stderr, " issuer \"%s\"\n and serial number ", issuer);
							aux_fprint_Serialnumber(stderr, serial);
						}
						else
			        			fprintf(stderr, " owner \"%s\"\n", subject);

						fprintf(stderr, "stored in your EKList. No update done!\n");
					}
				}
				else {
					fprintf(stderr, "\nToBeSigned with\n");
					if (issuer_dn && serial) {
		        			fprintf(stderr, " issuer \"%s\"\n and serial number ", issuer);
		       				aux_fprint_Serialnumber(stderr, serial);
					}
					else
						fprintf(stderr, " owner \"%s\"\n", subject);

					fprintf(stderr, "removed from your EKList.\n");

					fprintf(stderr, "\nYour updated EKList now looks like this:\n\n");
					pklist = af_pse_get_PKList(ENCRYPTION);
					if ( !pklist )
						fprintf(stderr, "Your EKList is EMPTY!\n");
					else {
						fprintf(stderr, " ****************** EKList ******************\n");
						aux_fprint_PKList (stderr, pklist);
						aux_free_PKList(& pklist);				
					}
				}
				if(subject_dn) aux_free_DName(& subject_dn);
				if(issuer_dn) aux_free_DName(& issuer_dn);
				if(subject) free(subject);
				if(issuer) free(issuer);
				break;
                        case DELETE:
                                psesel(1);
                                if(!std_pse.object.name || !strlen(std_pse.object.name)) {
                                        fprintf(stderr, "Do you really want to delete %s ? [yes/no]: ", std_pse.app_name);
                                        gets(inp);
                                        if(strcmp(inp, "yes")) {
                                                fprintf(stderr, "%s not deleted\n", std_pse.app_name);
                                                break;
                                        }
                                }
                                if(!(rcode = sec_delete(&std_pse))) {
					ack(&std_pse, "deleted");
					if(std_pse.object.name && strlen(std_pse.object.name) && !strcmp(std_pse.object.name, "AliasList"))
						retvalue = aux_get_AliasList();
				}
                                else ack(&std_pse, "does not exist");
                                break;
                        case DELKEY:
                                if(!(key = build_key("", 1)))  {
					fprintf(stderr,"Can't build key\n");
					break;
				}
                                if(!(rcode = sec_del_key(key->keyref))) {
					outtext = "";
#ifdef SCA
					if ((key->keyref & SC_KEY) == SC_KEY) {
						key->keyref =  key->keyref & ~SC_KEY;
						outtext = "in the smartcard";
						
					} 
					else if ((key->keyref & SCT_KEY) == SCT_KEY) {
						outtext = "in the SCT";
						key->keyref = key->keyref & ~SCT_KEY;
					}
					
#endif
					fprintf(stderr, "Key %s under keyref %d deleted\n", outtext, key->keyref);
				}
				else {
					aux_fprint_error(stderr, verbose);
				}

                                break;
			case DELPCA:
				subject_dn = getdname("PCA DName");
				if(! subject_dn){
					fprintf(stderr, "PCA DName has not been specified!\n");
					break;
				}
				subject = aux_DName2Name(subject_dn);
				if(! subject){
					fprintf(stderr, "Cannot transform DName-structure into Name!\n");
					break;
				}
				rcode = af_pse_delete_PCA(subject_dn);
				if ( rcode < 0 ) {
					if (err_stack && (err_stack->e_number == EOBJNAME)) {
			        		fprintf(stderr, "\nThere is no PCA with DName \"%s\"\n", subject);

						fprintf(stderr, "stored in your PCAList. No update done!\n");
					}
				}
				else {
					fprintf(stderr, "\nPCA with DName \"%s\"\n", subject);
					fprintf(stderr, "removed from your PCAList.\n");
					fprintf(stderr, "\nYour updated PCAList now looks like this:\n\n");
					pklist = af_pse_get_PCAList();
					if ( !pklist )
						fprintf(stderr, "Your PCAList is EMPTY!\n");
					else {
						fprintf(stderr, " ****************** PCAList ******************\n");
						aux_fprint_PKList(stderr, pklist);
						aux_free_PKList(& pklist);				
					}
				}
				if(subject_dn) aux_free_DName(&subject_dn);
				if(subject) free(subject);
				break;
 			case DELCRL:
 				issuer_dn = getdname("Issuer");
 				if(! issuer_dn){
 					fprintf(stderr, "No issuer specified!\n");
 					break;
 				}
 				issuer = aux_DName2Name(issuer_dn);
 				if(! issuer){
 					fprintf(stderr, "Cannot transform DName-structure into Name!\n");
 					break;
 				}
 				rcode = af_pse_delete_CRL(issuer_dn);
 				if ( rcode < 0 ) {
 					if (err_stack && (err_stack->e_number == EOBJNAME)) {
 		        			fprintf(stderr, "\nThere is no revocation list with ");
 		        			fprintf(stderr, "issuer \"%s\"\n", issuer);
 						fprintf(stderr, "stored in your PSE. No update done!\n");
 					}
 				}
 				else {
 					fprintf(stderr, "\nRevocation list issued by \"%s\" ", issuer);
 					fprintf(stderr, "removed from your PSE.\n\n");
 
 					fprintf(stderr, "Your updated set of locally stored revocation lists "); 
 					fprintf(stderr, "now looks like this:\n\n");
 					crlset = af_pse_get_CrlSet();
 					if (! crlset )
 						fprintf(stderr, "            E  M  P  T  Y  !\n\n");
 					else {
 						fprintf(stderr, " ****************** Set of Locally Stored Revocation Lists ******************\n");
 						aux_fprint_CrlSet (stderr, crlset);
 						aux_free_CrlSet(&crlset);				
 					}
 				}
 				if(issuer_dn) aux_free_DName(& issuer_dn);
 				if(issuer) free(issuer);
 				break;
			case DELPK:
				subject_dn = getdname("Owner");
				if(! subject_dn){
					issuer_dn = getdname("Issuer");
					if(! issuer_dn){
						fprintf(stderr, "Neither Owner nor Issuer has been specified!\n");
						break;
					}
					if ((serial = getserial()) == NULLOCTETSTRING) {
						fprintf(stderr, "No serial number specified!\n");
						break;
					}
				}
				if(subject_dn){
					subject = aux_DName2Name(subject_dn);
					if(! subject){
						fprintf(stderr, "Cannot transform DName-structure into Name!\n");
						break;
					}
					rcode = af_pse_delete_PK(SIGNATURE, subject_dn, NULLDNAME, 0);
				}
				else{
					issuer = aux_DName2Name(issuer_dn);
					if(! issuer){
						fprintf(stderr, "Cannot transform DName-structure into Name!\n");
						break;
					}
					rcode = af_pse_delete_PK(SIGNATURE, NULLDNAME, issuer_dn, serial);
				}
				if ( rcode < 0 ) {
					if (err_stack && (err_stack->e_number == EOBJNAME)) {
		        			fprintf(stderr, "\nThere is no ToBeSigned with\n");
						if (issuer_dn && serial) {
		        				fprintf(stderr, " issuer \"%s\"\n and serial number ", issuer);
		       					aux_fprint_Serialnumber(stderr, serial);
						}
						else
			        			fprintf(stderr, " owner \"%s\"\n", subject);

						fprintf(stderr, "stored in your PKList. No update done!\n");
					}
				}
				else {
					fprintf(stderr, "\nToBeSigned with\n");
					if (issuer_dn && serial) {
		        			fprintf(stderr, " issuer \"%s\"\n and serial number ", issuer);
		       				aux_fprint_Serialnumber(stderr, serial);
					}
					else
						fprintf(stderr, " owner \"%s\"\n", subject);

					fprintf(stderr, "removed from your PKList.\n");

					fprintf(stderr, "\nYour updated PKList now looks like this:\n\n");
					pklist = af_pse_get_PKList(SIGNATURE);
					if ( !pklist )
						fprintf(stderr, "Your PKList is EMPTY!\n");
					else {
						fprintf(stderr, " ****************** PKList ******************\n");
						aux_fprint_PKList(stderr, pklist);
						aux_free_PKList(& pklist);				
					}
				}
				if(subject_dn) aux_free_DName(&subject_dn);
				if(issuer_dn) aux_free_DName(&issuer_dn);
				if(subject) free(subject);
				if(issuer) free(issuer);
				break;
                        case DH1:
                                if(!(cc = nxtpar(CNULL))) {
                                        fprintf(stderr, "Use DH Parameter of default PSE object, peers public value or create temporary one? (d,p,t): ");
                                        cc = gets((char *)malloc(128));
					if( !cc ) {
						fprintf(stderr, "Can't allocate memory\n");
						break;
					}
				}
				if(!strncasecmp(cc, "default", strlen(cc))) {
					dh_with_pg = FALSE;
					free(cc);
				    	if(!(cc = nxtpar(""))) {
					    fprintf(stderr, "Use dhWithCommonModulus OID (without DH parameters)? (y/n): ");
					    cc = gets((char *)malloc(128));
					    if( !cc ) {
						    fprintf(stderr, "Can't allocate memory");
						    break;
					    }
					    if(!strcasecmp(cc, "no")) dh_with_pg = TRUE;
					    if(!strcasecmp(cc, "n")) dh_with_pg = TRUE;
					    free(cc);
				   	}
				    	else if(strcasecmp(cc, "common")) dh_with_pg = TRUE;

					if(!(dh_parm = af_pse_get_DHparam())) {
						fprintf(stderr,"There is no DH Parameter object on PSE, use dhinit before\n");
						break;
					}

				}
				else if(!strncasecmp(cc, "peer", strlen(cc))) {
					dh_with_pg = TRUE;
					free(cc);

					filen("File with peer's public DH key y: ");
					if(!(dh_peer_public_y_encoded = aux_file2OctetString(filename))) {
						fprintf(stderr,"Can't read file %s\n", filename);
						break;
					}
					if(!(dh_peer_public_y = d_KeyInfo(dh_peer_public_y_encoded))) {
						aux_free_OctetString(&dh_peer_public_y_encoded);
						fprintf(stderr,"Can't decode file %s\n", filename);
						break;
					}
					aux_free_OctetString(&dh_peer_public_y_encoded);
				
					if(!(dh_parm = dh_peer_public_y->subjectAI)) {
						aux_free_KeyInfo(&dh_peer_public_y);
						fprintf(stderr,"There is no DH Parameter added to peers key\n");
						break;
					}

					if(dh_peer_public_y->subjectkey.nbits) free(dh_peer_public_y->subjectkey.bits);
					free(dh_peer_public_y);
				}
				else if(!strncasecmp(cc, "temporary", strlen(cc))) {
					dh_with_pg = TRUE;
					free(cc);
					dh_keysize = getsize("Length of prime modulus p in bits");
					dh_privatevalue = getsize("Length of private values x in bits");
	    
					if(!dh_keysize) dh_keysize = 512;
	    
					fprintf(stderr, "Generating prime p and base g ...\n");
					if(!(dh_parm = sec_DH_init((dh_keysize + 7) / 8, dh_privatevalue))) {
						aux_fprint_error(stderr, verbose);
						fprintf(stderr,"Generating a DH Parameter failed\n");
						break;
					}
				}
				else break;

                                if(!(dh_private = build_key("private DH ", 1))) break;
                                filen("File for own public DH key y: ");

				if(!(dh_own_public_y = sec_DH_phase1(dh_parm, dh_private, dh_with_pg))) {
					aux_fprint_error(stderr, verbose);
					fprintf(stderr,"Phase 1 failed\n");
					aux_free_AlgId(&dh_parm);
					aux_free_Key(&dh_private);
					break;
				}
				aux_free_Key(&dh_private);
				aux_free_AlgId(&dh_parm);
				if(!(dh_own_public_y_encoded = e_KeyInfo(dh_own_public_y))) {
					fprintf(stderr,"Encoding of public keyinfo failed\n");
					aux_free_KeyInfo(&dh_own_public_y);
					break;
				}
				aux_free_KeyInfo(&dh_own_public_y);
				if(aux_OctetString2file(dh_own_public_y_encoded, filename, 2) < 0) {
					fprintf(stderr,"Can't write DH public key y to file %s\n", filename);
					aux_free_OctetString(&dh_own_public_y_encoded);
					break;
				}
				aux_free_OctetString(&dh_own_public_y_encoded);
                                break;
                        case DH2:
                                if(!(dh_private = build_key("private DH ", 0))) break;
                                filen("File of peer's public DH key y: ");
                                psesel(2);

				if(!(dh_peer_public_y_encoded = aux_file2OctetString(filename))) {
					fprintf(stderr,"Can't read DH public key y from file %s\n", filename);
					aux_free_Key(&dh_private);
					break;
				}
				if(!(dh_peer_public_y = d_KeyInfo(dh_peer_public_y_encoded))) {
					fprintf(stderr,"Can't decode DH public key y %s\n", filename);
					aux_free_OctetString(&dh_peer_public_y_encoded);
					break;
				}
				
				aux_free_OctetString(&dh_peer_public_y_encoded);

				if(!(dh_parm = af_pse_get_DHparam())) {
					fprintf(stderr,"Can't read DH parameters (PSE object DHparam)\n");
					aux_free_Key(&dh_private);
					aux_free_KeyInfo(&dh_peer_public_y);
					break;
				}

				if(!(dh_agreed_key = sec_DH_phase2(dh_parm, dh_private, dh_peer_public_y))) {
					aux_fprint_error(stderr, verbose);
					fprintf(stderr,"DH phase 2 failed\n");
					aux_free_AlgId(&dh_parm);
					aux_free_Key(&dh_private);
					aux_free_KeyInfo(&dh_peer_public_y);
					break;
				}

				aux_free_AlgId(&dh_parm);
				aux_free_Key(&dh_private);
				aux_free_KeyInfo(&dh_peer_public_y);

                                if(sec_open(&std_pse) < 0) 
					if(sec_create(&std_pse) < 0) {
						fprintf(stderr,"Can't create object on PSE\n");
						aux_free_BitString(&dh_agreed_key);
						break;
					}


	                        dh_agreed_key_oid = aux_cpy_ObjId(DHkey_OID);

				ostr = e_BitString(dh_agreed_key);

				if(sec_write_PSE(&std_pse, dh_agreed_key_oid, ostr) < 0) {
					fprintf(stderr,"Can't write agreed key to PSE\n");
					aux_free_BitString(&dh_agreed_key);
					aux_free_ObjId(&dh_agreed_key_oid);
					aux_free_OctetString(&ostr);
					break;
				}
				aux_free_BitString(&dh_agreed_key);
				aux_free_ObjId(&dh_agreed_key_oid);
				aux_free_OctetString(&ostr);
                                break;
                        case DHINIT:
				if((algid = af_pse_get_DHparam())) {
					fprintf(stderr, "DHparam exists already. Overwrite? (y/n): ");
					dd = gets(answ);
					if(!dd || *dd != 'y') break;
				}
				dh_keysize = getsize("Length of prime modulus p in bits");
				dh_privatevalue = getsize("Length of private values x in bits");

				if(!dh_keysize) dh_keysize = 512;
			
				fprintf(stderr, "Generating prime p and base g ...\n");

				if(!(dh_parm = sec_DH_init(dh_keysize, dh_privatevalue))) {
					fprintf(stderr,"Generating DH Parameter failed\n");
					break;
				}
				if(af_pse_update_DHparam(dh_parm) < 0) {
					fprintf(stderr,"Can't write DH Parameter to PSE object %s\n", DHparam_name);
					aux_free_AlgId(&dh_parm);
					break;
				}

				aux_free_AlgId(&dh_parm);
                                break;
                        case DNAME2ALIAS:
                                name = (Name *)nxtpar("pattern");
                                aux_fprint_dname2alias(stderr, (char *)name);
				if(name) free(name);
                                break;
#ifdef SCA
                        case EJECT:
                                if(dd = nxtpar("")) {
					n = atoi(dd);
					free(dd);
				}
                                else n = 0;
                                sec_sc_eject(ALL_SCTS);
                                break;
#endif
			case ENTER:
				if (af_access_directory == FALSE) {
					fprintf(stderr, "af_access_directory is set to FALSE!\n");
					fprintf(stderr, "If you want to access the Directory, you should invoke ");
					fprintf(stderr, "the 'psemaint' command with the -D option.\n");
					break;
				}
				if(!af_x500 && !af_afdb){
					fprintf(stderr, "No directory flags (AFDBFILE or X500) have been compiled.\n");
					fprintf(stderr, "Therfore, no directory access is provided.\n");
					break;
				}
				attrtype = getattrtype(x500);
				if (!strncasecmp(attrtype, "Certificate", 2) || !strncasecmp(attrtype, "UserCertificate", 1) || !strncasecmp(attrtype, "CACertificate", 2)){
#ifdef X500
					if (x500) {
						if(!strncasecmp(attrtype, "UserCertificate", 1)) 
							certtype = userCertificate;
                                       	        else certtype = cACertificate;
					}
#endif
#ifdef AFDBFILE
					if(onekeypaironly == TRUE){
						std_pse.object.name = Cert_name;
						ktype = SIGNATURE;  /* ktype is not relevant in this case, but should have an acceptable value */
					}
					else{
                                        	keytype();
                                        	if(*objtype == 'S') {
							std_pse.object.name = SignCert_name;
							ktype = SIGNATURE;
						}
                                        	else if(*objtype == 'E') {
							std_pse.object.name = EncCert_name;
							ktype = ENCRYPTION;
						}
                                        	else {
                                                	fprintf(stderr, "Type must me either 'S' or 'E'\n");
                                                	break;
                                        	}
					}
#endif
					fprintf(stderr, "\nEnter name of PSE object which contains ");
					fprintf(stderr, "certificate\n");
					fprintf(stderr, " to be entered into the directory:\n");
					i = psesel(2);
                               		if(i < 0)  {
						fprintf(stderr,"psesel failed\n");
						break;
					}
					ostr = &octetstring;
                                	if(sec_read_PSE(&std_pse, &object_oid, ostr) < 0)  {
						fprintf(stderr,"Can't read object  %s  from PSE\n", std_pse.object.name);
						break;
					}
					store_objpin();
                                	if(! aux_cmp_ObjId(&object_oid, SignCert_OID) || ! aux_cmp_ObjId(&object_oid, EncCert_OID) || ! aux_cmp_ObjId(&object_oid, Cert_OID)) { 
						if(!(certificate = d_Certificate(ostr))) {
                                        		fprintf(stderr, "Can't decode %s\n", std_pse.object.name);
                                        		free(ostr->octets);
 							aux_free2_ObjId(&object_oid);
                                      			break;
					        }
					aux_free2_ObjId(&object_oid);
					free(ostr->octets);	
                                        }
				}
				else if (!strncasecmp(attrtype,"CrossCertificatePair", 2)) {
					if(!cadir) {
						fprintf(stderr, "%s: CrossCertificatePairs are for CAs only\n", pgm_name);
						break;
					}	
					cpair = compose_CertificatePair();
					if (!cpair) {
						fprintf(stderr, "%s: No CrossCertificatePair specified\n", pgm_name);
						break;
					}
				}
				else {
					if(!cadir) {
						fprintf(stderr, "%s: Revocation lists are for CAs only\n", pgm_name);
						break;
					}
					dname = af_pse_get_Name();
					name = aux_DName2Name(dname);
					aux_free_DName(&dname);
					crlwithcerts = af_cadb_get_CRLWithCertificates(name, cadir);
					if(! crlwithcerts || ! crlwithcerts->crl){
						fprintf(stderr, "WARNING: Your own CRL is NOT stored in your local database!\n");
						break;
					}
					fprintf(stderr, "\nThis is your locally stored revocation list:\n\n");
					aux_fprint_CRL(stderr, crlwithcerts->crl);
					fprintf(stderr, "\n\nVerifying your locally stored CRL ...\n\n");
					certs = af_pse_get_Certificates(SIGNATURE, NULLDNAME); 
					rcode = af_verify(crlwithcerts->crl->tbs_DERcode, crlwithcerts->crl->sig, SEC_END, certs, (UTCTime * )0, (PKRoot * )0);
					aux_free_Certificates(&certs);
					aux_fprint_VerificationResult(stderr, verifresult);
					aux_free_VerificationResult(&verifresult);
					if (rcode == 0) {
						fprintf(stderr, "Verification of locally stored ");
						fprintf(stderr, "CRL s u c c e e d e d!\n\n");
					}
					else {
						fprintf(stderr, "WARNING: Verification of locally ");
						fprintf(stderr, "stored CRL f a i l e d!\n");
						break;
					}
				}
				if (!strncasecmp(attrtype, "Certificate", 2) || !strncasecmp(attrtype, "UserCertificate", 1) || !strncasecmp(attrtype, "CACertificate", 2)) {
#ifdef X500
					if ( x500 ) {
						fprintf(stderr, "\nAccessing your X.500 directory entry ...\n\n");
						if ( af_dir_enter_Certificate(certificate, certtype) == 0 )
							fprintf(stderr, "Directory entry (X.500) succeeded.\n");
                               			else fprintf(stderr, "Directory entry (X.500) f a i l e d.\n");
					}
#endif
#ifdef AFDBFILE
					fprintf(stderr, "\nAccessing your .af-db directory entry ...\n\n");
					if ( af_afdb_enter_Certificate(certificate, ktype) == 0 )
						fprintf(stderr, "Directory entry (.af-db) succeeded.\n");
                               		else fprintf(stderr, "Directory entry (.af-db) f a i l e d.\n");
#endif
					aux_free_Certificate(&certificate);
				}
				else if (!strncasecmp(attrtype,"CrossCertificatePair", 2)) {
#ifdef X500
					if ( x500 ) {
						fprintf(stderr, "\nAccessing your X.500 directory entry ...\n\n");
						if ( af_dir_enter_CertificatePair(cpair) == 0 )
							fprintf(stderr, "Directory entry (X.500) succeeded.\n");
                               			else fprintf(stderr, "Directory entry (X.500) f a i l e d.\n");
					}
#endif
#ifdef AFDBFILE
					own_dname = af_pse_get_Name();
					fprintf(stderr, "\nAccessing your .af-db directory entry ...\n\n");
					if ( af_afdb_enter_CertificatePair(cpair) == 0 )
						fprintf(stderr, "Directory entry (.af-db) succeeded.\n");
                               		else fprintf(stderr, "Directory entry (.af-db) f a i l e d.\n");
#endif
					aux_free_CertificatePair(&cpair);
				}
				else {   /* attrtype = CertificateRevocationList */
#ifdef X500
					if ( x500 ) {
						fprintf(stderr, "\nAccessing your X.500 directory entry ...\n\n");
						if ( af_dir_enter_CRL(crlwithcerts->crl) == 0 )
							fprintf(stderr, "Directory entry (X.500) succeeded.\n");
                               			else fprintf(stderr, "Directory entry (X.500) f a i l e d.\n");
					}
#endif
#ifdef AFDBFILE
					fprintf(stderr, "\nAccessing your .af-db directory entry ...\n\n");
					if ( af_afdb_enter_CRL(crlwithcerts->crl) == 0 )
						fprintf(stderr, "Directory entry (.af-db) succeeded.\n");
                               		else fprintf(stderr, "Directory entry (.af-db) f a i l e d.\n");
#endif
					aux_free_CRLWithCertificates(&crlwithcerts);
				}
                                break;
                        case EXIT:
                        case ENDE:
                        case QUIT:
                                exit(0);
                        case GENKEY:
                                replace = FALSE;
                                strrep(&algname, getalgname());
                                algenc = aux_Name2AlgEnc(algname);
                                if(algenc == DES || algenc == DES3) {
					if(!(secretkey = build_key("generated DES ", 1))) break;
				}
                                else {
					publickey = (Key *)0;
					secretkey = (Key *)0;
                                        if(!(publickey = build_key("generated public ", 1))) break;
                                        if(!(secretkey = build_key("generated secret ", 1))) break;
                                }
                                time(&atime);

				sec_gen_verbose = TRUE;
                                if((rcode = sec_gen_key(secretkey, replace)) < 0)  {
					fprintf(stderr, err_stack->e_text);
					fprintf(stderr, "\n");
					break;
				}

                                etime = time((time_t *)0) - atime;
                                if(algenc == RSA || algenc == DSA) fprintf(stderr, "Secret Key stored ");
                                else if(algenc == IDEA) fprintf(stderr, "IDEA Key stored ");
                                else fprintf(stderr, "DES Key stored ");
                                if (secretkey->pse_sel) 
					fprintf(stderr, " in object %s\n", secretkey->pse_sel->object.name);
                                else {
					outtext = "";

#ifdef SCA
					if ((secretkey->keyref & SC_KEY) == SC_KEY) {
						secretkey->keyref =  secretkey->keyref & ~SC_KEY;
						outtext = "in the smartcard";
						
					} 
					else if ((secretkey->keyref & SCT_KEY) == SCT_KEY) {
						outtext = "in the SCT";
						secretkey->keyref = secretkey->keyref & ~SCT_KEY;
					}
					
#endif
					fprintf(stderr, "%s under keyref %d \n", outtext, secretkey->keyref);

				}
                                if(algenc == RSA || algenc == DSA) {
					if(algenc == RSA) oid = RSA_PK_OID;
					else oid = DSA_PK_OID;
                                        /* store public key */
                                        if(publickey->pse_sel) {
                                                if(!(ostr = e_KeyInfo(secretkey->key)))  {
							fprintf(stderr,"Can't encode KeyInfo\n");
							aux_free_Key(&secretkey);
							aux_free_Key(&publickey);
							break;
						}
						aux_free_Key(&secretkey);
                                                if(sec_write_PSE(publickey->pse_sel, oid, ostr) < 0)  {
                                                        fprintf(stderr, "sec_write failed\n");
							aux_free_OctetString(&ostr);
							aux_free_Key(&publickey);
                                                        break;
                                                }
						aux_free_OctetString(&ostr);
                                                fprintf(stderr, "Public Key stored in object %s\n", publickey->pse_sel->object.name);
                                        }
                                        else {
                                                keyref = sec_put_key(secretkey->key, publickey->keyref);
                                                fprintf(stderr, "Public Key stored under keyref %d\n", keyref);
						aux_free_Key(&secretkey);
                                        }
					aux_free_Key(&publickey);
                                }
                                fprintf(stderr, "Time for key generation: %ld sec\n", etime);
                                break;
                        case HELPCMD:
                        case QM:
                                helpcmd();
                                break;
			case INITCRL:
				if(!cadir) {
					fprintf(stderr, "%s: This command is for CAs only\n", pgm_name);
					break;
				}
				crl = (CRL * )malloc(sizeof(CRL));
				if (! crl) {
					fprintf(stderr, "%s: ", pgm_name);
  	        			fprintf(stderr, "Can't allocate memory\n");
					break;
				}

				crl->tbs = (CRLTBS * )malloc(sizeof(CRLTBS));
				if (! crl->tbs) {
					fprintf(stderr, "%s: ", pgm_name);
  	        			fprintf(stderr, "Can't allocate memory\n");
					break;
				}

				if (!(crl->tbs->issuer = af_pse_get_Name())) {
					break;
				}

				crl->tbs->lastUpdate = aux_current_UTCTime();
				crl->tbs->nextUpdate = (UTCTime *)0;
				crl->tbs->nextUpdate = get_nextUpdate(crl->tbs->lastUpdate);

				crl->tbs->revokedCertificates = (SEQUENCE_OF_CRLEntry * )0;

				crl->sig = (Signature * )malloc(sizeof(Signature));
				if (! crl->sig) {
					fprintf(stderr, "%s: ", pgm_name);
  	        			fprintf(stderr, "Can't allocate memory\n");
					break;
				}
				crl->sig->signature.nbits = 0;
				crl->sig->signature.bits = CNULL;

				crl->sig->signAI = af_pse_get_signAI();
				if ( aux_ObjId2AlgType(crl->sig->signAI->objid) == ASYM_ENC )
					crl->sig->signAI = aux_cpy_AlgId(algorithm);

				crl->tbs->signatureAI = aux_cpy_AlgId(crl->sig->signAI);

				if ((crl->tbs_DERcode = e_CRLTBS(crl->tbs)) == NULLOCTETSTRING) {
					fprintf(stderr, "%s: ", pgm_name);
  	        			fprintf(stderr, "e_CRLTBS failed\n");
					break;
				}

				if ((crl->tbs_DERcode = e_CRLTBS(crl->tbs)) == NULLOCTETSTRING) {
					fprintf(stderr, "Can't encode crl->tbs\n");
					break;
				}

				fprintf(stderr, "\nThe following Crl is to be signed. ");
				fprintf(stderr, "Please check it:\n\n");
				aux_fprint_CRLTBS(stderr, crl->tbs);
				fprintf(stderr, "\nDo you want to sign the displayed ");
				fprintf(stderr, "revocation list ?\n");
				fprintf(stderr, "If you want to sign it, (re)enter the PIN of your chipcard:\n\n");
				af_pse_close(NULLOBJID);
				if ( af_sign(crl->tbs_DERcode, crl->sig, SEC_END) < 0 ) {
					fprintf(stderr, "Signature of revocation list failed\n");
					break;
				}

				/* Update on Directory entry, PSE, and CA directory: */
#ifdef X500
				if (x500 && af_access_directory == TRUE) {
					/* update X.500 directory entry */
					fprintf(stderr, "\n**********************************************\n");
					fprintf(stderr, "\nTrying to update your X.500 directory entry ...\n\n");
					if ( af_dir_enter_CRL(crl) < 0 ) 
						fprintf(stderr, "\n Directory entry (X.500) f a i l e d !\n");
					else fprintf(stderr, "\n Done!\n");
				}
#endif
#ifdef AFDBFILE
				if(af_access_directory == TRUE){
					/* update .af-db directory entry */
					fprintf(stderr, "\n**********************************************\n");
					fprintf(stderr, "\nTrying to update your .af-db directory entry ...");
					if ( af_afdb_enter_CRL(crl) < 0 ) 
						fprintf(stderr, "\n Directory entry (.af-db) f a i l e d !\n");
					else fprintf(stderr, "\n Done!\n");
				}
#endif

				/* update PSE object CrlSet, even if the directory entry failed */
				crlpse = CRL2Crl (crl);
				fprintf(stderr, "\n**********************************************\n");
				fprintf(stderr, "\nUpdating PSE object CrlSet ...\n");
				rcode = af_pse_add_CRL(crlpse);
				if (rcode != 0) {
					fprintf(stderr, "\n Cannot update PSE object CrlSet.\n");
					aux_free_Crl (&crlpse);
				}
				else fprintf(stderr, "\n Done!\n");
				aux_free_Crl (&crlpse);

				/* update 'crlwithcerts' database in CA directory, even if the directory entry failed */
				fprintf(stderr, "\n**********************************************\n");
				fprintf(stderr, "\nUpdating 'crlwithcerts' database in CA directory \"%s\" ...\n", cadir);
				if(*cadir != PATH_SEPARATION_CHAR) {
					strcpy(calogfile, getenv("HOME"));
					strcat(calogfile, PATH_SEPARATION_STRING);
					strcat(calogfile, cadir);
				}
				else strcpy(calogfile, cadir);
				strcat(calogfile, PATH_SEPARATION_STRING);
				strcat(calogfile, "calog");
				logfile = fopen(calogfile, LOGFLAGS);
				if(logfile == (FILE * ) 0) {
					fprintf(stderr, "%s: Can't open %s\n", pgm_name, CALOG);
					break;
				}
				crlwithcerts = (CRLWithCertificates * )malloc(sizeof(CRLWithCertificates));
				crlwithcerts->crl = crl;
				crlwithcerts->certificates = (Certificates * )0;
				rcode = af_cadb_add_CRLWithCertificates(crlwithcerts, cadir);
				if(rcode != 0){
					fprintf(stderr, "%s: ", pgm_name);
					fprintf(stderr, "Cannot store your updated CRL in your 'crlwithcerts' database!\n");
					aux_fprint_error(stderr, verbose);
					aux_free_CRLWithCertificates(&crlwithcerts);
					break;
				}
				fprintf(stderr, "\nMost current version of CRL stored in 'crlwithcerts' database in ");
				fprintf(stderr, "CA directory \"%s\".\n\n", cadir);
				aux_free_CRLWithCertificates(&crlwithcerts);
				fclose(logfile);
				logfile = (FILE * )0;
				break;
			case ISSUEDCERTIFICATE:
				if(!cadir) {
					fprintf(stderr, "%s: This command is for CAs only\n", pgm_name);
					break;
				}
                                if((dd = nxtpar(""))) {
					serial = aux_create_SerialNo(dd);
					free(dd);
				}
                                else if((serial = getserial()) == NULLOCTETSTRING){
					fprintf(stderr, "No serial number specified!\n");
					break;
				}
				certificate = af_cadb_get_Certificate(serial, cadir);
				if(certificate) {
					aux_fprint_Certificate(stderr, certificate);
					store_certificate(certificate, onekeypaironly);
				}
                                else {
					fprintf(stderr, "No certificate issued with SerialNo ");
					aux_fprint_Serialnumber(stderr, serial);
				}
				break;
                        case KEYTOC:
                                rcode = sec_print_toc(stderr, (PSESel *)0);
				if(rcode < 0) fprintf(stderr, "Can't read TOC of key_pool\n");
                                break;
			case MFLIST:
				MF_fprint(stderr);
				break;
                        case OPEN:
                                if (psesel(1) < 0)  {
                                        fprintf(stderr, "psesel failed \n");
                                        break;
                                }

                                AF_pse.app_id = std_pse.app_id;
				strrep(&(AF_pse.app_name), std_pse.app_name);
				if(AF_pse.pin) strzfree(&(AF_pse.pin));
                                if(std_pse.pin) strzfree(&(std_pse.pin));

                                pse_sel = af_pse_open((ObjId *)0, FALSE);
				if ( ! pse_sel ) {
                                	fprintf(stderr, "Can't open PSE %s\n", AF_pse.app_name);
					goto close;
                                	break;
                                }
				if(af_check_if_onekeypaironly(&onekeypaironly)){
					aux_fprint_error(stderr, verbose);
					break;
				}
				ack(pse_sel, "opened");
				aux_free_PSESel(& pse_sel);
				strrep(&(std_pse.pin), AF_pse.pin);

				if(AF_pse.pin) {
					for(i = 0; i < PSE_MAXOBJ; i++) {
						strrep(&(AF_pse.object[i].pin), AF_pse.pin);
						if(!AF_pse.object[i].pin) {
							fprintf(stderr, "Can't allocate memory\n");
							exit(1);
						}
					}
				}
				else {
					for (i = 0; i < PSE_MAXOBJ; i++) 
						if(AF_pse.object[i].pin) strzfree(&(AF_pse.object[i].pin)); 
				}
                                break;
                        case OWNER:
				certificate = af_pse_get_Certificate(SIGNATURE, NULLDNAME, 0);
				if(!certificate){
					fprintf(stderr, "Can't read own certificate from PSE\n");
					break;
				}
				name = aux_DName2Name(certificate->tbs->subject);
				if(!name){
					fprintf(stderr, "Cannot transform DName into printable representation\n");
					break;
				}
				fprintf(stderr, "PSE Owner:  %s\n", name);
				aux_free_Certificate(&certificate);
				free(name);
                                break;
			case PROLONG:
				if(!cadir) {
					fprintf(stderr, "%s: This command is for CAs only\n", pgm_name);
					break;
				}
				dname = af_pse_get_Name();
				name = aux_DName2Name(dname);
				aux_free_DName(&dname);
				crlwithcerts = af_cadb_get_CRLWithCertificates(name, cadir);
				if(! crlwithcerts || ! crlwithcerts->crl){
					fprintf(stderr, "WARNING: Your own CRL is NOT stored in your local database!\n");
					break;
				}
				fprintf(stderr, "\nThis is your locally stored revocation list:\n\n");
				aux_fprint_CRL(stderr, crlwithcerts->crl);
				fprintf(stderr, "\n\nVerifying your locally stored CRL ...\n\n");
				certs = af_pse_get_Certificates(SIGNATURE, NULLDNAME); 
				rcode = af_verify(crlwithcerts->crl->tbs_DERcode, crlwithcerts->crl->sig, SEC_END, certs, (UTCTime * )0, (PKRoot * )0);
				aux_free_Certificates(&certs);
				aux_fprint_VerificationResult(stderr, verifresult);
				aux_free_VerificationResult(&verifresult);
				if (rcode == 0) {
					fprintf(stderr, "Verification of locally stored ");
					fprintf(stderr, "CRL s u c c e e d e d!\n\n");
				}
				else {
					fprintf(stderr, "WARNING: Verification of locally ");
					fprintf(stderr, "stored CRL f a i l e d!\n");
					break;
				}
				crlwithcerts->crl->tbs->lastUpdate = aux_current_UTCTime();
				crlwithcerts->crl->tbs->nextUpdate = (UTCTime *)0;
				crlwithcerts->crl->tbs->nextUpdate = get_nextUpdate(crlwithcerts->crl->tbs->lastUpdate);
				if ((crlwithcerts->crl->tbs_DERcode = e_CRLTBS(crlwithcerts->crl->tbs)) == NULLOCTETSTRING) {
					fprintf(stderr, "Can't encode crlwithcerts->crl->tbs\n");
					break;
				}

				fprintf(stderr, "\nThe following Crl is to be signed. ");
				fprintf(stderr, "Please check it:\n\n");
				aux_fprint_CRLTBS(stderr, crlwithcerts->crl->tbs);
				fprintf(stderr, "\nDo you want to sign the displayed ");
				fprintf(stderr, "revocation list ?\n");
				fprintf(stderr, "If you want to sign it, (re)enter the PIN of your chipcard:\n\n");
				af_pse_close(NULLOBJID);
				if ( af_sign(crlwithcerts->crl->tbs_DERcode, crlwithcerts->crl->sig, SEC_END) < 0 ) {
					fprintf(stderr, "Signature of revocation list failed\n");
					break;
				}

				/* Update on Directory entry, PSE, and CA directory: */
#ifdef X500
				if (x500 && af_access_directory == TRUE) {
					fprintf(stderr, "\n**********************************************\n");
					/* update X.500 directory entry */
					fprintf(stderr, "\nTrying to update your X.500 directory entry ...");
					if ( af_dir_enter_CRL(crlwithcerts->crl) < 0 ) 
						fprintf(stderr, "\n Directory entry (X.500) f a i l e d !\n");
					else fprintf(stderr, "\n Done!\n");
					fprintf(stderr, "\n**********************************************\n");
				}
#endif
#ifdef AFDBFILE
				if (af_access_directory == TRUE) {
					fprintf(stderr, "\n**********************************************\n");
					/* update .af-db directory entry */
					fprintf(stderr, "\nTrying to update your .af-db directory entry ...");
					if ( af_afdb_enter_CRL(crlwithcerts->crl) < 0 ) 
						fprintf(stderr, "\n Directory entry (.af-db) f a i l e d !\n");
					else fprintf(stderr, "\n Done!\n");
					fprintf(stderr, "\n**********************************************\n");
				}
#endif

				/* update PSE object CrlSet, even if the directory entry failed */
				crlpse = CRL2Crl (crlwithcerts->crl);
				fprintf(stderr, "\nUpdating PSE object CrlSet ...\n");
				rcode = af_pse_add_CRL(crlpse);
				if (rcode != 0) {
					fprintf(stderr, "\n Cannot update PSE object CrlSet.\n");
					aux_free_Crl (&crlpse);
				}
				else fprintf(stderr, "\n Done!\n");
				aux_free_Crl (&crlpse);
				fprintf(stderr, "\n**********************************************\n");

				/* update 'crlwithcerts' database in CA directory, even if the directory entry failed */
				fprintf(stderr, "\nUpdating 'crlwithcerts' database in CA directory \"%s\" ...\n", cadir);
				if(*cadir != PATH_SEPARATION_CHAR) {
					strcpy(calogfile, getenv("HOME"));
					strcat(calogfile, PATH_SEPARATION_STRING);
					strcat(calogfile, cadir);
				}
				else strcpy(calogfile, cadir);
				strcat(calogfile, PATH_SEPARATION_STRING);
				strcat(calogfile, "calog");
				logfile = fopen(calogfile, LOGFLAGS);
				if(logfile == (FILE * ) 0) {
					fprintf(stderr, "%s: Can't open %s\n", pgm_name, CALOG);
					break;
				}
				rcode = af_cadb_add_CRLWithCertificates(crlwithcerts, cadir);
				if(rcode != 0){
					fprintf(stderr, "%s: ",cmd);
					fprintf(stderr, "Cannot store your updated CRL in your 'crlwithcerts' database!\n");
					aux_fprint_error(stderr, verbose);
					aux_free_CRLWithCertificates(&crlwithcerts);
					break;
				}
				fprintf(stderr, "\nMost current version of CRL stored in 'crlwithcerts' database in ");
				fprintf(stderr, "CA directory \"%s\".\n\n", cadir);
				aux_free_CRLWithCertificates(&crlwithcerts);
				fclose(logfile);
				logfile = (FILE * )0;
				break;
			case PROTOTYPE:
				certificate = af_pse_get_Certificate(SIGNATURE, NULLDNAME, 0);
				if(! certificate){
					fprintf(stderr,"Can't read own certificate from PSE\n");
					break;
				}
                                if(!(key = build_key("Self-signed prototype certificate to", 1)))  {
					fprintf(stderr,"Can't build key\n");
					aux_free_Certificate(&certificate);
					break;
				}
				aux_free_DName(&certificate->tbs->issuer);
				certificate->tbs->issuer = af_pse_get_Name();
				if (certificate->tbs->valid->notbefore) {
					free(certificate->tbs->valid->notbefore);
					certificate->tbs->valid->notbefore = CNULL;
				}
				if (certificate->tbs->valid->notafter) {
					free(certificate->tbs->valid->notafter);
					certificate->tbs->valid->notafter = CNULL;
				}
				if (! notbefore) {
					certificate->tbs->valid->notbefore = aux_current_UTCTime();
					certificate->tbs->valid->notafter = aux_delta_UTCTime(certificate->tbs->valid->notbefore);
				}
				else {
					certificate->tbs->valid->notbefore = (UTCTime *)malloc(TIMELEN);
					strcpy(certificate->tbs->valid->notbefore, notbefore);
					free(notbefore);
					certificate->tbs->valid->notafter = (UTCTime *)malloc(TIMELEN);
					strcpy(certificate->tbs->valid->notafter, notafter);
					free(notafter);
				}
				certificate->tbs->serialnumber = aux_new_OctetString(1);
				certificate->tbs->serialnumber->octets[0] = 0x00;
				certificate->tbs->version = 0;           /* default version */
				aux_free_OctetString(&certificate->tbs_DERcode);
				aux_free_Signature(&certificate->sig);
				certificate->sig = (Signature * )malloc(sizeof(Signature));
 				if(! certificate->sig) {
					fprintf(stderr, "%s: ",pgm_name);
                			fprintf(stderr, "Can't allocate memory\n");
					break;
				}
				certificate->sig->signAI = af_pse_get_signAI();
				if (! certificate->sig->signAI) {
					fprintf(stderr, "%s: ",pgm_name);
                			fprintf(stderr, "Cannot determine the algorithm associated to your own secret signature key\n");
					break;
				}
				if (aux_ObjId2AlgType(certificate->sig->signAI->objid) == ASYM_ENC )
					certificate->sig->signAI = aux_cpy_AlgId(algorithm);
				certificate->tbs->signatureAI = aux_cpy_AlgId(certificate->sig->signAI);
				certificate->tbs_DERcode = e_ToBeSigned(certificate->tbs);
				if (! certificate->tbs_DERcode || (af_sign(certificate->tbs_DERcode, certificate->sig, SEC_END) < 0)) {
					fprintf(stderr, "%s: ",pgm_name);
                			fprintf(stderr, "AF Error with CA Signature\n");
					LOGAFERR;
					break;
				}
                                if(key->pse_sel) {
                                        if(!(ostr = e_Certificate(certificate)))  {
						fprintf(stderr,"Can't encode new Certificate\n");
						aux_free_Certificate(&certificate);
  						break;
					}
					aux_free_Certificate(&certificate);
					if(onekeypaironly == TRUE)
						oid = af_get_objoid(Cert_name);
					else {
                                        	keytype();
                                        	if(*objtype == 'S') oid = af_get_objoid(SignCert_name);
                                        	else if(*objtype == 'E') oid = af_get_objoid(EncCert_name);
                                        	else {
                                                	fprintf(stderr, "Type must me either 'S' or 'E'\n");
                                                	break;
                                        	}
					}
                                        if(sec_write_PSE(key->pse_sel, oid, ostr) < 0)  {
						fprintf(stderr,"Can't write to PSE\n");
						aux_free_OctetString(&ostr);
 						break;
					}
                                        fprintf(stderr, "Self-signed prototype-certificate stored in object %s\n", key->pse_sel->object.name);
					aux_free_OctetString(&ostr);
                                }
				break;
                        case READ:
                                psesel(2);
                                filen(CNULL);
                                ostr = &octetstring;
                                if((rcode = sec_read_PSE(&std_pse, &objecttype, ostr)) == 0) {
					store_objpin();
					if (aux_cmp_ObjId(&objecttype, Uid_OID) == 0) {
						tmp_ostr = aux_cpy_OctetString(ostr);
						ostr = d_OctetString(tmp_ostr);
						aux_free_OctetString(&tmp_ostr);
					}
                                        printfile(ostr, filename);
                                        free(ostr->octets);
                                } 
				else   {
					fprintf(stderr,"Can't read object  %s  from PSE\n", std_pse.object.name);
					break;
				}

                                break;
			case REMOVE:
				if (af_access_directory == FALSE) {
					fprintf(stderr, "af_access_directory is set to FALSE!\n");
					fprintf(stderr, "If you want to access the Directory, you should invoke ");
					fprintf(stderr, "the 'psemaint' command with the -D option.\n");
					break;
				}
				if(!af_x500 && !af_afdb){
					fprintf(stderr, "No directory flags (AFDBFILE or X500) have been compiled.\n");
					fprintf(stderr, "Therfore, no directory access is provided.\n");
					break;
				}
				attrtype = getattrtype(x500);
				if (!strncasecmp(attrtype, "Certificate", 2) || !strncasecmp(attrtype, "UserCertificate", 1) || !strncasecmp(attrtype, "CACertificate", 2)){

					if (!(dd = nxtpar("cert"))) {
						if ((serial = getserial()) == NULLOCTETSTRING) {
							fprintf(stderr, "No serial number specified!\n");
							break;
						}
						issuer_dn = getdname("Issuer");
					}
					else {
						if (!strchr( dd, ',' )) {
							serial = aux_create_SerialNo(dd);
							issuer_dn = NULLDNAME;
						}
						else {
							ptr = strchr(dd, ',');
							*ptr = '\0';
							ptr++;
							serial = aux_create_SerialNo(dd);
							dd = ptr;
							issuer_dn = aux_alias2DName(dd);
							if (!issuer_dn) {
								fprintf(stderr, "Cannot transform alias <%s> into a Distinguished Name!\n", dd);
								if(dd){
									free(dd);
									dd = CNULL;
								}
								break;
							}
						}
					}
#ifdef X500
					if(x500){
						if(!strncasecmp(attrtype, "UserCertificate", 1)) 
							certtype = userCertificate;
						else
							certtype = cACertificate;
	
						if ( af_dir_delete_Certificate(serial, issuer_dn, certtype)  == 0 )
							fprintf(stderr, "\nDirectory operation (X.500) succeeded.\n");
						       else {
							fprintf(stderr, "\nDirectory operation (X.500) f a i l e d.\n");
							aux_fprint_error(stderr, verbose);
						}
					}
#endif
#ifdef AFDBFILE
					if(onekeypaironly == TRUE)
						ktype = SIGNATURE;  /* ktype is not relevant in this case, but should have an acceptable value */
					else{
                                        	keytype();
                                        	if(*objtype == 'S') 
							ktype = SIGNATURE;
                                        	else if (*objtype == 'E') 
							ktype = ENCRYPTION;
                                        	else {
                                                	fprintf(stderr, "Type must me either 'S' or 'E'\n");
                                                	break;
                                        	}
					}
					if ( af_afdb_delete_Certificate(serial, issuer_dn, ktype) == 0 )
						fprintf(stderr, "\nDirectory operation (.af-db) succeeded.\n");
                               		else {
						fprintf(stderr, "\nDirectory operation (.af-db) f a i l e d.\n");
						aux_fprint_error(stderr, verbose);
					}
#endif
				}
				else {    /* attrtype = CrossCertificatePair */
					cpair = specify_CertificatePair();
					if (! cpair) {
						aux_fprint_error(stderr, verbose);
                               			fprintf(stderr, "%s: unable to create Cross Certificate Pair\n", parm[0]);
                 				break;
					}
					if (! cpair->forward && ! cpair->reverse) {
						fprintf(stderr, "\n");
						fprintf(stderr, "ERROR: At least one component (forward or reverse) must be present.\n");
						aux_free_CertificatePair(&cpair);
						break;
					}
#ifdef X500
					if ( x500 ) {
						if ( af_dir_delete_CertificatePair(cpair) == 0 )
							fprintf(stderr, "\nDirectory operation (X.500) succeeded.\n");
                               			else {
							fprintf(stderr, "\nDirectory operation (X.500) f a i l e d.\n");
							aux_fprint_error(stderr, verbose);
						}
					}
#endif
#ifdef AFDBFILE 	
					if ( af_afdb_delete_CertificatePair(cpair) == 0 )
						fprintf(stderr, "\nDirectory operation (.af-db) succeeded.\n");
                               		else {
						fprintf(stderr, "\nDirectory operation (.af-db) f a i l e d.\n");
						aux_fprint_error(stderr, verbose);
					}
#endif
					aux_free_CertificatePair(&cpair);
				}
				break;
                        case RENAME:
                                psesel(2);
                                if(!(newname = nxtpar("newname"))) {
                                        while(!newname) {
                                                fprintf(stderr, "Enter new name of object on %s: ", std_pse.app_name);
                                                newname = gets((char *)malloc(128));
						if( !newname ) {
							fprintf(stderr, "Can't allocate memory\n");
							break;
						}
                                                if(strlen(newname) == 0) {
                                                        free(newname);
                                                        newname = CNULL;
                                                }
                                        }
                                }
				if(newname) {
                                	if((rcode = af_pse_rename(std_pse.object.name, newname)) < 0) {
						fprintf(stderr, "Can't rename %s\n", std_pse.object.name);
                                        	break;
                                	}
					free(newname);
				}
                                break;
			case RETRIEVE:
				if (af_access_directory == FALSE) {
					fprintf(stderr, "af_access_directory is set to FALSE!\n");
					fprintf(stderr, "If you want to access the Directory, you should invoke ");
					fprintf(stderr, "the 'psemaint' command with the -D option.\n");
					break;
				}
				if(!af_x500 && !af_afdb){
					fprintf(stderr, "No directory flags (AFDBFILE or X500) have been compiled.\n");
					fprintf(stderr, "Therfore, no directory access is provided.\n");
					break;
				}
				dname = getdname(CNULL);
				if(!dname) break;
				if ((dd = nxtpar("update"))) {
					if (!strcmp(dd,"TRUE")) update = TRUE;
					else if (!strcmp(dd,"FALSE")) update = FALSE;
					else {
						fprintf(stderr, "Update must be either 'TRUE' or 'FALSE'\n");
						break;
					}
				}
				else update = FALSE;
				attrtype = getattrtype(x500);
				if (!strncasecmp(attrtype, "Certificate", 2) || !strncasecmp(attrtype, "UserCertificate", 1) || !strncasecmp(attrtype, "CACertificate", 2)){
#ifdef X500
					if ( x500 ) {
						if(!strncasecmp(attrtype, "UserCertificate", 1)) 
							certtype = userCertificate;
                                        	else
							certtype = cACertificate;

						fprintf(stderr, "\nAccessing the X.500 directory entry of \"%s\" ...\n", aux_DName2Name(dname));
						certset = af_dir_retrieve_Certificate(dname,certtype);
					}
#endif
#ifdef AFDBFILE	
					if (!x500 || !af_x500) {
						keytype();
						if(*objtype == 'S') ktype = SIGNATURE;
						else if(*objtype == 'E') ktype = ENCRYPTION;
						else {
							fprintf(stderr, "Type must me either 'S' or 'E'\n");
							break;
						}
						fprintf(stderr, "\nAccessing the .af-db directory entry of \"%s\" ...\n", aux_DName2Name(dname));			
						certset = af_afdb_retrieve_Certificate(dname,ktype);
					}
#endif
					aux_fprint_CertificateSet(stderr, certset);
					if(!certset) {
						fprintf(stderr, "No certificates returned from Directory.\n");
						break;
					}

					if (update == FALSE) break;

					if (certset->next) {
						fprintf(stderr, "\nSpecify the certificate (within the returned");
						fprintf(stderr, " SET_OF_Certificate) whose DER-code is needed:\n");
						if ((serial = getserial()) == NULLOCTETSTRING) {
							fprintf(stderr, "No serial number specified!\n");
							break;
						}

						/* examine if there is more than one certificate with 
					           the specified serial number: */

						soc = (SET_OF_Certificate *)0;
						while (certset) {
							if (!aux_cmp_OctetString(certset->element->tbs->serialnumber, serial)) {
								if (!soc) {
									soc = (SET_OF_Certificate *)malloc(sizeof(SET_OF_Certificate));
									tmp_soc = soc;
								}
								else {
									tmp_soc->next = (SET_OF_Certificate *)malloc(sizeof(SET_OF_Certificate));
									tmp_soc = tmp_soc->next;
								}
								tmp_soc->element = certset->element;
								tmp_soc->next = (SET_OF_Certificate *)0;	
							}			
							certset = certset->next;
						}
			
						if (!soc) {
							fprintf(stderr,"No such serial number!\n");
							break;
						}

						if (soc->next) { /*more than one certificate with 
							                  specified serial number*/
							issuer_dn = getdname("Issuer");
							if(!issuer_dn) break;
							found = 0;
							while ( soc && !found ) {
								if (!aux_cmp_DName(soc->element->tbs->issuer,issuer_dn)) {
									found = 1;
									certificate = aux_cpy_Certificate(soc->element);
									break;
								}
								soc = soc->next;
							}
							if ( !found ) {
								fprintf(stderr, "The specified certificate does not exist ");
								fprintf(stderr, "in the returned SET_OF_Certificate !\n");
								break;
							}
						}
						else certificate = soc->element;	
					}
					else certificate = certset->element;

					store_certificate(certificate, onekeypaironly);
				}
				else if (!strncasecmp(attrtype,"CrossCertificatePair", 2)) {
#ifdef X500
					if ( x500 ) {
						fprintf(stderr, "\nAccessing the X.500 directory entry of \"%s\" ...\n", aux_DName2Name(dname));
						cpairset = af_dir_retrieve_CertificatePair(dname);
					}
#endif
#ifdef AFDBFILE
					if (!x500 || !af_x500) {
						fprintf(stderr, "\nAccessing the .af-db directory entry of \"%s\" ...\n", aux_DName2Name(dname));
						cpairset = af_afdb_retrieve_CertificatePair(dname);
					}
#endif
					aux_fprint_CertificatePairSet(stderr, cpairset);
					if(! cpairset) {
						fprintf(stderr, "No SET OF Cross Certificate Pairs returned from Directory.\n");
						break;
					}

					if (update == TRUE) {
						rcode = 0;
						rcode = af_pse_add_CertificatePairSet(cpairset);
						if (!rcode)
							fprintf(stderr, "\nUpdate done on object CrossCSet.\n");
						else
							fprintf(stderr, "\nNo update done on object CrossCSet.\n");
					}
				}
				else {
#ifdef X500
					if ( x500 ) {
						fprintf(stderr, "\nAccessing the X.500 directory entry of \"%s\" ...\n", aux_DName2Name(dname));
						crl = af_dir_retrieve_CRL(dname);
					}
#endif
#ifdef AFDBFILE
					if (!x500 || !af_x500) {
						fprintf(stderr, "\nAccessing the .af-db directory entry of \"%s\" ...\n", aux_DName2Name(dname));
						crl = af_afdb_retrieve_CRL(dname);
					}
#endif
					fprintf(stderr, "\n");
					aux_fprint_CRL(stderr, crl);
					if(! crl) {
						fprintf(stderr, "No revocation list returned from Directory.\n");
						break;
					}
				}
				if (strncasecmp(attrtype,"RevocationList", 1)) break;

				/* Verifying the returned revocation list: */

				fprintf(stderr, "\nVerifying the returned revocation list ...\n\n");

				own_dname = af_pse_get_Name();
				if (!aux_cmp_DName(dname, own_dname)) {
					certs = af_pse_get_Certificates(SIGNATURE, NULLDNAME);
					rcode = af_verify(crl->tbs_DERcode, crl->sig, SEC_END, certs, (UTCTime * )0, (PKRoot * )0);
					aux_free_Certificates(&certs);
					aux_fprint_VerificationResult(stderr, verifresult);
					aux_free_VerificationResult(&verifresult);
				}
				else {
					if ( !(tbs = af_pse_get_TBS(SIGNATURE, dname, NULLDNAME, 0))) {
#ifdef X500
						if(x500) {
 							if(!(certset = af_dir_retrieve_Certificate(dname, cACertificate)) ) {
								fprintf(stderr, "Verification f a i l e d\n");
                        	                    	        break;
							}
						}
#endif
#ifdef AFDBFILE
						if(!x500 || !af_x500) {
							ktype = SIGNATURE;
	 						if(!(certset = af_afdb_retrieve_Certificate(dname,ktype)) ) {
								fprintf(stderr, "Verification f a i l e d\n");
	                                             	        break;
							}
						}
#endif
					}
  
					pkroot = (PKRoot *)malloc(sizeof(PKRoot));
					if( !pkroot ) {
						fprintf(stderr, "Can't allocate memory\n");
						break;
					}
					pkroot->oldkey = (struct Serial *)0;
					pkroot->newkey = (struct Serial *)malloc(sizeof(struct Serial));

					if (tbs) {
						pkroot->ca = aux_cpy_DName(tbs->subject);
						pkroot->newkey->key = aux_cpy_KeyInfo(tbs->subjectPK);
						pkroot->newkey->serial = aux_cpy_OctetString(tbs->serialnumber);
						pkroot->newkey->valid = aux_cpy_Validity(tbs->valid);
						rcode = af_verify(crl->tbs_DERcode, crl->sig, SEC_END, (Certificates *)0, (UTCTime * )0, pkroot);
						aux_free_PKRoot(&pkroot);
						aux_fprint_VerificationResult(stderr, verifresult);
						aux_free_VerificationResult(&verifresult);
						if (rcode == 0) fprintf(stderr, "Verification s u c c e e d e d\n");
						else{
							fprintf(stderr, "Verification f a i l e d\n");
							break;
						}
					}
					else {		
						while (certset) {
							/* compare, if ENCRYPTION or SIGNATURE object identifier: */
							algtype = aux_ObjId2AlgType(certset->element->tbs->subjectPK->subjectAI->objid);
							if ((algtype == SIG) || (algtype == ASYM_ENC)){
								pkroot->ca = aux_cpy_DName(certset->element->tbs->subject);
								pkroot->newkey->key = aux_cpy_KeyInfo(certset->element->tbs->subjectPK);
								pkroot->newkey->serial = aux_cpy_OctetString(certset->element->tbs->serialnumber);
								pkroot->newkey->valid = aux_cpy_Validity(certset->element->tbs->valid);
								rcode = af_verify(crl->tbs_DERcode, crl->sig, SEC_END, (Certificates *)0, (UTCTime * )0, pkroot);
								aux_fprint_VerificationResult(stderr, verifresult);
								aux_free_VerificationResult(&verifresult);
								if (rcode == 0){
									fprintf(stderr, "Verification s u c c e e d e d\n");
									aux_free_PKRoot(&pkroot);
									break;
								}
								aux_free_DName(&pkroot->ca);
								aux_free_KeyInfo(&pkroot->newkey->key);
							};
							certset = certset->next;
						} /*while*/
						if (! certset) {
							fprintf(stderr, "Verification f a i l e d\n");
                                               		break;
						}
					}
				}
				if (update == TRUE) {
					crlpse = CRL2Crl(crl);
					rcode = af_pse_add_CRL(crlpse);
					if (rcode == 0)
						fprintf(stderr, "\nInstallation of revocation list on PSE s u c c e e d e d\n");
					else {
						fprintf(stderr, "\nInstallation of revocation list on PSE f a i l e d\n");
					}
				}
				break;	
			case REVOKE:
				if(!cadir) {
					fprintf(stderr, "%s: This command is for CAs only\n", pgm_name);
					break;
				}
				dname = af_pse_get_Name();
				name = aux_DName2Name(dname);
				aux_free_DName(&dname);
				crlwithcerts = af_cadb_get_CRLWithCertificates(name, cadir);
				if(! crlwithcerts || ! crlwithcerts->crl){
					fprintf(stderr, "WARNING: Your own CRL is NOT stored in your local database!\n");
					break;
				}
				fprintf(stderr, "\nThis is your locally stored revocation list:\n\n");
				aux_fprint_CRL(stderr, crlwithcerts->crl);
				fprintf(stderr, "\n\nVerifying your locally stored CRL ...\n\n");
				certs = af_pse_get_Certificates(SIGNATURE, NULLDNAME); 
				rcode = af_verify(crlwithcerts->crl->tbs_DERcode, crlwithcerts->crl->sig, SEC_END, certs, (UTCTime * )0, (PKRoot * )0);
				aux_free_Certificates(&certs);
				aux_fprint_VerificationResult(stderr, verifresult);
				aux_free_VerificationResult(&verifresult);
				if (rcode == 0) {
					fprintf(stderr, "Verification of locally stored ");
					fprintf(stderr, "CRL s u c c e e d e d!\n\n");
				}
				else {
					fprintf(stderr, "WARNING: Verification of locally ");
					fprintf(stderr, "stored CRL f a i l e d!\n");
					break;
				}
				xx = "y";
				update = 0;
				while (strcmp(xx, "n")) {
					free (xx);
					xx = CNULL;
					fprintf(stderr, "\nEnter serial number of certificate which ");
					fprintf(stderr, "is to be revoked:\n");
					serial = getserial();
					i = 0;
					while (!serial && i < 3) {
						fprintf(stderr, "Serial number must be a positive integer!\n");
						serial = getserial();
						i++;
					}
					if (i == 3) break;
					certificate = af_cadb_get_Certificate(serial, cadir);
					if (!certificate) {
						fprintf(stderr, "\nNo certificate with serial number ");
						aux_fprint_Serialnumber(stderr, serial);
						fprintf(stderr, "in CA database!\n");
						fprintf(stderr, "\nNew choice? [y/n]: ");
					}
					else {	
						crlentry = af_create_CRLEntry(serial);
						if (!af_search_CRLEntry(crlwithcerts->crl, crlentry)) { 
							fprintf(stderr, "\nThe following certificate with serial number ");
							aux_fprint_Serialnumber(stderr, serial);
							fprintf(stderr, "is being revoked:\n\n");
							aux_fprint_Certificate(stderr, certificate);
							crlentryseq = (SEQUENCE_OF_CRLEntry * )malloc(sizeof(SEQUENCE_OF_CRLEntry));
							if (!crlentryseq) {
								fprintf(stderr, "Can't allocate memory\n");
								aux_free_CRLEntry(&crlentry);
								aux_free_CRLWithCertificates(&crlwithcerts);
								break;
							}

							crlentryseq->element = aux_cpy_CRLEntry(crlentry);
							aux_free_CRLEntry(&crlentry);

							crlentryseq->next = crlwithcerts->crl->tbs->revokedCertificates;
							/* existing or NULL pointer */

							crlwithcerts->crl->tbs->revokedCertificates = crlentryseq;
							update = 1;
							fprintf(stderr, "\nMore certificates to be revoked? [y/n]: ");
						}
						else {
							fprintf(stderr, "\nCertificate with serial number ");
							aux_fprint_Serialnumber(stderr, serial);
							fprintf(stderr, "already revoked !\n");
							fprintf(stderr, "\nNew choice? [y/n]: ");
						}
					}
					gets(inp);
					xx = inp;
					while ( strcmp(xx, "y") && strcmp(xx, "n") ) {
						fprintf(stderr, "\nAnswer must be 'y' or 'n' !\n\n");
						fprintf(stderr, "\nNew choice? [y/n]: ");
						gets(inp);
						xx = inp;
					}
				}  /*while*/
		
				if (!update) {
					fprintf(stderr, "No update done on revocation list!\n");
					aux_free_CRLWithCertificates(&crlwithcerts);
					break;
				}

				if (create == FALSE) {
					crlwithcerts->crl->tbs->lastUpdate = aux_current_UTCTime();
					crlwithcerts->crl->tbs->nextUpdate = (UTCTime *)0;
					crlwithcerts->crl->tbs->nextUpdate = get_nextUpdate(crlwithcerts->crl->tbs->lastUpdate);
				}

				if ((crlwithcerts->crl->tbs_DERcode = e_CRLTBS(crlwithcerts->crl->tbs)) == NULLOCTETSTRING) {
					fprintf(stderr, "Can't encode crlwithcerts->crl->tbs\n");
					break;
				}

				fprintf(stderr, "\nThe following Crl is to be signed. ");
				fprintf(stderr, "Please check it:\n\n");
				aux_fprint_CRLTBS(stderr, crlwithcerts->crl->tbs);
				fprintf(stderr, "\nDo you want to sign the displayed ");
				fprintf(stderr, "revocation list ?\n");
				fprintf(stderr, "If you want to sign it, (re)enter the PIN of your chipcard:\n\n");
				af_pse_close(NULLOBJID);
				if ( af_sign(crlwithcerts->crl->tbs_DERcode, crlwithcerts->crl->sig, SEC_END) < 0 ) {
					fprintf(stderr, "Signature of revocation list failed\n");
					break;
				}

				/* Update on Directory entry, PSE, and CA directory: */
#ifdef X500
				if (x500 && af_access_directory == TRUE) {
					fprintf(stderr, "\n**********************************************\n");
					/* update X.500 directory entry */
					fprintf(stderr, "\nTrying to update your X.500 directory entry ...");
					if ( af_dir_enter_CRL(crlwithcerts->crl) < 0 ) 
						fprintf(stderr, "\n Directory entry (X.500) f a i l e d !\n");
					else fprintf(stderr, "\n Done!\n");
					fprintf(stderr, "\n**********************************************\n");
				}
#endif
#ifdef AFDBFILE
				if (af_access_directory == TRUE) {
					fprintf(stderr, "\n**********************************************\n");
					/* update .af-db directory entry */
					fprintf(stderr, "\nTrying to update your .af-db directory entry ...");
					if ( af_afdb_enter_CRL(crlwithcerts->crl) < 0 ) 
						fprintf(stderr, "\n Directory entry (.af-db) f a i l e d !\n");
					else fprintf(stderr, "\n Done!\n");
					fprintf(stderr, "\n**********************************************\n");
				}
#endif

				/* update PSE object CrlSet, even if the directory entry failed */
				crlpse = CRL2Crl (crlwithcerts->crl);
				fprintf(stderr, "\nUpdating PSE object CrlSet ...\n");
				rcode = af_pse_add_CRL(crlpse);
				if (rcode != 0) {
					fprintf(stderr, "\n Cannot update PSE object CrlSet.\n");
					aux_free_Crl (&crlpse);
				}
				else fprintf(stderr, "\n Done!\n");
				aux_free_Crl (&crlpse);
				fprintf(stderr, "\n**********************************************\n");

				/* update 'crlwithcerts' database in CA directory, even if the directory entry failed */
				fprintf(stderr, "\nUpdating 'crlwithcerts' database in CA directory \"%s\" ...\n", cadir);
				if(*cadir != PATH_SEPARATION_CHAR) {
					strcpy(calogfile, getenv("HOME"));
					strcat(calogfile, PATH_SEPARATION_STRING);
					strcat(calogfile, cadir);
				}
				else strcpy(calogfile, cadir);
				strcat(calogfile, PATH_SEPARATION_STRING);
				strcat(calogfile, "calog");
				logfile = fopen(calogfile, LOGFLAGS);
				if(logfile == (FILE * ) 0) {
					fprintf(stderr, "%s: Can't open %s\n", pgm_name, CALOG);
					break;
				}
				rcode = af_cadb_add_CRLWithCertificates(crlwithcerts, cadir);
				if(rcode != 0){
					fprintf(stderr, "%s: ",cmd);
					fprintf(stderr, "Cannot store your updated CRL in your 'crlwithcerts' database!\n");
					aux_fprint_error(stderr, verbose);
					aux_free_CRLWithCertificates(&crlwithcerts);
					break;
				}
				fprintf(stderr, "\nMost current version of CRL stored in 'crlwithcerts' database in ");
				fprintf(stderr, "CA directory \"%s\".\n\n", cadir);
				aux_free_CRLWithCertificates(&crlwithcerts);
				fclose(logfile);
				logfile = (FILE * )0;
				break;
                        case SETPARM:
				a = alglist;
				algid = (AlgId *)0;
                                strrep(&algname, getalgname());
				while (a->name) {
					if (strcmp(algname, a->name) == 0) {
						algid = a->algid;
						break;
					}
					a++;
				}

                                if(algid) switch(aux_Name2ParmType(algname)) {
                                        case PARM_INTEGER:
                                                rsaparm = (rsa_parm_type *)(algid->param);
                                                *rsaparm = getsize("keysize");
                                                break;
                                        case PARM_OctetString:
                                                filen("File: ");
                                                if(filename) ostr = aux_file2OctetString(filename);
                                                if(ostr) algid->param = (char *)ostr;
                                                break;
					default:
						if(aux_Name2AlgEnc(algname) == DSA) sec_dsa_keysize = getsize("keysize");
						else fprintf(stderr, "algorithm has no parameter\n");
                                }
                                break;
			case SETSERIAL:
				serial = getserial();
				psesel_zw = af_pse_open(SerialNumber_OID, TRUE);
				if(psesel_zw) aux_free_PSESel(&psesel_zw);
				af_pse_update_SerialNumber(serial);
				break;
                        case SHOW:
                                if(!(key = build_key("", 0))) break;
                                print_keyinfo_flag = ALGID;
                                print_cert_flag = TBS | ALG;
                                opt = 0;
                                while((cc = nxtpar(CNULL))) {
                                        if(!strncasecmp(cc, "TBS", 3)) opt |= TBS;
                                        else if(!strncasecmp(cc, "HAS", 3)) opt |= HSH;
                                        else if(!strncasecmp(cc, "VER", 3)) print_cert_flag |= VER;
                                        else if(!strncasecmp(cc, "ALG", 3)) opt |= ALG;
                                        else if(!strncasecmp(cc, "VAL", 3)) opt |= VAL;
                                        else if(!strncasecmp(cc, "DER", 3)) opt |= DER;
                                        else if(!strncasecmp(cc, "SIG", 3)) opt |= SIGNAT | ALG;
                                        else if(!strncasecmp(cc, "KEY", 3)) opt |= KEYINFO;
                                        else if(!strncasecmp(cc, "ISS", 3)) opt |= ISSUER;
                                        else if(!strncasecmp(cc, "SUB", 3)) opt |= SUBJECT;
                                        else if(!strncasecmp(cc, "BIT", 3)) print_keyinfo_flag |= BSTR;
                                        else if(!strncasecmp(cc, "COM", 3)) print_keyinfo_flag |= KEYBITS;
                                        else if(!strncasecmp(cc, "ALL", 3)) {
						print_keyinfo_flag |= KEYBITS;
						print_cert_flag |= SIGNAT | VER;
					}
					free(cc);
                                }
                                if(opt && opt != DER) print_cert_flag = opt;
                                else print_cert_flag |= opt;
                                if(key->pse_sel) {
					objname = key->pse_sel->object.name;
                                        if(strcmp(objname, SerialNumber_name) == 0){
                                                if(! (serial = af_pse_get_SerialNumber())) goto noobject;
						aux_fprint_Serialnumber(stderr, serial);
						aux_free_OctetString(&serial);
                                        }
                                        else if(strcmp(objname, SignCert_name) == 0
					   || strcmp(objname, Cert_name) == 0) { 
                                                if(!(certificate = af_pse_get_Certificate(SIGNATURE, (DName *)0, (OctetString *)0))) goto noobject;
                                                print_keyinfo_flag |= PK;
						print_indent += 4;
                                                aux_fprint_Certificate(stderr, certificate);
						print_indent -= 4;
                                                aux_free_Certificate(&certificate);
                                        }
                                        else if(strcmp(objname, EncCert_name) == 0) {
                                                if(!(certificate = af_pse_get_Certificate(ENCRYPTION, (DName *)0, (OctetString *)0))) goto noobject;
                                                print_keyinfo_flag |= PK;
						print_indent += 4;
                                                aux_fprint_Certificate(stderr, certificate);
						print_indent -= 4;
                                                aux_free_Certificate(&certificate);
                                        }
                                        else if(strcmp(objname, SignCSet_name) == 0
					   || strcmp(objname, CSet_name) == 0) { 
                                                if(!(certset = af_pse_get_CertificateSet(SIGNATURE))) goto noobject;
                                                print_keyinfo_flag |= PK;
                                                aux_fprint_CertificateSet(stderr, certset);
                                                aux_free_CertificateSet(&certset);
                                        }
                                        else if(strcmp(objname, EncCSet_name) == 0) {
                                                if(!(certset = af_pse_get_CertificateSet(ENCRYPTION))) goto noobject;
                                                print_keyinfo_flag |= PK;
                                                aux_fprint_CertificateSet(stderr, certset);
                                                aux_free_CertificateSet(&certset);
                                        }
                                        else if(strcmp(objname, FCPath_name) == 0) {
                                                if(!(fcpath = af_pse_get_FCPath((DName *)0))) goto noobject;
                                                print_keyinfo_flag |= PK;
                                                aux_fprint_FCPath(stderr, fcpath);
                                                aux_free_FCPath(&fcpath);
                                        }
                                        else if(strcmp(objname, PKRoot_name) == 0) {
                                                if(!(pkroot = af_pse_get_PKRoot())) goto noobject;
                                                print_keyinfo_flag |= PK;
						print_indent += 4;
                                                aux_fprint_PKRoot(stderr, pkroot);
						print_indent -= 4;
						aux_free_PKRoot(&pkroot);
                                        }
                                        else if(strcmp(objname, PKList_name) == 0) {
                                                if(!(pklist = af_pse_get_PKList(SIGNATURE))) goto noobject;
                                                print_keyinfo_flag |= PK;
                                                aux_fprint_PKList(stderr, pklist);
						aux_free_PKList(&pklist);
                                        }
                                        else if(strcmp(objname, EKList_name) == 0) {
                                                if(!(pklist = af_pse_get_PKList(ENCRYPTION))) goto noobject;
                                                print_keyinfo_flag |= PK;
                                                aux_fprint_PKList(stderr, pklist);
						aux_free_PKList(&pklist);
                                        }
                                        else if(strcmp(objname, PCAList_name) == 0) {
                                                if(!(pklist = af_pse_get_PCAList())) goto noobject;
                                                print_keyinfo_flag |= PK;
                                                aux_fprint_PKList(stderr, pklist);
						aux_free_PKList(&pklist);
                                        }
                                        else if(strcmp(objname, CrossCSet_name) == 0) {
                                                if(!(cpairset = af_pse_get_CertificatePairSet())) goto noobject;
                                                print_keyinfo_flag |= PK;
                                                aux_fprint_CertificatePairSet(stderr, cpairset);
						aux_free_CertificatePairSet(&cpairset);
                                        }
                                        else if(strcmp(objname, CrlSet_name) == 0) {
                                                if(!(crlset = af_pse_get_CrlSet())) goto noobject;
                                                aux_fprint_CrlSet(stderr, crlset);
                                                aux_free_CrlSet(&crlset);
                                        }
                                        else if(strcmp(objname, AliasList_name) == 0) {
                                                if(!(aliaslist = af_pse_get_AliasList())) goto noobject;
                                                aux_fprint_AliasList(stderr, aliaslist);
                                                aux_free_AliasList(&aliaslist);
                                        }
                                        else if(strcmp(objname, QuipuPWD_name) == 0) {
                                                if(!(dd = af_pse_get_QuipuPWD())) goto noobject;
                                                fprintf(stderr, "X.500 Password: %s\n", dd);
                                                if (dd) free(dd);
                                        }
                                        else if(strcmp(objname, DHparam_name) == 0) {
                                                if(!(algid = af_pse_get_DHparam())) goto noobject;
						print_indent += 4;
                                                FPRINTF(stderr, "DHParameter:  ");
						aux_fprint_AlgId(stderr, algid);
						print_indent -= 4;
                                                aux_free_AlgId(&algid);;
                                        }
					else {
                                        	ostr = &octetstring;
                                        	if(sec_read_PSE(key->pse_sel, &object_oid, ostr) < 0)  {
							fprintf(stderr,"Can't read object  %s  from PSE\n", key->pse_sel->object.name);
							break;
						}
						if(aux_cmp_ObjId(&object_oid, SignSK_OID) == 0
						   || aux_cmp_ObjId(&object_oid, DecSKnew_OID) == 0 
						   || aux_cmp_ObjId(&object_oid, DecSKold_OID) == 0
						   || aux_cmp_ObjId(&object_oid, SKnew_OID) == 0 
						   || aux_cmp_ObjId(&object_oid, RSA_SK_OID) == 0 
						   || aux_cmp_ObjId(&object_oid, DSA_SK_OID) == 0 
						   || aux_cmp_ObjId(&object_oid, SKold_OID) == 0) {
							if(!(keyinfo = d_KeyInfo(ostr))) goto decodeerr;
							print_indent += 4;
							FPRINTF(stderr, "Secret Key: ");
							print_keyinfo_flag |= SK;
							aux_fprint_KeyInfo(stderr, keyinfo);
							print_indent -= 4;
							aux_free_KeyInfo(&keyinfo);
						}
						else if(aux_cmp_ObjId(&object_oid, RSA_PK_OID) == 0
						   || aux_cmp_ObjId(&object_oid, DSA_PK_OID) == 0) {
							if(!(keyinfo = d_KeyInfo(ostr))) goto decodeerr;
							print_indent += 4;
							FPRINTF(stderr, "Public Key: ");
							print_keyinfo_flag |= PK;
							aux_fprint_KeyInfo(stderr, keyinfo);
							print_indent -= 4;
							aux_free_KeyInfo(&keyinfo);
						}
						else if(aux_cmp_ObjId(&object_oid, DHkey_OID) == 0) {
							bstr = d_BitString(ostr);
							print_indent += 4;
							FPRINTF(stderr, "DH agreed key: ");
							aux_fprint_BitString(stderr, bstr);
							print_indent -= 4;
							aux_free_BitString(&bstr);
						}
						else if(aux_cmp_ObjId(&object_oid, DHprivate_OID) == 0) {
							if(!(keyinfo = d_KeyInfo(ostr))) goto decodeerr;
							print_indent += 4;
							FPRINTF(stderr, "DHPrivateKey: ");
							print_keyinfo_flag |= SK;
							aux_fprint_KeyInfo(stderr, keyinfo);
							print_indent -= 4;
							aux_free_KeyInfo(&keyinfo);
						}
						else {
							fprintf(stderr, "Object OID { ");
							for(i = 0; i < object_oid.oid_nelem; i++) fprintf(stderr, "%d ", object_oid.oid_elements[i]);
							fprintf(stderr, " }\n");
	
							if((certificate = d_Certificate(ostr))) {
								print_keyinfo_flag |= PK;
								print_indent += 4;
								   aux_fprint_Certificate(stderr, certificate);
								print_indent -= 4;
								aux_free_Certificate(&certificate);
							}
							else if((dname = d_DName(ostr))) {
								if(!(name = aux_DName2Name(dname))) {
									       fprintf(stderr, "Can't build printable repr. of %s\n", key->pse_sel->object.name);
								}
								else fprintf(stderr, "%s\n", name);
								aux_free_DName(&dname);
							}
							else if((fcpath = d_FCPath(ostr))) {
								aux_fprint_FCPath(stderr, fcpath);
								aux_free_FCPath(&fcpath);
							}
							else if((pkroot = d_PKRoot(ostr))) {
								print_indent += 4;
								aux_fprint_PKRoot(stderr, pkroot);
								print_indent -= 4;
								aux_free_PKRoot(&pkroot);
							}
							else if((certset = d_CertificateSet(ostr))) {
								aux_fprint_CertificateSet(stderr, certset);
								aux_free_CertificateSet(&certset);
							}
							else if((pklist = d_PKList(ostr))) {
								aux_fprint_PKList(stderr, pklist);
								aux_free_PKList(&pklist);
							}
							else if((keyinfo = d_KeyInfo(ostr))) {
								print_indent += 4;
								FPRINTF(stderr, "PublicKey:    ");
								print_keyinfo_flag |= PK;
								aux_fprint_KeyInfo(stderr, keyinfo);
								print_indent -= 4;
								aux_free_KeyInfo(&keyinfo);
							}
							else if ((cpairset = d_CertificatePairSet(ostr))) {
								aux_fprint_CertificatePairSet(stderr, cpairset);
								aux_free_CertificatePairSet(&cpairset);
							}
							else if((crlset = d_CrlSet(ostr))) {
								aux_fprint_CrlSet(stderr, crlset);
								aux_free_CrlSet(&crlset);
							}
							else if((aliaslist = d_AliasList(ostr))) {
								aux_fprint_AliasList(stderr, aliaslist);
								aux_free_AliasList(&aliaslist);
							}
							else if((dd = d_GRAPHICString(ostr))) {
								fprintf(stderr, "X.500 Password: %s\n", dd);
								if (dd) free(dd);
							}
							else aux_xdump(ostr->octets, ostr->noctets, 0);
							if(ostr->octets) free(ostr->octets);
							aux_free2_ObjId(&object_oid);
						}
					}
                                        print_keyinfo_flag = ALGID;
                                        print_cert_flag = TBS | ALG | SIGNAT;
                                        break;
                                }
                                else {
                                        if((rcode = sec_get_key(&tmpkey, key->keyref, (Key *)0)) < 0)  {
						fprintf(stderr,"Can't get key\n");
						break;
					}
                                        fprintf(stderr, "    KeyAid: ");
                                        aux_fprint_KeyInfo(stderr, &tmpkey);
                                        print_keyinfo_flag = ALGID;
                                        print_cert_flag = TBS | ALG | SIGNAT;
					aux_free2_KeyInfo(&tmpkey);
                                        break;
                                }
decodeerr:
                                fprintf(stderr, "Can't decode %s\n", key->pse_sel->object.name);
				if(ostr->octets) free(ostr->octets);
                                break;
noobject:
                                fprintf(stderr, "PSE object %s doesn't exist\n", key->pse_sel->object.name);
                                break;

                        case SPLIT:
				cpairset = af_pse_get_CertificatePairSet();
				if (! cpairset) {
					fprintf(stderr,"No Cross Certificate Pairs stored in PSE\n");
					break;
				}
				cpair = specify_CertificatePair();
				if (! cpair) {
					aux_fprint_error(stderr, verbose);
                               		fprintf(stderr, "%s: unable to create Cross Certificate Pair\n", parm[0]);
                 			break;
				}
				if (! cpair->forward && ! cpair->reverse) {
					fprintf(stderr, "\n");
					fprintf(stderr, "ERROR: At least one component (forward or reverse) must be present.\n");
					aux_free_CertificatePair(&cpair);
					break;
				}
				while (cpairset) {
					if (! aux_cmp_CertificatePair(cpair, cpairset->element))
						break;
					cpairset = cpairset->next;
				}
				if (! cpairset) {
					fprintf(stderr, "Specified Cross Certificate Pair NOT found\n");
					break;
				}
				fprintf(stderr,"\n");
				if (cpairset->element && cpairset->element->forward) {
					if(!(key = build_key("F O R W A R D  certificate to", 1)))  {
						fprintf(stderr,"Cannot build key\n");
						aux_free_CertificatePairSet(&cpairset);
						break;
					}
					if(key->pse_sel) {
						if(!(ostr = e_Certificate(cpairset->element->forward)))  {
							fprintf(stderr,"Cannot encode forward certificate\n");
							aux_free_CertificatePairSet(&cpairset);
							aux_free_Key(&key);
							break;
						}
						if(onekeypaironly == TRUE)
							oid = af_get_objoid(Cert_name);
						else{
							keytype();
							if(*objtype == 'S') oid = af_get_objoid(SignCert_name);
							else if(*objtype == 'E') oid = af_get_objoid(EncCert_name);
							else {
								fprintf(stderr, "Type must me either 'S' or 'E'\n");	
								aux_free_OctetString(&ostr);
								aux_free_CertificatePairSet(&cpairset);
								aux_free_Key(&key);
								break;
							}
						}
						if(sec_write_PSE(key->pse_sel, oid, ostr) < 0)  {
							fprintf(stderr,"Can't write to PSE\n");
							aux_free_OctetString(&ostr);
							aux_free_CertificatePairSet(&cpairset);
							aux_free_Key(&key);
							break;
						}
						fprintf(stderr, "Forward stored in object %s\n", key->pse_sel->object.name);
						aux_free_OctetString(&ostr);
						aux_free_Key(&key);
					}
				}

				if (cpairset->element && cpairset->element->reverse) {
					if(!(key = build_key("R E V E R S E  certificate to", 1)))  {
						fprintf(stderr,"Cannot build key\n");
						aux_free_CertificatePairSet(&cpairset);
						break;
					}
					if(key->pse_sel) {
						if(!(ostr = e_Certificate(cpairset->element->reverse)))  {
							fprintf(stderr,"Cannot encode reverse certificate\n");
							aux_free_CertificatePairSet(&cpairset);
							aux_free_Key(&key);
							break;
						}
						if(onekeypaironly == TRUE)
							oid = af_get_objoid(Cert_name);
						else{
							keytype();
							if(*objtype == 'S') oid = af_get_objoid(SignCert_name);
							else if(*objtype == 'E') oid = af_get_objoid(EncCert_name);
							else {
								fprintf(stderr, "Type must me either 'S' or 'E'\n");	
								aux_free_OctetString(&ostr);
								aux_free_CertificatePairSet(&cpairset);
								aux_free_Key(&key);
								break;
							}
						}
						if(sec_write_PSE(key->pse_sel, oid, ostr) < 0)  {
							fprintf(stderr,"Can't write to PSE\n");
							aux_free_OctetString(&ostr);
							aux_free_CertificatePairSet(&cpairset);
							aux_free_Key(&key);
							break;
						}
						fprintf(stderr, "Forward stored in object %s\n", key->pse_sel->object.name);
						aux_free_OctetString(&ostr);
						aux_free_Key(&key);
					}
				}

				aux_free_CertificatePairSet(&cpairset);
                                break;
                        case STRING2KEY:
                                str2key();
                                if(!(key = build_key("", 1)))  {
					fprintf(stderr,"Can't build key\n");
					break;
				}
                                sec_string_to_key(pin, key);
                                fprintf(stderr, "DES key stored ");
                                if(key->pse_sel) fprintf(stderr, " in object %s\n", key->pse_sel->object.name);
                                else fprintf(stderr, "under keyref %d\n", key->keyref);
				aux_free_Key(&key);
                                break;
                        case TOC:
				rcode = sec_print_toc(stderr, &std_pse);
				if(rcode < 0) fprintf(stderr, "No PSE open\n");
                                break;
#ifdef SCA
                        case TOGGLE:
				SC_verify = 1 - SC_verify;
				SC_encrypt = 1 - SC_encrypt;
				if(SC_verify) fprintf(stderr, "Verification/Encryption in SCT\n");
				else fprintf(stderr, "Verification/Encryption with SW\n");
                                break;
#endif
			case VERBOSE:
				dd = nxtpar(CNULL);
				if(dd) i = atoi(dd);
				else i = -1;
				if(i >= 0 && i < 4) verbose = i;
				if(verbose < 2) af_verbose = sec_verbose = FALSE;
				else if(verbose == 2) {
					af_verbose = TRUE;
					sec_verbose = FALSE;
				}
				else af_verbose = sec_verbose = TRUE;
				fprintf(stderr, "Verbose level is %d\n", verbose);
				break;
                        case VERIFY:
                                ostr = &octetstring;

                                if((dd = nxtpar("certificate"))) {
					strrep(&(std_pse.object.name), dd);
					free(dd);

					if(sec_read_PSE(&std_pse, &object_oid, ostr) < 0) {
						fprintf(stderr,"Can't read object  %s  from PSE\n", std_pse.object.name);
						break;
					}
					store_objpin();
	
					if(aux_cmp_ObjId(&object_oid, SignCert_OID) && aux_cmp_ObjId(&object_oid, EncCert_OID) && aux_cmp_ObjId(&object_oid, Cert_OID)) { 
						fprintf(stderr, "%s is not a certificate\n", std_pse.object.name);
						free(ostr->octets);
						free(object_oid.oid_elements);
						break;
					}
					free(object_oid.oid_elements);
					if(!(certificate = d_Certificate(ostr))) {
						fprintf(stderr, "Can't decode %s\n", std_pse.object.name);
						free(ostr->octets);
						break;
					}
					free(ostr->octets);
				}
                                else certificate = af_pse_get_Certificate(SIGNATURE, (DName *)0, (OctetString *)0);
				if(!certificate) {
					fprintf(stderr, "Can't read certificate\n");
					break;
				}

				std_pse.object.name = CNULL;
                                if((dd = nxtpar("fcpath"))) {
					if(*dd != '-') strrep(&(std_pse.object.name), dd);
					free(dd);

					ostr = &octetstring;
					fcpath = (FCPath *)0;
					
					if(std_pse.object.name && strlen(std_pse.object.name)) {
						if(sec_read_PSE(&std_pse, &object_oid, ostr) == 0) {
							store_objpin();
							if(aux_cmp_ObjId(&object_oid, FCPath_OID) && 
							   aux_cmp_ObjId(&object_oid, SignCert_OID) && 
							   aux_cmp_ObjId(&object_oid, EncCert_OID) && 
							   aux_cmp_ObjId(&object_oid, Cert_OID)) { 
								fprintf(stderr, "%s is neither a FCPath nor a Certificate\n", std_pse.object.name);
								free(ostr->octets);
								free(object_oid.oid_elements);
								aux_free_Certificate(&certificate);
								break;
							}
							free(object_oid.oid_elements);
							if(!(fcpath = d_FCPath(ostr))) {
								if(!(fcpath_cert = d_Certificate(ostr))){
									fprintf(stderr, "Can't decode %s\n", std_pse.object.name);
									free(ostr->octets);
									aux_free_Certificate(&certificate);
										break;
								}
								free(ostr->octets);
								fcpath = aux_create_FCPath(fcpath_cert);
								if(! fcpath){
									fprintf(stderr, "Cannot transform Certificate into FCPath.\n");
									aux_free_Certificate(&fcpath_cert);
									break;
								}
							}
							else free(ostr->octets);
						}
						else{
							fprintf(stderr, "Can't read object  %s  from PSE\n", std_pse.object.name);
							break;
						}
					}
				}
				else fcpath = af_pse_get_FCPath((DName *)0);
                                certs = aux_create_Certificates(certificate, fcpath);
				if(!certs) {
					fprintf(stderr, "Can't allocate memory\n");
					aux_free_Certificate(&certificate);
					aux_free_FCPath(&fcpath);
					break;
				}
				aux_free_Certificate(&certificate);
				aux_free_FCPath(&fcpath);
                                if((dd = nxtpar("pkroot"))) {
                                        strrep(&(std_pse.object.name), dd);
					free(dd);
                                        ostr = &octetstring;
                                        if(sec_read_PSE(&std_pse, &object_oid, ostr) < 0)  {
        					fprintf(stderr,"Can't read object  %s  from PSE\n", std_pse.object.name);
        					break;
        				}
					store_objpin();
                                        if(aux_cmp_ObjId(&object_oid, PKRoot_OID)) { 
                                                fprintf(stderr, "%s is not a PKRoot\n", std_pse.object.name);
						free(object_oid.oid_elements);
                                                free(ostr->octets);
						aux_free_Certificates(&certs);
                                                break;
                                        }
					free(object_oid.oid_elements);
                                        if(!(pkroot = d_PKRoot(ostr))) {
                                                fprintf(stderr, "Can't decode %s\n", std_pse.object.name);
                                                free(ostr->octets);
                                                break;
                                        }
                                        free(ostr->octets);
                                }
                                else pkroot = (PKRoot *)0;
/*                                af_verbose = TRUE; */
                                print_keyinfo_flag = ALGID;

                                rcode = af_verify_Certificates(certs, (UTCTime *)0, pkroot);
				if(rcode < 0 ) {
					fprintf(stderr, "Verification  f a i l e d\n");
					aux_fprint_error(stderr, verbose);
				}
                                aux_free_Certificates(&certs);
                                if(pkroot) aux_free_PKRoot(&pkroot);
				aux_fprint_VerificationResult(stderr, verifresult);
				aux_free_VerificationResult(&verifresult);
                                break;
                        case WRITE:
                                psesel(2);
                                filen("File: ");
                                if(!(ostr = aux_file2OctetString(filename)))  {
					fprintf(stderr,"Can't read file\n");
					break;
				}
                                if((certificate = d_Certificate(ostr))) {
					if(onekeypaironly == TRUE)
						oid = af_get_objoid(Cert_name);
					else{
                                        	keytype();
                                        	if(*objtype == 'S') oid = af_get_objoid(SignCert_name);
                                        	else if(*objtype == 'E') oid = af_get_objoid(EncCert_name);
                                        	else {
                                                	fprintf(stderr, "Type must me either 'S' or 'E'\n");
                                                	break;
                                        	}
					}
                                        aux_free_Certificate(&certificate);
                                }
                                else if((fcpath = d_FCPath(ostr))) {
                                        oid = af_get_objoid(FCPath_name);
                                        aux_free_FCPath(&fcpath);
                                }
                                else if((pkroot = d_PKRoot(ostr))) {
                                        oid = af_get_objoid(PKRoot_name);
                                        aux_free_PKRoot(&pkroot);
                                }
                                else if((certset = d_CertificateSet(ostr))) {
					if(onekeypaironly == TRUE)
						oid = af_get_objoid(CSet_name);
					else{
                                        	keytype();
                                        	if(*objtype == 'S') oid = af_get_objoid(SignCSet_name);
                                        	else if(*objtype == 'E') oid = af_get_objoid(EncCSet_name);
                                        	else {
                                               		fprintf(stderr, "Type must me either 'S' or 'E'\n");
                                                	break;
                                        	}
					}
                                        aux_free_CertificateSet(&certset);
                                }
                                else if((pklist = d_PKList(ostr))) {
					if(onekeypaironly == TRUE)
						oid = af_get_objoid(PKList_name);
					else {
                                        	keytype();
                                        	if(*objtype == 'S') oid = af_get_objoid(PKList_name);
                                        	else if(*objtype == 'E') oid = af_get_objoid(EKList_name);
                                        	else {
                                                	fprintf(stderr, "Type must me either 'S' or 'E'\n");
                                                	break;
                                        	}
					}
                                        aux_free_PKList(&pklist);
                                }
                                else if((keyinfo = d_KeyInfo(ostr))) {
                                        oid = af_get_objoid(DecSKnew_name);
                                        aux_free_KeyInfo(&keyinfo);
                                }
				else if ((cpairset = d_CertificatePairSet(ostr))) {
					oid = af_get_objoid(CrossCSet_name);
					aux_free_CertificatePairSet(&cpairset);
				}
                                else if((crlset = d_CrlSet(ostr))) {
                                        oid = af_get_objoid(CrlSet_name);
                                        aux_free_CrlSet(&crlset);
                                }
                                else if((aliaslist = d_AliasList(ostr))) {
                                        oid = af_get_objoid(AliasList_name);
                                        aux_free_AliasList(&aliaslist);
                                }
                                else if((serial = d_OctetString(ostr))) {
                                        oid = af_get_objoid(SerialNumber_name);
                                        aux_free_OctetString(&serial);
                                }
				else {
					tmp_ostr = aux_cpy_OctetString(ostr);
					aux_free_OctetString(&ostr);
					ostr = e_OctetString(tmp_ostr);
	                                oid = aux_cpy_ObjId(Uid_OID);
					aux_free_OctetString(&tmp_ostr);
				}
                                rcode = 0;

                                if(sec_open(&std_pse) < 0) rcode = sec_create(&std_pse);

                                if(!rcode) {
					rcode = sec_write_PSE(&std_pse, oid, ostr);
                       	                if(rcode) fprintf(stderr,"Can't write to PSE\n");
				}
				else fprintf(stderr,"Can't create PSE\n");
 
                                aux_free_OctetString(&ostr);
				aux_free_ObjId(&oid);

                                break;
                        case XDUMP:
                                key = object();
				if(!key) break;
                                if(key->pse_sel) {
                                        ostr = &octetstring;
                                        if(sec_read(key->pse_sel, ostr) < 0)   {
						fprintf(stderr,"Can't read object  %s  from PSE\n", key->pse_sel->object.name);
						aux_free_Key(&key);
						break;
					}
					aux_free_Key(&key);
                                	if((objectvalue = d_PSEObject(&objecttype, ostr)) == (OctetString *)0 ) {
                                                dd = ostr->octets;          
                                                n = ostr->noctets;
                                	}
                                        else {
						free(ostr->octets);
                                                fprintf(stderr, "Object OID { ");
                                                for(i = 0; i < objecttype.oid_nelem; i++) {
     	                                                fprintf(stderr, "%d ", objecttype.oid_elements[i]);
                                                }
                                                fprintf(stderr, " }\n");
						free(objecttype.oid_elements);
                                                dd = objectvalue->octets; 
                                                n  = objectvalue->noctets;
                                        }
                                }
                                else {
                                        if((rcode = sec_get_key(&tmpkey, key->keyref, (Key *)0)) < 0)  {
						fprintf(stderr,"Can't get key\n");
						aux_free_Key(&key);
						free(dd);
						break;
					}
                                        dd = tmpkey.subjectkey.bits;
                                        n = (tmpkey.subjectkey.nbits + 7)/ 8;
					aux_free_AlgId(&(tmpkey.subjectAI));
                                }
                                aux_xdump(dd, n, 0);
				aux_free_Key(&key);
				free(dd);
                                break;
			case ERROR:
				aux_fprint_error(stderr, verbose);
				break;
			case RESETERROR:
				aux_free_error();
				break;
                        default:
                                break;
                }
                if(rcode < 0) {
			if(verbose) aux_fprint_error(stderr, verbose);
			aux_free_error();
		}

        }
}

static
num(par)
register char *par;
{
        while(*par) {
                if(*par < '0' || *par > '9') return(FALSE);
                par++;
        }
        return(TRUE);
}

static
ack(pse_sel, txt)
PSESel *pse_sel;
char *txt;
{
        if(!(pse_sel->object.name) || !strlen(pse_sel->object.name)) fprintf(stderr, "PSE %s %s\n", pse_sel->app_name, txt);
        else fprintf(stderr, "PSE object %s %s\n", pse_sel->object.name, txt);
        return(0);
}

static
psesel(option) 
int option; 
{             

/*
 * psesel(option) sets std_pse according to values given via command-line.
 * option  1: objectname is optional; 2: objectname must be given
 * psesel returns 0 if object was selected, 1 if PSE was selected
 */
	char *proc = "psesel";
	char *newstring;
        char *dd;

        if(cmd == OPEN || cmd == CLOSE) {
                if(!(dd = nxtpar(""))) {
                        if(cmd == OPEN) {
                                fprintf(stderr, "Enter PSE name: ");
				newstring = (char *)malloc(128);
				if( !newstring ) {
					aux_add_error(EMALLOC, "newstring", CNULL, 0, proc);
					fprintf(stderr, "Can't allocate memory\n");
					return(-1);
				}
                                dd = gets(newstring);
                        }
                        else dd = aux_cpy_String(std_pse.app_name);
                }

                if(cmd == OPEN) {
                        if(std_pse.app_name) {
                                if(strcmp(dd, std_pse.app_name) == 0) {
					free(dd);
                                        fprintf(stderr, "PSE %s is already open\n", dd);
                                        return(-1);
                                }
                                if(std_pse.app_name) if(af_pse_close((ObjId *)0) == 0) ack(&std_pse, "closed");
                        }
                        if(std_pse.pin) strzfree(&(std_pse.pin));
                }
                else {
                        if(!std_pse.app_name || strcmp(dd, std_pse.app_name)) {
                                fprintf(stderr, "%s not open\n", dd);
				free(dd);
                                return(-1);
                        }
                }
                strrep(&(std_pse.app_name), dd);
                if(std_pse.object.name) free(std_pse.object.name);
                std_pse.object.name = CNULL;
                if(std_pse.object.pin) strzfree(&(std_pse.object.pin));
                return(1);
        }

        if(std_pse.object.name) free(std_pse.object.name);
        if(!(std_pse.object.name = nxtpar(""))) {
                while(!std_pse.object.name) {
			if(cmd == DH2) fprintf(stderr, "Enter name of PSE object for the agreed key: ");
                        else fprintf(stderr, "Enter name of object on %s: ", std_pse.app_name);
                        newstring = (char *)malloc(128);
 			if( !newstring ) {
				aux_add_error(EMALLOC, "newstring", CNULL, 0, proc);
				fprintf(stderr, "Can't allocate memory\n");
				return(-1);
			}
                        std_pse.object.name = gets(newstring);
                        if(strlen(std_pse.object.name) == 0) {
                                free(std_pse.object.name);
                                std_pse.object.name = CNULL;
                        }
                        if(option == 1) break;
                }
        }
        return(0);
}

static
getsize(par) 
char *par;
{
        int size = 0;
        char *dd, sz[32];
        if(!(dd = nxtpar(par)))  {
                fprintf(stderr, "%s: ", par);
                dd = aux_cpy_String(gets(sz));
		if(!dd) return(0);
        }
        sscanf(dd, "%d", &size);
	free(dd);
        return(size);
}

static
filen(output) 
char *output;
{

	char *proc = "filen";
	char *newstring;

        if(!(filename = nxtpar("file"))) {
                if(output) fprintf(stderr, output);
                else fprintf(stderr, "File [CR for stderr]: ");
		newstring = (char *)malloc(128);
 		if( !newstring ) {
			aux_add_error(EMALLOC, "newstring", CNULL, 0, proc);
			fprintf(stderr, "Can't allocate memory\n");
			return(-1);
		}
                filename = gets(newstring);
                if(!filename || strlen(filename) == 0) {
                        free(newstring);
                        filename = CNULL;
                }
        }

	return(0);
}

static
keytype() {
	char *proc = "keytype";
	char *newstring;

        if(!(objtype = nxtpar("keytype"))) {
                fprintf(stderr, "SIGNATURE or ENCRYPTION type (S/E) [CR for S]: ");
 		newstring = (char *)malloc(16);
 		if( !newstring ) {
			aux_add_error(EMALLOC, "newstring", CNULL, 0, proc);
			fprintf(stderr, "Can't allocate memory\n");
			return(-1);
		}
                objtype = gets(newstring);
                if(!objtype || strlen(objtype) == 0) {
                        free(newstring);
                        objtype = "S";
                }
        }

	return(0);
}

static
printfile(ostring, fname)
OctetString *ostring;
char *fname;
{
        register int i, j;
        register char *dd;
        if(!fname || !strlen(fname)) {
                dd = ostring->octets;
                i = ostring->noctets;
                while(i > 0) {
                	j = (i < 1024) ? i : 1024;
                        write(1, dd, j);
                        i -= j;
                        dd += j;
                }
        }
        else aux_OctetString2file(ostring, fname, 2);

	return(0);
}

static
new_pin() {
        if(!(newpin = nxtpar("newpin"))) newpin = sec_read_pin("New PIN", std_pse.object.name, TRUE);

	return(0);
}        

static
str2key() {
        if(!(pin = nxtpar("pin"))) pin = sec_read_pin("PIN", "", TRUE);

	return(0);
}        

static
helpcmd() {
        int i;

        if((helpname = nxtpar(CNULL))) {
                for(i = 0; cmds[i].cmd; i++) {
                        if(strncmp(cmds[i].cmd, helpname, strlen(helpname)) == 0) {
                                fprintf(stderr, "Command:     %s %s\n", cmds[i].cmd, cmds[i].parms);
                                fprintf(stderr, "Description: %s\n", cmds[i].text);
                                fprintf(stderr, "%s\n", cmds[i].longtext);
                        }
                }
                return(0);
        }

        fprintf(stderr, "Command      Description\n");
        fprintf(stderr, "-------------------------------------------------------------------------\n");
        for(i = 0; cmds[i].cmd; i++) fprintf(stderr, "%-12s %s\n", cmds[i].cmd, cmds[i].text);
        fprintf(stderr, "-------------------------------------------------------------------------\n");
        fprintf(stderr, "Command and parameter names may be abbreviated, parameters may be omitted\n");
	return(0);
}

static
store_objpin()
/* stores object pin returned from "pin_check()" in AF_pse */
{
	int    i;
	char * proc = "store_objpin";

	for (i = 0; i < PSE_MAXOBJ; i++) 
		if (strcmp(AF_pse.object[i].name, std_pse.object.name) == 0) {
			strrep(&(AF_pse.object[i].pin), std_pse.object.pin);
		}

	return(0);
}

static
Key *build_key(s, flag) 
char *s; 
int flag;  /* 0: read,  1: write */
#define OPEN_TO_READ 0
#define OPEN_TO_WRITE 1
{
        char *dd, *cc;
	char answ[8];
	int i;
        KeyInfo zwkey;
        OctetString ostring;
	char *proc = "build_key";
	char newstring[64];
	Key *newkey;
	PSEConfig pse_config;
	AlgEnc   algenc;
	int	 keyref;


	newkey = (Key *)calloc(1, sizeof(Key));
	if(!newkey) return((Key *)0);

        if(cmd == GENKEY) {
                newkey->key = (KeyInfo *)calloc(1, sizeof(KeyInfo));
 		if(!newkey->key) {
			return((Key *)0);
		}
                newkey->key->subjectAI = aux_Name2AlgId(algname);
                if(!newkey->key->subjectAI) {
			free(newkey->key);
			free(newkey);
                        return((Key *)0);
                }
        }
nk:
        if(!(dd = nxtpar(CNULL))) {
                if(cmd == DELKEY) fprintf(stderr, "Keyref: %s", s);
                else if(cmd == CERT2KEYINFO || cmd == CERTIFY || cmd == CERT2PKROOT || cmd == PROTOTYPE || cmd == ADDEK || cmd == ADDPK || cmd == ADDPCA || cmd == SPLIT) fprintf(stderr, "%s: ", s);
                else fprintf(stderr, "Name or ref of %skey: ", s);
                dd = aux_cpy_String(gets(newstring));
		if(!dd) aux_cpy_String("");
        }
        if(strcmp(dd, "replace") == 0) {
                replace = TRUE;
                goto nk;
        }
        if(num(dd)) {
                if(strlen(dd) == 0) {
			newkey->keyref = -1;
		}
                else sscanf(dd, "%d", &(newkey->keyref));
		free(dd);
		if (newkey->keyref == 0) 
			newkey->keyref = -1;
#ifdef SCA
		else {
			/*
			 * GENKEY: 
			 *   check whether a DES/DES3 key shall be generated in SCT/SC 
			 * DELKEY: 
			 *   check whether a key stored in the SCT shall be deleted
			 */
		
			if (cmd == GENKEY) {
				algenc = aux_ObjId2AlgEnc(newkey->key->subjectAI->objid);
				if ((algenc == DES) || (algenc == DES3)) {
					fprintf	(stderr, "Store key in KeyPool (0), in the SCT (1), in the Smartcard (2):");
					gets(answ);
					if ((answ[0] == '1') || (answ[0] == '2')) {

						/*
						 *  Store key in SCT/SC
						 */

						if (answ[0] == '1') {
							 keyref = newkey->keyref | SCT_KEY;
                               				 newkey->keyref = keyref;
						}
						else {
							/*
							 * Check whether pse = pse on SC
							 */
							if ((pse_location = sec_psetest(std_pse.app_name)) == ERR_in_psetest) {
								fprintf(stderr, "Error during test for pse: %s\n", std_pse.app_name);
								aux_free_Key(&newkey);
								return((Key *)0);
							}

							if (pse_location == SWpse) {
                       						fprintf(stderr, "\nCannot generate a key on the smartcard:\n");
                       						fprintf(stderr, "PSE %s is not a smartcard PSE\n", std_pse.app_name);
								aux_free_Key(&newkey);
								return((Key *)0);
							}
                               				keyref = newkey->keyref | SC_KEY;
                               				newkey->keyref = keyref;
						}
						return (newkey);
					}
				}
			
			}
			if (cmd == DELKEY) {
				fprintf	(stderr, "Delete key in KeyPool (0), in the SCT (1):");
				gets(answ);
				if (answ[0] == '1') {

					/*
					 *  Delete key in SCT
					 */

                               		keyref = newkey->keyref | SCT_KEY;
                               		newkey->keyref = keyref;
					return (newkey);

				}
			}
			
		}
#endif
		if(cmd == GENKEY && publickey && newkey->keyref != -1) if(newkey->keyref == publickey->keyref) {
                       	fprintf(stderr, "Public key and secret key must have different keyrefs\n");
			aux_free_Key(&newkey);
			return((Key *)0);
		}
		if(newkey->keyref > 0 && cmd == GENKEY && replace == FALSE) {
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
        else {
                newkey->pse_sel = aux_cpy_PSESel(&std_pse);
		strrep(&(newkey->pse_sel->object.name), dd);
		free(dd);
		for (i = 0; i < PSE_MAXOBJ; i++) 
			if(strcmp(AF_pse.object[i].name, newkey->pse_sel->object.name) == 0) {
				strrep(&(newkey->pse_sel->object.pin), AF_pse.object[i].pin);
				break;
               		}
		if(cmd == GENKEY) {
			if(publickey) if(strcmp(newkey->pse_sel->object.name, publickey->pse_sel->object.name) == 0) {
	                       	fprintf(stderr, "Public key and secret key must be stored in different objects\n");
				aux_free_Key(&newkey);
				return((Key *)0);
			}
#ifdef SCA

			if ((pse_location = sec_psetest(newkey->pse_sel->app_name)) == ERR_in_psetest) {
				fprintf(stderr, "Error during test for pse: %s\n", newkey->pse_sel->app_name);
				aux_free_Key(&newkey);
				return((Key *)0);
			}

			if((pse_location == SCpse) && (!sec_open(newkey->pse_sel))) {
				if(strcmp(s, "generated secret ") == 0) {
                                	fprintf(stderr, "Replace existing secret key? (y/n): ");
                                	gets(answ);
                                	if(answ[0] == 'y') replace = TRUE;  /* replace is global */
				}
				if ((pse_config = sec_pse_config(newkey->pse_sel)) == ERR_in_pseconfig) {
	                       		fprintf(stderr, "Error in PSE configuration (SCINITFILE)\n");
					aux_free_Key(&newkey);
					return((Key *)0);
				}
				if (pse_config != NOT_ON_SC) {
					if(strcmp(s, "generated DES ") == 0) {
                                		fprintf(stderr, "Replace existing DES key? (y/n): ");
                                		gets(answ);
                                		if(answ[0] == 'y') replace = TRUE;  /* replace is global */
					}
				}
				return(newkey);
                        }
#endif
		}
                if(sec_open(newkey->pse_sel) < 0) {
                        if(flag == OPEN_TO_READ) {
                                fprintf(stderr, "Can't open Object %s\n", newkey->pse_sel->object.name);
				aux_add_error(EINVALID, "sec_open failed", CNULL, 0, proc);
				aux_free_Key(&newkey);
                                return((Key *)0);
                        }

			/*
			 *  Open (object) failed => object doesn't exist.
			 *    
			 *  If object to be generated is a key on the SC => no creation of object.
			 *      
			 */

			if ((pse_config = sec_pse_config(newkey->pse_sel)) == ERR_in_pseconfig) {
	                       	fprintf(stderr, "Error in PSE configuration (SCINITFILE)\n");
				aux_free_Key(&newkey);
				return((Key *)0);
			}
			if (pse_config != KEY_ON_SC) {

                        	strrep(&(newkey->pse_sel->object.pin), newkey->pse_sel->pin);
                        	if(sec_create(newkey->pse_sel) < 0) {
                                	fprintf(stderr, "Can't create Object %s\n", newkey->pse_sel->object.name);
					aux_add_error(EINVALID, "sec_open failed", CNULL, 0, proc);
					aux_free_Key(&newkey);
                                	return((Key *)0);
                        	}
                        	fprintf(stderr, "PSE object %s created\n", newkey->pse_sel->object.name);
				if(strcmp(s, "generated public ")) replace = TRUE;
				for (i = 0; i < PSE_MAXOBJ; i++) {
					if (!strcmp(AF_pse.object[i].name, newkey->pse_sel->object.name)) {
						strrep(&(AF_pse.object[i].pin), newkey->pse_sel->object.pin);
						break;
                      	      		}
				}
			}
                        return(newkey);
                }
 
		for (i = 0; i < PSE_MAXOBJ; i++) {
			if (!strcmp(AF_pse.object[i].name, newkey->pse_sel->object.name)) {
				strrep(&(AF_pse.object[i].pin), newkey->pse_sel->object.pin);
				break;
                        }
                        else if(flag == OPEN_TO_WRITE) {
				if(cmd == GENKEY && replace == TRUE) return(newkey);
                                fprintf(stderr, "Object %s already exists. Overwrite? (y/n): ", newkey->pse_sel->object.name);
                        	dd = gets(newstring);
                                if(!dd || *dd != 'y') {
					aux_free_Key(&newkey);
					return((Key *)0);
				}
				if(strcmp(s, "generated public ")) replace = TRUE;
                                return(newkey);
                        }
                }
        }
        return(newkey);
}

static
char *getalgname() {
        char aname[32], answ[8];
        char *dd, *ee;
        if(!(dd = nxtpar("algorithm"))) {
                if(cmd == ALGS) strcpy(aname, "ALL");
                else {
                        if(cmd == SETPARM) fprintf(stderr, "Algorithm name [CR for rsa]: ");
                        else fprintf(stderr, "Algorithm name [CR for desCBC]: ");
                        gets(aname);
                }
       	 	if(strlen(aname) == 0) {
                	if(cmd == SETPARM) strcpy(aname, "rsa");
                	else if(cmd == ALGS) strcpy(aname, "ALL");
                	else strcpy(aname, "desCBC");
        	}
		dd = aux_cpy_String(aname);
	}
	if(aux_Name2AlgEnc(dd) == DSA && cmd == GENKEY) {
		if(!(ee = nxtpar("primes"))) { 
			fprintf(stderr, "Predefined p, q, g? (y/n): ");
			gets(answ);
			if(answ[0] == 'y') sec_dsa_predefined = TRUE;
			else sec_dsa_predefined = FALSE;
		}
	}
        return(dd);
}

static
DName *getdname(s)
char * s;
{
	DName *dname;
	char *dd, name[64];

	if(! s){
        	if(!(dd = nxtpar("dirname"))) {
			fprintf(stderr, "Target Directory Entry [CR for your own Entry]: ");
			dd = aux_cpy_String(gets(name));
        	}
		if(!dd) return(af_pse_get_Name());
        	if (strlen(dd) == 0) {
			free(dd);
			return(af_pse_get_Name());
		}
	}
	else{
		fprintf(stderr, "%s's Distinguished Name [CR for NULLDNAME]: ", s);
		dd = aux_cpy_String(gets(name));
 		if( !dd ) return (NULLDNAME);
		if (strlen(dd) == 0) {
			free(dd);
			return (NULLDNAME);
		}
	}
	dname = aux_alias2DName(dd);
	if (!dname){
		fprintf(stderr, "Cannot transform alias name <%s> into a Distinguished Name!\n", dd);
	}
	free(dd);

        return(dname);
}


static
Name *getname() {
	DName *dname;
        char *dd, name[512];
	char *proc = "getname";


username:
        if(!(dd = nxtpar("dirname"))) {
                fprintf(stderr, "Target name: ");
                dd = aux_cpy_String(gets(name));
        }
	if(!dd) return((Name *)0);
	if(strlen(dd) == 0) {
		free(dd);
		return((Name *)0);
	}
	dname = aux_alias2DName(dd);
	free(dd);
	if (!dname) {
                incorrectName();
                goto username;
        }
        dd = aux_DName2Name(dname);
        aux_free_DName(&dname);
        return(dd);
}


static
Name *getalias(dirname, flag) 
char * dirname;
AliasType flag;
{
	DName *dname;
        char *dd, name[64];
	char *proc = "getalias";

	if(flag == LOCALNAME){
		if(!(dd = nxtpar("alias"))) {
			if(dirname) fprintf(stderr, "Alias name for <%s>: ", dirname);
		    	else fprintf(stderr, "Alias name: ");
		    	dd = aux_cpy_String(gets(name));
	    	}
	}
	else{
		if(!(dd = nxtpar("rfcmail"))) {
			if(dirname) fprintf(stderr, "Enter RFC mail address for <%s>, or CR only: ", dirname);
		    	else fprintf(stderr, "RFC mail address: ");
		    	dd = aux_cpy_String(gets(name));
	    	}
	}
	if(!dd) return((Name *)0);
	if(strlen(dd) == 0) {
		free(dd);
		return((Name *)0);
	}
        return(dd);
}


static
OctetString * getserial() 
{

	char        * dd, number[1200];
	OctetString * ret;
	RC	      rc;

	char        * proc = "getserial";

	fprintf(stderr, "Serial number (HEXadecimal representation): ");
	dd = aux_cpy_String(gets(number));
 	if(! dd) return(NULLOCTETSTRING);
	if (strlen(dd) == 0) {
		free(dd);
		return(NULLOCTETSTRING);
	}

	ret = aux_create_SerialNo(dd);
	free(dd);

        return(ret);
}


static
AliasFile getaliasfile()
{
	char      source[32];
	AliasFile aliasfile;
	char    * dd;

	dd = nxtpar("aliasfile");
sourcetype:
	if(!dd){
		fprintf(stderr, "USER/SYSTEM Alias Database ? [ U[ser] / S[ystem] ] [CR for User]:  ");
		gets(source);
       	 	if(strlen(source) == 0) strcpy(source, "U");
		dd = aux_cpy_String(source);
	}

	if (! strncasecmp(dd, "User", 1))
		aliasfile = useralias;
	else if (! strncasecmp(dd, "System", 1))
		aliasfile = systemalias;
	else {
		fprintf(stderr,"Source must be either 'User' or 'System'\n");
		if(dd){
			free(dd);
			dd = CNULL;
		}
		goto sourcetype;
	}
	return(aliasfile);
}



static
char *getattrtype(x500)
char x500;
{
        char type[32];
        char *dd;


        dd = nxtpar("attrtype");
attrtype:
	if(!dd){
                fprintf(stderr, "Select one of the following attribute types:\n\n");
#ifdef X500
		if(x500){
			fprintf(stderr, "       U[serCertificate]\n");
			fprintf(stderr, "       CA[Certificate]\n");
		}
#endif
#ifdef AFDBFILE
		if(!af_x500 || !x500)
			fprintf(stderr, "       Ce[rtificate]\n");
#endif
		fprintf(stderr, "       Cr[ossCertificatePair]\n");
		if(cmd == ENTER || cmd == RETRIEVE)
			fprintf(stderr, "       R[evocationList]\n");
		fprintf(stderr, "\n");
#ifdef X500
		if(x500)
			fprintf(stderr, "[CR for UserCertificate]:  ");
#endif
#ifdef AFDBFILE
		if(!af_x500 || !x500)
			fprintf(stderr, "[CR for Certificate]:  ");
#endif
                gets(type);
       	 	if(strlen(type) == 0){
#ifdef X500
			if(x500)
                		strcpy(type, "UserCertificate");
#endif
#ifdef AFDBFILE
			if(!af_x500 || !x500)
				strcpy(type, "Certificate");
#endif
		}
		dd = aux_cpy_String(type);
	}

#ifdef X500
	if(x500){
		if((strncasecmp(dd, "UserCertificate", 1) && strncasecmp(dd, "CACertificate", 2) &&
		    strncasecmp(dd, "CrossCertificatePair", 2) && strncasecmp(dd, "RevocationList", 1) &&
		    (cmd == ENTER || cmd == RETRIEVE)) ||
		   (strncasecmp(dd, "UserCertificate", 1) && strncasecmp(dd, "CACertificate", 2) &&
		    strncasecmp(dd, "CrossCertificatePair", 2) && (cmd == REMOVE))) {
			fprintf(stderr, "\n");
			fprintf(stderr, "Wrong Attribute Type!\n");
			if(dd){
				free(dd);
				dd = CNULL;
			}
			goto attrtype;
		}
	}
#endif
#ifdef AFDBFILE
	if(!af_x500 || !x500){
		if((strncasecmp(dd, "Certificate", 2) && strncasecmp(dd, "CrossCertificatePair", 2) && 
		    strncasecmp(dd, "RevocationList", 1) && (cmd == ENTER || cmd == RETRIEVE)) ||
		   (strncasecmp(dd, "Certificate", 2) && strncasecmp(dd, "CrossCertificatePair", 2) && 
		    (cmd == REMOVE))) {
			fprintf(stderr, "\n");
			fprintf(stderr, "Wrong Attribute Type!\n");
			if(dd){
				free(dd);
				dd = CNULL;
			}
			goto attrtype;
		}
	}    
#endif
	fprintf(stderr, "\n");

	return(dd);
}


static
int storeinfo()
{
        char type[32];
	int  select;

selection:
	fprintf(stderr, "\n\n");
        fprintf(stderr, " Where shall the certificate be stored:\n\n");
	fprintf(stderr, "       P[SE]\n");
	fprintf(stderr, "       F[ile]\n\n");
	fprintf(stderr, " [CR for NO STORAGE]:  ");
        gets(type);
	fprintf(stderr, "\n");
       	if(strlen(type) == 0) return(0);

	if(! strncasecmp(type, "PSE", 1)) select = 1;
	else if (! strncasecmp(type, "File", 1)) select = 2;
	else {
		fprintf(stderr, "\n");
		fprintf(stderr, " Wrong selection!\n");
		goto selection;
	}

	return(select);
}


static
Key *object() {
        char *dd;
	char *proc = "object";
	char name[64];
	char *newstring;
	Key  *newkey;
	KeyRef keyref;


        keyref = 0;
        if(!(dd = nxtpar(CNULL))) {
                fprintf(stderr, "Name or keyref: ");
                dd = aux_cpy_String(gets(name));
        }
        if(!dd || strlen(dd) == 0) keyref = -1;
        else sscanf(dd, "%d", &keyref);

	newkey = (Key *)calloc(1, sizeof(Key));
	if(!newkey) return(newkey);

        if(keyref != 0) {
                newkey->pse_sel = (PSESel *)0;
                newkey->keyref = keyref;
		free(dd);
        }
        else {
                newkey->pse_sel = aux_cpy_PSESel(&std_pse);
		if(newkey->pse_sel->object.name) free(newkey->pse_sel->object.name);
                newkey->pse_sel->object.name = dd;
                newkey->keyref = 0;
        }
	return(newkey);
}


static
char *nxtpar(search) 
char *search; 
{
        char *dd, *cc, *ret, *pp, *prm;
	char *proc = "nxtpar";

        int len_excl, len_incl, gl;
		/* len_incl: Argument-Laenge inklusive moeglicher Blanks */
		/* len_excl: Argument-Laenge ohne Blanks */
	int inword = 0;

        if(search) {
                dd = inp;
                while((dd = strchr(dd, '='))) {
                        cc = dd - 1;
                        *dd = '\0';
                        while(*cc && *cc != ' ') cc--;
                        cc++;
			ret = cc;
                        if(strncmp(search, cc, strlen(cc)) == 0) {
				*dd++ = '=';
                                pp = prm = (char *)malloc(128);
                                while (*dd && ( (*dd != ' ') || ((*dd == ' ') && inword) ) ) { 
					if (*dd == '"') {
						inword = 1 - inword;
						dd++;
					}
					else *pp++ = *dd++;
				} 
                                *pp = '\0';
				/* Remove keyword 'search' and parameter 'prm' from input line */
				while (ret < dd) {
					*ret++ = ' ';
				}
                                return(prm);
                        }
                        *dd++ = '=';
                }
                if(!strcmp(search, "pse") || !strcmp(search, "ppin")) return(CNULL);
        }
        dd = inp;
again:
        while(*dd && *dd == ' ') dd++;
        if(*dd) ret = dd;
        else return(CNULL);
        gl = FALSE;
	while (*dd && ( (*dd != ' ') || ((*dd == ' ') && inword) ) ) {
		if (*dd == '"') inword = 1 - inword;
                if ((*dd == '=') && !inword) gl = TRUE;
                dd++;
        }
        if (gl) goto again;
	len_incl = dd - ret;   
	if (*ret == '"') len_excl = dd - ret - 2;  /* Blanks vorne und hinten */ 
        else len_excl = len_incl;
        cc = (char *)malloc(len_excl+1);
 	if( !cc ) {
		aux_add_error(EMALLOC, "cc", CNULL, 0, proc);
		fprintf(stderr, "Can't allocate memory\n");
		return(CNULL);
	}
	if (*ret == '"') strncpy(cc, ret+1, len_excl);
        else strncpy(cc, ret, len_excl);
        cc[len_excl] = '\0';
        dd = ret;
        while (len_incl) {
                *dd++ = ' ';
                len_incl--;
        }
        return(cc);
}


static
char *strmtch(a, b)
char *a, *b;
{
	register char *aa, *bb;
	while(*a) {
		aa = a;
		bb = b;
		while(*aa) {
			if(*aa != *bb) break;
                        bb++;
			if(*bb == '\0') return(aa + 1);
                        aa++;
		}
		a++;   
	}
	return(CNULL);
}



static
off_t fsize(fd)
int fd;
{
        struct stat stat;
	char *proc = "fsize";


        if(fstat(fd, &stat) == 0) return(stat.st_size);
	aux_add_error(ESYSTEM, "fstat failed", CNULL, 0, proc);
        return(-1);
}


static 
CertificatePair *compose_CertificatePair()
{
	CertificatePair *cpair;
	OctetString     *ostr, octetstring;
	ObjId    	 object_oid;
	char            *proc = "compose_CertificatePair";

	if ( !(cpair = (CertificatePair *)malloc(sizeof(CertificatePair))) ) {
		aux_add_error(EMALLOC, "cpair", CNULL, 0, proc);
		return ((CertificatePair *)0);
	}
	cpair->forward = (Certificate *)0;
	cpair->reverse = (Certificate *)0;
	fprintf(stderr, "Composing the CrossCertificatePair...\n");

	fprintf(stderr, " PSE object containing forward certificate:\n");
	psesel(1);
	if (std_pse.object.name) {
		ostr = &octetstring;
		if(sec_read_PSE(&std_pse, &object_oid, ostr) < 0) {
			aux_add_error(EINVALID, "sec_read_PSE failed", (char *)&std_pse, PSESel_n, proc);
			return ((CertificatePair *)0);
		}
		if(aux_cmp_ObjId(&object_oid, SignCert_OID) && aux_cmp_ObjId(&object_oid, EncCert_OID) && aux_cmp_ObjId(&object_oid, Cert_OID)) { 
			aux_add_error(EINVALID, "Selected object on PSE is no certificate", (char *)&std_pse, PSESel_n, proc);
			free(ostr->octets);
			aux_free2_ObjId(&object_oid);
			return ((CertificatePair *)0);
                }	
		aux_free2_ObjId(&object_oid);
		if(!(cpair->forward = d_Certificate(ostr))) {
			aux_add_error(EDECODE, "Cannot decode forward certificate", CNULL, 0, proc);
			free(ostr->octets);
			return ((CertificatePair *)0);
                }
		free(std_pse.object.name);	
		free(ostr->octets);
	}

	fprintf(stderr, " PSE object containing reverse certificate:\n");
	psesel(1);
	if (std_pse.object.name) {
		ostr = &octetstring;
		if(sec_read_PSE(&std_pse, &object_oid, ostr) < 0) {
			aux_add_error(EINVALID, "sec_read_PSE failed", (char *)&std_pse, PSESel_n, proc);
			return ((CertificatePair *)0);
		}
		if(aux_cmp_ObjId(&object_oid, SignCert_OID) && aux_cmp_ObjId(&object_oid, EncCert_OID) && aux_cmp_ObjId(&object_oid, Cert_OID)) { 
			aux_add_error(EINVALID, "Selected object on PSE is no certificate", (char *)&std_pse, PSESel_n, proc);
			aux_free2_ObjId(&object_oid);
			free(ostr->octets);
			return ((CertificatePair *)0);
                }
		aux_free2_ObjId(&object_oid);
		if(!(cpair->reverse = d_Certificate(ostr))) {
			aux_add_error(EDECODE, "Cannot decode reverse certificate", CNULL, 0, proc);
			free(ostr->octets);
			return ((CertificatePair *)0);
                } 
		free(std_pse.object.name);   
		free(ostr->octets);
	}

	if (!cpair->forward && !cpair->reverse) {
		aux_add_error(EINVALID, "At least one component (forward or reverse) must be present", CNULL, 0, proc);
		return ((CertificatePair *)0);
	}

	return (cpair);
}



/* specify_CertificatePair() creates a CrossCertificatePair whose components
   (forward and reverse certificate) do only comprise a serial number and
   an issuer's distinguished name; this "incomplete" CrossCertificatePair
   universally identifies one "complete" CrossCertificatePair and is used to
   select that CrossCertificatePair by comparison */

static 
CertificatePair * specify_CertificatePair()
{
	CertificatePair * cpair;
	char            * dd, * ptr;
	OctetString	* serial;
	DName	        * issuer_dn;

	char 		* proc = "specify_CertificatePair";


	cpair = (CertificatePair *)malloc(sizeof(CertificatePair));
	if(! cpair){
		aux_add_error(EMALLOC, "cpair", CNULL, 0, proc);
		return ((CertificatePair *)0);
	}

	if (!(dd = nxtpar("for"))){
		fprintf(stderr, "Identify  F O R W A R D  certificate:\n");
		fprintf(stderr, "   (if forward certificate shall be empty, type CR when asked for serial number)\n\n");
		serial = getserial();
               	if(! serial) cpair->forward = (Certificate *)0;
		else{
			fprintf(stderr, "   ");
			issuer_dn = getdname("Issuer");
			if(! issuer_dn){
				aux_add_error(EINVALID, "Forward certificate insufficiently specified", CNULL, 0, proc);
				return ((CertificatePair *)0);
			}
			cpair->forward = (Certificate *)calloc(1, sizeof(Certificate));
			if( !cpair->forward ) {
				aux_add_error(EMALLOC, "cpair->forward", CNULL, 0, proc);
				return ((CertificatePair *)0);
			}
			cpair->forward->tbs = (ToBeSigned *)calloc(1, sizeof(ToBeSigned));
			if( !cpair->forward->tbs ) {
				aux_add_error(EMALLOC, "cpair->forward->tbs", CNULL, 0, proc);
				return ((CertificatePair *)0);
			}
			cpair->forward->tbs->serialnumber = aux_cpy_OctetString(serial);
			cpair->forward->tbs->issuer = aux_cpy_DName(issuer_dn);
			aux_free_DName(&issuer_dn);
			aux_free_OctetString(& serial);
		}
	}
	else {
		ptr = strchr( dd, ',' );
		if(! ptr){
			aux_add_error(EINVALID, "Forward certificate insufficiently specified", CNULL, 0, proc);
			return ((CertificatePair *)0);
		}
		cpair->forward = (Certificate *)calloc(1, sizeof(Certificate));
		if(! cpair->forward) {
			aux_add_error(EMALLOC, "cpair->forward", CNULL, 0, proc);
			return ((CertificatePair *)0);
		}
		cpair->forward->tbs = (ToBeSigned *)calloc(1, sizeof(ToBeSigned));
		if(! cpair->forward->tbs) {
			aux_add_error(EMALLOC, "cpair->forward->tbs", CNULL, 0, proc);
			return ((CertificatePair *)0);
		}
		*ptr = '\0';
		cpair->forward->tbs->serialnumber = aux_create_SerialNo(dd);
		if(! cpair->forward->tbs->serialnumber) incorrectSerialNumber();
		ptr++;
		cpair->forward->tbs->issuer = aux_alias2DName(ptr);
		if (! cpair->forward->tbs->issuer) incorrectName();
		free(dd);
	}

	if (!(dd = nxtpar("rev"))){
		fprintf(stderr, "\n");
		fprintf(stderr, "Identify  R E V E R S E  certificate:\n");
		fprintf(stderr, "   (if reverse certificate shall be empty, type CR when asked for serial number)\n\n");
		serial = getserial();
               	if(! serial) 
			cpair->reverse = (Certificate *)0;
		else{
			fprintf(stderr, "   ");
			issuer_dn = getdname("Issuer");
			if(! issuer_dn){
				aux_add_error(EINVALID, "Reverse certificate insufficiently specified", CNULL, 0, proc);
				return ((CertificatePair *)0);
			}
			cpair->reverse = (Certificate *)calloc(1, sizeof(Certificate));
			if( !cpair->reverse ) {
				aux_add_error(EMALLOC, "cpair->reverse", CNULL, 0, proc);
				return ((CertificatePair *)0);
			}
			cpair->reverse->tbs = (ToBeSigned *)calloc(1, sizeof(ToBeSigned));
			if( !cpair->reverse->tbs ) {
				aux_add_error(EMALLOC, "cpair->reverse->tbs", CNULL, 0, proc);
				return ((CertificatePair *)0);
			}
			cpair->reverse->tbs->serialnumber = aux_cpy_OctetString(serial);
			cpair->reverse->tbs->issuer = aux_cpy_DName(issuer_dn);
			aux_free_DName(&issuer_dn);
			aux_free_OctetString(& serial);
		}
	}
	else {
		ptr = strchr( dd, ',' );
		if(! ptr) {
			aux_add_error(EINVALID, "Reverse certificate insufficiently specified", CNULL, 0, proc);
			return ((CertificatePair *)0);
		}
		cpair->reverse = (Certificate *)calloc(1, sizeof(Certificate));
		if(! cpair->reverse) {
			aux_add_error(EMALLOC, "cpair->reverse", CNULL, 0, proc);
			return ((CertificatePair *)0);
		}
		cpair->reverse->tbs = (ToBeSigned *)calloc(1, sizeof(ToBeSigned));
		if(! cpair->reverse->tbs) {
			aux_add_error(EMALLOC, "cpair->reverse->tbs", CNULL, 0, proc);
			return ((CertificatePair *)0);
		}
		*ptr = '\0';
		cpair->reverse->tbs->serialnumber = aux_create_SerialNo(dd);
		if(! cpair->reverse->tbs->serialnumber) incorrectSerialNumber();
		ptr++;
		cpair->reverse->tbs->issuer = aux_alias2DName(ptr);
		if (! cpair->reverse->tbs->issuer) incorrectName();
		free(dd);
	}

	return (cpair);
}


static incorrectName()
{
	fprintf(stderr, "Don't understand. Printable DName format required.\n");
	return(0);
}


static incorrectSerialNumber()
{
	fprintf(stderr, "Don't understand. String with hexadecimal digits required.\n");
	return(0);
}


static
void usage(help)
int     help;
{

	if(help == LONG_HELP) {
		aux_fprint_version(stderr);

			fprintf(stderr, "psemaint: Maintain PSE\n\n\n");
		fprintf(stderr, "Description:\n\n"); 
		fprintf(stderr, "'psemaint' is a maintenance program which can be used by both\n");
		fprintf(stderr, "certification authority administrators and users for the purpose\n");
		fprintf(stderr, "of maintaining their PSEs. This includes moving information (e.g. keys,\n");
		fprintf(stderr, "certificates, revocation lists etc.) from Unix files or a X.500 Directory\n");
		fprintf(stderr, "into the PSE and vice versa, generating keys, changing PINs and displaying\n"); 
		fprintf(stderr, "the content of the PSE.\n\n\n");
	}

        fprintf(stderr, "usage:\n\n");
#ifdef X500
	fprintf(stderr, "psemaint [-hvzACFORDTVW] [-p <pse>] [-c <cadir>] [-a <issueralg>] [-f <notbefore>] [-l <notafter>]\n");
	fprintf(stderr, "         [-i <inputfile>] [-d <dsa name>] [-t <dsaptailor>] [-A <authlevel>] [cmd]\n");
#else 
	fprintf(stderr, "psemaint [-htvCFORDTVW] [-p <pse>] [-c <cadir>] [-a <issueralg>] [-f <notbefore>] [-l <notafter>]\n");
	fprintf(stderr, "         [-i <inputfile>] [cmd]\n");
#endif   

        if(help == LONG_HELP) {

        fprintf(stderr, "with:\n\n");
        fprintf(stderr, "-p <psename>        PSE name (default: environment variable PSE or %s)\n", DEF_PSE);
        fprintf(stderr, "-c <cadir>          Name of CA-directory (default: environment variable CADIR or %s)\n", DEF_CADIR);
	fprintf(stderr, "-i <inputfile>      Scriptfile containing the commands to be executed by 'psemaint'\n");
	fprintf(stderr, "-a <issueralg>      CA's signature algorithm (default: md2WithRsaEncryption)\n");
	fprintf(stderr, "-f <notbefore>      First date on which the certificate is valid\n");
	fprintf(stderr, "                    (evaluated by 'certify' command within 'psemaint')\n");
	fprintf(stderr, "-l <notafter>       Last date on which the certificate is valid\n");
	fprintf(stderr, "                    (evaluated by 'certify' command within 'psemaint')\n");
	fprintf(stderr, "-F                  consider own FCPath as trusted\n");
	fprintf(stderr, "-O                  RFC 1422 DName subordination not required\n");
	fprintf(stderr, "-R                  consult revocation lists during verification\n");
	fprintf(stderr, "-C                  show list of commands available with 'psemaint'\n");
	fprintf(stderr, "-D                  access Directory (X.500 or AF_DB)\n");
#ifdef SCA
        fprintf(stderr, "-T                  perform each public key RSA operation in the smartcard  terminal\n");
        fprintf(stderr, "                    instead of employing the software in the workstation (the latter is the default)\n");
#endif
        fprintf(stderr, "-h                  write this help text\n");
	fprintf(stderr, "-z                  control malloc/free behaviour\n");
        fprintf(stderr, "-v                  verbose\n");
        fprintf(stderr, "-V                  Verbose\n");
        fprintf(stderr, "-W                  Grand Verbose (for testing only)\n");
        fprintf(stderr, "-X                  Read random number generator seed from PSE-object Random\n");
        fprintf(stderr, "-Y                  Init random number generator seed through keyboard input\n");
#ifdef X500
	fprintf(stderr, "-d <dsa name>       Name of the DSA to be initially accessed (default: locally configured DSA)\n");
	fprintf(stderr, "-t <dsaptailor>     Location of dsaptailor file (default: dsaptailor in the ISODE ETCDIR directory)\n");
	fprintf(stderr, "-A <authlevel>      Level of authentication used for X.500 Directory access\n");
	fprintf(stderr, "                    <authlevel> may have one of the values 'SIMPLE' or 'STRONG'\n");
	fprintf(stderr, "                    (default: environment variable AUTHLEVEL or 'No authentication')\n");
	fprintf(stderr, "                    STRONG implies the use of signed DAP operations\n");
#endif
	fprintf(stderr, "<cmd>               Single command that shall be executed by 'psemaint'\n");
	fprintf(stderr, "                    (otherwise, commands can be provided interactively\n");
 	fprintf(stderr, "                    or are read from file <inputfile> (see option -i))\n");
        }

        if(MF_check) MF_fprint(stderr);

        exit(-1);                                /* IRREGULAR EXIT FROM PSEMAINT */
}


static int
check_if_number(number)
char * number;
{
	int len, i;

	len = strlen(number);
	for(i = 0; i < len; i++){
		if(!isdigit(number[i])){
			fprintf(stderr, "'%c' is not a digit[0-9]!\n", number[i]);
			return(-1);
		}
	}
	return(0);
}


static RC
store_certificate(cert, onekeyonly)
Certificate * cert;
Boolean onekeyonly;
{
	static OctetString * ostr;
	static int i, rcode;
	static ObjId * oid;

	if(! cert){
		fprintf(stderr,"No certificate provided\n");
		return(- 1);
	}

	i = storeinfo();
	if (i != 1 && i != 2) return(- 1);

	ostr = e_Certificate(cert);
	if (! ostr) {
		fprintf(stderr,"Can't encode certificate\n");
		return(- 1);
	}

	if(i == 1){
		psesel(2);
		if(onekeyonly == TRUE) oid = af_get_objoid(Cert_name);
		else{
			keytype();
			if(*objtype == 'S') oid = af_get_objoid(SignCert_name);
			else if(*objtype == 'E') oid = af_get_objoid(EncCert_name);
			else {
				fprintf(stderr, "Type must me either 'S' or 'E'\n");
				return(- 1);
			}
		}
		rcode = 0;
		if(sec_open(&std_pse) < 0) rcode = sec_create(&std_pse);
		if(!rcode) {
			rcode = sec_write_PSE(&std_pse, oid, ostr);
			if(rcode) {
				fprintf(stderr,"Can't write to PSE\n");
				return(- 1);
			}
		} 
		else {
			fprintf(stderr,"Can't create PSE\n");
			return(- 1);
		}
		fprintf(stderr, "Specified certificate stored in object <%s> on PSE.\n", std_pse.object.name);
	}
	else {
		filen(CNULL);
		printfile(ostr, filename);
	}

	aux_free_OctetString(&ostr);
	aux_free_Certificate(&cert);

	return(0);
}
