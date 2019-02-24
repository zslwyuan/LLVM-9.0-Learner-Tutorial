#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Pass.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/raw_ostream.h"
#include "HI_print.h"
#include "HI_VarWidthReduce.h"
#include <stdio.h>
#include <string>
#include <ios>
#include <stdlib.h>

using namespace llvm;

bool HI_VarWidthReduce::runOnFunction(Function &F) // The runOnModule declaration will overide the virtual one in ModulePass, which will be executed for each Module.
{
    const DataLayout &DL = F.getParent()->getDataLayout();
    SE = &getAnalysis<ScalarEvolutionWrapperPass>().getSE();
    LazyValueInfo* LazyI = &getAnalysis<LazyValueInfoWrapperPass>().getLVI();
    
    if (Function_id.find(&F)==Function_id.end())  // traverse functions and assign function ID
    {
        Function_id[&F] = ++Function_Counter;
    }
    bool changed = 0;
    for (BasicBlock &B : F) 
    {
        for (Instruction &I: B) 
        {
            if (I.getType()->isIntegerTy())
            {
                KnownBits tmp_KB = computeKnownBits(&I,DL); 
                const SCEV *tmp_S = SE->getSCEV(&I);
                ConstantRange tmp_CR1 = SE->getSignedRange(tmp_S);
                ConstantRange tmp_CR2 = HI_getSignedRangeRef(tmp_S);
                *VarWidthChangeLog << I << "---- Ori-CR: "<<tmp_CR1 << "(bw=" << I.getType()->getIntegerBitWidth() <<") ---- HI-CR:"<<tmp_CR2 << "(bw=" << bitNeededFor(tmp_CR2) <<")\n\n\n";
            }
        }    
        *VarWidthChangeLog << "\n";
    }
    VarWidthChangeLog->flush(); 
    return changed;
}



char HI_VarWidthReduce::ID = 0;  // the ID for pass should be initialized but the value does not matter, since LLVM uses the address of this variable as label instead of its value.

void HI_VarWidthReduce::getAnalysisUsage(AnalysisUsage &AU) const {
    AU.addRequired<DominatorTreeWrapperPass>();
    AU.addRequired<ScalarEvolutionWrapperPass>();
    AU.addRequired<TargetTransformInfoWrapperPass>();
    AU.addRequired<LoopInfoWrapperPass>();
    AU.addRequired<LazyValueInfoWrapperPass>();    
    AU.setPreservesCFG();
}



