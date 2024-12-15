#ifndef VALIDATE_H
#define VALIDATE_H

bool ValidateAll();
bool TestSettings();

bool ZKValidate();

bool MD5Validate();
bool SHAValidate();
bool HAVALValidate();
bool TigerValidate();
bool RIPEMDValidate();

bool MD5MACValidate();
bool HMACValidate();
bool XMACCValidate();

bool DESValidate();
bool IDEAValidate();
bool SAFERValidate();
bool RC4Validate();
bool RC5Validate();
bool BlowfishValidate();
bool Diamond2Validate();
bool ThreeWayValidate();
bool GOSTValidate();
bool SHARKValidate();
bool SHARK2Validate();
bool SEALValidate();
bool CASTValidate();
bool SquareValidate();

bool BBSValidate();
bool DHValidate();
bool RSAValidate();
bool ElGamalValidate();
bool DSAValidate();
bool LUCValidate();
bool LUCDIFValidate();
bool LUCELGValidate();
bool RabinValidate();
bool BlumGoldwasserValidate();
bool ECPValidate();
bool EC2NValidate();

#endif
