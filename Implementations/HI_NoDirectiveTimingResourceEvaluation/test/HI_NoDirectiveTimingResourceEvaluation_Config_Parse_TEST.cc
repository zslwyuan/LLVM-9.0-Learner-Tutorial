#include "HI_NoDirectiveTimingResourceEvaluation_TEST.h"
#include "HI_InstructionFiles.h"

#include <assert.h>

typedef std::uint64_t hash_t;  
    
constexpr hash_t prime = 0x100000001B3ull;  
constexpr hash_t basis = 0xCBF29CE484222325ull;  


hash_t hash_(char const* str)  
{  
    hash_t ret{basis};  
    
    while(*str){  
        ret ^= *str;  
        ret *= prime;  
        str++;  
    }  
    
    return ret;  
}  

std::string removeExtraSpace(std::string tmp_s)
{
    // remove extra space from the line
    while (tmp_s.find("  ")!=std::string::npos)
    {
        tmp_s=tmp_s.replace(tmp_s.find("  "),2," ");
    }
    while (tmp_s.find("\n")!=std::string::npos)
        tmp_s=tmp_s.replace(tmp_s.find("\n"),1,"");
    while (tmp_s.find("\t")!=std::string::npos)
        tmp_s=tmp_s.replace(tmp_s.find("\t"),1,"");
    return tmp_s;
    
}

constexpr hash_t hash_compile_time(char const* str, hash_t last_value = basis)  
{  
    return *str ? hash_compile_time(str+1, (*str ^ last_value) * prime) : last_value;  
}  

// Pass for simple evluation of the latency of the top function, without considering HLS directives
void HI_NoDirectiveTimingResourceEvaluation_TEST::Parse_Config()
{
    std::string  tmp_s;  
    while ( getline(*config_file,tmp_s) )
    {    
        tmp_s = removeExtraSpace(tmp_s);
        std::stringstream iss(tmp_s);    
        std::string param_name;
        iss >> param_name ; //  get the name of parameter
        iss.ignore(1, ' ');iss.ignore(1, '=');iss.ignore(1, ' ');
        switch (hash_(param_name.c_str()))
        {
            case hash_compile_time("clock"):
                iss >> clock_period_str;
                clock_period = std::stod(clock_period_str);
                break;

            case hash_compile_time("HLS_lib_path"):
                iss >> HLS_lib_path;
                break;

            default:
                break;
        }
    }
    std::cout << "HLS_lib_path=" << HLS_lib_path << "\n";
    assert(HLS_lib_path!="" && "The HLS Lib is necessary in the configuration file!\n");
}

void HI_NoDirectiveTimingResourceEvaluation_TEST::Load_Instruction_Info()
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

HI_NoDirectiveTimingResourceEvaluation_TEST::inst_timing_resource_info HI_NoDirectiveTimingResourceEvaluation_TEST::get_inst_info(std::string opcode, int operand_bitwid , int res_bitwidth, std::string period)
{
    if (checkInfoAvailability( opcode, operand_bitwid , res_bitwidth, period))
        return BiOp_Info_name2list_map[opcode][operand_bitwid][res_bitwidth][period];
    if (operand_bitwid%2)
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

    std::cout << "inquirying : " << opcode << " -- " << operand_bitwid << " -- " << res_bitwidth << " -- " << period << " \n";
    assert(false && "no such information in the database\n");
}

// int HI_NoDirectiveTimingResourceEvaluation_TEST::get_N_FF(std::string opcode, int operand_bitwid , int res_bitwidth, std::string period)
// {
//     if (checkInfoAvailability( opcode, operand_bitwid , res_bitwidth, period))
//         return BiOp_Info_name2list_map[opcode][operand_bitwid][res_bitwidth][period].FF;
//     BiOp_Info_name2list_map[opcode][operand_bitwid][res_bitwidth][period] = checkInfo_HigherFreq(opcode, operand_bitwid , res_bitwidth, period);
//     return BiOp_Info_name2list_map[opcode][operand_bitwid][res_bitwidth][period].FF;
    
// }


// int HI_NoDirectiveTimingResourceEvaluation_TEST::get_N_LUT(std::string opcode, int operand_bitwid , int res_bitwidth, std::string period)
// {
//     if (checkInfoAvailability( opcode, operand_bitwid , res_bitwidth, period))
//         return BiOp_Info_name2list_map[opcode][operand_bitwid][res_bitwidth][period].LUT;
//     BiOp_Info_name2list_map[opcode][operand_bitwid][res_bitwidth][period] = checkInfo_HigherFreq(opcode, operand_bitwid , res_bitwidth, period);
//     return BiOp_Info_name2list_map[opcode][operand_bitwid][res_bitwidth][period].LUT;
    
// }


// int HI_NoDirectiveTimingResourceEvaluation_TEST::get_N_Lat(std::string opcode, int operand_bitwid , int res_bitwidth, std::string period)
// {
//     if (checkInfoAvailability( opcode, operand_bitwid , res_bitwidth, period))
//         return BiOp_Info_name2list_map[opcode][operand_bitwid][res_bitwidth][period].Lat;
//     BiOp_Info_name2list_map[opcode][operand_bitwid][res_bitwidth][period] = checkInfo_HigherFreq(opcode, operand_bitwid , res_bitwidth, period);
//     return BiOp_Info_name2list_map[opcode][operand_bitwid][res_bitwidth][period].Lat;
    
// }


// double HI_NoDirectiveTimingResourceEvaluation_TEST::get_N_Delay(std::string opcode, int operand_bitwid , int res_bitwidth, std::string period)
// {
//     if (checkInfoAvailability( opcode, operand_bitwid , res_bitwidth, period))
//         return BiOp_Info_name2list_map[opcode][operand_bitwid][res_bitwidth][period].delay;
//     BiOp_Info_name2list_map[opcode][operand_bitwid][res_bitwidth][period] = checkInfo_HigherFreq(opcode, operand_bitwid , res_bitwidth, period);
//     return BiOp_Info_name2list_map[opcode][operand_bitwid][res_bitwidth][period].delay;
    
// }


bool HI_NoDirectiveTimingResourceEvaluation_TEST::checkInfoAvailability(std::string opcode, int operand_bitwid , int res_bitwidth, std::string period)
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
                    // std::cout << "not in BiOp_Info_name2list_map[opcode][operand_bitwid][res_bitwidth]\n";
                }
            }
            else
            {
                // std::cout << "not in BiOp_Info_name2list_map[opcode][operand_bitwid]\n";
            }
        }
        else
        {
            // std::cout << "not in BiOp_Info_name2list_map[opcode]\n";
        }
        
    }
    else
    {
        // std::cout << "not in BiOp_Info_name2list_map\n";
    }  

    return false;
}

HI_NoDirectiveTimingResourceEvaluation_TEST::inst_timing_resource_info HI_NoDirectiveTimingResourceEvaluation_TEST::checkInfo_HigherFreq(std::string opcode, int operand_bitwid , int res_bitwidth, std::string period)
{
    int i;
    for (i = clockNum - 1; i >= 0; i--)
        if (clockStrs[i] == period)
            break;
    i--;
    assert(i >= 0 && "The clock should be found.\n" );
    for (; i >= 0 ; i--)
    {
        if (checkInfoAvailability(opcode, operand_bitwid, res_bitwidth, clockStrs[i]))
            return BiOp_Info_name2list_map[opcode][operand_bitwid][res_bitwidth][clockStrs[i]];
    }
    std::cout << "inquirying : " << opcode << " -- " << operand_bitwid << " -- " << res_bitwidth << " -- " << period << " \n";
    assert(false && "no such information in the database\n");
}