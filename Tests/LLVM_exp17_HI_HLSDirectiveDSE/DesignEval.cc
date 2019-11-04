#include "LLVM_exp17_HI_HLSDirectiveDSE.h"
#include "ConfigParse.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "ConfigGen.h"
#include "DesignEval.h"

void modulePreProcessForInformation(
    llvm::Module &Mod,
    std::string top_str,
    std::map<std::string,Info_type_list> &BiOp_Info_name2list_map,
    std::map<std::string, std::vector<int>> &IRFunc2BeginLine,
    std::map<std::string, int> &FuncParamLine2OutermostSize,
    std::map<std::string, std::set<std::pair<std::string, std::pair<std::string, int>> >> &LoopLabel2DrivenArrayDimensions,
    std::map<std::string, std::string> &IRLoop2LoopLabel,
    std::map<std::string, int> &IRLoop2OriginTripCount,
    std::map<std::string, std::vector<std::string>> &LoopIRName2NextLevelSubLoopIRNames,
    std::map<std::string, int> &LoopIRName2Depth,
    std::map<std::string, std::vector< std::pair<std::string, std::string>>> &LoopIRName2Array,
    std::map<std::pair<std::string, std::string>, HI_PragmaArrayInfo> &TargetExtName2ArrayInfo,
    std::map<std::string, std::vector<int>> &Loop2PotentialUnrollFactors,
    bool debugFlag
) 
{
    LLVMTargetRef T;

    char *Error;

    std::error_code EC;

    legacy::PassManager PM_pre,PM_pre_check;

    std::unique_ptr<llvm::Module> Mod_pre = CloneModule(Mod);

    if(LLVMGetTargetFromTriple((Mod_pre->getTargetTriple()).c_str(), &T, &Error))
    {
        print_error(Error);
    }
    else
    {
        std::string targetname = LLVMGetTargetName(T);
        targetname = "The target machine is: " + targetname;
        print_info(targetname.c_str());
    }

    auto loopextract = createLoopExtractorPass(); //"HI_LoopUnroll"
    PM_pre.add(loopextract);

    
    auto hi_ir2sourcecode = new HI_IR2SourceCode("HI_IR2SourceCode",IRLoop2LoopLabel, IRFunc2BeginLine, IRLoop2OriginTripCount, debugFlag);
    PM_pre.add(hi_ir2sourcecode);
    print_info("Enable HI_IR2SourceCode Pass");

    print_info("Enable LoopSimplify Pass");
    auto loopsimplifypass = createLoopSimplifyPass();
    PM_pre.add(loopsimplifypass);

    auto indvarsimplifypass = createIndVarSimplifyPass();
    PM_pre.add(indvarsimplifypass);
    print_info("Enable IndVarSimplifyPass Pass");

    auto hi_mulorderopt = new HI_MulOrderOpt("HI_MulOrderOpt");
    PM_pre.add(hi_mulorderopt);
    print_info("Enable HI_MulOrderOpt Pass");

    auto CFGSimplification_pass22 = createCFGSimplificationPass();
    PM_pre.add(CFGSimplification_pass22);
    print_info("Enable CFGSimplificationPass Pass");

    auto hi_separateconstoffsetfromgep = new HI_SeparateConstOffsetFromGEP("HI_SeparateConstOffsetFromGEP",true, debugFlag);
    PM_pre.add(hi_separateconstoffsetfromgep);
    print_info("Enable HI_SeparateConstOffsetFromGEP Pass");

    auto hi_PragmaTargetExtraction = new HI_PragmaTargetExtraction(top_str.c_str(),
                                                                    IRLoop2LoopLabel, FuncParamLine2OutermostSize, IRFunc2BeginLine, 
                                                                    LoopIRName2NextLevelSubLoopIRNames,
                                                                    LoopIRName2Depth,
                                                                    LoopIRName2Array,
                                                                    TargetExtName2ArrayInfo,
                                                                    debugFlag);

    PM_pre.add(hi_PragmaTargetExtraction);
    print_info("Enable HI_PragmaTargetExtraction Pass");

    print_status("Start LLVM pre-processing");  
    PM_pre.run(*Mod_pre);
    print_status("Accomplished LLVM pre-processing");


    LoopLabel2DrivenArrayDimensions.clear();

    auto hi_ArraySensitiveToLoopLevel = new HI_ArraySensitiveToLoopLevel(
                                                        // configFile_str.c_str(),
                                                        "HI_ArraySensitiveToLoopLevel_eval","HI_ArraySensitiveToLoopLevel_bram",
                                                        "HI_ArraySensitiveToLoopLevel_array",
                                                        top_str.c_str(),
                                                        IRLoop2LoopLabel,
                                                        IRLoop2OriginTripCount,
                                                        FuncParamLine2OutermostSize,
                                                        IRFunc2BeginLine,
                                                        LoopLabel2DrivenArrayDimensions,
                                                        BiOp_Info_name2list_map,
                                                        true);
    PM_pre_check.add(hi_ArraySensitiveToLoopLevel);
    print_info("Enable HI_ArraySensitiveToLoopLevel Pass");
    PM_pre_check.run(*Mod_pre);


    if (debugFlag)
    {
        llvm::raw_fd_ostream OS_PM_pre("top_output_loopextract.bc", EC, llvm::sys::fs::F_None);
        WriteBitcodeToFile(*Mod_pre, OS_PM_pre);
        OS_PM_pre.flush();
    }

    Mod_pre.reset();

    Loop2PotentialUnrollFactors = getPotentialLoopUnrollFactor(
                                            IRLoop2LoopLabel,
                                            LoopIRName2NextLevelSubLoopIRNames,
                                            LoopIRName2Depth,
                                            IRLoop2OriginTripCount
                                        );

}

