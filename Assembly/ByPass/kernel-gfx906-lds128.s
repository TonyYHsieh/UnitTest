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
		workgroup_group_segment_byte_size = 512
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
        // init v0 to 0
	v_mov_b32_e32 v1, 0                                        // 000000001100: 7E020280

        // set v1, v2 with lds address lid and lid + 64
	v_add_co_u32_e32 v1, vcc, 0, v1                            // 000000001104: 32020280
	v_addc_co_u32_e32 v1, vcc, 64, v0, vcc                     // 000000001108: 380200C0
	v_lshlrev_b32_e32 v2, 2, v0                                // 00000000110C: 24040082
	v_lshlrev_b32_e32 v1, 2, v1                                // 000000001110: 24020282

        // assign value to lds
	ds_write_b32 v2, v0                                        // 000000001114: D81A0000 00000002
	ds_write_b32 v1, v0                                        // 00000000111C: D81A0000 00000001

        // wait count of lds, gds, constant read
	s_waitcnt lgkmcnt(0)                                       // 000000001124: BF8CC07F
        // sync waves within a work-group
	s_barrier                                                  // 000000001128: BF8A0000

        // hsa_kernel_dispatch_packet_s::workgroup_size_x
	s_load_dword s2, s[4:5], 0x4                               // 00000000112C: C0020082 00000004
        // get output address (ptr64)
	s_load_dwordx2 s[0:1], s[6:7], 0x8                         // 000000001134: C0060003 00000008
	s_waitcnt lgkmcnt(0)                                       // 000000001144: BF8CC07F

        // read lds data to register
	ds_read_b32 v2, v2                                         // 000000001148: D86C0000 02000002

        // get local size x
	s_and_b32 s2, s2, 0xffff                                   // 000000001150: 8602FF02 0000FFFF

        // read lds data
	ds_read_b32 v3, v1                                         // 000000001158: D86C0000 03000001

        // s8 is dispatch id, get global id 
	s_mul_i32 s8, s8, s2                                       // 000000001160: 92080208
	v_add_u32_e32 v1, s8, v0                                   // 000000001168: 68020003

        // v01 is global_id * 4
	v_mov_b32_e32 v0, 0                                        // 00000000116C: 7E000280
	v_ashrrev_i64 v[0:1], 30, v[0:1]                           // 000000001170: D2910000 0002009E

        // wait for lds read
	s_waitcnt lgkmcnt(0)                                       // 000000001178: BF8CC07F

        // v2 = v2 + v3; output value
	v_add_u32_e32 v2, v3, v2                                   // 00000000117C: 68040503

        // get output global address
	v_mov_b32_e32 v3, s1                                       // 000000001180: 7E060201
	v_add_co_u32_e32 v0, vcc, s0, v0                           // 000000001184: 32000000
	v_addc_co_u32_e32 v1, vcc, v3, v1, vcc                     // 00000000118C: 38020303

        // write result to global address
	global_store_dword v[0:1], v2, off                         // 000000001194: DC708000 007F0200
	s_endpgm                                                   // 00000000119C: BF810000

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

