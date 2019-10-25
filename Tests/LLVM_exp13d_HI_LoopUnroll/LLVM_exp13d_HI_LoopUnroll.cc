#include "LLVM_exp13d_HI_LoopUnroll.h"

using namespace llvm;

std:: string clock_period_str;
std:: string  HLS_lib_path;


int main(int argc, char **argv) {
  if (argc < 4) {
    errs() << "Usage: " << argv[0] << " <C/C++ file> <Top_Function_Name> <Config_File_Path>\n";
    return 1;
  }

  LLVMInitializeX86TargetInfo();
  LLVMInitializeX86Target();
  LLVMInitializeX86TargetMC();
  // Compile the source code into IR and Parse the input LLVM IR file into a module
  SMDiagnostic Err;
  LLVMContext Context;
  std::string cmd_str = "clang -O1 -emit-llvm -g -S "+std::string(argv[1])+" -o top.bc 2>&1";
  std::string top_str = std::string(argv[2]);
  std::string configFile_str = std::string(argv[3]);
  print_cmd(cmd_str.c_str());
  bool result = sysexec(cmd_str.c_str());
  assert(result); // ensure the cmd is executed successfully
 // system(cmd_str.c_str());

  std::unique_ptr<Module> Mod(parseIRFile("top.bc", Err, Context));
  if (!Mod) {
    Err.print(argv[0], errs());
    return 1;
  }
  

  // Create a pass manager and fill it with the passes we want to run.
  legacy::PassManager PM1,PM_pre;
  LLVMTargetRef T;
  ModulePassManager MPM;

  char *Error;

  if(LLVMGetTargetFromTriple((Mod->getTargetTriple()).c_str(), &T, &Error))
  {
    print_error(Error);
  }
  else
  {
    std::string targetname = LLVMGetTargetName(T);
    targetname = "The target machine is: " + targetname;
    print_info(targetname.c_str());
  }


  std::map<std::string, std::string> IRLoop2LoopLabel;
  std::map<std::string, int> LoopLabel2UnrollFactor;
  std::map<std::string, std::vector<int>> IRFunc2BeginLine; 
  std::map<std::string, int> IRLoop2OriginTripCount;
  Parse_Config(configFile_str.c_str(), LoopLabel2UnrollFactor);



// extract information from IR code
// map labels in source code to IR loops / functions...
  auto hi_ir2sourcecode = new HI_IR2SourceCode("HI_IR2SourceCode",IRLoop2LoopLabel, IRFunc2BeginLine,IRLoop2OriginTripCount);
  PM_pre.add(hi_ir2sourcecode);
  print_info("Enable HI_IR2SourceCode Pass");

  print_status("Start LLVM pre-processing");  
  PM_pre.run(*Mod);
  print_status("Accomplished LLVM pre-processing");





// unroll the specific loops
  Triple ModuleTriple(Mod->getTargetTriple());
  TargetLibraryInfoImpl TLII(ModuleTriple);
  PM1.add(new TargetLibraryInfoWrapperPass(TLII));
  

  print_info("Enable LoopSimplify Pass");
  auto loopsimplifypass = createLoopSimplifyPass();
  PM1.add(loopsimplifypass);

  auto indvarsimplifypass = createIndVarSimplifyPass();
  PM1.add(indvarsimplifypass);
  print_info("Enable IndVarSimplifyPass Pass");

  auto assumptioncachetracker = new AssumptionCacheTracker();
  print_info("Enable AssumptionCacheTracker Pass");
  PM1.add(assumptioncachetracker); 

  PM1.add(createTargetTransformInfoWrapperPass(TargetIRAnalysis()));
  print_info("Enable TargetIRAnalysis Pass");

  auto hi_loopunroll = new HI_LoopUnroll(IRLoop2LoopLabel, LoopLabel2UnrollFactor, 1, false, None); //"HI_LoopUnroll"
  PM1.add(hi_loopunroll);
  print_info("Enable HI_LoopUnroll Pass");

  PM1.run(*Mod);










  std::error_code EC;
  llvm::raw_fd_ostream OS1("top_output0.bc", EC, llvm::sys::fs::F_None);
  WriteBitcodeToFile(*Mod, OS1);
  OS1.flush();


  print_status("Writing LLVM IR to File");
  
  llvm::raw_fd_ostream OS("top_output.bc", EC, llvm::sys::fs::F_None);
  WriteBitcodeToFile(*Mod, OS);
  OS.flush();

  print_status("Translate the IR to be readable one");
  cmd_str = "llvm-dis top_output.bc 2>&1";
  print_cmd(cmd_str.c_str());
  result = sysexec(cmd_str.c_str());
  assert(result); // ensure the cmd is executed successfully
  return 0;
}
