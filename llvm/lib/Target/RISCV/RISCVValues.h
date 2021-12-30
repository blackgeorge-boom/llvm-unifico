//===----- AArch64TargetValues.cpp - AArch64 specific value generator -----===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "llvm/Target/TargetValues.h"

namespace llvm {

class RISCVValues final : public TargetValues {
public:
  RISCVValues() {}
  virtual TemporaryValuePtr getTemporaryValue(const MachineInstr *MI,
                                              const VirtRegMap *VRM) const;
  virtual MachineLiveValPtr getMachineValue(const MachineInstr *MI) const;

private:
  MachineLiveVal *genADDInstructions(const MachineInstr *MI) const;
  MachineLiveVal *genADRPInstructions(const MachineInstr *MI) const;
  MachineLiveVal *genBitfieldInstructions(const MachineInstr *MI) const;
  MachineLiveVal *genLoadRegValue(const MachineInstr *MI) const;
};

}
