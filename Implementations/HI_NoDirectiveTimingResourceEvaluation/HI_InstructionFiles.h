#ifndef _HI_InstructionFiles_HI_NoDirectiveTimingResourceEvaluation
#define _HI_InstructionFiles_HI_NoDirectiveTimingResourceEvaluation

#include <string>

// WARNING!!!!!!!
// WARNING!!!!!!!
// when you modify this file, remember that there is another file you need to modify for HI_WithDirectiveTimingResourceEvaluation

// WARNING!!!!!!!
// WARNING!!!!!!!

const int instructionInfoNum = 35; // should refer to Instruction_list.h

// The instruction opcodes which are included in the path specified
extern std::string instructionNames[100];

// Is the instruction opcode included in the path specified or not
extern bool instructionHasMappingFile[100];

#endif