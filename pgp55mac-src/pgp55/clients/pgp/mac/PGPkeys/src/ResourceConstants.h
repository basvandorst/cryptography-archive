#pragma once

// many resource constants are within a class,
// but some are used by more than one component.

enum	{
			kRSAKeyIconID = 3000,
			kRSAKeyPairIconID,
			kRSAUserIDIconID,
			kNoExportSignatureIconID,
			kRSARevokedKeyID,
			kRSAExpiredKeyID,
			kDSADisabledKeyID,
			kDSAKeyIconID,
			kDSAKeyPairIconID,
			kRevokedSigID,
			kRSADisabledKeyID,
			kBadSignatureID,
			kDSARevokedKeyID,
			kDSAExpiredKeyID,
			kValidKeyIconID,
			kKeyAttrOffIconID,
			kMRKActiveIconID,
			kAxiomaticIconID,
			kDomainIconID,
			kKeyServerIconID,
			kDHUserIDIconID,
			kExportSignatureIconID,
			kNoExportMetaSigIconID,
			kExportMetaSigIconID,
			kPressedValidityIconID,
			kPressedAxiomaticIconID,
			kDHGroupIconID
		};

enum	{
			kPGPPublicKeyringType	=	'pgPR',
			kPGPPrivateKeyringType	=	'pgRR',
			kPGPRandomSeedType		=	'pgRS'
		};
		

const ResIDT	kSendToServerMENUID		=	133;

