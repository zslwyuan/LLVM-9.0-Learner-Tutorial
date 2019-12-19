#include "HI_ArraySensitiveToLoopLevel.h"
#include <fstream>
#include <string>
#include <sys/stat.h>
#include <unistd.h>

using namespace llvm;

// Pass for simple evluation of the latency of the top function, without considering HLS directives
void HI_ArraySensitiveToLoopLevel::Parse_Config()
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

        case hash_compile_time("array_partition"):
            parseArrayPartition(iss);
            break;

        case hash_compile_time("loop_pipeline"):
            parseLoopPipeline(iss);
            break;

        case hash_compile_time("loop_unroll"):
            parseLoopUnroll(iss);
            break;

        default:
            break;
        }
    }
    assert(HLS_lib_path != "" && "The HLS Lib is necessary in the configuration file!\n");
}

// Organize the information into timingBase after getting the information of a specific instruction,
// based on its opcode, operand_bitwidth, result_bitwidth and clock period.
HI_ArraySensitiveToLoopLevel::timingBase
HI_ArraySensitiveToLoopLevel::get_inst_TimingInfo_result(std::string opcode, int operand_bitwid,
                                                         int res_bitwidth, std::string period)
{
    timingBase result(0, 0, 1, clock_period);
    inst_timing_resource_info info =
        get_inst_info(opcode, operand_bitwid, res_bitwidth, clock_period_str);
    result.latency = info.Lat;
    result.timing = info.delay;
    result.II = info.II;
    // result.strict_timing = "store"==opcode;  // TODO: turn the strict timing option into library
    return result;
}

// Organize the information into resourceBase after getting the information of a specific
// instruction, based on its opcode, operand_bitwidth, result_bitwidth and clock period.
HI_ArraySensitiveToLoopLevel::resourceBase
HI_ArraySensitiveToLoopLevel::get_inst_ResourceInfo_result(std::string opcode, int operand_bitwid,
                                                           int res_bitwidth, std::string period)
{
    resourceBase result(0, 0, 0, clock_period);
    inst_timing_resource_info info =
        get_inst_info(opcode, operand_bitwid, res_bitwidth, clock_period_str);
    result.DSP = info.DSP;
    result.FF = info.FF;
    result.LUT = info.LUT;
    return result;
}

// get the information of a specific instruction, based on its opcode, operand_bitwidth,
// result_bitwidth and clock period
inst_timing_resource_info HI_ArraySensitiveToLoopLevel::get_inst_info(std::string opcode,
                                                                      int operand_bitwid,
                                                                      int res_bitwidth,
                                                                      std::string period)
{
    if (checkInfoAvailability(opcode, operand_bitwid, res_bitwidth, period))
        return BiOp_Info_name2list_map[opcode][operand_bitwid][res_bitwidth][period];
    if (!checkFreqProblem(opcode, operand_bitwid, res_bitwidth, period) && (operand_bitwid % 2))
    {
        inst_timing_resource_info info_A =
            get_inst_info(opcode, operand_bitwid + 1, res_bitwidth + 1, period);
        inst_timing_resource_info info_B =
            get_inst_info(opcode, operand_bitwid - 1, res_bitwidth - 1, period);
        inst_timing_resource_info tmp_info;

        tmp_info.DSP = (info_A.DSP + info_B.DSP + 1) / 2;
        tmp_info.FF = (info_A.FF + info_B.FF + 1) / 2;
        tmp_info.LUT = (info_A.LUT + info_B.LUT) / 2;
        tmp_info.Lat = (info_A.Lat + info_B.Lat) / 2;
        tmp_info.delay = (info_A.delay + info_B.delay) / 2;
        tmp_info.II = (info_A.II + info_B.II + 1) / 2;
        tmp_info.core_name = info_A.core_name;
        BiOp_Info_name2list_map[opcode][operand_bitwid][res_bitwidth][period] = tmp_info;
    }
    else
    {
        BiOp_Info_name2list_map[opcode][operand_bitwid][res_bitwidth][period] =
            checkInfo_HigherFreq(opcode, operand_bitwid, res_bitwidth, period);
    }

    return BiOp_Info_name2list_map[opcode][operand_bitwid][res_bitwidth][period];

    llvm::errs() << "inquirying : " << opcode << " -- " << operand_bitwid << " -- " << res_bitwidth
                 << " -- " << period << " \n";
    assert(false && "no such information in the database\n");
}

