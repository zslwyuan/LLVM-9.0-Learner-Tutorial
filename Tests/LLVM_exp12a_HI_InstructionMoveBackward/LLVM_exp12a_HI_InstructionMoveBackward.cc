#include "LLVM_exp12a_HI_InstructionMoveBackward.h"

using namespace llvm;
using namespace polly;

int main(int argc, char **argv)
{
    if (argc < 4)
    {
        errs() << "Usage: " << argv[0] << " <C/C++ file> <Top_Function_Name> <Config_File_Path>\n";
        return 1;
    }

    LLVMInitializeX86TargetInfo();
    LLVMInitializeX86Target();
    LLVMInitializeX86TargetMC();
    // Compile the source code into IR and Parse the input LLVM IR file into a module
    SMDiagnostic Err;
    LLVMContext Context;
    std::string cmd_str = "clang -O1 -emit-llvm -S " + std::string(argv[1]) + " -o top.bc 2>&1";
    std::string top_str = std::string(argv[2]);
    std::string configFile_str = std::string(argv[3]);
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
    legacy::PassManager PM, PM1;
    LLVMTargetRef T;

    char *Error;

    if (LLVMGetTargetFromTriple((Mod->getTargetTriple()).c_str(), &T, &Error))
    {
        print_error(Error);
    }
    else
    {
        std::string targetname = LLVMGetTargetName(T);
        targetname = "The target machine is: " + targetname;
        print_info(targetname.c_str());
    }

    Triple ModuleTriple(Mod->getTargetTriple());
    TargetLibraryInfoImpl TLII(ModuleTriple);
    PM1.add(new TargetLibraryInfoWrapperPass(TLII));

    print_info("Enable LoopSimplify Pass");
    auto loopsimplifypass = createLoopSimplifyPass();
    PM1.add(loopsimplifypass);

    auto indvarsimplifypass = createIndVarSimplifyPass();
    PM1.add(indvarsimplifypass);
    print_info("Enable IndVarSimplifyPass Pass");

    PM1.add(createTargetTransformInfoWrapperPass(TargetIRAnalysis()));
    print_info("Enable TargetIRAnalysis Pass");

    auto hi_separateconstoffsetfromgep =
        new HI_SeparateConstOffsetFromGEP("HI_SeparateConstOffsetFromGEP", true);
    PM1.add(hi_separateconstoffsetfromgep);
    print_info("Enable HI_SeparateConstOffsetFromGEP Pass");
    // auto separateconstoffsetfromgep = createSeparateConstOffsetFromGEPPass(true);
    // PM.add(separateconstoffsetfromgep);
    // print_info("Enable SeparateConstOffsetFromGEP Pass");

    auto hi_duplicateinstrm = new HI_DuplicateInstRm("rmInsts");
    PM1.add(hi_duplicateinstrm);
    print_info("Enable HI_DuplicateInstRm Pass");

    // auto loopstrengthreducepass = createLoopStrengthReducePass();
    // PM1.add(loopstrengthreducepass);
    // print_info("Enable LoopStrengthReducePass Pass");

    // auto lazyvalueinfowrapperpass = new LazyValueInfoWrapperPass();
    // PM.add(lazyvalueinfowrapperpass);
    // print_info("Enable LazyValueInfoWrapperPass Pass");

    auto hi_varwidthreduce = new HI_VarWidthReduce("VarWidth");
    PM1.add(hi_varwidthreduce);
    print_info("Enable HI_VarWidthReduce Pass");

    // PM.add(createCorrelatedValuePropagationPass());
    // print_info("Enable CorrelatedValuePropagation Pass");

    auto hi_intstructionmovebackward =
        new HI_IntstructionMoveBackward("HI_IntstructionMoveBackward");
    PM1.add(hi_intstructionmovebackward);
    print_info("Enable HI_IntstructionMoveBackward Pass");

    // PM.add(createCorrelatedValuePropagationPass());
    // print_info("Enable CorrelatedValuePropagation Pass");

    PM1.run(*Mod);

    std::error_code EC;
    llvm::raw_fd_ostream OS1("top_output0.bc", EC, llvm::sys::fs::F_None);
    WriteBitcodeToFile(*Mod, OS1);
    OS1.flush();

    auto hi_duplicateinstrm1 = new HI_DuplicateInstRm("rmInsts");
    PM.add(hi_duplicateinstrm1);
    print_info("Enable HI_DuplicateInstRm Pass");

    // PM.add(createStraightLineStrengthReducePass());
    // print_info("Enable StraightLineStrengthReduce Pass");

    // auto instructioncombiningpass = createInstructionCombiningPass(true);
    // PM.add(instructioncombiningpass);
    // print_info("Enable InstructionCombiningPass Pass");

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

    auto hi_loopinformationcollect = new HI_LoopInFormationCollect("Loops");
    PM.add(hi_loopinformationcollect);
    print_info("Enable HI_LoopInFormationCollect Pass");

    auto hi_loopdependenceanalysis = new HI_LoopDependenceAnalysis("HI_LoopDependenceAnalysis");
    print_info("Enable HI_LoopDependenceAnalysis Pass");
    PM.add(hi_loopdependenceanalysis);

    // auto hi_simpletimingevaluation = new
    // HI_SimpleTimingEvaluation("HI_SimpleTimingEvaluation",top_str.c_str()); print_info("Enable
    // HI_SimpleTimingEvaluation Pass"); PM.add(hi_simpletimingevaluation);

    auto hi_nodirectivetimingresourceevaluation = new HI_NoDirectiveTimingResourceEvaluation(
        configFile_str.c_str(), "HI_NoDirectiveTimingResourceEvaluation", "BRAM_info",
        top_str.c_str());
    print_info("Enable HI_NoDirectiveTimingResourceEvaluation Pass");
    PM.add(hi_nodirectivetimingresourceevaluation);

    print_info("Enable HI_FindFunctions Pass");
    print_info("Enable HI_DependenceList Pass");
    auto hi_findfunction = new HI_FindFunctions();
    PM.add(hi_findfunction);
    auto hi_dependencelist = new HI_DependenceList("Instructions", "Instruction_Dep");
    PM.add(hi_dependencelist);

    // while (1)
    // {
    //     int opBitWid, outBitWid;
    //     std::string ClockPerid,opcode;
    //     std::cin >> opcode >> opBitWid >> outBitWid >> ClockPerid;
    //     if (opcode=="end")
    //       break;
    //     // int DSP = testObj->get_N_DSP(opcode, opBitWid , outBitWid, ClockPerid);
    //     // int FF = testObj->get_N_FF(opcode, opBitWid , outBitWid, ClockPerid);
    //     // int LUT = testObj->get_N_LUT(opcode, opBitWid , outBitWid, ClockPerid);
    //     // int Lat = testObj->get_N_Lat(opcode, opBitWid , outBitWid, ClockPerid);
    //     // double Delay = testObj->get_N_Delay(opcode, opBitWid , outBitWid, ClockPerid);
    //     hi_nodirectivetimingresourceevaluation->get_inst_info(opcode, opBitWid , outBitWid,
    //     ClockPerid).print();
    // }

    print_status("Start LLVM processing");
    PM.run(*Mod);
    print_status("Accomplished LLVM processing");

    assert(hi_nodirectivetimingresourceevaluation->topFunctionFound &&
           "The specified top function is not found in the program");

    print_status("Writing LLVM IR to File");

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
