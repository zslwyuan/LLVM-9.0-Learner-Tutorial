#ifndef _ConfigGen
#define _ConfigGen

#include "HI_print.h"
#include "HI_SysExec.h"
#include "HI_StringProcess.h"
#include <string>
#include <iostream>
#include <assert.h>
#include <map>
#include "LLVM_exp18_HI_HLSDirectiveDSE_clock.h"

#define accumulateResource(A,B) \
    A.DSP += B.DSP;         \
    A.FF += B.FF;           \
    A.LUT += B.LUT;         \
    A.BRAM += B.BRAM;

#define deductResource(A,B) \
    A.DSP -= B.DSP;         \
    A.FF -= B.FF;           \
    A.LUT -= B.LUT;         \
    A.BRAM -= B.BRAM;

#define resetResource(A) \
    A.DSP = 0;         \
    A.FF = 0;           \
    A.LUT = 0;         \
    A.BRAM = 0;

#define utilizationViolation(A) \
   ( A.DSP/DSP_limit > 1 ||   A.FF/FF_limit > 1 ||   A.LUT/LUT_limit > 1 ||   A.BRAM/BRAM_limit > 1   )


#define utilizationIndex(A) \
   ( A.DSP/DSP_limit + A.FF/FF_limit +  A.LUT/LUT_limit + A.BRAM/BRAM_limit   )


#define stringUtilizationIndex(A) \
   pushing_relaxtion_log <<  "DSP:" << A.DSP/DSP_limit <<   "  FF:" <<  A.FF/FF_limit <<  "  LUT:" <<  A.LUT/LUT_limit  << "  BRAM:" <<  A.BRAM/BRAM_limit << " current TotalAreaRate:" << A.DSP/DSP_limit+A.FF/FF_limit+A.LUT/LUT_limit+A.BRAM/BRAM_limit << "\n";


#define stringUtilization(A) \
   pushing_relaxtion_log <<  "DSP:" << A.DSP <<   "  FF:" <<  A.FF <<  "  LUT:" <<  A.LUT  << "  BRAM:" <<  A.BRAM  << "\n";
   
#define stringUtilizationIndexTo(A,out) \
   out <<  "DSP:" << A.DSP/DSP_limit <<   "  FF:" <<  A.FF/FF_limit <<  "  LUT:" <<  A.LUT/LUT_limit  << "  BRAM:" <<  A.BRAM/BRAM_limit  << "\n";


#define stringUtilizationTo(A,out) \
   out <<  "DSP:" << A.DSP <<   "  FF:" <<  A.FF <<  "  LUT:" <<  A.LUT  << "  BRAM:" <<  A.BRAM  << "\n";


class designEvalResult;

std::string findSlowestLastLevelLoop_InSlowestTopLoop(                    
                    std::map<std::string, std::string> &IRLoop2LoopLabel,
                    std::map<std::string, HI_WithDirectiveTimingResourceEvaluation::timingBase> &LoopLabel2Latency, 
                    std::map<std::string, std::vector<std::string>> &LoopIRName2NextLevelSubLoopIRNames,
                    std::map<std::string, int> &LoopIRName2Depth);

std::string findSlowestLastLevelLoop_InSlowestTopLoop( 
                    std::string funcScope,     
                    std::map<std::string, std::string> &TopLoopIR2FuncName,              
                    std::map<std::string, std::string> &IRLoop2LoopLabel,
                    std::map<std::string, HI_WithDirectiveTimingResourceEvaluation::timingBase> &LoopLabel2Latency, 
                    std::map<std::string, std::vector<std::string>> &LoopIRName2NextLevelSubLoopIRNames,
                    std::map<std::string, int> &LoopIRName2Depth
                    );

std::string findTheSlowestLoop(
                    std::string cur_LoopIRName,
                    std::map<std::string, std::string> &IRLoop2LoopLabel,
                    std::map<std::string, HI_WithDirectiveTimingResourceEvaluation::timingBase> &LoopLabel2Latency, 
                    std::map<std::string, std::vector<std::string>> &LoopIRName2NextLevelSubLoopIRNames,
                    std::map<std::string, int> &LoopIRName2Depth
                    );
                    
