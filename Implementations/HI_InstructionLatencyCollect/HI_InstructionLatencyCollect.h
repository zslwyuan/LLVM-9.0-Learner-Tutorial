#ifndef _HI_InstructionLatencyCollect
#define _HI_InstructionLatencyCollect
// related headers should be included.
#include "HI_print.h"
#include "polly/DependenceInfo.h"
#include "polly/LinkAllPasses.h"
#include "polly/Options.h"
#include "polly/PolyhedralInfo.h"
#include "polly/ScopInfo.h"
#include "llvm/ADT/SmallVector.h"
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
#include <set>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sys/time.h>

using namespace llvm;

// Pass for simple evluation of the latency of the top function, without considering HLS directives
class HI_InstructionLatencyCollect : public FunctionPass
{
  public:
    // Pass for simple evluation of the latency of the top function, without considering HLS
    // directives
    HI_InstructionLatencyCollect(const char *evaluating_log_name, const char *top_function,
                                 std::map<Loop *, std::vector<BasicBlock *> *> *L2Bs,
                                 std::map<BasicBlock *, std::vector<Loop *> *> *B2Ls)
        : FunctionPass(ID)
    {
        BlockEvaluated.clear();
        LoopEvaluated.clear();
        FunctionEvaluated.clear();
        Loop_id.clear();
        Loop_Counter = 0;
        Evaluating_log = new raw_fd_ostream(evaluating_log_name, ErrInfo, sys::fs::F_None);
        top_function_name = std::string(top_function);
        Loop2Blocks = L2Bs;
        Block2Loops = B2Ls;
    } // define a pass, which can be inherited from ModulePass, LoopPass, FunctionPass and etc.
    ~HI_InstructionLatencyCollect()
    {
        Evaluating_log->flush();
        delete Evaluating_log;
    }

    virtual bool doInitialization(Module &M)
    {

        print_status("Initilizing HI_InstructionLatencyCollect pass.");
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
    virtual bool runOnFunction(Function &F);

    static char ID;

    int Loop_Counter;

    double top_function_latency = 0.0;

    std::map<Loop *, int> Loop_id;

    // the latency of each loop
    std::map<Loop *, double> LoopLatency;

    // the latency of each block
    std::map<BasicBlock *, double> BlockLatency;

    // the latency of each function
    std::map<Function *, double> FunctionLatency;

    // record whether the component is evaluated
    std::set<BasicBlock *> BlockEvaluated;
    std::set<BasicBlock *> Func_BlockEvaluated;
    std::set<Loop *> LoopEvaluated;
    std::set<Function *> FunctionEvaluated;
    std::set<Instruction *> InstructionEvaluated;
    std::set<BasicBlock *> BlockVisited;
    std::set<BasicBlock *> Func_BlockVisited;

    // record the information of the processing
    raw_ostream *Evaluating_log;
    std::error_code ErrInfo;

    // the pass requires a specified top_function name
    std::string top_function_name;

    // record the relations between loops and blocks
    std::map<Loop *, std::vector<BasicBlock *> *> *Loop2Blocks;
    std::map<BasicBlock *, std::vector<Loop *> *> *Block2Loops;

    // record the list of outer loops for functions
    std::map<Function *, std::vector<Loop *>> Function2OuterLoops;

    // record which evaluated loop the block is belong to, so the pass can directly trace to the
    // loop for the latency
    std::map<BasicBlock *, Loop *> Block2EvaluatedLoop;

    // record the critical path from the loop header to the end of the specific block
    std::map<Loop *, std::map<BasicBlock *, double>> BlockCriticalPath_inLoop;

    // record the critical path to the end of block in the function
    std::map<BasicBlock *, double> tmp_BlockCriticalPath_inFunc;

    // record the critical path to the end of loops in the function
    std::map<Loop *, double> tmp_LoopCriticalPath_inFunc;

    // record the critical path to the end of sub-loops in the loop
    std::map<BasicBlock *, double> tmp_BlockCriticalPath_inLoop;

    // record the critical path from the outter loop header to the end of the specific sub-loop
    std::map<Loop *, double> tmp_SubLoop_CriticalPath;

    // record the critical path from the block entry to the end of the specific instruction
    std::map<BasicBlock *, std::map<Instruction *, double>> InstructionCriticalPath_inBlock;

    // some LLVM analysises could be involved
    ScalarEvolution *SE;
    LoopInfo *LI;
    LoopAccessLegacyAnalysis *LAA;

    bool topFunctionFound = 0;
    //  std::map<>
};
#endif
