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

void HI_NoDirectiveTimingResourceEvaluation::TraceMemoryDeclarationin(Function *F, bool isTopFunction)
{
    ValueVisited.clear();
    if (isTopFunction)
    {
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
}

void HI_NoDirectiveTimingResourceEvaluation::TraceAccessForTarget(Value *cur_node, Value *ori_node)
{
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
        if (LoadInst *LoadI = dyn_cast<LoadInst>(it->getUser()))
        {
            if (Access2TargetMap.find(LoadI)==Access2TargetMap.end())
            {
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
            TraceAccessForTarget(it->getUser(),ori_node);
        }        
    }
    ValueVisited.erase(cur_node);
}

// void HI_NoDirectiveTimingResourceEvaluation::scheduleBRAMAccess(Value *target, BasicBlock *cur_block, int cur_latency, double timing)
// {

//     if (checkBRAMAvailabilty(Value *target, BasicBlock *cur_block, int cur_latency))
// }

// bool HI_NoDirectiveTimingResourceEvaluation::checkBRAMAvailabilty(Value *target, BasicBlock *cur_block, int cur_latency)
// {
// }