/* ======================================================================

   Basic RFC-1847 Security-Multipart implementation
                 
   Last Edited: Apr 18, 1996

   Laurence Lundblade <lgl@qualcomm.com>

   QUALCOMM Inc. proprietary and confidential
   Copyright 1995, 1996 QUALCOMM Inc.
   
*/

OSErr    createMultipartSigned(Str255  sigType,
                               Str255  micalg, 
                               FSSpec *signedData,
                               FSSpec *signature,
                               FSSpec *mimeOutput,
                               Str255  MimeBoundary);
                      
OSErr           ParseMultipart(FSSpec    *inFsp,
                               FSSpec    *p1Fsp,
                               FSSpec    *p2Fsp,
                               StringPtr *boundaryHandle);                      
                     