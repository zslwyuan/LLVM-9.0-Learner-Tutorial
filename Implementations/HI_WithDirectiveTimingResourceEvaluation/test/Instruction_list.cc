
#include <string>

std::string instructionNames[100] = 
    {
        "add", // ID=1
        "and", // ID=2
        "ashr", // ID=3
        "dadd", // ID=4
        "ddiv", // ID=5
        "div", // ID=6
        "dmul", // ID=7
        "fadd", // ID=8
        "fcmp_double", // ID=9
        "fcmp_float", // ID=10
        "fdiv", // ID=11
        "fmul", // ID=12
        "fptosi", // ID=13
        "fptoui", // ID=14
        "icmp", // ID=15
        "lshr", // ID=16
        "mac", // ID=17
        "mul", // ID=18
        "or", // ID=19
        "shl", // ID=20
        "sitofp", // ID=21
        "uitofp", // ID=22
        "xor" // ID=23       
    };

bool instructionHasMappingFile[100] = 
    {
        1, // "add", // ID=1
        1, // "and", // ID=2
        1, // "ashr", // ID=3
        1, // "dadd", // ID=4
        1, // "ddiv", // ID=5
        1, // "div", // ID=6
        1, // "dmul", // ID=7
        1, // "fadd", // ID=8
        1, // "fcmp_double", // ID=9
        1, // "fcmp_float", // ID=10
        1, // "fdiv", // ID=11
        1, // "fmul", // ID=12
        1, // "fptosi", // ID=13
        1, // "fptoui", // ID=14
        1, // "icmp", // ID=15
        1, // "lshr", // ID=16
        1, // "mac", // ID=17
        1, // "mul", // ID=18
        1, // "or", // ID=19
        1, // "shl", // ID=20
        1, // "sitofp", // ID=21
        1, // "uitofp", // ID=22
        1  // "xor" // ID=23   
    };



    