#include "LLVM_exp17_HI_HLSDirectiveDSE.h"
#include "ConfigParse.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "ConfigGen.h"
#include "DesignEval.h"
using namespace llvm;

std:: string clock_period_str;
std:: string  HLS_lib_path;




using namespace clang;
using namespace clang::driver;
using namespace clang::tooling;



static llvm::cl::OptionCategory StatSampleCategory("Stat Sample");


double DSP_limit = 50;
double FF_limit = 6000;
double LUT_limit = 6000;
double BRAM_limit = 400;


int main(int argc, const char **argv) {
    if (argc < 4) {
      errs() << "Usage: " << argv[0] << " <C/C++ file> <Top_Function_Name> <Config_File_Path>\n";
      return 1;
    }

    //////////////////////////////////////////////////////////////////////////////////////
    //////////////////////    source code compiled into IR    ////////////////////////////  
    //////////////////////////////////////////////////////////////////////////////////////

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
    std::map<std::string, std::string> IRLoop2LoopLabel;
    std::map<std::string, int> IRLoop2OriginTripCount;
    std::map<std::string, std::vector<std::string>> LoopIRName2NextLevelSubLoopIRNames;
    std::map<std::string, int> LoopIRName2Depth;
    std::map<std::string, std::vector< std::pair<std::string, std::string>>> LoopIRName2Array;
    std::map<std::pair<std::string, std::string>, HI_PragmaArrayInfo> TargetExtName2ArrayInfo;
    std::map<std::string, std::vector<int>> Loop2PotentialUnrollFactors;
    std::map<std::string, std::vector<int>> IRFunc2BeginLine;

    

    
    LLVMTargetRef T;
    ModulePassManager MPM;

    char *Error;

    modulePreProcessForInformation(
        *Mod,
        top_str,
        BiOp_Info_name2list_map,
        IRFunc2BeginLine,
        FuncParamLine2OutermostSize,
        LoopLabel2DrivenArrayDimensions,
        IRLoop2LoopLabel,
        IRLoop2OriginTripCount,
        LoopIRName2NextLevelSubLoopIRNames,
        LoopIRName2Depth,
        LoopIRName2Array,
        TargetExtName2ArrayInfo,
        Loop2PotentialUnrollFactors,
        debugFlag
    );


    //////////////////////////////////////////////////////////////////////////////////////
    //////////////////////    iterative DSE for the module    ////////////////////////////  
    //////////////////////////////////////////////////////////////////////////////////////
    HI_DesignConfigInfo desginconfig_0;
    Parse_Config(configFile_str.c_str(), desginconfig_0);
        
    std::string slowestTopLoop = "";

    llvm::raw_fd_ostream checkedConfigs("configSettings.txt", EC, llvm::sys::fs::F_None);
    
    

    for (int i=0; i<1000; i++)
    {
        checkedConfigs << "config #" << i << ":\n" << desginconfig_0;
        checkedConfigs.flush();
        
        std::unique_ptr<llvm::Module> Mod_tmp = CloneModule(*Mod);
        std::string cntStr = std::to_string(i);


        std::map<std::string, int> LoopLabel2UnrollFactor;
        std::map<std::string, int> LoopLabel2II;
        std::map<std::string, int> IRLoop2OriginTripCount_eval;
        std::map<std::string, std::string> IRLoop2LoopLabel_eval;
    
        // set the unroll factors
        for (auto loopUnroll_pair : desginconfig_0.loopUnrollConfigs)
        {
            LoopLabel2UnrollFactor[loopUnroll_pair.first] = loopUnroll_pair.second;
        }


        modulePreProcessForFPGAHLS_optimization(
            *Mod_tmp,
            top_str,
            cntStr,
            BiOp_Info_name2list_map,
            IRFunc2BeginLine,
            FuncParamLine2OutermostSize,
            LoopLabel2DrivenArrayDimensions,
            IRLoop2LoopLabel,
            IRLoop2LoopLabel_eval,
            IRLoop2OriginTripCount,
            IRLoop2OriginTripCount_eval,
            LoopIRName2NextLevelSubLoopIRNames,
            LoopIRName2Depth,
            LoopIRName2Array,
            TargetExtName2ArrayInfo,
            Loop2PotentialUnrollFactors,
            LoopLabel2UnrollFactor,
            debugFlag
        );

        bool unrollNotChanged = 1;
        bool canFurtherOpt = 1;

        while (unrollNotChanged)
        {
    //////////////////////////////////////////////////////////////////////////////////////
    //////////////////////    DSE without changing unrolling factor          /////////////
    //////////////////////////////////////////////////////////////////////////////////////

            // set the pipeline II
            for (auto loopPipeline_pair : desginconfig_0.loopPipelineConfigs)
            {
                LoopLabel2II[loopPipeline_pair.first] = loopPipeline_pair.second;
            }

            std::unique_ptr<llvm::Module> Mod_tmp_eval = CloneModule(*Mod_tmp);
            
            bool muxWithMoreThan32;
            std::string muxTooMuchLoopIRName;

            modulePreProcessForPreEvaluation(
                *Mod_tmp_eval,
                top_str,
                cntStr,
                desginconfig_0,
                BiOp_Info_name2list_map,
                IRFunc2BeginLine,
                FuncParamLine2OutermostSize,
                LoopLabel2DrivenArrayDimensions,
                IRLoop2LoopLabel,
                IRLoop2LoopLabel_eval,
                IRLoop2OriginTripCount,
                IRLoop2OriginTripCount_eval,
                LoopIRName2NextLevelSubLoopIRNames,
                LoopIRName2Depth,
                LoopIRName2Array,
                TargetExtName2ArrayInfo,
                Loop2PotentialUnrollFactors,
                LoopLabel2UnrollFactor,
                muxWithMoreThan32,
                muxTooMuchLoopIRName,
                debugFlag
            );



            std::string slow_LoopIRName = "";
    //////////////////////////////////////////////////////////////////////////////////////
    ///////////   Decide the New Parameters for Configuration in Next Step   /////////////
    //////////////////////////////////////////////////////////////////////////////////////

            // bypassing the design with large MUX
            if (muxWithMoreThan32)
            {
                print_warning("Too larget mux detected. Non-optimal, bypass evaluation.");
                slow_LoopIRName = muxTooMuchLoopIRName;
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
                    desginconfig_0.increaseArrayCyclicPartition( demangeFunctionName(funcStr), arrayStr, dim, next_factor);
                    
                }
                unrollNotChanged = 0;
            }
            else
            {

                legacy::PassManager PM5;
                std::string logName_evaluation = "HI_WithDirectiveTimingResourceEvaluation_forCheck_"+cntStr ;
                std::string logName_array = "ArrayLog_forCheck_"+cntStr ;


                // obtain the evaluation of resource and timing for the source code
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
                                                                    debugFlag);
                // print_info("Enable HI_WithDirectiveTimingResourceEvaluation Pass");
                PM5.add(hi_withdirectivetimingresourceevaluation); 

                print_status("Start LLVM processing");  
                PM5.run(*Mod_tmp_eval);
                print_status("Accomplished LLVM processing");

                checkedConfigs << "\n";
                for (auto it : IRLoop2LoopLabel)
                {
                    if (hi_withdirectivetimingresourceevaluation->LoopLabel2Latency.find( it.second) != hi_withdirectivetimingresourceevaluation->LoopLabel2Latency.end())
                    {
                        checkedConfigs << "    " << it.second << " ====> " << hi_withdirectivetimingresourceevaluation->LoopLabel2Latency[it.second] << it.second 
                                                 << " ====> " << hi_withdirectivetimingresourceevaluation->LoopLabel2Resource[it.second] << "\n";
                    }
                    
                }

                
                checkedConfigs << "    Top Function Latency: " << hi_withdirectivetimingresourceevaluation->top_function_latency << " cycle\n\n";
                checkedConfigs << "    Top Function Delay: " << hi_withdirectivetimingresourceevaluation->top_function_latency*desginconfig_0.clock_period << " ns\n\n";
                checkedConfigs << "    Top Function Resource: " << hi_withdirectivetimingresourceevaluation->topFunction_resource << "\n\n";

                if (debugFlag)
                {
                    print_status("Writing LLVM IR to File");
                    llvm::raw_fd_ostream OSPM4("top_output.bc", EC, llvm::sys::fs::F_None);
                    WriteBitcodeToFile(*Mod_tmp_eval, OSPM4);
                    OSPM4.flush();
                    cmd_str = "llvm-dis top_output.bc 2>&1";
                    print_cmd(cmd_str.c_str());
                    result = sysexec(cmd_str.c_str());
                    assert(result); // ensure the cmd is executed successfully
                }

                bool reRun = 0;
                for (auto pair : hi_withdirectivetimingresourceevaluation->FuncArray2PartitionBenefit)
                {
                    if (!pair.second) // no benefit
                    {
                        desginconfig_0.eraseArrayPartition(pair.first.first, pair.first.second);
                        checkedConfigs << "Array [" << pair.first.second << "] in Func: [" << pair.first.first<< "] don't need partitioning. Re-run evaluation without partitioning. \n";
                        reRun = 1;
                    }
                }
                if (reRun)
                {
                    i++;
                    checkedConfigs << "config #" << i << ":\n" << desginconfig_0;
                    checkedConfigs.flush();
                    continue;
                }

                
                assert(hi_withdirectivetimingresourceevaluation->topFunctionFound && "The specified top function is not found in the program");

                stringUtilizationIndex(hi_withdirectivetimingresourceevaluation->topFunction_resource);

                if (utilizationViolation(hi_withdirectivetimingresourceevaluation->topFunction_resource))
                {
                    canFurtherOpt = 0;
                    print_error("no further step can optimize the throughput.");
                    break;
                }
                



                slow_LoopIRName = findSlowestLastLevelLoop_InSlowestTopLoop(IRLoop2LoopLabel,
                                                        hi_withdirectivetimingresourceevaluation->LoopLabel2Latency, 
                                                        LoopIRName2NextLevelSubLoopIRNames,
                                                        LoopIRName2Depth
                    );
                
                if (slow_LoopIRName=="")
                {
                    print_warning("all loop are optimized!\n");
                    break;
                }
                
                std::string opt_LoopLabel = IRLoop2LoopLabel[slow_LoopIRName];
                llvm::errs() << "loop to optimized is :" << opt_LoopLabel << "\n";





                bool directUnroll = hi_withdirectivetimingresourceevaluation->LoopLabel2IterationLatency[opt_LoopLabel]-1  == 0;

                if (!directUnroll && hi_withdirectivetimingresourceevaluation->LoopLabel2AchievedII.find(opt_LoopLabel) == hi_withdirectivetimingresourceevaluation->LoopLabel2AchievedII.end())
                {
                    llvm::errs() << "for config#" << i+1 << " suggested start loop pipeline II for it is :" << hi_withdirectivetimingresourceevaluation->LoopLabel2IterationLatency[opt_LoopLabel]-1 << "\n";
                    desginconfig_0.insertLoopPipeline(opt_LoopLabel, hi_withdirectivetimingresourceevaluation->LoopLabel2IterationLatency[opt_LoopLabel]-1);
                }
                else
                {
                    if (!directUnroll && hi_withdirectivetimingresourceevaluation->LoopLabel2AchievedII[opt_LoopLabel] != hi_withdirectivetimingresourceevaluation->LoopLabel2SmallestII[opt_LoopLabel])
                    {
                        llvm::errs()  << "LoopLabel2AchievedII=" << hi_withdirectivetimingresourceevaluation->LoopLabel2AchievedII[opt_LoopLabel] 
                                    << " LoopLabel2SmallestII=" << hi_withdirectivetimingresourceevaluation->LoopLabel2SmallestII[opt_LoopLabel] << "\n";
                        // llvm::errs()  << "for config#" << i+1 << "suggested next loop pipeline II for it is :" << (hi_withdirectivetimingresourceevaluation->LoopLabel2AchievedII[opt_LoopLabel] + hi_withdirectivetimingresourceevaluation->LoopLabel2SmallestII[opt_LoopLabel])/2 << "\n";
                        llvm::errs()  << "for config#" << i+1 << "suggested next loop pipeline II for it is :" << (hi_withdirectivetimingresourceevaluation->LoopLabel2AchievedII[opt_LoopLabel] - 1) << "\n";
                        // desginconfig_0.insertLoopPipeline(opt_LoopLabel, (hi_withdirectivetimingresourceevaluation->LoopLabel2AchievedII[opt_LoopLabel] + hi_withdirectivetimingresourceevaluation->LoopLabel2SmallestII[opt_LoopLabel])/2);
                        desginconfig_0.insertLoopPipeline(opt_LoopLabel, (hi_withdirectivetimingresourceevaluation->LoopLabel2AchievedII[opt_LoopLabel] - 1));
                    }
                    else
                    {
                        unrollNotChanged = 0;

                        desginconfig_0.eraseLoopPipeline(opt_LoopLabel);
                        
                        int next_factor = findNextUnrollFactor(slow_LoopIRName, LoopLabel2UnrollFactor[opt_LoopLabel] ,Loop2PotentialUnrollFactors);
                        llvm::errs()  << "for config#" << i+1 << "suggested next loop unroll factor for it is :" << next_factor << "\n";
                        desginconfig_0.insertLoopUnroll(opt_LoopLabel, next_factor);
                        
                        for (auto loop_array_pair : LoopLabel2DrivenArrayDimensions[opt_LoopLabel])
                        {
                            std::string funcStr = loop_array_pair.first;
                            std::string arrayStr = loop_array_pair.second.first;
                            int dim = loop_array_pair.second.second;
                            // std::map<std::string, std::set<std::pair<std::string, std::pair<std::string, int>> >>
                            desginconfig_0.increaseArrayCyclicPartition( demangeFunctionName(funcStr), arrayStr, dim, next_factor);
                            
                        }
                    }
                }
            }

            if (unrollNotChanged)
            {
                i++;
                checkedConfigs << "config #" << i << ":\n" << desginconfig_0;
                checkedConfigs.flush();
            }



            Mod_tmp_eval.reset();
    
        }


        Mod_tmp.reset();

        if (!canFurtherOpt)
            break;
    }
    checkedConfigs.flush();

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
