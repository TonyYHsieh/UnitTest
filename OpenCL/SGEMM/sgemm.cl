#pragma OPENCL EXTENSION cl_khr_fp16 : enable

#define MACRO_TILE_SIZE 32

#define A_LDS_OFFSET 0
#define B_LDS_OFFSET (MACRO_TILE_SIZE * PROCESS_K_PER_INSTRUCTION) 

#define PROCESS_K_PER_INSTRUCTION 2

#define MFMA_OUTPUT_WIDTH_PER_THREAD 4

// mimic mfma_f32_32x32x2f32 instruction
float16 __llvm_amdgcn_mfma_f32_32x32x2f32(float a, float b, float16 c, __local float* lds, size_t ld_lds)
{
    size_t local_id = get_local_id(0) + get_local_id(1) * get_local_size(0);

    size_t local_write_a_offset = A_LDS_OFFSET + (local_id % MACRO_TILE_SIZE) * ld_lds + (local_id / MACRO_TILE_SIZE);
    size_t local_write_b_offset = B_LDS_OFFSET + (local_id % MACRO_TILE_SIZE) * ld_lds + (local_id / MACRO_TILE_SIZE);

    // put data to LDS
    lds[local_write_a_offset] = a;
    lds[local_write_b_offset] = b;

    work_group_barrier(CLK_LOCAL_MEM_FENCE);

    size_t m = local_id / MACRO_TILE_SIZE * MFMA_OUTPUT_WIDTH_PER_THREAD;
    size_t n = local_id % MACRO_TILE_SIZE;

    size_t offset_a = A_LDS_OFFSET + m * ld_lds;
    size_t offset_b = B_LDS_OFFSET + n * ld_lds;

    float16 ret;

    ret.s0 = lds[offset_a] * lds[offset_b] + lds[offset_a+1] * lds[offset_b+1] + c.s0;
    offset_a += ld_lds;
    ret.s1 = lds[offset_a] * lds[offset_b] + lds[offset_a+1] * lds[offset_b+1] + c.s1;
    offset_a += ld_lds;
    ret.s2 = lds[offset_a] * lds[offset_b] + lds[offset_a+1] * lds[offset_b+1] + c.s2;
    offset_a += ld_lds;
    ret.s3 = lds[offset_a] * lds[offset_b] + lds[offset_a+1] * lds[offset_b+1] + c.s3;
    offset_a += ld_lds;

    offset_a += (4 * ld_lds);

    ret.s4 = lds[offset_a] * lds[offset_b] + lds[offset_a+1] * lds[offset_b+1] + c.s4;
    offset_a += ld_lds;
    ret.s5 = lds[offset_a] * lds[offset_b] + lds[offset_a+1] * lds[offset_b+1] + c.s5;
    offset_a += ld_lds;
    ret.s6 = lds[offset_a] * lds[offset_b] + lds[offset_a+1] * lds[offset_b+1] + c.s6;
    offset_a += ld_lds;
    ret.s7 = lds[offset_a] * lds[offset_b] + lds[offset_a+1] * lds[offset_b+1] + c.s7;
    offset_a += ld_lds;

    offset_a += (4 * ld_lds);

    ret.s8 = lds[offset_a] * lds[offset_b] + lds[offset_a+1] * lds[offset_b+1] + c.s8;
    offset_a += ld_lds;
    ret.s9 = lds[offset_a] * lds[offset_b] + lds[offset_a+1] * lds[offset_b+1] + c.s9;
    offset_a += ld_lds;
    ret.sa = lds[offset_a] * lds[offset_b] + lds[offset_a+1] * lds[offset_b+1] + c.sa;
    offset_a += ld_lds;
    ret.sb = lds[offset_a] * lds[offset_b] + lds[offset_a+1] * lds[offset_b+1] + c.sb;
    offset_a += ld_lds;

    offset_a += (4 * ld_lds);

    ret.sc = lds[offset_a] * lds[offset_b] + lds[offset_a+1] * lds[offset_b+1] + c.sc;
    offset_a += ld_lds;
    ret.sd = lds[offset_a] * lds[offset_b] + lds[offset_a+1] * lds[offset_b+1] + c.sd;
    offset_a += ld_lds;
    ret.se = lds[offset_a] * lds[offset_b] + lds[offset_a+1] * lds[offset_b+1] + c.se;
    offset_a += ld_lds;
    ret.sf = lds[offset_a] * lds[offset_b] + lds[offset_a+1] * lds[offset_b+1] + c.sf;

    return ret;
}

