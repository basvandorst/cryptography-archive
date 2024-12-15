#ifdef __STDC__


/***************************************************************
 *
 * Headers from file aux/aux_fprint.c
 *
 ***************************************************************/ 
       RC	aux_fprint_PemInfo	(FILE *ff, PemInfo *peminfo);
       RC 	aux_print_PemHeaderLocal(PemHeaderLocal	 *hd);
       RC 	aux_print_PemMicLocal(PemMicLocal	 *mic);
       RC 	aux_print_PemDekLocal(PemDekLocal	 *dek);
       RC 	aux_print_PemRecLocal(PemRecLocal	 *rec);
       RC 	aux_print_PemMessageLocal(PemMessageLocal	 *local);
       RC 	aux_fprint_PemHeaderLocal(FILE *ff, PemHeaderLocal	 *hd);
       RC 	aux_fprint_PemMicLocal(FILE *ff, PemMicLocal	 *mic);
       RC 	aux_fprint_PemDekLocal(FILE *ff, PemDekLocal	 *dek);
       RC 	aux_fprint_PemRecLocal(FILE *ff, PemRecLocal	 *rec);
       RC 	aux_fprint_PemMessageLocal(FILE *ff, PemMessageLocal	 *local);
       char 	*aux_sprint_PemHeaderLocal(char *string, PemHeaderLocal	 *hd);
       char 	*aux_sprint_PemMicLocal(char *string, PemMicLocal	 *mic);
       char 	*aux_sprint_PemDekLocal(char *string, PemDekLocal	 *dek);
       char 	*aux_sprint_PemRecLocal(char *string, PemRecLocal	 *rec);
       char 	*aux_sprint_PemMessageLocal(char *string, PemMessageLocal	 *local);

/***************************************************************
 *
 * Headers from file aux/aux_util.c
 *
 ***************************************************************/ 
       RecpList	*aux_cpy_RecpList	(RecpList *list);
       PemInfo	*aux_cpy_PemInfo	(PemInfo *peminfo);
       char     *aux_get_abs_path       (char *path, char *file);
       SET_OF_PemMessageLocal   *aux_cpy_SET_OF_PemMessageLocal (SET_OF_PemMessageLocal *old);
       SET_OF_DName     *aux_cpy_SET_OF_DName   (SET_OF_DName *old);
       PemMessageLocal  *aux_cpy_PemMessageLocal        (PemMessageLocal *old);
       PemHeaderLocal   *aux_cpy_PemHeaderLocal (PemHeaderLocal *old);
       PemDekLocal      *aux_cpy_PemDekLocal    (PemDekLocal *old);
       SET_OF_PemRecLocal       *aux_cpy_SET_OF_PemRecLocal     (SET_OF_PemRecLocal *old);
       SET_OF_CRLWithCertificates       *aux_cpy_SET_OF_CRLWithCertificates     (SET_OF_CRLWithCertificates *old);
       CRLWithCertificates      *aux_cpy_CRLWithCertificates    (CRLWithCertificates *old);
       PemRecLocal      *aux_cpy_PemRecLocal    (PemRecLocal *old);
       PemMicLocal      *aux_cpy_PemMicLocal    (PemMicLocal *old);

/***************************************************************
 *
 * Headers from file aux/aux_free.c
 *
 ***************************************************************/ 
       void	aux_free_RecpList	(RecpList **recp);
       void	aux_free2_PemInfo	(register PemInfo *info);
       void	aux_free_PemInfo	(PemInfo **info);
       void     aux_free_SET_OF_PemMessageCanon (SET_OF_PemMessageCanon **obj);
       void     aux_free_PemMessageCanon        (PemMessageCanon **obj);
       void     aux_free_PemHeaderCanon (PemHeaderCanon **obj);
       void     aux_free_PemDekCanon    (PemDekCanon **obj);
       void     aux_free_SET_OF_PemRecCanon     (SET_OF_PemRecCanon **obj);
       void     aux_free_SET_OF_PemCrlCanon     (SET_OF_PemCrlCanon **obj);
       void     aux_free_SET_OF_Name    (SET_OF_Name **obj);
       void     aux_free_PemRecCanon    (PemRecCanon **obj);
       void     aux_free_PemOriginatorCanon     (PemOriginatorCanon **obj);
       void     aux_free_PemMicCanon    (PemMicCanon **obj);
       void     aux_free_PemCrlCanon    (PemCrlCanon **obj);
       void     aux_free_PemHeaderLocal (PemHeaderLocal **obj);
       void     aux_free_PemDekLocal    (PemDekLocal **obj);
       void     aux_free_PemMicLocal    (PemMicLocal **obj);
       void     aux_free_SET_OF_PemRecLocal     (SET_OF_PemRecLocal **obj);
       void     aux_free_SET_OF_DName   (SET_OF_DName **obj);
       void     aux_free_NAME   (NAME **obj);
       void     aux_free_SET_OF_NAME    (SET_OF_NAME **obj);
       void     aux_free_SET_OF_PemMessageLocal (SET_OF_PemMessageLocal **obj);
       void     aux_free_PemMessageLocal        (PemMessageLocal **obj);
       void     aux_free_PemRecLocal    (PemRecLocal **obj);
