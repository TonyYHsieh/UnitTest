	.hsa_code_object_version 2,1

	.hsa_code_object_isa 8,0,3,"AMD","AMDGPU"

	.text
	.amdgpu_hsa_kernel VectorAdd

VectorAdd:
	.amd_kernel_code_t
		amd_code_version_major = 1
		amd_code_version_minor = 2
		amd_machine_kind = 1
		amd_machine_version_major = 8
		amd_machine_version_minor = 0
		amd_machine_version_stepping = 3
		kernel_code_entry_byte_offset = 256
		kernel_code_prefetch_byte_size = 0
		granulated_workitem_vgpr_count = 1
		granulated_wavefront_sgpr_count = 1
		priority = 0
		float_mode = 192
		priv = 0
		enable_dx10_clamp = 1
		debug_mode = 0
		enable_ieee_mode = 1
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
		enable_ordered_append_gds = 0
		private_element_size = 1
		is_ptr64 = 1
		is_dynamic_callstack = 0
		is_debug_enabled = 0
		is_xnack_enabled = 0
		workitem_private_segment_byte_size = 0
		workgroup_group_segment_byte_size = 0
		gds_segment_byte_size = 0
		kernarg_segment_byte_size = 80
		workgroup_fbarrier_count = 0
		wavefront_sgpr_count = 11
		workitem_vgpr_count = 6
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
        // s2: workgroup size x and y
	s_load_dword s2, s[4:5], 0x4                               // 000000001100: C0020082 00000004
        // s3: real global size
	s_load_dword s3, s[6:7], 0x10                              // 000000001108: C00200C3 00000010

	s_waitcnt lgkmcnt(0)                                       // 000000001118: BF8C007F

        // get globa id in v1
	s_and_b32 s1, s2, 0xffff                                   // 00000000111C: 8601FF02 0000FFFF
	s_mul_i32 s8, s8, s1                                       // 000000001124: 92080108
	v_add_u32_e32 v1, vcc, s8, v0                              // 000000001128: 32000008

        // vcc = s3 > v1
	v_cmp_gt_i32_e32 vcc, s3, v1                               // 000000001130: 7D880203
	s_and_saveexec_b64 s[0:1], vcc                             // 000000001134: BE80206A
	s_cbranch_execz BB0_2                                      // 000000001138: BF88001B
BB0_1:
	s_load_dwordx2 s[0:1], s[6:7], 0x0                         // 00000000113C: C0060003 00000000
	v_mov_b32_e32 v0, 0                                        // 000000001144: 7E000280
	s_load_dwordx2 s[2:3], s[6:7], 0x8                         // 000000001148: C0060083 00000008
	v_ashrrev_i64 v[0:1], 30, v[0:1]                           // 000000001150: D2910000 0002009E
	s_load_dwordx2 s[4:5], s[6:7], 0x18                        // 000000001158: C0060103 00000018
	s_waitcnt lgkmcnt(0)                                       // 000000001160: BF8C007F
	v_mov_b32_e32 v3, s1                                       // 000000001164: 7E060201
	v_add_u32_e32 v2, vcc, s0, v0                              // 000000001168: 32040000
	v_addc_u32_e32 v3, vcc, v3, v1, vcc                        // 00000000116C: 38060303
	v_mov_b32_e32 v5, s3                                       // 000000001170: 7E0A0203
	v_add_u32_e32 v4, vcc, s2, v0                              // 000000001174: 32080002
	v_addc_u32_e32 v5, vcc, v5, v1, vcc                        // 000000001178: 380A0305
	flat_load_dword v4, v[4:5]                                 // 00000000117C: DC500000 04000004
	flat_load_dword v2, v[2:3]                                 // 000000001184: DC500000 02000002
	v_mov_b32_e32 v3, s5                                       // 00000000118C: 7E060205
	v_add_u32_e32 v0, vcc, s4, v0                              // 000000001190: 32000004
	v_addc_u32_e32 v1, vcc, v3, v1, vcc                        // 000000001194: 38020303
	s_waitcnt vmcnt(0) lgkmcnt(0)                              // 000000001198: BF8C0070
	v_add_u32_e32 v2, vcc, v2, v4                              // 00000000119C: 32040902
	flat_store_dword v[0:1], v2                                // 0000000011A0: DC700000 00000200
BB0_2:
	s_endpgm                                                   // 0000000011A8: BF810000

.amd_amdgpu_hsa_metadata
{ Version: [1, 0],
  Kernels :
    - { Name: VectorAdd,
        SymbolName: VectorAdd,
        Language: OpenCL C, LanguageVersion: [ 1, 2 ],
        Attrs: { ReqdWorkGroupSize: [ 64, 1, 1 ] }
        CodeProps: { KernargSegmentSize: 80, GroupSegmentFixedSize : 0, PrivateSegmentFixedSize : 0, KernargSegmentAlign : 8, WavefrontSize : 64, MaxFlatWorkGroupSize : 512 }
        Args:
        - { Name: ina,  Size: 8, Align: 8, ValueKind: GlobalBuffer, ValueType: I32, TypeName: 'int*', AddrSpaceQual: Global, IsConst: true  }
        - { Name: inb,  Size: 8, Align: 8, ValueKind: GlobalBuffer, ValueType: I32, TypeName: 'int*', AddrSpaceQual: Global, IsConst: true  }
        - { Name: size, Size: 4, Align: 8, ValueKind: ByValue,      ValueType: I32, TypeName: 'int',  AddrSpaceQual: Global, IsConst: true }
        - { Name: out,  Size: 8, Align: 8, ValueKind: GlobalBuffer, ValueType: I32, TypeName: 'int*', AddrSpaceQual: Global  }
      }
}
.end_amd_amdgpu_hsa_metadata
