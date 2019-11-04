#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Pass.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/raw_ostream.h"
#include "HI_print.h"
#include "HI_ArraySensitiveToLoopLevel.h"
#include "polly/PolyhedralInfo.h"

#include <stdio.h>
#include <string>
#include <ios>
#include <stdlib.h>
#include <sstream>
using namespace llvm;


bool HI_ArraySensitiveToLoopLevel::runOnModule(Module &M) // The runOnFunction declaration will overide the virtual one in ModulePass, which will be executed for each Function.
{    
    gettimeofday (&tv_begin, NULL);
    if (DEBUG) *Evaluating_log << " ======================= the module begin =======================\n";
    if (DEBUG) *Evaluating_log << M;
    if (DEBUG) *Evaluating_log << " ======================= the module end =======================\n";

    // analyze BRAM accesses in the module before any other analysis
    TraceMemoryDeclarationAndAnalyzeAccessinModule(M);

    // gettimeofday (&tv_end, NULL);
    // print_status("done HI_ArraySensitiveToLoopLevel TraceMemoryDeclarationAndAnalyzeAccessinModule: " 
    //     + std::to_string((double)(tv_end.tv_sec-tv_begin.tv_sec)+(double)(tv_end.tv_usec-tv_begin.tv_usec)/1000000.0) + " s");


    // analyze the timing and resource for the functions in the module
    // by recursively analysis from the lowest subfunctions
    AnalyzeFunctions(M);

    // gettimeofday (&tv_end, NULL);
    // print_status("done HI_ArraySensitiveToLoopLevel AnalyzeFunctions: " 
    //     + std::to_string((double)(tv_end.tv_sec-tv_begin.tv_sec)+(double)(tv_end.tv_usec-tv_begin.tv_usec)/1000000.0) + " s");


    // // analyze and output the informaion of top function
    // analyzeTopFunction(M);

    // gettimeofday (&tv_end, NULL);
    // print_status("done HI_ArraySensitiveToLoopLevel analyzeTopFunction: " 
    //     + std::to_string((double)(tv_end.tv_sec-tv_begin.tv_sec)+(double)(tv_end.tv_usec-tv_begin.tv_usec)/1000000.0) + " s");

    return false;
}

// check whether all the sub-function are evaluated
bool HI_ArraySensitiveToLoopLevel::CheckDependencyFesilility(Function &F)
{
    for (auto &B : F)
        for (auto &I : B)
            if (CallInst *CI = dyn_cast<CallInst>(&I))
            {
                if (FunctionChecked.find(CI->getCalledFunction()) == FunctionChecked.end())
                {
                    if (CI->getCalledFunction()->getName().find("llvm.")!=std::string::npos || CI->getCalledFunction()->getName().find("HIPartitionMux")!=std::string::npos)
                    {
                        FunctionChecked.insert(CI->getCalledFunction());                 
                    }
                    else
                        return false;
                }
            }
    return true;
}

char HI_ArraySensitiveToLoopLevel::ID = 0;  // the ID for pass should be initialized but the value does not matter, since LLVM uses the address of this variable as label instead of its value.

// introduce the dependence of Pass
void HI_ArraySensitiveToLoopLevel::getAnalysisUsage(AnalysisUsage &AU) const {

    AU.setPreservesAll();
    // AU.addRequired<AAResultsWrapperPass>(); 
    AU.addRequiredTransitive<LoopInfoWrapperPass>();
    AU.addRequiredTransitive<ScalarEvolutionWrapperPass>();
    // AU.addRequired<LoopAccessLegacyAnalysis>();
    // AU.addRequired<DominatorTreeWrapperPass>();
    // AU.addRequired<OptimizationRemarkEmitterWrapperPass>();
    
}



// analyze the timing and resource for the functions in the module
// by recursively analysis from the lowest subfunctions
void HI_ArraySensitiveToLoopLevel::AnalyzeFunctions(Module &M)
{

    for (auto &F : M)
    {   
        if (F.getName().find("llvm.")!=std::string::npos || F.getName().find("HIPartitionMux")!=std::string::npos)
        {
            FunctionChecked.insert(&F);     
            continue;             
        }
        else
        {
            if (DEBUG) *Evaluating_log << "CHECKING FUNCTION "<< F.getName() <<"\n";
            if (DEBUG) Evaluating_log->flush();
            // LAA = &getAnalysis<LoopAccessLegacyAnalysis>(F);
            LI = &getAnalysis<LoopInfoWrapperPass>(F).getLoopInfo();
            SE = &getAnalysis<ScalarEvolutionWrapperPass>(F).getSE(); 
            // AA = &getAnalysis<AAResultsWrapperPass>(F).getAAResults(); 
            ArrayAccessCheckForFunction(&F);
            FunctionChecked.insert(&F);
            // getLoopBlockMap(&F);
            // analyzeFunction(&F);
        }
    }
    return;

    
    bool all_processed = 0;
    while (!all_processed)
    {
        all_processed = 1;
        for (auto &F : M)
        {   

            if (FunctionChecked.find(&F) != FunctionChecked.end())
            {
                continue;
            }
            else
            {                
                if (F.getName().find("llvm.")!=std::string::npos || F.getName().find("HIPartitionMux")!=std::string::npos)
                {
                    FunctionChecked.insert(&F);     
                    continue;             
                }
                all_processed = 0;
                if (CheckDependencyFesilility(F))
                {
                    if (DEBUG) *Evaluating_log << "CHECKING FUNCTION "<< F.getName() <<"\n";
                    if (DEBUG) Evaluating_log->flush();
                    // LAA = &getAnalysis<LoopAccessLegacyAnalysis>(F);
                    LI = &getAnalysis<LoopInfoWrapperPass>(F).getLoopInfo();
                    SE = &getAnalysis<ScalarEvolutionWrapperPass>(F).getSE(); 
                    // AA = &getAnalysis<AAResultsWrapperPass>(F).getAAResults(); 
                    ArrayAccessCheckForFunction(&F);
                    FunctionChecked.insert(&F);
                    // getLoopBlockMap(&F);
                    // analyzeFunction(&F);
                }
            }
            
        }
    }
}

