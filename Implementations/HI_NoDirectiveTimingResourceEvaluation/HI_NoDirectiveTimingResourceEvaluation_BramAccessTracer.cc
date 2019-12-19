#include "HI_NoDirectiveTimingResourceEvaluation.h"
#include "HI_print.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Pass.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/raw_ostream.h"

#include <ios>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
using namespace llvm;

// find the array declaration in the function F and trace the accesses to them
void HI_NoDirectiveTimingResourceEvaluation::findMemoryDeclarationin(Function *F,
                                                                     bool isTopFunction)
{
    *BRAM_log << "checking the BRAM information in Function: " << F->getName() << "\n";
    ValueVisited.clear();
    scheduledAccess_timing.clear();

    // for top function in HLS, arrays in interface may involve BRAM
    if (isTopFunction)
    {
        *BRAM_log << " is Top function "
                  << "\n";
        for (auto it = F->arg_begin(), ie = F->arg_end(); it != ie; ++it)
        {
            // llvm::errs() << *it << "\n";
            if (it->getType()->isPointerTy())
            {
                PointerType *tmp_PtrType = dyn_cast<PointerType>(it->getType());
                // llvm::errs() << *(tmp_PtrType->getElementType()) << "\n";
                if (tmp_PtrType->getElementType()->isArrayTy())
                {
                    TraceAccessForTarget(it, it);
                }
                else if (tmp_PtrType->getElementType()->isIntegerTy() ||
                         tmp_PtrType->getElementType()->isFloatingPointTy() ||
                         tmp_PtrType->getElementType()->isDoubleTy())
                {
                    TraceAccessForTarget(it, it);
                }
            }
        }
    }

    // for general function in HLS, arrays in functions are usually declared with alloca instruction
    for (auto &B : *F)
    {
        for (auto &I : B)
        {
            if (AllocaInst *allocI = dyn_cast<AllocaInst>(&I))
            {
                TraceAccessForTarget(allocI, allocI);
            }
        }
    }
    for (auto &B : *F)
    {
        for (auto &I : B)
        {
            if (I.getOpcode() == Instruction::Load || I.getOpcode() == Instruction::Store)
            {
                Block2AccessList[I.getParent()].push_back(&I);
            }
        }
    }
    *BRAM_log << "\n\n\n---------------- Access to Target ---------------"
              << "\n";
    for (auto it : Access2TargetMap)
    {
        *BRAM_log << "The instruction [" << *it.first << "] will access :";
        for (auto tmpI : it.second)
        {
            *BRAM_log << " (" << tmpI->getName() << ") ";
        }
        *BRAM_log << "\n";
    }
    *BRAM_log << "-------------------------------------------------"
              << "\n\n\n\n";
    BRAM_log->flush();
}

