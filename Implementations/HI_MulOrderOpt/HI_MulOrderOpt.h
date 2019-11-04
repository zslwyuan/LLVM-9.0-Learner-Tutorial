#ifndef _HI_MulOrderOpt
#define _HI_MulOrderOpt
// related headers should be included.
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Pass.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/ADT/APInt.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/DepthFirstIterator.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/MemoryBuiltins.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Transforms/Utils/Local.h"
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
#include "llvm/IR/Module.h"
#include "llvm/IR/PatternMatch.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/User.h"
#include "llvm/IR/Value.h"
#include "llvm/Pass.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Transforms/Scalar.h"
#include "HI_print.h"
#include <stdio.h>
#include <string>
#include <ios>
#include <stdlib.h>
#include <map>
#include <set>
#include <vector>
#include <sstream>
#include "llvm/Transforms/Utils/Local.h"
#include <sys/time.h>
#include <queue>
#include <cstdlib>
using namespace llvm;

class HI_MulOrderOpt : public FunctionPass {
public:
    HI_MulOrderOpt(const char* MulOrderOptLog_Name ) : FunctionPass(ID) 
    {
        Instruction_Counter = 0;
        Function_Counter = 0;
        BasicBlock_Counter = 0;
        Loop_Counter = 0;
        callCounter = 0;
        MulOrderOptLog = new raw_fd_ostream(MulOrderOptLog_Name, ErrInfo, sys::fs::F_None);
        tmp_stream = new raw_string_ostream(tmp_stream_str);
    } // define a pass, which can be inherited from ModulePass, LoopPass, FunctionPass and etc.

    ~HI_MulOrderOpt()
    {

        MulOrderOptLog->flush(); delete MulOrderOptLog;
        tmp_stream->flush(); delete tmp_stream;
    }

    virtual bool doInitialization(llvm::Module &M)
    {
        print_status("Initilizing HI_MulOrderOpt pass.");  

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

    void recursiveGetMulOpAndCounter(Value *MulI);

    Value* recursiveMul(std::priority_queue<std::pair<int, Value*>, std::vector<std::pair<int, Value*>>, cmp_mulorder> cur_heap, int tot_cnt, IRBuilder<> &Builder);

    
    int callCounter;
    int Instruction_Counter;
    int Function_Counter;
    int BasicBlock_Counter;
    int Loop_Counter;

    Function* TargeFunction;

    struct cmp_mulorder{
        bool operator()(std::pair<int, Value*> a, std::pair<int, Value*> b){
            if(a.first == b.first)	return a.second<b.second;
            return a.first<b.first;
        }
    };

    std::priority_queue<std::pair<int, Value*>, std::vector<std::pair<int, Value*>>, cmp_mulorder> heap_opCnt;
    std::map<MulOperator*, std::map<Value*, int>> op2Cnt;
    std::set<Value*> generatedI;
    

    std::error_code ErrInfo;
    raw_ostream *MulOrderOptLog;

    raw_string_ostream *tmp_stream;
    std::string tmp_stream_str;

    
/// Timer

    struct timeval tv_begin;
    struct timeval tv_end;
};



#endif