/// Determine the range for a particular SCEV.  If SignHint is
/// HINT_RANGE_UNSIGNED (resp. HINT_RANGE_SIGNED) then getRange prefers ranges
/// with a "cleaner" unsigned (resp. signed) representation.
const ConstantRange HI_VarWidthReduce::HI_getSignedRangeRef(const SCEV *S) {

  *VarWidthChangeLog << "        ------  HI_getSignedRangeRef handling SECV: " << *S->getType() << "\n";
  ConstantRange tmp_CR1 = SE->getSignedRange(S);
  if (!tmp_CR1.isFullSet())
  {
    *VarWidthChangeLog << "        ------  HI_getSignedRangeRef: it is not full-set " << tmp_CR1 << "\n";
    return tmp_CR1;
  }
  DenseMap<const SCEV *, ConstantRange> &Cache = SignedRanges;
  *VarWidthChangeLog << "        ------  handling full-set SECV: " << *S->getType() << "\n";
  // See if we've computed this range already.
  DenseMap<const SCEV *, ConstantRange>::iterator I = Cache.find(S);
  if (I != Cache.end())
    return I->second;

  if (const SCEVConstant *C = dyn_cast<SCEVConstant>(S))
    return setRange(C, ConstantRange(C->getAPInt()));

  unsigned BitWidth = SE->getTypeSizeInBits(S->getType());
  ConstantRange ConservativeResult(BitWidth, /*isFullSet=*/true);

  // If the value has known zeros, the maximum value will have those known zeros
  // as well.
  uint32_t TZ = SE->GetMinTrailingZeros(S);
  if (TZ != 0) {
      ConservativeResult = ConstantRange(
          APInt::getSignedMinValue(BitWidth),
          APInt::getSignedMaxValue(BitWidth).ashr(TZ).shl(TZ) + 1);
  }

  if (const SCEVAddExpr *Add = dyn_cast<SCEVAddExpr>(S)) {
    *VarWidthChangeLog << "        ------  Add\n";
    ConstantRange X = HI_getSignedRangeRef(Add->getOperand(0));
    for (unsigned i = 1, e = Add->getNumOperands(); i != e; ++i)
      if (bypassPTI(Add->getOperand(i))) 
        continue;
      else
        X = X.add(HI_getSignedRangeRef(Add->getOperand(i)));
      *VarWidthChangeLog << "            ------  handling full-set SECV new range: " << X << "\n";
    return setRange(Add, ConservativeResult.intersectWith(X));
  }

  if (const SCEVMulExpr *Mul = dyn_cast<SCEVMulExpr>(S)) {
    *VarWidthChangeLog << "        ------  Mul\n";
    ConstantRange X = HI_getSignedRangeRef(Mul->getOperand(0));
    for (unsigned i = 1, e = Mul->getNumOperands(); i != e; ++i)
      if (bypassPTI(Mul->getOperand(i))) 
        continue; 
      else
        X = X.multiply(HI_getSignedRangeRef(Mul->getOperand(i)));
      *VarWidthChangeLog << "            ------  handling full-set SECV new range: " << X << "\n";
    return setRange(Mul, ConservativeResult.intersectWith(X));
  }

  if (const SCEVSMaxExpr *SMax = dyn_cast<SCEVSMaxExpr>(S)) {
    *VarWidthChangeLog << "        ------  SMax\n";
    ConstantRange X = HI_getSignedRangeRef(SMax->getOperand(0));
    for (unsigned i = 1, e = SMax->getNumOperands(); i != e; ++i)
      if (bypassPTI(SMax->getOperand(i))) 
        continue;
      else
        X = X.smax(HI_getSignedRangeRef(SMax->getOperand(i)));
      *VarWidthChangeLog << "          ------  handling full-set SECV new range: " << X << "\n";
    return setRange(SMax, ConservativeResult.intersectWith(X));
  }

  if (const SCEVUMaxExpr *UMax = dyn_cast<SCEVUMaxExpr>(S)) {
    *VarWidthChangeLog << "        ------  UMax\n";
    ConstantRange X = HI_getSignedRangeRef(UMax->getOperand(0));
    for (unsigned i = 1, e = UMax->getNumOperands(); i != e; ++i)
      if (bypassPTI(UMax->getOperand(i))) 
        continue;
      else
        X = X.umax(HI_getSignedRangeRef(UMax->getOperand(i)));
      *VarWidthChangeLog << "          ------  handling full-set SECV new range: " << X << "\n";
    return setRange(UMax, ConservativeResult.intersectWith(X));
  }

  if (const SCEVUDivExpr *UDiv = dyn_cast<SCEVUDivExpr>(S)) {
    *VarWidthChangeLog << "        ------  UDiv\n";
    if (bypassPTI(UDiv->getLHS())) return ConservativeResult;
    if (bypassPTI(UDiv->getRHS())) return ConservativeResult;
    ConstantRange X = HI_getSignedRangeRef(UDiv->getLHS());
    ConstantRange Y = HI_getSignedRangeRef(UDiv->getRHS());
    return setRange(UDiv,
                    ConservativeResult.intersectWith(X.udiv(Y)));
  }

  if (const SCEVZeroExtendExpr *ZExt = dyn_cast<SCEVZeroExtendExpr>(S)) {
    *VarWidthChangeLog << "        ------  ZExt\n";
    if (bypassPTI(ZExt->getOperand())) return ConservativeResult;
    ConstantRange X = HI_getSignedRangeRef(ZExt->getOperand());
    return setRange(ZExt,
                    ConservativeResult.intersectWith(X.zeroExtend(BitWidth)));
  }

  if (const SCEVSignExtendExpr *SExt = dyn_cast<SCEVSignExtendExpr>(S)) {
    *VarWidthChangeLog << "        ------  SExt\n";
    if (bypassPTI(SExt->getOperand())) return ConservativeResult;
    ConstantRange X = HI_getSignedRangeRef(SExt->getOperand());
    return setRange(SExt,
                    ConservativeResult.intersectWith(X.signExtend(BitWidth)));
  }

  if (const SCEVTruncateExpr *Trunc = dyn_cast<SCEVTruncateExpr>(S)) {
    *VarWidthChangeLog << "        ------  Trunc\n";
    if (bypassPTI(Trunc->getOperand())) return ConservativeResult;
    ConstantRange X = HI_getSignedRangeRef(Trunc->getOperand());
    return setRange(Trunc,
                    ConservativeResult.intersectWith(X.truncate(BitWidth)));
  }

  if (const SCEVAddRecExpr *AddRec = dyn_cast<SCEVAddRecExpr>(S)) {
    *VarWidthChangeLog << "        ------  SCEVAddRecExpr\n";
  //   // If there's no unsigned wrap, the value will never be less than its
  //   // initial value.
  //   if (AddRec->hasNoUnsignedWrap())
  //     if (const SCEVConstant *C = dyn_cast<SCEVConstant>(AddRec->getStart()))
  //       if (!C->getValue()->isZero())
  //         ConservativeResult = ConservativeResult.intersectWith(
  //             ConstantRange(C->getAPInt(), APInt(BitWidth, 0)));

  //   // If there's no signed wrap, and all the operands have the same sign or
  //   // zero, the value won't ever change sign.
  //   if (AddRec->hasNoSignedWrap()) {
  //     bool AllNonNeg = true;
  //     bool AllNonPos = true;
  //     for (unsigned i = 0, e = AddRec->getNumOperands(); i != e; ++i) {
  //       if (!SE->isKnownNonNegative(AddRec->getOperand(i))) AllNonNeg = false;
  //       if (!SE->isKnownNonPositive(AddRec->getOperand(i))) AllNonPos = false;
  //     }
  //     if (AllNonNeg)
  //       ConservativeResult = ConservativeResult.intersectWith(
  //         ConstantRange(APInt(BitWidth, 0),
  //                       APInt::getSignedMinValue(BitWidth)));
  //     else if (AllNonPos)
  //       ConservativeResult = ConservativeResult.intersectWith(
  //         ConstantRange(APInt::getSignedMinValue(BitWidth),
  //                       APInt(BitWidth, 1)));
  //   }

  //   // TODO: non-affine addrec
  //   if (AddRec->isAffine()) {
  //     const SCEV *MaxBECount = SE->getMaxBackedgeTakenCount(AddRec->getLoop());
  //     if (!isa<SCEVCouldNotCompute>(MaxBECount) &&
  //         SE->getTypeSizeInBits(MaxBECount->getType()) <= BitWidth) {
  //       auto RangeFromAffine = SE->getRangeForAffineAR(
  //           AddRec->getStart(), AddRec->getStepRecurrence(*SE), MaxBECount,
  //           BitWidth);
  //       if (!RangeFromAffine.isFullSet())
  //         ConservativeResult =
  //             ConservativeResult.intersectWith(RangeFromAffine);

  //       auto RangeFromFactoring = SE->getRangeViaFactoring(
  //           AddRec->getStart(), AddRec->getStepRecurrence(*SE), MaxBECount,
  //           BitWidth);
  //       if (!RangeFromFactoring.isFullSet())
  //         ConservativeResult =
  //             ConservativeResult.intersectWith(RangeFromFactoring);
  //     }
  //   }

  //   return setRange(AddRec, std::move(ConservativeResult));
  }

  if (const SCEVUnknown *U = dyn_cast<SCEVUnknown>(S)) {
    *VarWidthChangeLog << "        ------  SCEVUnknown\n";
    if (PtrToIntInst *PTI = dyn_cast<PtrToIntInst>(U->getValue()))
    {   
      // for (unsigned i = 0, e = PTI->getNumOperands(); i != e; ++i) 
      // {
      //   *VarWidthChangeLog << "        ------  SCEVUnknown   " << *(PTI->getOperand(i))  << "\n";
        
      // }
      // ConstantRange ConservativeResult_empty(BitWidth, /*isFullSet=*/false);
      // return setRange(U, std::move(ConservativeResult_empty));
        // return 0.0;
    }
  //   // Check if the IR explicitly contains !range metadata.
  //   Optional<ConstantRange> MDRange = GetRangeFromMetadata(U->getValue());
  //   if (MDRange.hasValue())
  //     ConservativeResult = ConservativeResult.intersectWith(MDRange.getValue());

  //   // Split here to avoid paying the compile-time cost of calling both
  //   // computeKnownBits and ComputeNumSignBits.  This restriction can be lifted
  //   // if needed.
  //   const DataLayout &DL = getDataLayout();
  //   if (SignHint == ScalarEvolution::HINT_RANGE_UNSIGNED) {
  //     // For a SCEVUnknown, ask ValueTracking.
  //     KnownBits Known = computeKnownBits(U->getValue(), DL, 0, &AC, nullptr, &DT);
  //     if (Known.One != ~Known.Zero + 1)
  //       ConservativeResult =
  //           ConservativeResult.intersectWith(ConstantRange(Known.One,
  //                                                          ~Known.Zero + 1));
  //   } else {
  //     assert(SignHint == ScalarEvolution::HINT_RANGE_SIGNED &&
  //            "generalize as needed!");
  //     unsigned NS = ComputeNumSignBits(U->getValue(), DL, 0, &AC, nullptr, &DT);
  //     if (NS > 1)
  //       ConservativeResult = ConservativeResult.intersectWith(
  //           ConstantRange(APInt::getSignedMinValue(BitWidth).ashr(NS - 1),
  //                         APInt::getSignedMaxValue(BitWidth).ashr(NS - 1) + 1));
  //   }

  //   // A range of Phi is a subset of union of all ranges of its input.
  //   if (const PHINode *Phi = dyn_cast<PHINode>(U->getValue())) {
  //     // Make sure that we do not run over cycled Phis.
  //     if (PendingPhiRanges.insert(Phi).second) {
  //       ConstantRange RangeFromOps(BitWidth, /*isFullSet=*/false);
  //       for (auto &Op : Phi->operands()) {
  //         auto OpRange = SE->getSignedRange(getSCEV(Op));
  //         RangeFromOps = RangeFromOps.unionWith(OpRange);
  //         // No point to continue if we already have a full set.
  //         if (RangeFromOps.isFullSet())
  //           break;
  //       }
  //       ConservativeResult = ConservativeResult.intersectWith(RangeFromOps);
  //       bool Erased = PendingPhiRanges.erase(Phi);
  //       assert(Erased && "Failed to erase Phi properly?");
  //       (void) Erased;
  //     }
  //   }

  //   return setRange(U, std::move(ConservativeResult));
  }
  *VarWidthChangeLog << "        ------  Out of Scope\n";
  return setRange(S, std::move(ConservativeResult));
}

