#include "LLVM_exp15_HI_HighAccurateFastEvaluationWithPragma.h"
#include "ConfigParse.h"
#include "llvm/Transforms/Utils/Cloning.h"

using namespace llvm;
using namespace polly;

using namespace clang;
using namespace clang::driver;
using namespace clang::tooling;

std::string HLS_lib_path;
std:: string clock_period_str;


int main(int argc, const char **argv) 
{
    if (argc < 4) 
    {
        errs() << "Usage: " << argv[0] << " <C/C++ file> <Top_Function_Name> <Config_File_Path>\n";
        return 1;
    }
    std::error_code EC;

    std::string top_str = std::string(argv[2]);
    std::string configFile_str = std::string(argv[3]);    
    bool debugFlag = (argc == 5 && std::string(argv[4])=="DEBUG");

    // create a new Clang Tool instance (a LibTooling environment)
    std::map<std::string, int> FuncParamLine2OutermostSize;
    clangPreProcess(argv, top_str, FuncParamLine2OutermostSize);

    // Compile the source code into IR and Parse the input LLVM IR file into a module
    std::string cmd_str = "clang -O1 -emit-llvm -S -g tmp_loopLabeled.cc -o top_notinline.bc 2>&1";
    print_cmd(cmd_str.c_str());
    bool result = sysexec(cmd_str.c_str());
    assert(result); // ensure the cmd is executed successfully
    cmd_str = "opt -always-inline top_notinline.bc -o top.bc 2>&1";
    print_cmd(cmd_str.c_str());
    result = sysexec(cmd_str.c_str());
    assert(result); // ensure the cmd is executed successfully

  // system(cmd_str.c_str());

    SMDiagnostic Err;
    LLVMContext Context;
    LLVMInitializeX86TargetInfo();
    LLVMInitializeX86Target();
    LLVMInitializeX86TargetMC();
    std::unique_ptr<llvm::Module> Mod(parseIRFile("top.bc", Err, Context));
    if (!Mod) 
    {
        Err.print(argv[0], errs());
        return 1;
    }

    std::map<std::string,Info_type_list> BiOp_Info_name2list_map;
    DES_Load_Instruction_Info(configFile_str.c_str(), BiOp_Info_name2list_map);

    for (auto pair:BiOp_Info_name2list_map)
    {
        llvm::errs() << "instruction type: [" << pair.first << "] loaded\n";
    }
    

    // you can set different configuration for different iteration
    for (int i=0; i<1;i++)
    {
        std::string cntStr = std::to_string(i);
        std::unique_ptr<llvm::Module> Mod_tmp = CloneModule(*Mod);

        // Create a pass manager and fill it with the passes we want to run.
        legacy::PassManager PM_pre, PM0, PM1, PM2, PM3, PM4, PM_eval;
        
        LLVMTargetRef T;
        ModulePassManager MPM;

        char *Error;

        if(LLVMGetTargetFromTriple((Mod_tmp->getTargetTriple()).c_str(), &T, &Error))
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
        std::map<std::string, int> LoopLabel2UnrollFactor;
        std::map<std::string, int> LoopLabel2II;
        std::map<std::string, int> IRLoop2OriginTripCount;
        std::map<std::string, int> IRLoop2OriginTripCount_eval;

        Parse_Config(configFile_str.c_str(), LoopLabel2UnrollFactor, LoopLabel2II);






        auto loopextract = createLoopExtractorPass(); //"HI_LoopUnroll"
        PM_pre.add(loopextract);
        print_info("Enable LoopExtractor Pass");

        std::map<std::string, std::vector<int>> IRFunc2BeginLine;
        auto hi_ir2sourcecode = new HI_IR2SourceCode("HI_IR2SourceCode",IRLoop2LoopLabel, IRFunc2BeginLine, IRLoop2OriginTripCount, debugFlag);
        PM_pre.add(hi_ir2sourcecode);
        print_info("Enable HI_IR2SourceCode Pass");

        auto hi_PragmaTargetExtraction = new HI_PragmaTargetExtraction(top_str.c_str(),
                                                                IRLoop2LoopLabel, FuncParamLine2OutermostSize, IRFunc2BeginLine, debugFlag);
        PM_pre.add(hi_PragmaTargetExtraction);
        print_info("Enable HI_PragmaTargetExtraction Pass");

        print_status("Start LLVM pre-processing");  
        PM_pre.run(*Mod_tmp);
        print_status("Accomplished LLVM pre-processing");

        if (debugFlag)
        {
            llvm::raw_fd_ostream OS3("top_output_loopextract.bc", EC, llvm::sys::fs::F_None);
            WriteBitcodeToFile(*Mod_tmp, OS3);
            OS3.flush();
        }






        Triple ModuleTriple(Mod_tmp->getTargetTriple());
        TargetLibraryInfoImpl TLII(ModuleTriple);
        PM0.add(new TargetLibraryInfoWrapperPass(TLII));
        

        print_info("Enable LoopSimplify Pass");
        auto loopsimplifypass = createLoopSimplifyPass();
        PM0.add(loopsimplifypass);

        auto indvarsimplifypass = createIndVarSimplifyPass();
        PM0.add(indvarsimplifypass);
        print_info("Enable IndVarSimplifyPass Pass");

        PM0.add(createTargetTransformInfoWrapperPass(TargetIRAnalysis()));
        print_info("Enable TargetIRAnalysis Pass");


        auto hi_mulorderopt = new HI_MulOrderOpt("HI_MulOrderOpt");
        PM0.add(hi_mulorderopt);
        print_info("Enable HI_MulOrderOpt Pass");

        auto CFGSimplification_pass22 = createCFGSimplificationPass();
        PM0.add(CFGSimplification_pass22);        
        print_info("Enable CFGSimplificationPass Pass");
        
        auto hi_separateconstoffsetfromgep = new HI_SeparateConstOffsetFromGEP("HI_SeparateConstOffsetFromGEP",true, debugFlag);
        PM0.add(hi_separateconstoffsetfromgep);
        print_info("Enable HI_SeparateConstOffsetFromGEP Pass");

        auto hi_loopunroll = new HI_LoopUnroll(IRLoop2LoopLabel, LoopLabel2UnrollFactor, 1, false, None); //"HI_LoopUnroll"
        PM0.add(hi_loopunroll);
        print_info("Enable HI_LoopUnroll Pass");

        auto hi_mul2shl = new HI_Mul2Shl("HI_Mul2Shl", debugFlag);
        PM0.add(hi_mul2shl);
        print_info("Enable HI_Mul2Shl Pass");
        
        auto loopstrengthreducepass = createLoopStrengthReducePass();
        PM0.add(loopstrengthreducepass);
        print_info("Enable LoopStrengthReducePass Pass");


        auto hi_aggressivelsr_mul = new HI_AggressiveLSR_MUL("AggressiveLSR", debugFlag);
        PM0.add(hi_aggressivelsr_mul);
        print_info("Enable HI_AggressiveLSR_MUL Pass");


        PM0.run(*Mod_tmp);

        if (debugFlag)
        {
          llvm::raw_fd_ostream OSPM0("top_output_PM0.bc", EC, llvm::sys::fs::F_None);
          WriteBitcodeToFile(*Mod_tmp, OSPM0);
          OSPM0.flush();
        }





        // don't remove chained operations
        auto hi_hlsduplicateinstrm = new HI_HLSDuplicateInstRm("HLSrmInsts", debugFlag);
        PM1.add(hi_hlsduplicateinstrm);
        print_info("Enable HI_HLSDuplicateInstRm Pass");

        auto CFGSimplification_pass_PM1 = createCFGSimplificationPass();
        PM1.add(CFGSimplification_pass_PM1);
        print_info("Enable CFGSimplificationPass Pass");

        auto hi_functioninstantiation = new HI_FunctionInstantiation("HI_FunctionInstantiation",top_str);
        PM1.add(hi_functioninstantiation);
        print_info("Enable HI_FunctionInstantiation Pass");

        auto hi_replaceselectaccess = new HI_ReplaceSelectAccess("HI_ReplaceSelectAccess",debugFlag);
        PM1.add(hi_replaceselectaccess);
        print_info("Enable HI_ReplaceSelectAccess Pass");

        auto lowerswitch_pass = createLowerSwitchPass();
        PM1.add(lowerswitch_pass);
        print_info("Enable LowerSwitchPass Pass");
        
        auto ADCE_pass = createAggressiveDCEPass();
        PM1.add(ADCE_pass);
        print_info("Enable AggressiveDCEPass Pass");

        auto CFGSimplification_pass1 = createCFGSimplificationPass();
        PM1.add(CFGSimplification_pass1);
        print_info("Enable CFGSimplificationPass Pass");


        PM1.run(*Mod_tmp);

        if (debugFlag)
        {
          llvm::raw_fd_ostream OS111("top_output_PM1.bc", EC, llvm::sys::fs::F_None);
          WriteBitcodeToFile(*Mod_tmp, OS111);
          OS111.flush();
        }








        std::string logName_varwidthreduce = "VarWidth__forCheck_"+cntStr ;
        auto hi_varwidthreduce1 = new HI_VarWidthReduce(logName_varwidthreduce.c_str(), debugFlag);
        PM2.add(hi_varwidthreduce1);
        print_info("Enable HI_VarWidthReduce Pass");

        // don't remove chained operations
        auto hi_hlsduplicateinstrm1 = new HI_HLSDuplicateInstRm("HLSrmInsts1", (debugFlag));
        PM2.add(hi_hlsduplicateinstrm1);
        print_info("Enable HI_HLSDuplicateInstRm Pass");

        auto CFGSimplification_pass2 = createCFGSimplificationPass();
        PM2.add(CFGSimplification_pass2);
        print_info("Enable CFGSimplificationPass Pass");


        auto hi_removeredundantaccessPM2 = new HI_RemoveRedundantAccess("HI_RemoveRedundantAccessPM2", top_str,(debugFlag));
        PM2.add(hi_removeredundantaccessPM2);
        print_info("Enable HI_RemoveRedundantAccess Pass");

        auto hi_intstructionmovebackward1 = new HI_IntstructionMoveBackward("HI_IntstructionMoveBackward1", (debugFlag));
        PM2.add(hi_intstructionmovebackward1);
        print_info("Enable HI_IntstructionMoveBackward Pass");

        auto hi_removeredundantaccessPM2_2 = new HI_RemoveRedundantAccess("HI_RemoveRedundantAccessPM2_2", top_str,(debugFlag));
        PM2.add(hi_removeredundantaccessPM2_2);
        print_info("Enable HI_RemoveRedundantAccess Pass");

        auto HI_LoadALAPPM2 = new HI_LoadALAP("HI_LoadALAP", debugFlag);
        PM2.add(HI_LoadALAPPM2);
        print_info("Enable HI_LoadALAP Pass");
        

        PM2.run(*Mod_tmp);

        if (debugFlag)
        {
          llvm::raw_fd_ostream OSPM2("top_output_PM2.bc", EC, llvm::sys::fs::F_None);
          WriteBitcodeToFile(*Mod_tmp, OSPM2);
          OSPM2.flush();
        }





        





        std::map<std::string, std::string> IRLoop2LoopLabel_eval;
        // std::map<std::string, std::vector<int>> IRFunc2BeginLine_eval;
        auto hi_ir2sourcecode_eval = new HI_IR2SourceCode("HI_IR2SourceCode_eval",IRLoop2LoopLabel_eval, IRFunc2BeginLine, IRLoop2OriginTripCount_eval, debugFlag);
        PM3.add(hi_ir2sourcecode_eval);
        print_info("Enable HI_IR2SourceCode Pass");
        PM3.run(*Mod_tmp);



        auto loopinfowrapperpass = new LoopInfoWrapperPass();
        PM4.add(loopinfowrapperpass);
        print_info("Enable LoopInfoWrapperPass Pass");

        auto scalarevolutionwrapperpass = new ScalarEvolutionWrapperPass();
        PM4.add(scalarevolutionwrapperpass);
        print_info("Enable ScalarEvolutionWrapperPass Pass");


        auto hi_MuxInsertionArrayPartition = new HI_MuxInsertionArrayPartition(
                                                          configFile_str.c_str(),
                                                          top_str.c_str(),
                                                          FuncParamLine2OutermostSize,
                                                          IRFunc2BeginLine,
                                                          debugFlag);
        print_info("Enable HI_MuxInsertionArrayPartition Pass");
        PM4.add(hi_MuxInsertionArrayPartition); 

    
        auto lowerswitch_pass_eval = createLowerSwitchPass();
        PM4.add(lowerswitch_pass_eval);
        // print_info("Enable LowerSwitchPass Pass");

        auto ADCE_pass_eval = createAggressiveDCEPass();
        PM4.add(ADCE_pass_eval);
        // print_info("Enable AggressiveDCEPass Pass");

        auto CFGSimplification_pass1_eval = createCFGSimplificationPass();
        PM4.add(CFGSimplification_pass1_eval);
        print_status("Start LLVM processing");  
        PM4.run(*Mod_tmp);
        print_status("Accomplished LLVM processing");
        
        if (debugFlag)
        {
            llvm::raw_fd_ostream OSPM2("top_output_PM4.bc", EC, llvm::sys::fs::F_None);
            WriteBitcodeToFile(*Mod_tmp, OSPM2);
            OSPM2.flush();
        }


        std::string logName_evaluation = "HI_WithDirectiveTimingResourceEvaluation__forCheck_"+cntStr ;
        std::string logName_array = "ArrayLog__forCheck_"+cntStr ;
        auto hi_withdirectivetimingresourceevaluation = new HI_WithDirectiveTimingResourceEvaluation(
                                                          configFile_str.c_str(),
                                                          logName_evaluation.c_str(),"BRAM_info_0",
                                                          logName_array.c_str(),
                                                          top_str.c_str(),
                                                          IRLoop2LoopLabel_eval,
                                                          IRLoop2OriginTripCount,
                                                          LoopLabel2II,
                                                          LoopLabel2UnrollFactor,
                                                          FuncParamLine2OutermostSize,
                                                          IRFunc2BeginLine,
                                                          BiOp_Info_name2list_map,
                                                          debugFlag);
        print_info("Enable HI_WithDirectiveTimingResourceEvaluation Pass");
        PM_eval.add(hi_withdirectivetimingresourceevaluation);
        
        PM_eval.run(*Mod_tmp);



        assert(hi_withdirectivetimingresourceevaluation->topFunctionFound && "The specified top function is not found in the program");

        print_status("Writing LLVM IR to File");
        

        if (debugFlag)
        {
            llvm::raw_fd_ostream OS("top_output.bc", EC, llvm::sys::fs::F_None);
            WriteBitcodeToFile(*Mod_tmp, OS);
            OS.flush();
          


            cmd_str = "llvm-dis top_output.bc 2>&1";
            print_cmd(cmd_str.c_str());
            result = sysexec(cmd_str.c_str());
        }
        
        assert(result); // ensure the cmd is executed successfully
        Mod_tmp.reset();
    }



    return 0;
}
