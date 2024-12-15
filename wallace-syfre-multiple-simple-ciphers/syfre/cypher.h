// cypher.h -- class definition for class "cypher".
//
// written by Marc Wallace, 26 Apr 93.
// last change: version 1.1 (14 May 93).


// always include data.h before this file.


typedef Data& (*Cyfunct)(Data&, Data&);
extern void nuthin();


class Cypher
    {
    public:
        Cyfunct encode;  //  Data& (*encode)(Data& in, Data& out);
        Cyfunct decode;  //  Data& (*decode)(Data& in, Data& out);

        // Cypher class initializer.
        // Cypher("name", encode(), decode(), init())
        // init() is an (optional) function to be run for data initialization.
        Cypher(char* n, Cyfunct e, Cyfunct d, void (*i)() = nuthin);
        char  *name;
    };


