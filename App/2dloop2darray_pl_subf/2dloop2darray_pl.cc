__attribute__((always_inline)) int subFunc1(int i, int j)
{
    int ii = i + 1;
    int jj = j * 2;
    for (int k = 0; k < i; k++)
        if (k % 2)
            ii += k;
        else
            jj += k;

    return ii + jj;
}

int subFunc2(int i, int j)
{
    int ii = i + 1;
    int jj = j * 2;
    for (int k = 0; k < i; k++)
        if (k % 2)
            ii += k;

    return ii + jj;
}

void f(int A[50][100])
{
    int N = 100;
    int M = 50;
    for (int j = 1; j < N; j++)
        for (int i = 1; i < M; i++)
        {
            A[i][j] = A[i - 1][j - 1] + A[i][j - 1] + A[i - 1][j] + subFunc1(i, j) + subFunc2(i, j);
        }

    return;
}
