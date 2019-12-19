#ifndef _HI_PRINT
#define _HI_PRINT

#include <ios>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sys/time.h>

// Helper method for converting the name of a LLVM type to a string

void print_cmd(const char *tmp);
void print_info(const char *tmp);
void print_status(const char *tmp);
void print_error(const char *tmp);
void print_warning(const char *tmp);

void print_cmd(std::string tmp_string);
void print_info(std::string tmp_string);
void print_status(std::string tmp_string);
void print_error(std::string tmp_string);
void print_warning(std::string tmp_string);

#endif
