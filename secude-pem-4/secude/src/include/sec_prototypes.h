#include <stdio.h>


#ifdef __STDC__


#ifdef _PSAP_


/***************************************************************
 *
 * Headers from file secure/sec_ed.c
 *
 ***************************************************************/ 
       KeyBits	* dec_RSAAlgorithm	(PE pe);
       KeyBits	* dec_DSAAlgorithm	(PE pe);
       int        enc_f    (int typ, modtyp *mod, PE *pe, int explicit, int len, char *buffer, char *parm);
       int        dec_f    (int typ, modtyp *mod, PE  pe, int explicit, int *len, char **buffer, char **parm);
       int        prnt_f   (int typ, modtyp *mod, PE  pe, int explicit, int *len, char **buffer);
       int        fre_obj  (char *parm, ptpe  *p, modtyp *mod, int dofree);

#endif
       char *getpass(const char *);


/***************************************************************
 *
 * Headers from file aux/aux_fprint.c
 *
 ***************************************************************/ 
       RC aux_fprint_OctetString(FILE *ff, OctetString	 *ostr);
       RC aux_fprint_BitString(FILE *ff, BitString	 *bstr);
       RC aux_fprint_PSESel(FILE *ff, PSESel	 *pse);
       RC aux_fprint_Version(FILE *ff);
       RC aux_fprint_version(FILE *ff);
       RC aux_fprint_PSEToc(FILE *ff, PSEToc	 *toc1,PSEToc	 *toc2);
       RC aux_fprint_KeyInfo(FILE *ff, KeyInfo	 *pki);
       RC aux_fprint_KeyBits(FILE *ff, KeyBits	 *keybits);
       RC aux_fprint_ObjId(FILE *ff, ObjId	 *objid);
       RC aux_fprint_AlgId(FILE *ff, AlgId	 *aid);
       RC aux_fprint_Serialnumber(FILE *ff, OctetString	 *serial);
       RC aux_fprint_error(FILE	*ff, int	  verbose);

       char *aux_sprint_OctetString(char *string, OctetString	 *ostr);
       char *aux_sprint_BitString(char *string, BitString	 *bstr);
       char *aux_sprint_PSESel(char *string, PSESel	 *pse);
       char *aux_sprint_Version(char *string);
       char *aux_sprint_version(char *string);
       char *aux_sprint_PSEToc(char *string, PSEToc	 *toc1,PSEToc	 *toc2);
       char *aux_sprint_KeyInfo(char *string, KeyInfo	 *pki);
       char *aux_sprint_KeyBits(char *string, KeyBits	 *keybits);
       char *aux_sprint_ObjId(char *string, ObjId	 *objid);
       char *aux_sprint_AlgId(char *string, AlgId	 *aid);
       char *aux_sprint_Serialnumber(char *string, OctetString	 *serial);
       char *aux_sprint_error(char *string, int	  verbose);

       RC aux_print_OctetString(OctetString	 *ostr);
       RC aux_print_BitString(BitString	 *bstr);
       RC aux_print_PSESel(PSESel	 *pse);
       RC aux_print_Version();
       RC aux_print_version();
       RC aux_print_PSEToc(PSEToc	 *toc1,PSEToc	 *toc2);
       RC aux_print_KeyInfo(KeyInfo	 *pki);
       RC aux_print_KeyBits(KeyBits	 *keybits);
       RC aux_print_ObjId(ObjId	 *objid);
       RC aux_print_AlgId(AlgId	 *aid);
       RC aux_print_Serialnumber(OctetString	 *serial);
       RC aux_print_error(int	  verbose);


