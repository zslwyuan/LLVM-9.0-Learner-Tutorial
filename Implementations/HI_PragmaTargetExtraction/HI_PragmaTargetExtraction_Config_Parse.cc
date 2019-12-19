#include "HI_InstructionFiles.h"
#include "HI_PragmaTargetExtraction.h"
#include <fstream>
#include <string>
#include <sys/stat.h>
#include <unistd.h>

using namespace llvm;

// Pass for simple evluation of the latency of the top function, without considering HLS directives
void HI_PragmaTargetExtraction::Generate_Config()
{
    std::string tmp_s;
    std::string tmpStr_forParsing;
    while (getline(*config_file, tmp_s))
    {
        tmp_s = removeExtraSpace(tmp_s);
        std::stringstream iss(tmp_s);
        std::string param_name;
        iss >> param_name; //  get the name of parameter

        switch (hash_(param_name.c_str()))
        {
        case hash_compile_time("clock"):
            consumeEqual(iss);
            iss >> clock_period_str;
            clock_period = std::stod(clock_period_str);
            break;

        case hash_compile_time("HLS_lib_path"):
            consumeEqual(iss);
            iss >> HLS_lib_path;
            break;

            // case hash_compile_time("array_partition"):
            //     parseArrayPartition(iss);
            //     break;

            // case hash_compile_time("loop_pipeline"):
            //     parseLoopPipeline(iss);
            //     break;

        default:
            break;
        }
    }
    assert(HLS_lib_path != "" && "The HLS Lib is necessary in the configuration file!\n");
}

// generate the argument for array partitioning
void HI_PragmaTargetExtraction::generateArrayPartition(std::stringstream &iss)
{
    HI_PragmaInfo ans_pragma;
    ans_pragma.HI_PragmaInfoType = HI_PragmaInfo::arrayPartition_Pragma;
    while (!iss.eof())
    {
        std::string arg_name;
        std::string tmp_val;
        iss >> arg_name; //  get the name of parameter
        switch (hash_(arg_name.c_str()))
        {
        case hash_compile_time("variable"):
            consumeEqual(iss);
            iss >> tmp_val;
            ans_pragma.targetStr = (tmp_val);
            break;

        case hash_compile_time("scope"):
            consumeEqual(iss);
            iss >> tmp_val;
            ans_pragma.scopeStr = (tmp_val);
            break;

        case hash_compile_time("dim"):
            consumeEqual(iss);
            iss >> tmp_val;
            ans_pragma.dim =
                std::stoi(tmp_val) - 1; // count from dim="0" to match the storage format
            break;

        case hash_compile_time("factor"):
            consumeEqual(iss);
            iss >> tmp_val;
            ans_pragma.partition_factor = std::stoi(tmp_val);
            break;

        case hash_compile_time("cyclic"):
            ans_pragma.cyclic = 1;
            break;

        case hash_compile_time("block"):
            ans_pragma.cyclic = 0;
            break;

        default:
            print_error("Wrong argument for array partitioning.");
            break;
        }
    }
    PragmaInfo_List.push_back(ans_pragma);
    //
}

// parse the argument for loop pipelining
void HI_PragmaTargetExtraction::generateLoopPipeline(std::stringstream &iss)
{
    HI_PragmaInfo ans_pragma;
    ans_pragma.HI_PragmaInfoType = HI_PragmaInfo::loopPipeline_Pragma;
    while (!iss.eof())
    {
        std::string arg_name;
        std::string tmp_val;
        iss >> arg_name; //  get the name of parameter
        switch (hash_(arg_name.c_str()))
        {
        case hash_compile_time("label"):
            consumeEqual(iss);
            iss >> tmp_val;
            ans_pragma.labelStr = (tmp_val);
            break;

        case hash_compile_time("II"):
            consumeEqual(iss);
            iss >> tmp_val;
            ans_pragma.II = std::stoi(tmp_val);
            break;

        default:
            print_error("Wrong argument for loop pipelining.");
            break;
        }
    }
    PragmaInfo_List.push_back(ans_pragma);
}

// // match the configuration and the corresponding declaration of memory (array)
// void HI_PragmaTargetExtraction::matchArrayAndConfiguration(Value* target, HI_ArrayInfo
// &resArrayInfo)
// {
//     for (auto& pragma : PragmaInfo_List)
//     {
//         if (pragma.HI_PragmaInfoType == HI_PragmaInfo::arrayPartition_Pragma)
//         {
//             if (target->getName() == pragma.targetStr)
//             {
//                 Function *F = getFunctionOfValue(target);
//                 assert(F && "the parent function of the value should be found.");
//                 if (demangleFunctionName(F->getName()) != pragma.scopeStr )
//                 {
//                     continue;
//                 }
//                 assert(arrayDirectives.find(target)==arrayDirectives.end() && "The target should
//                 be not in arrayDirectives list.\n") ; pragma.targetArray = target;
//                 pragma.ScopeFunc = F;
//                 arrayDirectives[target].push_back(pragma);
//                 resArrayInfo.cyclic[pragma.dim] = pragma.cyclic;
//                 resArrayInfo.partition_size[pragma.dim] = pragma.partition_factor;
//             }
//         }
//     }
// }

