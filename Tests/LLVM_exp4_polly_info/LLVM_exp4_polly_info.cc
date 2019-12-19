#include "LLVM_exp4_polly_info.h"

using namespace llvm;
using namespace polly;

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

    PM.add(new LoopInfoWrapperPass());
    PM.add(new RegionInfoPass());
    PM.add(new ScalarEvolutionWrapperPass());
    PM.add(new LoopAccessLegacyAnalysis());
    PM.add(new DominatorTreeWrapperPass());
    PM.add(new OptimizationRemarkEmitterWrapperPass());

    print_info("Enable HI_FindFunctions Pass");
    print_info("Enable HI_DependenceList Pass");
    PM.add(new HI_FindFunctions());
    PM.add(new HI_DependenceList("Instructions", "Instruction_Dep"));

    // LPPassManager *LPPM = new LPPassManager();
    // LPPM->add(new IndVarSimplifyPass());
    // PM.add(LPPM);

    print_info("Enable LoopSimplify Pass");
    print_info("Enable IndVarSimplify Pass");
    PM.add(createLoopSimplifyPass());
    PM.add(createIndVarSimplifyPass());

    print_info("Enable HI_LoopInFormationCollect Pass");
    PM.add(new HI_LoopInFormationCollect("Loops"));

    print_info("Enable ScopDetectionWrapperPass Pass");
    PM.add(new ScopDetectionWrapperPass());
    print_info("Enable AAResultsWrapperPass Pass");
    PM.add(new AAResultsWrapperPass());
    print_info("Enable AssumptionCacheTracker Pass");
    PM.add(new AssumptionCacheTracker());
    print_info("Enable ScopInfoWrapperPass Pass");
    PM.add(new ScopInfoWrapperPass());
    print_info("Enable ScopInfoRegionPass Pass");
    PM.add(new ScopInfoRegionPass());
    print_info("Enable DependenceInfoWrapperPass Pass");
    PM.add(new DependenceInfoWrapperPass());
    print_info("Enable PolyhedralInfo Pass");
    PM.add(new PolyhedralInfo());
    print_info("Enable PollyInformation Pass");
    PM.add(new HI_Polly_Info("PollyInformation"));
    // AU.addRequiredTransitive<polly::DependenceInfoWrapperPass>();
    // AU.addRequiredTransitive<polly::ScopInfoWrapperPass>();

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
