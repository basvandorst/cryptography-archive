class rational {
public:
     rational (rvar&, rvar& = one);
     rational (long, long = 1);
     rational ();
     rational (rational&);

     friend class rvar;
     friend rational operator+ (rational&, rational&);
     friend rational operator- (rational&);
     friend rational operator- (rational&, rational&);
     friend rational operator* (rational&, rational&);
     friend rational operator/ (rational&, rational&);
     friend rational operator^ (rational&, rvar&);
     rational& operator= (rational&);
     void operator+= (rational&);
     void operator-= (rational&);
     void operator*= (rational&);
     void operator/= (rational&);
     friend int operator== (rational&, rational&);
     friend int operator!= (rational&, rational&);
     friend int operator>  (rational&, rational&);
     friend int operator>= (rational&, rational&);
     friend int operator<  (rational&, rational&);
     friend int operator<= (rational&, rational&);

     friend rational abs (rational&);
     friend void list_tokens(rational&);
     friend void entry(char*, rational&, svar&, svar& = empty);
     friend void evaluate(rational&, svar&, rational* = 0);
     friend svar rtos (rational&);
     friend void swap (rational&, rational&);

     static int evaluate_error_display;
     // Default is TRUE; causes display of error message and point where
     // error occured in svar of evaluate function.
     
     static int evaluate_exit_on_error;
     // Default is TRUE; 

     static int evaluate_error;
     // set to TRUE or FALSE after evaluate called, according as error
     // was or was not encountered.


private:
     rvar numer;
     rvar denom;

     void reduce();   
};

extern rational r_zero;
extern rational r_one;
extern rational r_num[evaluation_stack_size];

#ifdef main_program  

int rational::evaluate_error_display = TRUE;
int rational::evaluate_exit_on_error = TRUE;
int rational::evaluate_error = FALSE;
rational r_zero = rational(zero);
rational r_one  = rational(one);
rational r_num[evaluation_stack_size];

#endif


