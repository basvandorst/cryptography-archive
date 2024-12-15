
#ifdef __STDC__

/***************************************************************
 *
 * Headers from file aux/aux_fprint.c
 *
 ***************************************************************/ 
       RC aux_fprint_GRAPHICString(FILE *ff, char *graphic);
       RC aux_fprint_AliasList(FILE *ff, AliasList *aliaslist);
       RC aux_fprint_DNameOrAlias(FILE *ff, DName *dname);
       RC aux_fprint_NameOrAlias(FILE *ff, Name	 *name);
       RC aux_fprint_FCPath(FILE *ff, FCPath	 *fcpath);
       RC aux_fprint_Certificates(FILE *ff, Certificates	 *certs);
       RC aux_fprint_RootInfo(FILE *ff, Serial	 *rootinfo, Boolean	  new);
       RC aux_fprint_PKRoot(FILE *ff, PKRoot	 *pkroot);
       RC aux_fprint_PKList(FILE *ff, PKList	 *pklist);
       RC aux_fprint_Certificate(FILE *ff, Certificate	 *cert);
       RC aux_fprint_ToBeSigned(FILE *ff, ToBeSigned	 *tbs);
#ifdef COSINE
       RC aux_fprint_AuthorisationAttributes(FILE *ff, AuthorisationAttributes	 *authatts);
#endif
       RC aux_fprint_CertificateSet(FILE *ff, SET_OF_Certificate	 *certset);
       RC aux_fprint_CertificatePairSet(FILE *ff, SET_OF_CertificatePair	 *cpair);
       RC aux_fprint_Crl(FILE *ff, Crl	 *crlpse);
       RC aux_fprint_CrlSet(FILE *ff, CrlSet	 *crlset);
       RC aux_fprint_IssuedCertificate(FILE *ff, IssuedCertificate	 *isscert);
       RC aux_fprint_SET_OF_IssuedCertificate(FILE *ff, SET_OF_IssuedCertificate	 *isscertset);
       RC aux_fprint_SET_OF_Name(FILE *ff, SET_OF_Name	 *nameset);
       RC aux_fprint_CRLWithCertificates(FILE *ff, CRLWithCertificates	 *arg);
       RC aux_fprint_SET_OF_CRLWithCertificates(FILE *ff, SET_OF_CRLWithCertificates	 *set);
       RC aux_fprint_VerificationResult(FILE *ff, VerificationResult	 *verres);
       RC aux_fprint_TrustPath(FILE *ff, VerificationResult	 *verres);
       RC aux_fprint_CRLEntry(FILE *ff, CRLEntry	 *crlentry);
       RC aux_fprint_CRL(FILE *ff, CRL	 *crl);
       RC aux_fprint_CRLTBS(FILE *ff, CRLTBS	 *tbs);
       RC aux_fprint_OCList(FILE *ff, OCList	 *oclist);
       RC aux_fprint_Validity(FILE *ff, Validity *valid);
       RC aux_fprint_validity_timeframe(FILE *ff, Validity  *valid);

       char *aux_sprint_GRAPHICString(char *string, char *graphic);
       char *aux_sprint_AliasList(char *string, AliasList *aliaslist);
       char *aux_sprint_DNameOrAlias(char *string, DName	 *dname);
       char *aux_sprint_NameOrAlias(char *string, Name	 *name);
       char *aux_sprint_FCPath(char *string, FCPath	 *fcpath);
       char *aux_sprint_Certificates(char *string, Certificates	 *certs);
       char *aux_sprint_RootInfo(char *string, Serial	 *rootinfo, Boolean	  new);
       char *aux_sprint_PKRoot(char *string, PKRoot	 *pkroot);
       char *aux_sprint_PKList(char *string, PKList	 *pklist);
       char *aux_sprint_Certificate(char *string, Certificate	 *cert);
       char *aux_sprint_ToBeSigned(char *string, ToBeSigned	 *tbs);
#ifdef COSINE
       char *aux_sprint_AuthorisationAttributes(char *string, AuthorisationAttributes	 *authatts);
#endif
       char *aux_sprint_CertificateSet(char *string, SET_OF_Certificate	 *certset);
       char *aux_sprint_CertificatePairSet(char *string, SET_OF_CertificatePair	 *cpair);
       char *aux_sprint_Crl(char *string, Crl	 *crlpse);
       char *aux_sprint_CrlSet(char *string, CrlSet	 *crlset);
       char *aux_sprint_IssuedCertificate(char *string, IssuedCertificate	 *isscert);
       char *aux_sprint_SET_OF_IssuedCertificate(char *string, SET_OF_IssuedCertificate	 *isscertset);
       char *aux_sprint_SET_OF_Name(char *string, SET_OF_Name	 *nameset);
       char *aux_sprint_CRLWithCertificates(char *string, CRLWithCertificates	 *arg);
       char *aux_sprint_SET_OF_CRLWithCertificates(char *string, SET_OF_CRLWithCertificates	 *set);
       char *aux_sprint_VerificationResult(char *string, VerificationResult	 *verres);
       char *aux_sprint_TrustPath(char *string, VerificationResult	 *verres);
       char *aux_sprint_CRLEntry(char *string, CRLEntry	 *crlentry);
       char *aux_sprint_CRL(char *string, CRL	 *crl);
       char *aux_sprint_CRLTBS(char *string, CRLTBS	 *tbs);
       char *aux_sprint_OCList(char *string, OCList	 *oclist);
       char *aux_sprint_Validity(char *string, Validity *valid);
       char *aux_sprint_validity_timeframe(char *string, Validity  *valid);

       RC aux_print_GRAPHICString(char *graphic);
       RC aux_print_AliasList(AliasList *aliaslist);
       RC aux_print_DNameOrAlias(DName	 *dname);
       RC aux_print_NameOrAlias(Name	 *name);
       RC aux_print_FCPath(FCPath	 *fcpath);
       RC aux_print_Certificates(Certificates	 *certs);
       RC aux_print_RootInfo(Serial	 *rootinfo, Boolean	  new);
       RC aux_print_PKRoot(PKRoot	 *pkroot);
       RC aux_print_PKList(PKList	 *pklist);
       RC aux_print_Certificate(Certificate	 *cert);
       RC aux_print_ToBeSigned(ToBeSigned	 *tbs);
#ifdef COSINE
       RC aux_print_AuthorisationAttributes(AuthorisationAttributes	 *authatts);
