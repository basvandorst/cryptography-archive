#include "config.h"
#if (LICENSE == 3 || LICENSE == 0) /*Full License*/
#include <stdio.h>
#include "pgpLicense.h"

static const char *license = "\
PGP for Personal Privacy, Version 5.0 (Unix)\n\
(Commercial, Executable Version)\n\
Copyright (C) 1990-1997 Pretty Good Privacy, Inc.\n\
All Rights Reserved.\n\
\n\
End User License Agreement for PGP Client Software\n\
\n\
\n\
IMPORTANT-READ CAREFULLY: This PGP End-User License Agreement\n\
(\"Agreement\") is a legal agreement between you (either an individual\n\
or a single entity) and Pretty Good Privacy, Inc. (or \"PGP\") for the\n\
PGP software product identified above, which includes computer software\n\
and may include associated media, printed materials, and \"online\" or\n\
electronic documentation (\"Software Product\"). By installing, copying,\n\
or otherwise using the Software Product, you agree to be bound by the\n\
terms of this Agreement. If you do not agree to the terms of this\n\
Agreement, you may not install or use the Software Product; you may,\n\
however, return it to your place of purchase for a full refund.\n\
\n\
The Software Product is owned by Pretty Good Privacy, Inc. and is\n\
protected by copyright laws and international copyright treaties, as well as\n\
other intellectual property laws and treaties.\n\
\n\
1.      GRANT OF LICENSE.  PGP grants you (the original end-user and any\n\
subsequent transferee of the Software Product as permitted below) a\n\
non-exclusive license to put in use by a person or organization that agrees\n\
to be bound by the terms of this Agreement, one copy of the Software\n\
Product. The Software Product is \"in use\" when it is loaded into the\n\
temporary memory (i.e., RAM) or installed into the permanent memory (e.g.,\n\
hard disk, CD ROM, or other storage device) of a computer or workstation for\n\
the purpose of being accessible in client-mode by one end user.  (If you\n\
wish to use the Software Product on a computer or workstation in\n\
server-mode within a multi-user or networked environment, then you must\n\
obtain a server-mode license from PGP, which will require that you license\n\
one copy of the Software Product for each computer or workstation which is\n\
participating in such an environment and which has access to the Software\n\
Product).\n\
\n\
        Volume Licenses.  If this package is a volume license package (such\n\
as a \"corporate license\" or a \"corporate bundle\"), you may make and use\n\
additional copies of the Software Product up to the number of copies\n\
authorized in this package or in your corporate license agreement, or\n\
otherwise at the time of purchase. If the anticipated number of users of the\n\
Software Product will exceed the number of applicable licenses, then you\n\
must have a reasonable mechanism or process in place to ensure that the\n\
number of persons using the Software Product does not exceed the number of\n\
licenses you have obtained.\n\
\n\
Home Use.  If the Software Product is permanently installed on the hard disk\n\
or other storage device of a computer (other than a network server) and one\n\
person uses that computer more than 80%% of the time it is in use, then that\n\
person may also install and use the Software Product on a portable or home\n\
computer.\n\
\n\
2.      COPYRIGHT. The Software Product is licensed, not sold. All right,\n\
title and interest in the Software Product (including any images,\n\
\"applets,\" photographs, animations, video, audio, music, and text\n\
incorporated into the software), accompanying printed materials, and any\n\
copies you are permitted to make herein, are owned by PGP or its suppliers,\n\
and the Software Product is protected by United States copyright laws and\n\
international treaty provisions.  Therefore, you must treat the Software\n\
Product like any other copyrighted material (e.g., a book or musical\n\
recording) except that you may either (a) make one copy of the Software\n\
Product solely for backup or archival purposes or (b) transfer the\n\
Software Product to a single hard disk, provided you keep the original\n\
solely for backup or archival purposes.  Such copy shall include PGP's\n\
copyright and other proprietary notices.  You may not copy the printed\n\
materials accompanying the Software Product. 
\n\
3.      UPGRADES AND SUPPORT.  If this Software Product is labeled as an\n\
upgrade or trade-up from a prior version of a PGP product that you were\n\
properly licensed to use, PGP grants you the right to put in use either the\n\
current or prior version of the Software Product, and any prior version\n\
license is replaced by this Agreement. Subject to U.S. export control laws\n\
and regulations, PGP may provide you with technical support services\n\
relating to the Software Product according to PGP's standard support\n\
policies and procedures, which may be described in the user manual, in \"on\n\
line\" documentation and/or other materials provided by PGP or posted on\n\
PGP's web site (\"Support Services\"). Any supplemental software code\n\
provided to you as part of the Support Services shall be considered part of\n\
the Software Product and subject to the terms and conditions of this\n\
Agreement. With respect to technical information you provide to PGP as\n\
part of the Support Services, PGP may use such information for its\n\
business purposes, including for product support and development. PGP will\n\
not utilize such technical information in a form that personally\n\
identifies you.\n\
\n\
4.      OTHER RESTRICTIONS.  The original of this Agreement is your proof of\n\
license to exercise the rights granted herein and must be retained by you.\n\
You may not rent or lease the Software Product, but you may transfer your\n\
rights under this Agreement on a permanent basis provided you transfer this\n\
Agreement and all copies of the Software Product, including all accompanying\n\
printed materials, you do not retain any copies of the Software Product or\n\
such materials, and the recipient agrees to be bound by the terms of this\n\
Agreement.  Any transfer of the Software must include the most recent\n\
update and all prior revisions. You may not reverse engineer, decompile,\n\
disassemble or otherwise translate the Software Product, except and only to\n\
the extent that such activity is expressly permitted by applicable law\n\
notwithstanding this limitation. If this Software Product is labeled\n\
\"Evaluation Copy,\" \"Not For Resale,\" \"NFR\" or to any of those\n\
effects, this license only permits use for demonstration, test, or\n\
evaluation purposes.\n\
\n\
5.      DUAL MEDIA SOFTWARE AND MULTIPLE PLATFORM VERSION.  If the package\n\
from which you obtained this Software Product contains more than one medium\n\
(e.g., both 3 1/2\" disks and a CD), you may use only the medium appropriate\n\
to your computer. You may not use the other disk(s) on another computer or\n\
loan, rent, lease, or transfer them to another user except as permitted\n\
under this Agreement or as part of the permanent transfer (as provided\n\
above) of all the Software Product and related materials.  If the CD or\n\
disk(s) on which the Software Product resides contains several copies of the\n\
Software Product, each of which is compatible with a different operating\n\
system or platform architecture (such as Windows95/NT, Macintosh, one or\n\
more versions of Unix, the x86 architecture, or various RISC architectures),\n\
then you may install the Software Product for use with only one of those\n\
architectures and you may not use the other version(s) on another computer\n\
or loan, rent, lease, or transfer them to another user except as permitted\n\
under this Agreement or as part of a permanent transfer (as provided above).\n\
\n\
6.      U.S. GOVERNMENT RESTRICTED RIGHTS LEGEND. The SOFTWARE PRODUCT and\n\
documentation are provided to the U.S. Government with RESTRICTED RIGHTS.\n\
The U.S. Government acknowledges PGP's representation that the Software is\n\
\"commercial computer software\" as that term is defined in 48 C.F.R. 12.212\n\
of the Federal Acquisition Regulations (\"FAR\") and is \"Commercial\n\
Computer Software\" as that term is defined in 48 C.F.R. 227.7014 (a)(i)\n\
of the Department of Defense Federal Acquisition Regulation Supplement\n\
(\"DFARS\").  Use, duplication or disclosure by the U.S. Government is\n\
subject to restrictions set forth in subparagraphs (a) through (d) of\n\
the Commercial Computer-Restricted Rights clause at FAR 52.227-19 when\n\
applicable, or in subparagraph (c)(1)(ii) of the Rights in Technical Data\n\
and Computer Software clause at DFARS 252.227-7013, or at 252.211-7015,\n\
or to this commercial license, as applicable, and in similar clauses in\n\
the NASA FAR Supplement, as applicable. Contractor/manufacturer is\n\
Pretty Good Privacy, Inc. 2121 S. El Camino Real, Suite 902, San Mateo,\n\
CA 94403. \n\
\n\
7.      TERMINATION.  This Agreement will immediately and automatically\n\
terminate without notice if you fail to comply with any term or condition of\n\
this Agreement.  You agree upon termination to promptly destroy the Software\n\
Product together with all of its component parts and all copies,\n\
modifications and merged portions in any form.\n\
\n\
8.      LIMITED WARRANTY.  PGP warrants that the Software Product will\n\
perform substantially in accordance with the written materials in this\n\
package for a period of 60 days from the date of original purchase.  PGP's\n\
entire liability and your exclusive remedy shall be, at PGP's option, either\n\
(a) return of the purchase price paid for the license or (b) repair or\n\
replacement of the Software Product that does not meet PGP's limited\n\
warranty and which is returned at your expense to PGP with a copy of your\n\
receipt.  This limited warranty is void if failure of the Software Product\n\
has resulted from accident, abuse, or misapplication.  Any repaired or\n\
replacement Software Product will be warranted for the remainder of the\n\
original warranty period or 30 days, whichever is longer.  IF THE SOFTWARE\n\
PRODUCT IS EXPORT CONTROLLED (SEE BELOW), THESE REMEDIES MAY NOT BE\n\
AVAILABLE OUTSIDE THE UNITED STATES OF AMERICA.\n\
\n\
9.      NO OTHER WARRANTIES.  EXCEPT FOR THE WARRANTIES SET FORTH HEREIN,\n\
THE SOFTWARE AND DOCUMENTATION ARE PROVIDED \"AS IS\" AND PGP DISCLAIMS ALL\n\
OTHER WARRANTIES, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO\n\
IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE,\n\
CONFORMANCE WITH DESCRIPTION, AND NON-INFRINGEMENT OF THIRD PARTY RIGHTS.\n\
THIS LIMITED WARRANTY GIVES YOU SPECIFIC LEGAL RIGHTS.  YOU MAY HAVE\n\
OTHERS, WHICH VARY FROM JURISDICTION TO JURISDICTION.\n\
\n\
10.     LIMITATION OF LIABILITY.  PGP'S CUMULATIVE LIABILITY TO YOU OR ANY\n\
OTHER PARTY FOR ANY LOSS OR DAMAGES RESULTING FROM ANY CLAIMS, DEMANDS OR\n\
ACTIONS ARISING OUT OF OR RELATING TO THIS AGREEMENT SHALL NOT EXCEED THE\n\
PURCHASE PRICE PAID FOR THE LICENSE.  IN NO EVENT SHALL PGP OR ITS\n\
SUPPLIERS BE LIABLE FOR ANY INDIRECT, INCIDENTAL, CONSEQUENTIAL, SPECIAL OR\n\
EXEMPLARY DAMAGES OR LOST PROFITS WHATSOEVER (INCLUDING, WITHOUT LIMITATION,\n\
DAMAGES FOR LOSS OF BUSINESS PROFITS, BUSINESS INTERRUPTION, LOSS OF\n\
BUSINESS INFORMATION, OR OTHER PECUNIARY LOSS) ARISING OUT OF THE USE OR\n\
INABILITY TO USE THE SOFTWARE, EVEN IF PGP HAS BEEN ADVISED OF THE\n\
POSSIBILITY OF SUCH DAMAGES.  BECAUSE SOME STATES OR COUNTRIES DO NOT ALLOW\n\
THE EXCLUSION OR LIMITATION OF LIABILITY FOR CONSEQUENTIAL OR INCIDENTAL\n\
DAMAGES, THE ABOVE LIMITATION MAY NOT APPLY TO YOU.\n\
\n\
11.     EXPORT LAW. Export of the Software Product may be subject to\n\
compliance with the rules and regulations promulgated regulations from time\n\
to time by the Bureau of Export Administration, United States Department of\n\
Commerce, which restrict the export and re-export of certain products and\n\
technical data. If the export of the Software Product is controlled under\n\
such rules and regulations, then the Software shall not be exported or\n\
reexported, directly or indirectly, (a) without all export or reexport\n\
licenses and governmental approvals required by any applicable laws, or (b)\n\
in violation of any applicable prohibition against the export or reexport of\n\
any part of the Software.\n\
\n\
12.     GENERAL. This Agreement shall be construed and enforced in\n\
accordance with the laws of the State of California, United States of\n\
America. Any action or proceeding brought by anyone arising out of or\n\
related to this Agreement shall be brought only in a state or federal court\n\
of competent jurisdiction located in the county of San Francisco,\n\
California, and the parties hereby consent to the jurisdiction and venue of\n\
said courts. Should any term of this Agreement be declared void or\n\
unenforceable by any court of competent jurisdiction, such declaration shall\n\
have no effect on the remaining terms hereof. This Agreement is in the\n\
English language, and only the English language version hereof, regardless\n\
of the existence of other language translations of this Agreement, shall be\n\
controlling in all respects. The failure of either party to enforce any\n\
rights granted hereunder or to take action against the other party in the\n\
event of any breach hereunder shall not be deemed a waiver by that party as\n\
to subsequent enforcement of rights or subsequent actions in the event of\n\
future breaches. PGP reserves the right at any time without liability or\n\
prior notice to change the features or characteristics of this Software\n\
Product, or its documentation and related materials, or future versions\n\
thereof. This Agreement may not be modified, amended, canceled or in any way\n\
altered, nor may they be modified by custom and usage of trade or course of\n\
dealing, except by an instrument in writing and signed by a duly authorized\n\
officer of PGP. This Agreement constitutes the complete and exclusive\n\
statement of the agreement between us which supersedes any proposal or prior\n\
agreement, oral or written, and any other communication between us relating\n\
to the subject matter of these terms and conditions. YOU ACKNOWLEDGE THAT\n\
YOU HAVE READ THIS AGREEMENT, UNDERSTAND IT AND AGREE TO BE BOUND BY ALL OF\n\
ITS TERMS AND CONDITIONS.\n\
\n\
Should you have any questions concerning this Agreement, or if you desire to\n\
contact Pretty Good Privacy, Inc. for any reason, please write: Pretty Good\n\
Privacy, Inc. Customer Service, 2121 S. El Camino Real, Suite 902, San\n\
Mateo, CA 94403. http://www.pgp.com.  Copyright (C) 1990-1997 Pretty Good\n\
Privacy, Inc. All rights reserved. Pretty Good Privacy, Pretty Good, and PGP\n\
are registered trademarks of Pretty Good Privacy, Inc.  The Software\n\
Product uses public key algorithms described in U.S. patent numbers\n\
4,200,770, 4,218,582, 4,405,829, and 4,424,414, licensed exclusively\n\
by Public Key Partners; the IDEA (TM) cryptographic cipher described\n\
in U.S. patent number 5,214,703, licensed from Ascom Tech AG; and the\n\
Northern Telecom Ltd., CAST Encryption Algorithm, licensed from\n\
Northern Telecom, Ltd.  IDEA is a trademark of Ascom Tech AG.  Note:\n\
Some countries have laws and regulations regarding the use and export\n\
of cryptography products; please consult your local government\n\
authority for details.\n\
\n";


void display_license(void)
{
    fprintf(stderr, license);
}

#endif
