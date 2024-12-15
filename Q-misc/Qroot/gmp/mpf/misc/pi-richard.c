double sqrt ();

g(n)
{
  double a,b,p,x;
  int i;

  a=sqrt(2);
  x=sqrt(a);
  p=2+a;
  a=(x+1/x)/2;
  b=x;
  p=p*(a+1)/(b+1);
  printf ("%.16f\n", p);
  for (i=2; i<=n; i++)
    {
      x=sqrt(a);
      a=(x+1/x)/2;
      b=(b*x+1/x)/(b+1);
      p=p*(a+1)/(b+1);
      printf ("%.16f\n", p);
   }
}

main ()
{
  g(6);
}