std::map<std::string, std::vector<int>> getPotentialLoopUnrollFactor(
                                            std::map<std::string, std::string> &IRLoop2LoopLabel,
                                            std::map<std::string, std::vector<std::string>> &LoopIRName2NextLevelSubLoopIRNames,
                                            std::map<std::string, int> &LoopIRName2Depth,
                                            std::map<std::string, int> &IRLoop2OriginTripCount
                                        );
                                        

int findNextUnrollFactor(   std::string IRLoopName,
                            int cur_loopUnrollFactor,
                            std::map<std::string, std::vector<int>> &loop2PotentialUnrollFactors
                         );

std::string  getSlowestModuleInDataflow(std::map<std::string, HI_WithDirectiveTimingResourceEvaluation::timingBase> FuncName2Latency, 
                                        std::map<std::string, HI_WithDirectiveTimingResourceEvaluation::resourceBase> FuncName2Resource,
                                        std::string topFunction);

int getClockId(double clock_period, std::string clockStrs[100]);

std::vector<std::string> findSubLoopIRs(
                        std::string cur_LoopIRName,
                        std::map<std::string, std::string> &IRLoop2LoopLabel,
                        std::map<std::string, std::vector<std::string>> &LoopIRName2NextLevelSubLoopIRNames
                    );

bool singleClockDesignEvaluation(
                      HI_DesignConfigInfo desginconfig_0,
                      llvm::raw_fd_ostream &checkedConfigs, 
                      llvm::Module &Mod, 
                      designEvalResult &design_eval_result,
                      int &output_next_factor,
                      std::string &output_next_loopLabel,

                      std::map<std::string, std::string> &IRLoop2LoopLabel, 
                      std::map<std::string, std::vector<int>> &IRFunc2BeginLine,
                      std::map<std::string, int> &FuncParamLine2OutermostSize,
                      std::map<std::string, std::vector<int>> &Loop2PotentialUnrollFactors,
                      std::map<std::string, std::set<std::pair<std::string, std::pair<std::string, int>> >> &LoopLabel2DrivenArrayDimensions,
                      std::map<std::string, int> &IRLoop2OriginTripCount,
                      std::map<std::string, std::string> &TopLoopIR2FuncName,
                      std::map<std::string, std::vector<std::string>> &LoopIRName2NextLevelSubLoopIRNames,
                      std::map<std::string, int> &LoopIRName2Depth,
                      std::map<std::string, std::vector< std::pair<std::string, std::string>>> &LoopIRName2Array,
                      std::map<std::string,Info_type_list> &BiOp_Info_name2list_map,
                      int config_id,
                      std::string top_str,
                      bool debugFlag
                     );

bool multipleClockDesignEvaluation(
                      HI_DesignConfigInfo desginconfig_last,
                      llvm::raw_fd_ostream &checkedConfigs, 
                      llvm::Module &Mod, 

                      designEvalResult &entire_design_eval_result,
                      std::string &slowestFunctioName,
                      double &max_Latency,
                      double &second_max_Latency,
                      double &clock_forSlowestFunc,
                      HI_WithDirectiveTimingResourceEvaluation::resourceBase &ori_total_resource,
                      std::map<std::string, HI_WithDirectiveTimingResourceEvaluation::timingBase> &MultiClk_FuncName2Latency,
                      std::map<std::string, HI_WithDirectiveTimingResourceEvaluation::resourceBase> &MultiClk_FuncName2Resource,
                      int &output_next_factor,
                      std::string &output_next_loopLabel,

                      std::map<std::string, std::string> &clockForFunction,
                      std::vector<std::string> &FuncNames,
                      std::map<std::string, std::string> &IRLoop2LoopLabel, 
                      std::map<std::string, std::vector<int>> &IRFunc2BeginLine,
                      std::map<std::string, int> &FuncParamLine2OutermostSize,
                      std::map<std::string, std::vector<int>> &Loop2PotentialUnrollFactors,
                      std::map<std::string, std::set<std::pair<std::string, std::pair<std::string, int>> >> &LoopLabel2DrivenArrayDimensions,
                      std::map<std::string, int> &IRLoop2OriginTripCount,
                      std::map<std::string, std::string> &TopLoopIR2FuncName,
                      std::map<std::string, std::vector<std::string>> &LoopIRName2NextLevelSubLoopIRNames,
                      std::map<std::string, int> &LoopIRName2Depth,
                      std::map<std::string, std::vector< std::pair<std::string, std::string>>> &LoopIRName2Array,
                      std::map<std::string,Info_type_list> &BiOp_Info_name2list_map,


                      int config_id,
                      std::string top_str,
                      bool debugFlag
                     );

