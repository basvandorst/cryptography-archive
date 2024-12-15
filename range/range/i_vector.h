class Ivector {
public:
     Ivector ();
     Ivector (Ivector&);
     Ivector (int);
     
     Ivector& operator= (Ivector&);
     interval& operator() (int);
     
     int no_comps();
     
     ~Ivector ();

private:
     struct vec {
          char size;
          unsigned count;
          int no_comp;
          interval comp[1];};
     vec* ve;     
};

inline interval& Ivector::operator() (int i) {
     return ve->comp[i];}
inline int Ivector::no_comps() {
     return ve->no_comp;}
     
extern Ivector null_Ivector;

class Imatrix {
public:
     Imatrix ();
     Imatrix (Imatrix&);
     Imatrix (int, int);
     
     Imatrix& operator= (Imatrix&);
     interval& operator() (int, int);

     int no_rows();
     int no_cols();
     
     ~Imatrix ();

private:
     struct mat {
          char size;
          unsigned count;
          int no_rows;
          int no_cols;
          int no_comp;
          interval comp[1];};
     mat* ma;
};

inline interval& Imatrix::operator() (int i, int j) {
     return ma->comp[i + j * ma->no_rows];}
inline int Imatrix::no_rows() {
     return ma->no_rows;}
inline int Imatrix::no_cols() {
     return ma->no_cols;}

extern Imatrix null_Imatrix;

#ifdef main_program

Ivector null_Ivector = Ivector(0);
Imatrix null_Imatrix = Imatrix(0,0);

#endif
