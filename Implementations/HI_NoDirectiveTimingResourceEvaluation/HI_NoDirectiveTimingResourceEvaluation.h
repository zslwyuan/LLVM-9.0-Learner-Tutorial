#ifndef _HI_NoDirectiveTimingResourceEvaluation
#define _HI_NoDirectiveTimingResourceEvaluation

#define AggressiveSchedule

#ifdef AggressiveSchedule
    #define LoadStore_Thredhold 2
    #define CertaintyRatio (7.2/8.0)
#else
    #define LoadStore_Thredhold 3.25
    #define CertaintyRatio (7.0/8.0)
#endif


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
#include "polly/PolyhedralInfo.h"
#include "polly/DependenceInfo.h"
#include "polly/LinkAllPasses.h"
#include "polly/Options.h"
#include "polly/ScopInfo.h"
#include "HI_InstructionFiles.h"
#include <set>
#include  <iostream>
#include  <fstream>
#include <ios>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <cctype>
#include <algorithm>
#include <sstream>
#include "ClockInfo.h"
using namespace llvm;


// Pass for simple evluation of the latency of the top function, without considering HLS directives
class HI_NoDirectiveTimingResourceEvaluation : public ModulePass {
public:

    // Pass for simple evluation of the latency of the top function, without considering HLS directives
    HI_NoDirectiveTimingResourceEvaluation(const char* config_file_name, const char* evaluating_log_name, const char* BRAM_log_name, const char* top_function) : ModulePass(ID) 
    {
        BlockEvaluated.clear();
        LoopEvaluated.clear();
        FunctionEvaluated.clear();
        Loop_id.clear();
        Loop_Counter = 0;
        config_file = new std::ifstream(config_file_name);
        Evaluating_log = new raw_fd_ostream(evaluating_log_name, ErrInfo, sys::fs::F_None);
        BRAM_log = new raw_fd_ostream(BRAM_log_name, ErrInfo, sys::fs::F_None);
        top_function_name = std::string(top_function);

        // get the configureation from the file, e.g. clock period
        Parse_Config();

        // load the HLS database of timing and resource
        Load_Instruction_Info();
    }
    
    ~HI_NoDirectiveTimingResourceEvaluation()
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
        
        print_status("Initilizing HI_NoDirectiveTimingResourceEvaluation pass.");
        Loop_id.clear();
        LoopLatency.clear();
        BlockLatency.clear();
        FunctionLatency.clear();
        LoopResource.clear();
        BlockResource.clear();
        FunctionResource.clear();
        BlockEvaluated.clear();
        Func_BlockEvaluated.clear();
        LoopEvaluated.clear();
        FunctionEvaluated.clear();
        InstructionEvaluated.clear();
        BlockVisited.clear();
        Func_BlockVisited.clear();
        Instruction_FFAssigned.clear();
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

    class timingBase;
    class resourceBase;

    // set the dependence of Passes
    void getAnalysisUsage(AnalysisUsage &AU) const;

    // parse the file to get configuration
    void Parse_Config();

    // load the timing and resource information for the instructions
    void Load_Instruction_Info();

    virtual bool runOnModule(Module &M); 

    // check whether the block is in some loops
    bool isInLoop(BasicBlock *BB); 

    // evaluatate the latency of a outer loop, which could be a nested one,  and return the timing information
    timingBase analyzeOuterLoop(Loop *outerL);  

    // get the most outer loop which contains the block, treat the loop as a node for the evaluation of latency
    Loop* getOuterLoopOfBlock(BasicBlock* B);

    // find the inner unevaluated loop for processing
    Loop* getInnerUnevaluatedLoop(Loop* outerL);

    // evaluate a loop in which all the children loops have been evauluated and return the timing information
    timingBase analyzeLoop_InnerChecked(Loop *L); 
    
    static char ID;

    int Loop_Counter;

    std::map<Loop*, int> Loop_id;

    // the latency of each loop
    std::map<std::string, timingBase> LoopLatency;

    // the latency of each block
    std::map<BasicBlock*, timingBase> BlockLatency;

    // the latency of each function
    std::map<Function*, timingBase> FunctionLatency;

    // the resource of each loop
    std::map<std::string, resourceBase> LoopResource;

    // the resource of each block
    std::map<BasicBlock*, resourceBase> BlockResource;

    // the resource of each function
    std::map<Function*, resourceBase> FunctionResource;

    // record whether the component is evaluated
    std::set<BasicBlock*> BlockEvaluated;
    std::set<BasicBlock*> Func_BlockEvaluated;
    std::set<Loop*> LoopEvaluated;
    std::set<Function*> FunctionEvaluated;
    std::set<Instruction*> InstructionEvaluated;
    std::set<Instruction*> Instruction_FFAssigned;
    std::set<BasicBlock*> BlockVisited;
    std::set<BasicBlock*> Func_BlockVisited;
    std::set<Value*> ValueVisited;

