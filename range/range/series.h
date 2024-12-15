enum series_type {
         x_type = 0X1001,
         y_type = 0X1010,
         z_type = 0X1100,
         e_test = 0X0111,
         x_part = 0X000F,
         y_part = 0X00F0,
         z_part = 0X0F00,
       deg_part = 0XF000,
        deg_one = 0X1000,
       deg_high = 0XFFFF,
     max_factor = 0X1111};

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
     // above operator to be used  for obtaining term, but not for changing it

     static void initiate(int, unsigned, series*);
     // must call above function before using series.
     // first arg = 1 for x series, 2 for x, y series, 3 for x, y, z series, 
     // second arg gives max deg possible -- 14 is the limit 
     // third argument to hold x, y, and z series, previously declared.

     static void set_max_deg(int);
     // max_deg can be changed at any time, but can not exceed the max deg
     // fixed by initiate().  max_deg determines the maximum degree computed.

     void const_term(rvar&, rvar& = zero); // enters mid, wid in const term
     // use this member for changing constant term of a series
     friend interval integrate(series&, rvar&, rvar& = one, rvar& = one);
     friend void test (series&, int, int);
     friend void evaluate (series&, svar&, series*);

     friend series abs (series&);
     friend series atrig (series&, int);
     // int code can equal acos_, asin_, or atan_

     friend series exp (series&);
     friend series factorial_extend(series&);
     // multiplies the coefficient of x^i * y^j * z^k by i! * j! * k!
     // used in deriv program

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
          unsigned xyz; 
          interval mw; 
          int tnext;};
// xyz entry holds degree, z value, y value, and x value, each in  4 bits
// tnext is an integer pointer to next series term; tnext = 0 if term is last
     struct ser {
          unsigned count;
          ser* next;
          term comp[1];};
     ser* se;
     static int max_no_terms;
     static int max_size;
     static int max_deg_bound;
     static unsigned max_deg;
     static ser* sstack;
     series (ser*&);
};

extern series zero_series;
extern rvar rvar_int[17];
extern rvar x_power[16];
extern rvar y_power[16];
extern rvar z_power[16];
extern series sri[evaluation_stack_size];

#ifdef main_program

int series::max_no_terms = 0;
int series::max_size = 0;
int series::max_deg_bound = 0;
int series::evaluate_error_display = TRUE;
int series::evaluate_exit_on_error = TRUE;
int series::evaluate_error = FALSE;
unsigned series::max_deg = 0;
series::ser* series::sstack = 0;
series zero_series;
rvar rvar_int[17];
rvar x_power[16];
rvar y_power[16];
rvar z_power[16];
series sri[evaluation_stack_size];

#endif

