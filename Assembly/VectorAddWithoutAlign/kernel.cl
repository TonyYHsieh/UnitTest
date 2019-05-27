kernel void VectorAdd(
        const global int* ina,
        const global int* inb,
        int size,
        global int* out)
{
    int gid = get_global_id(0);
    if (gid < size)
    {
        out[gid] = ina[gid] + inb[gid];
    }
}

