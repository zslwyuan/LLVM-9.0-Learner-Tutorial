//===- HI_SeparateConstOffsetFromGEP.cpp -------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

// Modification is to change the alignment of arrayType address space form byte to element.
// Reader can track the modification by the flag : HI-MODIFICATION

//
// Loop unrolling may create many similar GEPs for array accesses.
// e.g., a 2-level loop
//
// float a[32][32]; // global variable
//
// for (int i = 0; i < 2; ++i) {
//   for (int j = 0; j < 2; ++j) {
//     ...
//     ... = a[x + i][y + j];
//     ...
//   }
// }
//
// will probably be unrolled to:
//
// gep %a, 0, %x, %y; load
// gep %a, 0, %x, %y + 1; load
// gep %a, 0, %x + 1, %y; load
// gep %a, 0, %x + 1, %y + 1; load
//
// LLVM's GVN does not use partial redundancy elimination yet, and is thus
// unable to reuse (gep %a, 0, %x, %y). As a result, this misoptimization incurs
// significant slowdown in targets with limited addressing modes. For instance,
// because the PTX target does not support the reg+reg addressing mode, the
// NVPTX backend emits PTX code that literally computes the pointer address of
// each GEP, wasting tons of registers. It emits the following PTX for the
// first load and similar PTX for other loads.
//
// mov.u32         %r1, %x;
// mov.u32         %r2, %y;
// mul.wide.u32    %rl2, %r1, 128;
// mov.u64         %rl3, a;
// add.s64         %rl4, %rl3, %rl2;
// mul.wide.u32    %rl5, %r2, 4;
// add.s64         %rl6, %rl4, %rl5;
// ld.global.f32   %f1, [%rl6];
//
// To reduce the register pressure, the optimization implemented in this file
// merges the common part of a group of GEPs, so we can compute each pointer
// address by adding a simple offset to the common part, saving many registers.
//
// It works by splitting each GEP into a variadic base and a constant offset.
// The variadic base can be computed once and reused by multiple GEPs, and the
// constant offsets can be nicely folded into the reg+immediate addressing mode
// (supported by most targets) without using any extra register.
//
// For instance, we transform the four GEPs and four loads in the above example
// into:
//
// base = gep a, 0, x, y
// load base
// laod base + 1  * sizeof(float)
// load base + 32 * sizeof(float)
// load base + 33 * sizeof(float)
//
// Given the transformed IR, a backend that supports the reg+immediate
// addressing mode can easily fold the pointer arithmetics into the loads. For
// example, the NVPTX backend can easily fold the pointer arithmetics into the
// ld.global.f32 instructions, and the resultant PTX uses much fewer registers.
//
// mov.u32         %r1, %tid.x;
// mov.u32         %r2, %tid.y;
// mul.wide.u32    %rl2, %r1, 128;
// mov.u64         %rl3, a;
// add.s64         %rl4, %rl3, %rl2;
// mul.wide.u32    %rl5, %r2, 4;
// add.s64         %rl6, %rl4, %rl5;
// ld.global.f32   %f1, [%rl6]; // so far the same as unoptimized PTX
// ld.global.f32   %f2, [%rl6+4]; // much better
// ld.global.f32   %f3, [%rl6+128]; // much better
// ld.global.f32   %f4, [%rl6+132]; // much better
//
// Another improvement enabled by the LowerGEP flag is to lower a GEP with
// multiple indices to either multiple GEPs with a single index or arithmetic
// operations (depending on whether the target uses alias analysis in codegen).
// Such transformation can have following benefits:
// (1) It can always extract constants in the indices of structure type.
// (2) After such Lowering, there are more optimization opportunities such as
//     CSE, LICM and CGP.
//
// E.g. The following GEPs have multiple indices:
//  BB1:
//    %p = getelementptr [10 x %struct]* %ptr, i64 %i, i64 %j1, i32 3
//    load %p
//    ...
//  BB2:
//    %p2 = getelementptr [10 x %struct]* %ptr, i64 %i, i64 %j1, i32 2
//    load %p2
//    ...
//
// We can not do CSE to the common part related to index "i64 %i". Lowering
// GEPs can achieve such goals.
// If the target does not use alias analysis in codegen, this pass will
// lower a GEP with multiple indices into arithmetic operations:
//  BB1:
//    %1 = ptrtoint [10 x %struct]* %ptr to i64    ; CSE opportunity
//    %2 = mul i64 %i, length_of_10xstruct         ; CSE opportunity
//    %3 = add i64 %1, %2                          ; CSE opportunity
//    %4 = mul i64 %j1, length_of_struct
//    %5 = add i64 %3, %4
//    %6 = add i64 %3, struct_field_3              ; Constant offset
//    %p = inttoptr i64 %6 to i32*
//    load %p
//    ...
//  BB2:
//    %7 = ptrtoint [10 x %struct]* %ptr to i64    ; CSE opportunity
//    %8 = mul i64 %i, length_of_10xstruct         ; CSE opportunity
//    %9 = add i64 %7, %8                          ; CSE opportunity
//    %10 = mul i64 %j2, length_of_struct
//    %11 = add i64 %9, %10
//    %12 = add i64 %11, struct_field_2            ; Constant offset
//    %p = inttoptr i64 %12 to i32*
//    load %p2
//    ...
//
// If the target uses alias analysis in codegen, this pass will lower a GEP
// with multiple indices into multiple GEPs with a single index:
//  BB1:
//    %1 = bitcast [10 x %struct]* %ptr to i8*     ; CSE opportunity
//    %2 = mul i64 %i, length_of_10xstruct         ; CSE opportunity
//    %3 = getelementptr i8* %1, i64 %2            ; CSE opportunity
//    %4 = mul i64 %j1, length_of_struct
//    %5 = getelementptr i8* %3, i64 %4
//    %6 = getelementptr i8* %5, struct_field_3    ; Constant offset
//    %p = bitcast i8* %6 to i32*
//    load %p
//    ...
//  BB2:
//    %7 = bitcast [10 x %struct]* %ptr to i8*     ; CSE opportunity
//    %8 = mul i64 %i, length_of_10xstruct         ; CSE opportunity
//    %9 = getelementptr i8* %7, i64 %8            ; CSE opportunity
//    %10 = mul i64 %j2, length_of_struct
//    %11 = getelementptr i8* %9, i64 %10
//    %12 = getelementptr i8* %11, struct_field_2  ; Constant offset
//    %p2 = bitcast i8* %12 to i32*
//    load %p2
//    ...
//
// Lowering GEPs can also benefit other passes such as LICM and CGP.
// LICM (Loop Invariant Code Motion) can not hoist/sink a GEP of multiple
// indices if one of the index is variant. If we lower such GEP into invariant
// parts and variant parts, LICM can hoist/sink those invariant parts.
// CGP (CodeGen Prepare) tries to sink address calculations that match the
// target's addressing modes. A GEP with multiple indices may not match and will
// not be sunk. If we lower such GEP into smaller parts, CGP may sink some of
// them. So we end up with a better addressing mode.
//
//===----------------------------------------------------------------------===//

