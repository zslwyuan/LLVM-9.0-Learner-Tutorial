#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Pass.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/raw_ostream.h"
#include "HI_FindFunction.h"
#include "HI_print.h"
#include <stdio.h>
#include <string>
#include <ios>
#include <stdlib.h>

using namespace llvm;


int main(int argc, char **argv) {
  if (argc < 2) {
    errs() << "Usage: " << argv[0] << " <C/C++ file>\n";
    return 1;
  }

  // Compile the source code into IR and Parse the input LLVM IR file into a module
  SMDiagnostic Err;
  LLVMContext Context;
  std::string cmd_str = "clang -O1 -emit-llvm -S "+std::string(argv[1])+" -o top.bc";
  print_cmd(cmd_str.c_str());
  system(cmd_str.c_str());


  std::unique_ptr<Module> Mod(parseIRFile("top.bc", Err, Context));
  if (!Mod) {
    Err.print(argv[0], errs());
    return 1;
  }
  

  // Create a pass manager and fill it with the passes we want to run.
  legacy::PassManager PM;
  PM.add(new HI_FindFunctions());
  PM.run(*Mod);

  return 0;
}
