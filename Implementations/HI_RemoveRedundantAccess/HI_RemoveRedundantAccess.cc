#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Pass.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/raw_ostream.h"
#include "HI_print.h"
#include "HI_RemoveRedundantAccess.h"
#include <stdio.h>
#include <string>
#include <ios>
#include <stdlib.h>

using namespace llvm;
 
bool HI_RemoveRedundantAccess::runOnModule(Module &M) // The runOnModule declaration will overide the virtual one in ModulePass, which will be executed for each Module.
{
    print_status("Running HI_RemoveRedundantAccess pass.");  
    noAliasHazard_cache.clear();
    checkMustAlias_cache.clear();
    bool changed = false;
    ValueVisited.clear();
    Alias2Target.clear();
    Access2TargetMap.clear();

    if (DEBUG) *RemoveRedundantAccess_Log << M << "\n\n\n\n\n=======================================\n\n\n\n";

    for (auto &F : M)
    {
        if (F.getName().find("llvm.")!=std::string::npos) // bypass the "llvm.xxx" functions..
            continue;
        std::string mangled_name = F.getName();
        std::string demangled_name;
        demangled_name = demangleFunctionName(mangled_name);
        findMemoryDeclarationin(&F, demangled_name == top_function_name && F.getName().find(".") == std::string::npos);
    }

    for (auto &F : M)
    {
        differentCache.clear();

        if (F.getName().find("llvm.")!=std::string::npos) // bypass the "llvm.xxx" functions..
            continue;

        SE = &getAnalysis<ScalarEvolutionWrapperPass>(F).getSE(); 

        std::string mangled_name = F.getName();
        std::string demangled_name;
        demangled_name = demangleFunctionName(mangled_name);

        bool ActionTaken = true;
        while (ActionTaken)
        {
            ActionTaken = false;

            if (DEBUG) *RemoveRedundantAccess_Log << "\n\n\n================================\nbefore FUNCTION:\n " << F << "\n";

            ActionTaken = checkAndRemoveRedundantLoadin(F);
            changed |= ActionTaken;

            ActionTaken |= checkAndRemoveRedundantStorein(F);
            changed |= ActionTaken;


        }
    }


    // return false;
    return changed;
}



char HI_RemoveRedundantAccess::ID = 0;  // the ID for pass should be initialized but the value does not matter, since LLVM uses the address of this variable as label instead of its value.

void HI_RemoveRedundantAccess::getAnalysisUsage(AnalysisUsage &AU) const {
    AU.addRequired<AAResultsWrapperPass>(); 
    AU.addRequired<DominatorTreeWrapperPass>();
    AU.addRequired<ScalarEvolutionWrapperPass>();
    AU.addRequired<TargetTransformInfoWrapperPass>();
    AU.addRequired<LoopInfoWrapperPass>();
    AU.setPreservesCFG();
}

// check whether the two instructions are MUST-alias instructions
bool HI_RemoveRedundantAccess::checkMustAlias(Instruction *I0, Instruction *I1)
{
    std::pair<Instruction*, Instruction*> I_pair(I0,I1);
    if (checkMustAlias_cache.find(I_pair) != checkMustAlias_cache.end())
        return checkMustAlias_cache[I_pair];

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
        return pointer_I0==pointer_I1;
    }

    assert(pointer_I1 && pointer_I1->getOpcode() == Instruction::IntToPtr && "ITP should be found for the access instruction");

    // std::string tmp0(""),tmp1("");
    // raw_string_ostream *SCEV_Stream0 = new raw_string_ostream(tmp0);
    // raw_string_ostream *SCEV_Stream1 = new raw_string_ostream(tmp1);
    
    const SCEV *tmp_S0 = SE->getSCEV(pointer_I0->getOperand(0));
    const SCEV *tmp_S1 = SE->getSCEV(pointer_I1->getOperand(0));

    if (!tmp_S0 || !tmp_S1)
    {
        checkMustAlias_cache[I_pair] = false;
        return false;
    }

    Optional<APInt> res=computeConstantDifference(tmp_S0,tmp_S1);

    if (res != None)
    {
        if (res.getValue().getSExtValue()==0)
        {
            checkMustAlias_cache[I_pair] = true;
            return true;
        }
        checkMustAlias_cache[I_pair] = false;
        return false;
    }
    checkMustAlias_cache[I_pair] = false;
    return false;



}

