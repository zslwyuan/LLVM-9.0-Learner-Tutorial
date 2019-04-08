#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Pass.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/raw_ostream.h"
#include "HI_print.h"
#include "HI_NoDirectiveTimingResourceEvaluation.h"
#include "polly/PolyhedralInfo.h"

#include <stdio.h>
#include <string>
#include <ios>
#include <stdlib.h>
#include <sstream>
using namespace llvm;

void HI_NoDirectiveTimingResourceEvaluation::findMemoryDeclarationin(Function *F, bool isTopFunction)
{
    *BRAM_log << "checking the BRAM information in Function: " << F->getName() << "\n";
    ValueVisited.clear();
    scheduledAccess_timing.clear();
    if (isTopFunction)
    {
        *BRAM_log << " is Top function " << "\n";
        for (auto it = F->arg_begin(),ie = F->arg_end(); it!=ie; ++it)
        {
            if (it->getType()->isPointerTy())
            {
                PointerType *tmp_PtrType = dyn_cast<PointerType>(it->getType());
                if (tmp_PtrType->getElementType()->isArrayTy())
                {
                    TraceAccessForTarget(it,it);
                }
            }
        }
    }
    else
    {
         *BRAM_log << " is not function " << "\n";
         return;
    }
    
    for (auto &B: *F)
    {
        for (auto &I: B)
        {
            if (AllocaInst *allocI = dyn_cast<AllocaInst>(&I))
            {
                TraceAccessForTarget(allocI,allocI);
            }
        }
    }
    *BRAM_log << "\n\n\n---------------- Access to Target ---------------" << "\n";
    for (auto it : Access2TargetMap)
    {
        *BRAM_log << "The instruction [" << *it.first << "] will access :";
        for (auto tmpI : it.second)
        {
            *BRAM_log << " (" << tmpI->getName() << ") ";
        }
        *BRAM_log << "\n";
    }
    *BRAM_log << "-------------------------------------------------" << "\n\n\n\n";
    BRAM_log->flush();
}

void HI_NoDirectiveTimingResourceEvaluation::TraceAccessForTarget(Value *cur_node, Value *ori_node)
{
    *BRAM_log << "\n\n\nTracing the access to Array " << ori_node->getName() << " and looking for the users of " << *cur_node<< "\n";
    if (Instruction* tmpI = dyn_cast<Instruction>(cur_node))
    {
        *BRAM_log << "    --- is an instruction:\n";
    }
    else
    {
        *BRAM_log << "    --- is not an instruction \n";
    }
    
    BRAM_log->flush();
    Function *cur_F;
    if (ValueVisited.find(cur_node)!=ValueVisited.end())
        return;
    ValueVisited.insert(cur_node);

    if (Argument *arg = dyn_cast<Argument>(cur_node))
    {            
        cur_F = arg->getParent();
    }
    else
    {
        if (Instruction *InstTmp = dyn_cast<Instruction>(cur_node))
        {
            cur_F = InstTmp->getParent()->getParent();
        }
        else
        {
            assert(false && "The parent function should be found.\n");
        }
    }
    for (auto it = cur_node->use_begin(),ie = cur_node->use_end(); it != ie; ++it)
    {
        *BRAM_log << "    find user of " << ori_node->getName() << " --> " << *it->getUser() <<  "\n";
        if (LoadInst *LoadI = dyn_cast<LoadInst>(it->getUser()))
        {
            if (Access2TargetMap.find(LoadI)==Access2TargetMap.end())
            {
                *BRAM_log << "    is an LOAD instruction: " << *LoadI << "\n";
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
                *BRAM_log << "    is an STORE instruction: " << *StoreI << "\n";
                std::vector<Value*> tmp_vec; 
                tmp_vec.push_back(ori_node);
                Access2TargetMap.insert(std::pair<Instruction*,std::vector<Value*>>(StoreI,tmp_vec));
            }
            else
            {
                Access2TargetMap[StoreI].push_back(ori_node);
            }
        }
        else if (CallInst *CallI = dyn_cast<CallInst>(it->getUser()))
        {
            *BRAM_log << "    is an CALL instruction: " << *CallI << "\n";
            for (int i = 0; i < CallI->getNumArgOperands(); ++i)
            {
                if (CallI->getArgOperand(i) == cur_node)
                {
                    auto arg_it = CallI->getCalledFunction()->arg_begin();
                    auto arg_ie = CallI->getCalledFunction()->arg_end();
                    for (int j = 0 ; ; ++j,++arg_it)
                        if (i==j)
                        {
                            TraceAccessForTarget(arg_it,ori_node);
                            break;
                        }                    
                }
            }
        }
        else
        {
            *BRAM_log << "    is an general instruction: " << *it->getUser() << "\n";
            TraceAccessForTarget(it->getUser(),ori_node);
        }        
    }
    ValueVisited.erase(cur_node);
}

HI_NoDirectiveTimingResourceEvaluation::timingBase HI_NoDirectiveTimingResourceEvaluation::scheduleBRAMAccess(Instruction *access, BasicBlock *cur_block, HI_NoDirectiveTimingResourceEvaluation::timingBase cur_Timing)
{
    assert(Access2TargetMap.find(access) != Access2TargetMap.end() && "The access should be recorded in the BRAM access info.\n");
    timingBase res(0,0,1,clock_period);
    for (auto target : Access2TargetMap[access])
    {        
        *BRAM_log << "\n\n\n sceduling access instruction: " << *access << " for the target [" << target->getName() << "]" << " of Block:" << cur_block->getName() <<"\n";
        timingBase targetTiming = handleBRAMAccessFor(access, target, cur_block, cur_Timing);        
        if (access->getOpcode()==Instruction::Store)
        {
            *BRAM_log << " sceduled access Store instruction: " << *access << " for the target [" << target->getName() << "] at cycle #" << targetTiming.latency << " of Block:" << cur_block->getName() << "\n";
        }
        else
        {
            *BRAM_log << " sceduled access Load instruction: " << *access << " for the target [" << target->getName() << "] at cycle #" << targetTiming.latency-1 << " of Block:" << cur_block->getName() << "\n";
        }
        
        if (targetTiming > res)
            res = targetTiming;
        BRAM_log->flush();
    }
    return res;
}