// find which function the value is located in
Function *HI_PragmaTargetExtraction::getFunctionOfValue(Value *target)
{

    if (auto I = dyn_cast<Instruction>(target))
    {
        return I->getParent()->getParent();
    }
    else if (auto A = dyn_cast<Argument>(target))
    {
        return A->getParent();
    }
    else
    {
        return nullptr;
    }
}

// int HI_PragmaTargetExtraction::get_N_DSP(std::string opcode, int operand_bitwid , int
// res_bitwidth, std::string period)
// {
//     if (checkInfoAvailability( opcode, operand_bitwid , res_bitwidth, period))
//         return BiOp_Info_name2list_map[opcode][operand_bitwid][res_bitwidth][period].DSP;
//     BiOp_Info_name2list_map[opcode][operand_bitwid][res_bitwidth][period] =
//     checkInfo_HigherFreq(opcode, operand_bitwid , res_bitwidth, period); return
//     BiOp_Info_name2list_map[opcode][operand_bitwid][res_bitwidth][period].DSP; llvm::errs() <<
//     "inquirying : " << opcode << " -- " << operand_bitwid << " -- " << res_bitwidth << " -- " <<
//     period << " \n"; assert(false && "no such information in the database\n");
// }

// int HI_PragmaTargetExtraction::get_N_FF(std::string opcode, int operand_bitwid , int
// res_bitwidth, std::string period)
// {
//     if (checkInfoAvailability( opcode, operand_bitwid , res_bitwidth, period))
//         return BiOp_Info_name2list_map[opcode][operand_bitwid][res_bitwidth][period].FF;
//     BiOp_Info_name2list_map[opcode][operand_bitwid][res_bitwidth][period] =
//     checkInfo_HigherFreq(opcode, operand_bitwid , res_bitwidth, period); return
//     BiOp_Info_name2list_map[opcode][operand_bitwid][res_bitwidth][period].FF; llvm::errs() <<
//     "inquirying : " << opcode << " -- " << operand_bitwid << " -- " << res_bitwidth << " -- " <<
//     period << " \n"; assert(false && "no such information in the database\n");
// }

// int HI_PragmaTargetExtraction::get_N_LUT(std::string opcode, int operand_bitwid , int
// res_bitwidth, std::string period)
// {
//     if (checkInfoAvailability( opcode, operand_bitwid , res_bitwidth, period))
//         return BiOp_Info_name2list_map[opcode][operand_bitwid][res_bitwidth][period].LUT;
//     BiOp_Info_name2list_map[opcode][operand_bitwid][res_bitwidth][period] =
//     checkInfo_HigherFreq(opcode, operand_bitwid , res_bitwidth, period); return
//     BiOp_Info_name2list_map[opcode][operand_bitwid][res_bitwidth][period].LUT; llvm::errs() <<
//     "inquirying : " << opcode << " -- " << operand_bitwid << " -- " << res_bitwidth << " -- " <<
//     period << " \n"; assert(false && "no such information in the database\n");
// }

// int HI_PragmaTargetExtraction::get_N_Lat(std::string opcode, int operand_bitwid , int
// res_bitwidth, std::string period)
// {
//     if (checkInfoAvailability( opcode, operand_bitwid , res_bitwidth, period))
//         return BiOp_Info_name2list_map[opcode][operand_bitwid][res_bitwidth][period].Lat;
//     BiOp_Info_name2list_map[opcode][operand_bitwid][res_bitwidth][period] =
//     checkInfo_HigherFreq(opcode, operand_bitwid , res_bitwidth, period); return
//     BiOp_Info_name2list_map[opcode][operand_bitwid][res_bitwidth][period].Lat; llvm::errs() <<
//     "inquirying : " << opcode << " -- " << operand_bitwid << " -- " << res_bitwidth << " -- " <<
//     period << " \n"; assert(false && "no such information in the database\n");
// }

// double HI_PragmaTargetExtraction::get_N_Delay(std::string opcode, int operand_bitwid , int
// res_bitwidth, std::string period)
// {
//     if (checkInfoAvailability( opcode, operand_bitwid , res_bitwidth, period))
//         return BiOp_Info_name2list_map[opcode][operand_bitwid][res_bitwidth][period].delay;
//     BiOp_Info_name2list_map[opcode][operand_bitwid][res_bitwidth][period] =
//     checkInfo_HigherFreq(opcode, operand_bitwid , res_bitwidth, period); return
//     BiOp_Info_name2list_map[opcode][operand_bitwid][res_bitwidth][period].delay; llvm::errs() <<
//     "inquirying : " << opcode << " -- " << operand_bitwid << " -- " << res_bitwidth << " -- " <<
//     period << " \n"; assert(false && "no such information in the database\n");
// }
