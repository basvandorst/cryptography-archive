class complex {
public:
     complex (rvar&, rvar& = zero);
     complex (long, long = 0) ;
     complex ();
     complex (complex&);

     friend complex operator+ (complex&, complex&);
     friend complex operator- (complex&);
     friend complex operator- (complex&, complex&);
     friend complex operator* (complex&, complex&);
     friend complex operator* (rvar&, complex&);
     friend complex operator/ (complex&, complex&);
     friend complex operator/ (complex&, rvar&);
     friend complex operator^ (complex&, rvar&);
     friend complex operator~ (complex&); // conjugate operator
     friend complex operator% (complex&, complex&);
     complex& operator=(complex&);
     void  operator+= (complex&);
     void  operator-= (complex&);
     void  operator*= (complex&);
     void  operator/= (complex&);
     friend int operator== (complex&, complex&);
     friend int operator!= (complex&, complex&);

     friend rvar abs (complex&);
     friend rvar abs_square (complex&);
     friend complex cos (complex&);
     friend complex cosh (complex&);
     friend complex exp (complex&);
     friend complex sin (complex&);
     friend complex sinh (complex&);
     friend complex tan (complex&);
     friend complex tanh (complex&);
     friend complex mid (complex&);
     friend void swap (complex&, complex&);
     friend void test (complex&, int, int);
     friend svar rtos (complex&, int=0, int=5);
     friend void list_tokens(complex&);
     friend void entry(char*, complex&, svar&, svar& = empty);
     friend void evaluate(complex&, svar&, complex* =0);

     static int evaluate_error_display;
     // Default is TRUE; causes display of error message and point where
     // error occured in svar of evaluate function.
     
     static int evaluate_exit_on_error;
     // Default is TRUE; 

     static int evaluate_error;
     // set to TRUE or FALSE after evaluate called, according as error
     // was or was not encountered.

     rvar real;
     rvar imag;
};

extern complex c_zero;
extern complex c_one;
extern complex c_num[evaluation_stack_size];

#ifdef main_program  

int complex::evaluate_error_display = TRUE;
int complex::evaluate_exit_on_error = TRUE;
int complex::evaluate_error = FALSE;
complex c_zero = complex(zero, zero);
complex c_one  = complex(one, zero);
complex c_num[evaluation_stack_size];

#endif



