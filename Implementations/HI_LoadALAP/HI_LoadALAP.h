#ifndef _HI_LoadALAP
#define _HI_LoadALAP
// related headers should be included.
#include "HI_print.h"
#include "llvm/ADT/APInt.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/DepthFirstIterator.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/MemoryBuiltins.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/Analysis/TargetTransformInfo.h"
#include "llvm/Analysis/ValueTracking.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constant.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/GetElementPtrTypeIterator.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PatternMatch.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/User.h"
#include "llvm/IR/Value.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Pass.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Utils/Local.h"
#include <cstdlib>
#include <ios>
#include <map>
#include <queue>
#include <set>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sys/time.h>
#include <vector>
using namespace llvm;

class HI_LoadALAP : public FunctionPass
{
  public:
    HI_LoadALAP(const char *HI_LoadALAPLog_Name, bool DEBUG = 0) : FunctionPass(ID), DEBUG(DEBUG)
    {
        HI_LoadALAPLog = new raw_fd_ostream(HI_LoadALAPLog_Name, ErrInfo, sys::fs::F_None);
        tmp_stream = new raw_string_ostream(tmp_stream_str);
    } // define a pass, which can be inherited from ModulePass, LoopPass, FunctionPass and etc.

    ~HI_LoadALAP()
    {

        HI_LoadALAPLog->flush();
        delete HI_LoadALAPLog;
        tmp_stream->flush();
        delete tmp_stream;
    }

    virtual bool doInitialization(llvm::Module &M)
    {
        print_status("Initilizing HI_LoadALAP pass.");

        return false;
    }

    bool DEBUG;

    struct cmp_mulorder;

    // virtual bool doFinalization(Module &M)
    // {
    //     return false;
    // }

    void getAnalysisUsage(AnalysisUsage &AU) const;
    virtual bool runOnFunction(llvm::Function &F);
    static char ID;

    bool tryReorderIntAdd(Instruction *AddI);

    void recursiveGetAddOperand(Value *AddI, std::vector<Value *> &nonLoadInsts, std::vector<Value *> &LoadInsts);

    bool tryReorderFloatAdd(Instruction *AddI);

    void recursiveGetFAddOperand(Value *AddI, std::vector<Value *> &nonLoadInsts, std::vector<Value *> &LoadInsts);

    bool tryReorderIntMul(Instruction *MulI);

    void recursiveGetMulOperand(Value *MulI, std::vector<Value *> &nonLoadInsts, std::vector<Value *> &LoadInsts);

    std::set<Value *> generatedI;

    Function *TargeFunction;

    std::error_code ErrInfo;
    raw_ostream *HI_LoadALAPLog;

    raw_string_ostream *tmp_stream;
    std::string tmp_stream_str;

    /// Timer

    struct timeval tv_begin;
    struct timeval tv_end;
};

#endif
