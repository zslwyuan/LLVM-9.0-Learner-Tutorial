#include "HI_NoDirectiveTimingResourceEvaluation.h"
#include "HI_InstructionFiles.h"

using namespace llvm;

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
void HI_NoDirectiveTimingResourceEvaluation::Parse_Config()
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
                iss >> clock_freq_str;
                clock_freq = std::stod(clock_freq_str);
                break;
        
            default:
                break;
        }
    }
}

void HI_NoDirectiveTimingResourceEvaluation::Load_Instruction_Info()
{
    int i;
    for (i = 0; i<instructionInfoNum; i++)
    {
        if (instructionHasMappingFile[i])
        {
            Info_type_list tmp_list;
            std::string info_file_name("./HLS_Data_Lib/");
            info_file_name+=instructionNames[i];
            info_file_name+="/";
            info_file_name+=instructionNames[i];
            info_file_name+="_data";
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
                std::string _clock_freq_str = data_ele[3];
                tmp_info.DSP = std::stoi(data_ele[4]);
                tmp_info.FF = std::stoi(data_ele[5]);
                tmp_info.LUT = std::stoi(data_ele[6]);
                tmp_info.Lat = std::stoi(data_ele[7]);
                tmp_info.delay = std::stod(data_ele[8]);
                tmp_info.II = std::stoi(data_ele[9]);
                tmp_info.core_name = data_ele[10];
                tmp_list[oprand_bitwidth][res_bitwid][_clock_freq_str] = tmp_info;
                // str(oprandA)+" "+str(oprandB)+" "+str(oprandC)+" "+str(freq)+" "
                // +str(DSP48E_N)+" "+str(FF_N)+" "+str(LUT_N)+" "+str(lat_tmp)+" "
                // +str(delay_tmp)+" "+str(II_tmp)+" "+str(Core_tmp)        
            }
            BiOp_Info_name2list_map[instructionNames[i]] = tmp_list;
        }
        
    }
}

int HI_NoDirectiveTimingResourceEvaluation::get_N_DSP(std::string opcode, int operand_bitwid , int res_bitwidth, std::string freq)
{
    if (checkInfoAvailability( opcode, operand_bitwid , res_bitwidth, freq))
        return BiOp_Info_name2list_map[opcode][operand_bitwid][res_bitwidth][freq].DSP;
    llvm::errs() << "inquirying : " << opcode << " -- " << operand_bitwid << " -- " << res_bitwidth << " -- " << freq << " \n";
    assert(false && "no such information in the database\n");
}

int HI_NoDirectiveTimingResourceEvaluation::get_N_FF(std::string opcode, int operand_bitwid , int res_bitwidth, std::string freq)
{
    if (checkInfoAvailability( opcode, operand_bitwid , res_bitwidth, freq))
        return BiOp_Info_name2list_map[opcode][operand_bitwid][res_bitwidth][freq].FF;
    llvm::errs() << "inquirying : " << opcode << " -- " << operand_bitwid << " -- " << res_bitwidth << " -- " << freq << " \n";
    assert(false && "no such information in the database\n");
}


int HI_NoDirectiveTimingResourceEvaluation::get_N_LUT(std::string opcode, int operand_bitwid , int res_bitwidth, std::string freq)
{
    if (checkInfoAvailability( opcode, operand_bitwid , res_bitwidth, freq))
        return BiOp_Info_name2list_map[opcode][operand_bitwid][res_bitwidth][freq].LUT;
    llvm::errs() << "inquirying : " << opcode << " -- " << operand_bitwid << " -- " << res_bitwidth << " -- " << freq << " \n";
    assert(false && "no such information in the database\n");
}


int HI_NoDirectiveTimingResourceEvaluation::get_N_Lat(std::string opcode, int operand_bitwid , int res_bitwidth, std::string freq)
{
    if (checkInfoAvailability( opcode, operand_bitwid , res_bitwidth, freq))
        return BiOp_Info_name2list_map[opcode][operand_bitwid][res_bitwidth][freq].Lat;
    llvm::errs() << "inquirying : " << opcode << " -- " << operand_bitwid << " -- " << res_bitwidth << " -- " << freq << " \n";
    assert(false && "no such information in the database\n");
}


double HI_NoDirectiveTimingResourceEvaluation::get_N_Delay(std::string opcode, int operand_bitwid , int res_bitwidth, std::string freq)
{
    if (checkInfoAvailability( opcode, operand_bitwid , res_bitwidth, freq))
        return BiOp_Info_name2list_map[opcode][operand_bitwid][res_bitwidth][freq].delay;
    llvm::errs() << "inquirying : " << opcode << " -- " << operand_bitwid << " -- " << res_bitwidth << " -- " << freq << " \n";
    assert(false && "no such information in the database\n");
}


bool HI_NoDirectiveTimingResourceEvaluation::checkInfoAvailability(std::string opcode, int operand_bitwid , int res_bitwidth, std::string freq)
{
    int operand_bitwid, res_bitwidth;
    std::string freq,opcode;
    std::cin >> opcode >> operand_bitwid >> res_bitwidth >> freq;
    if (BiOp_Info_name2list_map.find(opcode)!=BiOp_Info_name2list_map.end())
    {
        if (BiOp_Info_name2list_map[opcode].find(operand_bitwid)!=BiOp_Info_name2list_map[opcode].end())
        {
            if (BiOp_Info_name2list_map[opcode][operand_bitwid].find(res_bitwidth)!=BiOp_Info_name2list_map[opcode][operand_bitwid].end())
            {
                if (BiOp_Info_name2list_map[opcode][operand_bitwid][res_bitwidth].find(freq)!=BiOp_Info_name2list_map[opcode][operand_bitwid][res_bitwidth].end())
                {
                    return true;
                }
                else
                {
                    llvm::errs() << "not in BiOp_Info_name2list_map[opcode][operand_bitwid][res_bitwidth]\n";
                }
            }
            else
            {
                llvm::errs() << "not in BiOp_Info_name2list_map[opcode][operand_bitwid]\n";
            }
        }
        else
        {
            llvm::errs() << "not in BiOp_Info_name2list_map[opcode]\n";
        }
        
    }
    else
    {
        llvm::errs() << "not in BiOp_Info_name2list_map\n";
    }  

    return false;
}