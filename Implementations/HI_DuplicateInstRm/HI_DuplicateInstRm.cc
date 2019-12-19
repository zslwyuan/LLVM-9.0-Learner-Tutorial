#include "HI_DuplicateInstRm.h"
#include "HI_print.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Pass.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/raw_ostream.h"
#include <ios>
#include <stdio.h>
#include <stdlib.h>
#include <string>

using namespace llvm;

bool HI_DuplicateInstRm::runOnFunction(
    Function &F) // The runOnModule declaration will overide the virtual one in ModulePass, which
                 // will be executed for each Module.
{
    print_status("Running HI_DuplicateInstRm pass.");

    if (Function_id.find(&F) == Function_id.end()) // traverse functions and assign function ID
    {
        Function_id[&F] = ++Function_Counter;
    }

    if (F.getName().find("llvm.") != std::string::npos)
        return false;

    *RemoveLog << F << "\n";
    bool removed = 0;
    //    std::set<Instruction*> checkI;
    for (BasicBlock &B : F)
    {
        bool duplicationInBlock = 1;
        while (duplicationInBlock)
        {
            duplicationInBlock = 0;
            for (Instruction &I : B)
            {
                if (I.getOpcode() == Instruction::Load || I.getOpcode() == Instruction::Store ||
                    I.getOpcode() == Instruction::Call || I.getOpcode() == Instruction::Alloca ||
                    I.getOpcode() == Instruction::Br)
                    continue;
                // if (checkI.find(&I) != checkI.end())
                //     continue;
                // checkI.insert(&I);
                Instruction *dupI = checkDuplicationInBlock(&B, &I);
                if (dupI != nullptr)
                {
                    if (dupI->getType() != I.getType())
                        continue;
                    *RemoveLog << "duplicated: " << I << " -------  " << *dupI << "\n";
                    *RemoveLog << "Remove: " << *dupI << "\n";
                    dupI->replaceAllUsesWith(&I);
                    // RecursivelyDeleteTriviallyDeadInstructions(dupI);
                    dupI->eraseFromParent();
                    duplicationInBlock = 1;
                    removed = 1;
                    break; // remove the instruction and start the search again, until no
                           // duplication
                }
            }
        }
    }
    RemoveLog->flush();
    return removed;
}

char HI_DuplicateInstRm::ID =
    0; // the ID for pass should be initialized but the value does not matter, since LLVM uses the
       // address of this variable as label instead of its value.

void HI_DuplicateInstRm::getAnalysisUsage(AnalysisUsage &AU) const
{
    AU.addRequired<ScalarEvolutionWrapperPass>();
    AU.addRequired<LoopInfoWrapperPass>();
    AU.setPreservesCFG();
}

Instruction *HI_DuplicateInstRm::checkDuplicationInBlock(BasicBlock *B, Instruction *I)
{
    if (BranchInst *tmpI = dyn_cast<BranchInst>(I))
        return nullptr;

    if (AllocaInst *tmpI = dyn_cast<AllocaInst>(I))
        return nullptr;

    bool enableCheck = false;
    for (Instruction &tmpII : *B)
    {
        Instruction *tmpI = &tmpII;

        if (enableCheck)
        {
            // check if they are the same

            bool sameOpcode = tmpI->getOpcode() == I->getOpcode();
            bool sameOperands = 1;
            bool PHI_SameIncome = 1;
            if (tmpI->getNumOperands() != I->getNumOperands() || !sameOpcode)
                sameOperands = 0;
            else
            {
                if (PHINode *tmpI0 = dyn_cast<PHINode>(tmpI))
                {
                    // if it is a PHI Node, check the field too.
                    PHINode *tmpI1 = dyn_cast<PHINode>(I);
                    for (int i = 0; i < tmpI->getNumOperands(); ++i)
                    {
                        sameOperands &= (tmpI->getOperand(i) == I->getOperand(i));
                        sameOperands &= tmpI0->getIncomingBlock(i) == tmpI1->getIncomingBlock(i);
                    }
                }
                else
                {
                    for (int i = 0; i < tmpI->getNumOperands(); ++i)
                    {
                        sameOperands &= (tmpI->getOperand(i) == I->getOperand(i));
                    }
                }
            }
            if (sameOpcode && sameOperands)
                return tmpI;
        }
        enableCheck |= tmpI == I;
    }
    return nullptr;
}