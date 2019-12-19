#include "LLVM_exp5_SimpleTimingAnalysis.h"

using namespace llvm;
using namespace polly;

int main(int argc, char **argv)
{
    if (argc < 3)
    {
        errs() << "Usage: " << argv[0] << " <C/C++ file> <Top_Function_Name>\n";
        return 1;
    }

    // Compile the source code into IR and Parse the input LLVM IR file into a module
    SMDiagnostic Err;
    LLVMContext Context;
    std::string cmd_str = "clang -O1 -emit-llvm -S " + std::string(argv[1]) + " -o top.bc 2>&1";
    std::string top_str = std::string(argv[2]);
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

    // LPPassManager *LPPM = new LPPassManager();
    // LPPM->add(new IndVarSimplifyPass());
    // PM.add(LPPM);

    print_info("Enable LoopSimplify Pass");
    auto loopsimplifypass = createLoopSimplifyPass();
    PM.add(loopsimplifypass);

    auto indvarsimplifypass = createIndVarSimplifyPass();
    PM.add(indvarsimplifypass);
    print_info("Enable IndVarSimplifyPass Pass");

    // auto loopstrengthreducepass = createLoopStrengthReducePass();
    // PM.add(loopstrengthreducepass);
    // print_info("Enable LoopStrengthReducePass Pass");

    auto loopinfowrapperpass = new LoopInfoWrapperPass();
    PM.add(loopinfowrapperpass);
    print_info("Enable LoopInfoWrapperPass Pass");

    auto regioninfopass = new RegionInfoPass();
    PM.add(regioninfopass);
    print_info("Enable RegionInfoPass Pass");

    auto scalarevolutionwrapperpass = new ScalarEvolutionWrapperPass();
    PM.add(scalarevolutionwrapperpass);
    print_info("Enable ScalarEvolutionWrapperPass Pass");

    auto loopaccesslegacyanalysis = new LoopAccessLegacyAnalysis();
    PM.add(loopaccesslegacyanalysis);
    print_info("Enable LoopAccessLegacyAnalysis Pass");

    auto dominatortreewrapperpass = new DominatorTreeWrapperPass();
    PM.add(dominatortreewrapperpass);
    print_info("Enable DominatorTreeWrapperPass Pass");

    auto optimizationremarkemitterwrapperpass = new OptimizationRemarkEmitterWrapperPass();
    PM.add(optimizationremarkemitterwrapperpass);
    print_info("Enable OptimizationRemarkEmitterWrapperPass Pass");

    auto hi_loopinformationcollect = new HI_LoopInFormationCollect("Loops");
    PM.add(hi_loopinformationcollect);
    print_info("Enable HI_LoopInFormationCollect Pass");

    auto aaresultswrapperpass = new AAResultsWrapperPass();
    print_info("Enable AAResultsWrapperPass Pass");
    PM.add(aaresultswrapperpass);

    auto scopdetectionwrapperpass = new ScopDetectionWrapperPass();
    print_info("Enable ScopDetectionWrapperPass Pass");
    PM.add(scopdetectionwrapperpass);

    auto assumptioncachetracker = new AssumptionCacheTracker();
    print_info("Enable AssumptionCacheTracker Pass");
    PM.add(assumptioncachetracker);

    auto scopinfowrapperpass = new ScopInfoWrapperPass();
    print_info("Enable ScopInfoWrapperPass Pass");
    PM.add(scopinfowrapperpass);

    auto scopinforegionpass = new ScopInfoRegionPass();
    print_info("Enable ScopInfoRegionPass Pass");
    PM.add(scopinforegionpass);

    auto dependenceinfowrapperpass = new DependenceInfoWrapperPass();
    print_info("Enable DependenceInfoWrapperPass Pass");
    PM.add(dependenceinfowrapperpass);

    auto polyhedralinfo = new PolyhedralInfo();
    print_info("Enable PolyhedralInfo Pass");
    PM.add(polyhedralinfo);

    auto hi_polly_info = new HI_Polly_Info("PollyInformation");
    print_info("Enable PollyInformation Pass");
    PM.add(hi_polly_info);

    // auto hi_loopdependenceanalysis = new HI_LoopDependenceAnalysis("HI_LoopDependenceAnalysis");
    // print_info("Enable HI_LoopDependenceAnalysis Pass");
    // PM.add(hi_loopdependenceanalysis);

    auto hi_simpletimingevaluation =
        new HI_SimpleTimingEvaluation("HI_SimpleTimingEvaluation", top_str.c_str());
    print_info("Enable HI_SimpleTimingEvaluation Pass");
    PM.add(hi_simpletimingevaluation);

    print_info("Enable HI_FindFunctions Pass");
    print_info("Enable HI_DependenceList Pass");
    auto hi_findfunction = new HI_FindFunctions();
    PM.add(hi_findfunction);
    auto hi_dependencelist = new HI_DependenceList("Instructions", "Instruction_Dep");
    PM.add(hi_dependencelist);
    // AU.addRequiredTransitive<polly::DependenceInfoWrapperPass>();
    // AU.addRequiredTransitive<polly::ScopInfoWrapperPass>();

    print_status("Start LLVM processing");
    PM.run(*Mod);
    print_status("Accomplished LLVM processing");

    assert(hi_simpletimingevaluation->topFunctionFound &&
           "The specified top function is not found in the program");

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
