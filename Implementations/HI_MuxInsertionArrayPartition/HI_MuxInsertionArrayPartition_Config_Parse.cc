#include "HI_InstructionFiles.h"
#include "HI_MuxInsertionArrayPartition.h"
#include <fstream>
#include <string>
#include <sys/stat.h>
#include <unistd.h>

using namespace llvm;

// Pass for simple evluation of the latency of the top function, without considering HLS directives
void HI_MuxInsertionArrayPartition::Parse_Config()
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

        case hash_compile_time("array_port"):
            parseArrayPortNum(iss);
            break;

        case hash_compile_time("func_dataflow"):
            parseFuncDataflow(iss);
            break;

        case hash_compile_time("local_array"):
            parseLocalArray(iss);
            break;

        default:
            break;
        }
    }
    assert(HLS_lib_path != "" && "The HLS Lib is necessary in the configuration file!\n");
}

// Pass for simple evluation of the latency of the top function, without considering HLS directives
void HI_MuxInsertionArrayPartition::Parse_Config(const HI_DesignConfigInfo &configInfo)
{
    std::string tmp_s;
    std::string tmpStr_forParsing;

    // get clock
    clock_period_str = configInfo.clock_period_str;
    clock_period = std::stod(clock_period_str);
    HLS_lib_path = configInfo.HLS_lib_path;

    // get loop unroll configs
    for (auto loop_unroll_pair : configInfo.loopUnrollConfigs)
    {
        HI_PragmaInfo ans_pragma;
        ans_pragma.HI_PragmaInfoType = HI_PragmaInfo::loopUnroll_Pragma;
        ans_pragma.labelStr = loop_unroll_pair.first;
        ans_pragma.unroll_factor = loop_unroll_pair.second;
        PragmaInfo_List.push_back(ans_pragma);
    }

    // get loop pipeline configs
    for (auto loop_pipeline_pair : configInfo.loopPipelineConfigs)
    {
        HI_PragmaInfo ans_pragma;
        ans_pragma.HI_PragmaInfoType = HI_PragmaInfo::loopPipeline_Pragma;
        ans_pragma.labelStr = loop_pipeline_pair.first;
        ans_pragma.II = loop_pipeline_pair.second;
        PragmaInfo_List.push_back(ans_pragma);
    }

    // get array cyclic partition configs
    for (auto cyclic_partition_pair : configInfo.cyclicPartitionConfigs)
    {
        // (std::string functionName, std::string arrayName, int dim, int factor)
        HI_PragmaInfo ans_pragma;
        ans_pragma.HI_PragmaInfoType = HI_PragmaInfo::arrayPartition_Pragma;
        ans_pragma.scopeStr = cyclic_partition_pair.first;
        ans_pragma.targetStr = cyclic_partition_pair.second.first;
        ans_pragma.dim = cyclic_partition_pair.second.second.first - 1;
        ans_pragma.partition_factor = cyclic_partition_pair.second.second.second;
        ans_pragma.cyclic = 1;
        PragmaInfo_List.push_back(ans_pragma);
    }

    // get array block partition configs
    for (auto block_partition_pair : configInfo.blockPartitionConfigs)
    {
        // (std::string functionName, std::string arrayName, int dim, int factor)
        HI_PragmaInfo ans_pragma;
        ans_pragma.HI_PragmaInfoType = HI_PragmaInfo::arrayPartition_Pragma;
        ans_pragma.scopeStr = block_partition_pair.first;
        ans_pragma.targetStr = block_partition_pair.second.first;
        ans_pragma.dim = block_partition_pair.second.second.first - 1;
        ans_pragma.partition_factor = block_partition_pair.second.second.second;
        ans_pragma.cyclic = 0;
        PragmaInfo_List.push_back(ans_pragma);
    }

    // get array complete partition configs
    for (auto complete_partition_pair : configInfo.completePartitionConfigs)
    {
        // (std::string functionName, std::string arrayName, int dim, int factor)
        HI_PragmaInfo ans_pragma;
        ans_pragma.HI_PragmaInfoType = HI_PragmaInfo::arrayPartition_Pragma;
        ans_pragma.scopeStr = complete_partition_pair.first;
        ans_pragma.targetStr = complete_partition_pair.second.first;
        ans_pragma.dim = complete_partition_pair.second.second - 1;
        ans_pragma.cyclic = 1;
        ans_pragma.complete = 1;
        PragmaInfo_List.push_back(ans_pragma);
    }

    // get array port configs
    for (auto port_num_pair : configInfo.arrayPortConfigs)
    {
        HI_PragmaInfo ans_pragma;
        ans_pragma.HI_PragmaInfoType = HI_PragmaInfo::arrayPortNum_Pragma;
        ans_pragma.scopeStr = port_num_pair.first;
        ans_pragma.targetStr = port_num_pair.second.first;
        ans_pragma.port_num = port_num_pair.second.second;
        PragmaInfo_List.push_back(ans_pragma);
    }

    // get function dataflow configs
    for (auto func_dataflow_pair : configInfo.funcDataflowConfigs)
    {
        HI_PragmaInfo ans_pragma;
        ans_pragma.HI_PragmaInfoType = HI_PragmaInfo::funcDataflow_Pragma;
        ans_pragma.scopeStr = func_dataflow_pair.first;
        ans_pragma.dataflowEnable = func_dataflow_pair.second;
        PragmaInfo_List.push_back(ans_pragma);
    }

    // get local array configs
    for (auto local_set_pair : configInfo.localArrayConfigs)
    {
        HI_PragmaInfo ans_pragma;
        ans_pragma.HI_PragmaInfoType = HI_PragmaInfo::localArray_Pragma;
        ans_pragma.scopeStr = local_set_pair.first;
        ans_pragma.targetStr = local_set_pair.second.first;
        ans_pragma.localArrayEnable = local_set_pair.second.second;
        PragmaInfo_List.push_back(ans_pragma);
    }

    assert(HLS_lib_path != "" && "The HLS Lib is necessary in the configuration file!\n");
}

