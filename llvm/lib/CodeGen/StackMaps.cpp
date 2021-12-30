//===- StackMaps.cpp ------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "llvm/CodeGen/StackMaps.h"
#include "llvm/ADT/DenseMapInfo.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/Twine.h"
#include "llvm/CodeGen/AsmPrinter.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineOperand.h"
#include "llvm/CodeGen/TargetFrameLowering.h"
#include "llvm/CodeGen/TargetOpcodes.h"
#include "llvm/CodeGen/TargetRegisterInfo.h"
#include "llvm/CodeGen/TargetSubtargetInfo.h"
#include "llvm/CodeGen/UnwindInfo.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/DiagnosticInfo.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCObjectFileInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/MCSymbol.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/MathExtras.h"
#include "llvm/Support/raw_ostream.h"
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <iterator>
#include <utility>

using namespace llvm;

#define DEBUG_TYPE "stackmaps"

#define TYPE_AND_FLAGS(type, ptr, alloca, dup, temp) \
  ((uint8_t)type) << 4 | ((uint8_t)ptr) << 3 | \
  ((uint8_t)alloca) << 2 | ((uint8_t)dup << 1) | \
  ((uint8_t)temp)

#define ARCH_TYPE_AND_FLAGS(type, ptr) ((uint8_t)type) << 4 | ((uint8_t)ptr)

#define ARCH_OP_TYPE(inst, gen, op) \
  ((uint8_t)inst) << 4 | ((uint8_t)gen) << 3 | (uint8_t)op

static cl::opt<int> StackMapVersion(
    "stackmap-version", cl::init(3), cl::Hidden,
    cl::desc("Specify the stackmap encoding version (default = 3)"));

const char *StackMaps::WSMP = "Stack Maps: ";

StackMapOpers::StackMapOpers(const MachineInstr *MI)
  : MI(MI) {
  assert(getVarIdx() <= MI->getNumOperands() &&
         "invalid stackmap definition");
}

PatchPointOpers::PatchPointOpers(const MachineInstr *MI)
    : MI(MI), HasDef(MI->getOperand(0).isReg() && MI->getOperand(0).isDef() &&
                     !MI->getOperand(0).isImplicit()) {
#ifndef NDEBUG
  unsigned CheckStartIdx = 0, e = MI->getNumOperands();
  while (CheckStartIdx < e && MI->getOperand(CheckStartIdx).isReg() &&
         MI->getOperand(CheckStartIdx).isDef() &&
         !MI->getOperand(CheckStartIdx).isImplicit())
    ++CheckStartIdx;

  assert(getMetaIdx() == CheckStartIdx &&
         "Unexpected additional definition in Patchpoint intrinsic.");
#endif
}

unsigned PatchPointOpers::getNextScratchIdx(unsigned StartIdx) const {
  if (!StartIdx)
    StartIdx = getVarIdx();

  // Find the next scratch register (implicit def and early clobber)
  unsigned ScratchIdx = StartIdx, e = MI->getNumOperands();
  while (ScratchIdx < e &&
         !(MI->getOperand(ScratchIdx).isReg() &&
           MI->getOperand(ScratchIdx).isDef() &&
           MI->getOperand(ScratchIdx).isImplicit() &&
           MI->getOperand(ScratchIdx).isEarlyClobber()))
    ++ScratchIdx;

  assert(ScratchIdx != e && "No scratch register available");
  return ScratchIdx;
}

StackMaps::StackMaps(AsmPrinter &AP) : AP(AP) {
  if (StackMapVersion != 3)
    llvm_unreachable("Unsupported stackmap version!");
}

/// Go up the super-register chain until we hit a valid dwarf register number.
static unsigned getDwarfRegNum(unsigned Reg, const TargetRegisterInfo *TRI) {
  int RegNum = TRI->getDwarfRegNum(Reg, false);
  for (MCSuperRegIterator SR(Reg, TRI); SR.isValid() && RegNum < 0; ++SR)
    RegNum = TRI->getDwarfRegNum(*SR, false);

  assert(RegNum >= 0 && "Invalid Dwarf register number.");
  return (unsigned)RegNum;
}

/// If the instruction is simply casting a pointer to another type, return
/// the value used as the source of the cast.  This is required because allocas
/// may be cast to different pointer types (which appear as non-allocas) but
/// may still be represented by a direct memory reference in the stackmap.
static inline const Value *getPointerCastSrc(const Value *Inst) {
  assert(Inst->getType()->isPointerTy() && "Not a pointer type");

  if(isa<BitCastInst>(Inst)) {
    // Ensure that we're casting to another pointer type
    const BitCastInst *BC = cast<BitCastInst>(Inst);
    if(!BC->getSrcTy()->isPointerTy()) return nullptr;
    else return BC->getOperand(0);
  }
  else if(isa<GetElementPtrInst>(Inst)) {
    // Ensure that all indexes are 0, meaning we're only referencing the start
    // of the storage location
    const GetElementPtrInst *GEP = cast<GetElementPtrInst>(Inst);
    GetElementPtrInst::const_op_iterator it, e;
    for(it = GEP->idx_begin(), e = GEP->idx_end(); it != e; it++) {
      if(!isa<ConstantInt>(it->get())) return nullptr;
      const ConstantInt *Idx = cast<ConstantInt>(it->get());
      if(!Idx->isZero()) return nullptr;
    }
    return GEP->getOperand(0);
  }
  else return nullptr;
}

/// Get pointer typing information for a stackmap operand
void StackMaps::getPointerInfo(const Value *Op, const DataLayout &DL,
                               bool &isPtr, bool &isAlloca,
                               unsigned &AllocaSize) const {
  isPtr = false;
  isAlloca = false;
  AllocaSize = 0;
  const PtrToIntInst *PTII;

  if((PTII = dyn_cast<PtrToIntInst>(Op))) Op = PTII->getPointerOperand();

  assert(Op != nullptr && "Invalid stackmap operand");
  Type *Ty = Op->getType();
  if(Ty->isPointerTy())
  {
    // Walk through cast operations that potentially hide allocas
    while(!isa<AllocaInst>(Op) && (Op = getPointerCastSrc(Op)));
    if(Op && isa<AllocaInst>(Op)) {
      PointerType *PTy = cast<PointerType>(Ty);
      assert(PTy->getElementType()->isSized() && "Alloca of unknown size?");
      isPtr = PTy->getElementType()->isPointerTy();
      isAlloca = true;
      AllocaSize = DL.getTypeAllocSize(PTy->getElementType());
    }
    else isPtr = true;
  }
}

/// Get stackmap information for register location
void StackMaps::getRegLocation(unsigned Phys,
                               unsigned &Dwarf,
                               unsigned &Offset) const {
  const TargetRegisterInfo *TRI = AP.MF->getSubtarget().getRegisterInfo();
  assert(!TRI->isVirtualRegister(Phys) &&
         "Virtual registers should have been rewritten by now");
  Offset = 0;
  Dwarf = getDwarfRegNum(Phys, TRI);
  unsigned LLVMRegNum = TRI->getLLVMRegNum(Dwarf, false);
  unsigned SubRegIdx = TRI->getSubRegIndex(LLVMRegNum, Phys);
  if(SubRegIdx)
    Offset = TRI->getSubRegIdxOffset(SubRegIdx);
}

