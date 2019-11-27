#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Pass.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/raw_ostream.h"
#include "HI_print.h"
#include "HI_WithDirectiveTimingResourceEvaluation.h"

#include <stdio.h>
#include <string>
#include <ios>
#include <stdlib.h>
#include <sstream>
using namespace llvm;

// find the array declaration in the function F and trace the accesses to them
void HI_WithDirectiveTimingResourceEvaluation::findMemoryDeclarationAndAnalyzeAccessin(Function *F, bool isTopFunction)
{
    if (DEBUG) *BRAM_log << "checking the BRAM information in Function: " << F->getName() << "\n";
    if (DEBUG) *ArrayLog << "\n\nchecking the BRAM information in Function: " << F->getName() << "\n";
    ValueVisited.clear();
    scheduledAccess_timing.clear();

    // for top function in HLS, arrays in interface may involve BRAM
    if (isTopFunction)
    {
        if (DEBUG) *BRAM_log << " is Top function " << "\n";
        if (DEBUG) *ArrayLog << " is Top function " << "\n";
        for (auto it = F->arg_begin(),ie = F->arg_end(); it!=ie; ++it)
        {
            if (it->getType()->isPointerTy())
            {
                PointerType *tmp_PtrType = dyn_cast<PointerType>(it->getType());
                if (tmp_PtrType->getElementType()->isArrayTy())
                {
                    if (DEBUG) *ArrayLog << "  get array information of [" << it->getName() << "] from argument and its address=" << it << "\n";
                    Target2ArrayInfo[it]=getArrayInfo(it);
                    TraceAccessForTarget(it,it);   
                    Value2Target[it].insert(it);      
                    if (DEBUG) *ArrayLog << Target2ArrayInfo[it] << "\n";
                }
                else if (tmp_PtrType->getElementType()->isIntegerTy() || tmp_PtrType->getElementType()->isFloatingPointTy() ||tmp_PtrType->getElementType()->isDoubleTy() )
                {
                    if (DEBUG) *ArrayLog << "  get array information of [" << it->getName() << "] from argument and its address=" << it << "\n";
                    Target2ArrayInfo[it]=getArrayInfo(it);
                    TraceAccessForTarget(it,it);
                    Value2Target[it].insert(it);  
                    if (DEBUG) *ArrayLog << Target2ArrayInfo[it] << "\n";
                }
            }
        }
    }
    else
    {
        if (DEBUG) *BRAM_log << " is not top function " << "\n";
        if (DEBUG) *ArrayLog << " is not top function " << "\n";
    }
    
    // for general function in HLS, arrays in functions are usually declared with alloca instruction
    for (auto &B: *F)
    {
        for (auto &I: B)
        {
            if (AllocaInst *allocI = dyn_cast<AllocaInst>(&I))
            {
                if (DEBUG) *ArrayLog << "  get array information of [" << *allocI << "] from allocaInst and its address=" << allocI << "\n";
                Target2ArrayInfo[allocI]=getArrayInfo(allocI);
                TraceAccessForTarget(allocI,allocI);
                Value2Target[allocI].insert(allocI);
                if (DEBUG) *ArrayLog << Target2ArrayInfo[allocI] << "\n";
            }
        }
    }
    if (DEBUG) *BRAM_log << "\n\n\n---------------- Access to Target ---------------" << "\n";
    for (auto it : Access2TargetMap)
    {
        if (DEBUG) *BRAM_log << "The instruction [" << *it.first << "] will access :";
        for (auto tmpI : it.second)
        {
            if (DEBUG) *BRAM_log << " (" << tmpI->getName() << ") ";
        }
        if (DEBUG) *BRAM_log << "\n";
    }
    if (DEBUG) *BRAM_log << "-------------------------------------------------" << "\n\n\n\n";
    // BRAM_log->flush();
    // ArrayLog->flush();

}

// find out which instrctuins are related to the array, going through PtrToInt, Add, IntToPtr, Store, Load instructions
// record the corresponding target array which the access instructions try to touch
void HI_WithDirectiveTimingResourceEvaluation::TraceAccessForTarget(Value *cur_node, Value *ori_node)
{
    if (DEBUG) *BRAM_log << "\n\n\nTracing the access to Array " << ori_node->getName() << " and looking for the users of " << *cur_node<< "\n";
    
    if (Instruction* tmpI = dyn_cast<Instruction>(cur_node))
    {
        if (DEBUG) *BRAM_log << "    --- is an instruction:\n";
    }
    else
    {
        if (DEBUG) *BRAM_log << "    --- is not an instruction \n";
    }

    // BRAM_log->flush();
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
    for (auto it = cur_node->use_begin(),ie = cur_node->use_end(); it != ie; ++it)
    {
        if (DEBUG) *BRAM_log << "    find user of " << ori_node->getName() << " --> " << *it->getUser() <<  "\n";
        Value2Target[it->getUser()].insert(ori_node);
        
        // Load and Store Instructions are leaf nodes in the DFS
        if (LoadInst *LoadI = dyn_cast<LoadInst>(it->getUser()))
        {
            if (Access2TargetMap.find(LoadI)==Access2TargetMap.end())
            {                
                if (DEBUG) *BRAM_log << "    is an LOAD instruction: " << *LoadI << " will access [" << *ori_node << "]\n";
                std::vector<Value*> tmp_vec; 
                tmp_vec.push_back(ori_node);
                Access2TargetMap.insert(std::pair<Instruction*,std::vector<Value*>>(LoadI,tmp_vec));
            }
            else
            {
                Access2TargetMap[LoadI].push_back(ori_node);
            }
            
        }
        else if (StoreInst *StoreI = dyn_cast<StoreInst>(it->getUser()))
        {
            if (Access2TargetMap.find(StoreI)==Access2TargetMap.end())
            {
                if (DEBUG) *BRAM_log << "    is an STORE instruction: " << *StoreI << " will access [" << *ori_node << "]\n";
                std::vector<Value*> tmp_vec; 
                tmp_vec.push_back(ori_node);
                Access2TargetMap.insert(std::pair<Instruction*,std::vector<Value*>>(StoreI,tmp_vec));
            }
            else
            {
                Access2TargetMap[StoreI].push_back(ori_node);
            }
            
        }
        // if a pointer of arrray is passed as sub-function's argument, handle it
        else if (CallInst *CallI = dyn_cast<CallInst>(it->getUser()))
        {
            if (DEBUG) *BRAM_log << "    is an CALL instruction: " << *CallI << " will access [" << *ori_node << "]\n";
            if (Access2TargetMap.find(CallI)==Access2TargetMap.end())
            {
                std::vector<Value*> tmp_vec; 
                tmp_vec.push_back(ori_node);
                Access2TargetMap.insert(std::pair<Instruction*,std::vector<Value*>>(CallI,tmp_vec));
            }
            else
            {
                Access2TargetMap[CallI].push_back(ori_node);
            }
            for (int i = 0; i < CallI->getNumArgOperands(); ++i)
            {
                if (CallI->getArgOperand(i) == cur_node) // find which argument is exactly the pointer we are tracing
                {
                    auto arg_it = CallI->getCalledFunction()->arg_begin();
                    auto arg_ie = CallI->getCalledFunction()->arg_end();
                    for (int j = 0 ; ; ++j,++arg_it)
                        if (i==j)
                        {
                            // go into the sub-function to trace the accesses to the target
                            Value2Target[arg_it].insert(ori_node);
                            Alias2Target[arg_it] = ori_node;
                            TraceAccessForTarget(arg_it,ori_node);
                            break;
                        }                    
                }
            }
        }
        else
        {
            
            if (DEBUG) *BRAM_log << "    is an general instruction: " << *it->getUser() << "\n";
            TraceAccessForTarget(it->getUser(),ori_node);
        }        
    }
    ValueVisited.erase(cur_node);
}

// schedule the access to potential target (since an instructon may use the address for different target 
// (e.g. address comes from PHINode) or different parttions, we need to schedule all of them)
HI_WithDirectiveTimingResourceEvaluation::timingBase 
HI_WithDirectiveTimingResourceEvaluation::scheduleBRAMAccess(Instruction *access, BasicBlock *cur_block, 
                                                             HI_WithDirectiveTimingResourceEvaluation::timingBase cur_Timing,
                                                             partition_info target_partition)
{
    assert(Access2TargetMap.find(access) != Access2TargetMap.end() && "The access should be recorded in the BRAM access info.\n");
    timingBase res(0,0,1,clock_period);
    Value *reftarget = Access2TargetMap[access][0];
    if (Alias2Target.find(reftarget) != Alias2Target.end())
        reftarget = Alias2Target[reftarget];
    if (Access2TargetMap[access].size()>1)
    {
        
        for (auto target:Access2TargetMap[access])
        {

            Value *tmp_target = target, *tmp_reftarget = reftarget;
            if (Alias2Target.find(target) != Alias2Target.end())
                tmp_target = Alias2Target[target];
            if (Alias2Target.find(reftarget) != Alias2Target.end())
                tmp_reftarget = Alias2Target[reftarget];
            if (tmp_target!=tmp_reftarget)
            {
                llvm::errs()  << *access << " has multi-targets: \n";
                llvm::errs()  << *tmp_target << "  is different form " << *tmp_reftarget << "\n";
                for (auto target:Access2TargetMap[access])
                    llvm::errs()  <<  "    " <<*target << "\n";
            }
            assert(tmp_target==tmp_reftarget && "currently, we do not support 1-access-multi-target.");
        }
    }
    Value* target = reftarget;


    
    if (DEBUG) *BRAM_log << "\n\n\n sceduling access instruction: " << *access << " for the target [" << target->getName() << "]" << " of Block:" << cur_block->getName() <<"\n";

    // if the target is partitioned completely as registers, the delay of accesses might be ignored.
    // (implict: there won't be access contension to the target, since each element has its own register)
    if (Target2ArrayInfo[target].completePartition)
    {
        timingBase targetTiming = cur_Timing;  
        if (access->getOpcode()==Instruction::Store)
        {
            if (DEBUG) *BRAM_log << " sceduled access Store instruction: " << *access << 
                        " for the target ["  << target->getName() << 
                        "] in its partition #" << target_partition << 
                        " at cycle #" << targetTiming.latency << 
                        " of Block:" << cur_block->getName() << "\n";
        }
        else
        {
            if (DEBUG)*BRAM_log << " sceduled access Load instruction: " << *access << 
                        " for the target [" << target->getName() << 
                        "] in its partition #" << target_partition << 
                        " at cycle #" << targetTiming.latency << 
                        " of Block:" << cur_block->getName() << "\n";
        }
        
        if (targetTiming > res)
            res = targetTiming;
    }
    else
    {
        timingBase targetTiming = handleBRAMAccessFor(access, target, cur_block, cur_Timing, target_partition);        
        if (access->getOpcode()==Instruction::Store)
        {
            if (DEBUG) *BRAM_log << " sceduled access Store instruction: " << *access << 
                        " for the target ["  << target->getName() << 
                        "] in its partition #" << target_partition << 
                        " at cycle #" << targetTiming.latency << 
                        " of Block:" << cur_block->getName() << "\n";
        }
        else
        {
            if (DEBUG)*BRAM_log << " sceduled access Load instruction: " << *access << 
                        " for the target [" << target->getName() << 
                        "] in its partition #" << target_partition << 
                        " at cycle #" << targetTiming.latency-1 << 
                        " of Block:" << cur_block->getName() << "\n";
        }
        
        if (targetTiming > res)
            res = targetTiming;
    }

    if (DEBUG) BRAM_log->flush();

    return res;
}

// schedule the access to specific target for the instruction
HI_WithDirectiveTimingResourceEvaluation::timingBase 
HI_WithDirectiveTimingResourceEvaluation::handleBRAMAccessFor(Instruction *access, Value *target, 
                                                              BasicBlock *cur_block, 
                                                              HI_WithDirectiveTimingResourceEvaluation::timingBase cur_Timing,
                                                              partition_info target_partition)
{
    if (scheduledAccess_timing.find(std::pair<Instruction*,std::pair<Value*, partition_info>>(access, std::pair<Value*, partition_info>(target, target_partition))) != scheduledAccess_timing.end())
    {
        return scheduledAccess_timing[std::pair<Instruction*,std::pair<Value*, partition_info>>(access, std::pair<Value*, partition_info>(target, target_partition))]; 
    }

    std::string LoadOrStore;
    if (access->getOpcode() == Instruction::Store)
        LoadOrStore = "store";
    else    
        LoadOrStore = "load";   

    // if the access can take place at cur_Timing, schedule and record it
    if (checkBRAMAvailabilty(access, target, LoadOrStore, cur_block, cur_Timing, target_partition))
    {        
        if (DEBUG) *BRAM_log << "    the access instruction: " << *access << " for the target [" << target->getName() << "] can be scheduled in cycle #" <<cur_Timing.latency << " of Block:" << cur_block->getName() <<"\n";
        if (DEBUG) *BRAM_log << "    cur_timing is " << cur_Timing << " opTiming is " << getInstructionLatency(access) << " ";
        auto tripe = std::pair<Instruction*,std::pair<Value*, partition_info>>(access, std::pair<Value*, partition_info>(target, target_partition));
        scheduledAccess_timing[tripe] = cur_Timing + getInstructionLatency(access);
        if (DEBUG) *BRAM_log << "resultTiming is "<< scheduledAccess_timing[tripe] << " ";
        if (DEBUG) *BRAM_log << "Add_resultTiming is "<< cur_Timing + getInstructionLatency(access) << "\n";
        insertBRAMAccessInfo(target, cur_block, cur_Timing.latency, access, target_partition);
        if (DEBUG) BRAM_log->flush();
        assert(scheduledAccess_timing[tripe].clock_period == clock_period);
        return scheduledAccess_timing[tripe];
    }
    else
    {
        // otherwise, try later time slots and see whether the schedule can be successful.
        while (1)
        {
            if (DEBUG) *BRAM_log << "    the access instruction: " << *access << " for the target [" << target->getName() << "] CANNOT be scheduled in cycle #" <<cur_Timing.latency << " of Block:" << cur_block->getName() <<"\n";
            cur_Timing.latency++;
            cur_Timing.timing=0;
            if (checkBRAMAvailabilty(access, target, LoadOrStore, cur_block, cur_Timing, target_partition))
            {
                if (DEBUG) *BRAM_log << "    the access instruction: " << *access << " for the target [" << target->getName() << "] can be scheduled in cycle #" <<cur_Timing.latency << " of Block:" << cur_block->getName() <<"\n";
                auto tripe = std::pair<Instruction*,std::pair<Value*, partition_info>>(access, std::pair<Value*, partition_info>(target, target_partition));
                scheduledAccess_timing[tripe] = cur_Timing + getInstructionLatency(access);
                if (DEBUG) *BRAM_log << "resultTiming is "<< scheduledAccess_timing[tripe] << " ";
                if (DEBUG) *BRAM_log << "Add_resultTiming is "<< cur_Timing + getInstructionLatency(access) << "\n";
                insertBRAMAccessInfo(target, cur_block, cur_Timing.latency, access, target_partition);
                if (DEBUG) BRAM_log->flush();
                assert(scheduledAccess_timing[tripe].clock_period == clock_period);
                return scheduledAccess_timing[tripe];
            }
            // BRAM_log->flush();
        }
    }    
    // BRAM_log->flush();
}


