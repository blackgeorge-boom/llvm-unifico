//===- StackMaps.h - StackMaps ----------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CODEGEN_STACKMAPS_H
#define LLVM_CODEGEN_STACKMAPS_H

#include "llvm/ADT/MapVector.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/StackTransformTypes.h"
#include "llvm/IR/CallingConv.h"
#include "llvm/MC/MCSymbol.h"
#include "llvm/Support/Debug.h"
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <vector>

namespace llvm {

class AsmPrinter;
class MCExpr;
class MCStreamer;
class raw_ostream;
class TargetRegisterInfo;
class UnwindInfo;

/// MI-level stackmap operands.
///
/// MI stackmap operations take the form:
/// <id>, <numBytes>, live args...
class StackMapOpers {
public:
  /// Enumerate the meta operands.
  enum { IDPos, NBytesPos };

private:
  const MachineInstr* MI;

public:
  explicit StackMapOpers(const MachineInstr *MI);

  /// Return the ID for the given stackmap
  uint64_t getID() const { return MI->getOperand(IDPos).getImm(); }

  /// Return the number of patchable bytes the given stackmap should emit.
  uint32_t getNumPatchBytes() const {
    return MI->getOperand(NBytesPos).getImm();
  }

  /// Get the operand index of the variable list of non-argument operands.
  /// These hold the "live state".
  unsigned getVarIdx() const {
    // Skip ID, nShadowBytes.
    return 2;
  }
};

/// MI-level patchpoint operands.
///
/// MI patchpoint operations take the form:
/// [<def>], <id>, <numBytes>, <target>, <numArgs>, <cc>, ...
///
/// IR patchpoint intrinsics do not have the <cc> operand because calling
/// convention is part of the subclass data.
///
/// SD patchpoint nodes do not have a def operand because it is part of the
/// SDValue.
///
/// Patchpoints following the anyregcc convention are handled specially. For
/// these, the stack map also records the location of the return value and
/// arguments.
class PatchPointOpers {
public:
  /// Enumerate the meta operands.
  enum { IDPos, NBytesPos, TargetPos, NArgPos, CCPos, MetaEnd };

private:
  const MachineInstr *MI;
  bool HasDef;

  unsigned getMetaIdx(unsigned Pos = 0) const {
    assert(Pos < MetaEnd && "Meta operand index out of range.");
    return (HasDef ? 1 : 0) + Pos;
  }

  const MachineOperand &getMetaOper(unsigned Pos) const {
    return MI->getOperand(getMetaIdx(Pos));
  }

public:
  explicit PatchPointOpers(const MachineInstr *MI);

  bool isAnyReg() const { return (getCallingConv() == CallingConv::AnyReg); }
  bool hasDef() const { return HasDef; }

  /// Return the ID for the given patchpoint.
  uint64_t getID() const { return getMetaOper(IDPos).getImm(); }

  /// Return the number of patchable bytes the given patchpoint should emit.
  uint32_t getNumPatchBytes() const {
    return getMetaOper(NBytesPos).getImm();
  }

  /// Returns the target of the underlying call.
  const MachineOperand &getCallTarget() const {
    return getMetaOper(TargetPos);
  }

  /// Returns the calling convention
  CallingConv::ID getCallingConv() const {
    return getMetaOper(CCPos).getImm();
  }

  unsigned getArgIdx() const { return getMetaIdx() + MetaEnd; }

  /// Return the number of call arguments
  uint32_t getNumCallArgs() const {
    return MI->getOperand(getMetaIdx(NArgPos)).getImm();
  }

  /// Get the operand index of the variable list of non-argument operands.
  /// These hold the "live state".
  unsigned getVarIdx() const {
    return getMetaIdx() + MetaEnd + getNumCallArgs();
  }

  /// Get the index at which stack map locations will be recorded.
  /// Arguments are not recorded unless the anyregcc convention is used.
  unsigned getStackMapStartIdx() const {
    if (isAnyReg())
      return getArgIdx();
    return getVarIdx();
  }

  /// Get the next scratch register operand index.
  unsigned getNextScratchIdx(unsigned StartIdx = 0) const;
};

/// MI-level Statepoint operands
///
/// Statepoint operands take the form:
///   <id>, <num patch bytes >, <num call arguments>, <call target>,
///   [call arguments...],
///   <StackMaps::ConstantOp>, <calling convention>,
///   <StackMaps::ConstantOp>, <statepoint flags>,
///   <StackMaps::ConstantOp>, <num deopt args>, [deopt args...],
///   <gc base/derived pairs...> <gc allocas...>
/// Note that the last two sets of arguments are not currently length
///   prefixed.
class StatepointOpers {
  // TODO:: we should change the STATEPOINT representation so that CC and
  // Flags should be part of meta operands, with args and deopt operands, and
  // gc operands all prefixed by their length and a type code. This would be
  // much more consistent.
public:
  // These values are aboolute offsets into the operands of the statepoint
  // instruction.
  enum { IDPos, NBytesPos, NCallArgsPos, CallTargetPos, MetaEnd };

