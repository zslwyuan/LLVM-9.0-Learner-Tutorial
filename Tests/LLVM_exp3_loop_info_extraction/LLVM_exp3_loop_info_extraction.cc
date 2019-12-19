#include "LLVM_exp3_loop_info_extraction.h"

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
    auto hi_findfunction = new HI_FindFunctions();
    PM.add(hi_findfunction);
    auto hi_dependencelist = new HI_DependenceList("Instructions", "Instruction_Dep");
    PM.add(hi_dependencelist);

    print_info("Enable LoopSimplify Pass");
    print_info("Enable IndVarSimplify Pass");
    auto loopsimplifypass = createLoopSimplifyPass();
    PM.add(createLoopSimplifyPass());
    auto indvarsimplifypass = createIndVarSimplifyPass();
    PM.add(createIndVarSimplifyPass());

    print_info("Enable HI_LoopInFormationCollect Pass");
    auto hi_loopinformationcollect = new HI_LoopInFormationCollect("Loops");
    hi_loopinformationcollect->solveDependency(PM);
    PM.add(hi_loopinformationcollect);

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