/// Add duplicate target-specific locations for a stackmap operand
void StackMaps::addDuplicateLocs(const CallInst *StackMap, const Value *Oper,
                                 LocationVec &Locs, unsigned Size, bool Ptr,
                                 bool Alloca, unsigned AllocaSize) const {
  unsigned DwarfRegNum, Offset;
  int FrameOff;

  if(AP.MF->hasSMOpLocations(StackMap, Oper)) {
    const MachineLiveLocs &Dups = AP.MF->getSMOpLocations(StackMap, Oper);
    const TargetRegisterInfo *TRI = AP.MF->getSubtarget().getRegisterInfo();

    for(const MachineLiveLocPtr &LL : Dups) {
      if(LL->isReg()) {
        const MachineLiveReg &MR = (const MachineLiveReg &)*LL;
        getRegLocation(MR.getReg(), DwarfRegNum, Offset);

        Locs.emplace_back(Location::Register, Size, DwarfRegNum, Offset,
                          Ptr, Alloca, true, false, AllocaSize);
      }
      else if(LL->isStackAddr()) {
        MachineLiveStackAddr &MLSA = (MachineLiveStackAddr &)*LL;
        FrameOff = MLSA.calcAndGetRegOffset(AP, DwarfRegNum);

        Locs.emplace_back(Location::Indirect, Size,
          getDwarfRegNum(DwarfRegNum, TRI),
          FrameOff, Ptr, Alloca, true, false, AllocaSize);
      }
      else llvm_unreachable("Unknown machine live location type");
    }
  }
}

MachineInstr::const_mop_iterator
StackMaps::parsePcnOperand(MachineInstr::const_mop_iterator MOI,
                        MachineInstr::const_mop_iterator MOE, LocationVec &Locs,
                        LiveOutVec &LiveOuts, User::const_op_iterator &Op) const {
  bool isPtr, isAlloca, isTemporary = false;
  unsigned AllocaSize;
  auto &DL = AP.MF->getDataLayout();
  const TargetRegisterInfo *TRI = AP.MF->getSubtarget().getRegisterInfo();
  const CallInst *IRSM = cast<CallInst>(Op->getUser());
  const Value *IROp = Op->get();
  int64_t TemporaryOffset = 0;
  getPointerInfo(IROp, DL, isPtr, isAlloca, AllocaSize);

  if (MOI->isImm()) {
    // Peel off temporary value metadata
    if (MOI->getImm() == StackMaps::TemporaryOp) {
      isTemporary = true;
      AllocaSize = (++MOI)->getImm();
      TemporaryOffset = (++MOI)->getImm();
      ++MOI;
    }
    switch (MOI->getImm()) {
    default:
      llvm_unreachable("Unrecognized operand type.");
    case StackMaps::DirectMemRefOp: {
      auto &DL = AP.MF->getDataLayout();

      assert((isAlloca || isTemporary) &&
             "Did not find alloca value for direct memory reference");
      unsigned Size = DL.getPointerSizeInBits();
      assert((Size % 8) == 0 && "Need pointer size in bytes.");
      Size /= 8;
      unsigned Reg = (++MOI)->getReg();
      int64_t Imm = (++MOI)->getImm() + TemporaryOffset;
      Locs.emplace_back(Location::Direct, Size, getDwarfRegNum(Reg, TRI), Imm,
                        isPtr, true, false, isTemporary, AllocaSize);
      break;
    }
    case StackMaps::IndirectMemRefOp: {
      int64_t Size = (++MOI)->getImm();
      assert(Size > 0 && "Need a valid size for indirect memory locations.");
      Size = DL.getTypeAllocSize(IROp->getType());
      unsigned Reg = (++MOI)->getReg();
      int64_t Imm = (++MOI)->getImm();
      // Note: getPointerInfo() may have found a suitable alloca for this
      // operand, but the backend didn't actually turn it into one.
      Locs.emplace_back(Location::Indirect, (unsigned)Size,
                        getDwarfRegNum(Reg, TRI), Imm, isPtr, false, false,
                        isTemporary, 0);
      break;
    }
    case StackMaps::ConstantOp: {
      ++MOI;
      assert(MOI->isImm() && "Expected constant operand.");
      int64_t Imm = MOI->getImm();
      // Note: getPointerInfo() may have found a suitable alloca for this
      // operand, but the backend didn't actually turn it into one.
      Locs.emplace_back(Location::Constant, sizeof(int64_t), 0, Imm,
                        isPtr, false, false, isTemporary, 0);
      break;
    }
    }
    // Note: we shouldn't have alternate locations -- constants aren't stored
    // anywhere, and stack slots should be either allocas (which shouldn't have
    // alternate locations) or register spill locations (handled below in the
    // register path)
    assert(!AP.MF->hasSMOpLocations(IRSM, IROp) &&
           "Unhandled duplicate locations");
    ++Op;
    return ++MOI;
  }

  // The physical register number will ultimately be encoded as a DWARF regno.
  // The stack map also records the size of a spill slot that can hold the
  // register content, accurate to the actual size of the data type.
  if (MOI->isReg()) {
    // Skip implicit registers (this includes our scratch registers)
    if (MOI->isImplicit())
      return ++MOI;

    assert(TargetRegisterInfo::isPhysicalRegister(MOI->getReg()) &&
           "Virtreg operands should have been rewritten before now.");
    assert(!MOI->getSubReg() && "Physical subreg still around.");

    size_t ValSize = DL.getTypeAllocSize(IROp->getType());
    unsigned Offset, DwarfRegNum;
    getRegLocation(MOI->getReg(), DwarfRegNum, Offset);

    // Note: getPointerInfo() may have found a suitable alloca for this
    // operand, but the backend didn't actually turn it into one.
    Locs.emplace_back(Location::Register, ValSize, DwarfRegNum, Offset,
                      isPtr, false, false, isTemporary, 0);
    addDuplicateLocs(IRSM, IROp, Locs, ValSize, isPtr, false, 0);
    ++Op;
    return ++MOI;
  }

  if (MOI->isRegLiveOut())
    LiveOuts = parseRegisterLiveOutMask(MOI->getRegLiveOut());

  return ++MOI;
}

