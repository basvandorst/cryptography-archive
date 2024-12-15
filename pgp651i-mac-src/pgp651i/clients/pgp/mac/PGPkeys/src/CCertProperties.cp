/*____________________________________________________________________________	Copyright (C) 1998 Network Associates, Inc.	All rights reserved.	$Id: CCertProperties.cp,v 1.4 1998/12/30 10:06:21 wprice Exp $____________________________________________________________________________*/#include <URegistrar.h>#include <LPushButton.h>#include <LStaticText.h>#include <LTextGroupBox.h>#include <LIconPane.h>#include <LBevelButton.h>#include "MacStrings.h"#include "pgpUtilities.h"#include "CCertProperties.h"#include "CPGPKeys.h"#include "CKeyTable.h"#include "CKeyView.h"#include "CKeyInfoWindow.h"#include "ResourceConstants.h"const ResIDT		kCertPropWindowID		= 159;const Int16			kUserIDStringLength		= 256;short CCertPropertiesWindow::sNumCertPropWindows	= 0;CCertPropertiesWindow *CCertPropertiesWindow::sCertWindows[kMaxCertPropWindows];	BooleanCCertPropertiesWindow::OpenCertProp(	PGPSigRef				cert,	PGPKeyRef				ownerKey,	CKeyTable *				table){	CCertPropertiesWindow *	certPropWindow;	short cpIndex;		if(sNumCertPropWindows < kMaxCertPropWindows)	{		for(cpIndex = 0;cpIndex < sNumCertPropWindows;cpIndex++)		{			if( sCertWindows[cpIndex]->GetCert() == cert )			{				sCertWindows[cpIndex]->Select();				return TRUE;			}		}		RegisterClass_( CCertPropertiesWindow );				certPropWindow = (CCertPropertiesWindow *)			LWindow::CreateWindow( kCertPropWindowID, CPGPKeys::TheApp() );		sCertWindows[sNumCertPropWindows] = certPropWindow;		certPropWindow->SetInfo( cert, ownerKey, table );		sNumCertPropWindows++;		return TRUE;	}	else		return FALSE;}	voidCCertPropertiesWindow::CloseAll(){		while(sNumCertPropWindows > 0)	{		pgpAssertAddrValid(sCertWindows[0], VoidAlign);		sCertWindows[0]->DoClose();	}	pgpAssert(sNumCertPropWindows == 0);}	PGPSigRefCCertPropertiesWindow::GetCert(){	return mWindowCert;}CCertPropertiesWindow::CCertPropertiesWindow(LStream *inStream)	:	LWindow(inStream){}CCertPropertiesWindow::~CCertPropertiesWindow(){}	voidCCertPropertiesWindow::ProcessClose(void){	short	windowIndex;	short	windowID;		Hide();		// Remove this Cert Prop window from the list	for(windowID = 0;windowID < sNumCertPropWindows;windowID++)	{		if(sCertWindows[windowID] == this)		{			sCertWindows[windowID] = NULL;			break;		}	}		for(windowIndex = windowID + 1; windowIndex < sNumCertPropWindows;		windowIndex++)		sCertWindows[windowIndex-1] = sCertWindows[windowIndex];	sNumCertPropWindows--;}	voidCCertPropertiesWindow::AttemptClose(void){	ProcessClose();		LWindow::AttemptClose();}	voidCCertPropertiesWindow::DoClose(void){	ProcessClose();		LWindow::DoClose();}voidCCertPropertiesWindow::ListenToMessage(MessageT inMessage, void *ioParam){	PGPError	err;	PGPBoolean	x509;		switch(inMessage)	{		case kGetSignerButton:			err = PGPGetSigBoolean( mWindowCert, kPGPSigPropIsX509, &x509 );			pgpAssertNoErr( err );			if( x509 )			{				if( PGPSigRefIsValid( mSignerCert ) )				{					OpenCertProp( mSignerCert, PGPGetSigKey( mSignerCert ),									mParentTable );				}			}			else				CKeyInfoWindow::OpenKeyInfo( mSignerKey, mParentTable );			break;	}}	voidCCertPropertiesWindow::SetInfo(	PGPSigRef		cert,	PGPKeyRef		ownerKey,	CKeyTable *		table ){	PGPError		err;	PGPBoolean		x509;	LPushButton *	getSignerButton;	PGPTime			omegaDate;	Str255			pstr;	char			cstr[kUserIDStringLength];	PGPSize			stringLen;	PGPBoolean		revoked,					expired,					verified,					tried,					notCorrupt,					exportable;	PGPInt32		trustLevel;	ResIDT			iconID;		mWindowCert		= cert;	mOwnerKey		= ownerKey;	mParentTable	= table;	mSignerKey		= kInvalidPGPKeyRef;	mSignerCert		= kInvalidPGPSigRef;	getSignerButton = (LPushButton *)FindPaneByID( kGetSignerButton );	getSignerButton->AddListener( this );	PGPGetSigBoolean( mWindowCert, kPGPSigPropIsX509, &x509);	if( x509 )	{		err = PGPGetSigX509CertifierSig( mWindowCert,					table->GetKeySet(),					&mSignerCert );		if( IsPGPError( err ) && !table->IsDefault() )		{			err = PGPGetSigX509CertifierSig( mWindowCert,						CPGPKeys::TheApp()->GetKeySet(),						&mSignerCert );					}		if( mSignerCert == mWindowCert )			mSignerCert = kInvalidPGPSigRef;	}	else	{		err = PGPGetSigCertifierKey( mWindowCert, table->GetKeySet(),					&mSignerKey );		if( IsPGPError( err ) && !table->IsDefault() )		{			err = PGPGetSigCertifierKey( mWindowCert, 					CPGPKeys::TheApp()->GetKeySet(),					&mSignerKey);		}	}	if( IsntPGPError( err ) )		mParentTable =			CPGPKeys::TheApp()->GetDefaultKeyView()->GetKeyTable();	if( PGPKeyRefIsValid( mSignerKey ) || PGPSigRefIsValid( mSignerCert ) )		getSignerButton->Enable();		PGPGetSigTime( mWindowCert, kPGPSigPropCreation, &omegaDate );	omegaDate = PGPTimeToMacTime( omegaDate );	DateString( omegaDate, shortDate, pstr, NULL );	( (LStaticText *)FindPaneByID( kCertCreationDate ) )->SetDescriptor( pstr );	PGPGetSigTime( mWindowCert, kPGPSigPropExpiration, &omegaDate );	if( omegaDate != 0 )	{		omegaDate = PGPTimeToMacTime( omegaDate );		DateString( omegaDate, shortDate, pstr, NULL );	}	else		GetIndString( pstr, kCertPropStringListID, kNeverExpiresStringID );	( (LStaticText *)FindPaneByID( kCertExpirationDate ) )->SetDescriptor( pstr );		PGPGetSigBoolean( mWindowCert, kPGPSigPropIsRevoked, &revoked);	PGPGetSigBoolean( mWindowCert, kPGPSigPropIsVerified, &verified);	PGPGetSigBoolean( mWindowCert, kPGPSigPropIsTried, &tried);	PGPGetSigBoolean( mWindowCert, kPGPSigPropIsNotCorrupt, &notCorrupt);	PGPGetSigBoolean( mWindowCert, kPGPSigPropIsExportable, &exportable);	PGPGetSigBoolean( mWindowCert, kPGPSigPropIsExpired, &expired);	PGPGetSigNumber( mWindowCert, kPGPSigPropTrustLevel, &trustLevel);	if( x509 )	{		if( revoked )			iconID = kX509CertRevokedIconID;		else if( expired )			iconID = kX509CertExpiredIconID;		else			iconID = kX509CertIconID;	}	else if(revoked)		iconID = 	kRevokedSigID;	else if(expired)		iconID =	kSignatureExpiredIconID;	else if(verified)	{		if(exportable)		{			if(trustLevel == 1)				iconID =	kExportMetaSigIconID;			else				iconID =	kExportSignatureIconID;		}		else		{			if(trustLevel == 2)				iconID =	kNoExportMetaSigIconID;			else				iconID =	kNoExportSignatureIconID;		}	}	else if(tried)		iconID =	kBadSignatureID;	else if(notCorrupt)	{		if( exportable )			iconID =	kExportSignatureIconID;		else			iconID =	kNoExportSignatureIconID;	}	else		iconID =	kBadSignatureID;	( (LIconPane *) FindPaneByID( kCertIcon ) )->SetIconID( iconID );		if( exportable )		( (LBevelButton *)FindPaneByID( kExportableButton ) )->Show();	if( expired )		( (LBevelButton *)FindPaneByID( kExpiredButton ) )->Show();	if( revoked )		( (LBevelButton *)FindPaneByID( kRevokedButton ) )->Show();		if( x509 )	{		PGPBoolean	hasCRL;				( (LTextGroupBox *)FindPaneByID( kX509PropertiesBox ) )->Show();		err = PGPGetSigPropertyBuffer( mWindowCert, kPGPSigPropX509LongName,										sizeof(cstr), &cstr[0], &stringLen );		if( IsntPGPError( err ) )		{			cstr[stringLen] = 0;			CToPString(cstr, pstr);			( (LStaticText *)FindPaneByID( kX509LongName ) )->SetDescriptor( pstr );		}		err = PGPGetSigPropertyBuffer( mWindowCert, kPGPSigPropX509IssuerLongName,										sizeof(cstr), &cstr[0], &stringLen );		if( IsntPGPError( err ) )		{			cstr[stringLen] = 0;			CToPString(cstr, pstr);			( (LStaticText *)FindPaneByID( kX509IssuerName ) )->SetDescriptor( pstr );		}		PGPGetKeyBoolean( mOwnerKey, kPGPKeyPropHasCRL, &hasCRL );		if( hasCRL )		{			PGPGetKeyTime( mOwnerKey, kPGPKeyPropCRLThisUpdate, &omegaDate );			omegaDate = PGPTimeToMacTime( omegaDate );			DateString( omegaDate, shortDate, pstr, NULL );		}		else			GetIndString( pstr, kCertPropStringListID, kNAStringID );		( (LStaticText *)FindPaneByID( kX509LastCRL ) )->SetDescriptor( pstr );		if( hasCRL )		{			PGPGetKeyTime( mOwnerKey, kPGPKeyPropCRLNextUpdate, &omegaDate );			omegaDate = PGPTimeToMacTime( omegaDate );			DateString( omegaDate, shortDate, pstr, NULL );		}		( (LStaticText *)FindPaneByID( kX509NextCRL ) )->SetDescriptor( pstr );	}	else	{		Rect		windBounds;		PGPKeyID	keyID;		char		keyIDString[ kPGPMaxKeyIDStringSize ];				GetMinMaxSize(windBounds);		ResizeWindowTo(windBounds.right, windBounds.top);		( (LTextGroupBox *)FindPaneByID( kPGPPropertiesBox ) )->Show();		err = PGPGetKeyIDOfCertifier( mWindowCert, &keyID );		if( IsntPGPError( err ) )		{			PGPGetKeyIDString( &keyID, kPGPKeyIDString_Abbreviated,									keyIDString );			CToPString( keyIDString, pstr );			( (LStaticText *)FindPaneByID( kPGPKeyID ) )->SetDescriptor( pstr );		}		GetIndString( pstr, kCertPropStringListID, kNAStringID );		if( PGPKeyRefIsValid( mSignerKey ) )		{			err = PGPGetPrimaryUserIDNameBuffer( mSignerKey,					kUserIDStringLength, cstr, &stringLen );			if( IsntPGPError( err ) )			{				CToPString(cstr, pstr);			}		}		( (LStaticText *)FindPaneByID( kPGPSignerName ) )->SetDescriptor( pstr );	}	Show();}