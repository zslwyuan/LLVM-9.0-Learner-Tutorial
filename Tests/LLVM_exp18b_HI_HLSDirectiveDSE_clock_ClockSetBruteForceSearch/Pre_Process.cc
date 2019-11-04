#include "Pre_Process.h"
#include "ConfigGen.h"

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


void preAnalyzeProcess(
    llvm::Module &Mod,
    std::string top_str,
    std::string configFile_str,

    std::map<std::string,Info_type_list> &BiOp_Info_name2list_map,
    
    // record the specific level of loop is sensitive to specific partitioning scheme of the specific dimension of array
    std::map<std::string, std::set<std::pair<std::string, std::pair<std::string, int>> >> &LoopLabel2DrivenArrayDimensions,
    std::map<std::string, std::vector<int>> &IRFunc2BeginLine,
    std::map<std::string, std::string> &IRLoop2LoopLabel,
    std::map<std::string, int> &IRLoop2OriginTripCount,
    std::map<std::string, std::vector<std::string>> &LoopIRName2NextLevelSubLoopIRNames,
    std::map<std::string, int> &LoopIRName2Depth,
    std::map<std::string, std::vector< std::pair<std::string, std::string>>> &LoopIRName2Array,
    std::map<std::pair<std::string, std::string>, HI_PragmaArrayInfo> &TargetExtName2ArrayInfo,
    std::map<std::string, std::string> &TopLoopIR2FuncName,
    std::map<std::string, std::vector<int>> &Loop2PotentialUnrollFactors,
    std::map<std::string, int> &FuncParamLine2OutermostSize,
    std::vector<std::string> &FuncNames,
    bool debugFlag
)
{
    DES_Load_Instruction_Info(configFile_str.c_str(), BiOp_Info_name2list_map);



    // desginconfig_0.setClock("6.0");
    // desginconfig_0.HLS_lib_path = "../../../HLS_Data_Lib/";
    // desginconfig_0.insertLoopPipeline("Loop_kernel_2mm_6",1);
    // desginconfig_0.insertLoopUnroll("Loop_kernel_2mm_5",2);
    // desginconfig_0.insertLoopUnroll("Loop_kernel_2mm_6",18);
    // desginconfig_0.insertArrayCyclicPartition("kernel_2mm", "tmp",1, 18);
    // desginconfig_0.insertArrayCyclicPartition("kernel_2mm", "D",1, 2);
    // desginconfig_0.insertArrayCyclicPartition("kernel_2mm", "C",2, 18);

    std::unique_ptr<llvm::Module> Mod_pre = CloneModule(Mod);

    // Create a pass manager and fill it with the passes we want to run.
    legacy::PassManager PM_pre, PM_pre_check;
    
    LLVMTargetRef T;
    ModulePassManager MPM;

    char *Error;
    std::error_code EC;
    if(LLVMGetTargetFromTriple((Mod_pre->getTargetTriple()).c_str(), &T, &Error))
    {
        print_error(Error);
    }
    else
    {
        std::string targetname = LLVMGetTargetName(T);
        targetname = "The target machine is: " + targetname;
        print_info(targetname.c_str());
    }




    auto loopextract = createLoopExtractorPass(); //"HI_LoopUnroll"
    PM_pre.add(loopextract);
    // print_info("Enable LoopExtractor Pass");

    
    auto hi_ir2sourcecode = new HI_IR2SourceCode("HI_IR2SourceCode",IRLoop2LoopLabel, IRFunc2BeginLine, IRLoop2OriginTripCount, debugFlag);
    PM_pre.add(hi_ir2sourcecode);
    print_info("Enable HI_IR2SourceCode Pass");

    print_info("Enable LoopSimplify Pass");
    auto loopsimplifypass = createLoopSimplifyPass();
    PM_pre.add(loopsimplifypass);

    auto indvarsimplifypass = createIndVarSimplifyPass();
    PM_pre.add(indvarsimplifypass);
    print_info("Enable IndVarSimplifyPass Pass");

    // PM_pre.add(createTargetTransformInfoWrapperPass(TargetIRAnalysis()));
    // print_info("Enable TargetIRAnalysis Pass");


    auto hi_mulorderopt = new HI_MulOrderOpt("HI_MulOrderOpt");
    PM_pre.add(hi_mulorderopt);
    print_info("Enable HI_MulOrderOpt Pass");

    auto CFGSimplification_pass22 = createCFGSimplificationPass();
    PM_pre.add(CFGSimplification_pass22);
    print_info("Enable CFGSimplificationPass Pass");



    auto hi_separateconstoffsetfromgep = new HI_SeparateConstOffsetFromGEP("HI_SeparateConstOffsetFromGEP",true, debugFlag);
    PM_pre.add(hi_separateconstoffsetfromgep);
    print_info("Enable HI_SeparateConstOffsetFromGEP Pass");

    auto hi_PragmaTargetExtraction = new HI_PragmaTargetExtraction(top_str.c_str(),
                                                                    IRLoop2LoopLabel, FuncParamLine2OutermostSize, IRFunc2BeginLine, 
                                                                    LoopIRName2NextLevelSubLoopIRNames,
                                                                    LoopIRName2Depth,
                                                                    LoopIRName2Array,
                                                                    TargetExtName2ArrayInfo,
                                                                    debugFlag);


    PM_pre.add(hi_PragmaTargetExtraction);
    print_info("Enable HI_PragmaTargetExtraction Pass");


    auto hi_TopLoop2Func = new HI_TopLoop2Func("HI_TopLoop2Func", TopLoopIR2FuncName, FuncNames , top_str, debugFlag);
    PM_pre.add(hi_TopLoop2Func);
    // print_info("Enable HI_IR2SourceCode Pass");
    PM_pre.run(*Mod_pre);


    print_status("Start LLVM pre-processing");  
    PM_pre.run(*Mod_pre);
    print_status("Accomplished LLVM pre-processing");


    LoopLabel2DrivenArrayDimensions.clear();

    auto hi_ArraySensitiveToLoopLevel = new HI_ArraySensitiveToLoopLevel(
                                                        // configFile_str.c_str(),
                                                        "HI_ArraySensitiveToLoopLevel_eval","HI_ArraySensitiveToLoopLevel_bram",
                                                        "HI_ArraySensitiveToLoopLevel_array",
                                                        top_str.c_str(),
                                                        IRLoop2LoopLabel,
                                                        IRLoop2OriginTripCount,
                                                        FuncParamLine2OutermostSize,
                                                        IRFunc2BeginLine,
                                                        LoopLabel2DrivenArrayDimensions,
                                                        BiOp_Info_name2list_map,
                                                        true);
    PM_pre_check.add(hi_ArraySensitiveToLoopLevel);
    print_info("Enable HI_ArraySensitiveToLoopLevel Pass");
    PM_pre_check.run(*Mod_pre);
    
    // enumerateDesignConfiguration( "arrayConfigs",
    //                             IRLoop2LoopLabel, 
    //                             IRLoop2OriginTripCount,
    //                             LoopIRName2NextLevelSubLoopIRNames,
    //                             LoopIRName2Depth,
    //                             LoopIRName2Array,
    //                             TargetExtName2ArrayInfo);

    if (debugFlag)
    {
        llvm::raw_fd_ostream OS_PM_pre("top_output_loopextract.bc", EC, llvm::sys::fs::F_None);
        WriteBitcodeToFile(*Mod_pre, OS_PM_pre);
        OS_PM_pre.flush();
    }

    Mod_pre.reset();


    Loop2PotentialUnrollFactors = getPotentialLoopUnrollFactor(
                                            IRLoop2LoopLabel,
                                            LoopIRName2NextLevelSubLoopIRNames,
                                            LoopIRName2Depth,
                                            IRLoop2OriginTripCount
                                        );
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
