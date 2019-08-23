	.text
	.hsa_code_object_version 2,1
	.hsa_code_object_isa 9,0,8,"AMD","AMDGPU"
	.globl	mixgemm1K               ; -- Begin function mixgemm1K
	.p2align	8
	.type	mixgemm1K,@function
	.amdgpu_hsa_kernel mixgemm1K
mixgemm1K:                              ; @mixgemm1K
	.amd_kernel_code_t
		amd_code_version_major = 1
		amd_code_version_minor = 2
		amd_machine_kind = 1
		amd_machine_version_major = 9
		amd_machine_version_minor = 0
		amd_machine_version_stepping = 8
		kernel_code_entry_byte_offset = 256
		kernel_code_prefetch_byte_size = 0
		granulated_workitem_vgpr_count = 11
		granulated_wavefront_sgpr_count = 2
		priority = 0
		float_mode = 192
		priv = 0
		enable_dx10_clamp = 1
		debug_mode = 0
		enable_ieee_mode = 1
		enable_wgp_mode = 0
		enable_mem_ordered = 0
		enable_fwd_progress = 0
		enable_sgpr_private_segment_wave_byte_offset = 0
		user_sgpr_count = 6
		enable_trap_handler = 0
		enable_sgpr_workgroup_id_x = 1
		enable_sgpr_workgroup_id_y = 0
		enable_sgpr_workgroup_id_z = 0
		enable_sgpr_workgroup_info = 0
		enable_vgpr_workitem_id = 0
		enable_exception_msb = 0
		granulated_lds_size = 0
		enable_exception = 0
		enable_sgpr_private_segment_buffer = 1
		enable_sgpr_dispatch_ptr = 0
		enable_sgpr_queue_ptr = 0
		enable_sgpr_kernarg_segment_ptr = 1
		enable_sgpr_dispatch_id = 0
		enable_sgpr_flat_scratch_init = 0
		enable_sgpr_private_segment_size = 0
		enable_sgpr_grid_workgroup_count_x = 0
		enable_sgpr_grid_workgroup_count_y = 0
		enable_sgpr_grid_workgroup_count_z = 0
		enable_wavefront_size32 = 0
		enable_ordered_append_gds = 0
		private_element_size = 1
		is_ptr64 = 1
		is_dynamic_callstack = 0
		is_debug_enabled = 0
		is_xnack_enabled = 0
		workitem_private_segment_byte_size = 0
		workgroup_group_segment_byte_size = 8192
		gds_segment_byte_size = 0
		kernarg_segment_byte_size = 96
		workgroup_fbarrier_count = 0
		wavefront_sgpr_count = 20
		workitem_vgpr_count = 47
		reserved_vgpr_first = 0
		reserved_vgpr_count = 0
		reserved_sgpr_first = 0
		reserved_sgpr_count = 0
		debug_wavefront_private_segment_offset_sgpr = 0
		debug_private_segment_buffer_sgpr = 0
		kernarg_segment_alignment = 4
		group_segment_alignment = 4
		private_segment_alignment = 4
		wavefront_size = 6
		call_convention = -1
		runtime_loader_kernel_symbol = 0
	.end_amd_kernel_code_t
