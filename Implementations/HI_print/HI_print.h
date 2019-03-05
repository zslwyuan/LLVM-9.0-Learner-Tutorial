#ifndef _HI_PRINT
#define _HI_PRINT


#include <stdio.h>
#include <string>
#include <ios>
#include <stdlib.h>

// Helper method for converting the name of a LLVM type to a string

void print_cmd(const char *tmp);
void print_info(const char *tmp);
void print_status(const char *tmp);
void print_error(const char *tmp);
void print_warning(const char *tmp);

#endif
