#ifndef _HI_HI_POLLY_INFO
#define _HI_HI_POLLY_INFO
// related headers should be included.
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Pass.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/raw_ostream.h"
#include "HI_print.h"
#include <stdio.h>
#include <string>
#include <ios>
#include <stdlib.h>
#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/GraphWriter.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/LoopPass.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IRReader/IRReader.h"
#include <bits/stl_map.h>
#include "llvm/ADT/Statistic.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/PassAnalysisSupport.h"
#include "llvm/Transforms/Utils/LoopUtils.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/LoopAccessAnalysis.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/Transforms/Utils/LoopUtils.h"
#include "llvm/Transforms/Utils/LoopVersioning.h"
#include "llvm/Transforms/Utils/ValueMapper.h"
#include "polly/PolyhedralInfo.h"
#include "polly/DependenceInfo.h"
#include "polly/LinkAllPasses.h"
#include "polly/Options.h"
#include "polly/ScopInfo.h"
#include <sys/time.h>

using namespace llvm;

class HI_Polly_Info : public FunctionPass {
public:
    HI_Polly_Info(const char* Loop_out_file) : FunctionPass(ID) 
    {
        Loop_Counter = 0;
        Loop_out = new raw_fd_ostream(Loop_out_file, ErrInfo, sys::fs::F_None);
    } // define a pass, which can be inherited from ModulePass, LoopPass, FunctionPass and etc.
    ~HI_Polly_Info()
    {
        Loop_out->flush(); delete Loop_out;
    }
    virtual bool doInitialization(Module &M)
    {
        print_status("Initilizing HI_Polly_Info pass.");
        return false;      
    }
    void getAnalysisUsage(AnalysisUsage &AU) const;
    virtual bool runOnFunction(Function &F); 
    
    static char ID;

    int Loop_Counter;

    std::map<Loop*, int> Loop_id;
    std::map<Loop*, std::vector<BasicBlock*> *> Loop2Blocks;
    std::error_code ErrInfo;
    raw_ostream *Loop_out;


    
/// Timer

    struct timeval tv_begin;
    struct timeval tv_end;

};
#endif