// check whether the two instructions could be alias instructions
// true if it is not possible, false if it is possible
bool HI_RemoveRedundantAccess::noAliasHazard(Instruction *I0, Instruction *I1)
{
    std::pair<Instruction*, Instruction*> I_pair(I0,I1);
    if (noAliasHazard_cache.find(I_pair) != noAliasHazard_cache.end())
        return noAliasHazard_cache[I_pair];

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

    // std::string tmp0(""),tmp1("");
    // raw_string_ostream *SCEV_Stream0 = new raw_string_ostream(tmp0);
    // raw_string_ostream *SCEV_Stream1 = new raw_string_ostream(tmp1);
    
    const SCEV *tmp_S0 = SE->getSCEV(pointer_I0->getOperand(0));
    const SCEV *tmp_S1 = SE->getSCEV(pointer_I1->getOperand(0));

    Optional<APInt> res=computeConstantDifference(tmp_S0,tmp_S1);
    // if (DEBUG) *RemoveRedundantAccess_Log << "    compute diff between " << *tmp_S0 << " and " << *tmp_S1 << "\n";
    if (res != None)
    {
        // if (DEBUG) *RemoveRedundantAccess_Log << "    result= " << res << "\n";
        if (res.getValue().getSExtValue()==0)
        {
            // if (DEBUG) *RemoveRedundantAccess_Log << "    which means must alias\n";
            noAliasHazard_cache[I_pair] = false;
            return false;
        }
        // if (DEBUG) *RemoveRedundantAccess_Log << "    which means must no-alias\n";
        noAliasHazard_cache[I_pair] = true;
        return true;
    }
    noAliasHazard_cache[I_pair] = false;
    return false;
}


// check whether there is any MUST-alias for the specific insturction
// in its basic block
void HI_RemoveRedundantAccess::checkAliasFor(Instruction *I)
{
    if (I->getOpcode()!=Instruction::Load && I->getOpcode()!=Instruction::Store)
        return;

    BasicBlock *curB = I->getParent();

    for (auto anotherI : accessInsts)
    {
        if (anotherI->getOpcode()!=Instruction::Load && anotherI->getOpcode()!=Instruction::Store)
            continue;

        if (anotherI == I)
            continue;

        // if (DEBUG) *RemoveRedundantAccess_Log << "check alias between <<" << *I << ">> and <<" << anotherI << ">> .\n"; 
        if (!noAliasHazard(I, anotherI))
        {
            // if (DEBUG) *RemoveRedundantAccess_Log << " (== could be Alias==).\n";
            // accessAliasSet.insert(std::pair<Instruction*, Instruction*>(I, anotherI));
            AliasedAccess.insert(I);//[I].push_back(anotherI);
            break;
        }
        else
        {
            // if (DEBUG) *RemoveRedundantAccess_Log << " (NO  Alias).\n";
        }        
    }
}

// print the aliases for the instruction
void HI_RemoveRedundantAccess::printAliasFor(Instruction *I)
{
    // if (AliasedAccess.find(I) != AliasedAccess.end())
    // {
    //     if (DEBUG) *RemoveRedundantAccess_Log << "Instruction: " << *I << " alias with :\n";
    //     for (auto anotherI : AliasedAccess[I])
    //     {
    //         if (DEBUG) *RemoveRedundantAccess_Log << "    " << *anotherI << "\n";
    //     }
    //     if (DEBUG) *RemoveRedundantAccess_Log << "\n";
    // }
}

// for RAW aliases, we can forward the written data 
// so the load instruction could be removed.
void HI_RemoveRedundantAccess::forwardAndRemove(Value *resForward, Instruction *loadI)
{
    if (DEBUG) *RemoveRedundantAccess_Log << "replace " << *loadI << "--with--" << *resForward << "\n";
    loadI->replaceAllUsesWith(resForward);
    RecursivelyDeleteTriviallyDeadInstructions(loadI);
    // loadI->eraseFromParent();
}

