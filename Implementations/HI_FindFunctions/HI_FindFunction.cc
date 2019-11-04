#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Pass.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/raw_ostream.h"
#include "HI_print.h"
#include "HI_FindFunction.h"
#include <stdio.h>
#include <string>
#include <ios>
#include <stdlib.h>
#include <fstream>
#include <sstream>
#include <string>

using namespace llvm;

bool HI_FindFunctions::runOnModule(Module &M) // The runOnModule declaration will overide the virtual one in ModulePass, which will be executed for each Module.
{
    for (Module::iterator GI = M.begin(),GE = M.end(); GI != GE; ++GI) //Module iterator is used to iterate the functions in the module, ++GI is better than GI ++, for the sake of stability
    {
        *Function_Demangle << "Found function Definition named == " << GI->getName() << "\n"; // a Function class is inherited from Value class, which has a function to get the name of the value (function).
        std::string fname(GI->getName());
        if (fname[0]=='_')
            *Function_Demangle << "     its demangled name == " << DemangleFunctionName(GI->getName()) << "\n";
    }

    // find all the Call Instruction and demangle the called function name
    for (Function &F : M) 
    {
        for (BasicBlock &B : F) 
        {        
            for (Instruction &I : B) 
            {
                if (CallInst* Call_I = dyn_cast<CallInst>(&I))
                {
                    if (Function_Checked.find(Call_I->getCalledFunction())==Function_Checked.end())
                    {
                        *Function_Demangle << "Found CallInst == " << I << "\n";
                        *Function_Demangle << "     its getCalledFunction == " << Call_I->getCalledFunction()->getName() << "\n";
                        std::string fname(Call_I->getCalledFunction()->getName());
                        if (fname[0]=='_')
                        {
                            *Function_Demangle << Call_I->getCalledFunction()->getName() << " **** " <<  DemangleFunctionName(fname) << "\n";
                            *Function_Demangle << "     its demangled name == " << DemangleFunctionName(fname) << "\n";
                            Function_Demangle_Map[Call_I->getCalledFunction()->getName()] = DemangleFunctionName(fname);
                        }
                        Function_Checked.insert(Call_I->getCalledFunction());
                    }
                }
            }
        }
    }
    
    *Function_Demangle << "===============printing the module ====================================\n";
    *Function_Demangle << M << "\n";
    Function_Demangle->flush();
    return false;
}

char HI_FindFunctions::ID = 0;  // the ID for pass should be initialized but the value does not matter, since LLVM uses the address of this variable as label instead of its value.

void HI_FindFunctions::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesAll();
}
