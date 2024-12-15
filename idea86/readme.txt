
These files are the 8086 assembly source code for an implementation of
the International Data Encryption Algorithm (IDEA) cryptosystem.

idea.asm        - IDEA source
test.asm        - IDEA test driver source
hash.asm        - tandem DM hash algorithm
htest.asm       - tandem DM test driver source
cfb.asm         - cipher feedback (CFB) source
crypt.asm       - demo IDEA/CFB file encryption program source

The files were created for use with Borland's Turbo Assembler, but it
shouldn't require many changes to make them work with other assemblers.

Read the test.asm, htest.asm, and crypt.asm files for examples of how to
import the IDEA routines from the assembled object files.

The encryption routines operate on 2-byte words in little endian (Intel)
order. Big-endian input data is expected only for the tandem DM hash
routine, and for CFB encryption/decryption. If in doubt, read the source
code comments.

DESCRIPTION

The following routines are included (public labels):

    idea.asm:
        IDEA        {IDEA encryption}
        Expandkey   {user key to IDEA key expansion}

    hash.asm:
        TandemDM    {tandem DM hashing algorithm}
        hashBurn    {destroy hash and associated buffers}
        Hash        [128 bit hash]

    cfb.asm:
        Encrypt     {CFB encryption}
        Decrypt     {CFB decryption}
        CFBBuffer   [64 bit CFB buffer, required for IV]

Note: {Code} and [Data].

DISCLAIMER

The demo encryption program is just that... a demo of how to use the
IDEA code.

Don't use the demo program for real use. I keep changing the CFB IV
every time I update the code, which means if you lose your copy of the
program, and you don't know what the IV was, your encrypted data is
useless. It is safe to use for testing the IDEA code though.

Although the IDEA source is public domain, the demo program is copyright.
Feel free to do whatever you like with the IDEA source and the demo source,
provided you do not upload or distribute modified copies of the demo
software (source or executables).

    Fauzan Mirza
    fauzan@dcs.rhbnc.ac.uk

