//===- HI_SeparateConstOffsetFromGEP.cpp -------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

// Modification is to change the alignment of arrayType address space form byte to element.
// Reader can track the modification by the flag : HI-MODIFICATION

#ifndef _HI_SeparateConstOffsetFromGEP
#define _HI_SeparateConstOffsetFromGEP

#include "HI_print.h"
#include "llvm/ADT/APInt.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/DepthFirstIterator.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/Analysis/LazyValueInfo.h"
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
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PatternMatch.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/User.h"
#include "llvm/IR/Value.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Pass.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Utils/Local.h"
#include <cassert>
#include <cstdint>
#include <ios>
#include <map>
#include <set>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sys/time.h>
#include <vector>

#include "HI_print.h"

using namespace llvm;
using namespace llvm::PatternMatch;

/// A helper class for separating a constant offset from a GEP index.
///
/// In real programs, a GEP index may be more complicated than a simple addition
/// of something and a constant integer which can be trivially splitted. For
/// example, to split ((a << 3) | 5) + b, we need to search deeper for the
/// constant offset, so that we can separate the index to (a << 3) + b and 5.
///
/// Therefore, this class looks into the expression that computes a given GEP
/// index, and tries to find a constant integer that can be hoisted to the
/// outermost level of the expression as an addition. Not every constant in an
/// expression can jump out. e.g., we cannot transform (b * (a + 5)) to (b * a +
/// 5); nor can we transform (3 * (a + 5)) to (3 * a + 5), however in this case,
/// -instcombine probably already optimized (3 * (a + 5)) to (3 * a + 15).
class HI_ConstantOffsetExtractor
{
  public:
    /// Extracts a constant offset from the given GEP index. It returns the
    /// new index representing the remainder (equal to the original index minus
    /// the constant offset), or nullptr if we cannot extract a constant offset.
    /// \p Idx The given GEP index
    /// \p GEP The given GEP
    /// \p UserChainTail Outputs the tail of UserChain so that we can
    ///                  garbage-collect unused instructions in UserChain.
    static Value *Extract(Value *Idx, GetElementPtrInst *GEP, User *&UserChainTail,
                          const DominatorTree *DT);

    /// Looks for a constant offset from the given GEP index without extracting
    /// it. It returns the numeric value of the extracted constant offset (0 if
    /// failed). The meaning of the arguments are the same as Extract.
    static int64_t Find(Value *Idx, GetElementPtrInst *GEP, const DominatorTree *DT);

  private:
    HI_ConstantOffsetExtractor(Instruction *InsertionPt, const DominatorTree *DT)
        : IP(InsertionPt), DL(InsertionPt->getModule()->getDataLayout()), DT(DT)
    {
    }

    /// Searches the expression that computes V for a non-zero constant C s.t.
    /// V can be reassociated into the form V' + C. If the searching is
    /// successful, returns C and update UserChain as a def-use chain from C to V;
    /// otherwise, UserChain is empty.
    ///
    /// \p V            The given expression
    /// \p SignExtended Whether V will be sign-extended in the computation of the
    ///                 GEP index
    /// \p ZeroExtended Whether V will be zero-extended in the computation of the
    ///                 GEP index
    /// \p NonNegative  Whether V is guaranteed to be non-negative. For example,
    ///                 an index of an inbounds GEP is guaranteed to be
    ///                 non-negative. Levaraging this, we can better split
    ///                 inbounds GEPs.
    APInt find(Value *V, bool SignExtended, bool ZeroExtended, bool NonNegative);

    /// A helper function to look into both operands of a binary operator.
    APInt findInEitherOperand(BinaryOperator *BO, bool SignExtended, bool ZeroExtended);

    /// After finding the constant offset C from the GEP index I, we build a new
    /// index I' s.t. I' + C = I. This function builds and returns the new
    /// index I' according to UserChain produced by function "find".
    ///
    /// The building conceptually takes two steps:
    /// 1) iteratively distribute s/zext towards the leaves of the expression tree
    /// that computes I
    /// 2) reassociate the expression tree to the form I' + C.
    ///
    /// For example, to extract the 5 from sext(a + (b + 5)), we first distribute
    /// sext to a, b and 5 so that we have
    ///   sext(a) + (sext(b) + 5).
    /// Then, we reassociate it to
    ///   (sext(a) + sext(b)) + 5.
    /// Given this form, we know I' is sext(a) + sext(b).
    Value *rebuildWithoutConstOffset();

