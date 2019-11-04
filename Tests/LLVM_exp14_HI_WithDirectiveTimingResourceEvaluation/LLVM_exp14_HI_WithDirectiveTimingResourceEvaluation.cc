#include "LLVM_exp14_HI_WithDirectiveTimingResourceEvaluation.h"
#include "ConfigParse.h"

using namespace llvm;
using namespace polly;

std:: string clock_period_str;
std:: string  HLS_lib_path;

void ReplaceAll(std::string& strSource, const std::string& strOld, const std::string& strNew)
{
    int nPos = 0;
    while ((nPos = strSource.find(strOld, nPos)) != strSource.npos)
    {
        strSource.replace(nPos, strOld.length(), strNew);
        nPos += strNew.length();
    }
} 

void pathAdvice()
{
    std::cout<< "===============================================================================" << std::endl;
    std::cout<< "if undefined reference occurs, please check whether the following include paths are required." << std::endl;
    std::string line;
    std::string cmd_str = "clang++ ../testcase/test.c  -v 2> ciinfor";
    print_cmd(cmd_str.c_str());
    sysexec(cmd_str.c_str()); 
    std::ifstream infile("ciinfor");
    while (std::getline(infile, line))
    {
        if (line.find("#include <...> search starts here")!=std::string::npos)
        {
            while (std::getline(infile, line))
            {
                if (line.find("End of search list.")!=std::string::npos)
                {                                        
                    break;
                }
                else
                {
                    ReplaceAll(line," ","");
                    ReplaceAll(line,"\n","");                        
                  //  CI.getHeaderSearchOpts().AddPath(line,frontend::ExternCSystem,false,true);
                    line = "Potential Path : " + line;
                    print_info(line.c_str());
                }
                
            }
            break;
        }
    }
    std::cout<< "===============================================================================" << std::endl;
}

using namespace clang;
using namespace clang::driver;
using namespace clang::tooling;



static llvm::cl::OptionCategory StatSampleCategory("Stat Sample");


