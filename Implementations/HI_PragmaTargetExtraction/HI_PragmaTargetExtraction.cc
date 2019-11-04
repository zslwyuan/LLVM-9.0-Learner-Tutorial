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


bool HI_PragmaTargetExtraction::runOnModule(Module &M) // The runOnFunction declaration will overide the virtual one in ModulePass, which will be executed for each Function.
{    
    gettimeofday (&tv_begin, NULL);
    if (DEBUG) *loopTarget_log << " ======================= the module begin =======================\n";
    if (DEBUG) *loopTarget_log << M;
    if (DEBUG) *loopTarget_log << " ======================= the module end =======================\n";

    // analyze BRAM accesses in the module before any other analysis
    bool changed = TraceMemoryDeclarationAndAnalyzeAccessinModule(M);

    gettimeofday (&tv_end, NULL);
    print_status("done HI_PragmaTargetExtraction TraceMemoryDeclarationAndAnalyzeAccessinModule: " 
        + std::to_string((double)(tv_end.tv_sec-tv_begin.tv_sec)+(double)(tv_end.tv_usec-tv_begin.tv_usec)/1000000.0) + " s");


    return changed;
}


char HI_PragmaTargetExtraction::ID = 0;  // the ID for pass should be initialized but the value does not matter, since LLVM uses the address of this variable as label instead of its value.

// introduce the dependence of Pass
void HI_PragmaTargetExtraction::getAnalysisUsage(AnalysisUsage &AU) const {

    AU.setPreservesAll();
    AU.addRequiredTransitive<LoopInfoWrapperPass>();
    AU.addRequiredTransitive<ScalarEvolutionWrapperPass>();
}


// analyze BRAM accesses in the module before any other analysis
bool HI_PragmaTargetExtraction::TraceMemoryDeclarationAndAnalyzeAccessinModule(Module &M)
{
    bool changed = 0;
    for (auto &F : M)
    {
        if (F.getName().find("llvm.")!=std::string::npos) // bypass the "llvm.xxx" functions..
            continue;
        if (F.getName().find("HIPartitionMux")!=std::string::npos) // bypass the "llvm.xxx" functions..
            continue;
        std::string mangled_name = F.getName();
        std::string demangled_name;
        demangled_name = demangleFunctionName(mangled_name);
        findMemoryDeclarationAndAnalyzeAccessin(&F, demangled_name == top_function_name && F.getName().find(".") == std::string::npos); 

    }

    for (auto &F : M)
    {
        if (F.getName().find("llvm.")!=std::string::npos) // bypass the "llvm.xxx" functions..
            continue;
        if (F.getName().find("HIPartitionMux")!=std::string::npos) // bypass the "llvm.xxx" functions..
            continue;

        LI = &getAnalysis<LoopInfoWrapperPass>(F).getLoopInfo();
        SE = &getAnalysis<ScalarEvolutionWrapperPass>(F).getSE(); 
        
        checkLoopsIn(&F);
    }

    return false;
}
