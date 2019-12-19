#ifndef _HI_PRINT
#define _HI_PRINT

#include "HI_print.h"
#include <ios>
#include <stdio.h>
#include <stdlib.h>
#include <string>

void print_cmd(const char *tmp)
{
    printf("\x1b[%d;%dm%s\x1b[%dm \x1b[0;0m\x1b[0m %s\n", 40, 33, "Hi-LLVM CMD: ", 0, tmp);
}

void print_info(const char *tmp)
{
    printf("\x1b[%d;%dm%s\x1b[%dm \x1b[0;0m\x1b[0m %s\n", 40, 34, "Hi-LLVM INFO: ", 0, tmp);
}

void print_status(const char *tmp)
{
    printf("\x1b[%d;%dm%s\x1b[%dm \x1b[0;0m\x1b[0m %s\n", 40, 32, "Hi-LLVM STATUS: ", 0, tmp);
}

void print_error(const char *tmp)
{
    printf("\x1b[%d;%dm%s\x1b[%dm \x1b[0;0m\x1b[0m %s\n", 43, 31, "Hi-LLVM ERROR: ", 0, tmp);
}

void print_warning(const char *tmp)
{
    printf("\x1b[%d;%dm%s\x1b[%dm \x1b[0;0m\x1b[0m %s\n", 43, 31, "Hi-LLVM WARNING: ", 0, tmp);
}

void print_cmd(std::string tmp_string)
{
    printf("\x1b[%d;%dm%s\x1b[%dm \x1b[0;0m\x1b[0m %s\n", 40, 33, "Hi-LLVM CMD: ", 0,
           tmp_string.c_str());
}

void print_info(std::string tmp_string)
{
    printf("\x1b[%d;%dm%s\x1b[%dm \x1b[0;0m\x1b[0m %s\n", 40, 34, "Hi-LLVM INFO: ", 0,
           tmp_string.c_str());
}

void print_status(std::string tmp_string)
{
    printf("\x1b[%d;%dm%s\x1b[%dm \x1b[0;0m\x1b[0m %s\n", 40, 32, "Hi-LLVM STATUS: ", 0,
           tmp_string.c_str());
}

void print_error(std::string tmp_string)
{
    printf("\x1b[%d;%dm%s\x1b[%dm \x1b[0;0m\x1b[0m %s\n", 43, 31, "Hi-LLVM ERROR: ", 0,
           tmp_string.c_str());
}

void print_warning(std::string tmp_string)
{
    printf("\x1b[%d;%dm%s\x1b[%dm \x1b[0;0m\x1b[0m %s\n", 43, 31, "Hi-LLVM WARNING: ", 0,
           tmp_string.c_str());
}

#endif
