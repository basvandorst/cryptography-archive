rho1(n,x,y)=x=2;y=5;while(gcd(y-x,n)==1,x=(x*x+1)%n;y=(y*y+1)%n;y=(y*y+1)%n);gcd(n,y-x)
rho2(n,m)=m=rho1(n);if(isprime(m),print(m),rho2(m));if(isprime(n/m),print(n/m),rho2(n/m));
rho(n,m)=m=smallfact(n);print(m);n=m[length(m[,1]),1];if(isprime(n),,rho2(n));