; %bb.0:
	s_load_dwordx2 s[0:1], s[4:5], 0x0
	s_load_dword s2, s[4:5], 0x8
	s_load_dwordx2 s[8:9], s[4:5], 0x10
	s_load_dword s3, s[4:5], 0x18
	s_load_dwordx2 s[10:11], s[4:5], 0x20
	s_load_dword s7, s[4:5], 0x28
	s_load_dwordx2 s[12:13], s[4:5], 0x30
	s_lshl_b32 s4, s6, 8
	v_or_b32_e32 v0, s4, v0
	v_mov_b32_e32 v9, 0
	v_mov_b32_e32 v1, 0
	s_waitcnt lgkmcnt(0)
	v_add_co_u32_e32 v0, vcc, s12, v0
	v_mov_b32_e32 v2, s13
	v_addc_co_u32_e32 v2, vcc, v2, v9, vcc
	v_alignbit_b32 v2, v2, v0, 6
	v_lshlrev_b32_e32 v4, 5, v2
	v_and_b32_e32 v2, 31, v0
	v_or_b32_e32 v7, v2, v4
	v_lshlrev_b32_e32 v3, 2, v0
	v_and_b32_e32 v6, 63, v0
	v_mul_lo_i32 v0, v7, s3
	v_and_b32_e32 v3, 0x300, v3
	v_or_b32_e32 v3, v3, v6
	s_mov_b32 s5, 0
	v_lshlrev_b64 v[4:5], 2, v[0:1]
	v_mov_b32_e32 v0, s9
	v_add_co_u32_e32 v8, vcc, s8, v4
	v_mul_lo_i32 v4, v7, s7
	v_addc_co_u32_e32 v0, vcc, v0, v5, vcc
	v_cmp_lt_u32_e32 vcc, 31, v6
	v_cndmask_b32_e64 v10, 0, 16, vcc
	v_mov_b32_e32 v5, v1
	v_mov_b32_e32 v6, s11
	v_add_co_u32_e32 v7, vcc, s10, v10
	v_lshlrev_b64 v[4:5], 2, v[4:5]
	v_addc_co_u32_e32 v6, vcc, v6, v9, vcc
	v_add_co_u32_e32 v4, vcc, v7, v4
	v_addc_co_u32_e32 v5, vcc, v6, v5, vcc
	v_add_co_u32_e32 v6, vcc, v8, v10
	v_addc_co_u32_e32 v7, vcc, v0, v9, vcc
	v_add_co_u32_e32 v8, vcc, s0, v10
	v_mov_b32_e32 v0, s1
	v_lshlrev_b32_e32 v3, 3, v3
	v_addc_co_u32_e32 v9, vcc, v0, v9, vcc
	v_mov_b32_e32 v10, 0xffff
	s_movk_i32 s3, 0x60
	s_movk_i32 s6, 0xa0
	s_movk_i32 s7, 0xc0
	s_movk_i32 s8, 0xe0
	s_mov_b32 s9, 0
BB0_1:                                  ; =>This Loop Header: Depth=1
                                        ;     Child Loop BB0_2 Depth 2
	s_lshl_b32 s0, s9, 5
	s_mov_b32 s1, s5
	s_lshl_b64 s[0:1], s[0:1], 2
	v_mov_b32_e32 v0, s1
	v_add_co_u32_e32 v25, vcc, s0, v6
	v_addc_co_u32_e32 v26, vcc, v7, v0, vcc
	global_load_dwordx4 v[13:16], v[25:26], off
	global_load_dwordx4 v[17:20], v[25:26], off offset:32
	global_load_dwordx4 v[21:24], v[25:26], off offset:64
	global_load_dwordx4 v[25:28], v[25:26], off offset:96
	v_add_co_u32_e32 v11, vcc, s0, v8
	v_addc_co_u32_e32 v12, vcc, v9, v0, vcc
	v_cmp_eq_u32_e64 s[10:11], s9, 0
	s_mov_b64 s[12:13], 0
	s_mov_b32 s14, s5
	s_mov_b32 s15, s5
	s_waitcnt vmcnt(3)
	v_cvt_f16_f32_e32 v0, v13
	v_cvt_f16_f32_e32 v13, v14
	v_cvt_f16_f32_e32 v14, v15
	v_cvt_f16_f32_e32 v15, v16
	s_waitcnt vmcnt(2)
	v_cvt_f16_f32_e32 v16, v17
	v_cvt_f16_f32_e32 v17, v18
	v_cvt_f16_f32_e32 v18, v19
	v_cvt_f16_f32_e32 v19, v20
	s_waitcnt vmcnt(1)
	v_cvt_f16_f32_e32 v20, v21
	v_cvt_f16_f32_e32 v21, v22
	v_cvt_f16_f32_e32 v22, v23
	v_cvt_f16_f32_e32 v23, v24
	s_waitcnt vmcnt(0)
	v_cvt_f16_f32_e32 v24, v25
	v_cvt_f16_f32_e32 v25, v26
	v_cvt_f16_f32_e32 v26, v27
	v_cvt_f16_f32_e32 v27, v28
	v_and_b32_e32 v14, v10, v14
	v_and_b32_e32 v0, v10, v0
	v_and_b32_e32 v18, v10, v18
	v_and_b32_e32 v28, v10, v16
	v_and_b32_e32 v22, v10, v22
	v_and_b32_e32 v20, v10, v20
	v_and_b32_e32 v26, v10, v26
	v_and_b32_e32 v24, v10, v24
	v_lshl_or_b32 v16, v19, 16, v18
	v_lshl_or_b32 v14, v15, 16, v14
	v_lshl_or_b32 v15, v17, 16, v28
	v_lshl_or_b32 v17, v21, 16, v20
	v_lshl_or_b32 v13, v13, 16, v0
	v_lshl_or_b32 v18, v23, 16, v22
	v_lshl_or_b32 v20, v27, 16, v26
	v_lshl_or_b32 v19, v25, 16, v24
	ds_write2st64_b64 v3, v[13:14], v[15:16] offset1:1
	ds_write2st64_b64 v3, v[17:18], v[19:20] offset0:2 offset1:3
	; wave barrier
	ds_read2st64_b64 v[13:16], v3 offset1:1
	ds_read2st64_b64 v[17:20], v3 offset0:2 offset1:3