  // These values are relative offests from the start of the statepoint meta
  // arguments (i.e. the end of the call arguments).
  enum { CCOffset = 1, FlagsOffset = 3, NumDeoptOperandsOffset = 5 };

  explicit StatepointOpers(const MachineInstr *MI) : MI(MI) {}

  /// Get starting index of non call related arguments
  /// (calling convention, statepoint flags, vm state and gc state).
  unsigned getVarIdx() const {
    return MI->getOperand(NCallArgsPos).getImm() + MetaEnd;
  }

  /// Return the ID for the given statepoint.
  uint64_t getID() const { return MI->getOperand(IDPos).getImm(); }

  /// Return the number of patchable bytes the given statepoint should emit.
  uint32_t getNumPatchBytes() const {
    return MI->getOperand(NBytesPos).getImm();
  }

  /// Returns the target of the underlying call.
  const MachineOperand &getCallTarget() const {
    return MI->getOperand(CallTargetPos);
  }

private:
  const MachineInstr *MI;
};

class StackMaps {
public:
  struct Location {
    enum LocationType {
      Unprocessed,
      Register,
      Direct,
      Indirect,
      Constant,
      ConstantIndex
    };
    LocationType Type;
    unsigned Size;
    unsigned Reg;
    int64_t Offset;
    bool Ptr;
    bool Alloca;
    bool Duplicate;
    bool Temporary;
    unsigned AllocaSize;

    Location() {
      Type = Unprocessed;
      Size = 0;
      Reg = 0;
      Offset = 0;
      Ptr = false;
      Alloca = false;
      Duplicate = false;
      Temporary = false;
      AllocaSize = 0;
    }
    Location(LocationType Type, unsigned Size, unsigned Reg, int64_t Offset)
        : Type(Type), Size(Size), Reg(Reg), Offset(Offset) {}
    Location(LocationType Type, unsigned Size, unsigned Reg, int64_t Offset,
	     bool Ptr, bool Alloca, bool Duplicate, bool Temporary,
	     unsigned AllocaSize)
        : Type(Type), Size(Size), Reg(Reg), Offset(Offset), Ptr(Ptr),
          Alloca(Alloca), Duplicate(Duplicate), Temporary(Temporary),
          AllocaSize(AllocaSize) {}
  };

  struct LiveOutReg {
    unsigned short Reg = 0;
    unsigned short DwarfRegNum = 0;
    unsigned short Size = 0;

    LiveOutReg() = default;
    LiveOutReg(unsigned short Reg, unsigned short DwarfRegNum,
               unsigned short Size)
        : Reg(Reg), DwarfRegNum(DwarfRegNum), Size(Size) {}
  };

  struct Operation {
    ValueGenInst::InstType InstType;
    Location::LocationType OperandType;
    unsigned Size;
    unsigned DwarfReg;
    int64_t Constant;
    bool isGenerated;
    bool isSymbol;
    const MCSymbol *Symbol;

    Operation() {
      Size = 0;
      DwarfReg = 0;
      Constant = 0;
      isGenerated = false;
      isSymbol = false;
      Symbol = nullptr;
    }
    Operation(unsigned Size, unsigned DwarfReg, int64_t, bool isGenerated,
	      bool isSymbol)
      : Size(0), DwarfReg(0), Constant(0), isGenerated(false),
        isSymbol(false), Symbol(nullptr) {}
  };

  // OpTypes are used to encode information about the following logical
  // operand (which may consist of several MachineOperands) for the
  // OpParser.
  using OpType = enum { DirectMemRefOp, IndirectMemRefOp, ConstantOp, TemporaryOp };

  StackMaps(AsmPrinter &AP);

  void reset() {
    CSInfos.clear();
    ConstPool.clear();
    FnInfos.clear();
  }

  using LocationVec = SmallVector<Location, 8>;
  using LiveOutVec = SmallVector<LiveOutReg, 8>;
  using ConstantPool = MapVector<uint64_t, uint64_t>;

  struct FunctionInfo {
    uint64_t StackSize = 0;
    uint64_t RecordCount = 1;

    FunctionInfo() = default;
    explicit FunctionInfo(uint64_t StackSize) : StackSize(StackSize) {}
  };

  using ArchValue = std::pair<Location, Operation>;
  using ArchValues = SmallVector<ArchValue, 8>;

  struct CallsiteInfo {
    const MCSymbol *Func = nullptr;
    const MCExpr *CSOffsetExpr = nullptr;
    uint64_t ID = 0;
    LocationVec Locations;
    LiveOutVec LiveOuts;
    ArchValues Vals;

    CallsiteInfo() = default;
    CallsiteInfo(const MCExpr *CSOffsetExpr, uint64_t ID,
                 LocationVec &&Locations, LiveOutVec &&LiveOuts)
        : CSOffsetExpr(CSOffsetExpr), ID(ID), Locations(std::move(Locations)),
          LiveOuts(std::move(LiveOuts)) {}
    CallsiteInfo(const MCSymbol *Func, const MCExpr *CSOffsetExpr,
		 uint64_t ID, LocationVec &&Locations,
		 LiveOutVec &&LiveOuts, ArchValues &&Vals)
        : Func(Func), CSOffsetExpr(CSOffsetExpr), ID(ID),
	  Locations(std::move(Locations)),
          LiveOuts(std::move(LiveOuts)), Vals(std::move(Vals)) {}
  };

