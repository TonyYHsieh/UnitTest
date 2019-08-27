	.hsa_code_object_version 2,1

	.hsa_code_object_isa 9,0,6,"AMD","AMDGPU"

	.text
	.amdgpu_hsa_kernel ByPass

ByPass:
	.amd_kernel_code_t
		amd_code_version_major = 1
		amd_code_version_minor = 2
		amd_machine_kind = 1
		amd_machine_version_major = 9
		amd_machine_version_minor = 0
		amd_machine_version_stepping = 6
		kernel_code_entry_byte_offset = 256
		kernel_code_prefetch_byte_size = 0
		granulated_workitem_vgpr_count = 0
		granulated_wavefront_sgpr_count = 4
		priority = 0
		float_mode = 240
		priv = 0
		enable_dx10_clamp = 1
		debug_mode = 0
		enable_ieee_mode = 1
		enable_wgp_mode = 0
		enable_mem_ordered = 0
		enable_fwd_progress = 0
		enable_sgpr_private_segment_wave_byte_offset = 0
		user_sgpr_count = 8
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
		enable_sgpr_dispatch_ptr = 1
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
		workgroup_group_segment_byte_size = 0
		gds_segment_byte_size = 0
		kernarg_segment_byte_size = 72
		workgroup_fbarrier_count = 0
		wavefront_sgpr_count = 35
		workitem_vgpr_count = 4
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

// Disassembly:
	s_load_dword s4, s[4:5], 0x4                               // 000000001100: C0020102 00000004
	s_load_dwordx4 s[0:3], s[6:7], 0x0                         // 000000001108: C00A0003 00000000
	v_mov_b32_e32 v1, 0                                        // 000000001118: 7E020280
	s_mov_b32 s10, s9                                          // 00000000111C: BE8A0009
	s_waitcnt lgkmcnt(0)                                       // 000000001120: BF8CC07F
	s_and_b32 s4, s4, 0xffff                                   // 000000001124: 8604FF04 0000FFFF
	s_mul_i32 s8, s8, s4                                       // 00000000112C: 92080408
	v_add_u32_e32 v2, s8, v0                                   // 000000001134: 68040005
	v_ashrrev_i64 v[0:1], 30, v[1:2]                           // 000000001138: D2910000 0002029E
	v_mov_b32_e32 v3, s1                                       // 000000001140: 7E060201
	v_add_co_u32_e32 v2, vcc, s0, v0                           // 000000001144: 32040000
	v_addc_co_u32_e32 v3, vcc, v3, v1, vcc                     // 000000001148: 38060303
	global_load_dword v2, v[2:3], off                          // 00000000114C: DC508000 027F0002
	v_mov_b32_e32 v3, s3                                       // 000000001154: 7E060203
	v_add_co_u32_e32 v0, vcc, s2, v0                           // 000000001158: 32000002
	v_addc_co_u32_e32 v1, vcc, v3, v1, vcc                     // 00000000115C: 38020303
	s_mov_b32 s32, s10                                         // 000000001160: BEA0000A
	s_waitcnt vmcnt(0)                                         // 000000001164: BF8C0F70
	global_store_dword v[0:1], v2, off                         // 000000001168: DC708000 007F0200
	s_endpgm                                                   // 000000001170: BF810000

.amd_amdgpu_hsa_metadata
{ Version: [1, 0],
  Kernels :
    - { Name: ByPass,
        SymbolName: ByPass,
        Language: OpenCL C, LanguageVersion: [ 1, 2 ],
        Attrs: { ReqdWorkGroupSize: [ 64, 1, 1 ] }
        CodeProps: { KernargSegmentSize: 16, GroupSegmentFixedSize : 0, PrivateSegmentFixedSize : 0, KernargSegmentAlign : 8, WavefrontSize : 64, MaxFlatWorkGroupSize : 512 }
        Args:
        - { Name: in , Size: 8, Align: 8, ValueKind: GlobalBuffer, ValueType: I32, TypeName: 'int*', AddrSpaceQual: Global, IsConst: true  }
        - { Name: out, Size: 8, Align: 8, ValueKind: GlobalBuffer, ValueType: I32, TypeName: 'int*', AddrSpaceQual: Global  }
      }
}
.end_amd_amdgpu_hsa_metadata

