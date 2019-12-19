#include "HI_WithDirectiveTimingResourceEvaluation.h"
#include "HI_print.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Pass.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/raw_ostream.h"

#include <ios>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
using namespace llvm;

//   /// Return a SCEV expression for the full generality of the specified
//   /// expression.
//   const SCEV *getSCEV(Value *V);

//   const SCEV *getConstant(ConstantInt *V);
//   const SCEV *getConstant(const APInt &Val);
//   const SCEV *getConstant(Type *Ty, uint64_t V, bool isSigned = false);
//   const SCEV *getTruncateExpr(const SCEV *Op, Type *Ty);
//   const SCEV *getZeroExtendExpr(const SCEV *Op, Type *Ty, unsigned Depth = 0);
//   const SCEV *getSignExtendExpr(const SCEV *Op, Type *Ty, unsigned Depth = 0);
//   const SCEV *getAnyExtendExpr(const SCEV *Op, Type *Ty);
//   const SCEV *getAddExpr(SmallVectorImpl<const SCEV *> &Ops,
//                          SCEV::NoWrapFlags Flags = SCEV::FlagAnyWrap,
//                          unsigned Depth = 0);
//   const SCEV *getAddExpr(const SCEV *LHS, const SCEV *RHS,
//                          SCEV::NoWrapFlags Flags = SCEV::FlagAnyWrap,
//                          unsigned Depth = 0) {
//     SmallVector<const SCEV *, 2> Ops = {LHS, RHS};
//     return getAddExpr(Ops, Flags, Depth);
//   }
//   const SCEV *getAddExpr(const SCEV *Op0, const SCEV *Op1, const SCEV *Op2,
//                          SCEV::NoWrapFlags Flags = SCEV::FlagAnyWrap,
//                          unsigned Depth = 0) {
//     SmallVector<const SCEV *, 3> Ops = {Op0, Op1, Op2};
//     return getAddExpr(Ops, Flags, Depth);
//   }
//   const SCEV *getMulExpr(SmallVectorImpl<const SCEV *> &Ops,
//                          SCEV::NoWrapFlags Flags = SCEV::FlagAnyWrap,
//                          unsigned Depth = 0);
//   const SCEV *getMulExpr(const SCEV *LHS, const SCEV *RHS,
//                          SCEV::NoWrapFlags Flags = SCEV::FlagAnyWrap,
//                          unsigned Depth = 0) {
//     SmallVector<const SCEV *, 2> Ops = {LHS, RHS};
//     return getMulExpr(Ops, Flags, Depth);
//   }
//   const SCEV *getMulExpr(const SCEV *Op0, const SCEV *Op1, const SCEV *Op2,
//                          SCEV::NoWrapFlags Flags = SCEV::FlagAnyWrap,
//                          unsigned Depth = 0) {
//     SmallVector<const SCEV *, 3> Ops = {Op0, Op1, Op2};
//     return getMulExpr(Ops, Flags, Depth);
//   }
//   const SCEV *getUDivExpr(const SCEV *LHS, const SCEV *RHS);
//   const SCEV *getUDivExactExpr(const SCEV *LHS, const SCEV *RHS);
//   const SCEV *getURemExpr(const SCEV *LHS, const SCEV *RHS);
//   const SCEV *getAddRecExpr(const SCEV *Start, const SCEV *Step, const Loop *L,
//                             SCEV::NoWrapFlags Flags);
//   const SCEV *getAddRecExpr(SmallVectorImpl<const SCEV *> &Operands,
//                             const Loop *L, SCEV::NoWrapFlags Flags);
//   const SCEV *getAddRecExpr(const SmallVectorImpl<const SCEV *> &Operands,
//                             const Loop *L, SCEV::NoWrapFlags Flags) {
//     SmallVector<const SCEV *, 4> NewOp(Operands.begin(), Operands.end());
//     return getAddRecExpr(NewOp, L, Flags);
//   }

// get the unknown values in the expression
int HI_WithDirectiveTimingResourceEvaluation::getUnknownNum(const SCEV *ori_inputS)
{
    int res = 0;
    const SCEV *inputS = bypassExtTruntSCEV(ori_inputS);
    const SCEVNAryExpr *naryS = dyn_cast<SCEVNAryExpr>(inputS);
    const SCEVUDivExpr *divS = dyn_cast<SCEVUDivExpr>(inputS);
    if (naryS)
    {
        for (int i = 0; i < naryS->getNumOperands(); i++)
        {
            res += getUnknownNum(naryS->getOperand(i));
        }
    }
    else if (divS)
    {
        res += getUnknownNum(divS->getLHS());
        res += getUnknownNum(divS->getRHS());
    }
    else if (auto unknown = dyn_cast<SCEVUnknown>(inputS))
    {
        res = 1;
    }

    return res;
}

