
void subf(int A[56][100])
{
    int N = 100;
    int M = 50;
    for (int j = 1; j < N; j++)
        for (int i = 1; i < M + 1; i++)
            A[i][j] = A[i - 1][j - 1] + A[i][j - 1] + A[i - 1][j] + 1;
    for (int j = 1; j < N; j++)
        for (int i = 1; i < M + 1; i++)
            A[i][j] = A[i - 1][j - 1] + A[i][j - 1] + A[i - 1][j] + 1;
    return;
}

void f(int A[56][100])
{
    int N = 100;
    int M = 56;
    for (int j = 1; j < N; j++)
        for (int i = 1; i < M + 1; i++)
        {
            A[i][j] =
                A[i - 1][j - 1] + A[i][j - 1] + A[i - 1][j] +
                1; // load 3 store 1
                   // A[i+1][j] = A[i][j-1] + A[i+1][j-1] + A[i][j] + 1;       // load 1 store 1
                   // A[i+2][j] = A[i+1][j-1] + A[i+2][j-1] + A[i+1][j] + 1;   // load 1 store 1
                   // A[i+3][j] = A[i+2][j-1] + A[i+3][j-1] + A[i+2][j] + 1;   // load 1 store 1
        }
    subf(A);
    subf(A);
    return;
}
