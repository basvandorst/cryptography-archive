class svector {
public:
     svector ();
     svector (svector&);
     svector (int);
     
     svector& operator= (svector&);
     svar& operator() (int);

     int dim();
     void no_comps_reduced_to(int);
     friend svector copy(svector&);

     ~svector ();

private:
     struct svec {
          char size;
          unsigned count;
          int no_comp;
          svar comp[1];};
     svec* ve;     
};

inline svar& svector::operator() (int i) {
     return ve->comp[i];}
inline int svector::dim() {
     return ve->no_comp;}

extern svector null_svector;

class smatrix {
public:
     smatrix ();
     smatrix (smatrix&);
     smatrix (int, int);

     smatrix& operator= (smatrix&);
     svar& operator() (int, int) ;

     int dim(int);
     friend smatrix copy(smatrix&);

     ~smatrix ();

private:
     struct smat {
          char size;
          unsigned count;
          int no_rows;
          int no_cols;
          int no_comp;
          svar comp[1];};
     smat* ma;
};

inline svar& smatrix::operator() (int i, int j) {
     return ma->comp[i + j * ma->no_rows];}

extern smatrix null_smatrix;

#ifdef main_program

svector null_svector = svector(0);
smatrix null_smatrix = smatrix(0,0);

#endif

class vector {
public:
     vector ();
     vector (vector&);
     vector (int);
     
     vector& operator= (vector&);
     friend vector operator+ (vector&, vector&);
     friend vector operator- (vector&, vector&);
     friend vector operator- (vector&);
     friend rvar operator* (vector&, vector&);
     rvar& operator() (int);
     
     int no_comps();
     void no_comps_reduced_to(int);
     friend vector copy(vector&);
     friend void entry(char*, vector&, svector&);
     friend vector mid(vector&);
     friend void    test(vector&, int, int);
     friend svector rtos(vector&, int, int);
     friend void evaluate(vector&, svector&);
     friend rvar length(vector&);
     
     ~vector ();

private:
     struct vec {
          char size;
          unsigned count;
          int no_comp;
          rvar comp[1];};
     vec* ve;     
};

inline rvar& vector::operator() (int i) {
     return ve->comp[i];}
inline int vector::no_comps() {
     return ve->no_comp;}
     
extern vector null_vector;

class matrix {
public:
     matrix ();
     matrix (matrix&);
     matrix (int, int);
     
     matrix& operator= (matrix&);
     friend matrix operator+ (matrix&, matrix&);
     friend matrix operator- (matrix&, matrix&);
     friend matrix operator- (matrix&);
     friend matrix operator* (matrix&, matrix&);
     friend vector operator* (matrix&, vector&);
     friend matrix operator^ (matrix&, int);
     friend vector operator/ (vector&, matrix&);
     rvar& operator() (int, int);

     int no_rows();
     int no_cols();
     static int failure_report_wanted;
     static int failure;
     friend matrix copy(matrix&);
     friend void entry(char*, matrix&, smatrix&);
     friend void test(matrix&, int, int);
     friend smatrix rtos(matrix&, int, int);
     friend void evaluate(matrix&, smatrix&);
     friend matrix inverse(matrix&);
     friend rvar det(matrix&);
     
     ~matrix ();

private:
     struct mat {
          char size;
          unsigned count;
          int no_rows;
          int no_cols;
          int no_comp;
          rvar comp[1];};
     mat* ma;
};

inline rvar& matrix::operator() (int i, int j) {
     return ma->comp[i + j * ma->no_rows];}
inline int matrix::no_rows() {
     return ma->no_rows;}
inline int matrix::no_cols() {
     return ma->no_cols;}

extern matrix null_matrix;

#ifdef main_program

vector null_vector = vector(0);
matrix null_matrix = matrix(0,0);
int matrix::failure_report_wanted = FALSE;
int matrix::failure = FALSE;

#endif
