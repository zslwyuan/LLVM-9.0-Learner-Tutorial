#ifndef _HI_FINDFUNCTION 
#define _HI_FINDFUNCTION
// related headers should be included.
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Pass.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/raw_ostream.h"
#include "HI_print.h"
#include <stdio.h>
#include <string>
#include <ios>
#include <stdlib.h>

using namespace llvm;

class HI_FindFunctions : public ModulePass {
public:
  HI_FindFunctions() : ModulePass(ID) {} // define a pass, which can be inherited from ModulePass, LoopPass, FunctionPass and etc.
  void getAnalysisUsage(AnalysisUsage &AU) const;
  virtual bool runOnModule(Module &M);
  virtual bool doInitialization(Module &M)
  {
      print_status("Initilizing HI_FindFunctions pass.");
      return false;      
  }
  static char ID;
};



#endif
