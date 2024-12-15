#include "range.h"
#include "rational.h"
#include <ctype.h>

// Note: at end of this file there are include directives which read in
// the code files operator.cpp, function.cpp, rtos.cpp, and entry.cpp

const int rvar_fixed = sizeof(rvar::rva) - sizeof(int);
const int rvar_fixed_long = sizeof(rvar::rva) + 2 * sizeof(int);
const int svar_fixed = sizeof(svar::sva);

#ifdef four_digits

const int m_digits = 4;
const int m_base = 10000;
const int m_base_times2 = 20000;
const int m_base_half = 5000;
const int m_base_div10 = 1000;
const int m_max = 9999;
const int m_max_times2 = 19998;
const int m_min = -9999;
const int m_base_sqrt = 100;
const long m_base_long = 10000;
const long m_base_squared      = 100000000;
const long m_base_squared_div10 = 10000000;
const long m_base_squared_div100 = 1000000;
const long m_base_squared_left_bit = 67108864; // = 2^26

#endif

#ifdef six_digits

const int m_digits = 6;
const int m_base = 1000000;
const int m_base_times2 = 2000000;
const int m_base_half = 500000;
const int m_base_div10 = 100000;
const int m_max = 999999;
const int m_max_times2 = 1999998;
const int m_min = -999999;
const int m_base_sqrt = 1000;
const long m_base_long = 1000000;
const long m_base_squared      = 1000000000000;
const long m_base_squared_div10 = 100000000000;
const long m_base_squared_div100 = 10000000000;
const long m_base_squared_left_bit = 549755813888; // = 2^39

#endif

#ifdef eight_digits

const int m_digits = 8;
const int m_base = 100000000;
const int m_base_times2 = 200000000;
const int m_base_half = 50000000;
const int m_base_div10 = 10000000;
const int m_max = 99999999;
const int m_max_times2 = 199999998;
const int m_min = -99999999;
const int m_base_sqrt = 10000;
const long m_base_long = 100000000;
const long m_base_squared      = 10000000000000000;
const long m_base_squared_div10 = 1000000000000000;
const long m_base_squared_div100 = 100000000000000;
const long m_base_squared_left_bit = 9007199254740992; // = 2^53

#endif

// memory functions

void build_stack(char i)
{
char* q; memory::mem* p;
int k, stacksz = memory::stacksize[i], char_len = memory::no_chars[i];
q = (char*) malloc(stacksz * char_len);
if (q == 0) {errorstatus = exit_on_error; error("free store out of memory");}
memory::top[i] = (memory::mem*) q;
for (k=0; k<stacksz; k++) {
     p = (memory::mem*) q;
     p->size = i;
     q += char_len;
     p->next = (memory::mem*) q;}
p->next = 0;
}

void* get_memory(int num_chars)
{
char i=0; int* q = memory::no_chars;
if (num_chars > *q)
 if (num_chars > q[++i])
  if (num_chars > q[++i])
   if (num_chars > q[++i])
    if (num_chars > q[++i])
     if (num_chars > q[++i])
      if (num_chars > q[++i])
       if (num_chars > q[++i])
        if (num_chars > q[++i])
         if (num_chars > q[++i])
          if (num_chars > q[++i])
           if (num_chars > q[++i]) {
                errorstatus = exit_on_error;
                error("attempting to obtain too large memory strip");}
if (memory::top[i] == 0) build_stack(i);
memory::mem* p = memory::top[i];
memory::top[i] = p->next;
return p;
}

int get_max_len(char size)
{
if (size < 0 || size > 11) return 0;
return memory::no_chars[size];
}

void return_memory(void* p)
{
memory::mem* q = (memory::mem*) p;
char i = q->size;
q->next = memory::top[i];
memory::top[i] = q;
}

// rvar constructors and destructor