// static Optional<ConstantRange> HI_VarWidthReduce::GetRangeFromMetadata(Value *V) 
// {
//   if (Instruction *I = dyn_cast<Instruction>(V))
//     if (MDNode *MD = I->getMetadata(LLVMContext::MD_range))
//       return getConstantRangeFromMetadata(*MD);

//   return None;
// }

const ConstantRange &HI_VarWidthReduce::setRange(const SCEV *S,  ConstantRange CR) 
{
  DenseMap<const SCEV *, ConstantRange> &Cache = SignedRanges;

  auto Pair = Cache.try_emplace(S, std::move(CR));
  if (!Pair.second)
    Pair.first->second = std::move(CR);
  return Pair.first->second;
}

bool HI_VarWidthReduce::bypassPTI(const SCEV *S)
{
      if (const SCEVUnknown *U = dyn_cast<SCEVUnknown>(S))
        if (PtrToIntInst *PTI = dyn_cast<PtrToIntInst>(U->getValue()))
        {
          *VarWidthChangeLog << "            ------  bypassing range evaluation for PtrToIntInst: " << *U->getValue() << "\n";
          return true;
        }
      return false;
}

unsigned int HI_VarWidthReduce::bitNeededFor(ConstantRange CR)
{
    unsigned int lowerNeedBits = CR.getLower().getActiveBits();
    unsigned int upperNeedBits = CR.getUpper().getActiveBits();
    if (lowerNeedBits > upperNeedBits) 
      return lowerNeedBits;
    else
      return upperNeedBits;
}