// check whether the access to target array can be scheduled in a specific cycle
bool HI_WithDirectiveTimingResourceEvaluation::checkBRAMAvailabilty(Instruction* access, Value *target, std::string StoreOrLoad, 
                                                                    BasicBlock *cur_block, 
                                                                    HI_WithDirectiveTimingResourceEvaluation::timingBase cur_Timing,
                                                                    partition_info target_partition)
{
    if (DEBUG) *BRAM_log << "       checking the access to the target [" << target->getName() << "] in cycle #" << cur_Timing.latency << " of Block:" << cur_block->getName() <<"  --> ";
    timingBase opTiming =  getInstructionLatency(access); // get_inst_TimingInfo_result(StoreOrLoad.c_str(),-1,-1,clock_period_str);
    timingBase res = cur_Timing + opTiming;

    if (StoreOrLoad == "load" && (res.latency-1 != cur_Timing.latency))
    {
        if (DEBUG) *BRAM_log << " load timing is not fit in.\n";
        return false;
    }

    if (StoreOrLoad == "store" && (res.latency != cur_Timing.latency))
    {
        if (DEBUG) *BRAM_log << " store timing is not fit in.\n";
        return false;
    }

    if (target2LastAccessCycleInBlock.find(target) == target2LastAccessCycleInBlock.end())
    {
        if (DEBUG) *BRAM_log << " No access scheduled in the block yet\n";
        return true;
    }
    if (target2LastAccessCycleInBlock[target].find(cur_block) == target2LastAccessCycleInBlock[target].end())
    {
        if (DEBUG) *BRAM_log << " No access for the target in the block yet\n";
        return true;
    }
    
    if (access->getOpcode() == Instruction::Load)
    {
        if (hasRAWHazard(access, cur_Timing.latency))
            return false;
    }
    // for (auto aliasI : accessAliasMap[access])
    // {

    //     if (Inst_Schedule[aliasI].second)
    // }

    int cnt = 0;
    HI_AccessInfo curAccessInfo = getAccessInfoForAccessInst(access);

    // check whether there is congestion when trying to access the BRAM
    // by checking the list of scheduled accesses in the basic block
    for (auto lat_Inst_pair : target2LastAccessCycleInBlock[target][cur_block])
    {
        Instruction *preI = lat_Inst_pair.second;
        HI_AccessInfo preAccessInfo = getAccessInfoForAccessInst(preI);
        if (lat_Inst_pair.first.first == cur_Timing.latency && (lat_Inst_pair.first.second == target_partition /* ||  preAccessInfo.unpredictable || curAccessInfo.unpredictable */))
            cnt ++;
    }
    
    if (DEBUG) *BRAM_log << "for partition #" << target_partition << ": " << cnt << " access(es) in this cycle\n";

    // consider the number of BRAM port 
    if (cnt>= target_partition.port_num)
        return false;
    

    return true;
}

// record the schedule information
void HI_WithDirectiveTimingResourceEvaluation::insertBRAMAccessInfo(Value *target, BasicBlock *cur_block, int cur_latency, Instruction* access, partition_info target_partition)
{
    if (DEBUG) *BRAM_log << "       inserting the access to the target [" << target->getName() 
              << "] in its partition #" << target_partition
              << " in cycle #" << cur_latency 
              << " of Block:" << cur_block->getName() << "  --> ";

    std::pair<Value*, partition_info> tmp_pair = std::pair<Value*, partition_info>(target,target_partition);

    if (partition2cnt.find(target_partition) == partition2cnt.end())
        partition2cnt[target_partition] = std::pair<int, int>(0, 0);
    
    if (access->getOpcode() == Instruction::Load)
        partition2cnt[target_partition].first++;
    else
        partition2cnt[target_partition].second++;

    int checkCnt = 0;
    for (auto b_l_pair : targetPartition2BlockCycleAccessCnt[tmp_pair])
    {
        if (b_l_pair.first==cur_block && cur_latency ==b_l_pair.second)
        {
            checkCnt++;
            if (checkCnt>=target_partition.port_num) 
            {
                if (DEBUG) *BRAM_log << " error with block: " << cur_block->getName() << " at cycle#" << cur_latency << "\n";
                llvm::errs() << " error with block: " << cur_block->getName() << " at cycle#" << cur_latency << "\n";
                break;
            }
        }
    }
    targetPartition2BlockCycleAccessCnt[tmp_pair].push_back(std::pair<BasicBlock*, int>(cur_block,cur_latency));
    // td::map<BasicBlock*,std::map<std::pair<Value*, partition_info>, int>> accessCounterForBlock;
    if (accessCounterForBlock.find(cur_block) == accessCounterForBlock.end())
    {
        accessCounterForBlock[cur_block][tmp_pair] = 1;
    }
    else if (accessCounterForBlock[cur_block].find(tmp_pair) == accessCounterForBlock[cur_block].end())
    {
        accessCounterForBlock[cur_block][tmp_pair] = 1;
    }
    else 
    {
        accessCounterForBlock[cur_block][tmp_pair] ++;
    }

    if (target2LastAccessCycleInBlock.find(target) == target2LastAccessCycleInBlock.end())
    {
        std::map<BasicBlock*,std::vector<std::pair<std::pair<int, partition_info>,Instruction*>>> tmp_map;
        std::vector<std::pair<std::pair<int, partition_info>,Instruction*>> tmp_vec;
        tmp_vec.push_back(std::pair<std::pair<int, partition_info>,Instruction*>(std::pair<int, partition_info>(cur_latency,target_partition),access));
        tmp_map[cur_block] = tmp_vec;
        target2LastAccessCycleInBlock[target] = tmp_map;
        if (DEBUG) *BRAM_log << "(new map new vector)\n";
        return;
    }
    if (target2LastAccessCycleInBlock[target].find(cur_block) == target2LastAccessCycleInBlock[target].end())
    {
        std::vector<std::pair<std::pair<int, partition_info>,Instruction*>> tmp_vec;
        tmp_vec.push_back(std::pair<std::pair<int, partition_info>,Instruction*>(std::pair<int, partition_info>(cur_latency,target_partition),access));
        target2LastAccessCycleInBlock[target][cur_block] = tmp_vec;
        if (DEBUG) *BRAM_log << "(new vector)\n";
        return;
    }
    if (DEBUG) *BRAM_log << "(existed vector)\n";
    target2LastAccessCycleInBlock[target][cur_block].push_back(std::pair<std::pair<int, partition_info>,Instruction*>(std::pair<int, partition_info>(cur_latency, target_partition),access));
}