#endif
       RC aux_print_CertificateSet(SET_OF_Certificate	 *certset);
       RC aux_print_CertificatePairSet(SET_OF_CertificatePair	 *cpair);
       RC aux_print_Crl(Crl	 *crlpse);
       RC aux_print_CrlSet(CrlSet	 *crlset);
       RC aux_print_IssuedCertificate(IssuedCertificate	 *isscert);
       RC aux_print_SET_OF_IssuedCertificate(SET_OF_IssuedCertificate	 *isscertset);
       RC aux_print_SET_OF_Name(SET_OF_Name	 *nameset);
       RC aux_print_CRLWithCertificates(CRLWithCertificates	 *arg);
       RC aux_print_SET_OF_CRLWithCertificates(SET_OF_CRLWithCertificates	 *set);
       RC aux_print_VerificationResult(VerificationResult	 *verres);
       RC aux_print_TrustPath(VerificationResult	 *verres);
       RC aux_print_CRLEntry(CRLEntry	 *crlentry);
       RC aux_print_CRL(CRL	 *crl);
       RC aux_print_CRLTBS(CRLTBS	 *tbs);
       RC aux_print_OCList(OCList	 *oclist);
       RC aux_print_Validity(Validity *valid);
       RC aux_print_validity_timeframe(Validity  *valid);
/***************************************************************
 *
 * Headers from file aux/aux_free.c
 *
 ***************************************************************/ 
       void	RDName_comp_free	(RDName *rdn);
       void	aux_free2_RDName	(RDName *rdn);
       void	aux_free_RDName	(RDName **rdn);
       void	DName_comp_free	(DName *dn);
       void	aux_free2_DName	(DName *dn);
       void	aux_free_DName	(DName **dn);
       void	aux_free2_ToBeSigned	(ToBeSigned *tbs);
       void	aux_free_ToBeSigned	(ToBeSigned **tbs);
       void	aux_free2_Certificate	(Certificate *cert);
       void	aux_free_Certificate	(Certificate **cert);
       void	aux_free2_Key	(Key *key);
       void	aux_free_Key	(Key **key);
       void	aux_free_CertificateSet	(SET_OF_Certificate **certset);
       void	aux_free2_CertificatePair	(CertificatePair *cpair);
       void	aux_free_CertificatePair	(CertificatePair **cpair);
       void	aux_free_CertificatePairSet	(SET_OF_CertificatePair **cpairset);
       void	aux_free_CrossCertificates	(CrossCertificates **ccerts);
       void	aux_free2_CrossCertificates	(CrossCertificates *ccerts);
       void	aux_free_FCPath	(FCPath **path);
       void	aux_free_Certificates	(Certificates **certs);
       void	aux_free_CRLWithCertificates	(CRLWithCertificates **crlwithcerts);
       void	aux_free_SET_OF_CRLWithCertificates	(SET_OF_CRLWithCertificates **set);
       void	aux_free_CertificationPath	(CertificationPath **certpath);
       void	aux_free_CertificatePairs	(CertificatePairs **pairs);
       void	aux_free_SEQUENCE_OF_CertificatePair	(SEQUENCE_OF_CertificatePair **seq);
       void	aux_free_PKRoot	(PKRoot **pkroot);
       void	aux_free_RootInfo	(Serial **rootinfo);
       void	aux_free2_RootInfo	(Serial *rootinfo);
       void	aux_free_PKList	(PKList **pklist);
       void	aux_free_error	();
       void	aux_free2_Crl	(Crl *crlpse);
       void	aux_free_Crl	(Crl **crlpse);
       void	aux_free_CrlSet	(CrlSet **crlset);
       void	aux_free2_CRLEntry	(CRLEntry *crlentry);
       void	aux_free_CRLEntry	(CRLEntry **crlentry);
       void	aux_free_SEQUENCE_OF_CRLEntry	(SEQUENCE_OF_CRLEntry **seq);
       void	aux_free2_CRLTBS	(CRLTBS *tbs);
       void	aux_free_CRLTBS	(CRLTBS **tbs);
       void	aux_free2_CRL	(CRL *crl);
       void	aux_free_CRL	(CRL **crl);
       void	aux_free_OCList	(OCList **ocl);
       void	aux_free_IssuedCertificate	(IssuedCertificate **isscert);
       void	aux_free2_IssuedCertificate	(IssuedCertificate *isscert);
       void	aux_free_SET_OF_IssuedCertificate	(SET_OF_IssuedCertificate **isscertset);
       void	aux_free_VerificationStep	(VerificationStep **verstep);
       void	aux_free_VerificationResult	(VerificationResult **verres);
       void	aux_free2_VerificationResult	(VerificationResult *verres);
       void	aux_free_Validity	(Validity **valid);
       void	aux_free2_Validity	(Validity *valid);
       void	aux_free_AliasList	(AliasList **aliaslist);
/***************************************************************
 *
 * Headers from file aux/aux_util.c
 *
 ***************************************************************/ 
       int	aux_create_AFPSESel	(char *appname, char *pin);
       RC	aux_set_pse	(char *psename, char *cadir);
       int	RDName_comp_cmp	(RDName *a, RDName *b);
       int	aux_cmp_RDName	(RDName *a, RDName *b);
       int	aux_cmp_DName	(DName *a, DName *b);
       RC	aux_checkPemDNameSubordination	(DName *sup, DName *sub);
       Name	*aux_PemDNameSubordination	(DName *sup, DName *sub);
       Name	*aux_PCA	(VerificationResult *vr);
       RDName	*aux_cpy_RDName_comp	(RDName *rdn);
       RDName	*aux_cpy_RDName	(RDName *rdn);
       DName	*aux_cpy_DName_comp	(DName *dn);
       DName	*aux_cpy_DName	(DName *dn);
       int	aux_cpy2_DName_comp	(DName *dup_dname, DName *dname);
       int	aux_cpy2_DName	(DName *dup_dname, DName *dname);
       Certificate	*aux_cpy_Certificate	(Certificate *cert);
       int	aux_cpy2_Certificate	(Certificate *dup_cert, Certificate *cert);
       PKList	*aux_cpy_PKList	(PKList *list);
       Certificates	*aux_cpy_Certificates	(Certificates *certs);
       CertificatePair	*aux_cpy_CertificatePair	(CertificatePair *certs);
       SET_OF_Certificate	*aux_cpy_SET_OF_Certificate	(SET_OF_Certificate *certs);
       SET_OF_Name	*aux_cpy_SET_OF_Name	(SET_OF_Name *nameset);
       SET_OF_CertificatePair	*aux_cpy_SET_OF_CertificatePair	(SET_OF_CertificatePair *certs);
       FCPath	*aux_cpy_FCPath	(FCPath *fcpath);
       ToBeSigned	*aux_cpy_ToBeSigned	(ToBeSigned *tbs);
       Key	*aux_cpy_Key	(Key *key);
       PKRoot	*aux_cpy_PKRoot	(PKRoot *pkr);
       IssuedCertificate	*aux_cpy_IssuedCertificate	(IssuedCertificate *isscert);
       SET_OF_IssuedCertificate	*aux_cpy_SET_OF_IssuedCertificate	(SET_OF_IssuedCertificate *isscertset);
       Crl	*aux_cpy_Crl	(Crl *crlpse);
       CrlSet	*aux_cpy_CrlSet	(CrlSet *crlset);
       struct Serial	*aux_cpy_Serial	(struct Serial *serial);
       AliasList * aux_cpy_AliasList (AliasList *alist);
       Aliases * aux_cpy_Aliases (Aliases *aliases);
       Certificates	*aux_create_Certificates	(Certificate *cert, FCPath *fcpath);
       PKRoot	*aux_create_PKRoot	(Certificate *cert1, Certificate *cert2);
       FCPath	*aux_create_FCPath	(Certificate *cert);
       Certificate	*aux_create_Certificate	(ToBeSigned *tbs);
       int	aux_cmp_Certificate	(Certificate *a, Certificate *b);
       int	aux_cmp_CertificatePair	(CertificatePair *a, CertificatePair *b);
       CRLEntry	*aux_cpy_CRLEntry	(CRLEntry *crlentry);
       SEQUENCE_OF_CRLEntry	*aux_cpy_SEQUENCE_OF_CRLEntry	(SEQUENCE_OF_CRLEntry *seq);
       CRL	*aux_cpy_CRL	(CRL *crl);
       CRLTBS	*aux_cpy_CRLTBS	(CRLTBS *tbs);
       OCList	*aux_cpy_OCList	(OCList *ocl);
       Validity	*aux_cpy_Validity (Validity *valid);
       Name * aux_determine_Signer (VerificationResult * verres, Boolean * authentication);
       Key *PSEobj (char *name);

