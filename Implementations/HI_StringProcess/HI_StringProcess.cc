

#include <array>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <sys/stat.h>
#include <unistd.h>

#include "HI_StringProcess.h"

hash_t hash_(char const *str)
{
    hash_t ret{basis};

    while (*str)
    {
        ret ^= *str;
        ret *= prime;
        str++;
    }

    return ret;
}

std::string removeExtraSpace(std::string tmp_s)
{
    // remove extra space from the line
    if (tmp_s.find("=") != std::string::npos)
        findAndReplaceAll(tmp_s, "=", " = ");

    while (tmp_s.find("  ") != std::string::npos)
    {
        tmp_s = tmp_s.replace(tmp_s.find("  "), 2, " ");
    }
    while (tmp_s.find("  =") != std::string::npos)
    {
        tmp_s = tmp_s.replace(tmp_s.find("  ="), 3, " =");
    }
    while (tmp_s.find("=  ") != std::string::npos)
    {
        tmp_s = tmp_s.replace(tmp_s.find("=  "), 3, "= ");
    }

    while (tmp_s.find("\n") != std::string::npos)
        tmp_s = tmp_s.replace(tmp_s.find(" \n"), 2, "\n");
    while (tmp_s.find("\t") != std::string::npos)
        tmp_s = tmp_s.replace(tmp_s.find(" \t"), 2, "\t");

    while (tmp_s.find("\n") != std::string::npos)
        tmp_s = tmp_s.replace(tmp_s.find("\n"), 1, "");
    while (tmp_s.find("\t") != std::string::npos)
        tmp_s = tmp_s.replace(tmp_s.find("\t"), 1, "");
    return tmp_s;
}

void consumeEqual(std::stringstream &iss)
{
    iss.ignore(1, ' ');
    iss.ignore(1, '=');
    iss.ignore(1, ' ');
}

void findAndReplaceAll(std::string &data, std::string toSearch, std::string replaceStr)
{
    // Get the first occurrence
    size_t pos = data.find(toSearch);

    // Repeat till end is reached
    while (pos != std::string::npos)
    {
        // Replace this occurrence of Sub String
        data.replace(pos, toSearch.size(), replaceStr);
        // Get the next occurrence from the current position
        pos = data.find(toSearch, pos + replaceStr.size());
    }
}

std::string demangleFunctionName(std::string mangled_name)
{
    std::string demangled_name;

    // demangle the function
    if (mangled_name.find("_Z") == std::string::npos)
        demangled_name = mangled_name;
    else
    {
        std::stringstream iss(mangled_name);
        iss.ignore(1, '_');
        iss.ignore(1, 'Z');
        int len;
        iss >> len;
        while (len--)
        {
            char tc;
            iss >> tc;
            demangled_name += tc;
        }
    }
    return demangled_name;
}