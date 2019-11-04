#ifndef _DESIGNEVAL
#define _DESIGNEVAL

#include "LLVM_exp17_HI_HLSDirectiveDSE.h"


void modulePreProcessForInformation(
    llvm::Module &Mod,
    std::string top_str,
    std::map<std::string,Info_type_list> &BiOp_Info_name2list_map,
    std::map<std::string, std::vector<int>> &IRFunc2BeginLine,
    std::map<std::string, int> &FuncParamLine2OutermostSize,
    std::map<std::string, std::set<std::pair<std::string, std::pair<std::string, int>> >> &LoopLabel2DrivenArrayDimensions,
    std::map<std::string, std::string> &IRLoop2LoopLabel,
    std::map<std::string, int> &IRLoop2OriginTripCount,
    std::map<std::string, std::vector<std::string>> &LoopIRName2NextLevelSubLoopIRNames,
    std::map<std::string, int> &LoopIRName2Depth,
    std::map<std::string, std::vector< std::pair<std::string, std::string>>> &LoopIRName2Array,
    std::map<std::pair<std::string, std::string>, HI_PragmaArrayInfo> &TargetExtName2ArrayInfo,
    std::map<std::string, std::vector<int>> &Loop2PotentialUnrollFactors,
    bool debugFlag
);

void modulePreProcessForFPGAHLS_optimization(
    llvm::Module &Mod,
    std::string top_str,
    std::string cntStr,
    std::map<std::string,Info_type_list> &BiOp_Info_name2list_map,
    std::map<std::string, std::vector<int>> &IRFunc2BeginLine,
    std::map<std::string, int> &FuncParamLine2OutermostSize,
    std::map<std::string, std::set<std::pair<std::string, std::pair<std::string, int>> >> &LoopLabel2DrivenArrayDimensions,
    std::map<std::string, std::string> &IRLoop2LoopLabel,
    std::map<std::string, std::string> &IRLoop2LoopLabel_eval,
    std::map<std::string, int> &IRLoop2OriginTripCount,
    std::map<std::string, int> &IRLoop2OriginTripCount_eval,
    std::map<std::string, std::vector<std::string>> &LoopIRName2NextLevelSubLoopIRNames,
    std::map<std::string, int> &LoopIRName2Depth,
    std::map<std::string, std::vector< std::pair<std::string, std::string>>> &LoopIRName2Array,
    std::map<std::pair<std::string, std::string>, HI_PragmaArrayInfo> &TargetExtName2ArrayInfo,
    std::map<std::string, std::vector<int>> &Loop2PotentialUnrollFactors,
    std::map<std::string, int> &LoopLabel2UnrollFactor,
    bool debugFlag
);

void modulePreProcessForPreEvaluation(
    llvm::Module &Mod,
    std::string top_str,
    std::string cntStr,
    HI_DesignConfigInfo &desginconfig,
    std::map<std::string,Info_type_list> &BiOp_Info_name2list_map,
    std::map<std::string, std::vector<int>> &IRFunc2BeginLine,
    std::map<std::string, int> &FuncParamLine2OutermostSize,
    std::map<std::string, std::set<std::pair<std::string, std::pair<std::string, int>> >> &LoopLabel2DrivenArrayDimensions,
    std::map<std::string, std::string> &IRLoop2LoopLabel,
    std::map<std::string, std::string> &IRLoop2LoopLabel_eval,
    std::map<std::string, int> &IRLoop2OriginTripCount,
    std::map<std::string, int> &IRLoop2OriginTripCount_eval,
    std::map<std::string, std::vector<std::string>> &LoopIRName2NextLevelSubLoopIRNames,
    std::map<std::string, int> &LoopIRName2Depth,
    std::map<std::string, std::vector< std::pair<std::string, std::string>>> &LoopIRName2Array,
    std::map<std::pair<std::string, std::string>, HI_PragmaArrayInfo> &TargetExtName2ArrayInfo,
    std::map<std::string, std::vector<int>> &Loop2PotentialUnrollFactors,
    std::map<std::string, int> &LoopLabel2UnrollFactor,
    bool &muxWithMoreThan32,
    std::string &muxTooMuchLoopIRName,
    bool debugFlag
);

#endif