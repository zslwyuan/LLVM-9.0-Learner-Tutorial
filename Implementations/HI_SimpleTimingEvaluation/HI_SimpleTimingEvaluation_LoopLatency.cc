#include "HI_SimpleTimingEvaluation.h"
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

bool HI_SimpleTimingEvaluation::isInLoop(BasicBlock *BB)
{
    return (Block2Loops.find(BB) != Block2Loops.end());
}

/*
    find the outer loop and evaluate it as a integration
*/
Loop *HI_SimpleTimingEvaluation::getOuterLoopOfBlock(BasicBlock *B)
{
    for (auto tmp_Loop : *Block2Loops[B]) // find the most outer loop
    {
        if (tmp_Loop->getLoopDepth() == 1)
        {
            return tmp_Loop;
        }
    }
    assert(false && "a loop shoud be found but actually not");
}

void HI_SimpleTimingEvaluation::getLoopBlockMap(Function *F)
{
    *Evaluating_log << "    getLoopBlockMap for Function : " << F->getName() << " \n ";
    for (auto ele : Loop2Blocks)
    {
        delete ele.second;
    }
    for (auto ele : Block2Loops)
    {
        delete ele.second;
    }
    Loop2Blocks.clear();
    Block2Loops.clear();
    for (LoopInfo::iterator i = LI->begin(), e = LI->end(); i != e; ++i)
    {
        Loop *L = *i;
        *Evaluating_log << "--------- Loop: " << L->getName() << " contains:\n ";
        for (auto BinL : L->getBlocks())
        {
            *Evaluating_log << "------------- Block: " << BinL->getName() << " \n";
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
        }
    }
}

/*
    find the inner unevaluated loop,
    (1) check all the sub-loops
    (2) check the loop itself
*/
Loop *HI_SimpleTimingEvaluation::getInnerUnevaluatedLoop(Loop *outerL)
{
    int dep = 0;
    Loop *tmp_inner_Loop = NULL;
    for (auto tmp_Loop : *outerL) // find the most inner unevaluated loop
    {
        *Evaluating_log << "--------- checking sub-loop: " << tmp_Loop->getName()
                        << " -> dep = " << tmp_Loop->getLoopDepth() << " ";
        if (LoopEvaluated.find(tmp_Loop) != LoopEvaluated.end())
            *Evaluating_log << " which is evaluated.\n";
        else
            *Evaluating_log << " which is NOT evaluated.\n";

        // larger depth means more inner

        // go to the sub-sub-...-Loop to have a check
        Loop *tmp_inner_Sub_Loop = getInnerUnevaluatedLoop(tmp_Loop);
        if (tmp_inner_Sub_Loop)
        {
            if (tmp_inner_Sub_Loop->getLoopDepth() > dep &&
                LoopEvaluated.find(tmp_inner_Sub_Loop) == LoopEvaluated.end())
            {
                dep = tmp_inner_Sub_Loop->getLoopDepth();
                tmp_inner_Loop =
                    tmp_inner_Sub_Loop; //  the sub-sub-...-loop could be the most inner loop
                *Evaluating_log << "--------- update target sub-loop to Loop: "
                                << tmp_inner_Loop->getName() << "\n";
            }
        }
        else
        {
            if (tmp_Loop->getLoopDepth() > dep &&
                LoopEvaluated.find(tmp_Loop) == LoopEvaluated.end())
            {
                dep = tmp_Loop->getLoopDepth();
                tmp_inner_Loop = tmp_Loop; //  no the sub-sub-...-loop could be the most inner loop
                                           //  but current sub-loop could be
                *Evaluating_log << "--------- update target sub-loop to Loop: "
                                << tmp_inner_Loop->getName() << "\n";
            }
        }
    }
    auto tmp_Loop = outerL;
    if (tmp_inner_Loop == NULL) // all sub-loops are evaluated, check the loop itself.
    {
        *Evaluating_log << "--------- checking loop itself: " << tmp_Loop->getName()
                        << " -> dep = " << tmp_Loop->getLoopDepth() << " ";
        if (LoopEvaluated.find(tmp_Loop) != LoopEvaluated.end())
            *Evaluating_log << " which is evaluated.\n";
        else
            *Evaluating_log << " which is NOT evaluated.\n";

        // larger depth means more inner
        if (tmp_Loop->getLoopDepth() > dep && LoopEvaluated.find(tmp_Loop) == LoopEvaluated.end())
        {
            dep = tmp_Loop->getLoopDepth();
            tmp_inner_Loop = tmp_Loop;
            *Evaluating_log << "--------- update target loop to Loop: " << tmp_inner_Loop->getName()
                            << "\n";
        }
    }

    return tmp_inner_Loop;
}

