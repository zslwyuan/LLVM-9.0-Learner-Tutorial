#ifndef _HI_InstructionFiles
#define _HI_InstructionFiles

#include <string>
const int instructionInfoNum = 31; // should refer to Instruction_list.h

// The instruction opcodes which are included in the path specified
extern std::string instructionNames[100];

// Is the instruction opcode included in the path specified or not
extern bool instructionHasMappingFile[100];

#endif