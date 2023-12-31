; NOTE: Assertions have been autogenerated by utils/update_llc_test_checks.py
; RUN: llc -global-isel -mtriple=amdgcn-- -verify-machineinstrs -mem-intrinsic-expand-size=19 %s -o - | FileCheck -check-prefix=LOOP %s
; RUN: llc -global-isel -mtriple=amdgcn-- -verify-machineinstrs -mem-intrinsic-expand-size=21 %s -o - | FileCheck -check-prefix=UNROLL %s

declare void @llvm.memcpy.p1.p1.i32(ptr addrspace(1), ptr addrspace(1), i32, i1 immarg)

define amdgpu_cs void @memcpy_p1i8(ptr addrspace(1) %dst, ptr addrspace(1) %src) {
; LOOP-LABEL: memcpy_p1i8:
; LOOP:       ; %bb.0:
; LOOP-NEXT:    s_mov_b32 s6, 0
; LOOP-NEXT:    s_mov_b32 s7, 0xf000
; LOOP-NEXT:    s_mov_b64 s[4:5], 0
; LOOP-NEXT:    v_mov_b32_e32 v5, v3
; LOOP-NEXT:    v_mov_b32_e32 v4, v2
; LOOP-NEXT:    v_mov_b32_e32 v7, v1
; LOOP-NEXT:    v_mov_b32_e32 v6, v0
; LOOP-NEXT:    v_mov_b32_e32 v8, s6
; LOOP-NEXT:  .LBB0_1: ; %load-store-loop
; LOOP-NEXT:    ; =>This Inner Loop Header: Depth=1
; LOOP-NEXT:    buffer_load_ubyte v9, v[4:5], s[4:7], 0 addr64
; LOOP-NEXT:    s_waitcnt expcnt(6)
; LOOP-NEXT:    buffer_load_ubyte v10, v[4:5], s[4:7], 0 addr64 offset:1
; LOOP-NEXT:    s_waitcnt expcnt(3)
; LOOP-NEXT:    buffer_load_ubyte v11, v[4:5], s[4:7], 0 addr64 offset:2
; LOOP-NEXT:    s_waitcnt expcnt(0)
; LOOP-NEXT:    buffer_load_ubyte v12, v[4:5], s[4:7], 0 addr64 offset:3
; LOOP-NEXT:    buffer_load_ubyte v13, v[4:5], s[4:7], 0 addr64 offset:4
; LOOP-NEXT:    buffer_load_ubyte v14, v[4:5], s[4:7], 0 addr64 offset:5
; LOOP-NEXT:    buffer_load_ubyte v15, v[4:5], s[4:7], 0 addr64 offset:6
; LOOP-NEXT:    buffer_load_ubyte v16, v[4:5], s[4:7], 0 addr64 offset:7
; LOOP-NEXT:    buffer_load_ubyte v17, v[4:5], s[4:7], 0 addr64 offset:8
; LOOP-NEXT:    buffer_load_ubyte v18, v[4:5], s[4:7], 0 addr64 offset:9
; LOOP-NEXT:    buffer_load_ubyte v19, v[4:5], s[4:7], 0 addr64 offset:10
; LOOP-NEXT:    buffer_load_ubyte v20, v[4:5], s[4:7], 0 addr64 offset:11
; LOOP-NEXT:    buffer_load_ubyte v21, v[4:5], s[4:7], 0 addr64 offset:12
; LOOP-NEXT:    buffer_load_ubyte v22, v[4:5], s[4:7], 0 addr64 offset:13
; LOOP-NEXT:    buffer_load_ubyte v23, v[4:5], s[4:7], 0 addr64 offset:14
; LOOP-NEXT:    buffer_load_ubyte v24, v[4:5], s[4:7], 0 addr64 offset:15
; LOOP-NEXT:    v_add_i32_e32 v8, vcc, 1, v8
; LOOP-NEXT:    s_xor_b64 s[0:1], vcc, -1
; LOOP-NEXT:    s_xor_b64 s[0:1], s[0:1], -1
; LOOP-NEXT:    s_and_b64 vcc, s[0:1], exec
; LOOP-NEXT:    s_waitcnt vmcnt(14)
; LOOP-NEXT:    v_lshlrev_b32_e32 v10, 8, v10
; LOOP-NEXT:    s_waitcnt vmcnt(12)
; LOOP-NEXT:    v_lshlrev_b32_e32 v12, 24, v12
; LOOP-NEXT:    v_lshlrev_b32_e32 v11, 16, v11
; LOOP-NEXT:    s_waitcnt vmcnt(10)
; LOOP-NEXT:    v_lshlrev_b32_e32 v14, 8, v14
; LOOP-NEXT:    s_waitcnt vmcnt(8)
; LOOP-NEXT:    v_lshlrev_b32_e32 v16, 24, v16
; LOOP-NEXT:    v_lshlrev_b32_e32 v15, 16, v15
; LOOP-NEXT:    s_waitcnt vmcnt(6)
; LOOP-NEXT:    v_lshlrev_b32_e32 v18, 8, v18
; LOOP-NEXT:    s_waitcnt vmcnt(4)
; LOOP-NEXT:    v_lshlrev_b32_e32 v20, 24, v20
; LOOP-NEXT:    v_lshlrev_b32_e32 v19, 16, v19
; LOOP-NEXT:    s_waitcnt vmcnt(2)
; LOOP-NEXT:    v_lshlrev_b32_e32 v22, 8, v22
; LOOP-NEXT:    s_waitcnt vmcnt(0)
; LOOP-NEXT:    v_lshlrev_b32_e32 v24, 24, v24
; LOOP-NEXT:    v_lshlrev_b32_e32 v23, 16, v23
; LOOP-NEXT:    v_or_b32_e32 v9, v10, v9
; LOOP-NEXT:    v_or_b32_e32 v10, v12, v11
; LOOP-NEXT:    v_or_b32_e32 v11, v14, v13
; LOOP-NEXT:    v_or_b32_e32 v12, v16, v15
; LOOP-NEXT:    v_or_b32_e32 v13, v18, v17
; LOOP-NEXT:    v_or_b32_e32 v14, v20, v19
; LOOP-NEXT:    v_or_b32_e32 v15, v22, v21
; LOOP-NEXT:    v_or_b32_e32 v16, v24, v23
; LOOP-NEXT:    v_or_b32_e32 v9, v10, v9
; LOOP-NEXT:    v_or_b32_e32 v10, v12, v11
; LOOP-NEXT:    v_or_b32_e32 v11, v14, v13
; LOOP-NEXT:    v_or_b32_e32 v12, v16, v15
; LOOP-NEXT:    v_lshrrev_b32_e32 v13, 16, v9
; LOOP-NEXT:    v_bfe_u32 v14, v9, 8, 8
; LOOP-NEXT:    buffer_store_byte v9, v[6:7], s[4:7], 0 addr64
; LOOP-NEXT:    s_waitcnt expcnt(0)
; LOOP-NEXT:    v_lshrrev_b32_e32 v9, 24, v9
; LOOP-NEXT:    v_lshrrev_b32_e32 v15, 16, v10
; LOOP-NEXT:    v_bfe_u32 v16, v10, 8, 8
; LOOP-NEXT:    buffer_store_byte v10, v[6:7], s[4:7], 0 addr64 offset:4
; LOOP-NEXT:    s_waitcnt expcnt(0)
; LOOP-NEXT:    v_lshrrev_b32_e32 v10, 24, v10
; LOOP-NEXT:    v_lshrrev_b32_e32 v17, 16, v11
; LOOP-NEXT:    v_bfe_u32 v18, v11, 8, 8
; LOOP-NEXT:    buffer_store_byte v11, v[6:7], s[4:7], 0 addr64 offset:8
; LOOP-NEXT:    s_waitcnt expcnt(0)
; LOOP-NEXT:    v_lshrrev_b32_e32 v11, 24, v11
; LOOP-NEXT:    v_lshrrev_b32_e32 v19, 16, v12
; LOOP-NEXT:    v_bfe_u32 v20, v12, 8, 8
; LOOP-NEXT:    buffer_store_byte v12, v[6:7], s[4:7], 0 addr64 offset:12
; LOOP-NEXT:    s_waitcnt expcnt(0)
; LOOP-NEXT:    v_lshrrev_b32_e32 v12, 24, v12
; LOOP-NEXT:    buffer_store_byte v14, v[6:7], s[4:7], 0 addr64 offset:1
; LOOP-NEXT:    buffer_store_byte v13, v[6:7], s[4:7], 0 addr64 offset:2
; LOOP-NEXT:    buffer_store_byte v9, v[6:7], s[4:7], 0 addr64 offset:3
; LOOP-NEXT:    buffer_store_byte v16, v[6:7], s[4:7], 0 addr64 offset:5
; LOOP-NEXT:    buffer_store_byte v15, v[6:7], s[4:7], 0 addr64 offset:6
; LOOP-NEXT:    buffer_store_byte v10, v[6:7], s[4:7], 0 addr64 offset:7
; LOOP-NEXT:    buffer_store_byte v18, v[6:7], s[4:7], 0 addr64 offset:9
; LOOP-NEXT:    buffer_store_byte v17, v[6:7], s[4:7], 0 addr64 offset:10
; LOOP-NEXT:    buffer_store_byte v11, v[6:7], s[4:7], 0 addr64 offset:11
; LOOP-NEXT:    buffer_store_byte v20, v[6:7], s[4:7], 0 addr64 offset:13
; LOOP-NEXT:    buffer_store_byte v19, v[6:7], s[4:7], 0 addr64 offset:14
; LOOP-NEXT:    buffer_store_byte v12, v[6:7], s[4:7], 0 addr64 offset:15
; LOOP-NEXT:    v_add_i32_e64 v6, s[0:1], 16, v6
; LOOP-NEXT:    v_addc_u32_e64 v7, s[0:1], 0, v7, s[0:1]
; LOOP-NEXT:    v_add_i32_e64 v4, s[0:1], 16, v4
; LOOP-NEXT:    v_addc_u32_e64 v5, s[0:1], 0, v5, s[0:1]
; LOOP-NEXT:    s_cbranch_vccnz .LBB0_1
; LOOP-NEXT:  ; %bb.2: ; %memcpy-split
; LOOP-NEXT:    s_mov_b32 s2, 0
; LOOP-NEXT:    s_mov_b32 s3, 0xf000
; LOOP-NEXT:    s_mov_b64 s[0:1], 0
; LOOP-NEXT:    buffer_load_ubyte v4, v[2:3], s[0:3], 0 addr64 offset:17
; LOOP-NEXT:    buffer_load_ubyte v5, v[2:3], s[0:3], 0 addr64 offset:19
; LOOP-NEXT:    buffer_load_ubyte v6, v[2:3], s[0:3], 0 addr64 offset:18
; LOOP-NEXT:    buffer_load_ubyte v2, v[2:3], s[0:3], 0 addr64 offset:16
; LOOP-NEXT:    s_waitcnt vmcnt(3)
; LOOP-NEXT:    v_lshlrev_b32_e32 v3, 8, v4
; LOOP-NEXT:    s_waitcnt vmcnt(2)
; LOOP-NEXT:    v_lshlrev_b32_e32 v4, 24, v5
; LOOP-NEXT:    s_waitcnt vmcnt(1)
; LOOP-NEXT:    v_lshlrev_b32_e32 v5, 16, v6
; LOOP-NEXT:    s_waitcnt vmcnt(0)
; LOOP-NEXT:    v_or_b32_e32 v2, v3, v2
; LOOP-NEXT:    v_or_b32_e32 v3, v4, v5
; LOOP-NEXT:    v_or_b32_e32 v2, v3, v2
; LOOP-NEXT:    v_lshrrev_b32_e32 v3, 16, v2
; LOOP-NEXT:    v_bfe_u32 v4, v2, 8, 8
; LOOP-NEXT:    buffer_store_byte v2, v[0:1], s[0:3], 0 addr64 offset:16
; LOOP-NEXT:    s_waitcnt expcnt(0)
; LOOP-NEXT:    v_lshrrev_b32_e32 v2, 24, v2
; LOOP-NEXT:    buffer_store_byte v4, v[0:1], s[0:3], 0 addr64 offset:17
; LOOP-NEXT:    buffer_store_byte v3, v[0:1], s[0:3], 0 addr64 offset:18
; LOOP-NEXT:    buffer_store_byte v2, v[0:1], s[0:3], 0 addr64 offset:19
; LOOP-NEXT:    s_endpgm
;
; UNROLL-LABEL: memcpy_p1i8:
; UNROLL:       ; %bb.0:
; UNROLL-NEXT:    s_mov_b32 s2, 0
; UNROLL-NEXT:    s_mov_b32 s3, 0xf000
; UNROLL-NEXT:    s_mov_b64 s[0:1], 0
; UNROLL-NEXT:    buffer_load_ubyte v4, v[2:3], s[0:3], 0 addr64
; UNROLL-NEXT:    s_waitcnt vmcnt(0)
; UNROLL-NEXT:    buffer_store_byte v4, v[0:1], s[0:3], 0 addr64
; UNROLL-NEXT:    s_waitcnt expcnt(0)
; UNROLL-NEXT:    buffer_load_ubyte v4, v[2:3], s[0:3], 0 addr64 offset:1
; UNROLL-NEXT:    s_waitcnt vmcnt(0)
; UNROLL-NEXT:    buffer_store_byte v4, v[0:1], s[0:3], 0 addr64 offset:1
; UNROLL-NEXT:    s_waitcnt expcnt(0)
; UNROLL-NEXT:    buffer_load_ubyte v4, v[2:3], s[0:3], 0 addr64 offset:2
; UNROLL-NEXT:    s_waitcnt vmcnt(0)
; UNROLL-NEXT:    buffer_store_byte v4, v[0:1], s[0:3], 0 addr64 offset:2
; UNROLL-NEXT:    s_waitcnt expcnt(0)
; UNROLL-NEXT:    buffer_load_ubyte v4, v[2:3], s[0:3], 0 addr64 offset:3
; UNROLL-NEXT:    s_waitcnt vmcnt(0)
; UNROLL-NEXT:    buffer_store_byte v4, v[0:1], s[0:3], 0 addr64 offset:3
; UNROLL-NEXT:    s_waitcnt expcnt(0)
; UNROLL-NEXT:    buffer_load_ubyte v4, v[2:3], s[0:3], 0 addr64 offset:4
; UNROLL-NEXT:    s_waitcnt vmcnt(0)
; UNROLL-NEXT:    buffer_store_byte v4, v[0:1], s[0:3], 0 addr64 offset:4
; UNROLL-NEXT:    s_waitcnt expcnt(0)
; UNROLL-NEXT:    buffer_load_ubyte v4, v[2:3], s[0:3], 0 addr64 offset:5
; UNROLL-NEXT:    s_waitcnt vmcnt(0)
; UNROLL-NEXT:    buffer_store_byte v4, v[0:1], s[0:3], 0 addr64 offset:5
; UNROLL-NEXT:    s_waitcnt expcnt(0)
; UNROLL-NEXT:    buffer_load_ubyte v4, v[2:3], s[0:3], 0 addr64 offset:6
; UNROLL-NEXT:    s_waitcnt vmcnt(0)
; UNROLL-NEXT:    buffer_store_byte v4, v[0:1], s[0:3], 0 addr64 offset:6
; UNROLL-NEXT:    s_waitcnt expcnt(0)
; UNROLL-NEXT:    buffer_load_ubyte v4, v[2:3], s[0:3], 0 addr64 offset:7
; UNROLL-NEXT:    s_waitcnt vmcnt(0)
; UNROLL-NEXT:    buffer_store_byte v4, v[0:1], s[0:3], 0 addr64 offset:7
; UNROLL-NEXT:    s_waitcnt expcnt(0)
; UNROLL-NEXT:    buffer_load_ubyte v4, v[2:3], s[0:3], 0 addr64 offset:8
; UNROLL-NEXT:    s_waitcnt vmcnt(0)
; UNROLL-NEXT:    buffer_store_byte v4, v[0:1], s[0:3], 0 addr64 offset:8
; UNROLL-NEXT:    s_waitcnt expcnt(0)
; UNROLL-NEXT:    buffer_load_ubyte v4, v[2:3], s[0:3], 0 addr64 offset:9
; UNROLL-NEXT:    s_waitcnt vmcnt(0)
; UNROLL-NEXT:    buffer_store_byte v4, v[0:1], s[0:3], 0 addr64 offset:9
; UNROLL-NEXT:    s_waitcnt expcnt(0)
; UNROLL-NEXT:    buffer_load_ubyte v4, v[2:3], s[0:3], 0 addr64 offset:10
; UNROLL-NEXT:    s_waitcnt vmcnt(0)
; UNROLL-NEXT:    buffer_store_byte v4, v[0:1], s[0:3], 0 addr64 offset:10
; UNROLL-NEXT:    s_waitcnt expcnt(0)
; UNROLL-NEXT:    buffer_load_ubyte v4, v[2:3], s[0:3], 0 addr64 offset:11
; UNROLL-NEXT:    s_waitcnt vmcnt(0)
; UNROLL-NEXT:    buffer_store_byte v4, v[0:1], s[0:3], 0 addr64 offset:11
; UNROLL-NEXT:    s_waitcnt expcnt(0)
; UNROLL-NEXT:    buffer_load_ubyte v4, v[2:3], s[0:3], 0 addr64 offset:12
; UNROLL-NEXT:    s_waitcnt vmcnt(0)
; UNROLL-NEXT:    buffer_store_byte v4, v[0:1], s[0:3], 0 addr64 offset:12
; UNROLL-NEXT:    s_waitcnt expcnt(0)
; UNROLL-NEXT:    buffer_load_ubyte v4, v[2:3], s[0:3], 0 addr64 offset:13
; UNROLL-NEXT:    s_waitcnt vmcnt(0)
; UNROLL-NEXT:    buffer_store_byte v4, v[0:1], s[0:3], 0 addr64 offset:13
; UNROLL-NEXT:    s_waitcnt expcnt(0)
; UNROLL-NEXT:    buffer_load_ubyte v4, v[2:3], s[0:3], 0 addr64 offset:14
; UNROLL-NEXT:    s_waitcnt vmcnt(0)
; UNROLL-NEXT:    buffer_store_byte v4, v[0:1], s[0:3], 0 addr64 offset:14
; UNROLL-NEXT:    s_waitcnt expcnt(0)
; UNROLL-NEXT:    buffer_load_ubyte v4, v[2:3], s[0:3], 0 addr64 offset:15
; UNROLL-NEXT:    s_waitcnt vmcnt(0)
; UNROLL-NEXT:    buffer_store_byte v4, v[0:1], s[0:3], 0 addr64 offset:15
; UNROLL-NEXT:    s_waitcnt expcnt(0)
; UNROLL-NEXT:    buffer_load_ubyte v4, v[2:3], s[0:3], 0 addr64 offset:16
; UNROLL-NEXT:    s_waitcnt vmcnt(0)
; UNROLL-NEXT:    buffer_store_byte v4, v[0:1], s[0:3], 0 addr64 offset:16
; UNROLL-NEXT:    s_waitcnt expcnt(0)
; UNROLL-NEXT:    buffer_load_ubyte v4, v[2:3], s[0:3], 0 addr64 offset:17
; UNROLL-NEXT:    s_waitcnt vmcnt(0)
; UNROLL-NEXT:    buffer_store_byte v4, v[0:1], s[0:3], 0 addr64 offset:17
; UNROLL-NEXT:    s_waitcnt expcnt(0)
; UNROLL-NEXT:    buffer_load_ubyte v4, v[2:3], s[0:3], 0 addr64 offset:18
; UNROLL-NEXT:    s_waitcnt vmcnt(0)
; UNROLL-NEXT:    buffer_store_byte v4, v[0:1], s[0:3], 0 addr64 offset:18
; UNROLL-NEXT:    buffer_load_ubyte v2, v[2:3], s[0:3], 0 addr64 offset:19
; UNROLL-NEXT:    s_waitcnt vmcnt(0)
; UNROLL-NEXT:    buffer_store_byte v2, v[0:1], s[0:3], 0 addr64 offset:19
; UNROLL-NEXT:    s_endpgm
  call void @llvm.memcpy.p1.p1.i32(ptr addrspace(1) %dst, ptr addrspace(1) %src, i32 20, i1 false)
  ret void
}

