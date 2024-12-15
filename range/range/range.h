#include <iostream.h>
#include <stdlib.h>

class memory {
private:
     struct mem {
          char size;
          mem* next;};
     mem* me;

     friend void* get_memory(int);      
     friend void return_memory(void*);
     friend int get_max_len(char);

     static mem* top[12];
     static int no_chars[12];
     static int stacksize[12];

     friend void build_stack(char);

};

#ifdef main_program  

memory::mem* memory::top[12] = {0,0,0,0,0,0,0,0,0,0,0,0};
int memory::no_chars[12] = {8*sizeof(int), 16*sizeof(int), 32*sizeof(int),
64*sizeof(int), 128*sizeof(int), 256*sizeof(int), 512*sizeof(int),
1024*sizeof(int), 2024*sizeof(int), 4096*sizeof(int), 8192*sizeof(int),
10004*sizeof(int)}; // last is for max precision rvar
int memory::stacksize[12] = {200,100,50,25,12,6,3,2,1,1,1,1};

#endif

enum error_type {no_error=0, exit_on_error=0, error_reported=1,
     display_error=2, signal_error=4};

enum boolean {FALSE = 0, TRUE = 1};

class svar;
class rational;
const int evaluation_stack_size = 130; 

class rvar {
public:
     rvar (char*);
     rvar (long);
     rvar ();
     rvar (rvar&);
     rvar (rational&);

     friend class rational;
     friend rvar operator+ (rvar&, rvar&);
     friend rvar operator- (rvar&, rvar&);
     friend rvar operator* (rvar&, rvar&);
     friend rvar operator/ (rvar&, rvar&);
     friend rvar operator% (rvar&, rvar&);
     friend rvar operator^ (rvar&, rvar&);
     friend rvar operator- (rvar&);
     rvar& operator=  (rvar&);
     void  operator+= (rvar&);
     void  operator-= (rvar&);
     void  operator*= (rvar&);
     void  operator/= (rvar&);
     friend int operator== (rvar&, rvar&);
     friend int operator!= (rvar&, rvar&);
     friend int operator< (rvar&, rvar&);
     friend int operator<= (rvar&, rvar&);
     friend int operator> (rvar&, rvar&);
     friend int operator>= (rvar&, rvar&);

     friend rvar abs (rvar&);
     friend rvar acos (rvar&);
     friend rvar asin (rvar&);
     friend rvar atan (rvar&);
     friend rvar cos (rvar&);
     friend rvar cosh (rvar&);
     friend rvar expon_e (rvar&, rvar&); 
     friend rvar expon_o (rvar&, rvar&);
     friend rvar exp (rvar&);
     friend rvar log (rvar&);
     friend rvar max(rvar&, rvar&);
     friend rvar mid (rvar&);
     friend rvar min(rvar&, rvar&);
     friend rvar range (rvar&);
     friend rvar sin (rvar&);
     friend rvar sinh (rvar&);
     friend rvar sqrt (rvar&);
     friend rvar tan (rvar&);
     friend rvar tanh (rvar&);
     friend rvar trunc (rvar&);

     friend void set_precision (unsigned);
     friend void add_precision (unsigned = 0);
     friend unsigned current_precision();
     friend void error(char*);

     friend svar rtos (rvar&, int = 0, int = 5);
     friend void swap (rvar&, rvar&);
     friend void test (rvar&, int, int);
     friend int is_int (rvar&);
     friend int is_zero(rvar&);
     friend rvar div2 (rvar&);

     static int exact_arithmetic;
     // Default is FALSE; if set to TRUE, will maintain exact quantities
     // on operations +, -, *, and div2. Should be set TRUE only temporarily
     // when calculating quantities that must be found exactly
     // regardless of the precision setting. 

     static int strict_ambiguity_treatment;
     // normally FALSE; if set to TRUE, causes error message in various
     // ambiguous situations (asin, acos, sqrt, ^ operation).

     static int evaluate_error_display;
     // Default is TRUE; causes display of error message and point where
     // error occured in svar of evaluate function.
     
     static int evaluate_exit_on_error;
     // Default is TRUE; 

     static int evaluate_error;
     // set to TRUE or FALSE after evaluate called, according as error
     // was or was not encountered.

     ~rvar (); 

     struct rva {
          char size, sgn;
          unsigned count;
          int len, exp, rnge, m[1];};
private:
     rva* rv;
     static int precision, old_precision, max_precision;
     rvar(int, char); //constructor used by operators

     friend void temporary_top_precision();
     friend void restore_precision();
     friend int compare (rvar&, rvar&);
     friend rvar add_sub (rvar&, rvar&, int);
     friend rvar sqrt_rnge (rvar&);
     friend void set_up_mant (int*,rvar&,int,int,int,int,int);
};

inline void temporary_top_precision() {
     rvar::precision = rvar::max_precision;}
inline void restore_precision() {
     rvar::precision = rvar::old_precision;}

extern rvar::rva zero_init;
extern rvar::rva one_init;
extern rvar zero;
extern rvar one;
extern rvar two;
extern rvar ten;
extern rvar tenth;
extern rvar half;
extern rvar pi;
extern rvar logten;
extern rvar num[evaluation_stack_size]; // used by evaluate function
extern int errorstatus;
extern unsigned test_failure;

#ifdef main_program  

int rvar::precision = 3;
int rvar::old_precision = 3;
int rvar::max_precision = 9999;
int rvar::exact_arithmetic = FALSE;
int rvar::strict_ambiguity_treatment = FALSE;
int rvar::evaluate_error_display = TRUE;
int rvar::evaluate_exit_on_error = TRUE;
int rvar::evaluate_error = FALSE;

