#ifndef _HI_PRINT
#define _HI_PRINT


#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Pass.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/raw_ostream.h"
#include <stdio.h>
#include <string>
#include <ios>
#include <stdlib.h>

using namespace llvm;

// Helper method for converting the name of a LLVM type to a string
static std::string LLVMTypeAsString(const Type *T);
void print_cmd(const char *tmp);
void print_info(const char *tmp);
void print_status(const char *tmp);
void print_error(const char *tmp);
void print_warning(const char *tmp);

#endif
