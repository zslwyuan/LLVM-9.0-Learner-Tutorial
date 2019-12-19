#ifndef _HI_PragmaTargetExtraction
#define _HI_PragmaTargetExtraction

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

class HI_PragmaArrayInfo
{
  public:
    int dim_size[10];
    int sub_element_num[10];
    int partition_size[10];
    bool cyclic[10];
    int num_dims;
    bool isArgument = 0;
    llvm::Type *elementType;
    Value *target;
    std::string targetName;
    HI_PragmaArrayInfo()
    {
        targetName = "";
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
    HI_PragmaArrayInfo(const HI_PragmaArrayInfo &input)
    {
        targetName = input.targetName;
        elementType = input.elementType;
        num_dims = input.num_dims;
        target = input.target;
        isArgument = input.isArgument;
        for (int i = 0; i < num_dims; i++)
            dim_size[i] = input.dim_size[i];
        for (int i = 0; i < num_dims; i++)
            sub_element_num[i] = input.sub_element_num[i];
        for (int i = 0; i < num_dims; i++)
            partition_size[i] = input.partition_size[i];
        for (int i = 0; i < num_dims; i++)
            cyclic[i] = input.cyclic[i];
    }
    HI_PragmaArrayInfo &operator=(const HI_PragmaArrayInfo &input)
    {
        targetName = input.targetName;
        elementType = input.elementType;
        num_dims = input.num_dims;
        target = input.target;
        isArgument = input.isArgument;
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

raw_ostream &operator<<(raw_ostream &stream, const HI_PragmaArrayInfo &tb);

// Pass for simple evluation of the latency of the top function, without considering HLS directives
class HI_PragmaTargetExtraction : public ModulePass
{
  public:
    // Pass for simple evluation of the latency of the top function, without considering HLS
    // directives
    HI_PragmaTargetExtraction( // const char* config_file_name,
        const char *top_function, std::map<std::string, std::string> &IRLoop2LoopLabel,
        // std::map<std::string, int> &LoopLabel2II,
        std::map<std::string, int> &FuncParamLine2OutermostSize,
        std::map<std::string, std::vector<int>> &IRFunc2BeginLine, bool DEBUG = 0)
        : ModulePass(ID), IRLoop2LoopLabel(IRLoop2LoopLabel),
          FuncParamLine2OutermostSize(FuncParamLine2OutermostSize),
          IRFunc2BeginLine(IRFunc2BeginLine), DEBUG(DEBUG),
          LoopIRName2NextLevelSubLoopIRNames(this->_LoopIRName2NextLevelSubLoopIRNames),
          LoopIRName2Depth(this->_LoopIRName2Depth), LoopIRName2Array(this->_LoopIRName2Array),
          TargetExtName2ArrayInfo(this->_TargetExtName2ArrayInfo)

    {
        // config_file = new std::ifstream(config_file_name);
        loopTarget_log = new raw_fd_ostream("HI_PragmaTargetExtraction_loopTarget_log", ErrInfo,
                                            sys::fs::F_None);
        top_function_name = std::string(top_function);
        arrayTarget_Log = new raw_fd_ostream("HI_PragmaTargetExtraction_arrayTarget_Log", ErrInfo,
                                             sys::fs::F_None);
        // get the configureation from the file, e.g. clock period
        // Generate_Config();
    }

    // Pass for simple evluation of the latency of the top function, without considering HLS
    // directives
    HI_PragmaTargetExtraction( // const char* config_file_name,
        const char *top_function, std::map<std::string, std::string> &IRLoop2LoopLabel,
        // std::map<std::string, int> &LoopLabel2II,
        std::map<std::string, int> &FuncParamLine2OutermostSize,
        std::map<std::string, std::vector<int>> &IRFunc2BeginLine,
        std::map<std::string, std::vector<std::string>> &LoopIRName2NextLevelSubLoopIRNames,
        std::map<std::string, int> &LoopIRName2Depth,
        std::map<std::string, std::vector<std::pair<std::string, std::string>>> &LoopIRName2Array,
        std::map<std::pair<std::string, std::string>, HI_PragmaArrayInfo> &TargetExtName2ArrayInfo,
        bool DEBUG = 0)
        : ModulePass(ID), IRLoop2LoopLabel(IRLoop2LoopLabel),
          FuncParamLine2OutermostSize(FuncParamLine2OutermostSize),
          IRFunc2BeginLine(IRFunc2BeginLine), DEBUG(DEBUG),
          LoopIRName2NextLevelSubLoopIRNames(LoopIRName2NextLevelSubLoopIRNames),
          LoopIRName2Depth(LoopIRName2Depth), LoopIRName2Array(LoopIRName2Array),
          TargetExtName2ArrayInfo(TargetExtName2ArrayInfo)

    {
        // config_file = new std::ifstream(config_file_name);
        loopTarget_log = new raw_fd_ostream("HI_PragmaTargetExtraction_loopTarget_log", ErrInfo,
                                            sys::fs::F_None);
        top_function_name = std::string(top_function);
        arrayTarget_Log = new raw_fd_ostream("HI_PragmaTargetExtraction_arrayTarget_Log", ErrInfo,
                                             sys::fs::F_None);
        // get the configureation from the file, e.g. clock period
        // Generate_Config();
    }

    ~HI_PragmaTargetExtraction()
    {
        loopTarget_log->flush();
        delete loopTarget_log;
        arrayTarget_Log->flush();
        delete arrayTarget_Log;
    }

    virtual bool doInitialization(llvm::Module &M)
    {

        print_status("Initilizing HI_PragmaTargetExtraction pass.");
        return false;
    }

    bool DEBUG;

    class timingBase;
    class resourceBase;
    class HI_PragmaInfo;

    class HI_AccessInfo;

    // set the dependence of Passes
    void getAnalysisUsage(AnalysisUsage &AU) const;

    virtual bool runOnModule(llvm::Module &M);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////// Declaration related to the check of array and loops ////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    std::map<BasicBlock *, std::set<Value *>> Block2Targets;

    std::set<std::pair<Function *, Value *>> FuncTargetSet;

    std::map<std::string, std::vector<std::string>> &LoopIRName2NextLevelSubLoopIRNames;
    std::map<std::string, int> &LoopIRName2Depth;
    std::map<std::string, std::vector<std::pair<std::string, std::string>>> &LoopIRName2Array;
    std::map<std::pair<std::string, std::string>, HI_PragmaArrayInfo> &TargetExtName2ArrayInfo;

    std::map<BasicBlock *, std::vector<BasicBlock *>> Loop2NextLevelSubLoops;
    std::map<std::string, std::vector<std::string>> _LoopIRName2NextLevelSubLoopIRNames;
    std::map<std::string, int> _LoopIRName2Depth;
    std::map<std::string, std::vector<std::pair<std::string, std::string>>> _LoopIRName2Array;
    std::map<std::pair<std::string, std::string>, HI_PragmaArrayInfo> _TargetExtName2ArrayInfo;

    std::map<std::pair<std::string, std::string>, HI_PragmaArrayInfo> Array2Info;

    static char ID;

    int mux_cnt = 0;

    // record the information of the BRAMs and related accesses to BRAMs
    raw_ostream *loopTarget_log;

    // record the information of the BRAMs and related accesses to BRAMs
    raw_ostream *FF_log;

    std::error_code ErrInfo;
    std::ifstream *config_file;

    // the pass requires a specified top_function name
    std::string top_function_name;

    // Instruction related to target
    std::map<Value *, std::vector<Value *>> Instruction2Target;

    // some LLVM analysises could be involved
    ScalarEvolution *SE;
    LoopInfo *LI;

    bool topFunctionFound = 0;

    float clock_period = 10.0;

    int top_function_latency;

    std::string clock_period_str = "10.0";

    std::string HLS_lib_path = "";

    std::set<Value *> ValueVisited;

    // record which target arrays the instruction may access
    std::map<Instruction *, std::vector<Value *>> Access2TargetMap;

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

    /*
        extract loop in the function, organize them in hierary for directive generation
    */
    bool checkLoopsIn(Function *F);

    void checkSubLoops(Loop *L);

    void checkArrayAccessInLoop(Loop *L);

    class HI_AccessInfo
    {
      public:
        int dim_size[10];
        int sub_element_num[10];

        int partition_size[10];
        int index[10];
        int partition_id[10];
        bool cyclic[10];

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
            assert(input.num_dims > 0);
            elementType = input.elementType;
            num_dims = input.num_dims;
            target = input.target;
            isArgument = input.isArgument;
            partition = input.partition;
            initial_offset = input.initial_offset;
            unpredictable = input.unpredictable;
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
        HI_AccessInfo(const HI_PragmaArrayInfo &input)
        {
            elementType = input.elementType;
            num_dims = input.num_dims;
            target = input.target;
            isArgument = input.isArgument;
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
            assert(input.num_dims > 0);
            elementType = input.elementType;
            num_dims = input.num_dims;
            target = input.target;
            isArgument = input.isArgument;
            partition = input.partition;
            initial_offset = input.initial_offset;
            unpredictable = input.unpredictable;
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
        HI_AccessInfo &operator=(const HI_PragmaArrayInfo &input)
        {
            elementType = input.elementType;
            num_dims = input.num_dims;
            target = input.target;
            isArgument = input.isArgument;
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
    };

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

    raw_ostream *arrayTarget_Log;

    // record the array informtiion
    std::map<Value *, HI_PragmaArrayInfo> Target2ArrayInfo;

    // according to the address instruction (ptr+offset), get the access information
    std::map<Value *, HI_AccessInfo> AddressInst2AccessInfo;

    // get the array information, including the dimension, type and size
    HI_PragmaArrayInfo getArrayInfo(Value *target);

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
    std::map<std::string, std::string> IRLoop2LoopLabel;
    std::map<std::string, int> LoopLabel2II;
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
            unkown_Pragma
        };
        pragmaType HI_PragmaInfoType = unkown_Pragma;
        std::string targetStr, scopeStr, labelStr;
        int II, dim, unroll_factor, partition_factor;
        bool cyclic = 1;
        Value *targetArray;
        BasicBlock *targetLoop;
        Function *ScopeFunc;

        HI_PragmaInfo()
        {
            II = -1;
            unroll_factor = -1;
            partition_factor = -1;
            dim = -1;
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
        }
        HI_PragmaInfo &operator=(const HI_PragmaInfo &input)
        {
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
        return true;
    }

    // Pass for simple evluation of the latency of the top function, without considering HLS
    // directives
    void Generate_Config();

    // parse the argument for array partitioning
    void generateArrayPartition(std::stringstream &iss);

    // parse the argument for loop pipelining
    void generateLoopPipeline(std::stringstream &iss);

    // find which function the value is located in
    Function *getFunctionOfValue(Value *target);

    /// Timer

    struct timeval tv_begin;
    struct timeval tv_end;
};

#endif