    // record the information of the processing
    raw_ostream *Evaluating_log;
    
    // record the information of the BRAMs and related accesses to BRAMs
    raw_ostream *BRAM_log;

    std::error_code ErrInfo;
    std::ifstream *config_file;

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
    std::map<Loop*, std::map<BasicBlock*, timingBase> > BlockCriticalPath_inLoop;

    // record the critical path to the end of block in the function
    std::map<BasicBlock*, timingBase> tmp_BlockCriticalPath_inFunc;

    // record the critical path to the end of loops in the function
    std::map<Loop*, timingBase> tmp_LoopCriticalPath_inFunc;

    // record the critical path to the end of sub-loops in the loop
    std::map<BasicBlock*, timingBase> tmp_BlockCriticalPath_inLoop;

    // record the critical path from the outter loop header to the end of the specific sub-loop
    std::map<Loop*, timingBase> tmp_SubLoop_CriticalPath;

    // record the critical path from the block entry to the end of the specific instruction
    std::map<BasicBlock*, std::map<Instruction*, timingBase> > InstructionCriticalPath_inBlock;

    // demangle the name of functions
    std::string demangeFunctionName(std::string mangled_name);

    // get the latency of functions in the module  and compute the resource cost
    void AnalyzeFunctions(Module &M);

    // get the latency of TopFunction Latency
    void analyzeTopFunction(Module &M);
 
    // get the function latency  and compute the resource cost
    timingBase analyzeFunction(Function* F);

    // return the resource cost of the function
    resourceBase getFunctionResource(Function *F);

    // get how many state needed for the function
    int getFunctionStageNum(timingBase tmp_critical_path, Function *F, BasicBlock* curBlock);

    // get how many state needed for the application
    int getTotalStateNum(Module &M);

    // get the function critical path by traversing the blocks based on DFS and compute the resource cost
    void analyzeFunction_traverseFromEntryToExiting(timingBase tmp_critical_path, Function *F, BasicBlock* curBlock, resourceBase &resourceAccumulator);

    // get the loop latency by traversing from the header to the exiting blocks and evluation resource
    void LoopLatencyResourceEvaluation_traversFromHeaderToExitingBlocks(timingBase tmp_critical_path,  Loop* L, BasicBlock *curBlock, resourceBase &resourceAccumulator);

    // mark the block in loop with latency by traversing from the header to the exiting blocks
    void MarkBlock_traversFromHeaderToExitingBlocks(timingBase total_latency, Loop* L, BasicBlock *curBlock);

    // evaluate the block latency and resource by traversing the instructions
    timingBase BlockLatencyResourceEvaluation(BasicBlock *B);

    // check whether the instruction is in the block
    bool BlockContain(BasicBlock *B, Instruction *I);

    // get the latency of a specific instruction
    timingBase getInstructionLatency(Instruction *I);

    // get the resource cost of a specific instruction
    resourceBase getInstructionResource(Instruction *I);

    // check whether all the sub-function are evaluated
    bool CheckDependencyFesilility(Function &F);

    // get the relationship between loops and blocks
    void getLoopBlockMap(Function* F);

    // Trace Memory Declaration in Module
    void TraceMemoryDeclarationinModule(Module &M);

    // some LLVM analysises could be involved
    ScalarEvolution *SE;
    LoopInfo *LI;
    LoopAccessLegacyAnalysis *LAA;

    bool topFunctionFound = 0;
    
    float clock_period = 10.0;

    int top_function_latency;

    std::string clock_period_str = "10.0";

    std::string HLS_lib_path = "";

//////////////////// Declaration related to timing and resource of instructions ////////////////////  

    // A unit class to store the information of timing and resource for instruction
    class inst_timing_resource_info
    {
        public:
        // resource 
        int FF;
        int DSP;
        int LUT;

        // timing 
        int Lat;
        float delay;
        int II;

        // core
        std::string core_name;

        inst_timing_resource_info &operator=(inst_timing_resource_info input)
        {
            FF=input.FF;
            DSP=input.DSP;
            LUT=input.LUT;
            Lat=input.Lat;
            delay=input.delay;
            II=input.II;
            core_name=input.core_name;
        }

        inst_timing_resource_info(const inst_timing_resource_info &input)
        {
            FF=input.FF;
            DSP=input.DSP; 
            LUT=input.LUT;
            Lat=input.Lat;
            delay=input.delay;
            II=input.II;
            core_name=input.core_name;
        }

        inst_timing_resource_info()
        {
            FF=0;
            DSP=0; 
            LUT=0;
            Lat=0;
            delay=0;
            II=0;
            core_name="";
        }