// evaluate the number of LUT needed by the BRAM MUXs
HI_WithDirectiveTimingResourceEvaluation::resourceBase HI_WithDirectiveTimingResourceEvaluation::BRAM_MUX_Evaluate()
{
    resourceBase res(0,0,0,clock_period);
    int inputSize = 0;

    for (auto partition2cnt_pair : partition2cnt)
    {
        int read_counter_for_value = partition2cnt_pair.second.first;
        int write_counter_for_value = partition2cnt_pair.second.second;
        int access_counter_for_value = read_counter_for_value + write_counter_for_value;

        if (DEBUG) *Evaluating_log << " for partition:" << partition2cnt_pair.first << " read_counter_for_value=" << read_counter_for_value << " write_counter_for_value=" << write_counter_for_value << "\n";
        if (DEBUG) Evaluating_log->flush();

        // This analysis is based on observation
        if (access_counter_for_value == 1)
        {
            inputSize += 0;
        }
        else if (access_counter_for_value <= 2)
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
                else  if (read_counter_for_value == 2)
                {
                    inputSize += 4; 
                } 
                else  if (write_counter_for_value == 2)
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

    // for (auto Val_IT : target2LastAccessCycleInBlock)
    // {
    //     int access_counter_for_value = 0;
    //     int read_counter_for_value = 0;
    //     int write_counter_for_value = 0;
    //     if (DEBUG) *Evaluating_log << " The access to target: [" << Val_IT.first->getName() <<"] includes:\n";
    //     for (auto B2Cycles : Val_IT.second)
    //     {
    //         access_counter_for_value += B2Cycles.second.size();
    //         if (DEBUG) *Evaluating_log << " in block: [" <<B2Cycles.first->getParent()->getName() << "-" <<B2Cycles.first->getName() <<"] cycles: ";
    //         for (auto C_tmp : B2Cycles.second)
    //         {                
    //             if (auto readI = dyn_cast<LoadInst>(C_tmp.second))
    //             {
    //                 read_counter_for_value ++ ;
    //                 if (DEBUG) *Evaluating_log << " --- R(c" << C_tmp.first.first << ",p" << C_tmp.first.second <<") ";
    //             }
    //             if (auto writeI = dyn_cast<StoreInst>(C_tmp.second))
    //             {
    //                 write_counter_for_value ++ ;
    //                 if (DEBUG) *Evaluating_log << " --- W(c" << C_tmp.first.first << ",p" << C_tmp.first.second <<") ";
    //             }
    //         }
    //         if (DEBUG) *Evaluating_log << " \n";
    //         if (DEBUG) Evaluating_log->flush();
    //     }
    //     if (DEBUG) *Evaluating_log << " \n\n";

    //     // This analysis is based on observation
    //     if (access_counter_for_value == 1)
    //     {
    //         inputSize = 0;
    //     }
    //     else if (access_counter_for_value <= 2)
    //     {
    //         if (read_counter_for_value == 1)
    //         {
    //             inputSize += 3;            
    //         }
    //     }
    //     else if (access_counter_for_value <= 4)
    //     {
    //         if (access_counter_for_value == 3)
    //         {
    //             if (read_counter_for_value == 3)
    //             {
    //                 inputSize += 3; 
    //             }
    //             else if (write_counter_for_value == 3)
    //             {
    //                 inputSize += 4; 
    //             }
    //             else  if (read_counter_for_value == 2)
    //             {
    //                 inputSize += 4; 
    //             } 
    //             else  if (write_counter_for_value == 2)
    //             {
    //                 inputSize += 4; 
    //             }
    //             else
    //             {
    //                 inputSize += 3; 
    //             }
    //         }
    //         else
    //         {
    //             inputSize += 6;
    //         }
    //     }
    //     else 
    //     {
    //         inputSize += (access_counter_for_value + 2);
    //     }
    // }

    res.LUT = inputSize * 5;
    return res;

}


// get the number of BRAMs which are needed by the alloca instruction
HI_WithDirectiveTimingResourceEvaluation::resourceBase HI_WithDirectiveTimingResourceEvaluation::get_BRAM_Num_For(AllocaInst *alloca_I)
{
        // real_depth_for_one_partition = node['depth'] // partition_factor_for_the_array
        // if ((node['depth'] % partition_factor_for_the_array) > 0):
        //     real_depth_for_one_partition = real_depth_for_one_partition + 1

        // BRAM_cnt = BRAM_cnt + bramEval(
        //     node['width'],
        //     real_depth_for_one_partition) * partition_factor_for_the_array

    resourceBase res(0,0,0,0,clock_period);
    if (DEBUG) *BRAM_log << "\n\nchecking allocation instruction [" << *alloca_I << "] and its type is: " << *alloca_I->getType() << " and its ElementType is: [" << *alloca_I->getType()->getElementType()  << "]\n";
    Type* tmp_type = alloca_I->getType()->getElementType();

    while (auto array_T = dyn_cast<ArrayType>(tmp_type))
    {
        if (DEBUG) *BRAM_log << "----- element type of : " << *tmp_type << " is " << *(array_T->getElementType()) << " and the number of its elements is " << (array_T->getNumElements()) <<"\n";
        tmp_type = array_T->getElementType();
    }

    int eachPartitionDepth = getEachPartitionDepth(Target2ArrayInfo[alloca_I]);
    int totalPartitionNum = getTotalPartitionNum(Target2ArrayInfo[alloca_I]);

    int BW = 0;
    if (tmp_type->isIntegerTy())
        BW = tmp_type->getIntegerBitWidth();
    else if (tmp_type->isFloatTy())
        BW = 32;
    else if (tmp_type->isDoubleTy())
        BW = 64;
    assert(BW!=0 && "we should get BW for the basic element type.\n");
    res = get_BRAM_Num_For(BW, eachPartitionDepth) * totalPartitionNum;

    if (DEBUG) *BRAM_log << "checked allocation instruction [" << *alloca_I 
                         << "] and its basic elemenet type is: [" << *tmp_type 
                         << "] with BW=[" << BW 
                         << "] and the total number of basic elements of each partition is: ["  << eachPartitionDepth 
                         << "] and the total number of partitions is "  << totalPartitionNum 
                         << "and it need BRAMs [" << res.BRAM << "].\n\n";
    
    return res;
}

// get the number of BRAMs which are needed by the argument of topfunction interface
HI_WithDirectiveTimingResourceEvaluation::resourceBase HI_WithDirectiveTimingResourceEvaluation::get_BRAM_Num_For(Argument *ArgTarget)
{
        // real_depth_for_one_partition = node['depth'] // partition_factor_for_the_array
        // if ((node['depth'] % partition_factor_for_the_array) > 0):
        //     real_depth_for_one_partition = real_depth_for_one_partition + 1

        // BRAM_cnt = BRAM_cnt + bramEval(
        //     node['width'],
        //     real_depth_for_one_partition) * partition_factor_for_the_array

    resourceBase res(0,0,0,0,clock_period);

    auto ptrType = dyn_cast<PointerType>(ArgTarget->getType());

    if (DEBUG) *BRAM_log << "\n\nchecking argument [" << *ArgTarget << "] and its type is: " << *ptrType << " and its ElementType is: [" << *ptrType->getElementType()  << "]\n";
    if (DEBUG) BRAM_log->flush();
    Type* tmp_type = ptrType->getElementType();
    while (auto array_T = dyn_cast<ArrayType>(tmp_type))
    {
        if (DEBUG) *BRAM_log << "----- element type of : " << *tmp_type << " is " << *(array_T->getElementType()) << " and the number of its elements is " << (array_T->getNumElements()) <<"\n";
        tmp_type = array_T->getElementType();
    }

    int eachPartitionDepth = getEachPartitionDepth(Target2ArrayInfo[ArgTarget]);
    int totalPartitionNum = getTotalPartitionNum(Target2ArrayInfo[ArgTarget]);

    int BW = 0;
    if (tmp_type->isIntegerTy())
        BW = tmp_type->getIntegerBitWidth();
    else if (tmp_type->isFloatTy())
        BW = 32;
    else if (tmp_type->isDoubleTy())
        BW = 64;
    assert(BW!=0 && "we should get BW for the basic element type.\n");
    res = get_BRAM_Num_For(BW, eachPartitionDepth) * totalPartitionNum;

    if (DEBUG) *BRAM_log << "checked argument [" << *ArgTarget 
                         << "] and its basic elemenet type is: [" << *tmp_type 
                         << "] with BW=[" << BW 
                         << "] and the total number of basic elements of each partition is: ["  << eachPartitionDepth 
                         << "] and the total number of partitions is "  << totalPartitionNum 
                         << "and it need BRAMs [" << res.BRAM << "].\n\n";
    
    return res;
}


// get the depth of each partition of the target array
int HI_WithDirectiveTimingResourceEvaluation::getEachPartitionDepth(HI_ArrayInfo &refInfo)
{
    int res = 1;
    for (int i=0; i<refInfo.num_dims; i++)
    {
        int tmp_partition_size = refInfo.dim_size[i] /  refInfo.partition_size[i] + !(refInfo.dim_size[i] %  refInfo.partition_size[i]==0);
        res *= tmp_partition_size;
    }
    return res;
}

// get the total number of partitions of the target array
int HI_WithDirectiveTimingResourceEvaluation::getTotalPartitionNum(HI_ArrayInfo &refInfo)
{
    int res = 1;
    for (int i=0; i<refInfo.num_dims; i++)
    {
        res *= refInfo.partition_size[i];
    }
    return res;
}

// get the number of BRAMs which are needed by the alloca instruction
HI_WithDirectiveTimingResourceEvaluation::resourceBase HI_WithDirectiveTimingResourceEvaluation::get_BRAM_Num_For(int width, int depth)
{
    resourceBase res(0,0,0,0,clock_period);
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
        

    int  depth_factor = depth / depth_uint;
    if ((depth % depth_uint) > 0)
    {
        depth_factor = depth_factor + 1;
    }
    res.BRAM = width_factor * depth_factor;
    return res;
}



// get the offset of the access to array and get the access information based on the addition (address offset + array pointer)
// extract the access pattern from the traces like: ({{%0+100,+,1}<nuw><nsw><%for.cond1.preheader>,+,200}<nuw><%for.body4>)
// however, there are different patterns so we need to process them in different ways.
void HI_WithDirectiveTimingResourceEvaluation::TryArrayAccessProcess(Instruction *I, ScalarEvolution *SE)
{

    // handle the situation for access instruction, which address come from Value or allocaInst
    if (processNaiveAccess(I))
        return;
    
    // handle the situation for access instruction, which address come from ITP, where the I in ITP come from addition 
/*
1.  get the initial value of access address by using SCEV
2.  calculate the index of the access for different dimension
*/
    if (I->getOpcode() != Instruction:: Add)
        return;
    if (Inst_AccessRelated.find(I) == Inst_AccessRelated.end())
        return;

    auto ITP_I = dyn_cast<IntToPtrInst>(I->use_begin()->getUser());

    if (!ITP_I)
        return;

    bool isMemoryAccess = dyn_cast<LoadInst>(ITP_I->use_begin()->getUser()) || dyn_cast<StoreInst>(ITP_I->use_begin()->getUser()) ;

    if (DEBUG) *ArrayLog << *I << " --> is under processing of ArrayAccessProcess\n";    
    // ArrayLog->flush();
                // if (!(ITP_I->hasOneUse() && isMemoryAccess))truction:   store i32 %add20.1, i32* %10, align 4, !db
                // {
                //     llvm::errs() << "ERROR at : " << *I << "===>" << ITP_I->hasOneUse() << "-->" << isMemoryAccess << "\n";
                // }
                // assert(ITP_I->hasOneUse() && isMemoryAccess && "This instruction shoudl be directly used for one memory access.");

    Instruction* sample_accessI = dyn_cast<Instruction>(ITP_I->use_begin()->getUser());
    

    // 1.  get the initial value of access address by using SCEV
    const SCEV *tmp_S = SE->getSCEV(I);

    if (DEBUG) *ArrayLog << *I << " --> SCEV: " << *tmp_S << "\n";  


    // const SCEV *tryCleanS = tryGetPureAddOrAddRecSCEV(tmp_S, 0);
    // if (tryCleanS)
    //     *ArrayLog << *I << " --> tryGetPureAddOrAddRecSCEV SCEV: " << *tryCleanS << "\n";
    ArrayLog->flush();

    if (getUnknownNum(tmp_S)>1)
    {
        // it is a complex SCEV and hard to predict the access pattern, we assume that it could access all possible elements.
        // in the array
        if (DEBUG) *ArrayLog << *I << " --> SCEV: " << *tmp_S << " has multiple unknown values in expression. It will be a complex access.\n";
        handleComplexSCEVAccess(I, tmp_S);
        return;
    }

    const SCEVAddExpr *SAE = dyn_cast<SCEVAddExpr>(bypassExtTruntSCEV(tmp_S));

    if (SAE)
    {
        if ( auto constOffset = dyn_cast<SCEVConstant>(SAE->getOperand(0)))
            if (auto ptrUnknown = dyn_cast<SCEVUnknown>(SAE->getOperand(1)) )
            {
                handleConstantOffsetAccess(I, SAE);
                return;
            }
    }
    

    if (auto SUnkown = dyn_cast<SCEVUnknown>(tmp_S))
    {
        handleDirectAccess(I, SUnkown);
        return;
    }
    else if (auto SARE = dyn_cast<SCEVAddRecExpr>(tmp_S))
    {
        // example :   ({{%0+100,+,1}<nuw><nsw><%for.cond1.preheader>,+,200}<nuw><%for.body4>)
        handleSAREAccess(I, SARE);
        return;
    }
    else
    {        
        const SCEVAddRecExpr *SAREtmp = dyn_cast<SCEVAddRecExpr>(bypassExtTruntSCEV(SAE->getOperand(0)));
        const SCEVUnknown *SU = dyn_cast<SCEVUnknown>(findUnknown(tmp_S));
        const PtrToIntInst *PTI_I = dyn_cast<PtrToIntInst>(SU->getValue());
        if (SAE && SAREtmp && SU && PTI_I)
        {            
            // example :   ({{100,+,1}<nuw><nsw><%for.cond1.preheader>,+,200}<nuw><%for.body4> + %0)
            if (DEBUG) *ArrayLog << *I << " --> is a transformed SCEV\n";
            handleUnstandardSCEVAccess(I, tmp_S);
            return;
        }       
        
    }

    // it is a complex SCEV and hard to predict the access pattern, we assume that it could access all possible elements.
    // in the array
    handleComplexSCEVAccess(I, tmp_S);
    return;
}

// generate AccessInformation according to the target and the initial access
HI_WithDirectiveTimingResourceEvaluation::HI_AccessInfo HI_WithDirectiveTimingResourceEvaluation::getAccessInfoFor(Value* target, 
                                                                                                                    Instruction* access, int initial_offset, 
                                                                                                                     std::vector<int> *inc_indices, std::vector<int> *trip_counts, bool unpredictable)
{

    if (Alias2Target.find(target) != Alias2Target.end())
        target = Alias2Target[target];
        
    HI_AccessInfo res(Target2ArrayInfo[target]);

    if (unpredictable)
    {
        res.unpredictable = true;
        return res;
    }

    for (int i=0;i<res.num_dims;i++) // initial access indice
    {
        res.index[i] = (initial_offset / res.sub_element_num[i]) % res.dim_size[i];
    }

    if (inc_indices)
    {
        for (int i=0;i<trip_counts->size();i++) // initial access indice
        {        
            res.inc_index[i] = (*inc_indices)[i];
        }
    }
    if (trip_counts)
    {
        for (int i=0;i<trip_counts->size();i++) // initial access indice
        {        
            res.trip_count[i] = (*trip_counts)[i];
        }
        res.reverse_loop_dep = trip_counts->size();
    }
    else
    {
        res.reverse_loop_dep = -1;
    }

    res.initial_offset = initial_offset;

    for (int i=0;i<res.num_dims;i++) // initial access partition indice
    {
        if (res.cyclic[i])
            res.partition_id[i] = res.index[i] % res.partition_size[i];
        else
        {
            res.partition_id[i] = res.index[i] / ( (res.dim_size[i] + res.partition_size[i] - 1) / res.partition_size[i]);
        }
    }

    return res;
}


// get the initial index of the array access in the loop
const SCEV * HI_WithDirectiveTimingResourceEvaluation::findTheActualStartValue(const SCEVAddRecExpr *S)
{
    const SCEV *nextStart = bypassExtTruntSCEV(S->getOperand(0));
    if (const SCEVConstant *start_V = dyn_cast<SCEVConstant>(nextStart))
    {
        return start_V;
    }
    else if (const SCEVAddExpr *start_V = dyn_cast<SCEVAddExpr>(nextStart))
    {
        return start_V;
    }
    else if (const SCEVUnknown *start_V_unknown = dyn_cast<SCEVUnknown>(nextStart))
    {
        return start_V_unknown;
    }
    else
    {
        if (const SCEVAddRecExpr *SARE = dyn_cast<SCEVAddRecExpr>(nextStart))
        {
            return findTheActualStartValue(SARE);
        }
        else
        {
            assert(false && "should not reach here.");
        }
    }
}

// get the index incremental value of the array access in the loop
void HI_WithDirectiveTimingResourceEvaluation::findTheIncrementalIndexAndTripCount(const SCEVAddRecExpr *S, std::vector<int> &inc_indices, std::vector<int> &trip_counts)
{
    const SCEVConstant *IncrementalIndex = dyn_cast<SCEVConstant>(bypassExtTruntSCEV(S->getOperand(1)));
    assert(IncrementalIndex && "the incremental index should be found.");
    int inc_const = IncrementalIndex->getAPInt().getSExtValue();
    inc_indices.push_back(inc_const);


    int tripCount = SE->getSmallConstantMaxTripCount(S->getLoop());
    trip_counts.push_back(tripCount);

    const SCEV *nextStart = bypassExtTruntSCEV(S->getOperand(0));
    if (const SCEVConstant *start_V = dyn_cast<SCEVConstant>(nextStart))
    {
        return;
    }
    else if (const SCEVAddExpr *start_V = dyn_cast<SCEVAddExpr>(nextStart))
    {
        return;
    }
    else if (const SCEVUnknown *start_V_unknown = dyn_cast<SCEVUnknown>(nextStart))
    {
        return;
    }
    else
    {
        if (const SCEVAddRecExpr *SARE = dyn_cast<SCEVAddRecExpr>(nextStart))
        {
            findTheIncrementalIndexAndTripCount(SARE, inc_indices, trip_counts);
        }
        else
        {
            const SCEVAddRecExpr *SARE_bypass = dyn_cast<SCEVAddRecExpr>(nextStart);
            if (SARE_bypass)
                findTheIncrementalIndexAndTripCount(SARE_bypass, inc_indices, trip_counts);
            else
                assert(false && "should not reach here.");
        }
        
    }
    // if (const SCEVAddRecExpr *SARE = dyn_cast<SCEVAddRecExpr>(S->getOperand(0)))
    // {
    //     const SCEV * res = findTheIncrementalIndex(SARE,sonIsInnerMost);
    //     if (sonIsInnerMost)
    //     {
    //         sonIsInnerMost = false;
    //         return dyn_cast<SCEV>(S->getOperand(1));
    //     }
    //     else
    //     {
    //         return res;
    //     }
        
    // }
    // else if (const SCEVAddExpr *start_V = dyn_cast<SCEVAddExpr>(S->getOperand(0)))
    // {
    //     sonIsInnerMost = true;
    //     return nullptr;
    // }
    // else if (const SCEVUnknown *start_V_unknown = dyn_cast<SCEVUnknown>(S->getOperand(0)))
    // {
    //     sonIsInnerMost = true;
    //     return nullptr;
    // }
    // else
    // {
    //     assert(false && "the function should not reach here!!");
    // }
    // if (const SCEVAddExpr *start_V = dyn_cast<SCEVAddExpr>(S->getOperand(0)))
    // {
    //     return dyn_cast<SCEV>(S->getOperand(1));
    // }
    // else if (const SCEVUnknown *start_V_unknown = dyn_cast<SCEVUnknown>(S->getOperand(0)))
    // {
    //     return dyn_cast<SCEV>(S->getOperand(1));
    // }
    // else
    // {
    //     if (const SCEVAddRecExpr *SARE = dyn_cast<SCEVAddRecExpr>(S->getOperand(0)))
    //         return findTheIncrementalIndex(SARE);
    // }
}

// check the memory access in the function
void HI_WithDirectiveTimingResourceEvaluation::TraceMemoryAccessinFunction(Function &F)
{
    if (F.getName().find("llvm.")!=std::string::npos || F.getName().find("HIPartitionMux")!=std::string::npos) // bypass the "llvm.xxx" functions..
        return;
    findMemoryAccessin(&F);        
    
}


// find the array access in the function F and trace the accesses to them
// since except naive direct access (e.g. directly access to the address of array head pointer) defined 
// in the beginning of function TryArrayAccessProcess(), other accesses should have the address calculation
// process, like (array head pointer + a value). Based on this consideration, we trace Load/Store/IntToPtr in F
void HI_WithDirectiveTimingResourceEvaluation::findMemoryAccessin(Function *F)
{
    if (DEBUG) *ArrayLog << "checking the Memory Access information in Function: " << F->getName() << "\n";
    ArrayValueVisited.clear();


    // for general function in HLS, arrays in functions are usually declared with alloca instruction
    for (auto &B: *F)
    {
        for (auto &I: B)
        {
            if (IntToPtrInst *ITP_I = dyn_cast<IntToPtrInst>(&I))
            {
                if (DEBUG) *ArrayLog << "find a IntToPtrInst: [" << *ITP_I << "] backtrace to its operands.\n";
                TraceAccessRelatedInstructionForTarget(ITP_I);
            }
            else if (I.getOpcode() == Instruction::Load || I.getOpcode() == Instruction::Store)
            {
                Block2AccessList[I.getParent()].push_back(&I);
            }
        }
    }
    if (DEBUG) *ArrayLog << "-------------------------------------------------" << "\n\n\n\n";
    // ArrayLog->flush();
}

// find out which instrctuins are related to the array, going through PtrToInt, Add, IntToPtr, Store, Load instructions
void HI_WithDirectiveTimingResourceEvaluation::TraceAccessRelatedInstructionForTarget(Value *cur_node)
{
    if (DEBUG) *ArrayLog << "looking for the operands of " << *cur_node<< "\n";
    if (Instruction* tmpI = dyn_cast<Instruction>(cur_node))
    {       
        Inst_AccessRelated.insert(tmpI);
    }
    else
    {
        return;
    }    

    Instruction* curI = dyn_cast<Instruction>(cur_node);
    // ArrayLog->flush();

    // we are doing DFS now
    if (ArrayValueVisited.find(cur_node)!=ArrayValueVisited.end())
        return;

    ArrayValueVisited.insert(cur_node);

    // Trace the uses of the pointer value or integer generaed by PtrToInt
    for (int i = 0; i < curI->getNumOperands(); ++i)
    {
        Value * tmp_op = curI->getOperand(i);
        TraceAccessRelatedInstructionForTarget(tmp_op);
    }
    ArrayValueVisited.erase(cur_node);
}


HI_WithDirectiveTimingResourceEvaluation::HI_ArrayInfo HI_WithDirectiveTimingResourceEvaluation::getArrayInfo(Value* target)
{
    if (Target2ArrayInfo.find(target)!=Target2ArrayInfo.end())
        return Target2ArrayInfo[target];
    PointerType* ptr_type = dyn_cast<PointerType>(target->getType());
    if (!ptr_type)
    {
        llvm::errs() << "  " << "target:" << *target << " is not pointer type.\n";
        assert(false && "wrong type for array target.");
    }
    if (DEBUG) *ArrayLog << "\n\nchecking type : " << *ptr_type << " and its ElementType is: [" << *ptr_type->getElementType()  << "]\n";
    Type* tmp_type = ptr_type->getElementType();
    int total_ele = 1;
    int tmp_dim_size[10];
    int num_dims = 0;
    while (auto array_T = dyn_cast<ArrayType>(tmp_type))
    {
        if (DEBUG) *ArrayLog << "----- element type of : " << *tmp_type << " is " << *(array_T->getElementType()) << " and the number of its elements is " << (array_T->getNumElements()) <<"\n";
        total_ele *=  (array_T->getNumElements()) ;
        tmp_dim_size[num_dims] =  (array_T->getNumElements()) ;
        num_dims++;
        tmp_type = array_T->getElementType();
    }

    HI_ArrayInfo res_array_info;
    res_array_info.num_dims = num_dims;
    for (int i = 0; i<num_dims; i++)
    {
        res_array_info.dim_size[i] = tmp_dim_size[num_dims - i - 1];
    }

    res_array_info.sub_element_num[0] = 1;
    for (int i = 1; i<num_dims; i++)
    {
        res_array_info.sub_element_num[i] = res_array_info.sub_element_num[i-1] * res_array_info.dim_size[i-1];
    }

    if (auto arg_v = dyn_cast<Argument>(target))
    {

        if (num_dims==0)
            res_array_info.sub_element_num[num_dims] = 1;
        else
            res_array_info.sub_element_num[num_dims] = res_array_info.sub_element_num[num_dims-1] * res_array_info.dim_size[num_dims-1];

        std::string FuncName = demangleFunctionName(arg_v->getParent()->getName());
        std::string funcLine;
        std::string argName(arg_v->getName());

        for (int possibleLine :IRFunc2BeginLine[FuncName])
        {
            funcLine = std::to_string(possibleLine);
            if (FuncParamLine2OutermostSize.find(FuncName+"-"+argName+"-"+funcLine) != FuncParamLine2OutermostSize.end())
                break;
        }

        
        res_array_info.dim_size[num_dims] = FuncParamLine2OutermostSize[FuncName+"-"+argName+"-"+funcLine]; // set to nearly infinite
        res_array_info.num_dims ++;
        res_array_info.isArgument = 1;
    }
    else
    {
        if (auto global_v = dyn_cast<GlobalVariable>(target))
        {
            if (num_dims==0)
            {
                res_array_info.sub_element_num[num_dims] = 1;               
                res_array_info.dim_size[num_dims] = 1; // set to nearly infinite
                res_array_info.num_dims = 1;
            }
        }
        else if ( auto alloc_I = dyn_cast<AllocaInst>(target))
        {
            if (num_dims==0)
            {
                res_array_info.sub_element_num[num_dims] = 1;               
                res_array_info.dim_size[num_dims] = 1; // set to nearly infinite
                res_array_info.num_dims = 1;
            }
        }
    }
    
    res_array_info.elementType = tmp_type;
    res_array_info.target = target;

    matchArrayAndConfiguration(target, res_array_info);

    int totalPartitionNum = getTotalPartitionNum(res_array_info);

    if (!res_array_info.completePartition)
    {
        if (res_array_info.sub_element_num[num_dims-1] * res_array_info.dim_size[num_dims-1] == totalPartitionNum)
            res_array_info.completePartition = 1;
    }

    return res_array_info;
}

// get the targer partition for the specific memory access instruction
std::vector<HI_WithDirectiveTimingResourceEvaluation::partition_info> HI_WithDirectiveTimingResourceEvaluation::getPartitionFor(Instruction* access)
{
    int partition_factor = -1, partition_dimension = -1;
    
    auto &targetVec = Access2TargetMap[access];
    HI_PragmaInfo tmp_PragmaInfo;
    Value *reftarget = Access2TargetMap[access][0];
    if (Alias2Target.find(reftarget) != Alias2Target.end())
        reftarget = Alias2Target[reftarget];
    if (targetVec.size()>1)
    {
        for (auto target:Access2TargetMap[access])
        {

            Value *tmp_target = target, *tmp_reftarget = reftarget;
            if (Alias2Target.find(target) != Alias2Target.end())
                tmp_target = Alias2Target[target];
            if (Alias2Target.find(reftarget) != Alias2Target.end())
                tmp_reftarget = Alias2Target[reftarget];
            if (tmp_target!=tmp_reftarget)
            {
                llvm::errs()  << *access << " has multi-targets: \n";
                llvm::errs()  << *tmp_target << "  is different form " << *tmp_reftarget << "\n";
                for (auto target:Access2TargetMap[access])
                    llvm::errs()  <<  "    " <<*target << "\n";
            }
            assert(tmp_target==tmp_reftarget && "currently, we do not support 1-access-multi-target.");
        }
        // llvm::errs() << "handling access: " << *access << " (addr=" <<access << ") include following targets:  \n";
        // for (auto target : targetVec)
        // {
        //     llvm::errs() << "    " << *target << " (addr=" <<target << ") \n";
        // }
        // assert(false && "cannot handle access to multiple potential targets.\n");
    }

    HI_AccessInfo tmp_res = getAccessInfoForAccessInst(access);
    std::vector<partition_info> partitions;

    assert(!tmp_res.unpredictable && "unpredictable access shoudl not be considered to use getPartitionFor.");

    int tripCountMax = 1;
    if (LI->getLoopFor(access->getParent()))
        tripCountMax = SE->getSmallConstantMaxTripCount(LI->getLoopFor(access->getParent()));
    
    if (DEBUG) *ArrayLog << "checking possible access offsets for Inst: " << *access << ":\n";
    {
        Instruction *pointer_I = nullptr;
        Value *pointer_V = nullptr;
        if (access->getOpcode()==Instruction::Load)
        {
            pointer_I = dyn_cast<Instruction>(access->getOperand(0));
            pointer_V = (access->getOperand(0));
        }
        else
        {
            pointer_I = dyn_cast<Instruction>(access->getOperand(1));
            pointer_V = (access->getOperand(1));
        }
        Value *address_addI = nullptr;
        if (pointer_I)
        {
            if (DEBUG) *ArrayLog << "address_addI is intstruction : " << *pointer_I->getOperand(0) << ":\n";
            address_addI = (pointer_I->getOperand(0));
        }
        else
        {
            if (DEBUG) *ArrayLog << "address_addI is value : " << *pointer_V << ":\n";
            address_addI = pointer_V; // the access may not need the calculation of address, take the pointer directly
        }
            
    }
    
    int num_partitions = 1; 
    for (int i=0;i<tmp_res.num_dims;i++) 
        num_partitions *= tmp_res.partition_size[i];

    auto offsets = generatePotentialOffset(tmp_res);
    bool incByOne = 1;
    bool decByOne = 1;
    for (int i=0; i < offsets.size(); i++)
    {
        int tmp_offset = offsets[i];
        if (i>0)
        {
            if (offsets[i]-offsets[i-1]!=1) 
                incByOne = 0;
            if (offsets[i]-offsets[i-1]!=-1) 
                decByOne = 0;
        }

        if (num_partitions != partitions.size())
        {
            // ArrayLog->flush();
            // some time the remainder loop after unrolling will lead to fake out-of-range access, ignore it.
            partition_info newPartitionForCheck = getAccessPartitionBasedOnAccessInfoAndInc(tmp_res, tmp_offset);
            if (DEBUG) *ArrayLog << tmp_offset << "-" << newPartitionForCheck << ", ";
            // int cur_dim_index = (tmp_offset / tmp_res.sub_element_num[partition_dimension] % tmp_res.sub_element_num[partition_dimension+1]);
            // if (!tryRecordPartition(partitions, newPartitionForCheck))
            //     break;
            tryRecordPartition(partitions, newPartitionForCheck);
        }

    }


    // here we need to check:
    // whether the partition selection of array number requires remainder calculation.
    // add remainder latency for the mux if require, for example, the stride of access offset is 4, 
    // e.g. for (i=0;i<100;i+=4) A[i]++;, 
    // while the partition factor of A[] is 3
    // we need to calculate i%3 to map the partition for the load instruction
    if (incByOne || decByOne)
    {
        if (DEBUG) *ArrayLog << "addressCalculationCheck: the access Inst: " << *access << " index increases by one.\n";
        Access2EnableArrayNo[access] = 0;
    }
    else
    {
        // there are 3 situations we don't need urem (remainder calculator) for array partition accesses.
        //      1. The access is for one constant partition
        //      2. The partition factor for each dimension is the power of 2
        //      3. The access pattern of the array is (array index increased/decreased by one)
        // in these siutations, the array partition is easy to determine during runtime without remainder.
        if (partitions.size() == 1)
        {
            Access2EnableArrayNo[access] = 0;
            if (DEBUG) *ArrayLog << "addressCalculationCheck: the access Inst: " << *access << " access  one constant partition.\n";
        }
        else
        {
            for (int i=0; i<tmp_res.num_dims;i++)
            {
                if (!isPowerOf2_32(tmp_res.partition_size[i]))
                {
                    int part_id = -1;
                    for (auto partition : partitions)
                    {
                        if (part_id<0)
                        {
                            part_id = partition.partition_id[i];
                        }
                        else if (part_id != partition.partition_id[i])
                        {
                            if (DEBUG) *ArrayLog << "addressCalculationCheck: the access Inst: " << *access << " (non-2-power)"  << " partition@dim=" << " access different partition.\n";
                            Access2EnableArrayNo[access] = 1;
                        }
                    }
                    
                }
                else
                {
                    if (DEBUG) *ArrayLog << "addressCalculationCheck: the access Inst: " << *access << " partition@dim=" << i+1 << " 2-power .\n";
                }
            }
        }

    }

    if (Access2EnableArrayNo.find(access) == Access2EnableArrayNo.end())
    {
        Access2EnableArrayNo[access] = 1;
    }

    if (Access2EnableArrayNo[access])
    {
        if (DEBUG) *ArrayLog << "addressCalculationCheck: the access Inst: " << *access << " requires remainder calculation.\n";
    }
    else
    {
        if (DEBUG) *ArrayLog << "addressCalculationCheck: the access Inst: " << *access << " does not require remainder calculation.\n";
    }
    

    if (DEBUG) *ArrayLog << "\n";
    if (DEBUG) ArrayLog->flush();
    return partitions;
}

// try to record a partition as a potential target
bool HI_WithDirectiveTimingResourceEvaluation::tryRecordPartition(std::vector<partition_info> &partitions, partition_info try_target)
{
    for (auto val : partitions)
        if (val == try_target)
            return false;
    partitions.push_back(try_target);
    return true;
}

// get the offset of the access to array and get the access information
// in the function
void HI_WithDirectiveTimingResourceEvaluation::ArrayAccessCheckForFunction(Function *F)
{
    for (auto &B : *F)
    {
        for (auto &I : B)
        {
            TryArrayAccessProcess(&I, SE /*, demangleFunctionName(F->getName()) == top_function_name*/);
            // checkAliasFor(&I);
        }
    }
    // for (auto &B : *F)
    // {
    //     for (auto &I : B)
    //     {
    //         printAliasFor(&I);
    //     }
    // }
}


/*
    get the access information for the load/store instruction, by tracing back to the address instruction
*/
HI_WithDirectiveTimingResourceEvaluation::HI_AccessInfo HI_WithDirectiveTimingResourceEvaluation::getAccessInfoForAccessInst(Instruction* Load_or_Store)
{
    Instruction *pointer_I = nullptr;
    Value *pointer_V = nullptr;
    if (Load_or_Store->getOpcode()==Instruction::Load)
    {
        pointer_I = dyn_cast<Instruction>(Load_or_Store->getOperand(0));
        pointer_V = (Load_or_Store->getOperand(0));
    }
    else
    {
        pointer_I = dyn_cast<Instruction>(Load_or_Store->getOperand(1));
        pointer_V = (Load_or_Store->getOperand(1));
    }
    Value *address_addI = nullptr;
    if (pointer_I)
    {
        if (pointer_I->getOpcode() == Instruction::IntToPtr)
            address_addI = (pointer_I->getOperand(0));
        else
            address_addI = pointer_I;
    }
    else
    {
        address_addI = pointer_V; // the access may not need the calculation of address, take the pointer directly
        if (Alias2Target.find(address_addI) != Alias2Target.end()) // it could be argument. We need to trace back to get its original array declaration
        {
            address_addI = Alias2Target[address_addI];
        }  
    }

    // if (auto PTI = dyn_cast<PtrToIntInst>(address_addI))
    // {
    //     address_addI = PTI->getOperand(0);
    //     if (Alias2Target.find(address_addI) != Alias2Target.end())
    //         address_addI = Alias2Target[address_addI];
    // }


    assert(address_addI && "The pointer for this access should be found.\n");
    if (AddressInst2AccessInfo.find(address_addI)==AddressInst2AccessInfo.end())
        llvm::errs() << "Instruction: " << *Load_or_Store << " addressI: " << *address_addI << "<=======\n";
    assert(AddressInst2AccessInfo.find(address_addI)!=AddressInst2AccessInfo.end() && "The pointer should be checked by TryArrayAccessProcess() previously.");
    return AddressInst2AccessInfo[address_addI];
}




AliasResult HI_WithDirectiveTimingResourceEvaluation::HI_AAResult::alias(const MemoryLocation &LocA,
                               const MemoryLocation &LocB) {
//   auto PtrA = LocA.Ptr;
//   auto PtrB = LocB.Ptr;

//   if (PtrA != PtrA) {
//     return NoAlias;
//   }
  // NO USE
    // NO USE
    // NO USE
    // NO USE// NO USE// NO USE// NO USE
    // NO USE// NO USE// NO USE// NO USE
    // NO USE// NO USE// NO USE// NO USE
  // Forward the query to the next analysis.
  return AAResultBase::alias(LocA, LocB);
}

bool HI_WithDirectiveTimingResourceEvaluation::checkAccessAlias(Instruction *I0, Instruction *I1)
{
    Instruction *pointer_I0 = nullptr, *pointer_I1 = nullptr;
    if (I0->getOpcode()==Instruction::Load)
    {
        pointer_I0 = dyn_cast<IntToPtrInst>(I0->getOperand(0));
    }
    else if (I0->getOpcode()==Instruction::Store)
    {
        pointer_I0 = dyn_cast<IntToPtrInst>(I0->getOperand(1));
    }

    if (I1->getOpcode()==Instruction::Load)
    {
        pointer_I1 = dyn_cast<IntToPtrInst>(I1->getOperand(0));
    }
    else if (I1->getOpcode()==Instruction::Store)
    {
        pointer_I1 = dyn_cast<IntToPtrInst>(I1->getOperand(1));
    }


    if (!pointer_I0 ^ !pointer_I1)
    {
        return true;
    }
    else
    {
        return pointer_I0==pointer_I1;
    }
    
    // assert(pointer_I0 && pointer_I0->getOpcode() == Instruction::IntToPtr && "ITP should be found for the access instruction");
    // assert(pointer_I1 && pointer_I1->getOpcode() == Instruction::IntToPtr && "ITP should be found for the access instruction");

    std::string tmp0(""),tmp1("");
    raw_string_ostream *SCEV_Stream0 = new raw_string_ostream(tmp0);
    raw_string_ostream *SCEV_Stream1 = new raw_string_ostream(tmp1);
    
    const SCEV *tmp_S0 = SE->getSCEV(pointer_I0->getOperand(0));
    const SCEV *tmp_S1 = SE->getSCEV(pointer_I1->getOperand(0));
    
    if (!tmp_S0 || !tmp_S1)
        return false;

    Optional<APInt> res=computeConstantDifference(tmp_S0,tmp_S1);
    
    if (res != None)
    {
        if (DEBUG) *ArrayLog << "distance between them =" << res.getValue().getSExtValue() << "\n"; 
        if (res.getValue().getSExtValue() == 0)
            return true;
        else
            return false;
    }
    return true;

    *SCEV_Stream0 << *tmp_S0;
    *SCEV_Stream1 << *tmp_S1;


    bool result = SCEV_Stream0->str() == SCEV_Stream1->str();

    
    // llvm::errs() << *tmp_S0 << "\n";
    // llvm::errs() << *tmp_S1 << "\n";

    if (DEBUG) *ArrayLog << "    " << *I0 << " ==> " << SCEV_Stream0->str() << "\n"; 
    if (DEBUG) *ArrayLog << "    " << *I1 << " ==> " << SCEV_Stream1->str() << "\n";

    delete SCEV_Stream0;
    delete SCEV_Stream1;

    return result;

}


void HI_WithDirectiveTimingResourceEvaluation::checkAliasFor(Instruction *I)
{
    if (I->getOpcode()!=Instruction::Load && I->getOpcode()!=Instruction::Store)
        return;

    BasicBlock *curB = I->getParent();

    for (auto &anotherI : *curB)
    {
        if (anotherI.getOpcode()!=Instruction::Load && anotherI.getOpcode()!=Instruction::Store)
            continue;

        if (&anotherI == I)
            continue;

        if (DEBUG) *ArrayLog << "check alias between <<" << *I << ">> and <<" << anotherI << ">> .\n"; 
        if (!noAliasHazard(I, &anotherI))
        {
            if (DEBUG) *ArrayLog << " (==could be Alias==).\n";
            accessAliasSet.insert(std::pair<Instruction*, Instruction*>(I, &anotherI));
            accessAliasMap[I].push_back(&anotherI);
        }
        else
        {
            if (DEBUG) *ArrayLog << " (NO  Alias).\n";
        }        
    }
}

void HI_WithDirectiveTimingResourceEvaluation::printAliasFor(Instruction *I)
{
    if (accessAliasMap.find(I) != accessAliasMap.end())
    {
        if (DEBUG) *ArrayLog << "Instruction: " << *I << " alias with :\n";
        for (auto anotherI : accessAliasMap[I])
        {
            if (DEBUG) *ArrayLog << "    " << *anotherI << "\n";
        }
        *ArrayLog << "\n";
    }
}

bool HI_WithDirectiveTimingResourceEvaluation::hasRAWHazard(Instruction *loadI, int cycle)
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
                if (Inst_Schedule[preI].second >= cycle && !noAliasHazard(loadI, preI))
                {
                    if (DEBUG) *BRAM_log << "\nload instruction: " << *loadI << " RAW hazard with store instruction: " << *preI 
                               << " at cycle#" << Inst_Schedule[preI].second << "\n";
                    return true;
                }
            }                
        }
    }
    return false;
}