// find out which instrctuins are related to the array, going through PtrToInt, Add, IntToPtr,
// Store, Load instructions
void HI_NoDirectiveTimingResourceEvaluation::TraceAccessForTarget(Value *cur_node, Value *ori_node)
{
    *BRAM_log << "\n\n\nTracing the access to Array " << ori_node->getName()
              << " and looking for the users of " << *cur_node << "\n";
    if (Instruction *tmpI = dyn_cast<Instruction>(cur_node))
    {
        *BRAM_log << "    --- is an instruction:\n";
    }
    else
    {
        *BRAM_log << "    --- is not an instruction \n";
    }

    BRAM_log->flush();
    Function *cur_F;

    // we are doing DFS now
    if (ValueVisited.find(cur_node) != ValueVisited.end())
        return;
    ValueVisited.insert(cur_node);

    // if (Argument *arg = dyn_cast<Argument>(cur_node))
    // {
    //     cur_F = arg->getParent();
    // }
    // else
    // {
    //     if (Instruction *InstTmp = dyn_cast<Instruction>(cur_node))
    //     {
    //         cur_F = InstTmp->getParent()->getParent();
    //     }
    //     else
    //     {
    //         assert(false && "The parent function should be found.\n");
    //     }
    // }

    // Trace the uses of the pointer value or integer generaed by PtrToInt
    for (auto it = cur_node->use_begin(), ie = cur_node->use_end(); it != ie; ++it)
    {
        *BRAM_log << "    find user of " << ori_node->getName() << " --> " << *it->getUser()
                  << "\n";

        // Load and Store Instructions are leaf nodes in the DFS
        if (LoadInst *LoadI = dyn_cast<LoadInst>(it->getUser()))
        {
            if (Access2TargetMap.find(LoadI) == Access2TargetMap.end())
            {
                *BRAM_log << "    is an LOAD instruction: " << *LoadI << "\n";
                std::vector<Value *> tmp_vec;
                tmp_vec.push_back(ori_node);
                Access2TargetMap.insert(
                    std::pair<Instruction *, std::vector<Value *>>(LoadI, tmp_vec));
            }
            else
            {
                Access2TargetMap[LoadI].push_back(ori_node);
            }
        }
        else if (StoreInst *StoreI = dyn_cast<StoreInst>(it->getUser()))
        {
            if (Access2TargetMap.find(StoreI) == Access2TargetMap.end())
            {
                *BRAM_log << "    is an STORE instruction: " << *StoreI << "\n";
                std::vector<Value *> tmp_vec;
                tmp_vec.push_back(ori_node);
                Access2TargetMap.insert(
                    std::pair<Instruction *, std::vector<Value *>>(StoreI, tmp_vec));
            }
            else
            {
                Access2TargetMap[StoreI].push_back(ori_node);
            }
        }
        // if a pointer of arrray is passed as sub-function's argument, handle it
        else if (CallInst *CallI = dyn_cast<CallInst>(it->getUser()))
        {
            *BRAM_log << "    is an CALL instruction: " << *CallI << "\n";
            for (int i = 0; i < CallI->getNumArgOperands(); ++i)
            {
                if (CallI->getArgOperand(i) ==
                    cur_node) // find which argument is exactly the pointer we are tracing
                {
                    auto arg_it = CallI->getCalledFunction()->arg_begin();
                    auto arg_ie = CallI->getCalledFunction()->arg_end();
                    for (int j = 0;; ++j, ++arg_it)
                        if (i == j)
                        {
                            // go into the sub-function to trace the accesses to the target
                            Alias2Target[arg_it] = ori_node;
                            TraceAccessForTarget(arg_it, ori_node);
                            break;
                        }
                }
            }
        }
        else
        {
            *BRAM_log << "    is an general instruction: " << *it->getUser() << "\n";
            TraceAccessForTarget(it->getUser(), ori_node);
        }
    }
    ValueVisited.erase(cur_node);
}

// schedule the access to potential target (since an instructon may use the address for different
// target (e.g. address comes from PHINode), we need to schedule all of them)
HI_NoDirectiveTimingResourceEvaluation::timingBase
HI_NoDirectiveTimingResourceEvaluation::scheduleBRAMAccess(
    Instruction *access, BasicBlock *cur_block,
    HI_NoDirectiveTimingResourceEvaluation::timingBase cur_Timing)
{
    if (Access2TargetMap.find(access) == Access2TargetMap.end())
        llvm::errs() << "BramAccessTracer:171" << *access
                     << " in Block: " << access->getParent()->getName()
                     << " of Function: " << access->getParent()->getParent()->getName() << "\n";

    assert(Access2TargetMap.find(access) != Access2TargetMap.end() &&
           "The access should be recorded in the BRAM access info.\n");
    timingBase res(0, 0, 1, clock_period);
    for (auto target : Access2TargetMap[access])
    {
        *BRAM_log << "\n\n\n sceduling access instruction: " << *access << " for the target ["
                  << target->getName() << "]"
                  << " of Block:" << cur_block->getName() << "\n";
        timingBase targetTiming = handleBRAMAccessFor(access, target, cur_block, cur_Timing);
        if (access->getOpcode() == Instruction::Store)
        {
            *BRAM_log << " sceduled access Store instruction: " << *access << " for the target ["
                      << target->getName() << "] at cycle #" << targetTiming.latency
                      << " of Block:" << cur_block->getName() << "\n";
        }
        else
        {
            *BRAM_log << " sceduled access Load instruction: " << *access << " for the target ["
                      << target->getName() << "] at cycle #" << targetTiming.latency - 1
                      << " of Block:" << cur_block->getName() << "\n";
        }

        if (targetTiming > res)
            res = targetTiming;
        BRAM_log->flush();
    }
    return res;
}