BB0_2:                                  ;   Parent Loop BB0_1 Depth=1
                                        ; =>  This Inner Loop Header: Depth=2
	s_lshl_b32 s4, s15, 5
	s_lshl_b64 s[0:1], s[4:5], 2
	s_and_b64 s[16:17], exec, s[10:11]
	v_mov_b32_e32 v0, s1
	v_add_co_u32_e32 v24, vcc, s0, v4
	v_addc_co_u32_e32 v23, vcc, v5, v0, vcc
	v_cndmask_b32_e64 v21, 0, 1, s[16:17]
	v_cmp_ne_u32_e32 vcc, 1, v21
	v_mov_b32_e32 v0, s13
	v_add_co_u32_e64 v21, s[0:1], s12, v4
	s_and_b64 vcc, exec, vcc
	v_addc_co_u32_e64 v22, s[0:1], v5, v0, s[0:1]
	s_cbranch_vccz BB0_4
; %bb.3:                                ;   in Loop: Header=BB0_2 Depth=2
	global_load_dwordx4 v[31:34], v[21:22], off
	global_load_dwordx4 v[35:38], v[21:22], off offset:32
	global_load_dwordx4 v[39:42], v[21:22], off offset:64
	global_load_dwordx4 v[43:46], v[21:22], off offset:96
	v_add_co_u32_e32 v25, vcc, 32, v21
	v_addc_co_u32_e32 v26, vcc, 0, v22, vcc
	v_add_co_u32_e32 v27, vcc, 64, v21
	v_addc_co_u32_e32 v28, vcc, 0, v22, vcc
	v_add_co_u32_e32 v29, vcc, s3, v21
	v_addc_co_u32_e32 v30, vcc, 0, v22, vcc
	s_branch BB0_5
BB0_4:                                  ;   in Loop: Header=BB0_2 Depth=2
	v_add_co_u32_e32 v25, vcc, 32, v24
	v_addc_co_u32_e32 v26, vcc, 0, v23, vcc
	v_add_co_u32_e32 v27, vcc, 64, v24
	v_addc_co_u32_e32 v28, vcc, 0, v23, vcc
	v_add_co_u32_e32 v29, vcc, s3, v24
	v_addc_co_u32_e32 v30, vcc, 0, v23, vcc
	v_mov_b32_e32 v31, v1
	v_mov_b32_e32 v32, v1
	v_mov_b32_e32 v33, v1
	v_mov_b32_e32 v34, v1
	v_mov_b32_e32 v35, v1
	v_mov_b32_e32 v36, v1
	v_mov_b32_e32 v37, v1
	v_mov_b32_e32 v38, v1
	v_mov_b32_e32 v39, v1
	v_mov_b32_e32 v40, v1
	v_mov_b32_e32 v41, v1
	v_mov_b32_e32 v42, v1
	v_mov_b32_e32 v43, v1
	v_mov_b32_e32 v44, v1
	v_mov_b32_e32 v45, v1
	v_mov_b32_e32 v46, v1