MachineInstr::const_mop_iterator
StackMaps::parseOperand(MachineInstr::const_mop_iterator MOI,
                        MachineInstr::const_mop_iterator MOE, LocationVec &Locs,
                        LiveOutVec &LiveOuts) const {
  const TargetRegisterInfo *TRI = AP.MF->getSubtarget().getRegisterInfo();
  if (MOI->isImm()) {
    switch (MOI->getImm()) {
    default:
      llvm_unreachable("Unrecognized operand type.");
    case StackMaps::DirectMemRefOp: {
      auto &DL = AP.MF->getDataLayout();

      unsigned Size = DL.getPointerSizeInBits();
      assert((Size % 8) == 0 && "Need pointer size in bytes.");
      Size /= 8;
      unsigned Reg = (++MOI)->getReg();
      int64_t Imm = (++MOI)->getImm();
      Locs.emplace_back(StackMaps::Location::Direct, Size,
                        getDwarfRegNum(Reg, TRI), Imm);
      break;
    }
    case StackMaps::IndirectMemRefOp: {
      int64_t Size = (++MOI)->getImm();
      assert(Size > 0 && "Need a valid size for indirect memory locations.");
      unsigned Reg = (++MOI)->getReg();
      int64_t Imm = (++MOI)->getImm();
      Locs.emplace_back(StackMaps::Location::Indirect, Size,
                        getDwarfRegNum(Reg, TRI), Imm);
      break;
    }
    case StackMaps::ConstantOp: {
      ++MOI;
      assert(MOI->isImm() && "Expected constant operand.");
      int64_t Imm = MOI->getImm();
      Locs.emplace_back(Location::Constant, sizeof(int64_t), 0, Imm);
      break;
    }
    }
    return ++MOI;
  }

  // The physical register number will ultimately be encoded as a DWARF regno.
  // The stack map also records the size of a spill slot that can hold the
  // register content. (The runtime can track the actual size of the data type
  // if it needs to.)
  if (MOI->isReg()) {
    // Skip implicit registers (this includes our scratch registers)
    if (MOI->isImplicit())
      return ++MOI;

    assert(TargetRegisterInfo::isPhysicalRegister(MOI->getReg()) &&
           "Virtreg operands should have been rewritten before now.");
    const TargetRegisterClass *RC = TRI->getMinimalPhysRegClass(MOI->getReg());
    assert(!MOI->getSubReg() && "Physical subreg still around.");

    unsigned Offset = 0;
    unsigned DwarfRegNum = getDwarfRegNum(MOI->getReg(), TRI);
    unsigned LLVMRegNum = TRI->getLLVMRegNum(DwarfRegNum, false);
    unsigned SubRegIdx = TRI->getSubRegIndex(LLVMRegNum, MOI->getReg());
    if (SubRegIdx)
      Offset = TRI->getSubRegIdxOffset(SubRegIdx);

    Locs.emplace_back(Location::Register, TRI->getSpillSize(*RC),
                      DwarfRegNum, Offset);
    return ++MOI;
  }

  if (MOI->isRegLiveOut())
    LiveOuts = parseRegisterLiveOutMask(MOI->getRegLiveOut());

  return ++MOI;
}

void StackMaps::print(raw_ostream &OS) {
  const TargetRegisterInfo *TRI =
      AP.MF ? AP.MF->getSubtarget().getRegisterInfo() : nullptr;
  OS << WSMP << "callsites:\n";
  for (const auto &CSI : CSInfos) {
    const LocationVec &CSLocs = CSI.Locations;
    const LiveOutVec &LiveOuts = CSI.LiveOuts;
    const ArchValues &Values = CSI.Vals;

    OS << WSMP << "callsite " << CSI.ID << "\n";
    OS << WSMP << "  has " << CSLocs.size() << " locations\n";

    unsigned Idx = 0;
    for (const auto &Loc : CSLocs) {
      OS << WSMP << "\t\tLoc " << Idx << ": ";
      switch (Loc.Type) {
      case Location::Unprocessed:
        OS << "<Unprocessed operand>";
        break;
      case Location::Register:
        OS << "Register ";
        if (TRI)
          OS << printReg(Loc.Reg, TRI);
        else
          OS << Loc.Reg;
        break;
      case Location::Direct:
        OS << "Direct ";
        if (TRI)
          OS << printReg(Loc.Reg, TRI);
        else
          OS << Loc.Reg;
        if (Loc.Offset)
          OS << " + " << Loc.Offset;
        break;
      case Location::Indirect:
        OS << "Indirect ";
        if (TRI)
          OS << printReg(Loc.Reg, TRI);
        else
          OS << Loc.Reg;
        OS << "+" << Loc.Offset;
        break;
      case Location::Constant:
        OS << "Constant " << Loc.Offset;
        break;
      case Location::ConstantIndex:
        OS << "Constant Index " << Loc.Offset;
        break;
      }
      OS << ", pointer? " << Loc.Ptr << ", alloca? " << Loc.Alloca
         << ", duplicate? " << Loc.Duplicate
         << ", temporary? " << Loc.Temporary;

      unsigned TypeAndFlags = TYPE_AND_FLAGS(Loc.Type, Loc.Ptr, Loc.Alloca,
                                             Loc.Duplicate, Loc.Temporary);

      OS << "\t[encoding: .byte " << TypeAndFlags << ", .byte 0"
         << ", .short " << Loc.Size << ", .short " << Loc.Reg << ", .short 0"
         << ", .int " << Loc.Offset
         << ", .uint " << Loc.AllocaSize << "]\n";
      Idx++;
    }

    OS << WSMP << "\thas " << LiveOuts.size() << " live-out registers\n";

    Idx = 0;
    for (const auto &LO : LiveOuts) {
      OS << WSMP << "\t\tLO " << Idx << ": ";
      if (TRI)
        OS << printReg(LO.Reg, TRI);
      else
        OS << LO.Reg;
      OS << "\t[encoding: .short " << LO.DwarfRegNum << ", .byte 0, .byte "
         << LO.Size << "]\n";
      Idx++;
    }

    OS << WSMP << "\thas " << Values.size() << " arch-specific live values\n";

    Idx = 0;
    for (const auto &V : Values) {
      const Location &Loc = V.first;
      const Operation &Op = V.second;

      OS << WSMP << "\t\tArch-Val " << Idx << ": ";
      switch(Loc.Type) {
      case Location::Register:
        OS << "Register ";
        if (TRI)
          OS << printReg(Loc.Reg, TRI);
        else
          OS << Loc.Reg;
        break;
      case Location::Indirect:
        OS << "Indirect ";
        if (TRI)
          OS << printReg(Loc.Reg, TRI);
        else
          OS << Loc.Reg;
        if (Loc.Offset)
          OS << " + " << Loc.Offset;
        break;
      default:
        OS << "<Unknown live value type>";
        break;
      }

      OS << ", " << ValueGenInst::getInstName(Op.InstType) << " ";
      switch(Op.OperandType) {
      case Location::Register:
        OS << "register ";
        if (TRI)
          OS << printReg(Op.DwarfReg, TRI);
        else
          OS << Op.DwarfReg;
        break;
      case Location::Direct:
        OS << "value stored at register ";
        if (TRI)
          OS << printReg(Op.DwarfReg, TRI);
        else
          OS << Op.DwarfReg;
        if (Op.Constant)
          OS << " + " << Op.Constant;
        break;
      case Location::Indirect:
        OS << "register";
        if (TRI)
          OS << printReg(Op.DwarfReg, TRI);
        else
          OS << Op.DwarfReg;
        if (Op.Constant)
          OS << " + " << Op.Constant;
        break;
      case Location::Constant:
        if(Op.isSymbol)
          OS << "address of " << Op.Symbol->getName();
        else {
          OS << "immediate ";
          OS.write_hex(Op.Constant);
        }
        break;
      default:
        OS << "<Unknown operand type>";
        break;
      }

      unsigned TypeAndFlags = ARCH_TYPE_AND_FLAGS(Loc.Type, Loc.Ptr);
      unsigned OpType = ARCH_OP_TYPE(Op.InstType,
                                     Op.isGenerated,
                                     Op.OperandType);
      OS << "\t[encoding: .byte " << TypeAndFlags << ", .byte " << Loc.Size
         << ", .short " << Loc.Reg << ", .int " << Loc.Offset
         << ", .byte " << OpType << ", .byte " << Op.Size << ", .short "
         << Op.DwarfReg << ", .int64 " << (Op.isSymbol ? 0 : Op.Constant)
         << "]\n";
    }
  }
}