/***************************************************************
 *
 * Headers from file aux/aux_alias.c
 *
 ***************************************************************/ 
       AliasFile	aux_alias	(char *alias);
       DName	*aux_alias2DName	(Name *alias);
       Name	*aux_alias2Name		(Name *alias);
       char	*aux_DName2alias	(DName *dname, AliasType type);
       char	*aux_DName2NameOrAlias	(DName *dname);
       char	*aux_Name2NameOrAlias	(Name *name);
       char	*aux_Name2alias	(Name *name, AliasType type);
       char	*aux_DName2aliasf	(DName *dname, AliasType type, AliasFile afile);
       char	*aux_Name2aliasf	(char *name, AliasType type, AliasFile afile);
       RC	aux_add_alias_name	(char *alias, Name *name, AliasFile aliasf, Boolean prior, Boolean writef);
       RC	aux_add_alias	(char *alias, DName *dn, AliasFile aliasf, Boolean prior, Boolean writef);
       RC	aux_delete_alias	(char *alias, AliasFile aliasf, Boolean writef);
       RC	aux_fprint_alias2dname	(FILE *ff, char *alias);
       RC	aux_fprint_alias2dnames	(FILE *ff, char *alias);
       RC	aux_fprint_dname2alias	(FILE *ff, char *dname);
       Name	*aux_search_AliasList	(Name *name, char *pattern);
       Boolean	aux_alias_chkfile	(Name *name, AliasFile aliasf);
       Name	*aux_next_AliasList	(Name *name);
       Name	*aux_alias_nxtname	(Boolean reset);
       char	*aux_alias_getall	(char *name);
       Boolean	aux_get_AliasList	();
       RC	aux_put_AliasList	(AliasFile aliasf);
       Boolean	aux_check_AliasList	(FILE *ff);
       AliasList	*aux_select_AliasList	(AliasFile aliasf);
       Boolean	aux_alias_writeprotection	();
/***************************************************************
 *
 * Headers from file aux/aux_dname.c
 *
 ***************************************************************/ 
       void	aux_append_RDName	(RDName *a, RDName *b);
       RDName	*str2RDName	(char *str);
       DName	*aux_Name2DName	(char *str);
       void	aux_append_DName	(DName *a, DName *b);
       DName	*DName_comp_new	(RDName *rdn);
       char	*aux_DName2Name	(DName *name_ae);
       char	*aux_DName2Attr	(DName *name_ae, char *attr_key);
       DName	*aux_ORName2DName	(char *str);
       char	*aux_DName2CAPITALName	(DName *name_ae);
       char	*aux_DName2CAPITALString	(DName *name_ae, char *separator);
       char     *aux_DName2afdbname(DName *dname);


/***************************************************************
 *
 * Headers from file af/af_cadb.c
 *
 ***************************************************************/ 
       int	af_cadb_add_user	(Name *name, char *cadir);
       SET_OF_IssuedCertificate	*af_cadb_get_user	(Name *name, char *cadir);
       int	af_cadb_add_Certificate	(KeyType keytype, Certificate *newcert, char *cadir);
       Certificate	*af_cadb_get_Certificate	(OctetString *serial, char *cadir);
       SET_OF_Name	*af_cadb_list_user	(char *cadir);
       char	*logtime	();
       int	af_cadb_add_CRLWithCertificates	(CRLWithCertificates *crlwithcerts, char *cadir);
       SET_OF_CRLWithCertificates	*af_cadb_list_CRLWithCertificates	(char *cadir);
       CRLWithCertificates	*af_cadb_get_CRLWithCertificates	(Name *name, char *cadir);
/***************************************************************
 *
 * Headers from file af/af_crypt.c
 *
 ***************************************************************/ 
       RC	af_encrypt	(OctetString *inoctets, BitString *outbits, More more, Key *key, DName *dname);
       RC	af_decrypt	(BitString *inbits, OctetString *outoctets, More more, Key *key);
       RC	af_sign	(OctetString *inoctets, Signature *signature, More more);
       RC	af_verify	(OctetString *inocts, Signature *sign, More more, Certificates *or_cert, UTCTime *time, PKRoot *pkroot);
       RC	af_verify_Certificates	(Certificates *or_cert, UTCTime *time, PKRoot *pkroot);
       RC	af_check_validity_of_Certificate	(UTCTime *time, Certificate *cert);
       RC	af_check_validity_of_PKRoot	(UTCTime *time, PKRoot *pkroot);
       RC	af_check_validity_of_ToBeSigned	(UTCTime *time, ToBeSigned *tbs);
       FCPath	*reduce_FCPath_to_HierarchyPath	(FCPath *fpath);
       Certificates	*transform_reducedFCPath_into_Certificates	(FCPath *fpath);
       Crl	*CRL2Crl	(CRL *crl);
