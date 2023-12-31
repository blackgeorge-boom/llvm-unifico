; NOTE: Assertions have been autogenerated by utils/update_llc_test_checks.py
; RUN: llc < %s -mtriple=x86_64-unknown-linux-gnu | FileCheck %s

define void @test(ptr %p) nounwind {
; CHECK-LABEL: test:
; CHECK:       # %bb.0: # %bb
; CHECK-NEXT:    movq (%rdi), %rax
; CHECK-NEXT:    andl $-2, %eax
; CHECK-NEXT:    cmpq $2, %rax
; CHECK-NEXT:    cmpl $2, %eax
; CHECK-NEXT:    retq
bb:
  %i = load i64, ptr %p, align 8, !range !0, !noundef !{}
  %i1 = and i64 %i, 6
  %i2 = icmp eq i64 %i1, 2
  br i1 %i2, label %bb3, label %bb5

bb3:                                              ; preds = %bb
  %i4 = icmp ne ptr undef, null
  br label %bb5

bb5:                                              ; preds = %bb3, %bb
  br label %bb6

bb6:                                              ; preds = %bb5
  br i1 %i2, label %bb7, label %bb9

bb7:                                              ; preds = %bb6
  %i8 = getelementptr inbounds i64, ptr undef, i64 5
  br label %bb9

bb9:                                              ; preds = %bb7, %bb6
  ret void
}

!0 = !{i64 0, i64 5}
