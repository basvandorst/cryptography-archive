#include "config.h"
#if (LICENSE == 1)
#include <stdio.h>
#include "pgpLicense.h"

static const char *license = "\
PGP Freeware (Unix), Version 5.0\n\
(Source Code Version)\n\
Copyright (C) 1990-1997 Pretty Good Privacy, Inc.  All Rights Reserved.\n\
\n\
For Non-Commercial Distribution and Use Only\n\
Terms and Conditions\n\
This special freeware version of the source code (\"Source Code Package\")\n\
of PGP (Unix) 5.0 (\"Software Product\"), including the software, its source\n\
code, and its related user and development documentation, is owned by\n\
Pretty Good Privacy, Inc. (or \"PGP\") and is protected by copyright laws\n\
and international copyright treaties, as well as other intellectual property\n\
laws and treaties.\n\
\n\
The makers of PGP, however, are committed to making freely available to\n\
individuals around the world a trusted means to secure their personal\n\
information and communications from unwanted invasions of privacy. In\n\
keeping with the traditions of PGP, the source code of PGP 5.0 has been\n\
published for the purpose of facilitating peer review. The review of the\n\
source code by independent cryptographers, unrelated to PGP, assures that\n\
the cryptography employed in the PGP software may be trusted -- i.e., it is\n\
truly strong and that the software contains no hidden back doors allowing\n\
third party access to encrypted messages. In addition, it is important that\n\
the Software Product be available in as many languages, operate on as many\n\
operating system and hardware platforms, and be as solid and secure as\n\
possible.\n\
\n\
To facilitate these important goals, the makers of PGP have agreed, subject\n\
to all of the terms and conditions which follow, that it will permit certain\n\
uses of the Source Code Package by users who wish to freely contribute to\n\
the efforts of PGP, as specifically described below. Because export of this\n\
Source Code Package and Software Product outside of the United States may be\n\
restricted by the U.S. government, the rights granted in this license are\n\
restricted to the use of the Source Code Package and Software Product in\n\
the United States by residents of the United States. If you are located\n\
outside of the United States, you must use only code or software that was\n\
obtained directly from a printed book or other printed materials published\n\
by Pretty Good Privacy, Inc. (except that PGP expressly forbids any use of\n\
PGP software in any countries embargoed by the U.S. government (currently\n\
including, Cuba, Iran, Iraq, Libya, North Korea, Syria and Sudan)).\n\
\n\
1.      Non-Commercial Licenses\n\
\n\
a.      Non-Commercial Use.  You may use this Source Code Package solely for\n\
the non-commercial purposes permitted below. The term \"non-commercial\"\n\
means that the package (a) has been distributed or otherwise made available\n\
at no charge (direct or indirect) and (b) is not used for any commercial\n\
purpose, which includes, but is not limited to, any activity engaged for\n\
the purpose of generating revenues (directly or indirectly).  For example,\n\
a commercial purpose includes the use of the package within a commercial\n\
business or facility or the use of the package to provide a service, or in\n\
support of service, for which you charge. Commercial purpose also includes\n\
use by any government agency or organization. Examples of non-commercial\n\
purposes include use at home for personal correspondence, use by students\n\
for academic activities, or use by human rights organizations. The software\n\
is in \"use\" when it is loaded into the temporary memory (i.e., RAM) or\n\
installed into the permanent memory (e.g., hard disk, CD ROM, or other\n\
storage device) of a computer for the purpose of being accessible in\n\
client-mode by an end user.\n\
\n\
b.      Non-Commercial Distribution.  You may make exact, unmodified copies\n\
of this Source Code Package and distribute such copies solely by electronic\n\
means (for example, posting on Internet sites for others to download),\n\
subject to the following (a) that such distribution is solely for\n\
non-commercial purposes (see above definition), (b) that the Source Code\n\
Package is distributed unmodified and in its entirety (with its complete\n\
user documentation, its readme files, its copyright, trademark, other\n\
intellectual property notices, including these terms and conditions, and all\n\
of its other components), (c) that such distribution is limited to residents\n\
of the United States located in the United States, and (d) with respect to\n\
any such distribution, you take adequate precautions necessary to comply\n\
with any and all U.S. export control laws and regulations.\n\
\n\
c.      User Involvement. Subject to all of the conditions set forth\n\
in this section, you may use this Source Code Package solely for the\n\
following non-commercial purposes: (a) to review and compile the source code\n\
contained in the Source Code Package for the purpose of determining the\n\
strength and trustworthiness of the Software Product, reporting any problems\n\
you discover to PGP (or its designee) (b) to write bug fixes\n\
(\"Corrections\"), (c) to otherwise modify or enhance the source code\n\
(\"Enhancements\"), (d) to port the Software Product or its source code to\n\
operate on other operating system or hardware platforms (\"Port\"), (e) to\n\
write translations or localizations of the Software Product or its source\n\
code (\"Translations\"), and (f) to use internally and solely for\n\
non-commercial purposes (as defined above) a compiled version of the source\n\
code, including any Corrections, Enhancements, Ports, or Translations\n\
(collectively, \"Derivative Works\") you create, provided that you do NOT\n\
reproduce, electronically post, transmit, or otherwise distribute to anyone\n\
else any such Derivative Works (see reasons at the end of this paragraph).\n\
However, you may submit to PGP (or its designee) any Derivative Works that\n\
you create, but in doing so (a) you agree that any such submission of\n\
Derivative Works is a free contribution to PGP and that PGP is free to use,\n\
reproduce, modify, distribute, transmit, display, perform, prepare\n\
derivative works thereof, or otherwise make use of or exploit such\n\
Derivative Works, in any way and by any means now known or hereafter devised\n\
for as long as PGP wishes, and (b) you agree not to assert any intellectual\n\
property rights, including droit morale, you may have with respect to such\n\
Derivative Works. Because such Derivative Works could have an impact upon\n\
the cryptographic integrity and strict quality control of PGP, affect\n\
compliance with PGP's cryptographic algorithms and protocols, or could\n\
create interoperability issues, compatibility problems and user confusion,\n\
PGP will be under no obligation to use, distribute or otherwise exploit any\n\
such Derivative Works. For these same reasons, permission to allow you or\n\
others to reproduce and distribute any such Derivative Works or any\n\
compiled version of the source code of this Source Code Package\n\
(i.e., anything except the unmodified version of the Source Code Package),\n\
will generally NOT be given. Most important, PGP expressly forbids the\n\
creation of any Derivative Works that may compromise the privacy or\n\
security of the users of the Software Product or the Source Code Package.\n\
\n\
2.      Other Restrictions.  If the Source Code Package or Software Product\n\
was obtained under a license from MIT, you agree to fully comply with the\n\
terms of the MIT compilation license (which relates to MIT's claims to\n\
copyright in the software distribution package compilation, but not in the\n\
PGP software itself) and with the terms of the RSA Laboratories Program\n\
License Agreement for the RSAREF Program, if included in the Source Code\n\
Package or the Software Product.\n\
\n\
3.      Other Uses. All right, title and interest in this Source Code\n\
Package and the Software Product, including its accompanying documentation,\n\
and all copyrights therein and related trademarks, trade names and other\n\
intellectual property rights, are owned by Pretty Good Privacy, Inc. or its\n\
suppliers. All license rights not granted herein are reserved by PGP. Should\n\
you wish to use or distribute the Source Code Package or Software Product\n\
for commercial purposes or any other purpose not permitted under these terms\n\
and conditions, please refer to the PGP web site located at\n\
http://www.pgp.com for further information.\n\
\n\
4.      Technical Support. The Source Code Package and Software Product are\n\
user-supported, meaning that assistance in its installation and use may be\n\
limited to help available from friends, family, and other users on the\n\
Internet. Though PGP does not offer technical support for this Source Code\n\
Package or Software Product, we welcome your feedback. Technical support is\n\
available from PGP or its resellers for those who have purchased copies of\n\
the commercial version of the Software Product. For further information,\n\
please refer to http://www.pgp.com.\n\
\n\
5.      No Warranty. THIS SOURCE CODE PACKAGE IS PROVIDED FOR NON-COMMERCIAL\n\
USE AND DISTRIBUTION ONLY, COMES WITH NO WARRANTY, AND IS PROVIDED ON AN\n\
\"AS IS\" BASIS.  YOU ASSUME THE ENTIRE COST OF ANY DAMAGE RESULTING FROM\n
THE INFORMATION CONTAINED IN OR PRODUCED BY THIS SOURCE CODE PACKAGE. YOU\n\
ASSUME ALL RESPONSIBILITIES FOR SELECTION OF THIS SOURCE CODE PACKAGE TO\n\
ACHIEVE YOUR INTENDED RESULTS, AND FOR THE INSTALLATION OF, USE OF, AND\n\
RESULTS OBTAINED FROM IT. TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE\n\
LAW, PGP, ITS SUPPLIERS AND OTHERS WHO MAY DISTRIBUTE THIS SOURCE CODE\n\
PACKAGE DISCLAIM ALL WARRANTIES, EITHER EXPRESS OR IMPLIED, INCLUDING BUT\n\
NOT LIMITED TO IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A\n\
PARTICULAR PURPOSE, CONFORMANCE WITH DESCRIPTION, AND NON-INFRINGEMENT,\n\
WITH RESPECT TO THIS SOURCE CODE PACKAGE.\n\
\n\
6.      Limitation of Liability. THE CUMULATIVE LIABILITY OF PGP, ITS\n\
SUPPLIERS OR OTHERS WHO MAY DISTRIBUTE THIS SOURCE CODE PACKAGE, TO YOU OR\n\
ANY OTHER PARTY FOR ANY LOSS OR DAMAGES RESULTING FROM ANY CLAIMS, DEMANDS\n\
OR ACTIONS ARISING OUT OF OR RELATING TO THESE TERMS AND CONDITIONS SHALL\n\
NOT EXCEED ONE U.S. DOLLAR.  IN NO EVENT SHALL PGP, ITS SUPPLIERS OR OTHERS\n\
WHO MAY DISTRIBUTE THIS SOURCE CODE PACKAGE BE LIABLE FOR ANY INDIRECT,\n\
INCIDENTAL, CONSEQUENTIAL, SPECIAL OR EXEMPLARY DAMAGES OR LOST PROFITS\n\
WHATSOEVER (INCLUDING, WITHOUT LIMITATION, DAMAGES FOR LOSS OF BUSINESS\n\
PROFITS, BUSINESS INTERRUPTION, LOSS OF BUSINESS INFORMATION, OR OTHER\n\
PECUNIARY LOSS) ARISING OUT OF THE USE OR INABILITY TO USE THIS SOURCE CODE\n\
PACKAGE, EVEN IF PGP, ITS SUPPLIERS OR OTHERS WHO MAY DISTRIBUTE THIS SOURCE\n\
CODE PACKAGE HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.\n\
\n\
BECAUSE SOME JURISDICTIONS DO NOT ALLOW THE LIMITATIONS ON IMPLIED\n\
WARRANTIES OR THE EXCLUSION OR LIMITATION OF LIABILITY FOR CONSEQUENTIAL OR\n\
INCIDENTAL DAMAGES, THE ABOVE LIMITATIONS MAY NOT APPLY TO YOU.\n\
\n\
7.      General.  These terms and conditions may not be modified, amended,\n\
canceled or in any way altered, nor may they be modified by custom and usage\n\
of trade or course of dealing, except by an instrument in writing and signed\n\
by a duly authorized officer of PGP. These terms and conditions shall be\n\
construed and enforced in accordance with the laws of the State of\n\
California, United States of America. Any action or proceeding brought by\n\
anyone arising out of or related to these terms and conditions shall be\n\
brought only in a state or federal court of competent jurisdiction located\n\
in the county of San Francisco, California, and the parties hereby consent\n\
to the jurisdiction and venue of said courts.  Should any term of these\n\
terms and conditions be declared void or unenforceable by any court of\n\
competent jurisdiction, such declaration shall have no effect on the\n\
remaining terms hereof. These terms and conditions are in the English\n\
language, and only the English language version hereof, regardless of the\n\
existence of other language translations of these terms and conditions,\n\
shall be controlling in all respects. The failure of either party to enforce\n\
any rights granted hereunder or to take action against the other party in\n\
the event of any breach hereunder shall not be deemed a waiver by that\n\
party as to subsequent enforcement of rights or subsequent actions in the\n\
event of future breaches. PGP reserves the right at any time without\n\
liability or prior notice to change the features or characteristics of\n\
this Software Product, or its documentation and related materials, or\n\
future versions thereof. These terms and conditions constitute the complete\n\
and exclusive statement of the agreement between us which supersedes any\n\
proposal or prior agreement, oral or written, and any other communication\n\
between us relating to the subject matter of these terms and conditions.\n\
Should you have any questions concerning these terms and conditions, or if\n\
you desire to contact Pretty Good Privacy, Inc. for any reason, please\n\
write: Pretty Good Privacy, Inc. Customer Service, 2121 S. El Camino Real,\n\
Suite 902, San Mateo, CA 94403. http://www.pgp.com.  Copyright\n\
(C) 1990-1997 Pretty Good Privacy, Inc. All rights reserved. Pretty\n\
Good Privacy, Pretty Good, and PGP are registered trademarks of Pretty\n\
Good Privacy, Inc.\n\
\n\
The Software Product uses the IDEA (TM) cryptographic cipher described\n\
in U.S. patent number 5,214,703, licensed from Ascom Tech AG; and the\n\
Northern Telecom Ltd., CAST Encryption Algorithm, licensed from\n\
Northern Telecom, Ltd.  IDEA is a trademark of Ascom Tech AG.  Note:\n\
Some countries have laws and regulations regarding the use and export\n\
of cryptography products; please consult your local government\n\
authority for details.\n\
\n\
Uses the RSAREF(tm) Toolkit, which is copyright RSA Data Security, Inc.\n\
Distributed by the Massachusetts Institute of Technology.\n\
\n";

void display_license(void)
{
    fprintf(stderr, license);
}
#endif
