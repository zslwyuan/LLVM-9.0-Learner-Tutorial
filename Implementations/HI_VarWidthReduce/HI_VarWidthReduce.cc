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
    ScalarEvolution  *SE = &getAnalysis<ScalarEvolutionWrapperPass>().getSE();
    LazyValueInfo* LazyI = &getAnalysis<LazyValueInfoWrapperPass>().getLVI();
    
    if (Function_id.find(&F)==Function_id.end())  // traverse functions and assign function ID
    {
        Function_id[&F] = ++Function_Counter;
    }
    bool removed = 0;
    for (BasicBlock &B : F) 
    {
        for (Instruction &I: B) 
        {
            if (I.getType()->isIntegerTy())
            {
                ConstantRange tmp_CR0 = LazyI->getConstantRange(&I,&B);
                KnownBits tmp_KB = computeKnownBits(&I,DL); 
                const SCEV *tmp_S = SE->getSCEV(&I);
                ConstantRange tmp_CR1 = SE->getSignedRange(tmp_S);
                *VarWidthChangeLog << I << "----" << tmp_CR0 <<" ----- MinLeadingZeros = "<<tmp_KB.getBitWidth() <<"----"<<tmp_CR1 <<"\n";
            }
        }    
        *VarWidthChangeLog << "\n";
    }
    VarWidthChangeLog->flush(); 
    return removed;
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

Instruction* HI_VarWidthReduce::checkDuplicationInBlock(BasicBlock *B, Instruction *I)
{
    if (BranchInst *tmpI = dyn_cast<BranchInst>(I))
        return nullptr;

    for (Instruction &tmpII: *B) 
    {
        Instruction *tmpI = &tmpII;
        if ( tmpI != I )
        {
            // check if they are the same

            bool sameOpcode = tmpI->getOpcode() == I->getOpcode();
            bool sameOperands = 1;
            bool PHI_SameIncome = 1;
            if (tmpI->getNumOperands()!=I->getNumOperands() || !sameOpcode)  sameOperands = 0;
            else
            {
                if (PHINode *tmpI0 = dyn_cast<PHINode>(tmpI))
                {
                    // if it is a PHI Node, check the field too.
                    PHINode *tmpI1 = dyn_cast<PHINode>(I);
                    for (int i = 0; i <tmpI->getNumOperands(); ++i)
                    {
                        sameOperands &= (tmpI->getOperand(i) == I->getOperand(i));
                        sameOperands &= tmpI0->getIncomingBlock(i) == tmpI1->getIncomingBlock(i);
                    }
                }
                else
                {
                    for (int i = 0; i <tmpI->getNumOperands(); ++i)
                    {
                        sameOperands &= (tmpI->getOperand(i) == I->getOperand(i));
                    }
                }    
            }           
            if (sameOpcode && sameOperands)           
                return tmpI;           
        }
    }
    return nullptr;
}