// schedule the access to specific target for the instruction
HI_NoDirectiveTimingResourceEvaluation::timingBase
HI_NoDirectiveTimingResourceEvaluation::handleBRAMAccessFor(
    Instruction *access, Value *target, BasicBlock *cur_block,
    HI_NoDirectiveTimingResourceEvaluation::timingBase cur_Timing)
{
    if (scheduledAccess_timing.find(std::pair<Instruction *, Value *>(access, target)) !=
        scheduledAccess_timing.end())
    {
        return scheduledAccess_timing[std::pair<Instruction *, Value *>(access, target)];
    }

    std::string LoadOrStore;
    if (access->getOpcode() == Instruction::Store)
        LoadOrStore = "store";
    else
        LoadOrStore = "load";

    // if the access can take place at cur_Timing, schedule and record it
    if (checkBRAMAvailabilty(access, target, LoadOrStore, cur_block, cur_Timing))
    {
        *BRAM_log << "    the access instruction: " << *access << " for the target ["
                  << target->getName() << "] can be scheduled in cycle #" << cur_Timing.latency
                  << " of Block:" << cur_block->getName() << "\n";
        *BRAM_log << "    cur_timing is " << cur_Timing << " opTiming is "
                  << getInstructionLatency(access) << " ";
        scheduledAccess_timing[std::pair<Instruction *, Value *>(access, target)] =
            cur_Timing + getInstructionLatency(access);
        *BRAM_log << "resultTiming is "
                  << scheduledAccess_timing[std::pair<Instruction *, Value *>(access, target)]
                  << "\n";
        insertBRAMAccessInfo(target, cur_block, cur_Timing.latency, access);
        return scheduledAccess_timing[std::pair<Instruction *, Value *>(access, target)];
    }
    else
    {
        // otherwise, try later time slots and see whether the schedule can be successful.
        while (1)
        {
            *BRAM_log << "    the access instruction: " << *access << " for the target ["
                      << target->getName() << "] CANNOT be scheduled in cycle #"
                      << cur_Timing.latency << " of Block:" << cur_block->getName() << "\n";
            cur_Timing.latency++;
            cur_Timing.timing = 0;
            if (checkBRAMAvailabilty(access, target, LoadOrStore, cur_block, cur_Timing))
            {
                *BRAM_log << "    the access instruction: " << *access << " for the target ["
                          << target->getName() << "] can be scheduled in cycle #"
                          << cur_Timing.latency << " of Block:" << cur_block->getName() << "\n";
                scheduledAccess_timing[std::pair<Instruction *, Value *>(access, target)] =
                    cur_Timing + getInstructionLatency(access);
                insertBRAMAccessInfo(target, cur_block, cur_Timing.latency, access);
                return scheduledAccess_timing[std::pair<Instruction *, Value *>(access, target)];
            }
            BRAM_log->flush();
        }
    }
    BRAM_log->flush();
}

