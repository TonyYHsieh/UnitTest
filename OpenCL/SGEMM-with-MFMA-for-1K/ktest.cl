
#pragma OPENCL EXTENSION cl_khr_fp16 : enable
#pragma OPENCL EXTENSION cl_khr_subgroups : enable

#define MIMIC_MFMA

#ifndef MIMIC_MFMA

__attribute__((always_inline, const, convergent)) float16 __llvm_amdgcn_mfma_f32_32x32x8f16(half4, half4, float16, int, int, int) __asm("llvm.amdgcn.mfma.f32.32x32x8f16");

#else // MIMIC_MFMA

float dot8(__local half* a, __local half* b, float c)
{
    for(size_t i=0; i<8; i++)
    {
        c += (a[i] * b[i]);
    }
    return c;
}

float16 __llvm_amdgcn_mfma_f32_32x32x8f16(half4 a, half4 b, float16 c, __local half* lds, size_t ld_lds)
{

    size_t local_id = get_local_id(0) % 64;
    size_t wave_id  = get_local_id(0) / 64;

    size_t lds_a_offset = 8 * 32 * (0 + wave_id * 2);
    size_t lds_b_offset = 8 * 32 * (1 + wave_id * 2);

    // 4 half data per thread
    size_t lds_a_write_offset = lds_a_offset + (local_id % 32) * ld_lds + (local_id / 32) * 4;
    size_t lds_b_write_offset = lds_b_offset + (local_id % 32) * ld_lds + (local_id / 32) * 4;

    lds[lds_a_write_offset + 0] = a.s0;
    lds[lds_a_write_offset + 1] = a.s1;
    lds[lds_a_write_offset + 2] = a.s2;
    lds[lds_a_write_offset + 3] = a.s3;

    lds[lds_b_write_offset + 0] = b.s0;
    lds[lds_b_write_offset + 1] = b.s1;
    lds[lds_b_write_offset + 2] = b.s2;
    lds[lds_b_write_offset + 3] = b.s3;

    sub_group_barrier(CLK_LOCAL_MEM_FENCE);

    size_t m = local_id / 32 * 4;
    size_t n = local_id % 32;

    size_t offset_a = lds_a_offset + m * ld_lds;
    size_t offset_b = lds_b_offset + n * ld_lds;

    float16 ret = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

#if 1
    ret.s0 = dot8(lds + offset_a, lds + offset_b, c.s0);
    offset_a += ld_lds;
    ret.s1 = dot8(lds + offset_a, lds + offset_b, c.s1);
    offset_a += ld_lds;
    ret.s2 = dot8(lds + offset_a, lds + offset_b, c.s2);
    offset_a += ld_lds;
    ret.s3 = dot8(lds + offset_a, lds + offset_b, c.s3);
    offset_a += ld_lds;

    offset_a += (4 * ld_lds);

    ret.s4 = dot8(lds + offset_a, lds + offset_b, c.s4);
    offset_a += ld_lds;
    ret.s5 = dot8(lds + offset_a, lds + offset_b, c.s5);
    offset_a += ld_lds;
    ret.s6 = dot8(lds + offset_a, lds + offset_b, c.s6);
    offset_a += ld_lds;
    ret.s7 = dot8(lds + offset_a, lds + offset_b, c.s7);
    offset_a += ld_lds;

    offset_a += (4 * ld_lds);

    ret.s8 = dot8(lds + offset_a, lds + offset_b, c.s8);
    offset_a += ld_lds;
    ret.s9 = dot8(lds + offset_a, lds + offset_b, c.s9);
    offset_a += ld_lds;
    ret.sa = dot8(lds + offset_a, lds + offset_b, c.sa);
    offset_a += ld_lds;
    ret.sb = dot8(lds + offset_a, lds + offset_b, c.sb);
    offset_a += ld_lds;

    offset_a += (4 * ld_lds);

    ret.sc = dot8(lds + offset_a, lds + offset_b, c.sc);
    offset_a += ld_lds;
    ret.sd = dot8(lds + offset_a, lds + offset_b, c.sd);
    offset_a += ld_lds;
    ret.se = dot8(lds + offset_a, lds + offset_b, c.se);
    offset_a += ld_lds;
    ret.sf = dot8(lds + offset_a, lds + offset_b, c.sf);
    offset_a += ld_lds;
#endif

    return ret;
}

#endif // MIMIC_MFMA

// To reduce code size, assume A is already transposed and so A[i, k] == AT[k, i]

