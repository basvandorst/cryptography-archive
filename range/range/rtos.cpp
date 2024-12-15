svar dtos(int);
void examine(svar&, int);
void fixprnt(char, svar&, int, int, int, int, int, int, long, int*);
void floatprnt(char, svar&, int, int, int, int, int, long, int*);
void set_up_mant(int*, rvar&, int, int, int, int, int);
void round_mant(int*, int&, int&, int, int&, int&, int);

svar rtos (rvar& a, int format, int k)
{
svar buffer;
int round = 0, shift = 0, stflg = 1, len, leng, front, back, rng;
int s, t, i, j, l, p, firstint, lastint;
long exponent;
int* mant = 0;
char sgn;

if (format < 0 || format > 3) format = 0;
if (format == 0) if(is_int(a)) format = 3;

if (a.rv->len == 0) { // a is an exact zero
     if (format == 1) {
          buffer += svar("0.");
          for (i=0; i<k; i++) buffer += svar("0");
          buffer += svar(" ");}
     else if (format == 2) buffer = svar(" 0");
     else if (format == 3) buffer = svar("0");
     return buffer;}
                  
if (format == 3) { // integer format
     if (is_int(a)) {
          if (a.rv->sgn == '-') buffer += svar("-");
          buffer += ltos(a.rv->m[0]);
          for (i=1; i<a.rv->len; i++) {
               for (j = m_base/10; a.rv->m[i]<j && j>1; j /= 10) {
                    buffer += svar("0");}
               buffer += ltos(a.rv->m[i]);}
          for (; i < a.rv->exp; i++) {
               for (j = 0; j < m_digits; j++) buffer += svar("0");}}
     else buffer = svar("Not an Integer");
     return buffer;}

if (format == 0) { // choose format 1 or 2 according to type of number a is
     if (a.rv->exp >= -1 && a.rv->exp <= 2) {
          format = 1;
          k = m_digits * (a.rv->len - a.rv->exp);}
     else {
          format = 2;
          k = m_digits * a.rv->len;}
     stflg = 0;}

// format = 1 or 2 from here on
if (k < 0) k = 0;
t = k;
exponent = m_digits * ((long) a.rv->exp);
rng = a.rv->rnge;
sgn = a.rv->sgn;
len = a.rv->len;
firstint = a.rv->m[0];
lastint = a.rv->m[len - 1];
front = m_digits - 1;
for (i = 10; firstint >= i; i *= 10) front--;
back = m_digits - 1;
for (i = 10; rng >= i; i *= 10) back--;
if (rng > 9) {
     i /= 10; l = rng/i;
     j = (rng % i) + (lastint % i);
     if (j) {
          l++; if (j > i) l++;}
     if (l > 9) {
          i *= 10;
          j = (rng % i) + (lastint % i);
          if (j > i) l = 2; else l = 1;
          back--;}
     rng = l;}

if (len == 1 && back < front) {
     // no mantissa digits
     exponent -= back; s = 0; t = -exponent;
     if (t > k) t = k;
     if (format == 1) { 
          fixprnt(sgn, buffer, round, k, stflg, s, t, rng, exponent, mant);}
     else { 
          floatprnt(sgn, buffer, round, k, stflg, s, rng, exponent, mant);}
     if (sgn == '-') examine(buffer, format);
     return buffer;}

leng = m_digits * (len - 1) + (back - front + 1);
if (back == -1) {
     back = m_digits - 1; shift = 1;}
exponent -= front;
if (format == 1) s = k + exponent;
else s = k + 1;
if (s >= leng)
     if (rng > 0) {
          t -= s-leng+1; s = leng-1;}
     else {
          t -= s-leng; s = leng;}

svar smant(sizeof(int)*leng);
mant = (int*) smant.sv->m;
set_up_mant(mant, a, front, back, shift, firstint, lastint);

p = leng - s;
if (p) round_mant(mant, s, t, leng, rng, round, p);
if (format == 1) { 
     fixprnt(sgn, buffer, round, k, stflg, s, t, rng, exponent, mant);}
else { 
     floatprnt(sgn, buffer, round, k, stflg, s, rng, exponent, mant);}
if (sgn == '-') examine(buffer, format);
return buffer;
}