// check whether the access to target array can be scheduled in a specific cycle
bool HI_NoDirectiveTimingResourceEvaluation::checkBRAMAvailabilty(
    Instruction *access, Value *target, std::string StoreOrLoad, BasicBlock *cur_block,
    HI_NoDirectiveTimingResourceEvaluation::timingBase cur_Timing)
{
    *BRAM_log << "       checking the access to the target [" << target->getName() << "] in cycle #"
              << cur_Timing.latency << " of Block:" << cur_block->getName() << "  --> ";
    timingBase opTiming = getInstructionLatency(
        access); // get_inst_TimingInfo_result(StoreOrLoad.c_str(),-1,-1,clock_period_str);
    timingBase res = cur_Timing + opTiming;

    if (StoreOrLoad == "load" && (res.latency - 1 != cur_Timing.latency))
    {
        *BRAM_log << " load timing is not fit in.\n";
        return false;
    }

    if (StoreOrLoad == "store" && (res.latency != cur_Timing.latency))
    {
        *BRAM_log << " store timing is not fit in.\n";
        return false;
    }

    if (target2LastAccessCycleInBlock.find(target) == target2LastAccessCycleInBlock.end())
    {
        *BRAM_log << " No access scheduled in the block yet\n";
        return true;
    }
    if (target2LastAccessCycleInBlock[target].find(cur_block) ==
        target2LastAccessCycleInBlock[target].end())
    {
        *BRAM_log << " No access for the target in the block yet\n";
        return true;
    }

    if (access->getOpcode() == Instruction::Load)
    {
        if (hasRAWHazard(access, cur_Timing.latency))
            return false;
    }

    int cnt = 0;
    for (auto lat_Inst_pair : target2LastAccessCycleInBlock[target][cur_block])
    {
        if (lat_Inst_pair.first == cur_Timing.latency)
            cnt++;
    }
    *BRAM_log << cnt << " access(es) in this cycle\n";
    // consider the number of BRAM port (currently it is a constant (2). Later, consider array
    // partitioning)
    if (cnt < 2)
        return true;
    return false;
}

// record the schedule information
void HI_NoDirectiveTimingResourceEvaluation::insertBRAMAccessInfo(Value *target,
                                                                  BasicBlock *cur_block,
                                                                  int cur_latency,
                                                                  Instruction *access)
{
    *BRAM_log << "       inserting the access to the target [" << target->getName()
              << "] in cycle #" << cur_latency << " of Block:" << cur_block->getName() << "  --> ";
    if (target2LastAccessCycleInBlock.find(target) == target2LastAccessCycleInBlock.end())
    {
        std::map<BasicBlock *, std::vector<std::pair<int, Instruction *>>> tmp_map;
        std::vector<std::pair<int, Instruction *>> tmp_vec;
        tmp_vec.push_back(std::pair<int, Instruction *>(cur_latency, access));
        tmp_map[cur_block] = tmp_vec;
        target2LastAccessCycleInBlock[target] = tmp_map;
        *BRAM_log << "(new map new vector)\n";
        return;
    }
    if (target2LastAccessCycleInBlock[target].find(cur_block) ==
        target2LastAccessCycleInBlock[target].end())
    {
        std::vector<std::pair<int, Instruction *>> tmp_vec;
        tmp_vec.push_back(std::pair<int, Instruction *>(cur_latency, access));
        target2LastAccessCycleInBlock[target][cur_block] = tmp_vec;
        *BRAM_log << "(new vector)\n";
        return;
    }
    *BRAM_log << "(existed vector)\n";
    target2LastAccessCycleInBlock[target][cur_block].push_back(
        std::pair<int, Instruction *>(cur_latency, access));
}

