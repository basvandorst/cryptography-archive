/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.
	
	PGPclHlp.h - help IDs for PGPcl
	

	$Id: PGPclHlp.h,v 1.24 1999/04/07 21:05:10 pbj Exp $
____________________________________________________________________________*/

/* base ID for PGPkm is 0000	*/

#define IDH_PGPKM_DELETEKEY					130
#define IDH_PGPKM_DELETEALLKEYS				131
#define IDH_PGPKM_DONTDELETEKEY				132
#define IDH_PGPKM_CANCELDELETE				133
											
#define IDH_PGPKM_NEWUSERID					141
#define IDH_PGPKM_NEWEMAILADDR				147
#define IDH_PGPKM_NEWPHOTOID				148
#define IDH_PGPKM_BROWSENEWPHOTOID			149
											
#define IDH_PGPKM_KEYID						160
#define IDH_PGPKM_CREATEDATE				161
#define IDH_PGPKM_KEYTYPE					162
#define IDH_PGPKM_KEYEXPIRES				163
#define IDH_PGPKM_VALIDITY					164
#define IDH_PGPKM_VALIDITYBAR				165
#define IDH_PGPKM_TRUST						166
#define IDH_PGPKM_TRUSTSLIDER				167
#define IDH_PGPKM_HEXFINGERPRINT			168
#define IDH_PGPKM_ENABLED					169
#define IDH_PGPKM_CHANGEPHRASE				170
#define IDH_PGPKM_AXIOMATIC					171
#define IDH_PGPKM_PROPDIALOG				172
#define IDH_PGPKM_DISPLAYHEXFINGERPRINT		173
#define IDH_PGPKM_KEYSIZE					174
#define IDH_PGPKM_CIPHER					175
#define IDH_PGPKM_PHOTOID					176
#define IDH_PGPKM_JOINKEY					177
#define IDH_PGPKM_FINGERPRINT				178
											
#define IDH_PGPKM_SUBKEYLIST				180
#define IDH_PGPKM_SUBKEYCREATE				181
#define IDH_PGPKM_SUBKEYREVOKE				182
#define IDH_PGPKM_SUBKEYREMOVE				183
											
#define IDH_PGPKM_ADKLIST					190
											
#define IDH_PGPKM_REVOKERLIST				195
											
#define IDH_PGPKM_SIGNDIALOG				200
#define IDH_PGPKM_MORESIGCHOICES			201
#define IDH_PGPKM_FEWERSIGCHOICES			202
#define IDH_PGPKM_SIGNUSERIDLIST			203
#define IDH_PGPKM_ALLOWSIGEXPORT			204
#define IDH_PGPKM_SIGNONEXPORTABLE			205
#define IDH_PGPKM_SIGEXPORTABLE				206
#define IDH_PGPKM_SIGTRUSTED				207
#define IDH_PGPKM_SIGMETA					208
#define IDH_PGPKM_DOMAINRESTRICTION			209
#define IDH_PGPKM_SIGNEVEREXPIRES			210
#define IDH_PGPKM_SIGEXPIRES				211
#define IDH_PGPKM_SIGEXPIRATIONDATE			212

#define IDH_PGPKM_REVOKECERTDIALOG			220
											
#define	IDH_PGPKM_IMPORT					261
#define	IDH_PGPKM_IMPORTSELALL				262
#define	IDH_PGPKM_IMPORTUNSELALL			263
#define	IDH_PGPKM_IMPORTLIST				264
											
#define IDH_PGPKM_NEWSUBKEYSIZE				281
#define IDH_PGPKM_NEWSUBKEYSTARTDATE		282
#define IDH_PGPKM_NEWSUBKEYNEVEREXPIRES 	283
#define IDH_PGPKM_NEWSUBKEYEXPIREDATE		284
#define IDH_PGPKM_NEWSUBKEYEXPIRES			285

#define	IDH_PGPKM_SELECT					301
#define	IDH_PGPKM_SELECTSELALL				302
#define	IDH_PGPKM_SELECTUNSELALL			303
#define	IDH_PGPKM_SELECTLIST				304

#define IDH_PGPKM_EXPORTPRIVATEKEYS			321
#define IDH_PGPKM_INCLUDEEXTENSIONS			322

#define IDH_PGPKM_PGPCERTDIALOG				360
#define IDH_PGPKM_PGPCERTNAME				361
#define IDH_PGPKM_PGPCERTKEYID				362
#define IDH_PGPKM_PGPCERTCREATION			363
#define IDH_PGPKM_PGPCERTEXPIRATION			364
#define IDH_PGPKM_PGPCERTEXPORTABLE			365
#define IDH_PGPKM_PGPCERTEXPIRED			366
#define IDH_PGPKM_PGPCERTREVOKED			367
#define IDH_PGPKM_PGPCERTSHOWSIGNER			368

