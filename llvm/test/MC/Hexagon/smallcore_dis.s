# RUN: llvm-mc -triple=hexagon -mcpu=hexagonv67t -filetype=obj %s | llvm-objdump --no-print-imm-hex -d - | FileCheck %s
# RUN: llvm-mc -triple=hexagon -mcpu=hexagonv67t -filetype=obj %s | llvm-objdump --no-print-imm-hex --mcpu=hexagonv67t -d - | FileCheck %s

    .text
{
  r1 = memb(r0)
  if (p0) memb(r0) = r2
}

# CHECK:      { r1 = memb(r0+#0)
# CHECK-NEXT:   if (p0) memb(r0+#0) = r2 }
