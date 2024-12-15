/*
------------------------------------------------------------------
  Copyright
  Sun Microsystems, Inc.


  Copyright (C) 1994, 1995, 1996 Sun Microsystems, Inc.  All Rights
  Reserved.

  Permission is hereby granted, free of charge, to any person
  obtaining a copy of this software and associated documentation
  files (the "Software"), to deal in the Software without
  restriction, including without limitation the rights to use,
  copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software or derivatives of the Software, and to 
  permit persons to whom the Software or its derivatives is furnished 
  to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be
  included in all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
  OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
  NONINFRINGEMENT.  IN NO EVENT SHALL SUN MICROSYSTEMS, INC., BE LIABLE
  FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
  OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
  CONNECTION WITH THE SOFTWARE OR DERIVATES OF THIS SOFTWARE OR 
  THE USE OR OTHER DEALINGS IN THE SOFTWARE.

  Except as contained in this notice, the name of Sun Microsystems, Inc.
  shall not be used in advertising or otherwise to promote
  the sale, use or other dealings in this Software or its derivatives 
  without prior written authorization from Sun Microsystems, Inc.
*/
#pragma ident "@(#)randseed.C	1.11 96/01/29 Sun Microsystems"

#include <sys/types.h>	/* For pid_t, getpid(), getppid() */
#include <stdio.h>	/* For FILE, popen(), etc. */
#include <stdlib.h>
#include <string.h>	/* For memset(), memcpy() */
#include <unistd.h>
#include <time.h>
#include <sys/time.h>	/* For gettimeofday() */
#include "Bstream.h"
#include "sha.h"

/*
 * Random number generator theory of operation.
 *
 * This derives its seed values from running some "ps"-like commands,
 * using SHA to mix everything together.  The results are accumulated
 * into a static buffer which, hopefully, becomes less and less
 * predictible as time goes by.  The buffer is used to generate the
 * random output.  The actual commands are (tried with an explicit
 * path that should be good first, then implicit $PATH):
 * last		(Initialization only)
 * pstat -fp
 * netstat -n
 * vmstat -s
 * netstat -in
 * pstat -isS
 * netstat -sn
 * iostat -I
 * netstat -rn
 *
 * It hashes the output into an SHA hash context, and then calls
 * "randpool_update" with the SHA hash context.  randpool_update then
 * does the following:
 * - hashes the pool into the SHA hash context to make the state depend
 *   on the pool and thus, all previous bytes.
 * - copies the HSA hash context, finalizes the copy, and XORs the
 *   resultant 20-byte hash into the hash context at a location
 *   that is moved forward through the pool each time.  (Thus
 *   eventually updating the whole pool.)
 * - hashes the 20-byte hash into the original SHA hash context, ensuring
 *   (through the one-way property) that the output of this hash doesn't
 *   reveal anything about the the value XORed into the hash context and
 *   thus, indirectly, the state of the random number pool.
 *
 * The SHA context can then be finalized and the output used as a
 * random number.
 *
 * Actually, initially the run command--hash output--randpool_update
 * cycle is performed for all of the above-mentioned commands, plus
 * gettimeofday(), getpid() and getppid(), before the final hash is used
 * for output.
 *
 * After that, bytes are taken from the output buffer as needed.  If
 * it becomes empty, it is refilled.
 * 
 * If gettimeofday() shows that "a while" has passed since the last time
 * one of the commands was run, the next one is run in rotating order and
 * passed to randpool_update.  (Yes, the code works in the face of time
 * warping backwards.)
 * Then the gettimeofday() output is itself hashed and fed to randpool_update,
 * and the result of that used to refill the buffer.
 *
 * "A while" is defined as THROTTLE seconds, tunable to keep system load
 * down.  If multiples of THROTTLE seconds have passed, multiple commands
 * are run, within limits.
 *
 * Output can be produced by randpool_getbytes (which fills a supplied buffer)
 * or get_random_bytes (which returns a Bstream).
 *
 * You can also invoke this with external data if desired, by calling
 * randpool_addbytes and passing in a buffer.  This is used by
 * prompt_user_for_randomness, which timestamps each keystroke and
 * hashes in the character and the time.
 */