/***************************************************************
 *
 * Headers from file aux/aux_util.c
 *
 ***************************************************************/ 
       void	aux_add_error	(int number, char *text, char *addr, Struct_No addrtype, char *proc);
       int	aux_last_error	();
       ObjId	*aux_cpy_ObjId	(ObjId *oid);
       PSESel	*aux_cpy_PSESel	(PSESel *psesel);
       int	aux_cpy2_ObjId	(ObjId *dup_oid, ObjId *oid);
       int	aux_cmp_ObjId	(ObjId *oid1, ObjId *oid2);
       AlgId	*aux_cpy_AlgId	(AlgId *aid);
       int	aux_cmp_AlgId	(AlgId *aid1, AlgId *aid2);
       int	aux_cmp_KeyBits	(KeyBits *keybits1, KeyBits *keybits);
       KeyInfo	*aux_cpy_KeyInfo	(KeyInfo *keyinfo);
       int	aux_cpy2_KeyInfo	(KeyInfo *dup_ki, KeyInfo *keyinfo);
       int	aux_cmp_BitString	(BitString *bitstring1, BitString *bitstring2);
       int	aux_cmp_KeyInfo	(KeyInfo *keyinfo1, KeyInfo *keyinfo2);
       int	aux_cmp_Signature	(Signature *sig1, Signature *sig2);
       int 	aux_cmp_OctetString(OctetString *octetstring1, OctetString *octetstring2);
       char	*aux_ObjId2PSEObjectName	(ObjId *given_objid);
       ObjId	*aux_PSEObjectName2ObjId	(char *name);
       ObjId	*aux_Name2ObjId	(char *name);
       AlgId	*aux_Name2AlgId	(char *name);
       AlgType	aux_Name2AlgType	(char *name);
       AlgEnc	aux_Name2AlgEnc	(char *name);
       AlgHash	aux_Name2AlgHash	(char *name);
       AlgMode	aux_Name2AlgMode	(char *name);
       AlgSpecial	aux_Name2AlgSpecial	(char *name);
       ParmType	aux_Name2ParmType	(char *name);
       char	*aux_ObjId2Name	(ObjId *given_objid);
       AlgType	aux_ObjId2AlgType	(ObjId *given_objid);
       AlgEnc	aux_ObjId2AlgEnc	(ObjId *given_objid);
       AlgHash	aux_ObjId2AlgHash	(ObjId *given_objid);
       AlgSpecial	aux_ObjId2AlgSpecial	(ObjId *given_objid);
       AlgMode	aux_ObjId2AlgMode	(ObjId *given_objid);
       AlgId	*aux_ObjId2AlgId	(ObjId *given_objid);
       ParmType	aux_ObjId2ParmType	(ObjId *given_objid);
       ObjId	*aux_oid2syntaxoid	(ObjId *given_objid);
       char	*aux_cpy_Name	(char *namefrom);
       char	*aux_cpy_String	(char *str);
       char	*aux_cpy_ReducedString	(char *a);
       int	aux_pstrcmp	(char *a, char *b);
       int	aux_lexequ	(char *str1, char *str2);
       int	aux_cmp_Name	(char *name1, char *name2);
       ObjId	*aux_keyword2oid	(char *keyword);
       ObjId	*aux_keyword2syntaxoid	(char *keyword);
       char	*aux_oid2keyword	(ObjId *given_objid);
       int	aux_cpy2_OctetString	(OctetString *dup_ostr, OctetString *ostr);
       int	aux_cpy2_BitString	(BitString *dup_bstr, BitString *bstr);
       OctetString	*aux_cpy_OctetString	(OctetString *ostr);
       BitString	*aux_cpy_BitString	(BitString *bstr);
       PSEToc	*aux_cpy_PSEToc	(PSEToc *pse);
       struct PSE_Objects	*aux_cpy_PSEObjects	(struct PSE_Objects *pse);
       PSEObject	*aux_cpy_PSEObject	(PSEObject *pse);
       KeyBits	*aux_cpy_KeyBits	(KeyBits *keybits);
       Signature	*aux_cpy_Signature	(Signature *sig);
       int	aux_check_3166	(char *a);
       int	aux_check_print_string	(char *str);
/*       int	strcasecmp	(char *s1, char *s2);*/
       RC 	aux_input_from_device(char *output, char *input, Boolean echo);
       char 	*aux_getpass(char *output);
       RC 	aux_sappend_char();
       char     *CATSPRINTF();
       char 	*CATNSTR();
/***************************************************************
 *
 * Headers from file aux/aux_xdmp.c
 *
 ***************************************************************/ 
       void	aux_fxdumpp	(FILE *dump_file, char *buffer, int len, int addr_type, Boolean printable);
       void	aux_fxdump	(FILE *dump_file, char *buffer, int len, int addr_type);
       char	*aux_sxdump2	(char *dump_file, char *buffer, int len, int addr_type);
       char	*aux_sxdump	(char *dump_file, char *buffer, int len, int addr_type);
       void	aux_fxdump2	(FILE *dump_file, char *buffer, int len, int addr_type);
       void	aux_xdump	(char *buffer, int len, int addr_type);
       void	aux_xdump2	(char *buffer, int len, int addr_type);

/***************************************************************
 *
 * Headers from file aux/aux_PKCS.c
 *
 ***************************************************************/ 
       OctetString	*aux_create_PKCSBlock	(char BT, OctetString *D);
       OctetString	*aux_create_PKCS_MIC_D	(OctetString *hash_result, AlgId *signatureAlgorithm);
/***************************************************************
 *
 * Headers from file aux/aux_can.c
 *
 ***************************************************************/ 
       OctetString	*aux_canon	(OctetString *text);
       OctetString	*aux_decanon	(OctetString *canform);
       OctetString	*aux_64	(OctetString *text, OctetString *indent);
       OctetString	*aux_de64	(OctetString *form64, int indentlen);
/***************************************************************
 *
 * Headers from file aux/aux_conv.c
 *
 ***************************************************************/ 
       OctetString	*aux_encrfc	(OctetString *in);
       OctetString	*aux_decrfc	(OctetString *in);
       OctetString	*aux_enchex	(OctetString *in);
       OctetString	*aux_dechex	(OctetString *in);
#ifdef APCONV
       OctetString	*aux_encap	(OctetString *in);
       OctetString	*aux_decap	(OctetString *in);
#endif
/***************************************************************
 *
 * Headers from file aux/aux_ed.c
 *
 ***************************************************************/ 
#ifdef _PSAP_
       	OctetString	*aux_PE2OctetString	(PE pe);
       	PE		 aux_OctetString2PE	(OctetString *ostr);
	PE		 aux_DER_SETOF	        (PE pe);
#endif
/***************************************************************
 *
 * Headers from file aux/aux_file.c
 *
 ***************************************************************/ 
       OctetString	*aux_file2OctetString	(char *fn);
       int	aux_OctetString2file	(OctetString *ostr, char *fn, int flag);