// check the load instructions in the function
// check whether some of them are redundant and
// remove them
bool HI_RemoveRedundantAccess::checkAndRemoveRedundantLoadin(Function &F)
{
    if (DEBUG) *RemoveRedundantAccess_Log << "\n\ncheckAndRemoveRedundantLoadin:\n";
    bool ActionTaken = true;
    bool changed = false;
    while (ActionTaken)
    {
        ActionTaken = false;
        Instruction *lastI = nullptr;    
        for (auto &B : F)
        {
            accessInsts.clear();
            AliasedAccess.clear();
            accessAliasSet.clear();
            bool startCheckEnableFlag = false;
            for (auto &I : B)
            {
                if (!startCheckEnableFlag)
                {
                    if (lastI == &I || !lastI) 
                    {
                        startCheckEnableFlag = true;
                    }
                    else
                        continue;
                }
                if (I.getOpcode() == Instruction::Load || I.getOpcode() == Instruction::Store)
                {
                    accessInsts.push_back(&I);
                    if (DEBUG) *RemoveRedundantAccess_Log << "pushback: " << I << "\n";
                }
            }
            
            
            for (auto I : accessInsts)
            {

                if (I->getOpcode() != Instruction::Store)
                {
                    // check RAR
                    // assert(false && "TODO: implement RAR optimization.");
                    if (DEBUG) *RemoveRedundantAccess_Log << "\n\n\nchecking Load for RAR: " << *I;

                    checkAliasFor(I);
                    if (AliasedAccess.find(I) != AliasedAccess.end())
                    {
                        bool checkEnableFlag = false;
                        
                        for (auto nextI : accessInsts)
                        {
                            if (!checkEnableFlag)
                            {
                                checkEnableFlag = (nextI == I);
                            }
                            else
                            {
                                if (getTargetFromInst(I) == getTargetFromInst(nextI))
                                {
                                    if (nextI->getOpcode() != Instruction::Load)
                                    {
                                        if (noAliasHazard(nextI,I))
                                        {
                                            if (DEBUG) *RemoveRedundantAccess_Log << "----- find store " << *nextI << " but no possibility of alias\n";
                                            continue;
                                        }
                                        else
                                        {
                                            if (DEBUG) *RemoveRedundantAccess_Log << "----- find store " << *nextI << " with possibility of alias\n";
                                            break;
                                        }
                                        
                                    }

                                    if (checkMustAlias(I, nextI))
                                    {
                                        if (nextI->getOpcode() == Instruction::Load)
                                        {
                                            if (DEBUG) *RemoveRedundantAccess_Log << "----- remove RAR redundant load" << *nextI << "\n";
                                            nextI->replaceAllUsesWith(I);
                                            nextI->eraseFromParent();
                                            lastI = I;
                                            ActionTaken = true;
                                            changed = true;
                                            break;
                                        }
                                    }
                                }                            
                            }
                        }
                    }
                    if (ActionTaken)
                        break;
                }
                else
                {
                    if (DEBUG) *RemoveRedundantAccess_Log << "\n\n\nchecking Store: " << *I;

                    checkAliasFor(I);
                    if (AliasedAccess.find(I) != AliasedAccess.end())
                    {
                        bool checkEnableFlag = false;
                        
                        for (auto nextI : accessInsts)
                        {
                            if (!checkEnableFlag)
                            {
                                checkEnableFlag = (nextI == I);
                            }
                            else
                            {
                                if (getTargetFromInst(I) == getTargetFromInst(nextI))
                                {
                                    if (checkMustAlias(I, nextI))
                                    {
                                        if (nextI->getOpcode() == Instruction::Load)
                                        {
                                            if (DEBUG) *RemoveRedundantAccess_Log << "----- remove RAW redundant load" << *nextI << "\n";
                                            lastI = I;
                                            forwardAndRemove(I->getOperand(0),nextI);
                                            ActionTaken = true;
                                            changed = true;
                                            break;
                                        }
                                    }
                                    if (nextI->getOpcode() == Instruction::Store)
                                        break;
                                }                           
                            }
                        }
                    }
                }
                

                if (ActionTaken)
                    break;
            }

            if (ActionTaken)
                break;

        }
    }
    ActionTaken |= changed;
    while (ActionTaken)
    {
        ActionTaken = false;
        for (auto &B : F)
        {
            for (auto &I : B)
            {
                ActionTaken |= RecursivelyDeleteTriviallyDeadInstructions(&I);
                if (ActionTaken)
                    break;
            }
            if (ActionTaken)
                break;
        }
    }
    return changed;
}