void modulePreProcessForFPGAHLS_optimization(
    llvm::Module &Mod,
    std::string top_str,
    std::string cntStr,
    std::map<std::string,Info_type_list> &BiOp_Info_name2list_map,
    std::map<std::string, std::vector<int>> &IRFunc2BeginLine,
    std::map<std::string, int> &FuncParamLine2OutermostSize,
    std::map<std::string, std::set<std::pair<std::string, std::pair<std::string, int>> >> &LoopLabel2DrivenArrayDimensions,
    std::map<std::string, std::string> &IRLoop2LoopLabel,
    std::map<std::string, std::string> &IRLoop2LoopLabel_eval,
    std::map<std::string, int> &IRLoop2OriginTripCount,
    std::map<std::string, int> &IRLoop2OriginTripCount_eval,
    std::map<std::string, std::vector<std::string>> &LoopIRName2NextLevelSubLoopIRNames,
    std::map<std::string, int> &LoopIRName2Depth,
    std::map<std::string, std::vector< std::pair<std::string, std::string>>> &LoopIRName2Array,
    std::map<std::pair<std::string, std::string>, HI_PragmaArrayInfo> &TargetExtName2ArrayInfo,
    std::map<std::string, std::vector<int>> &Loop2PotentialUnrollFactors,
    std::map<std::string, int> &LoopLabel2UnrollFactor,
    bool debugFlag
)
{
    LLVMTargetRef T;

    char *Error;

    std::error_code EC;

    legacy::PassManager PM0,PM1,PM2,PM3;

    auto loopextract = createLoopExtractorPass(); //"HI_LoopUnroll"
    PM0.add(loopextract);
    // print_info("Enable LoopExtractor Pass");


    // print_info("Enable LoopSimplify Pass");
    auto loopsimplifypass = createLoopSimplifyPass();
    PM0.add(loopsimplifypass);

    auto indvarsimplifypass = createIndVarSimplifyPass();
    PM0.add(indvarsimplifypass);
    // print_info("Enable IndVarSimplifyPass Pass");

    // PM0.add(createTargetTransformInfoWrapperPass(TargetIRAnalysis()));
    // // print_info("Enable TargetIRAnalysis Pass");


    auto hi_mulorderopt = new HI_MulOrderOpt("HI_MulOrderOpt");
    PM0.add(hi_mulorderopt);
    // print_info("Enable HI_MulOrderOpt Pass");

    auto CFGSimplification_pass22 = createCFGSimplificationPass();
    PM0.add(CFGSimplification_pass22);
    // print_info("Enable CFGSimplificationPass Pass");



    auto hi_separateconstoffsetfromgep = new HI_SeparateConstOffsetFromGEP("HI_SeparateConstOffsetFromGEP",true, debugFlag);
    PM0.add(hi_separateconstoffsetfromgep);
    // print_info("Enable HI_SeparateConstOffsetFromGEP Pass");


    // print_info("Enable HI_ArraySensitiveToLoopLevel Pass");

    auto hi_loopunroll = new HI_LoopUnroll(IRLoop2LoopLabel, LoopLabel2UnrollFactor, 1, false, None); //"HI_LoopUnroll"
    PM0.add(hi_loopunroll);
    // print_info("Enable HI_LoopUnroll Pass");

    




    auto hi_mul2shl = new HI_Mul2Shl("HI_Mul2Shl", debugFlag);
    PM0.add(hi_mul2shl);
    // print_info("Enable HI_Mul2Shl Pass");



    auto loopstrengthreducepass = createLoopStrengthReducePass();
    PM0.add(loopstrengthreducepass);
    // print_info("Enable LoopStrengthReducePass Pass");

    auto hi_aggressivelsr_mul = new HI_AggressiveLSR_MUL("AggressiveLSR", debugFlag);
    PM0.add(hi_aggressivelsr_mul);
    // print_info("Enable HI_AggressiveLSR_MUL Pass");

    PM0.run(Mod);

    // for (auto label_function_array_dim : LoopLabel2DrivenArrayDimensions)
    // {
    //     llvm::errs() << "Loop: [" << label_function_array_dim.first << "] drives the partitioning of the array dimensions: \n    ";
    //     for (auto function_array_dim : label_function_array_dim.second)
    //     {
    //     llvm::errs() << " " << "{F[" << function_array_dim.first << "], A[" << function_array_dim.second.first << "], D[" << function_array_dim.second.second << "]},  ";
    //     }
    //     llvm::errs() << "\n";
    // }

    if (debugFlag)
    {
        llvm::raw_fd_ostream OSPM0("top_output_PM0.bc", EC, llvm::sys::fs::F_None);
        WriteBitcodeToFile(Mod, OSPM0);
        OSPM0.flush();
    }

    // don't remove chained operations
    auto hi_hlsduplicateinstrm = new HI_HLSDuplicateInstRm("HLSrmInsts", debugFlag);
    PM1.add(hi_hlsduplicateinstrm);
    // print_info("Enable HI_HLSDuplicateInstRm Pass");


    auto CFGSimplification_pass = createCFGSimplificationPass();
    PM1.add(CFGSimplification_pass);
    // print_info("Enable CFGSimplificationPass Pass");
    
    

    auto hi_functioninstantiation = new HI_FunctionInstantiation("HI_FunctionInstantiation",top_str);
    PM1.add(hi_functioninstantiation);
    // print_info("Enable HI_FunctionInstantiation Pass");

    auto hi_replaceselectaccess = new HI_ReplaceSelectAccess("HI_ReplaceSelectAccess",debugFlag);
    PM1.add(hi_replaceselectaccess);
    // print_info("Enable HI_ReplaceSelectAccess Pass");

    auto lowerswitch_pass = createLowerSwitchPass();
    PM1.add(lowerswitch_pass);
    // print_info("Enable LowerSwitchPass Pass");
    
    auto ADCE_pass = createAggressiveDCEPass();
    PM1.add(ADCE_pass);
    // print_info("Enable AggressiveDCEPass Pass");

    auto CFGSimplification_pass1 = createCFGSimplificationPass();
    PM1.add(CFGSimplification_pass1);
    // print_info("Enable CFGSimplificationPass Pass");

    PM1.run(Mod);

    if (debugFlag)
    {
        llvm::raw_fd_ostream OSPM1("top_output_PM1.bc", EC, llvm::sys::fs::F_None);
        WriteBitcodeToFile(Mod, OSPM1);
        OSPM1.flush();
    }


    std::string logName_varwidthreduce = "VarWidth__forCheck_"+cntStr ;
    auto hi_varwidthreduce1 = new HI_VarWidthReduce(logName_varwidthreduce.c_str(), debugFlag);
    PM2.add(hi_varwidthreduce1);
    // print_info("Enable HI_VarWidthReduce Pass");

    // don't remove chained operations
    auto hi_hlsduplicateinstrm1 = new HI_HLSDuplicateInstRm("HLSrmInsts1", (debugFlag));
    PM2.add(hi_hlsduplicateinstrm1);
    // print_info("Enable HI_HLSDuplicateInstRm Pass");

    auto CFGSimplification_pass2 = createCFGSimplificationPass();
    PM2.add(CFGSimplification_pass2);
    // print_info("Enable CFGSimplificationPass Pass");


    auto hi_removeredundantaccessPM2 = new HI_RemoveRedundantAccess("HI_RemoveRedundantAccessPM2", top_str,(debugFlag));
    PM2.add(hi_removeredundantaccessPM2);
    // print_info("Enable HI_RemoveRedundantAccess Pass");

    auto hi_intstructionmovebackward1 = new HI_IntstructionMoveBackward("HI_IntstructionMoveBackward1", (debugFlag));
    PM2.add(hi_intstructionmovebackward1);
    // print_info("Enable HI_IntstructionMoveBackward Pass");

    auto hi_removeredundantaccessPM2_2 = new HI_RemoveRedundantAccess("HI_RemoveRedundantAccessPM2_2", top_str,(debugFlag));
    PM2.add(hi_removeredundantaccessPM2_2);
    // print_info("Enable HI_RemoveRedundantAccess Pass");

    auto HI_LoadALAPPM2 = new HI_LoadALAP("HI_LoadALAP", debugFlag);
    PM2.add(HI_LoadALAPPM2);
    // print_info("Enable HI_LoadALAP Pass");


    PM2.run(Mod);

    if (debugFlag)
    {
        llvm::raw_fd_ostream OSPM2("top_output_PM2.bc", EC, llvm::sys::fs::F_None);
        WriteBitcodeToFile(Mod, OSPM2);
        OSPM2.flush();
    }
    
    // std::map<std::string, std::vector<int>> IRFunc2BeginLine_eval;
    auto hi_ir2sourcecode_eval = new HI_IR2SourceCode("HI_IR2SourceCode_eval",IRLoop2LoopLabel_eval, IRFunc2BeginLine, IRLoop2OriginTripCount_eval, debugFlag);
    PM3.add(hi_ir2sourcecode_eval);
    // print_info("Enable HI_IR2SourceCode Pass");
    PM3.run(Mod);
}