// check whether a specific information is in the database
bool HI_ArraySensitiveToLoopLevel::checkInfoAvailability(std::string opcode, int operand_bitwid,
                                                         int res_bitwidth, std::string period)
{
    if (BiOp_Info_name2list_map.find(opcode) != BiOp_Info_name2list_map.end())
    {
        if (BiOp_Info_name2list_map[opcode].find(operand_bitwid) !=
            BiOp_Info_name2list_map[opcode].end())
        {
            if (BiOp_Info_name2list_map[opcode][operand_bitwid].find(res_bitwidth) !=
                BiOp_Info_name2list_map[opcode][operand_bitwid].end())
            {
                if (BiOp_Info_name2list_map[opcode][operand_bitwid][res_bitwidth].find(period) !=
                    BiOp_Info_name2list_map[opcode][operand_bitwid][res_bitwidth].end())
                {
                    return true;
                }
                else
                {
                    // llvm::errs() << "not in
                    // BiOp_Info_name2list_map[opcode][operand_bitwid][res_bitwidth]\n";
                }
            }
            else
            {
                // llvm::errs() << "not in BiOp_Info_name2list_map[opcode][operand_bitwid]\n";
            }
        }
        else
        {
            // llvm::errs() << "not in BiOp_Info_name2list_map[opcode]\n";
        }
    }
    else
    {
        // llvm::errs() << "not in BiOp_Info_name2list_map\n";
    }

    return false;
}

// check whether we can infer the information by increasing the clock frequency
bool HI_ArraySensitiveToLoopLevel::checkFreqProblem(std::string opcode, int operand_bitwid,
                                                    int res_bitwidth, std::string period)
{
    if (BiOp_Info_name2list_map.find(opcode) != BiOp_Info_name2list_map.end())
    {
        if (BiOp_Info_name2list_map[opcode].find(operand_bitwid) !=
            BiOp_Info_name2list_map[opcode].end())
        {
            if (BiOp_Info_name2list_map[opcode][operand_bitwid].find(res_bitwidth) !=
                BiOp_Info_name2list_map[opcode][operand_bitwid].end())
            {
                return true;
            }
            else
            {
                // llvm::errs() << "not in BiOp_Info_name2list_map[opcode][operand_bitwid]\n";
            }
        }
        else
        {
            // llvm::errs() << "not in BiOp_Info_name2list_map[opcode]\n";
        }
    }
    else
    {
        // llvm::errs() << "not in BiOp_Info_name2list_map\n";
    }

    return false;
}

// if the information is not found in database, we may infer the information by increasing the clock
// frequency
inst_timing_resource_info HI_ArraySensitiveToLoopLevel::checkInfo_HigherFreq(std::string opcode,
                                                                             int operand_bitwid,
                                                                             int res_bitwidth,
                                                                             std::string period)
{
    int i;

    // locate the target clock in clock sequence
    for (i = clockNum - 1; i >= 0; i--)
        if (std::stof(clockStrs[i]) == std::stof(period))
        {
            if (checkInfoAvailability(opcode, operand_bitwid, res_bitwidth, clockStrs[i]))
            {
                BiOp_Info_name2list_map[opcode][operand_bitwid][res_bitwidth][period] =
                    BiOp_Info_name2list_map[opcode][operand_bitwid][res_bitwidth][clockStrs[i]];
                return BiOp_Info_name2list_map[opcode][operand_bitwid][res_bitwidth][clockStrs[i]];
            }
            break;
        }
    i--;
    if (i < 0)
        llvm::errs() << "inquirying : " << opcode << " -- " << operand_bitwid << " -- "
                     << res_bitwidth << " -- " << period << " \n";
    assert(i >= 0 && "The clock should be found.\n");

    // iterate to find available information in database
    for (; i >= 0; i--)
    {
        if (checkInfoAvailability(opcode, operand_bitwid, res_bitwidth, clockStrs[i]))
            return BiOp_Info_name2list_map[opcode][operand_bitwid][res_bitwidth][clockStrs[i]];
    }
    llvm::errs() << "inquirying : " << opcode << " -- " << operand_bitwid << " -- " << res_bitwidth
                 << " -- " << period << " \n";
    assert(false && "no such information in the database\n");
}