HI_NoDirectiveTimingResourceEvaluation::timingBase HI_NoDirectiveTimingResourceEvaluation::handleBRAMAccessFor(Instruction *access, Value *target, BasicBlock *cur_block, HI_NoDirectiveTimingResourceEvaluation::timingBase cur_Timing)
{
    if (scheduledAccess_timing.find(std::pair<Instruction*,Value*>(access, target)) != scheduledAccess_timing.end())
    {
        return scheduledAccess_timing[std::pair<Instruction*,Value*>(access, target)]; 
    }

    std::string LoadOrStore;
    if (access->getOpcode() == Instruction::Store)
        LoadOrStore = "store";
    else    
        LoadOrStore = "load";   

    if (checkBRAMAvailabilty(target, LoadOrStore, cur_block, cur_Timing))
    {        
        *BRAM_log << "    the access instruction: " << *access << " for the target [" << target->getName() << "] can be scheduled in cycle #" <<cur_Timing.latency << " of Block:" << cur_block->getName() <<"\n";
        *BRAM_log << "    cur_timing is " << cur_Timing << " opTiming is " << get_inst_info_result(LoadOrStore.c_str(),-1,-1,clock_period_str) << " ";
        scheduledAccess_timing[std::pair<Instruction*,Value*>(access, target)] = cur_Timing+get_inst_info_result(LoadOrStore.c_str(),-1,-1,clock_period_str);
        *BRAM_log << "resultTiming is "<< scheduledAccess_timing[std::pair<Instruction*,Value*>(access, target)] << "\n";
        insertBRAMAccessInfo(target, cur_block, cur_Timing.latency);
        return scheduledAccess_timing[std::pair<Instruction*,Value*>(access, target)];
    }
    else
    {
        while (1)
        {
            *BRAM_log << "    the access instruction: " << *access << " for the target [" << target->getName() << "] CANNOT be scheduled in cycle #" <<cur_Timing.latency << " of Block:" << cur_block->getName() <<"\n";
            cur_Timing.latency++;
            cur_Timing.timing=0;
            if (checkBRAMAvailabilty(target, LoadOrStore, cur_block, cur_Timing))
            {
                *BRAM_log << "    the access instruction: " << *access << " for the target [" << target->getName() << "] can be scheduled in cycle #" <<cur_Timing.latency << " of Block:" << cur_block->getName() <<"\n";
                scheduledAccess_timing[std::pair<Instruction*,Value*>(access, target)] = cur_Timing+get_inst_info_result(LoadOrStore.c_str(),-1,-1,clock_period_str);
                insertBRAMAccessInfo(target, cur_block, cur_Timing.latency);
                return scheduledAccess_timing[std::pair<Instruction*,Value*>(access, target)];
            }
            BRAM_log->flush();
        }
    }    
    BRAM_log->flush();
}

bool HI_NoDirectiveTimingResourceEvaluation::checkBRAMAvailabilty(Value *target, std::string StoreOrLoad, BasicBlock *cur_block, HI_NoDirectiveTimingResourceEvaluation::timingBase cur_Timing)
{
    *BRAM_log << "       checking the access to the target [" << target->getName() << "] in cycle #" << cur_Timing.latency << " of Block:" << cur_block->getName() <<"  --> ";
    timingBase opTiming = get_inst_info_result(StoreOrLoad.c_str(),-1,-1,clock_period_str);
    timingBase res = cur_Timing + opTiming;

    if (StoreOrLoad == "load" && (res.latency-1 != cur_Timing.latency))
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
    if (target2LastAccessCycleInBlock[target].find(cur_block) == target2LastAccessCycleInBlock[target].end())
    {
        *BRAM_log << " No access for the target in the block yet\n";
        return true;
    }
    int cnt = 0;
    for (int lat : target2LastAccessCycleInBlock[target][cur_block])
    {
        if (lat == cur_Timing.latency)
            cnt ++;
    }
    *BRAM_log << cnt << " access(es) in this cycle\n";
    if (cnt < 2)
        return true;
    return false;
}

void HI_NoDirectiveTimingResourceEvaluation::insertBRAMAccessInfo(Value *target, BasicBlock *cur_block, int cur_latency)
{
    *BRAM_log << "       inserting the access to the target [" << target->getName() << "] in cycle #" << cur_latency << " of Block:" << cur_block->getName() << "  --> ";
    if (target2LastAccessCycleInBlock.find(target) == target2LastAccessCycleInBlock.end())
    {
        std::map<BasicBlock*,std::vector<int>> tmp_map;
        std::vector<int> tmp_vec;
        tmp_vec.push_back(cur_latency);
        tmp_map[cur_block] = tmp_vec;
        target2LastAccessCycleInBlock[target] = tmp_map;
        *BRAM_log << "(new map new vector)\n";
        return;
    }
    if (target2LastAccessCycleInBlock[target].find(cur_block) == target2LastAccessCycleInBlock[target].end())
    {
        std::vector<int> tmp_vec;
        tmp_vec.push_back(cur_latency);
        target2LastAccessCycleInBlock[target][cur_block] = tmp_vec;
        *BRAM_log << "(new vector)\n";
        return;
    }
    *BRAM_log << "(existed vector)\n";
    target2LastAccessCycleInBlock[target][cur_block].push_back(cur_latency);
}