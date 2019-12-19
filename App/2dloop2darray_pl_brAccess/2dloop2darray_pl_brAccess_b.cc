void f(int A[100][50][321][456])
{
    int N = 50;
    int M = 100;
    int L = 321;
    int P = 456;
    for (int i = 1; i < M; i++)
        for (int j = 1; j < N; j++)
            for (int k = 1; k < L; k++)
                for (int q = 1; q < P; q++)
                {

                    A[i][j][k][q] = A[i][j][k][q] + 1;
                }
    return;
}