// parse the argument for array partitioning
void HI_ArraySensitiveToLoopLevel::parseArrayPartition(std::stringstream &iss)
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
            llvm::errs() << "wrong argument: " << arg_name << "\n";
            print_error("Wrong argument for array partitioning.");
            break;
        }
    }
    PragmaInfo_List.push_back(ans_pragma);
    //
}

// parse the argument for loop pipelining
void HI_ArraySensitiveToLoopLevel::parseLoopPipeline(std::stringstream &iss)
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
            llvm::errs() << "wrong argument: " << arg_name << "\n";
            if (ans_pragma.labelStr != "")
                llvm::errs() << " other info label=" << ans_pragma.labelStr << "\n";
            if (ans_pragma.II > 0)
                llvm::errs() << " other info II=" << ans_pragma.II << "\n";
            print_error("Wrong argument for loop pipelining.");
            break;
        }
    }
    PragmaInfo_List.push_back(ans_pragma);
}

// parse the argument for loop unrolling
void HI_ArraySensitiveToLoopLevel::parseLoopUnroll(std::stringstream &iss)
{
    HI_PragmaInfo ans_pragma;
    ans_pragma.HI_PragmaInfoType = HI_PragmaInfo::loopUnroll_Pragma;
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

        case hash_compile_time("factor"):
            consumeEqual(iss);
            iss >> tmp_val;
            ans_pragma.unroll_factor = std::stoi(tmp_val);
            break;

        default:
            print_error("Wrong argument for loop unrolling.");
            break;
        }
    }
    PragmaInfo_List.push_back(ans_pragma);
}

// match the configuration and the corresponding declaration of memory (array)
void HI_ArraySensitiveToLoopLevel::matchArrayAndConfiguration(Value *target,
                                                              HI_ArrayInfo &resArrayInfo)
{
    for (auto &pragma : PragmaInfo_List)
    {
        if (pragma.HI_PragmaInfoType == HI_PragmaInfo::arrayPartition_Pragma)
        {
            if (target->getName() == pragma.targetStr)
            {
                Function *F = getFunctionOfValue(target);
                std::string nameOfFunction = demangleFunctionName(F->getName());
                assert(F && "the parent function of the value should be found.");
                if (nameOfFunction != pragma.scopeStr)
                {
                    continue;
                }
                // assert(arrayDirectives.find(target)==arrayDirectives.end() && "The target should
                // be not in arrayDirectives list.\n") ;
                pragma.targetArray = target;
                pragma.ScopeFunc = F;
                arrayDirectives[target].push_back(pragma);
                resArrayInfo.cyclic[pragma.dim] = pragma.cyclic;
                resArrayInfo.partition_size[pragma.dim] = pragma.partition_factor;
            }
        }
    }
}

// find which function the value is located in
Function *HI_ArraySensitiveToLoopLevel::getFunctionOfValue(Value *target)
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

// int HI_ArraySensitiveToLoopLevel::get_N_DSP(std::string opcode, int operand_bitwid , int
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

// int HI_ArraySensitiveToLoopLevel::get_N_FF(std::string opcode, int operand_bitwid , int
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

// int HI_ArraySensitiveToLoopLevel::get_N_LUT(std::string opcode, int operand_bitwid , int
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

// int HI_ArraySensitiveToLoopLevel::get_N_Lat(std::string opcode, int operand_bitwid , int
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

// double HI_ArraySensitiveToLoopLevel::get_N_Delay(std::string opcode, int operand_bitwid , int
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
