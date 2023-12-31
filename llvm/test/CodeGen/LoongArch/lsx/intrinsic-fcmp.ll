; NOTE: Assertions have been autogenerated by utils/update_llc_test_checks.py
; RUN: llc --mtriple=loongarch64 --mattr=+lsx < %s | FileCheck %s

declare <4 x i32> @llvm.loongarch.lsx.vfcmp.caf.s(<4 x float>, <4 x float>)

define <4 x i32> @lsx_vfcmp_caf_s(<4 x float> %va, <4 x float> %vb) nounwind {
; CHECK-LABEL: lsx_vfcmp_caf_s:
; CHECK:       # %bb.0: # %entry
; CHECK-NEXT:    vfcmp.caf.s $vr0, $vr0, $vr1
; CHECK-NEXT:    ret
entry:
  %res = call <4 x i32> @llvm.loongarch.lsx.vfcmp.caf.s(<4 x float> %va, <4 x float> %vb)
  ret <4 x i32> %res
}

declare <2 x i64> @llvm.loongarch.lsx.vfcmp.caf.d(<2 x double>, <2 x double>)

define <2 x i64> @lsx_vfcmp_caf_d(<2 x double> %va, <2 x double> %vb) nounwind {
; CHECK-LABEL: lsx_vfcmp_caf_d:
; CHECK:       # %bb.0: # %entry
; CHECK-NEXT:    vfcmp.caf.d $vr0, $vr0, $vr1
; CHECK-NEXT:    ret
entry:
  %res = call <2 x i64> @llvm.loongarch.lsx.vfcmp.caf.d(<2 x double> %va, <2 x double> %vb)
  ret <2 x i64> %res
}

declare <4 x i32> @llvm.loongarch.lsx.vfcmp.cun.s(<4 x float>, <4 x float>)

define <4 x i32> @lsx_vfcmp_cun_s(<4 x float> %va, <4 x float> %vb) nounwind {
; CHECK-LABEL: lsx_vfcmp_cun_s:
; CHECK:       # %bb.0: # %entry
; CHECK-NEXT:    vfcmp.cun.s $vr0, $vr0, $vr1
; CHECK-NEXT:    ret
entry:
  %res = call <4 x i32> @llvm.loongarch.lsx.vfcmp.cun.s(<4 x float> %va, <4 x float> %vb)
  ret <4 x i32> %res
}

declare <2 x i64> @llvm.loongarch.lsx.vfcmp.cun.d(<2 x double>, <2 x double>)

define <2 x i64> @lsx_vfcmp_cun_d(<2 x double> %va, <2 x double> %vb) nounwind {
; CHECK-LABEL: lsx_vfcmp_cun_d:
; CHECK:       # %bb.0: # %entry
; CHECK-NEXT:    vfcmp.cun.d $vr0, $vr0, $vr1
; CHECK-NEXT:    ret
entry:
  %res = call <2 x i64> @llvm.loongarch.lsx.vfcmp.cun.d(<2 x double> %va, <2 x double> %vb)
  ret <2 x i64> %res
}

declare <4 x i32> @llvm.loongarch.lsx.vfcmp.ceq.s(<4 x float>, <4 x float>)

define <4 x i32> @lsx_vfcmp_ceq_s(<4 x float> %va, <4 x float> %vb) nounwind {
; CHECK-LABEL: lsx_vfcmp_ceq_s:
; CHECK:       # %bb.0: # %entry
; CHECK-NEXT:    vfcmp.ceq.s $vr0, $vr0, $vr1
; CHECK-NEXT:    ret
entry:
  %res = call <4 x i32> @llvm.loongarch.lsx.vfcmp.ceq.s(<4 x float> %va, <4 x float> %vb)
  ret <4 x i32> %res
}

declare <2 x i64> @llvm.loongarch.lsx.vfcmp.ceq.d(<2 x double>, <2 x double>)

define <2 x i64> @lsx_vfcmp_ceq_d(<2 x double> %va, <2 x double> %vb) nounwind {
; CHECK-LABEL: lsx_vfcmp_ceq_d:
; CHECK:       # %bb.0: # %entry
; CHECK-NEXT:    vfcmp.ceq.d $vr0, $vr0, $vr1
; CHECK-NEXT:    ret
entry:
  %res = call <2 x i64> @llvm.loongarch.lsx.vfcmp.ceq.d(<2 x double> %va, <2 x double> %vb)
  ret <2 x i64> %res
}