        void print()
        {
            llvm::errs() << " DSP=" << DSP << " FF=" << FF << " LUT=" << LUT << " Lat=" << Lat << " delay=" << delay << " II=" << II << "\n";  
        }

    };

    // A unit class to record the timing and latency for a(n) instruction/block/function/loop 
    class timingBase
    {
        public:
            timingBase( int l,double t, int i, double p)
            {
                latency = l;
                timing = t;
                clock_period = p;
                II = i;
            }
            int latency;
            int II;
            double timing;    
            double clock_period;

            timingBase &operator=(timingBase input)
            {
                latency=input.latency;
                II=input.II;
                timing=input.timing;
                clock_period=input.clock_period;
            }

            timingBase(const timingBase& input)
            {
                latency=input.latency;
                II=input.II;
                timing=input.timing;
                clock_period=input.clock_period;
            }
            timingBase()
            {
                latency=0;
                II=0;
                timing=0;
                clock_period=0;
            }
    };

    friend timingBase operator+(timingBase lhs, timingBase rhs)
    {
        assert(lhs.clock_period == rhs.clock_period);
        if (rhs.latency==-1) // for operation like br/ret
        {
            lhs.timing = rhs.timing;
            lhs.latency++;
            return lhs;
        }
        if (rhs.latency==-2) // for operation like load
        {
            if (lhs.clock_period-lhs.timing>LoadStore_Thredhold)
            {
                lhs.timing = rhs.timing;
                lhs.latency++;
                return lhs;
            }
            else
            {
                lhs.timing = rhs.timing;
                lhs.latency+=2;
                return lhs;
            }      
        }
        lhs.latency = lhs.latency + rhs.latency;
        lhs.timing = lhs.timing + rhs.timing;
        if (lhs.timing > lhs.clock_period*CertaintyRatio)
        {
            lhs.timing = rhs.timing;
            lhs.latency++;
        }
        return lhs;
    }


    friend timingBase operator-(timingBase lhs, timingBase rhs)
    {
        assert(lhs.clock_period == rhs.clock_period);
        lhs.latency -=  rhs.latency;
        return lhs;
    }

    friend bool operator>(timingBase lhs, timingBase rhs)
    {
        assert(lhs.clock_period == rhs.clock_period);
        return (((lhs.latency>rhs.latency))||(lhs.latency == rhs.latency && lhs.timing > rhs.timing));
    }

    friend timingBase operator*(timingBase lhs, int rhs)
    {
        if (lhs.timing>0.0001)
        {
            lhs.latency++;
            lhs.timing=0;
        }
        lhs.latency*=rhs;
        return lhs;
    }

    friend timingBase operator*(int rhs,timingBase lhs)
    {
        if (lhs.timing>0.0001)
        {
            lhs.latency++;
            lhs.timing=0;
        }
        lhs.latency*=rhs;
        return lhs;
    }

    friend raw_ostream& operator<< (raw_ostream& stream, const timingBase& tb)
    {
        stream << " [latency=" << tb.latency << ", timing="<<tb.timing<<"] ";
        return stream;
    }


    // A unit class to record the FPGA resource for a(n) instruction/block/function/loop 
    class resourceBase
    {
        public:
            resourceBase( int D, int F, int L, double C)
            {
                DSP = D;
                FF = F;
                LUT = L;
                clock_period = C;
            }
            int DSP,FF,LUT;
            double clock_period;

            resourceBase &operator=(resourceBase input)
            {
                DSP = input.DSP;
                FF = input.FF;
                LUT = input.LUT;
                clock_period = input.clock_period;
            }

            resourceBase(const resourceBase& input)
            {
                DSP=input.DSP;
                FF=input.FF;
                LUT=input.LUT;
                clock_period = input.clock_period;
            }

            resourceBase()
            {
                DSP = -1;
                FF = -1;
                LUT = -1;
                clock_period = -1;
            }
    };

    friend resourceBase operator+(resourceBase lhs, resourceBase rhs)
    {
        assert(lhs.clock_period == rhs.clock_period);
        lhs.DSP = lhs.DSP + rhs.DSP;
        lhs.FF = lhs.FF + rhs.FF;
        lhs.LUT = lhs.LUT + rhs.LUT;
        return lhs;
    }

    friend resourceBase operator*(resourceBase lhs, int rhs)
    {
        lhs.DSP = lhs.DSP * rhs;
        lhs.FF = lhs.FF * rhs;
        lhs.LUT = lhs.LUT * rhs;
        return lhs;
    }

    friend resourceBase operator*(int rhs , resourceBase lhs)
    {
        lhs.DSP = lhs.DSP * rhs;
        lhs.FF = lhs.FF * rhs;
        lhs.LUT = lhs.LUT * rhs;
        return lhs;
    }

