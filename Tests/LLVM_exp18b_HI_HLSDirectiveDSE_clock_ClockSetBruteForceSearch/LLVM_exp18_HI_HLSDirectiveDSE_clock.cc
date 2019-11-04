#include "LLVM_exp18_HI_HLSDirectiveDSE_clock.h"
#include "ConfigParse.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "ConfigGen.h"
#include "Pre_Process.h"

#include <ctime>



using namespace llvm;


std:: string clock_period_str;
std:: string  HLS_lib_path;

using namespace clang;
using namespace clang::driver;
using namespace clang::tooling;



double DSP_limit = -1;
double FF_limit = -1;
double LUT_limit = -1;
double BRAM_limit = -1;
int ClockNum_limit = -1;

int main(int argc, const char **argv) 
{


    //////////////////////////////////////////////////////////////////////////////////////
    //////////////////////    argument input and check        ////////////////////////////  
    //////////////////////////////////////////////////////////////////////////////////////

    if (argc < 4) 
    {
        errs() << "Usage: " << argv[0] << " <C/C++ file> <Top_Function_Name> <Config_File_Path>\n";
        return 1;
    }


    std::string top_str = std::string(argv[2]);
    std::string configFile_str = std::string(argv[3]);
    bool debugFlag = (argc == 5 && std::string(argv[4])=="DEBUG");


    //////////////////////////////////////////////////////////////////////////////////////
    ///////////////////    Source Code Process and Analysis   ////////////////////////////  
    //////////////////////////////////////////////////////////////////////////////////////


    std::map<std::string, int> FuncParamLine2OutermostSize;

    clangPreProcess(argv, top_str, FuncParamLine2OutermostSize);



    //////////////////////////////////////////////////////////////////////////////////////
    //////////////////////    pre-processing of the IR module ////////////////////////////  
    //////////////////////////////////////////////////////////////////////////////////////

    LLVMInitializeX86TargetInfo();
    LLVMInitializeX86Target();
    LLVMInitializeX86TargetMC();
    // Compile the source code into IR and Parse the input LLVM IR file into a module
    SMDiagnostic Err;
    LLVMContext Context;
    // std::string cmd_str = "clang -O1 -emit-llvm -S -g "+std::string(argv[1])+" -o top.bc 2>&1";
    std::string cmd_str = "clang -O1 -emit-llvm -S -g tmp_loopLabeled.cc -o top_notinline.bc 2>&1";

    print_cmd(cmd_str.c_str());
    bool result = sysexec(cmd_str.c_str());
    assert(result); // ensure the cmd is executed successfully


    cmd_str = "opt -always-inline top_notinline.bc -o top.bc 2>&1";
    print_cmd(cmd_str.c_str());
    result = sysexec(cmd_str.c_str());
    assert(result); // ensure the cmd is executed successfully

    std::unique_ptr<llvm::Module> Mod(parseIRFile("top.bc", Err, Context));
    if (!Mod) 
    {
        Err.print(argv[0], errs());
        return 1;
    }

    std::map<std::string,Info_type_list> BiOp_Info_name2list_map;
    
    // record the specific level of loop is sensitive to specific partitioning scheme of the specific dimension of array
    std::map<std::string, std::set<std::pair<std::string, std::pair<std::string, int>> >> LoopLabel2DrivenArrayDimensions;
    std::map<std::string, std::vector<int>> IRFunc2BeginLine;
    std::map<std::string, std::string> IRLoop2LoopLabel;
    std::map<std::string, int> IRLoop2OriginTripCount;
    std::map<std::string, std::vector<std::string>> LoopIRName2NextLevelSubLoopIRNames;
    std::map<std::string, int> LoopIRName2Depth;
    std::map<std::string, std::vector< std::pair<std::string, std::string>>> LoopIRName2Array;
    std::map<std::pair<std::string, std::string>, HI_PragmaArrayInfo> TargetExtName2ArrayInfo;
    std::map<std::string, std::string> TopLoopIR2FuncName;
    std::map<std::string, std::vector<int>> Loop2PotentialUnrollFactors;
    std::vector<std::string> FuncNames;

    preAnalyzeProcess(
        *Mod,
        top_str,
        configFile_str,
        BiOp_Info_name2list_map,
        LoopLabel2DrivenArrayDimensions,
        IRFunc2BeginLine,
        IRLoop2LoopLabel,
        IRLoop2OriginTripCount,
        LoopIRName2NextLevelSubLoopIRNames,
        LoopIRName2Depth,
        LoopIRName2Array,
        TargetExtName2ArrayInfo,
        TopLoopIR2FuncName,
        Loop2PotentialUnrollFactors,
        FuncParamLine2OutermostSize,
        FuncNames,
        debugFlag
    );

    //////////////////////////////////////////////////////////////////////////////////////
    //////////////////////    iterative DSE for the module    ////////////////////////////  
    //////////////////////////////////////////////////////////////////////////////////////
    
    
    HI_DesignConfigInfo desginconfig_last;
    Parse_Config(configFile_str.c_str(), desginconfig_last);
    
    enumerateDesignConfiguration configs_bruteForse("configs_brute",
                                   IRLoop2LoopLabel, 
                                   IRLoop2OriginTripCount,
                                   LoopIRName2NextLevelSubLoopIRNames,
                                   LoopIRName2Depth,
                                   LoopIRName2Array,
                                   TargetExtName2ArrayInfo);

        
    std::string slowestTopLoop = "";
    std::error_code EC;
    
    
    bool allLoopOptimized = false;

    std::vector<std::map<std::string, std::string>> potential_clockForFunction_vector;
    std::vector<int> clockIds;
    generateClockFunctionMaps(clockIds, FuncNames, potential_clockForFunction_vector);

    llvm::raw_fd_ostream pushing_relaxtion_log("pushing_relaxtion_log.txt", EC, llvm::sys::fs::F_None);


    int trial_cnt = 50000;
    std::map<std::string, HI_WithDirectiveTimingResourceEvaluation::timingBase> MultiClk_FuncName2Latency;
    std::map<std::string, HI_WithDirectiveTimingResourceEvaluation::resourceBase> MultiClk_FuncName2Resource;
    MultiClk_FuncName2Latency.clear();
    MultiClk_FuncName2Resource.clear();

    // for (auto clockForFunction : potential_clockForFunction_vector)
    //     llvm::errs() << printClocks(clockForFunction)  << "\n=============================\n";
    // return 0;

    std::clock_t clock_start;
    double duration;

    clock_start = std::clock();

    std::map<std::string, std::string> clockForFunction0;
    for (auto clockForFunction : potential_clockForFunction_vector)
    {
        pushing_relaxtion_log << "changing clock setting from " << printClocks(clockForFunction0) << " to " << printClocks(clockForFunction) << "\n";
        while (1)
        {
            std::string slowestFunctioName = "";
            double max_Latency = -1.0;
            double second_max_Latency = -1.0;
            double clock_forSlowestFunc = -1.0;
            HI_WithDirectiveTimingResourceEvaluation::resourceBase ori_total_resource, newclk_total_resource, newclk_func_resource, newloop_total_resource;
            designEvalResult entire_design_eval_result;
            std::map<std::string, std::string> new_clockForFunction;

            llvm::raw_fd_ostream checkedConfigs_multipleClockDesignEvaluation("checkedConfigs_multipleClockDesignEvaluation.txt", EC, llvm::sys::fs::F_None);

            int output_next_factor;
            std::string output_next_loopLabel;

            /////////////// evaluate the multiplce clock design ///////////////////////
            bool eval_success = multipleClockDesignEvaluation(
                            desginconfig_last,
                            checkedConfigs_multipleClockDesignEvaluation, 
                            *Mod, 
                            entire_design_eval_result,


                            slowestFunctioName,
                            max_Latency,
                            second_max_Latency,
                            clock_forSlowestFunc,
                            ori_total_resource,
                            MultiClk_FuncName2Latency,
                            MultiClk_FuncName2Resource,
                            output_next_factor,
                            output_next_loopLabel,

                            clockForFunction,
                            FuncNames,
                            IRLoop2LoopLabel, 
                            IRFunc2BeginLine,
                            FuncParamLine2OutermostSize,
                            Loop2PotentialUnrollFactors,
                            LoopLabel2DrivenArrayDimensions,
                            IRLoop2OriginTripCount,
                            TopLoopIR2FuncName,
                            LoopIRName2NextLevelSubLoopIRNames,
                            LoopIRName2Depth,
                            LoopIRName2Array,
                            BiOp_Info_name2list_map,
                            0,
                            top_str,
                            debugFlag
            );
            checkedConfigs_multipleClockDesignEvaluation.flush();
            checkedConfigs_multipleClockDesignEvaluation.close();
            assert(eval_success && "There should be no failure during evaluation");

            pushing_relaxtion_log << "===================================\ndesign result:\n" << entire_design_eval_result << "\n";
            pushing_relaxtion_log << ">>>>>>>>>>>>>>>> current resource rate ==" << utilizationIndex(ori_total_resource) << "\n";
            pushing_relaxtion_log << ">>>>>>>>>>>>>>>> current bottleneck latency ==" << max_Latency << "\n";
            pushing_relaxtion_log.flush();
            
            ///////////////////// try to push HLS directives ///////////////////////
            HI_DesignConfigInfo desginconfig_optLoop;
            Parse_Config(configFile_str.c_str(), desginconfig_optLoop);
            double tryDircetive_latency;
            llvm::raw_fd_ostream checkedConfigs_tryUpdateHLSDirectives("checkedConfigs_tryUpdateHLSDirectives.txt", EC, llvm::sys::fs::F_None);
            bool tryDirective = tryUpdateHLSDirectives(
                            desginconfig_last,
                            checkedConfigs_tryUpdateHLSDirectives, 
                            *Mod, 

                            slowestFunctioName,
                            max_Latency,
                            second_max_Latency,
                            ori_total_resource,
                            newloop_total_resource,
                            desginconfig_optLoop,
                            tryDircetive_latency,

                            clockForFunction,
                            FuncNames,
                            IRLoop2LoopLabel, 
                            IRFunc2BeginLine,
                            FuncParamLine2OutermostSize,
                            Loop2PotentialUnrollFactors,
                            LoopLabel2DrivenArrayDimensions,
                            IRLoop2OriginTripCount,
                            TopLoopIR2FuncName,
                            LoopIRName2NextLevelSubLoopIRNames,
                            LoopIRName2Depth,
                            LoopIRName2Array,
                            BiOp_Info_name2list_map,
                            0,
                            top_str,
                            debugFlag
                            );
                // 
                // clockForFunction[slowestFunctioName] = clockStrs_DSE[curClock_id];
            checkedConfigs_tryUpdateHLSDirectives.flush();
            checkedConfigs_tryUpdateHLSDirectives.close();

           ////////////////// try to push clock ///////////////////

            std::string optClock = "";
            HI_DesignConfigInfo desginconfig_optClock;
            Parse_Config(configFile_str.c_str(), desginconfig_optClock);
            bool tryClock = true;
            bool enableTryClock = false;

            new_clockForFunction = clockForFunction;
            if (enableTryClock)
            {
                llvm::raw_fd_ostream checkedConfigs_tryUpdateSlowestFuncClock("checkedConfigs_tryUpdateSlowestFuncClock.txt", EC, llvm::sys::fs::F_None);

                
                
                checkedConfigs_tryUpdateSlowestFuncClock.flush();
                checkedConfigs_tryUpdateSlowestFuncClock.close();
            }



            if (enableTryClock)
            {
                pushing_relaxtion_log << "EVALUATED update clock for function [" << slowestFunctioName 
                            << "] from " << printClocks(clockForFunction)  << "ns"
                            << " to " <<  printClocks(new_clockForFunction) << "ns\n";
                // pushing_relaxtion_log.flush();
            }
            else
            {
                pushing_relaxtion_log << "EVALUATED update clock for function [" << slowestFunctioName 
                            << "] : No NEED currently.\n";
                // pushing_relaxtion_log.flush();
            }

            
            pushing_relaxtion_log << "EVALUATED HLS directive for function [" << slowestFunctioName 
                    << "] from \n" << desginconfig_last 
                    << " to \n" << desginconfig_optLoop << "\n";
            // pushing_relaxtion_log.flush();

            pushing_relaxtion_log << "utilizationIndex(newloop_total_resource)=" << utilizationIndex(newloop_total_resource) << "\n";
            stringUtilizationIndex(newloop_total_resource);
            stringUtilization(newloop_total_resource);
            pushing_relaxtion_log << "utilizationIndex(newclk_total_resource)=" << utilizationIndex(newclk_total_resource) << "\n";
            stringUtilizationIndex(newclk_total_resource);
            stringUtilization(newclk_total_resource);
            

            if (tryClock && tryDirective)
            {
                if (utilizationViolation(newclk_total_resource))
                {
                    pushing_relaxtion_log << "clk utilization violation.\n";
                    tryClock = 0;
                }

                if (utilizationViolation(newloop_total_resource))
                {
                    pushing_relaxtion_log << "loop utilization violation.\n";
                    tryDirective = 0;
                }

                if (tryDirective)
                    tryClock = 0;

                if (tryClock && !tryDirective)
                {
                    pushing_relaxtion_log << ">>>>>>>>>>>>>>>>>>>>> clock update >>>>>>>>>>>>>>>>>>>>>>>>>>\n";
                    pushing_relaxtion_log << "update clock for function [" << slowestFunctioName 
                            << "] from " << printClocks(clockForFunction)  << "ns"
                            << " to " <<  printClocks(new_clockForFunction) << "ns\n";
                    pushing_relaxtion_log << "update HLS directive for function [" << slowestFunctioName 
                            << "] from \n" << desginconfig_last 
                            << " to \n" << desginconfig_optClock << "\n";
                    // pushing_relaxtion_log.flush();
                }
                else if (!tryClock && tryDirective)
                {
                    pushing_relaxtion_log << ">>>>>>>>>>>>>>>>>>>>> HLS directive update >>>>>>>>>>>>>>>>>>>>>>>>>>\n";
                    pushing_relaxtion_log << "update HLS directive for function [" << slowestFunctioName 
                            << "] from \n" << desginconfig_last 
                            << " to \n" << desginconfig_optLoop << "\n";
                    // pushing_relaxtion_log.flush();
                }

                if (tryClock && !tryDirective)
                {
                    desginconfig_last = desginconfig_optClock;
                    break;
                }
                else if (!tryClock && tryDirective)
                {
                    desginconfig_last = desginconfig_optLoop;
                }
                else if (!tryClock && !tryDirective)
                {
                    desginconfig_last = desginconfig_optClock;
                    desginconfig_last.loopUnrollConfigs.clear();
                    desginconfig_last.loopPipelineConfigs.clear();
                    desginconfig_last.cyclicPartitionConfigs.clear();
                    desginconfig_last.blockPartitionConfigs.clear();
                    desginconfig_last.LoopLabel2UnrollFactor.clear();
                    break;
                    pushing_relaxtion_log <<  "Both Utilization Violation, choose clock solution\n";
                    // pushing_relaxtion_log.flush();
                //  break;
                }
            }
            else if (!tryClock && tryDirective)
            {
                if (utilizationViolation(newloop_total_resource))
                {
                    pushing_relaxtion_log << "loop utilization violation.\n";
                    tryDirective = 0;
                }


                pushing_relaxtion_log << ">>>>>>>>>>>>>>>>>>>>> HLS directive update >>>>>>>>>>>>>>>>>>>>>>>>>>\n";
                pushing_relaxtion_log << "update HLS directive for function [" << slowestFunctioName 
                        << "] from \n" << desginconfig_last 
                        << " to \n" << desginconfig_optLoop << "\n";
                // pushing_relaxtion_log.flush();
                
                if (tryDirective)
                {
                    desginconfig_last = desginconfig_optLoop;
                }
                else
                {
                    print_error("No Further Step can take");
                    break;
                    //assert(false && "should not occur");
                    desginconfig_last = desginconfig_optLoop;
                    // print_error("Directive Utilization Violation and no Clock solution, choose Directive solution");
                }
            }
            else if (tryClock && !tryDirective)
            {
                if (utilizationViolation(newclk_total_resource))
                {
                    pushing_relaxtion_log << "clk utilization violation.\n";
                    tryClock = 0;
                }

                pushing_relaxtion_log << ">>>>>>>>>>>>>>>>>>>>> clock update >>>>>>>>>>>>>>>>>>>>>>>>>>\n";
                pushing_relaxtion_log << "update clock for function [" << slowestFunctioName 
                            << "] from " << printClocks(clockForFunction)  << "ns"
                            << " to " <<  printClocks(new_clockForFunction) << "ns\n";
                pushing_relaxtion_log << "update HLS directive for function [" << slowestFunctioName 
                        << "] from \n" << desginconfig_last 
                        << " to \n" << desginconfig_optClock << "\n";

                    // pushing_relaxtion_log.flush();

                if (tryClock)
                {                   
                    desginconfig_last = desginconfig_optClock;
                    break;
                }
                else
                {
                    desginconfig_last = desginconfig_optClock;
                    desginconfig_last = desginconfig_optClock;
                    desginconfig_last.loopUnrollConfigs.clear();
                    desginconfig_last.loopPipelineConfigs.clear();
                    desginconfig_last.cyclicPartitionConfigs.clear();
                    desginconfig_last.blockPartitionConfigs.clear();
                    desginconfig_last.LoopLabel2UnrollFactor.clear();
                    pushing_relaxtion_log << "Clock Utilization Violation and no Directive solution, choose Clock solution\n";
                    break;
                    // pushing_relaxtion_log.flush();
                }
            }
            else
            {

                print_error("No Further Step can take");
                break;
            }
            pushing_relaxtion_log.flush();
        }

    }
    pushing_relaxtion_log.close();
    duration = ( std::clock() - clock_start ) / (double) CLOCKS_PER_SEC;
    print_error("time cost: " + std::to_string(duration) + "sec");
    return 0;
}


