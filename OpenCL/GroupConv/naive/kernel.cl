
kernel void Conv3x3(global const float* input, global const float* weight, global float* output,
        const int N, const int C, const int H, const int W, const int K, const int G)
{
    const int Y = 3;
    const int X = 3;

    int GC = C / G;
    int GK = K / G;

    int w  = get_global_id(0); // w
    int h  = get_global_id(1); // h
    int nk = get_global_id(2); // n, k
    int n  = nk / K;
    int k  = nk % K;
    int g  = k / GK;
    int gk = k - GK * g;

    float out = 0;

    for (int c=0; c<GC; c++)
    {
        for (int y=0; y<Y; y++)
        {
            for (int x=0; x<X; x++)
            {
                int in_index = (((n * G + g) * GC + c) * H + h + y - 1) * W + w + x - 1;
                int we_index = (((g * GK + gk) * GC + c) * Y + y) * X + x;
                bool vis = !((w + x - 1 < 0) || (w + x - 1 >= W) || (h + y - 1 < 0) || (h + y - 1 >= H));

                out += (vis ? (input[in_index] * weight[we_index]) : 0);
            }
        }
    }

    int out_index = ((n * K + k) * H + h) * W + w;
    output[out_index] = out;
}

