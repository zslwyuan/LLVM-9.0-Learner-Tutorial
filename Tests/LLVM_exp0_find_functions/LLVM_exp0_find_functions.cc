#include "HI_FindFunction.h"
#include "HI_print.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Pass.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/raw_ostream.h"
#include <fstream>
#include <ios>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string>

using namespace llvm;

void ReplaceAll(std::string &strSource, const std::string &strOld, const std::string &strNew)
{
    int nPos = 0;
    while ((nPos = strSource.find(strOld, nPos)) != strSource.npos)
    {
        strSource.replace(nPos, strOld.length(), strNew);
        nPos += strNew.length();
    }
}

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        errs() << "Usage: " << argv[0] << " <C/C++ file>\n";
        return 1;
    }

    // Compile the source code into IR and Parse the input LLVM IR file into a module
    SMDiagnostic Err;
    LLVMContext Context;
    std::string cmd_str = "clang -O1 -emit-llvm -S " + std::string(argv[1]) + " -o top.bc";
    print_cmd(cmd_str.c_str());
    system(cmd_str.c_str());

    std::unique_ptr<Module> Mod(parseIRFile("top.bc", Err, Context));
    if (!Mod)
    {
        Err.print(argv[0], errs());
        return 1;
    }

    // Create a pass manager and fill it with the passes we want to run.
    legacy::PassManager PM;
    HI_FindFunctions *hi_findfunctions = new HI_FindFunctions();
    PM.add(hi_findfunctions);
    PM.run(*Mod);

    std::ifstream infile("top.bc");
    std::string line;
    std::ofstream outfile("ttoopp.ll");

    while (std::getline(infile, line))
    {

        for (auto it = hi_findfunctions->Function_Demangle_Map.begin();
             it != hi_findfunctions->Function_Demangle_Map.end(); it++)
        {
            if (line.find(it->first) != std::string::npos)
            {
                ReplaceAll(line, it->first, it->second);
                break;
            }
        }
        outfile << line << std::endl;
    }
    outfile.close();
    infile.close();

    return 0;
}
