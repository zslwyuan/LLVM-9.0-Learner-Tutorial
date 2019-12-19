#include "hls_video.h"
void f(int A[50][100])
{
    hls::Window<500, 500, int> window;
    int B[50][100];
    int N = 100;
    int M = 50;
    for (int j = 1; j < N; j++)
        for (int i = 1; i < M; i++)
        {
            window.insert_pixel(i + j, i, j);
            B[i][j] = 1 + i + j;
            A[i][j] = A[i - 1][j - 1] + A[i][j - 1] + A[i - 1][j] + 1;
        }
    for (int j = 1; j < N; j++)
        for (int i = 1; i < M; i++)
        {
            window.shift_left();

            A[i][j] = A[i - 1][j - 1] + A[i][j - 1] + A[i - 1][j] + window.getval(i, j) + B[i][j];
        }
    return;
}
