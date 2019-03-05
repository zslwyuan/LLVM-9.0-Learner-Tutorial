#include "ap_int.h"
void f ( int A[50][100]) {
  int N = 100;
  int M = 50;
  ap_int<13> a,b,c;
  a = 1;
  b = M;
  c = 4;
  a = b + c;
  int pp = M + N;
}

int main()
{
   int aaa[50][100];
   f(aaa);
}