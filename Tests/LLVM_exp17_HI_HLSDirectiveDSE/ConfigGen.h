#ifndef _ConfigGen
#define _ConfigGen

#include "HI_print.h"
#include "HI_SysExec.h"
#include "HI_StringProcess.h"
#include <string>
#include <iostream>
#include <assert.h>
#include <map>
#include "LLVM_exp17_HI_HLSDirectiveDSE.h"

#define utilizationViolation(A) \
   ( A.DSP/DSP_limit > 1 ||   A.FF/FF_limit > 1 ||   A.LUT/LUT_limit > 1 ||   A.BRAM/BRAM_limit > 1   )


#define utilizationIndex(A) \
   ( A.DSP/DSP_limit + A.FF/FF_limit +  A.LUT/LUT_limit + A.BRAM/BRAM_limit   )


#define stringUtilizationIndex(A) \
   llvm::errs() <<  "DSP:" << A.DSP/DSP_limit <<   "  FF:" <<  A.FF/FF_limit <<  "  LUT:" <<  A.LUT/LUT_limit  << "  BRAM:" <<  A.BRAM/BRAM_limit  << "\n";


std::string findSlowestLastLevelLoop_InSlowestTopLoop(                    
                    std::map<std::string, std::string> &IRLoop2LoopLabel,
                    std::map<std::string, HI_WithDirectiveTimingResourceEvaluation::timingBase> &LoopLabel2Latency, 
                    std::map<std::string, std::vector<std::string>> &LoopIRName2NextLevelSubLoopIRNames,
                    std::map<std::string, int> &LoopIRName2Depth);

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

    // for (auto IRLoop2LoopLabel : IRLoop2LoopLabel)
    // {
    //     std::string irLoopLabel = IRLoop2LoopLabel.first;
    //     std::string loopLabel = IRLoop2LoopLabel.second;
    //     if (LoopIRName2Depth[irLoopLabel] == 1)
    //     {
    //         findTheSlowestLoop(irLoopLabel, LoopLabel2Latency, LoopIRName2NextLevelSubLoopIRNames, LoopIRName2Depth);
    //     }

#endif