/*
    To get the latency of the entire outer loop,
    (1) iteratively handle the most inner loop,
    (2) traverse the blocks in loop by DFS to find the longest path
    (3) get the total latency by TripCount * IterationLatency
    (4) mark the blocks in loop with the loop latency, so later processing can regard this loop as
   an integration
*/
double HI_SimpleTimingEvaluation::getOuterLoopLatency(Loop *outerL)
{
    *Evaluating_log << "\n Evaluating Outer Loop Latency for Loop " << outerL->getName() << ":\n";
    if (LoopLatency.find(outerL) != LoopLatency.end())
    {
        *Evaluating_log << "Done evaluation outer Loop Latency for Loop " << outerL->getName()
                        << " and its latency is " << LoopLatency[outerL] << " cycles.\n\n\n";
        return LoopLatency[outerL];
    }
    Loop *cur_Loop;
    double outerL_latency = -1;
    double tmp_total_latency;

    // (1) iteratively handle the most inner loop
    cur_Loop = getInnerUnevaluatedLoop(outerL);
    while (cur_Loop != NULL)
    {
        *Evaluating_log << "-- Handling the inner Loop " << cur_Loop->getName() << ":\n";
        BasicBlock *tmp_LoopHeader = cur_Loop->getHeader(); // get the header of the loop
        *Evaluating_log << "---- its header: " << tmp_LoopHeader->getName() << ":\n";
        SmallVector<BasicBlock *, 8> tmp_ExitingBlocks;
        cur_Loop->getExitingBlocks(tmp_ExitingBlocks); // get the exiting blocks of the loop
        if (tmp_ExitingBlocks.size() != 1)
        {
            assert(tmp_ExitingBlocks.size() > 0);
            print_warning("the loop could be better to have only one exiting block for the "
                          "accuracy of latency evaluation.");
        }
        for (auto B_it : tmp_ExitingBlocks)
        {
            *Evaluating_log << "---- its exiting block(s): " << B_it->getName() << " -- ";
        }
        *Evaluating_log << "\n";

        // (2) traverse the block in loop by DFS to find the longest path
        double max_critial_path_in_curLoop = 0;
        tmp_BlockCriticalPath_inLoop.clear(); // record the block level critical path in the loop
        tmp_SubLoop_CriticalPath
            .clear(); // record the critical path to the end of sub-loops in the loop

        BlockVisited.clear();
        LoopLatencyEvaluation_traversFromHeaderToExitingBlocks(0, cur_Loop, tmp_LoopHeader);
        BlockCriticalPath_inLoop[cur_Loop] = tmp_BlockCriticalPath_inLoop;

        for (auto tmp_it : tmp_BlockCriticalPath_inLoop)
            if (tmp_it.second > max_critial_path_in_curLoop)
                max_critial_path_in_curLoop = tmp_it.second;
        for (auto tmp_it : tmp_SubLoop_CriticalPath)
            if (tmp_it.second > max_critial_path_in_curLoop)
                max_critial_path_in_curLoop = tmp_it.second;

        // (3) get the total latency by TripCount * IterationLatency
        tmp_total_latency =
            SE->getSmallConstantMaxTripCount(cur_Loop) * max_critial_path_in_curLoop;

        // COMMENT because preheader is not in the loop enity and if the prehearder is calculated,
        // it is actually duplicated calculation. but just need to add one cycle, as it seems that
        // in VivadoHLS, Loops are regarded as function and the "call" will take one cycle if
        // (cur_Loop->getLoopPreheader())
        //     tmp_total_latency += BlockLatencyEvaluation(cur_Loop->getLoopPreheader());
        tmp_total_latency += 1;

        // (4) mark the blocks in loop with the loop latency, so later processing can regard this
        // loop as an integration
        BlockVisited.clear();
        MarkBlock_traversFromHeaderToExitingBlocks(tmp_total_latency, cur_Loop, tmp_LoopHeader);
        LoopLatency[cur_Loop] = tmp_total_latency;
        LoopEvaluated.insert(cur_Loop);

        *Evaluating_log << "Trip Count for Loop " << cur_Loop->getName() << " is "
                        << SE->getSmallConstantMaxTripCount(cur_Loop) << "\n";
        *Evaluating_log << "Done evaluation Loop Latency for Loop " << cur_Loop->getName()
                        << " and its latency is " << tmp_total_latency << " cycles.\n\n\n";

        // (1) iteratively handle the most inner loop
        cur_Loop = getInnerUnevaluatedLoop(outerL);
    }
    outerL_latency =
        tmp_total_latency; // finally, we will get the latency of outer loop in the last iteration
    *Evaluating_log << "Done evaluation outer Loop Latency for Loop " << outerL->getName()
                    << " and its latency is " << outerL_latency << " cycles.\n\n\n";
    assert(outerL_latency > -0.5 && "The latency for a loop should be not be negative");
    return outerL_latency;
}

