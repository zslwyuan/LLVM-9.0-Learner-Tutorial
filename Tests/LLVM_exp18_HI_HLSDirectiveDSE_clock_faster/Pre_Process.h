#ifndef _Pre_Process
#define _Pre_Process

#include "clang/AST/Type.h"
#include "clang/Driver/Options.h"
#include "clang/AST/AST.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/ASTConsumers.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Pass.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Utils/LoopSimplify.h"
#include "llvm/Analysis/LoopPass.h"
#include "llvm/Analysis/LoopAnalysisManager.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/IRPrintingPasses.h"
#include "llvm/Transforms/Utils/LoopSimplify.h"
#include "llvm/ADT/DepthFirstIterator.h"
#include "llvm/ADT/SetOperations.h"
#include "llvm/ADT/SetVector.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/Analysis/AssumptionCache.h"
#include "llvm/Analysis/BasicAliasAnalysis.h"
#include "llvm/Analysis/DependenceAnalysis.h"
#include "llvm/Analysis/GlobalsModRef.h"
#include "llvm/Analysis/InstructionSimplify.h"
#include "llvm/Transforms/Utils.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/ScalarEvolutionAliasAnalysis.h"
#include "llvm/Transforms/Utils/Local.h"
#include "llvm/IR/CFG.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Utils.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Utils/LoopUtils.h"
#include "llvm/Bitcode/BitcodeWriter.h"
#include "llvm/Transforms/Utils/LoopUtils.h"
#include "llvm/ADT/ScopeExit.h"
#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/Analysis/BasicAliasAnalysis.h"
#include "llvm/Analysis/GlobalsModRef.h"
#include "llvm/Analysis/InstructionSimplify.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/LoopPass.h"
#include "llvm/Analysis/MustExecute.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/ScalarEvolutionAliasAnalysis.h"
#include "llvm/Analysis/ScalarEvolutionExpander.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"
#include "llvm/Transforms/Scalar/SimplifyCFG.h"
#include "llvm/Analysis/TargetTransformInfo.h"
#include "llvm/Analysis/ValueTracking.h"
#include "llvm/IR/DIBuilder.h"
#include "llvm/IR/DomTreeUpdater.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PatternMatch.h"
#include "llvm/IR/ValueHandle.h"
#include "llvm/Pass.h"
#include "HI_SysExec.h"
#include "HI_print.h"
#include <stdio.h>
#include <string>
#include <ios>
#include <stdlib.h>
#include <llvm/IR/Module.h>
#include <llvm/IRReader/IRReader.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/Support/SourceMgr.h>
#include <llvm/Support/FileSystem.h>
// #include "HI_DuplicateInstRm.h"
#include "HI_SeparateConstOffsetFromGEP.h"
// #include "polly/LinkAllPasses.h"
#include "llvm/Transforms/InstCombine/InstCombine.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm-c/TargetMachine.h"
#include "HI_VarWidthReduce.h"
#include "HI_IntstructionMoveBackward.h"
#include "HI_HLSDuplicateInstRm.h"
#include "HI_AggressiveLSR_MUL.h"
#include "HI_WithDirectiveTimingResourceEvaluation.h"
#include "HI_StringProcess.h"
#include "HI_FunctionInstantiation.h"
#include "HI_ReplaceSelectAccess.h"
#include "HI_LoopUnroll.h"
#include "HI_RemoveRedundantAccess.h"
#include "HI_IR2SourceCode.h"

#include "HI_LoopLabeler.h"

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <memory>
#include <utility>
#include "HI_print.h"
#include "HI_SysExec.h"
#include "clang/AST/Type.h"
#include "clang/Driver/Options.h"
#include "clang/AST/AST.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/ASTConsumers.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "HI_FunctionInterfaceInfo.h"
#include "HI_Mul2Shl.h"
#include "HI_MulOrderOpt.h"
#include "HI_MuxInsertionArrayPartition.h"
#include "llvm/Pass.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Transforms/IPO.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Utils.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Transforms/Utils/CodeExtractor.h"
#include "HI_LoadALAP.h"
#include "HI_PragmaTargetExtraction.h"
#include "HI_ArraySensitiveToLoopLevel.h"
#include "HI_TopLoop2Func.h"

void ReplaceAll(std::string& strSource, const std::string& strOld, const std::string& strNew);

void pathAdvice();

// load the HLS database of timing and resource
void DES_Load_Instruction_Info(const char* config_file_name, std::map<std::string,Info_type_list> &BiOp_Info_name2list_map);

void clangPreProcess(const char **argv, std::string top_str, std::map<std::string, int> &FuncParamLine2OutermostSize);

void preAnalyzeProcess(
    llvm::Module &Mod,
    std::string top_str,
    std::string configFile_str,

    std::map<std::string,Info_type_list> &BiOp_Info_name2list_map,
    
    // record the specific level of loop is sensitive to specific partitioning scheme of the specific dimension of array
    std::map<std::string, std::set<std::pair<std::string, std::pair<std::string, int>> >> &LoopLabel2DrivenArrayDimensions,
    std::map<std::string, std::vector<int>> &IRFunc2BeginLine,
    std::map<std::string, std::string> &IRLoop2LoopLabel,
    std::map<std::string, int> &IRLoop2OriginTripCount,
    std::map<std::string, std::vector<std::string>> &LoopIRName2NextLevelSubLoopIRNames,
    std::map<std::string, int> &LoopIRName2Depth,
    std::map<std::string, std::vector< std::pair<std::string, std::string>>> &LoopIRName2Array,
    std::map<std::pair<std::string, std::string>, HI_PragmaArrayInfo> &TargetExtName2ArrayInfo,
    std::map<std::string, std::string> &TopLoopIR2FuncName,
    std::map<std::string, std::vector<int>> &Loop2PotentialUnrollFactors,
    std::map<std::string, int> &FuncParamLine2OutermostSize,
    std::vector<std::string> &FuncNames,
    bool debugFlag
);

#endif