void test (rvar& a, int format, int k)
{
int round = 0, shift = 0, len;
int leng, front, back, rng, s, t, p;
int i, j, l, firstint, lastint;
long exponent;
int* mant = 0;

if (a.rv->len == 0 || a.rv->rnge == 0) return;
if (format != 1) format = 2;
if (k < 0) k = 0;
t = k;
exponent = m_digits * a.rv->exp;
rng = a.rv->rnge;
len = a.rv->len;
firstint = a.rv->m[0];
lastint = a.rv->m[len - 1];
front = m_digits - 1;
for (i = 10; firstint >= i; i *= 10) front--;
back = m_digits - 1;
for (i = 10; rng >= i; i *= 10) back--;
if (rng > 9) {
     i /= 10; l = rng/i;
     j = (rng % i) + (lastint % i);
     if (j) {
          l++; if (j > i) l++;}
     if (l > 9) {
          i *= 10;
          j = (rng % i) + (lastint % i);
          if (j > i) l = 2; else l = 1;
          back--;}
     rng = l;}

if (len == 1 && back < front) { // no mantissa digits
     exponent -= back;
     if (rng > 5) exponent++;
     if (format == 1) l = k + exponent;
     else {
          if (exponent < 0) l = k + exponent; else l = k;}
     if (l > 0 && test_failure < l) test_failure = l;
     return;}

leng = m_digits * (len - 1) + (back - front + 1);
if (back == -1) {
     back = m_digits - 1; shift = 1;}
exponent -= front;
if (format == 1) s = k + exponent;
else s = k + 1;
if (s >= leng) {t -= s-leng+1; s = leng-1;}
p = leng - s;

svar smant(sizeof(int)*leng);
mant = (int*) smant.sv->m;
set_up_mant(mant, a, front, back, shift, firstint, lastint);

round_mant(mant, s, t, leng, rng, round, p);

if (format == 1) {
     if (t > k) return;
     if (test_failure < k-t) test_failure = k-t;
     return;}

else {
     if (s == 0 && round == 1) {s = 1; exponent++;}
     if (s < 0) {s = 0; exponent++;}
     if (s > 0) exponent--;
     if (a == zero && exponent <= -k) return;
     if (s > k) return;
     if (s == 0) { 
          if (test_failure < k) test_failure = k;
          return;}
     if (test_failure < k+1-s) test_failure = k+1-s;
     return;}
}


int is_int(rvar& a)
{
if (a.rv->len == 0) return TRUE;
if (a.rv->rnge) return FALSE;
if (a.rv->exp >= a.rv->len) return TRUE;
return FALSE;
}

int is_zero(rvar& a)
{
if (a.rv->len == 0) return TRUE;
return FALSE;
}

svar dtos(int a)     // assumes a is a positive integer between 0 and 9
{
svar b;
switch (a) {
     case 0: b = svar("0"); break;
     case 1: b = svar("1"); break;
     case 2: b = svar("2"); break;
     case 3: b = svar("3"); break;
     case 4: b = svar("4"); break;
     case 5: b = svar("5"); break;
     case 6: b = svar("6"); break;
     case 7: b = svar("7"); break;
     case 8: b = svar("8"); break;
     case 9: b = svar("9"); break;}
return b;}

svar ltos(long val)
{
int i=11, a[12];
svar b;
if (val == 0) {
     b = svar("0");
     return b;}
if (val < 0) {
     b = svar("-");
     val = -val;}
while (val > 0) {
     a[i] = val%10;
     val = val/10;
     i--;}
i++;
while (i <= 11) {
     b += dtos(a[i]);
     i++;}
return b;
}