/***************************************************************
 *
 * Headers from file aux/aux_imp.c
 *
 ***************************************************************/ 
       int	catstr	(char **ptr_str, int *ptr_len, char *new);
       char	*str_low	(char *str);
       char	*str_up	(char *str);
       int	Trim	(char *str);
       int	Skip_blanks	(char *str, int *ptr_idx);
       int	ScanOp	(char *str, int *ptr_idx, char *oper);
       char	*Scanv	(char *str, int *ptr_idx, char *delim);
/***************************************************************
 *
 * Headers from file aux/aux_malloc.c
 *
 ***************************************************************/ 
       void	mftrace	();
       char	*aux_malloc	(char *proc, unsigned size);
       char	*aux_calloc	(char *proc, unsigned elem, unsigned size);
       char	*aux_realloc	(char *proc, void *ptr, unsigned size);
       void	aux_free	(char *proc, void *ptr);
       void	MF_fprint	(FILE *fp);
       void	MF_fprint_stderr	();
/***************************************************************
 *
 * Headers from file aux/aux_ostr.c
 *
 ***************************************************************/ 
       OctetString	*aux_create_OctetString	(char *text);
       OctetString	*aux_new_OctetString	(int length);
       RC	aux_append_BitString	(BitString *to, BitString *append);
       RC	aux_append_OctetString	(OctetString *to, OctetString *append);
       RC	aux_append_char	(OctetString *to, char *append);
       OctetString * aux_concatenate_OctetString (OctetString *part1, OctetString *part2);
       RC	aux_hex_append	(OctetString *to, OctetString *append);
       RC	aux_searchitem	(OctetString *bst, char *item, int *pos);
       OctetString	*aux_ostr_get64	(OctetString *msgbuf, int pos);
       RC	aux_cmpitem	(OctetString *msgbuf, char *item, int pos);
       OctetString 	*aux_create_SerialNo(char *number);
       int 	aux_OctetString2int(OctetString	 *ostr_repr);
       OctetString 	*aux_int2OctetString(int int_repr);
       RC aux_prepend_dash_(OctetString	 *ostr, int  number);
       RC aux_strip_dash_(OctetString *ostr, int number);
       RC aux_OctetString_delete_string(OctetString *ostr, unsigned int  pos, unsigned int  len);
       RC aux_OctetString_insert_string(OctetString *ostr, OctetString *item, unsigned int  pos);

/***************************************************************
 *
 * Headers from file aux/aux_free.c
 *
 ***************************************************************/ 
       void	aux_free_OctetString	(OctetString **ostr);
       void	aux_free2_OctetString	(OctetString *ostr);
       void	aux_free_BitString	(BitString **bstr);
       void	aux_free2_BitString	(BitString *bstr);
       void	aux_free_ObjId	(ObjId **oid);
       void	aux_free2_ObjId	(ObjId *oid);
       void	aux_free_AlgId	(AlgId **algid);
       void	aux_free2_KeyInfo	(KeyInfo *p);
       void	aux_free2_Signature	(Signature *p);
       void	aux_free_KeyInfo	(KeyInfo **keyinfo);
       void	aux_free_Signature	(Signature **signature);
       void	aux_free2_DigestInfo	(DigestInfo *p);
       void	aux_free_DigestInfo	(DigestInfo **digestinfo);
       void	aux_free_KeyBits	(KeyBits **keybits);
       void	aux_free2_KeyBits	(KeyBits *p);
       void	aux_free2_PSEToc	(PSEToc *p);
       void	aux_free2_PSESel	(PSESel *p);
       void	aux_free_PSEToc	(PSEToc **psetoc);
       void	aux_free_PSESel	(PSESel **psesel);
       void	aux_free2_AlgId	(AlgId *algid);
/***************************************************************
 *
 * Headers from file aux/aux_time.c
 *
 ***************************************************************/ 
       UTCTime	*aux_current_UTCTime	();
       char	*aux_readable_UTCTime	(UTCTime *tx);
       UTCTime	*aux_delta_UTCTime	(UTCTime *time);
       int	aux_interval_UTCTime	(UTCTime *time, UTCTime *notbefore, UTCTime *notafter);
       int	aux_cmp_UTCTime	(UTCTime *time1, UTCTime *time2);
       UTCTime	*get_nextUpdate	(UTCTime *lastUpdate);
       UTCTime	*get_date_of_expiry	();