BB0_5:                                  ;   in Loop: Header=BB0_2 Depth=2
	v_or_b32_e32 v0, s14, v2
	v_mul_lo_i32 v0, v0, s2
	s_waitcnt vmcnt(0)
	v_accvgpr_write_b32 a0, v31
	v_accvgpr_write_b32 a1, v32
	v_accvgpr_write_b32 a2, v33
	v_accvgpr_write_b32 a3, v34
	v_accvgpr_write_b32 a4, v35
	v_accvgpr_write_b32 a5, v36
	v_accvgpr_write_b32 a6, v37
	v_accvgpr_write_b32 a7, v38
	v_accvgpr_write_b32 a8, v39
	v_accvgpr_write_b32 a9, v40
	v_accvgpr_write_b32 a10, v41
	v_accvgpr_write_b32 a11, v42
	v_accvgpr_write_b32 a12, v43
	v_accvgpr_write_b32 a13, v44
	v_accvgpr_write_b32 a14, v45
	v_accvgpr_write_b32 a15, v46
	v_lshlrev_b64 v[31:32], 2, v[0:1]
	v_add_co_u32_e32 v39, vcc, v11, v31
	v_addc_co_u32_e32 v40, vcc, v12, v32, vcc
	global_load_dwordx4 v[31:34], v[39:40], off
	global_load_dwordx4 v[35:38], v[39:40], off offset:32
	s_and_b64 vcc, exec, s[16:17]
	s_waitcnt vmcnt(1)
	v_cvt_f16_f32_e32 v0, v33
	v_cvt_f16_f32_e32 v31, v31
	v_cvt_f16_f32_e32 v33, v34
	v_cvt_f16_f32_e32 v34, v32
	v_and_b32_e32 v0, v10, v0
	v_and_b32_e32 v31, v10, v31
	v_lshl_or_b32 v32, v33, 16, v0
	v_lshl_or_b32 v31, v34, 16, v31
	s_waitcnt vmcnt(0)
	v_cvt_f16_f32_e32 v0, v37
	v_cvt_f16_f32_e32 v33, v36
	s_waitcnt lgkmcnt(1)
	v_mfma_f32_32x32x8f16 a[0:15], v[31:32], v[13:14], a[0:15]
	v_cvt_f16_f32_e32 v31, v35
	v_cvt_f16_f32_e32 v32, v38
	v_and_b32_e32 v0, v10, v0
	v_and_b32_e32 v31, v10, v31
	v_lshl_or_b32 v32, v32, 16, v0
	v_lshl_or_b32 v31, v33, 16, v31
	s_nop 1
	v_mfma_f32_32x32x8f16 a[0:15], v[31:32], v[15:16], a[0:15]
	global_load_dwordx4 v[31:34], v[39:40], off offset:64
	global_load_dwordx4 v[35:38], v[39:40], off offset:96
	s_waitcnt vmcnt(1)
	v_cvt_f16_f32_e32 v0, v33
	v_cvt_f16_f32_e32 v31, v31
	v_cvt_f16_f32_e32 v33, v34
	v_cvt_f16_f32_e32 v34, v32
	v_and_b32_e32 v0, v10, v0
	v_and_b32_e32 v31, v10, v31
	v_lshl_or_b32 v32, v33, 16, v0
	v_lshl_or_b32 v31, v34, 16, v31
	s_waitcnt vmcnt(0)
	v_cvt_f16_f32_e32 v0, v37
	v_cvt_f16_f32_e32 v33, v36
	s_waitcnt lgkmcnt(0)
	v_mfma_f32_32x32x8f16 a[0:15], v[31:32], v[17:18], a[0:15]
	v_cvt_f16_f32_e32 v31, v35
	v_cvt_f16_f32_e32 v32, v38
	v_and_b32_e32 v0, v10, v0
	v_and_b32_e32 v31, v10, v31
	v_lshl_or_b32 v32, v32, 16, v0
	v_lshl_or_b32 v31, v33, 16, v31
	s_nop 1
	v_mfma_f32_32x32x8f16 a[0:15], v[31:32], v[19:20], a[0:15]
	s_nop 7
	s_nop 7
	s_nop 1
	v_accvgpr_read_b32 v34, a3
	v_accvgpr_read_b32 v33, a2
	v_accvgpr_read_b32 v32, a1
	v_accvgpr_read_b32 v31, a0
	v_accvgpr_read_b32 v38, a7
	v_accvgpr_read_b32 v37, a6
	v_accvgpr_read_b32 v36, a5
	v_accvgpr_read_b32 v35, a4
	v_accvgpr_read_b32 v42, a15
	v_accvgpr_read_b32 v41, a14
	v_accvgpr_read_b32 v40, a13
	v_accvgpr_read_b32 v39, a12
	v_accvgpr_read_b32 v46, a11
	v_accvgpr_read_b32 v45, a10
	v_accvgpr_read_b32 v44, a9
	v_accvgpr_read_b32 v43, a8
	global_store_dwordx4 v[21:22], v[31:34], off
	global_store_dwordx4 v[25:26], v[35:38], off
	global_store_dwordx4 v[27:28], v[43:46], off
	global_store_dwordx4 v[29:30], v[39:42], off
	s_cbranch_vccnz BB0_7
