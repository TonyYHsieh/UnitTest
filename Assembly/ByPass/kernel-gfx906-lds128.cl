kernel void ByPass(const global int* in, global int* out)
{
    int lid = get_local_id(0);

    int gid = get_global_id(0);

    local int lds[64*2];

    lds[lid]       = lid;
    lds[lid +  64] = lid;

    work_group_barrier(CLK_LOCAL_MEM_FENCE);

    out[gid] = lds[lid] + lds[lid + 64];
}