// check the store instructions in the function
// check whether some of them are redundant and
// remove them (consider that WAW for HLS may 
// be redundant.)
bool HI_RemoveRedundantAccess::checkAndRemoveRedundantStorein(Function &F)
{
    if (DEBUG) *RemoveRedundantAccess_Log << "\n\ncheckAndRemoveRedundantStorein:\n";
    bool ActionTaken = true;
    bool changed = false;
    while (ActionTaken)
    {
        ActionTaken = false;
        Instruction *lastI = nullptr;
        bool nomre = false;
        for (auto &B : F)
        {
            accessInsts.clear();
            AliasedAccess.clear();
            accessAliasSet.clear();
            bool startCheckEnableFlag = false;
            if (!nomre)
            {
                for (auto &I : B)
                {
                    if (!startCheckEnableFlag)
                    {
                        if (lastI == &I || !lastI) 
                        {
                            startCheckEnableFlag = true;
                        }
                        else
                            continue;
                    }
                    if (I.getOpcode() == Instruction::Load || I.getOpcode() == Instruction::Store)
                    {
                        accessInsts.push_back(&I);
                        if (DEBUG) *RemoveRedundantAccess_Log << "pushback: " << I << "\n";
                    }
                }
            }

            int tmp_cnt = 0;
            for (auto I : accessInsts)
            {
                tmp_cnt++;
                if (I->getOpcode() != Instruction::Store)
                    continue;

                if (DEBUG) *RemoveRedundantAccess_Log << "\n\n\nchecking Store: " << *I;

                checkAliasFor(I);
                if (AliasedAccess.find(I) != AliasedAccess.end())
                {
                    bool checkEnableFlag = false;
                    
                    for (auto nextI : accessInsts)
                    {
                        if (!checkEnableFlag)
                        {
                            checkEnableFlag = (nextI == I);
                        }
                        else
                        {
                            if (getTargetFromInst(I) == getTargetFromInst(nextI))
                            {
                                if (nextI->getOpcode() != Instruction::Store)
                                    break;

                                if (checkMustAlias(I, nextI))
                                {
                                    if (nextI->getOpcode() == Instruction::Store)
                                    {
                                        if (DEBUG) *RemoveRedundantAccess_Log << "----- remove redundant store" << *I << "\n";
                                        I->eraseFromParent();
                                        ActionTaken = true;
                                        changed = true;

                                        if (tmp_cnt>=accessInsts.size())
                                            nomre = true;
                                        else
                                            lastI = accessInsts[tmp_cnt];
                                    
                                        break;
                                    }
                                }
                            }                            
                        }
                    }
                }
                if (ActionTaken)
                    break;
            }

            if (ActionTaken)
                break;

        }
    }

    ActionTaken |= changed;
    while (ActionTaken)
    {
        ActionTaken = false;
        for (auto &B : F)
        {
            for (auto &I : B)
            {
                ActionTaken |= RecursivelyDeleteTriviallyDeadInstructions(&I);
                if (ActionTaken)
                    break;
            }
            if (ActionTaken)
                break;
        }
    }

    return changed;
}

