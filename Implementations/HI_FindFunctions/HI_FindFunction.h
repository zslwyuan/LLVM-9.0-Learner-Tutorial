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
#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/GraphWriter.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/LoopPass.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IRReader/IRReader.h"
#include <bits/stl_map.h>
#include "llvm/ADT/Statistic.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/PassAnalysisSupport.h"
#include "llvm/Transforms/Utils/LoopUtils.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/LoopAccessAnalysis.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/Transforms/Utils/LoopUtils.h"
#include "llvm/Transforms/Utils/LoopVersioning.h"
#include "llvm/Transforms/Utils/ValueMapper.h"
#include "llvm/ADT/SmallVector.h"
#include <set>
#include <map>
#include <cxxabi.h>
#include "llvm/Demangle/Demangle.h"
#include <sys/time.h>
#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>
#include <sys/stat.h>
#include <unistd.h>
#include <string>
#include <fstream>
#include <sstream>

using namespace llvm;

class HI_FindFunctions : public ModulePass {
public:
    HI_FindFunctions() : ModulePass(ID) 
    {
        Function_Demangle = new raw_fd_ostream("Function_Demangle", ErrInfo, sys::fs::F_None);
        Function_Demangle_Map.clear();
    } // define a pass, which can be inherited from ModulePass, LoopPass, FunctionPass and etc.
    ~HI_FindFunctions()
    {
        Function_Demangle->flush();
        delete Function_Demangle;
    }
    void getAnalysisUsage(AnalysisUsage &AU) const;
    bool runOnModule(llvm::Module &M);
    virtual bool doInitialization(llvm::Module &M)
    {
        print_status("Initilizing HI_FindFunctions pass.");
        Function_Checked.clear();
        return false;      
    }
    static char ID;
    std::string DemangleFunctionName (std::string mangled_name)
    {
        std::string demangled_name;

        // demangle the function
        if (mangled_name.find("_Z")==std::string::npos)
            demangled_name = mangled_name;
        else
            {
                std::stringstream iss(mangled_name);
                iss.ignore(1, '_');iss.ignore(1, 'Z');
                int len; iss >> len; while (len--) {char tc;iss>>tc;demangled_name+=tc;}
            }
        return demangled_name;
    }

    std::set<Function*> Function_Checked;
    std::error_code ErrInfo;
    raw_ostream *Function_Demangle;
    std::map<std::string,std::string> Function_Demangle_Map;
    
/// Timer

    struct timeval tv_begin;
    struct timeval tv_end;
};


#endif
