/* lib/obj/obj_dat.h */
/* Copyright (C) 1995 Eric Young (eay@mincom.oz.au)
 * All rights reserved.
 * 
 * This file is part of an SSL implementation written
 * by Eric Young (eay@mincom.oz.au).
 * The implementation was written so as to conform with Netscapes SSL
 * specification.  This library and applications are
 * FREE FOR COMMERCIAL AND NON-COMMERCIAL USE
 * as long as the following conditions are aheared to.
 * 
 * Copyright remains Eric Young's, and as such any Copyright notices in
 * the code are not to be removed.  If this code is used in a product,
 * Eric Young should be given attribution as the author of the parts used.
 * This can be in the form of a textual message at program startup or
 * in documentation (online or textual) provided with the package.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *    This product includes software developed by Eric Young (eay@mincom.oz.au)
 * 
 * THIS SOFTWARE IS PROVIDED BY ERIC YOUNG ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 * 
 * The licence and distribution terms for any publically available version or
 * derivative of this code cannot be changed.  i.e. this code cannot simply be
 * copied and put under another distribution licence
 * [including the GNU Public Licence.]
 */

/* THIS FILE IS GENERATED FROM Objects.h by obj_dat.pl via the
 * following command:
 * perl obj_dat.pl < objects.h > obj_dat.h
 */

#define NUM_NID 65
#define NUM_SN 36
#define NUM_LN 63
#define NUM_OBJ 48

static unsigned char lvalues[336]={
0x2A,0x86,0x48,0x86,0xF7,0x0D,               /* [  0] OBJ_rsadsi */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,          /* [  6] OBJ_pkcs */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x02,0x02,     /* [ 13] OBJ_md2 */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x02,0x05,     /* [ 21] OBJ_md5 */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x03,0x04,     /* [ 29] OBJ_rc4 */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x01,0x01,/* [ 37] OBJ_rsaEncryption */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x01,0x02,/* [ 46] OBJ_md2withRSAEncryption */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x01,0x04,/* [ 55] OBJ_md5withRSAEncryption */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x05,0x01,/* [ 64] OBJ_pbeWithMD2AndDES_CBC */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x05,0x03,/* [ 73] OBJ_pbeWithMD5AndDES_CBC */
0x55,                                        /* [ 82] OBJ_X500 */
0x55,0x04,                                   /* [ 83] OBJ_X509 */
0x55,0x04,0x03,                              /* [ 85] OBJ_commonName */
0x55,0x04,0x06,                              /* [ 88] OBJ_countryName */
0x55,0x04,0x07,                              /* [ 91] OBJ_localityName */
0x55,0x04,0x08,                              /* [ 94] OBJ_stateOrProvinceName */
0x55,0x04,0x0A,                              /* [ 97] OBJ_organizationName */
0x55,0x04,0x0B,                              /* [100] OBJ_organizationalUnitName */
0x55,0x08,0x01,0x01,                         /* [103] OBJ_rsa */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x07,     /* [107] OBJ_pkcs7 */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x07,0x01,/* [115] OBJ_pkcs7_data */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x07,0x02,/* [124] OBJ_pkcs7_signed */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x07,0x03,/* [133] OBJ_pkcs7_enveloped */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x07,0x04,/* [142] OBJ_pkcs7_signedAndEnveloped */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x07,0x05,/* [151] OBJ_pkcs7_digest */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x07,0x06,/* [160] OBJ_pkcs7_encrypted */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x03,     /* [169] OBJ_pkcs3 */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x03,0x01,/* [177] OBJ_dhKeyAgreement */
0x2B,0x0E,0x03,0x02,0x06,                    /* [186] OBJ_des_ecb */
0x2B,0x0E,0x03,0x02,0x09,                    /* [191] OBJ_des_cfb */
0x2B,0x0E,0x03,0x02,0x07,                    /* [196] OBJ_des_cbc */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x03,0x11,     /* [201] OBJ_des_ede3 */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x03,0x02,     /* [209] OBJ_rc2_cbc */
0x2B,0x0E,0x03,0x02,0x12,                    /* [217] OBJ_sha */
0x2B,0x0E,0x03,0x02,0x08,                    /* [222] OBJ_des_ofb */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x09,     /* [227] OBJ_pkcs9 */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x09,0x01,/* [235] OBJ_pkcs9_emailAddress */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x09,0x02,/* [244] OBJ_pkcs9_unstructuredName */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x09,0x03,/* [253] OBJ_pkcs9_contentType */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x09,0x04,/* [262] OBJ_pkcs9_messageDigest */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x09,0x05,/* [271] OBJ_pkcs9_signingTime */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x09,0x06,/* [280] OBJ_pkcs9_countersignature */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x09,0x07,/* [289] OBJ_pkcs9_challengePassword */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x09,0x08,/* [298] OBJ_pkcs9_unstructuredAddress */
0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x09,0x09,/* [307] OBJ_pkcs9_extCertAttributes */
0x60,0x86,0x48,0xD8,0x6A,                    /* [316] OBJ_netscape */
0x60,0x86,0x48,0xD8,0x6A,0x01,0x02,          /* [321] OBJ_netscape_gif2 */
0x60,0x86,0x48,0xD8,0x6A,0x01,0x03,          /* [328] OBJ_netscape_gif3 */
};