#define IDH_PGPKM_X509CERTDIALOG			380
#define IDH_PGPKM_X509CERTNAME				381
#define IDH_PGPKM_X509CERTISSUER			382
#define IDH_PGPKM_X509CERTLASTCRL			383
#define IDH_PGPKM_X509CERTNEXTCRL			384
#define IDH_PGPKM_X509CERTCREATION			385
#define IDH_PGPKM_X509CERTEXPIRATION		386
#define IDH_PGPKM_X509CERTEXPORTABLE		387
#define IDH_PGPKM_X509CERTEXPIRED			388
#define IDH_PGPKM_X509CERTREVOKED			389
#define IDH_PGPKM_X509CERTSHOWSIGNER		390


/* base ID for PGPcl is 1000	*/

#define IDH_PGPCLPREF_ENCRYPTDIALOG			1100
#define IDH_PGPCLPREF_ENCRYPTTOSELF			1101
#define IDH_PGPCLPREF_PASSCACHEENABLE		1303
#define IDH_PGPCLPREF_PASSCACHETIME			1103
#define IDH_PGPCLPREF_SIGNCACHEENABLE		1102
#define IDH_PGPCLPREF_SIGNCACHETIME			1105
#define IDH_PGPCLPREF_COMMENTBLOCK			1111
#define IDH_PGPCLPREF_USEFASTKEYGEN			1110
#define IDH_PGPCLPREF_WIPECONFIRM			1112
#define IDH_PGPCLPREF_NUMWIPEPASSES			1114
#define IDH_PGPCLPREF_NUMWIPEPASSESSPIN		1115
											
#define IDH_PGPCLPREF_FILEDIALOG			1200
#define IDH_PGPCLPREF_PRIVATEKEYRING		1201
#define IDH_PGPCLPREF_BROWSEPRIVATEKEYRING	1202
#define IDH_PGPCLPREF_PUBLICKEYRING			1203
#define IDH_PGPCLPREF_BROWSEPUBLICKEYRING	1204
#define IDH_PGPCLPREF_RANDOMSEEDFILE		1205
#define IDH_PGPCLPREF_BROWSERANDOMSEEDFILE	1206
											
#define IDH_PGPCLPREF_EMAILDIALOG			1300
#define IDH_PGPCLPREF_USEPGPMIME			1301
#define IDH_PGPCLPREF_ENCRYPTBYDEFAULT		1307
#define IDH_PGPCLPREF_SIGNBYDEFAULT			1308
#define IDH_PGPCLPREF_AUTODECRYPT			1113
#define IDH_PGPCLPREF_WORDWRAPENABLE		1305
#define IDH_PGPCLPREF_WORDWRAPCOLUMN		1306
#define IDH_PGPCLPREF_ALWAYSUSESECUREVIEWER 1309
											
#define IDH_PGPCLPREF_HOTKEYDIALOG			1320
#define IDH_PGPCLPREF_ENABLEPURGEHOTKEY		1321
#define IDH_PGPCLPREF_PURGEHOTKEY			1322
#define IDH_PGPCLPREF_ENABLEENCRYPTHOTKEY	1323
#define IDH_PGPCLPREF_ENCRYPTHOTKEY			1324
#define IDH_PGPCLPREF_ENABLESIGNHOTKEY		1325
#define IDH_PGPCLPREF_SIGNHOTKEY			1326
#define IDH_PGPCLPREF_ENABLEESHOTKEY		1327
#define IDH_PGPCLPREF_ENCRYPTSIGNHOTKEY		1328
#define IDH_PGPCLPREF_ENABLEDECRYPTHOTKEY	1329
#define IDH_PGPCLPREF_DECRYPTHOTKEY			1330

#define IDH_PGPCLPREF_KEYSERVERDIALOG		1400
#define IDH_PGPCLPREF_SERVERLIST			1401
#define IDH_PGPCLPREF_CREATESERVER			1402
#define IDH_PGPCLPREF_REMOVESERVER			1403
#define IDH_PGPCLPREF_EDITSERVER			1409
#define IDH_PGPCLPREF_SETROOTSERVER			1410
#define IDH_PGPCLPREF_SYNCONUNKNOWNKEYS		1405
#define IDH_PGPCLPREF_SYNCONADDNAME			1406
#define IDH_PGPCLPREF_SYNCONSIGN			1407
#define IDH_PGPCLPREF_SYNCONREVOKE			1408
#define IDH_PGPCLPREF_SYNCONVERIFY			1411
#define IDH_PGPCLPREF_MOVESERVERUP			1412
#define IDH_PGPCLPREF_MOVESERVERDOWN		1413

#define IDH_PGPCLPREF_SERVERNAME			1421
#define IDH_PGPCLPREF_SERVERPROTOCOL		1424
#define IDH_PGPCLPREF_SERVERPORT			1425
#define IDH_PGPCLPREF_SERVERAUTHKEY			1426
#define IDH_PGPCLPREF_SERVERNODOMAIN		1427
#define IDH_PGPCLPREF_SERVERDOMAIN			1428
#define IDH_PGPCLPREF_SERVERLISTED			1429

