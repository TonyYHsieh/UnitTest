kernel void Fill10(global float* a)
{
    int gid = get_global_id(0);
    a[gid] = 10;
}

