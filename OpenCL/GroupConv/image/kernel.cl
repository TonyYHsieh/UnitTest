
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

    for (int c=0; c<C; c++)
    {
        for (int y=0; y<Y; y++)
        {
            for (int x=0; x<X; x++)
            {
                int in_index = ((n * C + c) * H + h + y - 1) * W + w + x - 1;
                int we_index = ((k * C + c) * Y + y) * X + x;
                bool vis = !((w + x - 1 < 0) || (w + x - 1 >= W) || (h + y - 1 < 0) || (h + y - 1 >= H));
                out += (vis ? (input[in_index] * weight[we_index]) : 0);
//                out += (input[in_index]);
            }
        }
    }

    int out_index = ((n * K + k) * H + h) * W + w;
    output[out_index] = out;
}

#pragma OPENCL EXTENSION cl_khr_3d_image_writes : enable

kernel void Conv3x3_image(__read_only image3d_t input, __read_only image3d_t weight, __write_only image3d_t output,
        const int N, const int C, const int H, const int W, const int K)
{
    __constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_NONE | CLK_FILTER_NEAREST ;
    const int X = 3;
    const int Y = 3;


    int4 out_index;
    out_index.x = get_global_id(0); // w
    out_index.y = get_global_id(1); // h
    out_index.z = get_global_id(2); // n, k
    int n = out_index.z / K;
    int k = out_index.z % K;

    float4 out = 0;

    for (int c=0; c<C; c++)
    {
        for (int y=0; y<Y; y++)
        {
            for (int x=0; x<X; x++)
            {
                int4 in_index;
                int4 we_index;

                bool vis = !((out_index.x + x - 1 < 0) || (out_index.x + x - 1 >= W) || (out_index.y + y - 1 < 0) || (out_index.y + y - 1 >= H));

                // input index
                in_index.x = out_index.x + x - 1;
                in_index.y = out_index.y + y - 1;
                in_index.z = n * C + c;

                // weight index
                we_index.x = x;
                we_index.y = y;
                we_index.z = k * C + c;

                out.x += (vis ? (read_imagef(input, sampler, in_index).x * read_imagef(weight, sampler, we_index).x) : 0);
            }
        }
    }

    write_imagef(output, out_index, out);
}
