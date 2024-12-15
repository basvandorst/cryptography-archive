// this code comes from an anonymous Usenet post

#include "rc4.h"

RC4::RC4(const byte *key_data_ptr, int key_data_len)
    : state(SecAlloc(byte, 256))
{
    byte index1;
    byte index2;
    int counter;

    for (counter = 0; counter < 256; counter++)
        state[counter] = (byte) counter;
    x = 0;
    y = 0;
    index1 = 0;
    index2 = 0;
    for (counter = 0; counter < 256; counter++) {
        index2 = (key_data_ptr[index1] + state[counter] + index2);
        swap(state[counter], state[index2]);
        index1 = (index1 + 1) % key_data_len;
    }
}

RC4::~RC4()
{
    x=0;
    y=0;
    SecFree(state, 256);
}

void RC4::ProcessString(byte *outString, const byte *inString, unsigned int length)
{
    byte *const s=state;
    while(length--)
    {
        byte a = s[++x];
        y += a;
        byte b = s[y];
        s[y] = a;
        a += b;
        s[x] = b;
        *outString++ = *inString++ ^ s[a];
    }
}

void RC4::ProcessString(byte *inoutString, unsigned int length)
{
    byte *const s=state;
    while(length--)
    {
        byte a = s[++x];
        y += a;
        byte b = s[y];
        s[y] = a;
        a += b;
        s[x] = b;
        *inoutString++ ^= s[a];
    }
}