/***************************************************************
 *
 * Headers from file secure/sec_ed.c
 *
 ***************************************************************/ 
       OctetString	*e_AlgId	(AlgId *algid);
       AlgId	*d_AlgId	(OctetString *asn1_string);
       int	d2_AlgId	(OctetString *asn1_string, AlgId *aid);
       OctetString	*e_KeyInfo	(KeyInfo *ki);
       KeyInfo	*d_KeyInfo	(OctetString *asn1_string);
       int	d2_KeyInfo	(OctetString *asn1_string, KeyInfo *ki);
       OctetString	*e_EncryptedKey	(EncryptedKey *enki);
       EncryptedKey	*d_EncryptedKey	(OctetString *asn1_string);
       int	d2_EncryptedKey	(OctetString *asn1_string, EncryptedKey *enki);
       OctetString	*e_Signature	(Signature *sig);
       Signature	*d_Signature	(OctetString *asn1_string);
       int	d2_Signature	(OctetString *asn1_string, Signature *sig);
       BitString	*e_KeyBits	(KeyBits *kb);
       int	e2_KeyBits	(KeyBits *kb, BitString *bstr);
       KeyBits	*d_KeyBits	(BitString *asn1_bstr);
       OctetString	*e_PSEToc	(PSEToc *toc);
       PSEToc	*d_PSEToc	(OctetString *asn1_string);
       OctetString	*e_PSEObject	(ObjId *objectType, OctetString *objectValue);
       OctetString	*d_PSEObject	(ObjId *objectType, OctetString *asn1_string);
       OctetString	*e_OctetString	(OctetString *ostr);
       OctetString	*d_OctetString	(OctetString *asn1_string);
       OctetString	*e_BitString	(BitString *bstr);
       BitString	*d_BitString	(OctetString *asn1_string);
       OctetString	*e_DigestInfo	(DigestInfo *di);
       DigestInfo	*d_DigestInfo	(OctetString *asn1_string);
       int	d2_DigestInfo	(OctetString *asn1_string, DigestInfo *di);
       OctetString	*e_GRAPHICString	(char *string);
       char	*d_GRAPHICString	(OctetString *asn1_string);
       OctetString *e_INTEGER(OctetString *ostr);
       OctetString *d_INTEGER(OctetString *asn1_string);
/***************************************************************
 *
 * Headers from file secure/sec_rand.c
 *
 ***************************************************************/ 
       RC	sec_init_random	(L_NUMBER seed[]);
       RC	sec_get_random	();
       RC	sec_update_random	();
       void	sec_random_destroy_seed	();
       OctetString	*sec_random_ostr	(unsigned int noctets);
       BitString	*sec_random_bstr	(unsigned int nbits);
       char	*sec_random_str	(int nchars, char *chars);
       RC           sec_random_int          (int r1, int r2, int *ret);
       RC           sec_random_long         (long r1, long r2, long *ret);
/***************************************************************
 *
 * Headers from file secure/secure.c
 *
 ***************************************************************/ 
       RC	sec_chpin	(PSESel *pse_sel, char *newpin);
       RC	sec_close	(PSESel *pse_sel);
       RC	sec_create	(PSESel *pse_sel);
       RC	create_PSE	(PSESel *pse_sel);
       RC	sec_decrypt	(BitString *in_bits, OctetString *out_octets, More more, Key *key);
       RC	sec_decrypt_PKCS	(BitString *in_bits, OctetString *out_octets, More more, Key *key, Boolean sc);
       RC	sec_del_key	(KeyRef keyref);
       RC	sec_delete	(PSESel *pse_sel);
       RC	sec_rename	(PSESel *pse_sel, char *objname);
       RC	sec_encrypt	(OctetString *in_octets, BitString *out_bits, More more, Key *key);
       RC	sec_encrypt_PKCS	(OctetString *in_octets, BitString *out_bits, More more, Key *key, Boolean sc);
       RC	sec_gen_key	(Key *key, Boolean replace);
       RC	sec_get_EncryptedKey	(EncryptedKey *encrypted_key, Key *plain_key, Key *encryption_key);
       int	sec_get_key	(KeyInfo *keyinfo, KeyRef keyref, Key *key);
       int	sec_get_keysize	(KeyInfo *keyinfo);
       RC	sec_hash	(OctetString *in_octets, OctetString *hash_result, More more, AlgId *alg_id, HashInput *hash_input);
       int	sec_keysize	(KeyInfo *keyinfo);
       RC	sec_open	(PSESel *pse_sel);
       RC	sec_print_toc	(FILE *ff, PSESel *pse_sel);
       PSEToc	*sec_read_toc	(PSESel *pse_sel);
       RC	sec_read_tocs	(PSESel *pse_sel, PSEToc **SCtoc, PSEToc **PSEtoc);
       RC	sec_put_EncryptedKey	(EncryptedKey *encrypted_key, Key *plain_key, Key *decryption_key, Boolean replace);
       KeyRef	sec_put_key	(KeyInfo *keyinfo, KeyRef keyref);
       RC	sec_read	(PSESel *pse_sel, OctetString *content);
       RC	sec_read_PSE	(PSESel *pse_sel, ObjId *type, OctetString *value);
       int	sec_write_toc	(PSESel *pse_sel, PSEToc *toc);
       PSEConfig 	sec_pse_config	(PSESel *pse_sel);
       PSELocation	sec_psetest	(char *app_name);
       SCTDevice	sec_scttest	();
       void	sec_set_sct	(int sct_id);
       RC	sec_sign	(OctetString *in_octets, Signature *signature, More more, Key *key, HashInput *hash_input);
       RC	sec_string_to_key	(char *pin, Key *des_key);
       RC	sec_verify	(OctetString *in_octets, Signature *signature, More more, Key *key, HashInput *hash_input);
       RC	sec_checkSK	(Key *sk, KeyInfo *pkinfo);
       RC	sec_unblock_SCpin	(PSESel *pse_sel);
       RC	sec_write	(PSESel *pse_sel, OctetString *content);
       RC	sec_write_PSE	(PSESel *pse_sel, ObjId *type, OctetString *value);
       Boolean	sec_pin_check	(PSESel *pse_sel, char *obj, char *pin);
       KeyInfo	*get_keyinfo_from_key	(Key *key);
       int	get2_keyinfo_from_key	(KeyInfo *keyinfo, Key *key);
       KeyInfo	*get_keyinfo_from_keyref	(KeyRef keyref);
       RC	put_keyinfo_according_to_key	(KeyInfo *keyinfo, Key *key, ObjId *objid);
       char	*get_unixname	();
       Boolean	is_key_pool	(PSESel *psesel);
       char	*sec_read_pin	(char *text, char *object, int reenter);
       PSEToc	*chk_toc	(PSESel *pse_sel, Boolean create);
       void	strzfree	(char **str);
       int	strrep	(char **str1, char *str2);
       int	handle_in_SCTSC	(Key *key, Boolean SC_crypt);
       PSEToc	*read_SCToc	(PSESel *pse_sel);
       RC	write_SCToc	(PSESel *pse_sel, PSEToc *sc_toc);
       PSEToc	*create_SCToc	(PSESel *pse_sel);
       RC	update_SCToc	(PSESel *pse_sel, int length, int st);
       RC	delete_SCToc	(PSESel *pse_sel);
       Boolean	is_in_SCToc	(PSESel *pse_sel);
       PSEToc	*chk_SCToc	(PSESel *pse_sel);
       RC	get_update_time_SCToc	(PSESel *pse_sel, UTCTime **update_time);



