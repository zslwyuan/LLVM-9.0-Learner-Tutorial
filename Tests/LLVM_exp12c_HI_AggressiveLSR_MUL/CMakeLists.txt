project(LLVM_exp12c_HI_AggressiveLSR_MUL) 

set(Outputname "LLVM_exp12c_HI_AggressiveLSR_MUL")

cmake_minimum_required(VERSION 3.10) 

find_package(LLVM REQUIRED CONFIG) 
find_package(Polly) 

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}") 
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}") 

message(STATUS "Using PollyConfig.cmake in: ${Polly_CMAKE_DIR}") 

# Needed to use support library 
message(STATUS "C++ flags==== ${CMAKE_CXX_FLAGS}") 
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti -std=c++14 -g") 

include_directories(${LLVM_INCLUDE_DIRS}) 
include_directories(${Polly_INCLUDE_DIRS}) 

include_directories(./ 
../../Implementations/ 
../../Implementations/HI_FindFunctions 
../../Implementations/HI_print 
../../Implementations/HI_LoopInFormationCollect 
../../Implementations/HI_DependenceList 
../../Implementations/HI_SysExec 
../../Implementations/HI_Polly_Info 
../../Implementations/HI_LoopDependenceAnalysis
../../Implementations/HI_SimpleTimingEvaluation
../../Implementations/HI_DuplicateInstRm
../../Implementations/HI_VarWidthReduce
../../Implementations/HI_SeparateConstOffsetFromGEP
../../Implementations/HI_NoDirectiveTimingResourceEvaluation
../../Implementations/HI_IntstructionMoveBackward
../../Implementations/HI_HLSDuplicateInstRm
../../Implementations/HI_AggressiveLSR_MUL
)

link_directories("../LLVM_Learner_Libs")
set(MAKE_CMD "../../../BuildLibs.sh")
MESSAGE("COMMAND: ${MAKE_CMD} ")
execute_process(COMMAND ${MAKE_CMD}
   RESULT_VARIABLE CMD_ERROR
      OUTPUT_FILE CMD_OUTPUT)
add_definitions(${LLVM_DEFINITIONS}) 
add_definitions(${Polly_DEFINITIONS}) 

add_executable(${Outputname} LLVM_exp12c_HI_AggressiveLSR_MUL.cc) 

llvm_map_components_to_libnames(llvm_libs support core irreader engine nativecodegen) 

target_link_libraries(${Outputname} pthread dl 
HI_FindFunctions 
HI_print 
HI_DependenceList 
HI_SysExec 
HI_LoopInFormationCollect
HI_Polly_Info HI_LoopDependenceAnalysis 
HI_SimpleTimingEvaluation 
HI_DuplicateInstRm 
HI_VarWidthReduce 
HI_SeparateConstOffsetFromGEP
HI_NoDirectiveTimingResourceEvaluation
HI_IntstructionMoveBackward
HI_HLSDuplicateInstRm
HI_AggressiveLSR_MUL
) #link llvm component 

target_link_libraries(${Outputname} ${Polly_LIBRARIES})

message(STATUS "Polly Libraries are: ${Polly_LIBRARIES}") 


# set(llvm_libs "")

