\e
t=x^4+24*x^2+585*x+1791;nf=initalg(t);A=nf[1]
\precision=18
gc=galoisconj(A)
\precision=28
aut=gc[4]
pd=primedec(nf,7)
pr1=pd[1]
hp=idealmul(nf,idmat(4),pr1)
hp3=idealmul(nf,hp,idealmul(nf,hp,hp))
\\ or hp3=idealpow(nf,hp,3)
for(j=1,4,print(idealval(nf,hp3,pd[j])))
hpi3=[hp3,[0.,0.]];hr1=ideallllred(nf,hpi3,0)
hr=hr1;for(j=1,3,hr=ideallllred(nf,hr,[1,5]);print(hr))
arch=hr[2]-hr1[2];l1=arch[1];l2=arch[2];
s=real(l1+l2)/4;v1=[l1,l2,conj(l1),conj(l2)]~/2-[s,s,s,s]~
m1=nf[5][1];m=matrix(4,4,j,k,if(j<=2,m1[j,k],conj(m1[j-2,k])));
v=exp(v1);au=gauss(m,v)
vu=round(real(au))
u=mod(nf[7]*vu,A)
norm(u)
f1=factor(subst(char(u,x),x,x^2))
f1=factor(subst(char(-u,x),x,x^2))
v=sqrt(-v)
au=gauss(m,v)
v[1]=-v[1];v[3]=-v[3];au=gauss(m,v)
vu2=round(real(au))
u2=mod(nf[7]*vu2,A)
q=polred2(f1[1,1])
up2=modreverse(mod(q[3,1],f1[1,1]))
mod(subst(lift(up2),x,aut),A)
r=f1[1,1]%(x^2+u);-coeff(r,0)/coeff(r,1)
al=mod(x^2-9,A)
principalidele(nf,al)
for(j=1,4,print(j,": ",idealval(nf,al,pd[j])))
norm(al)
pd14=idealmul(nf,pd[1],pd[4])
idealmul(nf,al,idmat(4))
setrand(1);v=buchgenfu(A,0.2,0.2)[,1]
uf=mod(v[9][1],A)
uu=mod(v[8][2],A)
cu2=log(conjvec(u2));cuf=log(conjvec(uf));cuu=log(conjvec(uu));
lindep(real([cu2[1],cuf[1],cuu[1]]))
lindep([cu2[1],cuf[1],cuu[1],2*i*pi])
u2/uf
ru=nf[8]*vvector(4,j,coeff(v[8][2],j-1))
nf[7]*ru
setrand(1);bnf=buchinitfu(A,0.2,0.2);
bnf[8]
nf=bnf[7];
hp4=idealpow(nf,hp,4)
vis=isprincipal(bnf,hp4)
alpha=mod(nf[7]*vis[2],A)
norm(alpha)
idealmul(nf,idmat(4),mat(vis[2]))
vit=isprincipal(bnf,hp)
pp=isprincipal(bnf,pd14)
al2=mod(nf[7]*pp[2],A)
u3=al2/al
char(u3,x)
me=concat(bnf[3],[2,2]~)
cu3=principalidele(nf,u3)[2]
xc=gauss(real(me),real(cu3))
xd=cu3-me*xc
xu=principalidele(nf,uu)[2]
xd[1]/xu[1]
isunit(bnf,u3)
uu^3*uf