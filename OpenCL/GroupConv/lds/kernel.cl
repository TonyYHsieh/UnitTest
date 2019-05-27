
#define LCL_INPUT_SIZE (GROUP_SIZE+2)

#define Y 3
#define X 3

kernel void Conv3x3(global const float* input, global const float* weight, global float* output,
        const int N, const int C, const int H, const int W, const int K, const int G)
{
    __local float input_l[LCL_INPUT_SIZE][LCL_INPUT_SIZE];
    __local float weight_l[K_PER_WORK][3][3];

    float input_p[3][3];
    float weight_p[K_PER_WORK][3][3];
    
    float result[K_PER_WORK];
    for (int i=0; i<K_PER_WORK; i++)
    {
        result[i] = 0;
    }

    int GC = C / G;
    int GK = K / G;

    int group_x = get_group_id(0);
    int group_y = get_group_id(1);
    int nk = get_global_id(2) * K_PER_WORK;
    int n = nk / K;
    int k = nk - n * K;

    int g  = k / GK;
    int gk = k - g*GK; 

    int w_l = get_local_id(0);
    int h_l = get_local_id(1);
    int idx_l = h_l * GROUP_SIZE + w_l;

    int gid_x = group_x * GROUP_SIZE + w_l;
    int gid_y = group_y * GROUP_SIZE + h_l;

    // group input offset
    int input_offset_y = group_y * GROUP_SIZE - 1;
    int input_offset_x = group_x * GROUP_SIZE - 1;

    int input_offset_nc = ((n * G) + g) * GC * H * W;
    int wei_offset_kc = (g * GK + gk) * GC * Y * X;

    for (int c=0; c<GC; c++, input_offset_nc += H * W, wei_offset_kc += Y * X)
    {
        // get local data
        for (int i=idx_l; i<LCL_INPUT_SIZE*LCL_INPUT_SIZE; i+=GROUP_SIZE*GROUP_SIZE)
        {
            int y = i / LCL_INPUT_SIZE;
            int x = i - y * LCL_INPUT_SIZE;

            int gy = input_offset_y + y;
            int gx = input_offset_x + x;

            bool vis = (gy>=0 && gy<H && gx>=0 && gx<W);

            input_l[y][x] = vis ? input[input_offset_nc + gy * W + gx] : 0;
        }

        for (int i=idx_l; i<K_PER_WORK*Y*X; i+=GROUP_SIZE*GROUP_SIZE)
        {
            int k = i / (Y * X);
            int xy = i - k * Y * X;
            int y = xy / X;
            int x = xy - y * X;
            weight_l[k][y][x] = weight[wei_offset_kc + k * GC * Y * X + y * X + x];
        }

        barrier(CLK_LOCAL_MEM_FENCE);

        for(int j=0; j<3; j++)
        {
            for(int i=0; i<3; i++)
            {
                input_p[j][i] = input_l[h_l+j][w_l+i];
            }
        }

        for(int k_per_work=0; k_per_work<K_PER_WORK; k_per_work++)
        {
            for(int j=0; j<Y; j++) // for y
            {
                for(int i=0; i<X; i++) // for x
                {
                    result[k_per_work] += input_p[j][i] * weight_l[k_per_work][j][i];
                }
            }

        }

        barrier(CLK_LOCAL_MEM_FENCE);
    }

#if 1
    // write output
    int out_offset = nk * H * W + (group_y * GROUP_SIZE + h_l) * W + group_x * GROUP_SIZE + w_l;
    for(int k_per_work=0; k_per_work<K_PER_WORK; k_per_work++, out_offset += H*W)
    {
        output[out_offset] = result[k_per_work];
    }
#endif
}


