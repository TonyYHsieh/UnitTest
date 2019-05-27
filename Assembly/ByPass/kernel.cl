kernel void ByPass(const global int* in, global int* out)
{
    int gid = get_global_id(0);
    out[gid] = in[gid];
}