#include "HI_SeparateConstOffsetFromGEP.h"
#include "llvm/ADT/APInt.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/DepthFirstIterator.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/MemoryBuiltins.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/Analysis/TargetTransformInfo.h"
#include "llvm/Analysis/ValueTracking.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constant.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/GetElementPtrTypeIterator.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PatternMatch.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/User.h"
#include "llvm/IR/Value.h"
#include "llvm/Pass.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Utils/Local.h"
#include <cassert>
#include <cstdint>
#include <string>
using namespace llvm;
using namespace llvm::PatternMatch;

char HI_SeparateConstOffsetFromGEP::ID = 0;

bool HI_ConstantOffsetExtractor::CanTraceInto(bool SignExtended, bool ZeroExtended,
                                              BinaryOperator *BO, bool NonNegative)
{
    // We only consider ADD, SUB and OR, because a non-zero constant found in
    // expressions composed of these operations can be easily hoisted as a
    // constant offset by reassociation.
    if (BO->getOpcode() != Instruction::Add && BO->getOpcode() != Instruction::Sub &&
        BO->getOpcode() != Instruction::Or)
    {
        return false;
    }

    Value *LHS = BO->getOperand(0), *RHS = BO->getOperand(1);
    // Do not trace into "or" unless it is equivalent to "add". If LHS and RHS
    // don't have common bits, (LHS | RHS) is equivalent to (LHS + RHS).
    // FIXME: this does not appear to be covered by any tests
    //        (with x86/aarch64 backends at least)
    if (BO->getOpcode() == Instruction::Or && !haveNoCommonBitsSet(LHS, RHS, DL, nullptr, BO, DT))
        return false;

    // In addition, tracing into BO requires that its surrounding s/zext (if
    // any) is distributable to both operands.
    //
    // Suppose BO = A op B.
    //  SignExtended | ZeroExtended | Distributable?
    // --------------+--------------+----------------------------------
    //       0       |      0       | true because no s/zext exists
    //       0       |      1       | zext(BO) == zext(A) op zext(B)
    //       1       |      0       | sext(BO) == sext(A) op sext(B)
    //       1       |      1       | zext(sext(BO)) ==
    //               |              |     zext(sext(A)) op zext(sext(B))
    if (BO->getOpcode() == Instruction::Add && !ZeroExtended && NonNegative)
    {
        // If a + b >= 0 and (a >= 0 or b >= 0), then
        //   sext(a + b) = sext(a) + sext(b)
        // even if the addition is not marked nsw.
        //
        // Leveraging this invarient, we can trace into an sext'ed inbound GEP
        // index if the constant offset is non-negative.
        //
        // Verified in @sext_add in split-gep.ll.
        if (ConstantInt *ConstLHS = dyn_cast<ConstantInt>(LHS))
        {
            if (!ConstLHS->isNegative())
                return true;
        }
        if (ConstantInt *ConstRHS = dyn_cast<ConstantInt>(RHS))
        {
            if (!ConstRHS->isNegative())
                return true;
        }
    }

    // sext (add/sub nsw A, B) == add/sub nsw (sext A), (sext B)
    // zext (add/sub nuw A, B) == add/sub nuw (zext A), (zext B)
    if (BO->getOpcode() == Instruction::Add || BO->getOpcode() == Instruction::Sub)
    {
        if (SignExtended && !BO->hasNoSignedWrap())
            return false;
        if (ZeroExtended && !BO->hasNoUnsignedWrap())
            return false;
    }

    return true;
}

APInt HI_ConstantOffsetExtractor::findInEitherOperand(BinaryOperator *BO, bool SignExtended,
                                                      bool ZeroExtended)
{
    // BO being non-negative does not shed light on whether its operands are
    // non-negative. Clear the NonNegative flag here.
    APInt ConstantOffset = find(BO->getOperand(0), SignExtended, ZeroExtended,
                                /* NonNegative */ false);
    // If we found a constant offset in the left operand, stop and return that.
    // This shortcut might cause us to miss opportunities of combining the
    // constant offsets in both operands, e.g., (a + 4) + (b + 5) => (a + b) + 9.
    // However, such cases are probably already handled by -instcombine,
    // given this pass runs after the standard optimizations.
    if (ConstantOffset != 0)
        return ConstantOffset;
    ConstantOffset = find(BO->getOperand(1), SignExtended, ZeroExtended,
                          /* NonNegative */ false);
    // If U is a sub operator, negate the constant offset found in the right
    // operand.
    if (BO->getOpcode() == Instruction::Sub)
        ConstantOffset = -ConstantOffset;
    return ConstantOffset;
}

APInt HI_ConstantOffsetExtractor::find(Value *V, bool SignExtended, bool ZeroExtended,
                                       bool NonNegative)
{
    // TODO(jingyue): We could trace into integer/pointer casts, such as
    // inttoptr, ptrtoint, bitcast, and addrspacecast. We choose to handle only
    // integers because it gives good enough results for our benchmarks.
    unsigned BitWidth = cast<IntegerType>(V->getType())->getBitWidth();

    // We cannot do much with Values that are not a User, such as an Argument.
    User *U = dyn_cast<User>(V);
    if (U == nullptr)
        return APInt(BitWidth, 0);

    APInt ConstantOffset(BitWidth, 0);
    if (ConstantInt *CI = dyn_cast<ConstantInt>(V))
    {
        // Hooray, we found it!
        ConstantOffset = CI->getValue();
    }
    else if (BinaryOperator *BO = dyn_cast<BinaryOperator>(V))
    {
        // Trace into subexpressions for more hoisting opportunities.
        if (CanTraceInto(SignExtended, ZeroExtended, BO, NonNegative))
            ConstantOffset = findInEitherOperand(BO, SignExtended, ZeroExtended);
    }
    else if (isa<TruncInst>(V))
    {
        ConstantOffset =
            find(U->getOperand(0), SignExtended, ZeroExtended, NonNegative).trunc(BitWidth);
    }
    else if (isa<SExtInst>(V))
    {
        ConstantOffset = find(U->getOperand(0), /* SignExtended */ true, ZeroExtended, NonNegative)
                             .sext(BitWidth);
    }
    else if (isa<ZExtInst>(V))
    {
        // As an optimization, we can clear the SignExtended flag because
        // sext(zext(a)) = zext(a). Verified in @sext_zext in split-gep.ll.
        //
        // Clear the NonNegative flag, because zext(a) >= 0 does not imply a >= 0.
        ConstantOffset = find(U->getOperand(0), /* SignExtended */ false,
                              /* ZeroExtended */ true, /* NonNegative */ false)
                             .zext(BitWidth);
    }

    // If we found a non-zero constant offset, add it to the path for
    // rebuildWithoutConstOffset. Zero is a valid constant offset, but doesn't
    // help this optimization.
    if (ConstantOffset != 0)
        UserChain.push_back(U);
    return ConstantOffset;
}

