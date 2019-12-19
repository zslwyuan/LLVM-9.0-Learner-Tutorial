#ifndef _HI_MuxInsertionArrayPartition
#define _HI_MuxInsertionArrayPartition

#include "ClockInfo.h"
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

#endif

// Pass for simple evluation of the latency of the top function, without considering HLS directives
class HI_MuxInsertionArrayPartition : public ModulePass
{
  public:
    // Pass for simple evluation of the latency of the top function, without considering HLS
    // directives
    HI_MuxInsertionArrayPartition(const char *config_file_name, const char *top_function,
                                  std::map<std::string, int> &FuncParamLine2OutermostSize,
                                  std::map<std::string, std::vector<int>> &IRFunc2BeginLine,
                                  bool DEBUG = 0)
        : ModulePass(ID), FuncParamLine2OutermostSize(FuncParamLine2OutermostSize),
          IRFunc2BeginLine(IRFunc2BeginLine), DEBUG(DEBUG)
    {
        config_file = new std::ifstream(config_file_name);
        BRAM_log =
            new raw_fd_ostream("HI_MuxInsertionArrayPartition_BRAM_log", ErrInfo, sys::fs::F_None);
        top_function_name = std::string(top_function);
        ArrayLog =
            new raw_fd_ostream("HI_MuxInsertionArrayPartition_Array_Log", ErrInfo, sys::fs::F_None);
        // get the configureation from the file, e.g. clock period
        Parse_Config();
    }

    HI_MuxInsertionArrayPartition(const HI_DesignConfigInfo &configInfo, const char *top_function,
                                  std::map<std::string, int> &FuncParamLine2OutermostSize,
                                  std::map<std::string, std::vector<int>> &IRFunc2BeginLine,
                                  bool DEBUG = 0)
        : ModulePass(ID), FuncParamLine2OutermostSize(FuncParamLine2OutermostSize),
          IRFunc2BeginLine(IRFunc2BeginLine), DEBUG(DEBUG)
    {
        //  config_file = new std::ifstream(config_file_name);
        BRAM_log =
            new raw_fd_ostream("HI_MuxInsertionArrayPartition_BRAM_log", ErrInfo, sys::fs::F_None);
        top_function_name = std::string(top_function);
        ArrayLog =
            new raw_fd_ostream("HI_MuxInsertionArrayPartition_Array_Log", ErrInfo, sys::fs::F_None);
        // get the configureation from the file, e.g. clock period
        Parse_Config(configInfo);
    }

    ~HI_MuxInsertionArrayPartition()
    {
        BRAM_log->flush();
        delete BRAM_log;
        ArrayLog->flush();
        delete ArrayLog;
    }

    virtual bool doInitialization(llvm::Module &M)
    {

        print_status("Initilizing HI_MuxInsertionArrayPartition pass.");
        return false;
    }

    class timingBase;
    class resourceBase;
    class HI_PragmaInfo;
    class partition_info;

    // set the dependence of Passes
    void getAnalysisUsage(AnalysisUsage &AU) const;

    virtual bool runOnModule(llvm::Module &M);

    void Parse_Config(const HI_DesignConfigInfo &configInfo);

    bool DEBUG = 0;

    bool muxWithMoreThan32 = 0;

    std::string muxTooMuchLoopIRName = "";

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////// Declaration related to Mux Insertion ////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    bool MuxInsert(BasicBlock *B);

    std::set<Instruction *> muxed_load;

    static char ID;

    int mux_cnt = 0;

    // record the information of the BRAMs and related accesses to BRAMs
    raw_ostream *BRAM_log;

    // record the information of the BRAMs and related accesses to BRAMs
    raw_ostream *FF_log;

    std::error_code ErrInfo;
    std::ifstream *config_file;

    // the pass requires a specified top_function name
    std::string top_function_name;

    // Instruction related to target
    std::map<Value *, std::vector<Value *>> Instruction2Target;

    std::map<std::pair<Value *, partition_info>, std::vector<std::pair<BasicBlock *, int>>>
        targetPartition2BlockCycleAccessCnt;

    std::set<std::pair<Value *, partition_info>> accessPartitionsForIITest;

    void HI_takeOverAllUsesWith(Value *Old, Value *New);

    // some LLVM analysises could be involved
    ScalarEvolution *SE;
    LoopInfo *LI;

    bool topFunctionFound = 0;

    float clock_period = 10.0;

    int top_function_latency;

    std::string clock_period_str = "10.0";

    std::string HLS_lib_path = "";

    //////////////////////////////////////////////////////////////////////////////////////
    //////////////////// Declaration related to Memory Access Tracing ////////////////////
    //////////////////////////////////////////////////////////////////////////////////////

