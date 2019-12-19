void f(int A[56][100])
{
    int N = 100;
    int M = 56;
    for (int j = 1; j < N; j++)
    {
        for (int i = 1; i < M + 1; i++)
        {
            A[i][j] = (A[i - 1][j - 1] * A[i - 1][j - 1]) *
                          (A[i - 1][j] * A[i - 1][j] * A[i - 1][j] * A[i - 1][j] * A[i - 1][j]) +
                      1;
        }
    }
    return;
}
