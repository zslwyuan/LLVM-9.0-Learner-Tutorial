#ifndef _HI_WithDirectiveTimingResourceEvaluation
#define _HI_WithDirectiveTimingResourceEvaluation

#define AggressiveSchedule

#ifndef LoadStore_Thredhold
#ifdef AggressiveSchedule
#define LoadStore_Thredhold 2
#define CertaintyRatio (7.2 / 8.0)
#else
#define LoadStore_Thredhold 3.25
#define CertaintyRatio (7.0 / 8.0)
#endif
#endif

#define Strict_LoadStore_Thredhold 3.25
#define Strict_CertaintyRatio (7.0 / 8.0)

#include "ClockInfo.h"
#include "HI_InstructionFiles.h"
#include "HI_StringProcess.h"
#include "HI_print.h"
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
#include <algorithm>
#include <bits/stl_map.h>
#include <cctype>
#include <fstream>
#include <ios>
#include <iostream>
#include <set>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>

using namespace llvm;

#ifndef _HI_DesignConfigInfo
#define _HI_DesignConfigInfo

class HI_DesignConfigInfo
{
  public:
    HI_DesignConfigInfo()
    {
    }

    void setClock(std::string _clock_period_str)
    {
        clock_period_str = _clock_period_str;
        clock_period = std::stod(clock_period_str);
    }

    void insertLoopUnroll(std::string loopLabel, int factor)
    {
        int cnt = 0;
        for (auto unroll_pair : loopUnrollConfigs)
        {
            if (unroll_pair.first == loopLabel)
            {
                loopUnrollConfigs.erase(loopUnrollConfigs.begin() + cnt);
                break;
            }
            cnt++;
        }
        loopUnrollConfigs.push_back(std::pair<std::string, int>(loopLabel, factor));
        LoopLabel2UnrollFactor[loopLabel] = factor;
    }

    void eraseLoopPipeline(std::string loopLabel)
    {
        int cnt = 0;
        for (auto pipeline_pair : loopPipelineConfigs)
        {
            if (pipeline_pair.first == loopLabel)
            {
                loopPipelineConfigs.erase(loopPipelineConfigs.begin() + cnt);
                break;
            }
            cnt++;
        }
    }

    void insertLoopPipeline(std::string loopLabel, int II)
    {
        int cnt = 0;
        for (auto pipeline_pair : loopPipelineConfigs)
        {
            if (pipeline_pair.first == loopLabel)
            {
                loopPipelineConfigs.erase(loopPipelineConfigs.begin() + cnt);
                break;
            }
            cnt++;
        }
        loopPipelineConfigs.push_back(std::pair<std::string, int>(loopLabel, II));
    }

    void insertArrayCyclicPartition(std::string functionName, std::string arrayName, int dim,
                                    int factor)
    {
        int cnt = 0;
        for (auto partition_seq : cyclicPartitionConfigs)
        {
            if (partition_seq.first == functionName)
            {
                if (partition_seq.second.first == arrayName)
                {
                    if (partition_seq.second.second.first == dim)
                    {
                        cyclicPartitionConfigs.erase(cyclicPartitionConfigs.begin() + cnt);
                        break;
                    }
                }
            }
            cnt++;
        }
        cnt = 0;
        for (auto partition_seq : blockPartitionConfigs)
        {
            if (partition_seq.first == functionName)
            {
                if (partition_seq.second.first == arrayName)
                {
                    if (partition_seq.second.second.first == dim)
                    {
                        blockPartitionConfigs.erase(blockPartitionConfigs.begin() + cnt);
                        break;
                    }
                }
            }
            cnt++;
        }
        cyclicPartitionConfigs.push_back(
            std::pair<std::string, std::pair<std::string, std::pair<int, int>>>(
                functionName, std::pair<std::string, std::pair<int, int>>(
                                  arrayName, std::pair<int, int>(dim, factor))));
    }

    void insertArrayCompletePartition(std::string functionName, std::string arrayName, int dim)
    {
        int cnt = 0;
        for (auto partition_seq : cyclicPartitionConfigs)
        {
            if (partition_seq.first == functionName)
            {
                if (partition_seq.second.first == arrayName)
                {
                    if (partition_seq.second.second.first == dim)
                    {
                        cyclicPartitionConfigs.erase(cyclicPartitionConfigs.begin() + cnt);
                        break;
                    }
                }
            }
            cnt++;
        }
        cnt = 0;
        for (auto partition_seq : blockPartitionConfigs)
        {
            if (partition_seq.first == functionName)
            {
                if (partition_seq.second.first == arrayName)
                {
                    if (partition_seq.second.second.first == dim)
                    {
                        blockPartitionConfigs.erase(blockPartitionConfigs.begin() + cnt);
                        break;
                    }
                }
            }
            cnt++;
        }
        completePartitionConfigs.push_back(std::pair<std::string, std::pair<std::string, int>>(
            functionName, std::pair<std::string, int>(arrayName, dim)));
    }

    void increaseArrayCyclicPartition(std::string functionName, std::string arrayName, int dim,
                                      int factor)
    {
        int cnt = 0;
        for (auto partition_seq : cyclicPartitionConfigs)
        {
            if (partition_seq.first == functionName)
            {
                if (partition_seq.second.first == arrayName)
                {
                    if (partition_seq.second.second.first == dim)
                    {
                        if (partition_seq.second.second.second < factor)
                        {
                            cyclicPartitionConfigs.erase(cyclicPartitionConfigs.begin() + cnt);
                            break;
                        }
                        else
                        {
                            return;
                        }
                    }
                }
            }
            cnt++;
        }
        cnt = 0;
        for (auto partition_seq : blockPartitionConfigs)
        {
            if (partition_seq.first == functionName)
            {
                if (partition_seq.second.first == arrayName)
                {
                    if (partition_seq.second.second.second < factor)
                    {
                        blockPartitionConfigs.erase(blockPartitionConfigs.begin() + cnt);
                        break;
                    }
                    else
                    {
                        return;
                    }
                }
            }
            cnt++;
        }
        cyclicPartitionConfigs.push_back(
            std::pair<std::string, std::pair<std::string, std::pair<int, int>>>(
                functionName, std::pair<std::string, std::pair<int, int>>(
                                  arrayName, std::pair<int, int>(dim, factor))));
    }

    void eraseArrayPartition(std::string functionName, std::string arrayName)
    {
        int cnt = 0;
        for (auto partition_seq : cyclicPartitionConfigs)
        {
            if (partition_seq.first == functionName)
            {
                if (partition_seq.second.first == arrayName)
                {
                    cyclicPartitionConfigs.erase(cyclicPartitionConfigs.begin() + cnt);
                }
            }
            cnt++;
        }
        cnt = 0;
        for (auto partition_seq : blockPartitionConfigs)
        {
            if (partition_seq.first == functionName)
            {
                if (partition_seq.second.first == arrayName)
                {
                    blockPartitionConfigs.erase(blockPartitionConfigs.begin() + cnt);
                }
            }
            cnt++;
        }
    }

    void insertArrayblockPartition(std::string functionName, std::string arrayName, int dim,
                                   int factor)
    {
        int cnt = 0;
        for (auto partition_seq : cyclicPartitionConfigs)
        {
            if (partition_seq.first == functionName)
            {
                if (partition_seq.second.first == arrayName)
                {
                    if (partition_seq.second.second.first == dim)
                    {
                        cyclicPartitionConfigs.erase(cyclicPartitionConfigs.begin() + cnt);
                        break;
                    }
                }
            }
            cnt++;
        }
        cnt = 0;
        for (auto partition_seq : blockPartitionConfigs)
        {
            if (partition_seq.first == functionName)
            {
                if (partition_seq.second.first == arrayName)
                {
                    if (partition_seq.second.second.first == dim)
                    {
                        blockPartitionConfigs.erase(blockPartitionConfigs.begin() + cnt);
                        break;
                    }
                }
            }
            cnt++;
        }
        blockPartitionConfigs.push_back(
            std::pair<std::string, std::pair<std::string, std::pair<int, int>>>(
                functionName, std::pair<std::string, std::pair<int, int>>(
                                  arrayName, std::pair<int, int>(dim, factor))));
    }

    void insertArrayPortNum(std::string functionName, std::string arrayName, int port_num)
    {
        arrayPortConfigs.push_back(std::pair<std::string, std::pair<std::string, int>>(
            functionName, std::pair<std::string, int>(arrayName, port_num)));
    }

    void insertFuncDataflow(std::string functioName, bool enable)
    {
        funcDataflowConfigs.push_back(std::pair<std::string, bool>(functioName, enable));
    }

    void insertLocalArray(std::string functionName, std::string arrayName, bool enable)
    {
        localArrayConfigs.push_back(std::pair<std::string, std::pair<std::string, bool>>(
            functionName, std::pair<std::string, bool>(arrayName, enable)));
    }

    HI_DesignConfigInfo(const HI_DesignConfigInfo &input)
    {
        clock_period = input.clock_period;
        clock_period_str = input.clock_period_str;
        loopUnrollConfigs = input.loopUnrollConfigs;
        LoopLabel2UnrollFactor = input.LoopLabel2UnrollFactor;
        loopPipelineConfigs = input.loopPipelineConfigs;
        cyclicPartitionConfigs = input.cyclicPartitionConfigs;
        completePartitionConfigs = input.completePartitionConfigs;
        blockPartitionConfigs = input.blockPartitionConfigs;
        arrayPortConfigs = input.arrayPortConfigs;
        funcDataflowConfigs = input.funcDataflowConfigs;
        localArrayConfigs = input.localArrayConfigs;
        HLS_lib_path = input.HLS_lib_path;
    }

    HI_DesignConfigInfo &operator=(const HI_DesignConfigInfo &input)
    {
        clock_period = input.clock_period;
        clock_period_str = input.clock_period_str;
        loopUnrollConfigs = input.loopUnrollConfigs;
        LoopLabel2UnrollFactor = input.LoopLabel2UnrollFactor;
        loopPipelineConfigs = input.loopPipelineConfigs;
        cyclicPartitionConfigs = input.cyclicPartitionConfigs;
        completePartitionConfigs = input.completePartitionConfigs;
        blockPartitionConfigs = input.blockPartitionConfigs;
        arrayPortConfigs = input.arrayPortConfigs;
        funcDataflowConfigs = input.funcDataflowConfigs;
        localArrayConfigs = input.localArrayConfigs;
        HLS_lib_path = input.HLS_lib_path;
    }

