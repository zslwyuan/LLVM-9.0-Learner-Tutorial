// RUN: %clang -cc1 %s -triple x86_64-linux-pc -emit-llvm -o - | FileCheck %s -check-prefix LIN
// RUN: %clang -cc1 %s -triple x86_64-windows-pc -emit-llvm -o - | FileCheck %s -check-prefix WIN

// CHECK: @[[VAR:[a-zA-Z0-9_]+]] = internal constant { i65 } { i65 17 }, align 8
// CHECK: @[[ARRAY:[a-zA-Za-zA-Z0-9_]+]] = internal constant [3 x i65] [i65 1, i65 2, i65 3], align 16

// template <int Bits>
// using ap_int = __attribute__((__ap_int(Bits))) unsigned;

template <int Bits>
using ap_int = __attribute__((__ap_int(Bits))) int;

void f(ap_int<13> a[15][15])
{
    int i,j;
    for (i=0;i<15;i++)
        for (j=0;j<15;j++)
        {
            a[i][j]++;
        }
}