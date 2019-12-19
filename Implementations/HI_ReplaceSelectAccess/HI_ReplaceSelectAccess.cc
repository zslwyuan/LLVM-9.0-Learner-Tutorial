#include "HI_ReplaceSelectAccess.h"
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

bool HI_ReplaceSelectAccess::runOnFunction(
    Function &F) // The runOnModule declaration will overide the virtual one in ModulePass, which
                 // will be executed for each Module.
{
    bool changed = false;

    for (auto &B : F)
    {
        bool ActionTaken = true;
        while (ActionTaken)
        {
            ActionTaken = false;

            for (auto &I : B)
            {
                if (DEBUG)
                    *ReplaceSelectAccess_Log << "checking instruction: " << I << "\n";
                if (auto selectI = dyn_cast<SelectInst>(&I))
                {
                    if (DEBUG)
                        *ReplaceSelectAccess_Log << " is selectInst\n";
                    if (checkUseInMemoryAccess(&I) && checkOperandsAreAddress(&I))
                    {
                        if (DEBUG)
                            *ReplaceSelectAccess_Log << "\n need to be processed\n";
                        ActionTaken = true;
                        From_SelectAccess_To_AccessSelect(&I);
                        changed |= true;
                        if (DEBUG)
                            *ReplaceSelectAccess_Log << "\n\n\n";
                        break;
                    }
                }
            }
            // if (ActionTaken)
            //     break;
        }
    }

    // return false;
    return changed;
}

char HI_ReplaceSelectAccess::ID =
    0; // the ID for pass should be initialized but the value does not matter, since LLVM uses the
       // address of this variable as label instead of its value.

void HI_ReplaceSelectAccess::getAnalysisUsage(AnalysisUsage &AU) const
{
    AU.setPreservesCFG();
}

bool HI_ReplaceSelectAccess::checkOperandsAreAddress(Instruction *I)
{
    for (int i = 1; i < I->getNumOperands(); i++)
    {
        if (auto opI = dyn_cast<IntToPtrInst>(I->getOperand(i)))
        {
            if (DEBUG)
                *ReplaceSelectAccess_Log << "op#" << i << ": " << *(I->getOperand(i))
                                         << " is ITP\n";
        }
        else
        {
            if (DEBUG)
                *ReplaceSelectAccess_Log << "op#" << i << ": " << *(I->getOperand(i))
                                         << " is NOT ITP\n";
            return false;
        }
    }
    return true;
}

bool HI_ReplaceSelectAccess::checkUseInMemoryAccess(Instruction *I)
{
    if (!I->hasOneUse()) // currently, we only consider the 2-1 select for
        return false;

    User *tmp_user = I->use_begin()->getUser();

    if (auto LoadI = dyn_cast<LoadInst>(tmp_user))
    {
        if (DEBUG)
            *ReplaceSelectAccess_Log << *(I) << " is used for Load\n";
        return true;
    }
    else if (auto StoreI = dyn_cast<StoreInst>(tmp_user))
    {
        if (DEBUG)
            *ReplaceSelectAccess_Log << *(I) << " is used for Store\n";
        return true;
    }
    else
    {
        return false;
    }
}

Instruction *HI_ReplaceSelectAccess::getAccessInst(Instruction *I)
{
    assert(I->hasOneUse()); // currently, we only consider the 2-1 select for

    User *tmp_user = I->use_begin()->getUser();

    if (auto LoadI = dyn_cast<LoadInst>(tmp_user))
    {
        return LoadI;
    }
    else if (auto StoreI = dyn_cast<StoreInst>(tmp_user))
    {
        return StoreI;
    }
}

void HI_ReplaceSelectAccess::From_SelectAccess_To_AccessSelect(Instruction *I)
{
    if (DEBUG)
        *ReplaceSelectAccess_Log << *(I) << " is being From_SelectAccess_To_AccessSelect\n";
    SelectInst *selectI = dyn_cast<SelectInst>(I);

    assert(selectI && "The instruction has to be selectInst for this process.");

    Instruction *accessI = getAccessInst(I);
    IRBuilder<> Builder(I->getNextNode());
    std::string op0str = I->getName();
    op0str += ".access0";
    std::string op1str = I->getName();
    op1str += ".access1";
    std::string selectNewStr = selectI->getName();
    selectNewStr += ".accessSelect";

    Value *ResultPtr = nullptr;
    if (accessI->getOpcode() == Instruction::Load)
    {
        Value *op0 = Builder.CreateLoad(selectI->getOperand(1), op0str.c_str());
        Value *op1 = Builder.CreateLoad(selectI->getOperand(2), op1str.c_str());
        if (DEBUG)
            *ReplaceSelectAccess_Log << "newop0:" << *op0 << "\n";
        if (DEBUG)
            *ReplaceSelectAccess_Log << "newop1:" << *op1 << "\n";
        Value *newSelect =
            Builder.CreateSelect(selectI->getOperand(0), op0, op1, selectNewStr.c_str());
        if (DEBUG)
            *ReplaceSelectAccess_Log << "newSelect:" << *newSelect << "\n";
        accessI->replaceAllUsesWith(newSelect);
        accessI->eraseFromParent();
        selectI->eraseFromParent();
    }
}
