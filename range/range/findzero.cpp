#define main_program
#include <fstream.h>
#include "range.h"
#include "interval.h"
#include "series9.h"
#include "list.h"

const int N = 9; // N is max number of functions
enum types {no_zero, zero_enclosed, zero_on_boundary};
int d = 5, d1, dim, dim1, zero_trouble = FALSE, crossing_parity_check,
     num_solutions;
svar f_entry[N], f_let[N], arg[N];
svar left_entry[N], right_entry[N];
svar mess, variables, functions;
rvar delta, delta1, r, r1, r2, r3, r4, midpoint, factor = ".01";
rvar l_end[N], r_end[N], halfwidth[N], width[N];
interval I, I1, jacobian[N*N];
series var[N], s[N], s1, s2;
region *initial, *trouble;
list L, L2, L3; // L for crossing calc; L2 for regions; L3 for solution regions

void bisect_region();
void collect_ray_crossings(int);
int crossing_parity(region*);
region* enclose(region*);
void generate_solutions();
void increase_precision(int);
void initiate_with_boundary(region*);
int need_more_precision(region*);
int nonzero_jacobian();
void obtain_solution_list();
void obtain_unrepeated_boundary_elements();
void print_boundary_zero();
void print_end_message(region*);
void print_zero(region*);
void split_all(region*);
void split_boundary_region();
void split_region();
int trouble_with_functions();

void main() 
{
cout << "\n\
Program to find where n functions of n variables are simultaneously zero,\n\
in a region of search specified by bounding each variable in an interval.\n\n";

if (cin.seekg(0,ios::beg).tellg() != cin.seekg(0,ios::end).tellg()) {
     svar::input_from_file = TRUE;}
else svar::input_from_file = FALSE;
cin.seekg(0,ios::beg); cin.clear();

mess = ltos(N) + svar(" is the max\n");
int_entry("Enter the number, n, of functions and variables   ",
dim, 1, N,"", !mess);
dim1 = dim - 1;
region::initiate(dim);
svar spaced_variables; int i;
if (dim == 1) {
     arg[0] = svar("x"); f_let[0] = svar("f");}
else {
     for (i=0; i<dim; i++) {
          arg[i] = svar("x") + ltos(i+1);
          f_let[i] = svar("f") + ltos(i+1);}}
spaced_variables = variables = arg[0];
functions = f_let[0];
for (i=1; i<dim; i++) {
     variables +=  svar(",") + arg[i];
     spaced_variables += svar(", ") + arg[i];
     functions += svar(", ") + f_let[i];}
cout << "\n";
if (dim == 1) cout <<
"The function must have a derivative in the region of search,\n\
must not be zero on the region's boundary, nor at any point inside\n\
the region where its derivative is zero also.\n\n\
The function will be denoted by the symbol f\n\
            and the variable by the symbol x\n\n";
else cout <<
"The functions must have all first partial derivatives in the region\n\
of search, must not be simultaneously zero on the region's boundary, nor\n\
at any point inside the region where the functions' Jacobian is zero also.\n\n\
The functions will be denoted by the symbols " << functions << "\n\
            and the variables by the symbols " << spaced_variables << "\n\n";

list_tokens(zero);
for (i=0; i<dim; i++) {
     mess = svar("Enter the function ") + f_let[i];
     mess += svar("(") + variables + svar(")   ");
     entry(!mess, zero, f_entry[i], variables);}

series::initiate(dim, 1, var);
series::evaluate_exit_on_error = FALSE;
series::evaluate_error_display = FALSE;
get(initial);
for (i=0; i<dim; i++) {
     initial->data[i].left = zero; initial->data[i].right = one;
     initial->data[i].type = INTERVAL;}

cout << "The region R of search is specified by giving bounds \nfor ";
cout << spaced_variables << "\n";
for (i=0; i<dim; i++) {
     mess = svar("Enter lower bound for ") + arg[i] + svar("   ");
     entry(!mess, l_end[i], left_entry[i]);
     mess = svar("Enter upper bound for ") + arg[i] + svar("   ");
     entry(!mess, r_end[i], right_entry[i]);
     width[i] = r_end[i] - l_end[i];
     halfwidth[i] = div2(width[i]);
     if (r_end[i] <= l_end[i]) {
          cout << "upper bound not greater than lower bound; re-enter\n";
          i--;}}

int_entry("\
Enter the number of decimal places to which solutions are to be found   ",
d1, 0, 50, "", "Max is 50\n");

if (svar::input_from_file == FALSE && svar::input_logs_allowed == TRUE) {
     ofstream input_log("findzero.log");
     input_log << ltos(dim) << "\n";
     for (i=0; i<dim; i++) input_log << f_entry[i] << "\n";
     for (i=0; i<dim; i++) {
          input_log << left_entry[i] << "\n" << right_entry[i] << "\n";}
     input_log << ltos(d1) << "\n";
     input_log.close();}

delta = rvar(".2") * (ten ^ rvar(-(d+1))); // used by trouble test
delta1 = ten ^ rvar(-(d+3));               // used by crossing parity
set_precision(d + 10); num_solutions = 0;
if (! trouble_with_functions()) {
     crossing_parity_check = TRUE;
     if (crossing_parity(initial) == zero_on_boundary) {
          print_boundary_zero();}
     else {
          crossing_parity_check = FALSE;
          obtain_solution_list();}}
}