static ASN1_OBJECT nid_objs[NUM_NID]={
{"UNDEF","undefined",NID_undef,0,NULL},
{"rsadsi","rsadsi",NID_rsadsi,6,&(lvalues[0])},
{"pkcs","pkcs",NID_pkcs,7,&(lvalues[6])},
{"MD2","md2",NID_md2,8,&(lvalues[13])},
{"MD5","md5",NID_md5,8,&(lvalues[21])},
{"RC4","rc4",NID_rc4,8,&(lvalues[29])},
{"rsaEncryption","rsaEncryption",NID_rsaEncryption,9,&(lvalues[37])},
{"RSA-MD2","md2withRSAEncryption",NID_md2withRSAEncryption,9,
	&(lvalues[46])},
{"RSA-MD5","md5withRSAEncryption",NID_md5withRSAEncryption,9,
	&(lvalues[55])},
{"pbeWithMD2AndDES_CBC","pbeWithMD2AndDES_CBC",
	NID_pbeWithMD2AndDES_CBC,9,&(lvalues[64])},
{"pbeWithMD5AndDES_CBC","pbeWithMD5AndDES_CBC",
	NID_pbeWithMD5AndDES_CBC,9,&(lvalues[73])},
{"X500","X500",NID_X500,1,&(lvalues[82])},
{"X509","X509",NID_X509,2,&(lvalues[83])},
{"CN","commonName",NID_commonName,3,&(lvalues[85])},
{"C","countryName",NID_countryName,3,&(lvalues[88])},
{"L","localityName",NID_localityName,3,&(lvalues[91])},
{"SP","stateOrProvinceName",NID_stateOrProvinceName,3,&(lvalues[94])},
{"O","organizationName",NID_organizationName,3,&(lvalues[97])},
{"OU","organizationalUnitName",NID_organizationalUnitName,3,
	&(lvalues[100])},
{"RSA","rsa",NID_rsa,4,&(lvalues[103])},
{"pkcs7","pkcs7",NID_pkcs7,8,&(lvalues[107])},
{"pkcs7-data","pkcs7-data",NID_pkcs7_data,9,&(lvalues[115])},
{"pkcs7-signedData","pkcs7-signedData",NID_pkcs7_signed,9,
	&(lvalues[124])},
{"pkcs7-envelopedData","pkcs7-envelopedData",NID_pkcs7_enveloped,9,
	&(lvalues[133])},
{"pkcs7-signedAndEnvelopedData","pkcs7-signedAndEnvelopedData",
	NID_pkcs7_signedAndEnveloped,9,&(lvalues[142])},
{"pkcs7-digestData","pkcs7-digestData",NID_pkcs7_digest,9,
	&(lvalues[151])},
{"pkcs7-encryptedData","pkcs7-encryptedData",NID_pkcs7_encrypted,9,
	&(lvalues[160])},
{"pkcs3","pkcs3",NID_pkcs3,8,&(lvalues[169])},
{"dhKeyAgreement","dhKeyAgreement",NID_dhKeyAgreement,9,&(lvalues[177])},
{"DES-ECB","des-ecb",NID_des_ecb,5,&(lvalues[186])},
{"DES-CFB","des-cfb",NID_des_cfb,5,&(lvalues[191])},
{"DES-CBC","des-cbc",NID_des_cbc,5,&(lvalues[196])},
{"DES-EDE","des-ede",NID_des_ede,0,NULL},
{"DES-EDE3","des-ede3",NID_des_ede3,8,&(lvalues[201])},
{"IDEA-CBC","idea-cbc",NID_idea_cbc,0,NULL},
{"IDEA-CFB","idea-cfb",NID_idea_cfb,0,NULL},
{"IDEA-ECB","idea-ecb",NID_idea_ecb,0,NULL},
{"RC2-CBC","rc2-cbc",NID_rc2_cbc,8,&(lvalues[209])},
{NULL,NULL,NID_undef,0,NULL},
{NULL,NULL,NID_undef,0,NULL},
{"SHA","sha",NID_sha,5,&(lvalues[217])},
{"RSA-SHA","shaWithRSAEncryption",NID_shaWithRSAEncryption,0,NULL},
{"DES-EDE-CBC","des-ede-cbc",NID_des_ede_cbc,0,NULL},
{"DES-EDE3-CBC","des-ede3-cbc",NID_des_ede3_cbc,0,NULL},
{"DES-OFB","des-ofb",NID_des_ofb,5,&(lvalues[222])},
{"IDEA-OFB","idea-ofb",NID_idea_ofb,0,NULL},
{"pkcs9","pkcs9",NID_pkcs9,8,&(lvalues[227])},
{"Email","emailAddress",NID_pkcs9_emailAddress,9,&(lvalues[235])},
{"unstructuredName","unstructuredName",NID_pkcs9_unstructuredName,9,
	&(lvalues[244])},
{"contentType","contentType",NID_pkcs9_contentType,9,&(lvalues[253])},
{"messageDigest","messageDigest",NID_pkcs9_messageDigest,9,
	&(lvalues[262])},
{"signingTime","signingTime",NID_pkcs9_signingTime,9,&(lvalues[271])},
{"countersignature","countersignature",NID_pkcs9_countersignature,9,
	&(lvalues[280])},
{"challengePassword","challengePassword",NID_pkcs9_challengePassword,
	9,&(lvalues[289])},
{"unstructuredAddress","unstructuredAddress",
	NID_pkcs9_unstructuredAddress,9,&(lvalues[298])},
{"extendedCertificateAttributes","extendedCertificateAttributes",
	NID_pkcs9_extCertAttributes,9,&(lvalues[307])},
{"Netscape","Netscape Communications Corp.",NID_netscape,5,
	&(lvalues[316])},
{"ns-gif-2","ns-gif-2",NID_netscape_gif2,7,&(lvalues[321])},
{"ns-gif-3","ns-gif-3",NID_netscape_gif3,7,&(lvalues[328])},
{"DES-EDE-CFB","des-ede-cfb",NID_des_ede_cfb,0,NULL},
{"DES-EDE3-CFB","des-ede3-cfb",NID_des_ede3_cfb,0,NULL},
{"DES-EDE-OFB","des-ede-ofb",NID_des_ede_ofb,0,NULL},
{"DES-EDE3-OFB","des-ede3-ofb",NID_des_ede3_ofb,0,NULL},
{"SHA1","sha1",NID_sha1,0,NULL},
{"RSA-SHA1","sha1WithRSAEncryption",NID_sha1WithRSAEncryption,0,NULL},
};

