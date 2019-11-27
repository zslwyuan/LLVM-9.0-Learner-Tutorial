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

void Parse_Config(const char* config_file_name, std::map<std::string, int> &LoopLabel2UnrollFactor, std::map<std::string, int> &LoopLabel2II);

void parseArrayPartition(std::stringstream &iss);

void parseLoopUnroll(std::stringstream &iss, std::map<std::string, int> &LoopLabel2UnrollFactor);

void parseLoopPipeline(std::stringstream &iss, std::map<std::string, int> &LoopLabel2II);

void parseArrayPortNum(std::stringstream &iss);

void parseFuncDataflow(std::stringstream &iss);

void parseLocalArray(std::stringstream &iss);

void parseResourceLimit(std::stringstream &iss);

#endif