/***************************************************************
 *
 * Headers from file crypt/rsa/rsa.c
 *
 ***************************************************************/ 
       RC	rsa_get_key	(char *key, int keytype);
       RC	rsa_encrypt	(OctetString *in, BitString *out, More more, int keysize);
       RC	rsa_decrypt	(BitString *in, OctetString *out, More more, int keysize);
       RC	hash_sqmodn	(OctetString *in, OctetString *sum, More more, int keysize);
       RC	rsa_sign	(OctetString *hash, BitString *sign);
       RC	rsa_verify	(OctetString *hash, BitString *sign);
       RC	rsa_encblock2OctetString	(BitString *sign, OctetString *hash);
/***************************************************************
 *
 * Headers from file crypt/rsa/rsagen.c
 *
 ***************************************************************/ 
       RC	rsa_gen_key	(int keysize, BitString **skey, BitString **pkey);
/***************************************************************
 *
 * Headers from file crypt/dsa/dsa.c
 *
 ***************************************************************/ 
       RC	dsa_get_key	(char *key, int keytype);
       RC	dsa_sign	(OctetString *hash, BitString *sign);
       RC	dsa_verify	(OctetString *hash, BitString *sign);
/***************************************************************
 *
 * Headers from file crypt/dsa/dsagen.c
 *
 ***************************************************************/ 
       RC	dsa_gen_key	(int keysize, BitString **skey, BitString **pkey);
/***************************************************************
 *
 * Headers from file crypt/des/des.c
 *
 ***************************************************************/ 
       int	desinit	(int mode, Boolean des3);
       int	desdone	(Boolean des3);
       int	setkey_	(char *key);
       int	setkey1	(char *key);
       int	endes	(char *block);
       int	endes1	(char *block);
       int	dedes	(char *block);
       int	dedes1	(char *block);
       long	f	(unsigned long r, unsigned char subkey[8]);
       unsigned long	byteswap	(unsigned long x);
/***************************************************************
 *
 * Headers from file crypt/des/des_if.c
 *
 ***************************************************************/ 
       int	des_encrypt	(OctetString *in_octets, BitString *out_bits, More more, KeyInfo *keyinfo);
       void	endes_cbc	(char *outblock);
       void	endes_ecb	(char *outblock);
       int	des_decrypt	(BitString *in_bits, OctetString *out_octets, More more, KeyInfo *keyinfo);
       void	dedes_cbc	(char *outblock);
       void	dedes_ecb	(char *outblock);
       int	setdoublekey	(char *key);
       int	read_dec	(int fd, char *buf, int len, char *key);
       int	write_enc	(int fd, char *buf, int len, char *key);
       int	close_dec	(int fd);
       int	close_enc	(int fd);
       int	c_desdone	(Boolean des3);
       char	*string_to_key	(char *asckey);
/***************************************************************
 *
 * Headers from file crypt/md/md2_if.c
 *
 ***************************************************************/ 
       RC	md2_hash	(OctetString *in_octets, OctetString *hash_result, More more);
/***************************************************************
 *
 * Headers from file crypt/md/md4_if.c
 *
 ***************************************************************/ 
       RC	md4_hash	(OctetString *in_octets, OctetString *hash_result, More more);
/***************************************************************
 *
 * Headers from file crypt/md/md5_if.c
 *
 ***************************************************************/ 
       RC	md5_hash	(OctetString *in_octets, OctetString *hash_result, More more);
/***************************************************************
 *
 * Headers from file crypt/sha/sha_if.c
 *
 ***************************************************************/ 
       RC	sha_hash	(OctetString *in_octets, OctetString *hash_result, More more);

/***************************************************************
 *
 * Headers from file crypt/dh/dh.c
 *
 ***************************************************************/ 
       AlgId    *sec_DH_init(int size_of_p, int privat_value_length);
       KeyInfo 	*sec_DH_phase1(AlgId *key_pg, Key *key_x, Boolean	 with_pg);
       BitString *sec_DH_phase2(AlgId *key_pg, Key *key_x, KeyInfo *peer_y);






#else




