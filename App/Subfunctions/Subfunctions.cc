

void f2(int A[50][100])
{
    int N = 100;
    int M = 50;
    for (int j = 3; j < N; j++)
        for (int i = 1; i < M; i++)
            A[i][j] = A[i - 1][j] + A[i][j - 2] + A[i][j - 3];
    return;
}

void f1(int A[50][100], int B[50][100])
{
    f2(B);
    f2(A);
    return;
}