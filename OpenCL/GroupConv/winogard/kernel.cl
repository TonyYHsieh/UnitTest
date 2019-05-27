
#define LCL_INPUT_SIZE (GROUP_SIZE*2+2)

#define GROUP_TILE_SIZE (GROUP_SIZE * 2)

#define Y 3
#define X 3

kernel void WinoWeightConvert(global const float* input, global float* output, const int gkc)
{
    float wei_p[3][3];
    int idx = get_global_id(0);

    if (idx < gkc)
    {
        int in_offset = idx * 9;
        int out_offset = idx * 16;

        for (int j =0; j<3; j++)
        {
            for(int i=0; i<3; i++)
            {
                wei_p[j][i] = input[in_offset + j * 3 + i];
            }
        }

        output[out_offset + 0] = wei_p[0][0];
        output[out_offset + 1] = 0.5 * (wei_p[0][0] + wei_p[0][1] + wei_p[0][2]);
        output[out_offset + 2] = 0.5 * (wei_p[0][0] - wei_p[0][1] + wei_p[0][2]);
        output[out_offset + 3] = wei_p[0][2];

        output[out_offset + 1 * 4 + 0]
            = 0.5  * (wei_p[0][0] + wei_p[1][0] + wei_p[2][0]);
        output[out_offset + 1 * 4 + 1]
            = 0.25 * (wei_p[0][0] + wei_p[1][0] + wei_p[2][0] + wei_p[0][1] + wei_p[1][1] + wei_p[2][1] + wei_p[0][2] + wei_p[1][2] + wei_p[2][2]);
        output[out_offset + 1 * 4 + 2]
            = 0.25 * (wei_p[0][0] + wei_p[1][0] + wei_p[2][0] - wei_p[0][1] - wei_p[1][1] - wei_p[2][1] + wei_p[0][2] + wei_p[1][2] + wei_p[2][2]);
        output[out_offset + 1 * 4 + 3]
            = 0.5  * (wei_p[0][2] + wei_p[1][2] + wei_p[2][2]);

        output[out_offset + 2 * 4 + 0]
            = 0.5  * (wei_p[0][0] - wei_p[1][0] + wei_p[2][0]);
        output[out_offset + 2 * 4 + 1]
            = 0.25 * (wei_p[0][0] - wei_p[1][0] + wei_p[2][0] + wei_p[0][1] - wei_p[1][1] + wei_p[2][1] + wei_p[0][2] - wei_p[1][2] + wei_p[2][2]);
        output[out_offset + 2 * 4 + 2]
            = 0.25 * (wei_p[0][0] - wei_p[1][0] + wei_p[2][0] - wei_p[0][1] + wei_p[1][1] - wei_p[2][1] + wei_p[0][2] - wei_p[1][2] + wei_p[2][2]);
        output[out_offset + 2 * 4 + 3]
            = 0.5  * (wei_p[0][2] - wei_p[1][2] + wei_p[2][2]);

        output[out_offset + 3 * 4 + 0] = wei_p[2][0];
        output[out_offset + 3 * 4 + 1] = 0.5 * (wei_p[2][0] + wei_p[2][1] + wei_p[2][2]);
        output[out_offset + 3 * 4 + 2] = 0.5 * (wei_p[2][0] - wei_p[2][1] + wei_p[2][2]);
        output[out_offset + 3 * 4 + 3] = wei_p[2][2];
    }
}

