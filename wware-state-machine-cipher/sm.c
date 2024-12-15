/* =================================================================
This is a software implementation of the private key state machine
encryption scheme I've been tinkering with. The state machine produces
deterministic but hard-to-predict pseudo-random bits based on its
state, the current key byte, and the current cleartext byte.
================================================================= */

#include "stdio.h"
#include "conio.h"
#include "stdlib.h"
#include "string.h"
#include "malloc.h"

typedef unsigned char uchar;
typedef unsigned int uint;
typedef unsigned long ulong;

/* =================================================================
For efficient implementation with Xilinx FPGAs, NUM_INPUTS_PER_BIT should
be no more than four or five. This probably compromises security; in an
ideal chip implementation, each state bit would depend on all other state
bits, all key bits, and all cleartext bits, i.e. NUM_INPUTS_PER_BIT would
be MACHINE_SIZE + 16. For purposes of running this simulation painlessly on
a PC, NUM_INPUTS_PER_BIT has an upper limit of 14.
================================================================= */

#define MAX_KEY_SIZE 80
#define MACHINE_SIZE 100
#define NUM_INPUTS_PER_BIT 14
#define LUT_SIZE (1 << NUM_INPUTS_PER_BIT)

uchar huge lut[LUT_SIZE];

uchar key[MAX_KEY_SIZE], temp_bits[MACHINE_SIZE + 16],
    inputs_table[MACHINE_SIZE][NUM_INPUTS_PER_BIT],
    state_bits[MACHINE_SIZE], partial_state;

uint key_byte_pointer;

/* ================================================================= */

void propogate_machine(uchar x)
    {
    uint i, j, k;

    for (i = 0; i < MACHINE_SIZE; i++)
        temp_bits[i] = state_bits[i];
    for (i = 0; i < 8; i++)
        temp_bits[MACHINE_SIZE + i] = (x >> i) & 1;
    for (i = 0; i < 8; i++)
        temp_bits[MACHINE_SIZE + 8 + i] = (key[key_byte_pointer] >> i) & 1;
    key_byte_pointer = (key_byte_pointer + 1) % strlen(key);

    for (i = 0; i < MACHINE_SIZE; i++)
        {
        for (j = 0, k = 0; j < NUM_INPUTS_PER_BIT; j++)
            if (temp_bits[inputs_table[i][j]]) k |= 1 << j;
        state_bits[i] = lut[k];
        }

    for (i = 0, partial_state = 0; i < 8; i++)
        if (state_bits[i]) partial_state |= 1 << i;
#if VERBOSE
    for (i = 0; i < min(70, MACHINE_SIZE); i++)
        printf("%d", state_bits[i]);
    printf("\n");
#endif
    }

void reset_machine(void)
    {
    uint i;

    key_byte_pointer = 0;

    partial_state = 0;
    for (i = 0; i < MACHINE_SIZE; i++) state_bits[i] = 0;

    i = strlen(key);
    while (i--) propogate_machine(0);
    }

void initialize_machine(void)
    {
    uint i, j, k;

    for (i = 0; i < LUT_SIZE; i++)
        lut[i] = rand() & 1;
    for (i = 0; i < MACHINE_SIZE; i++)
        for (j = 0; j < NUM_INPUTS_PER_BIT; j++)
            {
try_another:
            inputs_table[i][j] = rand() % (MACHINE_SIZE + 16);
            for (k = 0; k < j; k++)
                if (inputs_table[i][k] == inputs_table[i][j])
                    goto try_another;
            }
    reset_machine();
    }

/* ================================================================= */

uchar encrypt_char(uchar x)
    {
    uchar y;

    y = partial_state ^ x;
    propogate_machine(x);
    return y;
    }

uchar decrypt_char(uchar y)
    {
    uchar x;

    x = partial_state ^ y;
    propogate_machine(x);
    return x;
    }

/* ================================================================= */

char help_string[] =
"Usage: sm <infile> <key> <outfile> [<flag>]\n"
"The input file is to be encrypted or decrypted.\n"
"The key is any ASCII string. If it includes spaces, enclose it\n"
"    in double quotes. Also, it can't include '<' or '>' or '|'\n"
"    which all have special meanings on the DOS command line.\n"
"The output file is the result.\n"
"The optional flag is any ASCII string; it selects\n"
"    decryption instead of encryption.\n";

void help(void)
    {
    printf("%s", help_string);
    exit(0);
    }

void main(int argc, char *argv[])
    {
    int c;
    FILE *f, *g;

    if (argc < 4) help();

    if ((f = fopen(argv[1], "rb")) == NULL) help();

    strcpy(key, argv[2]);

    if ((g = fopen(argv[3], "wb")) == NULL) help();

#if VERBOSE
    printf("Priming the state machine\n");
    initialize_machine();
    printf("Beginning encryption\n");
#else
    initialize_machine();
#endif

    while (1)
        {
        c = getc(f);
        if (c == -1 || feof(f))
            {
            fclose(f);
            fclose(g);
            return;
            }

        if (argc == 4) c = encrypt_char(c);
        else c = decrypt_char(c);

        putc(c, g);
        }
    }
