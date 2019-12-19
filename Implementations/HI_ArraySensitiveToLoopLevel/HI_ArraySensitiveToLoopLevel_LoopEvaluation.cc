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

bool HI_ArraySensitiveToLoopLevel::isInLoop(BasicBlock *BB)
{
    return (Block2Loops.find(BB) != Block2Loops.end());
}

/*
    find the outer loop and evaluate it as a integration
*/
Loop *HI_ArraySensitiveToLoopLevel::getOuterLoopOfBlock(BasicBlock *B)
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

void HI_ArraySensitiveToLoopLevel::getLoopBlockMap(Function *F)
{
    if (DEBUG)
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
        if (DEBUG)
            *Evaluating_log << "--------- Loop: " << L->getName() << " address: " << L->getHeader()
                            << " contains:\n ";
        for (auto BinL : L->getBlocks())
        {
            if (DEBUG)
                *Evaluating_log << "------------- Block: " << BinL->getName()
                                << " address: " << BinL << " \n";
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
Loop *HI_ArraySensitiveToLoopLevel::getInnerUnevaluatedLoop(Loop *outerL)
{
    int dep = 0;
    Loop *tmp_inner_Loop = NULL;
    for (auto tmp_Loop : *outerL) // find the most inner unevaluated loop
    {
        if (DEBUG)
            *Evaluating_log << "--------- checking sub-loop: " << tmp_Loop->getName()
                            << " address:" << tmp_Loop->getHeader()
                            << " -> dep = " << tmp_Loop->getLoopDepth() << " ";
        if (LoopEvaluated.find(tmp_Loop->getHeader()) != LoopEvaluated.end())
        {
            if (DEBUG)
                *Evaluating_log << " which is evaluated.\n";
        }
        else
        {
            if (DEBUG)
                *Evaluating_log << " which is NOT evaluated.\n";
        }

        // larger depth means more inner

        // go to the sub-sub-...-Loop to have a check
        Loop *tmp_inner_Sub_Loop = getInnerUnevaluatedLoop(tmp_Loop);
        if (tmp_inner_Sub_Loop)
        {
            if (tmp_inner_Sub_Loop->getLoopDepth() > dep &&
                LoopEvaluated.find(tmp_inner_Sub_Loop->getHeader()) == LoopEvaluated.end())
            {
                dep = tmp_inner_Sub_Loop->getLoopDepth();
                tmp_inner_Loop =
                    tmp_inner_Sub_Loop; //  the sub-sub-...-loop could be the most inner loop
                if (DEBUG)
                    *Evaluating_log
                        << "--------- update target sub-loop to Loop: " << tmp_inner_Loop->getName()
                        << "\n";
            }
        }
        else
        {
            if (tmp_Loop->getLoopDepth() > dep &&
                LoopEvaluated.find(tmp_Loop->getHeader()) == LoopEvaluated.end())
            {
                dep = tmp_Loop->getLoopDepth();
                tmp_inner_Loop = tmp_Loop; //  no the sub-sub-...-loop could be the most inner loop
                                           //  but current sub-loop could be
                if (DEBUG)
                    *Evaluating_log
                        << "--------- update target sub-loop to Loop: " << tmp_inner_Loop->getName()
                        << "\n";
            }
        }
    }
    auto tmp_Loop = outerL;
    if (tmp_inner_Loop == NULL) // all sub-loops are evaluated, check the loop itself.
    {
        if (DEBUG)
            *Evaluating_log << "--------- checking loop itself: " << tmp_Loop->getName()
                            << " address:" << tmp_Loop->getHeader()
                            << " -> dep = " << tmp_Loop->getLoopDepth() << " ";
        if (LoopEvaluated.find(tmp_Loop->getHeader()) != LoopEvaluated.end())
            if (DEBUG)
                *Evaluating_log << " which is evaluated.\n";
            else if (DEBUG)
                *Evaluating_log << " which is NOT evaluated.\n";

        // larger depth means more inner
        if (tmp_Loop->getLoopDepth() > dep &&
            LoopEvaluated.find(tmp_Loop->getHeader()) == LoopEvaluated.end())
        {
            dep = tmp_Loop->getLoopDepth();
            tmp_inner_Loop = tmp_Loop;
            if (DEBUG)
                *Evaluating_log << "--------- update target loop to Loop: "
                                << tmp_inner_Loop->getName() << "\n";
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

// HI_ArraySensitiveToLoopLevel::timingBase HI_ArraySensitiveToLoopLevel::analyzeOuterLoop(Loop*
// outerL)
// {
//     if (DEBUG) *Evaluating_log << "\n Evaluating Outer Loop Latency for Loop " <<
//     outerL->getName() <<":\n"; if (LoopLatency.find(outerL->getHeader()) != LoopLatency.end())
//     {
//         if (DEBUG) *Evaluating_log << "Done evaluation outer Loop Latency for Loop " <<
//         outerL->getName() << " and its latency is " << LoopLatency[outerL->getHeader()] <<"
//         cycles.\n\n\n"; return LoopLatency[outerL->getHeader()];
//     }
//     Loop *cur_Loop;
//     timingBase outerL_latency (-1,-1,1,clock_period);
//     timingBase tmp_total_latency(0,0,1,clock_period);
//     timingBase origin_latency(0,0,1,clock_period);
//     // (1) iteratively handle the most inner loop
//     cur_Loop = getInnerUnevaluatedLoop(outerL);
//     while (cur_Loop!=NULL)
//     {
//         if (DEBUG) *Evaluating_log << "-- Handling the inner Loop " << cur_Loop->getName()
//         <<":\n"; BasicBlock *tmp_LoopHeader = cur_Loop->getHeader(); //get the header of the loop
//         if (DEBUG) *Evaluating_log << "---- its header: " << tmp_LoopHeader->getName() <<":\n";
//         SmallVector<BasicBlock*, 8>  tmp_ExitingBlocks;
//         cur_Loop->getExitingBlocks(tmp_ExitingBlocks);  // get the exiting blocks of the loop
//         if (tmp_ExitingBlocks.size() != 1)
//         {
//             assert(tmp_ExitingBlocks.size() > 0);
//             print_warning("the loop could be better to have only one exiting block for the
//             accuracy of latency evaluation.");
//         }
//         for (auto B_it:tmp_ExitingBlocks)
//         {
//             if (DEBUG) *Evaluating_log << "---- its exiting block(s): " << B_it->getName() <<" --
//             ";
//         }
//         if (DEBUG) *Evaluating_log << "\n";

//         // (2) traverse the block in loop by DFS to find the longest path
//         timingBase max_critial_path_in_curLoop(0,0,1,clock_period);
//         resourceBase resourceAccumulator(0,0,0,clock_period);
//         tmp_BlockCriticalPath_inLoop.clear(); // record the block level critical path in the loop
//         tmp_SubLoop_CriticalPath.clear(); // record the critical path to the end of sub-loops in
//         the loop

//         BlockVisited.clear();
//         LoopLatencyResourceEvaluation_traversFromHeaderToExitingBlocks(origin_latency, cur_Loop,
//         tmp_LoopHeader, resourceAccumulator); BlockCriticalPath_inLoop[cur_Loop] =
//         tmp_BlockCriticalPath_inLoop;

//         for (auto tmp_it : tmp_BlockCriticalPath_inLoop)
//             if (tmp_it.second > max_critial_path_in_curLoop)
//                 max_critial_path_in_curLoop = tmp_it.second;
//         for (auto tmp_it : tmp_SubLoop_CriticalPath)
//             if (tmp_it.second > max_critial_path_in_curLoop)
//                 max_critial_path_in_curLoop = tmp_it.second;

//         Loop2CP[cur_Loop->getHeader()] = max_critial_path_in_curLoop.latency;
//         // (3) get the total latency by TripCount * IterationLatency? (consider whether the loop
//         is pipelined) int II_for_loop = checkIIForLoop(cur_Loop, tmp_BlockCriticalPath_inLoop);
//         if (II_for_loop <= -1 || II_for_loop >= max_critial_path_in_curLoop.latency)
//         {
//             if (II_for_loop >= max_critial_path_in_curLoop.latency)
//             {
//                 print_warning("Loop  " + std::string(cur_Loop->getName()) + " is pipelined with
//                 II="
//                                 + std::to_string(II_for_loop) + " which means it is not worthy to
//                                 pipeline the loop.");
//             }
//             tmp_total_latency = SE->getSmallConstantMaxTripCount(cur_Loop) *
//             max_critial_path_in_curLoop;
//         }
//         else
//         {
//             // llvm::errs() << cur_Loop->getHeader()->getName() << " is pipelined with II=" <<
//             II_for_loop << "\n"; std::string tmp_loop_name =
//             cur_Loop->getHeader()->getParent()->getName(); tmp_loop_name += "-"; tmp_loop_name +=
//             cur_Loop->getHeader()->getName();

//             int tmp_unroll_factor = -1;
//             int original_tripcount = -1;
//             if (LoopLabel2UnrollFactor.find(IRLoop2LoopLabel[tmp_loop_name]) !=
//             LoopLabel2UnrollFactor.end())
//             {
//                 tmp_unroll_factor = LoopLabel2UnrollFactor[IRLoop2LoopLabel[tmp_loop_name]];
//                 original_tripcount = IRLoop2OriginTripCount[tmp_loop_name];
//             }

//             // original trip count is not the multiple of the unroll factor
//             // and the overhead of the remainder iterations is too high
//             // VivadoHLS might shut down the last iteration ASAP
//             if (tmp_unroll_factor > 0 && original_tripcount > 0
//                 && (original_tripcount%tmp_unroll_factor<original_tripcount/2)
//                 && II_for_loop > 10)
//             {
//                 if (DEBUG) *Evaluating_log << "the overhead of the remainder iterations after
//                 unrolling is too high and approximately reduce the latency of the last iteration
//                 after unrolling\n"; if (DEBUG) *Evaluating_log << "the original trip count is "
//                 << original_tripcount << "\n"; timingBase tmp_total_latency_0 =
//                 (SE->getSmallConstantMaxTripCount(cur_Loop)-2) * II_for_loop *
//                 timingBase(1,0,1,clock_period) + max_critial_path_in_curLoop; timingBase
//                 tmp_total_latency_1 = (SE->getSmallConstantMaxTripCount(cur_Loop)-1) *
//                 II_for_loop * timingBase(1,0,1,clock_period) + max_critial_path_in_curLoop / 2;
//                 if (tmp_total_latency_0 > tmp_total_latency_1)
//                     tmp_total_latency = tmp_total_latency_0;
//                 else
//                     tmp_total_latency = tmp_total_latency_1;
//             }
//             else
//             {
//                 tmp_total_latency = (SE->getSmallConstantMaxTripCount(cur_Loop)-1) * II_for_loop
//                 * timingBase(1,0,1,clock_period) + max_critial_path_in_curLoop;
//             }

//             resourceAccumulator = resourceAccumulator
//                                   + costRescheduleIntDSPOperators_forLoop(cur_Loop,
//                                   tmp_BlockCriticalPath_inLoop, II_for_loop);
//             recordCostRescheduleFPDSPOperators_forLoop(cur_Loop, tmp_BlockCriticalPath_inLoop,
//             II_for_loop);

//             // assert(false && "TODO : implementation!!");
//         }

//         // COMMENT because preheader is not in the loop enity and if the prehearder is
//         calculated, it is actually duplicated calculation.
//         // but just need to add one cycle, as it seems that in VivadoHLS, Loops are regarded as
//         function and the "call" will take one cycle
//         // if (cur_Loop->getLoopPreheader())
//         //     tmp_total_latency = tmp_total_latency +
//         BlockLatencyResourceEvaluation(cur_Loop->getLoopPreheader()); tmp_total_latency =
//         tmp_total_latency + timingBase(1,0,1,clock_period);

//         // (4) mark the blocks in loop with the loop latency, so later processing can regard this
//         loop as an integration BlockVisited.clear();
//         MarkBlock_traversFromHeaderToExitingBlocks(tmp_total_latency, cur_Loop, tmp_LoopHeader);
//         LoopLatency[cur_Loop->getHeader()] = tmp_total_latency;
//         LoopResource[cur_Loop->getHeader()] = resourceAccumulator;
//         LoopEvaluated.insert(cur_Loop->getHeader());

//         if (DEBUG) *Evaluating_log << "Trip Count for Loop " << cur_Loop->getName() << " is " <<
//         SE->getSmallConstantMaxTripCount(cur_Loop) <<"\n"; if (DEBUG) *Evaluating_log << "Done
//         evaluation Loop Latency for Loop " << cur_Loop->getName() << " and its latency is " <<
//         tmp_total_latency <<" cycles and its resource cost is: " <<
//         LoopResource[cur_Loop->getHeader()] << ".\n\n\n";

//         // (1) iteratively handle the most inner loop
//         cur_Loop = getInnerUnevaluatedLoop(outerL);
//     }
//     outerL_latency = tmp_total_latency; // finally, we will get the latency of outer loop in the
//     last iteration if (DEBUG) *Evaluating_log << "Done evaluation outer Loop Latency for Loop "
//     << outerL->getName() << " and its latency is " << outerL_latency <<" cycles and its resource
//     cost is: " << LoopResource[outerL->getHeader()] << ".\n\n\n"; assert(outerL_latency.latency >
//     -0.5 && "The latency for a loop should be not be negative"); return outerL_latency*1;
// }

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
// void
// HI_ArraySensitiveToLoopLevel::LoopLatencyResourceEvaluation_traversFromHeaderToExitingBlocks(HI_ArraySensitiveToLoopLevel::timingBase
// tmp_critical_path, Loop* L, BasicBlock *curBlock, HI_ArraySensitiveToLoopLevel::resourceBase
// &resourceAccumulator)
// {

//     // (1) Mark the block visited, as a step of typical DFS
//     BlockVisited.insert(curBlock);

//     if (DEBUG) *Evaluating_log << "---- traverser arrive Block: " << curBlock->getName() <<" ";

//     // (2) Check whether the search reaches a block in the sub-loops
//     if (Block2EvaluatedLoop.find(curBlock) != Block2EvaluatedLoop.end())
//     {
//         // (3a) -- If it is a block in sub-loops, regard the loop as intergration and update the
//         critical path if necessary (max(ori_CP, lastStateCP + LoopLatency)). Loop* tmp_SubLoop =
//         Block2EvaluatedLoop[curBlock]; if (DEBUG) *Evaluating_log << " which is evluated in Loop
//         " << tmp_SubLoop->getName() <<" "; if (DEBUG) *Evaluating_log << " LoopLatency =  " <<
//         LoopLatency[tmp_SubLoop->getHeader()] <<" "; timingBase try_critical_path =
//         tmp_critical_path + LoopLatency[tmp_SubLoop->getHeader()];  // first, get the critical
//         path to the end of sub-loop if (DEBUG) *Evaluating_log << " NewCP =  " <<
//         try_critical_path <<" "; bool checkFlag = false;

//         if (tmp_SubLoop_CriticalPath.find(tmp_SubLoop) == tmp_SubLoop_CriticalPath.end() )
//         {
//             assert(LoopResource.find(tmp_SubLoop->getHeader())!=LoopResource.end());
//             resourceAccumulator = resourceAccumulator + LoopResource[tmp_SubLoop->getHeader()];
//             checkFlag = true;
//         }
//         else if (try_critical_path > tmp_SubLoop_CriticalPath[tmp_SubLoop]) checkFlag = true;

//         if (checkFlag)
//         {
//             if (tmp_SubLoop_CriticalPath.find(tmp_SubLoop) != tmp_SubLoop_CriticalPath.end())
//             {
//                 if (DEBUG) *Evaluating_log << " OriCP =  " <<
//                 tmp_SubLoop_CriticalPath[tmp_SubLoop] <<"\n";
//             }
//             else
//             {
//                 if (DEBUG) *Evaluating_log << " No OriCP" <<"\n";
//             }
//             tmp_SubLoop_CriticalPath[tmp_SubLoop] = try_critical_path;

//              //  (3a)  -- find the successors of the loop by checking its exiting blocks'
//              successors and continue the DFS
//             SmallVector<BasicBlock*, 8>  tmp_SubLoop_ExitingBlocks;
//             tmp_SubLoop->getExitingBlocks(tmp_SubLoop_ExitingBlocks);
//             for (auto ExitB : tmp_SubLoop_ExitingBlocks)
//             {
//                 for (auto B : successors(ExitB))
//                 {
//                     if (L->contains(B) && BlockVisited.find(B) == BlockVisited.end()  &&
//                     !tmp_SubLoop->contains(B))
//                     {
//                         if (DEBUG) *Evaluating_log << "---- loop continue to traverser to Block:
//                         " << B->getName()  << " from " << curBlock->getName() <<" ";
//                         LoopLatencyResourceEvaluation_traversFromHeaderToExitingBlocks(try_critical_path,L,B,resourceAccumulator);
//                     }
//                 }
//             }

//         }

//     }
//     else
//     {
//         //     (3b) -- If it is a block out of sub-loops, evaluate the block latency and update
//         the critical path if necessary (max(ori_CP, lastStateCP + BlockLatency)). if (DEBUG)
//         *Evaluating_log << " which is  not evaluated in Loop " << " "; timingBase
//         latency_CurBlock = BlockLatencyResourceEvaluation(curBlock); // first, get the latency of
//         the current block timingBase try_critical_path = tmp_critical_path + latency_CurBlock; if
//         (DEBUG) *Evaluating_log << "---- latencyBlock =  " << latency_CurBlock <<" "; if (DEBUG)
//         *Evaluating_log << " NewCP =  " << try_critical_path <<" "; bool checkFlag = false;

//         if (tmp_BlockCriticalPath_inLoop.find(curBlock) == tmp_BlockCriticalPath_inLoop.end() )
//         {
//             assert(BlockResource.find(curBlock)!=BlockResource.end());
//             resourceAccumulator = resourceAccumulator + BlockResource[curBlock];
//             checkFlag = true;
//         }
//         else if (try_critical_path > tmp_BlockCriticalPath_inLoop[curBlock]) checkFlag = true;

//         if (checkFlag) // update the block-level critical path
//         {
//             if (tmp_BlockCriticalPath_inLoop.find(curBlock) !=
//             tmp_BlockCriticalPath_inLoop.end())
//             {
//                 if (DEBUG) *Evaluating_log << " OriCP =  " <<
//                 tmp_BlockCriticalPath_inLoop[curBlock] <<"\n";
//             }
//             else
//             {
//                 if (DEBUG) *Evaluating_log << " No OriCP" <<"\n";
//             }

//             tmp_BlockCriticalPath_inLoop[curBlock] = try_critical_path;
//             BlockBegin_inLoop[curBlock] = tmp_critical_path;  // only one level of sub-loop in a
//             outermost loop will be pipelined
//                                                                // so don't worry about duplicated
//                                                                use of this map

//             // (3b)  -- find the successors of the block and continue the DFS
//             for (auto B : successors(curBlock))
//             {
//                 if (L->contains(B) && BlockVisited.find(B) == BlockVisited.end())
//                 {
//                     if (DEBUG) *Evaluating_log << "---- loop continue to traverser to Block: " <<
//                     B->getName()  << " from " << curBlock->getName() << " ";
//                     LoopLatencyResourceEvaluation_traversFromHeaderToExitingBlocks(try_critical_path,L,B,resourceAccumulator);
//                 }
//             }
//         }
//     }
//     // (4) Release the block from visited flag, as a step of typical DFS
//     BlockVisited.erase(curBlock);
// }

/*
    Simply mark all the blocks in the loop with the totoal_latency by DFS-traverse
*/
void HI_ArraySensitiveToLoopLevel::MarkBlock_traversFromHeaderToExitingBlocks(
    HI_ArraySensitiveToLoopLevel::timingBase total_latency, Loop *L, BasicBlock *curBlock)
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

// // get the II factor for loop pipelining, if there is directives of pipeline for this loop
// int HI_ArraySensitiveToLoopLevel::checkIIForLoop(Loop *curLoop,  std::map<BasicBlock*,
// timingBase> &tmp_BlockCriticalPath_inLoop)
// {
//     BasicBlock *Header = curLoop->getHeader();
//     std::string tmp_loop_name = Header->getParent()->getName();
//     tmp_loop_name += "-";
//     tmp_loop_name += Header->getName();
//     if (IRLoop2LoopLabel.find(tmp_loop_name) == IRLoop2LoopLabel.end())
//         return -1;

//     std::string label = IRLoop2LoopLabel[tmp_loop_name];
//     if (LoopLabel2II.find(label) == LoopLabel2II.end())
//         return -1;

//     if (DEBUG) *Evaluating_log << "--------- Loop is applied pipelining pragma\n";
//     // When pipelined, the loop should not have any sub-loop inside
//     if (curLoop->getSubLoops().size()>0)
//     {
//         std::string loopName = curLoop->getName();
//         print_warning( loopName + " should not be pipelined with subloops.");
//         return -1;
//     }

//     int min_II = LoopLabel2II[label];
//     // assert(false && "TODO");

//     int II_BRAM = checkAccessIIForLoop(curLoop);
//     int II_BRAM_enum = checkAccessIIForLoop_enumerateCheck(curLoop);

//     if (II_BRAM != II_BRAM_enum)
//     {
//         print_warning("Loop II of " + std::string(curLoop->getName()) + " has conflict
//         prediction: ideal=" + std::to_string(II_BRAM) + " enum=" + std::to_string(II_BRAM_enum)
//         );
//     }

//     // II_BRAM_enum is more reliable but very conservative
//     // so we believe that VivadoHLS can optimize
//     // II_BRAM_enum by reducing it by 1
//     if (II_BRAM_enum - 1 > II_BRAM)
//         II_BRAM = II_BRAM_enum - 1;

//     int II_dependence = checkDependenceIIForLoop(curLoop);

//     if (DEBUG) *Evaluating_log << "--------- Loop pipeline expected II is " << min_II << "\n";
//     if (DEBUG) *Evaluating_log << "--------- Loop pipeline BRAM-related II is " << II_BRAM <<
//     "\n"; if (DEBUG) *Evaluating_log << "--------- Loop pipeline Dependence-related II is " <<
//     II_dependence << "\n";

//     if (II_BRAM > min_II)
//     {
//         min_II = II_BRAM;
//         print_warning(std::string(curLoop->getName()) + " has hit the limitation of BRAM port,
//         min_II is updated to " + std::to_string(min_II) ); if (DEBUG) *Evaluating_log <<
//         "--------- Loop pipeline cannot achieve expected II because of BRAM limitation\n";
//     }

//     if (II_dependence > min_II)
//     {
//         min_II = II_dependence;
//         print_warning(std::string(curLoop->getName()) + " has loop carried dependence, min_II is
//         updated to " + std::to_string(min_II) ); if (DEBUG) *Evaluating_log << "--------- Loop
//         pipeline cannot achieve expected II because of loop carried dependence\n";
//     }

//     // TODO:
//     // check the II related to loop carried dependece
//     // II = ceiling ((C_store - C_load + 1) / dep_distance)

//     // Evaluating_log->flush();
//     // ArrayLog->flush();
//     return min_II;
// }

// the BRAM-related II for the loop
int HI_ArraySensitiveToLoopLevel::checkAccessIIForLoop(Loop *curLoop)
{
    int min_II = 1;
    std::map<std::pair<Value *, partition_info>, int> existingAccessCntForLoop;
    for (auto tmp_B : curLoop->getBlocks())
    {
        if (accessCounterForBlock.find(tmp_B) == accessCounterForBlock.end())
        {
            continue;
        }
        for (auto it_value_partition_2_cnt : accessCounterForBlock[tmp_B])
        {
            Value *target = it_value_partition_2_cnt.first.first;
            partition_info partID = it_value_partition_2_cnt.first.second;
            if (existingAccessCntForLoop.find(it_value_partition_2_cnt.first) ==
                existingAccessCntForLoop.end())
            {
                existingAccessCntForLoop[it_value_partition_2_cnt.first] =
                    it_value_partition_2_cnt.second;
            }
            else
            {
                existingAccessCntForLoop[it_value_partition_2_cnt.first] +=
                    it_value_partition_2_cnt.second;
            }
            int accessTotalCntInLoop = existingAccessCntForLoop[it_value_partition_2_cnt.first];
            if ((accessTotalCntInLoop + 1) / 2 > min_II)
            {
                if (DEBUG)
                    *Evaluating_log << "--------- min II is updated to "
                                    << (accessTotalCntInLoop + 1) / 2 << " because\n";
                if (DEBUG)
                    *Evaluating_log << "--------- access to the partition#" << partID << " of "
                                    << *target << " exceed the bandwidth\n";
                min_II = (accessTotalCntInLoop + 1) / 2;
            }
        }
    }
    return min_II;
}

// check the BRAM-related II for the loop by enumerating II and checking port constraint for each
// cycle for each partition
int HI_ArraySensitiveToLoopLevel::checkAccessIIForLoop_enumerateCheck(Loop *curLoop)
{
    if (DEBUG)
        *ArrayLog
            << "\n\n============================\n \n checkAccessIIForLoop_enumerateCheck for Loop:"
            << curLoop->getName() << " \n ============================\n \n";

    // initialize an empty seq for later tests for different partitions of different targets
    int latLoop = Loop2CP[curLoop->getHeader()];
    int lenSeq = latLoop + 5; // maybe to leave some space could be better

    std::vector<int> test_accessSeq(lenSeq);

    accessPartitionsForIITest.clear();
    for (auto tmp_B : curLoop->getBlocks())
    {
        if (accessCounterForBlock.find(tmp_B) == accessCounterForBlock.end())
        {
            continue;
        }
        for (auto it_value_partition_2_cnt : accessCounterForBlock[tmp_B])
        {
            if (accessPartitionsForIITest.find(it_value_partition_2_cnt.first) ==
                accessPartitionsForIITest.end())
                accessPartitionsForIITest.insert(it_value_partition_2_cnt.first);
        }
    }

    if (DEBUG)
        *ArrayLog << "\n\n============================\n \n printing out accesses in Loop:"
                  << curLoop->getName() << " \n ============================\n \n";

    if (DEBUG)
        for (auto val_partition_pair : accessPartitionsForIITest) // check whether all partitions
                                                                  // are met the constraints of port
        {
            *ArrayLog << "   access with partition#" << val_partition_pair.second
                      << " of target:" << val_partition_pair.first->getName() << "\n";

            for (auto block_cycle_pair : targetPartition2BlockCycleAccessCnt[val_partition_pair])
            {
                *ArrayLog << block_cycle_pair.first->getParent()->getName() << "-"
                          << block_cycle_pair.first->getName() << ":" << block_cycle_pair.second
                          << "\n";
            }
            *ArrayLog << "\n";
        }

    bool failFlag = false;
    for (int test_II = 1; test_II <= latLoop; test_II++)
    {
        failFlag = false;
        if (DEBUG)
            *ArrayLog << "testing II=" << test_II << "\n";
        for (auto val_partition_pair : accessPartitionsForIITest) // check whether all partitions
                                                                  // are met the constraints of port
        {
            if (DEBUG)
                *ArrayLog << "   testing with partition#" << val_partition_pair.second
                          << " of target:" << val_partition_pair.first->getName() << "\n";

            for (int i = 0; i < lenSeq; i++)
                test_accessSeq[i] = 0;

            for (int test_iter = 0; test_iter < latLoop; test_iter += test_II)
            {
                for (auto block_cycle_pair :
                     targetPartition2BlockCycleAccessCnt[val_partition_pair])
                {
                    if (!curLoop->contains(block_cycle_pair.first))
                        continue;

                    int slotoffset = BlockBegin_inLoop[block_cycle_pair.first].latency +
                                     block_cycle_pair.second + test_iter;
                    if (DEBUG)
                        *ArrayLog << "----->" << block_cycle_pair.first->getParent()->getName()
                                  << "-" << block_cycle_pair.first->getName() << ":"
                                  << block_cycle_pair.second << " exactSlot:" << slotoffset << "\n";

                    if (slotoffset <= latLoop)
                    {
                        test_accessSeq[slotoffset]++;
                        if (test_accessSeq[slotoffset] > 2)
                        {
                            failFlag = true;
                            if (DEBUG)
                                *ArrayLog << "   failed at: test_iter=" << test_iter << "\n";
                            if (DEBUG)
                            {
                                *ArrayLog << "   failed pattern :";
                                for (int i = 0; i < lenSeq; i++)
                                    *ArrayLog << test_accessSeq[i] << " ";
                                *ArrayLog << "\n";
                            }
                            break;
                        }
                    }
                }
                if (failFlag)
                    break;
                else if (test_iter == 0)
                {
                    if (DEBUG)
                        *ArrayLog << "   initial access pattern :";
                    if (DEBUG)
                        for (int i = 0; i < lenSeq; i++)
                            *ArrayLog << test_accessSeq[i] << " ";
                    if (DEBUG)
                        *ArrayLog << "\n";
                }
            }
            if (failFlag)
                break;
        }
        if (!failFlag)
        {
            if (DEBUG)
                *ArrayLog << "  II test passed with II=" << test_II << "\n";
            return test_II;
        }
        if (DEBUG)
            *ArrayLog << "\n\n\n";
    }
}

// // the Dependence-related II for the loop
// int HI_ArraySensitiveToLoopLevel::checkDependenceIIForLoop(Loop* curLoop)
// {
//     if (DEBUG) *ArrayLog << "\n========================\n\ncheckDependenceIIForLoop: "
//     <<curLoop->getName() << "\n========================\n\n=" << "\n"; int min_II = 1;
//     std::vector<Instruction*> potentialAccesses;

//     for (auto tmp_B : curLoop->getBlocks())
//     {
//         for (auto &I : *tmp_B)
//         {
//             if (I.getOpcode() == Instruction::Load || I.getOpcode() == Instruction::Store )
//             {
//                 potentialAccesses.push_back(&I);
//             }
//             else if (I.getOpcode() == Instruction::Call)
//             {
//                 if (CallInst *callI = dyn_cast<CallInst>(&I))
//                 {
//                     for (int i=0;i<callI->getNumArgOperands();i++)
//                     {
//                         if (callI->getArgOperand(i)->getType()->isPointerTy())
//                         {
//                            potentialAccesses.push_back(&I);
//                            break;
//                         }
//                     }
//                 }
//             }
//         }
//     }

//     if (DEBUG) *ArrayLog << "\npotential accesses are:\n";
//     if (DEBUG) for (auto potentialAccess : potentialAccesses)
//     {
//         *ArrayLog << "    " << *potentialAccess << " ==> [";
//         for (auto target : Instruction2Target[potentialAccess])
//             *ArrayLog << target->getName() << ", ";
//         *ArrayLog << "]\n";
//     }

//     InstInst2DependenceDistance.clear();

//     for (auto potentialAccess0 : potentialAccesses)
//     {
//         if (potentialAccess0->getOpcode() == Instruction::Store || potentialAccess0->getOpcode()
//         == Instruction::Call)
//         {
//             for (auto potentialAccess1 : potentialAccesses)
//             {
//                 if (potentialAccess1 == potentialAccess0)
//                     break;
//                 if (potentialAccess1->getOpcode() == Instruction::Load ||
//                 potentialAccess1->getOpcode() == Instruction::Call)
//                 {
//                     checkLoopCarriedDependent(potentialAccess0, potentialAccess1, curLoop);
//                 }
//             }
//         }

//     }

//     for (auto InstInst2DependenceDistance_pair : InstInst2DependenceDistance)
//     {
//         Instruction *W_I = InstInst2DependenceDistance_pair.first.first;
//         Instruction *R_I = InstInst2DependenceDistance_pair.first.second;
//         *ArrayLog << "checking loop-carried dependence II: LOAD:" << *R_I << " <===>  STORE:" <<
//         *W_I << "\n"; int W_I_time_offset = BlockBegin_inLoop[W_I->getParent()].latency +
//         Inst_Schedule[W_I].second; int R_I_time_offset = findEarlietUseTimeInTheLoop(curLoop,
//         R_I); // BlockBegin_inLoop[R_I->getParent()].latency +  Inst_Schedule[R_I].second;
//         *ArrayLog << "    W_I_time_offset:" << W_I_time_offset << " <===>  R_I_time_offset:" <<
//         R_I_time_offset << "\n";
//         // here, we assume that the load can be rescheduled as late as possible
//         // therefore, we need to find when its ealiest user use the data

//         int tmp_II = ( W_I_time_offset - R_I_time_offset +
//         InstInst2DependenceDistance_pair.second ) / InstInst2DependenceDistance_pair.second; if
//         (tmp_II > min_II)
//         {
//             min_II = tmp_II;
//             *ArrayLog << "\n      min_II updated to min_II=" << min_II << " due to RAW loop
//             carried dependence between store: [" << *W_I << "] and load [" << *R_I << "]\n";
//             *ArrayLog << "      W_I_time_offset=" << W_I_time_offset << " R_I_time_offset=" <<
//             R_I_time_offset << " DependenceDistance=" << InstInst2DependenceDistance_pair.second
//             << "\n";
//         }
//     }

//     return min_II;
// }

// get the time slot of the instruction in the loop
int HI_ArraySensitiveToLoopLevel::getTimeslotForInstInLoop(
    Loop *curLoop, Instruction *I, std::map<BasicBlock *, timingBase> &tmp_BlockCriticalPath_inLoop)
{
    int res = 1;
    int block_offset = BlockBegin_inLoop[I->getParent()].latency;
    int I_offset = Inst_Schedule[I].second;
    Inst2TimeSlotInLoop[I] = block_offset + I_offset;
    return block_offset + I_offset;
}

// get the time slot of the instruction in the loop
int HI_ArraySensitiveToLoopLevel::getTimeslotForInstInLoop(Instruction *I)
{
    assert(Inst2TimeSlotInLoop.find(I) != Inst2TimeSlotInLoop.end());
    return Inst2TimeSlotInLoop[I];
}

// check whether the two instructions have loop carried dependence
// if there is such dependence, record the distance in InstInst2DependenceDistance
void HI_ArraySensitiveToLoopLevel::checkLoopCarriedDependent(Instruction *I0, Instruction *I1,
                                                             Loop *curLoop)
{
    if (DEBUG)
        *ArrayLog << "checking LoopCarriedDependent:" << *I0 << " <=> " << *I1 << "\n";

    if (!hasSameTargets(I0, I1))
    {
        if (DEBUG)
            *ArrayLog << "**** different targets of " << *I0 << " <=> " << *I1 << "\n";
        return;
    }

    if (I0->getOpcode() == Instruction::Call && I1->getOpcode() == Instruction::Load)
    {
        InstInst2DependenceDistance[std::pair<Instruction *, Instruction *>(I0, I1)] = 1;
        if (DEBUG)
            *ArrayLog << "**** distance:" << *I0 << " <=> " << *I1 << " =1\n";
        return;
    }
    else if (I0->getOpcode() == Instruction::Call && I1->getOpcode() == Instruction::Call)
    {
        InstInst2DependenceDistance[std::pair<Instruction *, Instruction *>(I0, I1)] = 1;
        if (DEBUG)
            *ArrayLog << "**** distance:" << *I0 << " <=> " << *I1 << " =1\n";
        return;
    }
    else if (I0->getOpcode() == Instruction::Store && I1->getOpcode() == Instruction::Call)
    {
        InstInst2DependenceDistance[std::pair<Instruction *, Instruction *>(I0, I1)] = 1;
        if (DEBUG)
            *ArrayLog << "**** distance:" << *I0 << " <=> " << *I1 << " =1\n";
        return;
    }

    assert(I0->getOpcode() == Instruction::Store && I1->getOpcode() == Instruction::Load);

    Instruction *pointer_I0 = nullptr, *pointer_I1 = nullptr;
    if (I0->getOpcode() == Instruction::Load)
    {
        pointer_I0 = dyn_cast<Instruction>(I0->getOperand(0));
    }
    else if (I0->getOpcode() == Instruction::Store)
    {
        pointer_I0 = dyn_cast<Instruction>(I0->getOperand(1));
    }
    assert(pointer_I0 && pointer_I0->getOpcode() == Instruction::IntToPtr &&
           "ITP should be found for the access instruction");

    if (I1->getOpcode() == Instruction::Load)
    {
        pointer_I1 = dyn_cast<Instruction>(I1->getOperand(0));
    }
    else if (I1->getOpcode() == Instruction::Store)
    {
        pointer_I1 = dyn_cast<Instruction>(I1->getOperand(1));
    }

    assert(pointer_I1 && pointer_I1->getOpcode() == Instruction::IntToPtr &&
           "ITP should be found for the access instruction");

    std::string tmp0(""), tmp1("");
    raw_string_ostream *SCEV_Stream0 = new raw_string_ostream(tmp0);
    raw_string_ostream *SCEV_Stream1 = new raw_string_ostream(tmp1);

    const SCEV *tmp_S0 = SE->getSCEV(pointer_I0->getOperand(0));
    const SCEV *tmp_S1 = SE->getSCEV(pointer_I1->getOperand(0));

    Optional<APInt> res = computeConstantDifference(tmp_S0, tmp_S1);

    if (res != None)
    {
        int offset_dis = res.getValue().getSExtValue();
        if (DEBUG)
            *ArrayLog << " offset_dis=" << offset_dis << "\n";

        if (offset_dis > 0)
        {
            int stepLen = getStepLength(tmp_S0, tmp_S1);
            if (DEBUG)
                *ArrayLog << " stepLen=" << stepLen << "\n";
            if (offset_dis % stepLen == 0)
            {
                InstInst2DependenceDistance[std::pair<Instruction *, Instruction *>(I0, I1)] =
                    offset_dis / stepLen;
                if (DEBUG)
                    *ArrayLog << "**** distance:" << *I0 << " <=> " << *I1 << " ="
                              << offset_dis / stepLen << " offset_dis=" << offset_dis
                              << " stepLen=" << stepLen << "\n";
            }
            return;
        }
        else if (offset_dis == 0)
        {
            if (!checkConstantAccessInLoop(tmp_S0, tmp_S1, curLoop))
                return;
            int stepLen = getStepLength(tmp_S0, tmp_S1);
            if (DEBUG)
                *ArrayLog << " stepLen=" << stepLen << "\n";
            InstInst2DependenceDistance[std::pair<Instruction *, Instruction *>(I0, I1)] = 1;
            if (DEBUG)
                *ArrayLog << "**** distance:" << *I0 << " <=> " << *I1 << " =" << 1
                          << " offset_dis=" << offset_dis << " stepLen=" << stepLen << "\n";

            return;
        }
        else
        {
            if (DEBUG)
                *ArrayLog << "**** distance:" << *I0 << " <=> " << *I1 << " = None\n";
            return;
        }
    }
    if (DEBUG)
        *ArrayLog << "**** distance:" << *I0 << " <=> " << *I1 << " =1\n";
    InstInst2DependenceDistance[std::pair<Instruction *, Instruction *>(I0, I1)] = 1;
}

bool HI_ArraySensitiveToLoopLevel::hasSameTargets(Instruction *I0, Instruction *I1)
{
    for (auto target0 : Instruction2Target[I0])
        for (auto target1 : Instruction2Target[I1])
            if (target0 == target1)
                return true;
    return false;
}

// // find the earliest user of the load instruction (maybe for reschedule)
// int HI_ArraySensitiveToLoopLevel::findEarlietUseTimeInTheLoop(Loop* curLoop, Instruction
// *ori_R_I)
// {
//     // initialize the result with the current time slot
//     Instruction* R_I = nullptr;
//     bool muxDelayIsHigh = false;
//     if (auto callI = dyn_cast<CallInst>(ori_R_I->use_begin()->getUser()))
//     {
//         if (callI->getCalledFunction()->getName().find("HIPartitionMux") != std::string::npos)
//         {
//             R_I = callI; // if there is mux for the load, check the users of the mux instead
//             timingBase tmpMuxDelay = getInstructionLatency(R_I);
//             if (((tmpMuxDelay.timing+3.25) / clock_period) > 0.5) // when the mux delay is too
//             high to fit in the current {                                                     //
//             cycle, we should leave one more cycle before the earliest user
//                 muxDelayIsHigh = true;
//             }
//         }
//         else
//         {
//             R_I = ori_R_I;
//         }
//     }
//     else
//     {
//         R_I = ori_R_I;
//     }

//     int R_I_time_offset = BlockBegin_inLoop[R_I->getParent()].latency +
//     Inst_Schedule[R_I].second; int earliest_time_slot = 100000000;
//     // here, we assume that the load can be rescheduled as late as possible
//     // therefore, we need to find when its ealiest user use the data

//     for (auto tmp_user : R_I->users())
//     {
//         *ArrayLog << "       checking user: " << *tmp_user << "\n";
//         if (Instruction *tmp_user_I = dyn_cast<Instruction>(tmp_user))
//         {

//             if (curLoop->contains(tmp_user_I->getParent()))
//             {
//                 // the load might be scheduled in one or two cycles in advance
//                 int cycle_inadvance = 1;

//                 if (muxDelayIsHigh)
//                     cycle_inadvance = 2;

//                 if (tmp_user_I->getOpcode() == Instruction::Mul || tmp_user_I->getOpcode() ==
//                 Instruction::UDiv ||
//                     tmp_user_I->getOpcode() == Instruction::FDiv || tmp_user_I->getOpcode() ==
//                     Instruction::FSub || tmp_user_I->getOpcode() == Instruction::FMul ||
//                     tmp_user_I->getOpcode() == Instruction::FAdd || tmp_user_I->getOpcode() ==
//                     Instruction::URem || tmp_user_I->getOpcode() == Instruction::SRem ||
//                     tmp_user_I->getOpcode() == Instruction::SDiv )
//                     cycle_inadvance = 2;

//                 if (InstructionCriticalPath_inBlock[tmp_user_I->getParent()][tmp_user_I].timing -
//                 getInstructionLatency(tmp_user_I).timing <= 0.001)
//                 {
//                     if (getInstructionLatency(tmp_user_I).timing + 3.25 > 0.5 * clock_period)
//                     {
//                         // this situation, the scheduling of the block is relatively tight,
//                         reschedule the load ealier. cycle_inadvance = 2;
//                     }
//                 }

//                 if (tmp_user_I->getOpcode() == Instruction::FDiv || tmp_user_I->getOpcode() ==
//                 Instruction::FSub ||
//                     tmp_user_I->getOpcode() == Instruction::FMul || tmp_user_I->getOpcode() ==
//                     Instruction::FAdd )
//                 {
//                     std::vector<std::string> checkopcodes = {"fmul", "fadd", "fdiv", "fsub",
//                     "dmul", "dadd", "ddiv", "dsub"  }; std::string tmp_opcode_str =
//                     InstToOpcodeString(tmp_user_I); for (auto checkcode : checkopcodes)
//                         if (checkcode == tmp_opcode_str)
//                         {
//                             if (getInstructionLatency(tmp_user_I).latency >=2)
//                             {
//                                 cycle_inadvance = -1;
//                                 break;
//                                 // maybe for these kinds of instruction, VivadoHLS can forward
//                                 the result from others iteration
//                             }

//                         }
//                 }

//                 int tmp_tmp_slot = BlockBegin_inLoop[tmp_user_I->getParent()].latency +
//                 Inst_Schedule[tmp_user_I].second - cycle_inadvance;

//                 if (tmp_tmp_slot < earliest_time_slot && R_I_time_offset < tmp_tmp_slot)
//                 {
//                     *ArrayLog << "       find earlier user: " << *tmp_user_I << "  @ timeslot:"
//                     << tmp_tmp_slot << "\n"; earliest_time_slot = tmp_tmp_slot;
//                 }
//             }
//         }
//     }
//     return earliest_time_slot;
// }

// // if the loop is pipelined, the reused DSP-related operators might have conflicts when sharing
// DSPs.
// // therefore, we need to re-check the resource cost  FOR INTEGER OPERATION
// HI_ArraySensitiveToLoopLevel::resourceBase
// HI_ArraySensitiveToLoopLevel::costRescheduleIntDSPOperators_forLoop(Loop *curLoop,
// std::map<BasicBlock*, timingBase> &tmp_BlockCriticalPath_inLoop, int II)
// {
//     resourceBase res(0,0,0,clock_period);
//     for (auto tmp_block : curLoop->getBlocks())
//     {
//         std::vector<DSPReuseScheduleUnit> processed;
//         for (auto tmp_schUnit0 : Block2IntDSPReuseScheduleUnits[tmp_block])
//         {
//             bool found = false;
//             for (auto it : processed)
//                 if (it == tmp_schUnit0)
//                 {
//                     found = true;
//                 }
//             if (found)
//                 continue;

//             if (DEBUG) *Evaluating_log << "checking schedule unit : " << tmp_schUnit0 << " at
//             following time points: ";

//             processed.push_back(tmp_schUnit0);

//             std::vector<int> timepoints; timepoints.clear();
//             std::set<int> conflictPoints; conflictPoints.clear();

//             for (auto tmp_schUnit1 : Block2IntDSPReuseScheduleUnits[tmp_block])
//             {
//                 if (tmp_schUnit0 == tmp_schUnit1)
//                 {
//                     if (DEBUG) *Evaluating_log << tmp_schUnit1.timeslot_inBlock << ", ";
//                     timepoints.push_back(tmp_schUnit1.timeslot_inBlock);
//                     conflictPoints.insert(tmp_schUnit1.timeslot_inBlock);
//                 }
//             }
//             if (DEBUG) *Evaluating_log << "\n";

//             int max_originalConflictCnt = 0;
//             int max_newConflictCnt = 0;
//             for (auto conflictPoint : conflictPoints)
//             {
//                 int tmp_originalConflictCnt = 0;
//                 int tmp_newConflictCnt = 0;
//                 for (auto timepoint : timepoints)
//                 {
//                     if (timepoint == conflictPoint) tmp_originalConflictCnt++;
//                     if (conflictPoint >= timepoint && ((conflictPoint - timepoint)%II==0))
//                     {
//                         tmp_newConflictCnt++;
//                     }

//                 }
//                 if (tmp_originalConflictCnt > max_originalConflictCnt)
//                 {
//                     max_originalConflictCnt = tmp_originalConflictCnt;
//                 }
//                 if (tmp_newConflictCnt > max_newConflictCnt)
//                 {
//                     max_newConflictCnt = tmp_newConflictCnt;
//                 }
//             }
//             if (DEBUG) *Evaluating_log << "max_originalConflictCnt=" << max_originalConflictCnt
//             << "  max_newConflictCnt=" << max_newConflictCnt << "\n"; if (DEBUG) *Evaluating_log
//             << "increased resource=" <<
//             getInstructionResource(tmp_schUnit0.opI)*(max_newConflictCnt -
//             max_originalConflictCnt) << "\n"; res = res +
//             getInstructionResource(tmp_schUnit0.opI)*(max_newConflictCnt -
//             max_originalConflictCnt);
//         }
//     }

//     return res;
// }

// if the loop is pipelined, the reused DSP-related operators might have conflicts when sharing
// DSPs. therefore, we need to re-check the resource cost  FOR FLOATING POINT OPERATOR
void HI_ArraySensitiveToLoopLevel::recordCostRescheduleFPDSPOperators_forLoop(
    Loop *curLoop, std::map<BasicBlock *, timingBase> &tmp_BlockCriticalPath_inLoop, int II)
{
    resourceBase res(0, 0, 0, clock_period);

    std::vector<std::string> checkopcodes = {"fmul", "fadd", "fdiv", "fsub",
                                             "dmul", "dadd", "ddiv", "dsub"};

    // Block2FPDSPReuseScheduleUnits[I->getParent()][opcode].push_back(schUnit);
    for (auto cur_opcode : checkopcodes)
    {
        int op_totalcnt = 0;

        for (auto tmp_block : curLoop->getBlocks())
        {

            if (Block2FPDSPReuseScheduleUnits.find(tmp_block) ==
                Block2FPDSPReuseScheduleUnits.end())
                continue;
            if (Block2FPDSPReuseScheduleUnits[tmp_block].find(cur_opcode) ==
                Block2FPDSPReuseScheduleUnits[tmp_block].end())
                continue;

            op_totalcnt = op_totalcnt + Block2FPDSPReuseScheduleUnits[tmp_block][cur_opcode].size();
        }

        int reuse_DSPModule = 1;
        if (op_totalcnt == 0)
            reuse_DSPModule = 0;
        if (op_totalcnt > II)
            reuse_DSPModule = op_totalcnt / II;

        if (DEBUG)
            *Evaluating_log << "  for block: the amount of floating point operators (refI):"
                            << cur_opcode << " is " << reuse_DSPModule << " each cost =["
                            << checkFPOperatorCost(cur_opcode) << "]\n";

        // color the blocks in the loop with the same number of reuse module
        for (auto tmp_block : curLoop->getBlocks())
        {
            Block2FPDSPOpCnt[tmp_block][cur_opcode] = reuse_DSPModule;
        }
    }
}