kernel void Conv3x3(global const float* input, global const float* weight, global float* output,
        const int N, const int C, const int H, const int W, const int K, const int G)
{
    __local float input_l[LCL_INPUT_SIZE][LCL_INPUT_SIZE];
    __local float weight_l[K_PER_WORK][4][4];

    float in_p[4][4];
    float wino_D[4][4];
    float wino_M[4][4];
    float result[K_PER_WORK][2][2];

    int GC = C / G;
    int GK = K / G;

    int group_x = get_group_id(0);
    int group_y = get_group_id(1);
    int nk = get_global_id(2) * K_PER_WORK;
    int n = nk / K;
    int k = nk - n * K;

    int g = k / GK;
    int gk = k - g * GK;

    int w_l = get_local_id(0);
    int h_l = get_local_id(1);
    int idx_l = h_l * GROUP_SIZE + w_l;

    for(int k=0; k<K_PER_WORK; k++)
    {
        for(int j=0; j<2; j++)
        {
            for(int i=0; i<2; i++)
            {
                result[k][j][i] = 0;
            }
        }
    }

    // get local data
    int input_offset_y = group_y * GROUP_TILE_SIZE - 1;
    int input_offset_x = group_x * GROUP_TILE_SIZE - 1;

    int input_offset  = (n * G + g) * GC * H * W;
    int weight_offset = (g * GK + gk) * GC * (Y+1) * (X+1);

    for (int c=0; c<GC; c++, input_offset += H * W, weight_offset += (X+1)*(Y+1))
    {
#if 1
        for (int i=idx_l; i<LCL_INPUT_SIZE*LCL_INPUT_SIZE; i+=GROUP_SIZE*GROUP_SIZE)
        {
            int y = i / LCL_INPUT_SIZE;
            int x = i - y * LCL_INPUT_SIZE;

            int gy = input_offset_y + y;
            int gx = input_offset_x + x;

            bool vis = (gy>=0 && gy<H && gx>=0 && gx<W);

            input_l[y][x] = vis ? input[input_offset + gy * W + gx] : 0;
        }
#endif
#if 1
        for (int i=idx_l; i<K_PER_WORK*(Y+1)*(X+1); i+=GROUP_SIZE*GROUP_SIZE)
        {
            int k  = i / ((Y+1)*(X+1));
            int yx = i - k*(Y+1)*(X+1);
            int y  = yx / (X+1);
            int x  = yx - y * (X+1);
            weight_l[k][y][x] = weight[weight_offset + k * GC * (Y+1) * (X+1) + y * (X + 1) + x];
        }
#endif
#if 1
        barrier(CLK_LOCAL_MEM_FENCE);

        // get priv data
        for(int j=0; j<4; j++)
        {
            for(int i=0; i<4; i++)
            {
                in_p[j][i] = input_l[h_l*2+j][w_l*2+i];
            }
        }

        // get intermindate input data
        wino_D[0][0] =  in_p[0][0] - in_p[2][0] - in_p[0][2] + in_p[2][2];
        wino_D[0][1] =  in_p[0][1] - in_p[2][1] + in_p[0][2] - in_p[2][2];
        wino_D[0][2] = -in_p[0][1] + in_p[2][1] + in_p[0][2] - in_p[2][2];
        wino_D[0][3] =  in_p[0][1] - in_p[2][1] - in_p[0][3] + in_p[2][3];

        wino_D[1][0] =  in_p[1][0] + in_p[2][0] - in_p[1][2] - in_p[2][2];
        wino_D[1][1] =  in_p[1][1] + in_p[2][1] + in_p[1][2] + in_p[2][2];
        wino_D[1][2] = -in_p[1][1] - in_p[2][1] + in_p[1][2] + in_p[2][2];
        wino_D[1][3] =  in_p[1][1] + in_p[2][1] - in_p[1][3] - in_p[2][3];

        wino_D[2][0] = -in_p[1][0] + in_p[2][0] + in_p[1][2] - in_p[2][2];
        wino_D[2][1] = -in_p[1][1] + in_p[2][1] - in_p[1][2] + in_p[2][2];
        wino_D[2][2] =  in_p[1][1] - in_p[2][1] - in_p[1][2] + in_p[2][2];
        wino_D[2][3] = -in_p[1][1] + in_p[2][1] + in_p[1][3] - in_p[2][3];

        wino_D[3][0] =  in_p[1][0] - in_p[3][0] - in_p[1][2] + in_p[3][2];
        wino_D[3][1] =  in_p[1][1] - in_p[3][1] + in_p[1][2] - in_p[3][2];
        wino_D[3][2] = -in_p[1][1] + in_p[3][1] + in_p[1][2] - in_p[3][2];
        wino_D[3][3] =  in_p[1][1] - in_p[3][1] - in_p[1][3] + in_p[3][3];

        // dot product
        for (int k=0; k<K_PER_WORK; k++)
        {
            for (int j=0; j<4; j++)
            {
                for (int i=0; i<4; i++)
                {
                    wino_M[j][i] = weight_l[k][j][i] * wino_D[j][i];
                }
            }

            result[k][0][0] += (wino_M[0][0] + wino_M[1][0] + wino_M[2][0] + wino_M[0][1] + wino_M[1][1] + wino_M[2][1] + wino_M[0][2] + wino_M[1][2] + wino_M[2][2]);
            result[k][0][1] += (wino_M[0][1] + wino_M[1][1] + wino_M[2][1] - wino_M[0][2] - wino_M[1][2] - wino_M[2][2] - wino_M[0][3] - wino_M[1][3] - wino_M[2][3]);
            result[k][1][0] += (wino_M[1][0] - wino_M[2][0] - wino_M[3][0] + wino_M[1][1] - wino_M[2][1] - wino_M[3][1] + wino_M[1][2] - wino_M[2][2] - wino_M[3][2]);
            result[k][1][1] += (wino_M[1][1] - wino_M[2][1] - wino_M[3][1] - wino_M[1][2] + wino_M[2][2] + wino_M[3][2] - wino_M[1][3] + wino_M[2][3] + wino_M[3][3]);
        }

        barrier(CLK_LOCAL_MEM_FENCE);
#endif
    }

#if 1
    // write output
    int out_offset = (n * K + k) * H * W + (group_y * GROUP_SIZE + h_l) * 2 * W + (group_x * GROUP_SIZE + w_l) * 2;
    for (int k=0; k<K_PER_WORK; k++, out_offset += H * W)
    {
        int out_offset2 = out_offset;
        for(int j=0; j<2; j++, out_offset2 += W)
        {
            int out_offset3 = out_offset2;
            for(int i=0; i<2; i++, out_offset3 += 1)
            {
                output[out_offset3] = result[k][j][i];
            }
        }
    }
#endif
}