// get the target array from the load/store instruction
Value* HI_WithDirectiveTimingResourceEvaluation::getTargetFromInst(Instruction* accessI)
{
    // assert(Access2TargetMap[accessI].size()==1 && "currently, we do not support 1-access-multi-target.");
    if (Access2TargetMap.find(accessI) == Access2TargetMap.end())
    {
        llvm::errs() << " the instruction [" << *accessI << "] is not mapped to target.\n";
        assert(false && "the instruction should be mapped to target.\n");
    }
    if (Access2TargetMap[accessI].size()>1)
    {
        Value *reftarget = Access2TargetMap[accessI][0];
        for (auto target:Access2TargetMap[accessI])
        {

            Value *tmp_target = target, *tmp_reftarget = reftarget;
            if (Alias2Target.find(target) != Alias2Target.end())
                tmp_target = Alias2Target[target];
            if (Alias2Target.find(reftarget) != Alias2Target.end())
                tmp_reftarget = Alias2Target[reftarget];
            if (tmp_target!=tmp_reftarget)
            {
                llvm::errs()  << *accessI << " has multi-targets: \n";
                llvm::errs()  << *tmp_target << "  is different form " << *tmp_reftarget << "\n";
                for (auto target:Access2TargetMap[accessI])
                    llvm::errs()  <<  "    " <<*target << "\n";
            }
            assert(tmp_target==tmp_reftarget && "currently, we do not support 1-access-multi-target.");
        }
    }
    assert(Access2TargetMap.find(accessI)!=Access2TargetMap.end());
    Value* target = Access2TargetMap[accessI][0];
    if (Alias2Target.find(target) == Alias2Target.end())
        return target;
    else
    {
        assert(Alias2Target.find(target)!=Alias2Target.end());
        return Alias2Target[target];
    }
}
    