int main(int argc, const char **argv) {
    if (argc < 4) {
      errs() << "Usage: " << argv[0] << " <C/C++ file> <Top_Function_Name> <Config_File_Path>\n";
      return 1;
    }

    pathAdvice();
    std::error_code EC;
    
    // parse the command-line args passed to your code
    int labelerArgc = 2;
    CommonOptionsParser op(labelerArgc, argv, StatSampleCategory);     

    std::string top_str = std::string(argv[2]);
    std::string configFile_str = std::string(argv[3]);

    // create a new Clang Tool instance (a LibTooling environment)
    ClangTool Tool(op.getCompilations(), op.getSourcePathList());
    Rewriter TheRewriter0,TheRewriter1;

    // run the Clang Tool, creating a new FrontendAction, which will run the AST consumer 
    Tool.run(HI_LoopLabeler_rewrite_newFrontendActionFactory<HI_LoopLabeler_FrontendAction>("PLog",TheRewriter0,"tmp.cc").get());
    std::map<std::string, int> FuncParamLine2OutermostSize;
    Tool.run(HI_FunctionInterfaceInfo_rewrite_newFrontendActionFactory<HI_FunctionInterfaceInfo_FrontendAction>("PLog1",TheRewriter1,"tmp1.cc",FuncParamLine2OutermostSize, top_str).get());

    LLVMInitializeX86TargetInfo();
    LLVMInitializeX86Target();
    LLVMInitializeX86TargetMC();
    // Compile the source code into IR and Parse the input LLVM IR file into a module
    SMDiagnostic Err;
    LLVMContext Context;
    // std::string cmd_str = "clang -O1 -emit-llvm -S -g "+std::string(argv[1])+" -o top.bc 2>&1";
    std::string cmd_str = "clang -O1 -emit-llvm -S -g tmp.cc -o top.bc 2>&1";

    print_cmd(cmd_str.c_str());
    bool result = sysexec(cmd_str.c_str());
    assert(result); // ensure the cmd is executed successfully


  // system(cmd_str.c_str());

    std::unique_ptr<llvm::Module> Mod(parseIRFile("top.bc", Err, Context));
    if (!Mod) {
      Err.print(argv[0], errs());
      return 1;
    }
    

    // Create a pass manager and fill it with the passes we want to run.
    legacy::PassManager PM,PM1,PM11,PM2,PM_pre,PM3, PM4;
    LLVMTargetRef T;
    ModulePassManager MPM;

    char *Error;

    if(LLVMGetTargetFromTriple((Mod->getTargetTriple()).c_str(), &T, &Error))
    {
      print_error(Error);
    }
    else
    {
      std::string targetname = LLVMGetTargetName(T);
      targetname = "The target machine is: " + targetname;
      print_info(targetname.c_str());
    }

    std::map<std::string, std::string> IRLoop2LoopLabel;
    std::map<std::string, int> IRLoop2OriginTripCount;
    std::map<std::string, std::vector<std::string>> LoopLabel2SubLoopLabel;
    std::map<std::string, std::string> LoopLabel2ParentLoopLabel;
    std::map<std::string, int> LoopLabel2UnrollFactor;
    std::map<std::string, int> LoopLabel2II;

    Parse_Config(configFile_str.c_str(), LoopLabel2UnrollFactor, LoopLabel2II);
    


    
    auto loopextract = createLoopExtractorPass(); //"HI_LoopUnroll"
    PM_pre.add(loopextract);
    print_info("Enable LoopExtractor Pass");


    std::map<std::string, std::vector<int>>  IRFunc2BeginLine;
    auto hi_ir2sourcecode = new HI_IR2SourceCode("HI_IR2SourceCode",IRLoop2LoopLabel, IRFunc2BeginLine, IRLoop2OriginTripCount);
    PM_pre.add(hi_ir2sourcecode);
    print_info("Enable HI_IR2SourceCode Pass");

    print_status("Start LLVM pre-processing");  
    PM_pre.run(*Mod);
    print_status("Accomplished LLVM pre-processing");

    llvm::raw_fd_ostream OS3("top_output_loopextract.bc", EC, llvm::sys::fs::F_None);
    WriteBitcodeToFile(*Mod, OS3);
    OS3.flush();

    cmd_str = "llvm-dis top_output_loopextract.bc 2>&1";
    print_cmd(cmd_str.c_str());
    result = sysexec(cmd_str.c_str());
    

    // llvm::errs() << *Mod;
    // for (auto it : IRLoop2LoopLabel)
    // {
    //     llvm::errs() << it.first << " <==== " << it.second << "\n";
    // }


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


    auto hi_mulorderopt = new HI_MulOrderOpt("HI_MulOrderOpt");
    PM1.add(hi_mulorderopt);
    print_info("Enable HI_MulOrderOpt Pass");

    auto CFGSimplification_pass2 = createCFGSimplificationPass();
    PM1.add(CFGSimplification_pass2);
    print_info("Enable CFGSimplificationPass Pass");


    auto hi_loopunroll = new HI_LoopUnroll(IRLoop2LoopLabel, LoopLabel2UnrollFactor, 1, false, None); //"HI_LoopUnroll"
    PM1.add(hi_loopunroll);
    print_info("Enable HI_LoopUnroll Pass");


    auto hi_separateconstoffsetfromgep = new HI_SeparateConstOffsetFromGEP("HI_SeparateConstOffsetFromGEP",true);
    PM1.add(hi_separateconstoffsetfromgep);
    print_info("Enable HI_SeparateConstOffsetFromGEP Pass");

    auto hi_mul2shl = new HI_Mul2Shl("HI_Mul2Shl", (argc == 5 && std::string(argv[4])=="DEBUG"));
    PM1.add(hi_mul2shl);
    print_info("Enable HI_Mul2Shl Pass");

    
    // auto separateconstoffsetfromgep = createSeparateConstOffsetFromGEPPass(true);
    // PM.add(separateconstoffsetfromgep);
    // print_info("Enable SeparateConstOffsetFromGEP Pass");



    auto CFGSimplification_pass = createCFGSimplificationPass();
    PM11.add(CFGSimplification_pass);
    print_info("Enable CFGSimplificationPass Pass");
    // if (argc == 5 && std::string(argv[4])=="enable-lsr")
    // {
      auto loopstrengthreducepass = createLoopStrengthReducePass();
      PM1.add(loopstrengthreducepass);
      print_info("Enable LoopStrengthReducePass Pass");
    // }

    auto hi_duplicateinstrm = new HI_DuplicateInstRm("rmInsts");
    PM1.add(hi_duplicateinstrm);
    print_info("Enable HI_DuplicateInstRm Pass");


    

    // auto lazyvalueinfowrapperpass = new LazyValueInfoWrapperPass();
    // PM.add(lazyvalueinfowrapperpass);
    // print_info("Enable LazyValueInfoWrapperPass Pass");

    // auto hi_varwidthreduce = new HI_VarWidthReduce("VarWidth");
    // PM1.add(hi_varwidthreduce);
    // print_info("Enable HI_VarWidthReduce Pass");


    auto hi_intstructionmovebackward = new HI_IntstructionMoveBackward("HI_IntstructionMoveBackward");
    PM1.add(hi_intstructionmovebackward);
    print_info("Enable HI_IntstructionMoveBackward Pass");

    if (argc == 4 || (argc == 5 && std::string(argv[4])!="disable-lsr"))
    {
      auto hi_aggressivelsr_mul = new HI_AggressiveLSR_MUL("AggressiveLSR");
      PM1.add(hi_aggressivelsr_mul);
      print_info("Enable HI_AggressiveLSR_MUL Pass");
    }


    // don't remove chained operations
    auto hi_hlsduplicateinstrm = new HI_HLSDuplicateInstRm("HLSrmInsts");
    PM11.add(hi_hlsduplicateinstrm);
    print_info("Enable HI_HLSDuplicateInstRm Pass");


    auto hi_functioninstantiation = new HI_FunctionInstantiation("HI_FunctionInstantiation",top_str);
    PM11.add(hi_functioninstantiation);
    print_info("Enable HI_FunctionInstantiation Pass");

    auto hi_removeredundantaccess = new HI_RemoveRedundantAccess("HI_RemoveRedundantAccess", top_str, (argc == 5 && std::string(argv[4])=="DEBUG"));
    PM11.add(hi_removeredundantaccess);
    print_info("Enable HI_RemoveRedundantAccess Pass");
    



    auto hi_replaceselectaccess = new HI_ReplaceSelectAccess("HI_ReplaceSelectAccess");
    PM11.add(hi_replaceselectaccess);
    print_info("Enable HI_ReplaceSelectAccess Pass");

    auto lowerswitch_pass = createLowerSwitchPass();
    PM11.add(lowerswitch_pass);
    print_info("Enable LowerSwitchPass Pass");
    
    auto ADCE_pass = createAggressiveDCEPass();
    PM11.add(ADCE_pass);
    print_info("Enable HI_ReplaceSelectAccess Pass");

    auto CFGSimplification_pass1 = createCFGSimplificationPass();
    PM11.add(CFGSimplification_pass1);
    print_info("Enable CFGSimplificationPass Pass");

    // auto hi_arrayaccesspattern = new HI_ArrayAccessPattern("HI_ArrayAccessPattern",top_str);
    // PM.add(hi_arrayaccesspattern);
    // print_info("Enable HI_ArrayAccessPattern Pass");


    // PM.add(createCorrelatedValuePropagationPass());
    // print_info("Enable CorrelatedValuePropagation Pass");
    

    // PM.add(createCorrelatedValuePropagationPass());
    // print_info("Enable CorrelatedValuePropagation Pass");


    auto hi_varwidthreduce1 = new HI_VarWidthReduce("VarWidth1",(argc == 5 && std::string(argv[4])=="DEBUG"));
    PM2.add(hi_varwidthreduce1);
    print_info("Enable HI_VarWidthReduce Pass");



    // don't remove chained operations
    auto hi_hlsduplicateinstrm1 = new HI_HLSDuplicateInstRm("HLSrmInsts1");
    PM2.add(hi_hlsduplicateinstrm1);
    print_info("Enable HI_HLSDuplicateInstRm Pass");

    auto CFGSimplification_pass4 = createCFGSimplificationPass();
    PM2.add(CFGSimplification_pass4);
    print_info("Enable CFGSimplificationPass Pass");

    auto hi_intstructionmovebackward1 = new HI_IntstructionMoveBackward("HI_IntstructionMoveBackward1");
    PM2.add(hi_intstructionmovebackward1);
    print_info("Enable HI_IntstructionMoveBackward Pass");





    PM1.run(*Mod);

    
    llvm::raw_fd_ostream OS1("top_output0.bc", EC, llvm::sys::fs::F_None);
    WriteBitcodeToFile(*Mod, OS1);
    OS1.flush();

    

    PM11.run(*Mod);

    PM2.run(*Mod);

    auto CFGSimplification_pass3 = createCFGSimplificationPass();
    PM3.add(CFGSimplification_pass3);
    print_info("Enable CFGSimplificationPass Pass");
    std::map<std::string, std::string> IRLoop2LoopLabel_eval;
    std::map<std::string, std::vector<int>>  IRFunc2BeginLine_eval;
    auto hi_ir2sourcecode_eval = new HI_IR2SourceCode("HI_IR2SourceCode_eval",IRLoop2LoopLabel_eval, IRFunc2BeginLine_eval, IRLoop2OriginTripCount);
    PM3.add(hi_ir2sourcecode_eval);
    print_info("Enable HI_IR2SourceCode Pass");
    PM3.run(*Mod);
    // auto hi_duplicateinstrm1 = new HI_DuplicateInstRm("rmInsts");
    // PM.add(hi_duplicateinstrm1);
    // print_info("Enable HI_DuplicateInstRm Pass");


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
    
    // auto hi_simpletimingevaluation = new HI_SimpleTimingEvaluation("HI_SimpleTimingEvaluation",top_str.c_str());
    // print_info("Enable HI_SimpleTimingEvaluation Pass");
    // PM.add(hi_simpletimingevaluation); 

          auto hi_MuxInsertionArrayPartition = new HI_MuxInsertionArrayPartition(
                                                            configFile_str.c_str(),
                                                            top_str.c_str(),
                                                            IRLoop2LoopLabel_eval,
                                                            LoopLabel2II,
                                                            FuncParamLine2OutermostSize,
                                                            IRFunc2BeginLine,
                                                            (argc == 5 && std::string(argv[4])=="DEBUG"));
          print_info("Enable HI_MuxInsertionArrayPartition Pass");
          PM.add(hi_MuxInsertionArrayPartition); 


    auto hi_nodirectivetimingresourceevaluation = new HI_NoDirectiveTimingResourceEvaluation(configFile_str.c_str(),"HI_NoDirectiveTimingResourceEvaluation","BRAM_info",top_str.c_str());
    print_info("Enable HI_NoDirectiveTimingResourceEvaluation Pass");
    PM.add(hi_nodirectivetimingresourceevaluation); 

    auto hi_withdirectivetimingresourceevaluation = new HI_WithDirectiveTimingResourceEvaluation(
                                                      configFile_str.c_str(),
                                                      "HI_WithDirectiveTimingResourceEvaluation","BRAM_info_0",
                                                      "ArrayLog",
                                                      top_str.c_str(),
                                                      IRLoop2LoopLabel_eval,
                                                      IRLoop2OriginTripCount,
                                                      LoopLabel2II,
                                                      LoopLabel2UnrollFactor,
                                                      FuncParamLine2OutermostSize,
                                                      IRFunc2BeginLine,
                                                      (argc == 5 && std::string(argv[4])=="DEBUG"));
    print_info("Enable HI_WithDirectiveTimingResourceEvaluation Pass");
    PM.add(hi_withdirectivetimingresourceevaluation); 

    print_info("Enable HI_FindFunctions Pass");
    print_info("Enable HI_DependenceList Pass");
    auto hi_findfunction = new HI_FindFunctions();
    PM.add(hi_findfunction);
    auto hi_dependencelist = new HI_DependenceList("Instructions","Instruction_Dep");
    PM.add(hi_dependencelist);

    // std::map<std::string, std::string> IRLoop2LoopLabel;

    // auto hi_ir2sourcecode = new HI_IR2SourceCode("HI_IR2SourceCode",IRLoop2LoopLabel);
    // PM.add(hi_ir2sourcecode);
    // print_info("Enable HI_IR2SourceCode Pass");





    print_status("Start LLVM processing");  
    PM.run(*Mod);
    print_status("Accomplished LLVM processing");

    // for (auto it : IRLoop2LoopLabel)
    // {
    //     llvm::errs() << it.first << " <==== " << it.second << "\n";
    // }

    assert(hi_nodirectivetimingresourceevaluation->topFunctionFound && "The specified top function is not found in the program");

    print_status("Writing LLVM IR to File");
    
    llvm::raw_fd_ostream OS("top_output.bc", EC, llvm::sys::fs::F_None);
    WriteBitcodeToFile(*Mod, OS);
    OS.flush();

    cmd_str = "llvm-dis top_output.bc 2>&1";
    print_cmd(cmd_str.c_str());
    result = sysexec(cmd_str.c_str());
    
    assert(result); // ensure the cmd is executed successfully

    return 0;
}