Value *HI_ConstantOffsetExtractor::applyExts(Value *V)
{
    Value *Current = V;
    // ExtInsts is built in the use-def order. Therefore, we apply them to V
    // in the reversed order.
    for (auto I = ExtInsts.rbegin(), E = ExtInsts.rend(); I != E; ++I)
    {
        if (Constant *C = dyn_cast<Constant>(Current))
        {
            // If Current is a constant, apply s/zext using ConstantExpr::getCast.
            // ConstantExpr::getCast emits a ConstantInt if C is a ConstantInt.
            Current = ConstantExpr::getCast((*I)->getOpcode(), C, (*I)->getType());
        }
        else
        {
            Instruction *Ext = (*I)->clone();
            Ext->setOperand(0, Current);
            Ext->insertBefore(IP);
            Current = Ext;
        }
    }
    return Current;
}

Value *HI_ConstantOffsetExtractor::rebuildWithoutConstOffset()
{
    distributeExtsAndCloneChain(UserChain.size() - 1);
    // Remove all nullptrs (used to be s/zext) from UserChain.
    unsigned NewSize = 0;
    for (User *I : UserChain)
    {
        if (I != nullptr)
        {
            UserChain[NewSize] = I;
            NewSize++;
        }
    }
    UserChain.resize(NewSize);
    return removeConstOffset(UserChain.size() - 1);
}

Value *HI_ConstantOffsetExtractor::distributeExtsAndCloneChain(unsigned ChainIndex)
{
    User *U = UserChain[ChainIndex];
    if (ChainIndex == 0)
    {
        assert(isa<ConstantInt>(U));
        // If U is a ConstantInt, applyExts will return a ConstantInt as well.
        return UserChain[ChainIndex] = cast<ConstantInt>(applyExts(U));
    }

    if (CastInst *Cast = dyn_cast<CastInst>(U))
    {
        assert((isa<SExtInst>(Cast) || isa<ZExtInst>(Cast) || isa<TruncInst>(Cast)) &&
               "Only following instructions can be traced: sext, zext & trunc");
        ExtInsts.push_back(Cast);
        UserChain[ChainIndex] = nullptr;
        return distributeExtsAndCloneChain(ChainIndex - 1);
    }

    // Function find only trace into BinaryOperator and CastInst.
    BinaryOperator *BO = cast<BinaryOperator>(U);
    // OpNo = which operand of BO is UserChain[ChainIndex - 1]
    unsigned OpNo = (BO->getOperand(0) == UserChain[ChainIndex - 1] ? 0 : 1);
    Value *TheOther = applyExts(BO->getOperand(1 - OpNo));
    Value *NextInChain = distributeExtsAndCloneChain(ChainIndex - 1);

    BinaryOperator *NewBO = nullptr;
    if (OpNo == 0)
    {
        NewBO = BinaryOperator::Create(BO->getOpcode(), NextInChain, TheOther, BO->getName(), IP);
    }
    else
    {
        NewBO = BinaryOperator::Create(BO->getOpcode(), TheOther, NextInChain, BO->getName(), IP);
    }
    return UserChain[ChainIndex] = NewBO;
}

Value *HI_ConstantOffsetExtractor::removeConstOffset(unsigned ChainIndex)
{
    if (ChainIndex == 0)
    {
        assert(isa<ConstantInt>(UserChain[ChainIndex]));
        return ConstantInt::getNullValue(UserChain[ChainIndex]->getType());
    }

    BinaryOperator *BO = cast<BinaryOperator>(UserChain[ChainIndex]);
    assert(BO->getNumUses() <= 1 && "distributeExtsAndCloneChain clones each BinaryOperator in "
                                    "UserChain, so no one should be used more than "
                                    "once");

    unsigned OpNo = (BO->getOperand(0) == UserChain[ChainIndex - 1] ? 0 : 1);
    assert(BO->getOperand(OpNo) == UserChain[ChainIndex - 1]);
    Value *NextInChain = removeConstOffset(ChainIndex - 1);
    Value *TheOther = BO->getOperand(1 - OpNo);

    // If NextInChain is 0 and not the LHS of a sub, we can simplify the
    // sub-expression to be just TheOther.
    if (ConstantInt *CI = dyn_cast<ConstantInt>(NextInChain))
    {
        if (CI->isZero() && !(BO->getOpcode() == Instruction::Sub && OpNo == 0))
            return TheOther;
    }

    BinaryOperator::BinaryOps NewOp = BO->getOpcode();
    if (BO->getOpcode() == Instruction::Or)
    {
        // Rebuild "or" as "add", because "or" may be invalid for the new
        // expression.
        //
        // For instance, given
        //   a | (b + 5) where a and b + 5 have no common bits,
        // we can extract 5 as the constant offset.
        //
        // However, reusing the "or" in the new index would give us
        //   (a | b) + 5
        // which does not equal a | (b + 5).
        //
        // Replacing the "or" with "add" is fine, because
        //   a | (b + 5) = a + (b + 5) = (a + b) + 5
        NewOp = Instruction::Add;
    }

    BinaryOperator *NewBO;
    if (OpNo == 0)
    {
        NewBO = BinaryOperator::Create(NewOp, NextInChain, TheOther, "", IP);
    }
    else
    {
        NewBO = BinaryOperator::Create(NewOp, TheOther, NextInChain, "", IP);
    }
    NewBO->takeName(BO);
    return NewBO;
}