/// Create a live-out register record for the given register Reg.
StackMaps::LiveOutReg
StackMaps::createLiveOutReg(unsigned Reg, const TargetRegisterInfo *TRI) const {
  unsigned DwarfRegNum = getDwarfRegNum(Reg, TRI);
  unsigned Size = TRI->getSpillSize(*TRI->getMinimalPhysRegClass(Reg));
  return LiveOutReg(Reg, DwarfRegNum, Size);
}

/// Parse the register live-out mask and return a vector of live-out registers
/// that need to be recorded in the stackmap.
StackMaps::LiveOutVec
StackMaps::parseRegisterLiveOutMask(const uint32_t *Mask) const {
  assert(Mask && "No register mask specified");
  const TargetRegisterInfo *TRI = AP.MF->getSubtarget().getRegisterInfo();
  LiveOutVec LiveOuts;

  // Create a LiveOutReg for each bit that is set in the register mask.
  for (unsigned Reg = 0, NumRegs = TRI->getNumRegs(); Reg != NumRegs; ++Reg)
    if ((Mask[Reg / 32] >> Reg % 32) & 1)
      LiveOuts.push_back(createLiveOutReg(Reg, TRI));

  // We don't need to keep track of a register if its super-register is already
  // in the list. Merge entries that refer to the same dwarf register and use
  // the maximum size that needs to be spilled.

  llvm::sort(LiveOuts, [](const LiveOutReg &LHS, const LiveOutReg &RHS) {
    // Only sort by the dwarf register number.
    return LHS.DwarfRegNum < RHS.DwarfRegNum;
  });

  for (auto I = LiveOuts.begin(), E = LiveOuts.end(); I != E; ++I) {
    for (auto II = std::next(I); II != E; ++II) {
      if (I->DwarfRegNum != II->DwarfRegNum) {
        // Skip all the now invalid entries.
        I = --II;
        break;
      }
      I->Size = std::max(I->Size, II->Size);
      if (TRI->isSuperRegister(I->Reg, II->Reg))
        I->Reg = II->Reg;
      II->Reg = 0; // mark for deletion.
    }
  }

  LiveOuts.erase(
      llvm::remove_if(LiveOuts,
                      [](const LiveOutReg &LO) { return LO.Reg == 0; }),
      LiveOuts.end());

  return LiveOuts;
}

/// Convert a list of instructions used to generate an architecture-specific
/// live value into multiple individual records.
void StackMaps::genArchValsFromInsts(ArchValues &AV,
                                     Location &Loc,
                                     const MachineLiveVal &MLV) {
  assert(MLV.isGenerated() && "Invalid live value type");

  unsigned PtrSize = AP.MF->getDataLayout().getPointerSizeInBits() / 8;
  const MachineGeneratedVal &MGV = (const MachineGeneratedVal &)MLV;
  const ValueGenInstList &I = MGV.getInstructions();
  const TargetRegisterInfo *TRI = AP.MF->getSubtarget().getRegisterInfo();
  const TargetRegisterClass *RC;
  Operation Op;
  Op.isGenerated = true;

  for(auto &Inst : I) {
    const RegInstructionBase *RI;
    const ImmInstructionBase *II;
    const RefInstruction *RefI;

    Op.DwarfReg = 0;
    Op.Constant = 0;
    Op.isSymbol = false;
    Op.Symbol = nullptr;

    Op.InstType = Inst->type();
    if (Inst->opType() != ValueGenInst::OpType::Register
	&& Inst->opType() != ValueGenInst::OpType::Immediate
	&& Inst->opType() != ValueGenInst::OpType::Reference)
      llvm_unreachable("Invalid operand type");

    switch(Inst->opType()) {
    case ValueGenInst::OpType::Register:
      RI = (const RegInstructionBase *)Inst.get();
      assert(TRI->isPhysicalRegister(RI->getReg()) &&
             "Virtual should have been converted to physical register");
      RC = TRI->getMinimalPhysRegClass(RI->getReg());
      Op.OperandType = Location::Register;
      Op.Size = TRI->getRegSizeInBits(*RC) / 8;
      Op.DwarfReg = getDwarfRegNum(RI->getReg(), TRI);
      break;
    case ValueGenInst::OpType::Immediate:
      II = (const ImmInstructionBase *)Inst.get();
      Op.OperandType = Location::Constant;
      Op.Size = II->getImmSize();
      Op.Constant = II->getImm();
      break;
    case ValueGenInst::OpType::Reference:
      RefI = (const RefInstruction *)Inst.get();
      Op.OperandType = Location::Constant;
      Op.Size = PtrSize;
      Op.isSymbol = true;
      Op.Symbol = RefI->getReference(AP);
      break;
    }
    AV.emplace_back(ArchValue(Loc, Op));
  }
}

/// Add architecture-specific locations for the stackmap
void StackMaps::addArchLiveVals(const CallInst *SM, ArchValues &AV) {
  unsigned Offset, DwarfReg;
  unsigned PtrSize = AP.MF->getDataLayout().getPointerSizeInBits() / 8;
  const MachineFrameInfo *MFI = &AP.MF->getFrameInfo();
  const TargetRegisterInfo *TRI = AP.MF->getSubtarget().getRegisterInfo();

  if(AP.MF->hasSMArchSpecificLocations(SM)) {
    const ArchLiveValues &Vals = AP.MF->getSMArchSpecificLocations(SM);

    for(auto &Val : Vals) {
      Location Loc;
      Operation Op;

      Loc.Ptr = Val.second->isPtr();
      Loc.Alloca = false;
      Loc.Duplicate = false;
      Loc.AllocaSize = 0;

      // Parse the location
      if(Val.first->isReg()) {
        const MachineLiveReg &MR = (const MachineLiveReg &)*Val.first;
        const TargetRegisterClass *RC =
          TRI->getMinimalPhysRegClass(MR.getReg());
        getRegLocation(MR.getReg(), DwarfReg, Offset);

        Loc.Type = Location::Register;
        Loc.Size = TRI->getRegSizeInBits(*RC) / 8;
        Loc.Reg = DwarfReg;
        Loc.Offset = Offset;
      }
      else if(Val.first->isStackAddr()) {
        MachineLiveStackAddr &MLSA = (MachineLiveStackAddr &)*Val.first;

        Loc.Type = Location::Indirect;
        Loc.Size = MLSA.getSize(AP);
        Loc.Offset = MLSA.calcAndGetRegOffset(AP, DwarfReg);
        Loc.Reg = getDwarfRegNum(DwarfReg, TRI);
      }
      else llvm_unreachable("Invalid architecture-specific live value");

      // Parse the operation
      Op.InstType = ValueGenInst::Set;
      Op.isGenerated = false;
      if(Val.second->isReference()) {
        const MachineReference &MR = (const MachineReference &)*Val.second;
        if(MR.isLoad()) Op.InstType = ValueGenInst::Load64;
        Op.OperandType = Location::Constant;
        Op.Size = PtrSize;
        Op.isSymbol = true;
        Op.Symbol = MR.getReference(AP);
        AV.emplace_back(ArchValue(Loc, Op));
      }
      else if(Val.second->isStackObject()) {
        const MachineStackObject &MSO = (const MachineStackObject &)*Val.second;
        if(MSO.isLoad()) { // Loading a value from a stack slot
          Op.OperandType = Location::Direct;
          if(MSO.isCommonObject()) Op.Size = PtrSize;
          else Op.Size = MFI->getObjectSize(MSO.getIndex());
        }
        else { // Generating a reference to a stack slot
          Op.OperandType = Location::Indirect;
          Op.Size = PtrSize;
        }
        Op.Constant = MSO.getOffsetFromReg(AP, DwarfReg);
        Op.DwarfReg = getDwarfRegNum(DwarfReg, TRI);
        Op.isSymbol = false;
        AV.emplace_back(ArchValue(Loc, Op));
      }
      else if(Val.second->isImm()) {
        const MachineImmediate &MI = (const MachineImmediate &)*Val.second;
        Op.OperandType = Location::Constant;
        Op.Size = MI.getSize();
        Op.Constant = MI.getValue();
        Op.isSymbol = false;
        AV.emplace_back(ArchValue(Loc, Op));
      }
      else if(Val.second->isGenerated())
        genArchValsFromInsts(AV, Loc, *Val.second);
      else llvm_unreachable("Invalid architecture-specific live value");
    }
  }
}