; %bb.6:                                ;   in Loop: Header=BB0_2 Depth=2
	global_load_dwordx4 v[31:34], v[21:22], off offset:128
	global_load_dwordx4 v[35:38], v[21:22], off offset:160
	global_load_dwordx4 v[39:42], v[21:22], off offset:192
	global_load_dwordx4 v[43:46], v[21:22], off offset:224
	v_add_co_u32_e32 v25, vcc, s6, v21
	v_addc_co_u32_e32 v26, vcc, 0, v22, vcc
	v_add_co_u32_e32 v27, vcc, s7, v21
	v_addc_co_u32_e32 v28, vcc, 0, v22, vcc
	v_add_co_u32_e32 v29, vcc, s8, v21
	v_addc_co_u32_e32 v30, vcc, 0, v22, vcc
	s_branch BB0_8
BB0_7:                                  ;   in Loop: Header=BB0_2 Depth=2
	v_add_co_u32_e32 v25, vcc, s6, v24
	v_addc_co_u32_e32 v26, vcc, 0, v23, vcc
	v_add_co_u32_e32 v27, vcc, s7, v24
	v_addc_co_u32_e32 v28, vcc, 0, v23, vcc
	v_add_co_u32_e32 v29, vcc, s8, v24
	v_addc_co_u32_e32 v30, vcc, 0, v23, vcc
	v_mov_b32_e32 v31, v1
	v_mov_b32_e32 v32, v1
	v_mov_b32_e32 v33, v1
	v_mov_b32_e32 v34, v1
	v_mov_b32_e32 v35, v1
	v_mov_b32_e32 v36, v1
	v_mov_b32_e32 v37, v1
	v_mov_b32_e32 v38, v1
	v_mov_b32_e32 v39, v1
	v_mov_b32_e32 v40, v1
	v_mov_b32_e32 v41, v1
	v_mov_b32_e32 v42, v1
	v_mov_b32_e32 v43, v1
	v_mov_b32_e32 v44, v1
	v_mov_b32_e32 v45, v1
	v_mov_b32_e32 v46, v1