void bisect_region()
{
region *p = L2.base, *q;
int i=0, j, k;
rvar::exact_arithmetic = TRUE;     
r1 = p->data[i].right - p->data[i].left;
for (j=1; j<dim; j++) {
     r2 = p->data[j].right - p->data[j].left;
     if (r1 < r2) {
          i = j; r1 = r2;}}
get(q);
for (j=0; j<dim; j++) {
     q->data[j].type = INTERVAL;
     q->data[j].right = p->data[j].right;
     q->data[j].left = p->data[j].left;}
r = p->data[i].left + p->data[i].right;
midpoint = div2(r);
rvar::exact_arithmetic = FALSE;
p->data[i].left = midpoint; q->data[i].right = midpoint;
q->next = p; L2.base = q;
}

void collect_ray_crossings(int ray)
{
region *q = 0, *q1 = 0; int j, cycle_end, ray1 = ray + 1;
while (L.base) {
     cycle_end = FALSE;
     for (j=0; j<dim; j++) {
          r1 = L.base->data[j].left;
          if (L.base->data[j].type == INTERVAL) {
               r2 = L.base->data[j].right;
               r3 = l_end[j] + (r1 + r2) * halfwidth[j];
               r4 = (r2 - r1) * halfwidth[j];
               var[j].const_term(r3, r4);}
          else var[j].const_term(l_end[j] + r1 * width[j]);}
     for (j=ray1; j<dim; j++) {
          evaluate(s1, f_entry[j], var);
          if (series::evaluate_error) {
               split_boundary_region();
               cycle_end = TRUE; break;}
          if (! s1(0,0,0).has_zero() ) {
               L.remove_first();
               cycle_end = TRUE; break;}}
     if (cycle_end) continue;
     evaluate(s1, f_entry[ray], var);
     if (series::evaluate_error) {
          split_boundary_region(); continue;}
     I = s1(0,0,0);
     if (I.negative() ) L.remove_first();
     else if (I.positive() ) {
          q = L.base; L.base = q->next; // move element to other list
          q->next = q1; q1 = q;}
     else split_boundary_region();}
L.base = q;
}

int crossing_parity(region* p)
{
// The computation of crossing parity over a region can be done
// only if the functions are never all zero on the boundary
// of the region. The crossing parity equals the number of times (mod 2)
// the image of the boundary of the region crosses
// a ray from the origin.

int i, deg = 0; zero_trouble = FALSE;
initiate_with_boundary(p);
collect_ray_crossings(0);
if (zero_trouble) return zero_on_boundary;
for (i=1; i<dim; i++) {
     obtain_unrepeated_boundary_elements();
     collect_ray_crossings(i);}
while (L.base) {
     deg++;
     L.remove_first();}
if (deg & 1) return zero_enclosed;
else return no_zero;
}