#ifdef _PSAP_
/***************************************************************
 *
 * Headers from file secure/SEC.c
 *
 ***************************************************************/ 

/***************************************************************
 *
 * Headers from file secure/sec_ed.c
 *
 ***************************************************************/ 
       KeyBits	* dec_RSAAlgorithm	();
       KeyBits	* dec_DSAAlgorithm	();
       int        enc_f    ();
       int        dec_f    ();
       int        prnt_f   ();
       int        fre_obj  ();

#endif
       char *getpass();



/***************************************************************
 *
 * Headers from file aux/aux_fprint.c
 *
 ***************************************************************/ 
       RC aux_fprint_OctetString();
       RC aux_fprint_BitString();
       RC aux_fprint_PSESel();
       RC aux_fprint_Version();
       RC aux_fprint_version();
       RC aux_fprint_PSEToc();
       RC aux_fprint_KeyInfo();
       RC aux_fprint_KeyBits();
       RC aux_fprint_ObjId();
       RC aux_fprint_AlgId();
       RC aux_fprint_Serialnumber();
       RC aux_fprint_error();

       char *aux_sprint_OctetString();
       char *aux_sprint_BitString();
       char *aux_sprint_PSESel();
       char *aux_sprint_Version();
       char *aux_sprint_version();
       char *aux_sprint_PSEToc();
       char *aux_sprint_KeyInfo();
       char *aux_sprint_KeyBits();
       char *aux_sprint_ObjId();
       char *aux_sprint_AlgId();
       char *aux_sprint_Serialnumber();
       char *aux_sprint_error();

       RC aux_print_OctetString();
       RC aux_print_BitString();
       RC aux_print_PSESel();
       RC aux_print_Version();
       RC aux_print_version();
       RC aux_print_PSEToc();
       RC aux_print_KeyInfo();
       RC aux_print_KeyBits();
       RC aux_print_ObjId();
       RC aux_print_AlgId();
       RC aux_print_Serialnumber();
       RC aux_print_error();
/***************************************************************
 *
 * Headers from file aux/aux_util.c
 *
 ***************************************************************/ 
       void	aux_add_error	();
       int	aux_last_error	();
       ObjId	*aux_cpy_ObjId	();
       PSESel	*aux_cpy_PSESel	();
       int	aux_cpy2_ObjId	();
       int	aux_cmp_ObjId	();
       AlgId	*aux_cpy_AlgId	();
       int	aux_cmp_AlgId	();
       int	aux_cmp_KeyBits	();
       KeyInfo	*aux_cpy_KeyInfo	();
       int	aux_cpy2_KeyInfo	();
       int	aux_cmp_BitString	();
       int	aux_cmp_KeyInfo	();
       int	aux_cmp_Signature	();
       int 	aux_cmp_OctetString();
       char	*aux_ObjId2PSEObjectName	();
       ObjId	*aux_PSEObjectName2ObjId	();
       ObjId	*aux_Name2ObjId	();
       AlgId	*aux_Name2AlgId	();
       AlgType	aux_Name2AlgType	();
       AlgEnc	aux_Name2AlgEnc	();
       AlgHash	aux_Name2AlgHash	();
       AlgMode	aux_Name2AlgMode	();
       AlgSpecial	aux_Name2AlgSpecial	();
       ParmType	aux_Name2ParmType	();
       char	*aux_ObjId2Name	();
       AlgType	aux_ObjId2AlgType	();
       AlgEnc	aux_ObjId2AlgEnc	();
       AlgHash	aux_ObjId2AlgHash	();
       AlgSpecial	aux_ObjId2AlgSpecial	();
       AlgMode	aux_ObjId2AlgMode	();
       AlgId	*aux_ObjId2AlgId	();
       ParmType	aux_ObjId2ParmType	();
       ObjId	*aux_oid2syntaxoid	();
       char	*aux_cpy_Name	();
       char	*aux_cpy_String	();
       char	*aux_cpy_ReducedString	();
       int	aux_pstrcmp	();
       int	aux_lexequ	();
       int	aux_cmp_Name	();
       ObjId	*aux_keyword2oid	();
       ObjId	*aux_keyword2syntaxoid	();
       char	*aux_oid2keyword	();
       int	aux_cpy2_OctetString	();
       int	aux_cpy2_BitString	();
       OctetString	*aux_cpy_OctetString	();
       BitString	*aux_cpy_BitString	();
       PSEToc	*aux_cpy_PSEToc	();
       struct PSE_Objects	*aux_cpy_PSEObjects	();
       PSEObject	*aux_cpy_PSEObject	();
       KeyBits	*aux_cpy_KeyBits	();
       Signature	*aux_cpy_Signature	();
       int	aux_check_3166	();
       int	aux_check_print_string	();
/*       int	strcasecmp	();*/
       RC 	aux_input_from_device();
       char 	*aux_getpass();
       RC 	aux_sappend_char();
       char     *CATSPRINTF();
       char 	*CATNSTR();
/***************************************************************
 *
 * Headers from file aux/aux_xdmp.c
 *
 ***************************************************************/ 
       void	aux_fxdumpp	();
       void	aux_fxdump	();
       void	aux_fxdump2	();
       char	*aux_sxdump	();
       char	*aux_sxdump2	();
       void	aux_xdump	();
       void	aux_xdump2	();


