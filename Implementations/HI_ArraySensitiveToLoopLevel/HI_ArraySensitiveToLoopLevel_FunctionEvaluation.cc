#include "HI_ArraySensitiveToLoopLevel.h"
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
    also record the resource cost in the function
*/
// HI_ArraySensitiveToLoopLevel::timingBase HI_ArraySensitiveToLoopLevel::analyzeFunction(Function
// *F)
// {
//     if (FunctionLatency.find(F)!=FunctionLatency.end())
//         return FunctionLatency[F]*1;

//     if (DEBUG) *Evaluating_log << "Evaluating the latency of Function " << F->getName() <<":\n";
//     BasicBlock *Func_Entry = &F->getEntryBlock(); //get the entry of the function
//     if (DEBUG) *Evaluating_log << "-- its entry is: " << Func_Entry->getName() <<"\n";

//     // F->getA

//     // (2) traverse the block in loop by DFS to find the longest path
//     timingBase max_critial_path_in_F(0,0,1,clock_period);
//     timingBase origin_path_in_F(0,0,1,clock_period);
//     resourceBase resourceAccumulator(0,0,0,clock_period);

//     tmp_BlockCriticalPath_inFunc.clear(); // record the block level critical path in the loop
//     tmp_LoopCriticalPath_inFunc.clear(); // record the critical path to the end of sub-loops in
//     the loop

//     Func_BlockVisited.clear();
//     analyzeFunction_traverseFromEntryToExiting(origin_path_in_F, F, Func_Entry,
//     resourceAccumulator);

//     for (auto tmp_it : tmp_BlockCriticalPath_inFunc)
//         if (tmp_it.second > max_critial_path_in_F)
//             max_critial_path_in_F = tmp_it.second;
//     for (auto tmp_it : tmp_LoopCriticalPath_inFunc)
//         if (tmp_it.second > max_critial_path_in_F)
//             max_critial_path_in_F = tmp_it.second;

//     // analyze the cost of floating-point operators (most of them will be aggressive reused by
//     VivadoHLS binding) resourceAccumulator = resourceAccumulator +
//     costRescheduleFPDSPOperators_forFunction(F);

//     // (4) mark the blocks in loop with the loop latency, so later processing can regard this
//     loop as an integration Func_BlockVisited.clear(); FunctionLatency[F] = max_critial_path_in_F;
//     FunctionResource[F] = resourceAccumulator;
//     FunctionEvaluated.insert(F);

//     if (DEBUG) *Evaluating_log << "\n\n\nDone evaluation Function Latency for Function " <<
//     F->getName() << " and its latency is " << max_critial_path_in_F <<" cycles and its resource
//     cost is: " << resourceAccumulator << ".\n"; if (DEBUG) *Evaluating_log << "---- Function " <<
//     F->getName() << " includes "<<Function2OuterLoops[F].size()<<" following most outer
//     loop(s):\n-------"; for (auto tmpL : Function2OuterLoops[F])
//     {
//         if (DEBUG) *Evaluating_log << " Loop:" << tmpL->getName() << "(lat=" <<
//         LoopLatency[tmpL->getHeader()] << ") ";
//     }
//     if (DEBUG) *Evaluating_log << "\n---- Function " << F->getName() << " includes following
//     block(s) out of loops:\n-------"; for (auto &B_it : *F)
//     {
//         if (!isInLoop(&B_it))
//             if (DEBUG) *Evaluating_log << " Block:" << B_it.getName() << "(lat=" <<
//             BlockLatency[&B_it] << ") ";
//     }
//     if (DEBUG) *Evaluating_log << "\n\n\n\n";
//     return max_critial_path_in_F*1;

// }

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

// void
// HI_ArraySensitiveToLoopLevel::analyzeFunction_traverseFromEntryToExiting(HI_ArraySensitiveToLoopLevel::timingBase
// tmp_critical_path, Function *F, BasicBlock* curBlock, HI_ArraySensitiveToLoopLevel::resourceBase
// &resourceAccumulator)
// {

//     // (1) Mark the block visited, as a step of typical DFS
//     Func_BlockVisited.insert(curBlock);

//     if (DEBUG) *Evaluating_log << "---- FUNCTION traverser arrive Block: " << curBlock->getName()
//     <<" ";

//     // (2) Check whether the search reaches a block in the loops
//     if (isInLoop(curBlock))
//     {
//         // (3a) -- If it is a block in loops, regard the loop as intergration and update the
//         critical path if necessary (max(ori_CP, lastStateCP + LoopLatency)). Loop* tmp_OuterLoop
//         = getOuterLoopOfBlock(curBlock); if (DEBUG) *Evaluating_log << "---- Block: " <<
//         curBlock->getName() <<" is in Outer Loop : "<< tmp_OuterLoop->getName() <<" ";