#link llvm library 
target_link_libraries(${Outputname} 
LLVMLTO 
LLVMPasses 
LLVMObjCARCOpts 
LLVMMIRParser 
LLVMSymbolize 
LLVMDebugInfoPDB 
LLVMDebugInfoDWARF 
LLVMCoverage 
LLVMMCA 
LLVMTableGen 
LLVMDlltoolDriver 
LLVMXRay 
LLVMOrcJIT 
LLVMXCoreDisassembler 
LLVMXCoreCodeGen 
LLVMXCoreDesc 
LLVMXCoreInfo 
LLVMXCoreAsmPrinter 
LLVMWebAssemblyDisassembler 
LLVMWebAssemblyCodeGen 
LLVMWebAssemblyDesc 
LLVMWebAssemblyAsmPrinter 
LLVMWebAssemblyAsmParser 
LLVMWebAssemblyInfo 
LLVMSystemZDisassembler 
LLVMSystemZCodeGen 
LLVMSystemZAsmParser 
LLVMSystemZDesc 
LLVMSystemZInfo 
LLVMSystemZAsmPrinter 
LLVMSparcDisassembler 
LLVMSparcCodeGen 
LLVMSparcAsmParser 
LLVMSparcDesc 
LLVMSparcInfo 
LLVMSparcAsmPrinter 
LLVMPowerPCDisassembler 
LLVMPowerPCCodeGen 
LLVMPowerPCAsmParser 
LLVMPowerPCDesc 
LLVMPowerPCInfo 
LLVMPowerPCAsmPrinter 
LLVMNVPTXCodeGen 
LLVMNVPTXDesc 
LLVMNVPTXInfo 
LLVMNVPTXAsmPrinter 
LLVMMSP430Disassembler 
LLVMMSP430CodeGen 
LLVMMSP430AsmParser 
LLVMMSP430Desc 
LLVMMSP430Info 
LLVMMSP430AsmPrinter 
LLVMMipsDisassembler 
LLVMMipsCodeGen 
LLVMMipsAsmParser 
LLVMMipsDesc 
LLVMMipsInfo 
LLVMMipsAsmPrinter 
LLVMLanaiDisassembler 
LLVMLanaiCodeGen 
LLVMLanaiAsmParser 
LLVMLanaiDesc 
LLVMLanaiAsmPrinter 
LLVMLanaiInfo 
LLVMHexagonDisassembler 
LLVMHexagonCodeGen 
LLVMHexagonAsmParser 
LLVMHexagonDesc 
LLVMHexagonInfo 
LLVMBPFDisassembler 
LLVMBPFCodeGen 
LLVMBPFAsmParser 
LLVMBPFDesc 
LLVMBPFInfo 
LLVMBPFAsmPrinter 
LLVMARMDisassembler 
LLVMARMCodeGen 
LLVMARMAsmParser 
LLVMARMDesc 
LLVMARMInfo 
LLVMARMAsmPrinter 
LLVMARMUtils 
LLVMAMDGPUDisassembler 
LLVMAMDGPUCodeGen 
LLVMAMDGPUAsmParser 
LLVMAMDGPUDesc 
LLVMAMDGPUInfo 
LLVMAMDGPUAsmPrinter 
LLVMAMDGPUUtils 
LLVMAArch64Disassembler 
LLVMAArch64CodeGen 
LLVMAArch64AsmParser 
LLVMAArch64Desc 
LLVMAArch64Info 
LLVMAArch64AsmPrinter 
LLVMAArch64Utils 
LLVMObjectYAML 
LLVMLibDriver 
LLVMOption 
LLVMOptRemarks 
LLVMWindowsManifest 
LLVMTextAPI 
LLVMFuzzMutate 
LLVMX86Disassembler 
LLVMX86AsmParser 
LLVMX86CodeGen 
LLVMGlobalISel 
LLVMSelectionDAG 
LLVMAsmPrinter 
LLVMX86Desc 
LLVMMCDisassembler 
LLVMX86Info 
LLVMX86AsmPrinter 
LLVMX86Utils 
LLVMMCJIT 
LLVMLineEditor 
LLVMInterpreter 
LLVMExecutionEngine 
LLVMRuntimeDyld 
LLVMCodeGen 
LLVMTarget 
LLVMCoroutines 
LLVMipo 
LLVMInstrumentation 
LLVMVectorize 
LLVMScalarOpts 
LLVMLinker 
LLVMIRReader 
LLVMAsmParser 
LLVMInstCombine 
LLVMBitWriter 
LLVMAggressiveInstCombine 
LLVMTransformUtils 
LLVMAnalysis 
LLVMProfileData 
LLVMObject 
LLVMMCParser 
LLVMMC 
LLVMDebugInfoCodeView 
LLVMDebugInfoMSF 
LLVMBitReader 
LLVMCore 
LLVMBinaryFormat 
LLVMSupport 
LLVMDemangle
) 

# message(STATUS "LLVM Libraries0 are: ${llvm_libs}") 
# message(STATUS "LLVM Libraries1 are: ${ARG_LINK_COMPONENTS}") 
# message(STATUS "LLVM Libraries2 are: ${LLVM_LINK_COMPONENTS}") 
# message(STATUS "LLVM Definitions are: ${LLVM_PACKAGE_VERSION}") 
