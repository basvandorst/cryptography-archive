#include <stdio.h>

#include "ytalk.h"
#include "debug.h"
#include "config.h"
#include "pipes.h"
#include "crypto.h"
#include "mpilib.h"

/* offcrypt()

   Turn off encryption.  If initiate is true, ask all our peers to do
   the same
*/
void
offcrypt(int initiate)
{
    int i;
    
    crypto_req = 0;
    crypto_ack = 0;
    crec.code = CRYPTOFF;
    for (i = 0 ; i < pnum ; i++)
    {
	if (p[i].flags & P_CRYPT)
	{
	    p[i].flags &= ~(P_CRYPT | P_CRYPTWAIT | P_CRYPTACK);
	    change_title(p[i].win, p[i].name, p[i].host, p[i].flags);
	    if (initiate)
		    write(p[i].sfd, &crec, CSIZ);
	}
    }
    change_title(0, "YTalk 2.1", NULL, 0);
}


/* sendDHpublic()

   Send our DH public part.  If private is true, we use private key
   authentication.  If askpass is true, we ask the user for the private
   passphrase.  If askpass is false, the user was already asked for the
   passphrase.
*/
static int
sendDHpublic(int askpass, int private)
{
    int n;
    IDEAkey key;
    word16 iv[4];
    int explen;
    cpack crec1;
    
    int headlen;
    int bodylen;

    int pgp_status;
    
    headlen = RAND_PREFIX_LENGTH + 2;
    bodylen = sizeof(crec.ci_crypt) - headlen;
    
    /* ENCRYPT our key half */
    fill0(crec1.ci_crypt, sizeof(crec1.ci_crypt));
    
    if (crypto_req == 0)
    {
	if (private)
	{
	    if (askpass)
		    getpassphrase();
	    hashpass(inp.data, strlen(inp.data), ideakey);
	}

	init_crypto();
	explen = create_getDHpublic(crec1.ci_crypt + headlen);
	debug_printf (("sendDHpublic: Diffie-Hellman public exponent (create+send):\n"));
	debug_hexdump (crec1.ci_crypt + headlen + 2, bodylen);
	debug_printf (("\n\n"));
	change_title(0, "YTalk 2.1", NULL, P_CRYPT | P_CRYPTACK);
    }
    else
    {
	explen = getDHpublic(crec1.ci_crypt + headlen);
	debug_printf (("sendDHpublic: Diffie-Hellman public exponent (send):\n"));
	debug_hexdump (crec1.ci_crypt + headlen + 2, bodylen);
	debug_printf (("\n\n"));
    }
    
    crec1.ci_sig[0] = 0;
    crec1.ci_sig[1] = 0;
	
    if (crypto_req < pnum && !private && getenv("PGPPATH") != NULL)
    {
	const char *pgp_argv[10];
	char fname[32];
	FILE* pgpf;
	int c;
	
	strcpy(fname, "/tmp/ytkpXXXXXX");

	/* PGP insists on ".sig", and mktemp(3) insists on six
	   *trailing* X's, so there is no way to make a temporary file
	   with ".sig" at the end.  This should be reasonably safe.  */
	strcat (mktemp(fname), ".sig");

	pgp_argv[0] = "pgp";
	pgp_argv[1] = "-f";
	pgp_argv[2] = "+verbose=0";
	pgp_argv[3] = "+armor=off";
	pgp_argv[4] = "+clearsig=off";
	pgp_argv[5] = "+textmode=off";
	pgp_argv[6] = "-sb";
	pgp_argv[7] = "-o";
	pgp_argv[8] = fname;
	pgp_argv[9] = NULL;
	{
	  int i;
	  
	  debug_printf (("Executing:"));
	  for (i = 0; i < 9; i++)
	    debug_printf ((" %s", pgp_argv[i]));
	  debug_printf (("\n"));
	}
	shell_mode();
	fflush(stdout);
	pgpf = open_pgp_pipe (pgp_argv, "w");
	if (pgpf == NULL) {
	  curses_mode ();
	  panic ("Cannot open PGP pipe");
	  return -1;
	}
	fwrite(crec1.ci_crypt + headlen, 1, bodylen, pgpf);
	pgp_status = close_pgp_pipe (pgpf);
	if (pgp_status != 0) {
	  curses_mode ();
	  panic ("Error closing PGP pipe");
	  return -1;
	}
	curses_mode();
	pgpf = fopen(fname, "r");	
	c = fread(crec1.ci_sig + 2, 1, sizeof(crec1.ci_sig) - 2, pgpf);
	fclose(pgpf);
	if (unlink (fname) == -1)
	  panic ("Can't unlink crypto file (ignored)");

	debug_checkcpack (&crec1);
	if (c == sizeof(crec1.ci_sig) - 2 || c <= 0) {
		panic("Invalid output from pgp");
		return -1;
	}
	else
	{
	    crec1.ci_sig[0] = c & 255;
	    crec1.ci_sig[1] = c >> 8;
	}
    }
    else if (crypto_req < pnum && !private)
    {
	panic("Public-key authentication, but PGPPATH unset");
	return -1;
    }
    
    if (explen > bodylen)
	    abort();

    if (private)
    {
	init_idea_stream(crec1.ci_crypt, ideakey, ENCRYPT_IT, key, iv );
	ideacfb(crec1.ci_crypt + headlen, bodylen, key, iv, ENCRYPT_IT);
	crec1.code = CRYPT;
    }
    else
	    crec1.code = CRYPTP;
    
    for (n = 0 ; n < pnum ; n++)
    {
	if ((p[n].flags & P_CRYPT) == 0)
	{
	    write(p[n].sfd, &crec1, CSIZ);
	    p[n].flags |= P_CRYPT;
	    crypto_req++;
	    change_title(p[n].win, p[n].name, p[n].host, p[n].flags);
	}
    }
    return 0;
}