rvar :: rvar (char* s)
{
char* t;
int c, i, j, k, front, back, remainder, leng, leadzero, trailzero;
int counter = 0, expcounter = 1, exponent = 0, state = 0, point = -1;
int tlen, texp;
char tsgn;

tsgn = '\0';
if (*s == '\0') state = 13;
else            c = s[counter];
while (1) {
switch (state) {

case 0: if (c == '+') state = 1;
        else if (c == '-') {tsgn = '-'; state = 1;}
             else if (c == '.') {point = counter; state = 5;}
                  else if (isdigit(c)) {front = counter; state = 2;}
                       else if (isspace(c)) state = 0;
                            else            state = 13;
        break;

case 1: if (isspace(c)) state = 1;
        else if (isdigit(c)) {front = counter; state = 2;}
             else if (c == '.') {point = counter; state = 5;}
                  else          state = 13;
        break;

case 2: if (isdigit(c)) state = 2;
        else if (c == '.') {point = counter; state = 3;}
             else if (isspace(c)) {back = counter - 1; state = 6;}
                  else if (c=='E'||c=='e') {back = counter-1; state = 7;}
                       else if (c == '~') {back = counter - 1; state = 12;}
                            else {back = counter - 1; state = 11;}
        break;

case 3: if (isdigit(c)) state = 4;
        else if (isspace(c)) {back = counter - 2; state = 6;}
             else if (c == 'E' || c == 'e') {back = counter - 2; state = 7;}
                  else if (c == '~') {back = counter - 2; state = 12;}
                       else {back = counter - 2; state = 11;}
        break;

case 4: if (isdigit(c)) state = 4;
        else if (isspace(c)) {back = counter - 1; state = 6;}
             else if (c == 'E' || c == 'e') {back = counter - 1; state = 7;}
                  else if (c == '~') {back = counter - 1; state = 12;}
                       else {back = counter - 1; state = 11;}
        break;

case 5: if (isdigit(c)) {front = counter; state = 4;}
        else            state = 13;
        break;

case 6: if (isspace(c)) state = 6;
        else if (c == 'E' || c == 'e') state = 7;
             else if (c == '~') state = 12;
                  else state = 11;
        break;

case 7: if (isspace(c)) state = 7;
        else if (isdigit(c)) {t = s+counter; exponent = atoi(t); state=9;}
             else if (c == '+') {exponent = 1; state = 8;}
                  else if (c == '-') {exponent = -1; state = 8;}
                       else state = 13;
        break;

case 8: if (isspace(c)) state = 8;
        else if (isdigit(c)) {t = s+counter; exponent *= atoi(t); state=9;}
             else state = 13;
        break;

case 9: if (isdigit(c)) {if (++expcounter > 4) state = 13; else state = 9;}
        else if (c == '~') state = 12;
             else state = 11;
        break;}

if (state == 11 || state == 13) break;
counter++;
if (state == 12) break;
if (s[counter] == '\0') c = -1;
else               c = s[counter];}

if (state == 13) {error("constant syntax");
                  rv = &zero_init;
                  rv->count++;}
if (state == 11 || state == 12) {
for (i = front; i <= back; i++) if (s[i] != '0' && s[i] != '.') break;
if (i > back && state == 11) {
     rv = &zero_init;
     rv->count++;}
if (i > back && state == 12) i--;
if (i <= back) {
front = i;
if (point == -1) exponent += back - front + 1;
else if (point > front) exponent += point - front;
     else               exponent -= front - point - 1;
texp = exponent/m_digits;
remainder = exponent%m_digits;
if (remainder > 0) {texp++; leadzero = m_digits - remainder;}
else               leadzero = -remainder;
if (front < point && point < back) leng = back - front;
else                               leng = back - front + 1;
leng += leadzero;
tlen = leng/m_digits;
remainder = leng%m_digits;
if (remainder > 0) {tlen++; trailzero = m_digits - remainder;}
else               trailzero = 0;
rv = (rva*) get_memory(rvar_fixed + (tlen * sizeof(int)) );
rv->sgn = tsgn;
rv->count = 1;
rv->len = tlen;
rv->exp = texp;
if (state == 11) rv->rnge = 0;
else             {rv->rnge = 1;
                  for (i=0; i<trailzero; i++) rv->rnge *= 10;}
t = s + front;
for (i = 0; i < tlen; i++) rv->m[i] = 0;
if (tlen > 1) {
     for (i = 0; i < m_digits - leadzero; i++) {
          rv->m[0] = 10*rv->m[0] + *t++ - 48;
          if (*t == '.') t++;}
     for (j = 1; j < tlen - 1; j++) {
          for (i = 0; i < m_digits; i++) {
               rv->m[j] = 10*rv->m[j] + *t++ - 48;
               if (*t == '.') t++;}}
     for (i = 0; i < m_digits - trailzero; i++) {
          rv->m[tlen-1] = 10*rv->m[tlen-1] + *t++ - 48;
          if (*t == '.') t++;}
     for (i = 0; i < trailzero; i++) rv->m[tlen-1] *= 10;}
if (tlen == 1) {
     for (i = 0; i < m_digits - leadzero - trailzero; i++) {
          rv->m[0] = 10*rv->m[0] + *t++ - 48;
          if (*t == '.') t++;}
     for (i = 0; i < trailzero; i++) rv->m[0] *= 10;}}}
s += counter;
if (rv->len > 0)
     if (rv->rnge == 0)
          for (k = rv->len-1; rv->m[k] == 0 && k > 0; k--) rv->len--;
}