Value *HI_ConstantOffsetExtractor::Extract(Value *Idx, GetElementPtrInst *GEP, User *&UserChainTail,
                                           const DominatorTree *DT)
{
    HI_ConstantOffsetExtractor Extractor(GEP, DT);
    // Find a non-zero constant offset first.
    APInt ConstantOffset =
        Extractor.find(Idx, /* SignExtended */ false, /* ZeroExtended */ false, GEP->isInBounds());
    if (ConstantOffset == 0)
    {
        UserChainTail = nullptr;
        return nullptr;
    }
    // Separates the constant offset from the GEP index.
    Value *IdxWithoutConstOffset = Extractor.rebuildWithoutConstOffset();
    UserChainTail = Extractor.UserChain.back();
    return IdxWithoutConstOffset;
}

int64_t HI_ConstantOffsetExtractor::Find(Value *Idx, GetElementPtrInst *GEP,
                                         const DominatorTree *DT)
{
    // If Idx is an index of an inbound GEP, Idx is guaranteed to be non-negative.
    return HI_ConstantOffsetExtractor(GEP, DT)
        .find(Idx, /* SignExtended */ false, /* ZeroExtended */ false, GEP->isInBounds())
        .getSExtValue();
}

bool HI_SeparateConstOffsetFromGEP::canonicalizeArrayIndicesToPointerSize(GetElementPtrInst *GEP)
{
    bool Changed = false;
    Type *IntPtrTy = DL->getIntPtrType(GEP->getType());
    gep_type_iterator GTI = gep_type_begin(*GEP);
    for (User::op_iterator I = GEP->op_begin() + 1, E = GEP->op_end(); I != E; ++I, ++GTI)
    {
        // Skip struct member indices which must be i32.
        if (GTI.isSequential())
        {
            if ((*I)->getType() != IntPtrTy)
            {
                *I = CastInst::CreateIntegerCast(*I, IntPtrTy, true, "idxprom", GEP);
                Changed = true;
            }
        }
    }
    return Changed;
}

int64_t HI_SeparateConstOffsetFromGEP::accumulateByteOffset(GetElementPtrInst *GEP,
                                                            bool &NeedsExtraction)
{
    NeedsExtraction = false;
    int64_t AccumulativeByteOffset = 0;
    gep_type_iterator GTI = gep_type_begin(*GEP);
    for (unsigned I = 1, E = GEP->getNumOperands(); I != E; ++I, ++GTI)
    {
        if (GTI.isSequential())
        {
            // Tries to extract a constant offset from this GEP index.
            int64_t ConstantOffset = HI_ConstantOffsetExtractor::Find(GEP->getOperand(I), GEP, DT);
            if (ConstantOffset != 0)
            {
                NeedsExtraction = true;
                // A GEP may have multiple indices.  We accumulate the extracted
                // constant offset to a byte offset, and later offset the remainder of
                // the original GEP with this byte offset.

                AccumulativeByteOffset +=
                    ConstantOffset *
                    getLength(GTI.getIndexedType()); // HI-MODIFICATION: In HLS, data are not stored
                                                     // in bytes.
            }
        }
        else if (LowerGEP)
        {
            print_warning("GEP Instruction for struct type detected! Please note that currently, "
                          "Hi-LLVM does not support GEP for struct.");
            StructType *StTy = GTI.getStructType();
            uint64_t Field = cast<ConstantInt>(GEP->getOperand(I))->getZExtValue();
            // Skip field 0 as the offset is always 0.
            if (Field != 0)
            {
                NeedsExtraction = true;
                AccumulativeByteOffset += DL->getStructLayout(StTy)->getElementOffset(Field);
            }
        }
    }
    return AccumulativeByteOffset;
}

void HI_SeparateConstOffsetFromGEP::lowerToSingleIndexGEPs(GetElementPtrInst *Variadic,
                                                           int64_t AccumulativeByteOffset)
{
    if (DEBUG)
        *Sep_Log << "\nlowering GEP (lowerToSingleIndexGEPs): " << *Variadic << "\n";
    IRBuilder<> Builder(Variadic);
    Type *IntPtrTy = DL->getIntPtrType(Variadic->getType());

    Type *I8PtrTy = Builder.getInt8PtrTy(Variadic->getType()->getPointerAddressSpace());
    Value *ResultPtr = Variadic->getOperand(0);
    Loop *L = LI->getLoopFor(Variadic->getParent());
    // Check if the base is not loop invariant or used more than once.
    bool isSwapCandidate =
        L && L->isLoopInvariant(ResultPtr) && !hasMoreThanOneUseInLoop(ResultPtr, L);
    Value *FirstResult = nullptr;

    if (ResultPtr->getType() != I8PtrTy)
        ResultPtr = Builder.CreateBitCast(ResultPtr, I8PtrTy);

    gep_type_iterator GTI = gep_type_begin(*Variadic);
    // Create an ugly GEP for each sequential index. We don't create GEPs for
    // structure indices, as they are accumulated in the constant offset index.
    for (unsigned I = 1, E = Variadic->getNumOperands(); I != E; ++I, ++GTI)
    {
        if (GTI.isSequential())
        {
            Value *Idx = Variadic->getOperand(I);
            // Skip zero indices.
            if (ConstantInt *CI = dyn_cast<ConstantInt>(Idx))
                if (CI->isZero())
                    continue;

            APInt ElementSize =
                APInt(IntPtrTy->getIntegerBitWidth(), // 1);   //HI-MODIFICATION: we don't need to
                                                      // calculate by byte in HLS
                      getLength(GTI.getIndexedType()));

            // Scale the index by element size.
            if (ElementSize != 1)
            {
                if (ElementSize.isPowerOf2())
                {
                    Idx =
                        Builder.CreateShl(Idx, ConstantInt::get(IntPtrTy, ElementSize.logBase2()));
                }
                else
                {
                    Idx = Builder.CreateMul(Idx, ConstantInt::get(IntPtrTy, ElementSize));
                }
            }
            // Create an ugly GEP with a single index for each index.
            ResultPtr = Builder.CreateGEP(Builder.getInt8Ty(), ResultPtr, Idx, "uglygep");
            if (FirstResult == nullptr)
                FirstResult = ResultPtr;
        }
    }

    // Create a GEP with the constant offset index.
    //  if (AccumulativeByteOffset != 0) {
    Value *Offset = ConstantInt::get(IntPtrTy, AccumulativeByteOffset);
    ResultPtr = Builder.CreateGEP(Builder.getInt8Ty(), ResultPtr, Offset, "uglygep");
    //  } else
    //    isSwapCandidate = false;

    // If we created a GEP with constant index, and the base is loop invariant,
    // then we swap the first one with it, so LICM can move constant GEP out
    // later.
    GetElementPtrInst *FirstGEP = dyn_cast_or_null<GetElementPtrInst>(FirstResult);
    GetElementPtrInst *SecondGEP = dyn_cast_or_null<GetElementPtrInst>(ResultPtr);
    if (isSwapCandidate && isLegalToSwapOperand(FirstGEP, SecondGEP, L))
        swapGEPOperand(FirstGEP, SecondGEP);

    if (ResultPtr->getType() != Variadic->getType())
        ResultPtr = Builder.CreateBitCast(ResultPtr, Variadic->getType());

    if (DEBUG)
        *Sep_Log << "\nreplcing " << *Variadic << " with " << *ResultPtr << "\n";
    Variadic->replaceAllUsesWith(ResultPtr);
    Variadic->eraseFromParent();
}

