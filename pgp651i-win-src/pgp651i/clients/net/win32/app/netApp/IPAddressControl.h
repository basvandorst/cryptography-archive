/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.
	
	IPAddressControl.h - IP Address edit custom control
	
	$Id: IPAddressControl.h,v 1.6 1999/04/01 14:53:55 pbj Exp $
____________________________________________________________________________*/

#ifndef _PGP_IPADDRESSCONTROL_H
#define _PGP_IPADDRESSCONTROL_H

#define WC_PGPIPADDRESS		("PGPIPAdressControl32")

#define PGP_IPM_FIRST			0x2000
#define PGP_IPM_SETADDRESS		PGP_IPM_FIRST+0
#define PGP_IPM_GETADDRESS		PGP_IPM_FIRST+1
#define PGP_IPM_SETRANGE		PGP_IPM_FIRST+2
#define PGP_IPM_CLEARADDRESS	PGP_IPM_FIRST+3

#define PGP_IPN_FIRST           (0U-100U)       
#define PGP_IPN_LAST            (0U-199U)

#define	PGP_IPN_VALIDADDRESS	PGP_IPN_FIRST+0
#define	PGP_IPN_INVALIDADDRESS	PGP_IPN_FIRST+1
#define	PGP_IPN_EMPTYADDRESS	PGP_IPN_FIRST+2

VOID WINAPI 
InitPGPIPAddressControl (VOID); 

#endif //_PGP_IPADDRESSCONTROL_H