/* startcrypt()

   Initiate encryption with peers.  The user was already asked for the
   passphrase (if private is true).  If we are already encrypting,
   initiate turning encryption off.
*/
void
startcrypt(char* key, int private)
{
    char buf[4096];
    
    byte* expptr;
    
    if (crypto_req == pnum)
    {
	offcrypt(1);
	return;
    }
    
    if (sendDHpublic(0, private) < 0) {
      panic ("Can't send DH public part");
      return;
    }
    
    change_title(0, "YTalk 2.1", 0, P_CRYPT | P_CRYPTACK);
    putc('\07', stderr);
}



/* look_for_signature()

   Look for the strings "Signature" or "signature from" in PGP's
   output.
*/
static void
look_for_signature (char *str1, char *str2, char *str3, FILE *pgp_stdout)
{
  char *current_str;

  str1[0] = '\0';
  str2[0] = '\0';
  str3[0] = '\0';

  debug_printf (("In look_for_signature\n"));
  
  current_str = str1;
  do {
      debug_printf (("Before while..."));
    while (fgets (current_str, 60, pgp_stdout) != NULL
	   && strstr (current_str, "Signature") == NULL
	   && strstr (current_str, "signature from") == NULL) {
      debug_printf (("Reading..."));
    }

    debug_printf (("Ocurrence\n"));
    if (current_str == str1) {
      current_str = str2;
    debug_printf (("Change 1\n"));
    } else if (current_str == str2) {
      current_str = str3;
    debug_printf (("Change 2\n"));
    } else {
      current_str = 0;
    debug_printf (("Change 3\n"));
    }
  } while (current_str != 0);

  str1[strlen (str1) - 1] = '\0';
  str2[strlen (str2) - 1] = '\0';
  str3[strlen (str3) - 1] = '\0';

  debug_printf (("Leaving look_for_signature()\n"));
}


/* checksig()

   Check a PGP signature.
*/

