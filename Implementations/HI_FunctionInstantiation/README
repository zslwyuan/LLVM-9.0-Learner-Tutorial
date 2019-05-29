The HI_FunctionInitiation pass is used to initiate the called sub-functions.

consider the following code:

void f0(int A[100],int B[100])
{
#pragma HLS array_partition variable=A factor=2
#pragma HLS array_partition variable=B factor=4
    f1(A);
    f1(B);
}

Althouh the arrays A and B are both processed by functon f1(), but the function with 
different input structures will end up with different latency. This will confuse the
process of timing evaluation. To overcome this issue, we can just generate two separate
function f1_0() and f1_1(), for arrays A and B respectively, and forward the pragma information
to the subfunctions.

BE CAREFUL OF THOSE "llvm.xxx" functions!!!!!!!!!!!!!