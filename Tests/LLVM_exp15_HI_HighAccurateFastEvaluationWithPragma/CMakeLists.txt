project(LLVM_exp15_HI_HighAccurateFastEvaluationWithPragma) 

set(Outputname "LLVM_exp15_HI_HighAccurateFastEvaluationWithPragma")

cmake_minimum_required(VERSION 3.10) 

find_package(LLVM REQUIRED CONFIG) 
find_package(Polly) 
find_package(Clang)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}") 
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}") 


message(STATUS "Using PollyConfig.cmake in: ${Polly_CMAKE_DIR}") 

# Needed to use support library 
message(STATUS "C++ flags==== ${CMAKE_CXX_FLAGS}") 
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti -std=c++14 -g") 

include_directories(${LLVM_INCLUDE_DIRS}) 
include_directories(${Polly_INCLUDE_DIRS}) 
include_directories(${Clang_INCLUDE_DIRS}) 

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
../../Implementations/HI_IntstructionMoveBackward
../../Implementations/HI_HLSDuplicateInstRm
../../Implementations/HI_AggressiveLSR_MUL
../../Implementations/HI_ArrayAccessPattern
../../Implementations/HI_WithDirectiveTimingResourceEvaluation
../../Implementations/HI_StringProcess
../../Implementations/HI_FunctionInstantiation
../../Implementations/HI_ReplaceSelectAccess
../../Implementations/HI_LoopUnroll
../../Implementations/HI_RemoveRedundantAccess
../../Implementations/HI_IR2SourceCode
../../Implementations/HI_LoopLabeler
../../Implementations/HI_FunctionInterfaceInfo
../../Implementations/HI_Mul2Shl
../../Implementations/HI_MulOrderOpt
../../Implementations/HI_MuxInsertionArrayPartition
../../Implementations/HI_LoadALAP
../../Implementations/HI_PragmaTargetExtraction
)

link_directories("../LLVM_Learner_Libs")
set(MAKE_CMD "../../../BuildLibs.sh")
MESSAGE("COMMAND: ${MAKE_CMD} ")
execute_process(COMMAND ${MAKE_CMD}
   RESULT_VARIABLE CMD_ERROR
      OUTPUT_FILE CMD_OUTPUT)
add_definitions(${LLVM_DEFINITIONS}) 
add_definitions(${Polly_DEFINITIONS}) 

add_executable(${Outputname} LLVM_exp15_HI_HighAccurateFastEvaluationWithPragma.cc ConfigParse.cc Pre_Process.cc) 

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
HI_IntstructionMoveBackward
HI_HLSDuplicateInstRm
HI_AggressiveLSR_MUL
HI_ArrayAccessPattern
HI_WithDirectiveTimingResourceEvaluation
HI_StringProcess
HI_FunctionInstantiation
HI_ReplaceSelectAccess
HI_LoopUnroll
HI_RemoveRedundantAccess
HI_IR2SourceCode
HI_LoopLabeler
HI_FunctionInterfaceInfo
HI_Mul2Shl
HI_MulOrderOpt
HI_MuxInsertionArrayPartition
HI_LoadALAP
HI_PragmaTargetExtraction
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
clangFrontend 
clangParse 
clangSema 
clangAnalysis 
clangAST 
clangLex 
clangBasic 
clangDriver 
clangSerialization 
clangFrontendTool
clangCodeGen 
clangStaticAnalyzerFrontend 
clangStaticAnalyzerCheckers 
clangStaticAnalyzerCore 
clangRewrite 
clangEdit 
clangTooling
clangARCMigrate
clangRewriteFrontend
) 

# message(STATUS "LLVM Libraries0 are: ${llvm_libs}") 
# message(STATUS "LLVM Libraries1 are: ${ARG_LINK_COMPONENTS}") 
# message(STATUS "LLVM Libraries2 are: ${LLVM_LINK_COMPONENTS}") 
# message(STATUS "LLVM Definitions are: ${LLVM_PACKAGE_VERSION}") 
