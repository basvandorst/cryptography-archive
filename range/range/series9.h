const long max_factor = 0X15555555;
const long deg_high   = 0X3FFFFFFF;
const long deg_part   = 0X30000000;
const long deg_one    = 0X10000000;

// series with up to 14 variables, but the power of each variable <= 3
class series {
public:
     series (rvar&);   // creates constant series of a single term  
     series ();
     series (series&);
     
     series& operator= (series&);

     friend series operator+ (series&, series&);
     friend series operator+ (rvar&, series&);
     friend series operator- (series&);
     friend series operator- (series&, series&);
     friend series operator* (series&, series&);
     friend series operator* (rvar&, series&);
     friend series operator/ (series&, series&);
     friend series operator^ (series&, series&);
     interval& operator() (int, int, int);
     // Above operator to be used for obtaining a derivative of series,
     // or the constant term, but not for changing terms.
     // First int is order of derivative, second and third int define the term
     // If first int is zero, nothing else is needed; term is constant term
     // If first int is one, second int specifies which partial derivative
     // If first int is two, second and third ints specify which partial

     static void initiate(int, int, series*); // must call before using series
     // first arg = number of variables, n, between 1 and 14
     // second arg gives max deg possible -- 3 is the limit 
     // third argument to hold variables as series,
     // previously declared as series[n]

     static void set_max_deg(int);
     // max_deg can be changed at any time, but can not exceed the max deg
     // fixed by initiate(). max_deg is the maximum degree computed.

     void const_term(rvar&, rvar& = zero); // enters mid, wid in const term
     // use this member for changing constant term of a series
     friend void test (series&, int, int);
     friend void evaluate (series&, svar&, series*);

     friend series abs (series&);
     friend series atrig (series&, int);
     // int code can equal acos_, asin_, or atan_

     friend series exp (series&);
     friend series integer_power (series&, rvar&);
     // rvar argument is the power, which must equal an exact integer

     friend series log (series&);
     friend series power (series&, rvar&, int);
     // rvar argument is the power
     // int code can equal even_num_odd_denom, odd_num_odd_denom, or rvar_

     friend series sin_cos (series&, int);
     // int code can equal sin_, cos_, or tan_

     friend series sinh_cosh (series&, int);
     // int code can equal sinh_, cosh_, or tanh_

     static int evaluate_error_display;
     // Default is TRUE; causes display of error message and point where
     // error occured in svar of evaluate function.
     
     static int evaluate_exit_on_error;
     // Default is TRUE; 

     static int evaluate_error;
     // set to TRUE or FALSE after evaluate called, according as error
     // was or was not encountered.

     ~series ();

private:
     friend void make_series();

     struct term {
          long xyz;
          interval mw;
          int tnext;};
// xyz entry holds degree in code, 2 bits alloted to each variable
// tnext is an integer pointer to next series term; tnext = 0 if term is last
     struct ser {
          unsigned count;
          ser* next;
          term comp[1];};
     ser* se;
     static int max_no_terms;
     static int max_size;
     static int max_deg_bound;
     static long max_deg;
     static ser* sstack;
     series (ser*&);
};

extern series zero_series;
extern rvar rvar_int[4];
extern series sri[evaluation_stack_size];

#ifdef main_program

int series::max_no_terms = 0;
int series::max_size = 0;
int series::max_deg_bound = 0;
long series::max_deg = 0;
int series::evaluate_error_display = TRUE;
int series::evaluate_exit_on_error = TRUE;
int series::evaluate_error = FALSE;
series::ser* series::sstack = 0;
series zero_series;
rvar rvar_int[4];
series sri[evaluation_stack_size];

#endif