bool tryUpdateSlowestFuncClock(
                      HI_DesignConfigInfo desginconfig_last,
                      llvm::raw_fd_ostream &checkedConfigs, 
                      llvm::Module &Mod, 

                      std::string slowestFunctioName,
                      double max_Latency,
                      double second_max_Latency,
                      double clock_forSlowestFunc,
                      HI_WithDirectiveTimingResourceEvaluation::resourceBase ori_total_resource,
                      HI_WithDirectiveTimingResourceEvaluation::resourceBase &newclk_total_resource,
                      std::map<std::string, HI_WithDirectiveTimingResourceEvaluation::timingBase> &MultiClk_FuncName2Latency,
                      std::map<std::string, HI_WithDirectiveTimingResourceEvaluation::resourceBase> &MultiClk_FuncName2Resource,
                      std::string &optClock,
                      HI_DesignConfigInfo & desginconfig_optClock,

                      std::map<std::string, std::string> clockForFunction,
                      std::vector<std::string> &FuncNames,
                      std::map<std::string, std::string> &IRLoop2LoopLabel, 
                      std::map<std::string, std::vector<int>> &IRFunc2BeginLine,
                      std::map<std::string, int> &FuncParamLine2OutermostSize,
                      std::map<std::string, std::vector<int>> &Loop2PotentialUnrollFactors,
                      std::map<std::string, std::set<std::pair<std::string, std::pair<std::string, int>> >> &LoopLabel2DrivenArrayDimensions,
                      std::map<std::string, int> &IRLoop2OriginTripCount,
                      std::map<std::string, std::string> &TopLoopIR2FuncName,
                      std::map<std::string, std::vector<std::string>> &LoopIRName2NextLevelSubLoopIRNames,
                      std::map<std::string, int> &LoopIRName2Depth,
                      std::map<std::string, std::vector< std::pair<std::string, std::string>>> &LoopIRName2Array,
                      std::map<std::string,Info_type_list> &BiOp_Info_name2list_map,


                      int config_id,
                      std::string top_str,
                      bool debugFlag
                     );

bool tryUpdateHLSDirectives(
                      HI_DesignConfigInfo desginconfig_last,
                      llvm::raw_fd_ostream &checkedConfigs, 
                      llvm::Module &Mod, 

                      std::string ori_slowestFunctioName,
                      double ori_max_Latency,
                      double ori_second_max_Latency,
                      HI_WithDirectiveTimingResourceEvaluation::resourceBase ori_total_resource,
                      HI_WithDirectiveTimingResourceEvaluation::resourceBase &newloop_total_resource,
                      HI_DesignConfigInfo &desginconfig_optLoop,
                      double &new_max_Latency,

                      std::map<std::string, std::string> &clockForFunction,
                      std::vector<std::string> &FuncNames,
                      std::map<std::string, std::string> &IRLoop2LoopLabel, 
                      std::map<std::string, std::vector<int>> &IRFunc2BeginLine,
                      std::map<std::string, int> &FuncParamLine2OutermostSize,
                      std::map<std::string, std::vector<int>> &Loop2PotentialUnrollFactors,
                      std::map<std::string, std::set<std::pair<std::string, std::pair<std::string, int>> >> &LoopLabel2DrivenArrayDimensions,
                      std::map<std::string, int> &IRLoop2OriginTripCount,
                      std::map<std::string, std::string> &TopLoopIR2FuncName,
                      std::map<std::string, std::vector<std::string>> &LoopIRName2NextLevelSubLoopIRNames,
                      std::map<std::string, int> &LoopIRName2Depth,
                      std::map<std::string, std::vector< std::pair<std::string, std::string>>> &LoopIRName2Array,
                      std::map<std::string,Info_type_list> &BiOp_Info_name2list_map,


                      int config_id,
                      std::string top_str,
                      bool debugFlag
                     );