// evaluate the number of LUT needed by the BRAM MUXs
HI_NoDirectiveTimingResourceEvaluation::resourceBase
HI_NoDirectiveTimingResourceEvaluation::BRAM_MUX_Evaluate()
{
    resourceBase res(0, 0, 0, clock_period);
    int inputSize = 0;

    for (auto Val_IT : target2LastAccessCycleInBlock)
    {
        int access_counter_for_value = 0;
        int read_counter_for_value = 0;
        int write_counter_for_value = 0;
        *Evaluating_log << " The access to target: [" << Val_IT.first->getName() << "] includes:\n";
        for (auto B2Cycles : Val_IT.second)
        {
            access_counter_for_value += B2Cycles.second.size();
            *Evaluating_log << " in block: [" << B2Cycles.first->getName() << "] cycles: ";
            for (auto C_tmp : B2Cycles.second)
            {
                if (auto readI = dyn_cast<LoadInst>(C_tmp.second))
                {
                    read_counter_for_value++;
                    *Evaluating_log << " --- R" << C_tmp.first << " ";
                }
                if (auto writeI = dyn_cast<StoreInst>(C_tmp.second))
                {
                    write_counter_for_value++;
                    *Evaluating_log << " --- W" << C_tmp.first << " ";
                }
            }
            *Evaluating_log << " \n";
            Evaluating_log->flush();
        }
        *Evaluating_log << " \n\n";

        // This analysis is based on observation
        if (access_counter_for_value <= 2)
        {
            if (read_counter_for_value == 1)
            {
                inputSize += 3;
            }
        }
        else if (access_counter_for_value <= 4)
        {
            if (access_counter_for_value == 3)
            {
                if (read_counter_for_value == 3)
                {
                    inputSize += 3;
                }
                else if (write_counter_for_value == 3)
                {
                    inputSize += 4;
                }
                else if (read_counter_for_value == 2)
                {
                    inputSize += 4;
                }
                else if (write_counter_for_value == 2)
                {
                    inputSize += 4;
                }
                else
                {
                    inputSize += 3;
                }
            }
            else
            {
                inputSize += 6;
            }
        }
        else
        {
            inputSize += (access_counter_for_value + 2);
        }
    }

    res.LUT = inputSize * 5;
    return res;
}

// get the number of BRAMs which are needed by the alloca instruction
HI_NoDirectiveTimingResourceEvaluation::resourceBase
HI_NoDirectiveTimingResourceEvaluation::get_BRAM_Num_For(AllocaInst *alloca_I)
{
    resourceBase res(0, 0, 0, 0, clock_period);
    *BRAM_log << "\n\nchecking allocation instruction [" << *alloca_I
              << "] and its type is: " << *alloca_I->getType() << " and its ElementType is: ["
              << *alloca_I->getType()->getElementType() << "]\n";
    Type *tmp_type = alloca_I->getType()->getElementType();
    int total_ele = 1;
    while (auto array_T = dyn_cast<ArrayType>(tmp_type))
    {
        *BRAM_log << "----- element type of : " << *tmp_type << " is "
                  << *(array_T->getElementType()) << " and the number of its elements is "
                  << (array_T->getNumElements()) << "\n";
        total_ele *= (array_T->getNumElements());
        tmp_type = array_T->getElementType();
    }
    int BW = 0;
    if (tmp_type->isIntegerTy())
        BW = tmp_type->getIntegerBitWidth();
    else if (tmp_type->isFloatTy())
        BW = 32;
    else if (tmp_type->isDoubleTy())
        BW = 64;
    assert(BW != 0 && "we should get BW for the basic element type.\n");
    res = get_BRAM_Num_For(BW, total_ele);
    *BRAM_log << "checked allocation instruction [" << *alloca_I
              << "] and its basic elemenet type is: [" << *tmp_type << "] with BW=[" << BW
              << "] and the total number of basic elements is: [" << total_ele
              << "] and it need BRAMs [" << res.BRAM << "].\n\n";

    return res;
}

