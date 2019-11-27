#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Pass.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/raw_ostream.h"
#include "HI_print.h"
#include "HI_MuxInsertionArrayPartition.h"

#include <stdio.h>
#include <string>
#include <ios>
#include <stdlib.h>
#include <sstream>
using namespace llvm;

// find the array declaration in the function F and trace the accesses to them
void HI_MuxInsertionArrayPartition::findMemoryDeclarationAndAnalyzeAccessin(Function *F, bool isTopFunction)
{
    if (DEBUG) *BRAM_log << "checking the BRAM information in Function: " << F->getName() << "\n";
    if (DEBUG) *ArrayLog << "\n\nchecking the BRAM information in Function: " << F->getName() << "\n";
    ValueVisited.clear();

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
                    if (DEBUG) *ArrayLog << Target2ArrayInfo[it] << "\n";
                }
                else if (tmp_PtrType->getElementType()->isIntegerTy() || tmp_PtrType->getElementType()->isFloatingPointTy() ||tmp_PtrType->getElementType()->isDoubleTy() )
                {
                    if (DEBUG) *ArrayLog << "  get array information of [" << it->getName() << "] from argument and its address=" << it << "\n";
                    Target2ArrayInfo[it]=getArrayInfo(it);
                    TraceAccessForTarget(it,it);
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
void HI_MuxInsertionArrayPartition::TraceAccessForTarget(Value *cur_node, Value *ori_node)
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
        Instruction2Target[it->getUser()].push_back(ori_node);
        // Load and Store Instructions are leaf nodes in the DFS
        if (LoadInst *LoadI = dyn_cast<LoadInst>(it->getUser()))
        {
            if (Access2TargetMap.find(LoadI)==Access2TargetMap.end())
            {                
                if (DEBUG) *BRAM_log << "    is an LOAD instruction: " << *LoadI << "\n";
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
                if (DEBUG) *BRAM_log << "    is an STORE instruction: " << *StoreI << "\n";
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
            if (DEBUG) *BRAM_log << "    is an CALL instruction: " << *CallI << "\n";
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


// get the offset of the access to array and get the access information
void HI_MuxInsertionArrayPartition::TryArrayAccessProcess(Instruction *I, ScalarEvolution *SE)
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
HI_MuxInsertionArrayPartition::HI_AccessInfo HI_MuxInsertionArrayPartition::getAccessInfoFor(Value* target, Instruction* access, int initial_offset, 
                                                                                              std::vector<int> *inc_indices, std::vector<int> *trip_counts, bool unpredictable)
{
    if (Alias2Target.find(target) != Alias2Target.end())
    {
        target = Alias2Target[target];
    }

    if (Target2ArrayInfo.find(target) == Target2ArrayInfo.end())
    {
        llvm::errs() << "Instruction: [" << *access << "\n";
        llvm::errs() << "Target: [" << *target << "\n";
        assert(false && "the target should be processed.");
    }
        
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
const SCEV * HI_MuxInsertionArrayPartition::findTheActualStartValue(const SCEVAddRecExpr *S)
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
            if (DEBUG) ArrayLog->flush();
            llvm::errs() << "nextStart: " << *nextStart << " and its type is " << nextStart->getSCEVType() << "\n";
            llvm::errs() << "bypass trunc nextStart: " << *bypassExtTruntSCEV(nextStart) << "\n";
            assert(false && "should not reach here.");
        }
    }
}

// get the index incremental value of the array access in the loop
void HI_MuxInsertionArrayPartition::findTheIncrementalIndexAndTripCount(const SCEVAddRecExpr *S, std::vector<int> &inc_indices, std::vector<int> &trip_counts)
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
            if (DEBUG) ArrayLog->flush();
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
void HI_MuxInsertionArrayPartition::TraceMemoryAccessinFunction(Function &F)
{
    if (F.getName().find("llvm.")!=std::string::npos || F.getName().find("HIPartitionMux")!=std::string::npos ) // bypass the "llvm.xxx" functions..
        return;
    findMemoryAccessin(&F);        
    
}


// find the array access in the function F and trace the accesses to them
void HI_MuxInsertionArrayPartition::findMemoryAccessin(Function *F)
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
void HI_MuxInsertionArrayPartition::TraceAccessRelatedInstructionForTarget(Value *cur_node)
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


HI_MuxInsertionArrayPartition::HI_ArrayInfo HI_MuxInsertionArrayPartition::getArrayInfo(Value* target)
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
        if (auto global_v = dyn_cast<GlobalVariable>(target) )
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

// get the total number of partitions of the target array
int HI_MuxInsertionArrayPartition::getTotalPartitionNum(HI_ArrayInfo &refInfo)
{
    int res = 1;
    for (int i=0; i<refInfo.num_dims; i++)
    {
        res *= refInfo.partition_size[i];
    }
    return res;
}

// get the targer partition for the specific memory access instruction
std::vector<HI_MuxInsertionArrayPartition::partition_info> HI_MuxInsertionArrayPartition::getPartitionFor(Instruction* access)
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
    for (int tmp_offset : generatePotentialOffset(tmp_res))
    {
        if (DEBUG) *ArrayLog << tmp_offset << ", ";
        // ArrayLog->flush();
        partition_info newPartitionForCheck = getAccessPartitionBasedOnAccessInfoAndInc(tmp_res, tmp_offset);
        // int cur_dim_index = (tmp_offset / tmp_res.sub_element_num[partition_dimension] % tmp_res.sub_element_num[partition_dimension+1]);
        // if (!tryRecordPartition(partitions, newPartitionForCheck))
        //     break;
        tryRecordPartition(partitions, newPartitionForCheck);
    }

    if (DEBUG) *ArrayLog << "\n";
    // ArrayLog->flush();
    return partitions;
}

