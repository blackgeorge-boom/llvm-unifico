; NOTE: Assertions have been autogenerated by utils/update_llc_test_checks.py
; RUN: llc -relocation-model=pic -verify-machineinstrs -mcpu=pwr8 -ppc-vsr-nums-as-vr \
; RUN:   -ppc-asm-full-reg-names -mtriple=powerpc64le-unknown-linux-gnu \
; RUN:   -O3 < %s | FileCheck %s

; RUN: llc -relocation-model=pic -mcpu=pwr9 -mtriple=powerpc64le-unknown-linux-gnu -O3 \
; RUN:   -ppc-vsr-nums-as-vr -ppc-asm-full-reg-names -verify-machineinstrs \
; RUN:   < %s | FileCheck %s --check-prefix=CHECK-P9 \
; RUN:   --implicit-check-not xxswapd

; RUN: llc -relocation-model=pic -mcpu=pwr9 -mtriple=powerpc64le-unknown-linux-gnu -O3 \
; RUN:   -ppc-vsr-nums-as-vr -ppc-asm-full-reg-names -verify-machineinstrs \
; RUN:   -mattr=-power9-vector < %s | FileCheck %s --check-prefix=CHECK-P9-NOVECTOR

; These tests verify that VSX swap optimization works when loading a scalar
; into a vector register.


@x = global <2 x double> <double 9.970000e+01, double -1.032220e+02>, align 16
@z = global <2 x double> <double 2.332000e+01, double 3.111111e+01>, align 16
@y = global double 1.780000e+00, align 8

define void @bar0() {
; CHECK-LABEL: bar0:
; CHECK:       # %bb.0: # %entry
; CHECK-NEXT:    addis r3, r2, .LC0@toc@ha
; CHECK-NEXT:    ld r3, .LC0@toc@l(r3)
; CHECK-NEXT:    lxvd2x vs0, 0, r3
; CHECK-NEXT:    addis r3, r2, .LC1@toc@ha
; CHECK-NEXT:    ld r3, .LC1@toc@l(r3)
; CHECK-NEXT:    lfd f1, 0(r3)
; CHECK-NEXT:    addis r3, r2, .LC2@toc@ha
; CHECK-NEXT:    ld r3, .LC2@toc@l(r3)
; CHECK-NEXT:    xxswapd vs0, vs0
; CHECK-NEXT:    xxmrghd vs0, vs0, vs1
; CHECK-NEXT:    xxswapd vs0, vs0
; CHECK-NEXT:    stxvd2x vs0, 0, r3
; CHECK-NEXT:    blr
;
; CHECK-P9-LABEL: bar0:
; CHECK-P9:       # %bb.0: # %entry
; CHECK-P9-NEXT:    addis r3, r2, .LC0@toc@ha
; CHECK-P9-NEXT:    ld r3, .LC0@toc@l(r3)
; CHECK-P9-NEXT:    lxv vs0, 0(r3)
; CHECK-P9-NEXT:    addis r3, r2, .LC1@toc@ha
; CHECK-P9-NEXT:    ld r3, .LC1@toc@l(r3)
; CHECK-P9-NEXT:    lfd f1, 0(r3)
; CHECK-P9-NEXT:    addis r3, r2, .LC2@toc@ha
; CHECK-P9-NEXT:    ld r3, .LC2@toc@l(r3)
; CHECK-P9-NEXT:    xxmrghd vs0, vs0, vs1
; CHECK-P9-NEXT:    stxv vs0, 0(r3)
; CHECK-P9-NEXT:    blr
;
; CHECK-P9-NOVECTOR-LABEL: bar0:
; CHECK-P9-NOVECTOR:       # %bb.0: # %entry
; CHECK-P9-NOVECTOR-NEXT:    addis r3, r2, .LC0@toc@ha
; CHECK-P9-NOVECTOR-NEXT:    ld r3, .LC0@toc@l(r3)
; CHECK-P9-NOVECTOR-NEXT:    lxvd2x vs0, 0, r3
; CHECK-P9-NOVECTOR-NEXT:    addis r3, r2, .LC1@toc@ha
; CHECK-P9-NOVECTOR-NEXT:    ld r3, .LC1@toc@l(r3)
; CHECK-P9-NOVECTOR-NEXT:    xxswapd vs0, vs0
; CHECK-P9-NOVECTOR-NEXT:    lfd f1, 0(r3)
; CHECK-P9-NOVECTOR-NEXT:    addis r3, r2, .LC2@toc@ha
; CHECK-P9-NOVECTOR-NEXT:    ld r3, .LC2@toc@l(r3)
; CHECK-P9-NOVECTOR-NEXT:    xxmrghd vs0, vs0, vs1
; CHECK-P9-NOVECTOR-NEXT:    xxswapd vs0, vs0
; CHECK-P9-NOVECTOR-NEXT:    stxvd2x vs0, 0, r3
; CHECK-P9-NOVECTOR-NEXT:    blr
entry:
  %0 = load <2 x double>, ptr @x, align 16
  %1 = load double, ptr @y, align 8
  %vecins = insertelement <2 x double> %0, double %1, i32 0
  store <2 x double> %vecins, ptr @z, align 16
  ret void
}

