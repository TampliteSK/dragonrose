// misc.c

#include "stdio.h"
#include <time.h>

#ifdef WIN32
#include "windows.h"
#else
#include "sys/time.h"
#endif

unsigned long long GetTimeMs() {
#ifdef WIN32
  return GetTickCount();
#else
  struct timeval t;
  gettimeofday(&t, NULL);
  return t.tv_sec*1000 + t.tv_usec/1000;
#endif
}