region* enclose(region* q)
{
int i, j; region *p = L2.base, *p1;
rvar::exact_arithmetic = TRUE;
for (i=0; i<dim; i++) {
     if (q->data[i].type == POINT) {
          r1 = q->data[i].left; r2 = r1;
          q->data[i].type = INTERVAL;}
     else {
          r1 = q->data[i].left; r2 = q->data[i].right;}
     r = div2(div2(p->data[i].right - p->data[i].left));
     r1 -= r; if (r1 < zero) r1 = zero;
     r2 += r; if (r2 > one)  r2 = one;
     q->data[i].left = r1; q->data[i].right = r2;}
rvar::exact_arithmetic = FALSE;
// Check if q region intersects any region in list L3.
// If any are found, these are removed from L3 and returned to L2, 
// and a reduction made in num_solutions. The split_all function
// will remove that part of each region in L2 which intersects q region.
p = L3.base; p1 = 0;
while (p) {
     for (i=0; i<dim; i++) {
          if (q->data[i].left >= p->data[i].right) break;
          if (p->data[i].left >= q->data[i].right) break;}
     if (i == dim) {
          cout << "Number of solutions found = " << --num_solutions << "\r";
          cout.flush();
          if (p1 == 0) {
               L3.base = p->next; L2.insert(p); p = L3.base;}
          else {
               p1->next = p->next; L2.insert(p); p = p1->next;}}
     else {
          p1 = p; p = p->next;}}
return q;
}

void generate_solutions()
{
int i, i1, j, j1, k, error_run, max_len, sol_num = 0,
     need_higher_precision, no_width_reduction;
svar output[N]; region *p;
rvar low[N], high[N], w[N], w1[N], A[N*N], F[N];
set_precision(d1 + 10);
delta = rvar(".2") * (ten ^ rvar(-(d1+1)));
delta1 = ten ^ rvar(-(d1+3));
while (L3.base) {
     p = L3.base; error_run = FALSE;
     cout << "Solution No. " << ++sol_num << " :\n";
     for (i=0; i<dim; i++) {
          low[i] = l_end[i] + p->data[i].left * width[i];
          high[i] = l_end[i] + p->data[i].right * width[i];}
     for (i=0; i<dim; i++) w[i] = mid(div2(low[i] + high[i]));
     need_higher_precision = FALSE;
     while (1) {
          if (need_higher_precision) {
               increase_precision(8); error_run = FALSE;
               for (j=0; j<dim; j++) {
                    low[j] = l_end[j] + p->data[j].left * width[j]; 
                    high[j] = l_end[j] + p->data[j].right * width[j];}
               need_higher_precision = FALSE;}
          if (error_run) r = delta; else r = zero;
          for (i=0; i<dim; i++) var[i].const_term(w[i], r);
          for (i=0; i<dim; i++) {
               evaluate(s1, f_entry[i], var);
               F[i] = s1(0,0,0).mid;
               if (error_run) {
                    for (j=0; j<dim; j++) {
                         I = s1(1,j,0);
                         A[i*dim+j] = I.mid % I.wid;}}
               else     for (j=0; j<dim; j++) A[i*dim+j] = s1(1,j,0).mid;}
          for (i=0; i<dim; i++) { // Newton's method
               i1 = i+1; j1 = -1;
               for (j=i; j<dim; j++) {
                    r = abs(A[j*dim+i]);
                    if (r == zero) continue;
                    if (j1 == -1 || r > r1) {
                         j1 = j; r1 = r;}}
               if (j1 == -1) error("findzero7");
               if (j1 != i) {
                    swap(F[i], F[j1]);
                    for (k=i; k<dim; k++) swap(A[j1*dim+k], A[i*dim+k]);}
               r = A[i*dim+i];
               for (j=i1; j<dim; j++) {
                    r1 = A[j*dim+i] / r;
                    for (k=i1; k<dim; k++) A[j*dim+k] -= A[i*dim+k] * r1;
                    F[j] -= F[i] * r1;}}
          for (i=dim1; i>=0; i--) {
               for (j=dim1; j>i; j--) F[i] -= A[i*dim+j] * F[j];
               F[i] /= A[i*dim+i];}
          if (error_run) {
               for (i=0; i<dim; i++) {
                    r = abs(F[i]);
                    if (r >= delta) break;}
               if (i < dim) {
                    need_higher_precision = TRUE; continue;}
               for (i=0; i<dim; i++) output[i] = rtos(w[i] % delta, 1, d1);
               break;}
          else {
               do {
                    for (i=0; i<dim; i++) w1[i] = w[i] - F[i];
                    no_width_reduction = TRUE;
                    for (i=0; i<dim; i++) {
                         if (w1[i] <= low[i] || w1[i] >= high[i]) {
                              if (no_width_reduction) { // check precision
                                   for (j=0; j<dim; j++) {
                                        if (range(w1[i]) >= tenth *
                                        (high[i] - low[i])) break;}
                                   if (j < dim) need_higher_precision = TRUE;}
                              for (j=0; j<dim; j++) F[j] = div2(F[j]);
                              no_width_reduction = FALSE; break;}}}
               while (i < dim);
               if (no_width_reduction) {
                    for (i=0; i<dim; i++) {
                         if (abs(F[i]) > delta1) break;}
                    if (i == dim) {
                         error_run = TRUE; continue;}}
               for (i=0; i<dim; i++) {
                    if (w[i] != w1[i]) {
                         if (w1[i] != zero) w[i] = mid(w1[i]);
                         else w[i] = zero;}}}}
     max_len = 0;
     for (i=0; i<dim; i++) {
          if (max_len < output[i].len()) max_len = output[i].len();}
     for (i=0; i<dim; i++) {
          cout << arg[i] << " = ";
          cout.width(max_len); cout << output[i] << "\n";}
     L3.remove_first();}               
}