declare <4 x i32> @llvm.loongarch.lsx.vfcmp.cueq.s(<4 x float>, <4 x float>)

define <4 x i32> @lsx_vfcmp_cueq_s(<4 x float> %va, <4 x float> %vb) nounwind {
; CHECK-LABEL: lsx_vfcmp_cueq_s:
; CHECK:       # %bb.0: # %entry
; CHECK-NEXT:    vfcmp.cueq.s $vr0, $vr0, $vr1
; CHECK-NEXT:    ret
entry:
  %res = call <4 x i32> @llvm.loongarch.lsx.vfcmp.cueq.s(<4 x float> %va, <4 x float> %vb)
  ret <4 x i32> %res
}

declare <2 x i64> @llvm.loongarch.lsx.vfcmp.cueq.d(<2 x double>, <2 x double>)

define <2 x i64> @lsx_vfcmp_cueq_d(<2 x double> %va, <2 x double> %vb) nounwind {
; CHECK-LABEL: lsx_vfcmp_cueq_d:
; CHECK:       # %bb.0: # %entry
; CHECK-NEXT:    vfcmp.cueq.d $vr0, $vr0, $vr1
; CHECK-NEXT:    ret
entry:
  %res = call <2 x i64> @llvm.loongarch.lsx.vfcmp.cueq.d(<2 x double> %va, <2 x double> %vb)
  ret <2 x i64> %res
}

declare <4 x i32> @llvm.loongarch.lsx.vfcmp.clt.s(<4 x float>, <4 x float>)

define <4 x i32> @lsx_vfcmp_clt_s(<4 x float> %va, <4 x float> %vb) nounwind {
; CHECK-LABEL: lsx_vfcmp_clt_s:
; CHECK:       # %bb.0: # %entry
; CHECK-NEXT:    vfcmp.clt.s $vr0, $vr0, $vr1
; CHECK-NEXT:    ret
entry:
  %res = call <4 x i32> @llvm.loongarch.lsx.vfcmp.clt.s(<4 x float> %va, <4 x float> %vb)
  ret <4 x i32> %res
}

declare <2 x i64> @llvm.loongarch.lsx.vfcmp.clt.d(<2 x double>, <2 x double>)

define <2 x i64> @lsx_vfcmp_clt_d(<2 x double> %va, <2 x double> %vb) nounwind {
; CHECK-LABEL: lsx_vfcmp_clt_d:
; CHECK:       # %bb.0: # %entry
; CHECK-NEXT:    vfcmp.clt.d $vr0, $vr0, $vr1
; CHECK-NEXT:    ret
entry:
  %res = call <2 x i64> @llvm.loongarch.lsx.vfcmp.clt.d(<2 x double> %va, <2 x double> %vb)
  ret <2 x i64> %res
}

declare <4 x i32> @llvm.loongarch.lsx.vfcmp.cult.s(<4 x float>, <4 x float>)

define <4 x i32> @lsx_vfcmp_cult_s(<4 x float> %va, <4 x float> %vb) nounwind {
; CHECK-LABEL: lsx_vfcmp_cult_s:
; CHECK:       # %bb.0: # %entry
; CHECK-NEXT:    vfcmp.cult.s $vr0, $vr0, $vr1
; CHECK-NEXT:    ret
entry:
  %res = call <4 x i32> @llvm.loongarch.lsx.vfcmp.cult.s(<4 x float> %va, <4 x float> %vb)
  ret <4 x i32> %res
}

declare <2 x i64> @llvm.loongarch.lsx.vfcmp.cult.d(<2 x double>, <2 x double>)

define <2 x i64> @lsx_vfcmp_cult_d(<2 x double> %va, <2 x double> %vb) nounwind {
; CHECK-LABEL: lsx_vfcmp_cult_d:
; CHECK:       # %bb.0: # %entry
; CHECK-NEXT:    vfcmp.cult.d $vr0, $vr0, $vr1
; CHECK-NEXT:    ret
entry:
  %res = call <2 x i64> @llvm.loongarch.lsx.vfcmp.cult.d(<2 x double> %va, <2 x double> %vb)
  ret <2 x i64> %res
}

declare <4 x i32> @llvm.loongarch.lsx.vfcmp.cle.s(<4 x float>, <4 x float>)