void examine(svar& buffer, int format)
{ // eliminate negative sign from buffer when not needed
int i, l;
char *p = buffer.sv->m + 1, *q;
if (format == 2) {
     while (*p != 'E') {
          if (*p != '0' && *p != '.' && *p != '*' && *p != ' ') return;
          p++;}
     buffer.sv->m[0] = ' ';
     return;}
if (format == 1) {
     while (*p != '\0') {
          if (*p != '0' && *p != '.' && *p != '*' && *p != ' ') return;
          p++;}
     l = buffer.sv->len;
     buffer.sv->len--;
     q = buffer.sv->m; p = q + 1;
     for (i=0; i<l; i++) *q++ = *p++;
     return;}
}


void fixprnt(char sgn, svar& buffer, int round, int k, int stflg,
             int s, int t, int rng, long exponent, int* mant)
{
int i, w = -1, z = 0, lastdigit = 11, nextdigit = 11;

if (sgn == '-') buffer += svar("-");

if (s<0) {
     if (exponent>-1) {
          for (i=0; i<=exponent; i++) buffer += svar("*");
          buffer += svar(".");
          if (stflg) for (i=0; i<k; i++) buffer += svar("*");}
     else if (exponent==-1) {
          buffer += svar("0."); 
          if (stflg) for (i=0;i<k;i++) buffer += svar("*");}
     else if (exponent<-1) {
          buffer += svar("0.");
          for (i=1;i<=k&&i<-exponent;i++) buffer += svar("0");
          if (stflg) for (; i<=k; i++) buffer += svar("*");}
     buffer += svar(" ");
     return;}

if (s == 0) z = round;
else lastdigit = mant[s-1] + round;
if (s == 1 && lastdigit == 10) {lastdigit = 0; z = 1;}
if (s > 1) nextdigit = mant[s-2];
if (lastdigit == 10) {
     if (nextdigit < 9) {lastdigit = 0; nextdigit++;}
     else if (nextdigit == 9) {
          lastdigit = 0;
          nextdigit = 0;
          for (i = s-2; mant[i] == 9; i--) if (i == 0) break;
          w = i;
          if (i == 0 && mant[0] == 9) z = 1;}}

if (z==1) {
     if (exponent>=0) {
          buffer += svar("1");
          for (i=0;i<exponent && i<s;i++) buffer += svar("0");
          for (; i<exponent; i++) buffer += svar("*");
          buffer += svar(".");
          for (i = 0; i < t; i++) buffer += svar("0");
          if (stflg) for (; i<k; i++) buffer += svar("*");}
     else {
          buffer += svar("0.");
          for (i=-1; i>exponent; i--) buffer += svar("0");
          buffer += svar("1");
          for (i = 0; i < s; i++) buffer += svar("0");
          if (stflg) for (i=0;i<k-t;i++) buffer += svar("*");}
     buffer += svar(" ");
     return;}

if (w>-1) {
     if (exponent>0) {
          for (i=0; i<w && i<exponent; i++)
          buffer += dtos(mant[i]);
          if (i==exponent) {
               buffer += svar(".");
               for (; i<w; i++) buffer += dtos(mant[i]);}
          buffer += ltos(mant[w]+1);
          for (i++;i<exponent && i<s;i++) buffer += svar("0");
          if (i==s) for (;i<exponent;i++) buffer += svar("*");
          if (i==exponent) buffer +=  svar(".");
          if (i<s) for (; i<s; i++) buffer += svar("0");
          if (stflg) {
               if (t >= 0) {
                    for (i=0; i<k-t; i++) buffer += svar("*");}
               else {
                    for (i=0; i<k; i++) buffer += svar("*");}}}
     else {
          buffer += svar("0.");
          for (i=0; i>exponent; i--) buffer += svar("0");
          for (i=0; i<w; i++) buffer += dtos(mant[i]);
          buffer += ltos(mant[w]+1);
          for (i++; i < s; i++) buffer += svar("0");
          if (stflg) for (i=0;i<k-t;i++) buffer += svar("*");}
     buffer += svar(" ");
     return;}

if (s==0) {
     if (exponent > 0) {
          for (i=0; i<exponent; i++) buffer += svar("*");
          buffer += svar(".");
          if (stflg) for (i=0;i<k;i++) buffer += svar("*");}
     else {
          buffer += svar("0.");
          for (i=0; i<t; i++) buffer += svar("0");
          if (stflg) for (;i<k;i++) buffer += svar("*");}
     buffer += svar(" ");
     return;}

if (s==1) {
     if (exponent>0) {
          buffer += dtos(lastdigit);
          if (rng == 0)
               for (i=1; i<exponent; i++) buffer += svar("0");
          else   
               for (i=1; i<exponent; i++) buffer += svar("*");
          buffer += svar(".");
          if (rng == 0)
               for (i=0; i<k; i++) buffer += svar("0");
          else
               if (stflg) for (i=0; i<k; i++) buffer += svar("*");}
     else {
          buffer += svar("0.");
          for (i=0; i<t-1; i++) buffer += svar("0");
          buffer += dtos(lastdigit);
          if (rng == 0)
               for (i++; i<k; i++) buffer += svar("0");
          else 
               if (stflg) for (i++; i<k; i++) buffer += svar("*");}
     if (t < k + 1) buffer += svar(" ");
     return;}

if (s>=2) {
     if (exponent>0) {
          for (i=0; i<exponent && i<s-2; i++) buffer += dtos(mant[i]);
          if (i==exponent) {
               buffer += svar(".");
               for (; i<s-2; i++) buffer += dtos(mant[i]);}
          buffer += dtos(nextdigit);
          if (++i == exponent) buffer += svar(".");
          buffer += dtos(lastdigit);
          if (++i == exponent) buffer += svar(".");
          if (i<exponent) {
               if (rng==0) {
                    for (;i<exponent;i++) buffer += svar("0");}
               else  {
                    for (;i<exponent;i++) buffer += svar("*");}
               buffer += svar(".");}
          if (t>=0) {
               if (rng == 0) {
                    for (i=0;i<k-t;i++) buffer += svar("0");}
               else {
                    if (stflg) for (i=0;i<k-t;i++) buffer += svar("*");}}
          else {
               if (rng == 0) {
                    for (i=0;i<k;i++) buffer += svar("0");}
               else {
                    if (stflg) for (i=0;i<k;i++) buffer += svar("*");}}}
     else {
          buffer += svar("0.");
          for (i=0; i>exponent; i--) buffer += svar("0");
          for (i=0; i<s-2; i++) buffer += dtos(mant[i]);
          buffer += dtos(nextdigit);
          buffer += dtos(lastdigit);
          if (rng == 0) {
               for (i=0; i<k-t; i++) buffer += svar("0");}
          else {
               if (stflg) for (i=0; i<k-t; i++) buffer += svar("*");}}
     if (t < k + 1) buffer += svar(" ");
     return;}
}

