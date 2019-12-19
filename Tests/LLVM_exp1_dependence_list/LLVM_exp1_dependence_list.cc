#include "HI_DependenceList.h"
#include "HI_FindFunction.h"
#include "HI_SysExec.h"
#include "HI_print.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Pass.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/raw_ostream.h"
#include <ios>
#include <stdio.h>
#include <stdlib.h>
#include <string>

using namespace llvm;

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        errs() << "Usage: " << argv[0] << " <C/C++ file>\n";
        return 1;
    }

    // Compile the source code into IR and Parse the input LLVM IR file into a module
    SMDiagnostic Err;
    LLVMContext Context;
    std::string cmd_str = "clang -O1 -emit-llvm -S " + std::string(argv[1]) + " -o top.bc 2>&1";
    print_cmd(cmd_str.c_str());
    bool result = sysexec(cmd_str.c_str());
    assert(result); // ensure the cmd is executed successfully
                    // system(cmd_str.c_str());

    std::unique_ptr<Module> Mod(parseIRFile("top.bc", Err, Context));
    if (!Mod)
    {
        Err.print(argv[0], errs());
        return 1;
    }

    // Create a pass manager and fill it with the passes we want to run.
    legacy::PassManager PM;

    auto dominatortreewrapperpass = new DominatorTreeWrapperPass();
    PM.add(dominatortreewrapperpass);
    print_info("Enable DominatorTreeWrapperPass Pass");

    print_info("Enable HI_FindFunctions Pass");
    print_info("Enable HI_DependenceList Pass");
    PM.add(new HI_FindFunctions());
    PM.add(new HI_DependenceList("Instructions", "Instruction_Dep"));

    print_status("Start LLVM processing");
    PM.run(*Mod);
    print_status("Accomplished LLVM processing");
    return 0;
}
