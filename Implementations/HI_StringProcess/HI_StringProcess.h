#ifndef _HI_STRINGPROCESS
#define _HI_STRINGPROCESS

#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>
#include <stdexcept>
#include <string>
#include <array>
#include <sys/stat.h>
#include <unistd.h>
#include <string>
#include <fstream>
#include <iostream>
#include <fstream>
#include <ios>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <cctype>
#include <algorithm>
#include <sstream>
#include <vector>
#include <sys/time.h>

typedef std::uint64_t hash_t;  
    
constexpr hash_t prime = 0x100000001B3ull;  
constexpr hash_t basis = 0xCBF29CE484222325ull;  
    

inline bool exists_test (const std::string& name) {
  struct stat buffer;   
  return (stat (name.c_str(), &buffer) == 0); 
}

hash_t hash_(char const* str);

void findAndReplaceAll(std::string & data, std::string toSearch, std::string replaceStr);

std::string removeExtraSpace(std::string tmp_s);

void consumeEqual(std::stringstream &iss);

constexpr hash_t hash_compile_time(char const* str, hash_t last_value = basis)  
{  
    return *str ? hash_compile_time(str+1, (*str ^ last_value) * prime) : last_value;  
}  

std::string demangleFunctionName(std::string mangled_name);

#endif