#define IDH_PGPCLPREF_ADVANCEDDIALOG		1500
#define IDH_PGPCLPREF_PREFERREDALG			1504
#define IDH_PGPCLPREF_ENABLECAST			1501
#define IDH_PGPCLPREF_ENABLE3DES			1502
#define IDH_PGPCLPREF_ENABLEIDEA			1503
#define IDH_PGPCLPREF_ENABLETWOFISH			1509
#define IDH_PGPCLPREF_DISPMARGVALIDITY		1505
#define IDH_PGPCLPREF_WARNONMARGINAL		1104
#define IDH_PGPCLPREF_WARNONADK				1506
#define IDH_PGPCLPREF_EXPORTCOMPATIBLE		1507
#define IDH_PGPCLPREF_EXPORTCOMPLETE		1508

#define IDH_PGPCLPREF_CADIALOG				1520
#define IDH_PGPCLPREF_CAURL					1521
#define IDH_PGPCLPREF_CAREVOCATIONURL		1522
#define IDH_PGPCLPREF_CATYPE				1523
#define IDH_PGPCLPREF_CAROOTCERT			1524
#define IDH_PGPCLPREF_CACLEARROOTCERT		1525
#define IDH_PGPCLPREF_CASELECTROOTCERT		1526

#define IDH_PGPCLSPLIT_SPLITDIALOG			1600
#define IDH_PGPCLSPLIT_NEWSHAREHOLDER		1601
#define IDH_PGPCLSPLIT_KEYTOSPLIT			1602 
#define IDH_PGPCLSPLIT_SHAREHOLDERS			1603
#define IDH_PGPCLSPLIT_SELECTEDSHAREHOLDER	1604 
#define IDH_PGPCLSPLIT_SELECTEDSHARES		1605
#define IDH_PGPCLSPLIT_REMOVESHARES			1606
#define IDH_PGPCLSPLIT_ADDSHARES			1607
#define IDH_PGPCLSPLIT_TOTALSHARES			1608
#define IDH_PGPCLSPLIT_THRESHOLD			1609
#define IDH_PGPCLSPLIT_SPLIT				1610

#define IDH_PGPCLRECON_KEYTORECON			1631
#define IDH_PGPCLRECON_SHAREHOLDERLIST		1632
#define IDH_PGPCLRECON_SHARESCOLLECTED		1633
#define IDH_PGPCLRECON_SHARESNEEDED			1634
#define IDH_PGPCLRECON_STARTNETWORK			1635
#define IDH_PGPCLRECON_NETWORKSTATUS		1636
#define IDH_PGPCLRECON_NETWORKAUTHENTICATION	1637
#define IDH_PGPCLRECON_SELECTSHAREFILE		1638
#define IDH_PGPCLRECON_STOPNETWORK			1639

#define IDH_PGPCLSEND_SHAREFILE				1661
#define IDH_PGPCLSEND_NUMSHARESINFILE		1662
#define IDH_PGPCLSEND_REMOTECOMPUTER		1663
#define IDH_PGPCLSEND_NETWORKSTATUS			1664
#define IDH_PGPCLSEND_NETWORKAUTHENTICATION	1665
#define IDH_PGPCLSEND_SENDSHARES			1666
#define IDH_PGPCLSEND_DONE					1667

#define IDH_PGPCLAV_CATYPE					1701
#define IDH_PGPCLAV_AVLIST					1702
#define IDH_PGPCLAV_ADDAV					1703
#define IDH_PGPCLAV_EDITAV					1704
#define IDH_PGPCLAV_REMOVEAV				1705

#define IDH_PGPCLAV_ATTRIBUTE				1711
#define IDH_PGPCLAV_VALUE					1712

#define IDH_PGPCLMISC_AUTHSERVERNAME		1901
#define IDH_PGPCLMISC_AUTHKEYNAME			1902
#define IDH_PGPCLMISC_AUTHKEYFINGERPRINT	1903
#define IDH_PGPCLMISC_AUTHKEYVALIDITY		1904
#define IDH_PGPCLMISC_AUTHIMPORTKEY			1905
#define IDH_PGPCLMISC_AUTHCONFIRM			1906
#define	IDH_PGPCLMISC_AUTHCERTIFICATE		1907
#define	IDH_PGPCLMISC_AUTHSIGNATURE			1908
#define	IDH_PGPCLMISC_AUTHEXCHANGE			1909
#define	IDH_PGPCLMISC_AUTHCIPHER			1910
#define	IDH_PGPCLMISC_AUTHHASH				1911

#define IDH_PGPCLX509_SELECTLIST			1921
#define IDH_PGPCLX509_SELECT				1922
