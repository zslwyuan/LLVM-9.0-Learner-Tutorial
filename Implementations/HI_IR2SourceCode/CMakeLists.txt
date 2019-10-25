
cmake_minimum_required(VERSION 3.10) 


find_package(LLVM REQUIRED CONFIG) 
# Needed to use support library 

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti -std=c++14") 

include_directories(${LLVM_INCLUDE_DIRS}) 
add_definitions(${LLVM_DEFINITIONS}) 

aux_source_directory(. DIR_HI_IR2SourceCode_SRCS)
add_library(HI_IR2SourceCode ${DIR_HI_IR2SourceCode_SRCS})


# set(llvm_libs "")