static void
checksig(int siglen, int i)
{
    int headlen;
    int bodylen;
    const char *pgp_argv[6];
    char fname[32];
    char sname[32];
    char str1[60];
    char str2[60];
    char str3[60];
    FILE* pgpf;
    
    headlen = RAND_PREFIX_LENGTH + 2;
    bodylen = sizeof(crec.ci_crypt) - headlen;
    
    if (getenv("PGPPATH") == NULL) {
        panic ("Can't check pgp signature 'cause PGPPATH is unset");
	return;
    }

    strcpy(fname, "/tmp/ytkxXXXXXX");
#if !(defined(sco) || defined(__ultrix))
    close(mkstemp(fname));
#else
    mktemp(fname);
#endif
	
    pgpf = fopen(fname, "w");
    if (pgpf == NULL) {
        panic ("Can't open crypt file #1");
	return;
    }
    
    if (fwrite(crec.ci_crypt + headlen, 1, bodylen, pgpf) != bodylen) {
        panic ("write error during crypt write #1");
	return;
    }
    
    (void) fclose(pgpf);
	
    strcpy(sname, "/tmp/ytksXXXXXX");
#if !(defined(sco) || defined(__ultrix))
    close(mkstemp(sname));
#else
    mktemp(sname);
#endif
	
    pgpf = fopen(sname, "w");
    if (pgpf == NULL) {
        panic ("Can't open signature file");
	return;
    }
    
    if (fwrite(crec.ci_sig + 2, 1, siglen, pgpf) != siglen) {
        panic ("write error during signature write");
	return;
    }

    (void) fclose(pgpf);
	
    fflush(stdout);
    pgp_argv[0] = "pgp";
    pgp_argv[1] = "+batch";
    pgp_argv[2] = "+verbose=0";
    pgp_argv[3] = sname;
    pgp_argv[4] = fname;
    pgp_argv[5] = NULL;
    {
      int i;
	  
      debug_printf (("Executing:"));
      for (i = 0; i < 5; i++)
	debug_printf ((" %s", pgp_argv[i]));
      debug_printf (("\n"));
    }

    pgpf = open_pgp_pipe (pgp_argv, "r");
    if (pgpf == NULL) {
        panic ("Can't open pipe to PGP");
	return;
    }

    look_for_signature (str1, str2, str3, pgpf);
    if (close_pgp_pipe (pgpf) < 0)
      panic ("Error closing PGP pipe");
    
    if (unlink(fname) == -1)
      panic ("Can't unlink crypt file");
    if (unlink(sname) == -1)
      panic ("Can't unlink signature file");
	
    /*fprintf(stderr, "Press any key:");
      mycbreak();
      getchar();*/
	
    if (str1[0] == '\0')
        strcpy(str1, "Invalid pgp signature");

    showmsg(str1, str2, str3);
}

/* gotcrypt()

   This is called when we get a DH public key packet.
*/
void
gotcrypt(int i, int private)
{
    char buf[4096];
    unit peerkey[MAX_UNIT_PRECISION];
    
    int n, j;
    
    IDEAkey key;
    word16 iv[4];
    
    byte* expptr;
    int explen;
    
    int headlen;
    int bodylen;
    int siglen;
    
    headlen = RAND_PREFIX_LENGTH + 2;
    bodylen = sizeof(crec.ci_crypt) - headlen;
    
    if (p[i].flags & P_CRYPTWAIT)
    {
	panic("Protocol error! Already got session key from peer");
	return;
    }
    
    if (sendDHpublic(1, private) < 0) {
      panic ("Can't send DH pubpic part");
      return;
    }
    
    /* DECRYPT their key half */
    if (private)
    {
	if (init_idea_stream(crec.ci_crypt, ideakey, DECRYPT_IT, key, iv )
	    < 0)
	{
	    panic("Warning! Keys don't match.");
	    mp_init(peerkey, 2);
	    /* Some sane value so we can continue -
	     This value will force the session keys to be different.
	     This is useful to demonstrate that someone with the
	     wrong key will get garbage. */
	}
	else
	{
	    ideacfb(crec.ci_crypt + headlen, bodylen, key, iv, DECRYPT_IT) ;
	    mpi2reg(peerkey, crec.ci_crypt + headlen);
	}
    }
    else
    {
	mpi2reg(peerkey, crec.ci_crypt + headlen);
    }

    siglen = (unsigned char)crec.ci_sig[0];
    siglen += (unsigned char)crec.ci_sig[1] << 8;
    
    if (siglen)
	    checksig(siglen, i);

    explen = computeDHagreed_key(peerkey, &expptr);
    debug_printf (("gotcrypt: Diffie-Hellman public key (received):\n"));
    debug_hexdump ((byte *)peerkey + 0x50, explen);
    debug_printf (("\n\n"));
    
    fill0(p[i].key, sizeof(p[i].key));
    for (n = 0 ; n < 16 ; n ++)
    {
	for (j = 0 ; j + n < explen ; j += 16)
	{
	    p[i].key[n] ^= expptr[j + n];
	}
    }
    
    crec.code = CRYPTIV;
    init_idea_stream(crec.ci_crypt, p[i].key, ENCRYPT_IT,
		     p[i].outkey, p[i].outiv );
    
    write(p[i].sfd, &crec, CSIZ);
    p[i].flags |= P_CRYPTWAIT;
}   

void
gotiv(int i)
{
    if ((p[i].flags & P_CRYPTWAIT) == 0)
    {
	panic("Protocol error!  Got IV before session key from peer");
	return;
    }
    
    if (init_idea_stream(crec.ci_crypt, p[i].key, DECRYPT_IT,
			 p[i].inkey, p[i].iniv ) < 0)
	    panic("Warning!  Session keys don't match.");
    p[i].flags |= P_CRYPTACK;
    crypto_ack++;
    
    change_title(p[i].win, p[i].name, p[i].host, p[i].flags);
    
    if (crypto_ack == pnum)
	    putc('\07', stderr);
}
