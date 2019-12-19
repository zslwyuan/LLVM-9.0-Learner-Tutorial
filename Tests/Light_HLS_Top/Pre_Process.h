#ifndef _Pre_Process
#define _Pre_Process

#include "HI_SysExec.h"
#include "HI_print.h"
#include "clang/AST/AST.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/AST/Type.h"
#include "clang/Driver/Options.h"
#include "clang/Frontend/ASTConsumers.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/ADT/DepthFirstIterator.h"
#include "llvm/ADT/ScopeExit.h"
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
#include "llvm/Analysis/LoopAnalysisManager.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/LoopPass.h"
#include "llvm/Analysis/MustExecute.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/ScalarEvolutionAliasAnalysis.h"
#include "llvm/Analysis/ScalarEvolutionExpander.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"
#include "llvm/Analysis/TargetTransformInfo.h"
#include "llvm/Analysis/ValueTracking.h"
#include "llvm/Bitcode/BitcodeWriter.h"
#include "llvm/IR/CFG.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DIBuilder.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/DomTreeUpdater.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRPrintingPasses.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PatternMatch.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/ValueHandle.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Pass.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Scalar/SimplifyCFG.h"
#include "llvm/Transforms/Utils.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Transforms/Utils/Local.h"
#include "llvm/Transforms/Utils/LoopSimplify.h"
#include "llvm/Transforms/Utils/LoopUtils.h"
#include <ios>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IRReader/IRReader.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/SourceMgr.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
// #include "HI_DuplicateInstRm.h"
#include "HI_SeparateConstOffsetFromGEP.h"
// #include "polly/LinkAllPasses.h"
#include "HI_AggressiveLSR_MUL.h"
#include "HI_FunctionInstantiation.h"
#include "HI_HLSDuplicateInstRm.h"
#include "HI_IR2SourceCode.h"
#include "HI_IntstructionMoveBackward.h"
#include "HI_LoopUnroll.h"
#include "HI_RemoveRedundantAccess.h"
#include "HI_ReplaceSelectAccess.h"
#include "HI_StringProcess.h"
#include "HI_VarWidthReduce.h"
#include "HI_WithDirectiveTimingResourceEvaluation.h"
#include "llvm-c/TargetMachine.h"
#include "llvm/Transforms/InstCombine/InstCombine.h"
#include "llvm/Transforms/Scalar.h"

#include "HI_LoopLabeler.h"

#include "HI_FunctionInterfaceInfo.h"
#include "HI_LoadALAP.h"
#include "HI_Mul2Shl.h"
#include "HI_MulOrderOpt.h"
#include "HI_MuxInsertionArrayPartition.h"
#include "HI_PragmaTargetExtraction.h"
#include "HI_SysExec.h"
#include "HI_print.h"
#include "clang/AST/AST.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/AST/Type.h"
#include "clang/Driver/Options.h"
#include "clang/Frontend/ASTConsumers.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/Pass.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Transforms/IPO.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Utils.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Transforms/Utils/CodeExtractor.h"
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <memory>
#include <utility>

void ReplaceAll(std::string &strSource, const std::string &strOld, const std::string &strNew);

void pathAdvice();

// load the HLS database of timing and resource
void DES_Load_Instruction_Info(const char *config_file_name,
                               std::map<std::string, Info_type_list> &BiOp_Info_name2list_map);

void clangPreProcess(const char **argv, std::string top_str,
                     std::map<std::string, int> &FuncParamLine2OutermostSize);

#endif