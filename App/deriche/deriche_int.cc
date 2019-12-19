#define W 64
#define H 64

/* Main computational kernel. The whole function will be timed,
   including the call and return. */
/* Original code provided by Gael Deest */

void kernel_deriche(int w, int h, int alpha, int imgIn[W][H], int imgOut[W][H], int y1[W][H],
                    int y2[W][H], int xm1, int tm1, int ym1, int ym2, int xp1, int xp2, int tp1,
                    int tp2, int yp1, int yp2,

                    int k, int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8, int b1,
                    int b2, int c1, int c2)
{
    int i, j;

    for (i = 0; i < W; i++)
    {
        ym1 = (int)(0);
        ym2 = (int)(0);
        xm1 = (int)(0);
        for (j = 0; j < H; j++)
        {
            y1[i][j] = a1 * imgIn[i][j] + a2 * xm1 + b1 * ym1 + b2 * ym2;
            xm1 = imgIn[i][j];
            ym2 = ym1;
            ym1 = y1[i][j];
        }
    }

    for (i = 0; i < W; i++)
    {
        yp1 = (int)(0);
        yp2 = (int)(0);
        xp1 = (int)(0);
        xp2 = (int)(0);
        for (j = H - 1; j >= 0; j--)
        {
            y2[i][j] = a3 * xp1 + a4 * xp2 + b1 * yp1 + b2 * yp2;
            xp2 = xp1;
            xp1 = imgIn[i][j];
            yp2 = yp1;
            yp1 = y2[i][j];
        }
    }

    for (i = 0; i < W; i++)
        for (j = 0; j < H; j++)
        {
            imgOut[i][j] = c1 * (y1[i][j] + y2[i][j]);
        }

    for (j = 0; j < H; j++)
    {
        tm1 = (int)(0);
        ym1 = (int)(0);
        ym2 = (int)(0);
        for (i = 0; i < W; i++)
        {
            y1[i][j] = a5 * imgOut[i][j] + a6 * tm1 + b1 * ym1 + b2 * ym2;
            tm1 = imgOut[i][j];
            ym2 = ym1;
            ym1 = y1[i][j];
        }
    }

    for (j = 0; j < H; j++)
    {
        tp1 = (int)(0);
        tp2 = (int)(0);
        yp1 = (int)(0);
        yp2 = (int)(0);
        for (i = W - 1; i >= 0; i--)
        {
            y2[i][j] = a7 * tp1 + a8 * tp2 + b1 * yp1 + b2 * yp2;
            tp2 = tp1;
            tp1 = imgOut[i][j];
            yp2 = yp1;
            yp1 = y2[i][j];
        }
    }

    for (i = 0; i < W; i++)
        for (j = 0; j < H; j++)
            imgOut[i][j] = c2 * (y1[i][j] + y2[i][j]);
}
