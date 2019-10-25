#ifndef _Gobal_Confic_Parse
#define _Gobal_Confic_Parse

#include "HI_print.h"
#include "HI_SysExec.h"
#include "HI_StringProcess.h"
#include <string>
#include <iostream>
#include <assert.h>
#include <map>

extern std:: string clock_period_str;
extern std:: string  HLS_lib_path;

void Parse_Config(const char* config_file_name, std::map<std::string, int> &LoopLabel2UnrollFactor);

void parseArrayPartition(std::stringstream &iss);

void parseLoopUnroll(std::stringstream &iss, std::map<std::string, int> &LoopLabel2UnrollFactor);

#endif