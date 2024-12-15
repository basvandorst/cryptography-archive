class rvector {
public:
     rvector ();
     rvector (rvector&);
     rvector (int);
     
     rvector& operator= (rvector&);
     friend rvector operator+ (rvector&, rvector&);
     friend rvector operator- (rvector&, rvector&);
     friend rvector operator- (rvector&);
     friend rational operator* (rvector&, rvector&);
     rational& operator() (int);
     
     int no_comps();
     void no_comps_reduced_to(int);
     friend rvector copy(rvector&);
     friend void entry(char*, rvector&, svector&);
     friend svector rtos(rvector&);
     friend rvar length(rvector&);
     
     ~rvector ();

private:
     struct rvec {
     char size;
     unsigned count;
     int no_comp;
     rational comp[1];};
     rvec* ve;     
};

inline rational& rvector::operator() (int i) {
     return ve->comp[i];}
inline int rvector::no_comps() {
     return ve->no_comp;}

extern rvector null_rvector;

class rmatrix {
public:
     rmatrix ();
     rmatrix (rmatrix&);
     rmatrix (int, int);
     
     rmatrix& operator= (rmatrix&);
     friend rmatrix operator+ (rmatrix&, rmatrix&);
     friend rmatrix operator- (rmatrix&, rmatrix&);
     friend rmatrix operator- (rmatrix&);
     friend rmatrix operator* (rmatrix&, rmatrix&);
     friend rvector operator* (rmatrix&, rvector&);
     friend rmatrix operator^ (rmatrix&, int);
     friend rvector operator/ (rvector&, rmatrix&);
     rational& operator() (int, int);

     int no_rows();
     int no_cols();
     friend rmatrix copy(rmatrix&);
     friend void entry(char*, rmatrix&, smatrix&);
     friend smatrix rtos(rmatrix&);
     friend rmatrix inverse(rmatrix&);
     friend rational det(rmatrix&);
     
     ~rmatrix ();

private:
     struct rmat {
     char size;
     unsigned count;
     int no_rows;
     int no_cols;
     int no_comp;
     rational comp[1];};
     rmat* ma;
};

inline rational& rmatrix::operator() (int i, int j) {
     return ma->comp[i + j * ma->no_rows];}
inline int rmatrix::no_rows() {
     return ma->no_rows;}
inline int rmatrix::no_cols() {
     return ma->no_cols;}

extern rmatrix null_rmatrix;

#ifdef main_program 

rvector null_rvector = rvector(0);
rmatrix null_rmatrix = rmatrix(0,0);

#endif

