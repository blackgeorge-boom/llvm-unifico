; RUN: llc -filetype=obj -relocation-model=pic --verify-machineinstrs -print-after=postrapseudos < %s 2>&1 | FileCheck -check-prefix=CHECK-POST-RA %s
; RUN: llc -filetype=obj -relocation-model=pic --verify-machineinstrs -print-after=arm-pseudo < %s 2>&1 | FileCheck -check-prefix=CHECK-POST-AP %s

; CHECK-POST-RA: $r12 = t2LDRLIT_ga_pcrel target-flags(arm-got) @__stack_chk_guard
; CHECK-POST-AP: $r12 = t2LDRpci %const.0, 14, $noreg

target triple = "thumbv7-unknown-linux-android23"

%"class.v8::internal::Assembler" = type {}
%"class.v8::internal::Operand" = type { %"class.v8::internal::Register", %"class.v8::internal::Register", i32, i32, %"union.v8::internal::Operand::Value" }
%"class.v8::internal::Register" = type {}
%"union.v8::internal::Operand::Value" = type { i32, [20 x i8] }
%"class.v8::internal::wasm::LiftoffAssembler" = type {}

declare void @_ZN2v88internal9Assembler3addENS0_8RegisterES2_RKNS0_7OperandENS0_4SBitENS0_9ConditionE(ptr, [1 x i32], [1 x i32], ptr, i32, i32)

; Function Attrs: ssp
define void @_ZN2v88internal4wasm16LiftoffAssembler13emit_i32_addiENS0_8RegisterES3_i(ptr %0, [1 x i32] %1, [1 x i32] %2, i32 %3) #0 {
  %5 = alloca %"class.v8::internal::Operand", align 8
  %6 = getelementptr %"class.v8::internal::Operand", ptr %5
  %7 = getelementptr %"class.v8::internal::Operand", ptr %5
  %8 = getelementptr %"class.v8::internal::Operand", ptr %5
  %9 = getelementptr %"class.v8::internal::Operand", ptr %5
  %10 = getelementptr %"class.v8::internal::Operand", ptr %5, i32 0, i32 4, i32 0
  store i32 %3, ptr %10, align 4
  call void @_ZN2v88internal9Assembler3addENS0_8RegisterES2_RKNS0_7OperandENS0_4SBitENS0_9ConditionE(ptr %0, [1 x i32] %1, [1 x i32] %2, ptr %5, i32 0, i32 2)
  ret void
}

attributes #0 = { ssp }

!llvm.module.flags = !{!0}
!0 = !{i32 7, !"PIC Level", i32 2}
