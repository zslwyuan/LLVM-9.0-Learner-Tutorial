void subf2(int A[50][100], int B[50][100], int i, int j)
{
    A[i][j] = A[i - 1][j] * A[i - 1][j] + A[i - 2][j] + B[i][j];
}

void f(int A[50][100])
{
    int N = 100;
    int M = 50;
    int B[50][100];
    for (int j = 0; j < N; j++)
        for (int i = 0; i < M; i++)
            B[i][j] = i;
    for (int j = 1; j < N; j++)
        for (int i = 1; i < M; i++)
        {

            A[i][j] = A[i - 1][j] * A[i - 1][j] + A[i - 2][j] + B[i][j];
        }
    for (int j = 1; j < N; j++)
        for (int i = 1; i < M; i++)
        {
            subf2(A, B, i, j);
        }
    return;
}