rvar::rva zero_init = {0,0,1,0,0,0,0};
rvar::rva one_init  = {0,0,1,1,1,0,1};
rvar zero = "0"; 
rvar one = "1";
rvar two = "2";
rvar ten = "10";
rvar tenth = ".1";
rvar half = ".5";

rvar pi = "3.141592653589793238462643383279502884197169399375105820974\
9445923078164062862089986280348253421170679821480865132823066470938446\
0955058223172535940812848111745028410270193852110555964462294895493038\
1964428810975665933446128475648233786783165271201909145648566923460348\
6104543266482133936072602491412737245870066063155881748815209209628292\
5409171536436789259036001133053054882046652138414695194151160943305727\
0365759591953092186117381932611793105118548074462379962749567351885752\
7248912279381830119491298336733624406566430860213949463952247371907021\
7986094370277053921717629317675238467481846766940513200056812714526356\
0827785771342757789609173637178721468440901224953430146549585371050792\
2796892589235420199561121290219608640344181598136297747713099605187072\
1134999999837297804995105973173281609631859502445945534690830264252230\
8253344685035261931188171010003137838752886587533208381420617177669147\
3035982534904287554687311595628638823537875937519577818577805321712268\
066130019278766111959092164201989~"; 

rvar logten = "2.30258509299404568401799145468436420760110148862877297\
6033327900967572609677352480235997205089598298341967784042286248633409\
5254650828067566662873690987816894829072083255546808437998948262331985\
2839350530896537773262884616336622228769821988674654366747440424327436\
5155048934314939391479619404400222105101714174800368808401264708068556\
7743216228355220114804663715659121373450747856947683463616792101806445\
0706480002775026849167465505868569356734206705811364292245544057589257\
2420824131469568901675894025677631135691929203337658714166023010570308\
9634572075440370847469940168269282808481184289314848524948644871927809\
6762712757753970276686059524967166741834857044225071979650047149510504\
9221477656763693866297697952211071826454973477266242570942932258279850\
2585509785265383207606726317164309505995087807523710333101197857547331\
5414218084275438635917781170543098274823850456480190956102992918243182\
3752535770975053956518769751037497088869218020518933950723853920514463\
4197265287286965110862571492198849978~"; 

rvar num[evaluation_stack_size];
int errorstatus = exit_on_error;
unsigned test_failure = 0;
      
#endif

#define four_digits
// Line above sets for code files the number of decimal digits held by an int.
// (4 is always possible).
// Two other possibilities: replace four_digits in line above by:
// six_digits if sizeof(int) >= 3 and sizeof(long) >= 6
// eight_digits if sizeof(int) >= 4 and sizeof(long) >= 8
// The largest number of digits possible should be used for maximum efficiency.
// The program digit_ck.cpp when compiled and run will indicate the best
// setting to use.


// svar is a string class, but for the purpose of holding token symbols, it
// is convenient to allow a null character ('\0') to appear within the string,
// as well as being the usual terminating character.  The various entry
// functions append the token symbols after the keyboard entry characters,
// and use a null character to separate the two parts,  so when the svar is
// displayed at the console, or read into a file, the token symbols will not
// appear.
extern svar empty;
class svar {
public:
     svar (char*);
     svar (int); // if int = n, string has n chars, otherwise indeterminate
     svar ();
     svar (svar&);

     friend svar operator+ (svar&, svar&);
     friend void operator+= (svar&, svar&);
     svar& operator= (svar&);
     friend int  operator== (svar&, svar&);
     friend int  operator!= (svar&, svar&);
     friend char* operator! (svar&);
     friend ostream& operator<< (ostream&, svar&);
     int len();
     char* tokens();
     friend svar rtos (rvar&, int, int);
     friend svar ltos (long); // converts number to svar for output
     friend void test (rvar&, int, int);
     friend void compile(svar&, svar& = empty);

     static int input_from_file;
     static int input_logs_allowed;
     
     ~svar();

     struct sva {
          char size;
          unsigned count;
          int len;
          int max_len;
          char m[1];};

private:
     sva* sv;


     friend void examine (svar&, int);
};

inline char* operator! (svar& a) {return a.sv->m;}
inline int svar::len() {return sv->len;}

extern svar::sva empty_init;

#ifdef main_program  

svar::sva empty_init = {0,1,0,0,0};
svar empty = "";
int svar::input_from_file = FALSE;
int svar::input_logs_allowed = TRUE;  // FALSE prevents program logs

#endif

void show_error_pos(svar&, unsigned);   // private function

// following public functions are for console input
void svar_entry(char*, svar&);
void int_entry(char*, int&, int, int, char* = "", char* = "");
void entry(char*, rvar&, svar&, svar& = empty);
void evaluate(rvar&, svar&, rvar* = 0);
void list_tokens(rvar&);

enum code_types {
     abs_ = 'A', acos_ = 'B', asin_ = 'C', atan_ = 'D', const_ = 'E',
     cos_ = 'F', cosh_ = 'G', end_ = 'H', exp_ = 'I', log_ = 'J',
     pi_const_ = 'K', power_even_p_odd_q = 'L', power_odd_p_odd_q = 'M',
     sin_ = 'N', sinh_ = 'O', sqrt_ = 'P', tan_ = 'Q', tanh_ = 'R',
     unary_minus_ = 'S', var_ = 'T'};

