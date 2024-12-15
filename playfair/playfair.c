char s[99],*z,*k,t,j,r,c,R,C,n,x,y,m,g;X(h){for(z=s;*z&&h!=*z;z++);j=z-s;}L(g
){if(isupper(g))g=tolower(g);return isalpha(g)?g=='j'?'i':g:0;}A(h){X(t=L(h))
;if(!*z&&t){*(z++)=t;*z=0;}}D(){r=j/5;c=j%5;}main(int a,char*v[]){k=v[2];if(a
<3){puts("playfair {e|d} key");}else{for(m=*v[1]=='e'?1:4;t=*(k++);)A(t);for(
c=0;c<99;c++)A(c);for(;c=getchar();) {if(n==1){if(c<0){P(x,'q');break;}y=L(c)
;if(x==y)P(x,'q');else if(y){P(x,y);n=0;}}else{if(c<0)break;if(x=L(c))n=1;}}}
}U(a,b){putchar(s[5*(a%5)+b%5]);if(++g>59){puts("");g=0;}}P(x,y){X(x);D();R=r
;C=c;X(y);D();if(R==r){U(R,C+m);U(r,c+m);}else if(C==c){U(R+m,C);U(r+m,c);}
else{U(R,c);U(r,C);}}/*mrr@ripem.msu.edu*/
