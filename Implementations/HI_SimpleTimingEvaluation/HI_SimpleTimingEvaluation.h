#ifndef _HI_SimpleTimingEvaluation
#define _HI_SimpleTimingEvaluation
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
#include "llvm/ADT/SmallVector.h"
#include <set>
#include <sys/time.h>

using namespace llvm;


// Pass for simple evluation of the latency of the top function, without considering HLS directives
class HI_SimpleTimingEvaluation : public ModulePass {
public:

    // Pass for simple evluation of the latency of the top function, without considering HLS directives
    HI_SimpleTimingEvaluation(const char* evaluating_log_name, const char* top_function) : ModulePass(ID) 
    {
        BlockEvaluated.clear();
        LoopEvaluated.clear();
        FunctionEvaluated.clear();
        Loop_id.clear();
        Loop_Counter = 0;
        Evaluating_log = new raw_fd_ostream(evaluating_log_name, ErrInfo, sys::fs::F_None);
        top_function_name = std::string(top_function);
    } // define a pass, which can be inherited from ModulePass, LoopPass, FunctionPass and etc.
    ~HI_SimpleTimingEvaluation()
    {
        for (auto ele : Loop2Blocks) 
        {
            delete ele.second;
        }
        for (auto ele : Block2Loops) 
        {
            delete ele.second;
        }
        Evaluating_log->flush(); delete Evaluating_log;
    }

    virtual bool doInitialization(Module &M)
    {
        
        print_status("Initilizing HI_SimpleTimingEvaluation pass.");
        Loop_id.clear();
        LoopLatency.clear();
        BlockLatency.clear();
        FunctionLatency.clear();
        BlockEvaluated.clear();
        Func_BlockEvaluated.clear();
        LoopEvaluated.clear();
        FunctionEvaluated.clear();
        InstructionEvaluated.clear();
        BlockVisited.clear();
        Func_BlockVisited.clear();

        Function2OuterLoops.clear();
        Block2EvaluatedLoop.clear();
        BlockCriticalPath_inLoop.clear();
        tmp_BlockCriticalPath_inFunc.clear();
        tmp_LoopCriticalPath_inFunc.clear();
        tmp_BlockCriticalPath_inLoop.clear();
        tmp_SubLoop_CriticalPath.clear();
        InstructionCriticalPath_inBlock.clear();

        return false;
        
    }

    void getAnalysisUsage(AnalysisUsage &AU) const;
    virtual bool runOnModule(Module &M); 

    // check whether the block is in some loops
    bool isInLoop(BasicBlock *BB); 

    // evaluatate the latency of a outer loop, which could be a nested one
    double getOuterLoopLatency(Loop *outerL); 

    // get the most outer loop which contains the block, treat the loop as a node for the evaluation of latency
    Loop* getOuterLoopOfBlock(BasicBlock* B);

    // find the inner unevaluated loop for processing
    Loop* getInnerUnevaluatedLoop(Loop* outerL);

    // evaluate a loop in which all the children loops have been evauluated
    double getLoopLatency_InnerChecked(Loop *L); 
    
    static char ID;

    int Loop_Counter;

    double top_function_latency = 0.0;

    std::map<Loop*, int> Loop_id;

    // the latency of each loop
    std::map<Loop*, double> LoopLatency;

    // the latency of each block
    std::map<BasicBlock*, double> BlockLatency;

    // the latency of each function
    std::map<Function*, double> FunctionLatency;

    // record whether the component is evaluated
    std::set<BasicBlock*> BlockEvaluated;
    std::set<BasicBlock*> Func_BlockEvaluated;
    std::set<Loop*> LoopEvaluated;
    std::set<Function*> FunctionEvaluated;
    std::set<Instruction*> InstructionEvaluated;
    std::set<BasicBlock*> BlockVisited;
    std::set<BasicBlock*> Func_BlockVisited;

    // record the information of the processing
    raw_ostream *Evaluating_log;
    std::error_code ErrInfo;

    // the pass requires a specified top_function name
    std::string top_function_name;

    // record the relations between loops and blocks
    std::map<Loop*, std::vector<BasicBlock*>*> Loop2Blocks;
    std::map<BasicBlock*, std::vector<Loop*>*> Block2Loops;

    // record the list of outer loops for functions
    std::map<Function*, std::vector<Loop*> > Function2OuterLoops;

    // record which evaluated loop the block is belong to, so the pass can directly trace to the loop for the latency
    std::map<BasicBlock*, Loop*> Block2EvaluatedLoop;

    // record the critical path from the loop header to the end of the specific block
    std::map<Loop*, std::map<BasicBlock*, double> > BlockCriticalPath_inLoop;

    // record the critical path to the end of block in the function
    std::map<BasicBlock*, double> tmp_BlockCriticalPath_inFunc;

    // record the critical path to the end of loops in the function
    std::map<Loop*, double> tmp_LoopCriticalPath_inFunc;

    // record the critical path to the end of sub-loops in the loop
    std::map<BasicBlock*, double> tmp_BlockCriticalPath_inLoop;

    // record the critical path from the outter loop header to the end of the specific sub-loop
    std::map<Loop*, double> tmp_SubLoop_CriticalPath;

    // record the critical path from the block entry to the end of the specific instruction
    std::map<BasicBlock*, std::map<Instruction*, double> > InstructionCriticalPath_inBlock;

    // get the function latency
    double getFunctionLatency(Function* F);

    // get the function critical path by traversing the blocks based on DFS
    void getFunctionLatency_traverseFromEntryToExiting(double tmp_critical_path, Function *F, BasicBlock* curBlock);

    // get the loop latency by traversing from the header to the exiting blocks
    void LoopLatencyEvaluation_traversFromHeaderToExitingBlocks(double tmp_critical_path,  Loop* L, BasicBlock *curBlock);

    // mark the block in loop with latency by traversing from the header to the exiting blocks
    void MarkBlock_traversFromHeaderToExitingBlocks(double total_latency, Loop* L, BasicBlock *curBlock);

    // evaluate the block latency by traversing the instructions
    double BlockLatencyEvaluation(BasicBlock *B);

    // check whether the instruction is in the block
    bool BlockContain(BasicBlock *B, Instruction *I);

    // get the latency of a specific instruction
    double getInstructionLatency(Instruction *I);

    // check whether all the sub-function are evaluated
    bool CheckDependencyFesilility(Function &F);

    // get the relationship between loops and blocks
    void getLoopBlockMap(Function* F);

    // some LLVM analysises could be involved
    ScalarEvolution *SE;
    LoopInfo *LI;
    LoopAccessLegacyAnalysis *LAA;

    bool topFunctionFound = 0;
  //  std::map<>


    
/// Timer

    struct timeval tv_begin;
    struct timeval tv_end;

};
#endif
