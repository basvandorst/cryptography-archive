class cvector {
public:
     cvector ();
     cvector (cvector&);
     cvector (int);
     
     cvector& operator= (cvector&);
     friend cvector operator+ (cvector&, cvector&);
     friend cvector operator- (cvector&, cvector&);
     friend cvector operator- (cvector&);
     friend complex operator* (cvector&, cvector&);
     complex& operator() (int);
     
     int no_comps();
     void no_comps_reduced_to(int);
     friend cvector copy(cvector&);
     friend void entry(char*, cvector&, svector&);
     friend void    test(cvector&, int, int);
     friend svector rtos(cvector&, int, int);
     friend void evaluate(cvector&, svector&);
     friend rvar length(cvector&);
     
     ~cvector ();

private:
     struct cvec {
          char size;
          unsigned count;
          int no_comp;
          complex comp[1];};
     cvec* ve;     
};

inline complex& cvector::operator() (int i) {
     return ve->comp[i];}
inline int cvector::no_comps() {
     return ve->no_comp;}

extern cvector null_cvector;

class cmatrix {
public:
     cmatrix ();
     cmatrix (cmatrix&);
     cmatrix (int, int);
     
     cmatrix& operator= (cmatrix&);
     friend cmatrix operator+ (cmatrix&, cmatrix&);
     friend cmatrix operator- (cmatrix&, cmatrix&);
     friend cmatrix operator- (cmatrix&);
     friend cmatrix operator* (cmatrix&, cmatrix&);
     friend cvector operator* (cmatrix&, cvector&);
     friend cmatrix operator^ (cmatrix&, int);
     friend cvector operator/ (cvector&, cmatrix&);
     complex& operator() (int, int);

     int no_rows();
     int no_cols();
     static int failure_report_wanted;
     static int failure;
     friend cmatrix copy(cmatrix&);
     friend void entry(char*, cmatrix&, smatrix&);
     friend void test(cmatrix&, int, int);
     friend smatrix rtos(cmatrix&, int, int);
     friend void evaluate(cmatrix&, smatrix&);
     friend cmatrix inverse(cmatrix&);
     friend complex det(cmatrix&);
     
     ~cmatrix ();

private:
     struct cmat {
          char size;
          unsigned count;
          int no_rows;
          int no_cols;
          int no_comp;
          complex comp[1];};
     cmat* ma;
};

inline complex& cmatrix::operator() (int i, int j) {
     return ma->comp[i + j * ma->no_rows];}
inline int cmatrix::no_rows() {
     return ma->no_rows;}
inline int cmatrix::no_cols() {
     return ma->no_cols;}

extern cmatrix null_cmatrix;

#ifdef main_program

cvector null_cvector = cvector(0);
cmatrix null_cmatrix = cmatrix(0,0);
int cmatrix::failure_report_wanted = FALSE;
int cmatrix::failure = FALSE;

#endif