void floatprnt(char sgn, svar& buffer, int round, int k, int stflg,
               int s, int rng, long exponent, int* mant)
{
int i, w = -1, z = 0, lastdigit = 11, nextdigit = 11;

if (s > 0) lastdigit = mant[s-1] + round;
if (s == 1 && lastdigit == 10) {lastdigit = 1; exponent++;}
if (s == 0 && round == 1) {exponent++; s = 1; lastdigit = 1;}
if (s < 0) {exponent++; s = 0;}
if (s > 1) nextdigit = mant[s-2];
if (lastdigit == 10) {
     if (nextdigit < 9) {lastdigit = 0; nextdigit++;}
     else if (nextdigit == 9) {
          lastdigit = 0;
          nextdigit = 0;
          for (i = s-2; mant[i] == 9; i--) if (i == 0) break;
          w = i;
          if (i == 0 && mant[0] == 9) {exponent++; z = 1;}}}
if (sgn == '-') buffer += svar("-"); else buffer += svar(" ");
if (s == 0) buffer += svar("0");
else {
     exponent--;
     if (s == 1) buffer += dtos(lastdigit);
     else {
          if (z == 1) buffer += svar("1");
          else {
               if (w == 0) buffer += ltos(mant[0]+1);
               else {
                    if (s == 2) buffer += dtos(nextdigit);
                    else buffer += dtos(mant[0]);}}}}
buffer += svar(".");
if (s == 0 || s == 1) ;
else if (z == 1) for (i = 1; i < s; i++) buffer += svar("0");
else if (w == 0) for (i = 1; i < s; i++) buffer += svar("0");
else if (w > 0) {
     for (i=1; i<w; i++) buffer += dtos(mant[i]);
     buffer += ltos(mant[w]+1);
     for (i = w+1; i < s; i++) buffer += svar("0");}
else if (s == 2) buffer += dtos(lastdigit);
else {
     for (i=1; i<s-2; i++) buffer += dtos(mant[i]);
     buffer += dtos(nextdigit);
     buffer += dtos(lastdigit);}
if (rng == 0) for (i = 1; i <= k+1-s; i++) buffer += svar("0");
else {
     if (stflg) for (i = 1; i <= k+1-s && i <= k; i++) buffer += svar("*");}
if (s < k + 2) buffer += svar(" ");
buffer += svar(" E");
buffer += ltos(exponent);
return;
}


