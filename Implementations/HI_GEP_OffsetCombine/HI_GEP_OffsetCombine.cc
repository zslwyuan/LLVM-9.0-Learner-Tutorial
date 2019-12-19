#include "HI_GEP_OffsetCombine.h"
#include "HI_print.h"
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

bool HI_GEP_OffsetCombine::runOnFunction(Function &F)
{
    print_status("Running HI_GEP_OffsetCombine pass.");
    if (F.getName().find("llvm.") != std::string::npos)
    {
        return false;
    }

    const DataLayout &DL = F.getParent()->getDataLayout();
    //  if (skipFunction(F))
    //     return false;

    if (DisableSeparateConstOffsetFromGEP)
        return false;

    // TLI = &getAnalysis<TargetLibraryInfoWrapperPass>().getTLI();
    bool Changed = false;
    for (BasicBlock &B : F)
    {
        for (Instruction &I : B)
            if (GetElementPtrInst *GEP = dyn_cast<GetElementPtrInst>(&I))
            {
                Changed |= 0; // splitGEP(GEP);
            }
        // No need to split GEP ConstantExprs because all its indices are constant
        // already.
    }
    return Changed;
}

char HI_GEP_OffsetCombine::ID =
    0; // the ID for pass should be initialized but the value does not matter, since LLVM uses the
       // address of this variable as label instead of its value.

// introduce the dependence of Pass
void HI_GEP_OffsetCombine::getAnalysisUsage(AnalysisUsage &AU) const
{
    AU.setPreservesCFG();
}