/***************************************************************
 *
 * Headers from file af/af_dir.c
 *
 ***************************************************************/ 
       int	security_syntaxes	();
       RC	af_dir_enter_Certificate	(Certificate *cert, CertificateType type);
       SET_OF_Certificate	*af_dir_retrieve_Certificate	(DName *dname, CertificateType type);
       RC	af_dir_delete_Certificate	(OctetString *serial, DName *issuer, CertificateType type);
       RC	af_dir_enter_CRL	(CRL *crl);
       CRL	*af_dir_retrieve_CRL	(DName *dname);
       RC	af_dir_enter_CertificatePair	(CertificatePair *cpair);
       SET_OF_CertificatePair	*af_dir_retrieve_CertificatePair	(DName *dname);
       RC	af_dir_delete_CertificatePair	(CertificatePair *cpair);
       RC	af_dir_enter_OCList	(OCList *ocl_attr);
       OCList	*af_dir_retrieve_OCList	(DName *dname);
       RC	af_dir_delete_Certificate_from_targetObject	(DName *target, DName *issuer, CertificateType type);
       SET_OF_Certificate	*af_afdb_retrieve_Certificate	(DName *dname, KeyType ktype);
       RC	af_afdb_enter_Certificate	(Certificate *cert, KeyType ktype);
       RC	af_afdb_delete_Certificate	(OctetString *serial, DName *issuer, KeyType ktype);
       RC	af_afdb_enter_CRL	(CRL *crl);
       CRL	*af_afdb_retrieve_CRL	(DName *dname);
       RC	af_afdb_enter_OCList	();
       OCList	*af_afdb_retrieve_OCList	();
       RC	af_afdb_enter_CertificatePair	(CertificatePair *cpair);
       SET_OF_CertificatePair	*af_afdb_retrieve_CertificatePair	(DName *dname);
       RC	af_afdb_delete_CertificatePair	(CertificatePair *cpair);

/***************************************************************
 *
 * Headers from file af/af_dirop.c
 *
 ***************************************************************/ 
       CRL	*af_create_CRL	(UTCTime *lastUpdate, UTCTime *nextUpdate);
       CRLEntry	*af_create_CRLEntry	(OctetString *serial);
       RC	af_search_CRLEntry	(CRL *crl, CRLEntry *crlentry);
       OCList	*af_create_OCList	(KeyInfo *new_pubkey);
       RC	af_dir_update_OCList	(OCList *first_line);
/***************************************************************
 *
 * Headers from file af/af_ed.c
 *
 ***************************************************************/ 
       OctetString	*e_DName	(DName *namestruct);
       DName	*d_DName	(OctetString *asn1_string);
       OctetString	*e_Attribute	(Attr *attr);
       OctetString	*e_AttributeType	(AttrType *attrtype);
       OctetString	*e_AttributeValueAssertion	(AttrValueAssertion *ava);
       OctetString	*e_Certificates	(Certificates *certificates);
       Certificates	*d_Certificates	(OctetString *asn1_string);
       OctetString	*e_Certificate	(Certificate *certificate);
       Certificate	*d_Certificate	(OctetString *asn1_string);
       OctetString	*e_CertificateSet	(SET_OF_Certificate *certset);
       SET_OF_Certificate	*d_CertificateSet	(OctetString *asn1_string);
       OctetString	*e_CertificatePairSet	(SET_OF_CertificatePair *cpairset);
       SET_OF_CertificatePair	*d_CertificatePairSet	(OctetString *asn1_string);
       OctetString	*e_ToBeSigned	(ToBeSigned *tobesigned);
#ifdef COSINE
       OctetString	*e_AuthorisationAttributes	(AuthorisationAttributes *authattrbts);
       AuthorisationAttributes	*d_AuthorisationAttributes	(OctetString *asn1_string);
#endif
       OctetString	*e_FCPath	(FCPath *fcpath);
       FCPath	*d_FCPath	(OctetString *asn1_string);
       OctetString	*e_PKRoot	(PKRoot *pkroot);
       PKRoot	*d_PKRoot	(OctetString *asn1_string);
       OctetString	*e_PKList	(PKList *pklist);
       PKList	*d_PKList	(OctetString *asn1_string);
       OctetString	*e_CRL	(CRL *crl);
       CRL	*d_CRL	(OctetString *asn1_string);
       OctetString	*e_CRLEntry	(CRLEntry *crlentry);
       CRLEntry	*d_CRLEntry	(OctetString *asn1_string);
       OctetString	*e_CRLEntrySequence	(SEQUENCE_OF_CRLEntry *seq);
       SEQUENCE_OF_CRLEntry	*d_CRLEntrySequence	(OctetString *asn1_string);
       OctetString	*e_CrlSet	(CrlSet *crlset);
       CrlSet	*d_CrlSet	(OctetString *asn1_string);
       OctetString	*e_CRLTBS	(CRLTBS *tbs);
       PE	certificate_enc	(Certificate *parm);
       Certificate	*certificate_dec	(PE pe);
       OctetString	*e_CRLWithCertificates	(CRLWithCertificates *arg);
       CRLWithCertificates	*d_CRLWithCertificates	(OctetString *asn1_string);
       OctetString	*e_AliasList	(AliasList *alist);
       AliasList	*d_AliasList	(OctetString *asn1_string);
       OctetString	*e_SerialNumber	(SerialNumber *serial);
       SerialNumber	*d_SerialNumber	(OctetString *asn1_string);
       AttrValues       *dec_SetOfPE    (PE pe);
       RDName           *dec_SetOfAva   (PE pe);

