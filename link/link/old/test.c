
#include <sys/time.h>

main()
{
  int s;
  struct timeval t,t2;

  printf("test\n");
  gettimeofday(&t2,0);
  for(s=0;s<10;s++) {
    gettimeofday(&t,0);
    if(t.tv_usec < t2.tv_usec) {
      t.tv_usec += 1000000;
      t.tv_sec -=1;
    }
    if(t.tv_usec - t2.tv_usec > 900000) break;
  }
  printf("done.\n");
}

