#include "aaaa.h"
#include <stdio.h>


void do_math(int *x) {
  *x += 5;
}

int main(void) {
  int result = -1, val = 4;
  do_math(&val);
  printf("%d\n",a);
  return result;
}