//         if (Function2OuterLoops.find(F) == Function2OuterLoops.end())
//         {
//             std::vector<Loop*> tmp_vec_loop;
//             Function2OuterLoops[F] = tmp_vec_loop;
//         }

//         timingBase latency_Loop = analyzeOuterLoop(tmp_OuterLoop); // treat the entire loop as a
//         block node and get the latency timingBase try_critical_path = tmp_critical_path +
//         latency_Loop;  // first, get the critical path to the end of loop

//         if (DEBUG) *Evaluating_log << " LoopLatency =  " << latency_Loop <<" ";
//         if (DEBUG) *Evaluating_log << " NewCP =  " << try_critical_path <<" ";
//         bool checkFlag = false;

//         if (tmp_LoopCriticalPath_inFunc.find(tmp_OuterLoop) == tmp_LoopCriticalPath_inFunc.end()
//         )
//         {
//             resourceAccumulator = resourceAccumulator + LoopResource[tmp_OuterLoop->getHeader()]
//             ; checkFlag = true;
//         }
//         else if (try_critical_path > tmp_LoopCriticalPath_inFunc[tmp_OuterLoop]) checkFlag =
//         true;

//         if (checkFlag)
//         {
//             if (tmp_LoopCriticalPath_inFunc.find(tmp_OuterLoop) !=
//             tmp_LoopCriticalPath_inFunc.end())
//             {
//                 if (DEBUG) *Evaluating_log << " OriCP =  " <<
//                 tmp_LoopCriticalPath_inFunc[tmp_OuterLoop] <<"\n";
//             }
//             else
//             {
//                 Function2OuterLoops[F].push_back(tmp_OuterLoop);
//                 if (DEBUG) *Evaluating_log << " No OriCP" <<"\n";
//             }
//             tmp_LoopCriticalPath_inFunc[tmp_OuterLoop] = try_critical_path;

//              //  (3a)  -- find the successors of the loop by checking its exiting blocks'
//              successors and continue the DFS
//             SmallVector<BasicBlock*, 8>  tmp_OuterLoop_ExitingBlocks;
//             tmp_OuterLoop->getExitingBlocks(tmp_OuterLoop_ExitingBlocks);
//             for (auto ExitB : tmp_OuterLoop_ExitingBlocks)
//             {
//                 for (auto B : successors(ExitB))
//                 {
//                     if (F == B->getParent() && Func_BlockVisited.find(B) ==
//                     Func_BlockVisited.end() && !tmp_OuterLoop->contains(B))
//                     {
//                         if (DEBUG) *Evaluating_log << "---- function continue to traverser to
//                         Block: " << B->getName()  << " from " << curBlock->getName() <<"\n";
//                         analyzeFunction_traverseFromEntryToExiting(try_critical_path, F, B,
//                         resourceAccumulator);
//                     }
//                 }
//             }

//         }

//     }
//     else
//     {
//         //     (3b) -- If it is a block out of loops, evaluate the block latency and update the
//         critical path if necessary (max(ori_CP, lastStateCP + BlockLatency)). if (DEBUG)
//         *Evaluating_log << "---- Block: " << curBlock->getName() <<" is NOT in any Outer Loop ";
//         timingBase latency_CurBlock = BlockLatencyResourceEvaluation(curBlock); // first, get the
//         latency of the current block timingBase try_critical_path = tmp_critical_path +
//         latency_CurBlock; if (DEBUG) *Evaluating_log << "---- latencyBlock =  " <<
//         latency_CurBlock <<" "; if (DEBUG) *Evaluating_log << " NewCP =  " << try_critical_path
//         <<" "; bool checkFlag = false;

//         if (tmp_BlockCriticalPath_inFunc.find(curBlock) == tmp_BlockCriticalPath_inFunc.end() )
//         {
//             resourceAccumulator = resourceAccumulator + BlockResource[curBlock] ;
//             checkFlag = true;
//         }
//         else if (try_critical_path > tmp_BlockCriticalPath_inFunc[curBlock]) checkFlag = true;

//         if (checkFlag) // update the block-level critical path
//         {
//             if (tmp_BlockCriticalPath_inFunc.find(curBlock) !=
//             tmp_BlockCriticalPath_inFunc.end())
//             {
//                 if (DEBUG) *Evaluating_log << " OriCP =  " <<
//                 tmp_BlockCriticalPath_inFunc[curBlock] <<"\n";
//             }
//             else
//             {
//                 if (DEBUG) *Evaluating_log << " No OriCP" <<"\n";
//             }

