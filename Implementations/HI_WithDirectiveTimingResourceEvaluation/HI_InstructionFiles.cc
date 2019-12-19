
#include <string>
// WARNING!!!!!!!
// WARNING!!!!!!!
// when you modify this file, remember that there is another file you need to modify for
// HI_NoDirectiveTimingResourceEvaluation

// WARNING!!!!!!!
// WARNING!!!!!!!
std::string instructionNames[100] = {
    "add",    "ashr",  "br",     "dcmp",   "dmul", "fcmp", "fmul", "fptoui", "getelementptr",
    "load",   "mac",   "or",     "shl",    "srem", "udiv", "urem", "and",    "dadd",
    "ddiv",   "fadd",  "fdiv",   "fptosi", "icmp", "lshr", "mul",  "ret",    "sdiv",
    "sitofp", "store", "uitofp", "xor",    "sub",  "fsub", "dsub", "tadd"};

bool instructionHasMappingFile[100] = {
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
    1, // "srem", // ID=22
    1, // "uitofp", // ID=23
    1, // "urem", // ID=24
    1, // "sitofp", // ID=21
    1, // "srem", // ID=22
    1, // "uitofp", // ID=23
    1, // "urem", // ID=24
    1, // "sitofp", // ID=21
    1, // "srem", // ID=22
    1, // "uitofp", // ID=23
    1, // "urem", // ID=24
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
    1, // "srem", // ID=22
    1, // "uitofp", // ID=23
    1, // "urem", // ID=24
    1, // "sitofp", // ID=21
    1, // "srem", // ID=22
    1, // "uitofp", // ID=23
    1, // "urem", // ID=24
    1, // "sitofp", // ID=21
    1, // "srem", // ID=22
    1, // "uitofp", // ID=23
    1, // "urem", // ID=24
    1, // ""sub""
    1, // "fsub"
    1, // "dsub"
    1  // "tadd"
};
