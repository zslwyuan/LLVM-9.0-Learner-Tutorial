#include "ConfigGen.h"


extern double DSP_limit;
extern double FF_limit;
extern double LUT_limit;
extern double BRAM_limit;
extern int ClockNum_limit;

std::string findSlowestLastLevelLoop_InSlowestTopLoop(
                    std::map<std::string, std::string> &IRLoop2LoopLabel,
                    std::map<std::string, HI_WithDirectiveTimingResourceEvaluation::timingBase> &LoopLabel2Latency, 
                    std::map<std::string, std::vector<std::string>> &LoopIRName2NextLevelSubLoopIRNames,
                    std::map<std::string, int> &LoopIRName2Depth
    )
{
    int max_latency = -1;
    std::string slowest_sub_loop = "";
    for (auto IRLoop_label_pair : IRLoop2LoopLabel)
    {
        std::string irName = IRLoop_label_pair.first;
        std::string loopLabel = IRLoop_label_pair.second;
        if (LoopIRName2Depth[irName] == 1)
        {
            if (LoopLabel2Latency.find(IRLoop2LoopLabel[irName]) != LoopLabel2Latency.end())
            {
                int curLatency = LoopLabel2Latency[IRLoop2LoopLabel[irName]].latency;
                if (curLatency > max_latency)
                {
                    max_latency = curLatency;
                    slowest_sub_loop = irName;
                }
            }
        }
    }
    
    if (slowest_sub_loop=="")
        return slowest_sub_loop;

    assert(slowest_sub_loop!= "");

    if (LoopIRName2NextLevelSubLoopIRNames.find(slowest_sub_loop) != LoopIRName2NextLevelSubLoopIRNames.end())
        return findTheSlowestLoop(slowest_sub_loop, IRLoop2LoopLabel, LoopLabel2Latency, LoopIRName2NextLevelSubLoopIRNames, LoopIRName2Depth);
    else
        return slowest_sub_loop;    
}


std::string findSlowestLastLevelLoop_InSlowestTopLoop(
                    std::string funcScope,
                    std::map<std::string, std::string> &TopLoopIR2FuncName,     
                    std::map<std::string, std::string> &IRLoop2LoopLabel,
                    std::map<std::string, HI_WithDirectiveTimingResourceEvaluation::timingBase> &LoopLabel2Latency, 
                    std::map<std::string, std::vector<std::string>> &LoopIRName2NextLevelSubLoopIRNames,
                    std::map<std::string, int> &LoopIRName2Depth
    )
{
    double max_latency = -1;
    std::string slowest_sub_loop = "";

    std::string topIRLoop = "";
    for (auto it : TopLoopIR2FuncName)
        if (it.second == funcScope)
        {
            topIRLoop = it.first;
            break;
        }

    if (LoopLabel2Latency.find(IRLoop2LoopLabel[topIRLoop]) != LoopLabel2Latency.end())
    {
        double curLatency = LoopLabel2Latency[IRLoop2LoopLabel[topIRLoop]].latency * LoopLabel2Latency[IRLoop2LoopLabel[topIRLoop]].clock_period;
        if (curLatency > max_latency)
        {
            max_latency = curLatency;
            slowest_sub_loop = topIRLoop;
        }
    }
            
    
    if (slowest_sub_loop=="")
        return slowest_sub_loop;

    assert(slowest_sub_loop!= "");

    if (LoopIRName2NextLevelSubLoopIRNames.find(slowest_sub_loop) != LoopIRName2NextLevelSubLoopIRNames.end())
        return findTheSlowestLoop(slowest_sub_loop, IRLoop2LoopLabel, LoopLabel2Latency, LoopIRName2NextLevelSubLoopIRNames, LoopIRName2Depth);
    else
        return slowest_sub_loop;    
}


std::string findTheSlowestLoop(
                    std::string cur_LoopIRName,
                    std::map<std::string, std::string> &IRLoop2LoopLabel,
                    std::map<std::string, HI_WithDirectiveTimingResourceEvaluation::timingBase> &LoopLabel2Latency, 
                    std::map<std::string, std::vector<std::string>> &LoopIRName2NextLevelSubLoopIRNames,
                    std::map<std::string, int> &LoopIRName2Depth
                    )
{
    if (LoopIRName2NextLevelSubLoopIRNames.find(cur_LoopIRName) != LoopIRName2NextLevelSubLoopIRNames.end())
    {
        double max_latency = -1;
        std::string slowest_sub_loop = "";
        for (auto next_level_IRname : LoopIRName2NextLevelSubLoopIRNames[cur_LoopIRName])
        {
            if (LoopLabel2Latency.find(IRLoop2LoopLabel[next_level_IRname]) != LoopLabel2Latency.end())
            {
                double curLatency = LoopLabel2Latency[IRLoop2LoopLabel[next_level_IRname]].latency * LoopLabel2Latency[IRLoop2LoopLabel[next_level_IRname]].clock_period;
                if (curLatency > max_latency)
                {
                    max_latency = curLatency;
                    slowest_sub_loop = next_level_IRname;
                }
            }
        }
        if (slowest_sub_loop == "")
            return cur_LoopIRName;
        assert(slowest_sub_loop!= "");
        return findTheSlowestLoop(slowest_sub_loop, IRLoop2LoopLabel, LoopLabel2Latency, LoopIRName2NextLevelSubLoopIRNames, LoopIRName2Depth);
    }
    else
    {
        if (LoopLabel2Latency.find(IRLoop2LoopLabel[cur_LoopIRName]) != LoopLabel2Latency.end())
        {
            return cur_LoopIRName;
        }
    }
    assert(false && "should not reach here");
}

std::vector<std::string> findSubLoopIRs(
                        std::string cur_LoopIRName,
                        std::map<std::string, std::string> &IRLoop2LoopLabel,
                        std::map<std::string, std::vector<std::string>> &LoopIRName2NextLevelSubLoopIRNames
                    )
{
    std::vector<std::string> res;
    res.push_back(cur_LoopIRName);
    for (auto next_level_IRname : LoopIRName2NextLevelSubLoopIRNames[cur_LoopIRName])
    {
        auto tmp_IRs = findSubLoopIRs(next_level_IRname, IRLoop2LoopLabel, LoopIRName2NextLevelSubLoopIRNames);
        for (auto IR_name : tmp_IRs)
            res.push_back(IR_name);
    }
    return res;
}


std::map<std::string, std::vector<int>> getPotentialLoopUnrollFactor(
                                            std::map<std::string, std::string> &IRLoop2LoopLabel,
                                            std::map<std::string, std::vector<std::string>> &LoopIRName2NextLevelSubLoopIRNames,
                                            std::map<std::string, int> &LoopIRName2Depth,
                                            std::map<std::string, int> &IRLoop2OriginTripCount
                                        )
{
    std::map<std::string, std::vector<int>> res_map;
    res_map.clear();
    
    std::string slowest_sub_loop = "";
    for (auto IRLoop_label_pair : IRLoop2LoopLabel)
    {
        std::vector<int> unrollFactorForLoop;
        int tripCount = IRLoop2OriginTripCount[IRLoop_label_pair.first];
        for (int i=2; i<=32 && i<=tripCount; i++)
        {
            if (isPowerOf2_32(i) || tripCount==i )//|| tripCount%i==0 && i%2==0 )
            {
                unrollFactorForLoop.push_back(i);
            }
        }
        res_map[IRLoop_label_pair.first]=unrollFactorForLoop;
    }
    return res_map;
}

int findNextUnrollFactor(   std::string IRLoopName,
                            int cur_loopUnrollFactor,
                            std::map<std::string, std::vector<int>> &loop2PotentialUnrollFactors
                         )
{
    for (auto nextFactor : loop2PotentialUnrollFactors[IRLoopName])
    {
        if (nextFactor > cur_loopUnrollFactor)
        {
            return nextFactor;
        }
    }
    return -1;
}


