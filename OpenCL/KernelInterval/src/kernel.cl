kernel void copy(global const float* in, global float* out)
{
    int gid = get_global_id(0);
    out[gid] = in[gid];
}