void StackMaps::recordPcnStackMapOpers(const MachineInstr &MI, uint64_t ID,
                                    MachineInstr::const_mop_iterator MOI,
                                    MachineInstr::const_mop_iterator MOE,
                                    bool recordResult) {
  MCContext &OutContext = AP.OutStreamer->getContext();
  MCSymbol *MILabel = OutContext.createTempSymbol();
  AP.OutStreamer->EmitLabel(MILabel);
  User::const_op_iterator Op = nullptr;

  LocationVec Locations;
  LiveOutVec LiveOuts;
  ArchValues Constants;

  if (recordResult) {
    assert(PatchPointOpers(&MI).hasDef() && "Stackmap has no return value.");
    parsePcnOperand(MI.operands_begin(), std::next(MI.operands_begin()),
		    Locations, LiveOuts, Op);
  }

  // Find the IR stackmap instruction which corresponds to MI so we can emit
  // type information along with the value's location
  const BasicBlock *BB = MI.getParent()->getBasicBlock();
  const IntrinsicInst *IRSM = nullptr;
  const std::string SMName("llvm.experimental.pcn.stackmap");
  for(auto BBI = BB->begin(), BBE = BB->end(); BBI != BBE; BBI++)
  {
    const IntrinsicInst *II;
    if((II = dyn_cast<IntrinsicInst>(&*BBI)) &&
       II->getCalledFunction()->getName() == SMName &&
       cast<ConstantInt>(II->getArgOperand(0))->getZExtValue() == ID)
    {
      IRSM = cast<IntrinsicInst>(&*BBI);
      break;
    }
  }
  assert(IRSM && "Could not find associated stackmap instruction");

  // Parse operands.
  unsigned NumRepeat;
  Op = std::next(IRSM->op_begin(), 2);
  MachineInstr::const_mop_iterator MFirst = MI.operands_begin();
  while (MOI != MOE) {
    MOI = parsePcnOperand(MOI, MOE, Locations, LiveOuts, Op);
    NumRepeat = AP.MF->getNumLegalizedOps(ID, MOI - MFirst) - 1;
    for(size_t i = 0; i < NumRepeat; i++) {
      MOI = parsePcnOperand(MOI, MOE, Locations, LiveOuts, Op);
      --Op;
    }
  }
  assert(Op == (IRSM->op_end() - 1) && "did not lower all stackmap operands");

  // Add architecture-specific live values
  addArchLiveVals(IRSM, Constants);

  // Move large constants into the constant pool.
  for (auto &Loc : Locations) {
    // Constants are encoded as sign-extended integers.
    // -1 is directly encoded as .long 0xFFFFFFFF with no constant pool.
    if (Loc.Type == Location::Constant && !isInt<32>(Loc.Offset)) {
      Loc.Type = Location::ConstantIndex;
      // ConstPool is intentionally a MapVector of 'uint64_t's (as
      // opposed to 'int64_t's).  We should never be in a situation
      // where we have to insert either the tombstone or the empty
      // keys into a map, and for a DenseMap<uint64_t, T> these are
      // (uint64_t)0 and (uint64_t)-1.  They can be and are
      // represented using 32 bit integers.
      assert((uint64_t)Loc.Offset != DenseMapInfo<uint64_t>::getEmptyKey() &&
             (uint64_t)Loc.Offset !=
                 DenseMapInfo<uint64_t>::getTombstoneKey() &&
             "empty and tombstone keys should fit in 32 bits!");
      auto Result = ConstPool.insert(std::make_pair(Loc.Offset, Loc.Offset));
      Loc.Offset = Result.first - ConstPool.begin();
    }
  }

  // Create an expression to calculate the offset of the callsite from function
  // entry.
  // TODO for Popcorn, we actually want the return address of the call
  // instruction to which this stackmap is attached.  However some backend
  // writers, in their infinite wisdom, decided to abstract multiple assembly
  // instructions into a single machine IR instruction (*ahem* PowerPC *ahem*).
  // Generate an expression to correct for this "feature".
  int RAOffset = AP.getCanonicalReturnAddr(MI.getPrevNode());
  const MCExpr *RAFixup = MCBinaryExpr::createSub(
      MCSymbolRefExpr::create(MILabel, OutContext),
      MCConstantExpr::create(RAOffset, OutContext), OutContext);
  const MCExpr *CSOffsetExpr = MCBinaryExpr::createSub(RAFixup,
      MCSymbolRefExpr::create(AP.CurrentFnSymForSize, OutContext), OutContext);

  CSInfos.emplace_back(AP.CurrentFnSym, CSOffsetExpr, ID,
                       std::move(Locations), std::move(LiveOuts),
                       std::move(Constants));

  // Record the stack size of the current function and update callsite count.
  const MachineFrameInfo &MFI = AP.MF->getFrameInfo();
  const TargetRegisterInfo *RegInfo = AP.MF->getSubtarget().getRegisterInfo();
  bool HasDynamicFrameSize =
      MFI.hasVarSizedObjects() || RegInfo->needsStackRealignment(*(AP.MF));
  uint64_t FrameSize = HasDynamicFrameSize ? UINT64_MAX : MFI.getStackSize();

  auto CurrentIt = FnInfos.find(AP.CurrentFnSym);
  if (CurrentIt != FnInfos.end())
    CurrentIt->second.RecordCount++;
  else
    FnInfos.insert(std::make_pair(AP.CurrentFnSym, FunctionInfo(FrameSize)));
}