/***************************************************************
 *
 * Headers from file af/af_pse.c
 *
 ***************************************************************/ 
       PSESel	*af_pse_open	(ObjId *af_object, Boolean create);
       PSESel	*af_pse_create	(ObjId *af_object);
       RC	af_pse_close	(ObjId *af_object);
       RC	af_pse_reset	(char *objname);
       RC	af_pse_rename	(char *oldname, char *newname);
       void	*af_pse_get	(char *objname, ObjId *objtype);
       RC	af_pse_update	(char *objname, void *opaque, ObjId *objtype);
       OctetString *af_pse_encode	(char *objname, void *opaque, ObjId *objtype);
       void 	*af_pse_decode	(char *objname, OctetString *ostr, ObjId *objtype);
       void 	*af_pse_cpy_opaque	(char *objname, void *opaque, ObjId *opaque_type);
       RC	af_pse_free	(void *opaque, ObjId *opaque_type);
       RC	af_pse_print	(char *objname);
       RC	af_pse_fprint	(FILE *ff, char *objname);
       char	*af_pse_sprint	(char *string, char *objname);
       SerialNumber * af_pse_get_SerialNumber();
       DName	*af_pse_get_Name	();
       AliasList	*af_pse_get_AliasList	();
       char	*af_pse_get_QuipuPWD	();
       Certificate	*af_pse_get_Certificate	(KeyType type, DName *issuer, OctetString *serial);
       SET_OF_Certificate	*af_pse_get_CertificateSet	(KeyType type);
       SET_OF_CertificatePair	*af_pse_get_CertificatePairSet	();
       FCPath	*af_pse_get_FCPath	(DName *name);
       Certificates	*af_pse_get_Certificates	(KeyType type, DName *name);
       PKRoot	*af_pse_get_PKRoot	();
       PKList	*af_pse_get_PKList	(KeyType type);
       PKList	*af_pse_get_PCAList	();
       CrlSet	*af_pse_get_CrlSet	();
       AlgId	*af_pse_get_DHparam	();
       RC	af_pse_update_Certificate	(KeyType type, Certificate *cert, Boolean hierarchy);
       RC	af_pse_update_FCPath	(FCPath *fcpath);
       RC	af_pse_update_CertificatePairSet	(SET_OF_CertificatePair *cpairset);
       RC	af_pse_update_Name	(DName *dname);
       RC	af_pse_update_AliasList	(AliasList *alist);
       RC	af_pse_update_QuipuPWD	(char *pwd);
       RC	af_pse_update_SerialNumber	(SerialNumber *serial);
       RC	af_pse_update_PKRoot	(PKRoot *pkroot);
       RC	af_pse_update_PKList	(KeyType type, PKList *list);
       RC	af_pse_update_PCAList	(PKList *list);
       RC	af_pse_update_CrlSet	(CrlSet *crlset);
       RC	af_pse_update_DHparam	(AlgId *dhparam);
       RC	af_pse_add_PK	        (KeyType type, ToBeSigned *tbs);
       RC	af_pse_add_PCA	        (ToBeSigned *tbs);
       RC	af_pse_delete_PK	(KeyType type, DName *name, DName *issuer, OctetString *serial);
       RC	af_pse_delete_PCA	(DName *name);
       RC	af_pse_add_CertificatePairSet	(SET_OF_CertificatePair *cpairset);
       RC	af_pse_add_CRL	(Crl *crlpse);
       RC	af_pse_delete_CRL	(DName *issuer);
       RC	af_pse_exchange_PK	(KeyType type, ToBeSigned *tbs);
       DName	*af_pse_get_owner	(KeyType type, KeyInfo *pk);
       KeyInfo	*af_pse_get_PK	(KeyType type, DName *subject, DName *issuer, OctetString *serial);
       ToBeSigned	*af_pse_get_TBS	(KeyType type, DName *subject, DName *issuer, OctetString *serial);
       RC	af_gen_key	(Key *key, KeyType ktype, Boolean replace);
       AlgId	*af_pse_get_signAI	();
       char	*getobjectpin	(char *objectname);
       RC	setobjectpin	(char *objectname, char *newpin);
       RC	af_check_if_onekeypaironly	(Boolean *onekeypaironly);
       OctetString	*af_pse_incr_serial	();
/***************************************************************
 *
 * Headers from file af/af_sntx.c
 *
 ***************************************************************/ 
       PE	AlgId_enc	(AlgId *parm);
       AlgId	*AlgId_dec	(PE pe);
       int	revoke_syntax	();
       PE	oclist_enc	(OCList *parm);
       OCList	*oclist_dec	(PE pe);
       OCList	*str2ocl	(char *str);
       int	printocl	(PS ps, OCList *parm, int format);
       int	aux_oclist_cmp	(OCList *a, OCList *b);
       int	oclist_syntax	();
       PE	crlentry_enc	(CRLEntry *parm);
       CRLEntry	*crlentry_dec	(PE pe);
       int	print_crlentry	(PS ps, CRLEntry *parm, int format);
       int	aux_crlentry_cmp	(CRLEntry *a, CRLEntry *b);
       PE	crlentryseq_enc	(SEQUENCE_OF_CRLEntry *parm);
       SEQUENCE_OF_CRLEntry	*crlentryseq_dec	(PE pe);
       PE	crl_enc	(CRL *parm);
       CRL	*crl_dec	(PE pe);
       PE	crltbs_enc	(CRLTBS *parm);
       CRLTBS	*crltbs_dec	(PE pe);
       CRL	*str2crl	(char *str);
       int	printcrl	(PS ps, CRL *parm, int format);
       int	aux_crl_cmp	(CRL *a, CRL *b);
       int	crl_syntax	();
/***************************************************************
 *
 * Headers from file af/af_util.c
 *
 ***************************************************************/ 
       RC	af_get_EncryptedKey	(EncryptedKey *encrypted_key, Key *plain_key, Key *encryption_key, DName *name, AlgId *encryption_key_algid);
       RC	af_put_EncryptedKey	(EncryptedKey *encrypted_key, Key *plain_key, AlgId *decryption_key_algid);
       Certificate	*af_create_Certificate	(KeyInfo *keyinfo, AlgId *sig_alg, char *obj_name, DName *subject, OctetString *serial);
       ObjId	*af_get_objoid	(char *objname);
       Certificate	*af_search_Certificate	(KeyType type, DName *dname);
       Certificate	*af_PKRoot2Protocert	(PKRoot *pkroot);
       RC	af_OctetString2SignedFile	(char *file, OctetString *ostr, Boolean withcerts, AlgId *signai);
       OctetString	*af_SignedFile2OctetString	(char *file);
/***************************************************************
 *
 * Headers from file af/cadb_ed.c
 *
 ***************************************************************/ 
       OctetString	*e_SET_OF_IssuedCertificate	(SET_OF_IssuedCertificate *isscertset);
       SET_OF_IssuedCertificate	*d_SET_OF_IssuedCertificate	(OctetString *asn1_string);

/***************************************************************
 *
 * Headers from file af/strong_int.c
 *
 ***************************************************************/ 
       struct SecurityServices	*use_serv_secude	();
       struct signature	*secudesigned	(caddr_t arg, int type, AlgId *algorithm);
       int	secudeverify	();
       struct certificate_list	*secude_mkpath	();
       struct encrypted	*secudeencrypted	();
       int	secudedecrypted	();
       struct Nonce	*secudemknonce	();
       int	secudecknonce	(struct Nonce *nonce);
       int	strong_int_dummy	();




#else



/***************************************************************
 *
 * Headers from file aux/aux_fprint.c
 *
 ***************************************************************/ 
       RC aux_fprint_GRAPHICString();
       RC aux_fprint_AliasList();
       RC aux_fprint_DNameOrAlias();
       RC aux_fprint_NameOrAlias();
       RC aux_fprint_FCPath();
       RC aux_fprint_Certificates();
       RC aux_fprint_RootInfo();
       RC aux_fprint_PKRoot();
       RC aux_fprint_PKList();
       RC aux_fprint_Certificate();
       RC aux_fprint_ToBeSigned();
