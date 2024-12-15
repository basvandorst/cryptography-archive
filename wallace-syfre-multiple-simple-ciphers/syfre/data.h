// data.h -- class definitions for class "Data".
//
// written by Marc Wallace, 30 Apr 93.
// last change: version 1.0 (02 May 93).


class Data
    {
    public:
        unsigned char*  data;
        int    size;
        int    maxsize;

        Data(int m);
        Data(int m, int s, unsigned char* d);

        void reinit(int m);
        void reinit(int m, int s, unsigned char* d);
    };


