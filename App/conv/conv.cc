
void convs(int input[32][32], int output[32][32], int filter2[3][3], int filter1[5][5],
           int filter0[5][5])
{
    int mid0[32][32];
    for (int i = 2; i < 30; i++)
    {
        for (int j = 2; j < 30; j++)
        {
            int tmp = 0;
            for (int k = -2; k <= 2; k++)
            {
                for (int l = -2; l <= 2; l++)
                {
                    tmp += filter0[2 + k][2 + l] * input[i + k][j + l];
                }
            }
            mid0[i][j] = tmp;
        }
    }

    int mid1[32][32];
    for (int i = 2; i < 30; i += 1)
    {
        for (int j = 2; j < 30; j += 1)
        {
            int tmp = 0;
            for (int k = -2; k <= 2; k++)
            {
                for (int l = -2; l <= 2; l++)
                {
                    tmp += filter1[2 + k][2 + l] * mid0[i + k][j + l] * 321;
                }
            }
            mid1[i][j] = tmp > 0 ? tmp * 23 : tmp;
        }
    }

    for (int i = 1; i < 31; i += 1)
    {
        for (int j = 1; j < 31; j += 1)
        {
            int tmp = 0;
            for (int k = -1; k <= 1; k++)
            {
                for (int l = -1; l <= 1; l++)
                {
                    tmp += filter2[1 + k][1 + l] * mid1[i + k][j + l];
                }
            }
            output[i][j] = tmp;
        }
    }
}