const SCEV *HI_WithDirectiveTimingResourceEvaluation::findUnknown(const SCEV *ori_inputS)
{
    const SCEV *inputS = bypassExtTruntSCEV(ori_inputS);
    const SCEVNAryExpr *naryS = dyn_cast<SCEVNAryExpr>(inputS);
    const SCEVUDivExpr *divS = dyn_cast<SCEVUDivExpr>(inputS);
    if (naryS)
    {
        for (int i = 0; i < naryS->getNumOperands(); i++)
        {
            const SCEV *tmp = findUnknown(naryS->getOperand(i));
            if (tmp)
                return tmp;
        }
    }
    else if (divS)
    {
        const SCEV *tmp0 = findUnknown(divS->getLHS());
        if (tmp0)
            return tmp0;
        const SCEV *tmp1 = findUnknown(divS->getRHS());
        if (tmp1)
            return tmp1;
    }
    else if (auto unknown = dyn_cast<SCEVUnknown>(inputS))
    {
        return unknown;
    }
    else
    {
        return nullptr;
    }
}

const SCEV *
HI_WithDirectiveTimingResourceEvaluation::tryGetPureAddOrAddRecSCEV(const SCEV *ori_inputS,
                                                                    int depth)
{
    if (DEBUG)
        *ArrayLog << "      try to tryGetPureAddOrAddRecSCEV: " << *ori_inputS << "\n";
    const SCEV *inputS = bypassExtTruntSCEV(ori_inputS);
    if (DEBUG)
        *ArrayLog << "      get bypassExtTruntSCEV: " << *inputS << "\n";
    if (DEBUG)
        ArrayLog->flush();

    if (auto addSCEV = dyn_cast<SCEVAddExpr>(inputS))
    {
        if (DEBUG)
            *ArrayLog << "      " << *inputS << " is SCEVAddExpr\n";
        // SmallVectorImpl<const SCEV *> Ops();
        std::vector<const SCEV *> Ops;
        SmallVector<const SCEV *, 2> SmallVec_Ops;
        SmallVec_Ops.clear();
        SmallVec_Ops.resize(addSCEV->getNumOperands());
        for (int i = 0; i < addSCEV->getNumOperands(); i++)
        {
            const SCEV *opSCEV = addSCEV->getOperand(i);
            const SCEV *clear_opSCEV = tryGetPureAddOrAddRecSCEV(opSCEV, depth + 1);
            if (!clear_opSCEV)
                return nullptr;
            // SmallVec_Ops.a
            // Ops.push_back(clear_opSCEV);
            if (DEBUG)
                *ArrayLog << "           " << *inputS
                          << " get bypassExtTruntSCEV op: " << *clear_opSCEV << "\n";
            if (DEBUG)
                ArrayLog->flush();
            SmallVec_Ops.push_back(clear_opSCEV);
        }
        return SE->getAddExpr(SmallVec_Ops, SCEV::FlagAnyWrap, depth + 1);
    }
    else if (auto addrecSCEV = dyn_cast<SCEVAddRecExpr>(inputS))
    {
        if (DEBUG)
            *ArrayLog << "      " << *inputS << " is SCEVAddRecExpr\n";
        const SCEV *start = tryGetPureAddOrAddRecSCEV(addrecSCEV->getStart(), depth + 1);
        const SCEV *step = tryGetPureAddOrAddRecSCEV(addrecSCEV->getStepRecurrence(*SE), depth + 1);
        if (start && step)
        {
            if (DEBUG)
                *ArrayLog << "           " << *inputS
                          << "get bypassExtTruntSCEV startop: " << *start << "\n";
            if (DEBUG)
                *ArrayLog << "           " << *inputS << "get bypassExtTruntSCEV stepop: " << *step
                          << "\n";
            if (DEBUG)
                ArrayLog->flush();
            return SE->getAddRecExpr(start, step, addrecSCEV->getLoop(), SCEV::FlagAnyWrap);
        }
        else
            return nullptr;
    }
    else if (auto unknown = dyn_cast<SCEVUnknown>(inputS))
    {
        return inputS;
    }
}

const SCEV *HI_WithDirectiveTimingResourceEvaluation::bypassExtTruntSCEV(const SCEV *inputS)
{
    if (auto castSCEV = dyn_cast<SCEVCastExpr>(inputS))
        return bypassExtTruntSCEV(castSCEV->getOperand());
    else
        return inputS;
}