rvar :: rvar (long j)
{
rv = (rva*) get_memory(rvar_fixed_long);
rv->count = 1;
if (j == 0) rv->len = 0;
else {
     rv->rnge = 0;
     if (j > 0) rv->sgn = '\0';
     else       {rv->sgn = '-'; j = -j;}
     if (j < m_base) {
          rv->len = 1;
          rv->exp = 1;
          rv->m[0] = j;}
     else if (j < m_base_squared) {
          rv->exp = 2;
          rv->m[0] = j/m_base;
          rv->m[1] = j%m_base;
          if (rv->m[1]) rv->len = 2;
          else          rv->len = 1;}
     else {
          rv->exp = 3;
          rv->m[0] = j/(m_base_squared);
          rv->m[1] = (j/m_base)%m_base;
          rv->m[2] = j%m_base;
          if (rv->m[2]) rv->len = 3;
          else if (rv->m[1]) rv->len = 2;
          else          rv->len = 1;}}
}

rvar :: rvar ()
{
rv = &zero_init; rv->count++;
}

rvar :: rvar (rvar& a)
{
rv = a.rv;
rv->count++;
}

rvar :: rvar (rational& r)
{
rvar a = r.numer / r.denom;
rv = a.rv;
rv->count++;
}

rvar :: rvar (int length, char sign)
{
rv = (rva*) get_memory(rvar_fixed + (length * sizeof(int)) );
rv->count = 1;
rv->len = length;
rv->sgn = sign;
}

rvar::~rvar()
{
if (--rv->count == 0) return_memory(rv);
}

// svar constructors, destructor, and operators

svar :: svar ()
{
sv = &empty_init;
sv->count++;
}

svar :: svar (svar& a)
{
sv = a.sv;
sv->count++;
}

svar :: svar (char* p)
{
int b; char* q = p;
while (*q++) ;
b = q - p - 1;
sv = (sva*) get_memory(svar_fixed + b);
sv->count = 1;
sv->len = b;
sv->max_len = get_max_len(sv->size) - svar_fixed;
q = sv->m;
while (*p) *q++ = *p++; 
*q = '\0';
}

svar :: svar (int num_chars)
{
if (num_chars < 0) num_chars = 0;
sv = (sva*) get_memory(svar_fixed + num_chars);
sv->count = 1;
sv->len = num_chars;
sv->max_len = get_max_len(sv->size) - svar_fixed;
sv->m[num_chars] = '\0';
}

svar operator+(svar& a, svar& b)
{
int i = a.sv->len, j = b.sv->len, k;
svar c(i+j);
char *p = a.sv->m, *q = c.sv->m;
for (k=0; k<i; k++) *q++ = *p++;
p = b.sv->m;
for (k=0; k<j; k++) *q++ = *p++;
*q = '\0';
return c;
}

void operator+=(svar& a, svar& b)
{
int i = a.sv->len + b.sv->len;
if (a.sv->count > 1 || i > a.sv->max_len) a = a + b;
else {
     int k = b.sv->len;
     char *q = a.sv->m + a.sv->len, *p = b.sv->m;
     a.sv->len = i;
     for (i=0; i<k; i++) *q++ = *p++;
     *q = '\0';}
}

svar& svar::operator=(svar& b)
{
b.sv->count++;
if (--sv->count == 0) return_memory(sv);
sv = b.sv; return *this;
}

int operator== (svar& a, svar& b)
{
int i, j = a.sv->len;
if (j != b.sv->len) return 0;
char *p = a.sv->m, *q = b.sv->m;
for (i=0; i<j; i++) if (*p++ != *q++) return 0;
return 1;
}

int operator!= (svar& a, svar& b)
{
int i, j = a.sv->len;
if (j != b.sv->len) return 1;
char *p = a.sv->m, *q = b.sv->m;
for (i=0; i<j; i++) if (*p++ != *q++) return 1;
return 0;
}

ostream& operator<< (ostream& a, svar& b)
{
a << !b;
return a;
}

svar::~svar()
{
if (--sv->count == 0) return_memory(sv);
}

char* svar::tokens()
{
char *p = sv->m;
int j = sv->len, k;
if (j == 0 || p[j-1] != 0) {
     errorstatus = exit_on_error;
     error("entry function required before using svar tokens");}
k = (p[j-3] << 7) + p[j-2]; p = p + k;
return p;
}

#include "operator.cpp"
#include "function.cpp"
#include "entry.cpp"
#include "rtos.cpp"