// find the declaration in the function
// and trace the access to it
void HI_RemoveRedundantAccess::findMemoryDeclarationin(Function *F, bool isTopFunction)
{
    if (DEBUG) *RemoveRedundantAccess_Log << "checking the BRAM information in Function: " << F->getName() << "\n";
    ValueVisited.clear();

    // for top function in HLS, arrays in interface may involve BRAM
    if (isTopFunction)
    {
        if (DEBUG) *RemoveRedundantAccess_Log << " is Top function " << "\n";
        for (auto it = F->arg_begin(),ie = F->arg_end(); it!=ie; ++it)
        {
            if (it->getType()->isPointerTy())
            {
                PointerType *tmp_PtrType = dyn_cast<PointerType>(it->getType());
                if (tmp_PtrType->getElementType()->isArrayTy())
                {
                    if (DEBUG) *RemoveRedundantAccess_Log << " beging to trace arg: " << it << "\n";
                    TraceAccessForTarget(it,it);              
                }
                else if (tmp_PtrType->getElementType()->isIntegerTy() || tmp_PtrType->getElementType()->isFloatingPointTy() ||tmp_PtrType->getElementType()->isDoubleTy() )
                {
                    if (DEBUG) *RemoveRedundantAccess_Log << " beging to trace arg: " << it << "\n";
                    TraceAccessForTarget(it,it);
                }
            }
        }
    }
    else
    {
         if (DEBUG) *RemoveRedundantAccess_Log << " is not top function " << "\n";
    }
    
    // for general function in HLS, arrays in functions are usually declared with alloca instruction
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
    if (DEBUG) *RemoveRedundantAccess_Log << "\n\n\n---------------- Access to Target ---------------" << "\n";
    // for (auto it : Access2TargetMap)
    // {
    //     if (DEBUG) *RemoveRedundantAccess_Log << "The instruction [" << *it.first << "] will access :";
    //     for (auto tmpI : it.second)
    //     {
    //         if (Alias2Target.find(tmpI) == Alias2Target.end())
    //             if (DEBUG) *RemoveRedundantAccess_Log << " (" << tmpI->getName() << ") ";
    //         else
    //             if (DEBUG) *RemoveRedundantAccess_Log << " (" << Alias2Target[tmpI]->getName() << ") ";
            
    //     }
    //     if (DEBUG) *RemoveRedundantAccess_Log << "\n";
    // }
    if (DEBUG) *RemoveRedundantAccess_Log << "-------------------------------------------------" << "\n\n\n\n";
    RemoveRedundantAccess_Log->flush();

}

// find out which instrctuins are related to the array, going through PtrToInt, Add, IntToPtr, Store, Load instructions
void HI_RemoveRedundantAccess::TraceAccessForTarget(Value *cur_node, Value *ori_node)
{
    if (DEBUG) *RemoveRedundantAccess_Log << "\n\n\nTracing the access to Array " << ori_node->getName() << " and looking for the users of " << *cur_node<< "\n";
    if (Instruction* tmpI = dyn_cast<Instruction>(cur_node))
    {
        if (DEBUG) *RemoveRedundantAccess_Log << "    --- is an instruction:\n";
    }
    else
    {
        if (DEBUG) *RemoveRedundantAccess_Log << "    --- is not an instruction \n";
    }

    RemoveRedundantAccess_Log->flush();
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
        if (DEBUG) *RemoveRedundantAccess_Log << "    find user of " << ori_node->getName() << " --> " << *it->getUser() <<  "\n";

        // Load and Store Instructions are leaf nodes in the DFS
        if (LoadInst *LoadI = dyn_cast<LoadInst>(it->getUser()))
        {
            if (Access2TargetMap.find(LoadI)==Access2TargetMap.end())
            {
                if (DEBUG) *RemoveRedundantAccess_Log << "    is an LOAD instruction: " << *LoadI << "\n";
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
                if (DEBUG) *RemoveRedundantAccess_Log << "    is an STORE instruction: " << *StoreI << "\n";
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
            if (DEBUG) *RemoveRedundantAccess_Log << "    is an CALL instruction: " << *CallI << "\n";
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
            if (DEBUG) *RemoveRedundantAccess_Log << "    is an general instruction: " << *it->getUser() << "\n";
            TraceAccessForTarget(it->getUser(),ori_node);
        }        
    }
    ValueVisited.erase(cur_node);
}

//  (no-use now) check whether there is risk
bool HI_RemoveRedundantAccess::hasRisk(Instruction* accessI )
{
    Instruction *pointer_I = nullptr;
    if (accessI->getOpcode()==Instruction::Load)
    {
        pointer_I = dyn_cast<Instruction>(accessI->getOperand(0));
    }
    else if (accessI->getOpcode()==Instruction::Store)
    {
        pointer_I = dyn_cast<Instruction>(accessI->getOperand(1));
    }
    assert(pointer_I && pointer_I->getOpcode() == Instruction::IntToPtr && "the pointer for access should be found.");

  //  const SCEVAddRecExpr *SARE = dyn_cast<SCEVAddRecExpr>(SE->getSCEV());
}

