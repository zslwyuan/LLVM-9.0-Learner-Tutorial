#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Pass.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/raw_ostream.h"
#include "HI_print.h"
#include "HI_PragmaTargetExtraction.h"

#include <stdio.h>
#include <string>
#include <ios>
#include <stdlib.h>
#include <sstream>
using namespace llvm;

// find the array declaration in the function F and trace the accesses to them
void HI_PragmaTargetExtraction::findMemoryDeclarationAndAnalyzeAccessin(Function *F, bool isTopFunction)
{
    if (DEBUG) *arrayTarget_Log << "checking the BRAM information in Function: " << F->getName() << "\n";
    if (DEBUG) *arrayTarget_Log << "\n\nchecking the BRAM information in Function: " << F->getName() << "\n";
    ValueVisited.clear();

    // for top function in HLS, arrays in interface may involve BRAM
    if (isTopFunction)
    {
        if (DEBUG) *arrayTarget_Log << " is Top function " << "\n";
        if (DEBUG) *arrayTarget_Log << " is Top function " << "\n";
        for (auto it = F->arg_begin(),ie = F->arg_end(); it!=ie; ++it)
        {
            if (it->getType()->isPointerTy())
            {
                PointerType *tmp_PtrType = dyn_cast<PointerType>(it->getType());
                if (tmp_PtrType->getElementType()->isArrayTy())
                {
                    if (DEBUG) *arrayTarget_Log << "  get array information of [" << it->getName() << "] from argument and its address=" << it << "\n";
                    Target2ArrayInfo[it]=getArrayInfo(it);
                    Array2Info[std::pair<std::string, std::string>(F->getName(), it->getName())] = Target2ArrayInfo[it];
                    FuncTargetSet.insert(std::pair<Function*, Value*>(F, it));
                    TraceAccessForTarget(it,it);         
                    if (DEBUG) *arrayTarget_Log << Target2ArrayInfo[it] << "\n";
                }
                else if (tmp_PtrType->getElementType()->isIntegerTy() || tmp_PtrType->getElementType()->isFloatingPointTy() ||tmp_PtrType->getElementType()->isDoubleTy() )
                {
                    if (DEBUG) *arrayTarget_Log << "  get array information of [" << it->getName() << "] from argument and its address=" << it << "\n";
                    FuncTargetSet.insert(std::pair<Function*, Value*>(F, it));
                    Target2ArrayInfo[it]=getArrayInfo(it);
                    Array2Info[std::pair<std::string, std::string>(F->getName(), it->getName())] = Target2ArrayInfo[it];
                    TraceAccessForTarget(it,it);
                    if (DEBUG) *arrayTarget_Log << Target2ArrayInfo[it] << "\n";
                }
            }
        }
    }
    else
    {
         if (DEBUG) *arrayTarget_Log << " is not top function " << "\n";
         if (DEBUG) *arrayTarget_Log << " is not top function " << "\n";
    }
    
    // for general function in HLS, arrays in functions are usually declared with alloca instruction
    for (auto &B: *F)
    {
        for (auto &I: B)
        {
            if (AllocaInst *allocI = dyn_cast<AllocaInst>(&I))
            {
                if (DEBUG) *arrayTarget_Log << "  get array information of [" << *allocI << "] from allocaInst and its address=" << allocI << "\n";
                Target2ArrayInfo[allocI]=getArrayInfo(allocI);
                Array2Info[std::pair<std::string, std::string>(F->getName(), allocI->getName())] = Target2ArrayInfo[allocI];
                FuncTargetSet.insert(std::pair<Function*, Value*>(F, allocI));
                TraceAccessForTarget(allocI,allocI);
                if (DEBUG) *arrayTarget_Log << Target2ArrayInfo[allocI] << "\n";
            }
        }
    }
    if (DEBUG) *arrayTarget_Log << "\n\n\n---------------- Access to Target ---------------" << "\n";
    for (auto it : Access2TargetMap)
    {
        if (DEBUG) *arrayTarget_Log << "The instruction [" << *it.first << "] will access :";
        for (auto tmpI : it.second)
        {
            if (DEBUG) *arrayTarget_Log << " (" << tmpI->getName() << ") ";
        }
        if (DEBUG) *arrayTarget_Log << "\n";
    }
    if (DEBUG) *arrayTarget_Log << "-------------------------------------------------" << "\n\n\n\n";
    arrayTarget_Log->flush();
    arrayTarget_Log->flush();

}

