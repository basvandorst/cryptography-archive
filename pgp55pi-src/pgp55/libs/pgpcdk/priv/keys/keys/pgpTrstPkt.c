/*
 * pgpTrust.c - manage translations of trust description strings
 * for the maintenence display.
 *
 * Written by Colin Plumb.
 *
 * $Id: pgpTrstPkt.c,v 1.2 1997/05/02 22:45:48 lloyd Exp $
 */
#include "pgpConfig.h"

#include <string.h>

#include "pgpTrstPkt.h"
#include "pgpMemPool.h"

#define USERTRANS(x) x
#define userTrans(x) x

char const *keyTrustTable[14] = {
	USERTRANS("undefined"),
	USERTRANS("unknown"),
	USERTRANS("untrusted"),
	"<3>",
	"<4>",
	USERTRANS("marginal"),
	USERTRANS("complete"),
	USERTRANS("ultimate"),
/* Some special cases displayed in the same column */
	"",
	USERTRANS("retired"),
	USERTRANS("revoked"),
	USERTRANS("untested"),
	USERTRANS("INVALID"),
	USERTRANS("** BAD **")
};

char const *uidValidityTable[4] = {
	USERTRANS("undefined"),
	USERTRANS("invalid"),
	USERTRANS("marginal"),
	USERTRANS("complete"),
};

/*
 * Translate the trust tables at once, to avoid massive overhead while
 * displaying the output.
 */
#if 0
void
trustInitTables(void)
{
	static int initialized = 0;
	int i, len;
	char const *s;
	char *p;

	if (initialized)
		return;

	for (i = 0; i < sizeof(keyTrustTable)/sizeof(*keyTrustTable); i++) {
		s = userTrans(keyTrustTable[i]);
		if (s != keyTrustTable[i]) {
			len = strlen(s)+1;
			p = memPoolAlloc(&MiscPool, len, 1);
			if (p) {
				memcpy(p, s, len);
				keyTrustTable[i] = p;
			}
		}
	}
	for (i = 0; i < sizeof(uidValidityTable)/sizeof(*uidValidityTable);
			i++)
	{
		s = userTrans(uidValidityTable[i]);
		if (s != uidValidityTable[i]) {
			len = strlen(s)+1;
			p = memPoolAlloc(&MiscPool, len, 1);
			if (p) {
				memcpy(p, s, len);
				uidValidityTable[i] = p;
			}
		}
	}
}
#endif