// get the target array from the call instruction
std::vector<Value*> HI_WithDirectiveTimingResourceEvaluation::getTargetsFromInst(Instruction* accessI)
{
    // assert(Access2TargetMap[accessI].size()==1 && "currently, we do not support 1-access-multi-target.");
    if (Access2TargetMap.find(accessI) == Access2TargetMap.end())
    {
        llvm::errs() << " the instruction [" << *accessI << "] is not mapped to target.\n";
        assert(false && "the instruction should be mapped to target.\n");
    }
    
    std::vector<Value*> res_vec;
    for (auto target:Access2TargetMap[accessI])
    {
        Value *tmp_target = target;
        if (Alias2Target.find(target) != Alias2Target.end())
            tmp_target = Alias2Target[target];
        res_vec.push_back(tmp_target);
    }
    return res_vec;

}

HI_WithDirectiveTimingResourceEvaluation::partition_info HI_WithDirectiveTimingResourceEvaluation::getAccessPartitionBasedOnAccessInfoAndInc(HI_WithDirectiveTimingResourceEvaluation::HI_AccessInfo refInfo, int cur_offset)
{
    partition_info res_partiton_info;
    res_partiton_info.num_dims = refInfo.num_dims; 
    res_partiton_info.port_num = refInfo.port_num; 
    Value *reftarget = refInfo.target;
    if (Alias2Target.find(reftarget) != Alias2Target.end())
        reftarget = Alias2Target[reftarget];
    res_partiton_info.target = reftarget;
    int initial_offset = cur_offset;
    for (int i=0;i<refInfo.num_dims;i++) // initial access indice
    {
        refInfo.index[i] = (initial_offset / refInfo.sub_element_num[i]) % refInfo.dim_size[i];
    }
    refInfo.initial_offset = initial_offset;

    for (int i=0;i<refInfo.num_dims;i++) // initial access partition indice
    {
        if (refInfo.cyclic[i])
        {
            refInfo.partition_id[i] = refInfo.index[i] % refInfo.partition_size[i];
            res_partiton_info.partition_id[i] = refInfo.partition_id[i];
        }
        else
        {
            refInfo.partition_id[i] = refInfo.index[i] / ( (refInfo.dim_size[i] + refInfo.partition_size[i] - 1) / refInfo.partition_size[i]);
            res_partiton_info.partition_id[i] = refInfo.partition_id[i];
        }
    }
    return res_partiton_info;
}

