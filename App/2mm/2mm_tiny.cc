
#define NI 16
#define NJ 18
#define NK 22
#define NL 24

void kernel_2mm(int ni, int nj, int nk, int nl, int alpha, int beta, int tmp[NI][NJ], int A[NI][NK],
                int B[NK][NJ], int C[NJ][NL], int D_input[NI][NL], int D_output[NI][NL])
{
    int i, j, k;

    for (i = 0; i < NI; i++)
        for (j = 0; j < NJ; j++)
        {
            int res = 0;
            for (k = 0; k < NK; ++k)
                res += alpha * A[i][k] * B[k][j];
            tmp[i][j] = res;
        }

    for (i = 0; i < NI; i++)
        for (j = 0; j < NL; j++)
        {
            int sum = D_input[i][j] * beta;
            for (k = 0; k < NJ; ++k)
                sum += tmp[i][k] * C[k][j];
            D_output[i][j] = sum;
        }
}
