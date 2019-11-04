#include "LLVM_exp16_HI_SingleModuleDSE.h"
#include "ConfigParse.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "ConfigGen.h"

using namespace llvm;

std:: string clock_period_str;
std:: string  HLS_lib_path;




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


    bool debugFlag = (argc == 5 && std::string(argv[4])=="DEBUG");
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
    std::string cmd_str = "clang -O1 -emit-llvm -S -g tmp1.cc -o top_notinline.bc 2>&1";

    print_cmd(cmd_str.c_str());
    bool result = sysexec(cmd_str.c_str());
    assert(result); // ensure the cmd is executed successfully


    cmd_str = "opt -always-inline top_notinline.bc -o top.bc 2>&1";
    print_cmd(cmd_str.c_str());
    result = sysexec(cmd_str.c_str());
    assert(result); // ensure the cmd is executed successfully

  // system(cmd_str.c_str());

    std::unique_ptr<llvm::Module> Mod(parseIRFile("top.bc", Err, Context));
    if (!Mod) {
      Err.print(argv[0], errs());
      return 1;
    }




    //////////////////////////////////////////////////////////////////////////////////////
    //////////////////////    pre-processing of the IR module ////////////////////////////  
    //////////////////////////////////////////////////////////////////////////////////////




    std::map<std::string,Info_type_list> BiOp_Info_name2list_map;
    DES_Load_Instruction_Info(configFile_str.c_str(), BiOp_Info_name2list_map);
        // record the specific level of loop is sensitive to specific partitioning scheme of the specific dimension of array
    std::map<std::string, std::set<std::pair<std::string, std::pair<std::string, int>> >> LoopLabel2DrivenArrayDimensions;


    // desginconfig_0.setClock("6.0");
    // desginconfig_0.HLS_lib_path = "../../../HLS_Data_Lib/";
    // desginconfig_0.insertLoopPipeline("Loop_kernel_2mm_6",1);
    // desginconfig_0.insertLoopUnroll("Loop_kernel_2mm_5",2);
    // desginconfig_0.insertLoopUnroll("Loop_kernel_2mm_6",18);
    // desginconfig_0.insertArrayCyclicPartition("kernel_2mm", "tmp",1, 18);
    // desginconfig_0.insertArrayCyclicPartition("kernel_2mm", "D",1, 2);
    // desginconfig_0.insertArrayCyclicPartition("kernel_2mm", "C",2, 18);

    std::unique_ptr<llvm::Module> Mod_pre = CloneModule(*Mod);

    // Create a pass manager and fill it with the passes we want to run.
    legacy::PassManager PM_pre, PM_pre_check, PM0, PM1, PM2, PM3, PM4, PM5;
    
    LLVMTargetRef T;
    ModulePassManager MPM;

    char *Error;

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

    std::map<std::string, std::string> IRLoop2LoopLabel;
    std::map<std::string, int> IRLoop2OriginTripCount;


    auto loopextract = createLoopExtractorPass(); //"HI_LoopUnroll"
    PM_pre.add(loopextract);
    print_info("Enable LoopExtractor Pass");


    std::map<std::string, std::vector<int>> IRFunc2BeginLine;
    auto hi_ir2sourcecode = new HI_IR2SourceCode("HI_IR2SourceCode",IRLoop2LoopLabel, IRFunc2BeginLine, IRLoop2OriginTripCount, debugFlag);
    PM_pre.add(hi_ir2sourcecode);
    print_info("Enable HI_IR2SourceCode Pass");


    

    std::map<std::string, std::vector<std::string>> LoopIRName2NextLevelSubLoopIRNames;
    std::map<std::string, int> LoopIRName2Depth;
    std::map<std::string, std::vector< std::pair<std::string, std::string>>> LoopIRName2Array;
    std::map<std::pair<std::string, std::string>, HI_PragmaArrayInfo> TargetExtName2ArrayInfo;


    print_info("Enable LoopSimplify Pass");
    auto loopsimplifypass = createLoopSimplifyPass();
    PM_pre.add(loopsimplifypass);

    auto indvarsimplifypass = createIndVarSimplifyPass();
    PM_pre.add(indvarsimplifypass);
    print_info("Enable IndVarSimplifyPass Pass");

    // PM_pre.add(createTargetTransformInfoWrapperPass(TargetIRAnalysis()));
    // print_info("Enable TargetIRAnalysis Pass");


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
                                                        debugFlag);
    PM_pre_check.add(hi_ArraySensitiveToLoopLevel);
    print_info("Enable HI_ArraySensitiveToLoopLevel Pass");
    PM_pre_check.run(*Mod_pre);
    enumerateDesignConfiguration( "arrayConfigs",
                                IRLoop2LoopLabel, 
                                IRLoop2OriginTripCount,
                                LoopIRName2NextLevelSubLoopIRNames,
                                LoopIRName2Depth,
                                LoopIRName2Array,
                                TargetExtName2ArrayInfo);

    if (debugFlag)
    {
        llvm::raw_fd_ostream OS_PM_pre("top_output_loopextract.bc", EC, llvm::sys::fs::F_None);
        WriteBitcodeToFile(*Mod_pre, OS_PM_pre);
        OS_PM_pre.flush();
    }

    Mod_pre.reset();





    std::map<std::string, std::vector<int>> Loop2PotentialUnrollFactors = getPotentialLoopUnrollFactor(
                                            IRLoop2LoopLabel,
                                            LoopIRName2NextLevelSubLoopIRNames,
                                            LoopIRName2Depth,
                                            IRLoop2OriginTripCount
                                        );


    //////////////////////////////////////////////////////////////////////////////////////
    //////////////////////    iterative DSE for the module    ////////////////////////////  
    //////////////////////////////////////////////////////////////////////////////////////
    HI_DesignConfigInfo desginconfig_0;
    Parse_Config(configFile_str.c_str(), desginconfig_0);
        
    std::string slowestTopLoop = "";
    for (int i=0; i<1; i++)
    {
        legacy::PassManager PM0, PM1, PM2, PM3, PM4, PM5;
        
        LLVMTargetRef T;
        ModulePassManager MPM;

        std::unique_ptr<llvm::Module> Mod_tmp = CloneModule(*Mod);
        std::string cntStr = std::to_string(i);


        std::map<std::string, int> LoopLabel2UnrollFactor;
        std::map<std::string, int> LoopLabel2II;
        std::map<std::string, int> IRLoop2OriginTripCount_eval;
    
        for (auto loopUnroll_pair : desginconfig_0.loopUnrollConfigs)
        {
            LoopLabel2UnrollFactor[loopUnroll_pair.first] = loopUnroll_pair.second;
        }

        for (auto loopPipeline_pair : desginconfig_0.loopPipelineConfigs)
        {
            LoopLabel2II[loopPipeline_pair.first] = loopPipeline_pair.second;
        }


        auto loopextract = createLoopExtractorPass(); //"HI_LoopUnroll"
        PM0.add(loopextract);
        print_info("Enable LoopExtractor Pass");


        print_info("Enable LoopSimplify Pass");
        auto loopsimplifypass = createLoopSimplifyPass();
        PM0.add(loopsimplifypass);

        auto indvarsimplifypass = createIndVarSimplifyPass();
        PM0.add(indvarsimplifypass);
        print_info("Enable IndVarSimplifyPass Pass");

        // PM0.add(createTargetTransformInfoWrapperPass(TargetIRAnalysis()));
        // print_info("Enable TargetIRAnalysis Pass");


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

        for (auto label_function_array_dim : LoopLabel2DrivenArrayDimensions)
        {
            llvm::errs() << "Loop: [" << label_function_array_dim.first << "] drives the partitioning of the array dimensions: \n    ";
            for (auto function_array_dim : label_function_array_dim.second)
            {
            llvm::errs() << " " << "{F[" << function_array_dim.first << "], A[" << function_array_dim.second.first << "], D[" << function_array_dim.second.second << "]},  ";
            }
            llvm::errs() << "\n";
        }

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


        auto CFGSimplification_pass = createCFGSimplificationPass();
        PM1.add(CFGSimplification_pass);
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
            llvm::raw_fd_ostream OSPM1("top_output_PM1.bc", EC, llvm::sys::fs::F_None);
            WriteBitcodeToFile(*Mod_tmp, OSPM1);
            OSPM1.flush();
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
                                                        // configFile_str.c_str(),
                                                        desginconfig_0,
                                                        top_str.c_str(),
                                                        FuncParamLine2OutermostSize,
                                                        IRFunc2BeginLine,
                                                        debugFlag);
        print_info("Enable HI_MuxInsertionArrayPartition Pass");
        PM4.add(hi_MuxInsertionArrayPartition); 

        print_status("Start LLVM processing");  
        PM4.run(*Mod_tmp);
        print_status("Accomplished LLVM processing");


        std::string slow_LoopIRName = "";

        if (hi_MuxInsertionArrayPartition->muxWithMoreThan32)
        {
            print_warning("Too larget mux detected. Non-optimal, bypass evaluation.");
            slow_LoopIRName = hi_MuxInsertionArrayPartition->muxTooMuchLoopIRName;
            std::string opt_LoopLabel = IRLoop2LoopLabel[slow_LoopIRName];
    
            desginconfig_0.eraseLoopPipeline(opt_LoopLabel);
            
            int next_factor = findNextUnrollFactor(slow_LoopIRName, LoopLabel2UnrollFactor[opt_LoopLabel] ,Loop2PotentialUnrollFactors);
            llvm::errs() << "suggested next loop unroll factor for it is :" << next_factor << "\n";
            desginconfig_0.insertLoopUnroll(opt_LoopLabel, next_factor);
            
            for (auto loop_array_pair : LoopLabel2DrivenArrayDimensions[opt_LoopLabel])
            {
                std::string funcStr = loop_array_pair.first;
                std::string arrayStr = loop_array_pair.second.first;
                int dim = loop_array_pair.second.second;
                // std::map<std::string, std::set<std::pair<std::string, std::pair<std::string, int>> >>
                desginconfig_0.insertArrayCyclicPartition( demangeFunctionName(funcStr), arrayStr, dim, next_factor);
                
            }
        }
        else
        {
            std::string logName_evaluation = "HI_WithDirectiveTimingResourceEvaluation__forCheck_"+cntStr ;
            std::string logName_array = "ArrayLog__forCheck_"+cntStr ;

            auto hi_withdirectivetimingresourceevaluation = new HI_WithDirectiveTimingResourceEvaluation(
                                                                // configFile_str.c_str(),
                                                                desginconfig_0,
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
                                                                debugFlag
                                                                );
            print_info("Enable HI_WithDirectiveTimingResourceEvaluation Pass");
            PM5.add(hi_withdirectivetimingresourceevaluation); 

            print_status("Start LLVM processing");  
            PM5.run(*Mod_tmp);
            print_status("Accomplished LLVM processing");

            for (auto it : IRLoop2LoopLabel)
            {
                if (hi_withdirectivetimingresourceevaluation->LoopLabel2Latency.find( it.second) != hi_withdirectivetimingresourceevaluation->LoopLabel2Latency.end())
                {
                    llvm::errs() << it.second << " ====> " << hi_withdirectivetimingresourceevaluation->LoopLabel2Latency[it.second] << "\n";
                }
                
            }

            assert(hi_withdirectivetimingresourceevaluation->topFunctionFound && "The specified top function is not found in the program");

            print_status("Writing LLVM IR to File");
            

            if (debugFlag)
            {
                llvm::raw_fd_ostream OSPM4("top_output.bc", EC, llvm::sys::fs::F_None);
                WriteBitcodeToFile(*Mod_tmp, OSPM4);
                OSPM4.flush();
                cmd_str = "llvm-dis top_output.bc 2>&1";
                print_cmd(cmd_str.c_str());
                result = sysexec(cmd_str.c_str());
            }
            
            assert(result); // ensure the cmd is executed successfully

            slow_LoopIRName = findSlowestLastLevelLoop_InSlowestTopLoop(IRLoop2LoopLabel,
                                                    hi_withdirectivetimingresourceevaluation->LoopLabel2Latency, 
                                                    LoopIRName2NextLevelSubLoopIRNames,
                                                    LoopIRName2Depth
                );

            std::string opt_LoopLabel = IRLoop2LoopLabel[slow_LoopIRName];
            llvm::errs() << "loop to optimized is :" << opt_LoopLabel << "\n";


            if (LoopLabel2II.find(opt_LoopLabel) == LoopLabel2II.end())
            {
                llvm::errs() << "suggested start loop pipeline II for it is :" << hi_withdirectivetimingresourceevaluation->LoopLabel2IterationLatency[opt_LoopLabel]-1 << "\n";
                desginconfig_0.insertLoopPipeline(opt_LoopLabel, hi_withdirectivetimingresourceevaluation->LoopLabel2IterationLatency[opt_LoopLabel]-1);
            }
            else
            {
                if (LoopLabel2II[opt_LoopLabel] != hi_withdirectivetimingresourceevaluation->LoopLabel2SmallestII[opt_LoopLabel])
                {
                    llvm::errs() << "suggested next loop pipeline II for it is :" << (LoopLabel2II[opt_LoopLabel] + hi_withdirectivetimingresourceevaluation->LoopLabel2SmallestII[opt_LoopLabel])/2 << "\n";
                    desginconfig_0.insertLoopPipeline(opt_LoopLabel, (LoopLabel2II[opt_LoopLabel] + hi_withdirectivetimingresourceevaluation->LoopLabel2SmallestII[opt_LoopLabel])/2);
                    continue;
                }
                else
                {
                    desginconfig_0.eraseLoopPipeline(opt_LoopLabel);
                    
                    int next_factor = findNextUnrollFactor(slow_LoopIRName, LoopLabel2UnrollFactor[opt_LoopLabel] ,Loop2PotentialUnrollFactors);
                    llvm::errs() << "suggested next loop unroll factor for it is :" << next_factor << "\n";
                    desginconfig_0.insertLoopUnroll(opt_LoopLabel, next_factor);
                    
                    for (auto loop_array_pair : LoopLabel2DrivenArrayDimensions[opt_LoopLabel])
                    {
                        std::string funcStr = loop_array_pair.first;
                        std::string arrayStr = loop_array_pair.second.first;
                        int dim = loop_array_pair.second.second;
                        // std::map<std::string, std::set<std::pair<std::string, std::pair<std::string, int>> >>
                        desginconfig_0.insertArrayCyclicPartition( demangeFunctionName(funcStr), arrayStr, dim, next_factor);
                        
                    }
                }
            }


        }


        Mod_tmp.reset();


        assert(slow_LoopIRName!="");



    }
    
    return 0;
}


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