bool justTryUpdateSlowestFuncClock_withoutHLSDSE(
                      HI_DesignConfigInfo desginconfig_last,
                      llvm::raw_fd_ostream &checkedConfigs, 
                      llvm::Module &Mod, 

                      std::string slowestFunctioName,
                      double ori_max_Latency,
                      double ori_second_max_Latency,
                      double clock_forSlowestFunc,
                      HI_WithDirectiveTimingResourceEvaluation::resourceBase ori_total_resource,
                      HI_WithDirectiveTimingResourceEvaluation::resourceBase &newclk_total_resource,
                      std::map<std::string, HI_WithDirectiveTimingResourceEvaluation::timingBase> &MultiClk_FuncName2Latency,
                      std::map<std::string, HI_WithDirectiveTimingResourceEvaluation::resourceBase> &MultiClk_FuncName2Resource,
                      HI_DesignConfigInfo & desginconfig_optClock,

                      std::map<std::string, std::string> &clockForFunction,
                      std::vector<std::string> &FuncNames,
                      std::map<std::string, std::string> &IRLoop2LoopLabel, 
                      std::map<std::string, std::vector<int>> &IRFunc2BeginLine,
                      std::map<std::string, int> &FuncParamLine2OutermostSize,
                      std::map<std::string, std::vector<int>> &Loop2PotentialUnrollFactors,
                      std::map<std::string, std::set<std::pair<std::string, std::pair<std::string, int>> >> &LoopLabel2DrivenArrayDimensions,
                      std::map<std::string, int> &IRLoop2OriginTripCount,
                      std::map<std::string, std::string> &TopLoopIR2FuncName,
                      std::map<std::string, std::vector<std::string>> &LoopIRName2NextLevelSubLoopIRNames,
                      std::map<std::string, int> &LoopIRName2Depth,
                      std::map<std::string, std::vector< std::pair<std::string, std::string>>> &LoopIRName2Array,
                      std::map<std::string,Info_type_list> &BiOp_Info_name2list_map,


                      int config_id,
                      std::string top_str,
                      bool debugFlag
                     );

bool findNextPossibleClockCombination
    (
        std::string slowestFunctioName,
        std::map<std::string, std::string> &new_clockForFunction,
        std::map<std::string, HI_WithDirectiveTimingResourceEvaluation::timingBase> &MultiClk_FuncName2Latency,
        llvm::raw_fd_ostream &checkedConfigs
    );

std::string printClocks(std::map<std::string, std::string> &new_clockForFunction);

class designEvalResult
{
    public:
        designEvalResult()
        {
            success = 0;
        }

        // designEvalResult(
        //     std::map<std::string, HI_WithDirectiveTimingResourceEvaluation::timingBase> LoopLabel2Latency,
        //     std::map<std::string, HI_WithDirectiveTimingResourceEvaluation::timingBase> FuncName2Latency,
        //     std::map<std::string, HI_WithDirectiveTimingResourceEvaluation::resourceBase> FuncName2Resource,
        //     std::map<std::string, HI_WithDirectiveTimingResourceEvaluation::resourceBase> LoopLabel2Resource,
        //     std::map<std::string, int> LoopLabel2SmallestII,
        //     std::map<std::string, int> LoopLabel2IterationLatency,
        //     std::map<std::string, int> LoopLabel2AchievedII

        // ) : LoopLabel2Latency(LoopLabel2Latency), FuncName2Latency(FuncName2Latency), FuncName2Resource(FuncName2Resource),
        //     LoopLabel2Resource(LoopLabel2Resource), LoopLabel2SmallestII(LoopLabel2SmallestII), LoopLabel2IterationLatency(LoopLabel2IterationLatency),
        //     LoopLabel2AchievedII(LoopLabel2AchievedII)
        // {
        //     success = 1;
        // }

