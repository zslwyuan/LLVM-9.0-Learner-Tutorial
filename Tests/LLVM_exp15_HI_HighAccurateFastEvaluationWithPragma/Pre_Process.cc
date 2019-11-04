#include "Pre_Process.h"

using namespace clang;
using namespace clang::driver;
using namespace clang::tooling;

extern std:: string clock_period_str;
extern std:: string  HLS_lib_path;

static llvm::cl::OptionCategory StatSampleCategory("Stat Sample");

void clangPreProcess(const char **argv, std::string top_str, std::map<std::string, int> &FuncParamLine2OutermostSize)
{
    pathAdvice();

    // parse the command-line args passed to your code
    int labelerArgc = 2;
    CommonOptionsParser op(labelerArgc, argv, StatSampleCategory);     

    // create a new Clang Tool instance (a LibTooling environment)
    ClangTool Tool(op.getCompilations(), op.getSourcePathList());
    Rewriter TheRewriter0,TheRewriter1;

    // run the Clang Tool, creating a new FrontendAction, which will run the AST consumer 
    Tool.run(HI_LoopLabeler_rewrite_newFrontendActionFactory<HI_LoopLabeler_FrontendAction>("PLog",TheRewriter0,"tmp.cc").get());
    Tool.run(HI_FunctionInterfaceInfo_rewrite_newFrontendActionFactory<HI_FunctionInterfaceInfo_FrontendAction>("PLog1",TheRewriter1,"tmp_loopLabeled.cc",FuncParamLine2OutermostSize, top_str).get());

}



void ReplaceAll(std::string& strSource, const std::string& strOld, const std::string& strNew)
{
    int nPos = 0;
    while ((nPos = strSource.find(strOld, nPos)) != strSource.npos)
    {
        strSource.replace(nPos, strOld.length(), strNew);
        nPos += strNew.length();
    }
} 

void pathAdvice()
{
    std::cout<< "===============================================================================" << std::endl;
    std::cout<< "if undefined reference occurs, please check whether the following include paths are required." << std::endl;
    std::string line;
    std::string cmd_str = "clang++ ../testcase/test.c  -v 2> ciinfor";
    print_cmd(cmd_str.c_str());
    sysexec(cmd_str.c_str()); 
    std::ifstream infile("ciinfor");
    while (std::getline(infile, line))
    {
        if (line.find("#include <...> search starts here")!=std::string::npos)
        {
            while (std::getline(infile, line))
            {
                if (line.find("End of search list.")!=std::string::npos)
                {                                        
                    break;
                }
                else
                {
                    ReplaceAll(line," ","");
                    ReplaceAll(line,"\n","");                        
                  //  CI.getHeaderSearchOpts().AddPath(line,frontend::ExternCSystem,false,true);
                    line = "Potential Path : " + line;
                    print_info(line.c_str());
                }
                
            }
            break;
        }
    }
    std::cout<< "===============================================================================" << std::endl;
}

// load the HLS database of timing and resource
void DES_Load_Instruction_Info(const char* config_file_name, std::map<std::string,Info_type_list> &BiOp_Info_name2list_map)
{
    auto config_file = new std::ifstream(config_file_name);

    std::string  tmp_s;  
    std::string  tmpStr_forParsing;

    std::string HLS_lib_path;
    while ( getline(*config_file,tmp_s) )
    {    
        tmp_s = removeExtraSpace(tmp_s);
        std::stringstream iss(tmp_s);    
        std::string param_name;
        iss >> param_name ; //  get the name of parameter
        
        switch (hash_(param_name.c_str()))
        {
            case hash_compile_time("HLS_lib_path"):
                consumeEqual(iss);
                iss >> HLS_lib_path;
                break;

            default:
                break;
        }
    }
    assert(HLS_lib_path!="" && "The HLS Lib is necessary in the configuration file!\n");

    delete config_file;
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
