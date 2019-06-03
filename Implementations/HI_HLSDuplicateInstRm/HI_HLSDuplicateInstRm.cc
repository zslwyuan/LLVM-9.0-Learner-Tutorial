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
#include "HI_HLSDuplicateInstRm.h"
#include <stdio.h>
#include <string>
#include <ios>
#include <stdlib.h>

using namespace llvm;
 
bool HI_HLSDuplicateInstRm::runOnFunction(Function &F) // The runOnModule declaration will overide the virtual one in ModulePass, which will be executed for each Module.
{
    if (Function_id.find(&F)==Function_id.end())  // traverse functions and assign function ID
    {
        Function_id[&F] = ++Function_Counter;
    }
    bool removed = 0;
    for (BasicBlock &B : F) 
    {
        bool duplicationInBlock = 1;
        while (duplicationInBlock)
        {
            duplicationInBlock = 0;
            for (Instruction &I: B) 
            {
                Instruction *dupI = checkDuplicationInBlock(&B,&I);
                if ( dupI != nullptr)
                {
                    if (dupI->getType() != I.getType())
                        continue;
                    *RemoveLog <<"duplicated: " << I << " -------  " << *dupI <<"\n";
                    *RemoveLog <<"Remove: " << *dupI <<"\n";
                    *RemoveLog <<"Block: \n" <<  B <<"\n";
                    RemoveLog->flush(); 
                    dupI->replaceAllUsesWith(&I);
                    dupI->eraseFromParent();
                    duplicationInBlock = 1;
                    removed = 1;
                    break; // remove the instruction and start the search again, until no duplication
                }
            }
        }

    }
    RemoveLog->flush(); 
    return removed;
}



char HI_HLSDuplicateInstRm::ID = 0;  // the ID for pass should be initialized but the value does not matter, since LLVM uses the address of this variable as label instead of its value.

void HI_HLSDuplicateInstRm::getAnalysisUsage(AnalysisUsage &AU) const {
    AU.addRequired<DominatorTreeWrapperPass>();
    AU.addRequired<ScalarEvolutionWrapperPass>();
    AU.addRequired<TargetTransformInfoWrapperPass>();
    AU.addRequired<LoopInfoWrapperPass>();
    AU.setPreservesCFG();
}

Instruction* HI_HLSDuplicateInstRm::checkDuplicationInBlock(BasicBlock *B, Instruction *I)
{
    if (BranchInst *tmpI = dyn_cast<BranchInst>(I))
        return nullptr;

    // the Mul and Add might end up with MAC, don't rm them
    if (MulOperator *tmpI = dyn_cast<MulOperator>(I))
    {
        for (auto it = I->use_begin(), ie=I->use_end(); it!=ie; ++it)
        {
            if (AddOperator *Add_I = dyn_cast<AddOperator>(it->getUser()))
            {
                return nullptr;
            }
        }
    }

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