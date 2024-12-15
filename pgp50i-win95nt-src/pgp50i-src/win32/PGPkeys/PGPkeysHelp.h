/*
 * Copyright (C) 1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 */
/*: PGPkeysHelp.h - defines constants for help file */

#define IDH_PGPKEYS_HIDETYPING		120
#define IDH_PGPKEYS_OLDPHRASE		121
#define IDH_PGPKEYS_NEWPHRASE1		122
#define IDH_PGPKEYS_NEWPHRASE2		123
#define IDH_PGPKEYS_CHANGEDIALOG	124

#define IDH_PGPKEYS_DELETEKEY		130
#define IDH_PGPKEYS_DELETEALLKEYS	131
#define IDH_PGPKEYS_DONTDELETEKEY	132
#define IDH_PGPKEYS_CANCELDELETE	133

#define IDH_PGPKEYS_CREATENEWKEY	140
#define IDH_PGPKEYS_NEWUSERID		141
#define IDH_PGPKEYS_KEYSIZE			142
#define IDH_PGPKEYS_SETKEYTYPE		143
#define IDH_PGPKEYS_KEYNEVEREXPIRES	144
#define IDH_PGPKEYS_SETKEYEXPIRES	145
#define IDH_PGPKEYS_CREATEDIALOG	146
#define IDH_PGPKEYS_NEWEMAILADDR	147

#define IDH_PGPKEYS_KEYID			160
#define IDH_PGPKEYS_CREATEDATE		161
#define IDH_PGPKEYS_KEYTYPE			162
#define IDH_PGPKEYS_KEYEXPIRES		163
#define IDH_PGPKEYS_VALIDITY		164
#define IDH_PGPKEYS_VALIDITYBAR		165
#define IDH_PGPKEYS_TRUST			166
#define IDH_PGPKEYS_TRUSTSLIDER		167
#define IDH_PGPKEYS_FINGERPRINT		168
#define IDH_PGPKEYS_ENABLED			169
#define IDH_PGPKEYS_CHANGEPHRASE	170
#define IDH_PGPKEYS_AXIOMATIC		171
#define IDH_PGPKEYS_PROPDIALOG		172
#define IDH_PGPKEYS_MRKAGENT		173

#define IDH_PGPKEYS_SENDSIGNATURE	200

#define IDH_PGPKEYWIZ_ABOUT			300	//introductory page	
#define IDH_PGPKEYWIZ_NAME			301	//user name and email address dialog
#define IDH_PGPKEYWIZ_TYPE			302	//key type (RSA/DSA) dialog
#define IDH_PGPKEYWIZ_SIZE			303	//key size (number of bits) dialog
#define IDH_PGPKEYWIZ_EXPIRATION	304	//key expiration dialog
#define IDH_PGPKEYWIZ_PASSPHRASE	305	//key passphrase dialog
#define IDH_PGPKEYWIZ_RANDOBITS		306	//entropy collection dialog
#define IDH_PGPKEYWIZ_GENERATION	307	//key generation dialog
#define IDH_PGPKEYWIZ_SIGNOLD		308	//sign new key with old dialog
#define IDH_PGPKEYWIZ_PRESEND		309	//asks about sending key to keyserver
#define IDH_PGPKEYWIZ_SEND			310	//dialog shown during send to keyserver
#define IDH_PGPKEYWIZ_DONE			311	//done dialog
