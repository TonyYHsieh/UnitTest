
kernel void Conv3x3(global const float* input, global const float* weight, global float* output,
        const int N, const int C, const int H, const int W, const int K)
{
    const int Y = 3;
    const int X = 3;

    int w = get_global_id(0); // w
    int h = get_global_id(1); // h
    int nk = get_global_id(2); // n, k
    int n = nk / K;
    int k = nk % K;

    float out = 0;

    for (int y=0; y<Y; y++)
    {
        for (int x=0; x<X; x++)
        {
            for (int c=0; c<C; c++)
            {    
                int in_index = ((n * H + h + y - 1) * W + w + x - 1) * C + c;
                int we_index = ((k * Y + y) * X + x) * C + c;
                bool vis = !((w + x - 1 < 0) || (w + x - 1 >= W) || (h + y - 1 < 0) || (h + y - 1 >= H));
                out += (vis ? (input[in_index] * weight[we_index]) : 0);
//                out += (input[in_index]);
            }
        }
    }

    int out_index = ((n * K + k) * H + h) * W + w;
    output[out_index] = out;
}