// get the number of BRAMs which are needed by the alloca instruction
HI_NoDirectiveTimingResourceEvaluation::resourceBase
HI_NoDirectiveTimingResourceEvaluation::get_BRAM_Num_For(int width, int depth)
{
    resourceBase res(0, 0, 0, 0, clock_period);
    if (depth <= 0)
        return res;

    // when an array is large, we need to partition it into multiple BRAM
    // therefore, we need to set the depth and bidwith for each unit BRAM
    int width_uint = 1;
    int depth_uint = 1;
    if (width <= 1 || depth > 16 * 1024)
    {
        width_uint = 1;
        depth_uint = 16 * 1024;
    }
    else
    {
        if (width <= 2 || depth > 8 * 1024)
        {
            width_uint = 2;
            depth_uint = 8 * 1024;
        }
        else
        {
            if (width <= 4 || depth > 4 * 1024)
            {
                width_uint = 4;
                depth_uint = 4 * 1024;
            }
            else
            {
                if (width <= 9 || depth > 2 * 1024)
                {
                    width_uint = 9;
                    depth_uint = 2 * 1024;
                }
                else
                {
                    if (width <= 18 || depth > 1 * 1024)
                    {
                        width_uint = 18;
                        depth_uint = 1 * 1024;
                    }
                    else
                    {
                        width_uint = 36;
                        depth_uint = 512;
                    }
                }
            }
        }
    }

    int width_factor = width / width_uint;
    if ((width % width_uint) > 0)
    {
        width_factor = width_factor + 1;
    }

    int depth_factor = depth / depth_uint;
    if ((depth % depth_uint) > 0)
    {
        depth_factor = depth_factor + 1;
    }
    res.BRAM = width_factor * depth_factor;
    return res;
}

AliasResult HI_NoDirectiveTimingResourceEvaluation::HI_AAResult::alias(const MemoryLocation &LocA,
                                                                       const MemoryLocation &LocB)
{
    auto PtrA = LocA.Ptr;
    auto PtrB = LocB.Ptr;

    if (PtrA != PtrA)
    {
        return NoAlias;
    }

    // Forward the query to the next analysis.
    return AAResultBase::alias(LocA, LocB);
}

bool HI_NoDirectiveTimingResourceEvaluation::hasRAWHazard(Instruction *loadI, int cycle)
{
    assert(loadI->getOpcode() == Instruction::Load);
    bool checkEnableFlag = false;

    for (auto preI : Block2AccessList[loadI->getParent()])
    {
        if (preI == loadI)
        {
            break;
        }
        if (getTargetFromInst(loadI) == getTargetFromInst(preI))
        {
            if (preI->getOpcode() == Instruction::Store)
            {
                // Redundant load with alias access should have been removed
                // therefore, this could be potential conflict, reject the access
                if (Inst_Schedule[preI].second >= cycle)
                {
                    *BRAM_log << "\nload instruction: " << *loadI
                              << " RAW hazard with store instruction: " << *preI << " at cycle#"
                              << Inst_Schedule[preI].second << "\n";
                    return true;
                }
            }
        }
    }
    return false;
}

Value *HI_NoDirectiveTimingResourceEvaluation::getTargetFromInst(Instruction *accessI)
{
    // assert(Access2TargetMap[accessI].size()==1 && "currently, we do not support
    // 1-access-multi-target.");
    if (Access2TargetMap[accessI].size() > 1)
    {
        Value *reftarget = Access2TargetMap[accessI][0];
        for (auto target : Access2TargetMap[accessI])
        {

            Value *tmp_target = target, *tmp_reftarget = reftarget;
            if (Alias2Target.find(target) != Alias2Target.end())
                tmp_target = Alias2Target[target];
            if (Alias2Target.find(reftarget) != Alias2Target.end())
                tmp_reftarget = Alias2Target[reftarget];
            if (tmp_target != tmp_reftarget)
            {
                llvm::errs() << *accessI << " has multi-targets: \n";
                llvm::errs() << *tmp_target << "  is different form " << *tmp_reftarget << "\n";
                for (auto target : Access2TargetMap[accessI])
                    llvm::errs() << "    " << *target << "\n";
            }
            assert(tmp_target == tmp_reftarget &&
                   "currently, we do not support 1-access-multi-target.");
        }
    }
    Value *target = Access2TargetMap[accessI][0];
    if (Alias2Target.find(target) == Alias2Target.end())
        return target;
    else
        return Alias2Target[target];
}