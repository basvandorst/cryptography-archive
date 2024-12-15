    This package contains 2 DOS executables of the ENIGMA program.
    The brunigma.exe on this disk wasn't compiled from the C Source
    included.  I made use of a special (and really not portable)
    library included with my compiler.  

    This is due to the fact that ANSI.SYS wasn't installed on the
    Laptop of Prof. Dr. Brunnstein.  I decided to compile an
    executable that he could use too. Since ANSI.SYS isn't
    installed on all Systems, I included this special version in the
    package.   

    There are two disadvantages however:

        1. If I had asked somebody to compile it on a Borland 
           Compiler, it would be far shorter.
           (This also applies to enigma4j.exe.)

        2. I could't hide the Cursor. It seems to appear always
           at the wrong places.

    To avoid these disadvantages, just compile the source
    as ANSI.SYS Version. On a Borland Compiler 

        bcc enigma4j.c

    should do it. 

           Andreas G.Lessig, B"onningstedt 14.7.1995
           1lessig@rzdspc1.informatik.uni-hamburg.de