#ifdef COSINE
       RC aux_fprint_AuthorisationAttributes();
#endif
       RC aux_fprint_CertificateSet();
       RC aux_fprint_CertificatePairSet();
       RC aux_fprint_Crl();
       RC aux_fprint_CrlSet();
       RC aux_fprint_IssuedCertificate();
       RC aux_fprint_SET_OF_IssuedCertificate();
       RC aux_fprint_SET_OF_Name();
       RC aux_fprint_CRLWithCertificates();
       RC aux_fprint_SET_OF_CRLWithCertificates();
       RC aux_fprint_VerificationResult();
       RC aux_fprint_TrustPath();
       RC aux_fprint_CRLEntry();
       RC aux_fprint_CRL();
       RC aux_fprint_CRLTBS();
       RC aux_fprint_OCList();
       RC aux_fprint_Validity();
       RC aux_fprint_validity_timeframe();

       char *aux_sprint_GRAPHICString();
       char *aux_sprint_AliasList();
       char *aux_sprint_DNameOrAlias();
       char *aux_sprint_NameOrAlias();
       char *aux_sprint_FCPath();
       char *aux_sprint_Certificates();
       char *aux_sprint_RootInfo();
       char *aux_sprint_PKRoot();
       char *aux_sprint_PKList();
       char *aux_sprint_Certificate();
       char *aux_sprint_ToBeSigned();
#ifdef COSINE
       char *aux_sprint_AuthorisationAttributes();
#endif
       char *aux_sprint_CertificateSet();
       char *aux_sprint_CertificatePairSet();
       char *aux_sprint_Crl();
       char *aux_sprint_CrlSet();
       char *aux_sprint_IssuedCertificate();
       char *aux_sprint_SET_OF_IssuedCertificate();
       char *aux_sprint_SET_OF_Name();
       char *aux_sprint_CRLWithCertificates();
       char *aux_sprint_SET_OF_CRLWithCertificates();
       char *aux_sprint_VerificationResult();
       char *aux_sprint_TrustPath();
       char *aux_sprint_CRLEntry();
       char *aux_sprint_CRL();
       char *aux_sprint_CRLTBS();
       char *aux_sprint_OCList();
       char *aux_sprint_Validity();
       char *aux_sprint_validity_timeframe();

       RC aux_print_GRAPHICString();
       RC aux_print_AliasList();
       RC aux_print_DNameOrAlias();
       RC aux_print_NameOrAlias();
       RC aux_print_FCPath();
       RC aux_print_Certificates();
       RC aux_print_RootInfo();
       RC aux_print_PKRoot();
       RC aux_print_PKList();
       RC aux_print_Certificate();
       RC aux_print_ToBeSigned();
#ifdef COSINE
       RC aux_print_AuthorisationAttributes();
#endif
       RC aux_print_CertificateSet();
       RC aux_print_CertificatePairSet();
       RC aux_print_Crl();
       RC aux_print_CrlSet();
       RC aux_print_IssuedCertificate();
       RC aux_print_SET_OF_IssuedCertificate();
       RC aux_print_SET_OF_Name();
       RC aux_print_CRLWithCertificates();
       RC aux_print_SET_OF_CRLWithCertificates();
       RC aux_print_VerificationResult();
       RC aux_print_TrustPath();
       RC aux_print_CRLEntry();
       RC aux_print_CRL();
       RC aux_print_CRLTBS();
       RC aux_print_OCList();
       RC aux_print_Validity();
       RC aux_print_validity_timeframe();
/***************************************************************
 *
 * Headers from file aux/aux_free.c
 *
 ***************************************************************/ 
       void	RDName_comp_free	();
       void	aux_free2_RDName	();
       void	aux_free_RDName	();
       void	DName_comp_free	();
       void	aux_free2_DName	();
       void	aux_free_DName	();
       void	aux_free2_ToBeSigned	();
       void	aux_free_ToBeSigned	();
       void	aux_free2_Certificate	();
       void	aux_free_Certificate	();
       void	aux_free2_Key	();
       void	aux_free_Key	();
       void	aux_free_CertificateSet	();
       void	aux_free2_CertificatePair	();
       void	aux_free_CertificatePair	();
       void	aux_free_CertificatePairSet	();
       void	aux_free_CrossCertificates	();
       void	aux_free2_CrossCertificates	();
       void	aux_free_FCPath	();
       void	aux_free_Certificates	();
       void	aux_free_CRLWithCertificates	();
       void	aux_free_SET_OF_CRLWithCertificates	();
       void	aux_free_CertificationPath	();
       void	aux_free_CertificatePairs	();
       void	aux_free_SEQUENCE_OF_CertificatePair	();
       void	aux_free_PKRoot	();
       void	aux_free_RootInfo	();
       void	aux_free2_RootInfo	();
       void	aux_free_PKList	();
       void	aux_free_error	();
       void	aux_free2_Crl	();
       void	aux_free_Crl	();
       void	aux_free_CrlSet	();
       void	aux_free2_CRLEntry	();
       void	aux_free_CRLEntry	();
       void	aux_free_SEQUENCE_OF_CRLEntry	();
       void	aux_free2_CRLTBS	();
       void	aux_free_CRLTBS	();
       void	aux_free2_CRL	();
       void	aux_free_CRL	();
       void	aux_free_OCList	();
       void	aux_free_IssuedCertificate	();
       void	aux_free2_IssuedCertificate	();
       void	aux_free_SET_OF_IssuedCertificate	();
       void	aux_free_VerificationStep	();
       void	aux_free_VerificationResult	();
       void	aux_free2_VerificationResult	();
       void	aux_free_Validity	();
       void	aux_free2_Validity	();
       void	aux_free_AliasList	();