    /// After the first step of rebuilding the GEP index without the constant
    /// offset, distribute s/zext to the operands of all operators in UserChain.
    /// e.g., zext(sext(a + (b + 5)) (assuming no overflow) =>
    /// zext(sext(a)) + (zext(sext(b)) + zext(sext(5))).
    ///
    /// The function also updates UserChain to point to new subexpressions after
    /// distributing s/zext. e.g., the old UserChain of the above example is
    /// 5 -> b + 5 -> a + (b + 5) -> sext(...) -> zext(sext(...)),
    /// and the new UserChain is
    /// zext(sext(5)) -> zext(sext(b)) + zext(sext(5)) ->
    ///   zext(sext(a)) + (zext(sext(b)) + zext(sext(5))
    ///
    /// \p ChainIndex The index to UserChain. ChainIndex is initially
    ///               UserChain.size() - 1, and is decremented during
    ///               the recursion.
    Value *distributeExtsAndCloneChain(unsigned ChainIndex);

    /// Reassociates the GEP index to the form I' + C and returns I'.
    Value *removeConstOffset(unsigned ChainIndex);

    /// A helper function to apply ExtInsts, a list of s/zext, to value V.
    /// e.g., if ExtInsts = [sext i32 to i64, zext i16 to i32], this function
    /// returns "sext i32 (zext i16 V to i32) to i64".
    Value *applyExts(Value *V);

    /// A helper function that returns whether we can trace into the operands
    /// of binary operator BO for a constant offset.
    ///
    /// \p SignExtended Whether BO is surrounded by sext
    /// \p ZeroExtended Whether BO is surrounded by zext
    /// \p NonNegative Whether BO is known to be non-negative, e.g., an in-bound
    ///                array index.
    bool CanTraceInto(bool SignExtended, bool ZeroExtended, BinaryOperator *BO, bool NonNegative);

    /// The path from the constant offset to the old GEP index. e.g., if the GEP
    /// index is "a * b + (c + 5)". After running function find, UserChain[0] will
    /// be the constant 5, UserChain[1] will be the subexpression "c + 5", and
    /// UserChain[2] will be the entire expression "a * b + (c + 5)".
    ///
    /// This path helps to rebuild the new GEP index.
    SmallVector<User *, 8> UserChain;

    /// A data structure used in rebuildWithoutConstOffset. Contains all
    /// sext/zext instructions along UserChain.
    SmallVector<CastInst *, 16> ExtInsts;

    /// Insertion position of cloned instructions.
    Instruction *IP;

    const DataLayout &DL;
    const DominatorTree *DT;
};

/// A pass that tries to split every GEP in the function into a variadic
/// base and a constant offset. It is a FunctionPass because searching for the
/// constant offset may inspect other basic blocks.
class HI_SeparateConstOffsetFromGEP : public FunctionPass
{
  public:
    static char ID;

    HI_SeparateConstOffsetFromGEP(const char *Sep_Log_Name, bool LowerGEP = false, bool DEBUG = 0)
        : FunctionPass(ID), LowerGEP(LowerGEP), DEBUG(DEBUG)
    {
        Sep_Log = new raw_fd_ostream(Sep_Log_Name, ErrInfo, sys::fs::F_None);
    }
    ~HI_SeparateConstOffsetFromGEP()
    {
        Sep_Log->flush();
        delete Sep_Log;
    }

    void getAnalysisUsage(AnalysisUsage &AU) const override
    {
        AU.addRequired<DominatorTreeWrapperPass>();
        AU.addRequired<ScalarEvolutionWrapperPass>();
        AU.addRequired<TargetTransformInfoWrapperPass>();
        AU.addRequired<LoopInfoWrapperPass>();
        AU.setPreservesCFG();
        AU.addRequired<TargetLibraryInfoWrapperPass>();
    }

    bool doInitialization(Module &M) override
    {
        DL = &M.getDataLayout();
        return false;
    }

    bool runOnFunction(Function &F) override;

  private:
    bool DEBUG;
    /// Tries to split the given GEP into a variadic base and a constant offset,
    /// and returns true if the splitting succeeds.
    bool splitGEP(GetElementPtrInst *GEP);

