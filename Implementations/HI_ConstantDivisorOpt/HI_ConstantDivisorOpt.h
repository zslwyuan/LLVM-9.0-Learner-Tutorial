#ifndef _HI_ConstantDivisorOpt
#define _HI_ConstantDivisorOpt
// related headers should be included.
#include "HI_print.h"
#include "llvm/ADT/APInt.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/DepthFirstIterator.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/MemoryBuiltins.h"
#include "llvm/Analysis/ScalarEvolution.h"
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

class HI_ConstantDivisorOpt : public FunctionPass
{
  public:
    HI_ConstantDivisorOpt(const char *DivisionOptLog_Name) : FunctionPass(ID)
    {
        Instruction_Counter = 0;
        Function_Counter = 0;
        BasicBlock_Counter = 0;
        Loop_Counter = 0;
        callCounter = 0;
        DivisionOptLog = new raw_fd_ostream(DivisionOptLog_Name, ErrInfo, sys::fs::F_None);
        tmp_stream = new raw_string_ostream(tmp_stream_str);
    } // define a pass, which can be inherited from ModulePass, LoopPass, FunctionPass and etc.

    ~HI_ConstantDivisorOpt()
    {

        DivisionOptLog->flush();
        delete DivisionOptLog;
        tmp_stream->flush();
        delete tmp_stream;
    }

    virtual bool doInitialization(llvm::Module &M)
    {
        print_status("Initilizing HI_ConstantDivisorOpt pass.");

        return false;
    }

    struct cmp_mulorder;

    // virtual bool doFinalization(Module &M)
    // {
    //     return false;
    // }

    void getAnalysisUsage(AnalysisUsage &AU) const;
    virtual bool runOnFunction(llvm::Function &F);
    static char ID;

    int callCounter;
    int Instruction_Counter;
    int Function_Counter;
    int BasicBlock_Counter;
    int Loop_Counter;

    Function *TargeFunction;

    struct cmp_mulorder
    {
        bool operator()(std::pair<int, Value *> a, std::pair<int, Value *> b)
        {
            if (a.first == b.first)
                return a.second < b.second;
            return a.first < b.first;
        }
    };

    std::priority_queue<std::pair<int, Value *>, std::vector<std::pair<int, Value *>>, cmp_mulorder> heap_opCnt;
    std::map<MulOperator *, std::map<Value *, int>> op2Cnt;
    std::set<Value *> generatedI;

    /// Generate code for signed division by a constant. Implementation follows
    /// TargetLowering::BuildSDIV that replaces division with multiplication
    /// by a magic number.
    Value *generateSignedDivisionByConstant(Value *Dividend, ConstantInt *Divisor, IRBuilder<> &Builder);

    /// Generate code for unsigned division by a constant. Implementation follows
    /// TargetLowering::BuildSDIV that replaces division with multiplication
    /// by a magic number.
    Value *generateUnsignedDivisionByConstant(Value *Dividend, ConstantInt *Divisor, IRBuilder<> &Builder);

    std::error_code ErrInfo;
    raw_ostream *DivisionOptLog;

    raw_string_ostream *tmp_stream;
    std::string tmp_stream_str;

    /// Timer

    struct timeval tv_begin;
    struct timeval tv_end;
};

#endif
