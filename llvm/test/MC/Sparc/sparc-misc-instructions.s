! RUN: llvm-mc %s -triple=sparc   -show-encoding | FileCheck %s
! RUN: llvm-mc %s -triple=sparcv9 -show-encoding | FileCheck %s

        ! CHECK: unimp 0   ! encoding: [0x00,0x00,0x00,0x00]
        unimp

        ! CHECK: unimp 0   ! encoding: [0x00,0x00,0x00,0x00]
        unimp 0