define void @bar1() {
; CHECK-LABEL: bar1:
; CHECK:       # %bb.0: # %entry
; CHECK-NEXT:    addis r3, r2, .LC0@toc@ha
; CHECK-NEXT:    ld r3, .LC0@toc@l(r3)
; CHECK-NEXT:    lxvd2x vs0, 0, r3
; CHECK-NEXT:    addis r3, r2, .LC1@toc@ha
; CHECK-NEXT:    ld r3, .LC1@toc@l(r3)
; CHECK-NEXT:    lfd f1, 0(r3)
; CHECK-NEXT:    addis r3, r2, .LC2@toc@ha
; CHECK-NEXT:    ld r3, .LC2@toc@l(r3)
; CHECK-NEXT:    xxswapd vs0, vs0
; CHECK-NEXT:    xxpermdi vs0, vs1, vs0, 1
; CHECK-NEXT:    xxswapd vs0, vs0
; CHECK-NEXT:    stxvd2x vs0, 0, r3
; CHECK-NEXT:    blr
;
; CHECK-P9-LABEL: bar1:
; CHECK-P9:       # %bb.0: # %entry
; CHECK-P9-NEXT:    addis r3, r2, .LC0@toc@ha
; CHECK-P9-NEXT:    ld r3, .LC0@toc@l(r3)
; CHECK-P9-NEXT:    lxv vs0, 0(r3)
; CHECK-P9-NEXT:    addis r3, r2, .LC1@toc@ha
; CHECK-P9-NEXT:    ld r3, .LC1@toc@l(r3)
; CHECK-P9-NEXT:    lfd f1, 0(r3)
; CHECK-P9-NEXT:    addis r3, r2, .LC2@toc@ha
; CHECK-P9-NEXT:    ld r3, .LC2@toc@l(r3)
; CHECK-P9-NEXT:    xxpermdi vs0, vs1, vs0, 1
; CHECK-P9-NEXT:    stxv vs0, 0(r3)
; CHECK-P9-NEXT:    blr
;
; CHECK-P9-NOVECTOR-LABEL: bar1:
; CHECK-P9-NOVECTOR:       # %bb.0: # %entry
; CHECK-P9-NOVECTOR-NEXT:    addis r3, r2, .LC0@toc@ha
; CHECK-P9-NOVECTOR-NEXT:    ld r3, .LC0@toc@l(r3)
; CHECK-P9-NOVECTOR-NEXT:    lxvd2x vs0, 0, r3
; CHECK-P9-NOVECTOR-NEXT:    addis r3, r2, .LC1@toc@ha
; CHECK-P9-NOVECTOR-NEXT:    ld r3, .LC1@toc@l(r3)
; CHECK-P9-NOVECTOR-NEXT:    xxswapd vs0, vs0
; CHECK-P9-NOVECTOR-NEXT:    lfd f1, 0(r3)
; CHECK-P9-NOVECTOR-NEXT:    addis r3, r2, .LC2@toc@ha
; CHECK-P9-NOVECTOR-NEXT:    ld r3, .LC2@toc@l(r3)
; CHECK-P9-NOVECTOR-NEXT:    xxpermdi vs0, vs1, vs0, 1
; CHECK-P9-NOVECTOR-NEXT:    xxswapd vs0, vs0
; CHECK-P9-NOVECTOR-NEXT:    stxvd2x vs0, 0, r3
; CHECK-P9-NOVECTOR-NEXT:    blr
entry:
  %0 = load <2 x double>, ptr @x, align 16
  %1 = load double, ptr @y, align 8
  %vecins = insertelement <2 x double> %0, double %1, i32 1
  store <2 x double> %vecins, ptr @z, align 16
  ret void
}