/***************************************************************
 *
 * Headers from file aux/aux_util.c
 *
 ***************************************************************/ 
       int	aux_create_AFPSESel	();
       int	aux_set_pse	();
       int	RDName_comp_cmp	();
       int	aux_cmp_RDName	();
       int	aux_cmp_DName	();
       RC	aux_checkPemDNameSubordination	();
       Name	*aux_PemDNameSubordination	();
       Name	*aux_PCA	();
       RDName	*aux_cpy_RDName_comp	();
       RDName	*aux_cpy_RDName	();
       DName	*aux_cpy_DName_comp	();
       DName	*aux_cpy_DName	();
       int	aux_cpy2_DName_comp	();
       int	aux_cpy2_DName	();
       Certificate	*aux_cpy_Certificate	();
       int	aux_cpy2_Certificate	();
       PKList	*aux_cpy_PKList	();
       Certificates	*aux_cpy_Certificates	();
       CertificatePair	*aux_cpy_CertificatePair	();
       SET_OF_Certificate	*aux_cpy_SET_OF_Certificate	();
       SET_OF_Name	*aux_cpy_SET_OF_Name	();
       SET_OF_CertificatePair	*aux_cpy_SET_OF_CertificatePair	();
       FCPath	*aux_cpy_FCPath	();
       ToBeSigned	*aux_cpy_ToBeSigned	();
       Key	*aux_cpy_Key	();
       PKRoot	*aux_cpy_PKRoot	();
       IssuedCertificate	*aux_cpy_IssuedCertificate	();
       SET_OF_IssuedCertificate	*aux_cpy_SET_OF_IssuedCertificate	();
       Crl	*aux_cpy_Crl	();
       CrlSet	*aux_cpy_CrlSet	();
       struct Serial	*aux_cpy_Serial	();
       AliasList * aux_cpy_AliasList ();
       Aliases * aux_cpy_Aliases ();
       Certificates	*aux_create_Certificates	();
       PKRoot	*aux_create_PKRoot	();
       FCPath	*aux_create_FCPath      ();
       Certificate	*aux_create_Certificate	();
       int	aux_cmp_Certificate	();
       int	aux_cmp_CertificatePair	();
       CRLEntry	*aux_cpy_CRLEntry	();
       SEQUENCE_OF_CRLEntry	*aux_cpy_SEQUENCE_OF_CRLEntry	();
       CRL	*aux_cpy_CRL	();
       CRLTBS	*aux_cpy_CRLTBS	();
       OCList	*aux_cpy_OCList	();
       Validity	*aux_cpy_Validity ();
       Name * aux_determine_Signer ();
       Key *PSEobj();
/***************************************************************
 *
 * Headers from file aux/aux_alias.c
 *
 ***************************************************************/ 
       AliasFile	aux_alias	();
       DName	*aux_alias2DName	();
       Name	*aux_alias2Name		();
       char	*aux_DName2alias	();
       char	*aux_DName2NameOrAlias	();
       char	*aux_Name2NameOrAlias	();
       char	*aux_Name2alias		();
       char	*aux_DName2aliasf	();
       char	*aux_Name2aliasf	();
       RC	aux_add_alias_name	();
       RC	aux_add_alias		();
       RC	aux_delete_alias	();
       RC	aux_fprint_alias2dname	();
       RC	aux_fprint_alias2dnames	();
       RC	aux_fprint_dname2alias	();
       Name	*aux_search_AliasList	();
       Boolean	aux_alias_chkfile	();
       Name	*aux_next_AliasList	();
       Name	*aux_alias_nxtname	();
       char	*aux_alias_getall	();
       Boolean	aux_get_AliasList	();
       RC	aux_put_AliasList	();
       Boolean	aux_check_AliasList	();
       AliasList	*aux_select_AliasList	();
       Boolean	aux_alias_writeprotection	();
/***************************************************************
 *
 * Headers from file aux/aux_dname.c
 *
 ***************************************************************/ 
       void	aux_append_RDName	();
       RDName	*str2RDName	();
       DName	*aux_Name2DName	();
       void	aux_append_DName	();
       DName	*DName_comp_new	();
       char	*aux_DName2Name	();
       char	*aux_DName2Attr	();
       DName	*aux_ORName2DName	();
       char	*aux_DName2CAPITALName	();
       char	*aux_DName2CAPITALString	();
       char     *aux_DName2afdbname     ();


/***************************************************************
 *
 * Headers from file af/af_cadb.c
 *
 ***************************************************************/ 
       int	af_cadb_add_user	();
       SET_OF_IssuedCertificate	*af_cadb_get_user	();
       int	af_cadb_add_Certificate	();
       Certificate	*af_cadb_get_Certificate	();
       SET_OF_Name	*af_cadb_list_user	();
       char	*logtime	();
       int	af_cadb_add_CRLWithCertificates	();
       SET_OF_CRLWithCertificates	*af_cadb_list_CRLWithCertificates	();
       CRLWithCertificates	*af_cadb_get_CRLWithCertificates	();
/***************************************************************
 *
 * Headers from file af/af_crypt.c
 *
 ***************************************************************/ 
       RC	af_encrypt	();
       RC	af_decrypt	();
       RC	af_sign	();
       RC	af_verify	();
       RC	af_verify_Certificates	();
       RC	af_check_validity_of_Certificate	();
       RC	af_check_validity_of_PKRoot	();
       RC	af_check_validity_of_ToBeSigned	();
       FCPath	*reduce_FCPath_to_HierarchyPath	();
       Certificates	*transform_reducedFCPath_into_Certificates	();
       Crl	*CRL2Crl	();
/***************************************************************
 *
 * Headers from file af/af_dir.c
 *
 ***************************************************************/ 
       int	security_syntaxes	();
       RC	af_dir_enter_Certificate	();
       SET_OF_Certificate	*af_dir_retrieve_Certificate	();
       RC	af_dir_delete_Certificate	();
       RC	af_dir_enter_CRL	();
       CRL	*af_dir_retrieve_CRL	();
       RC	af_dir_enter_CertificatePair	();
       SET_OF_CertificatePair	*af_dir_retrieve_CertificatePair	();
       RC	af_dir_delete_CertificatePair	();
       RC	af_dir_enter_OCList	();
       OCList	*af_dir_retrieve_OCList	();
       RC	af_dir_delete_Certificate_from_targetObject	();
       SET_OF_Certificate	*af_afdb_retrieve_Certificate	();
       RC	af_afdb_enter_Certificate	();
       RC	af_afdb_delete_Certificate	();
       RC	af_afdb_enter_CRL	();
       CRL	*af_afdb_retrieve_CRL	();
       RC	af_afdb_enter_OCList	();
       OCList	*af_afdb_retrieve_OCList	();
       RC	af_afdb_enter_CertificatePair	();
       SET_OF_CertificatePair	*af_afdb_retrieve_CertificatePair	();
       RC	af_afdb_delete_CertificatePair	();

/***************************************************************
 *
 * Headers from file af/af_dirop.c
 *
 ***************************************************************/ 
       CRL	*af_create_CRL	();
       CRLEntry	*af_create_CRLEntry	();
       RC	af_search_CRLEntry	();
       OCList	*af_create_OCList	();
       RC	af_dir_update_OCList	();