// find out which instrctuins are related to the array, going through PtrToInt, Add, IntToPtr, Store, Load instructions
// record the corresponding target array which the access instructions try to touch
void HI_PragmaTargetExtraction::TraceAccessForTarget(Value *cur_node, Value *ori_node)
{
    if (DEBUG) *arrayTarget_Log << "\n\n\nTracing the access to Array " << ori_node->getName() << " and looking for the users of " << *cur_node<< "\n";
    
    if (Instruction* tmpI = dyn_cast<Instruction>(cur_node))
    {
        if (DEBUG) *arrayTarget_Log << "    --- is an instruction:\n";
    }
    else
    {
        if (DEBUG) *arrayTarget_Log << "    --- is not an instruction \n";
    }

    arrayTarget_Log->flush();
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
        if (DEBUG) *arrayTarget_Log << "    find user of " << ori_node->getName() << " --> " << *it->getUser() <<  "\n";
        Instruction2Target[it->getUser()].push_back(ori_node);
        // Load and Store Instructions are leaf nodes in the DFS
        if (LoadInst *LoadI = dyn_cast<LoadInst>(it->getUser()))
        {
            Block2Targets[LoadI->getParent()].insert(ori_node);
            
            if (Access2TargetMap.find(LoadI)==Access2TargetMap.end())
            {                
                if (DEBUG) *arrayTarget_Log << "    is an LOAD instruction: " << *LoadI << "\n";
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
            Block2Targets[StoreI->getParent()].insert(ori_node);
            if (Access2TargetMap.find(StoreI)==Access2TargetMap.end())
            {
                if (DEBUG) *arrayTarget_Log << "    is an STORE instruction: " << *StoreI << "\n";
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
            Block2Targets[CallI->getParent()].insert(ori_node);
            if (DEBUG) *arrayTarget_Log << "    is an CALL instruction: " << *CallI << "\n";
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
            
            if (DEBUG) *arrayTarget_Log << "    is an general instruction: " << *it->getUser() << "\n";
            TraceAccessForTarget(it->getUser(),ori_node);
        }        
    }
    ValueVisited.erase(cur_node);
}





HI_PragmaArrayInfo HI_PragmaTargetExtraction::getArrayInfo(Value* target)
{

    PointerType* ptr_type = dyn_cast<PointerType>(target->getType());
    if (DEBUG) *arrayTarget_Log << "\n\nchecking type : " << *ptr_type << " and its ElementType is: [" << *ptr_type->getElementType()  << "]\n";
    Type* tmp_type = ptr_type->getElementType();
    int total_ele = 1;
    int tmp_dim_size[10];
    int num_dims = 0;
    while (auto array_T = dyn_cast<ArrayType>(tmp_type))
    {
        if (DEBUG) *arrayTarget_Log << "----- element type of : " << *tmp_type << " is " << *(array_T->getElementType()) << " and the number of its elements is " << (array_T->getNumElements()) <<"\n";
        total_ele *=  (array_T->getNumElements()) ;
        tmp_dim_size[num_dims] =  (array_T->getNumElements()) ;
        num_dims++;
        tmp_type = array_T->getElementType();
    }

    HI_PragmaArrayInfo res_array_info;
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

    res_array_info.elementType = tmp_type;
    res_array_info.target = target;
    res_array_info.targetName = target->getName();

    return res_array_info;
}








AliasResult HI_PragmaTargetExtraction::HI_AAResult::alias(const MemoryLocation &LocA,
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







raw_ostream& operator<< (raw_ostream& stream, const HI_PragmaArrayInfo& tb)
{
    stream << "HI_PragmaArrayInfo for: << (" << tb.target << ") " << *tb.target << ">> [ele_Type= " << *tb.elementType << ", num_dims=" << tb.num_dims << ", " ;
    for (int i = 0; i<tb.num_dims; i++)
    {
        if (tb.cyclic)
            stream << "dim-" << i << "C-s" << tb.dim_size[i] << "-p" << tb.partition_size[i] << ",  ";
        else
            stream << "dim-" << i << "B-s" << tb.dim_size[i] << "-p" << tb.partition_size[i] << ",  ";
    }

    // for (int i = 0; i<tb.num_dims; i++)
    // {
    //     stream << "dim-" << i << "-subnum=" << tb.sub_element_num[i] << ", ";
    // }


    stream << "] ";
    //timing="<<tb.timing<<"] ";
    return stream;
}