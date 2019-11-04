#include "ConfigParse.h"

void Parse_Config(const char* config_file_name, 
                  HI_DesignConfigInfo& desginconfig)
{
    std::string  tmp_s;  
    std::string  tmpStr_forParsing;
    std::ifstream *config_file = new std::ifstream(config_file_name);
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
                desginconfig.setClock(clock_period_str);
                // clock_period = std::stod(clock_period_str);
                break;

            case hash_compile_time("HLS_lib_path"):
                consumeEqual(iss);
                iss >> HLS_lib_path;
                desginconfig.HLS_lib_path = HLS_lib_path;
                break;

            case hash_compile_time("array_partition"):
                parseArrayPartition(iss, desginconfig);
                break;

            case hash_compile_time("loop_unroll"):
                parseLoopUnroll(iss,  desginconfig);
                break;

            case hash_compile_time("loop_pipeline"):
                parseLoopPipeline(iss, desginconfig);
                break;

            default:
                break;
        }
    }
    assert(HLS_lib_path!="" && "The HLS Lib is necessary in the configuration file!\n");
    delete config_file;
}


void Parse_Config(const char* config_file_name, 
                  std::map<std::string, int> &LoopLabel2UnrollFactor, 
                  std::map<std::string, int> &LoopLabel2II
                  )
{
    HI_DesignConfigInfo desginconfig;
    std::string  tmp_s;  
    std::string  tmpStr_forParsing;
    std::ifstream *config_file = new std::ifstream(config_file_name);
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
                desginconfig.setClock(clock_period_str);
                // clock_period = std::stod(clock_period_str);
                break;

            case hash_compile_time("HLS_lib_path"):
                consumeEqual(iss);
                iss >> HLS_lib_path;
                desginconfig.HLS_lib_path = HLS_lib_path;
                break;

            case hash_compile_time("array_partition"):
                parseArrayPartition(iss, desginconfig);
                break;

            case hash_compile_time("loop_unroll"):
                parseLoopUnroll(iss, LoopLabel2UnrollFactor, desginconfig);
                break;

            case hash_compile_time("loop_pipeline"):
                parseLoopPipeline(iss, LoopLabel2II, desginconfig);
                break;

            default:
                break;
        }
    }
    assert(HLS_lib_path!="" && "The HLS Lib is necessary in the configuration file!\n");
    delete config_file;
}


// parse the argument for array partitioning
void parseArrayPartition(std::stringstream &iss, HI_DesignConfigInfo& desginconfig)
{
    std::string targetStr,scopeStr;
    int dim,partition_factor;
    bool cyclic;
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
                targetStr = (tmp_val);
                break;

            case hash_compile_time("scope"):
                consumeEqual(iss);
                iss >> tmp_val;
                scopeStr = (tmp_val);
                break;

            case hash_compile_time("dim"):
                consumeEqual(iss);
                iss >> tmp_val;
                dim = std::stoi(tmp_val);  // count from dim="0" to match the storage format
                break;

            case hash_compile_time("factor"):
                consumeEqual(iss);
                iss >> tmp_val;
                partition_factor = std::stoi(tmp_val);
                break;

            case hash_compile_time("cyclic"):
                cyclic = 1;
                break;

            case hash_compile_time("block"):
                cyclic = 0;
                break;

            default:
                print_error("Wrong argument for array partitioning.");
                break;
        }
    }
    if (cyclic)
    {
        desginconfig.insertArrayCyclicPartition(scopeStr, targetStr, dim, partition_factor);
    }
    else
    {
        desginconfig.insertArrayblockPartition(scopeStr, targetStr, dim, partition_factor);
    }
    
}

// parse the argument for loop unrolling
void parseLoopUnroll(std::stringstream &iss, std::map<std::string, int> &LoopLabel2UnrollFactor, HI_DesignConfigInfo& desginconfig)
{
    int factor = -1;
    std::string loopLabel("");
    while (!iss.eof())
    {
        std::string arg_name;
        std::string tmp_val;
        iss >> arg_name ; //  get the name of parameter
        switch (hash_(arg_name.c_str()))
        {
            case hash_compile_time("label"):
                consumeEqual(iss);
                iss >> tmp_val;
                loopLabel = (tmp_val);
                break;

            case hash_compile_time("factor"):
                consumeEqual(iss);
                iss >> tmp_val;
                factor = std::stoi(tmp_val);
                break;

            default:
                print_error("Wrong argument for loop unrolling.");
                break;
        }
    }
    assert(loopLabel != "" && factor > -1);
    LoopLabel2UnrollFactor[loopLabel] = factor;
    desginconfig.insertLoopUnroll(loopLabel, factor);
}



// parse the argument for loop unrolling
void parseLoopUnroll(std::stringstream &iss, HI_DesignConfigInfo& desginconfig)
{
    int factor = -1;
    std::string loopLabel("");
    while (!iss.eof())
    {
        std::string arg_name;
        std::string tmp_val;
        iss >> arg_name ; //  get the name of parameter
        switch (hash_(arg_name.c_str()))
        {
            case hash_compile_time("label"):
                consumeEqual(iss);
                iss >> tmp_val;
                loopLabel = (tmp_val);
                break;

            case hash_compile_time("factor"):
                consumeEqual(iss);
                iss >> tmp_val;
                factor = std::stoi(tmp_val);
                break;

            default:
                print_error("Wrong argument for loop unrolling.");
                break;
        }
    }
    assert(loopLabel != "" && factor > -1);
    desginconfig.insertLoopUnroll(loopLabel, factor);
}


// parse the argument for loop pipelining
void parseLoopPipeline(std::stringstream &iss, std::map<std::string, int> &LoopLabel2II, HI_DesignConfigInfo& desginconfig)
{
    int factor = -1;
    std::string loopLabel("");
    while (!iss.eof())
    {
        std::string arg_name;
        std::string tmp_val;
        iss >> arg_name ; //  get the name of parameter
        switch (hash_(arg_name.c_str()))
        {
            case hash_compile_time("label"):
                consumeEqual(iss);
                iss >> tmp_val;
                loopLabel = (tmp_val);
                break;

            case hash_compile_time("II"):
                consumeEqual(iss);
                iss >> tmp_val;
                factor = std::stoi(tmp_val);
                break;

            default:
                print_error("Wrong argument for loop pipelining.");
                break;
        }
    }
    assert(loopLabel != "" && factor > -1);
    LoopLabel2II[loopLabel] = factor;
    desginconfig.insertLoopPipeline(loopLabel, factor);
}



// parse the argument for loop pipelining
void parseLoopPipeline(std::stringstream &iss,  HI_DesignConfigInfo& desginconfig)
{
    int factor = -1;
    std::string loopLabel("");
    while (!iss.eof())
    {
        std::string arg_name;
        std::string tmp_val;
        iss >> arg_name ; //  get the name of parameter
        switch (hash_(arg_name.c_str()))
        {
            case hash_compile_time("label"):
                consumeEqual(iss);
                iss >> tmp_val;
                loopLabel = (tmp_val);
                break;

            case hash_compile_time("II"):
                consumeEqual(iss);
                iss >> tmp_val;
                factor = std::stoi(tmp_val);
                break;

            default:
                print_error("Wrong argument for loop pipelining.");
                break;
        }
    }
    assert(loopLabel != "" && factor > -1);
    desginconfig.insertLoopPipeline(loopLabel, factor);
}