    double clock_period;
    std::string clock_period_str;
    std::string HLS_lib_path;
    std::vector<std::pair<std::string, int>> loopUnrollConfigs;
    std::vector<std::pair<std::string, int>> loopPipelineConfigs;
    std::vector<std::pair<std::string, std::pair<std::string, std::pair<int, int>>>>
        cyclicPartitionConfigs;
    std::vector<std::pair<std::string, std::pair<std::string, int>>> completePartitionConfigs;
    std::vector<std::pair<std::string, std::pair<std::string, std::pair<int, int>>>>
        blockPartitionConfigs;
    std::vector<std::pair<std::string, std::pair<std::string, bool>>> localArrayConfigs;
    std::vector<std::pair<std::string, std::pair<std::string, int>>> arrayPortConfigs;
    std::vector<std::pair<std::string, bool>> funcDataflowConfigs;
    std::map<std::string, int> LoopLabel2UnrollFactor;
};

raw_ostream &operator<<(raw_ostream &stream, const HI_DesignConfigInfo &tb);

#endif

#ifndef _HI_inst_timing_resource_info
#define _HI_inst_timing_resource_info
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
        FF = input.FF;
        DSP = input.DSP;
        LUT = input.LUT;
        Lat = input.Lat;
        delay = input.delay;
        II = input.II;
        core_name = input.core_name;
    }

    inst_timing_resource_info(const inst_timing_resource_info &input)
    {
        FF = input.FF;
        DSP = input.DSP;
        LUT = input.LUT;
        Lat = input.Lat;
        delay = input.delay;
        II = input.II;
        core_name = input.core_name;
    }

    inst_timing_resource_info()
    {
        FF = 0;
        DSP = 0;
        LUT = 0;
        Lat = 0;
        delay = 0;
        II = 0;
        core_name = "";
    }

    void print()
    {
        llvm::errs() << " DSP=" << DSP << " FF=" << FF << " LUT=" << LUT << " Lat=" << Lat
                     << " delay=" << delay << " II=" << II << "\n";
    }
};

// For each type of instruction, there will be a list to store a series of information corresponding
// to different parameters
typedef std::map<int, std::map<int, std::map<std::string, inst_timing_resource_info>>>
    Info_type_list;
#endif