    /// Lower a GEP with multiple indices into multiple GEPs with a single index.
    /// Function splitGEP already split the original GEP into a variadic part and
    /// a constant offset (i.e., AccumulativeByteOffset). This function lowers the
    /// variadic part into a set of GEPs with a single index and applies
    /// AccumulativeByteOffset to it.
    /// \p Variadic                  The variadic part of the original GEP.
    /// \p AccumulativeByteOffset    The constant offset.
    void lowerToSingleIndexGEPs(GetElementPtrInst *Variadic, int64_t AccumulativeByteOffset);

    /// Lower a GEP with multiple indices into ptrtoint+arithmetics+inttoptr form.
    /// Function splitGEP already split the original GEP into a variadic part and
    /// a constant offset (i.e., AccumulativeByteOffset). This function lowers the
    /// variadic part into a set of arithmetic operations and applies
    /// AccumulativeByteOffset to it.
    /// \p Variadic                  The variadic part of the original GEP.
    /// \p AccumulativeByteOffset    The constant offset.
    void lowerToArithmetics(GetElementPtrInst *Variadic, int64_t AccumulativeByteOffset);

    /// Finds the constant offset within each index and accumulates them. If
    /// LowerGEP is true, it finds in indices of both sequential and structure
    /// types, otherwise it only finds in sequential indices. The output
    /// NeedsExtraction indicates whether we successfully find a non-zero constant
    /// offset.
    int64_t accumulateByteOffset(GetElementPtrInst *GEP, bool &NeedsExtraction);

    /// Canonicalize array indices to pointer-size integers. This helps to
    /// simplify the logic of splitting a GEP. For example, if a + b is a
    /// pointer-size integer, we have
    ///   gep base, a + b = gep (gep base, a), b
    /// However, this equality may not hold if the size of a + b is smaller than
    /// the pointer size, because LLVM conceptually sign-extends GEP indices to
    /// pointer size before computing the address
    /// (http://llvm.org/docs/LangRef.html#id181).
    ///
    /// This canonicalization is very likely already done in clang and
    /// instcombine. Therefore, the program will probably remain the same.
    ///
    /// Returns true if the module changes.
    ///
    /// Verified in @i32_add in split-gep.ll
    bool canonicalizeArrayIndicesToPointerSize(GetElementPtrInst *GEP);

    /// Optimize sext(a)+sext(b) to sext(a+b) when a+b can't sign overflow.
    /// HI_SeparateConstOffsetFromGEP distributes a sext to leaves before extracting
    /// the constant offset. After extraction, it becomes desirable to reunion the
    /// distributed sexts. For example,
    ///
    ///                              &a[sext(i +nsw (j +nsw 5)]
    ///   => distribute              &a[sext(i) +nsw (sext(j) +nsw 5)]
    ///   => constant extraction     &a[sext(i) + sext(j)] + 5
    ///   => reunion                 &a[sext(i +nsw j)] + 5
    bool reuniteExts(Function &F);

    /// A helper that reunites sexts in an instruction.
    bool reuniteExts(Instruction *I);

    /// Find the closest dominator of <Dominatee> that is equivalent to <Key>.
    Instruction *findClosestMatchingDominator(const SCEV *Key, Instruction *Dominatee);
    /// Verify F is free of dead code.
    void verifyNoDeadCode(Function &F);

    bool hasMoreThanOneUseInLoop(Value *v, Loop *L);

    // Swap the index operand of two GEP.
    void swapGEPOperand(GetElementPtrInst *First, GetElementPtrInst *Second);

    // Check if it is safe to swap operand of two GEP.
    bool isLegalToSwapOperand(GetElementPtrInst *First, GetElementPtrInst *Second, Loop *CurLoop);

    // Get the length of a Type which is aligned by element. (Array [100 x i32] --> length = 100;
    // i32 ---> length = 1)
    unsigned int getLength(Type *TY);

    const DataLayout *DL = nullptr;
    DominatorTree *DT = nullptr;
    ScalarEvolution *SE;

    LoopInfo *LI;
    TargetLibraryInfo *TLI;

    /// Whether to lower a GEP with multiple indices into arithmetic operations or
    /// multiple GEPs with a single index.
    bool LowerGEP;

    DenseMap<const SCEV *, SmallVector<Instruction *, 2>> DominatingExprs;

    // record the information of the processing
    raw_ostream *Sep_Log;
    std::error_code ErrInfo;

    /// Timer

    struct timeval tv_begin;
    struct timeval tv_end;
};

#endif