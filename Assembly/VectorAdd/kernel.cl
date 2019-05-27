kernel void VectorAdd(
        const global int* ina,
        const global int* inb,
        global int* out)
{
    int gid = get_global_id(0);
    out[gid] = ina[gid] + inb[gid];
}