    std::set<Value *> ValueVisited;

    // record which target arrays the instruction may access
    std::map<Instruction *, std::vector<Value *>> Access2TargetMap;

    // record that in the basic block, which instruction access which array at which cycle
    std::map<Value *,
             std::map<BasicBlock *,
                      std::vector<std::pair<std::pair<int, partition_info>, Instruction *>>>>
        target2LastAccessCycleInBlock;
    //  the first int is for time slot
    //  and the second int is for partition

    // register number for target array
    std::vector<Instruction *> AccessesList;

    // alias of array from functions to subfunctions
    std::map<Value *, Value *> Alias2Target;

    std::map<BasicBlock *, std::vector<Instruction *>> Block2AccessList;

    std::map<std::pair<Instruction *, Instruction *>, int> InstInst2DependenceDistance;

    // Trace Memory Declaration in Module
    // analyze BRAM accesses in the module before any other analysis
    bool TraceMemoryDeclarationAndAnalyzeAccessinModule(llvm::Module &M);

    // find the array declaration in the function F and trace the accesses to them
    void findMemoryDeclarationAndAnalyzeAccessin(Function *F, bool isTopFunction);

    // find out which instrctuins are related to the array, going through PtrToInt, Add, IntToPtr,
    // Store, Load instructions record the corresponding target array which the access instructions
    // try to touch
    void TraceAccessForTarget(Value *cur_node, Value *ori_node);

    // handle the information of memory access for the function
    // get the offset of the access to array and get the access information
    // in the function
    bool ArrayAccessCheckForFunction(Function *F);

    class HI_ArrayInfo;
    class HI_AccessInfo;

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

    const SCEV *bypassExtTruntSCEV(const SCEV *inputS);

    const SCEV *findUnknown(const SCEV *ori_inputS);

    // get the unknown values in the expression
    int getUnknownNum(const SCEV *ori_inputS);

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

    // get the total number of partitions of the target array
    int getTotalPartitionNum(HI_ArrayInfo &refInfo);

    // get the targer partition according to the specific memory access instruction
    std::vector<partition_info> getPartitionFor(Instruction *access);

    // try to add the partiton into the record list, if there is no duplicated partition
    bool tryRecordPartition(std::vector<partition_info> &partitions, partition_info try_target);

    // get the target array from the load/store instruction
    Value *getTargetFromInst(Instruction *accessI);

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

        partition_info()
        {
            num_dims = 10;
            for (int i = 0; i < num_dims; i++)
                partition_id[i] = 0;
            num_dims = -1;
            target = nullptr;
        }

        partition_info operator=(const partition_info &input)
        {
            assert(input.target);
            target = input.target;
            num_dims = input.num_dims;
            for (int i = 0; i < num_dims; i++)
                partition_id[i] = input.partition_id[i];
        }

        partition_info(const partition_info &input)
        {
            assert(input.target);
            target = input.target;
            num_dims = input.num_dims;
            for (int i = 0; i < num_dims; i++)
                partition_id[i] = input.partition_id[i];
        }
    };

    friend bool operator==(const partition_info &lhs, const partition_info &rhs)
    {
        assert(lhs.num_dims == rhs.num_dims);
        if (lhs.target != rhs.target)
        {
            return false;
        }
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
        stream << tb.partition_id[0] << "]   ";
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
        stream << "] ";
        // timing="<<tb.timing<<"] ";
        return stream;
    }

    friend raw_ostream &operator<<(raw_ostream &stream, const HI_AccessInfo &tb)
    {
        stream << "HI_AccessInfo for: <<" << *tb.target << ">> [ele_Type= " << *tb.elementType
               << ", num_dims=" << tb.num_dims << ", "; // ", partition=" << tb.partition << ", ";
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
        // timing="<<tb.timing<<"] ";
        return stream;
    }

    std::set<Value *> ArrayValueVisited;

    std::set<Instruction *> Inst_AccessRelated;

    raw_ostream *ArrayLog;

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

    /////////////////////////////////////////////////////////////////////////////////////////////
    ////////////////////// Parse the configuration for the program //////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////

    std::map<Value *, std::vector<HI_PragmaInfo>> arrayDirectives;
    std::map<BasicBlock *, HI_PragmaInfo> LoopDirectives;
    std::vector<HI_PragmaInfo> PragmaInfo_List;
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
    void Parse_Config();

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
bool operator<(const HI_MuxInsertionArrayPartition::partition_info A,
               const HI_MuxInsertionArrayPartition::partition_info B);
};

#endif
