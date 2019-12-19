#include "HI_LoopInFormationCollect.h"
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

bool HI_LoopInFormationCollect::runOnLoop(
    Loop *L, LPPassManager &) // The runOnLoop declaration will overide the virtual one in LoopPass,
                              // which will be executed for each Loop.
{
    // Loop_out->flush();

    SE = &getAnalysis<ScalarEvolutionWrapperPass>().getSE();
    LI = &getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
    LAA = &getAnalysis<LoopAccessLegacyAnalysis>();
    if (Loop_id.find(L) ==
        Loop_id.end()) // traverse instructions in the block assign instruction ID
    {
        Loop_id[L] = ++Loop_Counter;
    }

    if (L->isInvalid())
    {
        print_info("Pass encounter invalid loop");
        errs() << "Loop-ID:" << Loop_id[L] << " is invalid but being processed by pass.\n";
    }
    //   else
    //  return false;

    *Loop_out << "L_ID" << Loop_id[L]
              << "--getSmallConstantMaxTripCount=" << SE->getSmallConstantMaxTripCount(L)
              << "--depth=" << L->getLoopDepth() << "\n"; // estimate the trip count of the loop
    // *Loop_out << "L_ID"
    // <<Loop_id[L]<<"--getLoopEstimatedTripCount="<<getLoopEstimatedTripCount(L)<<"\n";
    *Loop_out << "-----------Loop content---------------\n";
    *Loop_out << *L;
    *Loop_out << "---------------Loop report----------------\n";
    LAA->getInfo(L).print(*Loop_out);
    *Loop_out << "---------------Loop Header----------------\n";
    *Loop_out << L->getHeader()->getName() << "\n";
    *Loop_out << "---------------Loop PreHeader----------------\n";
    if (L->getLoopPreheader())
        *Loop_out << L->getLoopPreheader()->getName() << "\n";
    *Loop_out << "---------------Loop blocks----------------\n";
    for (auto BinL : L->getBlocks())
    {
        std::vector<BasicBlock *> *tmp_vec_block;
        std::vector<Loop *> *tmp_vec_loop;

        if (Block2Loops.find(BinL) == Block2Loops.end())
        {
            tmp_vec_loop = new std::vector<Loop *>;
            Block2Loops[BinL] = tmp_vec_loop;
        }
        else
        {
            tmp_vec_loop = Block2Loops[BinL];
        }
        if (Loop2Blocks.find(L) == Loop2Blocks.end())
        {
            tmp_vec_block = new std::vector<BasicBlock *>;
            Loop2Blocks[L] = tmp_vec_block;
        }
        else
        {
            tmp_vec_block = Loop2Blocks[L];
        }
        tmp_vec_block->push_back(BinL);
        tmp_vec_loop->push_back(L);
        *Loop_out << BinL->getName() << " ";
    }
    *Loop_out << "\n";
    // const LoopAccessInfo &cur_LI = (LAA->getInfo(L));
    *Loop_out << "\n================================\n";
    int i = 0;
    for (auto TL : *L)
    {
        i++;
        *Loop_out << "=============== " << i << "th SubLoop=================\n";
        *Loop_out << *TL;
        *Loop_out << "\n";
    }
    *Loop_out << "\n\n\n";
    printMaps();
    Loop_out->flush();
    return false;
}

char HI_LoopInFormationCollect::ID =
    0; // the ID for pass should be initialized but the value does not matter, since LLVM uses the
       // address of this variable as label instead of its value.

void HI_LoopInFormationCollect::getAnalysisUsage(AnalysisUsage &AU) const
{
    AU.setPreservesAll();
    AU.addRequired<LoopInfoWrapperPass>();
    AU.addRequired<ScalarEvolutionWrapperPass>();
    // AU.addRequired<ScalarEvolutionWrapperPass>();
    // AU.addRequired<LoopInfoWrapperPass>();
    // AU.addPreserved<LoopInfoWrapperPass>();
    AU.addRequired<LoopAccessLegacyAnalysis>();
    AU.addRequired<DominatorTreeWrapperPass>();
    // AU.addPreserved<DominatorTreeWrapperPass>();
    AU.addRequired<OptimizationRemarkEmitterWrapperPass>();
    // AU.addPreserved<GlobalsAAWrapperPass>();
}