// try to record a partition as a potential target
bool HI_MuxInsertionArrayPartition::tryRecordPartition(std::vector<partition_info> &partitions, partition_info try_target)
{
    for (auto val : partitions)
        if (val == try_target)
            return false;
    partitions.push_back(try_target);
    return true;
}

// get the offset of the access to array and get the access information
// in the function
bool HI_MuxInsertionArrayPartition::ArrayAccessCheckForFunction(Function *F)
{
    bool changed = 0;
    for (auto &B : *F)
    {
        for (auto &I : B)
        {
            if (DEBUG) *ArrayLog << "TryArrayAccessProcess: " << I << "\n";
            TryArrayAccessProcess(&I, SE /*, demangleFunctionName(F->getName()) == top_function_name*/);
        }
    }
    for (auto &B : *F)
    {
        changed |= MuxInsert(&B);
        if (muxWithMoreThan32)
            return true;
    }
    return changed;
}


/*
    get the access information for the load/store instruction, by tracing back to the address instruction
*/
HI_MuxInsertionArrayPartition::HI_AccessInfo HI_MuxInsertionArrayPartition::getAccessInfoForAccessInst(Instruction* Load_or_Store)
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
    {
        llvm::errs() << *address_addI << "<=======\n";
        if (auto arg_target = dyn_cast<Argument>(address_addI))
        {
            llvm::errs() << (arg_target->getParent()->getName()) << "<=======\n";            
        }
    }
        

    assert(AddressInst2AccessInfo.find(address_addI)!=AddressInst2AccessInfo.end() && "The pointer should be checked by TryArrayAccessProcess() previously.");
    return AddressInst2AccessInfo[address_addI];
}




