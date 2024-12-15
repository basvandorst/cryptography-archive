class ivector {
public:
     ivector ();
     ivector (ivector&);
     ivector (int);
     
     ivector& operator= (ivector&);
     friend ivector operator+ (ivector&, ivector&);
     friend ivector operator- (ivector&, ivector&);
     friend ivector operator- (ivector&);
     friend int operator* (ivector&, ivector&);
     int& operator() (int);
     
     int no_comps();
     void no_comps_reduced_to(int);
     friend ivector copy(ivector&);

     ~ivector ();

private:
     struct ivec {
          char size;
          unsigned count;
          int no_comp;
          int comp[1];};
     ivec* ve;     
};

inline int& ivector::operator() (int i) {
     return ve->comp[i];}
inline int ivector::no_comps() {
     return ve->no_comp;}

extern ivector null_ivector;

class imatrix {
public:
     imatrix ();
     imatrix (imatrix&);
     imatrix (int, int);
     
     imatrix& operator= (imatrix&);
     friend imatrix operator+ (imatrix&, imatrix&);
     friend imatrix operator- (imatrix&, imatrix&);
     friend imatrix operator- (imatrix&);
     friend imatrix operator* (imatrix&, imatrix&);
     friend ivector operator* (imatrix&, ivector&);
     int& operator() (int, int);

     int no_rows();
     int no_cols();
     friend imatrix copy(imatrix&);
     
     ~imatrix ();

private:
     struct imat {
          char size;
          unsigned count;
          int no_rows;
          int no_cols;
          int no_comp;
          int comp[1];};
     imat* ma;
};

inline int& imatrix::operator() (int i, int j) {
     return ma->comp[i + j * ma->no_rows];}
inline int imatrix::no_rows() {
     return ma->no_rows;}
inline int imatrix::no_cols() {
     return ma->no_cols;}

extern imatrix null_imatrix;

#ifdef main_program

ivector null_ivector = ivector(0);
imatrix null_imatrix = imatrix(0,0);

#endif

