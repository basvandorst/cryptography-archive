#include "config.h"
#if (LICENSE == 2) /*Beta License*/
#include <stdio.h>
#include "pgpLicense.h"

static const char *license = "\
U.S. Patent numbers 4,200,770, 4,218,582, 4,405,829, and 4,424,414\n\
licensed exlusively by Public Key Partners.\n\
U.S. Patent number 5,214,703 licensed by Ascom Tech A.G.\n\
\n\
Copyright (C) 1997 Pretty Good Privacy, Inc.  All rights reserved.\n\
PGP and Pretty Good Privacy are trademarks of Pretty Good Privacy,\n\
Inc.  All other trademarks are property of their respective holders.\n\
Export of this software may be restricted by the U.S. Government.\n\
\n\
PRETTY GOOD PRIVACY, INC. SOFTWARE FIELD TEST LICENSE AGREEMENT\n\
\n\
\n\
	This is a license agreement and not an agreement for sale.\n\
Pretty Good Privacy, Inc. hereby licenses this version of the Pretty\n\
Good Privacy, Inc. software identified above (the \"Software\") to the\n\
original end-user and any subsequent transferee of the Software SOLELY\n\
FOR TESTING PURPOSES and for use only on the terms set forth in this\n\
Agreement. THIS TEST SOFTWARE CONTAINS A TIME-OUT FEATURE THAT\n\
DISABLES ITS OPERATION AFTER A CERTAIN PERIOD OF TIME. THE WEB PAGE\n\
FROM WHICH THE SOFTWARE WAS DOWNLOADED OR A TEXT FILE DELIVERED WITH\n\
THE SOFTWARE WILL STATE THE SPECIFIC DATE ON WHICH THE SOFTWARE WILL\n\
EXPIRE. Further, though PGP does not offer technical support for the\n\
this test Software, we welcome your feedback.\n\
\n\
1.	GRANT OF LICENSE.  Pretty Good Privacy, Inc. grants you a\n\
non-exclusive license to put in use solely for testing and evaluation\n\
by a person or organization that agrees to be bound by the terms of\n\
this Agreement, the number of copies of the Software specified above\n\
provided that (1) if the Software is in use in single-user\n\
environments, each copy of the Software is in use on only one computer\n\
at any time or (2) if the Software is in use on computers or computer\n\
terminals in multi-user or networked environments, you have licensed\n\
one copy of the Software for each computer or computer terminal which\n\
is participating in such an environment and which has access to the\n\
Software.  The Software is \"in use\" on a computer when it is loaded\n\
into the temporary memory (i.e., RAM) or installed into the permanent\n\
memory (e.g., hard disc, CD-ROM, or other storage device) of that\n\
computer, except that a copy installed on a network server for the\n\
sole purpose of distribution to other computers is not \"in use.\"  If\n\
this Software is an upgrade or trade-up from a previous version of a\n\
Pretty Good Privacy, Inc. product, Pretty Good Privacy, Inc. grants\n\
you the right to put in use either the current or prior version of the\n\
Software in the quantity indicated above, and any prior version\n\
license is replaced by this Agreement.\n\
\n\
This Software is still under development and may contain bugs. Though\n\
Pretty Good Privacy, Inc. does not offer technical support for the\n\
Software, we welcome your feedback.  While Pretty Good Privacy,\n\
Inc. intends to distribute a commercial release of the Software,\n\
Pretty Good Privacy, Inc. reserves the right at any time not to\n\
release a commercial release of the Software or, if it does so, to\n\
alter prices, features, specifications, capabilities, functions,\n\
licensing terms, release dates, general availability, or other\n\
characteristics of the commercial release.\n\
\n\
2.	COPYRIGHT.  All right, title and interest in the Software, the\n\
accompanying documentation (the \"Documentation\") and related\n\
trademarks, trade names and other intellectual property rights are\n\
owned by Pretty Good Privacy, Inc. or its suppliers and the Software\n\
and Documentation are protected by United States copyright laws and\n\
international treaty provisions.  Therefore, you must treat the\n\
Software and Documentation like any other copyrighted material (e.g.,\n\
a book or musical recording) except that in addition to the number of\n\
copies you are permitted to put in use, you may make one copy of the\n\
Software solely for backup or archival purposes.  Such copy shall\n\
include Pretty Good Privacy, Inc.'s copyright and other proprietary\n\
notices.  You may not copy the Documentation.\n\
\n\
3.	OTHER RESTRICTIONS.  The original of this Agreement is your\n\
proof of license to exercise the rights granted herein and must be\n\
retained by you.  You may not rent or lease the Software, but you may\n\
transfer your rights under this Agreement on a permanent basis\n\
provided you transfer this Agreement and all copies of the Software\n\
and Documentation, you do not retain any copies of the Software or\n\
Documentation, and the recipient agrees to the terms of this\n\
Agreement.  Any transfer of the Software must include the most recent\n\
update and all prior revisions.  You may not reverse engineer,\n\
decompile, disassemble or otherwise translate the Software.\n\
\n\
4.	DUAL MEDIA SOFTWARE.  If the Software package contains both 3\n\
1/2\" and 5 1/4\" disks, and/or a CD, you may use only the disks\n\
appropriate to your computer. You may not use the other disks on\n\
another computer or loan, rent, lease, or transfer them to another\n\
user except as permitted under this Agreement or as part of the\n\
permanent transfer (as provided above) of all Software and\n\
Documentation.\n\
\n\
5.	GOVERNMENT LICENSEE.  If you are acquiring the Software on\n\
behalf of the United States Government, the following provisions\n\
apply:  The Government acknowledges Pretty Good Privacy, Inc.'s\n\
representation that the Software and Documentation were developed at\n\
private expense and no part of them is in public domain.  The\n\
Government also acknowledges Pretty Good Privacy, Inc.'s\n\
representation that the Software is \"commercial computer software\"\n\
as that term is defined in 48 C.F.R. 12.212 of the Federal Acquisition\n\
Regulations (\"FAR\") and is \"Commercial Computer Software\" as that\n\
term is defined in 48 C.F.R. 227.7014 (a)(i) of the Department of\n\
Defense Federal Acquisition Regulation Supplement (\"DFARS\").  The\n\
Government agrees that: if the Software is supplied to any unit or\n\
agency of the United States Government, the Government's rights in the\n\
Software and its documentation will be only as specified in this\n\
Agreement.\n\
	Restricted Rights Legend\n\
\n\
	The Contractor/Manufacturer is:  Pretty Good Privacy, Inc.,\n\
2121 S. El Camino Real, Suite 902, San Mateo, CA 94403.\n\
\n\
6.	TERMINATION.  This Agreement will immediately and\n\
automatically terminate without notice if you fail to comply with any\n\
term or condition of this Agreement.  You agree upon termination to\n\
promptly destroy all of the Software and Documentation in your\n\
possession together with all copies, modifications and merged portions\n\
in any form.\n\
\n\
7.	NO WARRANTY FOR TEST SOFTWARE. THE SOFTWARE IS PROVIDED FOR\n\
TESTING AND EVALUATION PURPOSES ONLY, COMES WITH NO WARRANTY, AND IS\n\
PROVIDED ON AN \"AS IS\" BASIS.  YOU ASSUME THE ENTIRE COST OF ANY\n\
DAMAGE RESULTING FROM THE INFORMATION CONTAINED IN OR PRODUCED BY THE\n\
SOFTWARE. YOU ASSUME ALL RESPONSIBILITIES FOR SELECTION OF THE\n\
SOFTWARE TO ACHIEVE YOUR INTENDED RESULTS, AND FOR THE INSTALLATION\n\
OF, USE OF, AND RESULTS OBTAINED FROM THE SOFTWARE.  TO THE MAXIMUM\n\
EXTENT PERMITTED BY APPLICABLE LAW, PGP DISCLAIMS ALL WARRANTIES,\n\
EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO IMPLIED\n\
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, AND\n\
NON-INFRINGEMENT, WITH RESPECT TO THE SOFTWARE. SOME STATES DO NOT\n\
ALLOW LIMITATIONS ON IMPLIED WARRANTIES, SO THE ABOVE LIMITATIONS MAY\n\
NOT APPLY TO YOU.\n\
\n\
8.	LIMITATION OF LIABILITY.  PRETTY GOOD PRIVACY, INC.'S\n\
CUMULATIVE LIABILITY TO YOU OR ANY OTHER PARTY FOR ANY LOSS OR DAMAGES\n\
RESULTING FROM ANY CLAIMS, DEMANDS OR ACTIONS ARISING OUT OF OR\n\
RELATING TO THIS AGREEMENT SHALL NOT EXCEED THE PURCHASE PRICE PAID\n\
FOR THE LICENSE.  IN NO EVENT SHALL PRETTY GOOD PRIVACY, INC. OR ITS\n\
SUPPLIERS BE LIABLE FOR ANY INDIRECT, INCIDENTAL, CONSEQUENTIAL,\n\
SPECIAL OR EXEMPLARY DAMAGES OR LOST PROFITS WHATSOEVER (INCLUDING,\n\
WITHOUT LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS, BUSINESS\n\
INTERRUPTION, LOSS OF BUSINESS INFORMATION, OR OTHER PECUNIARY LOSS)\n\
ARISING OUT OF THE USE OR INABILITY TO USE THE SOFTWARE, EVEN IF\n\
PRETTY GOOD PRIVACY, INC. HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH\n\
DAMAGES.  BECAUSE SOME STATES DO NOT ALLOW THE EXCLUSION OR LIMITATION\n\
OF LIABILITY FOR CONSEQUENTIAL OR INCIDENTAL DAMAGES, THE ABOVE\n\
LIMITATION MAY NOT APPLY TO YOU.\n\
\n\
9.	EXPORT LAW. Export of the Software may be subject to\n\
compliance with the rules and regulations promulgated regulations from\n\
time to time by the Bureau of Export Administration, United States\n\
Department of Commerce, which restrict the export and re-export of\n\
certain products and technical data. If the export of the Software is\n\
controlled under such rules and regulations, then the Software shall\n\
not be exported or reexported, directly or indirectly, (a) without all\n\
export or reexport licenses and governmental approvals required by any\n\
applicable laws, or (b) in violation of any applicable prohibition\n\
against the export or reexport of any part of the Software.\n\
\n\
10.	GENERAL.  This Agreement will be governed by the laws of the\n\
State of California and you consent to the jurisdiction and venue of\n\
the courts of the State of California.  If any action is brought by\n\
either party to this Agreement against the other party regarding the\n\
subject matter hereof, the prevailing party shall be entitled to\n\
recover, in addition to any other relief granted, reasonable attorneys\n\
fees and expenses of litigation.  Should any term of this Agreement be\n\
declared void or unenforceable by any court of competent jurisdiction,\n\
such declaration shall have no effect on the remaining terms hereof.\n\
The failure of either party to enforce any rights granted hereunder or\n\
to take action against the other party in the event of any breach\n\
hereunder shall not be deemed a waiver by that party as to subsequent\n\
enforcement of rights or subsequent actions in the event of future\n\
breaches.  YOU ACKNOWLEDGE THAT YOU HAVE READ THIS AGREEMENT,\n\
UNDERSTAND IT AND AGREE TO BE BOUND BY ITS TERMS AND CONDITIONS.\n\
\n\
	YOU FURTHER AGREE THAT IT IS THE COMPLETE AND EXCLUSIVE\n\
STATEMENT OF THE AGREEMENT BETWEEN US WHICH SUPERSEDES ANY PROPOSAL OR\n\
PRIOR AGREEMENT, ORAL OR WRITTEN, AND ANY OTHER COMMUNICATION BETWEEN\n\
US RELATING TO THE SUBJECT MATTER OF THIS AGREEMENT.  Should you have\n\
any questions concerning this Agreement, or if you desire to contact\n\
Pretty Good Privacy, Inc. for any reason, please write:  Pretty Good\n\
Privacy, Inc. Customer Service, 2121 S. El Camino Real, Suite 902, San\n\
Mateo, CA 94403.\n\
\n\
Copyright (c) 1990-1997 Pretty Good Privacy, Inc. All rights\n\
reserved.\n\
Pretty Good Privacy, Pretty Good, and PGP are registered trademarks\n\
of\n\
\n\
Pretty Good Privacy, Inc.\n\
	581-00030C\n";

void display_license(void)
{
    fprintf(stderr, license);
}
#endif
