void f(int A[50][100], int C[50], int rc)
{
    int N = 100;
    int M = 50;
    for (int j = 1; j < N; j++)
    {
        // #pragma clang loop unroll_count(4)
        for (int i = 1; i < M + 1; i++)
            C[rc] += A[i][j] + 1;
    }
    // A[i][j] = A[i-1][j] + 1;
    // for ( int j = 1; j < N; j++ )
    //   for ( int i = 1; i < M; i++ )
    //     A[i][j] = A[i-1][j-1] + A[i][j-1] + A[i-1][j] + 1;
    return;
}