static const __global float4 * ab_col(const __global float * restrict a, uint lda, uint ib, uint jb, uint l)
{
    return (const __global float4 *)(a + (jb*32 + l%32)*lda + ib*32 + (l > 31 ? 4 : 0));
}

static __global float4 * cd_col(__global float * restrict d, uint ldd, uint ib, uint jb, uint l)
{
    return (__global float4 *)      (d + (jb*32 + l%32)*ldd + ib*32 + (l > 31 ? 4 : 0));
}


// 1Kx1K matrix multiply
//     Decomposed into 32x32 blocks requiring only 4 v_mfma_f32_32x32x8f16 instructions to multiply
//     Everything is comes in as float; conversion to half4 where needed
//     Note lda, ldb must be a multiple of 4 to keep pointers aligned, and ldd should be a multiple of 16
//     Input A matrix is assumed transposed to simplify code
//
// Block based algorithm where each wave computes a block column of the product:
//
//   for jb = 0...31
//     for kb = 0...31
//        for ib = 0...31
//           D[ib,jb] += A[ib, kb] * B[kb, jb]
//
// Note that block B[kb, jb] is fixed in the inner loop and is stored in LDS
// Also note that block A[ib, kb] could be broadcast if all waves are in sync

__attribute__((reqd_work_group_size(256, 1, 1))) __kernel void
mixgemm1K(const __global float * restrict a, uint lda,
          const __global float * restrict b, uint ldb,
                __global float * restrict d, uint ldd)
{
#ifdef MIMIC_MFMA
    // (buffer:8*32) * (A/B:2) * (wave:4)
    __local half workspace[8 * 32 * 2 * 4];
#endif

    // 8 half4 per block column, 32 columns/wave, 4 wave/workgroup = 8KiB
    __local half4 bbs[4*64*4];

    // Each wave handles a block column of the result
    uint jb = get_global_id(0) / 64; // wave id

    // Lane id
    uint l = get_global_id(0) % 64;  // id in wave

    // B block column for this wave and lane in local memory
    // l : [0-63]
    __local half4 *bb = &bbs[4*64*(jb % 4) + l];

    for (uint kb=0; kb<MATRIX_SIZE/32; ++kb) {
        // Load LDS with block B[kb, jb]
        const __global float4 *bp = ab_col(b, ldb, kb, jb, l);
        bb[  0] = convert_half4(bp[0]); // 32 * 8 for B
        bb[ 64] = convert_half4(bp[2]);
        bb[128] = convert_half4(bp[4]);
        bb[192] = convert_half4(bp[6]);
        sub_group_barrier(CLK_LOCAL_MEM_FENCE);

        for (uint ib=0; ib<MATRIX_SIZE/32; ++ib) {
            const __global float4 *ap = ab_col(a, lda, kb, ib, l);
            __global float4 *dp = cd_col(d, ldd, ib, jb, l);

            float16 cd;
            if (kb != 0)
                cd = (float16)(dp[0], dp[2], dp[4], dp[6]);
            else
                cd = 0.0f;

#ifndef MIMIC_MFMA
            cd = __llvm_amdgcn_mfma_f32_32x32x8f16(convert_half4(ap[0]), bb[  0], cd, 0, 0, 0);
            cd = __llvm_amdgcn_mfma_f32_32x32x8f16(convert_half4(ap[2]), bb[ 64], cd, 0, 0, 0);
            cd = __llvm_amdgcn_mfma_f32_32x32x8f16(convert_half4(ap[4]), bb[128], cd, 0, 0, 0);
            cd = __llvm_amdgcn_mfma_f32_32x32x8f16(convert_half4(ap[6]), bb[192], cd, 0, 0, 0);
#else
            cd = __llvm_amdgcn_mfma_f32_32x32x8f16(convert_half4(ap[0]), bb[  0], cd, workspace, 8);
            cd = __llvm_amdgcn_mfma_f32_32x32x8f16(convert_half4(ap[2]), bb[ 64], cd, workspace, 8);
            cd = __llvm_amdgcn_mfma_f32_32x32x8f16(convert_half4(ap[4]), bb[128], cd, workspace, 8);
            cd = __llvm_amdgcn_mfma_f32_32x32x8f16(convert_half4(ap[6]), bb[192], cd, workspace, 8);
#endif

            dp[0] = cd.s0123;
            dp[2] = cd.s4567;
            dp[4] = cd.s89ab;
            dp[6] = cd.scdef;
        }
    }
}