BB0_8:                                  ;   in Loop: Header=BB0_2 Depth=2
	s_add_i32 s0, s14, 32
	v_or_b32_e32 v0, s0, v2
	v_mul_lo_i32 v23, v0, s2
	v_mov_b32_e32 v24, v1
	s_waitcnt vmcnt(0)
	v_accvgpr_write_b32 a0, v31
	v_accvgpr_write_b32 a1, v32
	v_lshlrev_b64 v[23:24], 2, v[23:24]
	v_accvgpr_write_b32 a2, v33
	v_add_co_u32_e32 v23, vcc, v11, v23
	v_addc_co_u32_e32 v24, vcc, v12, v24, vcc
	v_accvgpr_write_b32 a3, v34
	v_accvgpr_write_b32 a4, v35
	v_accvgpr_write_b32 a5, v36
	v_accvgpr_write_b32 a6, v37
	v_accvgpr_write_b32 a7, v38
	v_accvgpr_write_b32 a8, v39
	v_accvgpr_write_b32 a9, v40
	v_accvgpr_write_b32 a10, v41
	v_accvgpr_write_b32 a11, v42
	v_accvgpr_write_b32 a12, v43
	v_accvgpr_write_b32 a13, v44
	v_accvgpr_write_b32 a14, v45
	v_accvgpr_write_b32 a15, v46
	global_load_dwordx4 v[31:34], v[23:24], off
	global_load_dwordx4 v[35:38], v[23:24], off offset:32
	s_add_i32 s15, s15, 2
	s_add_u32 s12, s12, 0x100
	s_addc_u32 s13, s13, 0
	s_add_i32 s14, s14, 64
	s_cmpk_eq_i32 s12, 0x1000
	s_waitcnt vmcnt(1)
	v_cvt_f16_f32_e32 v0, v33
	v_cvt_f16_f32_e32 v31, v31
	v_cvt_f16_f32_e32 v33, v34
	v_cvt_f16_f32_e32 v34, v32
	v_and_b32_e32 v0, v10, v0
	v_and_b32_e32 v31, v10, v31
	v_lshl_or_b32 v32, v33, 16, v0
	v_lshl_or_b32 v31, v34, 16, v31
	s_waitcnt vmcnt(0)
	v_cvt_f16_f32_e32 v0, v37
	v_cvt_f16_f32_e32 v33, v36
	v_mfma_f32_32x32x8f16 a[0:15], v[31:32], v[13:14], a[0:15]
	v_cvt_f16_f32_e32 v31, v35
	v_cvt_f16_f32_e32 v32, v38
	v_and_b32_e32 v0, v10, v0
	v_and_b32_e32 v31, v10, v31
	v_lshl_or_b32 v32, v32, 16, v0
	v_lshl_or_b32 v31, v33, 16, v31
	s_nop 1
	v_mfma_f32_32x32x8f16 a[0:15], v[31:32], v[15:16], a[0:15]
	global_load_dwordx4 v[31:34], v[23:24], off offset:64
	global_load_dwordx4 v[35:38], v[23:24], off offset:96
	s_waitcnt vmcnt(1)
	v_cvt_f16_f32_e32 v23, v31
	v_cvt_f16_f32_e32 v0, v33
	v_cvt_f16_f32_e32 v24, v34
	v_cvt_f16_f32_e32 v31, v32
	v_and_b32_e32 v23, v10, v23
	v_and_b32_e32 v0, v10, v0
	v_lshl_or_b32 v24, v24, 16, v0
	v_lshl_or_b32 v23, v31, 16, v23
	s_waitcnt vmcnt(0)
	v_cvt_f16_f32_e32 v0, v37
	v_cvt_f16_f32_e32 v31, v36
	v_mfma_f32_32x32x8f16 a[0:15], v[23:24], v[17:18], a[0:15]
	v_cvt_f16_f32_e32 v23, v35
	v_cvt_f16_f32_e32 v24, v38
	v_and_b32_e32 v0, v10, v0
	v_and_b32_e32 v23, v10, v23
	v_lshl_or_b32 v24, v24, 16, v0
	v_lshl_or_b32 v23, v31, 16, v23
	s_nop 1
	v_mfma_f32_32x32x8f16 a[0:15], v[23:24], v[19:20], a[0:15]
	s_nop 7
	s_nop 7
	s_nop 1
	v_accvgpr_read_b32 v34, a3
	v_accvgpr_read_b32 v33, a2
	v_accvgpr_read_b32 v32, a1
	v_accvgpr_read_b32 v31, a0
	v_accvgpr_read_b32 v38, a15
	v_accvgpr_read_b32 v37, a14
	v_accvgpr_read_b32 v36, a13
	v_accvgpr_read_b32 v35, a12
	v_accvgpr_read_b32 v42, a7
	v_accvgpr_read_b32 v41, a6
	v_accvgpr_read_b32 v40, a5
	v_accvgpr_read_b32 v39, a4
	v_accvgpr_read_b32 v46, a11
	v_accvgpr_read_b32 v45, a10
	v_accvgpr_read_b32 v44, a9
	v_accvgpr_read_b32 v43, a8
	global_store_dwordx4 v[21:22], v[31:34], off offset:128
	global_store_dwordx4 v[25:26], v[39:42], off
	global_store_dwordx4 v[27:28], v[43:46], off
	global_store_dwordx4 v[29:30], v[35:38], off
	s_cbranch_scc0 BB0_2
; %bb.9:                                ;   in Loop: Header=BB0_1 Depth=1
	s_add_i32 s9, s9, 1
	s_cmp_eq_u32 s9, 32
	s_cbranch_scc0 BB0_1