define <4 x i32> @lsx_vfcmp_cle_s(<4 x float> %va, <4 x float> %vb) nounwind {
; CHECK-LABEL: lsx_vfcmp_cle_s:
; CHECK:       # %bb.0: # %entry
; CHECK-NEXT:    vfcmp.cle.s $vr0, $vr0, $vr1
; CHECK-NEXT:    ret
entry:
  %res = call <4 x i32> @llvm.loongarch.lsx.vfcmp.cle.s(<4 x float> %va, <4 x float> %vb)
  ret <4 x i32> %res
}

declare <2 x i64> @llvm.loongarch.lsx.vfcmp.cle.d(<2 x double>, <2 x double>)

define <2 x i64> @lsx_vfcmp_cle_d(<2 x double> %va, <2 x double> %vb) nounwind {
; CHECK-LABEL: lsx_vfcmp_cle_d:
; CHECK:       # %bb.0: # %entry
; CHECK-NEXT:    vfcmp.cle.d $vr0, $vr0, $vr1
; CHECK-NEXT:    ret
entry:
  %res = call <2 x i64> @llvm.loongarch.lsx.vfcmp.cle.d(<2 x double> %va, <2 x double> %vb)
  ret <2 x i64> %res
}

declare <4 x i32> @llvm.loongarch.lsx.vfcmp.cule.s(<4 x float>, <4 x float>)

define <4 x i32> @lsx_vfcmp_cule_s(<4 x float> %va, <4 x float> %vb) nounwind {
; CHECK-LABEL: lsx_vfcmp_cule_s:
; CHECK:       # %bb.0: # %entry
; CHECK-NEXT:    vfcmp.cule.s $vr0, $vr0, $vr1
; CHECK-NEXT:    ret
entry:
  %res = call <4 x i32> @llvm.loongarch.lsx.vfcmp.cule.s(<4 x float> %va, <4 x float> %vb)
  ret <4 x i32> %res
}

declare <2 x i64> @llvm.loongarch.lsx.vfcmp.cule.d(<2 x double>, <2 x double>)

define <2 x i64> @lsx_vfcmp_cule_d(<2 x double> %va, <2 x double> %vb) nounwind {
; CHECK-LABEL: lsx_vfcmp_cule_d:
; CHECK:       # %bb.0: # %entry
; CHECK-NEXT:    vfcmp.cule.d $vr0, $vr0, $vr1
; CHECK-NEXT:    ret
entry:
  %res = call <2 x i64> @llvm.loongarch.lsx.vfcmp.cule.d(<2 x double> %va, <2 x double> %vb)
  ret <2 x i64> %res
}

declare <4 x i32> @llvm.loongarch.lsx.vfcmp.cne.s(<4 x float>, <4 x float>)

define <4 x i32> @lsx_vfcmp_cne_s(<4 x float> %va, <4 x float> %vb) nounwind {
; CHECK-LABEL: lsx_vfcmp_cne_s:
; CHECK:       # %bb.0: # %entry
; CHECK-NEXT:    vfcmp.cne.s $vr0, $vr0, $vr1
; CHECK-NEXT:    ret
entry:
  %res = call <4 x i32> @llvm.loongarch.lsx.vfcmp.cne.s(<4 x float> %va, <4 x float> %vb)
  ret <4 x i32> %res
}

declare <2 x i64> @llvm.loongarch.lsx.vfcmp.cne.d(<2 x double>, <2 x double>)

define <2 x i64> @lsx_vfcmp_cne_d(<2 x double> %va, <2 x double> %vb) nounwind {
; CHECK-LABEL: lsx_vfcmp_cne_d:
; CHECK:       # %bb.0: # %entry
; CHECK-NEXT:    vfcmp.cne.d $vr0, $vr0, $vr1
; CHECK-NEXT:    ret
entry:
  %res = call <2 x i64> @llvm.loongarch.lsx.vfcmp.cne.d(<2 x double> %va, <2 x double> %vb)
  ret <2 x i64> %res
}

declare <4 x i32> @llvm.loongarch.lsx.vfcmp.cor.s(<4 x float>, <4 x float>)

