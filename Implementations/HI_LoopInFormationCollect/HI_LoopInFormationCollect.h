#ifndef _HI_LOOPINFORMATIONCOLLECT
#define _HI_LOOPINFORMATIONCOLLECT
// related headers should be included.
#include "HI_print.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/Analysis/LoopAccessAnalysis.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/LoopPass.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Pass.h"
#include "llvm/PassAnalysisSupport.h"
#include "llvm/Support/GraphWriter.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/Transforms/Utils/LoopUtils.h"
#include "llvm/Transforms/Utils/LoopVersioning.h"
#include "llvm/Transforms/Utils/ValueMapper.h"
#include <bits/stl_map.h>
#include <ios>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sys/time.h>

using namespace llvm;

class HI_LoopInFormationCollect : public LoopPass
{
  public:
    HI_LoopInFormationCollect(const char *loopfile) : LoopPass(ID)
    {
        //  this->getAsPMDataManager();
        Loop_Counter = 0;
        Loop_out = new raw_fd_ostream(loopfile, ErrInfo, sys::fs::F_None);
        // Loop2Blocks.clear();
        // Block2Loops.clear();
        // Loop_id.clear();
    } // define a pass, which can be inherited from ModulePass, LoopPass, FunctionPass and etc.
    ~HI_LoopInFormationCollect()
    {
        for (auto ele : Loop2Blocks)
        {
            delete ele.second;
        }
        for (auto ele : Block2Loops)
        {
            delete ele.second;
        }
        Loop_out->flush();
        delete Loop_out;
    }
    virtual bool doInitialization(Loop *L, LPPassManager &LPPM)
    {
        std::string tmpInfo = std::string("Initilizing HI_LoopInFormationCollect pass for Loop ") +
                              L->getName().str();
        print_status(tmpInfo.c_str());

        return false;
    }
    void printMaps()
    {
        *Loop_out << "\n=============content in maps================\n";
        for (auto ele : Loop2Blocks)
        {
            if (ele.first->isInvalid())
            {
                print_info("Pass finalization encounter invalid loop");
                errs() << "Loop-ID:" << Loop_id[ele.first]
                       << " is invalid when being printed by pass.\n";
                continue;
            }
            *Loop_out << "Loop: " << ele.first->getName() << " <--- Blocks: ";
            for (auto vec_ele : *ele.second)
            {
                *Loop_out << vec_ele->getName() << " ";
            }
            *Loop_out << "\n";
        }
        for (auto ele : Block2Loops)
        {
            *Loop_out << "Block: " << ele.first->getName() << " ---> Loops: ";
            for (auto vec_ele : *ele.second)
            {
                if (vec_ele->isInvalid())
                {
                    print_info("Pass finalization encounter invalid loop");
                    errs() << "Loop-ID:" << Loop_id[vec_ele]
                           << " is invalid when being printed by pass.\n";
                    continue;
                }
                *Loop_out << vec_ele->getName() << " ";
            }
            *Loop_out << "\n";
        }
    }
    virtual bool doFinalization()
    {
        // print out the content in maps
        // printMaps();
        return false;
    }
    void getAnalysisUsage(AnalysisUsage &AU) const;
    virtual bool runOnLoop(Loop *L, LPPassManager &) override;

    static char ID;

    int Loop_Counter;

    std::map<Loop *, int> Loop_id;
    std::map<Loop *, std::vector<BasicBlock *> *> Loop2Blocks;
    std::error_code ErrInfo;
    raw_ostream *Loop_out;
    std::map<BasicBlock *, std::vector<Loop *> *> Block2Loops;

    ScalarEvolution *SE;
    LoopInfo *LI;
    LoopAccessLegacyAnalysis *LAA;

    void solveDependency(legacy::PassManager &PM)
    {

        auto loopinfowrapperpass = new LoopInfoWrapperPass();
        PM.add(loopinfowrapperpass);

        auto scalarevolutionwrapperpass = new ScalarEvolutionWrapperPass();
        PM.add(scalarevolutionwrapperpass);

        auto loopaccesslegacyanalysis = new LoopAccessLegacyAnalysis();
        PM.add(loopaccesslegacyanalysis);

        auto dominatortreewrapperpass = new DominatorTreeWrapperPass();
        PM.add(dominatortreewrapperpass);

        auto optimizationremarkemitterwrapperpass = new OptimizationRemarkEmitterWrapperPass();
        PM.add(optimizationremarkemitterwrapperpass);
    }

    /// Timer

    struct timeval tv_begin;
    struct timeval tv_end;
};

namespace llvm
{
// void initializeHI_LoopInFormationCollectPass(PassRegistry &);
// void initializeHI_LoopInFormationCollectPassOnce(PassRegistry &);

// void initializeHI_LoopInFormationCollectPass(PassRegistry&);
// void *initializeHI_LoopInFormationCollectPassOnce(PassRegistry &Registry);
//  void initializeHI_LoopInFormationCollectPass(PassRegistry &Registry);
} // namespace llvm

#endif