/***************************************************************
 *
 * Headers from file aux/aux_PKCS.c
 *
 ***************************************************************/ 
       OctetString	*aux_create_PKCSBlock	();
       OctetString	*aux_create_PKCS_MIC_D	();
/***************************************************************
 *
 * Headers from file aux/aux_can.c
 *
 ***************************************************************/ 
       OctetString	*aux_canon	();
       OctetString	*aux_decanon	();
       OctetString	*aux_64	();
       OctetString	*aux_de64	();
/***************************************************************
 *
 * Headers from file aux/aux_conv.c
 *
 ***************************************************************/ 
       OctetString	*aux_encrfc	();
       OctetString	*aux_decrfc	();
       OctetString	*aux_enchex	();
       OctetString	*aux_dechex	();
#ifdef APCONV
       OctetString	*aux_encap	();
       OctetString	*aux_decap	();
#endif
/***************************************************************
 *
 * Headers from file aux/aux_ed.c
 *
 ***************************************************************/ 
#ifdef _PSAP_
       	OctetString	*aux_PE2OctetString	();
       	PE		aux_OctetString2PE	();
	PE		aux_DER_SETOF		();
#endif
/***************************************************************
 *
 * Headers from file aux/aux_file.c
 *
 ***************************************************************/ 
       OctetString	*aux_file2OctetString	();
       int	aux_OctetString2file	();

/***************************************************************
 *
 * Headers from file aux/aux_imp.c
 *
 ***************************************************************/ 
       int	catstr	();
       char	*str_low	();
       char	*str_up	();
       int	Trim	();
       int	Skip_blanks	();
       int	ScanOp	();
       char	*Scanv	();
/***************************************************************
 *
 * Headers from file aux/aux_malloc.c
 *
 ***************************************************************/ 
       void	mftrace	();
       char	*aux_malloc	();
       char	*aux_calloc	();
       char	*aux_realloc	();
       void	aux_free	();
       void	MF_fprint	();
       void	MF_fprint_stderr	();
/***************************************************************
 *
 * Headers from file aux/aux_ostr.c
 *
 ***************************************************************/ 
       OctetString	*aux_create_OctetString	();
       OctetString	*aux_new_OctetString	();
       RC	aux_append_OctetString	();
       RC	aux_append_char	();
       OctetString * aux_concatenate_OctetString ();
       RC	aux_hex_append	();
       RC	aux_searchitem	();
       OctetString	*aux_ostr_get64	();
       RC	aux_cmpitem	();
       OctetString 	*aux_create_SerialNo();
       int 	aux_OctetString2int();
       OctetString 	*aux_int2OctetString();
       RC aux_prepend_dash_();
       RC aux_strip_dash_();
       RC aux_OctetString_delete_string();
       RC aux_OctetString_insert_string();

/***************************************************************
 *
 * Headers from file aux/aux_free.c
 *
 ***************************************************************/ 
       void	aux_free_OctetString	();
       void	aux_free2_OctetString	();
       void	aux_free_BitString	();
       void	aux_free2_BitString	();
       void	aux_free_ObjId	();
       void	aux_free2_ObjId	();
       void	aux_free_AlgId	();
       void	aux_free2_KeyInfo	();
       void	aux_free2_Signature	();
       void	aux_free_KeyInfo	();
       void	aux_free_Signature	();
       void	aux_free2_DigestInfo	();
       void	aux_free_DigestInfo	();
       void	aux_free_KeyBits	();
       void	aux_free2_KeyBits	();
       void	aux_free2_PSEToc	();
       void	aux_free2_PSESel	();
       void	aux_free_PSEToc	();
       void	aux_free_PSESel	();
       void	aux_free2_AlgId	();
/***************************************************************
 *
 * Headers from file aux/aux_time.c
 *
 ***************************************************************/ 
       UTCTime	*aux_current_UTCTime	();
       char	*aux_readable_UTCTime	();
       UTCTime	*aux_delta_UTCTime	();
       int	aux_interval_UTCTime	();
       int	aux_cmp_UTCTime	();
       UTCTime	*get_nextUpdate	();
       UTCTime	*get_date_of_expiry	();


/***************************************************************
 *
 * Headers from file secure/sec_ed.c
 *
 ***************************************************************/ 
       OctetString	*e_AlgId	();
       AlgId	*d_AlgId	();
       int	d2_AlgId	();
       OctetString	*e_KeyInfo	();
       KeyInfo	*d_KeyInfo	();
       int	d2_KeyInfo	();
       OctetString	*e_EncryptedKey	();
       EncryptedKey	*d_EncryptedKey	();
       int	d2_EncryptedKey	();
       OctetString	*e_Signature	();
       Signature	*d_Signature	();
       int	d2_Signature	();
       BitString	*e_KeyBits	();
       int	e2_KeyBits	();
       KeyBits	*d_KeyBits	();
       OctetString	*e_PSEToc	();
       PSEToc	*d_PSEToc	();
       OctetString	*e_PSEObject	();
       OctetString	*d_PSEObject	();
       OctetString	*e_OctetString	();
       OctetString	*d_OctetString	();
       OctetString	*e_BitString	();
       BitString	*d_BitString	();
       OctetString	*e_DigestInfo	();
       DigestInfo	*d_DigestInfo	();
       int	d2_DigestInfo	();
       OctetString	*e_GRAPHICString	();
       char	*d_GRAPHICString	();
       OctetString *e_INTEGER();
       OctetString *d_INTEGER();