define <4 x i32> @lsx_vfcmp_cor_s(<4 x float> %va, <4 x float> %vb) nounwind {
; CHECK-LABEL: lsx_vfcmp_cor_s:
; CHECK:       # %bb.0: # %entry
; CHECK-NEXT:    vfcmp.cor.s $vr0, $vr0, $vr1
; CHECK-NEXT:    ret
entry:
  %res = call <4 x i32> @llvm.loongarch.lsx.vfcmp.cor.s(<4 x float> %va, <4 x float> %vb)
  ret <4 x i32> %res
}

declare <2 x i64> @llvm.loongarch.lsx.vfcmp.cor.d(<2 x double>, <2 x double>)

define <2 x i64> @lsx_vfcmp_cor_d(<2 x double> %va, <2 x double> %vb) nounwind {
; CHECK-LABEL: lsx_vfcmp_cor_d:
; CHECK:       # %bb.0: # %entry
; CHECK-NEXT:    vfcmp.cor.d $vr0, $vr0, $vr1
; CHECK-NEXT:    ret
entry:
  %res = call <2 x i64> @llvm.loongarch.lsx.vfcmp.cor.d(<2 x double> %va, <2 x double> %vb)
  ret <2 x i64> %res
}

declare <4 x i32> @llvm.loongarch.lsx.vfcmp.cune.s(<4 x float>, <4 x float>)

define <4 x i32> @lsx_vfcmp_cune_s(<4 x float> %va, <4 x float> %vb) nounwind {
; CHECK-LABEL: lsx_vfcmp_cune_s:
; CHECK:       # %bb.0: # %entry
; CHECK-NEXT:    vfcmp.cune.s $vr0, $vr0, $vr1
; CHECK-NEXT:    ret
entry:
  %res = call <4 x i32> @llvm.loongarch.lsx.vfcmp.cune.s(<4 x float> %va, <4 x float> %vb)
  ret <4 x i32> %res
}

declare <2 x i64> @llvm.loongarch.lsx.vfcmp.cune.d(<2 x double>, <2 x double>)

define <2 x i64> @lsx_vfcmp_cune_d(<2 x double> %va, <2 x double> %vb) nounwind {
; CHECK-LABEL: lsx_vfcmp_cune_d:
; CHECK:       # %bb.0: # %entry
; CHECK-NEXT:    vfcmp.cune.d $vr0, $vr0, $vr1
; CHECK-NEXT:    ret
entry:
  %res = call <2 x i64> @llvm.loongarch.lsx.vfcmp.cune.d(<2 x double> %va, <2 x double> %vb)
  ret <2 x i64> %res
}

declare <4 x i32> @llvm.loongarch.lsx.vfcmp.saf.s(<4 x float>, <4 x float>)

define <4 x i32> @lsx_vfcmp_saf_s(<4 x float> %va, <4 x float> %vb) nounwind {
; CHECK-LABEL: lsx_vfcmp_saf_s:
; CHECK:       # %bb.0: # %entry
; CHECK-NEXT:    vfcmp.saf.s $vr0, $vr0, $vr1
; CHECK-NEXT:    ret
entry:
  %res = call <4 x i32> @llvm.loongarch.lsx.vfcmp.saf.s(<4 x float> %va, <4 x float> %vb)
  ret <4 x i32> %res
}

declare <2 x i64> @llvm.loongarch.lsx.vfcmp.saf.d(<2 x double>, <2 x double>)

define <2 x i64> @lsx_vfcmp_saf_d(<2 x double> %va, <2 x double> %vb) nounwind {
; CHECK-LABEL: lsx_vfcmp_saf_d:
; CHECK:       # %bb.0: # %entry
; CHECK-NEXT:    vfcmp.saf.d $vr0, $vr0, $vr1
; CHECK-NEXT:    ret
entry:
  %res = call <2 x i64> @llvm.loongarch.lsx.vfcmp.saf.d(<2 x double> %va, <2 x double> %vb)
  ret <2 x i64> %res
}

declare <4 x i32> @llvm.loongarch.lsx.vfcmp.sun.s(<4 x float>, <4 x float>)

define <4 x i32> @lsx_vfcmp_sun_s(<4 x float> %va, <4 x float> %vb) nounwind {
; CHECK-LABEL: lsx_vfcmp_sun_s:
; CHECK:       # %bb.0: # %entry
; CHECK-NEXT:    vfcmp.sun.s $vr0, $vr0, $vr1
; CHECK-NEXT:    ret
entry:
  %res = call <4 x i32> @llvm.loongarch.lsx.vfcmp.sun.s(<4 x float> %va, <4 x float> %vb)
  ret <4 x i32> %res
}

