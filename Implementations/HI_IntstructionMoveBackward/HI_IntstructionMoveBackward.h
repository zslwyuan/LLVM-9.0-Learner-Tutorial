#ifndef _HI_IntstructionMoveBackward
#define _HI_IntstructionMoveBackward
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
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/Analysis/TargetTransformInfo.h"
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
#include <sys/time.h>

using namespace llvm;

class HI_IntstructionMoveBackward : public FunctionPass {
public:
    HI_IntstructionMoveBackward(const char* BackwardLog_Name, bool DEBUG=0 ) : FunctionPass(ID), DEBUG(DEBUG)
    {
        Instruction_Counter = 0;
        Function_Counter = 0;
        BasicBlock_Counter = 0;
        Loop_Counter = 0;
        callCounter = 0;
        BackwardLog = new raw_fd_ostream(BackwardLog_Name, ErrInfo, sys::fs::F_None);
        tmp_stream = new raw_string_ostream(tmp_stream_str);
    } // define a pass, which can be inherited from ModulePass, LoopPass, FunctionPass and etc.

    ~HI_IntstructionMoveBackward()
    {
        for (auto it : Block_Successors) 
        {   
            delete it.second;
        }
        for (auto it : Block_Predecessors) 
        {   
            delete it.second;
        }
        BackwardLog->flush(); delete BackwardLog;
        tmp_stream->flush(); delete tmp_stream;
    }

    virtual bool doInitialization(llvm::Module &M)
    {
        print_status("Initilizing HI_IntstructionMoveBackward pass.");  
        for (auto it : Block_Successors) 
        {   
            delete it.second;
        }
        for (auto it : Block_Predecessors) 
        {   
            delete it.second;
        }
        Function_id.clear();
        Instruction_id.clear();
        BasicBlock_id.clear();
        Loop_id.clear();
        Block_Successors.clear();
        Block_Predecessors.clear();
        

        return false;
    }

    bool DEBUG;

    // virtual bool doFinalization(Module &M)
    // {
    //     return false;
    // }

    void getAnalysisUsage(AnalysisUsage &AU) const;
    virtual bool runOnFunction(Function &M);
    static char ID;

    // print out the graph of dominator tree
    void printDominatorTree(DominatorTree &DT);

    // find the lowest unprocessed block in the dominator tree
    BasicBlock* findUnprocessedLowestBlock(DominatorTree &DT);

    // find the instructions independent with the PHI nodes
    std::vector<Instruction*> getInstructions_PhiIndependent(BasicBlock *cur_block);

    // move the Instruction to another block
    bool transferInstructionTo(Instruction* I, BasicBlock *To_B);

    // print the IRs in the function
    void printFunction(Function* F);

    int callCounter;
    int Instruction_Counter;
    int Function_Counter;
    int BasicBlock_Counter;
    int Loop_Counter;

    Function* TargeFunction;

    std::map<Function*, int> Function_id;
    std::map<Instruction*, int> Instruction_id;
    std::set<BasicBlock*> processedBlock;
    std::map<BasicBlock*, int> BasicBlock_id;
    std::map<Loop*, int> Loop_id;
    std::map<BasicBlock*, std::vector<BasicBlock*>*> Block_Successors;
    std::map<BasicBlock*, std::vector<BasicBlock*>*> Block_Predecessors;

    std::set<Value*> isInstruction_PHI_dependent;
    std::set<Value*> isPHI_Instruction_dependent;
    
    std::error_code ErrInfo;
    raw_ostream *BackwardLog;

    raw_string_ostream *tmp_stream;
    std::string tmp_stream_str;

    
/// Timer

    struct timeval tv_begin;
    struct timeval tv_end;
};



#endif