/*
 * Don't run more than one entropy-gathering command per this many seconds.
 * (This only matters to the keymgrd, since it's the only thing that
 * runs for any appreciable amount of time.)
 */
#define THROTTLE 300

/* Number of SHA_DIGESTSIZE chunks there are in the randpool. */
#define RANDPOOL_MULTIPLE 20

/*
 * This takes an SHAContext and updates the random pool with it, returning
 * it in a state which is ready to have SHAFinal() called on it.
 * It hashes the pool into the context, clones the context, and
 * finalizes the clone.  That final hash is then also hashed into the
 * original context, and XORed into the pool at position "randpoolindex",
 * which is subsequently incremented.
 *
 * This modifies the random number pool in a way that depends on all of
 * the past history data, and returns a number also depending on every
 * call to randpool_update so far, but it is computationally infeasible
 * to derive the randpool's internal state from the output data.
 */
static void
randpool_update(struct SHAContext *sha)
{
	static unsigned char randpool[SHA_DIGESTSIZE*RANDPOOL_MULTIPLE];
	static unsigned char randpoolindex = 0;
	unsigned char buffer[SHA_DIGESTSIZE];
	struct SHAContext sha2;
	unsigned i;
	
	shaUpdate(sha, randpool, sizeof(randpool));
	sha2 = *sha;
	shaFinal(&sha2, buffer);
	shaUpdate(sha, buffer, sizeof(buffer));

	for (i = 0; i < sizeof(buffer); i++)
		randpool[randpoolindex+i] = buffer[i];
	randpoolindex += i;
	if (randpoolindex >= sizeof(randpool))
		randpoolindex = 0;
	memset(buffer, 0, sizeof(buffer));
}

// Get entropy from system statistics.  Pick the next available system
// statistics info from a list of commands and run it.
// (Try the usual location first, then search the $PATH.)
// Hash the output into the given SHAContext and call randpool_update.
//
// Note that these commands are for SunOS 4.1.3; other systems will be
// slightly different and need them changed.
static void
randpool_getentropy(struct SHAContext *sha)
{
	FILE *f;
	static char const * const commands[] = {
#ifdef SYSV		// Actually SOLARIS
		"(/usr/bin/last || last) 2>&1", /* Initialization only */
		"(/usr/ucb/netstat -k || netstat -k) 2>&1",
		"(/usr/ucb/netstat -n || netstat -n) 2>&1",
		"(/usr/ucb/vmstat -s || vmstat -s) 2>&1",
		"(/usr/ucb/netstat -in || netstat -in) 2>&1",
		"(/usr/etc/pstat -isS || pstat -isS) 2>&1",
		"(/usr/ucb/netstat -sn || netstat -sn) 2>&1",
		"(/usr/bin/iostat -I || iostat -I) 2>&1",
		"(/usr/ucb/netstat -rn || netstat -rn) 2>&1"
#else
		"(/usr/ucb/last || last) 2>&1",	/* Initialization only */
		"(/usr/etc/pstat -fp || pstat -fp) 2>&1",
		"(/usr/ucb/netstat -n || netstat -n) 2>&1",
		"(/usr/ucb/vmstat -s || vmstat -s) 2>&1",
		"(/usr/ucb/netstat -in || netstat -in) 2>&1",
		"(/usr/etc/pstat -isS || pstat -isS) 2>&1",
		"(/usr/ucb/netstat -sn || netstat -sn) 2>&1",
		"(/usr/bin/iostat -I || iostat -I) 2>&1",
		"(/usr/ucb/netstat -rn || netstat -rn) 2>&1"
#endif
	};
	static int cmdindex = 0;
	int i;
	int status;
	char buffer[256];

	i = cmdindex;
	status = 1;
	do {
		f = popen(commands[i++], "r");
		if (i >= sizeof(commands)/sizeof(commands[0]))	
			i = 1;	/* last(1) is only useful once. */
		if (!f)
			continue;
		if (i == cmdindex)
			break;

		while (!feof(f)) {
			status = fread(buffer, 1, sizeof(buffer), f);
			if (status < 0)
				break;
			shaUpdate(sha, (unsigned char *)buffer,
			          (unsigned)status);
		}
		status = pclose(f);
	}  while (status);	/* Repeat until one succeeds */

	cmdindex = i;
	memset(buffer, 0, sizeof(buffer));

	randpool_update(sha);
}

