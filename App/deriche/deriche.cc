#define W 64
#define H 64

/* Main computational kernel. The whole function will be timed,
   including the call and return. */
/* Original code provided by Gael Deest */

void kernel_deriche(int w, int h, float alpha, float imgIn[W][H], float imgOut[W][H],
                    float y1[W][H], float y2[W][H], float xm1, float tm1, float ym1, float ym2,
                    float xp1, float xp2, float tp1, float tp2, float yp1, float yp2,

                    float k, float a1, float a2, float a3, float a4, float a5, float a6, float a7,
                    float a8, float b1, float b2, float c1, float c2)
{
    int i, j;

    for (i = 0; i < W; i++)
    {
        ym1 = (float)(0.0);
        ym2 = (float)(0.0);
        xm1 = (float)(0.0);
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
        yp1 = (float)(0.0);
        yp2 = (float)(0.0);
        xp1 = (float)(0.0);
        xp2 = (float)(0.0);
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
        tm1 = (float)(0.0);
        ym1 = (float)(0.0);
        ym2 = (float)(0.0);
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
        tp1 = (float)(0.0);
        tp2 = (float)(0.0);
        yp1 = (float)(0.0);
        yp2 = (float)(0.0);
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
