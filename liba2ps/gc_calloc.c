#include "config.h"

#include <stdlib.h>
#include <string.h>

void *
GC_calloc(size_t nmemb, size_t size)
{
  void *res = malloc (nmemb * size);
  if (res)
     memset (res, 0, nmemb * size);
  return res;
}