// parse the argument for array partitioning
void HI_MuxInsertionArrayPartition::parseArrayPartition(std::stringstream &iss)
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

        case hash_compile_time("complete"):
            ans_pragma.complete = 1;
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

// parse the argument for  array port number setting
void HI_MuxInsertionArrayPartition::parseArrayPortNum(std::stringstream &iss)
{
    HI_PragmaInfo ans_pragma;
    ans_pragma.HI_PragmaInfoType = HI_PragmaInfo::arrayPortNum_Pragma;
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

        case hash_compile_time("port_num"):
            consumeEqual(iss);
            iss >> tmp_val;
            ans_pragma.port_num =
                std::stoi(tmp_val); // count from dim="0" to match the storage format
            break;

        default:
            llvm::errs() << "wrong argument: " << arg_name << "\n";
            print_error("Wrong argument for array port num setting.");
            break;
        }
    }
    PragmaInfo_List.push_back(ans_pragma);
    //
}

// parse the argument for  function dataflow
void HI_MuxInsertionArrayPartition::parseFuncDataflow(std::stringstream &iss)
{
    HI_PragmaInfo ans_pragma;
    ans_pragma.HI_PragmaInfoType = HI_PragmaInfo::funcDataflow_Pragma;
    while (!iss.eof())
    {
        std::string arg_name;
        std::string tmp_val;
        iss >> arg_name; //  get the name of parameter
        switch (hash_(arg_name.c_str()))
        {
        case hash_compile_time("scope"):
            consumeEqual(iss);
            iss >> tmp_val;
            ans_pragma.scopeStr = (tmp_val);
            break;

        case hash_compile_time("enable"):
            ans_pragma.dataflowEnable = true;
            break;

        case hash_compile_time("disable"):
            ans_pragma.dataflowEnable = false;
            break;

        default:
            llvm::errs() << "wrong argument: " << arg_name << "\n";
            print_error("Wrong argument for function dataflow setting.");
            break;
        }
    }
    PragmaInfo_List.push_back(ans_pragma);
}

// parse the argument for local array setting
void HI_MuxInsertionArrayPartition::parseLocalArray(std::stringstream &iss)
{
    HI_PragmaInfo ans_pragma;
    ans_pragma.HI_PragmaInfoType = HI_PragmaInfo::localArray_Pragma;
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

        case hash_compile_time("enable"):
            ans_pragma.localArrayEnable = true;
            break;

        case hash_compile_time("disable"):
            ans_pragma.localArrayEnable = true;
            break;

        default:
            llvm::errs() << "wrong argument: " << arg_name << "\n";
            print_error("Wrong argument for local array setting.");
            break;
        }
    }
    PragmaInfo_List.push_back(ans_pragma);
    //
}

// parse the argument for loop pipelining
void HI_MuxInsertionArrayPartition::parseLoopPipeline(std::stringstream &iss)
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
            print_error("Wrong argument for loop pipelining.");
            break;
        }
    }
    PragmaInfo_List.push_back(ans_pragma);
}

// parse the argument for loop unrolling
void HI_MuxInsertionArrayPartition::parseLoopUnroll(std::stringstream &iss)
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
void HI_MuxInsertionArrayPartition::matchArrayAndConfiguration(Value *target,
                                                               HI_ArrayInfo &resArrayInfo)
{
    // by default, we set the port number for BRAM to 1 (single port BRAM)
    resArrayInfo.port_num = 1;
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
                pragma.targetArray = target;
                pragma.ScopeFunc = F;
                arrayDirectives[target].push_back(pragma);
                resArrayInfo.cyclic[pragma.dim] = pragma.cyclic;
                if (pragma.complete)
                {
                    resArrayInfo.partition_size[pragma.dim] = resArrayInfo.dim_size[pragma.dim];
                    resArrayInfo.completePartition = 1;
                }
                else
                    resArrayInfo.partition_size[pragma.dim] = pragma.partition_factor;
            }
        }
        else if (pragma.HI_PragmaInfoType == HI_PragmaInfo::arrayPortNum_Pragma)
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
                pragma.targetArray = target;
                pragma.ScopeFunc = F;
                arrayDirectives[target].push_back(pragma);
                resArrayInfo.port_num = pragma.port_num;
            }
        }
    }
}

// find which function the value is located in
Function *HI_MuxInsertionArrayPartition::getFunctionOfValue(Value *target)
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

// int HI_MuxInsertionArrayPartition::get_N_DSP(std::string opcode, int operand_bitwid , int
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

// int HI_MuxInsertionArrayPartition::get_N_FF(std::string opcode, int operand_bitwid , int
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

// int HI_MuxInsertionArrayPartition::get_N_LUT(std::string opcode, int operand_bitwid , int
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

// int HI_MuxInsertionArrayPartition::get_N_Lat(std::string opcode, int operand_bitwid , int
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

// double HI_MuxInsertionArrayPartition::get_N_Delay(std::string opcode, int operand_bitwid , int
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