#else


/***************************************************************
 *
 * Headers from file aux/aux_fprint.c
 *
 ***************************************************************/ 
       RC	aux_fprint_PemInfo	();
       RC 	aux_print_PemHeaderLocal();
       RC 	aux_print_PemMicLocal();
       RC 	aux_print_PemDekLocal();
       RC 	aux_print_PemRecLocal();
       RC 	aux_print_PemMessageLocal();
       RC 	aux_fprint_PemHeaderLocal();
       RC 	aux_fprint_PemMicLocal();
       RC 	aux_fprint_PemDekLocal();
       RC 	aux_fprint_PemRecLocal();
       RC 	aux_fprint_PemMessageLocal();
       char 	*aux_sprint_PemHeaderLocal();
       char 	*aux_sprint_PemMicLocal();
       char 	*aux_sprint_PemDekLocal();
       char 	*aux_sprint_PemRecLocal();
       char 	*aux_sprint_PemMessageLocal();

/***************************************************************
 *
 * Headers from file aux/aux_util.c
 *
 ***************************************************************/ 
       RecpList	*aux_cpy_RecpList	();
       PemInfo	*aux_cpy_PemInfo	();
       char     *aux_get_abs_path       ();
       SET_OF_PemMessageLocal   *aux_cpy_SET_OF_PemMessageLocal ();
       SET_OF_DName     *aux_cpy_SET_OF_DName   ();
       PemMessageLocal  *aux_cpy_PemMessageLocal        ();
       PemHeaderLocal   *aux_cpy_PemHeaderLocal ();
       PemDekLocal      *aux_cpy_PemDekLocal    ();
       SET_OF_PemRecLocal       *aux_cpy_SET_OF_PemRecLocal     ();
       SET_OF_CRLWithCertificates       *aux_cpy_SET_OF_CRLWithCertificates     ();
       CRLWithCertificates      *aux_cpy_CRLWithCertificates    ();
       PemRecLocal      *aux_cpy_PemRecLocal    ();
       PemMicLocal      *aux_cpy_PemMicLocal    ();

/***************************************************************
 *
 * Headers from file aux/aux_free.c
 *
 ***************************************************************/ 
       void	aux_free_RecpList	();
       void	aux_free2_PemInfo	();
       void	aux_free_PemInfo	();
       void     aux_free_SET_OF_PemMessageCanon ();
       void     aux_free_PemMessageCanon        ();
       void     aux_free_PemHeaderCanon ();
       void     aux_free_PemDekCanon    ();
       void     aux_free_SET_OF_PemRecCanon     ();
       void     aux_free_SET_OF_PemCrlCanon     ();
       void     aux_free_SET_OF_Name    ();
       void     aux_free_PemRecCanon    ();
       void     aux_free_PemOriginatorCanon     ();
       void     aux_free_PemMicCanon    ();
       void     aux_free_PemCrlCanon    ();
       void     aux_free_PemHeaderLocal ();
       void     aux_free_PemDekLocal    ();
       void     aux_free_PemMicLocal    ();
       void     aux_free_SET_OF_PemRecLocal     ();
       void     aux_free_SET_OF_DName   ();
       void     aux_free_NAME   ();
       void     aux_free_SET_OF_NAME    ();
       void     aux_free_SET_OF_PemMessageLocal ();
       void     aux_free_PemMessageLocal        ();
       void     aux_free_PemRecLocal    ();
#endif

#ifdef __STDC__
/***************************************************************
 *
 * Headers from file pem_build.c
 *
 ***************************************************************/ 
       OctetString      *pem_Local2Clear        (PemMessageLocal *local, Boolean insert_boundaries);
       OctetString      *pem_LocalSet2Clear     (SET_OF_PemMessageLocal *local_mess, Boolean insert_boundaries);
       OctetString      *pem_mic_clear_bodys    (SET_OF_PemMessageLocal *local_mess);
       OctetString      *pem_build_one  (PemMessageCanon *message);
       OctetString      *pem_build      (SET_OF_PemMessageCanon *canon_mess);
/***************************************************************
 *
 * Headers from file pem_cnvt.c
 *
 ***************************************************************/ 
       int      *pem_proctypes  (SET_OF_PemMessageLocal *local);
       PemMessageCanon  *pem_Local2Canon        (PemMessageLocal *local);
       PemMessageLocal  *pem_Canon2Local        (PemMessageCanon *canon);
       SET_OF_PemMessageLocal   *pem_CanonSet2LocalSet  (SET_OF_PemMessageCanon *canon);
       SET_OF_PemMessageCanon   *pem_LocalSet2CanonSet  (SET_OF_PemMessageLocal *local);
