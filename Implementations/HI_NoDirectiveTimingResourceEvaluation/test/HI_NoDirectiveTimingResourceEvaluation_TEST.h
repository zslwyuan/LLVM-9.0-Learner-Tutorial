#ifndef _HI_NoDirectiveTimingResourceEvaluation_TEST
#define _HI_NoDirectiveTimingResourceEvaluation_TEST
// related headers should be included.

#include <stdio.h>
#include <string>
#include <ios>
#include <stdlib.h>
#include "HI_InstructionFiles.h"
#include <set>
#include <map>
#include  <iostream>
#include  <fstream>
#include <ios>
#include <stdlib.h>
#include <sstream>
#include "ClockInfo.h"

// Pass for simple evluation of the latency of the top function, without considering HLS directives
class HI_NoDirectiveTimingResourceEvaluation_TEST {
public:

    // Pass for simple evluation of the latency of the top function, without considering HLS directives
    HI_NoDirectiveTimingResourceEvaluation_TEST(const char* config_file_name) 
    {
        config_file = new std::ifstream(config_file_name);
    } // define a pass, which can be inherited from ModulePass, LoopPass, FunctionPass and etc.

    ~HI_NoDirectiveTimingResourceEvaluation_TEST()
    {
    }



    // parse the file to get configuration
    void Parse_Config();

    // load the timing and resource information for the instructions
    void Load_Instruction_Info();

    bool topFunctionFound = 0;
    
    float clock_period = 10.0;

    std::string clock_period_str = "10.0";

    std::string HLS_lib_path = "";

    // record the information of the processing
    std::error_code ErrInfo;
    std::ifstream *config_file;

    class inst_timing_resource_info
    {
        public:
        // resource 
        int FF;
        int DSP;
        int LUT;

        // timing 
        int Lat;
        float delay;
        int II;

        // core
        std::string core_name;


        inst_timing_resource_info(const inst_timing_resource_info &input)
        {
            FF=input.FF;
            DSP=input.DSP; 
            LUT=input.LUT;
            Lat=input.Lat;
            delay=input.delay;
            II=input.II;
            core_name=input.core_name;
        }

        inst_timing_resource_info()
        {
            FF=0;
            DSP=0; 
            LUT=0;
            Lat=0;
            delay=0;
            II=0;
            core_name="";
        }

        void print()
        {
            std::cout << " DSP=" << DSP << " FF=" << FF << " LUT=" << LUT << " Lat=" << Lat << " delay=" << delay << " II=" << II << "\n";  
        }
    };

    typedef std::map<int,std::map<int, std::map<std::string,inst_timing_resource_info>>> Info_type_list;

    std::map<std::string,Info_type_list> BiOp_Info_name2list_map;
    inst_timing_resource_info get_inst_info(std::string opcode, int operand_bitwid , int res_bitwidth, std::string period);
    // int get_N_DSP(std::string opcode, int operand_bitwid , int res_bitwidth, std::string period);
    // int get_N_FF(std::string opcode, int operand_bitwid , int res_bitwidth, std::string period);
    // int get_N_LUT(std::string opcode, int operand_bitwid , int res_bitwidth, std::string period);
    // int get_N_Lat(std::string opcode, int operand_bitwid , int res_bitwidth, std::string period);
    // double get_N_Delay(std::string opcode, int operand_bitwid , int res_bitwidth, std::string period);
    bool checkInfoAvailability(std::string opcode, int operand_bitwid , int res_bitwidth, std::string period);
    inst_timing_resource_info checkInfo_HigherFreq(std::string opcode, int operand_bitwid , int res_bitwidth, std::string period);

};

#endif