// load the HLS database of timing and resource
void DES_Load_Instruction_Info(const char* config_file_name, std::map<std::string,Info_type_list> &BiOp_Info_name2list_map)
{
    auto config_file = new std::ifstream(config_file_name);

    std::string  tmp_s;  
    std::string  tmpStr_forParsing;
    while ( getline(*config_file,tmp_s) )
    {    
        tmp_s = removeExtraSpace(tmp_s);
        std::stringstream iss(tmp_s);    
        std::string param_name;
        iss >> param_name ; //  get the name of parameter
        
        switch (hash_(param_name.c_str()))
        {
            case hash_compile_time("HLS_lib_path"):
                consumeEqual(iss);
                iss >> HLS_lib_path;
                break;

            default:
                break;
        }
    }
    assert(HLS_lib_path!="" && "The HLS Lib is necessary in the configuration file!\n");

    delete config_file;
    int i;
    for (i = 0; i<instructionInfoNum; i++)
    {
        if (instructionHasMappingFile[i])
        {
            Info_type_list tmp_list;
            std::string info_file_name(HLS_lib_path.c_str());
            info_file_name+=instructionNames[i];
            info_file_name+="/";
            info_file_name+=instructionNames[i];
            // info_file_name+="_data";
            std::ifstream info_file(info_file_name.c_str());

            if (!exists_test(info_file_name))
            {
                llvm::errs() << "The HLS info file ["+info_file_name+"] does not exist.\n";
                assert(false && "check the HLS information library path.\n");
            }
            std::string  tmp_s;  
            while ( getline(info_file,tmp_s) )
            {
                tmp_s = removeExtraSpace(tmp_s);
                std::stringstream iss(tmp_s);    
                std::string data_ele[11];
                for (int j=0; j<11; j++)
                {
                    iss >> data_ele[j];
                    iss.ignore(1,' ');
                }
                if (instructionNames[i]!="mac")
                {
                    if (data_ele[0]!=data_ele[1]||data_ele[1]!=data_ele[2])
                        continue; // ignore those unused information
                }
                else
                {
                    if (data_ele[0]!=data_ele[1])
                        continue; // ignore those unused information
                }
                
                inst_timing_resource_info tmp_info;

                int oprand_bitwidth = std::stoi(data_ele[0]);
                int res_bitwid = std::stoi(data_ele[2]);
                std::string _clock_period_str = data_ele[3];
                tmp_info.DSP = std::stoi(data_ele[4]);
                tmp_info.FF = std::stoi(data_ele[5]);
                tmp_info.LUT = std::stoi(data_ele[6]);
                tmp_info.Lat = std::stoi(data_ele[7]);
                tmp_info.delay = std::stod(data_ele[8]);
                tmp_info.II = std::stoi(data_ele[9]);
                tmp_info.core_name = data_ele[10];
                tmp_list[oprand_bitwidth][res_bitwid][_clock_period_str] = tmp_info;
                // str(oprandA)+" "+str(oprandB)+" "+str(oprandC)+" "+str(period)+" "
                // +str(DSP48E_N)+" "+str(FF_N)+" "+str(LUT_N)+" "+str(lat_tmp)+" "
                // +str(delay_tmp)+" "+str(II_tmp)+" "+str(Core_tmp)        
            }
            BiOp_Info_name2list_map[instructionNames[i]] = tmp_list;
        }
        
    }
}