void modulePreProcessForPreEvaluation(
    llvm::Module &Mod,
    std::string top_str,
    std::string cntStr,
    HI_DesignConfigInfo &desginconfig,
    std::map<std::string,Info_type_list> &BiOp_Info_name2list_map,
    std::map<std::string, std::vector<int>> &IRFunc2BeginLine,
    std::map<std::string, int> &FuncParamLine2OutermostSize,
    std::map<std::string, std::set<std::pair<std::string, std::pair<std::string, int>> >> &LoopLabel2DrivenArrayDimensions,
    std::map<std::string, std::string> &IRLoop2LoopLabel,
    std::map<std::string, std::string> &IRLoop2LoopLabel_eval,
    std::map<std::string, int> &IRLoop2OriginTripCount,
    std::map<std::string, int> &IRLoop2OriginTripCount_eval,
    std::map<std::string, std::vector<std::string>> &LoopIRName2NextLevelSubLoopIRNames,
    std::map<std::string, int> &LoopIRName2Depth,
    std::map<std::string, std::vector< std::pair<std::string, std::string>>> &LoopIRName2Array,
    std::map<std::pair<std::string, std::string>, HI_PragmaArrayInfo> &TargetExtName2ArrayInfo,
    std::map<std::string, std::vector<int>> &Loop2PotentialUnrollFactors,
    std::map<std::string, int> &LoopLabel2UnrollFactor,
    bool &muxWithMoreThan32,
    std::string &muxTooMuchLoopIRName,
    bool debugFlag
)
{
    legacy::PassManager PM4;

    auto loopinfowrapperpass = new LoopInfoWrapperPass();
    PM4.add(loopinfowrapperpass);
    // print_info("Enable LoopInfoWrapperPass Pass");


    auto scalarevolutionwrapperpass = new ScalarEvolutionWrapperPass();
    PM4.add(scalarevolutionwrapperpass);
    // print_info("Enable ScalarEvolutionWrapperPass Pass");

    auto hi_MuxInsertionArrayPartition = new HI_MuxInsertionArrayPartition(
                                                    // configFile_str.c_str(),
                                                    desginconfig,
                                                    top_str.c_str(),
                                                    FuncParamLine2OutermostSize,
                                                    IRFunc2BeginLine,
                                                    debugFlag);
    // print_info("Enable HI_MuxInsertionArrayPartition Pass");
    PM4.add(hi_MuxInsertionArrayPartition); 

    auto lowerswitch_pass = createLowerSwitchPass();
    PM4.add(lowerswitch_pass);
    // print_info("Enable LowerSwitchPass Pass");
    
    auto ADCE_pass = createAggressiveDCEPass();
    PM4.add(ADCE_pass);
    // print_info("Enable AggressiveDCEPass Pass");

    auto CFGSimplification_pass1 = createCFGSimplificationPass();
    PM4.add(CFGSimplification_pass1);


    print_status("Start LLVM processing");  
    PM4.run(Mod);
    print_status("Accomplished LLVM processing");
    std::error_code EC;
    if (debugFlag)
    {
        llvm::raw_fd_ostream OSPM2("top_output_PM4.bc", EC, llvm::sys::fs::F_None);
        WriteBitcodeToFile(Mod, OSPM2);
        OSPM2.flush();
    }
    muxWithMoreThan32 = hi_MuxInsertionArrayPartition->muxWithMoreThan32;

    muxTooMuchLoopIRName = hi_MuxInsertionArrayPartition->muxTooMuchLoopIRName;

}