void increase_precision(int i)
{
int j;
add_precision(i);
for (j=0; j<dim; j++) {
     evaluate(l_end[j], left_entry[j]);
     evaluate(r_end[j], right_entry[j]);
     width[j] = r_end[j] - l_end[j];
     halfwidth[j] = div2(width[j]);}
}

void initiate_with_boundary(region *b)
{
int i, j; region *p1, *p2;
L.clear();
for (i=dim1; i>=0; i--) {
     get(p1); get(p2);
     p1->data[i].left = b->data[i].left; p2->data[i].left = b->data[i].right;
     p1->data[i].type = POINT; p2->data[i].type = POINT;
     for (j=0; j<dim; j++) {
          if (j != i) {
               p1->data[j].left = b->data[j].left;
               p1->data[j].right = b->data[j].right;
               p2->data[j].left = b->data[j].left;
               p2->data[j].right = b->data[j].right;
               p1->data[j].type = INTERVAL; p2->data[j].type = INTERVAL;}}
     p1->next = p2; p2->next = L.base; L.base = p1;}
}

int need_more_precision(region* p)
{
int i;
for (i=0; i<dim; i++) {
     r1 = p->data[i].left;
     if (p->data[i].type == INTERVAL) {
          r2 = p->data[i].right;
          r3 = l_end[i] + (r1 + r2) * halfwidth[i];
          r4 = (r2 - r1) * halfwidth[i];
          var[i].const_term(r3, r4);}
     else var[i].const_term(l_end[i] + r1 * width[i]);}
for (i=0; i<dim; i++) {
     evaluate(s1, f_entry[i], var);
     if (series::evaluate_error) continue;
     I = s1(0,0,0);
     if (! is_zero(I.wid)) {
          r1 = tenth * abs(mid(I.wid));
          if (range(I.mid) >= r1 || range(I.wid) >= r1) return TRUE;}}
return FALSE;
}

int nonzero_jacobian()
{
int i, i1, j, j1, k;
rvar ratio_least, ratio;
for (i=0; i<dim; i++) {
     for (j=0; j<dim; j++) {
          jacobian[i*dim+j] = s[i](1,j,0);}}
for (i=0; i<dim; i++) {
     i1 = i + 1; j1 = -1;
     for (j=i; j<dim; j++) {
          I = jacobian[j*dim+i];
          if (I.has_zero() ) continue;
          ratio = I.wid / abs(I.mid);
          if (j1 == -1 || ratio < ratio_least) {
               j1 = j; I1 = I; ratio_least = ratio;}}
     if (j1 == -1) return FALSE;
     if (j1 != i) {
          for (k=i; k<dim; k++) swap(jacobian[i*dim+k], jacobian[j1*dim+k]);}
     for (j=i1; j<dim; j++) {
          I = jacobian[j*dim+i] / I1;
          for (k=i1; k<dim; k++) {
               jacobian[j*dim+k] = jacobian[j*dim+k] - jacobian[i*dim+k]*I;}}}
return TRUE;
}                    
               