// get the target array for the access instruction
Value* HI_RemoveRedundantAccess::getTargetFromInst(Instruction* accessI)
{
    if (getTargetFromInst_cache.find(accessI) != getTargetFromInst_cache.end())
        return getTargetFromInst_cache[accessI];
    // assert(Access2TargetMap[accessI].size()==1 && "currently, we do not support 1-access-multi-target.");
    if (Access2TargetMap[accessI].size()>1)
    {
        
        Value* reftarget = Access2TargetMap[accessI][0];
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
    {
       getTargetFromInst_cache[accessI] = target;
       return target;
    }
    else
    {
        getTargetFromInst_cache[accessI] = Alias2Target[target];
        return Alias2Target[target];
    }
}

Optional<APInt> HI_RemoveRedundantAccess::computeConstantDifference(const SCEV *More,
                                                           const SCEV *Less) {
  std::pair<const SCEV *, const SCEV *> ML_pair(More, Less);
  if (differentCache.find(ML_pair) != differentCache.end())
    return differentCache[ML_pair];
  // We avoid subtracting expressions here because this function is usually
  // fairly deep in the call stack (i.e. is called many times).

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
                //if (DEBUG) *RemoveRedundantAccess_Log << "----- downgrade " << *nextI << " but no possibility of alias\n";
                differentCache[ML_pair] = computeConstantDifference(MA->getOperand(0), LA->getOperand(0));
                return differentCache[ML_pair];
            }
        }
  }

  while (isa<SCEVAddRecExpr>(Less) && isa<SCEVAddRecExpr>(More)) {
    const auto *LAR = cast<SCEVAddRecExpr>(Less);
    const auto *MAR = cast<SCEVAddRecExpr>(More);

    if (LAR->getLoop() != MAR->getLoop())
    {
        differentCache[ML_pair] = None;
        return None;
    }

    // We look at affine expressions only; not for correctness but to keep
    // getStepRecurrence cheap.
    if (!LAR->isAffine() || !MAR->isAffine())
    {
        differentCache[ML_pair] = None;
        return None;
    }

    if (LAR->getStepRecurrence(*SE) != MAR->getStepRecurrence(*SE))
    {
        differentCache[ML_pair] = None;
        return None;
    }

    Less = LAR->getStart();
    More = MAR->getStart();

    // fall through
  }

  if (isa<SCEVConstant>(Less) && isa<SCEVConstant>(More)) {
    const auto &M = cast<SCEVConstant>(More)->getAPInt();
    const auto &L = cast<SCEVConstant>(Less)->getAPInt();
    differentCache[ML_pair] = M - L;
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
      {
          differentCache[ML_pair] = -(C1->getAPInt());
        return -(C1->getAPInt());
      }

  // Compare X vs (X + C2).
  if (splitBinaryAdd(More, LMore, RMore, Flags))
    if ((C2 = dyn_cast<SCEVConstant>(LMore)))
      if (RMore == Less)
      {
          differentCache[ML_pair] = C2->getAPInt();
        return C2->getAPInt();
      }

  // Compare (X + C1) vs (X + C2).
  if (C1 && C2 && RLess == RMore)
  {
      differentCache[ML_pair] = C2->getAPInt() - C1->getAPInt();
    return C2->getAPInt() - C1->getAPInt();
  }

    
    differentCache[ML_pair] = None;
    return None;
    
}

bool HI_RemoveRedundantAccess::splitBinaryAdd(const SCEV *Expr,
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

bool HI_RemoveRedundantAccess::hasSameTargetPair(Instruction *I0, Instruction *I1)
{
    if (SameTargetPair.find(std::pair<Instruction*,Instruction*>(I0,I1)) == SameTargetPair.end())
    {
        return SameTargetPair[std::pair<Instruction*,Instruction*>(I0,I1)];
    }
    else
    {
        bool res = getTargetFromInst(I0) == getTargetFromInst(I1);
        SameTargetPair[std::pair<Instruction*,Instruction*>(I0,I1)] = res;
        SameTargetPair[std::pair<Instruction*,Instruction*>(I1,I0)] = res;
        return res;
    }
    
}