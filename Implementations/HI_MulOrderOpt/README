HI_MulOrderOpt will try to transform the series of multiplication (e.g. A*A*A*A*A*B*B) into a parallelism-friendly IR code sequence

possible computation sequence:
stage0: a0 = A * A
stage1: a1 = A * a0;
stage2: a2 = A * a1;
stage3: a3 = A * a2;
stage4: o0 = a3 * B;
stage5: o1 = o0 * B

which is not parallelized and cannot take the advantage of FPGA

Then re-genenrate the multiplication to reduce computation dependence and maximize parallelism

stage0: a0 = A * A              a1 = A * A        b = B * B
stage1: a2 = a0 * a1;           c0 = A * b
stage2: c1 = a2 * c0

(a0 and a1 is duplicate, which will be removed by latter passes.)