/***************************************************************
 *
 * Headers from file secure/sec_rand.c
 *
 ***************************************************************/ 
       RC	sec_init_random	();
       RC	sec_get_random	();
       RC	sec_update_random	();
       void	sec_random_destroy_seed	();
       OctetString	*sec_random_ostr	();
       BitString	*sec_random_bstr	();
       char	*sec_random_str	();
       RC           sec_random_int          ();
       RC           sec_random_long         ();
/***************************************************************
 *
 * Headers from file secure/secure.c
 *
 ***************************************************************/ 
       RC	sec_chpin	();
       RC	sec_close	();
       RC	sec_create	();
       RC	create_PSE	();
       RC	sec_decrypt	();
       RC	sec_del_key	();
       RC	sec_delete	();
       RC	sec_rename	();
       RC	sec_encrypt	();
       RC	sec_gen_key	();
       RC	sec_get_EncryptedKey	();
       int	sec_get_key	();
       int	sec_get_keysize	();
       RC	sec_hash	();
       int	sec_keysize	();
       RC	sec_open	();
       RC	sec_print_toc	();
       PSEToc	*sec_read_toc	();
       RC	sec_read_tocs	();
       RC	sec_put_EncryptedKey	();
       KeyRef	sec_put_key	();
       RC	sec_read	();
       RC	sec_read_PSE	();
       int	sec_write_toc	();
       PSEConfig 	sec_pse_config	();
       PSELocation	sec_psetest	();
       SCTDevice	sec_scttest	();

       void	sec_set_sct	();
       RC	sec_sign	();
       RC	sec_string_to_key	();
       RC	sec_verify	();
       RC	sec_checkSK	();
       RC	sec_unblock_SCpin	();
       RC	sec_write	();
       RC	sec_write_PSE	();
       Boolean	sec_pin_check	();
       KeyInfo	*get_keyinfo_from_key	();
       int	get2_keyinfo_from_key	();
       KeyInfo	*get_keyinfo_from_keyref();
       RC	put_keyinfo_according_to_key();
       char	*get_unixname	();
       Boolean	is_key_pool	();
       char	*sec_read_pin	();
       PSEToc	*chk_toc	();
       void	strzfree	();
       int	strrep	();
       int	handle_in_SCTSC	();
       PSEToc	*read_SCToc	();
       RC	write_SCToc	();
       PSEToc	*create_SCToc	();
       RC	update_SCToc	();
       RC	delete_SCToc	();
       Boolean	is_in_SCToc	();
       PSEToc	*chk_SCToc	();
       RC	get_update_time_SCToc	();


/***************************************************************
 *
 * Headers from file crypt/rsa/rsablock.c
 *
 ***************************************************************/ 
       void	rsa_encblock	();
       void	rsa_decblock	();
/***************************************************************
 *
 * Headers from file crypt/rsa/rsagen.c
 *
 ***************************************************************/ 
       RC	rsa_gen_key	();
/***************************************************************
 *
 * Headers from file crypt/dsa/dsa.c
 *
 ***************************************************************/ 
       RC	dsa_get_key	();
       RC	dsa_sign	();
       RC	dsa_verify	();
/***************************************************************
 *
 * Headers from file crypt/dsa/dsablock.c
 *
 ***************************************************************/ 
       void	ln_ggt	();
       void	ln_inv	();
       void	dsa_signblock	();
       int	dsa_verifyblock	();
/***************************************************************
 *
 * Headers from file crypt/dsa/dsagen.c
 *
 ***************************************************************/ 
       RC	dsa_gen_key	();
/***************************************************************
 *
 * Headers from file crypt/des/des.c
 *
 ***************************************************************/ 
       int	desinit	();
       int	desdone	();
       int	setkey_	();
       int	setkey1	();
       int	endes	();
       int	endes1	();
       int	dedes	();
       int	dedes1	();
       long	f	();
       unsigned long	byteswap	();
/***************************************************************
 *
 * Headers from file crypt/des/des_if.c
 *
 ***************************************************************/ 
       int	des_encrypt	();
       void	endes_cbc	();
       void	endes_ecb	();
       int	des_decrypt	();
       void	dedes_cbc	();
       void	dedes_ecb	();
       int	setdoublekey	();
       int	read_dec	();
       int	write_enc	();
       int	close_dec	();
       int	close_enc	();
       int	c_desdone	();
       char	*string_to_key	();
/***************************************************************
 *
 * Headers from file crypt/md/md2_if.c
 *
 ***************************************************************/ 
       RC	md2_hash	();
/***************************************************************
 *
 * Headers from file crypt/md/md4_if.c
 *
 ***************************************************************/ 
       RC	md4_hash	();
/***************************************************************
 *
 * Headers from file crypt/md/md5_if.c
 *
 ***************************************************************/ 
       RC	md5_hash	();
/***************************************************************
 *
 * Headers from file crypt/sha/sha_if.c
 *
 ***************************************************************/ 
       RC	sha_hash	();
/***************************************************************
 *
 * Headers from file crypt/dh/dh.c
 *
 ***************************************************************/ 
       AlgId    *sec_DH_init();
       KeyInfo 	*sec_DH_phase1();
       BitString *sec_DH_phase2();
#endif