declare <2 x i64> @llvm.loongarch.lsx.vfcmp.sun.d(<2 x double>, <2 x double>)

define <2 x i64> @lsx_vfcmp_sun_d(<2 x double> %va, <2 x double> %vb) nounwind {
; CHECK-LABEL: lsx_vfcmp_sun_d:
; CHECK:       # %bb.0: # %entry
; CHECK-NEXT:    vfcmp.sun.d $vr0, $vr0, $vr1
; CHECK-NEXT:    ret
entry:
  %res = call <2 x i64> @llvm.loongarch.lsx.vfcmp.sun.d(<2 x double> %va, <2 x double> %vb)
  ret <2 x i64> %res
}

declare <4 x i32> @llvm.loongarch.lsx.vfcmp.seq.s(<4 x float>, <4 x float>)

define <4 x i32> @lsx_vfcmp_seq_s(<4 x float> %va, <4 x float> %vb) nounwind {
; CHECK-LABEL: lsx_vfcmp_seq_s:
; CHECK:       # %bb.0: # %entry
; CHECK-NEXT:    vfcmp.seq.s $vr0, $vr0, $vr1
; CHECK-NEXT:    ret
entry:
  %res = call <4 x i32> @llvm.loongarch.lsx.vfcmp.seq.s(<4 x float> %va, <4 x float> %vb)
  ret <4 x i32> %res
}

declare <2 x i64> @llvm.loongarch.lsx.vfcmp.seq.d(<2 x double>, <2 x double>)

define <2 x i64> @lsx_vfcmp_seq_d(<2 x double> %va, <2 x double> %vb) nounwind {
; CHECK-LABEL: lsx_vfcmp_seq_d:
; CHECK:       # %bb.0: # %entry
; CHECK-NEXT:    vfcmp.seq.d $vr0, $vr0, $vr1
; CHECK-NEXT:    ret
entry:
  %res = call <2 x i64> @llvm.loongarch.lsx.vfcmp.seq.d(<2 x double> %va, <2 x double> %vb)
  ret <2 x i64> %res
}

declare <4 x i32> @llvm.loongarch.lsx.vfcmp.sueq.s(<4 x float>, <4 x float>)

define <4 x i32> @lsx_vfcmp_sueq_s(<4 x float> %va, <4 x float> %vb) nounwind {
; CHECK-LABEL: lsx_vfcmp_sueq_s:
; CHECK:       # %bb.0: # %entry
; CHECK-NEXT:    vfcmp.sueq.s $vr0, $vr0, $vr1
; CHECK-NEXT:    ret
entry:
  %res = call <4 x i32> @llvm.loongarch.lsx.vfcmp.sueq.s(<4 x float> %va, <4 x float> %vb)
  ret <4 x i32> %res
}

declare <2 x i64> @llvm.loongarch.lsx.vfcmp.sueq.d(<2 x double>, <2 x double>)

define <2 x i64> @lsx_vfcmp_sueq_d(<2 x double> %va, <2 x double> %vb) nounwind {
; CHECK-LABEL: lsx_vfcmp_sueq_d:
; CHECK:       # %bb.0: # %entry
; CHECK-NEXT:    vfcmp.sueq.d $vr0, $vr0, $vr1
; CHECK-NEXT:    ret
entry:
  %res = call <2 x i64> @llvm.loongarch.lsx.vfcmp.sueq.d(<2 x double> %va, <2 x double> %vb)
  ret <2 x i64> %res
}

declare <4 x i32> @llvm.loongarch.lsx.vfcmp.slt.s(<4 x float>, <4 x float>)

define <4 x i32> @lsx_vfcmp_slt_s(<4 x float> %va, <4 x float> %vb) nounwind {
; CHECK-LABEL: lsx_vfcmp_slt_s:
; CHECK:       # %bb.0: # %entry
; CHECK-NEXT:    vfcmp.slt.s $vr0, $vr0, $vr1
; CHECK-NEXT:    ret
entry:
  %res = call <4 x i32> @llvm.loongarch.lsx.vfcmp.slt.s(<4 x float> %va, <4 x float> %vb)
  ret <4 x i32> %res
}

declare <2 x i64> @llvm.loongarch.lsx.vfcmp.slt.d(<2 x double>, <2 x double>)

