The HI_RemoveRedundantAccess pass is used to remove redundant load by checking RAW and forwarding the data from previous store.

for (i=1;i<101;i++)
A[i][j] =  A[i-1][j] + 1;    1 R + 1 W

unroll 2  ===>

A[i][j] =  A[i-1][j] + 1;
A[i+1][j] =  A[i][j] + 1;

the stored data A[i][j] can be forwarded and a redundant load can be removed.