void obtain_solution_list()
{
int i, j, k, not_through;
region *p, *q, *L3_end = 0; 
L2.initiate(initial); L3.clear();
cout << "Number of solutions found = 0\r"; cout.flush();
while (L2.base) {
     p = L2.base; not_through = TRUE;
     for (i=0; i<dim; i++) {
          r1 = p->data[i].left; r2 = p->data[i].right;
          r3 = l_end[i] + (r1 + r2) * halfwidth[i];
          r4 = (r2 - r1) * halfwidth[i];
          var[i].const_term(r3, r4);}
     for (i=0; i<dim; i++) {
          evaluate(s[i], f_entry[i], var);
          if (series::evaluate_error) {
               bisect_region();
               not_through = FALSE; break;}
          if (! s[i](0,0,0).has_zero() ) {
               L2.remove_first();
               not_through = FALSE; break;}}
     if (not_through) {
          if (nonzero_jacobian()) {
               k = crossing_parity(p);
               if (k == no_zero) L2.remove_first();
               else if (k == zero_enclosed) {
                    L2.base = p->next;
                    if (L3_end == 0) { // fast append
                         L3.insert(p); L3_end = p;}
                    else {
                         L3_end->next = p; p->next = 0; L3_end = p;} 
                    num_solutions++;
                    cout << "Number of solutions found = ";
                    cout << num_solutions << "\r"; cout.flush();}
               else { // zero_on_boundary case
                    q = enclose(trouble);
                    split_all(q);
                    L2.append(q);}} 
          else { // size check
                    for (i=0; i<dim; i++) {
                    if (var[i](0,0,0).wid >= delta) break;}
               if (i == dim) {
                    print_end_message(p);
                    return;}
               bisect_region();}}}
cout << "Number of solutions = " << num_solutions << "               \n";

generate_solutions();
}

void obtain_unrepeated_boundary_elements()
{
region *p = L.base, *q = 0, *q1, *q2, *q3, *q4; int i, j, k;
// first obtain all boundary elements
while (p) {
     for (i=0; i<dim; i++) {
          if (p->data[i].type == POINT) continue;
          else {
               get(q1); get(q2); q1->next = q2; q2->next = q; q = q1;
               for (j=0; j<dim; j++) {
                    if (j != i) {
                         q1->data[j].type = p->data[j].type;
                         q2->data[j].type = p->data[j].type;
                         q1->data[j].left = p->data[j].left;
                         q2->data[j].left = p->data[j].left;
                         if (p->data[i].type == INTERVAL) {
                              q1->data[j].right = p->data[j].right;
                              q2->data[j].right = p->data[j].right;}}}
               q1->data[i].type = POINT; q2->data[i].type = POINT;
               q1->data[i].left = p->data[i].left;
               q2->data[i].left = p->data[i].right;}}
     p = p->next;}
L.clear(); L.base = q; q = 0;
// Remove duplicates; here a region can not partially overlap another,
// but will either include, or be included in the other, or else have
// no point in common.
int intersection, match, reverse;
while (L.base) {
     p = L.base; q1 = p->next; q2 = p;
     intersection = FALSE;
     while (q1) {
          for (i=0; i<dim; i++) {
               if (p->data[i].type == POINT) {
                    if (q1->data[i].type == INTERVAL) break;
                    if (p->data[i].left != q1->data[i].left) break;}
               else {
                    if (q1->data[i].type == POINT) break;
                    if (q1->data[i].left >= p->data[i].right) break;
                    if (q1->data[i].right <= p->data[i].left) break;}}
          if (i < dim) {
               q2 = q1; q1 = q1->next;}
          else {
               intersection = TRUE; break;}}
     if (intersection) {
          q2->next = q1->next; // take q1 out of chain
          L.base = p->next; // take p out of chain
          match = TRUE;
          for (i=0; i<dim; i++) {
               if (p->data[i].type == INTERVAL) {
                    if (p->data[i].left != q1->data[i].left) {
                         match = FALSE;
                         if (p->data[i].left < q1->data[i].left) {
                              reverse = TRUE;}
                         else reverse = FALSE;
                         break;}
                    if (p->data[i].right != q1->data[i].right) {
                         match = FALSE;
                         if (p->data[i].right > q1->data[i].right) {
                              reverse = TRUE;}
                         else reverse = FALSE;
                         break;}}}
          if (match) {
               remove(p); remove(q1);}
          else { // need to break q1 into subregions
               if (reverse) { // p region contains q1 region; switch p and q1
                    q2 = p; p = q1; q1 = q2;} 
               q1->next = 0;
               for (i=0; i<dim; i++) {
                    if (p->data[i].type == POINT) continue;
                    if (p->data[i].left == q1->data[i].left) {
                         if (p->data[i].right == q1->data[i].right) continue;
                         else j = 1;}
                    else {
                         if (p->data[i].right == q1->data[i].right) j = 2;
                         else j = 3;}
                    q2 = q1;
                    while (q2) {
                         get(q3); q3->next = q2->next; q2->next = q3;
                         for (k=0; k<dim; k++) {
                              q3->data[k].type = q2->data[k].type;
                              q3->data[k].left = q2->data[k].left;
                              if (q2->data[k].type == INTERVAL) {
                                   q3->data[k].right = q2->data[k].right;}}
                         if (j == 1)  {
                              q2->data[i].right = p->data[i].right;
                              q3->data[i].left  = p->data[i].right;
                              q2 = q3->next;}
                         else if (j == 2) {
                              q2->data[i].right = p->data[i].left;
                              q3->data[i].left  = p->data[i].left;
                              q2 = q3->next;}
                         else {
                              get(q4); q4->next = q3->next; q3->next = q4;
                              for (k=0; k<dim; k++) {
                                   q4->data[k].type = q2->data[k].type;
                                   q4->data[k].left = q2->data[k].left;
                                   if (q2->data[k].type == INTERVAL) {
                                        q4->data[k].right = q2->data[k].right;}}
                              q2->data[i].right = p->data[i].left;
                              q3->data[i].left  = p->data[i].left;
                              q3->data[i].right = p->data[i].right;
                              q4->data[i].left  = p->data[i].right;
                              q2 = q4->next;}}}
               p->next = q1; q2 = q1->next;
               while (q2) {
                    q1 = q2; q2 = q2->next;}
               q1->next = L.base; L.base = p;}} // continuation removes p
     else {
          L.base = p->next; p->next = q; q = p;}} // if no overlap accept p
L.base = q;
}