// Buffer of pre-accumulated random bytes.
static unsigned char randbuf[SHA_DIGESTSIZE];
static unsigned randbufindex = SHA_DIGESTSIZE;

// Insert some noise of external origin into the random number pool.
static void
randpool_addbytes(unsigned char const *buf, unsigned len)
{
	struct SHAContext sha;

	shaInit(&sha);
	shaUpdate(&sha, buf, len);
	randpool_update(&sha);
	shaFinal(&sha, randbuf);
	randbufindex = 0;
}

// Return the desired number of bytes.  Consumes data left in the randbuf
// first, then hashes in the current time and, if it isn't too soon,
// a bunch of system statistics, to the randpool.
void
randpool_getbytes(unsigned char *buf, unsigned len)
{
	struct timeval tv;
	unsigned avail;
	struct SHAContext sha;
	static time_t prev = 0;

	if (!prev) {	// First time, do full initialization
		pid_t p;

		gettimeofday(&tv, 0);
		shaInit(&sha);
		shaUpdate(&sha, (unsigned char *)&tv, sizeof(tv));
		prev = tv.tv_sec;
		memset(&tv, 0, sizeof(tv));
		p = getpid();
		shaUpdate(&sha, (unsigned char *)&p, sizeof(p));
		p = getppid();
		shaUpdate(&sha, (unsigned char *)&p, sizeof(p));
		randpool_update(&sha);
		
		for (avail = 0; avail < 9; avail++)
			randpool_getentropy(&sha);
		shaFinal(&sha, randbuf);
		randbufindex = 0;
	}

	avail = SHA_DIGESTSIZE-randbufindex;
	if (avail > len)
		avail = len;
	if (avail) {
		memcpy(buf, randbuf+randbufindex, avail);
		len -= avail;
		buf += avail;
		memset(randbuf+randbufindex, 0, avail);
	}
	
	if (!len) {
		randbufindex += avail;
		return;
	}

	gettimeofday(&tv, 0);
	shaInit(&sha);

	// If it's been at least THROTTLE seconds since last
	// time, get more system statistics.  If it's been a lot more,
	// get multiple statistics.
	// NOTE the comparison which deals properly with time warping
	// backwards by considering time to have expired.
	if ((unsigned)(tv.tv_sec-prev) > THROTTLE) {
		avail = 5;	/* Maximum to hash in */
		do {
			randpool_getentropy(&sha);
			prev += THROTTLE;
		} while ((unsigned)(tv.tv_sec-prev) > THROTTLE && --avail);
		prev = tv.tv_sec;
	}
	shaUpdate(&sha, (unsigned char *)&tv, sizeof(tv));
	memset(&tv, 0, sizeof(tv));

	while (len > SHA_DIGESTSIZE) {
		randpool_update(&sha);
		shaFinal(&sha, buf);
		buf += SHA_DIGESTSIZE;
		len -= SHA_DIGESTSIZE;
		shaInit(&sha);
	}

	shaFinal(&sha, randbuf);
	memcpy(buf, randbuf, len);
	memset(randbuf, 0, len);
	randbufindex = len;
}

// In interactive applications, this function can be used to
// augment available randomness by getting some from the user.
void
prompt_user_for_randomness()
{
	struct timeval t;
	int count = 0;
	char c;

	printf("It would help the quality of the random numbers if you would\n"
	       "type 50-100 random keys on the keyboard.  Hit return when\n"
	       "you are done.\n");
	system("stty raw -echo");
	for (;;) {
		if (read(0, &c, 1) != 1) {
			perror("read");
			break;
		}
		gettimeofday(&t, (struct timezone *) 0);
		// Place c into timeval to save a call to randpool_addbytes
		t.tv_usec += c << 24;
		randpool_addbytes((unsigned char *)&t, sizeof(t));
		if (c == '\n' || c == '\r')
		  break;
		printf("\r%-5d", ++count);
		fflush(stdout);
	}
	printf("\r\n");
	system("stty sane");
}

Bstream
get_random_bytes(int len)
{
	byte c;
	Bstream output(len, &c);	// Ugly hack...

	randpool_getbytes(output.getdatap(), len);
	return output;
}
