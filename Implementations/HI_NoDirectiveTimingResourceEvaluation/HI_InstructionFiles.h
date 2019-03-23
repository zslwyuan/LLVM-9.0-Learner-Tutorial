#ifndef _HI_InstructionFiles
#define _HI_InstructionFiles

const int instructionInfoNum = 3;

const char* instructionNames[100] = 
    {
        "add", // ID=1
        "mul", // ID=2
        "mac"  // ID=3
    };

bool instructionHasMappingFile[100] = 
    {
        1, // "add", // ID=1
        1, // "mul", // ID=2
        1  // "mac"  // ID=3
    };

#endif