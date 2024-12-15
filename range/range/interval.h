struct interval {
     rvar mid, wid;

     interval (rvar&, rvar& = zero);
     interval ();
     interval (interval&);
     
     interval& operator= (interval&);
     friend interval operator+ (interval&, interval&);
     friend interval operator+ (rvar&, interval&);
     friend interval operator- (interval&);
     friend interval operator- (interval&, interval&);
     friend interval operator* (interval&, interval&);
     friend interval operator* (rvar&, interval&);
     friend interval operator/ (interval&, interval&);
     friend interval operator/ (interval&, rvar&);
     friend interval operator^ (interval&, rvar&);
     friend interval operator<< (interval&, rvar&);
     // a << b  for power_even_p_odd_q operation
     friend interval operator>> (interval&, rvar&);
     // a >> b  for power_odd_p_odd_q operation
     
     int has_zero();
     int positive();
     int not_positive();
     int negative();
     friend interval abs (interval&);
     friend interval acos(interval&);
     friend interval asin(interval&);
     friend interval atan(interval&);
     friend interval cos (interval&);
     friend interval cosh(interval&);
     friend interval exp (interval&);
     friend interval log (interval&);
     friend interval sin (interval&);
     friend interval sinh(interval&);
     friend interval sqrt(interval&);
     friend void swap(interval&, interval&);
};     

extern interval I_zero;

#ifdef main_program

interval I_zero(zero, zero);
 
#endif