        designEvalResult(const designEvalResult &input)
        {
            LoopLabel2Latency = input.LoopLabel2Latency;
            FuncName2Latency = input.FuncName2Latency;
            FuncName2Resource = input.FuncName2Resource;
            LoopLabel2Resource = input.LoopLabel2Resource;
            LoopLabel2SmallestII = input.LoopLabel2SmallestII;
            LoopLabel2IterationLatency = input.LoopLabel2IterationLatency;
            LoopLabel2AchievedII = input.LoopLabel2AchievedII;
            FuncArray2PartitionBenefit = input.FuncArray2PartitionBenefit;
            success = input.success;
        }


        designEvalResult& operator=(const designEvalResult &input)
        {
            LoopLabel2Latency = input.LoopLabel2Latency;
            FuncName2Latency = input.FuncName2Latency;
            FuncName2Resource = input.FuncName2Resource;
            LoopLabel2Resource = input.LoopLabel2Resource;
            LoopLabel2SmallestII = input.LoopLabel2SmallestII;
            LoopLabel2IterationLatency = input.LoopLabel2IterationLatency;
            LoopLabel2AchievedII = input.LoopLabel2AchievedII;
            FuncArray2PartitionBenefit = input.FuncArray2PartitionBenefit;
            success = input.success;
        }

        std::map<std::string, HI_WithDirectiveTimingResourceEvaluation::timingBase> LoopLabel2Latency;
        std::map<std::string, HI_WithDirectiveTimingResourceEvaluation::timingBase> FuncName2Latency;
        std::map<std::string, HI_WithDirectiveTimingResourceEvaluation::resourceBase> FuncName2Resource;
        
        std::map<std::string, HI_WithDirectiveTimingResourceEvaluation::resourceBase> LoopLabel2Resource;

        std::map<std::pair<std::string, std::string>, bool> FuncArray2PartitionBenefit;
        std::map<std::string, int> LoopLabel2SmallestII;
        std::map<std::string, int> LoopLabel2IterationLatency;
        std::map<std::string, int> LoopLabel2AchievedII;
        bool success = 0;

        friend class raw_ostream;
};

    // for (auto IRLoop2LoopLabel : IRLoop2LoopLabel)
    // {
    //     std::string irLoopLabel = IRLoop2LoopLabel.first;
    //     std::string loopLabel = IRLoop2LoopLabel.second;
    //     if (LoopIRName2Depth[irLoopLabel] == 1)
    //     {
    //         findTheSlowestLoop(irLoopLabel, LoopLabel2Latency, LoopIRName2NextLevelSubLoopIRNames, LoopIRName2Depth);
    //     }


#ifndef _arrayPragmaGenerator
#define _arrayPragmaGenerator

class arrayPragmaGenerator
{
    public:
        arrayPragmaGenerator(HI_PragmaArrayInfo ref_arrayInfo) : ref_arrayInfo(ref_arrayInfo)
        {
            int numDim = ref_arrayInfo.num_dims;
            isForArray = 1;
            generateArrayConfigurations(0,ref_arrayInfo);
        }


        bool forArray()
        {
            return isForArray;
        }

        const std::vector<HI_PragmaArrayInfo> &getArrayConfigurations()
        {
            return arrayCconfigurations;
        }


    private:
        HI_PragmaArrayInfo ref_arrayInfo;
        std::vector<HI_PragmaArrayInfo> arrayCconfigurations;
        bool isForArray = 0;
        bool isForLoop = 0;


