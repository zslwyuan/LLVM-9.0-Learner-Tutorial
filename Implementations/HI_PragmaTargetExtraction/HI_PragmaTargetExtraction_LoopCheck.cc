#include "HI_PragmaTargetExtraction.h"
#include "HI_print.h"
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

/*
    extract loop in the function, organize them in hierary for directive generation
*/
bool HI_PragmaTargetExtraction::checkLoopsIn(Function *F)
{
    for (auto tmp_loop : LI->getLoopsInPreorder())
    {
        if (tmp_loop->getLoopDepth() == 1)
        {
            checkSubLoops(tmp_loop);
            checkArrayAccessInLoop(tmp_loop);
        }
    }
}

void HI_PragmaTargetExtraction::checkSubLoops(Loop *L)
{
    int cur_depth = L->getLoopDepth();
    std::string curLoopName = L->getHeader()->getParent()->getName();
    curLoopName += "-";
    curLoopName += L->getHeader()->getName();
    LoopIRName2Depth[curLoopName] = cur_depth;
    for (auto sub_loop : *L)
    {
        if (sub_loop->getLoopDepth() == cur_depth + 1)
        {
            Loop2NextLevelSubLoops[L->getHeader()].push_back(sub_loop->getHeader());

            std::string nextLoopName = sub_loop->getHeader()->getParent()->getName();
            nextLoopName += "-";
            nextLoopName += sub_loop->getHeader()->getName();
            LoopIRName2NextLevelSubLoopIRNames[curLoopName].push_back(nextLoopName);
            if (DEBUG)
                *loopTarget_log << "parentLoop: " << curLoopName << " ==> sonLoop: " << nextLoopName
                                << "\n";
        }
    }
    for (auto sub_loop : *L)
    {
        if (sub_loop->getLoopDepth() == cur_depth + 1)
        {
            Loop2NextLevelSubLoops[L->getHeader()].push_back(sub_loop->getHeader());

            std::string nextLoopName = sub_loop->getHeader()->getParent()->getName();
            nextLoopName += "-";
            nextLoopName += sub_loop->getHeader()->getName();
            LoopIRName2NextLevelSubLoopIRNames[curLoopName].push_back(nextLoopName);
            checkSubLoops(sub_loop);
        }
    }
}

void HI_PragmaTargetExtraction::checkArrayAccessInLoop(Loop *L)
{
    std::string curLoopName = L->getHeader()->getParent()->getName();
    curLoopName += "-";
    curLoopName += L->getHeader()->getName();
    for (auto tmp_Block : L->getBlocks())
    {
        for (auto target : Block2Targets[tmp_Block])
        {
            Function *F = nullptr;
            if (auto arg = dyn_cast<Argument>(target))
            {
                F = arg->getParent();
            }
            if (auto allocI = dyn_cast<AllocaInst>(target))
            {
                F = allocI->getParent()->getParent();
            }
            LoopIRName2Array[curLoopName].push_back(std::pair<std::string, std::string>(
                L->getHeader()->getParent()->getName(), target->getName()));
            TargetExtName2ArrayInfo[std::pair<std::string, std::string>(
                L->getHeader()->getParent()->getName(), target->getName())] =
                Target2ArrayInfo[target];
            if (DEBUG)
                *arrayTarget_Log << "array/pointer : " << target->getName()
                                 << ", declared at function: " << demangleFunctionName(F->getName())
                                 << ", is used by loop: " << curLoopName << " in function "
                                 << L->getHeader()->getParent()->getName() << "\n";
        }
    }
}