void HI_SeparateConstOffsetFromGEP::lowerToArithmetics(GetElementPtrInst *Variadic,
                                                       int64_t AccumulativeByteOffset)
{
    IRBuilder<> Builder(Variadic);
    Type *IntPtrTy = DL->getIntPtrType(Variadic->getType());

    Value *ResultPtr = Builder.CreatePtrToInt(Variadic->getOperand(0), IntPtrTy);
    if (ConstantExpr *constE = dyn_cast<ConstantExpr>(ResultPtr))
    {
        Instruction *tmpI = constE->getAsInstruction();
        tmpI->insertBefore(Variadic);
        ResultPtr = tmpI;
    }
    Value *tmp_ResultPtr = nullptr;
    gep_type_iterator GTI = gep_type_begin(*Variadic);
    // Create ADD/SHL/MUL arithmetic operations for each sequential indices. We
    // don't create arithmetics for structure indices, as they are accumulated
    // in the constant offset index.

    if (DEBUG)
        *Sep_Log << "\nlowering GEP (lowerToArithmetics): " << *Variadic << "\n";
    for (unsigned I = 1, E = Variadic->getNumOperands(); I != E; ++I, ++GTI)
    {
        if (GTI.isSequential())
        {

            Value *Idx = Variadic->getOperand(I);
            // Skip zero indices.
            if (ConstantInt *CI = dyn_cast<ConstantInt>(Idx))
                if (CI->isZero())
                    continue;
            if (DEBUG)
                *Sep_Log << " --- handling Index " << *Idx << "\n";
            if (DEBUG)
                *Sep_Log << " --- handling IndexedType " << *GTI.getIndexedType() << "\n";
            if (DEBUG)
                *Sep_Log << " --- handling TypeAllocSize "
                         << DL->getTypeAllocSize(GTI.getIndexedType()) << "\n";
            if (DEBUG)
                *Sep_Log << " --- handling Length " << getLength(GTI.getIndexedType()) << "\n";
            if (DEBUG)
                Sep_Log->flush();
            APInt ElementSize =
                APInt(IntPtrTy->getIntegerBitWidth(), // HI-MODIFICATION: we don't need to calculate
                                                      // by byte in HLS
                      getLength(GTI.getIndexedType()));
            // Scale the index by element size.
            if (ElementSize != 1)
            {
                if (ElementSize.isPowerOf2())
                {
                    Idx =
                        Builder.CreateShl(Idx, ConstantInt::get(IntPtrTy, ElementSize.logBase2()));
                }
                else
                {
                    if (DEBUG)
                        *Sep_Log << "    --- IntPtrTy " << *IntPtrTy << "\n";
                    if (DEBUG)
                        *Sep_Log << "    --- ElementSize " << ElementSize << "\n";
                    if (DEBUG)
                        Sep_Log->flush();
                    Idx = Builder.CreateMul(Idx, ConstantInt::get(IntPtrTy, ElementSize));
                }
            }
            // Create an ADD for each index.
            if (tmp_ResultPtr)
            {
                tmp_ResultPtr = Builder.CreateAdd(tmp_ResultPtr, Idx);
            }
            else
            {
                tmp_ResultPtr = Idx;
            }
        }
    }

    if (AccumulativeByteOffset != 0)
    {
        if (tmp_ResultPtr)
        {
            tmp_ResultPtr = Builder.CreateAdd(tmp_ResultPtr,
                                              ConstantInt::get(IntPtrTy, AccumulativeByteOffset));
        }
        else
        {
            tmp_ResultPtr = ConstantInt::get(IntPtrTy, AccumulativeByteOffset, true);
        }
    }

    if (DEBUG)
        *Sep_Log << "  Builder.CreateAdd(ResultPtr, tmp_ResultPtr) ResultPtr=" << ResultPtr
                 << "  tmp_ResultPtr=" << tmp_ResultPtr << "\n";
    if (tmp_ResultPtr)
        if (DEBUG)
            *Sep_Log << "  Builder.CreateAdd(ResultPtr, tmp_ResultPtr) Result=" << *ResultPtr
                     << "  tmp_Result=" << *tmp_ResultPtr << "\n";
    if (DEBUG)
        Sep_Log->flush();

    if (tmp_ResultPtr)
        ResultPtr = Builder.CreateAdd(ResultPtr, tmp_ResultPtr);

    // Create an ADD for the constant offset index.
    if (DEBUG)
        *Sep_Log << "\ncreated adder " << *ResultPtr << "\n";

    ResultPtr = Builder.CreateIntToPtr(ResultPtr, Variadic->getType());

    // ResultPtr = Builder.Insert(CastInst::Create(Instruction::IntToPtr, ResultPtr,
    // Variadic->getType()), "");

    if (DEBUG)
        *Sep_Log << "\ncreated ITP " << *ResultPtr << "\n";

    if (DEBUG)
        *Sep_Log << "\nreplcing " << *Variadic << " with " << *ResultPtr << "\n";
    Variadic->replaceAllUsesWith(ResultPtr);
    Variadic->eraseFromParent();
}

