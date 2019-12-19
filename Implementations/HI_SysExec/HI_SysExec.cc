

#include "HI_print.h"
#include <array>
#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>

bool sysexec(const char *cmd)
{
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
    if (!pipe)
    {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr)
    {
        result += buffer.data();
    }
    std::string tmp(result);
    if (tmp.find("error") != -1 || tmp.find("ERROR") != -1 || tmp.find("Error") != -1)
    {
        std::string tmp0 = cmd;
        tmp0 = "command failure: " + tmp0;
        print_error(tmp0.c_str());
        return 0;
    }
    return 1;
}