void StackMaps::recordStackMapOpers(const MachineInstr &MI, uint64_t ID,
                                    MachineInstr::const_mop_iterator MOI,
                                    MachineInstr::const_mop_iterator MOE,
                                    bool recordResult) {
  MCContext &OutContext = AP.OutStreamer->getContext();
  MCSymbol *MILabel = OutContext.createTempSymbol();
  AP.OutStreamer->EmitLabel(MILabel);

  LocationVec Locations;
  LiveOutVec LiveOuts;

  if (recordResult) {
    assert(PatchPointOpers(&MI).hasDef() && "Stackmap has no return value.");
    parseOperand(MI.operands_begin(), std::next(MI.operands_begin()), Locations,
                 LiveOuts);
  }

  // Parse operands.
  while (MOI != MOE) {
    MOI = parseOperand(MOI, MOE, Locations, LiveOuts);
  }

  // Move large constants into the constant pool.
  for (auto &Loc : Locations) {
    // Constants are encoded as sign-extended integers.
    // -1 is directly encoded as .long 0xFFFFFFFF with no constant pool.
    if (Loc.Type == Location::Constant && !isInt<32>(Loc.Offset)) {
      Loc.Type = Location::ConstantIndex;
      // ConstPool is intentionally a MapVector of 'uint64_t's (as
      // opposed to 'int64_t's).  We should never be in a situation
      // where we have to insert either the tombstone or the empty
      // keys into a map, and for a DenseMap<uint64_t, T> these are
      // (uint64_t)0 and (uint64_t)-1.  They can be and are
      // represented using 32 bit integers.
      assert((uint64_t)Loc.Offset != DenseMapInfo<uint64_t>::getEmptyKey() &&
             (uint64_t)Loc.Offset !=
                 DenseMapInfo<uint64_t>::getTombstoneKey() &&
             "empty and tombstone keys should fit in 32 bits!");
      auto Result = ConstPool.insert(std::make_pair(Loc.Offset, Loc.Offset));
      Loc.Offset = Result.first - ConstPool.begin();
    }
  }

  // Create an expression to calculate the offset of the callsite from function
  // entry.
  const MCExpr *CSOffsetExpr = MCBinaryExpr::createSub(
      MCSymbolRefExpr::create(MILabel, OutContext),
      MCSymbolRefExpr::create(AP.CurrentFnSymForSize, OutContext), OutContext);

  CSInfos.emplace_back(CSOffsetExpr, ID, std::move(Locations),
                       std::move(LiveOuts));

  // Record the stack size of the current function and update callsite count.
  const MachineFrameInfo &MFI = AP.MF->getFrameInfo();
  const TargetRegisterInfo *RegInfo = AP.MF->getSubtarget().getRegisterInfo();
  bool HasDynamicFrameSize =
      MFI.hasVarSizedObjects() || RegInfo->needsStackRealignment(*(AP.MF));
  uint64_t FrameSize = HasDynamicFrameSize ? UINT64_MAX : MFI.getStackSize();

  auto CurrentIt = FnInfos.find(AP.CurrentFnSym);
  if (CurrentIt != FnInfos.end())
    CurrentIt->second.RecordCount++;
  else
    FnInfos.insert(std::make_pair(AP.CurrentFnSym, FunctionInfo(FrameSize)));
}

void StackMaps::recordStackMap(const MachineInstr &MI) {
  assert(MI.getOpcode() == TargetOpcode::STACKMAP && "expected stackmap");

  StackMapOpers opers(&MI);
  const int64_t ID = MI.getOperand(PatchPointOpers::IDPos).getImm();
  recordStackMapOpers(MI, ID, std::next(MI.operands_begin(), opers.getVarIdx()),
                      MI.operands_end());
}

void StackMaps::recordPcnStackMap(const MachineInstr &MI) {
  assert(MI.getOpcode() == TargetOpcode::PCN_STACKMAP
	 && "expected pcn_stackmap");

  StackMapOpers opers(&MI);
  const int64_t ID = MI.getOperand(PatchPointOpers::IDPos).getImm();
  recordPcnStackMapOpers(MI, ID,
			 std::next(MI.operands_begin(), opers.getVarIdx()),
			 MI.operands_end());
}

void StackMaps::recordPatchPoint(const MachineInstr &MI) {
  assert(MI.getOpcode() == TargetOpcode::PATCHPOINT && "expected patchpoint");

  PatchPointOpers opers(&MI);
  const int64_t ID = opers.getID();
  auto MOI = std::next(MI.operands_begin(), opers.getStackMapStartIdx());
  recordStackMapOpers(MI, ID, MOI, MI.operands_end(),
                      opers.isAnyReg() && opers.hasDef());

#ifndef NDEBUG
  // verify anyregcc
  auto &Locations = CSInfos.back().Locations;
  if (opers.isAnyReg()) {
    unsigned NArgs = opers.getNumCallArgs();
    for (unsigned i = 0, e = (opers.hasDef() ? NArgs + 1 : NArgs); i != e; ++i)
      assert(Locations[i].Type == Location::Register &&
             "anyreg arg must be in reg.");
  }
#endif
}

void StackMaps::recordStatepoint(const MachineInstr &MI) {
  assert(MI.getOpcode() == TargetOpcode::STATEPOINT && "expected statepoint");

  StatepointOpers opers(&MI);
  // Record all the deopt and gc operands (they're contiguous and run from the
  // initial index to the end of the operand list)
  const unsigned StartIdx = opers.getVarIdx();
  recordStackMapOpers(MI, opers.getID(), MI.operands_begin() + StartIdx,
                      MI.operands_end(), false);
}

/// Emit the stackmap header.
///
/// Header {
///   uint8  : Stack Map Version (currently 2)
///   uint8  : Reserved (expected to be 0)
///   uint16 : Reserved (expected to be 0)
/// }
/// uint32 : NumFunctions
/// uint32 : NumConstants
/// uint32 : NumRecords
void StackMaps::emitStackmapHeader(MCStreamer &OS) {
  // Header.
  OS.EmitIntValue(StackMapVersion, 1); // Version.
  OS.EmitIntValue(0, 1);               // Reserved.
  OS.EmitIntValue(0, 2);               // Reserved.

  // Num functions.
  LLVM_DEBUG(dbgs() << WSMP << "#functions = " << FnInfos.size() << '\n');
  OS.EmitIntValue(FnInfos.size(), 4);
  // Num constants.
  LLVM_DEBUG(dbgs() << WSMP << "#constants = " << ConstPool.size() << '\n');
  OS.EmitIntValue(ConstPool.size(), 4);
  // Num callsites.
  LLVM_DEBUG(dbgs() << WSMP << "#callsites = " << CSInfos.size() << '\n');
  OS.EmitIntValue(CSInfos.size(), 4);
}

/// Emit the function frame record for each function.
///
/// StkSizeRecord[NumFunctions] {
///   uint64 : Function Address
///   uint64 : Stack Size
///   uint64 : Record Count
/// }
void StackMaps::emitFunctionFrameRecords(MCStreamer &OS) {
  // Function Frame records.
  LLVM_DEBUG(dbgs() << WSMP << "functions:\n");
  for (auto const &FR : FnInfos) {
    LLVM_DEBUG(dbgs() << WSMP << "function addr: " << FR.first
                      << " frame size: " << FR.second.StackSize
                      << " callsite count: " << FR.second.RecordCount << '\n');
    OS.EmitSymbolValue(FR.first, 8);
    OS.EmitIntValue(FR.second.StackSize, 8);
    OS.EmitIntValue(FR.second.RecordCount, 8);
  }
}