bool HI_SeparateConstOffsetFromGEP::splitGEP(GetElementPtrInst *GEP)
{
    if (DEBUG)
        *Sep_Log << "\n================================================\nSplitting GEP : " << *GEP
                 << "\n";
    // Skip vector GEPs.
    if (GEP->getType()->isVectorTy())
    {
        if (DEBUG)
            *Sep_Log << *GEP << " isVectorTy\n";
        return false;
    }

    // // Even if it has all constant indice, we need to lower it to get
    // the array partition for it
    // if (GEP->hasAllConstantIndices())
    // {
    //   if (DEBUG) *Sep_Log << *GEP << " hasAllConstantIndices\n";
    //   return false;
    // }

    bool Changed = canonicalizeArrayIndicesToPointerSize(GEP);

    bool NeedsExtraction;
    int64_t AccumulativeByteOffset = accumulateByteOffset(GEP, NeedsExtraction);

    if (!NeedsExtraction)
    {
        if (DEBUG)
            *Sep_Log << *GEP << " does not Need Extraction\n";
        if (DEBUG)
            *Sep_Log << " AccumulativeByteOffset = " << AccumulativeByteOffset << "\n";
        if (DEBUG)
            *Sep_Log << " Enforce it to lowering GEP for test \n"; // HI-MODIFICATION: we don't need
                                                                   // to calculate by byte in HLS
        //   return Changed;
    }

    TargetTransformInfo &TTI =
        getAnalysis<TargetTransformInfoWrapperPass>().getTTI(*GEP->getFunction());

    // If LowerGEP is disabled, before really splitting the GEP, check whether the
    // backend supports the addressing mode we are about to produce. If no, this
    // splitting probably won't be beneficial.
    // If LowerGEP is enabled, even the extracted constant offset can not match
    // the addressing mode, we can still do optimizations to other lowered parts
    // of variable indices. Therefore, we don't check for addressing modes in that
    // case.
    if (!LowerGEP)
    {
        unsigned AddrSpace = GEP->getPointerAddressSpace();
        if (!TTI.isLegalAddressingMode(GEP->getResultElementType(),
                                       /*BaseGV=*/nullptr, AccumulativeByteOffset,
                                       /*HasBaseReg=*/true, /*Scale=*/0, AddrSpace))
        {
            if (DEBUG)
                *Sep_Log << *GEP << " is not LegalAddressingMode\n";
            return Changed;
        }
    }

    // Remove the constant offset in each sequential index. The resultant GEP
    // computes the variadic base.
    // Notice that we don't remove struct field indices here. If LowerGEP is
    // disabled, a structure index is not accumulated and we still use the old
    // one. If LowerGEP is enabled, a structure index is accumulated in the
    // constant offset. LowerToSingleIndexGEPs or lowerToArithmetics will later
    // handle the constant offset and won't need a new structure index.
    gep_type_iterator GTI = gep_type_begin(*GEP);
    for (unsigned I = 1, E = GEP->getNumOperands(); I != E; ++I, ++GTI)
    {
        if (GTI.isSequential())
        {
            // Splits this GEP index into a variadic part and a constant offset, and
            // uses the variadic part as the new index.
            Value *OldIdx = GEP->getOperand(I);
            User *UserChainTail;
            Value *NewIdx = HI_ConstantOffsetExtractor::Extract(OldIdx, GEP, UserChainTail, DT);
            if (NewIdx != nullptr)
            {
                // Switches to the index with the constant offset removed.
                GEP->setOperand(I, NewIdx);
                // After switching to the new index, we can garbage-collect UserChain
                // and the old index if they are not used.
                RecursivelyDeleteTriviallyDeadInstructions(UserChainTail);
                RecursivelyDeleteTriviallyDeadInstructions(OldIdx);
            }
        }
    }

    // Clear the inbounds attribute because the new index may be off-bound.
    // e.g.,
    //
    //   b     = add i64 a, 5
    //   addr  = gep inbounds float, float* p, i64 b
    //
    // is transformed to:
    //
    //   addr2 = gep float, float* p, i64 a ; inbounds removed
    //   addr  = gep inbounds float, float* addr2, i64 5
    //
    // If a is -4, although the old index b is in bounds, the new index a is
    // off-bound. http://llvm.org/docs/LangRef.html#id181 says "if the
    // inbounds keyword is not present, the offsets are added to the base
    // address with silently-wrapping two's complement arithmetic".
    // Therefore, the final code will be a semantically equivalent.
    //
    // TODO(jingyue): do some range analysis to keep as many inbounds as
    // possible. GEPs with inbounds are more friendly to alias analysis.
    bool GEPWasInBounds = GEP->isInBounds();
    GEP->setIsInBounds(false);

    // Lowers a GEP to either GEPs with a single index or arithmetic operations.
    if (LowerGEP)
    {
        // As currently BasicAA does not analyze ptrtoint/inttoptr, do not lower to
        // arithmetic operations if the target uses alias analysis in codegen.
        if (TTI.useAA())
            lowerToSingleIndexGEPs(GEP, AccumulativeByteOffset);
        else
            lowerToArithmetics(GEP, AccumulativeByteOffset);
        return true;
    }

    if (DEBUG)
        *Sep_Log << *GEP << " is not beging LowerGEP\n";

    // No need to create another GEP if the accumulative byte offset is 0.
    if (AccumulativeByteOffset == 0)
        return true;

    // Offsets the base with the accumulative byte offset.
    //
    //   %gep                        ; the base
    //   ... %gep ...
    //
    // => add the offset
    //
    //   %gep2                       ; clone of %gep
    //   %new.gep = gep %gep2, <offset / sizeof(*%gep)>
    //   %gep                        ; will be removed
    //   ... %gep ...
    //
    // => replace all uses of %gep with %new.gep and remove %gep
    //
    //   %gep2                       ; clone of %gep
    //   %new.gep = gep %gep2, <offset / sizeof(*%gep)>
    //   ... %new.gep ...
    //
    // If AccumulativeByteOffset is not a multiple of sizeof(*%gep), we emit an
    // uglygep (http://llvm.org/docs/GetElementPtr.html#what-s-an-uglygep):
    // bitcast %gep2 to i8*, add the offset, and bitcast the result back to the
    // type of %gep.
    //
    //   %gep2                       ; clone of %gep
    //   %0       = bitcast %gep2 to i8*
    //   %uglygep = gep %0, <offset>
    //   %new.gep = bitcast %uglygep to <type of %gep>
    //   ... %new.gep ...
    Instruction *NewGEP = GEP->clone();
    NewGEP->insertBefore(GEP);

    // Per ANSI C standard, signed / unsigned = unsigned and signed % unsigned =
    // unsigned.. Therefore, we cast ElementTypeSizeOfGEP to signed because it is
    // used with unsigned integers later.
    int64_t ElementTypeSizeOfGEP = static_cast<int64_t>( // 1);     //HI-MODIFICATION: we don't need
                                                         // to calculate by byte in HLS
        getLength(GEP->getResultElementType()));
    Type *IntPtrTy = DL->getIntPtrType(GEP->getType());
    if (AccumulativeByteOffset % ElementTypeSizeOfGEP == 0)
    {
        // Very likely. As long as %gep is naturally aligned, the byte offset we
        // extracted should be a multiple of sizeof(*%gep).
        int64_t Index = AccumulativeByteOffset / ElementTypeSizeOfGEP;
        NewGEP =
            GetElementPtrInst::Create(GEP->getResultElementType(), NewGEP,
                                      ConstantInt::get(IntPtrTy, Index, true), GEP->getName(), GEP);
        NewGEP->copyMetadata(*GEP);
        // Inherit the inbounds attribute of the original GEP.
        cast<GetElementPtrInst>(NewGEP)->setIsInBounds(GEPWasInBounds);
    }
    else
    {
        // Unlikely but possible. For example,
        // #pragma pack(1)
        // struct S {
        //   int a[3];
        //   int64 b[8];
        // };
        // #pragma pack()
        //
        // Suppose the gep before extraction is &s[i + 1].b[j + 3]. After
        // extraction, it becomes &s[i].b[j] and AccumulativeByteOffset is
        // sizeof(S) + 3 * sizeof(int64) = 100, which is not a multiple of
        // sizeof(int64).
        //
        // Emit an uglygep in this case.
        Type *I8PtrTy = Type::getInt8PtrTy(GEP->getContext(), GEP->getPointerAddressSpace());
        NewGEP = new BitCastInst(NewGEP, I8PtrTy, "", GEP);
        NewGEP = GetElementPtrInst::Create(Type::getInt8Ty(GEP->getContext()), NewGEP,
                                           ConstantInt::get(IntPtrTy, AccumulativeByteOffset, true),
                                           "uglygep", GEP);
        NewGEP->copyMetadata(*GEP);
        // Inherit the inbounds attribute of the original GEP.
        cast<GetElementPtrInst>(NewGEP)->setIsInBounds(GEPWasInBounds);
        if (GEP->getType() != I8PtrTy)
            NewGEP = new BitCastInst(NewGEP, GEP->getType(), GEP->getName(), GEP);
    }

    GEP->replaceAllUsesWith(NewGEP);
    GEP->eraseFromParent();

    return true;
}