  using FnInfoMap = MapVector<const MCSymbol *, FunctionInfo>;
  using CallsiteInfoList = std::vector<CallsiteInfo>;

  /// Generate a stackmap record for a stackmap instruction.
  ///
  /// MI must be a raw STACKMAP, not a PATCHPOINT.
  void recordStackMap(const MachineInstr &MI);

  /// MI must be a raw PCN_STACKMAP, not a STACKMAP.
  void recordPcnStackMap(const MachineInstr &MI);
  
  /// Generate a stackmap record for a patchpoint instruction.
  void recordPatchPoint(const MachineInstr &MI);

  /// Generate a stackmap record for a statepoint instruction.
  void recordStatepoint(const MachineInstr &MI);

  /// If there is any stack map data, create a stack map section and serialize
  /// the map info into it. This clears the stack map data structures
  /// afterwards.
  void serializeToStackMapSection();
  void serializeToPcnStackMapSection(const UnwindInfo *UI = nullptr);

  /// Get call site info.
  CallsiteInfoList &getCSInfos() { return CSInfos; }

  /// Get function info.
  FnInfoMap &getFnInfos() { return FnInfos; }

private:
  static const char *WSMP;

  AsmPrinter &AP;
  CallsiteInfoList CSInfos;
  ConstantPool ConstPool;
  FnInfoMap FnInfos;

  /// Get stackmap information for register location
  void getRegLocation(unsigned Phys, unsigned &Dwarf, unsigned &Offset) const;

  /// Get pointer typing information for stackmap operand
  void getPointerInfo(const Value *Op, const DataLayout &DL, bool &isPtr,
                      bool &isAlloca, unsigned &AllocaSize) const;

  /// Add duplicate target-specific locations for a stackmap operand
  void addDuplicateLocs(const CallInst *StackMap, const Value *Oper,
                        LocationVec &Locs, unsigned Size, bool Ptr,
                        bool Alloca, unsigned AllocaSize) const;

  MachineInstr::const_mop_iterator
  parsePcnOperand(MachineInstr::const_mop_iterator MOI,
               MachineInstr::const_mop_iterator MOE, LocationVec &Locs,
               LiveOutVec &LiveOuts, User::const_op_iterator &Op) const;

  MachineInstr::const_mop_iterator
  parseOperand(MachineInstr::const_mop_iterator MOI,
               MachineInstr::const_mop_iterator MOE, LocationVec &Locs,
               LiveOutVec &LiveOuts) const;

  /// Create a live-out register record for the given register @p Reg.
  LiveOutReg createLiveOutReg(unsigned Reg,
                              const TargetRegisterInfo *TRI) const;

  /// Parse the register live-out mask and return a vector of live-out
  /// registers that need to be recorded in the stackmap.
  LiveOutVec parseRegisterLiveOutMask(const uint32_t *Mask) const;

  /// Convert a list of instructions used to generate an architecture-specific
  /// live value into multiple individual records.
  void genArchValsFromInsts(ArchValues &AV,
                            Location &Loc,
                            const MachineLiveVal &MLV);

  /// Add architecture-specific locations for the stackmap.
  void addArchLiveVals(const CallInst *SM, ArchValues &AV);

  /// This should be called by the MC lowering code _immediately_ before
  /// lowering the MI to an MCInst. It records where the operands for the
  /// instruction are stored, and outputs a label to record the offset of
  /// the call from the start of the text section. In special cases (e.g. AnyReg
  /// calling convention) the return register is also recorded if requested.
  void recordStackMapOpers(const MachineInstr &MI, uint64_t ID,
                           MachineInstr::const_mop_iterator MOI,
                           MachineInstr::const_mop_iterator MOE,
                           bool recordResult = false);

  void recordPcnStackMapOpers(const MachineInstr &MI, uint64_t ID,
			      MachineInstr::const_mop_iterator MOI,
			      MachineInstr::const_mop_iterator MOE,
			      bool recordResult = false);

  /// Emit the stackmap header.
  void emitStackmapHeader(MCStreamer &OS);

  /// Emit the function frame record for each function.
  void emitFunctionFrameRecords(MCStreamer &OS);
  void emitPcnFunctionFrameRecords(MCStreamer &OS, const UnwindInfo *UI);

  /// Emit the constant pool.
  void emitConstantPoolEntries(MCStreamer &OS);

  /// Emit the callsite info for each stackmap/patchpoint intrinsic call.
  void emitCallsiteEntries(MCStreamer &OS);
  void emitPcnCallsiteEntries(MCStreamer &OS);

  void print(raw_ostream &OS);
  void debug() { print(dbgs()); }
};

} // end namespace llvm

#endif // LLVM_CODEGEN_STACKMAPS_H