void set_up_mant(int* mant, rvar& a, int front, int back, int shift,
     int firstint, int lastint)
{
int l, i, j, i2; 

if (a.rv->len >= 2) {
     l = -front;
     for (i = m_base; i > 1; i /= 10) {
          j = i/10;
          if (l >= 0) mant[l] = (firstint%i)/j;
          l++;}
     for (i2 = 1; i2 < a.rv->len-1; i2++) {
          for (i = m_base; i > 1; i /= 10) {
               j = i/10;
               mant[l] = (a.rv->m[i2]%i)/j;
               l++;}}
     if (shift == 0) {
          i2 = -1;
          for (i = m_base; i > 1 && i2 < back; i /= 10) {
               j = i/10;
               mant[l] = (lastint%i)/j;
               l++; i2++;}}}
else {
     l = -front; i2 = -1;
     for (i = m_base; i > 1 && i2 < back; i /= 10) {
          j = i/10;
          if (l >= 0) mant[l] = (firstint%i)/j;
          l++; i2++;}}
return;
}


void round_mant(int* mant,int& s,int& t,int leng,int& rng,int& round,int p)
{
int i, i1;
long v, test;

while (1) {
     if (s < 0) return;
     if (p >= 2) {
          if (mant[s] < 4) {round = 0; return;}
          if (mant[s] > 5) {round = 1; return;}}
     v = mant[s];
     test = 5; i1 = s + 9;
     for (i = s+1; i < leng && i < i1; i++) {
          v = v*10 + mant[i];
          test *= 10;}
     if (i < leng) rng = 2;
     if (p == 1) {
          if ((v + rng) <= 5) {round = 0; return;}
          else if ((v - rng) >= 5) {round = 1; return;}
          else {p = 2; s--; t--; continue;}}
     if (p == 2) {
           if (mant[s] == 5) {
               if ((v - rng) >= 50) {round = 1; return;}
               else if ((v + rng) <= 55) {round = 0; s++; t++; return;}
               else {s--; t--; p = 3; continue;}}
          else if (mant[s] == 4) {
               if ((v + rng) <= 50) {round = 0; return;}
               else if ((v - rng) >= 45) {round = 1; s++; t++; return;}
               else {s--; t--; p = 3; continue;}}}
     if (p >= 3) {
           if (mant[s] == 5) {
               if ((v - rng) >= test) round = 1;
               else {round = 0; s++; t++;}
               return;}
          else if (mant[s] == 4) {
               if ((v + rng) <= test) round = 0;
               else {round = 1; s++; t++;}
               return;}}}
}

