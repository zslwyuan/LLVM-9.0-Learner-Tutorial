#ifndef _HI_PRINT
#define _HI_PRINT


#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Pass.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/raw_ostream.h"
#include "HI_print.h"
#include <stdio.h>
#include <string>
#include <ios>
#include <stdlib.h>

using namespace llvm;

// Helper method for converting the name of a LLVM type to a string
static std::string LLVMTypeAsString(const Type *T) {
  std::string TypeName;
  raw_string_ostream N(TypeName);
  T->print(N);
  return N.str();
}

void print_cmd(const char *tmp)
{
  printf("\x1b[%d;%dm%s\x1b[%dm \x1b[0;0m\x1b[0m %s\n", 40, 33, "Hi-LLVM CMD: ", 0, tmp);
}

void print_info(const char *tmp)
{
  printf("\x1b[%d;%dm%s\x1b[%dm \x1b[0;0m\x1b[0m %s\n", 40, 34, "Hi-LLVM INFO: ", 0, tmp);
}

void print_status(const char *tmp)
{
  printf("\x1b[%d;%dm%s\x1b[%dm \x1b[0;0m\x1b[0m %s\n", 40, 32, "Hi-LLVM STATUS: ", 0, tmp);
}

void print_error(const char *tmp)
{
  printf("\x1b[%d;%dm%s\x1b[%dm \x1b[0;0m\x1b[0m %s\n", 43, 31, "Hi-LLVM ERROR: ", 0, tmp);
}

void print_warning(const char *tmp)
{
  printf("\x1b[%d;%dm%s\x1b[%dm \x1b[0;0m\x1b[0m %s\n", 43, 31, "Hi-LLVM WARNING: ", 0, tmp);
}

#endif
