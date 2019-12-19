#define TMAX 40
#define NX 15
#define NY 80

void kernel_fdtd_2d(int tmax, int nx, int ny, float ex_input[NX][NY], float ex_output[NX][NY],
                    float ey_input[NX][NY], float ey_output[NX][NY], float hz_input[NX][NY],
                    float hz_output[NX][NY])
{
    int t, i, j;

    for (i = 1; i < NX; i++)
        for (j = 0; j < NY; j++)
            ey_output[i][j] = ey_input[i][j] - (float)(0.5) * (hz_input[i][j] - hz_input[i - 1][j]);

    for (i = 0; i < NX; i++)
        for (j = 1; j < NY; j++)
            ex_output[i][j] = ex_input[i][j] - (float)(0.5) * (hz_input[i][j] - hz_input[i][j - 1]);

    for (i = 0; i < NX - 1; i++)
        for (j = 0; j < NY - 1; j++)
            hz_output[i][j] =
                hz_input[i][j] - (float)(0.7) * (ex_output[i][j + 1] - ex_output[i][j] +
                                                 ey_output[i + 1][j] - ey_output[i][j]);
}
