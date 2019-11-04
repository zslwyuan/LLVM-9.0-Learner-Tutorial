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
    print_status("Running HI_HLSDuplicateInstRm pass."); 
    if (F.getName().find("llvm.") != std::string::npos)
        return false;
    if (Function_id.find(&F)==Function_id.end())  // traverse functions and assign function ID
    {
        Function_id[&F] = ++Function_Counter;
    }
    bool removed = 0;

    Block2Inst2DupInst.clear();

    std::vector<Instruction*> PHINodeList;
    for (BasicBlock &B : F) 
    {
        for (Instruction &I: B) 
        {
            if (I.getOpcode()==Instruction::PHI)
            {
                PHINodeList.push_back(&I);
            }
            if (I.getOpcode() == Instruction::Load || I.getOpcode() == Instruction::Store || I.getOpcode() == Instruction::Call || I.getOpcode() == Instruction::Alloca || I.getOpcode() == Instruction::Br)
                continue;
            checkDuplicationInBlock(&B,&I);
        }
    }

    std::set<Instruction*> removedI;
    removedI.clear();
    
    for (BasicBlock &B : F) 
    {
        for (auto dupIMap : Block2Inst2DupInst[&B])
        {
            if (removedI.find(dupIMap.first) == removedI.end())
            {
                Instruction *I = dupIMap.first;
                for (auto dupI : dupIMap.second)
                {
                    if (removedI.find(dupI) != removedI.end())
                        continue;
                    if (dupI->getType() != I->getType())
                        continue;
                    if (DEBUG) *RemoveLog <<"duplicated: " << *I << " -------  " << *dupI <<"\n";
                    if (DEBUG) *RemoveLog <<"Remove: " << *dupI <<"\n";
                    if (DEBUG) *RemoveLog <<"Block: \n" <<  B <<"\n";
                    if (DEBUG) RemoveLog->flush(); 
                    dupI->replaceAllUsesWith(I);
                    removedI.insert(dupI);
                    // RecursivelyDeleteTriviallyDeadInstructions(dupI);
                    
                    dupI->eraseFromParent();
                    removed = 1;
                }
            }            
        }
    }

    for (Instruction *tmp_PHI_I : PHINodeList) 
    {
        auto PHI_I = dyn_cast<llvm::PHINode>(tmp_PHI_I);
        assert(PHI_I);
        if (PHI_I->getNumIncomingValues()==1)
        {
            PHI_I->replaceAllUsesWith(PHI_I->getOperand(0));
            removedI.insert(PHI_I);
            PHI_I->eraseFromParent();
            removed = 1;
        }
    }

    RemoveLog->flush(); 
    return removed;
}



char HI_HLSDuplicateInstRm::ID = 0;  // the ID for pass should be initialized but the value does not matter, since LLVM uses the address of this variable as label instead of its value.

void HI_HLSDuplicateInstRm::getAnalysisUsage(AnalysisUsage &AU) const {
    AU.setPreservesCFG();
}

Instruction* HI_HLSDuplicateInstRm::checkDuplicationInBlock(BasicBlock *B, Instruction *I)
{
    if (BranchInst *tmpI = dyn_cast<BranchInst>(I))
        return nullptr;

    if (AllocaInst *tmpI = dyn_cast<AllocaInst>(I))
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

    bool  enableCheck = false;
    for (Instruction &tmpII: *B) 
    {
        Instruction *tmpI = &tmpII;
        if (enableCheck)
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
            {
                Block2Inst2DupInst[B][I].push_back(tmpI);
                // return tmpI; 
            }       
                          
        }
        enableCheck |= tmpI == I;
        
    }
    return nullptr;
}