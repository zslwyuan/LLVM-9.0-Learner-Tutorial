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

/*
    find the longest path in the function as its latency
*/
double HI_SimpleTimingEvaluation::getFunctionLatency(Function *F)
{
    if (FunctionLatency.find(F) != FunctionLatency.end())
        return FunctionLatency[F];

    *Evaluating_log << "Evaluating the latency of Function " << F->getName() << ":\n";
    BasicBlock *Func_Entry = &F->getEntryBlock(); // get the entry of the function
    *Evaluating_log << "-- its entry is: " << Func_Entry->getName() << "\n";

    // (2) traverse the block in loop by DFS to find the longest path
    double max_critial_path_in_F = 0;
    tmp_BlockCriticalPath_inFunc.clear(); // record the block level critical path in the loop
    tmp_LoopCriticalPath_inFunc
        .clear(); // record the critical path to the end of sub-loops in the loop

    Func_BlockVisited.clear();
    getFunctionLatency_traverseFromEntryToExiting(0.0, F, Func_Entry);

    for (auto tmp_it : tmp_BlockCriticalPath_inFunc)
        if (tmp_it.second > max_critial_path_in_F)
            max_critial_path_in_F = tmp_it.second;
    for (auto tmp_it : tmp_LoopCriticalPath_inFunc)
        if (tmp_it.second > max_critial_path_in_F)
            max_critial_path_in_F = tmp_it.second;

    // (4) mark the blocks in loop with the loop latency, so later processing can regard this loop
    // as an integration
    Func_BlockVisited.clear();
    FunctionLatency[F] = max_critial_path_in_F;
    FunctionEvaluated.insert(F);

    *Evaluating_log << "\n\n\nDone evaluation Function Latency for Function " << F->getName()
                    << " and its latency is " << max_critial_path_in_F << " cycles.\n";
    *Evaluating_log << "---- Function " << F->getName() << " includes "
                    << Function2OuterLoops[F].size() << " following most outer loop(s):\n-------";
    for (auto tmpL : Function2OuterLoops[F])
    {
        *Evaluating_log << " Loop:" << tmpL->getName() << "(lat=" << LoopLatency[tmpL] << ") ";
    }
    *Evaluating_log << "\n---- Function " << F->getName()
                    << " includes following block(s) out of loops:\n-------";
    for (auto &B_it : *F)
    {
        if (!isInLoop(&B_it))
            *Evaluating_log << " Block:" << B_it.getName() << "(lat=" << BlockLatency[&B_it]
                            << ") ";
    }
    *Evaluating_log << "\n\n\n\n";
    return max_critial_path_in_F;
}

