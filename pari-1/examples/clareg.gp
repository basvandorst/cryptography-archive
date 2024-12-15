rgcd(a,b,r)=a=abs(a);b=abs(b);while(b>0.01,r=a-b*trunc(a/b);a=b;b=r);a
{f(a,b,s,n,l)=s=a+b*t;n=abs(norm(s));nin=n;mv=vvector(li,j,0);
forprime(k=2,plim,l=0;while((n%k)==0,l=l+1;n=n/k);if(l,j=ind[k];cp=v[j][2];
while((a+b*cp)%k,j=j+1;cp=v[j][2]);mv[j]=l,));
if(n==1,lno=log(nin)/dep;vreg=vvector(lireg,j,if(j<=r1,log(abs(a+b*re[j])),log(norm(a+b*re[j]))));
if(res,mreg=concat(mreg,vreg);m=concat(m,mv);
areg=concat(areg,a+b*t),mreg=mat(vreg);m=mat(mv);areg=[a+b*t]);
res=res+1;print1("(",res,": ",a,", ",b,")"),)}
{clareg(p,plim,lima,extra)=vi=initalg(p);p=vi[1];t=modp(x,p);
dp=disc(p);r=vi[6];r1=vi[2][1];findex=vi[4];
if(findex>1,print("sorry, the case f>1 is not implemented");1/0,);
print("discriminant = ",vi[3],", signature = ",vi[2]);
dep=length(p)-1;re=vector(lireg=(dep+r1)/2,j,if(j<=r1,real(r[j]),r[2*j-r1-1]));
ind=vector(plim,j,0);v=[];
forprime(k=2,plim,w=lift(factmod(p,k));find=0;for(l=1,length(w[,1]),\
fa=w[l,1];if(length(fa)==2,if(find,,find=1;ind[k]=length(v)+1);
v=concat(v,[[k,-coeff(fa,0),w[l,2]]]),)));
li=length(v);co=li+extra;res=0;print("need ",co);
a=1;b=1;f(0,1);while(res<co,if(gcd(a,b)==1,f(a,b);f(-a,b),);
a=a+1;if(a*b>lima,b=b+1;a=1,));print(" ");mh=hermite(m);ms=matsize(mh);
if(ms[1]==ms[2],mhs=smith(mh);mh1=[mhs[1]];j=1;clh=mhs[1];
while(j<length(mhs),j=j+1;if(mhs[j]>1,mh1=concat(mh1,mhs[j]);clh=clh*mhs[j],));
print("class number = ",clh,", class group = ",mh1);km=kerint(m);mregh=mreg*km;
if(lireg==1,a1=1;print("regulator = 1"),coreg=length(mregh);
if(coreg<lireg-1,print("not enough relations for regulator: matrix size = ",matsize(mregh)),mreg1=extract(mregh~,cov=vector(lireg-1,k,k))~;a1=det(extract(mreg1,cov));
for(j=lireg,coreg,a1=rgcd(a1,det(extract(mreg1,vector(lireg-1,k,k+j-lireg+1)))));
print("regulator = ",a1))),print("not enough relations for class group: matrix size = ",ms));}
{check(lim)=r1=vi[2][1];pol=vi[1];
z=2^(-r1)*(2*pi)^((r1+1-length(pol))/2)*sqrt(abs(vi[3]));
forprime(q=2,lim,z=z*(q-1)/q;fa=factmod(pol,q);
for(j=1,length(fa[,1]),z=z/(1-1/q^(length(fa[j,1])-1))));
clh*a1/rootsof1(vi)[1]/z}
{fu()=fw=[];for(k=1,length(km),ckm=km[,k];s=1;
for(j=1,length(ckm),s=s*areg[j]^ckm[j]);fw=concat(fw,s));fw}