/// Emit the function frame record for each function.
///
/// StkSizeRecord[NumFunctions] {
///   uint64 : Function Address
///   uint64 : Stack Size
///   uint64 : Record Count
///   uint32 : Number of Unwinding Entries
///   uint32 : Offset into Unwinding Section
/// }
void StackMaps::emitPcnFunctionFrameRecords(MCStreamer &OS,
                                         const UnwindInfo *UI) {
  // Function Frame records.
  LLVM_DEBUG(dbgs() << WSMP << "functions:\n");
  for (auto const &FR : FnInfos) {
    LLVM_DEBUG(dbgs() << WSMP << "function addr: " << FR.first
                      << " frame size: " << FR.second.StackSize
                      << " callsite count: " << FR.second.RecordCount << '\n');
    OS.EmitSymbolValue(FR.first, 8);
    OS.EmitIntValue(FR.second.StackSize, 8);
    OS.EmitIntValue(FR.second.RecordCount, 8);

    if(UI) {
      const UnwindInfo::FuncUnwindInfo &FUI = UI->getUnwindInfo(FR.first);
      LLVM_DEBUG(dbgs() << " unwind info start: " << FUI.SecOffset
                   << " (" << FUI.NumUnwindRecord << " entries)\n");
      OS.EmitIntValue(FUI.NumUnwindRecord, 4);
      OS.EmitIntValue(FUI.SecOffset, 4);
    }
    else OS.EmitIntValue(0, 8);
  }
}

/// Emit the constant pool.
///
/// int64  : Constants[NumConstants]
void StackMaps::emitConstantPoolEntries(MCStreamer &OS) {
  // Constant pool entries.
  LLVM_DEBUG(dbgs() << WSMP << "constants:\n");
  for (const auto &ConstEntry : ConstPool) {
    LLVM_DEBUG(dbgs() << WSMP << ConstEntry.second << '\n');
    OS.EmitIntValue(ConstEntry.second, 8);
  }
}

/// Emit the callsite info for each callsite.
///
/// StkMapRecord[NumRecords] {
///   uint64 : PatchPoint ID
///   uint32 : Instruction Offset
///   uint16 : Reserved (record flags)
///   uint16 : NumLocations
///   Location[NumLocations] {
///     uint8  : Register | Direct | Indirect | Constant | ConstantIndex
///     uint8  : Size in Bytes
///     uint16 : Dwarf RegNum
///     int32  : Offset
///   }
///   uint16 : Padding
///   uint16 : NumLiveOuts
///   LiveOuts[NumLiveOuts] {
///     uint16 : Dwarf RegNum
///     uint8  : Reserved
///     uint8  : Size in Bytes
///   }
///   uint32 : Padding (only if required to align to 8 byte)
/// }
///
/// Location Encoding, Type, Value:
///   0x1, Register, Reg                 (value in register)
///   0x2, Direct, Reg + Offset          (frame index)
///   0x3, Indirect, [Reg + Offset]      (spilled value)
///   0x4, Constant, Offset              (small constant)
///   0x5, ConstIndex, Constants[Offset] (large constant)
void StackMaps::emitCallsiteEntries(MCStreamer &OS) {
  LLVM_DEBUG(print(dbgs()));
  // Callsite entries.
  for (const auto &CSI : CSInfos) {
    const LocationVec &CSLocs = CSI.Locations;
    const LiveOutVec &LiveOuts = CSI.LiveOuts;

    // Verify stack map entry. It's better to communicate a problem to the
    // runtime than crash in case of in-process compilation. Currently, we do
    // simple overflow checks, but we may eventually communicate other
    // compilation errors this way.
    if (CSLocs.size() > UINT16_MAX || LiveOuts.size() > UINT16_MAX) {
      OS.EmitIntValue(UINT64_MAX, 8); // Invalid ID.
      OS.EmitValue(CSI.CSOffsetExpr, 4);
      OS.EmitIntValue(0, 2); // Reserved.
      OS.EmitIntValue(0, 2); // 0 locations.
      OS.EmitIntValue(0, 2); // padding.
      OS.EmitIntValue(0, 2); // 0 live-out registers.
      OS.EmitIntValue(0, 4); // padding.
      continue;
    }

    OS.EmitIntValue(CSI.ID, 8);
    OS.EmitValue(CSI.CSOffsetExpr, 4);

    // Reserved for flags.
    OS.EmitIntValue(0, 2);
    OS.EmitIntValue(CSLocs.size(), 2);

    for (const auto &Loc : CSLocs) {
      OS.EmitIntValue(Loc.Type, 1);
      OS.EmitIntValue(0, 1);  // Reserved
      OS.EmitIntValue(Loc.Size, 2);
      OS.EmitIntValue(Loc.Reg, 2);
      OS.EmitIntValue(0, 2);  // Reserved
      OS.EmitIntValue(Loc.Offset, 4);
    }

    // Emit alignment to 8 byte.
    OS.EmitValueToAlignment(8);

    // Num live-out registers and padding to align to 4 byte.
    OS.EmitIntValue(0, 2);
    OS.EmitIntValue(LiveOuts.size(), 2);

    for (const auto &LO : LiveOuts) {
      OS.EmitIntValue(LO.DwarfRegNum, 2);
      OS.EmitIntValue(0, 1);
      OS.EmitIntValue(LO.Size, 1);
    }
    // Emit alignment to 8 byte.
    OS.EmitValueToAlignment(8);
  }
}

