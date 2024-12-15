#include <stdio.h>

int
compare_name_depth (char * a, char * b)
{
  int count = 0;
  while (*a && *b && (*a == *b)) {
    if (*a == '.')
      count++;
    printf ("%c,%c\n", *a, *b);
    a++; b++;
  }
  if (*b && !*a && *b == '.')
    count++;
  if (*a && !*b && *a == '.')
    count++;

  return count;
}


int
group_length (char *a)
{
  int count = 0;
  while (*a) {
    if (*a == '.')
      count++;
    a++;
  }
  return (count+1);
}

main(int argc, char * argv[])
{
  printf ("%d\n",
	  compare_name_depth (argv[1],argv[2]));
  printf ("%d, %d\n",
	  group_length (argv[1]),
	  group_length (argv[2]));
}	  
