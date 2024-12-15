class series {// termwise series computation for differential equations
public:
     series (int);
     series ();
     series (series&);
     
     interval& operator() (int);
     interval& operator() (int, int); // first int specifies x, second u
     series& operator= (series&);

     static int error;
     static int function_count;
     static int num_u_variables;

     friend void prepare_series (svector&, series*, series*, series*);
     friend void compute_series_constants();
     friend void series_computation (int);
     friend void series_computation1 (int);
     friend series new_series(int, rvar& = zero);

     ~series ();

private:
     struct ser {
          char size;
          unsigned count;
          int num_terms;
          interval term[1];};
     ser* se;
};
const int max_x_deg = 14, max_x_deg1 = max_x_deg-1, max_x_deg2 = max_x_deg-2;
const int x_len = max_x_deg + 1;

const int operation_max = 800;
extern series zero_series;
extern int op_count;
extern int op1_count;
extern char op[operation_max];
extern char op1[operation_max];
extern series v1[operation_max];
extern series v2[operation_max];
extern series v3[operation_max];
extern series w1[operation_max];
extern series w2[operation_max];
extern series w3[operation_max];
extern rvar integ[max_x_deg+1];

#ifdef main_program

int series::error = FALSE;
int series::function_count  = 0;
int series::num_u_variables = 0;
series zero_series(0);
int op_count  = 0;
int op1_count = 0;
char op[ operation_max];
char op1[operation_max];
series v1[operation_max];
series v2[operation_max];
series v3[operation_max];
series w1[operation_max];
series w2[operation_max];
series w3[operation_max];
rvar integ[max_x_deg+1];

#endif

