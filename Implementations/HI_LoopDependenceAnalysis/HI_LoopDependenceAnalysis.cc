#include "HI_LoopDependenceAnalysis.h"
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

// Compute the dependence matrix of a loop.  Only consider perfectly
// nested, single block loops with easily analyzed array accesses.
bool HI_LoopDependenceAnalysis::runOnLoop(Loop *L, LPPassManager &LPM)
{

    // Get our Loop and Alias Analysis information...
    LI = &getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
    AA = &getAnalysis<AAResultsWrapperPass>().getAAResults();
    // iVars.clear();
    // iVarsIndex.clear();
    // nestedLoops.clear();
    // depMatrix.clear();

    // only process outermost loop
    if (L->getParentLoop())
        return false;
    NumLoopNests++;

    // now check that these loops are perfectly nested inside one another
    BasicBlock *body;

    if (!isPerfectNest(L, body))
        return false;

    // go through the basic block and make sure only loads and stores
    // access memory
    for (BasicBlock::iterator I = body->begin(), E = body->end(); I != E; I++)
    {
        switch (I->getOpcode())
        {
        case Instruction::Invoke:
        case Instruction::VAArg:
        case Instruction::Call:
            return false; // can't handle these
        }
    }

    NumPerfectLoopNests++;
    file << "Loop Nest " << NumPerfectLoopNests << "\n";
    // depMatrix = DependenceMatrix(iVars.size());

    // computeDependencies(body);

    file << "Matrix " << NumPerfectLoopNests << "\n";
    // depMatrix.print(file);
    // if(!depMatrix.isTrivial())
    // 	NumInterestingLoops++;
    return false;
}

char HI_LoopDependenceAnalysis::ID =
    0; // the ID for pass should be initialized but the value does not matter, since LLVM uses the
       // address of this variable as label instead of its value.

void HI_LoopDependenceAnalysis::getAnalysisUsage(AnalysisUsage &AU) const
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
    // AU.addPreserved<GlobalsAAWrapperPass>();
    AU.addRequiredID(LoopSimplifyID);
    AU.addRequiredTransitive<AAResultsWrapperPass>();
    AU.addPreserved<AAResultsWrapperPass>();
}

bool HI_LoopDependenceAnalysis::hasNoMemoryOps(BasicBlock *b)
{
    for (BasicBlock::iterator I = b->begin(), E = b->end(); I != E; I++)
    {
        switch (I->getOpcode())
        {
        case Instruction::Store:
        case Instruction::Invoke:
        case Instruction::VAArg:
        case Instruction::Call:
        case Instruction::Load:
            // errs() << *I << "\n";
            return false;
        }
    }
    return true;
}

bool HI_LoopDependenceAnalysis::isPerfectNest(Loop *L, BasicBlock *&body)
{
    // get induction variable
    PHINode *ivar = L->getCanonicalInductionVariable();
    if (!ivar)
        return false;
    // record ivar
    iVarsIndex[ivar] = iVars.size();
    iVars.push_back(ivar);
    nestedLoops.push_back(L);
    // check to see if we're the innermost nest
    if (L->getBlocks().size() == 1)
    {
        body = *L->block_begin();
        return true;
    }
    else
    {
        // do we have a single subloop?
        if (L->getSubLoops().size() != 1)
            return false;
        // make sure all our non-nested loop blocks are innocuous
        for (Loop::block_iterator b = L->block_begin(), e = L->block_end(); b != e; b++)
        {
            BasicBlock *block = *b;
            if (LI->getLoopFor(block) == L)
            {
                if (!hasNoMemoryOps(block))
                    return false;
            }
        }

        // recursively check subloops
        return isPerfectNest(*L->begin(), body);
    }
}
