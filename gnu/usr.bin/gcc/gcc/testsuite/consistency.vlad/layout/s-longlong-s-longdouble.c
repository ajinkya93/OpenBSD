#include <stdio.h>

static struct sss{
  long long f;
  struct {long double m;} snd;
} sss;

#define _offsetof(st,f) ((char *)&((st *) 16)->f - (char *) 16)

int main (void) {
  printf ("+++Struct longdouble inside struct starting with longlong:\n");
  printf ("size=%d,align=%d\n", sizeof (sss), __alignof__ (sss));
  printf ("offset-longlong=%d,offset-sss-longdouble=%d,\nalign-longlong=%d,align-sss-longdouble=%d\n",
          _offsetof (struct sss, f), _offsetof (struct sss, snd),
          __alignof__ (sss.f), __alignof__ (sss.snd));
  return 0;
}
