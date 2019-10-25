#include "HI_WithDirectiveTimingResourceEvaluation.h"
#include "HI_InstructionFiles.h"
#include <sys/stat.h>
#include <unistd.h>
#include <string>
#include <fstream>

using namespace llvm;


// Pass for simple evluation of the latency of the top function, without considering HLS directives
void HI_WithDirectiveTimingResourceEvaluation::Parse_Config()
{
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

            default:
                break;
        }
    }
    assert(HLS_lib_path!="" && "The HLS Lib is necessary in the configuration file!\n");
}

// load the HLS database of timing and resource
void HI_WithDirectiveTimingResourceEvaluation::Load_Instruction_Info()
{
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

// Organize the information into timingBase after getting the information of a specific instruction, based on its opcode, operand_bitwidth, result_bitwidth and clock period.
HI_WithDirectiveTimingResourceEvaluation::timingBase HI_WithDirectiveTimingResourceEvaluation::get_inst_TimingInfo_result(std::string opcode, int operand_bitwid , int res_bitwidth, std::string period)
{
    timingBase result(0,0,1,clock_period);
    inst_timing_resource_info info = get_inst_info(opcode,operand_bitwid,res_bitwidth,clock_period_str);
    result.latency = info.Lat;
    result.timing = info.delay;
    result.II = info.II;
   // result.strict_timing = "store"==opcode;  // TODO: turn the strict timing option into library
    return result; 
}

// Organize the information into resourceBase after getting the information of a specific instruction, based on its opcode, operand_bitwidth, result_bitwidth and clock period.
HI_WithDirectiveTimingResourceEvaluation::resourceBase HI_WithDirectiveTimingResourceEvaluation::get_inst_ResourceInfo_result(std::string opcode, int operand_bitwid , int res_bitwidth, std::string period)
{
    resourceBase result(0,0,0,clock_period);
    inst_timing_resource_info info = get_inst_info(opcode,operand_bitwid,res_bitwidth,clock_period_str);
    result.DSP = info.DSP;
    result.FF = info.FF;
    result.LUT = info.LUT;
    return result; 
}

// get the information of a specific instruction, based on its opcode, operand_bitwidth, result_bitwidth and clock period
HI_WithDirectiveTimingResourceEvaluation::inst_timing_resource_info HI_WithDirectiveTimingResourceEvaluation::get_inst_info(std::string opcode, int operand_bitwid , int res_bitwidth, std::string period)
{
    if (checkInfoAvailability( opcode, operand_bitwid , res_bitwidth, period))
        return BiOp_Info_name2list_map[opcode][operand_bitwid][res_bitwidth][period];
    if (!checkFreqProblem(opcode, operand_bitwid , res_bitwidth, period) && (operand_bitwid%2))
    {   
        inst_timing_resource_info info_A = get_inst_info(opcode, operand_bitwid+1 , res_bitwidth+1, period);
        inst_timing_resource_info info_B = get_inst_info(opcode, operand_bitwid-1,  res_bitwidth-1, period);
        inst_timing_resource_info tmp_info;

        tmp_info.DSP = (info_A.DSP + info_B.DSP + 1)/2;
        tmp_info.FF = (info_A.FF + info_B.FF + 1)/2;
        tmp_info.LUT = (info_A.LUT + info_B.LUT)/2;
        tmp_info.Lat = (info_A.Lat + info_B.Lat)/2;
        tmp_info.delay = (info_A.delay + info_B.delay)/2;
        tmp_info.II = (info_A.II + info_B.II + 1 )/2;
        tmp_info.core_name = info_A.core_name;
        BiOp_Info_name2list_map[opcode][operand_bitwid][res_bitwidth][period] = tmp_info;
    }
    else
    {
        BiOp_Info_name2list_map[opcode][operand_bitwid][res_bitwidth][period] = checkInfo_HigherFreq(opcode, operand_bitwid , res_bitwidth, period);
    }
        
    return BiOp_Info_name2list_map[opcode][operand_bitwid][res_bitwidth][period];

    llvm::errs() << "inquirying : " << opcode << " -- " << operand_bitwid << " -- " << res_bitwidth << " -- " << period << " \n";
    assert(false && "no such information in the database\n");
}

// check whether a specific information is in the database
bool HI_WithDirectiveTimingResourceEvaluation::checkInfoAvailability(std::string opcode, int operand_bitwid , int res_bitwidth, std::string period)
{
    if (BiOp_Info_name2list_map.find(opcode)!=BiOp_Info_name2list_map.end())
    {
        if (BiOp_Info_name2list_map[opcode].find(operand_bitwid)!=BiOp_Info_name2list_map[opcode].end())
        {
            if (BiOp_Info_name2list_map[opcode][operand_bitwid].find(res_bitwidth)!=BiOp_Info_name2list_map[opcode][operand_bitwid].end())
            {
                if (BiOp_Info_name2list_map[opcode][operand_bitwid][res_bitwidth].find(period)!=BiOp_Info_name2list_map[opcode][operand_bitwid][res_bitwidth].end())
                {
                    return true;
                }
                else
                {
                    // llvm::errs() << "not in BiOp_Info_name2list_map[opcode][operand_bitwid][res_bitwidth]\n";
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
bool HI_WithDirectiveTimingResourceEvaluation::checkFreqProblem(std::string opcode, int operand_bitwid , int res_bitwidth, std::string period)
{
    if (BiOp_Info_name2list_map.find(opcode)!=BiOp_Info_name2list_map.end())
    {
        if (BiOp_Info_name2list_map[opcode].find(operand_bitwid)!=BiOp_Info_name2list_map[opcode].end())
        {
            if (BiOp_Info_name2list_map[opcode][operand_bitwid].find(res_bitwidth)!=BiOp_Info_name2list_map[opcode][operand_bitwid].end())
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

// if the information is not found in database, we may infer the information by increasing the clock frequency
HI_WithDirectiveTimingResourceEvaluation::inst_timing_resource_info HI_WithDirectiveTimingResourceEvaluation::checkInfo_HigherFreq(std::string opcode, int operand_bitwid , int res_bitwidth, std::string period)
{
    int i;
    
    // locate the target clock in clock sequence
    for (i = clockNum - 1; i >= 0; i--)
        if (std::stof(clockStrs[i]) == std::stof(period))
        {
            if (checkInfoAvailability( opcode, operand_bitwid , res_bitwidth, clockStrs[i]))
            {
                BiOp_Info_name2list_map[opcode][operand_bitwid][res_bitwidth][period] = BiOp_Info_name2list_map[opcode][operand_bitwid][res_bitwidth][clockStrs[i]];
                return BiOp_Info_name2list_map[opcode][operand_bitwid][res_bitwidth][clockStrs[i]];
            }
            break;
        }
    i--;
    if (i<0)
        llvm::errs() << "inquirying : " << opcode << " -- " << operand_bitwid << " -- " << res_bitwidth << " -- " << period << " \n";
    assert(i >= 0 && "The clock should be found.\n" );

    // iterate to find available information in database
    for (; i >= 0 ; i--)
    {
        if (checkInfoAvailability(opcode, operand_bitwid, res_bitwidth, clockStrs[i]))
            return BiOp_Info_name2list_map[opcode][operand_bitwid][res_bitwidth][clockStrs[i]];
    }
    llvm::errs() << "inquirying : " << opcode << " -- " << operand_bitwid << " -- " << res_bitwidth << " -- " << period << " \n";
    assert(false && "no such information in the database\n");
}


void HI_WithDirectiveTimingResourceEvaluation::parseArrayPartition(std::stringstream &iss)
{
    HI_PragmaInfo ans_pragma;  
    ans_pragma.HI_PragmaInfoType = HI_PragmaInfo::arrayPartition_Pragma;
    while (!iss.eof())
    {
        std::string arg_name;
        std::string tmp_val;
        iss >> arg_name ; //  get the name of parameter
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
                ans_pragma.dim = std::stoi(tmp_val);
                break;

            case hash_compile_time("factor"):
                consumeEqual(iss);
                iss >> tmp_val;
                ans_pragma.partition_factor = std::stoi(tmp_val);
                break;

            default:
                print_error("Wrong argument for array partitioning.");
                break;
        }
    }
    PragmaInfo_List.push_back(ans_pragma);
    // 

}

// match the configuration and the corresponding declaration of memory (array)
void HI_WithDirectiveTimingResourceEvaluation::matchArrayAndConfiguration(Value* target)
{
    for (auto& pragma : PragmaInfo_List)
    {
        if (pragma.HI_PragmaInfoType == HI_PragmaInfo::arrayPartition_Pragma)
        {
            if (target->getName() == pragma.targetStr)
            {
                Function *F = getFunctionOfValue(target);
                assert(F && "the parent function of the value should be found.");
                if (demangleFunctionName(F->getName()) != pragma.scopeStr )
                {   
                    continue;
                }
                assert(arrayDirectives.find(target)==arrayDirectives.end() && "The target should be not in arrayDirectives list.\n") ;
                pragma.targetArray = target;
                pragma.ScopeFunc = F;
                arrayDirectives[target] = pragma;
            }
        }
    }
}

// find which function the value is located in
Function* HI_WithDirectiveTimingResourceEvaluation::getFunctionOfValue(Value* target)
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


// int HI_WithDirectiveTimingResourceEvaluation::get_N_DSP(std::string opcode, int operand_bitwid , int res_bitwidth, std::string period)
// {
//     if (checkInfoAvailability( opcode, operand_bitwid , res_bitwidth, period))
//         return BiOp_Info_name2list_map[opcode][operand_bitwid][res_bitwidth][period].DSP;
//     BiOp_Info_name2list_map[opcode][operand_bitwid][res_bitwidth][period] = checkInfo_HigherFreq(opcode, operand_bitwid , res_bitwidth, period);
//     return BiOp_Info_name2list_map[opcode][operand_bitwid][res_bitwidth][period].DSP;
//     llvm::errs() << "inquirying : " << opcode << " -- " << operand_bitwid << " -- " << res_bitwidth << " -- " << period << " \n";
//     assert(false && "no such information in the database\n");
// }

// int HI_WithDirectiveTimingResourceEvaluation::get_N_FF(std::string opcode, int operand_bitwid , int res_bitwidth, std::string period)
// {
//     if (checkInfoAvailability( opcode, operand_bitwid , res_bitwidth, period))
//         return BiOp_Info_name2list_map[opcode][operand_bitwid][res_bitwidth][period].FF;
//     BiOp_Info_name2list_map[opcode][operand_bitwid][res_bitwidth][period] = checkInfo_HigherFreq(opcode, operand_bitwid , res_bitwidth, period);
//     return BiOp_Info_name2list_map[opcode][operand_bitwid][res_bitwidth][period].FF;
//     llvm::errs() << "inquirying : " << opcode << " -- " << operand_bitwid << " -- " << res_bitwidth << " -- " << period << " \n";
//     assert(false && "no such information in the database\n");
// }


// int HI_WithDirectiveTimingResourceEvaluation::get_N_LUT(std::string opcode, int operand_bitwid , int res_bitwidth, std::string period)
// {
//     if (checkInfoAvailability( opcode, operand_bitwid , res_bitwidth, period))
//         return BiOp_Info_name2list_map[opcode][operand_bitwid][res_bitwidth][period].LUT;
//     BiOp_Info_name2list_map[opcode][operand_bitwid][res_bitwidth][period] = checkInfo_HigherFreq(opcode, operand_bitwid , res_bitwidth, period);
//     return BiOp_Info_name2list_map[opcode][operand_bitwid][res_bitwidth][period].LUT;
//     llvm::errs() << "inquirying : " << opcode << " -- " << operand_bitwid << " -- " << res_bitwidth << " -- " << period << " \n";
//     assert(false && "no such information in the database\n");
// }


// int HI_WithDirectiveTimingResourceEvaluation::get_N_Lat(std::string opcode, int operand_bitwid , int res_bitwidth, std::string period)
// {
//     if (checkInfoAvailability( opcode, operand_bitwid , res_bitwidth, period))
//         return BiOp_Info_name2list_map[opcode][operand_bitwid][res_bitwidth][period].Lat;
//     BiOp_Info_name2list_map[opcode][operand_bitwid][res_bitwidth][period] = checkInfo_HigherFreq(opcode, operand_bitwid , res_bitwidth, period);
//     return BiOp_Info_name2list_map[opcode][operand_bitwid][res_bitwidth][period].Lat;
//     llvm::errs() << "inquirying : " << opcode << " -- " << operand_bitwid << " -- " << res_bitwidth << " -- " << period << " \n";
//     assert(false && "no such information in the database\n");
// }


// double HI_WithDirectiveTimingResourceEvaluation::get_N_Delay(std::string opcode, int operand_bitwid , int res_bitwidth, std::string period)
// {
//     if (checkInfoAvailability( opcode, operand_bitwid , res_bitwidth, period))
//         return BiOp_Info_name2list_map[opcode][operand_bitwid][res_bitwidth][period].delay;
//     BiOp_Info_name2list_map[opcode][operand_bitwid][res_bitwidth][period] = checkInfo_HigherFreq(opcode, operand_bitwid , res_bitwidth, period);
//     return BiOp_Info_name2list_map[opcode][operand_bitwid][res_bitwidth][period].delay;
//     llvm::errs() << "inquirying : " << opcode << " -- " << operand_bitwid << " -- " << res_bitwidth << " -- " << period << " \n";
//     assert(false && "no such information in the database\n");
// }