    friend raw_ostream& operator<< (raw_ostream& stream, const resourceBase& rb)
    {
        stream << " [DSP=" << rb.DSP << ", FF="<<rb.FF << ", LUT="<<rb.LUT <<"] ";
        return stream;
    }

    // For each type of instruction, there will be a list to store a series of information corresponding to different parameters
    typedef std::map<int,std::map<int, std::map<std::string,inst_timing_resource_info>>> Info_type_list;

    // A map from opcode to the information list of timing and resource
    std::map<std::string,Info_type_list> BiOp_Info_name2list_map;

    // get the information of a specific instruction, based on its opcode, operand_bitwidth, result_bitwidth and clock period
    inst_timing_resource_info get_inst_info(std::string opcode, int operand_bitwid , int res_bitwidth, std::string period);

    // Organize the information into timingBase after getting the information of a specific instruction, based on its opcode, operand_bitwidth, result_bitwidth and clock period.
    timingBase get_inst_TimingInfo_result(std::string opcode, int operand_bitwid , int res_bitwidth, std::string period);

    // Organize the information into resourceBase after getting the information of a specific instruction, based on its opcode, operand_bitwidth, result_bitwidth and clock period.
    resourceBase get_inst_ResourceInfo_result(std::string opcode, int operand_bitwid , int res_bitwidth, std::string period);

    // int get_N_DSP(std::string opcode, int operand_bitwid , int res_bitwidth, std::string period);
    // int get_N_FF(std::string opcode, int operand_bitwid , int res_bitwidth, std::string period);
    // int get_N_LUT(std::string opcode, int operand_bitwid , int res_bitwidth, std::string period);
    // int get_N_Lat(std::string opcode, int operand_bitwid , int res_bitwidth, std::string period);
    // double get_N_Delay(std::string opcode, int operand_bitwid , int res_bitwidth, std::string period);

    // evaluate the number of FF needed by the instruction
    resourceBase FF_Evaluate(std::map<Instruction*, timingBase> &cur_InstructionCriticalPath, Instruction* cur_I);

    // trace back to find the original operator, bypassing SExt and ZExt operations
    Instruction* byPassUnregisterOp(Instruction* cur_I);

    Instruction* byPassBitcastOp(Instruction* cur_I);

    // evaluate the number of LUT needed by the PHI instruction
    resourceBase IndexVar_LUT(std::map<Instruction*, timingBase> &cur_InstructionCriticalPath, Instruction* I);

    // evaluate the number of LUT needed by the BRAM Mux
    resourceBase BRAM_MUX_Evaluate();

    // check whether a specific information is in the database
    bool checkInfoAvailability(std::string opcode, int operand_bitwid , int res_bitwidth, std::string period);

    // check whether we can infer the information by increasing the clock frequency
    bool checkFreqProblem(std::string opcode, int operand_bitwid , int res_bitwidth, std::string period);

    // if the information is not found in database, we may infer the information by increasing the clock frequency
    inst_timing_resource_info checkInfo_HigherFreq(std::string opcode, int operand_bitwid , int res_bitwidth, std::string period);

    // Trace back to get the bitwidth of an operand, bypassing truct/zext/sext
    int getOriginalBitwidth(Value *Val);

    // Trace forward to get the number of users, bypassing truct/zext/sext
    int getActualUsersNum(Instruction *I, int dep);

    // check whether the two operations can be chained
    bool canChainOrNot(Instruction *PredI,Instruction *I);

    // check whether the two operations can be chained into a MAC operation
    bool isMACpossible(Instruction *PredI,Instruction *I);

    // check whether the three operations can be chained into a AMA operation
    bool isAMApossible(Instruction *PredI,Instruction *I);

//////////////////// Declaration related to Memory Access Tracing ////////////////////  
 
    std::map<Instruction*,std::vector<Value*>> Access2TargetMap;
    std::map<Value*, std::map<BasicBlock*,std::vector<int>>> target2LastAccessCycleInBlock;
    std::map<std::pair<Instruction*,Value*>,timingBase> scheduledAccess_timing;
    void findMemoryDeclarationin(Function *F, bool isTopFunction);
    void TraceAccessForTarget(Value *cur_node,Value *ori_node);

    timingBase scheduleBRAMAccess(Instruction *access, BasicBlock *cur_block,  timingBase cur_Timing);
    timingBase handleBRAMAccessFor(Instruction *access, Value *target, BasicBlock *cur_block,  timingBase cur_Timing);
    bool checkBRAMAvailabilty(Value *target, std::string StoreOrLoad, BasicBlock *cur_block, timingBase cur_Timing);
    void insertBRAMAccessInfo(Value *target, BasicBlock *cur_block, int cur_latency);
};

#endif