; %bb.10:
	s_endpgm
.Lfunc_end0:
	.size	mixgemm1K, .Lfunc_end0-mixgemm1K
                                        ; -- End function
	.section	.AMDGPU.csdata
; Kernel info:
; codeLenInByte = 2144
; NumSgprs: 20
; NumVgprs: 47
; ScratchSize: 0
; MemoryBound: 0
; FloatMode: 192
; IeeeMode: 1
; LDSByteSize: 8192 bytes/workgroup (compile time only)
; SGPRBlocks: 2
; VGPRBlocks: 11
; NumSGPRsForWavesPerEU: 20
; NumVGPRsForWavesPerEU: 47
; WaveLimiterHint : 1
; COMPUTE_PGM_RSRC2:USER_SGPR: 6
; COMPUTE_PGM_RSRC2:TRAP_HANDLER: 0
; COMPUTE_PGM_RSRC2:TGID_X_EN: 1
; COMPUTE_PGM_RSRC2:TGID_Y_EN: 0
; COMPUTE_PGM_RSRC2:TGID_Z_EN: 0
; COMPUTE_PGM_RSRC2:TIDIG_COMP_CNT: 0
	.hidden	redret                  ; @redret
	.type	redret,@object
	.comm	redret,8,2

	.ident	"clang version 4.0 "
	.section	".note.GNU-stack"
	.amd_amdgpu_isa "amdgcn-amd-amdhsa--gfx908"
	.amd_amdgpu_hsa_metadata
---
Version:         [ 1, 0 ]
Kernels:         
  - Name:            mixgemm1K
    SymbolName:      'mixgemm1K@kd'
    Language:        OpenCL C
    LanguageVersion: [ 2, 0 ]
    Attrs:           
      ReqdWorkGroupSize: [ 256, 1, 1 ]
    Args:            
      - TypeName:        'float*'
        Size:            8
        Align:           8
        ValueKind:       GlobalBuffer
        ValueType:       F32
        AddrSpaceQual:   Global
        AccQual:         ReadOnly
        IsConst:         true
        IsRestrict:      true
      - TypeName:        uint
        Size:            4
        Align:           4
        ValueKind:       ByValue
        ValueType:       U32
        AccQual:         Default
      - TypeName:        'float*'
        Size:            8
        Align:           8
        ValueKind:       GlobalBuffer
        ValueType:       F32
        AddrSpaceQual:   Global
        AccQual:         ReadOnly
        IsConst:         true
        IsRestrict:      true
      - TypeName:        uint
        Size:            4
        Align:           4
        ValueKind:       ByValue
        ValueType:       U32
        AccQual:         Default
      - TypeName:        'float*'
        Size:            8
        Align:           8
        ValueKind:       GlobalBuffer
        ValueType:       F32
        AddrSpaceQual:   Global
        AccQual:         Default
        IsRestrict:      true
      - TypeName:        uint
        Size:            4
        Align:           4
        ValueKind:       ByValue
        ValueType:       U32
        AccQual:         Default
      - Size:            8
        Align:           8
        ValueKind:       HiddenGlobalOffsetX
        ValueType:       I64
      - Size:            8
        Align:           8
        ValueKind:       HiddenGlobalOffsetY
        ValueType:       I64
      - Size:            8
        Align:           8
        ValueKind:       HiddenGlobalOffsetZ
        ValueType:       I64
      - Size:            8
        Align:           8
        ValueKind:       HiddenNone
        ValueType:       I8
        AddrSpaceQual:   Global
      - Size:            8
        Align:           8
        ValueKind:       HiddenNone
        ValueType:       I8
        AddrSpaceQual:   Global
      - Size:            8
        Align:           8
        ValueKind:       HiddenNone
        ValueType:       I8
        AddrSpaceQual:   Global
    CodeProps:       
      KernargSegmentSize: 96
      GroupSegmentFixedSize: 8192
      PrivateSegmentFixedSize: 0
      KernargSegmentAlign: 8
      WavefrontSize:   64
      NumSGPRs:        20
      NumVGPRs:        47
      MaxFlatWorkGroupSize: 256
...

	.end_amd_amdgpu_hsa_metadata