std::string  getSlowestModuleInDataflow(std::map<std::string, HI_WithDirectiveTimingResourceEvaluation::timingBase> FuncName2Latency, 
                                        std::map<std::string, HI_WithDirectiveTimingResourceEvaluation::resourceBase> FuncName2Resource,
                                        std::string topFunction)
{
    double latency = -1;
    std::string res_funcName = "";
    for (auto it : FuncName2Latency)
    {
        if (demangleFunctionName(it.first) == topFunction && it.first.find(".") == std::string::npos)
            continue;
        if (it.second.latency * it.second.clock_period > latency)
        {
            latency = it.second.latency * it.second.clock_period;
            res_funcName = it.first;
        }
    }
    assert(res_funcName!="" && "slowest subfunction should be found.");
    return res_funcName;
}


bool singleClockDesignEvaluation(
                      HI_DesignConfigInfo desginconfig_0,
                      llvm::raw_fd_ostream &checkedConfigs, 
                      llvm::Module &Mod, 
                      designEvalResult &design_eval_result,
                      int &output_next_factor,
                      std::string &output_next_loopLabel,

                      std::map<std::string, std::string> &IRLoop2LoopLabel, 
                      std::map<std::string, std::vector<int>> &IRFunc2BeginLine,
                      std::map<std::string, int> &FuncParamLine2OutermostSize,
                      std::map<std::string, std::vector<int>> &Loop2PotentialUnrollFactors,
                      std::map<std::string, std::set<std::pair<std::string, std::pair<std::string, int>> >> &LoopLabel2DrivenArrayDimensions,
                      std::map<std::string, int> &IRLoop2OriginTripCount,
                      std::map<std::string, std::string> &TopLoopIR2FuncName,
                      std::map<std::string, std::vector<std::string>> &LoopIRName2NextLevelSubLoopIRNames,
                      std::map<std::string, int> &LoopIRName2Depth,
                      std::map<std::string, std::vector< std::pair<std::string, std::string>>> &LoopIRName2Array,
                      std::map<std::string,Info_type_list> &BiOp_Info_name2list_map,
                      int config_id,
                      std::string top_str,
                      bool debugFlag
                     )
{
    std::error_code EC;
    

    checkedConfigs << "config #" << config_id << ":\n" << desginconfig_0;
    checkedConfigs.flush();
    legacy::PassManager PM0, PM1, PM2, PM3, PM4, PM5;
    
    LLVMTargetRef T;
    ModulePassManager MPM;

    std::unique_ptr<llvm::Module> Mod_tmp = CloneModule(Mod);
    std::string cntStr = std::to_string(config_id);


    std::map<std::string, int> LoopLabel2UnrollFactor;
    std::map<std::string, int> LoopLabel2II;
    std::map<std::string, int> IRLoop2OriginTripCount_eval;

    for (auto loopUnroll_pair : desginconfig_0.loopUnrollConfigs)
    {
        LoopLabel2UnrollFactor[loopUnroll_pair.first] = loopUnroll_pair.second;
    }



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

    PM0.run(*Mod_tmp);

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
        WriteBitcodeToFile(*Mod_tmp, OSPM0);
        OSPM0.flush();
        OSPM0.close();
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

    PM1.run(*Mod_tmp);

    if (debugFlag)
    {
        llvm::raw_fd_ostream OSPM1("top_output_PM1.bc", EC, llvm::sys::fs::F_None);
        WriteBitcodeToFile(*Mod_tmp, OSPM1);
        OSPM1.flush();
        OSPM1.close();
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


    PM2.run(*Mod_tmp);

    if (debugFlag)
    {
        llvm::raw_fd_ostream OSPM2("top_output_PM2.bc", EC, llvm::sys::fs::F_None);
        WriteBitcodeToFile(*Mod_tmp, OSPM2);
        OSPM2.flush();
        OSPM2.close();
    }


    std::map<std::string, std::string> IRLoop2LoopLabel_eval;
    
    // std::map<std::string, std::vector<int>> IRFunc2BeginLine_eval;
    auto hi_ir2sourcecode_eval = new HI_IR2SourceCode("HI_IR2SourceCode_eval",
                                                        IRLoop2LoopLabel_eval, 
                                                        IRFunc2BeginLine, 
                                                        IRLoop2OriginTripCount_eval, 
                                                        debugFlag);
    PM3.add(hi_ir2sourcecode_eval);
    // print_info("Enable HI_IR2SourceCode Pass");
    PM3.run(*Mod_tmp);


    bool unrollNotChanged = 1;


    legacy::PassManager PM6,PM7;
    std::unique_ptr<llvm::Module> Mod_tmp_eval = CloneModule(*Mod_tmp);
    for (auto loopPipeline_pair : desginconfig_0.loopPipelineConfigs)
    {
        LoopLabel2II[loopPipeline_pair.first] = loopPipeline_pair.second;
    }

    auto loopinfowrapperpass = new LoopInfoWrapperPass();
    PM6.add(loopinfowrapperpass);
    // print_info("Enable LoopInfoWrapperPass Pass");


    auto scalarevolutionwrapperpass = new ScalarEvolutionWrapperPass();
    PM6.add(scalarevolutionwrapperpass);
    // print_info("Enable ScalarEvolutionWrapperPass Pass");

    auto hi_MuxInsertionArrayPartition = new HI_MuxInsertionArrayPartition(
                                                    // configFile_str.c_str(),
                                                    desginconfig_0,
                                                    top_str.c_str(),
                                                    FuncParamLine2OutermostSize,
                                                    IRFunc2BeginLine,
                                                    debugFlag);
    // print_info("Enable HI_MuxInsertionArrayPartition Pass");
    PM6.add(hi_MuxInsertionArrayPartition); 


    print_status("Start LLVM processing");  
    PM6.run(*Mod_tmp_eval);
    print_status("Accomplished LLVM processing");


    std::string slow_LoopIRName = "";

    bool bypassEval = false;

    if (hi_MuxInsertionArrayPartition->muxWithMoreThan32)
    {
        print_warning("Too larget mux detected. Non-optimal, bypass evaluation.");
        slow_LoopIRName = hi_MuxInsertionArrayPartition->muxTooMuchLoopIRName;
        std::string opt_LoopLabel = IRLoop2LoopLabel[slow_LoopIRName];

        desginconfig_0.eraseLoopPipeline(opt_LoopLabel);
        
        int next_factor = findNextUnrollFactor(slow_LoopIRName, LoopLabel2UnrollFactor[opt_LoopLabel] ,Loop2PotentialUnrollFactors);
        llvm::errs() << "suggested next loop unroll factor for it is :" << next_factor << "\n";
        output_next_factor = next_factor;
        desginconfig_0.insertLoopUnroll(opt_LoopLabel, next_factor);
        output_next_loopLabel = opt_LoopLabel;
        
        for (auto loop_array_pair : LoopLabel2DrivenArrayDimensions[opt_LoopLabel])
        {
            std::string funcStr = loop_array_pair.first;
            std::string arrayStr = loop_array_pair.second.first;
            int dim = loop_array_pair.second.second;
            // std::map<std::string, std::set<std::pair<std::string, std::pair<std::string, int>> >>
            desginconfig_0.insertArrayCyclicPartition( demangleFunctionName(funcStr), arrayStr, dim, next_factor);
            
        }
        unrollNotChanged = 0;
        design_eval_result.success = 0;
        bypassEval = 1;
    }
    else
    {
        std::string logName_evaluation = "HI_WithDirectiveTimingResourceEvaluation_forCheck_"+cntStr ;
        std::string logName_array = "ArrayLog_forCheck_"+cntStr ;





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
        PM7.add(hi_withdirectivetimingresourceevaluation); 

        print_status("Start LLVM processing");  
        PM7.run(*Mod_tmp_eval);
        print_status("Accomplished LLVM processing");

        checkedConfigs << "\n";
        for (auto it : IRLoop2LoopLabel)
        {
            if (hi_withdirectivetimingresourceevaluation->LoopLabel2Latency.find( it.second) != hi_withdirectivetimingresourceevaluation->LoopLabel2Latency.end())
            {
                checkedConfigs << "    " << it.second 
                                << " ====> " << hi_withdirectivetimingresourceevaluation->LoopLabel2Latency[it.second] 
                                << "\n";
            }
            if (TopLoopIR2FuncName.find(it.first) != TopLoopIR2FuncName.end())
            {
                checkedConfigs << "    "  <<  it.second 
                                << " is top-level loop extracted in function: [" << TopLoopIR2FuncName[it.first] 
                                << "] with latency="                             << hi_withdirectivetimingresourceevaluation->FuncName2Latency[TopLoopIR2FuncName[it.first]] 
                                << " at clock_period="                           << hi_withdirectivetimingresourceevaluation->FuncName2Latency[TopLoopIR2FuncName[it.first]].clock_period << "ns"
                                << " with resource="                             << hi_withdirectivetimingresourceevaluation->FuncName2Resource[TopLoopIR2FuncName[it.first]]
                                << "\n";
            }
            checkedConfigs << "\n";
        }

        std::string slowestFuncName = getSlowestModuleInDataflow(hi_withdirectivetimingresourceevaluation->FuncName2Latency, hi_withdirectivetimingresourceevaluation->FuncName2Resource, top_str);

        checkedConfigs << "    Slowest Function : " << slowestFuncName  << "\n";
        
        checkedConfigs << "    Top Function Latency: " << hi_withdirectivetimingresourceevaluation->top_function_latency << " cycle\n\n";
        checkedConfigs << "    Top Function Delay: " << hi_withdirectivetimingresourceevaluation->top_function_latency*desginconfig_0.clock_period << " ns\n\n";
        checkedConfigs << "    Top Function Resource: " << hi_withdirectivetimingresourceevaluation->topFunction_resource << "\n\n";


        assert(hi_withdirectivetimingresourceevaluation->topFunctionFound && "The specified top function is not found in the program");

        
        std::string cmd_str;
        bool result = 1;

        if (debugFlag)
        {
            print_status("Writing LLVM IR to File");
            llvm::raw_fd_ostream OSPM6("top_output.bc", EC, llvm::sys::fs::F_None);
            WriteBitcodeToFile(*Mod_tmp_eval, OSPM6);
            OSPM6.flush();
            OSPM6.close();
            cmd_str = "llvm-dis top_output.bc 2>&1";
            print_cmd(cmd_str.c_str());
            result = sysexec(cmd_str.c_str());
            assert(result); // ensure the cmd is executed successfully
        }

        design_eval_result.FuncName2Latency = hi_withdirectivetimingresourceevaluation->FuncName2Latency;
        design_eval_result.FuncName2Resource = hi_withdirectivetimingresourceevaluation->FuncName2Resource;
        design_eval_result.LoopLabel2Latency = hi_withdirectivetimingresourceevaluation->LoopLabel2Latency;
        design_eval_result.LoopLabel2Resource = hi_withdirectivetimingresourceevaluation->LoopLabel2Resource;
        design_eval_result.LoopLabel2IterationLatency = hi_withdirectivetimingresourceevaluation->LoopLabel2IterationLatency;
        design_eval_result.LoopLabel2SmallestII = hi_withdirectivetimingresourceevaluation->LoopLabel2SmallestII;
        design_eval_result.LoopLabel2AchievedII = hi_withdirectivetimingresourceevaluation ->LoopLabel2AchievedII;
        design_eval_result.FuncArray2PartitionBenefit = hi_withdirectivetimingresourceevaluation->FuncArray2PartitionBenefit;
        design_eval_result.success = 1;
    }

    checkedConfigs.flush();

    Mod_tmp_eval.reset();

    Mod_tmp.reset();

    if (bypassEval)
        return false;

    return true;

}


bool multipleClockDesignEvaluation(
                      HI_DesignConfigInfo desginconfig_last,
                      llvm::raw_fd_ostream &checkedConfigs, 
                      llvm::Module &Mod, 

                      designEvalResult &entire_design_eval_result,
                      std::string &slowestFunctioName,
                      double &max_Latency,
                      double &second_max_Latency,
                      double &clock_forSlowestFunc,
                      HI_WithDirectiveTimingResourceEvaluation::resourceBase &ori_total_resource,
                      std::map<std::string, HI_WithDirectiveTimingResourceEvaluation::timingBase> &MultiClk_FuncName2Latency,
                      std::map<std::string, HI_WithDirectiveTimingResourceEvaluation::resourceBase> &MultiClk_FuncName2Resource,
                      int &output_next_factor,
                      std::string &output_next_loopLabel,

                      std::map<std::string, std::string> &clockForFunction,
                      std::vector<std::string> &FuncNames,
                      std::map<std::string, std::string> &IRLoop2LoopLabel, 
                      std::map<std::string, std::vector<int>> &IRFunc2BeginLine,
                      std::map<std::string, int> &FuncParamLine2OutermostSize,
                      std::map<std::string, std::vector<int>> &Loop2PotentialUnrollFactors,
                      std::map<std::string, std::set<std::pair<std::string, std::pair<std::string, int>> >> &LoopLabel2DrivenArrayDimensions,
                      std::map<std::string, int> &IRLoop2OriginTripCount,
                      std::map<std::string, std::string> &TopLoopIR2FuncName,
                      std::map<std::string, std::vector<std::string>> &LoopIRName2NextLevelSubLoopIRNames,
                      std::map<std::string, int> &LoopIRName2Depth,
                      std::map<std::string, std::vector< std::pair<std::string, std::string>>> &LoopIRName2Array,
                      std::map<std::string,Info_type_list> &BiOp_Info_name2list_map,


                      int config_id,
                      std::string top_str,
                      bool debugFlag
                     )
{

    resetResource(ori_total_resource);

    std::error_code EC;

    entire_design_eval_result.FuncName2Latency.clear();
    entire_design_eval_result.FuncName2Resource.clear();
    entire_design_eval_result.LoopLabel2Latency.clear();
    entire_design_eval_result.LoopLabel2Resource.clear();
    entire_design_eval_result.LoopLabel2SmallestII.clear();
    entire_design_eval_result.LoopLabel2IterationLatency.clear();
    entire_design_eval_result.LoopLabel2AchievedII.clear();

    

    for (auto funcName : FuncNames)
    {
        desginconfig_last.setClock(clockForFunction[funcName]);
    //    llvm::raw_fd_ostream checkedConfigs("configSettings.txt", EC, llvm::sys::fs::F_None);
        designEvalResult design_eval_result;
        bool result_bypass = singleClockDesignEvaluation(
                            desginconfig_last,
                            checkedConfigs, 
                            Mod, 
                            design_eval_result,
                            output_next_factor,
                            output_next_loopLabel,

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

        if (!design_eval_result.success)
            return false;
        
        llvm::errs() << "function [" << funcName << "] latency=" << design_eval_result.FuncName2Latency[funcName] << " resource=" << design_eval_result.FuncName2Resource[funcName] << "\n";
        if (design_eval_result.FuncName2Latency[funcName].latency * design_eval_result.FuncName2Latency[funcName].clock_period > max_Latency)
        {
            max_Latency = design_eval_result.FuncName2Latency[funcName].latency * design_eval_result.FuncName2Latency[funcName].clock_period;
            slowestFunctioName = funcName;
            clock_forSlowestFunc = design_eval_result.FuncName2Latency[funcName].clock_period;
        }
        MultiClk_FuncName2Latency[funcName] = design_eval_result.FuncName2Latency[funcName];
        MultiClk_FuncName2Resource[funcName] = design_eval_result.FuncName2Resource[funcName];

        utilizationViolation(MultiClk_FuncName2Resource[funcName]);
        
        entire_design_eval_result.FuncName2Latency[funcName] = design_eval_result.FuncName2Latency[funcName];
        entire_design_eval_result.FuncName2Resource[funcName] = design_eval_result.FuncName2Resource[funcName];
        for (auto it : TopLoopIR2FuncName)
        {
            if (it.second == funcName)
            {
                for (auto IR_Name : findSubLoopIRs(it.first, IRLoop2LoopLabel, LoopIRName2NextLevelSubLoopIRNames))
                {
                    // if (design_eval_result.LoopLabel2Latency.find(IRLoop2LoopLabel[IR_Name]) == design_eval_result.LoopLabel2Latency.end())
                    // {
                    //     llvm::errs() << " IR_Name=" << IR_Name << " IRLoop2LoopLabel[IR_Name]=" << IRLoop2LoopLabel[IR_Name] << "\n";
                    //     for (auto it : design_eval_result.LoopLabel2Latency)
                    //         llvm::errs() << "    " << it.first << "----" << it.second << "\n";
                    // }
                    // assert(design_eval_result.LoopLabel2Latency.find(IRLoop2LoopLabel[IR_Name]) != design_eval_result.LoopLabel2Latency.end());
                    // if (design_eval_result.LoopLabel2Resource.find(IRLoop2LoopLabel[IR_Name]) == design_eval_result.LoopLabel2Resource.end())
                    // {
                    //     llvm::errs() << " IR_Name=" << IR_Name << " IRLoop2LoopLabel[IR_Name]=" << IRLoop2LoopLabel[IR_Name] << "\n";
                    //     for (auto it : design_eval_result.LoopLabel2Resource)
                    //         llvm::errs() << "    " << it.first << "----" << it.second << "\n";
                    // }
                    // assert(design_eval_result.LoopLabel2Resource.find(IRLoop2LoopLabel[IR_Name]) != design_eval_result.LoopLabel2Resource.end());
                    // if (design_eval_result.LoopLabel2SmallestII.find(IRLoop2LoopLabel[IR_Name]) == design_eval_result.LoopLabel2SmallestII.end())
                    // {
                    //     llvm::errs() << " IR_Name=" << IR_Name << " IRLoop2LoopLabel[IR_Name]=" << IRLoop2LoopLabel[IR_Name] << "\n";
                    //     for (auto it : design_eval_result.LoopLabel2SmallestII)
                    //         llvm::errs() << "    " << it.first << "----" << it.second << "\n";
                    // }
                    // assert(design_eval_result.LoopLabel2SmallestII.find(IRLoop2LoopLabel[IR_Name]) != design_eval_result.LoopLabel2SmallestII.end());
                    // if (design_eval_result.LoopLabel2IterationLatency.find(IRLoop2LoopLabel[IR_Name]) == design_eval_result.LoopLabel2IterationLatency.end())
                    // {
                    //     llvm::errs() << " IR_Name=" << IR_Name << " IRLoop2LoopLabel[IR_Name]=" << IRLoop2LoopLabel[IR_Name] << "\n";
                    //     for (auto it : design_eval_result.LoopLabel2IterationLatency)
                    //         llvm::errs() << "    " << it.first << "----" << it.second << "\n";
                    // }
                    // assert(design_eval_result.LoopLabel2IterationLatency.find(IRLoop2LoopLabel[IR_Name]) != design_eval_result.LoopLabel2IterationLatency.end());
                    if (design_eval_result.LoopLabel2Resource.find(IRLoop2LoopLabel[IR_Name]) != design_eval_result.LoopLabel2Resource.end())
                    {
                        entire_design_eval_result.LoopLabel2Latency[IRLoop2LoopLabel[IR_Name]] = design_eval_result.LoopLabel2Latency[IRLoop2LoopLabel[IR_Name]];
                        entire_design_eval_result.LoopLabel2Resource[IRLoop2LoopLabel[IR_Name]] = design_eval_result.LoopLabel2Resource[IRLoop2LoopLabel[IR_Name]];
                    }
                    if (design_eval_result.LoopLabel2SmallestII.find(IRLoop2LoopLabel[IR_Name]) != design_eval_result.LoopLabel2SmallestII.end())
                        entire_design_eval_result.LoopLabel2SmallestII[IRLoop2LoopLabel[IR_Name]] = design_eval_result.LoopLabel2SmallestII[IRLoop2LoopLabel[IR_Name]];
                    if (design_eval_result.LoopLabel2IterationLatency.find(IRLoop2LoopLabel[IR_Name]) != design_eval_result.LoopLabel2IterationLatency.end())
                        entire_design_eval_result.LoopLabel2IterationLatency[IRLoop2LoopLabel[IR_Name]] = design_eval_result.LoopLabel2IterationLatency[IRLoop2LoopLabel[IR_Name]];
                    if ((design_eval_result.LoopLabel2AchievedII.find(IRLoop2LoopLabel[IR_Name]) != design_eval_result.LoopLabel2AchievedII.end()))
                        entire_design_eval_result.LoopLabel2AchievedII[IRLoop2LoopLabel[IR_Name]] = design_eval_result.LoopLabel2AchievedII[IRLoop2LoopLabel[IR_Name]];
                }                
            }
        }
        entire_design_eval_result.FuncArray2PartitionBenefit = design_eval_result.FuncArray2PartitionBenefit;

        accumulateResource(ori_total_resource,MultiClk_FuncName2Resource[funcName]);

        // get the BRAM cost of top function (whcich will not be changed even clock is changed)
        for (auto func_resource_pair : design_eval_result.FuncName2Resource)
        {
            if (demangleFunctionName(func_resource_pair.first) == top_str || func_resource_pair.first.find(".")==std::string::npos)
            {
                ori_total_resource.BRAM = func_resource_pair.second.BRAM;
                break;
            }
        }
    }
    checkedConfigs.flush();
    entire_design_eval_result.success = 1;

    llvm::errs() << "the max latency =" << max_Latency << " ( function [" << slowestFunctioName << "]\n";

    assert(slowestFunctioName != "");
    for (auto funcName : FuncNames)
    {
        if (funcName == slowestFunctioName)
            continue;
        if (MultiClk_FuncName2Latency[funcName].latency * MultiClk_FuncName2Latency[funcName].clock_period > second_max_Latency)
        {
            second_max_Latency = MultiClk_FuncName2Latency[funcName].latency * MultiClk_FuncName2Latency[funcName].clock_period;
        }
    }
    llvm::errs() << "the second max latency =" << second_max_Latency << "\n";
    return true;
}

std::string clockStrs_DSE[100]={
    "5",
    "7",
    "8",
    "9",
    "10",
    "12.5", 
    "15", // 7
    "16",
    "17.5", //
    "20", // 10
};
// std::string clockStrs_DSE[100]={
//     "5", // 0
//     "6", // 1
//     "7", // 2
//     "8", // 3
//     "9", // 4
//     "10", // 5
//     "12.5", // 6
//     "15", // 7
//     "16", // 8
//     "17.5", // 9
//     "20", // 10
//     "25", // 11
//     "30" // 12
// };

// bool tryUpdateSlowestFuncClock(
//                       HI_DesignConfigInfo desginconfig_last,
//                       llvm::raw_fd_ostream &checkedConfigs, 
//                       llvm::Module &Mod, 

//                       std::string &slowestFunctioName,
//                       double &max_Latency,
//                       double &second_max_Latency,
//                       double &clock_forSlowestFunc,
//                       HI_WithDirectiveTimingResourceEvaluation::resourceBase &ori_total_resource,
//                       HI_WithDirectiveTimingResourceEvaluation::resourceBase &newclk_total_resource,
//                       std::map<std::string, HI_WithDirectiveTimingResourceEvaluation::timingBase> &MultiClk_FuncName2Latency,
//                       std::map<std::string, HI_WithDirectiveTimingResourceEvaluation::resourceBase> &MultiClk_FuncName2Resource,
//                       std::string &optClock,
//                       HI_DesignConfigInfo & desginconfig_optClock,

//                       std::map<std::string, std::string> clockForFunction,
//                       std::vector<std::string> &FuncNames,
//                       std::map<std::string, std::string> &IRLoop2LoopLabel, 
//                       std::map<std::string, std::vector<int>> &IRFunc2BeginLine,
//                       std::map<std::string, int> &FuncParamLine2OutermostSize,
//                       std::map<std::string, std::vector<int>> &Loop2PotentialUnrollFactors,
//                       std::map<std::string, std::set<std::pair<std::string, std::pair<std::string, int>> >> &LoopLabel2DrivenArrayDimensions,
//                       std::map<std::string, int> &IRLoop2OriginTripCount,
//                       std::map<std::string, std::string> &TopLoopIR2FuncName,
//                       std::map<std::string, std::vector<std::string>> &LoopIRName2NextLevelSubLoopIRNames,
//                       std::map<std::string, int> &LoopIRName2Depth,
//                       std::map<std::string, std::vector< std::pair<std::string, std::string>>> &LoopIRName2Array,
//                       std::map<std::string,Info_type_list> &BiOp_Info_name2list_map,


//                       int config_id,
//                       std::string top_str,
//                       bool debugFlag
//                      )
// {

//     print_status("try to pushing clock setting.");
//     std::error_code EC;

//     resetResource(newclk_total_resource);
//     accumulateResource(newclk_total_resource, ori_total_resource);

//     HI_WithDirectiveTimingResourceEvaluation::resourceBase newclk_func_resource;
//     resetResource(newclk_func_resource);

//     HI_DesignConfigInfo desginconfig_new = desginconfig_last;
//     int curClock_id = getClockId(clock_forSlowestFunc, clockStrs_DSE) - 1;

//     desginconfig_optClock = desginconfig_last;

//     while (curClock_id>=0)
//     {
//         desginconfig_new.setClock(clockStrs_DSE[curClock_id]);
//        // llvm::raw_fd_ostream checkedConfigs("configSettings.txt", EC, llvm::sys::fs::F_None);
//         designEvalResult design_eval_result;
//         singleClockDesignEvaluation(
//                             desginconfig_new,
//                             checkedConfigs, 
//                             Mod, 
//                             design_eval_result,
//                             IRLoop2LoopLabel, 
//                             IRFunc2BeginLine,
//                             FuncParamLine2OutermostSize,
//                             Loop2PotentialUnrollFactors,
//                             LoopLabel2DrivenArrayDimensions,
//                             IRLoop2OriginTripCount,
//                             TopLoopIR2FuncName,
//                             LoopIRName2NextLevelSubLoopIRNames,
//                             LoopIRName2Depth,
//                             LoopIRName2Array,
//                             BiOp_Info_name2list_map,
//                             0,
//                             top_str,
//                             debugFlag
//                         );
//         checkedConfigs.flush();
//         llvm::errs() << "function [" << slowestFunctioName << "] latency=" << design_eval_result.FuncName2Latency[slowestFunctioName] << " resource=" << design_eval_result.FuncName2Resource[slowestFunctioName] << "\n";
//         if (design_eval_result.FuncName2Latency[slowestFunctioName].latency * design_eval_result.FuncName2Latency[slowestFunctioName].clock_period <= second_max_Latency + 0.001)
//         {
//             optClock = clockStrs_DSE[curClock_id];
//             for (auto it : design_eval_result.LoopLabel2AchievedII)
//             {
//                 desginconfig_optClock.insertLoopPipeline(it.first, it.second);
//             }
//             accumulateResource(newclk_func_resource, design_eval_result.FuncName2Resource[slowestFunctioName] );
//             llvm::errs() << "update clock for function [" << slowestFunctioName << "] to " <<  clockStrs_DSE[curClock_id] << " can resolve the bottleneck\n";
//             break;
//         }

//         curClock_id --;
//     }
    

//     if (curClock_id<0)
//     {
//         llvm::errs() << "tuning the clock frequency does not help function [" << slowestFunctioName << "]\n";
//         accumulateResource(newclk_total_resource, ori_total_resource);
//         return false;
//     }
//     else
//     {
        
//         deductResource(newclk_total_resource, MultiClk_FuncName2Resource[slowestFunctioName]);
//         accumulateResource(newclk_total_resource, newclk_func_resource);
//         return true;
//     }

// }


bool tryUpdateSlowestFuncClock(
                      HI_DesignConfigInfo desginconfig_last,
                      llvm::raw_fd_ostream &checkedConfigs, 
                      llvm::Module &Mod, 

                      std::string slowestFunctioName,
                      double ori_max_Latency,
                      double ori_second_max_Latency,
                      double clock_forSlowestFunc,
                      HI_WithDirectiveTimingResourceEvaluation::resourceBase ori_total_resource,
                      HI_WithDirectiveTimingResourceEvaluation::resourceBase &newclk_total_resource,
                      std::map<std::string, HI_WithDirectiveTimingResourceEvaluation::timingBase> &MultiClk_FuncName2Latency,
                      std::map<std::string, HI_WithDirectiveTimingResourceEvaluation::resourceBase> &MultiClk_FuncName2Resource,
                      std::string &optClock,
                      HI_DesignConfigInfo & desginconfig_optClock,

                      std::map<std::string, std::string> clockForFunction,
                      std::vector<std::string> &FuncNames,
                      std::map<std::string, std::string> &IRLoop2LoopLabel, 
                      std::map<std::string, std::vector<int>> &IRFunc2BeginLine,
                      std::map<std::string, int> &FuncParamLine2OutermostSize,
                      std::map<std::string, std::vector<int>> &Loop2PotentialUnrollFactors,
                      std::map<std::string, std::set<std::pair<std::string, std::pair<std::string, int>> >> &LoopLabel2DrivenArrayDimensions,
                      std::map<std::string, int> &IRLoop2OriginTripCount,
                      std::map<std::string, std::string> &TopLoopIR2FuncName,
                      std::map<std::string, std::vector<std::string>> &LoopIRName2NextLevelSubLoopIRNames,
                      std::map<std::string, int> &LoopIRName2Depth,
                      std::map<std::string, std::vector< std::pair<std::string, std::string>>> &LoopIRName2Array,
                      std::map<std::string,Info_type_list> &BiOp_Info_name2list_map,


                      int config_id,
                      std::string top_str,
                      bool debugFlag
                     )
{

    print_status("try to pushing clock setting.");
    std::error_code EC;
    

    HI_WithDirectiveTimingResourceEvaluation::resourceBase newclk_func_resource;
    resetResource(newclk_func_resource);

    HI_DesignConfigInfo desginconfig_new = desginconfig_last;
    int curClock_id = getClockId(clock_forSlowestFunc, clockStrs_DSE) - 1;

    
    std::vector<std::string> subLoopLabels;
    subLoopLabels.clear();
    for (auto it : TopLoopIR2FuncName)
    {
        if (it.second == slowestFunctioName)
        {
            for (auto IR_Name : findSubLoopIRs(it.first, IRLoop2LoopLabel, LoopIRName2NextLevelSubLoopIRNames))
                subLoopLabels.push_back(IRLoop2LoopLabel[IR_Name]);
            break;
        }
    }

    if (curClock_id==0)
    {
        int u=0;
    }

    
    while (curClock_id>=0)
    {
        desginconfig_optClock = desginconfig_last;
        clockForFunction[slowestFunctioName] = clockStrs_DSE[curClock_id];

        desginconfig_optClock.loopUnrollConfigs.clear();
        desginconfig_optClock.loopPipelineConfigs.clear();
        desginconfig_optClock.cyclicPartitionConfigs.clear();
        desginconfig_optClock.blockPartitionConfigs.clear();
        desginconfig_optClock.LoopLabel2UnrollFactor.clear();
       // llvm::raw_fd_ostream checkedConfigs("configSettings.txt", EC, llvm::sys::fs::F_None);


        resetResource(newclk_total_resource);
        double new_max_Latency;
        bool tryDirective = tryUpdateHLSDirectives(
                        desginconfig_optClock,
                        checkedConfigs, 
                        Mod, 

                        slowestFunctioName,
                        ori_max_Latency,
                        ori_second_max_Latency,
                        ori_total_resource,
                        newclk_total_resource,
                        desginconfig_optClock,
                        new_max_Latency,

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
        checkedConfigs.flush();



        llvm::errs() << "bottleneck latency=" << new_max_Latency << "\n";
        if (new_max_Latency <= ori_max_Latency)
        {
            optClock = clockStrs_DSE[curClock_id];
            llvm::errs() << "CAN update clock for function [" << slowestFunctioName << "] to " <<  clockStrs_DSE[curClock_id] << " can resolve the bottleneck\n";
            break;
        }

        curClock_id --;
    }
    

    if (curClock_id<0)
    {
        llvm::errs() << "tuning the clock frequency does not help function [" << slowestFunctioName << "]\n";
        return false;
    }
    else
    {
        return true;
    }

}

bool tryUpdateHLSDirectives(
                      HI_DesignConfigInfo desginconfig_last,
                      llvm::raw_fd_ostream &checkedConfigs, 
                      llvm::Module &Mod, 

                      std::string ori_slowestFunctioName,
                      double ori_max_Latency,
                      double ori_second_max_Latency,
                      HI_WithDirectiveTimingResourceEvaluation::resourceBase ori_total_resource,
                      HI_WithDirectiveTimingResourceEvaluation::resourceBase &newloop_total_resource,
                      HI_DesignConfigInfo &desginconfig_optLoop,
                      double &new_max_Latency,

                      std::map<std::string, std::string> &clockForFunction,
                      std::vector<std::string> &FuncNames,
                      std::map<std::string, std::string> &IRLoop2LoopLabel, 
                      std::map<std::string, std::vector<int>> &IRFunc2BeginLine,
                      std::map<std::string, int> &FuncParamLine2OutermostSize,
                      std::map<std::string, std::vector<int>> &Loop2PotentialUnrollFactors,
                      std::map<std::string, std::set<std::pair<std::string, std::pair<std::string, int>> >> &LoopLabel2DrivenArrayDimensions,
                      std::map<std::string, int> &IRLoop2OriginTripCount,
                      std::map<std::string, std::string> &TopLoopIR2FuncName,
                      std::map<std::string, std::vector<std::string>> &LoopIRName2NextLevelSubLoopIRNames,
                      std::map<std::string, int> &LoopIRName2Depth,
                      std::map<std::string, std::vector< std::pair<std::string, std::string>>> &LoopIRName2Array,
                      std::map<std::string,Info_type_list> &BiOp_Info_name2list_map,


                      int config_id,
                      std::string top_str,
                      bool debugFlag
                     )
{
    print_status("try to pushing HLS directive setting.");

    std::string continueUnrollLoop = "";
    bool HLSDirective_Updated = false;
    while (1)
    {
        std::error_code EC;

        designEvalResult entire_design_eval_result;

                        //   double &max_Latency,
                        //   double &second_max_Latency,
        double clock_forSlowestFunc;
                        //   HI_WithDirectiveTimingResourceEvaluation::resourceBase &ori_total_resource,
        std::map<std::string, HI_WithDirectiveTimingResourceEvaluation::timingBase> MultiClk_FuncName2Latency;
        std::map<std::string, HI_WithDirectiveTimingResourceEvaluation::resourceBase> MultiClk_FuncName2Resource;
        std::string slowestFunctioName = "";
        double second_max_Latency = -1;
        double max_Latency = -1;
        HI_WithDirectiveTimingResourceEvaluation::resourceBase total_resource;
        resetResource(newloop_total_resource);
        int output_next_factor;
        std::string output_next_loopLabel;

        bool eval_success = multipleClockDesignEvaluation(
                        desginconfig_last,
                        checkedConfigs, 
                        Mod, 
                        entire_design_eval_result,

                        slowestFunctioName,
                        max_Latency,
                        second_max_Latency,
                        clock_forSlowestFunc,
                        newloop_total_resource,
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

        std::string slow_LoopIRName;
        std::string opt_LoopLabel;
        checkedConfigs << "utilizationIndex(newloop_total_resource)=" << utilizationIndex(newloop_total_resource) << "\n";
        stringUtilizationIndexTo(newloop_total_resource, checkedConfigs);
        stringUtilizationTo(newloop_total_resource, checkedConfigs);
        checkedConfigs.flush();
        if (utilizationViolation(newloop_total_resource))
        {
            checkedConfigs << "loop utilization violation.\n";
            checkedConfigs.flush();
            print_warning("cannot resolve loop utilization violation\n");
            return false;
        }
        if (!eval_success)
        {
            print_warning( " try multipleClockDesignEvaluation failed, may be caused by Large Mux");
            opt_LoopLabel = output_next_loopLabel;
        }
        else
        {
            checkedConfigs << "cur_Latency=" << max_Latency << "   ori_max_Latency=" << ori_max_Latency << "\n";
            checkedConfigs << entire_design_eval_result;
        }
        checkedConfigs.flush();

        bool reRun = 0;
        for (auto pair : entire_design_eval_result.FuncArray2PartitionBenefit)
        {
            if (!pair.second) // no benefit
            {
                desginconfig_last.eraseArrayPartition(pair.first.first, pair.first.second);
                checkedConfigs << "Array [" << pair.first.second << "] in Func: [" << pair.first.first<< "] don't need partitioning. Re-run evaluation without partitioning. \n";
                reRun = 1;
                HLSDirective_Updated = 1;
            }
        }
        if (reRun)
        {
            continue;
        }


        if (eval_success)
        {
            // if (slowestFunctioName != ori_slowestFunctioName && max_Latency == ori_max_Latency || max_Latency < ori_max_Latency+0.01)
            // if (max_Latency < ori_max_Latency+0.01)
            // if (slowestFunctioName != ori_slowestFunctioName && max_Latency <= ori_max_Latency+0.01 || slowestFunctioName == ori_slowestFunctioName && max_Latency < ori_max_Latency)
            if (HLSDirective_Updated)
            {
                new_max_Latency = max_Latency;
                llvm::errs() << "CAN function ["  << ori_slowestFunctioName 
                            << " has been optimized to not bottle neck and new bottleneck funtion is [" << slowestFunctioName << "]\n";
                desginconfig_optLoop = desginconfig_last;
                return true;
            }

            assert(slowestFunctioName!="");

            slow_LoopIRName = findSlowestLastLevelLoop_InSlowestTopLoop(
                                                    slowestFunctioName,
                                                    TopLoopIR2FuncName,
                                                    IRLoop2LoopLabel,
                                                    entire_design_eval_result.LoopLabel2Latency, 
                                                    LoopIRName2NextLevelSubLoopIRNames,
                                                    LoopIRName2Depth
                );
                
            if (slow_LoopIRName=="")
            {
                print_warning("all loop are optimized!\n");
                return false;
            }
            assert(slow_LoopIRName!="");

            opt_LoopLabel = IRLoop2LoopLabel[slow_LoopIRName];
        }
        assert(opt_LoopLabel!="");
        

        
        llvm::errs() << "loop to optimized is :" << opt_LoopLabel << "\n";

        bool directUnroll = entire_design_eval_result.LoopLabel2IterationLatency[opt_LoopLabel]-1  == 0 
                            || !eval_success;
        if (!directUnroll && entire_design_eval_result.LoopLabel2AchievedII.find(opt_LoopLabel) == entire_design_eval_result.LoopLabel2AchievedII.end())
        {
            assert(entire_design_eval_result.LoopLabel2SmallestII.find(opt_LoopLabel) != entire_design_eval_result.LoopLabel2SmallestII.end());
            if (entire_design_eval_result.LoopLabel2SmallestII[opt_LoopLabel] > entire_design_eval_result.LoopLabel2IterationLatency[opt_LoopLabel] )
            {
                directUnroll = 1;
            }
        }
        if (!directUnroll && entire_design_eval_result.LoopLabel2AchievedII.find(opt_LoopLabel) == entire_design_eval_result.LoopLabel2AchievedII.end())
        {
            llvm::errs() << "for config#" << config_id+1 << " suggested start loop pipeline II for it is :" << entire_design_eval_result.LoopLabel2IterationLatency[opt_LoopLabel]-1 << "\n";

            if (entire_design_eval_result.LoopLabel2SmallestII[opt_LoopLabel] > entire_design_eval_result.LoopLabel2IterationLatency[opt_LoopLabel] )
            {
                llvm::errs() << "previous result:\n" << entire_design_eval_result << "\n";
                llvm::errs() << "previous desginconfig_last:\n" << desginconfig_last << "\n";
            }
            assert(entire_design_eval_result.LoopLabel2SmallestII[opt_LoopLabel] <= entire_design_eval_result.LoopLabel2IterationLatency[opt_LoopLabel] );
            desginconfig_last.insertLoopPipeline(opt_LoopLabel, entire_design_eval_result.LoopLabel2IterationLatency[opt_LoopLabel]);
            HLSDirective_Updated = true;
        }
        else
        {
            if (!directUnroll && entire_design_eval_result.LoopLabel2AchievedII[opt_LoopLabel] != entire_design_eval_result.LoopLabel2SmallestII[opt_LoopLabel])
            {
                llvm::errs()  << "LoopLabel2AchievedII=" << entire_design_eval_result.LoopLabel2AchievedII[opt_LoopLabel] 
                            << " LoopLabel2SmallestII=" << entire_design_eval_result.LoopLabel2SmallestII[opt_LoopLabel] << "\n";
                // llvm::errs()  << "for config#" << config_id+1 << "suggested next loop pipeline II for it is :" << (entire_design_eval_result.LoopLabel2AchievedII[opt_LoopLabel] + entire_design_eval_result.LoopLabel2SmallestII[opt_LoopLabel])/2 << "\n";
                llvm::errs()  << "for config#" << config_id+1 << "suggested next loop pipeline II for it is :" << (entire_design_eval_result.LoopLabel2AchievedII[opt_LoopLabel] - 1) << "\n";
                // desginconfig_last.insertLoopPipeline(opt_LoopLabel, (entire_design_eval_result.LoopLabel2AchievedII[opt_LoopLabel] + entire_design_eval_result.LoopLabel2SmallestII[opt_LoopLabel])/2);

                int dis_achieve_smallest = entire_design_eval_result.LoopLabel2AchievedII[opt_LoopLabel] - entire_design_eval_result.LoopLabel2SmallestII[opt_LoopLabel];
                if (dis_achieve_smallest>10)
                    desginconfig_last.insertLoopPipeline(opt_LoopLabel, (entire_design_eval_result.LoopLabel2AchievedII[opt_LoopLabel] - dis_achieve_smallest/5));
                else
                    desginconfig_last.insertLoopPipeline(opt_LoopLabel, (entire_design_eval_result.LoopLabel2AchievedII[opt_LoopLabel] - 1));

                HLSDirective_Updated = true;
            }
            else
            {

                desginconfig_last.eraseLoopPipeline(opt_LoopLabel);
                
                int next_factor;
                if (eval_success)
                    next_factor = findNextUnrollFactor(slow_LoopIRName, desginconfig_last.LoopLabel2UnrollFactor[opt_LoopLabel] ,Loop2PotentialUnrollFactors);
                else
                    next_factor = output_next_factor;

                if (next_factor<=0)
                {
                    print_warning("cannot resolve large mux\n");
                    return false;
                }

                assert(next_factor > 0);
                llvm::errs()  << "for config#" << config_id+1 << "suggested next loop unroll factor for it is :" << next_factor << "\n";
                desginconfig_last.insertLoopUnroll(opt_LoopLabel, next_factor);
                HLSDirective_Updated = true;
                
                for (auto loop_array_pair : LoopLabel2DrivenArrayDimensions[opt_LoopLabel])
                {
                    std::string funcStr = loop_array_pair.first;
                    std::string arrayStr = loop_array_pair.second.first;
                    int dim = loop_array_pair.second.second;
                    // std::map<std::string, std::set<std::pair<std::string, std::pair<std::string, int>> >>
                    desginconfig_last.increaseArrayCyclicPartition( demangleFunctionName(funcStr), arrayStr, dim, next_factor);
                    
                }
            }
        }

        continueUnrollLoop = slow_LoopIRName;
        
    }




    // if (unrollNotChanged)
    // {
    //     config_id++;
    //     checkedConfigs << "config #" << config_id << ":\n" << desginconfig_0;
    //     checkedConfigs.flush();
    // }
    assert(false && "should not reach here.");

}

std::string printArray(const HI_PragmaArrayInfo& tb)
{
    std::string output = "";
    output += "HI_PragmaArrayInfo for: << " + tb.targetName + ">> num_dims=" + std::to_string(tb.num_dims) + ", " ;
    for (int i = 0; i<tb.num_dims; i++)
    {
        if (tb.cyclic)
            output+= "dim-" + std::to_string(i) + "C-s" + std::to_string(tb.dim_size[i]) + "-p" + std::to_string(tb.partition_size[i]) + ",  ";
        else
            output+= "dim-" + std::to_string(i) + "B-s" + std::to_string(tb.dim_size[i]) + "-p" + std::to_string(tb.partition_size[i]) + ",  ";
    }

    // for (int i = 0; i<tb.num_dims; i++)
    // {
    //     stream << "dim-" << i << "-subnum=" << tb.sub_element_num[i] << ", ";
    // }


    output+= "] ";
    //timing="<<tb.timing<<"] ";
    return output;
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
                *genFile << "    possible array configuration for: " << printArray(TargetExtName2ArrayInfo[func_array_pair])<< "\n";
                std::string funcStr = func_array_pair.first;
                std::string arrayStr = func_array_pair.second;
                arrayPragmaGenerator array_configurationGen(TargetExtName2ArrayInfo[func_array_pair]);
                for (auto arrayCfg : array_configurationGen.getArrayConfigurations())
                {
                    *genFile << "        -----------> " << printArray(arrayCfg) << "\n"; 
                }
            }
            
            // std::vector<std::string>
            // generateLoopConfiguration(loopIRName_dep_pair.first, 0);

        }
    }
    genFile->flush();
    delete genFile;
}

#define _clocknum 10

int getClockId(double clock_period, std::string clockStrs[100])
{
    for (int i=0; i<_clocknum;i++)
    {
        if (std::stod(clockStrs[i])-0.001 < clock_period && std::stod(clockStrs[i])+0.001 > clock_period )
        {
            return i;
        }
    }
    return -1;
}

void generateClockFunctionMaps(std::vector<int> &clockIds, std::vector<std::string> &funcNames, std::vector<std::map<std::string, std::string>> &clockFunctionMaps)
{
    if (funcNames.size()==clockIds.size())
    {
        std::map<std::string, std::string> tmp_map;
        int cnt = 0;
        for (auto funcname : funcNames)
        {
            tmp_map[funcname] = clockStrs_DSE[clockIds[cnt]];
            cnt++;
        }
        clockFunctionMaps.push_back(tmp_map);
        return;
    }
    for (int i=0; i<_clocknum;i++)
    {
        clockIds.push_back(i);
        generateClockFunctionMaps(clockIds, funcNames, clockFunctionMaps);
        clockIds.pop_back();
    }
    return;
}




raw_ostream& operator<< (raw_ostream& stream, designEvalResult& tb)
{
    stream << "designEvalResult:\n";
    for (auto it : tb.FuncName2Latency)
    {
        std::string funcName = it.first ;
        stream <<  "        func:" << funcName
               << " latency=" << it.second 
               << " resource=" << tb.FuncName2Resource[funcName] << "\n";
    }
    for (auto it : tb.LoopLabel2Latency)
    {
        std::string loopLabel = it.first;
        if (tb.LoopLabel2AchievedII.find(loopLabel)!= tb.LoopLabel2AchievedII.end())
            stream <<  "        Loop:" << loopLabel  << "\n"
                << "                Iterlatency=" <<  tb.LoopLabel2IterationLatency[loopLabel]  << "\n"
                << "                Totlatency=" <<  tb.LoopLabel2Latency[loopLabel] << "\n"
                << "                Resource=" << tb.LoopLabel2Resource[it.first] << "\n"
                << "                AchievedII=" << tb.LoopLabel2AchievedII[it.first] << "\n"
                << "                SmallestII=" << tb.LoopLabel2SmallestII[it.first] << "\n";
        else
            stream <<  "        Loop:" << loopLabel  << "\n"
                << "                Iterlatency=" <<  tb.LoopLabel2IterationLatency[loopLabel]  << "\n"
                << "                Totlatency=" <<  tb.LoopLabel2Latency[loopLabel] << "\n"
                << "                Resource=" << tb.LoopLabel2Resource[it.first] << "\n"
                << "                AchievedII=" << "  unpipelined" << "\n"
                << "                SmallestII=" << tb.LoopLabel2SmallestII[it.first] << "\n";  
    }
    stream <<  "\n\n\n";
    return stream;
}




bool justTryUpdateSlowestFuncClock_withoutHLSDSE(
                      HI_DesignConfigInfo desginconfig_last,
                      llvm::raw_fd_ostream &checkedConfigs, 
                      llvm::Module &Mod, 

                      std::string slowestFunctioName,
                      double ori_max_Latency,
                      double ori_second_max_Latency,
                      double clock_forSlowestFunc,
                      HI_WithDirectiveTimingResourceEvaluation::resourceBase ori_total_resource,
                      HI_WithDirectiveTimingResourceEvaluation::resourceBase &newclk_total_resource,
                      std::map<std::string, HI_WithDirectiveTimingResourceEvaluation::timingBase> &MultiClk_FuncName2Latency,
                      std::map<std::string, HI_WithDirectiveTimingResourceEvaluation::resourceBase> &MultiClk_FuncName2Resource,
                      HI_DesignConfigInfo & desginconfig_optClock,

                      std::map<std::string, std::string> &clockForFunction,
                      std::vector<std::string> &FuncNames,
                      std::map<std::string, std::string> &IRLoop2LoopLabel, 
                      std::map<std::string, std::vector<int>> &IRFunc2BeginLine,
                      std::map<std::string, int> &FuncParamLine2OutermostSize,
                      std::map<std::string, std::vector<int>> &Loop2PotentialUnrollFactors,
                      std::map<std::string, std::set<std::pair<std::string, std::pair<std::string, int>> >> &LoopLabel2DrivenArrayDimensions,
                      std::map<std::string, int> &IRLoop2OriginTripCount,
                      std::map<std::string, std::string> &TopLoopIR2FuncName,
                      std::map<std::string, std::vector<std::string>> &LoopIRName2NextLevelSubLoopIRNames,
                      std::map<std::string, int> &LoopIRName2Depth,
                      std::map<std::string, std::vector< std::pair<std::string, std::string>>> &LoopIRName2Array,
                      std::map<std::string,Info_type_list> &BiOp_Info_name2list_map,


                      int config_id,
                      std::string top_str,
                      bool debugFlag
                     )
{

    print_status("try to pushing clock setting.");
    std::error_code EC;
    

    HI_WithDirectiveTimingResourceEvaluation::resourceBase newclk_func_resource;
    resetResource(newclk_func_resource);

    HI_DesignConfigInfo desginconfig_new = desginconfig_last;
    
    std::vector<std::string> subLoopLabels;
    subLoopLabels.clear();
    for (auto it : TopLoopIR2FuncName)
    {
        if (it.second == slowestFunctioName)
        {
            for (auto IR_Name : findSubLoopIRs(it.first, IRLoop2LoopLabel, LoopIRName2NextLevelSubLoopIRNames))
                subLoopLabels.push_back(IRLoop2LoopLabel[IR_Name]);
            break;
        }
    }


    designEvalResult entire_design_eval_result;
    desginconfig_optClock = desginconfig_last;



    if (!findNextPossibleClockCombination(slowestFunctioName, clockForFunction, MultiClk_FuncName2Latency, checkedConfigs))
    {
        llvm::errs() << "failed to tune the clock frequency to help function [" << slowestFunctioName << "]\n";
        return false;
    }

    desginconfig_optClock.loopUnrollConfigs.clear();
    desginconfig_optClock.loopPipelineConfigs.clear();
    desginconfig_optClock.cyclicPartitionConfigs.clear();
    desginconfig_optClock.blockPartitionConfigs.clear();
    desginconfig_optClock.LoopLabel2UnrollFactor.clear();
    // llvm::raw_fd_ostream checkedConfigs("configSettings.txt", EC, llvm::sys::fs::F_None);


    resetResource(newclk_total_resource);
    double new_max_Latency;
    double second_max_Latency;
    std::string output_next_loopLabel;
    int output_next_factor;
    bool tryDirective = true;

    bool eval_success = multipleClockDesignEvaluation(
                    desginconfig_last,
                    checkedConfigs, 
                    Mod, 
                    entire_design_eval_result,

                    slowestFunctioName,
                    new_max_Latency,
                    second_max_Latency,
                    clock_forSlowestFunc,
                    newclk_total_resource,
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
    checkedConfigs.flush();


    return true;

}

bool findNextPossibleClockCombination
    (
        std::string slowestFunctioName,
        std::map<std::string, std::string> &new_clockForFunction,
        std::map<std::string, HI_WithDirectiveTimingResourceEvaluation::timingBase> &MultiClk_FuncName2Latency,
        llvm::raw_fd_ostream &checkedConfigs
    )
{
    std::set<std::string> FuncNameForClockUpdate;
    for (auto func_xx_pair : MultiClk_FuncName2Latency)
        FuncNameForClockUpdate.insert(func_xx_pair.first);

    while (FuncNameForClockUpdate.size()>0)
    {
        double max_latency_in_set = -1;
        std::string funcName_withMaxLat = "";
        for (auto funcName : FuncNameForClockUpdate)
        {
            if (MultiClk_FuncName2Latency[funcName].clock_period * MultiClk_FuncName2Latency[funcName].latency > max_latency_in_set)
            {
                max_latency_in_set = MultiClk_FuncName2Latency[funcName].clock_period * MultiClk_FuncName2Latency[funcName].latency;
                funcName_withMaxLat = funcName;
            }
        }
        int ori_curClock_id = getClockId(MultiClk_FuncName2Latency[funcName_withMaxLat].clock_period, clockStrs_DSE);
        int curClock_id = ori_curClock_id-1;
        if (curClock_id<0)
        {
            llvm::errs() << "tuning the clock frequency does not help function [" << funcName_withMaxLat << "]\n";
            FuncNameForClockUpdate.erase(funcName_withMaxLat);
            continue;
        }

        std::map<std::string, std::string> tmp_clockForFunction = new_clockForFunction;

        std::set<std::string> clockSet;
        clockSet.clear();
        tmp_clockForFunction[funcName_withMaxLat] = clockStrs_DSE[curClock_id];

        for (auto func_clkstr_pair : tmp_clockForFunction)
        {
            clockSet.insert(func_clkstr_pair.second);
        }

        if (clockSet.size()>ClockNum_limit) // if the clock number exceeds the limitation, there is two options:
        {                                   // (1) find a lower clock for the slowest function (2) lower the clocks of those functions with the same clock with the original slowest function
            checkedConfigs << "     bypass clock [" << clockStrs_DSE[curClock_id] << "]  for function [" << funcName_withMaxLat << "] due to\n"
                            << " the clock number exceeding the limitation. Clocks are: ";
            for (auto func_clkstr_pair : tmp_clockForFunction)
            {
                checkedConfigs << "        func: [" << func_clkstr_pair.first << "] " 
                                << " clock=" << func_clkstr_pair.second << "ns\n";
            }

            // (1) find a lower clock for the slowest function 
            int tmp_curClock_id = curClock_id - 1;
            while (tmp_curClock_id>=0)
            {
                if (clockSet.find(clockStrs_DSE[tmp_curClock_id]) != clockSet.end())
                {
                    new_clockForFunction[funcName_withMaxLat] = clockStrs_DSE[tmp_curClock_id];
                    checkedConfigs << "     Successfully find the combination of clocks. Clocks are: ";
                    for (auto func_clkstr_pair : new_clockForFunction)
                    {
                        checkedConfigs << "        func: [" << func_clkstr_pair.first << "] " 
                                        << " clock=" << func_clkstr_pair.second << "ns\n";
                    }
                    return true;
                }
                tmp_curClock_id--;
            }

            // (2) lower the clocks of those functions with the same clock with the original slowest function
            std::vector<std::string> funcNames;
            for (auto func_clkstr_pair : tmp_clockForFunction)
            {
                funcNames.push_back(func_clkstr_pair.first);
            }
            for (auto funcName : funcNames)
            {
                if (new_clockForFunction[funcName] == clockStrs_DSE[ori_curClock_id])
                {
                    new_clockForFunction[funcName] =  clockStrs_DSE[ori_curClock_id-1];
                }
            }
            checkedConfigs << "     Successfully find the combination of clocks. Clocks are: ";
            for (auto func_clkstr_pair : new_clockForFunction)
            {
                checkedConfigs << "        func: [" << func_clkstr_pair.first << "] " 
                                << " clock=" << func_clkstr_pair.second << "ns\n";
            }
            return true;
        }
        else
        {
            new_clockForFunction = tmp_clockForFunction;
            return true;
        }
    }
    return false;
}

std::string printClocks(std::map<std::string, std::string> &new_clockForFunction)
{
    std::string output = "";
    output +="     Clocks are: ";
    for (auto func_clkstr_pair : new_clockForFunction)
    {
        output += "        func: [" + func_clkstr_pair.first + "] " 
                        + " clock=" + func_clkstr_pair.second + "ns\n";
    }
    return output;
}