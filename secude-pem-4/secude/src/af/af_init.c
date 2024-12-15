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

/*-----------------------------------------------------------------------*/
/* FILE  af_init.c                                                       */
/* Initialization of global variables of AF-IF                           */
/*-----------------------------------------------------------------------*/

#include "af.h"

/*-----------------------------------------------------------------------*/
/*    Object Identifiers of PSE-Objects                                  */
/*-----------------------------------------------------------------------*/

/*
 *    ObjectIdentifier macro (parameter obj) builds 
 *               ObjId <obj>_oid
 *    from <obj>_OID 
 *
 */

#if defined(MAC) || defined(__STDC__)
#define ObjectIdentifier(obj)                                             \
        unsigned int obj##_oid_elements[] = obj##_oid_EL;                 \
        ObjId obj##_oid = {                                               \
        sizeof(obj##_oid_elements)/sizeof(int), obj##_oid_elements };     \
        ObjId *obj##_OID = &obj##_oid

#else
#define ObjectIdentifier(obj)                                             \
        unsigned int obj/**/_oid_elements[] = obj/**/_oid_EL;             \
        ObjId obj/**/_oid = {                                             \
        sizeof(obj/**/_oid_elements)/sizeof(int), obj/**/_oid_elements }; \
        ObjId *obj/**/_OID = &obj/**/_oid
#endif /* MAC */

#define SignCert_oid_EL       { 1, 3, 36, 2, 1, 1 }
#define EncCert_oid_EL        { 1, 3, 36, 2, 1, 2 }
#define Cert_oid_EL           { 1, 3, 36, 2, 1, 3 }
#define SignCSet_oid_EL       { 1, 3, 36, 2, 2, 1 }
#define EncCSet_oid_EL        { 1, 3, 36, 2, 2, 2 }
#define CSet_oid_EL           { 1, 3, 36, 2, 2, 3 }
#define SignSK_oid_EL         { 1, 3, 36, 2, 3, 1 }
#define DecSKnew_oid_EL       { 1, 3, 36, 2, 3, 2 }
#define DecSKold_oid_EL       { 1, 3, 36, 2, 3, 3 }
#define SKnew_oid_EL          { 1, 3, 36, 2, 3, 4 }
#define SKold_oid_EL          { 1, 3, 36, 2, 3, 5 }
#define FCPath_oid_EL         { 1, 3, 36, 2, 4, 1 }
#define PKRoot_oid_EL         { 1, 3, 36, 2, 5, 1 }
#define PKList_oid_EL         { 1, 3, 36, 2, 6, 1 }
#define EKList_oid_EL         { 1, 3, 36, 2, 6, 2 }
#define PCAList_oid_EL        { 1, 3, 36, 2, 6, 3 }
#define CrossCSet_oid_EL      { 1, 3, 36, 2, 8, 1 }
#define CrlSet_oid_EL         { 1, 3, 36, 2, 9, 1 }
#define Name_oid_EL           { 1, 3, 36, 2, 7, 1 }
#define SerialNumber_oid_EL   { 1, 3, 36, 2, 10, 1 }
#define EDBKey_oid_EL         { 1, 3, 36, 2, 11, 1 }
#define AliasList_oid_EL      { 1, 3, 36, 2, 12, 1 }
#define QuipuPWD_oid_EL       { 1, 3, 36, 2, 13, 1 }
#define Random_oid_EL         { 1, 3, 36, 2, 14, 1 }
#define DHparam_oid_EL        { 1, 3, 36, 2, 15, 1 }
#define DHprivate_oid_EL      { 1, 3, 36, 2, 15, 2 }
#define DHkey_oid_EL          { 1, 3, 36, 2, 15, 3 }
#define KeyInfo_oid_EL        { 1, 3, 36, 2, 16, 1 }
#define RSA_SK_oid_EL         { 1, 3, 36, 2, 99, 1 }
#define RSA_PK_oid_EL         { 1, 3, 36, 2, 99, 2 }
#define DSA_SK_oid_EL         { 1, 3, 36, 2, 99, 3 }
#define DSA_PK_oid_EL         { 1, 3, 36, 2, 99, 4 }
#define DES_oid_EL            { 1, 3, 36, 2, 99, 5 }
#define DES3_oid_EL           { 1, 3, 36, 2, 99, 6 }
#define IDEA_oid_EL           { 1, 3, 36, 2, 99, 7 }
#define Uid_oid_EL            { 1, 3, 36, 2, 0 }

ObjectIdentifier(SignCert);
ObjectIdentifier(EncCert);
ObjectIdentifier(Cert);
ObjectIdentifier(SignCSet);
ObjectIdentifier(EncCSet);
ObjectIdentifier(CSet);
ObjectIdentifier(SignSK);
ObjectIdentifier(DecSKnew);
ObjectIdentifier(DecSKold);
ObjectIdentifier(SKnew);
ObjectIdentifier(SKold);
ObjectIdentifier(FCPath);
ObjectIdentifier(PKRoot);
ObjectIdentifier(PKList);
ObjectIdentifier(EKList);
ObjectIdentifier(PCAList);
ObjectIdentifier(CrossCSet);
ObjectIdentifier(CrlSet);
ObjectIdentifier(Name);
ObjectIdentifier(SerialNumber);
ObjectIdentifier(EDBKey);
ObjectIdentifier(AliasList);
ObjectIdentifier(Random);
ObjectIdentifier(QuipuPWD);
ObjectIdentifier(DHparam);
ObjectIdentifier(DHprivate);
ObjectIdentifier(DHkey);
ObjectIdentifier(KeyInfo);
ObjectIdentifier(RSA_SK);
ObjectIdentifier(RSA_PK);
ObjectIdentifier(DSA_SK);
ObjectIdentifier(DSA_PK);
ObjectIdentifier(DES);
ObjectIdentifier(DES3);
ObjectIdentifier(IDEA);
ObjectIdentifier(Uid);

AFPSESel AF_pse = {
        DEF_PSE, CNULL, FALSE,
        {
        { SignSK_name, CNULL, &SignSK_oid, FALSE } ,
        { DecSKnew_name, CNULL, &DecSKnew_oid, FALSE } ,
        { DecSKold_name, CNULL, &DecSKold_oid, FALSE } ,
        { SKnew_name, CNULL, &SKnew_oid, FALSE } ,
        { SKold_name, CNULL, &SKold_oid, FALSE } ,
        { SignCert_name, CNULL, &SignCert_oid, FALSE } ,
        { EncCert_name, CNULL, &EncCert_oid, FALSE } ,
        { Cert_name, CNULL, &Cert_oid, FALSE } ,
        { FCPath_name, CNULL, &FCPath_oid, FALSE } ,
        { PKRoot_name, CNULL, &PKRoot_oid, FALSE } ,
        { PKList_name, CNULL, &PKList_oid, FALSE } ,
        { EKList_name, CNULL, &EKList_oid, FALSE } ,
        { PCAList_name, CNULL, &PCAList_oid, FALSE } ,
        { CrossCSet_name, CNULL, &CrossCSet_oid, FALSE } ,
        { CrlSet_name, CNULL, &CrlSet_oid, FALSE } ,
        { SignCSet_name, CNULL, &SignCSet_oid, FALSE } ,
        { EncCSet_name, CNULL, &EncCSet_oid, FALSE } ,
        { CSet_name, CNULL, &CSet_oid, FALSE } ,
        { SerialNumber_name, CNULL, &SerialNumber_oid, FALSE } ,
        { EDBKey_name, CNULL, &EDBKey_oid, FALSE } ,
        { AliasList_name, CNULL, &AliasList_oid, FALSE } ,
        { Random_name, CNULL, &Random_oid, FALSE } ,
        { QuipuPWD_name, CNULL, &QuipuPWD_oid, FALSE } ,
        { DHparam_name, CNULL, &DHparam_oid, FALSE } ,
        } ,
        0 
};

VerificationResult * verifresult = (VerificationResult * )0;


Boolean    af_verbose = FALSE;
Boolean    af_chk_crl = FALSE;
Boolean    af_access_directory = FALSE;
Boolean    af_FCPath_is_trusted = FALSE;
Boolean    af_check_Validity = TRUE;
Boolean    af_sign_check_Validity = TRUE;
Boolean    chk_PEM_subordination = FALSE;
Boolean    accept_alias_without_verification = FALSE;
Boolean    af_use_alias = TRUE;

DName    * directory_user_dname = NULLDNAME;

#ifdef X500
Boolean    af_x500 = TRUE;
char     * af_dir_dsaname = CNULL;
char     * af_dir_tailor = CNULL;
int        af_dir_authlevel = DBA_AUTH_NONE;
Boolean    af_dir_dsabound = FALSE;
#ifdef STRONG
Boolean    af_strong = TRUE;
#else
Boolean    af_strong = FALSE;
#endif
#else
Boolean    af_x500 = FALSE;
Boolean    af_strong = FALSE;
#endif
#ifdef COSINE
Boolean    af_COSINE = TRUE;
#else
Boolean    af_COSINE = FALSE;
#endif

#ifdef AFDBFILE
Boolean    af_afdb = TRUE;
#else
Boolean    af_afdb = FALSE;
#endif