static ASN1_OBJECT *sn_objs[NUM_SN]={
&(nid_objs[14]),/* "C" */
&(nid_objs[13]),/* "CN" */
&(nid_objs[31]),/* "DES-CBC" */
&(nid_objs[30]),/* "DES-CFB" */
&(nid_objs[29]),/* "DES-ECB" */
&(nid_objs[32]),/* "DES-EDE" */
&(nid_objs[42]),/* "DES-EDE-CBC" */
&(nid_objs[59]),/* "DES-EDE-CFB" */
&(nid_objs[61]),/* "DES-EDE-OFB" */
&(nid_objs[33]),/* "DES-EDE3" */
&(nid_objs[43]),/* "DES-EDE3-CBC" */
&(nid_objs[60]),/* "DES-EDE3-CFB" */
&(nid_objs[62]),/* "DES-EDE3-OFB" */
&(nid_objs[44]),/* "DES-OFB" */
&(nid_objs[47]),/* "Email" */
&(nid_objs[34]),/* "IDEA-CBC" */
&(nid_objs[35]),/* "IDEA-CFB" */
&(nid_objs[36]),/* "IDEA-ECB" */
&(nid_objs[45]),/* "IDEA-OFB" */
&(nid_objs[15]),/* "L" */
&(nid_objs[ 3]),/* "MD2" */
&(nid_objs[ 4]),/* "MD5" */
&(nid_objs[56]),/* "Netscape" */
&(nid_objs[17]),/* "O" */
&(nid_objs[18]),/* "OU" */
&(nid_objs[37]),/* "RC2-CBC" */
&(nid_objs[ 5]),/* "RC4" */
&(nid_objs[19]),/* "RSA" */
&(nid_objs[ 7]),/* "RSA-MD2" */
&(nid_objs[ 8]),/* "RSA-MD5" */
&(nid_objs[41]),/* "RSA-SHA" */
&(nid_objs[64]),/* "RSA-SHA1" */
&(nid_objs[40]),/* "SHA" */
&(nid_objs[63]),/* "SHA1" */
&(nid_objs[16]),/* "SP" */
&(nid_objs[ 0]),/* "UNDEF" */
};

