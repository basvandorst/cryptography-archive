From raylau@MIT.EDU Tue Apr 13 18:41:09 1993
Received: from MIT.EDU (MIT.MIT.EDU) by scss3.cl.msu.edu (4.1/4.7)  id AA03388; Tue, 13 Apr 93 18:41:08 EDT
Received: from TMS-E40-PORT-7.MIT.EDU by MIT.EDU with SMTP
	id AA26649; Tue, 13 Apr 93 18:40:39 EDT
Message-Id: <9304132240.AA26649@MIT.EDU>
Date: Tue, 13 Apr 93 18:36:02 EST
From: raylau@MIT.EDU (Raymond Lau)
To: 71755.204@compuserve.com, somogyi@macuser.ziff.com, mrr@scss3.cl.msu.edu
Subject: [jefft@RSA.COM (Jeff Thompson): Low Assurance public key]
Status: ORS

I don't know if this will do any of you any good but this is RSA Low Assurance PCA's public 
key...  DER coded of course.  I guess we don't have a certificate for it bec. the one who will 
sign it, namely the IPRA, doesn't yet exist.

 -Ray

-----BEGIN PRIVACY-ENHANCED MESSAGE-----
Proc-Type: 4,MIC-CLEAR
Content-Domain: RFC822
Originator-Certificate:
 MIIBoTCCAT0CARMwDQYJKoZIhvcNAQECBQAwTTELMAkGA1UEBhMCVVMxIDAeBgNV
 BAoTF1JTQSBEYXRhIFNlY3VyaXR5LCBJbmMuMRwwGgYDVQQLExNQZXJzb25hIENl
 cnRpZmljYXRlMB4XDTkzMDQxMzE2MzI1NVoXDTk1MDQxMzE2MzI1NVowXTELMAkG
 A1UEBhMCVVMxIDAeBgNVBAoTF1JTQSBEYXRhIFNlY3VyaXR5LCBJbmMuMRwwGgYD
 VQQLExNQZXJzb25hIENlcnRpZmljYXRlMQ4wDAYDVQQDEwVKZWZmVDBbMA0GCSqG
 SIb3DQEBAQUAA0oAMEcCQAt3kFy6tvoepaa9i2aLcd+dOhBPM0CU/X+htM3QzuzC
 obmhMQDqb27FyXELnCRxpQ2fBtOwlORzTzpDrPbvu9MCAwEAATANBgkqhkiG9w0B
 AQIFAANPAAML7UZsy+mKS5uNXtUC0cyqDmY3mRAU5XvlKY/LtlrlMPhjpM0HhgJL
 BfDv21hC2oRnBVw/dRiasaaHVTlGbgeXaGesNZtDCQBOcmXi1Q==
Issuer-Certificate:
 MIIBxjCCAVACBF4AAAMwDQYJKoZIhvcNAQECBQAwXzELMAkGA1UEBhMCVVMxIDAe
 BgNVBAoTF1JTQSBEYXRhIFNlY3VyaXR5LCBJbmMuMS4wLAYDVQQLEyVMb3cgQXNz
 dXJhbmNlIENlcnRpZmljYXRpb24gQXV0aG9yaXR5MB4XDTkyMTIyMzA4MDAwMFoX
 DTk0MDEwMTA3NTk1OVowTTELMAkGA1UEBhMCVVMxIDAeBgNVBAoTF1JTQSBEYXRh
 IFNlY3VyaXR5LCBJbmMuMRwwGgYDVQQLExNQZXJzb25hIENlcnRpZmljYXRlMGkw
 DQYJKoZIhvcNAQEBBQADWAAwVQJOBqoIUA2vV4v9swHWBKiVSHGIZSzdRaSPbV0N
 Zus2d/T2FyvFIaI9BLO5Fkb/IQtOL6pDisJ3Vm81bb6B0Dpj/JzpJLgYvgEL4BaE
 XIDlAgMBAAEwDQYJKoZIhvcNAQECBQADYQCPF1HZAPzWWKSyspFjbUGkmQAWGLtz
 3Zvl1nn3EztPPVtR6GirTpFRa07ov7isHWEdZxGKIwbmFPJuh8pn8tTrSyYfWfD6
 /CHEa04fhZ4jVoAmKmjdgbRTqfraABsBAC8=
