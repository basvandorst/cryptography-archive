// test.c -- testing functions for syfre (alternate main()).
//
// written by Marc Wallace, 02 May 93.
// last change: version 1.0 (02 May 93).


#include "include.h"
#include <stream.h>
#include <string.h>
#include <stdiostream.h>
#include <stdio.h>


extern void syfdump();
extern char undo(char);


int main()
    {
    Data     d1(1024), d2(1024), d3(1024);
    int      r, s, t;
    Cypher*  curr;

    randinit();

    // list all available cyphers
    cout << "List of all cyphers compiled in version ";
    cout << version[VERSION] << ":\n";
    for (r = 0; (r < verlimit[VERSION]); r++)
        cout << "    " << (cypherlist[r])->name << "\n";
    cout << "\n";

    for (r = 0; (r < verlimit[VERSION]); r++)
        {
        curr = cypherlist[r];
        cout << "Testing " << curr->name << ":\n    ";

        // test each cypher for each possible size
        for (d1.size = 4; d1.size < 1024; d1.size += 4)
            {
            // fill d1 with random data
            for (t = 0; t < d1.size; t++)
                d1.data[t] = randc();

            // encode and then decode (should reverse)
            (curr->encode)(d1, d2);
            (curr->decode)(d2, d3);

            // if the sizes differ, it failed miserably
            if (d1.size != d3.size)
                {
                cout << "\n";
                cout << "    Sizes of X and d(e(X)) do not match.\n";
                cout << "    Sizes were " << d1.size << " and " << d3.size;
                cout << "\n\nTerminating cypher tests.\n";
                return 1;
                };

            // if one of the bytes is different, it failed
            for (t = 0; t < d1.size; t++)
                if (d1.data[t] != d3.data[t])
                    {
                    cout << "\n";
                    cout << "    X and d(e(X)) differ at byte " << t << ".\n";

                    cout << "    debug data:\n";
                    for (r = t - 4; (r < t + 16) && (r < d3.size); r++)
                        printf("          %-3d>>   %8x %8x %8x\n", r,
                        int(d1.data[r]), int(d2.data[r]), int(d3.data[r]));
                    if (r < t + 16)
                        for (; (r < t + 16) && (r < d2.size); r++)
                            printf("          %-3d>>            %8x\n",
                            r, int(d2.data[r]));

                    cout << "\n\nTerminating cypher tests.\n";
                    return 2;
                    }

            // echo something so we know we're working
            if (d1.size % 32 == 0)
                {
                cout << '.';
                cout.flush();
                };
            };

        cout << " passed reversibility test.\n";
        };
    };