bool HI_SeparateConstOffsetFromGEP::runOnFunction(Function &F)
{
    if (skipFunction(F))
        return false;

    DT = &getAnalysis<DominatorTreeWrapperPass>().getDomTree();
    SE = &getAnalysis<ScalarEvolutionWrapperPass>().getSE();
    LI = &getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
    TLI = &getAnalysis<TargetLibraryInfoWrapperPass>().getTLI();
    bool Changed = false;

    for (BasicBlock &B : F)
    {
        bool modified = true;
        while (modified)
        {
            modified = false;
            for (BasicBlock::iterator I = B.begin(), IE = B.end(); I != IE; I++)
            {
                for (int i = 0; i < I->getNumOperands(); i++)
                {
                    Instruction *tmpI = dyn_cast<Instruction>(I);
                    if (auto constE = dyn_cast<ConstantExpr>(I->getOperand(i)))
                    {
                        if (constE->getOpcode() == Instruction::GetElementPtr)
                        {
                            if (DEBUG)
                                *Sep_Log << "Inst: " << *tmpI
                                         << " should hoist the constant expr into an instuction "
                                            "for later address tracing.\n";
                            if (DEBUG)
                                *Sep_Log << "OriBlock:\n " << *tmpI->getParent() << "\n";

                            if (DEBUG)
                                Sep_Log->flush();

                            assert(constE);
                            assert(constE->getOpcode() == Instruction::GetElementPtr);

                            Instruction *gepI = constE->getAsInstruction();
                            if (auto PHI_I = dyn_cast<llvm::PHINode>(tmpI))
                            {
                                gepI->insertBefore(PHI_I->getIncomingBlock(i)->getTerminator());
                            }
                            else
                            {
                                gepI->insertBefore(tmpI);
                            }

                            tmpI->setOperand(i, gepI);
                            modified = true;
                            Changed = true;

                            if (DEBUG)
                                *Sep_Log << "NewBlock:\n " << *I->getParent() << "\n";

                            break;
                        }
                    }
                }
                if (modified)
                    break;
            }
        }
    }

    if (DEBUG)
        *Sep_Log << "\n\n after constant hoisting, F=\n"
                 << F << "\n================================\n";
    if (DEBUG)
        Sep_Log->flush();

    for (BasicBlock &B : F)
    {
        for (BasicBlock::iterator I = B.begin(), IE = B.end(); I != IE;)
            if (GetElementPtrInst *GEP = dyn_cast<GetElementPtrInst>(I++))
                Changed |= splitGEP(GEP);
        // No need to split GEP ConstantExprs because all its indices are constant
        // already.
    }

    Changed |= reuniteExts(F);

    if (DEBUG)
        *Sep_Log << "\n\n after GEP lowering, F=\n" << F << "\n================================\n";
    if (DEBUG)
        Sep_Log->flush();

    return Changed;
}

Instruction *HI_SeparateConstOffsetFromGEP::findClosestMatchingDominator(const SCEV *Key,
                                                                         Instruction *Dominatee)
{
    auto Pos = DominatingExprs.find(Key);
    if (Pos == DominatingExprs.end())
        return nullptr;

    auto &Candidates = Pos->second;
    // Because we process the basic blocks in pre-order of the dominator tree, a
    // candidate that doesn't dominate the current instruction won't dominate any
    // future instruction either. Therefore, we pop it out of the stack. This
    // optimization makes the algorithm O(n).
    while (!Candidates.empty())
    {
        Instruction *Candidate = Candidates.back();
        if (DT->dominates(Candidate, Dominatee))
            return Candidate;
        Candidates.pop_back();
    }
    return nullptr;
}