enumerateDesignConfiguration::enumerateDesignConfiguration( const char *genFile_name,
                                   std::map<std::string, std::string> &IRLoop2LoopLabel, 
                                   std::map<std::string, int> &IRLoop2OriginTripCount,
                                   std::map<std::string, std::vector<std::string>> &LoopIRName2NextLevelSubLoopIRNames,
                                   std::map<std::string, int> &LoopIRName2Depth,
                                   std::map<std::string, std::vector< std::pair<std::string, std::string>>> &LoopIRName2Array,
                                   std::map<std::pair<std::string, std::string>, HI_PragmaArrayInfo> &TargetExtName2ArrayInfo)
                                   :
                                   IRLoop2LoopLabel(IRLoop2LoopLabel), 
                                   IRLoop2OriginTripCount(IRLoop2OriginTripCount),
                                   LoopIRName2NextLevelSubLoopIRNames(LoopIRName2NextLevelSubLoopIRNames),
                                   LoopIRName2Depth(LoopIRName2Depth),
                                   LoopIRName2Array(LoopIRName2Array),
                                   TargetExtName2ArrayInfo(TargetExtName2ArrayInfo)
{
    std::error_code ErrInfo;
    auto genFile = new raw_fd_ostream(genFile_name, ErrInfo, sys::fs::F_None);
    for (auto loopIRName_dep_pair : LoopIRName2Depth)
    {
        if (loopIRName_dep_pair.second == 1)
        {
            *genFile << "find top-level-loop: IRname:" << loopIRName_dep_pair.first 
                     << " LABEL=" << IRLoop2LoopLabel[loopIRName_dep_pair.first] 
                     << "\n";
            for (auto func_array_pair : LoopIRName2Array[loopIRName_dep_pair.first])
            {
                *genFile << "    possible array configuration for: " << TargetExtName2ArrayInfo[func_array_pair] << "\n";
                std::string funcStr = func_array_pair.first;
                std::string arrayStr = func_array_pair.second;
                arrayPragmaGenerator array_configurationGen(TargetExtName2ArrayInfo[func_array_pair]);
                for (auto arrayCfg : array_configurationGen.getArrayConfigurations())
                {
                    *genFile << "        -----------> " << arrayCfg << "\n"; 
                }
            }
            
            // std::vector<std::string>
            // generateLoopConfiguration(loopIRName_dep_pair.first, 0);

        }
    }
    genFile->flush();
    delete genFile;
}


// void enumerateDesignConfiguration::generateLoopConfiguration(std::string LoopName, bool pragmaSet)
// {
//   // should be BFS!!
//     if (LoopIRName2NextLevelSubLoopIRNames.find(LoopName) != LoopIRName2NextLevelSubLoopIRNames.end())
//     {
//         for (auto )
//     }
//     else
//     {
//     }
    
    
// }