bool HI_WithDirectiveTimingResourceEvaluation::processNaiveAccess(Instruction *Load_or_Store)
{

    if (Load_or_Store->getOpcode()!=Instruction::Load && Load_or_Store->getOpcode()!=Instruction::Store)
        return false;


    Instruction *pointer_I = nullptr;
    Value *pointer_V = nullptr;
    if (Load_or_Store->getOpcode()==Instruction::Load)
    {
        pointer_I = dyn_cast<Instruction>(Load_or_Store->getOperand(0));
        pointer_V = (Load_or_Store->getOperand(0));
    }
    else
    {
        pointer_I = dyn_cast<Instruction>(Load_or_Store->getOperand(1));
        pointer_V = (Load_or_Store->getOperand(1));
    }
    Value *address_addI = nullptr;
    if (!pointer_I && pointer_V)
    {
        Value *target = pointer_V;
        if (Target2ArrayInfo.find(target) == Target2ArrayInfo.end())
        {
            if (Alias2Target.find(target) != Alias2Target.end()) // it could be argument. We need to trace back to get its original array declaration
            {
                target = Alias2Target[target];
            }
            else
            {
                llvm::errs() << "ERRORS: cannot find target [" << *target << "] in Target2ArrayInfo and its address=" 
                                << target << "\n";
                assert(Target2ArrayInfo.find(target) != Target2ArrayInfo.end() 
                            && Alias2Target.find(target) != Alias2Target.end()
                            && "Fail to find the array inforamtion for the target.");
            }
        }
        if (auto arg_pointer = dyn_cast<Argument>(target))
        {
            AddressInst2AccessInfo[target] = getAccessInfoFor(target, Load_or_Store, 0, nullptr, nullptr);
            if (DEBUG) *ArrayLog << " -----> access info with array index: " << AddressInst2AccessInfo[target] << "\n\n\n";
            if (DEBUG) ArrayLog->flush();
        }
        else if (auto alloc_pointer = dyn_cast<AllocaInst>(target))
        {
            AddressInst2AccessInfo[target] = getAccessInfoFor(target, Load_or_Store, 0, nullptr, nullptr);
            if (DEBUG) *ArrayLog << " -----> access info with array index: " << AddressInst2AccessInfo[target] << "\n\n\n";
            if (DEBUG) ArrayLog->flush();
        }
        else if (auto GV = dyn_cast<GlobalVariable>(target))
        {
            AddressInst2AccessInfo[target] = getAccessInfoFor(target, Load_or_Store, 0, nullptr, nullptr);
            if (DEBUG) *ArrayLog << " -----> access info with array index: " << AddressInst2AccessInfo[target] << "\n\n\n";
        }
    }


    if (pointer_I)
    {
        if (auto IPT=dyn_cast<IntToPtrInst>(pointer_I))
        {
            if (auto PTI=dyn_cast<PtrToIntInst>(IPT->getOperand(0)))
            {
                Value *target = PTI->getOperand(0);
                if (Target2ArrayInfo.find(target) == Target2ArrayInfo.end())
                {
                    if (Alias2Target.find(target) != Alias2Target.end()) // it could be argument. We need to trace back to get its original array declaration
                    {
                        target = Alias2Target[target];
                    }
                    else
                    {
                        llvm::errs() << "ERRORS: cannot find target [" << *target << "] in Target2ArrayInfo and its address=" 
                                        << target << "\n";
                        assert(Target2ArrayInfo.find(target) != Target2ArrayInfo.end() 
                                    && Alias2Target.find(target) != Alias2Target.end()
                                    && "Fail to find the array inforamtion for the target.");
                    }
                }
                if (auto arg_pointer = dyn_cast<Argument>(target))
                {
                    AddressInst2AccessInfo[pointer_I->getOperand(0)] = getAccessInfoFor(target, Load_or_Store, 0, nullptr, nullptr);
                    if (DEBUG) *ArrayLog << " -----> access info with array index: " << AddressInst2AccessInfo[pointer_I->getOperand(0)] << "\n\n\n";
                    // ArrayLog->flush();
                }
                else if (auto alloc_pointer = dyn_cast<AllocaInst>(target))
                {
                    AddressInst2AccessInfo[pointer_I->getOperand(0)] = getAccessInfoFor(target, Load_or_Store, 0, nullptr, nullptr);
                    if (DEBUG) *ArrayLog << " -----> access info with array index: " << AddressInst2AccessInfo[pointer_I->getOperand(0)] << "\n\n\n";
                    // ArrayLog->flush();
                }
            }
        }
        else if (auto alloc_pointer = dyn_cast<AllocaInst>(pointer_I))
        {
            AddressInst2AccessInfo[pointer_I] = getAccessInfoFor(pointer_I, Load_or_Store, 0, nullptr, nullptr);
            if (DEBUG) *ArrayLog << " -----> access info with array index: " << AddressInst2AccessInfo[pointer_I] << "\n\n\n";
            if (DEBUG) ArrayLog->flush();
        }
    }
    
    return false;
}

void HI_WithDirectiveTimingResourceEvaluation::handleSAREAccess(Instruction *I, const SCEVAddRecExpr* SARE)
{
    if (SARE->isAffine())
    {

        int initial_const = -1;
        
        if (DEBUG) *ArrayLog << *I << " --> is add rec Affine Add: " << *SARE  << " it operand (0) " << *SARE->getOperand(0)  << " it operand (1) " << *SARE->getOperand(1) << "\n";
        if (DEBUG) *ArrayLog << " -----> intial offset expression: " << *findTheActualStartValue(SARE) <<"\n";
        
        
        std::vector<int> inc_indices, trip_counts;
        findTheIncrementalIndexAndTripCount(SARE, inc_indices, trip_counts);        
        if (DEBUG) *ArrayLog << " -----> inccremental value: ";
        if (DEBUG) for (auto inc_const_tmp : inc_indices) *ArrayLog << inc_const_tmp  << " ";
        if (DEBUG)  *ArrayLog << "\n";
        if (DEBUG) *ArrayLog << " -----> tripcount value: ";
        if (DEBUG) for (auto trip_count_tmp : trip_counts) *ArrayLog << trip_count_tmp  << " ";
        if (DEBUG)  *ArrayLog << "\n";
        // ArrayLog->flush();

        const SCEV *initial_expr_tmp = findTheActualStartValue(SARE);

        Value* target = nullptr;
        

        // the actual start value is (pointer + offset)
        if (auto initial_expr_add = dyn_cast<SCEVAddExpr>(initial_expr_tmp))
        {
            // find the constant in the SCEV and that will be the initial offset
            for (int i = 0; i<initial_expr_add->getNumOperands(); i++)
            {
                if (const SCEVConstant *start_V = dyn_cast<SCEVConstant>(initial_expr_add->getOperand(i)))
                {                 
                    initial_const = start_V->getAPInt().getSExtValue();
                    if (DEBUG) *ArrayLog << " -----> intial offset const: " << initial_const <<"\n";
                    // assert(initial_const >= 0 && "the initial offset should be found.\n");
                    // some time, using 2-complement will end with fake negative initial offset
                    if (initial_const<0)
                    {
                        llvm::errs() << " -----> intial offset const: " << initial_const <<"\n";
                        llvm::errs() << "    -----> (1<<getMinSignedBits)-1 " 
                                    << ((initial_const)&((1<<start_V->getAPInt().getMinSignedBits())-1)) 
                                    << " [" << start_V->getAPInt().getMinSignedBits() << "]" <<"\n";
                        llvm::errs() << "    -----> (1<<getActiveBits)-1 " 
                                    << ((initial_const)&((1<<start_V->getAPInt().getActiveBits())-1)) 
                                    << " [" << start_V->getAPInt().getActiveBits() << "]" <<"\n";
                        llvm::errs() << "    -----> (getZExtValue) " 
                                    << (((1<<start_V->getAPInt().getZExtValue())-1)) 
                                    << " bw=[" << start_V->getAPInt().getBitWidth() << "]" <<"\n";
                        initial_const = (initial_const)&((1<<start_V->getAPInt().getZExtValue())-1);
                    }
                
                    
             
                }
                else
                {
                    if (const SCEVUnknown* array_value_scev = dyn_cast<SCEVUnknown>(initial_expr_add->getOperand(i)))
                    {
                        if (DEBUG) *ArrayLog << " -----> access target: " << *array_value_scev->getValue() << "\n";
                        if (auto tmp_PTI_I = dyn_cast<PtrToIntInst>(array_value_scev->getValue()))
                        {
                            target = tmp_PTI_I->getOperand(0);
                        }
                        else
                        {
                            assert(target && "There should be an PtrToInt Instruction for the addition operation.\n");
                        }

                        if (Target2ArrayInfo.find(target) == Target2ArrayInfo.end())
                        {
                            if (Alias2Target.find(target) != Alias2Target.end()) // it could be argument. We need to trace back to get its original array declaration
                            {
                                target = Alias2Target[target];
                            }
                            else
                            {
                                llvm::errs() << "ERRORS: cannot find target [" << *target << "] in Target2ArrayInfo and its address=" 
                                                << target << "\n";
                                assert(Target2ArrayInfo.find(target) != Target2ArrayInfo.end() 
                                            && Alias2Target.find(target) != Alias2Target.end()
                                            && "Fail to find the array inforamtion for the target.");
                            }
                        }

                        
                        if (DEBUG) *ArrayLog << " -----> access target info: " << Target2ArrayInfo[target] << "\n";         
                        // ArrayLog->flush();               
                    }
                    else
                    {
                        assert(false && "The access target should be found.\n");
                    }
                }       
            }    
        }

        // the actual start value is (pointer), when the offset is zero
        else if (auto initial_expr_unknown = dyn_cast<SCEVUnknown>(initial_expr_tmp))
        {

            initial_const = 0;
            if (DEBUG) *ArrayLog << " -----> intial offset const: " << initial_const <<"\n";
            if (DEBUG) *ArrayLog << " -----> access target: " << *initial_expr_unknown->getValue() << "\n";
            if (auto tmp_PTI_I = dyn_cast<PtrToIntInst>(initial_expr_unknown->getValue()))
            {
                target = tmp_PTI_I->getOperand(0);
            }
            else
            {
                assert(target && "There should be an PtrToInt Instruction for the addition operation.\n");
            }

            if (Target2ArrayInfo.find(target) == Target2ArrayInfo.end())
            {
                if (Alias2Target.find(target) != Alias2Target.end()) // it could be argument. We need to trace back to get its original array declaration
                {
                    target = Alias2Target[target];
                }
                else
                {
                    llvm::errs() << "ERRORS: cannot find target [" << *target << "] in Target2ArrayInfo and its address=" 
                                    << target << "\n";
                    assert(Target2ArrayInfo.find(target) != Target2ArrayInfo.end() 
                                && Alias2Target.find(target) != Alias2Target.end()
                                && "Fail to find the array inforamtion for the target.");
                }
            }

            if (DEBUG) *ArrayLog << " -----> access target info: " << Target2ArrayInfo[target] << "\n";         
            // ArrayLog->flush();     
        }
        else
        {
            assert(false && "Shound not reach here.");
        }


        // assert(initial_const >= 0 && "the initial offset should be found.\n");
        // some time, using 2-complement will end with fake negative initial offset

        assert(target && "the target array should be found.\n");
        AddressInst2AccessInfo[I] = getAccessInfoFor(target, I, initial_const, &inc_indices, &trip_counts);
        if (DEBUG) *ArrayLog << " -----> access info with array index: " << AddressInst2AccessInfo[I] << "\n\n\n";
        // ArrayLog->flush();
    }
}


void HI_WithDirectiveTimingResourceEvaluation::handleDirectAccess(Instruction *I, const SCEVUnknown* SUnkown)
{

    int initial_const = 0;
    
    if (DEBUG) *ArrayLog << *I << " --> is unkown " << *SUnkown <<"\n";
        
    Value* target = nullptr;

    auto PTI = dyn_cast<PtrToIntInst>(SUnkown->getValue());

    assert(PTI);

    target = PTI->getOperand(0);
    
    if (Alias2Target.find(target) != Alias2Target.end()) // it could be argument. We need to trace back to get its original array declaration
    {
        target = Alias2Target[target];
    }

    assert(initial_const >= 0 && "the initial offset should be found.\n");
    assert(target && "the target array should be found.\n");
    AddressInst2AccessInfo[I] = getAccessInfoFor(target, I, initial_const, nullptr, nullptr);
    if (DEBUG) *ArrayLog << " -----> access info with array index: " << AddressInst2AccessInfo[I] << "\n\n\n";
    // ArrayLog->flush();
    
}



void HI_WithDirectiveTimingResourceEvaluation::handleConstantOffsetAccess(Instruction *I, const SCEVAddExpr* SAE)
{
    auto constOffset = dyn_cast<SCEVConstant>(SAE->getOperand(0));
    auto ptrUnknown = dyn_cast<SCEVUnknown>(SAE->getOperand(1));

    int initial_const = constOffset->getAPInt().getSExtValue();
    
    if (DEBUG) *ArrayLog << *I << " --> is unkown " << *ptrUnknown <<"\n";
       
    if (DEBUG) *ArrayLog << *I << " --> has constant offset " << initial_const <<"\n";

    Value* target = nullptr;

    auto PTI = dyn_cast<PtrToIntInst>(ptrUnknown->getValue());

    assert(PTI);

    target = PTI->getOperand(0);
    
    if (Alias2Target.find(target) != Alias2Target.end()) // it could be argument. We need to trace back to get its original array declaration
    {
        target = Alias2Target[target];
    }

    assert(initial_const >= 0 && "the initial offset should be found.\n");
    assert(target && "the target array should be found.\n");
    AddressInst2AccessInfo[I] = getAccessInfoFor(target, I, initial_const, nullptr, nullptr);
    if (DEBUG) *ArrayLog << " -----> access info with array index: " << AddressInst2AccessInfo[I] << "\n\n\n";
    // ArrayLog->flush();
    
}




