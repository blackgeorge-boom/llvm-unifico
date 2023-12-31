; NOTE: Assertions have been autogenerated by utils/update_test_checks.py UTC_ARGS: --version 2
; RUN: opt -S -passes='require<profile-summary>,chr' -verify-region-info < %s | FileCheck %s

define void @test(i1 %c, i1 %c2) !prof !29 {
; CHECK-LABEL: define void @test
; CHECK-SAME: (i1 [[C:%.*]], i1 [[C2:%.*]]) !prof [[PROF29:![0-9]+]] {
; CHECK-NEXT:  entry:
; CHECK-NEXT:    [[TMP0:%.*]] = xor i1 true, [[C]]
; CHECK-NEXT:    [[TMP1:%.*]] = freeze i1 [[TMP0]]
; CHECK-NEXT:    [[TMP2:%.*]] = select i1 true, i1 [[TMP1]], i1 false
; CHECK-NEXT:    [[TMP3:%.*]] = xor i1 true, [[C2]]
; CHECK-NEXT:    [[TMP4:%.*]] = freeze i1 [[TMP3]]
; CHECK-NEXT:    [[TMP5:%.*]] = select i1 [[TMP2]], i1 [[TMP4]], i1 false
; CHECK-NEXT:    br i1 [[TMP5]], label [[ENTRY_SPLIT:%.*]], label [[ENTRY_SPLIT_NONCHR:%.*]], !prof [[PROF30:![0-9]+]]
; CHECK:       entry.split:
; CHECK-NEXT:    switch i8 0, label [[BB1:%.*]] [
; CHECK-NEXT:    i8 1, label [[BB2:%.*]]
; CHECK-NEXT:    i8 2, label [[BB3:%.*]]
; CHECK-NEXT:    ]
; CHECK:       bb1:
; CHECK-NEXT:    [[SELECT:%.*]] = select i1 false, i32 0, i32 1, !prof [[PROF31:![0-9]+]]
; CHECK-NEXT:    br label [[EXIT:%.*]]
; CHECK:       bb2:
; CHECK-NEXT:    [[SELECT3:%.*]] = select i1 false, i32 0, i32 1, !prof [[PROF32:![0-9]+]]
; CHECK-NEXT:    br label [[EXIT]]
; CHECK:       dead:
; CHECK-NEXT:    br label [[BB3]]
; CHECK:       bb3:
; CHECK-NEXT:    [[PHI:%.*]] = phi i64 [ 0, [[DEAD:%.*]] ], [ 1, [[ENTRY_SPLIT]] ]
; CHECK-NEXT:    br label [[EXIT]]
; CHECK:       entry.split.nonchr:
; CHECK-NEXT:    switch i8 0, label [[BB1_NONCHR:%.*]] [
; CHECK-NEXT:    i8 1, label [[BB2_NONCHR:%.*]]
; CHECK-NEXT:    i8 2, label [[BB3_NONCHR:%.*]]
; CHECK-NEXT:    ]
; CHECK:       bb1.nonchr:
; CHECK-NEXT:    [[SELECT_NONCHR:%.*]] = select i1 [[C]], i32 0, i32 1, !prof [[PROF31]]
; CHECK-NEXT:    br label [[EXIT]]
; CHECK:       bb2.nonchr:
; CHECK-NEXT:    [[SELECT3_NONCHR:%.*]] = select i1 [[C2]], i32 0, i32 1, !prof [[PROF32]]
; CHECK-NEXT:    br label [[EXIT]]
; CHECK:       bb3.nonchr:
; CHECK-NEXT:    [[PHI_NONCHR:%.*]] = phi i64 [ 1, [[ENTRY_SPLIT_NONCHR]] ]
; CHECK-NEXT:    br label [[EXIT]]
; CHECK:       exit:
; CHECK-NEXT:    ret void
;
entry:
  switch i8 0, label %bb1 [
  i8 1, label %bb2
  i8 2, label %bb3
  ]

bb1:                                              ; preds = %entry
  %select = select i1 %c, i32 0, i32 1, !prof !30
  br label %exit

bb2:                                              ; preds = %entry
  %select3 = select i1 %c2, i32 0, i32 1, !prof !31
  br label %exit

dead:                                             ; No predecessors!
  br label %bb3

bb3:                                              ; preds = %dead, %entry
  %phi = phi i64 [ 0, %dead ], [ 1, %entry ]
  br label %exit

exit:                                             ; preds = %bb3, %bb2, %bb1
  ret void
}

!llvm.module.flags = !{!0}

!0 = !{i32 1, !"ProfileSummary", !1}
!1 = !{!2, !3, !4, !5, !6, !7, !8, !9, !10, !11}
!2 = !{!"ProfileFormat", !"InstrProf"}
!3 = !{!"TotalCount", i64 597326977313}
!4 = !{!"MaxCount", i64 12561793713}
!5 = !{!"MaxInternalCount", i64 2509052618}
!6 = !{!"MaxFunctionCount", i64 12561793713}
!7 = !{!"NumCounts", i64 1694881}
!8 = !{!"NumFunctions", i64 129214}
!9 = !{!"IsPartialProfile", i64 0}
!10 = !{!"PartialProfileRatio", double 0.000000e+00}
!11 = !{!"DetailedSummary", !12}
!12 = !{!13, !14, !15, !16, !17, !18, !19, !20, !21, !22, !23, !24, !25, !26, !27, !28}
!13 = !{i32 10000, i64 12561793713, i32 1}
!14 = !{i32 100000, i64 1733566697, i32 20}
!15 = !{i32 200000, i64 820928443, i32 71}
!16 = !{i32 300000, i64 404967336, i32 182}
!17 = !{i32 400000, i64 233162193, i32 376}
!18 = !{i32 500000, i64 120552435, i32 741}
!19 = !{i32 600000, i64 69388652, i32 1402}
!20 = !{i32 700000, i64 33926336, i32 2643}
!21 = !{i32 800000, i64 15635940, i32 5288}
!22 = !{i32 900000, i64 5547105, i32 11637}
!23 = !{i32 950000, i64 2224405, i32 20074}
!24 = !{i32 990000, i64 359838, i32 44778}
!25 = !{i32 999000, i64 37485, i32 81744}
!26 = !{i32 999900, i64 3465, i32 119656}
!27 = !{i32 999990, i64 529, i32 155440}
!28 = !{i32 999999, i64 70, i32 178344}
!29 = !{!"function_entry_count", i64 33781183}
!30 = !{!"branch_weights", i32 0, i32 9263770}
!31 = !{!"branch_weights", i32 0, i32 634318}
