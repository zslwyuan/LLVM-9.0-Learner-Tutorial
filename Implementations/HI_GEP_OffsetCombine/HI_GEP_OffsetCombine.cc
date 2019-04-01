#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Pass.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/raw_ostream.h"
#include "HI_print.h"
#include "HI_GEP_OffsetCombine.h"
#include "polly/PolyhedralInfo.h"

#include <stdio.h>
#include <string>
#include <ios>
#include <stdlib.h>

using namespace llvm;


bool HI_GEP_OffsetCombine::runOnFunction(Function &F) 
{
    if (F.getName().find("llvm.")!=std::string::npos)
    {
        return false;             
    }

    const DataLayout &DL = F.getParent()->getDataLayout();
    //  if (skipFunction(F))
    //     return false;

    if (DisableSeparateConstOffsetFromGEP)
        return false;

    DT = &getAnalysis<DominatorTreeWrapperPass>().getDomTree();
    SE = &getAnalysis<ScalarEvolutionWrapperPass>().getSE();
    LI = &getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
    // TLI = &getAnalysis<TargetLibraryInfoWrapperPass>().getTLI();
    bool Changed = false;
    for (BasicBlock &B : F) 
    {        
        for (Instruction &I : B) 
        if (GetElementPtrInst *GEP = dyn_cast<GetElementPtrInst>(&I))
        {
            Changed |= 1;//splitGEP(GEP);
        }
    // No need to split GEP ConstantExprs because all its indices are constant
    // already.
    }
    return Changed;
}

char HI_GEP_OffsetCombine::ID = 0;  // the ID for pass should be initialized but the value does not matter, since LLVM uses the address of this variable as label instead of its value.

// introduce the dependence of Pass
void HI_GEP_OffsetCombine::getAnalysisUsage(AnalysisUsage &AU) const 
{
    AU.addRequired<DominatorTreeWrapperPass>();
    AU.addRequired<ScalarEvolutionWrapperPass>();
    AU.addRequired<TargetTransformInfoWrapperPass>();
    AU.addRequired<LoopInfoWrapperPass>();
    AU.setPreservesCFG();
    
}