// handle non-standard SARE, where the pointer value is in the outermost expression, 
// and extract array access information from it
void HI_WithDirectiveTimingResourceEvaluation::handleUnstandardSCEVAccess(Instruction *I, const SCEV* tmp_S)
{
    const SCEVAddExpr *SAE = dyn_cast<SCEVAddExpr>(bypassExtTruntSCEV(tmp_S));
    const SCEVAddRecExpr *SARE = dyn_cast<SCEVAddRecExpr>(bypassExtTruntSCEV(SAE->getOperand(0)));
    const SCEVUnknown *SU = dyn_cast<SCEVUnknown>(findUnknown(tmp_S));
    const PtrToIntInst *PTI_I = dyn_cast<PtrToIntInst>(SU->getValue());

    if (SARE->isAffine())
    {

        int initial_const = -1;
        
        if (DEBUG) *ArrayLog << *I << " --> is add rec Affine Add: " << *SARE  << " it operand (0) " << *SARE->getOperand(0)  << " it operand (1) " << *SARE->getOperand(1) << "\n";
        if (DEBUG) *ArrayLog << " -----> intial offset expression: " << *findTheActualStartValue(SARE) <<"\n";
        
        std::vector<int> inc_indices, trip_counts;
        findTheIncrementalIndexAndTripCount(SARE, inc_indices, trip_counts);        
        if (DEBUG) *ArrayLog << " -----> inccremental value: ";
        if (DEBUG) for (auto inc_const_tmp : inc_indices) *ArrayLog << inc_const_tmp  << " ";
        if (DEBUG)  *ArrayLog << "\n";
        if (DEBUG) *ArrayLog << " -----> tripcount value: ";
        if (DEBUG) for (auto trip_count_tmp : trip_counts) *ArrayLog << trip_count_tmp  << " ";
        if (DEBUG)  *ArrayLog << "\n";
        // ArrayLog->flush();

        const SCEV *initial_expr_tmp = findTheActualStartValue(SARE);

        Value* target = nullptr;
        
        
        // the actual start value is (pointer + offset)
        if (auto initial_const_scev = dyn_cast<SCEVConstant>(initial_expr_tmp))
        {
            // find the constant in the SCEV and that will be the initial offset
            initial_const = initial_const_scev->getAPInt().getSExtValue();
            if (DEBUG) *ArrayLog << " -----> intial offset const: " << initial_const <<"\n";
            // ArrayLog->flush();

            // assert(initial_const >= 0 && "the initial offset should be found.\n");
            // some time, using 2-complement will end with fake negative initial offset
            if (initial_const<0)
            {
                llvm::errs() << " -----> intial offset const: " << initial_const <<"\n";
                llvm::errs() << "    -----> (1<<getMinSignedBits)-1 " 
                             << ((initial_const)&((1<<initial_const_scev->getAPInt().getMinSignedBits())-1)) 
                             << " [" << initial_const_scev->getAPInt().getMinSignedBits() << "]" <<"\n";
                llvm::errs() << "    -----> (1<<getActiveBits)-1 " 
                             << ((initial_const)&((1<<initial_const_scev->getAPInt().getActiveBits())-1)) 
                             << " [" << initial_const_scev->getAPInt().getActiveBits() << "]" <<"\n";
                llvm::errs() << "    -----> (getZExtValue) " 
                             << (((1<<initial_const_scev->getAPInt().getZExtValue())-1)) 
                             << " bw=[" << initial_const_scev->getAPInt().getBitWidth() << "]" <<"\n";
                initial_const = (initial_const)&((1<<initial_const_scev->getAPInt().getZExtValue())-1);
            }
                
                    
             

            if (const SCEVUnknown* array_value_scev = dyn_cast<SCEVUnknown>(SU))
            {
                if (DEBUG) *ArrayLog << " -----> access target: " << *array_value_scev->getValue() << "\n";
                if (auto tmp_PTI_I = dyn_cast<PtrToIntInst>(array_value_scev->getValue()))
                {
                    target = tmp_PTI_I->getOperand(0);
                }
                else
                {
                    assert(target && "There should be an PtrToInt Instruction for the addition operation.\n");
                }

                if (Target2ArrayInfo.find(target) == Target2ArrayInfo.end())
                {
                    if (Alias2Target.find(target) != Alias2Target.end()) // it could be argument. We need to trace back to get its original array declaration
                    {
                        target = Alias2Target[target];
                    }
                    else
                    {
                        llvm::errs() << "ERRORS: cannot find target [" << *target << "] in Target2ArrayInfo and its address=" 
                                        << target << "\n";
                        assert(Target2ArrayInfo.find(target) != Target2ArrayInfo.end() 
                                    && Alias2Target.find(target) != Alias2Target.end()
                                    && "Fail to find the array inforamtion for the target.");
                    }
                }

      
                // ArrayLog->flush();               
            }
            else
            {
                assert(false && "The access target should be found.\n");
            }

        }
        else
        {
            assert(false && "according to the pattern, the analysis should not reach here.");
        }

        // assert(initial_const >= 0 && "the initial offset should be found.\n");
        
        assert(target && "the target array should be found.\n");
        if (DEBUG) *ArrayLog << " -----> access target info: " << Target2ArrayInfo[target] << "\n";  
        AddressInst2AccessInfo[I] = getAccessInfoFor(target, I, initial_const, &inc_indices, &trip_counts);
        if (DEBUG) *ArrayLog << " -----> access info with array index: " << AddressInst2AccessInfo[I] << "\n\n\n";
        // ArrayLog->flush();
    }
}

// handle complex SCEV, based on which we cannot predict the array access information
void HI_WithDirectiveTimingResourceEvaluation::handleComplexSCEVAccess(Instruction *I, const SCEV* tmp_S)
{
    auto ITP_I = dyn_cast<IntToPtrInst>(I->use_begin()->getUser());

    if (!ITP_I)
        return;

    if (DEBUG) *ArrayLog << *I << " --> is under processing of ArrayAccessProcess\n";    
    // ArrayLog->flush();

    Instruction* sample_accessI = dyn_cast<Instruction>(ITP_I->use_begin()->getUser());

    if (DEBUG) *ArrayLog << *I << " --> is a complex SCEV. We need special processing for it.\n";
    
    Value* target = getTargetFromInst(sample_accessI);
    if (DEBUG) *ArrayLog << *I << " --> target array: " << *target << "\n";
    if (Target2ArrayInfo.find(target) == Target2ArrayInfo.end())
    {
        if (Alias2Target.find(target) != Alias2Target.end()) // it could be argument. We need to trace back to get its original array declaration
        {
            target = Alias2Target[target];
        }
        else
        {
            llvm::errs() << "ERRORS: cannot find target [" << *target << "] in Target2ArrayInfo and its address=" 
                            << target << "\n";
            assert(Target2ArrayInfo.find(target) != Target2ArrayInfo.end() 
                        && Alias2Target.find(target) != Alias2Target.end()
                        && "Fail to find the array inforamtion for the target.");
        }
    }
    
    assert(target && "the target array should be found.\n");
    AddressInst2AccessInfo[I] = getAccessInfoFor(target, I, -1, nullptr, nullptr, /*unpredictable*/true);
    if (DEBUG) *ArrayLog << " -----> access info with array index: " << AddressInst2AccessInfo[I] << "\n\n\n";
    // ArrayLog->flush();
}

bool std::operator<(const HI_WithDirectiveTimingResourceEvaluation::partition_info &A, const HI_WithDirectiveTimingResourceEvaluation::partition_info &B)
{
    if (A.target < B.target)
        return true;
    if (A.target > B.target)
        return false;
    for (int i=0;i<A.num_dims;i++)
        if (A.partition_id[i] < B.partition_id[i])
            return true;
        else if (A.partition_id[i] > B.partition_id[i])
            return false;
    return false;
}


// according to the initial offset, trip counts of loop, and incremental per iteration
// generate a list of possible offset for a specific access. The list will be used for
// partition check
std::vector<int> HI_WithDirectiveTimingResourceEvaluation::generatePotentialOffset(HI_AccessInfo &accessInfo)
{
    std::vector<int> res_offsets;
    if (accessInfo.reverse_loop_dep > 0)
        getAllPotentialOffsetByRecuresiveSearch(accessInfo, accessInfo.reverse_loop_dep, accessInfo.initial_offset, res_offsets);
    else
        res_offsets.push_back(accessInfo.initial_offset); // offset unrelated to loop
    return res_offsets;
}

// recursively emulate all the loops where the access is inside
// to check the offset of the access
void HI_WithDirectiveTimingResourceEvaluation::getAllPotentialOffsetByRecuresiveSearch(HI_AccessInfo &accessInfo, int loopDep, int last_level_offset, std::vector<int> &res)
{
    if (loopDep==-1)
    {
        // the access is out of loop or not ralated to loop 
        res.push_back(last_level_offset);
        return;
    }
    if (loopDep>1)
    {
        for (int i = 0; i < accessInfo.trip_count[loopDep-1]; i++)
        {
            getAllPotentialOffsetByRecuresiveSearch(accessInfo, loopDep-1, last_level_offset + i*accessInfo.inc_index[loopDep-1], res);
        }
    }
    else
    {
        for (int i = 0; i < accessInfo.trip_count[loopDep-1]; i++)
        {
            int res_offset = last_level_offset + i*accessInfo.inc_index[loopDep-1];
            if (res_offset < accessInfo.dim_size[accessInfo.num_dims-1]*accessInfo.sub_element_num[accessInfo.num_dims-1])
                res.push_back(res_offset);
        }
    }
}

// get all the partitions for the access target of the access instruction
std::vector<HI_WithDirectiveTimingResourceEvaluation::partition_info> HI_WithDirectiveTimingResourceEvaluation::getAllPartitionFor(Instruction *access)
{
    HI_AccessInfo tmp_info = getAccessInfoForAccessInst(access);
    std::vector<partition_info> res;
    std::vector<int> tmp_partID = {-1,-1,-1,-1,-1,-1,-1,-1};
    getAllPartitionBasedOnInfo(tmp_info, 0, tmp_partID, res);
    return res;
}

// get all the partitions for the access target of the access instruction
void HI_WithDirectiveTimingResourceEvaluation::getAllPartitionBasedOnInfo(HI_AccessInfo &info, int curDim, std::vector<int> &tmp_partID, std::vector<partition_info> &res)
{
    if (curDim == info.num_dims)
    {
        partition_info tmp_part_bank;
        tmp_part_bank.num_dims = info.num_dims;
        tmp_part_bank.port_num = info.port_num;
        Value *reftarget = info.target;
        if (Alias2Target.find(reftarget) != Alias2Target.end())
            reftarget = Alias2Target[reftarget];
        tmp_part_bank.target = reftarget;
        for (int i=0;i<info.num_dims;i++)
            tmp_part_bank.partition_id[i] = tmp_partID[i];
        res.push_back(tmp_part_bank);
    }
    else
    {
        for (int i=0;i<info.partition_size[curDim];i++)
        {
            tmp_partID[curDim] = i;
            getAllPartitionBasedOnInfo(info, curDim + 1, tmp_partID, res);
        }
    }
}




Optional<APInt> HI_WithDirectiveTimingResourceEvaluation::computeConstantDifference(const SCEV *ori_More,
                                                           const SCEV *ori_Less) 
{
  // We avoid subtracting expressions here because this function is usually
  // fairly deep in the call stack (i.e. is called many times).

  const SCEV *More = bypassExtTruntSCEV(ori_More);
  const SCEV *Less = bypassExtTruntSCEV(ori_Less);
  if (isa<SCEVAddExpr>(More) && isa<SCEVAddExpr>(Less))
  {
        const auto *LA = cast<SCEVAddExpr>(Less);
        const auto *MA = cast<SCEVAddExpr>(More);
        const auto *LU = dyn_cast<SCEVUnknown>(LA->getOperand(1));
        const auto *MU = dyn_cast<SCEVUnknown>(MA->getOperand(1));
        if (LU && MU)
        {
            if ( LU->getValue() == LU->getValue() )
            {
                //*RemoveRedundantAccess_Log << "----- downgrade " << *nextI << " but no possibility of alias\n";
                return computeConstantDifference(MA->getOperand(0), LA->getOperand(0));
            }
        }
  }

  while (isa<SCEVAddRecExpr>(Less) && isa<SCEVAddRecExpr>(More)) {
    const auto *LAR = cast<SCEVAddRecExpr>(Less);
    const auto *MAR = cast<SCEVAddRecExpr>(More);

    if (LAR->getLoop() != MAR->getLoop())
      return None;

    // We look at affine expressions only; not for correctness but to keep
    // getStepRecurrence cheap.
    if (!LAR->isAffine() || !MAR->isAffine())
      return None;

    if (LAR->getStepRecurrence(*SE) != MAR->getStepRecurrence(*SE))
      return None;

    Less = LAR->getStart();
    More = MAR->getStart();
    More = bypassExtTruntSCEV(More);
    Less = bypassExtTruntSCEV(Less);
    // fall through
  }

  if (isa<SCEVConstant>(Less) && isa<SCEVConstant>(More)) {
    const auto &M = cast<SCEVConstant>(More)->getAPInt();
    const auto &L = cast<SCEVConstant>(Less)->getAPInt();
    return M - L;
  }

  SCEV::NoWrapFlags Flags;
  const SCEV *LLess = nullptr, *RLess = nullptr;
  const SCEV *LMore = nullptr, *RMore = nullptr;
  const SCEVConstant *C1 = nullptr, *C2 = nullptr;
  // Compare (X + C1) vs X.
  if (splitBinaryAdd(Less, LLess, RLess, Flags))
    if ((C1 = dyn_cast<SCEVConstant>(LLess)))
      if (RLess == More)
        return -(C1->getAPInt());

  // Compare X vs (X + C2).
  if (splitBinaryAdd(More, LMore, RMore, Flags))
    if ((C2 = dyn_cast<SCEVConstant>(LMore)))
      if (RMore == Less)
        return C2->getAPInt();

  // Compare (X + C1) vs (X + C2).
  if (C1 && C2 && RLess == RMore)
    return C2->getAPInt() - C1->getAPInt();

  return None;
}



// some accesses with offset which is not related to the loop of current level
bool HI_WithDirectiveTimingResourceEvaluation::checkConstantAccessInLoop(const SCEV *ori_More,
                                                           const SCEV *ori_Less, Loop* curLoop) 
{
  // We avoid subtracting expressions here because this function is usually
  // fairly deep in the call stack (i.e. is called many times).
  if (DEBUG)
  {
    *ArrayLog << "checkConstantAccessInLoop: ori_More=" << *ori_More 
              << "   ori_Less=" << *ori_Less << " curLoop:" << curLoop->getName() << "\n";
    ArrayLog->flush();
  }
              
  const SCEV *More = bypassExtTruntSCEV(ori_More);
  const SCEV *Less = bypassExtTruntSCEV(ori_Less);
  if (isa<SCEVAddExpr>(More) && isa<SCEVAddExpr>(Less))
  {
        const auto *LA = cast<SCEVAddExpr>(Less);
        const auto *MA = cast<SCEVAddExpr>(More);
        const auto *LU = dyn_cast<SCEVUnknown>(LA->getOperand(1));
        const auto *MU = dyn_cast<SCEVUnknown>(MA->getOperand(1));
        if (LU && MU)
        {
            if ( LU->getValue() == LU->getValue() )
            {
                //*RemoveRedundantAccess_Log << "----- downgrade " << *nextI << " but no possibility of alias\n";
                return checkConstantAccessInLoop(MA->getOperand(0), LA->getOperand(0), curLoop);
            }
        }
  }

  if (!(isa<SCEVAddRecExpr>(Less) && isa<SCEVAddRecExpr>(More)))
  {
      llvm::errs() << "Less=" << *Less << "\n";
      llvm::errs() << "More=" << *More << "\n";
  }

  assert(isa<SCEVAddRecExpr>(Less) && isa<SCEVAddRecExpr>(More));
    const auto *LAR = cast<SCEVAddRecExpr>(Less);
    const auto *MAR = cast<SCEVAddRecExpr>(More);

    *ArrayLog << "checkConstantAccessInLoop: LAR->getLoop()!=curLoop && MAR->getLoop()!=curLoop ==>" <<(LAR->getLoop()!=curLoop && MAR->getLoop()!=curLoop) << "\n";
    ArrayLog->flush();

  return (LAR->getLoop()!=curLoop && MAR->getLoop()!=curLoop );
  

}