// analyze and output the informaion of top function
// void HI_ArraySensitiveToLoopLevel::analyzeTopFunction(Module &M)
// {
//     if (DEBUG) *Evaluating_log << "======================================\n                    analyze Top Function \n======================================\n";
    
//     int state_total_num = getTotalStateNum(M);
//     int LUT_needed_by_FSM = LUT_for_FSM(state_total_num);
//     if (DEBUG) *Evaluating_log << "total LUT for FSM: "<< LUT_needed_by_FSM << "\n";
//     int FF_needed_by_FSM = state_total_num;
//     for (auto &F : M)
//     {
//         std::string mangled_name = F.getName();
//         std::string demangled_name;
//         demangled_name = demangleFunctionName(mangled_name);
//         mangled_name = "find function " + mangled_name + "and its demangled name is : " + demangled_name;
// 	if (F.getName().find(".") == std::string::npos && F.getName().find("HIPartitionMux")  == std::string::npos)
//             print_info(mangled_name.c_str());

//         if (demangled_name == top_function_name && F.getName().find(".") == std::string::npos)
//         {
//             if (DEBUG) *Evaluating_log << "Top Function: "<< F.getName() <<" is found";
//             topFunctionFound = 1;
//             top_function_latency = analyzeFunction(&F).latency;
            
//             std::string printOut("");
//             if (DEBUG) *Evaluating_log << "total mux for BRAM: "<< BRAM_MUX_Evaluate() << "\n";
//             FunctionResource[&F] = FunctionResource[&F] + BRAM_MUX_Evaluate();
//             // print out the information of top function in terminal
//             printOut = "Done latency evaluation of top function: [" + demangled_name + "] and its latency is " + std::to_string(top_function_latency) 
//                                                                                     + " the state num is: " + std::to_string(state_total_num) 
//                                                                                     + " and its resource cost is [DSP=" + std::to_string(FunctionResource[&F].DSP) 
//                                                                                     + ", FF=" + std::to_string(FunctionResource[&F].FF+FF_needed_by_FSM) 
//                                                                                     + ", LUT=" + std::to_string(FunctionResource[&F].LUT +  LUT_needed_by_FSM) 
//                                                                                     + ", BRAM=" + std::to_string(FunctionResource[&F].BRAM ) 
//                                                                                     + "]";
//             if (DEBUG) *Evaluating_log << printOut << "\n";
//             print_info(printOut);
//         }
//     }
// }

// analyze BRAM accesses in the module before any other analysis
void HI_ArraySensitiveToLoopLevel::TraceMemoryDeclarationAndAnalyzeAccessinModule(Module &M)
{
    for (auto &F : M)
    {
        
        if (F.getName().find("llvm.")!=std::string::npos || F.getName().find("HIPartitionMux")!=std::string::npos) // bypass the "llvm.xxx" functions..
            continue;
        std::string mangled_name = F.getName();
        std::string demangled_name;
        demangled_name = demangleFunctionName(mangled_name);
        findMemoryDeclarationAndAnalyzeAccessin(&F, demangled_name == top_function_name && F.getName().find(".") == std::string::npos); 
        TraceMemoryAccessinFunction(F);

    }
}

// int HI_ArraySensitiveToLoopLevel::getTotalStateNum(Module &M)
// {

//     if (DEBUG)
//     {
//         if (DEBUG) *Evaluating_log << "================================\n              printing schedule \n==================================\n";
//         for (auto it : Inst_Schedule)
//         {
//             if (DEBUG) *Evaluating_log << "inst: [" << *it.first << "] in Block: [" << it.second.first ->getName() << "] #cycle: [" <<  it.second.second << "]\n";
//         }
//         if (DEBUG) Evaluating_log->flush();
//         if (DEBUG) *Evaluating_log << "================================\n              counting stage num \n==================================\n";
//     }

//     int state_total = 0;
//     for (auto &F : M)
//     {
//         if (F.getName().find("llvm.")!=std::string::npos || F.getName().find("HIPartitionMux")!=std::string::npos) // bypass the "llvm.xxx" functions..
//             continue;
//         BasicBlock *Func_Entry = &(F.getEntryBlock()); //get the entry of the function
//         timingBase origin_path_in_F(0,0,1,clock_period);
//         tmp_BlockCriticalPath_inFunc.clear(); // record the block level critical path in the loop
//         tmp_LoopCriticalPath_inFunc.clear(); // record the critical path to the end of sub-loops in the loop
//         Func_BlockVisited.clear();
//         state_total += getFunctionStageNum(origin_path_in_F, &F, Func_Entry) ; 
//     }
//     return state_total + 2;  // TODO: check +2 is for function or module (reset/idle)
// }


// Calculate the LUT for state
// based on regression model
int HI_ArraySensitiveToLoopLevel::LUT_for_FSM(int stateNum)
{
    double x = stateNum;
    double y = -0.44444444444444475*x*x+10.555555555555559*x-14.11111111111112;
    if (y < x*5)
    {
        y = x*5;
    }
    return round(y);
}

