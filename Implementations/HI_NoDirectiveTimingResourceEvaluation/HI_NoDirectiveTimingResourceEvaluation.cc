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


bool HI_NoDirectiveTimingResourceEvaluation::runOnModule(Module &M) // The runOnFunction declaration will overide the virtual one in ModulePass, which will be executed for each Function.
{    
    TraceMemoryDeclarationinModule(M);

    AnalyzeFunctions(M);

    analyzeTopFunction(M);

    return false;
}

bool HI_NoDirectiveTimingResourceEvaluation::CheckDependencyFesilility(Function &F)
{
    for (auto &B : F)
        for (auto &I : B)
            if (CallInst *CI = dyn_cast<CallInst>(&I))
            {
                if (FunctionLatency.find(CI->getCalledFunction()) == FunctionLatency.end())
                {
                    if (CI->getCalledFunction()->getName().find("llvm.")!=std::string::npos)
                    {
                        timingBase tmp(0,0,1,clock_period);
                        FunctionLatency[CI->getCalledFunction()] = tmp;                    
                    }
                    else
                        return false;
                }
            }
    return true;
}

char HI_NoDirectiveTimingResourceEvaluation::ID = 0;  // the ID for pass should be initialized but the value does not matter, since LLVM uses the address of this variable as label instead of its value.

// introduce the dependence of Pass
void HI_NoDirectiveTimingResourceEvaluation::getAnalysisUsage(AnalysisUsage &AU) const {
    AU.setPreservesAll();
    AU.addRequired<LoopInfoWrapperPass>();
    AU.addRequiredTransitive<ScalarEvolutionWrapperPass>();
    
    // AU.addRequired<ScalarEvolutionWrapperPass>();
    // AU.addRequired<LoopInfoWrapperPass>();
    // AU.addPreserved<LoopInfoWrapperPass>();
    AU.addRequired<LoopAccessLegacyAnalysis>();
    AU.addRequired<DominatorTreeWrapperPass>();
    // AU.addPreserved<DominatorTreeWrapperPass>();
    AU.addRequired<OptimizationRemarkEmitterWrapperPass>();
    // AU.addRequiredTransitive<polly::DependenceInfoWrapperPass>();
    // AU.addRequired<LoopInfoWrapperPass>();
    // AU.addRequiredTransitive<polly::ScopInfoWrapperPass>();
    // AU.addRequired<polly::PolyhedralInfo>();
    // AU.addPreserved<GlobalsAAWrapperPass>();
    
}


std::string HI_NoDirectiveTimingResourceEvaluation::demangeFunctionName(std::string mangled_name)
{
    std::string demangled_name;

    // demangle the function
    if (mangled_name.find("_Z")==std::string::npos)
        demangled_name = mangled_name;
    else
        {
            std::stringstream iss(mangled_name);
            iss.ignore(1, '_');iss.ignore(1, 'Z');
            int len; iss >> len; while (len--) {char tc;iss>>tc;demangled_name+=tc;}
        }
    return demangled_name;
}

void HI_NoDirectiveTimingResourceEvaluation::AnalyzeFunctions(Module &M)
{
    bool all_processed = 0;
    while (!all_processed)
    {
        all_processed = 1;
        for (auto &F : M)
        {   
            *Evaluating_log << "CHECKING FUNCTION "<< F.getName() <<"\n";
            Evaluating_log->flush();
            if (FunctionLatency.find(&F) != FunctionLatency.end())
            {
                continue;
            }
            else
            {                
                if (F.getName().find("llvm.")!=std::string::npos)
                {
                    timingBase tmp(0,0,1,clock_period);
                    FunctionLatency[&F] = tmp;       
                    continue;             
                }
                all_processed = 0;
                if (CheckDependencyFesilility(F))
                {
                    LAA = &getAnalysis<LoopAccessLegacyAnalysis>(F);
                    LI = &getAnalysis<LoopInfoWrapperPass>(F).getLoopInfo();
                    SE = &getAnalysis<ScalarEvolutionWrapperPass>(F).getSE();  
                    getLoopBlockMap(&F);
                    analyzeFunction(&F);
                }
            }
            
        }
    }
}

void HI_NoDirectiveTimingResourceEvaluation::analyzeTopFunction(Module &M)
{
    for (auto &F : M)
    {
        std::string mangled_name = F.getName();
        std::string demangled_name;
        demangled_name = demangeFunctionName(mangled_name);
        mangled_name = "find function " + mangled_name + "and its demangled name is : " + demangled_name;
        print_info(mangled_name.c_str());
        if (demangled_name == top_function_name)
        {
            *Evaluating_log << "Top Function: "<< F.getName() <<" is found";
            topFunctionFound = 1;
            top_function_latency = analyzeFunction(&F).latency;
            *Evaluating_log << "Done latency evaluation of top function: "<< F.getName() <<" and its latency is "<< top_function_latency << "\n\n\n";
            std::string printOut("");
            FunctionResource[&F] = FunctionResource[&F] + BRAM_MUX_Evaluate();
            // print out the information of top function in terminal
            printOut = "Done latency evaluation of top function: [" + demangled_name + "] and its latency is " + std::to_string(top_function_latency) + " and its resource cost is [DSP=" + std::to_string(FunctionResource[&F].DSP) + ", FF=" + std::to_string(FunctionResource[&F].FF) + ", LUT=" + std::to_string(FunctionResource[&F].LUT) + "]";
            print_info(printOut);
        }
    }
}

void HI_NoDirectiveTimingResourceEvaluation::TraceMemoryDeclarationinModule(Module &M)
{
    for (auto &F : M)
    {
        std::string mangled_name = F.getName();
        std::string demangled_name;
        demangled_name = demangeFunctionName(mangled_name);
        findMemoryDeclarationin(&F, demangled_name == top_function_name);        
    }
}