bool HI_SeparateConstOffsetFromGEP::reuniteExts(Instruction *I)
{
    if (!SE->isSCEVable(I->getType()))
        return false;

    //   Dom: LHS+RHS
    //   I: sext(LHS)+sext(RHS)
    // If Dom can't sign overflow and Dom dominates I, optimize I to sext(Dom).
    // TODO: handle zext
    Value *LHS = nullptr, *RHS = nullptr;
    if (match(I, m_Add(m_SExt(m_Value(LHS)), m_SExt(m_Value(RHS)))) ||
        match(I, m_Sub(m_SExt(m_Value(LHS)), m_SExt(m_Value(RHS)))))
    {
        if (LHS->getType() == RHS->getType())
        {
            const SCEV *Key = SE->getAddExpr(SE->getUnknown(LHS), SE->getUnknown(RHS));
            if (auto *Dom = findClosestMatchingDominator(Key, I))
            {
                Instruction *NewSExt = new SExtInst(Dom, I->getType(), "", I);
                NewSExt->takeName(I);
                I->replaceAllUsesWith(NewSExt);
                RecursivelyDeleteTriviallyDeadInstructions(I);
                return true;
            }
        }
    }

    // Add I to DominatingExprs if it's an add/sub that can't sign overflow.
    if (match(I, m_NSWAdd(m_Value(LHS), m_Value(RHS))) ||
        match(I, m_NSWSub(m_Value(LHS), m_Value(RHS))))
    {
        if (programUndefinedIfFullPoison(I))
        {
            const SCEV *Key = SE->getAddExpr(SE->getUnknown(LHS), SE->getUnknown(RHS));
            DominatingExprs[Key].push_back(I);
        }
    }
    return false;
}

bool HI_SeparateConstOffsetFromGEP::reuniteExts(Function &F)
{
    bool Changed = false;
    DominatingExprs.clear();
    for (const auto Node : depth_first(DT))
    {
        BasicBlock *BB = Node->getBlock();
        for (auto I = BB->begin(); I != BB->end();)
        {
            Instruction *Cur = &*I++;
            Changed |= reuniteExts(Cur);
        }
    }
    return Changed;
}

void HI_SeparateConstOffsetFromGEP::verifyNoDeadCode(Function &F)
{
    for (BasicBlock &B : F)
    {
        for (Instruction &I : B)
        {
            if (isInstructionTriviallyDead(&I))
            {
                std::string ErrMessage;
                raw_string_ostream RSO(ErrMessage);
                RSO << "Dead instruction detected!\n" << I << "\n";
                llvm_unreachable(RSO.str().c_str());
            }
        }
    }
}

bool HI_SeparateConstOffsetFromGEP::isLegalToSwapOperand(GetElementPtrInst *FirstGEP,
                                                         GetElementPtrInst *SecondGEP,
                                                         Loop *CurLoop)
{
    if (!FirstGEP || !FirstGEP->hasOneUse())
        return false;

    if (!SecondGEP || FirstGEP->getParent() != SecondGEP->getParent())
        return false;

    if (FirstGEP == SecondGEP)
        return false;

    unsigned FirstNum = FirstGEP->getNumOperands();
    unsigned SecondNum = SecondGEP->getNumOperands();
    // Give up if the number of operands are not 2.
    if (FirstNum != SecondNum || FirstNum != 2)
        return false;

    Value *FirstBase = FirstGEP->getOperand(0);
    Value *SecondBase = SecondGEP->getOperand(0);
    Value *FirstOffset = FirstGEP->getOperand(1);
    // Give up if the index of the first GEP is loop invariant.
    if (CurLoop->isLoopInvariant(FirstOffset))
        return false;

    // Give up if base doesn't have same type.
    if (FirstBase->getType() != SecondBase->getType())
        return false;

    Instruction *FirstOffsetDef = dyn_cast<Instruction>(FirstOffset);

    // Check if the second operand of first GEP has constant coefficient.
    // For an example, for the following code,  we won't gain anything by
    // hoisting the second GEP out because the second GEP can be folded away.
    //   %scevgep.sum.ur159 = add i64 %idxprom48.ur, 256
    //   %67 = shl i64 %scevgep.sum.ur159, 2
    //   %uglygep160 = getelementptr i8* %65, i64 %67
    //   %uglygep161 = getelementptr i8* %uglygep160, i64 -1024

    // Skip constant shift instruction which may be generated by Splitting GEPs.
    if (FirstOffsetDef && FirstOffsetDef->isShift() &&
        isa<ConstantInt>(FirstOffsetDef->getOperand(1)))
        FirstOffsetDef = dyn_cast<Instruction>(FirstOffsetDef->getOperand(0));

    // Give up if FirstOffsetDef is an Add or Sub with constant.
    // Because it may not profitable at all due to constant folding.
    if (FirstOffsetDef)
        if (BinaryOperator *BO = dyn_cast<BinaryOperator>(FirstOffsetDef))
        {
            unsigned opc = BO->getOpcode();
            if ((opc == Instruction::Add || opc == Instruction::Sub) &&
                (isa<ConstantInt>(BO->getOperand(0)) || isa<ConstantInt>(BO->getOperand(1))))
                return false;
        }
    return true;
}

bool HI_SeparateConstOffsetFromGEP::hasMoreThanOneUseInLoop(Value *V, Loop *L)
{
    int UsesInLoop = 0;
    for (User *U : V->users())
    {
        if (Instruction *User = dyn_cast<Instruction>(U))
            if (L->contains(User))
                if (++UsesInLoop > 1)
                    return true;
    }
    return false;
}

void HI_SeparateConstOffsetFromGEP::swapGEPOperand(GetElementPtrInst *First,
                                                   GetElementPtrInst *Second)
{
    Value *Offset1 = First->getOperand(1);
    Value *Offset2 = Second->getOperand(1);
    First->setOperand(1, Offset2);
    Second->setOperand(1, Offset1);

    // We changed p+o+c to p+c+o, p+c may not be inbound anymore.
    const DataLayout &DAL = First->getModule()->getDataLayout();
    APInt Offset(DAL.getIndexSizeInBits(cast<PointerType>(First->getType())->getAddressSpace()), 0);
    Value *NewBase = First->stripAndAccumulateInBoundsConstantOffsets(DAL, Offset);
    uint64_t ObjectSize;
    if (!getObjectSize(NewBase, ObjectSize, DAL, TLI) || Offset.ugt(ObjectSize))
    {
        First->setIsInBounds(false);
        Second->setIsInBounds(false);
    }
    else
        First->setIsInBounds(true);
}

unsigned int HI_SeparateConstOffsetFromGEP::getLength(Type *TY)
{
    if (ArrayType *VTy = dyn_cast<ArrayType>(TY))
    {
        int numElements = VTy->getNumElements() * getLength(VTy->getElementType());
        return numElements;
    }
    else
    {
        if (DEBUG)
            *Sep_Log << "No ARRAY TYPE: " << *TY << "\n";
        return 1; // HI-MODIFICATION: In HLS, data are not stored in bytes. regard it as one element
    }
}