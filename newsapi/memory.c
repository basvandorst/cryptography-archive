#include <stdlib.h>
#include <stdio.h>

/* these are simply wrappers around malloc and free */
#ifndef DEBUG_MALLOC

void * news_malloc (size_t size)
{
  return(malloc(size));
}

void news_free (void * ptr)
{
  free (ptr);
}

void * news_realloc (void * ptr, size_t newsize)
{
  return (realloc (ptr, newsize));
}

#else
void * news_malloc (size_t size)
{
  void * ptr;
  ptr = malloc(size);

  fprintf (stderr, "malloc: %p %d\n", ptr, (int)size);
  return (ptr);
}

void news_free (void * ptr)
{
  fprintf (stderr, "free: %p\n", ptr);
  free (ptr);
}

void * news_realloc (void * ptr, size_t newsize)
{
  fprintf (stderr, "realloc: %p %d\n", ptr, (int)newsize);
  return (realloc (ptr, newsize));
}

#endif