kernel void sgemm(__global const float* buffer_a,
                  __global const float* buffer_b,
                  __global const float* buffer_c,
                  __global       float* buffer_d,
                  int                   M,
                  int                   N,
                  int                   K,
                  int                   lda,
                  int                   ldb,
                  int                   ldc,
                  int                   ldd,
                  float                 alpha,
                  float                 beta)
{
    __local float _mfma_workspace_[MACRO_TILE_SIZE * PROCESS_K_PER_INSTRUCTION * 2];

    size_t local_x  = get_local_id(0);
    size_t local_y  = get_local_id(1);
    size_t local_id = get_local_id(0) + get_local_id(1) * get_local_size(0);

    size_t group_x = get_group_id(0);
    size_t group_y = get_group_id(1);

    float16 accumalte = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    for(size_t k=0; k<(K/PROCESS_K_PER_INSTRUCTION); k++)
    {
        // read A to LDS A
        size_t global_read_a_micro_tile_offset = group_x * MACRO_TILE_SIZE * lda + k * PROCESS_K_PER_INSTRUCTION;
        size_t global_read_a_offset            = global_read_a_micro_tile_offset + (local_id % MACRO_TILE_SIZE) * lda + (local_id / MACRO_TILE_SIZE);

        // read B to LDS B
        size_t global_read_b_micro_tile_offset = group_y * MACRO_TILE_SIZE * ldb + k * PROCESS_K_PER_INSTRUCTION;
        size_t global_read_b_offset            = global_read_b_micro_tile_offset + (local_id % MACRO_TILE_SIZE) * ldb + (local_id / MACRO_TILE_SIZE);

        accumalte = __llvm_amdgcn_mfma_f32_32x32x2f32(buffer_a[global_read_a_offset], buffer_b[global_read_b_offset], accumalte, _mfma_workspace_, PROCESS_K_PER_INSTRUCTION);
    }

#if 1
    if (K & 0x1)
    {
        size_t k = K/PROCESS_K_PER_INSTRUCTION;

        // read A to LDS A
        size_t global_read_a_micro_tile_offset = group_x * MACRO_TILE_SIZE * lda + k * PROCESS_K_PER_INSTRUCTION;
        size_t global_read_a_offset            = global_read_a_micro_tile_offset + (local_id % MACRO_TILE_SIZE) * lda + (local_id / MACRO_TILE_SIZE);
        float  a                               = (local_id < 32) ? buffer_a[global_read_a_offset] : 0;

        // read B to LDS B
        size_t global_read_b_micro_tile_offset = group_y * MACRO_TILE_SIZE * ldb + k * PROCESS_K_PER_INSTRUCTION;
        size_t global_read_b_offset            = global_read_b_micro_tile_offset + (local_id % MACRO_TILE_SIZE) * ldb + (local_id / MACRO_TILE_SIZE);
        float  b                               = (local_id < 32) ? buffer_b[global_read_b_offset] : 0;

        accumalte = __llvm_amdgcn_mfma_f32_32x32x2f32(a, b, accumalte, _mfma_workspace_, PROCESS_K_PER_INSTRUCTION);
    }
#endif

#if 1
    int out_x = group_x * MACRO_TILE_SIZE + (local_id / 32 * MFMA_OUTPUT_WIDTH_PER_THREAD);
    int out_y = group_y * MACRO_TILE_SIZE + (local_id % 32);

    __global const float* cp = buffer_c + out_x + out_y * ldc;
    __global       float* dp = buffer_d + out_x + out_y * ldd;

    // write result out
    if (out_y < N)
    {
        if (out_x<M)
            dp[0] = accumalte.s0 * alpha +  cp[0] * beta;
        out_x++; cp++; dp++;
        if (out_x<M)
            dp[0] = accumalte.s1 * alpha +  cp[0] * beta;
        out_x++; cp++; dp++;
        if (out_x<M)
            dp[0] = accumalte.s2 * alpha +  cp[0] * beta;
        out_x++; cp++; dp++;
        if (out_x<M)
            dp[0] = accumalte.s3 * alpha +  cp[0] * beta;
        out_x++; cp++; dp++;

        out_x+=4; cp+=4; dp+=4;

        if (out_x<M)
            dp[0] = accumalte.s4 * alpha +  cp[0] * beta;
        out_x++; cp++; dp++;
        if (out_x<M)
            dp[0] = accumalte.s5 * alpha +  cp[0] * beta;
        out_x++; cp++; dp++;
        if (out_x<M)
            dp[0] = accumalte.s6 * alpha +  cp[0] * beta;
        out_x++; cp++; dp++;
        if (out_x<M)
            dp[0] = accumalte.s7 * alpha +  cp[0] * beta;
        out_x++; cp++; dp++;

        out_x+=4; cp+=4; dp+=4;

        if (out_x<M)
            dp[0] = accumalte.s8 * alpha +  cp[0] * beta;
        out_x++; cp++; dp++;
        if (out_x<M)
            dp[0] = accumalte.s9 * alpha +  cp[0] * beta;
        out_x++; cp++; dp++;
        if (out_x<M)
            dp[0] = accumalte.sa * alpha +  cp[0] * beta;
        out_x++; cp++; dp++;
        if (out_x<M)
            dp[0] = accumalte.sb * alpha +  cp[0] * beta;
        out_x++; cp++; dp++;

        out_x+=4; cp+=4; dp+=4;

        if (out_x<M)
            dp[0] = accumalte.sc * alpha +  cp[0] * beta;
        out_x++; cp++; dp++;
        if (out_x<M)
            dp[0] = accumalte.sd * alpha +  cp[0] * beta;
        out_x++; cp++; dp++;
        if (out_x<M)
            dp[0] = accumalte.se * alpha +  cp[0] * beta;
        out_x++; cp++; dp++;
        if (out_x<M)
            dp[0] = accumalte.sf * alpha +  cp[0] * beta;
        out_x++; cp++; dp++;
    }
#else
    // calculate
    size_t globla_write_c_micro_tile_offset = group_x * MACRO_TILE_SIZE + group_y * MACRO_TILE_SIZE * ldc;
    size_t globla_read_c_offset             = globla_write_c_micro_tile_offset + (local_id % MACRO_TILE_SIZE) * ldc + local_id / MACRO_TILE_SIZE * MFMA_OUTPUT_WIDTH_PER_THREAD;
    __global const float4* cp               = (__global const float4*)(buffer_c + globla_read_c_offset);

    size_t globla_write_d_micro_tile_offset = group_x * MACRO_TILE_SIZE + group_y * MACRO_TILE_SIZE * ldd;
    size_t globla_write_d_offset            = globla_write_d_micro_tile_offset + (local_id % MACRO_TILE_SIZE) * ldd + local_id / MACRO_TILE_SIZE * MFMA_OUTPUT_WIDTH_PER_THREAD;
    __global float4* dp                     = (__global float4*)(buffer_d + globla_write_d_offset);

    // write result out
    dp[0] = accumalte.s0123 * alpha + cp[0] * beta;
    dp[2] = accumalte.s4567 * alpha + cp[2] * beta;
    dp[4] = accumalte.s89ab * alpha + cp[4] * beta;
    dp[6] = accumalte.scdef * alpha + cp[6] * beta;
#endif


}