void print_boundary_zero()
{
cout << "At the following point on the boundary of the region R\n";
cout << "the function";
if (dim == 1) cout << " " << functions << " is ";
else if (dim == 2) cout << "s " << functions << " both are ";
else cout << "s " << functions << " all are ";
cout << "less in absolute value than 10^-" << d+3 << " :\n";
print_zero(trouble); remove(trouble);
}

void print_end_message(region* p)
{
int i;
cout << "Search terminated: A point inside the region R has been found\n";
cout << "where the function";
if (dim == 1) cout << " and its derivative ";
else cout << "s and their Jacobian ";
cout << "are too close to zero.\nThe point is :\n";
print_zero(p); 
}

void print_zero(region *p)
{
int i;
for (i=0; i<dim; i++) {
     cout << arg[i] << " = " ;
     r1 = p->data[i].left;
     if (p->data[i].type == INTERVAL) {
          r2 = p->data[i].right;
          r = l_end[i] + (r2 + r1) * halfwidth[i];
          r = r % ((r2 - r1) * halfwidth[i]);}
     else r = l_end[i] + r1 * width[i];
     cout << rtos(r, 1, d) << "\n";}
}

void split_all(region* p)
{
region *q, *q1, *q2, *q3, *q4, *q5, *q6; int i, j, k;
// find all L2 regions with overlap and split these to eliminate overlap
q = L2.base; get(q6); q6->next = q; q1 = q6;
while (q) {               
     for (i=0; i<dim; i++) {
          if (p->data[i].left >= q->data[i].right) break;
          if (q->data[i].left >= p->data[i].right) break;}
     if (i < dim) { // no overlap
          q1 = q; q = q->next;}
     else {
          for (i=0; i<dim; i++) {
               if (p->data[i].left  > q->data[i].left)  break;
               if (p->data[i].right < q->data[i].right) break;}
          if (i == dim) { // p contains q; remove region q
               q1->next = q->next; remove(q); q = q1->next;}
          else { // need to break q into subregions
               q2 = q->next;
               for (i=0; i<dim; i++) {
                    if (p->data[i].left <= q->data[i].left) {
                         if (p->data[i].right >= q->data[i].right) continue;
                         else j = 1;}
                    else {
                         if (p->data[i].right >= q->data[i].right) j = 2;
                         else j = 3;}
                    q3 = q;
                    while (q3 != q2) {
                         get(q4); q4->next = q3->next; q3->next = q4;
                         for (k=0; k<dim; k++) {
                              q4->data[k].type = INTERVAL;
                              q4->data[k].left  = q3->data[k].left;
                              q4->data[k].right = q3->data[k].right;}
                         if (j == 1) {
                              q3->data[i].right = p->data[i].right;
                              q4->data[i].left  = p->data[i].right;
                              q3 = q4->next;}
                         else if (j == 2) {
                              q3->data[i].right = p->data[i].left;
                              q4->data[i].left  = p->data[i].left;
                              q3 = q4->next;}
                         else {
                              get(q5); q5->next = q4->next; q4->next = q5;
                              for (k=0; k<dim; k++) {
                                   q5->data[k].type = INTERVAL;
                                   q5->data[k].left  = q3->data[k].left;
                                   q5->data[k].right = q3->data[k].right;}
                              q3->data[i].right = p->data[i].left;
                              q4->data[i].left  = p->data[i].left;
                              q4->data[i].right = p->data[i].right;
                              q5->data[i].left  = p->data[i].right;
                              q3 = q5->next;}}}}}}
                              // q subregion contained by p removed later
L2.base = q6->next; remove(q6);
}