        void generateArrayConfigurations(int curDim, HI_PragmaArrayInfo ref_arrayInfo)
        {
            if (curDim >= ref_arrayInfo.num_dims)
            {
                arrayCconfigurations.push_back(ref_arrayInfo);
                return;
            }
            std::set<int> possiblePartitionFactor;
            for (int i=1;i<=ref_arrayInfo.dim_size[curDim] && i<=64;i*=2)
                possiblePartitionFactor.insert(i);
            for (int i=1; i<=ref_arrayInfo.dim_size[curDim]; i++)
            {
                if (!(ref_arrayInfo.dim_size[curDim]%i))
                {
                    possiblePartitionFactor.insert(i);
                }
            }

            for (auto partition_factor : possiblePartitionFactor)
            {
                ref_arrayInfo.partition_size[curDim] = partition_factor;
                ref_arrayInfo.cyclic[curDim] = 1;
                generateArrayConfigurations(curDim+1, ref_arrayInfo);
                ref_arrayInfo.cyclic[curDim] = 0;
                generateArrayConfigurations(curDim+1, ref_arrayInfo);
            }
        }

};

#endif


#ifndef _enumerateDesignConfiguration
#define _enumerateDesignConfiguration


class enumerateDesignConfiguration
{
    public:
        enumerateDesignConfiguration( const char *genFile_name,
                                        std::map<std::string, std::string> &IRLoop2LoopLabel, 
                                        std::map<std::string, int> &IRLoop2OriginTripCount,
                                        std::map<std::string, std::vector<std::string>> &LoopIRName2NextLevelSubLoopIRNames,
                                        std::map<std::string, int> &LoopIRName2Depth,
                                        std::map<std::string, std::vector< std::pair<std::string, std::string>>> &LoopIRName2Array,
                                        std::map<std::pair<std::string, std::string>, HI_PragmaArrayInfo> &TargetExtName2ArrayInfo);
        

        class loopSetting
        {
            public:
                loopSetting(std::string loopName, bool hasPragma, bool isUnroll, int Val) : 
                    loopName(loopName),
                    hasPragma(hasPragma), isUnroll(isUnroll), Val(Val)
                {

                }

                loopSetting(const loopSetting &input)
                {
                    hasPragma = input.hasPragma;
                    isUnroll = input.isUnroll;
                    Val = input.Val;
                    loopName = input.loopName;
                }

                loopSetting& operator=(const loopSetting &input)
                {
                    hasPragma = input.hasPragma;
                    isUnroll = input.isUnroll;
                    Val = input.Val;
                    loopName = input.loopName;
                }

                bool hasPragma, isUnroll;
                int Val;
                std::string loopName;
        };

        class arraySetting
        {
            public:
                arraySetting(std::string arrayName, bool hasPragma, bool isCyclic, int Val) : 
                    arrayName(arrayName),
                    hasPragma(hasPragma), isCyclic(isCyclic), Val(Val)
                {

                }

                arraySetting(const arraySetting &input)
                {
                    hasPragma = input.hasPragma;
                    isCyclic = input.isCyclic;
                    Val = input.Val;
                    arrayName = input.arrayName;
                }

                arraySetting& operator=(const arraySetting &input)
                {
                    hasPragma = input.hasPragma;
                    isCyclic = input.isCyclic;
                    Val = input.Val;
                    arrayName = input.arrayName;
                }

                bool hasPragma, isCyclic;
                int Val;
                std::string arrayName;
        };

        // std::vector<loopSetting> generateLoopConfigurationBFS(std::string LoopName, );
  
    private:
                                        std::map<std::string, std::string> &IRLoop2LoopLabel; 
                                        std::map<std::string, int> &IRLoop2OriginTripCount;
                                        std::map<std::string, std::vector<std::string>> &LoopIRName2NextLevelSubLoopIRNames;
                                        std::map<std::string, int> &LoopIRName2Depth;
                                        std::map<std::string, std::vector< std::pair<std::string, std::string>>> &LoopIRName2Array;
                                        std::map<std::pair<std::string, std::string>, HI_PragmaArrayInfo> &TargetExtName2ArrayInfo;
};

#endif


raw_ostream& operator<< (raw_ostream& stream, designEvalResult& tb);
void generateClockFunctionMaps(std::vector<int> &clockIds, std::vector<std::string> &funcNames, std::vector<std::map<std::string, std::string>> &clockFunctionMaps);
#endif