//             tmp_BlockCriticalPath_inFunc[curBlock] = try_critical_path;

//             // (3b)  -- find the successors of the block and continue the DFS
//             for (auto B : successors(curBlock))
//             {
//                 if (F == B->getParent() && Func_BlockVisited.find(B) == Func_BlockVisited.end())
//                 {
//                     if (DEBUG) *Evaluating_log << "---- function continue to traverser to Block:
//                     " << B->getName() << " from " << curBlock->getName() <<" ";
//                     analyzeFunction_traverseFromEntryToExiting(try_critical_path, F, B,
//                     resourceAccumulator);
//                 }
//             }
//         }
//     }
//     // (4) Release the block from visited flag, as a step of typical DFS
//     Func_BlockVisited.erase(curBlock);
// }

// // get how many state needed for the function
// int HI_ArraySensitiveToLoopLevel::getFunctionStageNum(HI_ArraySensitiveToLoopLevel::timingBase
// tmp_critical_path, Function *F, BasicBlock* curBlock)
// {

//     // // (1) Mark the block visited, as a step of typical DFS
//     // Func_BlockVisited.insert(curBlock);
//     // timingBase latency_CurBlock = BlockLatencyResourceEvaluation(curBlock); // first, get the
//     latency of the current block

//     // std::string Block_name = curBlock->getName();

//     // for (auto &I : *curBlock)
//     // {
//     //     if (auto call_I = dyn_cast<CallInst>(&I))
//     //     {
//     //         latency_CurBlock = latency_CurBlock -
//     FunctionLatency[call_I->getCalledFunction()]; latency_CurBlock.latency++;
//     //     }
//     // }

//     // timingBase try_critical_path = tmp_critical_path + latency_CurBlock*1;

//     // // (2) get the longest STATE path from entry to the specific block
//     // int CP = (try_critical_path*1).latency;
//     // for (auto B : successors(curBlock))
//     // {
//     //     if (F == B->getParent() && Func_BlockVisited.find(B) == Func_BlockVisited.end())
//     //     {
//     //         int tmp = getFunctionStageNum(try_critical_path*1, F, B);
//     //         if (tmp > CP)
//     //             CP = tmp;
//     //     }
//     // }

//     // Func_BlockVisited.erase(curBlock);

//     // return CP;

//     int res = 0;
//     for (auto &B : *F)
//     {
//         res = res + getStageNumOfBlock(&B);
//     }
//     return res;
// }

// return the resource cost of the function
HI_ArraySensitiveToLoopLevel::resourceBase
HI_ArraySensitiveToLoopLevel::getFunctionResource(Function *F)
{
    if (F->getName().find("llvm.") != std::string::npos ||
        F->getName().find("HIPartitionMux") !=
            std::string::npos) // bypass the "llvm.xxx" functions..
        return resourceBase(0, 0, 0, clock_period);
    return FunctionResource[F];
}

HI_ArraySensitiveToLoopLevel::resourceBase
HI_ArraySensitiveToLoopLevel::costRescheduleFPDSPOperators_forFunction(Function *F)
{
    resourceBase res(0, 0, 0, clock_period);

    std::vector<std::string> checkopcodes = {"fmul", "fadd", "fdiv", "fsub",
                                             "dmul", "dadd", "ddiv", "dsub"};

    // Block2FPDSPReuseScheduleUnits[I->getParent()][opcode].push_back(schUnit);
    for (auto cur_opcode : checkopcodes)
    {
        int op_totalcnt = 0;

        int max_cnt = 0;
        for (auto &cur_block : *F)
        {
            BasicBlock *tmp_block = &cur_block;

            if (Block2FPDSPOpCnt.find(tmp_block) == Block2FPDSPOpCnt.end())
                continue;
            if (Block2FPDSPOpCnt[tmp_block].find(cur_opcode) == Block2FPDSPOpCnt[tmp_block].end())
                continue;

            if (max_cnt < Block2FPDSPOpCnt[tmp_block][cur_opcode])
                max_cnt = Block2FPDSPOpCnt[tmp_block][cur_opcode];
        }

        if (DEBUG)
            *Evaluating_log << "  for function: the amount of floating point operators (refI):"
                            << cur_opcode << " is " << max_cnt << " each cost =["
                            << checkFPOperatorCost(cur_opcode) << "]\n";

        res = res + checkFPOperatorCost(cur_opcode) * max_cnt;
    }

    return res;
}