static ASN1_OBJECT *ln_objs[NUM_LN]={
&(nid_objs[56]),/* "Netscape Communications Corp." */
&(nid_objs[11]),/* "X500" */
&(nid_objs[12]),/* "X509" */
&(nid_objs[53]),/* "challengePassword" */
&(nid_objs[13]),/* "commonName" */
&(nid_objs[49]),/* "contentType" */
&(nid_objs[52]),/* "countersignature" */
&(nid_objs[14]),/* "countryName" */
&(nid_objs[31]),/* "des-cbc" */
&(nid_objs[30]),/* "des-cfb" */
&(nid_objs[29]),/* "des-ecb" */
&(nid_objs[32]),/* "des-ede" */
&(nid_objs[42]),/* "des-ede-cbc" */
&(nid_objs[59]),/* "des-ede-cfb" */
&(nid_objs[61]),/* "des-ede-ofb" */
&(nid_objs[33]),/* "des-ede3" */
&(nid_objs[43]),/* "des-ede3-cbc" */
&(nid_objs[60]),/* "des-ede3-cfb" */
&(nid_objs[62]),/* "des-ede3-ofb" */
&(nid_objs[44]),/* "des-ofb" */
&(nid_objs[28]),/* "dhKeyAgreement" */
&(nid_objs[47]),/* "emailAddress" */
&(nid_objs[55]),/* "extendedCertificateAttributes" */
&(nid_objs[34]),/* "idea-cbc" */
&(nid_objs[35]),/* "idea-cfb" */
&(nid_objs[36]),/* "idea-ecb" */
&(nid_objs[45]),/* "idea-ofb" */
&(nid_objs[15]),/* "localityName" */
&(nid_objs[ 3]),/* "md2" */
&(nid_objs[ 7]),/* "md2withRSAEncryption" */
&(nid_objs[ 4]),/* "md5" */
&(nid_objs[ 8]),/* "md5withRSAEncryption" */
&(nid_objs[50]),/* "messageDigest" */
&(nid_objs[57]),/* "ns-gif-2" */
&(nid_objs[58]),/* "ns-gif-3" */
&(nid_objs[17]),/* "organizationName" */
&(nid_objs[18]),/* "organizationalUnitName" */
&(nid_objs[ 9]),/* "pbeWithMD2AndDES_CBC" */
&(nid_objs[10]),/* "pbeWithMD5AndDES_CBC" */
&(nid_objs[ 2]),/* "pkcs" */
&(nid_objs[27]),/* "pkcs3" */
&(nid_objs[20]),/* "pkcs7" */
&(nid_objs[21]),/* "pkcs7-data" */
&(nid_objs[25]),/* "pkcs7-digestData" */
&(nid_objs[26]),/* "pkcs7-encryptedData" */
&(nid_objs[23]),/* "pkcs7-envelopedData" */
&(nid_objs[24]),/* "pkcs7-signedAndEnvelopedData" */
&(nid_objs[22]),/* "pkcs7-signedData" */
&(nid_objs[46]),/* "pkcs9" */
&(nid_objs[37]),/* "rc2-cbc" */
&(nid_objs[ 5]),/* "rc4" */
&(nid_objs[19]),/* "rsa" */
&(nid_objs[ 6]),/* "rsaEncryption" */
&(nid_objs[ 1]),/* "rsadsi" */
&(nid_objs[40]),/* "sha" */
&(nid_objs[63]),/* "sha1" */
&(nid_objs[64]),/* "sha1WithRSAEncryption" */
&(nid_objs[41]),/* "shaWithRSAEncryption" */
&(nid_objs[51]),/* "signingTime" */
&(nid_objs[16]),/* "stateOrProvinceName" */
&(nid_objs[ 0]),/* "undefined" */
&(nid_objs[54]),/* "unstructuredAddress" */
&(nid_objs[48]),/* "unstructuredName" */
};

