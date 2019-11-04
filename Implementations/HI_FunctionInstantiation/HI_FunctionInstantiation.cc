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
#include "HI_FunctionInstantiation.h"
#include <stdio.h>
#include <string>
#include <ios>
#include <stdlib.h>
#include <queue>

using namespace llvm;
 
bool HI_FunctionInstantiation::runOnModule(Module &M) // The runOnModule declaration will overide the virtual one in ModulePass, which will be executed for each Module.
{
    print_status("Running HI_FunctionInstantiation pass."); 

    bool initiated = false;
    collectCallInsts(&M);
    initiated = FunctionShouldBeInitiated.size()>0;
    for (auto &F : M)
    {
        if (F.getName().find("llvm.")!=std::string::npos)
            continue;
        if (demangleFunctionName(F.getName().str()) == topFunctionName)
        {
            *FuncInitiationLog << "Find the top function: [" << topFunctionName << "] and begin BFS.\n";
            BFS_check_and_initiate(&F, &M);
            BFS_checkDeadFunction(&F, &M);
            break;
        }
    }
    return initiated;
}



char HI_FunctionInstantiation::ID = 0;  // the ID for pass should be initialized but the value does not matter, since LLVM uses the address of this variable as label instead of its value.

void HI_FunctionInstantiation::getAnalysisUsage(AnalysisUsage &AU) const {
    AU.setPreservesCFG();
}

// initiate a function for a specific call instruction
void HI_FunctionInstantiation::initiateFunctionFor(Instruction *CallI, Function *F)
{
    return;
}

// collect the information of call instructions, mainly for checking the necessity of initiation
void HI_FunctionInstantiation::collectCallInsts(Module *M)
{
    *FuncInitiationLog << "Collecting call graph and call instruction for the module.\n";
    for (auto &F : *M)
        for (auto &B : F)
            for (auto &I : B)
            {
                if (CallInst* callI = dyn_cast<CallInst>(&I))
                {
                    if (callI->getCalledFunction()->getName().find("llvm.")!=std::string::npos)
                        continue;
                    for (auto _callI : CallInst_List)
                    {
                        auto pre_callI = dyn_cast<CallInst>(_callI);
                        if (callI->getCalledFunction() == pre_callI->getCalledFunction())
                        {
                            insertFunctionShouldBeInitiated(callI->getCalledFunction());
                            break;
                        }
                    }
                    CallInst_List.push_back(callI);
                }
            }
    
    return;
}

void HI_FunctionInstantiation::insertFunctionShouldBeInitiated(Function *F)
{
    for (auto tmpF : FunctionShouldBeInitiated)
    {
        if (tmpF == F)
            return;
    }
    *FuncInitiationLog << "    The function [" << F->getName() << "] is inserted into instantiation list.\n" ;
    FunctionShouldBeInitiated.push_back(F);
}

bool HI_FunctionInstantiation::ShouldFunctionBeInitiated(Function *F)
{
    for (auto tmpF : FunctionShouldBeInitiated)
    {
        if (tmpF == F)
            return true;
    }
    return false;
}

void HI_FunctionInstantiation::BFS_check_and_initiate(Function *startF, Module *M)
{
    std::queue<Function *> FuncQ;
    std::set<Function *> FuncVisited;
    FuncQ.push(startF);
    
    while (FuncQ.size()>0)
    {
        Function *curF = FuncQ.front();
        FuncVisited.insert(curF);
        FuncQ.pop();
        if (curF->getName().find("llvm.")!=std::string::npos)
            continue;
        *FuncInitiationLog << "    BFS reach the function [" << curF->getName() << "] \n" ;
        checkAndInitiateCallInstIn(curF);
        pushCalledFunctionIntoQueue(curF, FuncQ, FuncVisited);
    }
}

// generate instantiation and replace call instrutctions
void HI_FunctionInstantiation::checkAndInitiateCallInstIn(Function *curF)
{
    // generate instantiation and replace call instrutctions
    *FuncInitiationLog << "          check and initiate sub-functions invoked by call instructions for the function [" << curF->getName() << "] \n" ;
    bool updated = 1;
    while (updated)
    {
        updated = 0;
        for (auto &B : *curF)
        {
            for (auto &I : B)
            {
                if (CallInst* callI = dyn_cast<CallInst>(&I))
                {
                    auto targetF = callI->getCalledFunction();
                    if (targetF->getName().find("llvm.")!=std::string::npos)
                        continue;
                    if (ShouldFunctionBeInitiated(targetF))
                    {
                        ValueToValueMapTy vmap;
                        Function *F_clone = CloneFunction(targetF, vmap);
                        std::string newName = targetF->getName().str() + std::to_string(newFuncID(targetF));
                        F_clone->setName(newName);
                        *FuncInitiationLog << "          ---- going to replace called function for CallInst [" << *callI <<  "] with function: [" << F_clone->getName() << "]\n";
                        replaceCallInst(callI, F_clone);
                        updated = 1;
                        break;
                    }
                }
            }
            if (updated)
                break;
        }
    }

}