/*
    traverse the block in loop by DFS to find the longest path:
    (1) Mark the block visited, as a step of typical DFS
    (2) Check whether the search reaches a block in the sub-loops
    (3a) -- If it is a block in sub-loops, regard the loop as intergration and update the critical
   path if necessary (max(ori_CP, lastStateCP + LoopLatency)).
         -- find the successors of the loop by checking its exiting blocks' successors and continue
   the DFS (3b) -- If it is a block out of sub-loops, evaluate the block latency and update the
   critical path if necessary (max(ori_CP, lastStateCP + BlockLatency)).
         -- find the successors of the block and continue the DFS
    (4) Release the block from visited flag, as a step of typical DFS

*/
void HI_SimpleTimingEvaluation::LoopLatencyEvaluation_traversFromHeaderToExitingBlocks(
    double tmp_critical_path, Loop *L, BasicBlock *curBlock)
{

    // (1) Mark the block visited, as a step of typical DFS
    BlockVisited.insert(curBlock);

    *Evaluating_log << "---- traverser arrive Block: " << curBlock->getName() << " ";

    // (2) Check whether the search reaches a block in the sub-loops
    if (Block2EvaluatedLoop.find(curBlock) != Block2EvaluatedLoop.end())
    {
        // (3a) -- If it is a block in sub-loops, regard the loop as intergration and update the
        // critical path if necessary (max(ori_CP, lastStateCP + LoopLatency)).
        Loop *tmp_SubLoop = Block2EvaluatedLoop[curBlock];
        *Evaluating_log << " which is evluated in Loop " << tmp_SubLoop->getName() << " ";
        *Evaluating_log << " LoopLatency =  " << LoopLatency[tmp_SubLoop] << " ";
        double try_critical_path =
            tmp_critical_path +
            LoopLatency[tmp_SubLoop]; // first, get the critical path to the end of sub-loop
        *Evaluating_log << " NewCP =  " << try_critical_path << " ";
        bool checkFlag = false;

        if (tmp_SubLoop_CriticalPath.find(tmp_SubLoop) == tmp_SubLoop_CriticalPath.end())
            checkFlag = true;
        else if (try_critical_path > tmp_SubLoop_CriticalPath[tmp_SubLoop])
            checkFlag = true;

        if (checkFlag)
        {
            if (tmp_SubLoop_CriticalPath.find(tmp_SubLoop) != tmp_SubLoop_CriticalPath.end())
            {
                *Evaluating_log << " OriCP =  " << tmp_SubLoop_CriticalPath[tmp_SubLoop] << "\n";
            }
            else
            {
                *Evaluating_log << " No OriCP"
                                << "\n";
            }
            tmp_SubLoop_CriticalPath[tmp_SubLoop] = try_critical_path;

            //  (3a)  -- find the successors of the loop by checking its exiting blocks' successors
            //  and continue the DFS
            SmallVector<BasicBlock *, 8> tmp_SubLoop_ExitingBlocks;
            tmp_SubLoop->getExitingBlocks(tmp_SubLoop_ExitingBlocks);
            for (auto ExitB : tmp_SubLoop_ExitingBlocks)
            {
                for (auto B : successors(ExitB))
                {
                    if (L->contains(B) && BlockVisited.find(B) == BlockVisited.end())
                    {
                        *Evaluating_log << "---- continue to traverser to Block: " << B->getName()
                                        << " ";
                        LoopLatencyEvaluation_traversFromHeaderToExitingBlocks(try_critical_path, L,
                                                                               B);
                    }
                }
            }
        }
    }
    else
    {
        //     (3b) -- If it is a block out of sub-loops, evaluate the block latency and update the
        //     critical path if necessary (max(ori_CP, lastStateCP + BlockLatency)).
        *Evaluating_log << " which is  not evaluated in Loop "
                        << " ";
        double latency_CurBlock =
            BlockLatencyEvaluation(curBlock); // first, get the latency of the current block
        double try_critical_path = tmp_critical_path + latency_CurBlock;
        *Evaluating_log << "---- latencyBlock =  " << latency_CurBlock << " ";
        *Evaluating_log << " NewCP =  " << try_critical_path << " ";
        bool checkFlag = false;

        if (tmp_BlockCriticalPath_inLoop.find(curBlock) == tmp_BlockCriticalPath_inLoop.end())
            checkFlag = true;
        else if (try_critical_path > tmp_BlockCriticalPath_inLoop[curBlock])
            checkFlag = true;

        if (checkFlag) // update the block-level critical path
        {
            if (tmp_BlockCriticalPath_inLoop.find(curBlock) != tmp_BlockCriticalPath_inLoop.end())
            {
                *Evaluating_log << " OriCP =  " << tmp_BlockCriticalPath_inLoop[curBlock] << "\n";
            }
            else
            {
                *Evaluating_log << " No OriCP"
                                << "\n";
            }

            tmp_BlockCriticalPath_inLoop[curBlock] = try_critical_path;

            // (3b)  -- find the successors of the block and continue the DFS
            for (auto B : successors(curBlock))
            {
                if (L->contains(B) && BlockVisited.find(B) == BlockVisited.end())
                {
                    *Evaluating_log << "---- continue to traverser to Block: " << B->getName()
                                    << " ";
                    LoopLatencyEvaluation_traversFromHeaderToExitingBlocks(try_critical_path, L, B);
                }
            }
        }
    }
    // (4) Release the block from visited flag, as a step of typical DFS
    BlockVisited.erase(curBlock);
}

/*
    Simply mark all the blocks in the loop with the totoal_latency by DFS-traverse
*/
void HI_SimpleTimingEvaluation::MarkBlock_traversFromHeaderToExitingBlocks(double total_latency,
                                                                           Loop *L,
                                                                           BasicBlock *curBlock)
{
    BlockVisited.insert(curBlock);
    Block2EvaluatedLoop[curBlock] = L;

    for (auto B : successors(curBlock))
    {
        if (L->contains(B) && BlockVisited.find(B) == BlockVisited.end())
        {
            MarkBlock_traversFromHeaderToExitingBlocks(total_latency, L, B);
        }
    }
}
