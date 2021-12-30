//===- RISCVTargetValues.cpp - RISCV specific value generator -===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "RISCVValues.h"
#include "RISCV.h"
#include "llvm/CodeGen/MachineConstantPool.h"
#include "llvm/CodeGen/TargetInstrInfo.h"
#include "llvm/CodeGen/TargetSubtargetInfo.h"
#include "llvm/IR/Constants.h"
#include "llvm/MC/MCSymbol.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

#define DEBUG_TYPE "stacktransform"

using namespace llvm;

static TemporaryValue *getTemporaryReference(const MachineInstr *MI,
                                             const VirtRegMap *VRM,
                                             unsigned Size) {
  TemporaryValue *Val = nullptr;
  if(MI->getOperand(0).isReg()) {
    // Instruction format:    ADDI  xd    xn    imm#
    // Stack slot reference:                <fi>  0 
    if(MI->getOperand(1).isFI() &&
       MI->getOperand(2).isImm() && MI->getOperand(2).getImm() == 0) {
      Val = new TemporaryValue;
      Val->Type = TemporaryValue::StackSlotRef;
      Val->Size = Size;
      Val->Vreg = MI->getOperand(0).getReg();
      Val->StackSlot = MI->getOperand(1).getIndex();
      Val->Offset = 0;
    }
  }

  return Val;
}

TemporaryValuePtr
RISCVValues::getTemporaryValue(const MachineInstr *MI,
                                 const VirtRegMap *VRM) const {
  TemporaryValue *Val = nullptr;
  switch(MI->getOpcode()) {
  case RISCV::ADDI: Val = getTemporaryReference(MI, VRM, 8); break;
  default: break;
  }
  return TemporaryValuePtr(Val);
}

typedef ValueGenInst::InstType InstType;
template <InstType T> using RegInstruction = RegInstruction<T>;
template <InstType T> using ImmInstruction = ImmInstruction<T>;

// Bitwise-conversions between floats & ints
union IntFloat64 { double d; uint64_t i; };
union IntFloat32 { float f; uint64_t i; };

MachineLiveVal *
RISCVValues::
genADDInstructions(const MachineInstr *MI) const {
  int Index;
  const MachineOperand *MO;

  switch(MI->getOpcode()) {
  case RISCV::ADDI:
    MO = &MI->getOperand(2);
    if(MI->getOperand(1).isFI()) {
      Index = MI->getOperand(1).getIndex();
      assert(MI->getOperand(2).isImm() && MI->getOperand(2).getImm() == 0);
      return new MachineStackObject(Index, false, MI, true);
    } else if (TargetValues::isSymbolValue(MO))
      return new MachineSymbolRef(*MO, false, MI);
    break;
  default:
    LLVM_DEBUG(dbgs() << "Unhandled ADD machine instruction");
    break;
  }
  return nullptr;
}

MachineLiveVal *
RISCVValues::genLoadRegValue(const MachineInstr *MI) const {
  switch(MI->getOpcode()) {
  case RISCV::LWU:
    if(MI->getOperand(2).isCPI()) {
      int Idx = MI->getOperand(2).getIndex();
      const MachineFunction *MF = MI->getParent()->getParent();
      const MachineConstantPool *MCP = MF->getConstantPool();
      const std::vector<MachineConstantPoolEntry> &CP = MCP->getConstants();
      if(CP[Idx].isMachineConstantPoolEntry()) {
        // TODO unhandled for now
      }
      else {
        const Constant *Val = CP[Idx].Val.ConstVal;
        if(isa<ConstantFP>(Val)) {
          const ConstantFP *FPVal = cast<ConstantFP>(Val);
          const APFloat &Flt = FPVal->getValueAPF();
          switch(APFloat::getSizeInBits(Flt.getSemantics())) {
          case 32: {
            IntFloat32 I2F = { Flt.convertToFloat() };
            return new MachineImmediate(4, I2F.i, MI, false);
          }
          case 64: {
            IntFloat64 I2D = { Flt.convertToDouble() };
            return new MachineImmediate(8, I2D.i, MI, false);
          }
          default: break;
          }
        }
      }
    }
    break;
  case RISCV::LD:
    // Note: if this is of the form %vreg, <ga:...>, then the compiler has
    // emitted multiple instructions in order to form the full address.  We,
    // however, don't have the instruction encoding limitations.
    // TODO verify this note above is true, maybe using MO::getTargetFlags?
    // Note 2: we *must* ensure the symbol is const-qualified, otherwise we
    // risk creating a new value if the symbol's value changes between when the
    // initial load would have occurred and the transformation, e.g.,
    //
    //   ldr x20, <ga:mysym>
    //   ... (somebody changes mysym's value) ...
    //   bl <ga:myfunc>
    //
    // In this situation, the transformation occurs at the call site and
    // retrieves the updated value rather than the value that would have been
    // loaded at the ldr instruction.
    if(TargetValues::isSymbolValue(MI->getOperand(2)) &&
       TargetValues::isSymbolValueConstant(MI->getOperand(2)))
      return new MachineSymbolRef(MI->getOperand(2), true, MI);
    break;
  default: break;
  }
  return nullptr;
}

MachineLiveValPtr RISCVValues::getMachineValue(const MachineInstr *MI) const {
  IntFloat64 Conv64;
  MachineLiveVal* Val = nullptr;
  const MachineOperand *MO;
  const TargetInstrInfo *TII;

  switch(MI->getOpcode()) {
  case RISCV::ADDI:
    Val = genADDInstructions(MI);
    break;
  case RISCV::PseudoLLA:
  case RISCV::PseudoLA:
  case RISCV::PseudoLA_TLS_IE:
  case RISCV::PseudoLA_TLS_GD:
    break;
  case RISCV::COPY:
    MO = &MI->getOperand(1);
    if(MO->isReg() && MO->getReg() == RISCV::X1) Val = new ReturnAddress(MI);
    break;
  case RISCV::LWU: // load 32-bit value, zero-extend
  case RISCV::LD: // load 64-bit value
    Val = genLoadRegValue(MI);
    break;
  default:
    TII =  MI->getParent()->getParent()->getSubtarget().getInstrInfo();
    LLVM_DEBUG(dbgs() << "Unhandled opcode: "
                 << TII->getName(MI->getOpcode()) << "\n");
    break;
  }

  return MachineLiveValPtr(Val);
}