// get the step length (stride) of the access
int HI_WithDirectiveTimingResourceEvaluation::getStepLength(const SCEV *ori_More,
                                                            const SCEV *ori_Less) 
{
    // We avoid subtracting expressions here because this function is usually
    // fairly deep in the call stack (i.e. is called many times).
    const SCEV *More = bypassExtTruntSCEV(ori_More);
    const SCEV *Less = bypassExtTruntSCEV(ori_Less);
    if (isa<SCEVAddExpr>(More) && isa<SCEVAddExpr>(Less))
    {
            const auto *LA = cast<SCEVAddExpr>(Less);
            const auto *MA = cast<SCEVAddExpr>(More);
            const auto *LU = dyn_cast<SCEVUnknown>(LA->getOperand(1));
            const auto *MU = dyn_cast<SCEVUnknown>(MA->getOperand(1));
            if (LU && MU)
            {
                if ( LU->getValue() == LU->getValue() )
                {
                    //*RemoveRedundantAccess_Log << "----- downgrade " << *nextI << " but no possibility of alias\n";
                    return getStepLength(MA->getOperand(0), LA->getOperand(0));
                }
            }
    }


    assert (isa<SCEVAddRecExpr>(Less) && isa<SCEVAddRecExpr>(More));

    const auto *LAR = cast<SCEVAddRecExpr>(Less);
    const auto *MAR = cast<SCEVAddRecExpr>(More);

    assert(LAR->getLoop() == MAR->getLoop());
    assert(LAR->isAffine() && MAR->isAffine());
    assert(LAR->getStepRecurrence(*SE) == MAR->getStepRecurrence(*SE));
    const SCEVConstant *C1 = nullptr, *C2 = nullptr;
    C1 = dyn_cast<SCEVConstant>(LAR->getStepRecurrence(*SE));
    C2 = dyn_cast<SCEVConstant>(MAR->getStepRecurrence(*SE));
    assert(C1 && C2); // we only consider the situation where the step is constant

    return C1->getAPInt().getSExtValue();

}

bool HI_WithDirectiveTimingResourceEvaluation::splitBinaryAdd(const SCEV *Expr,
                                     const SCEV *&L, const SCEV *&R,
                                     SCEV::NoWrapFlags &Flags) {
  const auto *AE = dyn_cast<SCEVAddExpr>(Expr);
  if (!AE || AE->getNumOperands() != 2)
    return false;

  L = AE->getOperand(0);
  R = AE->getOperand(1);
  Flags = AE->getNoWrapFlags();
  return true;
}

// check whether the two instructions could be alias instructions
// true if it is not possible, false if it is possible
bool HI_WithDirectiveTimingResourceEvaluation::noAliasHazard(Instruction *I0, Instruction *I1)
{
    Instruction *pointer_I0 = nullptr, *pointer_I1 = nullptr;
    if (I0->getOpcode()==Instruction::Load)
    {
        pointer_I0 = dyn_cast<IntToPtrInst>(I0->getOperand(0));
    }
    else if (I0->getOpcode()==Instruction::Store)
    {
        pointer_I0 = dyn_cast<IntToPtrInst>(I0->getOperand(1));
    }

    if (I1->getOpcode()==Instruction::Load)
    {
        pointer_I1 = dyn_cast<IntToPtrInst>(I1->getOperand(0));
    }
    else if (I1->getOpcode()==Instruction::Store)
    {
        pointer_I1 = dyn_cast<IntToPtrInst>(I1->getOperand(1));
    }


    if (!pointer_I0 ^ !pointer_I1)
    {
        return false;
    }
    else
    {
        return pointer_I0!=pointer_I1;
    }
    

    // assert(pointer_I0 && pointer_I0->getOpcode() == Instruction::IntToPtr && "ITP should be found for the access instruction");
    // assert(pointer_I1 && pointer_I1->getOpcode() == Instruction::IntToPtr && "ITP should be found for the access instruction");

    std::string tmp0(""),tmp1("");
    raw_string_ostream *SCEV_Stream0 = new raw_string_ostream(tmp0);
    raw_string_ostream *SCEV_Stream1 = new raw_string_ostream(tmp1);
    
    const SCEV *tmp_S0 = SE->getSCEV(pointer_I0->getOperand(0));
    const SCEV *tmp_S1 = SE->getSCEV(pointer_I1->getOperand(0));

    Optional<APInt> res=computeConstantDifference(tmp_S0,tmp_S1);
    
    if (res != None)
    {
        
        if (res.getValue().getSExtValue()==0)
        {
            
            return false;
        }
        
        return true;
    }
    return false;
}


// check whether the array partitioning helps for parallelism
void HI_WithDirectiveTimingResourceEvaluation::checkPartitionBenefit(std::vector<HI_WithDirectiveTimingResourceEvaluation::partition_info> &partitions, Instruction* accessI)
{
    int num_potential_target_partitions = partitions.size();

    Value* target_array = getTargetFromInst(accessI);

    HI_ArrayInfo targetInfo = Target2ArrayInfo[target_array];

    int num_partitions = 1; 
    for (int i=0;i<targetInfo.num_dims;i++) 
        num_partitions *= targetInfo.partition_size[i];

    // if for some accesses, not all the partitions are potential partitions, 
    // array partitioning will help to improve parallelism
    if (num_partitions!=num_potential_target_partitions)
    {
        Target2PartitionBenefit[target_array] = true;
        Function *F = getFunctionOfValue(target_array);
        std::string nameOfFunction = demangleFunctionName(F->getName());
        assert(F && "the parent function of the value should be found.");
        FuncArray2PartitionBenefit[std::pair<std::string, std::string>(nameOfFunction,target_array->getName() )] = true;
        if (DEBUG)
        {
            *ArrayLog << "INFO: partitioning helps to improve the parallelism for the accesses: " << *accessI 
                      << "(target=" << *target_array << ")\n\n";
        }
    }
}

// Calculate the BRAM cost (BRAM+MUX LUT) for the top function
HI_WithDirectiveTimingResourceEvaluation::resourceBase HI_WithDirectiveTimingResourceEvaluation::BRAMRelatedCostForTopFunction(Function &F)
{
    resourceBase BRAM_related_cost_for_TopFunc(0,0,0,0,clock_period);

    resourceBase BRAM_MUX = BRAM_MUX_Evaluate();            

    if (DEBUG) *Evaluating_log << "total mux for BRAM: "<< BRAM_MUX << "\n";

    if (DEBUG) *Evaluating_log << " is Top function, check cost of BRAMs for topfunction interface" << "\n";

    bool dataflowEnable = isFunctionDataflow(&F);
     
    for (auto it = F.arg_begin(),ie = F.arg_end(); it!=ie; ++it)
    {
        if (it->getType()->isPointerTy())
        {
            PointerType *tmp_PtrType = dyn_cast<PointerType>(it->getType());
            if (tmp_PtrType->getElementType()->isArrayTy())
            {
                if (DEBUG) *Evaluating_log << "  get array information of [" << it->getName() << "] from argument and its address=" << it << "\n";
                if (isLocalArray(it)||!dataflowEnable) // if the top function is set to be dataflow, the BRAM cost of interface might have been calculated.
                {
                    auto bramCost = get_BRAM_Num_For(it);
                    BRAM_related_cost_for_TopFunc = BRAM_related_cost_for_TopFunc + bramCost;
                    if (DEBUG) *Evaluating_log << " the BRAMs cost by this argument interfact is " << bramCost << "\n";
                }
                else
                {
                    if (DEBUG) *Evaluating_log << " the BRAMs cost has been calculated in dataflow analyzis.\n";
                }       
            }
            else if (tmp_PtrType->getElementType()->isIntegerTy() || tmp_PtrType->getElementType()->isFloatingPointTy() ||tmp_PtrType->getElementType()->isDoubleTy() )
            {
                if (DEBUG) *Evaluating_log << "  get array information of [" << it->getName() << "] from argument and its address=" << it << "\n";
                if (isLocalArray(it)||!dataflowEnable) // if the top function is set to be dataflow, the BRAM cost of interface might have been calculated.
                {
                    auto bramCost = get_BRAM_Num_For(it);
                    BRAM_related_cost_for_TopFunc = BRAM_related_cost_for_TopFunc + bramCost;
                    if (DEBUG) *Evaluating_log << " the BRAMs cost by this argument interfact is " << bramCost << "\n";
                }
                else
                {
                    if (DEBUG) *Evaluating_log << " the BRAMs cost has been calculated in dataflow analyzis.\n";
                }    
            }
        }
    }

    if (DEBUG) *Evaluating_log << " Compuation LUT: " << FunctionResource[&F].LUT 
                                << "\n BRAM_MUX LUT: " << BRAM_MUX.LUT << "\n";

    BRAM_related_cost_for_TopFunc = BRAM_related_cost_for_TopFunc +  BRAM_MUX ;

    return  BRAM_related_cost_for_TopFunc;
}


// check whether the target is set to be local array
bool HI_WithDirectiveTimingResourceEvaluation::isLocalArray(Value *target)
{
    std::string functionDemangleName;
    if (auto allocI = dyn_cast<AllocaInst>(target))
        functionDemangleName = demangleFunctionName(allocI->getParent()->getParent()->getName());
    else if (auto argV = dyn_cast<Argument>(target))
        functionDemangleName = demangleFunctionName(argV->getParent()->getName());
    else
        assert(false && "should not reach here.\n");
    


    bool localArrayEnable = false;
    for (auto local_array_pair : configInfo.localArrayConfigs)
    {
        if (local_array_pair.first == functionDemangleName)
        {
            if (local_array_pair.second.first == target->getName())
            {
                localArrayEnable = local_array_pair.second.second;
                break;
            }
        }
    }
    return localArrayEnable;
        
}

// check the access property of the target arrays in the function
void HI_WithDirectiveTimingResourceEvaluation::accessPropertyAnalysis_Function(Function* F)
{
    for (auto &B : *F)
    {
        for (auto &I : B)
        {
            if (auto loadI = dyn_cast<LoadInst>(&I))
            {
                Value* target = getTargetFromInst(loadI);
                std::pair<Value*, Function*> val_func_pair(target, F);
                updateAccessProperty_Function(val_func_pair, ReadOnlyInFunction);
            }
            else if (auto storeI = dyn_cast<StoreInst>(&I))
            {
                Value* target = getTargetFromInst(storeI);
                std::pair<Value*, Function*> val_func_pair(target, F);
                updateAccessProperty_Function(val_func_pair, WriteOnlyInFunction);
            }
            else if (auto callI = dyn_cast<CallInst>(&I))
            {
                Function* calledF = callI->getCalledFunction();
                if (calledF->getName().find("llvm.")!=std::string::npos || calledF->getName().find("HIPartitionMux")!=std::string::npos) // bypass the "llvm.xxx" functions..
                    continue;
                std::vector<Value*> targets = getTargetsFromInst(callI);
                for (auto target : targets)
                {
                    std::pair<Value*, Function*> val_func_pair(target, F);
                    
                    std::pair<Value*, Function*> val_calledfunc_pair(target, calledF);
                    if (targetAccessPropertyInFunction.find(val_calledfunc_pair) != targetAccessPropertyInFunction.end())
                    {
                        updateAccessProperty_Function(val_func_pair, targetAccessPropertyInFunction[val_calledfunc_pair]);
                    }
                }

            }
        }
    }

    if (DEBUG)
    {
        for (auto val_func_prop : targetAccessPropertyInFunction)
        {
            if (val_func_prop.first.second == F)
            {
                if (val_func_prop.second == ReadOnlyInFunction)
                    *ArrayLog << "  In Function [" << F->getName() << "], the array [" << val_func_prop.first.first->getName() << "] is readOnly\n";
                if (val_func_prop.second == WriteOnlyInFunction)
                    *ArrayLog << "  In Function [" << F->getName() << "], the array [" << val_func_prop.first.first->getName() << "] is writeOnly\n";
                if (val_func_prop.second == ReadWriteInFunction)
                    *ArrayLog << "  In Function [" << F->getName() << "], the array [" << val_func_prop.first.first->getName() << "] is readAndWrite\n";
            }
        }
    }

}


// update access property of the targets 
void HI_WithDirectiveTimingResourceEvaluation::updateAccessProperty_Function(std::pair<Value*, Function*> val_func_pair, HI_WithDirectiveTimingResourceEvaluation::accessPropertyType updateTy)
{
    if (targetAccessPropertyInFunction.find(val_func_pair) == targetAccessPropertyInFunction.end())
    {
        targetAccessPropertyInFunction[val_func_pair] = updateTy;
    }
    else if (targetAccessPropertyInFunction[val_func_pair] == ReadOnlyInFunction)
    {
        if (updateTy == WriteOnlyInFunction || updateTy == ReadWriteInFunction)
        {
            targetAccessPropertyInFunction[val_func_pair] = ReadWriteInFunction;
        }
    }
    else if (targetAccessPropertyInFunction[val_func_pair] == WriteOnlyInFunction)
    {
        if (updateTy == ReadOnlyInFunction || updateTy == ReadWriteInFunction)
        {
            targetAccessPropertyInFunction[val_func_pair] = ReadWriteInFunction;
        }
    }
}