static ASN1_OBJECT *obj_objs[NUM_OBJ]={
&(nid_objs[11]),/* OBJ_X500                         2 5 */
&(nid_objs[12]),/* OBJ_X509                         2 5 4 */
&(nid_objs[13]),/* OBJ_commonName                   2 5 4 3 */
&(nid_objs[14]),/* OBJ_countryName                  2 5 4 6 */
&(nid_objs[15]),/* OBJ_localityName                 2 5 4 7 */
&(nid_objs[16]),/* OBJ_stateOrProvinceName          2 5 4 8 */
&(nid_objs[17]),/* OBJ_organizationName             2 5 4 10 */
&(nid_objs[18]),/* OBJ_organizationalUnitName       2 5 4 11 */
&(nid_objs[19]),/* OBJ_rsa                          2 5 8 1 1 */
&(nid_objs[29]),/* OBJ_des_ecb                      1 3 14 3 2 6 */
&(nid_objs[31]),/* OBJ_des_cbc                      1 3 14 3 2 7 */
&(nid_objs[44]),/* OBJ_des_ofb                      1 3 14 3 2 8 */
&(nid_objs[30]),/* OBJ_des_cfb                      1 3 14 3 2 9 */
&(nid_objs[40]),/* OBJ_sha                          1 3 14 3 2 18 */
&(nid_objs[56]),/* OBJ_netscape                     2 16 840 11370 */
&(nid_objs[ 1]),/* OBJ_rsadsi                       1 2 840 113549 */
&(nid_objs[ 2]),/* OBJ_pkcs                         1 2 840 113549 1 */
&(nid_objs[57]),/* OBJ_netscape_gif2                2 16 840 11370 1 2 */
&(nid_objs[58]),/* OBJ_netscape_gif3                2 16 840 11370 1 3 */
&(nid_objs[27]),/* OBJ_pkcs3                        1 2 840 113549 1 3 */
&(nid_objs[20]),/* OBJ_pkcs7                        1 2 840 113549 1 7 */
&(nid_objs[46]),/* OBJ_pkcs9                        1 2 840 113549 1 9 */
&(nid_objs[ 3]),/* OBJ_md2                          1 2 840 113549 2 2 */
&(nid_objs[ 4]),/* OBJ_md5                          1 2 840 113549 2 5 */
&(nid_objs[37]),/* OBJ_rc2_cbc                      1 2 840 113549 3 2 */
&(nid_objs[ 5]),/* OBJ_rc4                          1 2 840 113549 3 4 */
&(nid_objs[33]),/* OBJ_des_ede3                     1 2 840 113549 3 17 */
&(nid_objs[ 6]),/* OBJ_rsaEncryption                1 2 840 113549 1 1 1 */
&(nid_objs[ 7]),/* OBJ_md2withRSAEncryption         1 2 840 113549 1 1 2 */
&(nid_objs[ 8]),/* OBJ_md5withRSAEncryption         1 2 840 113549 1 1 4 */
&(nid_objs[28]),/* OBJ_dhKeyAgreement               1 2 840 113549 1 3 1 */
&(nid_objs[ 9]),/* OBJ_pbeWithMD2AndDES_CBC         1 2 840 113549 1 5 1 */
&(nid_objs[10]),/* OBJ_pbeWithMD5AndDES_CBC         1 2 840 113549 1 5 3 */
&(nid_objs[21]),/* OBJ_pkcs7_data                   1 2 840 113549 1 7 1 */
&(nid_objs[22]),/* OBJ_pkcs7_signed                 1 2 840 113549 1 7 2 */
&(nid_objs[23]),/* OBJ_pkcs7_enveloped              1 2 840 113549 1 7 3 */
&(nid_objs[24]),/* OBJ_pkcs7_signedAndEnveloped     1 2 840 113549 1 7 4 */
&(nid_objs[25]),/* OBJ_pkcs7_digest                 1 2 840 113549 1 7 5 */
&(nid_objs[26]),/* OBJ_pkcs7_encrypted              1 2 840 113549 1 7 6 */
&(nid_objs[47]),/* OBJ_pkcs9_emailAddress           1 2 840 113549 1 9 1 */
&(nid_objs[48]),/* OBJ_pkcs9_unstructuredName       1 2 840 113549 1 9 2 */
&(nid_objs[49]),/* OBJ_pkcs9_contentType            1 2 840 113549 1 9 3 */
&(nid_objs[50]),/* OBJ_pkcs9_messageDigest          1 2 840 113549 1 9 4 */
&(nid_objs[51]),/* OBJ_pkcs9_signingTime            1 2 840 113549 1 9 5 */
&(nid_objs[52]),/* OBJ_pkcs9_countersignature       1 2 840 113549 1 9 6 */
&(nid_objs[53]),/* OBJ_pkcs9_challengePassword      1 2 840 113549 1 9 7 */
&(nid_objs[54]),/* OBJ_pkcs9_unstructuredAddress    1 2 840 113549 1 9 8 */
&(nid_objs[55]),/* OBJ_pkcs9_extCertAttributes      1 2 840 113549 1 9 9 */
};

