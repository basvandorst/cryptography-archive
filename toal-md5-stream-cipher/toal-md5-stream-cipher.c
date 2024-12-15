From msuinfo!agate!howland.reston.ans.net!pipex!uknet!demon!an-teallach.com!gtoal Thu Mar 10 20:28:14 1994
Path: msuinfo!agate!howland.reston.ans.net!pipex!uknet!demon!an-teallach.com!gtoal
Newsgroups: sci.crypt
From: gtoal@an-teallach.com (Graham Toal)
Subject: Message digest -> cypher.
X-Phone: +44 31 662 0366
X-Fax: +44 31 662 4678
X-Organisation: An Teallach Limited
Date: Thu, 10 Mar 1994 16:14:29 +0000
Message-ID: <199403101614.QAA29710@an-teallach.com>
Sender: usenet@demon.co.uk
Lines: 107

Oops - I accidentally mailed this back to Paul instead of posting it here.
Let's try again...

Subject: Re: Message digest algorithms

: Why are message digest algorithms separate from encryption algorithms? 
: Surely if you started with a block of 128 zeros and encrypted it using
: IDEA in some complex feedback mode using every 128 characters of message
: as a different key, then learning anything about the message, or finding
: another with the same effect, would be akin to a known plaintext attack
: on multiple invocations of the cypher?

I've no idea why.  I suspect that MD algorithm + suitable code to adapt
into cypher together *are* treated by the US authorities as an encryption
algorithm.  Whether either is, independantly, I've no idea.

By the way, here's my quick hack to turn md5 into a cypher.  This is
what I'm currently using as a test cypher in the hack I'm doing to
offer encryption cheaply over any sort of comms link with user-level
code.  I still haven't found a safe non-patented way of sharing a
secret yet though.  Looks like it'll have to be DH after all.

/* md5 CFB mode stream encryption module */
#include "md5.h"

#define STATE_SIZE MD5_BLOCKSIZE
typedef struct cypherstate {
  MD5_CTX md5;
  char iv[STATE_SIZE];
  char input[STATE_SIZE];
  int next_free_input;
  long int byteno;
} CYPHER_STATE;

void new_cypher(CYPHER_STATE *s, char *unique_tag);

char encode_stream_cypher(CYPHER_STATE *s, char byte);
char decode_stream_cypher(CYPHER_STATE *s, char byte);

void new_cypher(CYPHER_STATE *s, char *tag)
{
int i;
  for (i = 0; i < STATE_SIZE; i++) {
    /* This should be set to a random secret iv by key-exchange
       I still haven't done that bit yet.  *Any* sort of shared
       secret will do, as I can feed it in to md5 to create the
       initial (excuse the tautology) iv.   Doesn't have to be
       a random bitstring - a single prime number or a password
       would do. */
    s->iv[i] = 0xff;
  }
  s->next_free_input = 0;
  s->byteno = 0L;
}

char encode_stream_cypher(CYPHER_STATE *s, char byte)
{
  int i;
  char crypto;

  s->byteno += 1L;
  crypto = byte ^ s->iv[s->next_free_input];
  s->input[s->next_free_input] = crypto;
  s->next_free_input += 1;
  if (s->next_free_input == STATE_SIZE) {
    MD5SetConst(0);
    MD5Init(&s->md5);
    MD5Update(&s->md5, (BYTE *)&s->input[0], STATE_SIZE);
    MD5Final(&s->md5);
    for (i = 0; i < STATE_SIZE; i++) {
      s->iv[i] = s->md5.digest[i];
    }
    s->next_free_input = 0;
  }
  return(crypto);
}

char decode_stream_cypher(CYPHER_STATE *s, char crypto)
{
  int i;
  char byte;

  s->byteno += 1L;
  byte = crypto ^ s->iv[s->next_free_input];
  s->input[s->next_free_input] = crypto;
  s->next_free_input += 1;
  if (s->next_free_input == STATE_SIZE) {
    MD5SetConst(0);
    MD5Init(&s->md5);
    MD5Update(&s->md5, (BYTE *)&s->input[0], STATE_SIZE);
    MD5Final(&s->md5);
    for (i = 0; i < STATE_SIZE; i++) {
      s->iv[i] = s->md5.digest[i];
    }
    s->next_free_input = 0;
  }
  return(byte);
}

: Does this have well-known (or well-suspected) weaknesses, or
: disadvantages over other feedback modes?  If so, where are they
: discussed?  

Same question as Paul, for the code above please.

G


