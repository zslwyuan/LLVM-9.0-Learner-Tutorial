#ifndef _HI_FunctionInstantiation
#define _HI_FunctionInstantiation
// related headers should be included.
#include "HI_StringProcess.h"
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
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/Transforms/Utils/Local.h"
#include "llvm/Transforms/Utils/ValueMapper.h"
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

class HI_FunctionInstantiation : public ModulePass
{
  public:
    HI_FunctionInstantiation(const char *FuncInitiationLog_Name, const std::string _topFunctionName) : ModulePass(ID)
    {
        topFunctionName = _topFunctionName;
        FuncInitiationLog = new raw_fd_ostream(FuncInitiationLog_Name, ErrInfo, sys::fs::F_None);
        tmp_stream = new raw_string_ostream(tmp_stream_str);
    } // define a pass, which can be inherited from ModulePass, LoopPass, ModulePass and etc.

    ~HI_FunctionInstantiation()
    {
        FuncInitiationLog->flush();
        delete FuncInitiationLog;
    }

    std::string topFunctionName;

    std::vector<Instruction *> CallInst_List;
    std::vector<Function *> FunctionShouldBeInitiated;
    std::map<Function *, int> IntitiatedFunctionCnt;

    void getAnalysisUsage(AnalysisUsage &AU) const;

    bool runOnModule(llvm::Module &M);
    static char ID;

    // initiate a function for a specific call instruction
    void initiateFunctionFor(Instruction *CallI, Function *F);

    // collect the information of call instructions, mainly for checking the necessity of initiation
    void collectCallInsts(llvm::Module *M);

    void insertFunctionShouldBeInitiated(Function *F);

    void BFS_check_and_initiate(Function *startF, llvm::Module *M);

    void checkAndInitiateCallInstIn(Function *curF);

    void pushCalledFunctionIntoQueue(Function *curF, std::queue<Function *> &FuncQ, std::set<Function *> &FuncVisited);

    bool ShouldFunctionBeInitiated(Function *F);

    int newFuncID(Function *F);

    void replaceCallInst(Instruction *callI, Function *F);

    void BFS_checkDeadFunction(Function *startF, llvm::Module *M);

    std::error_code ErrInfo;
    raw_ostream *FuncInitiationLog;

    raw_string_ostream *tmp_stream;
    std::string tmp_stream_str;

    /// Timer

    struct timeval tv_begin;
    struct timeval tv_end;
};

#endif
