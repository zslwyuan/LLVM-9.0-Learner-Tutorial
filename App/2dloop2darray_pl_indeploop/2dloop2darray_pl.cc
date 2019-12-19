
void f(int A[56][100], int B[56][100], int *C)
{
    int N = 100;
    int M = 56;
    for (int j = 1; j < N; j++)
        for (int i = 1; i < M + 1; i++)
        {
            A[i][j] = A[i - 1][j - 1] + A[i][j - 1] + A[i - 1][j] + 1;
        }

    {
        for (int j = 1; j < N; j++)
            for (int i = 1; i < M + 1; i++)
            {
                B[i][j] = B[i - 1][j - 1] + B[i][j - 1] + B[i - 1][j] + 1;
            }
    }
    return;
}
