{squfof(n)=if(isprime(n),s=n,if(issquare(n),s=isqrt(n),p=smallfact(n)[1,1];
if(p!=n,s=p,if(n%4==1,dd=n;d=isqrt(dd);b=2*((d-1)\2)+1,dd=4*n;d=isqrt(dd);
b=2*(d\2));f=qfr(1,b,(b^2-dd)/4,0.);q=[];lq=0;ii=0;l=isqrt(d);flag=1;
while(flag,f=rhorealnod(f,d);ii=ii+1;a=compo(f,1);
if(ii%2,fl=0,fl=issquare(a));if(fl,as=isqrt(a);j=1;
while((j<=lq)&&fl,fl=(as!=q[j]);
j=j+1);if(as==1,s=0;flag=0,),);if(fl==0,if(abs(a)<=l,q=concat(q,abs(a));
print(q);lq=lq+1,),flag=0;gs=gcd(as,dd);print("i = ",ii);print(f);
gs=gcd(gs,bb=compo(f,2));
if(gs>1,s=gs,g=redrealnod(qfr(as,-bb,as*compo(f,3),0.),d);
fl=1;b=compo(g,2);while(fl,b1=b;g=rhorealnod(g,d);b=compo(g,2);fl=(b1!=b));
a=abs(compo(g,1));if(a%2,,a=a/2);s=a))))));s}





