void find_roots(cvector&, cvector&, ivector&, int);

/*
find_roots(cvector&, cvector&, ivector&, int)

     You supply the  coefficients of a complex monic polynomial as
     a cvector p with n components, with p(i), i=0 to n-1,
     equal to the polynomial's z^i coefficient. You designate a cvector
     to hold the roots, and an ivector to hold the multiplicity values.

     On return the number of components of the cvector and the ivector 
     match the number of distinct roots found.
     The ith component of the cvector gives the ith root approximation,
     and the ith component of the ivector gives the apparent
     multiplicity of this root.  This multiplicity is not necessarily
     the true multiplicity, since its computed value depends
     on the precision of computation, which you control.
     
     If the integer argument is zero, the range of each root
     approximation is not computed and is set to zero.

     If the integer argument is not zero, the computed range of each root
     bounds the error of the root. Also the order of the roots in the cvector
     is rearranged, so that root(i) has a greater real part than
     root(i+1), or, if their real parts are equal, the imaginary part of
     root(i) is greater. 

*/