AliasResult HI_MuxInsertionArrayPartition::HI_AAResult::alias(const MemoryLocation &LocA,
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


Value* HI_MuxInsertionArrayPartition::getTargetFromInst(Instruction* accessI)
{
    // assert(Access2TargetMap[accessI].size()==1 && "currently, we do not support 1-access-multi-target.");
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
    Value* target = Access2TargetMap[accessI][0];
    if (Alias2Target.find(target) == Alias2Target.end())
        return target;
    else
        return Alias2Target[target];
}


HI_MuxInsertionArrayPartition::partition_info HI_MuxInsertionArrayPartition::getAccessPartitionBasedOnAccessInfoAndInc(HI_MuxInsertionArrayPartition::HI_AccessInfo refInfo, int cur_offset)
{
    partition_info res_partiton_info;
    res_partiton_info.num_dims = refInfo.num_dims; 
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

bool HI_MuxInsertionArrayPartition::processNaiveAccess(Instruction *Load_or_Store)
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
        if (DEBUG) *ArrayLog << "processNaiveAccess: target" << *target << "\n";
        if (auto arg_pointer = dyn_cast<Argument>(target))
        {
            AddressInst2AccessInfo[target] = getAccessInfoFor(target, Load_or_Store, 0, nullptr, nullptr);
            if (DEBUG) *ArrayLog << " -----> access info with array index: " << AddressInst2AccessInfo[target] << "\n\n\n";
            // ArrayLog->flush();
        }
        else if (auto alloc_pointer = dyn_cast<AllocaInst>(target))
        {
            AddressInst2AccessInfo[target] = getAccessInfoFor(target, Load_or_Store, 0, nullptr, nullptr);
            if (DEBUG) *ArrayLog << " -----> access info with array index: " << AddressInst2AccessInfo[target] << "\n\n\n";
            // ArrayLog->flush();
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
                                        
                        if (auto arg_target = dyn_cast<Argument>(target))
                            llvm::errs() << "function: [" << *arg_target->getParent() << "\n";

                        assert(Target2ArrayInfo.find(target) != Target2ArrayInfo.end() 
                                    && Alias2Target.find(target) != Alias2Target.end()
                                    && "Fail to find the array inforamtion for the target.");
                    }
                }
                if (auto arg_pointer = dyn_cast<Argument>(target))
                {
                    AddressInst2AccessInfo[target] = getAccessInfoFor(target, Load_or_Store, 0, nullptr, nullptr);
                    AddressInst2AccessInfo[PTI] = getAccessInfoFor(target, Load_or_Store, 0, nullptr, nullptr);
                    if (DEBUG) *ArrayLog << " -----> access info with array index: " << AddressInst2AccessInfo[target] << "\n\n\n";
                    // ArrayLog->flush();
                }
                else if (auto alloc_pointer = dyn_cast<AllocaInst>(target))
                {
                    AddressInst2AccessInfo[target] = getAccessInfoFor(target, Load_or_Store, 0, nullptr, nullptr);
                    AddressInst2AccessInfo[PTI] = getAccessInfoFor(target, Load_or_Store, 0, nullptr, nullptr);
                    if (DEBUG) *ArrayLog << " -----> access info with array index: " << AddressInst2AccessInfo[target] << "\n\n\n";
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

void HI_MuxInsertionArrayPartition::handleSAREAccess(Instruction *I, const SCEVAddRecExpr* SARE)
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


void HI_MuxInsertionArrayPartition::handleDirectAccess(Instruction *I, const SCEVUnknown* SUnkown)
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



void HI_MuxInsertionArrayPartition::handleConstantOffsetAccess(Instruction *I, const SCEVAddExpr* SAE)
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
void HI_MuxInsertionArrayPartition::handleUnstandardSCEVAccess(Instruction *I, const SCEV* tmp_S)
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
        if (DEBUG) ArrayLog->flush();
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
                if (DEBUG) ArrayLog->flush();
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
void HI_MuxInsertionArrayPartition::handleComplexSCEVAccess(Instruction *I, const SCEV* tmp_S)
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

bool std::operator<(const HI_MuxInsertionArrayPartition::partition_info A, const HI_MuxInsertionArrayPartition::partition_info B)
{
    assert(A.num_dims==B.num_dims);
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
std::vector<int> HI_MuxInsertionArrayPartition::generatePotentialOffset(HI_AccessInfo &accessInfo)
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
void HI_MuxInsertionArrayPartition::getAllPotentialOffsetByRecuresiveSearch(HI_AccessInfo &accessInfo, int loopDep, int last_level_offset, std::vector<int> &res)
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
std::vector<HI_MuxInsertionArrayPartition::partition_info> HI_MuxInsertionArrayPartition::getAllPartitionFor(Instruction *access)
{
    HI_AccessInfo tmp_info = getAccessInfoForAccessInst(access);
    std::vector<partition_info> res;
    std::vector<int> tmp_partID = {-1,-1,-1,-1,-1,-1,-1,-1};
    getAllPartitionBasedOnInfo(tmp_info, 0, tmp_partID, res);
    return res;
}

// get all the partitions for the access target of the access instruction
void HI_MuxInsertionArrayPartition::getAllPartitionBasedOnInfo(HI_AccessInfo &info, int curDim, std::vector<int> &tmp_partID, std::vector<partition_info> &res)
{
    if (curDim == info.num_dims)
    {
        partition_info tmp_part_bank;
        Value *reftarget = info.target;
        if (Alias2Target.find(reftarget) != Alias2Target.end())
            reftarget = Alias2Target[reftarget];
        tmp_part_bank.target = reftarget;
        tmp_part_bank.num_dims = info.num_dims;
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





const SCEV* HI_MuxInsertionArrayPartition::bypassExtTruntSCEV(const SCEV* inputS)
{
    if (auto castSCEV = dyn_cast<SCEVCastExpr>(inputS))
        return bypassExtTruntSCEV(castSCEV->getOperand());
    else
        return inputS;
}


const SCEV* HI_MuxInsertionArrayPartition::findUnknown(const SCEV* ori_inputS)
{
    const SCEV* inputS = bypassExtTruntSCEV(ori_inputS);
    const SCEVNAryExpr* naryS = dyn_cast<SCEVNAryExpr>(inputS);
    const SCEVUDivExpr* divS = dyn_cast<SCEVUDivExpr>(inputS);
    if (naryS)
    {
        for (int i = 0; i<naryS->getNumOperands(); i++)
        {
            const SCEV *tmp = findUnknown(naryS->getOperand(i));
            if (tmp)
                return tmp;
        }
    }
    else if (divS)
    {
        const SCEV *tmp0 = findUnknown(divS->getLHS());
        if (tmp0)
            return tmp0;
        const SCEV *tmp1 = findUnknown(divS->getRHS());
        if (tmp1)
            return tmp1;
    } 
    else if (auto unknown = dyn_cast<SCEVUnknown>(inputS))
    {
        return unknown;
    }
    else
    {
        return nullptr;
    }    
}


// get the unknown values in the expression
int HI_MuxInsertionArrayPartition::getUnknownNum(const SCEV* ori_inputS)
{
    int res = 0;
    const SCEV* inputS = bypassExtTruntSCEV(ori_inputS);
    const SCEVNAryExpr* naryS = dyn_cast<SCEVNAryExpr>(inputS);
    const SCEVUDivExpr* divS = dyn_cast<SCEVUDivExpr>(inputS);
    if (naryS)
    {
        for (int i = 0; i<naryS->getNumOperands(); i++)
        {
            res += getUnknownNum(naryS->getOperand(i));
        }
    }
    else if (divS)
    {
        res += getUnknownNum(divS->getLHS());
        res += getUnknownNum(divS->getRHS());
    } 
    else if (auto unknown = dyn_cast<SCEVUnknown>(inputS))
    {
        res = 1;
    }

    return res;
}
