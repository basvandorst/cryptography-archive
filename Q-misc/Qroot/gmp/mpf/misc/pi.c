#define a 10000
void *malloc ();
main (argc, argv)
     int argc;
     char **argv;
{
  int b,c,d,e,*f,g;
  int n = atoi (argv[1]);
  b=0,c=n*14/4,d=0,e=0,f=malloc((n*14/4 + 1) * sizeof (int)),g=0;
  for (;b-c;)
    f[b++]=a/5;
  for ( ;d=0, g=c*2; c-=14, printf("%.4d",e+d/a), e=d%a)
    for (b=c;d+=f[b]*a,f[b]=d%--g,d/=g--,--b;d*=b);
}