void split_boundary_region()
{
region *p = L.base, *p1 = p->next, *q, *q1;
int i, j;
rvar::exact_arithmetic = TRUE;
for (i=0; i<dim; i++) {
     if (p->data[i].type == POINT) continue;
     r1 = p->data[i].left; r2 = p->data[i].right; r3 = r2 - r1;
     if ((r3 * halfwidth[i] >= delta1) || (crossing_parity_check == FALSE &&
     r3 >= (factor * (L2.base->data[i].right - L2.base->data[i].left)))) {
          midpoint = div2(r1+r2);
          q = p;
          do {
               get(q1); q1->next = q->next; q->next = q1; 
               for (j=0; j<dim; j++) {
                    if (j != i) {
                         q1->data[j].type = q->data[j].type;
                         q1->data[j].left = q->data[j].left;
                         if (q->data[j].type == INTERVAL) {
                              q1->data[j].right = q->data[j].right;}}}
               q1->data[i].type = INTERVAL;
               q1->data[i].right = r2; q1->data[i].left = midpoint;
               q->data[i].right = midpoint; q = q1->next;}
          while (q != p1);}}
rvar::exact_arithmetic = FALSE;
if (p->next == p1) { // no split;
     if (need_more_precision(p)) increase_precision(8);
     else {
          trouble = p;
          L.base = p1; L.clear(); // this terminates crossing computation
          zero_trouble = TRUE;}}
}

void split_region()
{
region *p = L.base, *p1 = p->next, *q, *q1;
int i, j;
rvar::exact_arithmetic = TRUE;
for (i=0; i<dim; i++) {
     r = p->data[i].right - p->data[i].left;
     if (r * halfwidth[i] >= delta) {
          midpoint = div2(p->data[i].right + p->data[i].left);
          q = p;
          do {
               get(q1); q1->next = q->next; q->next = q1;
               for (j=0; j<dim; j++) {
                    if (j != i) {
                         q1->data[j].right = q->data[j].right;
                         q1->data[j].left = q->data[j].left;}}
               q1->data[i].right = q->data[i].right;
               q1->data[i].left = midpoint;
               q->data[i].right = midpoint; q = q1->next;}
          while (q != p1);}}
rvar::exact_arithmetic = FALSE;
}

int trouble_with_functions()
{
int i, j;
for (i=0; i<dim; i++) {
     L.initiate(initial);
     do {
          for (j=0; j<dim; j++) {
               r1 = L.base->data[j].left; r2 = L.base->data[j].right;
               r3 = l_end[j] + (r1 + r2) * halfwidth[j];
               r4 = (r2 - r1) * halfwidth[j];
               var[j].const_term(r3, r4);}
          evaluate(s[i], f_entry[i], var);
          if (series::evaluate_error) {
               for (j=0; j<dim; j++) if (var[j](0,0,0).wid >= delta) break;
               if (j < dim) split_region();
               else {
                    cout << "Unable to verify that in the region R\n";
                    cout << "the function " << f_let[i];
                    cout << "(" << variables << ") ";
                    cout << "or a derivative is defined at the following";
                    cout << " point:\n";
                    for (j=0; j<dim; j++) {
                         cout << arg[j] << " = ";
                         r = var[j](0,0,0).mid % var[j](0,0,0).wid;
                         cout << rtos(r,1,d) << "\n";}
                    return TRUE;}}
          else L.remove_first();}
     while (L.base);}
return FALSE;
}



