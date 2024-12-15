#include <string.h>

int no_rvar_error(svar&);          // used by entry function
void process_exponential (svar&);  // used by entry function

enum states {PREFIX, OPERAND, OPERATION, DIGITS, DECIMAL_DIGITS, E_CHECK,
     E_SIGN, EXPECTING_E_DIGITS, E_DIGITS, CYCLE_START};

enum {const_error_ = 'U', error_ = 'V'};
void compile(svar& entry_line, svar& variables)
{
int e_len = entry_line.len(), v_count, v_len, v_len1, v_no, done = FALSE,
     state, t_add;
svar a(e_len), t(e_len), ta1(e_len), ta2(e_len), tv(e_len);
rvar r;
char *p = !a, *p0 = p, *tp = !t, *ta1p = !ta1, *ta2p = !ta2,
     *tvp = !tv, *vp, *vp1, c;

// get count of variables
v_count = 0;
vp = !variables; c = *vp; if (c) v_count++;
while (c) {
     if (c == ',') v_count++; c = *++vp;}

// read entry_line into svar a and change capital letters
// to small letters
vp = !entry_line; c = *vp; vp1 = !a;
while (c) {
     if (c >= 'A' && c <= 'Z') c = c - 'A' + 'a';
     *vp1++ = c; c = *++vp;}
*vp1 = '\0';
     
// Get token list in svar t, token char addresses in svars ta1 and ta2
// two bytes are used for char addresses
state = CYCLE_START; int i = 0;
do {
     t_add = p - p0; ta1p[i] = (t_add & 0xfff0) >> 7; ta2p[i] = t_add & 0x7f;
     switch (*p) {
          case ' ':
          if (state == DIGITS || state == DECIMAL_DIGITS) {
               state = E_CHECK; p++; break;}
          else if (state == E_DIGITS) {
               state = CYCLE_START; p++; break;}
          else {
               p++; break;}
          
          case '\0': tp[i] = end_; done = TRUE; break;
          
          case '+': case '-': 
          if (state == E_SIGN) {
               state = EXPECTING_E_DIGITS; p++; break;}
          
          case '*': case '/': case '^': case '(': case ')':
          state = CYCLE_START; tp[i++] = *p++; break;
          
          case '0': case '1': case '2': case '3': case '4': 
          case '5': case '6': case '7': case '8': case '9':
          if (state == CYCLE_START) {
               errorstatus = signal_error; r = rvar(p);
               if (errorstatus == error_reported) {
                    tp[i++] = const_error_; done = TRUE;}
               else tp[i++] = const_;
               errorstatus = exit_on_error;
               state = DIGITS; p++; break;}
          else if (state == E_SIGN || state == EXPECTING_E_DIGITS) {
               state = E_DIGITS; p++; break;}
          else if (state == E_CHECK) {
               state = CYCLE_START; break;}
          else {
               p++; break;}
          
          case '.': 
          if (state == CYCLE_START) {
               errorstatus = signal_error; r = rvar(p);
               if (errorstatus == error_reported) {
                    tp[i++] = const_error_; done = TRUE;}
               else tp[i++] = const_;
               errorstatus = exit_on_error;
               state = DECIMAL_DIGITS; p++; break;}
          else if (state == DIGITS) {
               state = DECIMAL_DIGITS; p++; break;}
          else state = CYCLE_START; break;
          
          case 'a':
          state = CYCLE_START;
          if (strncmp(p,"abs",3) == 0) {
               tp[i++] = abs_; p += 3; break;}
          if (strncmp(p,"acos",4) == 0) {
               tp[i++] = acos_; p += 4; break;}
          if (strncmp(p,"asin",4) == 0) {
               tp[i++] = asin_; p += 4; break;}
          if (strncmp(p,"atan",4) == 0) {
               tp[i++] = atan_; p += 4; break;}
          goto def;
          
          case 'c':
          state = CYCLE_START;
          if (strncmp(p,"cosh",4) == 0) {
               tp[i++] = cosh_; p += 4; break;}
          if (strncmp(p,"cos",3) == 0) {
               tp[i++] = cos_; p += 3; break;}
          goto def;
          
          case 'e':
          if (state == DIGITS || state == DECIMAL_DIGITS || state == E_CHECK) {
               state = E_SIGN; p++; break;}
          state = CYCLE_START;
          if (strncmp(p,"exp",3) == 0) {
               tp[i++] = exp_; p += 3; break;}
          goto def;
          
          case 'i':
          state = CYCLE_START;
          c = *(p+1);
          if (c >= 'a' && c <= 'z') goto def;
          if (c >= '0' && c <= '9') goto def;
          tp[i++] = *p++; break;
          
          case 'l':
          state = CYCLE_START;
          if (strncmp(p, "log",3) == 0) {
               tp[i++] = log_; p += 3; break;}
          if (strncmp(p,"ln",2) == 0) {
               tp[i++] = log_; p += 2; break;}
          goto def;
          
          case 'p':
          state = CYCLE_START;
          if (strncmp(p,"pi",2) == 0) {
               tp[i++] = pi_const_; p += 2; break;}
          goto def;
         
          case 's':
          state = CYCLE_START;
          if (strncmp(p,"sinh",4) == 0) {
               tp[i++] = sinh_; p += 4; break;}
          if (strncmp(p,"sin",3) == 0) {
               tp[i++] = sin_; p += 3; break;}
          if (strncmp(p,"sqrt",4) == 0) {
                    tp[i++] = sqrt_; p += 4; break;}
          goto def;
          
          case 't':
          state = CYCLE_START;
          if (strncmp(p,"tanh",4) == 0) {
               tp[i++] = tanh_; p += 4; break;}
          if (strncmp(p,"tan",3) == 0) {
               tp[i++] = tan_; p += 3;  break;}
          
          def:
          default:
          state = CYCLE_START;
          if (*p >= 'a' && *p <= 'z') {
               vp = p + 1; c = *vp; v_len1 = 1;
               while ((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9')
               || (c == '\'')) {
                    v_len1++; c = *++vp;}}
          else v_len1 = -1;
          vp = !variables; v_no = 0;
          while (v_no < v_count) {
               while (*vp == ' ') vp++;
               vp1 = vp; c = *vp1;
               while (c != '\0' && c != ',' && c != ' ') c = *++vp1;
               v_len = vp1 - vp;
               if (v_len == v_len1 && strncmp(p,vp,v_len) == 0) {
                    tvp[i] = v_no; // v_no replaces char address later
                    tp[i++] = var_; p += v_len; break;}
               vp += v_len;
               while (*vp != ',' && *vp != '\0') vp++;
               v_no++; vp++;}
          if (v_no >= v_count) {
               tp[i] = error_; done = TRUE;}}}
while (!done);

// Next go through tokens and arrange them on svar list in execution order.
// svar o is a stack of pending operations (addresses in svars oa1 and oa2).

int alert1 = FALSE, alert2 = FALSE, i1,
     parenth_count = 0, parenth_pos, stack_count;
svar o(e_len), oa1(e_len), oa2(e_len), list(3*e_len), s;
char *op = !o, *oa1p = !oa1, *oa2p = !oa2, *listp = !list, d;

op[0] = end_; state = PREFIX;
errorstatus = display_error; i = -1; int j = 0, k = 0;
while (1) {
     c = tp[++i];
     if (state == PREFIX) switch (c) {
          case '-':
          op[++j] = unary_minus_; oa1p[j] = ta1p[i]; oa2p[j] = ta2p[i];
          
          case '+': state = OPERAND; break;
          
          default:
          i--; state = OPERAND;}
     else if (state == OPERAND) switch (c) {
          case '(':
          op[++j] = c; oa1p[j] = ta1p[i]; oa2p[j] = ta2p[i];
          if (parenth_count++ == 0) parenth_pos = j;
          state = PREFIX; break;
          
          case pi_const_: case const_: case 'i':
          listp[k++] = c; listp[k++] = ta1p[i]; listp[k++] = ta2p[i];
          state = OPERATION; break;
          
          case var_: // v_no replaces char address here
          listp[k++] = c; listp[k++] = '\0'; listp[k++] = tvp[i];
          state = OPERATION; break;
          
          case const_error_:
          error("syntax of number construction"); break;
          
          case abs_: case acos_: case asin_: case atan_:
          case cosh_: case cos_: case exp_: case log_:
          case sinh_: case sin_: case sqrt_: case tanh_: case tan_:
          if (tp[i+1] != '(' ) {
               error("argument parenthesis missing"); break;}
          op[++j] = '('; oa1p[j] = ta1p[i+1]; oa2p[j] = ta2p[i+1];
          if (parenth_count++ == 0) parenth_pos = j;
          op[++j] = c; oa1p[j] = ta1p[i]; oa2p[j] = ta2p[i];
          i++; state = PREFIX; break;
          
          default: error("syntax");}
     else if (state == OPERATION) switch (c) {
          case '+': case '-':
          d = op[j];
          while ( d == '+' || d == '-' || d == '*' || d == '/'
          || d == '^' || d == unary_minus_) {
               listp[k++] = d; listp[k++] = oa1p[j]; listp[k++] = oa2p[j];
               d = op[--j];}
          op[++j] = c; oa1p[j] = ta1p[i]; oa2p[j] = ta2p[i]; state = OPERAND;
          break;
          
          case '/': alert1 = TRUE; case '*':
          d = op[j];
          while ( d == '*' || d == '/' || d == '^' || d == unary_minus_) {
               listp[k++] = d; listp[k++] = oa1p[j]; listp[k++] = oa2p[j];
               d = op[--j];}
          op[++j] = c; oa1p[j] = ta1p[i]; oa2p[j] = ta2p[i]; state = PREFIX;
          break;
          
          case '^': alert2 = TRUE;
          op[++j] = c;  oa1p[j] = ta1p[i]; oa2p[j] = ta2p[i]; state = PREFIX;
          break;
          
          case ')':
          if (parenth_count-- < 1) {
               error("missing opening left parenthesis"); break;}
          d = op[j];
          while (d != '(' ) {
               listp[k++] = d; listp[k++] = oa1p[j]; listp[k++] = oa2p[j];
               d = op[--j];}
          j--; break;
          
          case end_:
          if (parenth_count > 0) {
               error("missing closing right parenthesis");
               ta1p[i] = oa1p[parenth_pos]; ta2p[i] = oa2p[parenth_pos];
               break;}
          d = op[j];
          while (d != end_) {
               listp[k++] = d; listp[k++] = oa1p[j]; listp[k++] = oa2p[j];
               d = op[--j];}
          stack_count = 0;
          for (j=0; j<k; j+=3) {
               d = listp[j];
               switch (d) {
                    case const_: case var_: case pi_const_:
                    if (++stack_count > evaluation_stack_size) {
                         error("required evaluation stack too large");}
                    break;
                    
                    case '+': case '-': case '*': case '/': case '^':
                    stack_count--;}}
          if (errorstatus == error_reported) break;
          errorstatus = no_error; s = svar(e_len + k + 5);
          vp = !entry_line; vp1 = !s;
          for (i1=0; i1<=e_len; i1++) *vp1++ = *vp++;
          vp = !list; 
          for (i1=0; i1<k; i1++) *vp1++ = *vp++;
          *vp1++ = end_;
          *vp1++ = (++e_len & 0xfff0) >> 7;
          *vp1++ = e_len & 0x7f; *vp1++ = '\0'; *vp1 = '\0';
          entry_line = s;
          if (alert1 & alert2) process_exponential(entry_line);
          return;
          default: error("syntax");}
     if (errorstatus == error_reported) {
          show_error_pos (entry_line, (ta1p[i] << 7) + ta2p[i]);
          return;}}
}

void entry(char* message, rvar& cc, svar& entry_line, svar& variables)
{
int i = rvar::evaluate_error_display, j = rvar::evaluate_exit_on_error;
// save the evaluate settings; entry needs to impose own conditions
rvar::evaluate_error_display = TRUE;
rvar::evaluate_exit_on_error = FALSE;

int programmed_entry_line, first_cycle = TRUE;
if (*message) programmed_entry_line = FALSE; else programmed_entry_line = TRUE;
     
while (1) {
     if (programmed_entry_line) {
          if (first_cycle) first_cycle = FALSE;
          else exit(1);}
     else svar_entry(message, entry_line);
     compile(entry_line, variables);
     if (errorstatus == no_error) {
          if (no_rvar_error(entry_line)) {
               if (variables == empty) { // constant entry
                    evaluate (cc, entry_line); 
                    if (rvar::evaluate_error == FALSE) break;}
               else break;}}}
rvar::evaluate_error_display = i;
rvar::evaluate_exit_on_error = j;
}

void evaluate (rvar& result, svar& entry_line, rvar* x) 
{
char c, *p = entry_line.tokens(), *q = !entry_line, *q1;
int i = -1; rvar r;
if (rvar::evaluate_error_display == TRUE) errorstatus = display_error;
else if (rvar::evaluate_exit_on_error == FALSE) errorstatus = signal_error;
else errorstatus = exit_on_error;
while (1) {
     c = *p;
     switch (c) {
          case end_: rvar::evaluate_error = FALSE; result = num[0];
          errorstatus = exit_on_error; return;
          
          case const_: q1 = q + ((*(p+1) << 7) + *(p+2));
          num[++i] = rvar(q1); break;
          
          case pi_const_: num[++i] = pi; break;
          case var_: num[++i] = x[*(p+2)]; break;
          case unary_minus_: num[i] = - num[i]; break;
          case abs_: num[i] = abs(num[i]); break;
          case acos_: num[i] = acos(num[i]); break;
          case asin_: num[i] = asin(num[i]); break;
          case atan_: num[i] = atan(num[i]); break;
          case cos_: num[i] = cos(num[i]); break;
          case cosh_: num[i] = cosh(num[i]); break;
          case exp_: num[i] = exp(num[i]); break;
          case log_: num[i] = log(num[i]); break;
          case sin_: num[i] = sin(num[i]); break;
          case sinh_: num[i] = sinh(num[i]); break;
          case sqrt_: num[i] = sqrt(num[i]); break;
          case tan_: num[i] = tan(num[i]); break;
          case tanh_: num[i] = tanh(num[i]); break;
          case '+': r = num[i--]; num[i] += r; break;
          case '-': r = num[i--]; num[i] -= r; break;
          case '*': r = num[i--]; num[i] *= r; break;
          case '/': r = num[i--]; num[i] /= r; break;
          case '^': r = num[i--]; num[i] = num[i] ^ r; break;
          case power_odd_p_odd_q: r = num[i--]; num[i] = expon_o(num[i], r);
          break;
          case power_even_p_odd_q: r = num[i--]; num[i] = expon_e(num[i], r);
          break;
          default: error("rvar specification faulty");}
     if (errorstatus == error_reported) {
          rvar::evaluate_error = TRUE;
          if (rvar::evaluate_error_display == TRUE) {
               show_error_pos(entry_line, (*(p+1) << 7) + *(p+2));}
          if (rvar::evaluate_exit_on_error == TRUE) exit(1);
          errorstatus = exit_on_error;
          return;}
     else p += 3;} 
}

void int_entry( char* mes, int& v, int lo, int hi, char* lo_mes, char* hi_mes)
{
long i; int bad_int, too_large, sign; svar entry_line; char *c, d;
do {
     svar_entry( mes, entry_line);
     bad_int = FALSE; too_large = FALSE; i = 0; sign = 1;
     c = !entry_line; d = *c++;
     while (d == ' ') d = *c++;
     if (d == '+' || d == '-') {
          if (d == '-') sign = -1;
          do d = *c++; while (d == ' ');}
     if ( isdigit(d) ) {
          do {
               i = 10 * i + (d - '0'); 
               if (i > 32000) too_large = TRUE;
               d = *c++;}
          while (isdigit(d));}
     else bad_int = TRUE;
          
     while (bad_int == FALSE && d != '\0') {
          if (d != ' ') bad_int = TRUE; d = *c++;}
     if (bad_int == FALSE)
          if (too_large)
               {cout << "integer out of bounds \n"; bad_int = TRUE;}
          else {
               i = i * sign; 
               if (i < lo) {cout << lo_mes; bad_int = TRUE;}     
               if (i > hi) {cout << hi_mes; bad_int = TRUE;}}}
while (bad_int);
v = i;
}

void list_tokens(rvar& dummy)
{
rvar a = dummy; // this line to avoid "unused variable" compiler warnings
cout << "\
                           SYMBOLS RECOGNIZED\n\
   (     )       abs()  exp() sqrt()      Operations            Evaluation\n\
 ln() = log()   asin() acos() atan()    low rank: + -          left to right\n\
      pi         sin()  cos()  tan()    med rank: * /          left to right\n\
  constants      sinh() cosh() tanh()    hi rank: ^ -(prefix) right to left\n\n";
}

int no_rvar_error(svar& entry_line)
{
char *p = entry_line.tokens(), c;
while (1) {
     c = *p;
     switch (c) {
          case 'i':
          errorstatus = display_error; 
          error("complex imaginary i not appropriate");
          show_error_pos (entry_line, (*(p+1) << 7) + *(p+2));
          errorstatus = exit_on_error;
          return FALSE;
          
          case end_: return TRUE;}
     p += 3;}
}

enum {even_int_, odd_int_, even_num_odd_denom_, odd_num_odd_denom_, rvar_};

void process_exponential (svar& entry) 
{
// Check for a ^ (p / q ) case:
// If p and q are integers with q odd, then the symbol ^
// is changed to power_odd_p_odd_q if p is odd,
// and to power_even_p_odd_q if p is even;

rvar r, r1; svar t(entry.len()); 
char *p = entry.tokens(), *q = !entry, *q1, c, *type = !t;
int i = -1, i1;
while (1) {
     c = *p;
     switch (c) {
          case end_: case 'i': return;

          case const_: q1 = q + ((*(p+1) << 7) + *(p+2)); r = rvar(q1);
          if (is_int(r)) {
               temporary_top_precision();
               r1 = trunc(div2(r));
               if (r1 + r1 == r) type[++i] = even_int_;
               else type[++i] = odd_int_;
               restore_precision();}
          else type[++i] = rvar_;
          break;

          case pi_const_: case var_: type[++i] = rvar_; break;

          case unary_minus_: break;

          case abs_: case acos_: case asin_: case atan_: case cos_:
          case cosh_: case exp_: case log_: case sin_: case sinh_:
          case sqrt_: case tan_: case tanh_:
               type[i] = rvar_; break;

          case '+': case '-': case '*': type[--i] = rvar_; break;
          case '/':i1 = i--;
               if (type[i1] == odd_int_) {
                    if (type[i] == odd_int_) type[i] = odd_num_odd_denom_;
                    else if (type[i] == even_int_) {
                         type[i] = even_num_odd_denom_;}
                    else type[i] = rvar_;}
               else type[i] = rvar_;
               break;

          case '^': i1 = i--;
               if (type[i1] == odd_num_odd_denom_) *p = power_odd_p_odd_q;
               else if (type[i1] == even_num_odd_denom_) {
                    *p = power_even_p_odd_q;}
               type[i] = rvar_; break;}
     p += 3;}
}

void show_error_pos(svar& entry_line, unsigned position)
{
unsigned i, t = position;
cout << !entry_line << "\n";
for (i=0; i<t; i++) cout << ".";
cout << "!\n";
}     
     
void svar_entry(char* message, svar& entry_line)
{
char *p, c; svar d(1), e("");
p = !d;
cout << message; cout.flush();
do {
     cin.get(c);
     if (c == '\n') break;
     *p = c; e += d;}
while (1);
p = !e;
if ((p[0] == 'q' || p[0] == 'Q') && p[1] == '\0') exit(1);
entry_line = e;
if (svar::input_from_file) cout << e << "\n";
}



