#include "HI_Polly_Info.h"
#include "HI_print.h"
#include "polly/PolyhedralInfo.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Pass.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/raw_ostream.h"

#include <ios>
#include <stdio.h>
#include <stdlib.h>
#include <string>

using namespace llvm;

bool HI_Polly_Info::runOnFunction(
    Function &F) // The runOnModule declaration will overide the virtual one in ModulePass, which
                 // will be executed for each Module.
{

    auto &SE = getAnalysis<ScalarEvolutionWrapperPass>().getSE();
    auto &LI = getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
    auto *LAA = &getAnalysis<LoopAccessLegacyAnalysis>();
    auto *PI = &getAnalysis<polly::PolyhedralInfo>();
    return false;
}

char HI_Polly_Info::ID =
    0; // the ID for pass should be initialized but the value does not matter, since LLVM uses the
       // address of this variable as label instead of its value.

void HI_Polly_Info::getAnalysisUsage(AnalysisUsage &AU) const
{
    AU.setPreservesAll();
    AU.addRequired<LoopInfoWrapperPass>();
    AU.addRequired<ScalarEvolutionWrapperPass>();

    // AU.addRequired<ScalarEvolutionWrapperPass>();
    // AU.addRequired<LoopInfoWrapperPass>();
    // AU.addPreserved<LoopInfoWrapperPass>();
    AU.addRequired<LoopAccessLegacyAnalysis>();
    AU.addRequired<DominatorTreeWrapperPass>();
    //  AU.addPreserved<DominatorTreeWrapperPass>();
    AU.addRequired<OptimizationRemarkEmitterWrapperPass>();
    AU.addRequiredTransitive<polly::DependenceInfoWrapperPass>();
    AU.addRequired<LoopInfoWrapperPass>();
    AU.addRequiredTransitive<polly::ScopInfoWrapperPass>();
    AU.addRequired<polly::PolyhedralInfo>();
    // AU.addPreserved<GlobalsAAWrapperPass>();
}