/***************************************************************
 *
 * Headers from file pem_init.c
 *
 ***************************************************************/ 
/***************************************************************
 *
 * Headers from file pem_parse.c
 *
 ***************************************************************/ 
       SET_OF_PemMessageCanon   *pem_parse      (OctetString *text);
/***************************************************************
 *
 * Headers from file pem_pse.c
 *
 ***************************************************************/ 
       int      pem_pse_open    (char *ca_dir, char *psepath, char *pin);
       RC       pem_pse_close   (int pse);
       Certificate      *pem_get_Certificate    (int pse, KeyType type, Certificate *certificate);
       RC       pem_fill_Certificate    (int pse, KeyType type, Certificate *certificate);
       RC       pem_look_for_Certificate        (int pse, KeyType type, Certificate *certificate);
       RC       pem_accept_certification_reply  (int pse, PemMessageLocal *local);
       RC       pem_pse_store_crlset    (int pse, SET_OF_CRLWithCertificates *set_of_pemcrlwithcerts);
       RC       pem_cadb_store_crlset   (int pse, SET_OF_CRLWithCertificates *set_of_pemcrlwithcerts);
       OctetString      *pem_reply_crl_request  (int pse, PemMessageLocal *local_mess);
       PemMessageLocal  *pem_certify    (int pse, PemMessageLocal *local_mess);
       PemMessageLocal  *pem_validate   (int pse, PemMessageLocal *local_mess);
       SET_OF_PemMessageLocal   *pem_validateSet        (int pse, SET_OF_PemMessageLocal *message);
       OctetString      *pem_reply_crl_requestSet       (int pse, SET_OF_PemMessageLocal *local_message);
       PemMessageLocal  *pem_crl        (int pse, SET_OF_NAME *issuers);
       PemMessageLocal  *pem_crl_rr     (int pse, SET_OF_NAME *issuers);
       PemMessageLocal  *pem_enhance    (int pse, PEM_Proc_Types proc_type, OctetString *text, int no_of_certs, SET_OF_NAME *recipients);
       RC       pem_check_name  (int pse, NAME *name);
       PemMessageLocal *pem_forward(int pse, PemMessageLocal *message, PEM_Proc_Types proc_type, SET_OF_NAME *recipients);
       SET_OF_PemMessageLocal *pem_forwardSet(int pse,SET_OF_PemMessageLocal *message, PEM_Proc_Types proc_type, SET_OF_NAME *recipients);
/***************************************************************
 *
 * Headers from file pem_util.c
 *
 ***************************************************************/ 
       SET_OF_NAME      *pem_scan822hdr (char *buffer);
#else
/***************************************************************
 *
 * Headers from file pem_build.c
 *
 ***************************************************************/ 
       OctetString      *pem_Local2Clear        ();
       OctetString      *pem_LocalSet2Clear     ();
       OctetString      *pem_mic_clear_bodys    ();
       OctetString      *pem_build_one  ();
       OctetString      *pem_build      ();
/***************************************************************
 *
 * Headers from file pem_cnvt.c
 *
 ***************************************************************/ 
       int      *pem_proctypes  ();
       PemMessageCanon  *pem_Local2Canon        ();
       PemMessageLocal  *pem_Canon2Local        ();
       SET_OF_PemMessageLocal   *pem_CanonSet2LocalSet  ();
       SET_OF_PemMessageCanon   *pem_LocalSet2CanonSet  ();
/***************************************************************
 *
 * Headers from file pem_init.c
 *
 ***************************************************************/ 
/***************************************************************
 *
 * Headers from file pem_parse.c
 *
 ***************************************************************/ 
       SET_OF_PemMessageCanon   *pem_parse      ();
/***************************************************************
 *
 * Headers from file pem_pse.c
 *
 ***************************************************************/ 
       int      pem_pse_open    ();
       RC       pem_pse_close   ();
       Certificate      *pem_get_Certificate    ();
       RC       pem_fill_Certificate    ();
       RC       pem_look_for_Certificate        ();
       RC       pem_accept_certification_reply  ();
       RC       pem_pse_store_crlset    ();
       RC       pem_cadb_store_crlset   ();
       OctetString      *pem_reply_crl_request  ();
       PemMessageLocal  *pem_certify    ();
       PemMessageLocal  *pem_validate   ();
       SET_OF_PemMessageLocal   *pem_validateSet        ();
       OctetString      *pem_reply_crl_requestSet       ();
       PemMessageLocal  *pem_crl        ();
       PemMessageLocal  *pem_crl_rr     ();
       PemMessageLocal  *pem_enhance    ();
       RC       pem_check_name  ();
       PemMessageLocal *pem_forward();
       SET_OF_PemMessageLocal *pem_forwardSet();
/***************************************************************
 *
 * Headers from file pem_util.c
 *
 ***************************************************************/ 
       SET_OF_NAME      *pem_scan822hdr ();
#endif
