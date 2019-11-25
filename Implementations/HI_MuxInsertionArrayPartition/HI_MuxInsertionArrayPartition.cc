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


bool HI_MuxInsertionArrayPartition::runOnModule(Module &M) // The runOnFunction declaration will overide the virtual one in ModulePass, which will be executed for each Function.
{    
    print_status("Running HI_MuxInsertionArrayPartition pass."); 
    gettimeofday (&tv_begin, NULL);
    if (DEBUG) *BRAM_log << " ======================= the module begin =======================\n";
    if (DEBUG) *BRAM_log << M;
    if (DEBUG) *BRAM_log << " ======================= the module end =======================\n";

    // analyze BRAM accesses in the module before any other analysis
    bool changed = TraceMemoryDeclarationAndAnalyzeAccessinModule(M);

    if (muxWithMoreThan32)
        return true;

    // gettimeofday (&tv_end, NULL);
    // print_status("done HI_MuxInsertionArrayPartition TraceMemoryDeclarationAndAnalyzeAccessinModule: " 
    //     + std::to_string((double)(tv_end.tv_sec-tv_begin.tv_sec)+(double)(tv_end.tv_usec-tv_begin.tv_usec)/1000000.0) + " s");


    return changed;
}


char HI_MuxInsertionArrayPartition::ID = 0;  // the ID for pass should be initialized but the value does not matter, since LLVM uses the address of this variable as label instead of its value.

// introduce the dependence of Pass
void HI_MuxInsertionArrayPartition::getAnalysisUsage(AnalysisUsage &AU) const {

    AU.setPreservesAll();
    AU.addRequiredTransitive<LoopInfoWrapperPass>();
    AU.addRequiredTransitive<ScalarEvolutionWrapperPass>();
}


// analyze BRAM accesses in the module before any other analysis
bool HI_MuxInsertionArrayPartition::TraceMemoryDeclarationAndAnalyzeAccessinModule(Module &M)
{
    for (auto &it  : M.global_values())
    {
        if (auto GV = dyn_cast<GlobalVariable>(&it))
        {
            if (DEBUG) *ArrayLog << it << " is a global variable\n";
            if (DEBUG) *ArrayLog << "  get array information of [" << it.getName() << "] from argument and its address=" << &it << "\n";
            Target2ArrayInfo[&it]=getArrayInfo(&it);
            TraceAccessForTarget(&it,&it);   
            Instruction2Target[&it].push_back(&it);      
            if (DEBUG) *ArrayLog << Target2ArrayInfo[&it] << "\n";
            if (DEBUG) ArrayLog->flush();
        }        
    }
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
        TraceMemoryAccessinFunction(F);
    }
    for (auto &F : M)
    {
        if (F.getName().find("llvm.")!=std::string::npos) // bypass the "llvm.xxx" functions..
            continue;
        if (F.getName().find("HIPartitionMux")!=std::string::npos) // bypass the "llvm.xxx" functions..
            continue;
        LI = &getAnalysis<LoopInfoWrapperPass>(F).getLoopInfo();
        SE = &getAnalysis<ScalarEvolutionWrapperPass>(F).getSE(); 
        changed |= ArrayAccessCheckForFunction(&F);
        if (muxWithMoreThan32)
            return true;
    }
    return changed;
}
