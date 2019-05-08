

#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>
#include <sys/stat.h>
#include <unistd.h>
#include <string>
#include <fstream>

#include "HI_StringProcess.h"


hash_t hash_(char const* str)  
{  
    hash_t ret{basis};  
    
    while(*str){  
        ret ^= *str;  
        ret *= prime;  
        str++;  
    }  
    
    return ret;  
}  

std::string removeExtraSpace(std::string tmp_s)
{
    // remove extra space from the line
    while (tmp_s.find("  ")!=std::string::npos)
    {
        tmp_s=tmp_s.replace(tmp_s.find("  "),2," ");
    }
    while (tmp_s.find("\n")!=std::string::npos)
        tmp_s=tmp_s.replace(tmp_s.find("\n"),1,"");
    while (tmp_s.find("\t")!=std::string::npos)
        tmp_s=tmp_s.replace(tmp_s.find("\t"),1,"");
    return tmp_s;
    
}