/// Emit the Popcorn callsite info for each callsite.
///
/// StkMapRecord[NumRecords] {
///   uint64 : PatchPoint ID
///   uint32 : Index of Function Record
///   uint32 : Instruction Offset
///   uint16 : Reserved (record flags)
///   uint16 : NumLocations
///   Location[NumLocations] {
///     uint8 (4 bits) : Register | Direct | Indirect | Constant | ConstantIndex
///     uint8 (1 bit)  : Is it a pointer?
///     uint8 (1 bit)  : Is it an alloca?
///     uint8 (1 bit)  : Is it a duplicate record for the same live value?
///     uint8 (1 bit)  : Is it a temporary value created for the stackmap?
///     uint8          : Size in Bytes
///     uint16         : Dwarf RegNum
///     int32          : Offset
///     uint32         : Size of pointed-to alloca data
///   }
///   uint16 : Padding
///   uint16 : NumLiveOuts
///   LiveOuts[NumLiveOuts] {
///     uint16 : Dwarf RegNum
///     uint8  : Reserved
///     uint8  : Size in Bytes
///   }
///   uint16 : Padding
///   uint16 : NumArchValues
///   ArchValues[NumArchValues] {
///     Location {
///       uint8 (4 bits) : Register | Indirect
///       uint8 (3 bits) : Padding
///       uint8 (1 bit)  : Is it a pointer?
///       uint8          : Size in Bytes
///       uint16         : Dwarf RegNum
///       int32          : Offset
///     }
///     Value {
///       uint8_t (4 bits) : Instruction
///       uint8_t (4 bits) : Register | Direct | Constant
///       uint8_t          : Size
///       uint16_t         : Dwarf RegNum
///       int64_t          : Offset or Constant
///     }
///   }
///   uint32 : Padding (only if required to align to 8 byte)
/// }
///
/// Location Encoding, Type, Value:
///   0x1, Register, Reg                 (value in register)
///   0x2, Direct, Reg + Offset          (frame index)
///   0x3, Indirect, [Reg + Offset]      (spilled value)
///   0x4, Constant, Offset              (small constant)
///   0x5, ConstIndex, Constants[Offset] (large constant)
void StackMaps::emitPcnCallsiteEntries(MCStreamer &OS) {
  LLVM_DEBUG(print(dbgs()));
  // Callsite entries.
  for (const auto &CSI : CSInfos) {
    const LocationVec &CSLocs = CSI.Locations;
    const LiveOutVec &LiveOuts = CSI.LiveOuts;
    const ArchValues &Values = CSI.Vals;

    // Determine the function index.
    int FunctionIndex = 0;
    for (const auto &FNI : FnInfos) {
      if (FNI.first == CSI.Func)
	break;
      FunctionIndex++;
    }

    // Verify stack map entry. It's better to communicate a problem to the
    // runtime than crash in case of in-process compilation. Currently, we do
    // simple overflow checks, but we may eventually communicate other
    // compilation errors this way.
    if (CSLocs.size() > UINT16_MAX || LiveOuts.size() > UINT16_MAX ||
        Values.size() > UINT16_MAX) {
      OS.EmitIntValue(UINT64_MAX, 8); // Invalid ID.
      OS.EmitIntValue(UINT32_MAX, 4); // Invalid index.
      OS.EmitValue(CSI.CSOffsetExpr, 4);
      OS.EmitIntValue(0, 2); // Reserved.
      OS.EmitIntValue(0, 2); // 0 locations.
      OS.EmitIntValue(0, 2); // padding.
      OS.EmitIntValue(0, 2); // 0 live-out registers.
      OS.EmitIntValue(0, 2); // padding.
      OS.EmitIntValue(0, 2); // 0 arch-specific values.
      OS.EmitIntValue(0, 4); // padding.
      continue;
    }

    OS.EmitIntValue(CSI.ID, 8);
    OS.EmitIntValue(FunctionIndex, 4);
    OS.EmitValue(CSI.CSOffsetExpr, 4);

    // Reserved for flags.
    OS.EmitIntValue(0, 2);
    OS.EmitIntValue(CSLocs.size(), 2);

    for (const auto &Loc : CSLocs) {
      uint8_t TypeAndFlags =
        TYPE_AND_FLAGS(Loc.Type, Loc.Ptr, Loc.Alloca,
                       Loc.Duplicate, Loc.Temporary);
      OS.EmitIntValue(TypeAndFlags, 1);
      OS.EmitIntValue(Loc.Size, 1);
      OS.EmitIntValue(Loc.Reg, 2);
      OS.EmitIntValue(Loc.Offset, 4);
      OS.EmitIntValue(Loc.AllocaSize, 4);
    }

    // Num live-out registers and padding to align to 4 byte.
    OS.EmitIntValue(0, 2);
    OS.EmitIntValue(LiveOuts.size(), 2);

    for (const auto &LO : LiveOuts) {
      OS.EmitIntValue(LO.DwarfRegNum, 2);
      OS.EmitIntValue(0, 1);
      OS.EmitIntValue(LO.Size, 1);
    }

    // Num arch-specific constants and padding to align to 4 bytes.
    OS.EmitIntValue(0, 2);
    OS.EmitIntValue(Values.size(), 2);

    for (const auto &C : Values) {
      const Location &Loc = C.first;
      const Operation &Op = C.second;

      uint8_t TypeAndFlags = ARCH_TYPE_AND_FLAGS(Loc.Type, Loc.Ptr);
      OS.EmitIntValue(TypeAndFlags, 1);
      OS.EmitIntValue(Loc.Size, 1);
      OS.EmitIntValue(Loc.Reg, 2);
      OS.EmitIntValue(Loc.Offset, 4);

      uint8_t OpType = ARCH_OP_TYPE(Op.InstType,
                                    Op.isGenerated,
                                    Op.OperandType);
      OS.EmitIntValue(OpType, 1);
      OS.EmitIntValue(Op.Size, 1);
      OS.EmitIntValue(Op.DwarfReg, 2);
      if(Op.isSymbol) OS.EmitSymbolValue(Op.Symbol, 8);
      else OS.EmitIntValue(Op.Constant, 8);
    }

    // Emit alignment to 8 byte.
    OS.EmitValueToAlignment(8);
  }
}

/// Serialize the stackmap data.
void StackMaps::serializeToStackMapSection() {
  (void)WSMP;
  // Bail out if there's no stack map data.
  assert((!CSInfos.empty() || ConstPool.empty()) &&
         "Expected empty constant pool too!");
  assert((!CSInfos.empty() || FnInfos.empty()) &&
         "Expected empty function record too!");
  if (CSInfos.empty())
    return;

  MCContext &OutContext = AP.OutStreamer->getContext();
  MCStreamer &OS = *AP.OutStreamer;

  // Create the section.
  MCSection *StackMapSection =
      OutContext.getObjectFileInfo()->getStackMapSection();
  OS.SwitchSection(StackMapSection);

  // Emit a dummy symbol to force section inclusion.
  OS.EmitLabel(OutContext.getOrCreateSymbol(Twine("__LLVM_StackMaps")));

  // Serialize data.
  LLVM_DEBUG(dbgs() << "********** Stack Map Output **********\n");
  emitStackmapHeader(OS);
  emitFunctionFrameRecords(OS);
  emitConstantPoolEntries(OS);
  emitCallsiteEntries(OS);
  OS.AddBlankLine();

  // Clean up.
  CSInfos.clear();
  ConstPool.clear();
}

/// Serialize the popcorn stackmap data.
void StackMaps::serializeToPcnStackMapSection(const UnwindInfo *UI) {
  (void)WSMP;
  // Bail out if there's no stack map data.
  assert((!CSInfos.empty() || ConstPool.empty()) &&
         "Expected empty constant pool too!");
  assert((!CSInfos.empty() || FnInfos.empty()) &&
         "Expected empty function record too!");
  if (CSInfos.empty())
    return;

  MCContext &OutContext = AP.OutStreamer->getContext();
  MCStreamer &OS = *AP.OutStreamer;

  // Create the section.
  MCSection *StackMapSection =
      OutContext.getObjectFileInfo()->getPcnStackMapSection();
  OS.SwitchSection(StackMapSection);

  // Emit a dummy symbol to force section inclusion.
  OS.EmitLabel(OutContext.getOrCreateSymbol(Twine("__LLVM_PcnStackMaps")));

  // Serialize data.
  LLVM_DEBUG(dbgs() << "********** Stack Map Output **********\n");
  emitStackmapHeader(OS);
  emitPcnFunctionFrameRecords(OS, UI);
  emitConstantPoolEntries(OS);
  emitPcnCallsiteEntries(OS);
  OS.AddBlankLine();

  // Clean up.
  CSInfos.clear();
  ConstPool.clear();
}
