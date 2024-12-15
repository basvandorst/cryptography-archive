#include <stdio.h>
#include <string.h>

/* this should be called bruterc4.c in case you got it via my 2nd
   goofed cpunks post with name rsabrute.c */

/* get latest copy from: http://dcs.ex.ac.uk/~aba/bruterc4.c  */

/*
 *  Quick hacked up rc4 brute force attack program.
 *
 *  (Slightly optimised, could do more)
 *
 *  excellent optimised prepare key by Tatu Ylonen ylo@cs.hut.fi
 *  - boosts performance by >4 x !!!
 *
 *  Gets me 38k keys/sec on a 100 Mhz SGI Indy
 *
 *  Usage:
 *
 *    bruterc4 known cypher start-key [keyspace]
 *
 *      known = your known plaintext file
 *      cypher = the plaintext file encrypted with 40 bit RC4
 *      start-key = start brute forcing keys from this hex key
 *      keyspace = how many bits of keyspace to search (default 31)
 *
 *  One dot gets printed for each 64k keys tried (every 8 seconds on Indy)
 *
 *  Adam Back <aba@dcs.ex.ac.uk>
 *
 */

#define KEY_SIZE 5 /* key size in bytes, ie 40 bits */

#define buf_size 1024

typedef struct rc4_key
{
   unsigned char state[256];
   unsigned char x;
   unsigned char y;
} rc4_key;

#define swap_byte(x,y) t = (x); (x) = (y); (y) = t

void inc_key(unsigned char key[])
{
  int i;
  for (i=KEY_SIZE-1; i && key[i] == 255; i--)
  {
    key[i] = 0;
  }
  key[i]++;
}

const char* print_hex(unsigned char key[],int len)
{
  static char space[513];
  int i, lo, hi;
  for (i=0; i<len; i++)
  {
    sprintf(space+i*2,"%02x",key[i]);
  }
  space[i*2]=0;
  return space;
}

char sequence[256];

/* excellent optimised prepare key by Tatu Ylonen ylo@cs.hut.fi */

void prepare_key(unsigned char *key_data_ptr, int key_data_len, rc4_key *key)
{
  int i;
  unsigned int t, u;
  unsigned int index2;
  unsigned char* state;
  unsigned int counter;
  unsigned int k0, k1, k2, k3, k4;

  state = &key->state[0];
  memcpy(state,sequence,256);

  index2 = 0;
  k0 = key_data_ptr[0];
  k1 = key_data_ptr[1];
  k2 = key_data_ptr[2];
  k3 = key_data_ptr[3];
  k4 = key_data_ptr[4];
  for(counter = 0; counter < 255; counter += 5)
  {
    t = state[counter];
    index2 = (index2 + k0 + t) & 0xff;
    u = state[index2];
    state[index2] = t;
    state[counter] = u;

    t = state[counter + 1];
    index2 = (index2 + k1 + t) & 0xff;
    u = state[index2];
    state[index2] = t;
    state[counter + 1] = u;

    t = state[counter + 2];
    index2 = (index2 + k2 + t) & 0xff;
    u = state[index2];
    state[index2] = t;
    state[counter + 2] = u;

    t = state[counter + 3];
    index2 = (index2 + k3 + t) & 0xff;
    u = state[index2];
    state[index2] = t;
    state[counter + 3] = u;

    t = state[counter + 4];
    index2 = (index2 + k4 + t) & 0xff;
    u = state[index2];
    state[index2] = t;
    state[counter + 4] = u;
  }
  t = state[255];
  index2 = (index2 + k0 + t) & 0xff;
  u = state[index2];
  state[index2] = t;
  state[255] = u;
}

int rc4_eq(unsigned char *buffer_ptr, 
	   unsigned char *known, 
	   unsigned char *cypher_txt,
	   int buffer_len, rc4_key *key)
{
  unsigned int t;
  unsigned int x = 0;
  unsigned int y = 0;
  unsigned char* state;
  unsigned int xorIndex;
  unsigned int counter;

  state = &key->state[0];
  for(counter = 0; counter < buffer_len; counter++)
  {
    x = (x + 1) & 0xFF;
    y = (state[x] + y) & 0xFF;
    swap_byte(state[x], state[y]);
    xorIndex = (state[x] + state[y]) & 0xFF;
    buffer_ptr[counter] ^= state[xorIndex];
    if (known[counter] != buffer_ptr[counter])
    {
      memcpy(buffer_ptr,cypher_txt,counter+1);
      return 0;
    }
  }
  return 1;
}

int main(int argc, char* argv[])
{
  char data[KEY_SIZE*2+1];
  char known[buf_size];
  char cypher_txt[buf_size];
  char try[buf_size];
  char seed[256];
  char digit[5];
  int hex, rd,i, step = 65536;
  int n;
  unsigned int sweep = 1 << 31, done = 0;
  rc4_key key;
  FILE *known_fp, *cypher_txt_fp;

  if (argc < 4)
  {
    fprintf(stderr,"%s known cypher-txt start-key [bits-to-sweep]\n",argv[0]);
    exit(1);
  }
  strncpy(data,argv[3],KEY_SIZE*2);
  data[KEY_SIZE*2]=0;
  if (argc > 4)
  {
    sweep = atoi(argv[4]);
    if (sweep < 31)
    {
      sweep = (1 << sweep);
    }
  }
  n = strlen(data);
  if (n<KEY_SIZE*2)
  {
    char tmp[KEY_SIZE*2+1];
    strncpy(tmp,data,KEY_SIZE*2+1);
    memset(data,'0',KEY_SIZE*2-n);
    memcpy(data+KEY_SIZE*2-n,tmp,n);
    data[KEY_SIZE*2] = 0;
  }
  fprintf(stderr,"start-key:  %s\n",data);
  fprintf(stderr,"iterations: %d\n",sweep);
  for (i=0;i<KEY_SIZE;i++)
  {
    digit[0] = data[i*2];
    digit[1] = data[i*2+1];
    sscanf(digit,"%x",&hex);
    seed[i] = hex;
  }

  known_fp = fopen(argv[1],"rb");
  if (!known_fp) 
  {
    fprintf(stderr,"error cannot open %s\n",argv[1]); 
    exit(1);
  }
  cypher_txt_fp = fopen(argv[2],"rb");
  if (!known_fp)
  {
    fprintf(stderr,"error cannot open %s\n",argv[2]); 
    exit(1);
  }

  rd = fread(known,1,buf_size,known_fp);
  if ( rd <= 0 )
  {
    fprintf(stderr, "error reading known file\n");
  }

  i = fread(cypher_txt,1,rd,cypher_txt_fp);
  if ( i < rd )
  {
    fprintf(stderr, "error reading cypher text file\n");
    exit( 1 );
  }

  fprintf(stderr,"known-text-size: %d\n",rd);

  for(i = 0; i < 256; i++)
    sequence[i] = i;

  memcpy(try,cypher_txt,rd);

  if (sweep < 65536)
  {
    step = 256;
  }

  fprintf(stderr,"%d keys per '.' printed\n", step);

  for (done=0; done<sweep; done+=step)
  {
    for (i=0; i < step; i++)
    {
      prepare_key(seed,KEY_SIZE,&key);
      if (rc4_eq(try, known, cypher_txt, rd, &key))
      {
	fprintf(stderr,"\n0x%s\n", print_hex(seed,KEY_SIZE));
	exit(0);
      }
      inc_key(seed);
    }
    fputc('.',stderr);
  }
}