/***************************************************************
 *
 * Headers from file af/af_ed.c
 *
 ***************************************************************/ 
       OctetString	*e_DName	();
       DName	*d_DName	();
       OctetString	*e_Attribute	();
       OctetString	*e_AttributeType	();
       OctetString	*e_AttributeValueAssertion	();
       OctetString	*e_Certificates	();
       Certificates	*d_Certificates	();
       OctetString	*e_Certificate	();
       Certificate	*d_Certificate	();
       OctetString	*e_CertificateSet	();
       SET_OF_Certificate	*d_CertificateSet	();
       OctetString	*e_CertificatePairSet	();
       SET_OF_CertificatePair	*d_CertificatePairSet	();
       OctetString	*e_ToBeSigned	();
#ifdef COSINE
       OctetString	*e_AuthorisationAttributes	();
       AuthorisationAttributes	*d_AuthorisationAttributes	();
#endif
       OctetString	*e_FCPath	();
       FCPath	*d_FCPath	();
       OctetString	*e_PKRoot	();
       PKRoot	*d_PKRoot	();
       OctetString	*e_PKList	();
       PKList	*d_PKList	();
       OctetString	*e_Crl	();
       OctetString	*e_CRL	();
       CRL	*d_CRL	();
       OctetString	*e_CRLEntry	();
       CRLEntry	*d_CRLEntry	();
       OctetString	*e_CRLEntrySequence	();
       SEQUENCE_OF_CRLEntry	*d_CRLEntrySequence	();
       OctetString	*e_OCList	();
       OCList	*d_OCList	();
       OctetString	*e_CrlSet	();
       CrlSet	*d_CrlSet	();
       OctetString	*e_CrlTBS	();
       OctetString	*e_CRLTBS	();
       PE	         certificate_enc	();
       Certificate	*certificate_dec	();
       OctetString	*e_CRLWithCertificates	();
       CRLWithCertificates	*d_CRLWithCertificates	();
       OctetString	*e_AliasList	();
       AliasList	*d_AliasList	();
       OctetString	*e_SerialNumber	();
       SerialNumber	*d_SerialNumber	();
       AttrValues       *dec_SetOfPE    ();
       RDName           *dec_SetOfAva   ();
/***************************************************************
 *
 * Headers from file af/af_pse.c
 *
 ***************************************************************/ 
       PSESel	*af_pse_open	();
       PSESel	*af_pse_create	();
       RC	af_pse_close	();
       RC	af_pse_reset	();
       RC	af_pse_rename	();
       void	*af_pse_get	();
       RC	af_pse_update	();
       OctetString	*af_pse_encode	();
       void	*af_pse_decode	();
       void 	*af_pse_cpy_opaque	();
       RC	af_pse_free	();
       RC	af_pse_print	();
       RC	af_pse_fprint	();
       char	*af_pse_sprint	();
       SerialNumber * af_pse_get_SerialNumber();
       DName	*af_pse_get_Name	();
       AliasList	*af_pse_get_AliasList	();
       char	*af_pse_get_QuipuPWD	();
       Certificate	*af_pse_get_Certificate	();
       SET_OF_Certificate	*af_pse_get_CertificateSet	();
       SET_OF_CertificatePair	*af_pse_get_CertificatePairSet	();
       FCPath	*af_pse_get_FCPath	();
       Certificates	*af_pse_get_Certificates	();
       PKRoot	*af_pse_get_PKRoot	();
       PKList	*af_pse_get_PKList	();
       PKList	*af_pse_get_PCAList	();
       CrlSet	*af_pse_get_CrlSet	();
       AlgId	*af_pse_get_DHparam	();
       RC	af_pse_update_Certificate	();
       RC	af_pse_update_FCPath	();
       RC	af_pse_update_CertificatePairSet	();
       RC	af_pse_update_Name	();
       RC	af_pse_update_AliasList	();
       RC	af_pse_update_QuipuPWD	();
       RC	af_pse_update_SerialNumber	();
       RC	af_pse_update_PKRoot	();
       RC	af_pse_update_PKList	();
       RC	af_pse_update_CrlSet	();
       RC	af_pse_update_DHparam	();
       RC	af_pse_add_PK	();
       RC	af_pse_delete_PK	();
       RC	af_pse_add_CertificatePairSet	();
       RC	af_pse_add_CRL	();
       RC	af_pse_delete_CRL	();
       RC	af_pse_exchange_PK	();
       DName	*af_pse_get_owner	();
       KeyInfo	*af_pse_get_PK	();
       ToBeSigned	*af_pse_get_TBS	();
       RC	af_gen_key	();
       AlgId	*af_pse_get_signAI	();
       char	*getobjectpin	();
       RC	setobjectpin	();
       RC	af_check_if_onekeypaironly	();
       OctetString	*af_pse_incr_serial	();
/***************************************************************
 *
 * Headers from file af/af_sntx.c
 *
 ***************************************************************/ 
       PE	AlgId_enc	();
       AlgId	*AlgId_dec	();
       int	revoke_syntax	();
       PE	oclist_enc	();
       OCList	*oclist_dec	();
       OCList	*str2ocl	();
       int	printocl	();
       int	aux_oclist_cmp	();
       int	oclist_syntax	();
       PE	crlentry_enc	();
       CRLEntry	*crlentry_dec	();
       int	print_crlentry	();
       int	aux_crlentry_cmp	();
       PE	crlentryseq_enc	();
       SEQUENCE_OF_CRLEntry	*crlentryseq_dec	();
       PE	crl_enc	();
       CRL	*crl_dec	();
       PE	crltbs_enc	();
       CRLTBS	*crltbs_dec	();
       CRL	*str2crl	();
       int	printcrl	();
       int	aux_crl_cmp	();
       int	crl_syntax	();
/***************************************************************
 *
 * Headers from file af/af_util.c
 *
 ***************************************************************/ 
       RC	af_get_EncryptedKey	();
       RC	af_put_EncryptedKey	();
       Certificate	*af_create_Certificate	();
       ObjId	*af_get_objoid	();
       Certificate	*af_search_Certificate	();
       Certificate	*af_PKRoot2Protocert	();
       RC	af_OctetString2SignedFile	();
       OctetString	*af_SignedFile2OctetString	();
/***************************************************************
 *
 * Headers from file af/cadb_ed.c
 *
 ***************************************************************/ 
       OctetString	*e_SET_OF_IssuedCertificate	();
       SET_OF_IssuedCertificate	*d_SET_OF_IssuedCertificate	();

/***************************************************************
 *
 * Headers from file af/strong_int.c
 *
 ***************************************************************/ 
       struct SecurityServices	*use_serv_secude	();
       struct signature	*secudesigned	();
       int	secudeverify	();
       struct certificate_list	*secude_mkpath	();
       struct encrypted	*secudeencrypted	();
       int	secudedecrypted	();
       struct Nonce	*secudemknonce	();
       int	secudecknonce	();
       int	strong_int_dummy	();



#endif
