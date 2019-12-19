#include "ap_int.h"

void f(int A[50][100], ap_int<271> *result0, ap_int<13> *result1, ap_int<13> *result2,
       ap_int<271> *result3, ap_int<13> *result4)
{
    int N = 100;
    int M = 50;
    ap_int<271> a, b, c, tmp0;
    ap_int<13> a1, b1, c1, tmp1;
    ap_int<13> BB[5][5];
    a = 1;
    b = M;

    b = a + b;

    c = 4;
    a = b + c;

    a1 = 1;
    b1 = M;
    c1 = 4;
    a1 = b1 + c1;

    *result0 = a;
    *result1 = a1;

    BB[1][1] = 2;
    BB[2][2] = BB[1][1] + 4;

    *result2 = a + b1;
    *result3 = a1 + b;

    *result4 = BB[2][2];

    int pp = M + N;
}