define <2 x i64> @lsx_vfcmp_slt_d(<2 x double> %va, <2 x double> %vb) nounwind {
; CHECK-LABEL: lsx_vfcmp_slt_d:
; CHECK:       # %bb.0: # %entry
; CHECK-NEXT:    vfcmp.slt.d $vr0, $vr0, $vr1
; CHECK-NEXT:    ret
entry:
  %res = call <2 x i64> @llvm.loongarch.lsx.vfcmp.slt.d(<2 x double> %va, <2 x double> %vb)
  ret <2 x i64> %res
}

declare <4 x i32> @llvm.loongarch.lsx.vfcmp.sult.s(<4 x float>, <4 x float>)

define <4 x i32> @lsx_vfcmp_sult_s(<4 x float> %va, <4 x float> %vb) nounwind {
; CHECK-LABEL: lsx_vfcmp_sult_s:
; CHECK:       # %bb.0: # %entry
; CHECK-NEXT:    vfcmp.sult.s $vr0, $vr0, $vr1
; CHECK-NEXT:    ret
entry:
  %res = call <4 x i32> @llvm.loongarch.lsx.vfcmp.sult.s(<4 x float> %va, <4 x float> %vb)
  ret <4 x i32> %res
}

declare <2 x i64> @llvm.loongarch.lsx.vfcmp.sult.d(<2 x double>, <2 x double>)

define <2 x i64> @lsx_vfcmp_sult_d(<2 x double> %va, <2 x double> %vb) nounwind {
; CHECK-LABEL: lsx_vfcmp_sult_d:
; CHECK:       # %bb.0: # %entry
; CHECK-NEXT:    vfcmp.sult.d $vr0, $vr0, $vr1
; CHECK-NEXT:    ret
entry:
  %res = call <2 x i64> @llvm.loongarch.lsx.vfcmp.sult.d(<2 x double> %va, <2 x double> %vb)
  ret <2 x i64> %res
}

declare <4 x i32> @llvm.loongarch.lsx.vfcmp.sle.s(<4 x float>, <4 x float>)

define <4 x i32> @lsx_vfcmp_sle_s(<4 x float> %va, <4 x float> %vb) nounwind {
; CHECK-LABEL: lsx_vfcmp_sle_s:
; CHECK:       # %bb.0: # %entry
; CHECK-NEXT:    vfcmp.sle.s $vr0, $vr0, $vr1
; CHECK-NEXT:    ret
entry:
  %res = call <4 x i32> @llvm.loongarch.lsx.vfcmp.sle.s(<4 x float> %va, <4 x float> %vb)
  ret <4 x i32> %res
}

declare <2 x i64> @llvm.loongarch.lsx.vfcmp.sle.d(<2 x double>, <2 x double>)

define <2 x i64> @lsx_vfcmp_sle_d(<2 x double> %va, <2 x double> %vb) nounwind {
; CHECK-LABEL: lsx_vfcmp_sle_d:
; CHECK:       # %bb.0: # %entry
; CHECK-NEXT:    vfcmp.sle.d $vr0, $vr0, $vr1
; CHECK-NEXT:    ret
entry:
  %res = call <2 x i64> @llvm.loongarch.lsx.vfcmp.sle.d(<2 x double> %va, <2 x double> %vb)
  ret <2 x i64> %res
}

declare <4 x i32> @llvm.loongarch.lsx.vfcmp.sule.s(<4 x float>, <4 x float>)

define <4 x i32> @lsx_vfcmp_sule_s(<4 x float> %va, <4 x float> %vb) nounwind {
; CHECK-LABEL: lsx_vfcmp_sule_s:
; CHECK:       # %bb.0: # %entry
; CHECK-NEXT:    vfcmp.sule.s $vr0, $vr0, $vr1
; CHECK-NEXT:    ret
entry:
  %res = call <4 x i32> @llvm.loongarch.lsx.vfcmp.sule.s(<4 x float> %va, <4 x float> %vb)
  ret <4 x i32> %res
}

declare <2 x i64> @llvm.loongarch.lsx.vfcmp.sule.d(<2 x double>, <2 x double>)

define <2 x i64> @lsx_vfcmp_sule_d(<2 x double> %va, <2 x double> %vb) nounwind {
; CHECK-LABEL: lsx_vfcmp_sule_d:
; CHECK:       # %bb.0: # %entry
; CHECK-NEXT:    vfcmp.sule.d $vr0, $vr0, $vr1
; CHECK-NEXT:    ret
entry:
  %res = call <2 x i64> @llvm.loongarch.lsx.vfcmp.sule.d(<2 x double> %va, <2 x double> %vb)
  ret <2 x i64> %res
}