MIC-Info: RSA-MD5,RSA,
 AtzMc+WGuSH56bq21eCA5e5SGV4EkjqQ+6FIDNLFSXml9qvSxV7i1RK9CLNGPtNB
 P7rU8Vi76t0t3Mv/o5pj0w==

Ray,

Here is the Low Assurance name and public key in C source form.

- - Jeff

/* Copyright (C) RSA Data Security, Inc. created 1993.

   This file is used to demonstrate how to interface to an
   RSA Data Security, Inc. licensed development product.

   You have a royalty-free right to use, modify, reproduce and
   distribute this demonstration file (including any modified
   version), provided that you agree that RSA Data Security,
   Inc. has no warranty, implied or otherwise, or liability
   for this demonstration file or any modified version.
 */

static unsigned char LOW_ASSURANCE_NAME[] = {
  0x30, 0x5f, 0x31, 0x0b, 0x30, 0x09, 0x06, 0x03, 0x55, 0x04, 0x06, 0x13,
  0x02, 0x55, 0x53, 0x31, 0x20, 0x30, 0x1e, 0x06, 0x03, 0x55, 0x04, 0x0a,
  0x13, 0x17, 0x52, 0x53, 0x41, 0x20, 0x44, 0x61, 0x74, 0x61, 0x20, 0x53,
  0x65, 0x63, 0x75, 0x72, 0x69, 0x74, 0x79, 0x2c, 0x20, 0x49, 0x6e, 0x63,
  0x2e, 0x31, 0x2e, 0x30, 0x2c, 0x06, 0x03, 0x55, 0x04, 0x0b, 0x13, 0x25,
  0x4c, 0x6f, 0x77, 0x20, 0x41, 0x73, 0x73, 0x75, 0x72, 0x61, 0x6e, 0x63,
  0x65, 0x20, 0x43, 0x65, 0x72, 0x74, 0x69, 0x66, 0x69, 0x63, 0x61, 0x74,
  0x69, 0x6f, 0x6e, 0x20, 0x41, 0x75, 0x74, 0x68, 0x6f, 0x72, 0x69, 0x74,
  0x79
};

static unsigned char LOW_ASSURANCE_PUBLIC_KEY[] = {
  0x30, 0x7c, 0x30, 0x0d, 0x06, 0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d,
  0x01, 0x01, 0x01, 0x05, 0x00, 0x03, 0x6b, 0x00, 0x30, 0x68, 0x02, 0x61,
  0x00, 0xb0, 0xb4, 0x0e, 0x9a, 0x3a, 0x46, 0x4e, 0x87, 0x03, 0xff, 0xb8,
  0xdb, 0xca, 0xd8, 0xaf, 0x41, 0xf3, 0xc3, 0xf4, 0x13, 0x1c, 0xf6, 0x57,
  0x1e, 0x39, 0xa5, 0x35, 0x49, 0xb4, 0x20, 0x94, 0xdc, 0x92, 0xf8, 0xee,
  0x1e, 0xa1, 0x03, 0x5f, 0x94, 0x21, 0x2b, 0x75, 0x1a, 0x3b, 0x07, 0x44,
  0x5d, 0xbd, 0xd6, 0xef, 0x4d, 0x7e, 0x23, 0x00, 0xf4, 0x2c, 0xf5, 0x73,
  0x47, 0x90, 0xbc, 0xe8, 0xba, 0x51, 0x7f, 0xa8, 0x19, 0xee, 0xf7, 0x5f,
  0x17, 0x46, 0xdc, 0xe5, 0xff, 0xd1, 0x23, 0xfe, 0x64, 0x2e, 0x68, 0x94,
  0xb3, 0x81, 0xde, 0x5a, 0x40, 0x28, 0x8d, 0xd6, 0xd7, 0x14, 0xaf, 0x84,
  0xef, 0x02, 0x03, 0x01, 0x00, 0x01
};

-----END PRIVACY-ENHANCED MESSAGE-----


