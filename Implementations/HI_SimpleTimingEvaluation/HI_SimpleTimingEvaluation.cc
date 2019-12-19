#include "HI_SimpleTimingEvaluation.h"
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

bool HI_SimpleTimingEvaluation::runOnModule(
    Module &M) // The runOnFunction declaration will overide the virtual one in ModulePass, which
               // will be executed for each Function.
{
    bool all_processed = 0;
    while (!all_processed)
    {
        all_processed = 1;
        for (auto &F : M)
        {

            if (FunctionLatency.find(&F) != FunctionLatency.end())
            {
                continue;
            }
            else
            {
                if (F.getName().find("llvm.") != std::string::npos)
                {
                    FunctionLatency[&F] = 0;
                    continue;
                }
                all_processed = 0;
                if (CheckDependencyFesilility(F))
                {
                    SE = &getAnalysis<ScalarEvolutionWrapperPass>(F).getSE();
                    LI = &getAnalysis<LoopInfoWrapperPass>(F).getLoopInfo();
                    LAA = &getAnalysis<LoopAccessLegacyAnalysis>(F);
                    getLoopBlockMap(&F);
                    getFunctionLatency(&F);
                }
            }
        }
    }

    for (auto &F : M)
    {

        std::string mangled_name = F.getName();
        std::string demangled_name;

        // demangle the function
        if (mangled_name.find("_Z") == std::string::npos)
            demangled_name = mangled_name;
        else
        {
            std::stringstream iss(mangled_name);
            iss.ignore(1, '_');
            iss.ignore(1, 'Z');
            int len;
            iss >> len;
            while (len--)
            {
                char tc;
                iss >> tc;
                demangled_name += tc;
            }
        }

        mangled_name =
            "find function " + mangled_name + "and its demangled name is : " + demangled_name;
        print_info(mangled_name.c_str());
        if (demangled_name == top_function_name)
        {
            *Evaluating_log << "Top Function: " << F.getName() << " is found";
            topFunctionFound = 1;
            top_function_latency = getFunctionLatency(&F);
            *Evaluating_log << "Done latency evaluation of top function: " << F.getName()
                            << " and its latency is " << top_function_latency << "\n\n\n";
        }
    }

    return false;
}

bool HI_SimpleTimingEvaluation::CheckDependencyFesilility(Function &F)
{
    for (auto &B : F)
        for (auto &I : B)
            if (CallInst *CI = dyn_cast<CallInst>(&I))
            {
                if (FunctionLatency.find(CI->getCalledFunction()) == FunctionLatency.end())
                {
                    return false;
                }
            }
    return true;
}

char HI_SimpleTimingEvaluation::ID =
    0; // the ID for pass should be initialized but the value does not matter, since LLVM uses the
       // address of this variable as label instead of its value.

// introduce the dependence of Pass
void HI_SimpleTimingEvaluation::getAnalysisUsage(AnalysisUsage &AU) const
{
    AU.setPreservesAll();
    AU.addRequired<LoopInfoWrapperPass>();
    AU.addRequired<ScalarEvolutionWrapperPass>();

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