declare <4 x i32> @llvm.loongarch.lsx.vfcmp.sne.s(<4 x float>, <4 x float>)

define <4 x i32> @lsx_vfcmp_sne_s(<4 x float> %va, <4 x float> %vb) nounwind {
; CHECK-LABEL: lsx_vfcmp_sne_s:
; CHECK:       # %bb.0: # %entry
; CHECK-NEXT:    vfcmp.sne.s $vr0, $vr0, $vr1
; CHECK-NEXT:    ret
entry:
  %res = call <4 x i32> @llvm.loongarch.lsx.vfcmp.sne.s(<4 x float> %va, <4 x float> %vb)
  ret <4 x i32> %res
}

declare <2 x i64> @llvm.loongarch.lsx.vfcmp.sne.d(<2 x double>, <2 x double>)

define <2 x i64> @lsx_vfcmp_sne_d(<2 x double> %va, <2 x double> %vb) nounwind {
; CHECK-LABEL: lsx_vfcmp_sne_d:
; CHECK:       # %bb.0: # %entry
; CHECK-NEXT:    vfcmp.sne.d $vr0, $vr0, $vr1
; CHECK-NEXT:    ret
entry:
  %res = call <2 x i64> @llvm.loongarch.lsx.vfcmp.sne.d(<2 x double> %va, <2 x double> %vb)
  ret <2 x i64> %res
}

declare <4 x i32> @llvm.loongarch.lsx.vfcmp.sor.s(<4 x float>, <4 x float>)

define <4 x i32> @lsx_vfcmp_sor_s(<4 x float> %va, <4 x float> %vb) nounwind {
; CHECK-LABEL: lsx_vfcmp_sor_s:
; CHECK:       # %bb.0: # %entry
; CHECK-NEXT:    vfcmp.sor.s $vr0, $vr0, $vr1
; CHECK-NEXT:    ret
entry:
  %res = call <4 x i32> @llvm.loongarch.lsx.vfcmp.sor.s(<4 x float> %va, <4 x float> %vb)
  ret <4 x i32> %res
}

declare <2 x i64> @llvm.loongarch.lsx.vfcmp.sor.d(<2 x double>, <2 x double>)

define <2 x i64> @lsx_vfcmp_sor_d(<2 x double> %va, <2 x double> %vb) nounwind {
; CHECK-LABEL: lsx_vfcmp_sor_d:
; CHECK:       # %bb.0: # %entry
; CHECK-NEXT:    vfcmp.sor.d $vr0, $vr0, $vr1
; CHECK-NEXT:    ret
entry:
  %res = call <2 x i64> @llvm.loongarch.lsx.vfcmp.sor.d(<2 x double> %va, <2 x double> %vb)
  ret <2 x i64> %res
}

declare <4 x i32> @llvm.loongarch.lsx.vfcmp.sune.s(<4 x float>, <4 x float>)

define <4 x i32> @lsx_vfcmp_sune_s(<4 x float> %va, <4 x float> %vb) nounwind {
; CHECK-LABEL: lsx_vfcmp_sune_s:
; CHECK:       # %bb.0: # %entry
; CHECK-NEXT:    vfcmp.sune.s $vr0, $vr0, $vr1
; CHECK-NEXT:    ret
entry:
  %res = call <4 x i32> @llvm.loongarch.lsx.vfcmp.sune.s(<4 x float> %va, <4 x float> %vb)
  ret <4 x i32> %res
}

declare <2 x i64> @llvm.loongarch.lsx.vfcmp.sune.d(<2 x double>, <2 x double>)

define <2 x i64> @lsx_vfcmp_sune_d(<2 x double> %va, <2 x double> %vb) nounwind {
; CHECK-LABEL: lsx_vfcmp_sune_d:
; CHECK:       # %bb.0: # %entry
; CHECK-NEXT:    vfcmp.sune.d $vr0, $vr0, $vr1
; CHECK-NEXT:    ret
entry:
  %res = call <2 x i64> @llvm.loongarch.lsx.vfcmp.sune.d(<2 x double> %va, <2 x double> %vb)
  ret <2 x i64> %res
}
