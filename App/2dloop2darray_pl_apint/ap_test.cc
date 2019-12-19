#include "ap_int.h"

int f(ap_int<317> a, ap_int<17> b)
{

    a = a + 1;
    b = b + a;
    a = a + partSelect(b, 4, 1) + 1;

    return a;
}
