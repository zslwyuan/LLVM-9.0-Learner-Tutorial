#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Pass.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/raw_ostream.h"
#include "HI_print.h"
#include "HI_WithDirectiveTimingResourceEvaluation.h"
#include "polly/PolyhedralInfo.h"

#include <stdio.h>
#include <string>
#include <ios>
#include <stdlib.h>
#include <sstream>
using namespace llvm;

// find the array declaration in the function F and trace the accesses to them
void HI_WithDirectiveTimingResourceEvaluation::findMemoryDeclarationin(Function *F, bool isTopFunction)
{
    *BRAM_log << "checking the BRAM information in Function: " << F->getName() << "\n";
    *ArrayLog << "\n\nchecking the BRAM information in Function: " << F->getName() << "\n";
    ValueVisited.clear();
    scheduledAccess_timing.clear();

    // for top function in HLS, arrays in interface may involve BRAM
    if (isTopFunction)
    {
        *BRAM_log << " is Top function " << "\n";
        *ArrayLog << " is Top function " << "\n";
        for (auto it = F->arg_begin(),ie = F->arg_end(); it!=ie; ++it)
        {
            if (it->getType()->isPointerTy())
            {
                PointerType *tmp_PtrType = dyn_cast<PointerType>(it->getType());
                if (tmp_PtrType->getElementType()->isArrayTy())
                {
                    TraceAccessForTarget(it,it);
                    Target2ArrayInfo[it]=getArrayInfo(it);
                    matchArrayAndConfiguration(it);
                    *ArrayLog << Target2ArrayInfo[it] << "\n";
                }
            }
        }
    }
    else
    {
         *BRAM_log << " is not top function " << "\n";
         *ArrayLog << " is not top function " << "\n";
    }
    
    // for general function in HLS, arrays in functions are usually declared with alloca instruction
    for (auto &B: *F)
    {
        for (auto &I: B)
        {
            if (AllocaInst *allocI = dyn_cast<AllocaInst>(&I))
            {
                TraceAccessForTarget(allocI,allocI);
                Target2ArrayInfo[allocI]=getArrayInfo(allocI);
                matchArrayAndConfiguration(allocI);
                *ArrayLog << Target2ArrayInfo[allocI] << "\n";
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
    ArrayLog->flush();

}

// find out which instrctuins are related to the array, going through PtrToInt, Add, IntToPtr, Store, Load instructions
void HI_WithDirectiveTimingResourceEvaluation::TraceAccessForTarget(Value *cur_node, Value *ori_node)
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
        *BRAM_log << "    find user of " << ori_node->getName() << " --> " << *it->getUser() <<  "\n";

        // Load and Store Instructions are leaf nodes in the DFS
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
        // if a pointer of arrray is passed as sub-function's argument, handle it
        else if (CallInst *CallI = dyn_cast<CallInst>(it->getUser()))
        {
            *BRAM_log << "    is an CALL instruction: " << *CallI << "\n";
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

// schedule the access to potential target (since an instructon may use the address for different target (e.g. address comes from PHINode), we need to schedule all of them)
HI_WithDirectiveTimingResourceEvaluation::timingBase 
HI_WithDirectiveTimingResourceEvaluation::scheduleBRAMAccess(Instruction *access, BasicBlock *cur_block, 
                                                             HI_WithDirectiveTimingResourceEvaluation::timingBase cur_Timing)
{
    assert(Access2TargetMap.find(access) != Access2TargetMap.end() && "The access should be recorded in the BRAM access info.\n");
    timingBase res(0,0,1,clock_period);
    for (auto target : Access2TargetMap[access])
    {        
        *BRAM_log << "\n\n\n sceduling access instruction: " << *access << " for the target [" << target->getName() << "]" << " of Block:" << cur_block->getName() <<"\n";
        timingBase targetTiming = handleBRAMAccessFor(access, target, cur_block, cur_Timing);        
        if (access->getOpcode()==Instruction::Store)
        {
            *BRAM_log << " sceduled access Store instruction: " << *access << 
                        " for the target ["  << target->getName() << 
                        "] in its partition #" << getPartitionFor(access) << 
                        " at cycle #" << targetTiming.latency << 
                        " of Block:" << cur_block->getName() << "\n";
        }
        else
        {
            *BRAM_log << " sceduled access Load instruction: " << *access << 
                        " for the target [" << target->getName() << 
                        "] in its partition #" << getPartitionFor(access) << 
                        " at cycle #" << targetTiming.latency-1 << 
                        " of Block:" << cur_block->getName() << "\n";
        }
        
        if (targetTiming > res)
            res = targetTiming;
        BRAM_log->flush();
    }
    return res;
}

// schedule the access to specific target for the instruction
HI_WithDirectiveTimingResourceEvaluation::timingBase 
HI_WithDirectiveTimingResourceEvaluation::handleBRAMAccessFor(Instruction *access, Value *target, 
                                                              BasicBlock *cur_block, 
                                                              HI_WithDirectiveTimingResourceEvaluation::timingBase cur_Timing)
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

    // if the access can take place at cur_Timing, schedule and record it
    if (checkBRAMAvailabilty(access, target, LoadOrStore, cur_block, cur_Timing))
    {        
        *BRAM_log << "    the access instruction: " << *access << " for the target [" << target->getName() << "] can be scheduled in cycle #" <<cur_Timing.latency << " of Block:" << cur_block->getName() <<"\n";
        *BRAM_log << "    cur_timing is " << cur_Timing << " opTiming is " << getInstructionLatency(access) << " ";
        scheduledAccess_timing[std::pair<Instruction*,Value*>(access, target)] = cur_Timing + getInstructionLatency(access);
        *BRAM_log << "resultTiming is "<< scheduledAccess_timing[std::pair<Instruction*,Value*>(access, target)] << "\n";
        insertBRAMAccessInfo(target, cur_block, cur_Timing.latency, access);
        return scheduledAccess_timing[std::pair<Instruction*,Value*>(access, target)];
    }
    else
    {
        // otherwise, try later time slots and see whether the schedule can be successful.
        while (1)
        {
            *BRAM_log << "    the access instruction: " << *access << " for the target [" << target->getName() << "] CANNOT be scheduled in cycle #" <<cur_Timing.latency << " of Block:" << cur_block->getName() <<"\n";
            cur_Timing.latency++;
            cur_Timing.timing=0;
            if (checkBRAMAvailabilty(access, target, LoadOrStore, cur_block, cur_Timing))
            {
                *BRAM_log << "    the access instruction: " << *access << " for the target [" << target->getName() << "] can be scheduled in cycle #" <<cur_Timing.latency << " of Block:" << cur_block->getName() <<"\n";
                scheduledAccess_timing[std::pair<Instruction*,Value*>(access, target)] = cur_Timing + getInstructionLatency(access);
                insertBRAMAccessInfo(target, cur_block, cur_Timing.latency, access);
                return scheduledAccess_timing[std::pair<Instruction*,Value*>(access, target)];
            }
            BRAM_log->flush();
        }
    }    
    BRAM_log->flush();
}


// TODO: insert the information of array partition here
// check whether the access to target array can be scheduled in a specific cycle
// TODO: map access operator to 
bool HI_WithDirectiveTimingResourceEvaluation::checkBRAMAvailabilty(Instruction* access, Value *target, std::string StoreOrLoad, BasicBlock *cur_block, HI_WithDirectiveTimingResourceEvaluation::timingBase cur_Timing)
{
    *BRAM_log << "       checking the access to the target [" << target->getName() << "] in cycle #" << cur_Timing.latency << " of Block:" << cur_block->getName() <<"  --> ";
    timingBase opTiming =  getInstructionLatency(access); // get_inst_TimingInfo_result(StoreOrLoad.c_str(),-1,-1,clock_period_str);
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
    int target_partition = getPartitionFor(access);
    for (auto lat_Inst_pair : target2LastAccessCycleInBlock[target][cur_block])
    {
        if (lat_Inst_pair.first.first == cur_Timing.latency && lat_Inst_pair.first.second == target_partition )
            cnt ++;
    }
    *BRAM_log << cnt << " access(es) in this cycle\n";
    // consider the number of BRAM port (currently it is a constant (2). Later, consider array partitioning)
    if (cnt < 2)
        return true;
    return false;
}

// record the schedule information
void HI_WithDirectiveTimingResourceEvaluation::insertBRAMAccessInfo(Value *target, BasicBlock *cur_block, int cur_latency, Instruction* access)
{
    *BRAM_log << "       inserting the access to the target [" << target->getName() 
              << "] in its partition #" << getPartitionFor(access) 
              << " in cycle #" << cur_latency 
              << " of Block:" << cur_block->getName() << "  --> ";
    if (target2LastAccessCycleInBlock.find(target) == target2LastAccessCycleInBlock.end())
    {
        std::map<BasicBlock*,std::vector<std::pair<std::pair<int, int>,Instruction*>>> tmp_map;
        std::vector<std::pair<std::pair<int, int>,Instruction*>> tmp_vec;
        tmp_vec.push_back(std::pair<std::pair<int, int>,Instruction*>(std::pair<int, int>(cur_latency,getPartitionFor(access)),access));
        tmp_map[cur_block] = tmp_vec;
        target2LastAccessCycleInBlock[target] = tmp_map;
        *BRAM_log << "(new map new vector)\n";
        return;
    }
    if (target2LastAccessCycleInBlock[target].find(cur_block) == target2LastAccessCycleInBlock[target].end())
    {
        std::vector<std::pair<std::pair<int, int>,Instruction*>> tmp_vec;
        tmp_vec.push_back(std::pair<std::pair<int, int>,Instruction*>(std::pair<int, int>(cur_latency,getPartitionFor(access)),access));
        target2LastAccessCycleInBlock[target][cur_block] = tmp_vec;
        *BRAM_log << "(new vector)\n";
        return;
    }
    *BRAM_log << "(existed vector)\n";
    target2LastAccessCycleInBlock[target][cur_block].push_back(std::pair<std::pair<int, int>,Instruction*>(std::pair<int, int>(cur_latency,getPartitionFor(access)),access));
}



// evaluate the number of LUT needed by the BRAM MUXs
HI_WithDirectiveTimingResourceEvaluation::resourceBase HI_WithDirectiveTimingResourceEvaluation::BRAM_MUX_Evaluate()
{
    resourceBase res(0,0,0,clock_period);
    int inputSize = 0;
       
    for (auto Val_IT : target2LastAccessCycleInBlock)
    {
        int access_counter_for_value = 0;
        int read_counter_for_value = 0;
        int write_counter_for_value = 0;
        *Evaluating_log << " The access to target: [" << Val_IT.first->getName() <<"] includes:\n";
        for (auto B2Cycles : Val_IT.second)
        {
            access_counter_for_value += B2Cycles.second.size();
            *Evaluating_log << " in block: [" << B2Cycles.first->getName() <<"] cycles: ";
            for (auto C_tmp : B2Cycles.second)
            {                
                if (auto readI = dyn_cast<LoadInst>(C_tmp.second))
                {
                    read_counter_for_value ++ ;
                    *Evaluating_log << " --- R(c" << C_tmp.first.first << ",p" << C_tmp.first.second <<") ";
                }
                if (auto writeI = dyn_cast<StoreInst>(C_tmp.second))
                {
                    write_counter_for_value ++ ;
                    *Evaluating_log << " --- W(c" << C_tmp.first.first << ",p" << C_tmp.first.second <<") ";
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

    res.LUT = inputSize * 5;
    return res;

}


// get the number of BRAMs which are needed by the alloca instruction
HI_WithDirectiveTimingResourceEvaluation::resourceBase HI_WithDirectiveTimingResourceEvaluation::get_BRAM_Num_For(AllocaInst *alloca_I)
{
    resourceBase res(0,0,0,0,clock_period);
    *BRAM_log << "\n\nchecking allocation instruction [" << *alloca_I << "] and its type is: " << *alloca_I->getType() << " and its ElementType is: [" << *alloca_I->getType()->getElementType()  << "]\n";
    Type* tmp_type = alloca_I->getType()->getElementType();
    int total_ele = 1;
    while (auto array_T = dyn_cast<ArrayType>(tmp_type))
    {
        *BRAM_log << "----- element type of : " << *tmp_type << " is " << *(array_T->getElementType()) << " and the number of its elements is " << (array_T->getNumElements()) <<"\n";
        total_ele *=  (array_T->getNumElements()) ;
        tmp_type = array_T->getElementType();
    }
    int BW = 0;
    if (tmp_type->isIntegerTy())
        BW = tmp_type->getIntegerBitWidth();
    else if (tmp_type->isFloatTy())
        BW = 32;
    else if (tmp_type->isDoubleTy())
        BW = 64;
    assert(BW!=0 && "we should get BW for the basic element type.\n");
    res = get_BRAM_Num_For(BW, total_ele);
    *BRAM_log << "checked allocation instruction [" << *alloca_I << "] and its basic elemenet type is: [" << *tmp_type << "] with BW=[" <<BW << "] and the total number of basic elements is: [" << total_ele << "] and it need BRAMs [" << res.BRAM << "].\n\n";
    
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



// get the offset of the access to array and get the access information
void HI_WithDirectiveTimingResourceEvaluation::TryArrayAccessProcess(Instruction *I, ScalarEvolution *SE)
{
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

    // 1.  get the initial value of access address by using SCEV
    const SCEV *tmp_S = SE->getSCEV(I);
    const SCEVAddRecExpr *SARE = dyn_cast<SCEVAddRecExpr>(tmp_S);
    if (SARE)
    {
        if (SARE->isAffine())
        {
            *ArrayLog << *I << " --> is add rec Affine Add: " << *SARE  << " it operand (0) " << *SARE->getOperand(0)  << " it operand (1) " << *SARE->getOperand(1) << "\n";
            *ArrayLog << " -----> intial offset expression: " << *findTheActualStartValue(SARE) <<"\n";
            ArrayLog->flush();
            const SCEV *initial_expr_tmp = findTheActualStartValue(SARE);
            int initial_const = -1;
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
                        *ArrayLog << " -----> intial offset const: " << initial_const <<"\n";
                        ArrayLog->flush();
                    }
                    else
                    {
                        if (const SCEVUnknown* array_value_scev = dyn_cast<SCEVUnknown>(initial_expr_add->getOperand(i)))
                        {
                            *ArrayLog << " -----> access target: " << *array_value_scev->getValue() << "\n";
                            if (auto tmp_PTI_I = dyn_cast<PtrToIntInst>(array_value_scev->getValue()))
                            {
                                target = tmp_PTI_I->getOperand(0);
                            }
                            else
                            {
                                assert(target && "There should be an PtrToInt Instruction for the addition operation.\n");
                            }
                            *ArrayLog << " -----> access target info: " << Target2ArrayInfo[target] << "\n";         
                            ArrayLog->flush();               
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
                *ArrayLog << " -----> intial offset const: " << initial_const <<"\n";
                *ArrayLog << " -----> access target: " << *initial_expr_unknown->getValue() << "\n";
                if (auto tmp_PTI_I = dyn_cast<PtrToIntInst>(initial_expr_unknown->getValue()))
                {
                    target = tmp_PTI_I->getOperand(0);
                }
                else
                {
                    assert(target && "There should be an PtrToInt Instruction for the addition operation.\n");
                }
                
                *ArrayLog << " -----> access target info: " << Target2ArrayInfo[target] << "\n";         
                ArrayLog->flush();     
            }

            assert(initial_const >= 0 && "the initial offset should be found.\n");
            assert(target && "the target array should be found.\n");
            AddressInst2AccessInfo[I] = getAccessInfoFor(target, I, initial_const);
            *ArrayLog << " -----> access info with array index: " << AddressInst2AccessInfo[I] << "\n\n\n";
            ArrayLog->flush();
        }
    }
    return;
}


HI_WithDirectiveTimingResourceEvaluation::HI_AccessInfo HI_WithDirectiveTimingResourceEvaluation::getAccessInfoFor(Value* target, Instruction* access, int initial_offset)
{
    HI_AccessInfo res(Target2ArrayInfo[target]);
    for (int i=0;i<res.num_dims;i++)
    {
        res.index[i] = (initial_offset / res.sub_element_num[i]) % res.dim_size[i];
    }
    return res;
}

const SCEV * HI_WithDirectiveTimingResourceEvaluation::findTheActualStartValue(const SCEVAddRecExpr *S)
{
    if (const SCEVAddExpr *start_V = dyn_cast<SCEVAddExpr>(S->getOperand(0)))
    {
        return start_V;
    }
    else if (const SCEVUnknown *start_V_unknown = dyn_cast<SCEVUnknown>(S->getOperand(0)))
    {
        return start_V_unknown;
    }
    else
    {
        if (const SCEVAddRecExpr *SARE = dyn_cast<SCEVAddRecExpr>(S->getOperand(0)))
            findTheActualStartValue(SARE);
    }
}

// check the memory access in the function
void HI_WithDirectiveTimingResourceEvaluation::TraceMemoryAccessinFunction(Function &F)
{
    if (F.getName().find("llvm.")!=std::string::npos) // bypass the "llvm.xxx" functions..
        return;
    findMemoryAccessin(&F);        
    
}


// find the array access in the function F and trace the accesses to them
void HI_WithDirectiveTimingResourceEvaluation::findMemoryAccessin(Function *F)
{
    *ArrayLog << "checking the Memory Access information in Function: " << F->getName() << "\n";
    ArrayValueVisited.clear();


    // for general function in HLS, arrays in functions are usually declared with alloca instruction
    for (auto &B: *F)
    {
        for (auto &I: B)
        {
            if (IntToPtrInst *ITP_I = dyn_cast<IntToPtrInst>(&I))
            {
                *ArrayLog << "find a IntToPtrInst: [" << *ITP_I << "] backtrace to its operands.\n";
                TraceAccessForTarget(ITP_I);
            }
        }
    }
    *ArrayLog << "-------------------------------------------------" << "\n\n\n\n";
    ArrayLog->flush();
}

// find out which instrctuins are related to the array, going through PtrToInt, Add, IntToPtr, Store, Load instructions
void HI_WithDirectiveTimingResourceEvaluation::TraceAccessForTarget(Value *cur_node)
{
    *ArrayLog << "looking for the operands of " << *cur_node<< "\n";
    if (Instruction* tmpI = dyn_cast<Instruction>(cur_node))
    {       
        Inst_AccessRelated.insert(tmpI);
    }
    else
    {
        return;
    }    

    Instruction* curI = dyn_cast<Instruction>(cur_node);
    ArrayLog->flush();

    // we are doing DFS now
    if (ArrayValueVisited.find(cur_node)!=ArrayValueVisited.end())
        return;

    ArrayValueVisited.insert(cur_node);

    // Trace the uses of the pointer value or integer generaed by PtrToInt
    for (int i = 0; i < curI->getNumOperands(); ++i)
    {
        Value * tmp_op = curI->getOperand(i);
        TraceAccessForTarget(tmp_op);
    }
    ArrayValueVisited.erase(cur_node);
}


HI_WithDirectiveTimingResourceEvaluation::ArrayInfo HI_WithDirectiveTimingResourceEvaluation::getArrayInfo(Value* target)
{

    PointerType* ptr_type = dyn_cast<PointerType>(target->getType());
    *ArrayLog << "\n\nchecking type : " << *ptr_type << " and its ElementType is: [" << *ptr_type->getElementType()  << "]\n";
    Type* tmp_type = ptr_type->getElementType();
    int total_ele = 1;
    int tmp_dim_size[10];
    int num_dims = 0;
    while (auto array_T = dyn_cast<ArrayType>(tmp_type))
    {
        *ArrayLog << "----- element type of : " << *tmp_type << " is " << *(array_T->getElementType()) << " and the number of its elements is " << (array_T->getNumElements()) <<"\n";
        total_ele *=  (array_T->getNumElements()) ;
        tmp_dim_size[num_dims] =  (array_T->getNumElements()) ;
        num_dims++;
        tmp_type = array_T->getElementType();
    }

    ArrayInfo res_array_info;
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
        res_array_info.sub_element_num[num_dims] = res_array_info.sub_element_num[num_dims-1] * res_array_info.dim_size[num_dims-1];
        res_array_info.dim_size[num_dims] = 100000000; // set to nearly infinite
        res_array_info.num_dims ++;
        res_array_info.isArgument = 1;
    }

    res_array_info.elementType = tmp_type;
    res_array_info.target = target;
    return res_array_info;
}

// get the targer partition according to the specific memory access information
int HI_WithDirectiveTimingResourceEvaluation::getPartitionFor(HI_WithDirectiveTimingResourceEvaluation::HI_AccessInfo access, int partition_factor, int partition_dimension)
{
    int res = -1;
    res = access.index[partition_dimension-1]%partition_factor;
    return res;
}

// get the targer partition for the specific memory access instruction
int HI_WithDirectiveTimingResourceEvaluation::getPartitionFor(Instruction* access, int partition_factor, int partition_dimension)
{
    int res = -1;
    res = getAccessInfoForAccessInst(access).index[partition_dimension-1]%partition_factor;
    return res;
}

// get the targer partition for the specific memory access instruction
int HI_WithDirectiveTimingResourceEvaluation::getPartitionFor(Instruction* access)
{
    int res = -1;
    int partition_factor = -1, partition_dimension = -1;
    
    auto &targetVec = Access2TargetMap[access];
    HI_PragmaInfo tmp_PragmaInfo;
    if (targetVec.size()==1)
    {
        Value* target = targetVec[0];
        if (arrayDirectives.find(target) != arrayDirectives.end())
        {
            tmp_PragmaInfo = arrayDirectives[target];
            partition_dimension = tmp_PragmaInfo.dim;
            partition_factor = tmp_PragmaInfo.partition_factor;
            assert(partition_factor!=-1 && partition_dimension!=-1  && "The information should be valid.\n");
        }
        else
        {
            partition_dimension = 1;
            partition_factor = 1;
        }
    }
    else
    {
        assert(false && "TODO: handle access to multiple potential targets.\n");
    }


    res = getAccessInfoForAccessInst(access).index[partition_dimension-1]%partition_factor;
    return res;
}


void HI_WithDirectiveTimingResourceEvaluation::ArrayAccessCheckForFunction(Function *F)
{
    for (auto &B : *F)
    {
        for (auto &I : B)
        {
            TryArrayAccessProcess(&I, SE /*, demangeFunctionName(F->getName()) == top_function_name*/);
        }
    }
}


/*
    get the access information for the load/store instruction, by tracing back to the address instruction
*/
HI_WithDirectiveTimingResourceEvaluation::HI_AccessInfo HI_WithDirectiveTimingResourceEvaluation::getAccessInfoForAccessInst(Instruction* Load_or_Store)
{
    Instruction *pointer_I = nullptr;
    if (Load_or_Store->getOpcode()==Instruction::Load)
    {
        pointer_I = dyn_cast<Instruction>(Load_or_Store->getOperand(0));
    }
    else
    {
        pointer_I = dyn_cast<Instruction>(Load_or_Store->getOperand(1));
    }
    Instruction *address_addI = dyn_cast<Instruction>(pointer_I->getOperand(0));
    assert(address_addI && "The pointer for this access should be found.\n");
    assert(AddressInst2AccessInfo.find(address_addI)!=AddressInst2AccessInfo.end() && "The pointer should be checked by TryArrayAccessProcess() previously.");
    return AddressInst2AccessInfo[address_addI];
}