void HI_FunctionInstantiation::pushCalledFunctionIntoQueue(Function *curF, std::queue<Function *> &FuncQ, std::set<Function *> &FuncVisited)
{
    *FuncInitiationLog << "          push called sub-function into queue for the function [" << curF->getName() << "] \n" ;
    for (auto &B : *curF)
    {
        for (auto &I : B)
        {
            if (CallInst* callI = dyn_cast<CallInst>(&I))
            {
                auto targetF = callI->getCalledFunction();
                if (targetF->getName().find("llvm.")!=std::string::npos)
                    continue;
                if (FuncVisited.find(targetF) == FuncVisited.end())
                {
                    *FuncInitiationLog << "          ---- push the sub-function [" << targetF->getName() << "] into queue\n" ;
                    FuncQ.push(targetF);
                }
            }
        }
    }
}

int HI_FunctionInstantiation::newFuncID(Function* F)
{
    if (IntitiatedFunctionCnt.find(F)==IntitiatedFunctionCnt.end())
    {
        IntitiatedFunctionCnt[F] = 0;
        return 0;
    }
    IntitiatedFunctionCnt[F] = IntitiatedFunctionCnt[F] + 1;
    return IntitiatedFunctionCnt[F];
}

void HI_FunctionInstantiation::replaceCallInst(Instruction *callI, Function* F)
{
    *FuncInitiationLog << "          ---- replacing called function for CallInst [" << *callI <<  "] with function: [" << F->getName() << "]\n";
                    


    IRBuilder<> Builder(callI);
    std::vector<Value*> ori_args;
    *FuncInitiationLog << "          ---- args are: ";
    for (int i = 0; i < F->getFunctionType()->getNumParams();i++)
    {
        ori_args.push_back(callI->getOperand(i));
        *FuncInitiationLog << "#" << i << ": [" << *callI->getOperand(i) << "] ";
    }
    *FuncInitiationLog << "\n";
    *FuncInitiationLog << "          ---- args_size=" << ori_args.size() << " FuncParams_size=" << F->getFunctionType()->getNumParams() << "\n";

    *FuncInitiationLog << "\n\n================== new function =================================\n" 
                        << *F
                       << "\n===================================================\n\n";

    CallInst* newCallI = Builder.CreateCall(F, ori_args);
    callI->replaceAllUsesWith(newCallI);
    callI->eraseFromParent();
    *FuncInitiationLog << "\n\n=============== new module ============================\n" 
                        << *F->getParent()  
                       << "\n===================================================\n\n";
    *FuncInitiationLog << "          ---- replaced\n";
    
}

void HI_FunctionInstantiation::BFS_checkDeadFunction(Function *startF, Module *M)
{
    *FuncInitiationLog << "\n\n\n=========================================\nremoving dead function \n" ;
    std::queue<Function *> FuncQ;
    std::set<Function *> FuncVisited;
    FuncQ.push(startF);
    
    while (FuncQ.size()>0)
    {
        Function *curF = FuncQ.front();
        FuncVisited.insert(curF);
        FuncQ.pop();
        if (curF->getName().find("llvm.")!=std::string::npos)
            continue;
        for (auto &B : *curF)
        {
            for (auto &I : B)
            {
                if (CallInst* callI = dyn_cast<CallInst>(&I))
                {
                    auto targetF = callI->getCalledFunction();
                    if (targetF->getName().find("llvm.")!=std::string::npos)
                        continue;
                    if (FuncVisited.find(targetF) == FuncVisited.end())
                    {
                        FuncQ.push(targetF);
                    }
                }
            }
        }
    }
    bool updated = 1;
    while (updated)
    {
        updated = 0;
        for (auto &F : *M)
        {
            if (FuncVisited.find(&F) == FuncVisited.end())
            {
                if (F.getName().find("llvm.")!=std::string::npos)
                    continue;

                *FuncInitiationLog << "          ---- remove dead function [" << F.getName() << "] from module\n";
                F.eraseFromParent();
                updated = 1;
                break;
            }
        }
    }
}