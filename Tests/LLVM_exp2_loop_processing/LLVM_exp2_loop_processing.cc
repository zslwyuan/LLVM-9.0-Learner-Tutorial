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
#include "HI_FindFunction.h"
#include "HI_DependenceList.h"
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

using namespace llvm;


int main(int argc, char **argv) {
  if (argc < 2) {
    errs() << "Usage: " << argv[0] << " <C/C++ file>\n";
    return 1;
  }

  // Compile the source code into IR and Parse the input LLVM IR file into a module
  SMDiagnostic Err;
  LLVMContext Context;
  std::string cmd_str = "clang -O1 -emit-llvm -S "+std::string(argv[1])+" -o top.bc 2>&1";
  print_cmd(cmd_str.c_str());
  bool result = sysexec(cmd_str.c_str());
  assert(result); // ensure the cmd is executed successfully
 // system(cmd_str.c_str());

  std::unique_ptr<Module> Mod(parseIRFile("top.bc", Err, Context));
  if (!Mod) {
    Err.print(argv[0], errs());
    return 1;
  }
  

  // Create a pass manager and fill it with the passes we want to run.
  legacy::PassManager PM;

  print_info("Enable HI_FindFunctions Pass");
  print_info("Enable HI_DependenceList Pass");
  PM.add(new HI_FindFunctions());
  PM.add(new HI_DependenceList("Instructions","Instruction_Dep"));

  // LPPassManager *LPPM = new LPPassManager();
  // LPPM->add(new IndVarSimplifyPass());
  // PM.add(LPPM);
  print_info("Enable LoopSimplify Pass");
  print_info("Enable IndVarSimplify Pass");
  PM.add(createLoopSimplifyPass());
  PM.add(createIndVarSimplifyPass());
  print_status("Start LLVM processing");
  PM.run(*Mod);
  print_status("Accomplished LLVM processing");

  print_status("Writing LLVM IR to File");
  std::error_code EC;
  llvm::raw_fd_ostream OS("top_output.bc", EC, llvm::sys::fs::F_None);
  WriteBitcodeToFile(*Mod, OS);
  OS.flush();

  print_status("Translate the IR to be readable one");
  cmd_str = "llvm-dis top_output.bc 2>&1";
  print_cmd(cmd_str.c_str());
  result = sysexec(cmd_str.c_str());
  assert(result); // ensure the cmd is executed successfully
  return 0;
}