/*
    traverse the block in function by DFS to find the longest paths to each block in the function:
    (1) Mark the block visited, as a step of typical DFS
    (2) Check whether the search reaches a block in the loops
    (3a) -- If it is a block in loops, regard the most outer loop as intergration and update the
   critical path if necessary (max(ori_CP, lastStateCP + LoopLatency)).
         -- find the successors of the loop by checking its exiting blocks' successors and continue
   the DFS (3b) -- If it is a block out of loops, evaluate the block latency and update the critical
   path if necessary (max(ori_CP, lastStateCP + BlockLatency)).
         -- find the successors of the block and continue the DFS
    (4) Release the block from visited flag, as a step of typical DFS
*/
void HI_SimpleTimingEvaluation::getFunctionLatency_traverseFromEntryToExiting(
    double tmp_critical_path, Function *F, BasicBlock *curBlock)
{

    // (1) Mark the block visited, as a step of typical DFS
    Func_BlockVisited.insert(curBlock);

    *Evaluating_log << "---- FUNCTION traverser arrive Block: " << curBlock->getName() << " ";

    // (2) Check whether the search reaches a block in the loops
    if (isInLoop(curBlock))
    {
        // (3a) -- If it is a block in loops, regard the loop as intergration and update the
        // critical path if necessary (max(ori_CP, lastStateCP + LoopLatency)).
        Loop *tmp_OuterLoop = getOuterLoopOfBlock(curBlock);
        *Evaluating_log << "---- Block: " << curBlock->getName()
                        << " is in Outer Loop : " << tmp_OuterLoop->getName() << " ";

        if (Function2OuterLoops.find(F) == Function2OuterLoops.end())
        {
            std::vector<Loop *> tmp_vec_loop;
            Function2OuterLoops[F] = tmp_vec_loop;
        }

        double latency_Loop = getOuterLoopLatency(
            tmp_OuterLoop); // treat the entire loop as a block node and get the latency
        double try_critical_path =
            tmp_critical_path + latency_Loop; // first, get the critical path to the end of loop

        *Evaluating_log << " LoopLatency =  " << latency_Loop << " ";
        *Evaluating_log << " NewCP =  " << try_critical_path << " ";
        bool checkFlag = false;

        if (tmp_LoopCriticalPath_inFunc.find(tmp_OuterLoop) == tmp_LoopCriticalPath_inFunc.end())
            checkFlag = true;
        else if (try_critical_path > tmp_LoopCriticalPath_inFunc[tmp_OuterLoop])
            checkFlag = true;

        if (checkFlag)
        {
            if (tmp_LoopCriticalPath_inFunc.find(tmp_OuterLoop) !=
                tmp_LoopCriticalPath_inFunc.end())
            {
                *Evaluating_log << " OriCP =  " << tmp_LoopCriticalPath_inFunc[tmp_OuterLoop]
                                << "\n";
            }
            else
            {
                Function2OuterLoops[F].push_back(tmp_OuterLoop);
                *Evaluating_log << " No OriCP"
                                << "\n";
            }
            tmp_LoopCriticalPath_inFunc[tmp_OuterLoop] = try_critical_path;

            //  (3a)  -- find the successors of the loop by checking its exiting blocks' successors
            //  and continue the DFS
            SmallVector<BasicBlock *, 8> tmp_OuterLoop_ExitingBlocks;
            tmp_OuterLoop->getExitingBlocks(tmp_OuterLoop_ExitingBlocks);
            for (auto ExitB : tmp_OuterLoop_ExitingBlocks)
            {
                for (auto B : successors(ExitB))
                {
                    if (F == B->getParent() && Func_BlockVisited.find(B) == Func_BlockVisited.end())
                    {
                        *Evaluating_log << "---- continue to traverser to Block: " << B->getName()
                                        << "\n";
                        getFunctionLatency_traverseFromEntryToExiting(try_critical_path, F, B);
                    }
                }
            }
        }
    }
    else
    {
        //     (3b) -- If it is a block out of loops, evaluate the block latency and update the
        //     critical path if necessary (max(ori_CP, lastStateCP + BlockLatency)).
        *Evaluating_log << "---- Block: " << curBlock->getName() << " is NOT in any Outer Loop ";
        double latency_CurBlock =
            BlockLatencyEvaluation(curBlock); // first, get the latency of the current block
        double try_critical_path = tmp_critical_path + latency_CurBlock;
        *Evaluating_log << "---- latencyBlock =  " << latency_CurBlock << " ";
        *Evaluating_log << " NewCP =  " << try_critical_path << " ";
        bool checkFlag = false;

        if (tmp_BlockCriticalPath_inFunc.find(curBlock) == tmp_BlockCriticalPath_inFunc.end())
            checkFlag = true;
        else if (try_critical_path > tmp_BlockCriticalPath_inFunc[curBlock])
            checkFlag = true;

        if (checkFlag) // update the block-level critical path
        {
            if (tmp_BlockCriticalPath_inFunc.find(curBlock) != tmp_BlockCriticalPath_inFunc.end())
            {
                *Evaluating_log << " OriCP =  " << tmp_BlockCriticalPath_inFunc[curBlock] << "\n";
            }
            else
            {
                *Evaluating_log << " No OriCP"
                                << "\n";
            }

            tmp_BlockCriticalPath_inFunc[curBlock] = try_critical_path;

            // (3b)  -- find the successors of the block and continue the DFS
            for (auto B : successors(curBlock))
            {
                if (F == B->getParent() && Func_BlockVisited.find(B) == Func_BlockVisited.end())
                {
                    *Evaluating_log << "---- continue to traverser to Block: " << B->getName()
                                    << " ";
                    getFunctionLatency_traverseFromEntryToExiting(try_critical_path, F, B);
                }
            }
        }
    }
    // (4) Release the block from visited flag, as a step of typical DFS
    Func_BlockVisited.erase(curBlock);
}