// Pass for simple evluation of the latency of the top function, without considering HLS directives
class HI_WithDirectiveTimingResourceEvaluation : public ModulePass
{
  public:
    // Pass for simple evluation of the latency of the top function, without considering HLS
    // directives
    HI_WithDirectiveTimingResourceEvaluation(
        const char *config_file_name, const char *evaluating_log_name, const char *BRAM_log_name,
        const char *ArrayLog_name, const char *top_function,
        std::map<std::string, std::string> &IRLoop2LoopLabel,
        std::map<std::string, int> &IRLoop2OriginTripCount,
        std::map<std::string, int> &LoopLabel2II,
        std::map<std::string, int> &LoopLabel2UnrollFactor,
        std::map<std::string, int> &FuncParamLine2OutermostSize,
        std::map<std::string, std::vector<int>> &IRFunc2BeginLine,
        std::map<std::string, Info_type_list> &BiOp_Info_name2list_map, bool DEBUG = 0)
        : ModulePass(ID), IRLoop2LoopLabel(IRLoop2LoopLabel),
          IRLoop2OriginTripCount(IRLoop2OriginTripCount), LoopLabel2II(LoopLabel2II),
          LoopLabel2UnrollFactor(LoopLabel2UnrollFactor),
          FuncParamLine2OutermostSize(FuncParamLine2OutermostSize),
          IRFunc2BeginLine(IRFunc2BeginLine), BiOp_Info_name2list_map(BiOp_Info_name2list_map),
          DEBUG(DEBUG)
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
        FF_log = new raw_fd_ostream("FF_LOG", ErrInfo, sys::fs::F_None);
        ArrayLog = new raw_fd_ostream(ArrayLog_name, ErrInfo, sys::fs::F_None);
        // get the configureation from the file, e.g. clock period
        Parse_Config_fromFile();
        config_file->clear();
        config_file->seekg(0, std::ifstream::beg);
        Parse_Config_toInfo(configInfo);
    }

    // Pass for simple evluation of the latency of the top function, without considering HLS
    // directives
    HI_WithDirectiveTimingResourceEvaluation( // const char* config_file_name,
        const HI_DesignConfigInfo &configInfo, const char *evaluating_log_name,
        const char *BRAM_log_name, const char *ArrayLog_name, const char *top_function,
        std::map<std::string, std::string> &IRLoop2LoopLabel,
        std::map<std::string, int> &IRLoop2OriginTripCount,
        std::map<std::string, int> &LoopLabel2II,
        std::map<std::string, int> &LoopLabel2UnrollFactor,
        std::map<std::string, int> &FuncParamLine2OutermostSize,
        std::map<std::string, std::vector<int>> &IRFunc2BeginLine,
        std::map<std::string, Info_type_list> &BiOp_Info_name2list_map, bool DEBUG = 0)
        : ModulePass(ID), IRLoop2LoopLabel(IRLoop2LoopLabel),
          IRLoop2OriginTripCount(IRLoop2OriginTripCount), LoopLabel2II(LoopLabel2II),
          LoopLabel2UnrollFactor(LoopLabel2UnrollFactor),
          FuncParamLine2OutermostSize(FuncParamLine2OutermostSize),
          IRFunc2BeginLine(IRFunc2BeginLine), BiOp_Info_name2list_map(BiOp_Info_name2list_map),
          DEBUG(DEBUG), configInfo(configInfo)
    {
        BlockEvaluated.clear();
        LoopEvaluated.clear();
        FunctionEvaluated.clear();
        Loop_id.clear();
        Loop_Counter = 0;
        //     config_file = new std::ifstream(config_file_name);
        Evaluating_log = new raw_fd_ostream(evaluating_log_name, ErrInfo, sys::fs::F_None);
        BRAM_log = new raw_fd_ostream(BRAM_log_name, ErrInfo, sys::fs::F_None);
        top_function_name = std::string(top_function);
        FF_log = new raw_fd_ostream("FF_LOG", ErrInfo, sys::fs::F_None);
        ArrayLog = new raw_fd_ostream(ArrayLog_name, ErrInfo, sys::fs::F_None);
        Parse_Config(configInfo);
    }

    // Pass for simple evluation of the latency of the top function, without considering HLS
    // directives
    HI_WithDirectiveTimingResourceEvaluation(
        const char *config_file_name, const char *evaluating_log_name, const char *BRAM_log_name,
        const char *ArrayLog_name, const char *top_function,
        std::map<std::string, std::string> &IRLoop2LoopLabel,
        std::map<std::string, int> &IRLoop2OriginTripCount,
        std::map<std::string, int> &LoopLabel2II,
        std::map<std::string, int> &LoopLabel2UnrollFactor,
        std::map<std::string, int> &FuncParamLine2OutermostSize,
        std::map<std::string, std::vector<int>> &IRFunc2BeginLine, bool DEBUG = 0)
        : ModulePass(ID), IRLoop2LoopLabel(IRLoop2LoopLabel), LoopLabel2II(LoopLabel2II),
          LoopLabel2UnrollFactor(LoopLabel2UnrollFactor),
          FuncParamLine2OutermostSize(FuncParamLine2OutermostSize),
          IRFunc2BeginLine(IRFunc2BeginLine),
          BiOp_Info_name2list_map(this->BiOp_Info_name2list_map_contain), DEBUG(DEBUG)
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
        FF_log = new raw_fd_ostream("FF_LOG", ErrInfo, sys::fs::F_None);
        ArrayLog = new raw_fd_ostream(ArrayLog_name, ErrInfo, sys::fs::F_None);
        // get the configureation from the file, e.g. clock period
        Parse_Config_fromFile();
        config_file->clear();
        config_file->seekg(0, std::ifstream::beg);
        Parse_Config_toInfo(configInfo);
        // load the HLS database of timing and resource
        Load_Instruction_Info();
    }

    ~HI_WithDirectiveTimingResourceEvaluation()
    {
        for (auto ele : Loop2Blocks)
        {
            delete ele.second;
        }
        for (auto ele : Block2Loops)
        {
            delete ele.second;
        }
        Evaluating_log->flush();
        delete Evaluating_log;
        BRAM_log->flush();
        delete BRAM_log;
        FF_log->flush();
        delete FF_log;
        ArrayLog->flush();
        delete ArrayLog;
    }

    virtual bool doInitialization(llvm::Module &M)
    {

        print_status("Initilizing HI_WithDirectiveTimingResourceEvaluation pass.");
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
        Value_FFAssigned.clear();
        Function2OuterLoops.clear();
        Block2EvaluatedLoop.clear();
        BlockCriticalPath_inLoop.clear();
        tmp_BlockCriticalPath_inFunc.clear();
        tmp_LoopCriticalPath_inFunc.clear();
        tmp_BlockCriticalPath_inLoop.clear();
        tmp_SubLoop_CriticalPath.clear();
        InstructionCriticalPath_inBlock.clear();
        FuncName2LoopIRNames.clear();

        return false;
    }

    bool DEBUG; // print out info or not

    class timingBase;
    class resourceBase;
    class HI_PragmaInfo;
    class partition_info;
    class DSPReuseScheduleUnit;
    class HI_ArrayInfo;
    class HI_AccessInfo;

    // set the dependence of Passes
    void getAnalysisUsage(AnalysisUsage &AU) const;

    virtual bool runOnModule(llvm::Module &M);

    std::map<std::string, timingBase> LoopLabel2Latency;
    std::map<std::string, resourceBase> LoopLabel2Resource;
    std::map<std::string, int> LoopLabel2SmallestII;
    std::map<std::string, int> LoopLabel2IterationLatency;
    HI_DesignConfigInfo configInfo;

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////// Declaration related to timing and resource evaluation of Basic
    ///Block/Loop/Function ////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    // check whether the block is in some loops
    bool isInLoop(BasicBlock *BB);

    // evaluatate the latency of a outer loop, which could be a nested one,  and return the timing
    // information
    timingBase analyzeOuterLoop(Loop *outerL);

    // get the most outer loop which contains the block, treat the loop as a node for the evaluation
    // of latency
    Loop *getOuterLoopOfBlock(BasicBlock *B);

    // find the inner unevaluated loop for processing
    Loop *getInnerUnevaluatedLoop(Loop *outerL);

    // evaluate a loop in which all the children loops have been evauluated and return the timing
    // information
    timingBase analyzeLoop_InnerChecked(Loop *L);

    static char ID;

    int Loop_Counter;

    std::map<Loop *, int> Loop_id;

    // the latency of each loop
    std::map<BasicBlock *, timingBase> LoopLatency;

    // the latency of each block
    std::map<BasicBlock *, timingBase> BlockLatency;

    // the latency of each function
    std::map<Function *, timingBase> FunctionLatency;

    // the latency of each function
    std::map<std::string, timingBase> FuncName2Latency;

    // the resource of each loop
    std::map<BasicBlock *, resourceBase> LoopResource;

    // the resource of each block
    std::map<BasicBlock *, resourceBase> BlockResource;

    // the resource of each function
    std::map<Function *, resourceBase> FunctionResource;

    // the resource of each function
    std::map<std::string, resourceBase> FuncName2Resource;

    // record whether the component is evaluated
    std::set<BasicBlock *> BlockEvaluated;
    std::set<BasicBlock *> Func_BlockEvaluated;
    std::set<BasicBlock *> LoopEvaluated;
    std::set<Function *> FunctionEvaluated;
    std::set<Instruction *> InstructionEvaluated;
    std::set<Value *> Value_FFAssigned;
    std::set<BasicBlock *> BlockVisited;
    std::set<BasicBlock *> Func_BlockVisited;
    std::set<Value *> ValueVisited;

    // record the information of the processing
    raw_fd_ostream *Evaluating_log;

    // record the information of the BRAMs and related accesses to BRAMs
    raw_fd_ostream *BRAM_log;

    // record the information of the BRAMs and related accesses to BRAMs
    raw_fd_ostream *FF_log;

    std::error_code ErrInfo;
    std::ifstream *config_file;

    // the pass requires a specified top_function name
    std::string top_function_name;

    // record the relations between loops and blocks
    std::map<Loop *, std::vector<BasicBlock *> *> Loop2Blocks;
    std::map<BasicBlock *, std::vector<Loop *> *> Block2Loops;

    // record the list of outer loops for functions
    std::map<Function *, std::vector<Loop *>> Function2OuterLoops;

    std::map<std::string, std::vector<std::string>> FuncName2LoopIRNames;

    // record which evaluated loop the block is belong to, so the pass can directly trace to the
    // loop for the latency
    std::map<BasicBlock *, Loop *> Block2EvaluatedLoop;

    // record the critical path from the loop header to the end of the specific block
    std::map<Loop *, std::map<BasicBlock *, timingBase>> BlockCriticalPath_inLoop;

    // record the critical path from the loop header to the beginning of the specific block
    std::map<BasicBlock *, timingBase> BlockBegin_inLoop;

    // record the critical path to the end of block in the function
    std::map<BasicBlock *, timingBase> tmp_BlockCriticalPath_inFunc;

    // record the critical path to the end of loops in the function
    std::map<Loop *, timingBase> tmp_LoopCriticalPath_inFunc;

    // record the critical path to the end of sub-loops in the loop
    std::map<BasicBlock *, timingBase> tmp_BlockCriticalPath_inLoop;

    // record the critical path from the outter loop header to the end of the specific sub-loop
    std::map<Loop *, timingBase> tmp_SubLoop_CriticalPath;

    // for II evaluation, check the instruction time slot
    std::map<Instruction *, int> Inst2TimeSlotInLoop;

    // the CP for each iteration of the loop
    std::map<BasicBlock *, int> Loop2CP;

    // record the critical path from the block entry to the end of the specific instruction
    std::map<BasicBlock *, std::map<Instruction *, timingBase>> InstructionCriticalPath_inBlock;

    // Instruction Schedule
    std::map<Instruction *, std::pair<BasicBlock *, int>> Inst_Schedule;

    // record the latest operand of each instruction
    std::map<Instruction *, Instruction *> Inst2LatestOperand;

    // record when the register for the result of Instruction can be release
    std::map<Instruction *, std::pair<BasicBlock *, int>> RegRelease_Schedule;

    // record whether the result reg of the instruction I has been reused
    std::set<Instruction *> I_RegReused;

    // Instruction related to target
    std::map<Value *, std::set<Value *>> Value2Target;

    // record the accesses to the partitions are done at which cycle in which basic block
    std::map<std::pair<Value *, partition_info>, std::vector<std::pair<BasicBlock *, int>>>
        targetPartition2BlockCycleAccessCnt;

    // an intermediate set for II evaluation recording which partitions will be accessed in a loop
    std::set<std::pair<Value *, partition_info>> accessPartitionsForIITest;

    // record the access property of the target in function
    enum accessPropertyType
    {
        ReadOnlyInFunction,
        WriteOnlyInFunction,
        ReadWriteInFunction
    };
    std::map<std::pair<Value *, Function *>, accessPropertyType> targetAccessPropertyInFunction;

    // update access property of the targets
    void updateAccessProperty_Function(std::pair<Value *, Function *> val_func_pair,
                                       accessPropertyType updateTy);

    std::map<BasicBlock *, timingBase> Loop2IterationLatency;
    std::map<BasicBlock *, int> Loop2AchievedII;
    std::map<std::string, int> LoopLabel2AchievedII;

    // check whether the two instructions have the same access targets
    bool hasSameTargets(Instruction *I0, Instruction *I1);

    // among the scheduled instructions, find the latest access to the specific target
    Instruction *
    findLatestPointerAccess(Instruction *curI, std::set<Value *> targets,
                            std::map<Instruction *, timingBase> &cur_InstructionCriticalPath);

    // // demangle the name of functions
    // std::string demangleFunctionName(std::string mangled_name);

    // get the latency of functions in the module  and compute the resource cost
    // analyze the timing and resource for the functions in the module
    // by recursively analysis from the lowest subfunctions
    void AnalyzeFunctions(llvm::Module &M);

    // get the latency of TopFunction Latency
    // analyze and output the informaion of top function
    void analyzeTopFunction(llvm::Module &M);

    /*
        find the longest path in the function as its latency
        also record the resource cost in the function
    */
    timingBase analyzeFunction(Function *F);

    // return the resource cost of the function
    resourceBase getFunctionResource(Function *F);

    // get how many state needed for the function
    int getFunctionStageNum(timingBase tmp_critical_path, Function *F, BasicBlock *curBlock);

    // get how many state needed for the application
    int getTotalStateNum(llvm::Module &M);

    // get the number of stage arrive the instruction
    int getStageTo(Instruction *I);

    /*
        get the latency of functions in the path to the instruction
    */
    int getFunctionLatencyInPath(Instruction *I);

    // get the number of stage in the block
    int getStageNumOfBlock(BasicBlock *B);

    // get the function critical path by traversing the blocks based on DFS and compute the resource
    // cost
    void analyzeFunction_traverseFromEntryToExiting(timingBase tmp_critical_path, Function *F,
                                                    BasicBlock *curBlock,
                                                    resourceBase &resourceAccumulator);

    // analyze the access property of the targets in the function
    void accessPropertyAnalysis_Function(Function *F);

    // check whether the dataflow scheduling can be applied to the function
    // if possible and the dataflow is enabled, evaluate the II and BRAM resouce
    // for it
    void functionDataflowCheck(Function *F);

    // check whether the function is applied with dataflow pragma
    bool isFunctionDataflow(Function *F);

    // check whether the array is specified as local array in dataflow, (unchanged during dataflow)
    bool isLocalArray(Value *target);

    // get the loop latency by traversing from the header to the exiting blocks and evluation
    // resource
    void LoopLatencyResourceEvaluation_traversFromHeaderToExitingBlocks(
        timingBase tmp_critical_path, Loop *L, BasicBlock *curBlock,
        resourceBase &resourceAccumulator);

    // get the II factor for loop pipelining, if there is directives of pipeline for this loop
    int checkIIForLoop(Loop *curLoop,
                       std::map<BasicBlock *, timingBase> &tmp_BlockCriticalPath_inLoop);

    // the BRAM-related II for the loop
    int checkAccessIIForLoop(Loop *curLoop);

    // check the BRAM-related II for the loop by enumerating II and checking port constraint for
    // each cycle for each partition
    int checkAccessIIForLoop_enumerateCheck(Loop *curLoop);

    // the Dependence-related II for the loop
    int checkDependenceIIForLoop(Loop *curLoop);

    // find the earliest user of the load instruction (maybe for reschedule)
    int findEarlietUseTimeInTheLoop(Loop *curLoop, Instruction *R_I);

    // if the loop is pipelined, the reused DSP-related operators might have conflicts when sharing
    // DSPs. therefore, we need to re-check the resource cost FOR INTEGER OPERATION
    resourceBase costRescheduleIntDSPOperators_forLoop(
        Loop *curLoop, std::map<BasicBlock *, timingBase> &tmp_BlockCriticalPath_inLoop, int II);

    // if the loop is pipelined, the reused DSP-related operators might have conflicts when sharing
    // DSPs. therefore, we need to re-check the resource cost FOR FLOATING POINT OPERATOR
    void recordCostRescheduleFPDSPOperators_forLoop(
        Loop *curLoop, std::map<BasicBlock *, timingBase> &tmp_BlockCriticalPath_inLoop, int II);

    // if the loop is not pipelined, the FP DSP-related operators might not share resource but
    // to maximize performance and parallel FP operators
    resourceBase costRescheduleFPDSPOperators_forNotPipelineLoop(
        Loop *curLoop, std::map<BasicBlock *, timingBase> &tmp_BlockCriticalPath_inLoop);

    // check whether the loop is perfectly nested
    bool isPerfectPipelinedNest(Loop *L, BasicBlock *&innerHeader, int &total_nesttripcount);

    // check whether any access in the block
    bool hasNoMemoryOps(BasicBlock *b);

    // for blocks, we need to re-check the resource cost  FOR FLOATING POINT OPERATOR since some of
    // them might be reused.
    void recordCostRescheduleFPDSPOperators_forBlock(BasicBlock *tmp_B, int block_latency);

    // each basic block requires its own DSP operators. Find the maximum number of DSP operators for
    // each FP opcode among block.
    resourceBase costRescheduleFPDSPOperators_forFunction(Function *F);

    // mark the block in loop with latency by traversing from the header to the exiting blocks
    void MarkBlock_traversFromHeaderToExitingBlocks(timingBase total_latency, Loop *L,
                                                    BasicBlock *curBlock);

    // evaluate the block latency and resource by traversing the instructions
    timingBase BlockLatencyResourceEvaluation(BasicBlock *B);

    // check whether the instruction is in the block
    bool BlockContain(BasicBlock *B, Instruction *I);

    // get the latency of a specific instruction
    timingBase getInstructionLatency(Instruction *I);

    // get the resource cost of a specific instruction
    resourceBase getInstructionResource(Instruction *I);

    // get the resource cost of FP operator with opcode string
    resourceBase checkFPOperatorCost(std::string opcode_str);

    // transform instruction to its opcode string
    std::string InstToOpcodeString(Instruction *I);

    // update the latest user of the the specific user, based on which we can determine the lifetime
    // of a register
    void updateResultRelease(Instruction *I, Instruction *I_Pred, int time_point);

    // check whether all the sub-function are evaluated
    bool CheckDependencyFesilility(Function &F);

    // get the relationship between loops and blocks
    void getLoopBlockMap(Function *F);

    // Calculate the LUT for selection
    int LUT_for_select(int stateNum);

    // get the time slot of the instruction in the loop
    int getTimeslotForInstInLoop(Loop *curLoop, Instruction *I,
                                 std::map<BasicBlock *, timingBase> &tmp_BlockCriticalPath_inLoop);
    int getTimeslotForInstInLoop(Instruction *I);

    // get the pointers in the operands
    void checkPtrInOperands(Instruction *I, std::vector<Value *> &ptrInOperands);

    // some LLVM analysises could be involved
    ScalarEvolution *SE;
    LoopInfo *LI;
    LoopAccessLegacyAnalysis *LAA;
    AliasAnalysis *AA;

    bool topFunctionFound = 0;

    float clock_period = 10.0;

    int top_function_latency;

    std::string clock_period_str = "10.0";

    std::string HLS_lib_path = "";
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////// Declaration related to timing and resource of instructions
    ///////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////////////////

    // A unit class to record the timing and latency for a(n) instruction/block/function/loop
    class timingBase
    {
      public:
        timingBase(int l, double t, int i, double p)
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
            latency = input.latency;
            II = input.II;
            timing = input.timing;
            clock_period = input.clock_period;
            strict_timing = input.strict_timing;
        }

        timingBase(const timingBase &input)
        {
            latency = input.latency;
            II = input.II;
            timing = input.timing;
            clock_period = input.clock_period;
            strict_timing = input.strict_timing;
        }
        timingBase()
        {
            latency = 0;
            II = 0;
            timing = 0;
            clock_period = 0;
        }
        bool strict_timing = false;
    };

    friend timingBase operator+(timingBase lhs, timingBase rhs)
    {
        assert(lhs.clock_period == rhs.clock_period);
        if (rhs.latency == -1) // for operation like br/ret
        {
            lhs.timing = rhs.timing;
            lhs.latency++;
            return lhs;
        }
        if (rhs.strict_timing)
        {
            if (rhs.latency == -2) // for operation like load
            {
                if (lhs.clock_period - lhs.timing > rhs.timing)
                {
                    lhs.timing = rhs.timing;
                    lhs.latency++;
                    return lhs;
                }
                else
                {
                    lhs.timing = rhs.timing;
                    lhs.latency += 2;
                    return lhs;
                }
            }
        }
        else
        {
            if (rhs.latency == -2) // for operation like load
            {
                if (lhs.clock_period - lhs.timing > rhs.timing)
                {
                    lhs.timing = rhs.timing;
                    lhs.latency++;
                    return lhs;
                }
                else
                {
                    lhs.timing = rhs.timing;
                    lhs.latency += 2;
                    return lhs;
                }
            }
        }

        lhs.latency = lhs.latency + rhs.latency;
        lhs.timing = lhs.timing + rhs.timing;
        if (rhs.strict_timing)
        {
            if (lhs.timing > lhs.clock_period * Strict_CertaintyRatio)
            {
                lhs.timing = rhs.timing;
                lhs.latency++;
            }
        }
        else
        {
            if (lhs.timing > lhs.clock_period * CertaintyRatio)
            {
                lhs.timing = rhs.timing;
                lhs.latency++;
            }
        }
        return lhs;
    }

    friend timingBase operator-(timingBase lhs, timingBase rhs)
    {
        assert(lhs.clock_period == rhs.clock_period);
        if (rhs.latency < 0) // for operation like br/ret
        {
            lhs.latency--;
            return lhs;
        }
        lhs.latency -= rhs.latency;
        return lhs;
    }

    friend bool operator>(timingBase lhs, timingBase rhs)
    {
        assert(lhs.clock_period == rhs.clock_period);
        return (((lhs.latency > rhs.latency)) ||
                (lhs.latency == rhs.latency && lhs.timing > rhs.timing));
    }

    friend bool operator>=(timingBase lhs, timingBase rhs)
    {
        assert(lhs.clock_period == rhs.clock_period);
        return (((lhs.latency > rhs.latency)) ||
                (lhs.latency == rhs.latency && lhs.timing >= rhs.timing));
    }

    friend timingBase operator*(timingBase lhs, int rhs)
    {
        if (lhs.timing > 0.0001)
        {
            lhs.latency++;
            lhs.timing = 0;
        }
        lhs.latency *= rhs;
        return lhs;
    }

    friend timingBase operator*(int rhs, timingBase lhs)
    {
        if (lhs.timing > 0.0001)
        {
            lhs.latency++;
            lhs.timing = 0;
        }
        lhs.latency *= rhs;
        return lhs;
    }

    friend timingBase operator/(timingBase lhs, int rhs)
    {
        if (lhs.timing > 0.0001)
        {
            lhs.latency++;
            lhs.timing = 0;
        }
        lhs.latency /= rhs;
        return lhs;
    }

    friend raw_ostream &operator<<(raw_ostream &stream, const timingBase &tb)
    {
        stream << " [latency=" << tb.latency << ", timing=" << tb.timing
               << " @period=" << tb.clock_period << "] ";
        return stream;
    }

    // A unit class to record the FPGA resource for a(n) instruction/block/function/loop
    class resourceBase
    {
      public:
        resourceBase(int D, int F, int L, double C)
        {
            DSP = D;
            FF = F;
            LUT = L;
            clock_period = C;
            BRAM = 0;
        }
        resourceBase(int B, int D, int F, int L, double C)
        {
            DSP = D;
            FF = F;
            LUT = L;
            clock_period = C;
            BRAM = B;
        }
        int DSP, FF, LUT;
        int BRAM = 0;
        double clock_period;

        resourceBase &operator=(const resourceBase &input)
        {
            DSP = input.DSP;
            FF = input.FF;
            LUT = input.LUT;
            BRAM = input.BRAM;
            clock_period = input.clock_period;
        }

        resourceBase(const resourceBase &input)
        {
            DSP = input.DSP;
            FF = input.FF;
            LUT = input.LUT;
            BRAM = input.BRAM;
            clock_period = input.clock_period;
        }

        resourceBase()
        {
            DSP = -1;
            FF = -1;
            LUT = -1;
            clock_period = -1;
            BRAM = -1;
        }
    };

    friend resourceBase operator+(resourceBase lhs, resourceBase rhs)
    {
        assert(lhs.clock_period == rhs.clock_period);
        lhs.DSP = lhs.DSP + rhs.DSP;
        lhs.FF = lhs.FF + rhs.FF;
        lhs.LUT = lhs.LUT + rhs.LUT;
        lhs.BRAM = lhs.BRAM + rhs.BRAM;
        return lhs;
    }

    friend resourceBase operator-(resourceBase lhs, resourceBase rhs)
    {
        assert(lhs.clock_period == rhs.clock_period);
        lhs.DSP = lhs.DSP - rhs.DSP;
        lhs.FF = lhs.FF - rhs.FF;
        lhs.LUT = lhs.LUT - rhs.LUT;
        lhs.BRAM = lhs.BRAM - rhs.BRAM;
        return lhs;
    }

    friend resourceBase operator*(resourceBase lhs, int rhs)
    {
        lhs.DSP = lhs.DSP * rhs;
        lhs.FF = lhs.FF * rhs;
        lhs.LUT = lhs.LUT * rhs;
        lhs.BRAM = lhs.BRAM * rhs;
        return lhs;
    }

    friend resourceBase operator*(int rhs, resourceBase lhs)
    {
        lhs.DSP = lhs.DSP * rhs;
        lhs.FF = lhs.FF * rhs;
        lhs.LUT = lhs.LUT * rhs;
        lhs.BRAM = lhs.BRAM * rhs;
        return lhs;
    }

    friend raw_ostream &operator<<(raw_ostream &stream, const resourceBase &rb)
    {
        stream << " [DSP=" << rb.DSP << ", FF=" << rb.FF << ", LUT=" << rb.LUT
               << ", BRAM=" << rb.BRAM << "] ";
        return stream;
    }

    resourceBase topFunction_resource; // = resourceBase();

    // A map from opcode to the information list of timing and resource
    std::map<std::string, Info_type_list> &BiOp_Info_name2list_map;

    std::map<std::string, Info_type_list> BiOp_Info_name2list_map_contain;

    // get the information of a specific instruction, based on its opcode, operand_bitwidth,
    // result_bitwidth and clock period
    inst_timing_resource_info get_inst_info(std::string opcode, int operand_bitwid,
                                            int res_bitwidth, std::string period);

    // Organize the information into timingBase after getting the information of a specific
    // instruction, based on its opcode, operand_bitwidth, result_bitwidth and clock period.
    timingBase get_inst_TimingInfo_result(std::string opcode, int operand_bitwid, int res_bitwidth,
                                          std::string period);

    // Organize the information into resourceBase after getting the information of a specific
    // instruction, based on its opcode, operand_bitwidth, result_bitwidth and clock period.
    resourceBase get_inst_ResourceInfo_result(std::string opcode, int operand_bitwid,
                                              int res_bitwidth, std::string period);

    // int get_N_DSP(std::string opcode, int operand_bitwid , int res_bitwidth, std::string period);
    // int get_N_FF(std::string opcode, int operand_bitwid , int res_bitwidth, std::string period);
    // int get_N_LUT(std::string opcode, int operand_bitwid , int res_bitwidth, std::string period);
    // int get_N_Lat(std::string opcode, int operand_bitwid , int res_bitwidth, std::string period);
    // double get_N_Delay(std::string opcode, int operand_bitwid , int res_bitwidth, std::string
    // period);

    // evaluate the number of FF needed by the instruction
    resourceBase FF_Evaluate(std::map<Instruction *, timingBase> &cur_InstructionCriticalPath,
                             Instruction *cur_I);

    // trace back to find the original operator, bypassing SExt and ZExt operations
    Value *byPassUnregisterOp(Instruction *cur_I);

    Value *byPassBitcastOp(Instruction *cur_I);

    Value *byPassBitcastOp(Value *cur_I_val);

    // evaluate the number of LUT needed by the PHI instruction
    resourceBase IndexVar_LUT(std::map<Instruction *, timingBase> &cur_InstructionCriticalPath,
                              Instruction *I);

    // check whether a specific information is in the database
    bool checkInfoAvailability(std::string opcode, int operand_bitwid, int res_bitwidth,
                               std::string period);

    // check whether we can infer the information by increasing the clock frequency
    bool checkFreqProblem(std::string opcode, int operand_bitwid, int res_bitwidth,
                          std::string period);

    // if the information is not found in database, we may infer the information by increasing the
    // clock frequency
    inst_timing_resource_info checkInfo_HigherFreq(std::string opcode, int operand_bitwid,
                                                   int res_bitwidth, std::string period);

    // Trace back to get the bitwidth of an operand, bypassing truct/zext/sext
    int getOriginalBitwidth(Value *Val);

    // Trace forward to get the number of users, bypassing truct/zext/sext
    int getActualUsersNum(Instruction *I, int dep);

    // check whether the two operations can be chained
    bool canCompleteChainOrNot(Instruction *PredI, Instruction *I);

    // mainly reduce resource while delay reduced relatively unsiginificantly
    bool canPartitalChainOrNot(Instruction *PredI, Instruction *I);

    // check whether the two operations can be chained into a MAC operation
    bool isMACpossible(Instruction *PredI, Instruction *I);

    // check whether the three operations can be chained into a AMA operation
    bool isAMApossible(Instruction *PredI, Instruction *I);

    // check whether the two operations can be chained into a Ternary Adder operation
    bool isTernaryAddpossible(Instruction *PredI, Instruction *I);

    // get the timing overhead of the partial chaining
    timingBase getPartialTimingOverhead(Instruction *PredI, Instruction *I);

    // get the resource overhead of the partial chaining
    resourceBase getPartialResourceOverhead(Instruction *PredI, Instruction *I);

    std::set<Instruction *> chained_I;

    //////////////////////////////////////////////////////////////////////////////////////
    ////////// Implemente the SCEV which HI needs (bypass SExt/ZExt/Trunc) ///////////////
    //////////////////////////////////////////////////////////////////////////////////////

    const SCEV *tryGetPureAddOrAddRecSCEV(const SCEV *inputS, int depth);

    const SCEV *bypassExtTruntSCEV(const SCEV *inputS);

    const SCEV *findUnknown(const SCEV *ori_inputS);

    // get the unknown values in the expression
    int getUnknownNum(const SCEV *ori_inputS);

    //////////////////////////////////////////////////////////////////////////////////////
    //////////////////// Declaration related to Memory Access Tracing ////////////////////
    //////////////////////////////////////////////////////////////////////////////////////

    // record which target arrays the instruction may access
    std::map<Instruction *, std::vector<Value *>> Access2TargetMap;

    // record that in the basic block, which instruction access which array at which cycle
    std::map<Value *,
             std::map<BasicBlock *,
                      std::vector<std::pair<std::pair<int, partition_info>, Instruction *>>>>
        target2LastAccessCycleInBlock;
    //  the first int is for time slot
    //  and the second int is for partition

    // record the access take place in which cycle
    std::map<std::pair<Instruction *, std::pair<Value *, partition_info>>, timingBase>
        scheduledAccess_timing;

    // record the access take place in which cycle
    std::map<BasicBlock *, std::map<std::pair<Value *, partition_info>, int>> accessCounterForBlock;

    // register number for target array
    std::vector<Instruction *> AccessesList;

    // alias of array from functions to subfunctions
    // indicate which actual array the pointer argument of the function
    // is referred to
    std::map<Value *, Value *> Alias2Target;

    std::map<BasicBlock *, std::vector<Instruction *>> Block2AccessList;

    std::map<std::pair<Instruction *, Instruction *>, int> InstInst2DependenceDistance;

    // Trace Memory Declaration in Module
    // analyze BRAM accesses in the module before any other analysis
    void TraceMemoryDeclarationAndAnalyzeAccessinModule(llvm::Module &M);

    // find the array declaration in the function F and trace the accesses to them
    void findMemoryDeclarationAndAnalyzeAccessin(Function *F, bool isTopFunction);

    // get the number of BRAMs which are needed by the alloca instruction
    resourceBase get_BRAM_Num_For(AllocaInst *alloca_I);

    // get the number of BRAMs which are needed by the argument of topfunction interface
    resourceBase get_BRAM_Num_For(Argument *ArgTarget);

    // get the number of BRAMs which are needed by the array with specific parameters
    resourceBase get_BRAM_Num_For(int bitwidth, int depth);

    // Calculate the BRAM cost (BRAM+MUX LUT) for the top function
    resourceBase BRAMRelatedCostForTopFunction(Function &F);

    // get the depth of each partition of the target array
    int getEachPartitionDepth(HI_ArrayInfo &refInfo);

    // get the total number of partitions of the target array
    int getTotalPartitionNum(HI_ArrayInfo &refInfo);

    // find out which instrctuins are related to the array, going through PtrToInt, Add, IntToPtr,
    // Store, Load instructions record the corresponding target array which the access instructions
    // try to touch
    void TraceAccessForTarget(Value *cur_node, Value *ori_node);

    // check whether the access to target array can be scheduled in a specific cycle
    bool checkBRAMAvailabilty(Instruction *access, Value *target, std::string StoreOrLoad,
                              BasicBlock *cur_block, timingBase cur_Timing,
                              partition_info target_partition);

    // schedule the access to potential target (since an instructon may use the address for
    // different target (e.g. address comes from PHINode) or different parttions, we need to
    // schedule all of them)
    timingBase scheduleBRAMAccess(Instruction *access, BasicBlock *cur_block, timingBase cur_Timing,
                                  partition_info target_partition);

    // schedule the access to specific target for the instruction
    timingBase handleBRAMAccessFor(Instruction *access, Value *target, BasicBlock *cur_block,
                                   timingBase cur_Timing, partition_info target_partition);

    // record the schedule information
    void insertBRAMAccessInfo(Value *target, BasicBlock *cur_block, int cur_latency,
                              Instruction *access, partition_info target_partition);

    // evaluate the number of LUT needed by the BRAM Mux
    resourceBase BRAM_MUX_Evaluate();

    // for load instructions, HLS will reuse the register for the data
    bool checkLoadOpRegisterReusable(Instruction *Load_I, int time_point);

    // handle the information of memory access for the function
    // get the offset of the access to array and get the access information
    // in the function
    void ArrayAccessCheckForFunction(Function *F);

    // find the array access in the function F and trace the accesses to them
    void findMemoryAccessin(Function *F);

    // find out which instrctuins are related to the array, going through PtrToInt, Add, IntToPtr,
    // Store, Load instructions
    void TraceAccessRelatedInstructionForTarget(Value *cur_node);

    // check the memory access in the function
    void TraceMemoryAccessinFunction(Function &F);

    // if it is a memory access instruction, calculate the array access offset for it.
    void TryArrayAccessProcess(Instruction *I, ScalarEvolution *SE);

    // handle the situation for access instruction, which address come from Value or allocaInst
    bool processNaiveAccess(Instruction *I);

    // handle standard SARE and extract array access information from it
    void handleSAREAccess(Instruction *I, const SCEVAddRecExpr *SARE);

    void handleDirectAccess(Instruction *I, const SCEVUnknown *SUnkown);

    void handleConstantOffsetAccess(Instruction *I, const SCEVAddExpr *SAE);

    // handle non-standard SARE, where the pointer value is in the outermost expression,
    // and extract array access information from it
    void handleUnstandardSCEVAccess(Instruction *I, const SCEV *tmp_S);

    // handle complex SCEV, based on which we cannot predict the array access information
    void handleComplexSCEVAccess(Instruction *I, const SCEV *tmp_S);

    // get the initial index of the array access in the loop
    const SCEV *findTheActualStartValue(const SCEVAddRecExpr *S);

    // get the index incremental value of the array access and the trip counts in the loop
    void findTheIncrementalIndexAndTripCount(const SCEVAddRecExpr *S, std::vector<int> &inc_indices,
                                             std::vector<int> &trip_counts);

    // generate AccessInformation according to the target and the initial access
    HI_AccessInfo getAccessInfoFor(Value *target, Instruction *access, int initial_offset,
                                   std::vector<int> *inc_indices, std::vector<int> *trip_counts,
                                   bool unpredictable = false);

    // get the exact access information for a specific load or store information
    HI_AccessInfo getAccessInfoForAccessInst(Instruction *Load_or_Store);

    // get the targer partition according to the specific memory access instruction
    std::vector<partition_info> getPartitionFor(Instruction *access);

    // try to add the partiton into the record list, if there is no duplicated partition
    bool tryRecordPartition(std::vector<partition_info> &partitions, partition_info try_target);

    // check whether there is RAW hazard for the load instruction in the basic block,
    // if it is scheduled at the specific cycle
    bool hasRAWHazard(Instruction *loadI, int cycle);

    // get the target array from the load/store instruction
    Value *getTargetFromInst(Instruction *accessI);

    // get the target array from the call instruction
    std::vector<Value *> getTargetsFromInst(Instruction *accessI);

    // get new partition information according to a new "cur_offset" in the array
    // refInfo can provide arrayInfo and pragmaInfo for the analysis
    partition_info getAccessPartitionBasedOnAccessInfoAndInc(HI_AccessInfo refInfo, int cur_offset);

    // according to the initial offset, trip counts of loop, and incremental per iteration
    // generate a list of possible offset for a specific access. The list will be used for
    // partition check
    std::vector<int> generatePotentialOffset(HI_AccessInfo &accessInfo);

    // recursively emulate all the loops where the access is inside
    // to check the offset of the access
    void getAllPotentialOffsetByRecuresiveSearch(HI_AccessInfo &accessInfo, int loopDep,
                                                 int last_level_offset, std::vector<int> &res);

    // get all the partitions for the access target of the access instruction
    std::vector<partition_info> getAllPartitionFor(Instruction *access);

    // get all the partitions for the access target of the access instruction
    void getAllPartitionBasedOnInfo(HI_AccessInfo &info, int curDim, std::vector<int> &tmp_partID,
                                    std::vector<partition_info> &res);

    // check whether the two SCEV have const distance
    Optional<APInt> computeConstantDifference(const SCEV *More, const SCEV *Less);

    // check whether the two instructions have loop carried dependence
    // if there is such dependence, record the distance in InstInst2DependenceDistance
    void checkLoopCarriedDependent(Instruction *I0, Instruction *I1, Loop *curLoop);

    // some accesses with offset which is not related to the loop of current level
    bool checkConstantAccessInLoop(const SCEV *ori_More, const SCEV *ori_Less, Loop *curLoop);
    // get the step length (stride) of the access
    int getStepLength(const SCEV *More, const SCEV *Less);

    bool splitBinaryAdd(const SCEV *Expr, const SCEV *&L, const SCEV *&R, SCEV::NoWrapFlags &Flags);

    // check whether the two instructions could be alias instructions
    // true if it is possible, false if not possible
    bool noAliasHazard(Instruction *I0, Instruction *I1);

    // check whether the array partitioning helps for parallelism
    void checkPartitionBenefit(std::vector<partition_info> &partitions, Instruction *accessI);

    class HI_ArrayInfo
    {
      public:
        int dim_size[10];
        int sub_element_num[10];
        int partition_size[10];
        bool cyclic[10];
        int num_dims;
        bool isArgument = 0;
        bool completePartition = 0;
        llvm::Type *elementType;
        Value *target;
        int port_num = -1;
        HI_ArrayInfo()
        {
            num_dims = 10;
            for (int i = 0; i < num_dims; i++)
                dim_size[i] = -1;
            for (int i = 0; i < num_dims; i++)
                sub_element_num[i] = -1;
            for (int i = 0; i < num_dims; i++)
                partition_size[i] = 1;
            for (int i = 0; i < num_dims; i++)
                cyclic[i] = 1;
            num_dims = -1;
        }
        HI_ArrayInfo(const HI_ArrayInfo &input)
        {
            elementType = input.elementType;
            num_dims = input.num_dims;
            target = input.target;
            isArgument = input.isArgument;
            port_num = input.port_num;
            completePartition = input.completePartition;
            for (int i = 0; i < num_dims; i++)
                dim_size[i] = input.dim_size[i];
            for (int i = 0; i < num_dims; i++)
                sub_element_num[i] = input.sub_element_num[i];
            for (int i = 0; i < num_dims; i++)
                partition_size[i] = input.partition_size[i];
            for (int i = 0; i < num_dims; i++)
                cyclic[i] = input.cyclic[i];
        }
        HI_ArrayInfo &operator=(const HI_ArrayInfo &input)
        {
            elementType = input.elementType;
            num_dims = input.num_dims;
            target = input.target;
            isArgument = input.isArgument;
            port_num = input.port_num;
            completePartition = input.completePartition;
            for (int i = 0; i < num_dims; i++)
                dim_size[i] = input.dim_size[i];
            for (int i = 0; i < num_dims; i++)
                sub_element_num[i] = input.sub_element_num[i];
            for (int i = 0; i < num_dims; i++)
                partition_size[i] = input.partition_size[i];
            for (int i = 0; i < num_dims; i++)
                cyclic[i] = input.cyclic[i];
        }
    };

    class HI_AccessInfo
    {
      public:
        int dim_size[10];
        int sub_element_num[10];

        int partition_size[10];
        int index[10];
        int partition_id[10];
        bool cyclic[10];

        bool completePartition = 0;

        int num_dims;
        int partition = -1;
        bool isArgument = 0;
        bool unpredictable = 0;
        int inc_index[10];
        int trip_count[10];
        int initial_offset = -1;
        int reverse_loop_dep = -1;
        llvm::Type *elementType;
        Value *target;
        int port_num = -1;

        HI_AccessInfo()
        {
            num_dims = 10;
            for (int i = 0; i < num_dims; i++)
                dim_size[i] = -1;
            for (int i = 0; i < num_dims; i++)
                sub_element_num[i] = -1;
            for (int i = 0; i < num_dims; i++)
                cyclic[i] = 1;
            for (int i = 0; i < num_dims; i++)
                partition_size[i] = 1;
            for (int i = 0; i < num_dims; i++)
                partition_id[i] = 0;
            num_dims = -1;
            reverse_loop_dep = -1;
        }
        HI_AccessInfo(const HI_AccessInfo &input)
        {
            assert(input.target);
            assert(input.num_dims > 0);
            elementType = input.elementType;
            num_dims = input.num_dims;
            target = input.target;
            isArgument = input.isArgument;
            partition = input.partition;
            initial_offset = input.initial_offset;
            unpredictable = input.unpredictable;
            port_num = input.port_num;
            completePartition = input.completePartition;
            for (int i = 0; i < 10; i++)
            {
                dim_size[i] = -1;
                sub_element_num[i] = -1;
                index[i] = -1;
            }
            reverse_loop_dep = input.reverse_loop_dep;
            for (int i = 0; i < reverse_loop_dep; i++)
                inc_index[i] = input.inc_index[i];
            for (int i = 0; i < reverse_loop_dep; i++)
                trip_count[i] = input.trip_count[i];
            for (int i = 0; i < num_dims; i++)
                dim_size[i] = input.dim_size[i];
            for (int i = 0; i < num_dims; i++)
                sub_element_num[i] = input.sub_element_num[i];
            for (int i = 0; i < num_dims; i++)
                index[i] = input.index[i];
            for (int i = 0; i < num_dims; i++)
                partition_size[i] = input.partition_size[i];
            for (int i = 0; i < num_dims; i++)
                partition_id[i] = input.partition_id[i];
            for (int i = 0; i < num_dims; i++)
                cyclic[i] = input.cyclic[i];
        }
        HI_AccessInfo(const HI_ArrayInfo &input)
        {
            assert(input.target);
            elementType = input.elementType;
            num_dims = input.num_dims;
            target = input.target;
            isArgument = input.isArgument;
            port_num = input.port_num;
            completePartition = input.completePartition;
            for (int i = 0; i < 10; i++)
            {
                dim_size[i] = -1;
                sub_element_num[i] = -1;
                index[i] = -1;
            }
            for (int i = 0; i < num_dims; i++)
                dim_size[i] = input.dim_size[i];
            for (int i = 0; i < num_dims; i++)
                sub_element_num[i] = input.sub_element_num[i];
            for (int i = 0; i < num_dims; i++)
                partition_size[i] = input.partition_size[i];
            for (int i = 0; i < num_dims; i++)
                cyclic[i] = input.cyclic[i];
        }
        HI_AccessInfo &operator=(const HI_AccessInfo &input)
        {
            assert(input.target);
            assert(input.num_dims > 0);
            elementType = input.elementType;
            num_dims = input.num_dims;
            target = input.target;
            isArgument = input.isArgument;
            partition = input.partition;
            initial_offset = input.initial_offset;
            unpredictable = input.unpredictable;
            port_num = input.port_num;
            completePartition = input.completePartition;

            for (int i = 0; i < 10; i++)
            {
                dim_size[i] = -1;
                sub_element_num[i] = -1;
                index[i] = -1;
            }
            reverse_loop_dep = input.reverse_loop_dep;
            for (int i = 0; i < reverse_loop_dep; i++)
                inc_index[i] = input.inc_index[i];
            for (int i = 0; i < reverse_loop_dep; i++)
                trip_count[i] = input.trip_count[i];
            for (int i = 0; i < num_dims; i++)
                dim_size[i] = input.dim_size[i];
            for (int i = 0; i < num_dims; i++)
                sub_element_num[i] = input.sub_element_num[i];
            for (int i = 0; i < num_dims; i++)
                index[i] = input.index[i];
            for (int i = 0; i < num_dims; i++)
                partition_size[i] = input.partition_size[i];
            for (int i = 0; i < num_dims; i++)
                partition_id[i] = input.partition_id[i];
            for (int i = 0; i < num_dims; i++)
                cyclic[i] = input.cyclic[i];
        }
        HI_AccessInfo &operator=(const HI_ArrayInfo &input)
        {
            assert(input.target);
            elementType = input.elementType;
            num_dims = input.num_dims;
            target = input.target;
            isArgument = input.isArgument;
            port_num = input.port_num;
            completePartition = input.completePartition;

            for (int i = 0; i < 10; i++)
            {
                dim_size[i] = -1;
                sub_element_num[i] = -1;
                index[i] = -1;
            }
            for (int i = 0; i < num_dims; i++)
                dim_size[i] = input.dim_size[i];
            for (int i = 0; i < num_dims; i++)
                sub_element_num[i] = input.sub_element_num[i];
            for (int i = 0; i < num_dims; i++)
                partition_size[i] = input.partition_size[i];
            for (int i = 0; i < num_dims; i++)
                cyclic[i] = input.cyclic[i];
        }

        void setPartitionId(const partition_info &input)
        {
            assert(num_dims == input.num_dims);
            for (int i = 0; i < num_dims; i++)
                partition_id[i] = input.partition_id[i];
        }
    };

    class partition_info
    {
      public:
        int num_dims = -1;
        int partition_id[10];
        Value *target;
        int port_num = -1;
        partition_info()
        {
            num_dims = 10;
            for (int i = 0; i < num_dims; i++)
                partition_id[i] = 0;
            num_dims = -1;
            port_num = -1;
            target = nullptr;
        }

        partition_info operator=(const partition_info &input)
        {
            assert(input.target);
            target = input.target;
            num_dims = input.num_dims;
            port_num = input.port_num;
            for (int i = 0; i < num_dims; i++)
                partition_id[i] = input.partition_id[i];
        }

        partition_info(const partition_info &input)
        {
            assert(input.target);
            target = input.target;
            num_dims = input.num_dims;
            port_num = input.port_num;
            for (int i = 0; i < num_dims; i++)
                partition_id[i] = input.partition_id[i];
        }
    };

    friend bool operator==(const partition_info &lhs, const partition_info &rhs)
    {
        if (lhs.target != rhs.target)
        {
            return false;
        }
        assert(lhs.num_dims == rhs.num_dims);

        for (int i = 0; i < lhs.num_dims; i++)
            if (lhs.partition_id[i] != rhs.partition_id[i])
                return false;
        return true;
    }

    friend raw_ostream &operator<<(raw_ostream &stream, const partition_info &tb)
    {
        stream << "partition_id=";
        stream << "(" << tb.target << ")" << tb.target->getName() << "[";

        for (int i = tb.num_dims - 1; i > 0; i--)
        {
            stream << tb.partition_id[i] << "][";
        }
        stream << tb.partition_id[0] << "]   port_num=";
        stream << tb.port_num << " ";
    }

    friend raw_ostream &operator<<(raw_ostream &stream, const HI_ArrayInfo &tb)
    {
        stream << "HI_ArrayInfo for: << (" << tb.target << ") " << *tb.target
               << ">> [ele_Type= " << *tb.elementType << ", num_dims=" << tb.num_dims << ", ";
        for (int i = 0; i < tb.num_dims; i++)
        {
            stream << "dim-" << i << "-size=" << tb.dim_size[i] << ", ";
        }

        for (int i = 0; i < tb.num_dims; i++)
        {
            stream << "dim-" << i << "-subnum=" << tb.sub_element_num[i] << ", ";
        }

        for (int i = 0; i < tb.num_dims; i++)
        {
            stream << "dim-" << i << "-partitionSize=" << tb.partition_size[i] << ", ";
        }

        stream << "] ";

        stream << " port_num=" << tb.port_num << " ";

        // timing="<<tb.timing<<"] ";
        return stream;
    }

    friend raw_ostream &operator<<(raw_ostream &stream, const HI_AccessInfo &tb)
    {
        // stream << "HI_AccessInfo for: <<" << *tb.target << ">> [ele_Type= " << *tb.elementType <<
        // ", num_dims=" << tb.num_dims << ", ";// ", partition=" << tb.partition << ", ";

        stream << "HI_AccessInfo for: <<" << *tb.target << ">> [num_dims=" << tb.num_dims << ", ";

        for (int i = 0; i < tb.num_dims; i++)
        {
            stream << "dim-" << i << "-size=" << tb.dim_size[i] << ", ";
        }

        for (int i = 0; i < tb.num_dims; i++)
        {
            stream << "dim-" << i << "-subnum=" << tb.sub_element_num[i] << ", ";
        }

        for (int i = 0; i < tb.num_dims; i++)
        {
            if (tb.unpredictable)
                stream << "dim-" << i << "-index=?"
                       << ", ";
            else
                stream << "dim-" << i << "-index=" << tb.index[i] << ", ";
        }

        stream << "representation: " << tb.target->getName() << "[";

        for (int i = tb.num_dims - 1; i > 0; i--)
        {
            if (tb.unpredictable)
                stream << "?][";
            else
                stream << tb.index[i] << "][";
        }
        if (tb.unpredictable)
            stream << "?]   ";
        else
            stream << tb.index[0] << "]   ";

        stream << "partition_id=[";

        for (int i = tb.num_dims - 1; i > 0; i--)
        {
            if (tb.unpredictable)
                stream << "?][";
            else
                stream << tb.partition_id[i] << "][";
        }
        if (tb.unpredictable)
            stream << "?]   ";
        else
            stream << tb.partition_id[0] << "]   ";

        stream << "] ";
        stream << " port_num=" << tb.port_num << " ";
        stream.flush();
        // timing="<<tb.timing<<"] ";
        return stream;
    }

    std::set<Value *> ArrayValueVisited;

    std::set<Instruction *> Inst_AccessRelated;

    raw_fd_ostream *ArrayLog;

    std::map<Instruction *, std::vector<partition_info>> Inst2Partitions;

    // indicate whether array partition helps for the target array
    std::map<Value *, bool> Target2PartitionBenefit;
    std::map<std::pair<std::string, std::string>, bool> FuncArray2PartitionBenefit;

    // record whether the access address will require remainder operation
    std::map<Instruction *, bool> Access2EnableArrayNo;

    std::map<partition_info, std::pair<int, int>> partition2cnt;

    // record the array informtiion
    std::map<Value *, HI_ArrayInfo> Target2ArrayInfo;

    // according to the address instruction (ptr+offset), get the access information
    std::map<Value *, HI_AccessInfo> AddressInst2AccessInfo;

    // get the array information, including the dimension, type and size
    HI_ArrayInfo getArrayInfo(Value *target);

    class HI_AAResult : public AAResultBase<HI_AAResult>
    {
      public:
        explicit HI_AAResult() : AAResultBase()
        {
        }
        HI_AAResult(HI_AAResult &&Arg) : AAResultBase(std::move(Arg))
        {
        }

        AliasResult alias(const MemoryLocation &LocA, const MemoryLocation &LocB);
    };

    // check whether there is MUST-alias access for the instruction in its basic block
    void checkAliasFor(Instruction *I);

    // check whether the two instructions are MUST-alias
    bool checkAccessAlias(Instruction *I0, Instruction *I1);

    // print out all the aliases for the instruction
    void printAliasFor(Instruction *I);

    std::set<std::pair<Instruction *, Instruction *>> accessAliasSet;
    std::map<Instruction *, std::vector<Instruction *>> accessAliasMap;

    /////////////////////////////////////////////////////////////////////////////
    ///////////////////////////// Analyze the Reuse of DSPs//////////////////////
    /////////////////////////////////////////////////////////////////////////////

    // analyze DSP usage and try to reuse DSP
    class DSPReuseScheduleUnit
    {
      public:
        DSPReuseScheduleUnit(Instruction *opI, int DSPcost, int opcode, int timeslot_inBlock,
                             partition_info LPartition, partition_info RPartition)
            : opI(opI), DSPcost(DSPcost), LPartition(LPartition), RPartition(RPartition),
              opcode(opcode), timeslot_inBlock(timeslot_inBlock)
        {
            partitionBased = 1;
            DSPforIntegerComputation = 1;
        }

        DSPReuseScheduleUnit(Instruction *opI, int DSPcost, int opcode, int timeslot_inBlock,
                             Value *LVal, Value *RVal)
            : opI(opI), DSPcost(DSPcost), LVal(LVal), RVal(RVal), opcode(opcode),
              timeslot_inBlock(timeslot_inBlock)
        {
            DSPforIntegerComputation = 1;
        }

        DSPReuseScheduleUnit(Instruction *opI, int DSPcost, int opcode, int timeslot_inBlock)
            : opI(opI), DSPcost(DSPcost), opcode(opcode), timeslot_inBlock(timeslot_inBlock)
        {
            DSPforIntegerComputation = 0;
        }

        DSPReuseScheduleUnit(const DSPReuseScheduleUnit &input)
        {
            DSPcost = input.DSPcost;
            opcode = input.opcode;
            LVal = input.LVal;
            RVal = input.RVal;
            DSPforIntegerComputation = input.DSPforIntegerComputation;
            partitionBased = input.partitionBased;
            if (partitionBased)
            {
                if (DSPforIntegerComputation)
                {
                    LPartition = input.LPartition;
                    RPartition = input.RPartition;
                }
            }
            timeslot_inBlock = input.timeslot_inBlock;
            opI = input.opI;
        }

        DSPReuseScheduleUnit &operator=(const DSPReuseScheduleUnit &input)
        {
            DSPcost = input.DSPcost;
            opcode = input.opcode;
            LVal = input.LVal;
            RVal = input.RVal;
            DSPforIntegerComputation = input.DSPforIntegerComputation;
            partitionBased = input.partitionBased;
            if (partitionBased)
            {
                if (DSPforIntegerComputation)
                {
                    LPartition = input.LPartition;
                    RPartition = input.RPartition;
                }
            }

            timeslot_inBlock = input.timeslot_inBlock;
            opI = input.opI;
        }

        int DSPcost;
        unsigned opcode;
        bool partitionBased = 0;
        partition_info LPartition;
        partition_info RPartition;
        Value *LVal = nullptr;
        Value *RVal = nullptr;
        int timeslot_inBlock;
        Instruction *opI;
        bool DSPforIntegerComputation = 0;
    };

    friend bool operator==(const DSPReuseScheduleUnit &lhs, const DSPReuseScheduleUnit &rhs)
    {
        if (!(lhs.opcode == rhs.opcode))
            return false;

        if (lhs.DSPforIntegerComputation)
        {
            if (lhs.partitionBased != rhs.partitionBased)
                return false;
            if (lhs.partitionBased)
            {
                if (!lhs.LPartition.target || !lhs.RPartition.target)
                {
                    llvm::errs() << "lhs.opI: " << *lhs.opI << " rhs.opI: " << *rhs.opI << "\n";
                    assert(false && "target should exit");
                }
                if (!(lhs.LPartition == rhs.LPartition))
                    return false;
                if (!(lhs.RPartition == rhs.RPartition))
                    return false;
            }
            else
            {

                Instruction *LInst = dyn_cast<Instruction>(lhs.LVal);
                Instruction *RInst = dyn_cast<Instruction>(rhs.LVal);

                if ((!LInst) || (!RInst))
                    return false;

                if (LInst->getOpcode() == Instruction::Load ||
                    RInst->getOpcode() == Instruction::Load)
                    return false;

                if (LInst && RInst)
                {
                    if (LInst->getOpcode() != RInst->getOpcode())
                        return false;
                    if (LInst->getType() != RInst->getType())
                        return false;
                }
                else
                {
                    if (lhs.LVal != lhs.LVal)
                        return false;
                }

                LInst = dyn_cast<Instruction>(lhs.RVal);
                RInst = dyn_cast<Instruction>(rhs.RVal);

                if ((!LInst) || (!RInst))
                    return false;

                if (LInst->getOpcode() == Instruction::Load ||
                    RInst->getOpcode() == Instruction::Load)
                    return false;

                if (LInst && RInst)
                {
                    if (LInst->getOpcode() != RInst->getOpcode())
                        return false;
                    if (LInst->getType() != RInst->getType())
                        return false;
                }
                else
                {
                    if (lhs.RVal != lhs.RVal)
                        return false;
                }
            }
        }
        else
        {
            if (lhs.opI->getType()->isDoubleTy() != rhs.opI->getType()->isDoubleTy())
                return false;
        }

        if (!(lhs.DSPcost == rhs.DSPcost))
            return false;
        return true;
    }

    friend raw_ostream &operator<<(raw_ostream &stream, const DSPReuseScheduleUnit &rb)
    {
        if (rb.DSPforIntegerComputation)
        {
            if (rb.partitionBased)
            {
                stream << " [refInst=" << *rb.opI << ", timeslot=" << rb.timeslot_inBlock
                       << ", DSPCOST=" << rb.DSPcost << ", LPartition=" << rb.LPartition
                       << ", RPartition=" << rb.RPartition << "] ";
            }
            else
            {
                stream << " [refInst=" << *rb.opI << ", timeslot=" << rb.timeslot_inBlock
                       << ", DSPCOST=" << rb.DSPcost << ", LVal=" << *rb.LVal
                       << ", RVal=" << *rb.RVal << "] ";
            }
        }

        else
            stream << " [refInst=" << *rb.opI << ", timeslot=" << rb.timeslot_inBlock
                   << ", DSPCOST=" << rb.DSPcost << "] ";

        return stream;
    }

    std::map<BasicBlock *, std::vector<DSPReuseScheduleUnit>> Block2IntDSPReuseScheduleUnits;
    std::map<BasicBlock *, std::map<std::string, std::vector<DSPReuseScheduleUnit>>>
        Block2FPDSPReuseScheduleUnits;
    std::map<BasicBlock *, std::map<std::string, int>> Block2FPDSPOpCnt;

    // check whether the DSP used for this instruction could be reused by the others
    // reuse condition: operator / bitwidth / data source (partition) are the same
    bool checkAndTryRecordReuseOperatorDSP(Instruction *I, int timeslot);

    /////////////////////////////////////////////////////////////////////////////////////////////
    ////////////////////// Parse the configuration for the program //////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////

    std::map<Value *, std::vector<HI_PragmaInfo>> arrayDirectives;
    std::map<BasicBlock *, HI_PragmaInfo> LoopDirectives;
    std::vector<HI_PragmaInfo> PragmaInfo_List;
    std::map<std::string, std::string> IRLoop2LoopLabel;
    std::map<std::string, int> IRLoop2OriginTripCount;
    std::map<std::string, int> LoopLabel2II;
    std::map<std::string, int> LoopLabel2UnrollFactor;
    std::map<std::string, int> &FuncParamLine2OutermostSize;
    std::map<std::string, std::vector<int>> &IRFunc2BeginLine;

    class HI_PragmaInfo
    {
      public:
        enum pragmaType
        {
            arrayPartition_Pragma,
            loopUnroll_Pragma,
            loopPipeline_Pragma,
            arrayPortNum_Pragma,
            funcDataflow_Pragma,
            localArray_Pragma,
            unkown_Pragma
        };
        pragmaType HI_PragmaInfoType = unkown_Pragma;
        std::string targetStr, scopeStr, labelStr;
        int II, dim, unroll_factor, partition_factor, port_num;
        bool cyclic = 1, dataflowEnable = 0, localArrayEnable = 0, complete = 0;
        Value *targetArray;
        BasicBlock *targetLoop;
        Function *ScopeFunc;

        HI_PragmaInfo()
        {
            II = -1;
            unroll_factor = -1;
            partition_factor = -1;
            dim = -1;
            port_num = -1;
            HI_PragmaInfoType = unkown_Pragma;
            targetArray = nullptr;
            targetLoop = nullptr;
            ScopeFunc = nullptr;
            targetStr = "";
            scopeStr = "";
            labelStr = "";
        }
        HI_PragmaInfo(const HI_PragmaInfo &input)
        {
            port_num = input.port_num;
            II = input.II;
            unroll_factor = input.unroll_factor;
            dim = input.dim;
            partition_factor = input.partition_factor;
            targetArray = input.targetArray;
            targetLoop = input.targetLoop;
            HI_PragmaInfoType = input.HI_PragmaInfoType;
            targetStr = input.targetStr;
            scopeStr = input.scopeStr;
            ScopeFunc = input.ScopeFunc;
            labelStr = input.labelStr;
            dataflowEnable = input.dataflowEnable;
            localArrayEnable = input.localArrayEnable;
            cyclic = input.cyclic;
            complete = input.complete;
        }
        HI_PragmaInfo &operator=(const HI_PragmaInfo &input)
        {
            port_num = input.port_num;
            II = input.II;
            unroll_factor = input.unroll_factor;
            dim = input.dim;
            partition_factor = input.partition_factor;
            targetArray = input.targetArray;
            targetLoop = input.targetLoop;
            HI_PragmaInfoType = input.HI_PragmaInfoType;
            targetStr = input.targetStr;
            scopeStr = input.scopeStr;
            ScopeFunc = input.ScopeFunc;
            labelStr = input.labelStr;
            dataflowEnable = input.dataflowEnable;
            localArrayEnable = input.localArrayEnable;
            cyclic = input.cyclic;
            complete = input.complete;
        }
    };

    friend bool operator==(HI_PragmaInfo lhs, HI_PragmaInfo rhs)
    {
        if (lhs.HI_PragmaInfoType != rhs.HI_PragmaInfoType)
            return false;
        if (lhs.II != rhs.II)
            return false;
        if (lhs.unroll_factor != rhs.unroll_factor)
            return false;
        if (lhs.partition_factor != rhs.partition_factor)
            return false;
        if (lhs.dim != rhs.dim)
            return false;
        if (lhs.scopeStr != rhs.scopeStr)
            return false;
        if (lhs.targetStr != rhs.targetStr)
            return false;
        if (lhs.port_num != rhs.port_num)
            return false;
        if (lhs.dataflowEnable != rhs.dataflowEnable)
            return false;
        if (lhs.localArrayEnable != rhs.localArrayEnable)
            return false;
        if (lhs.cyclic != rhs.cyclic)
            return false;
        if (lhs.complete != rhs.complete)
            return false;
        return true;
    }

    // Pass for simple evluation of the latency of the top function, without considering HLS
    // directives
    void Parse_Config_fromFile();
    void Parse_Config(const HI_DesignConfigInfo &configInfo);

    // load the timing and resource information for the instructions
    void Load_Instruction_Info();

    // parse the argument for array partitioning
    void parseArrayPartition(std::stringstream &iss);

    // parse the argument for array port number setting
    void parseArrayPortNum(std::stringstream &iss);

    // parse the argument for  function dataflow
    void parseFuncDataflow(std::stringstream &iss);

    // parse the argument for local array settting
    void parseLocalArray(std::stringstream &iss);

    // parse the argument for loop pipelining
    void parseLoopPipeline(std::stringstream &iss);

    // parse the argument for loop unrolling
    void parseLoopUnroll(std::stringstream &iss);

    void Parse_Config_toInfo(HI_DesignConfigInfo &desginconfig);

    // parse the argument for array partitioning
    void parseArrayPartition(std::stringstream &iss, HI_DesignConfigInfo &desginconfig);

    // parse the argument for array port number setting
    void parseArrayPortNum(std::stringstream &iss, HI_DesignConfigInfo &desginconfig);

    // parse the argument for function dataflow setting
    void parseFuncDataflow(std::stringstream &iss, HI_DesignConfigInfo &desginconfig);

    // parse the argument for local array settting
    void parseLocalArray(std::stringstream &iss, HI_DesignConfigInfo &desginconfig);

    // parse the argument for loop unrolling
    void parseLoopUnroll(std::stringstream &iss, HI_DesignConfigInfo &desginconfig);

    // parse the argument for loop pipelining
    void parseLoopPipeline(std::stringstream &iss, HI_DesignConfigInfo &desginconfig);

    // match the configuration and the corresponding declaration of memory (array)
    void matchArrayAndConfiguration(Value *target, HI_ArrayInfo &resArrayInfo);

    // find which function the value is located in
    Function *getFunctionOfValue(Value *target);

    /// Timer

    struct timeval tv_begin;
    struct timeval tv_end;
};

namespace std
{
bool operator<(const HI_WithDirectiveTimingResourceEvaluation::partition_info &A,
               const HI_WithDirectiveTimingResourceEvaluation::partition_info &B);
};

#endif
