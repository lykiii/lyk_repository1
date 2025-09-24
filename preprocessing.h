#ifndef PREPROCESSING_H

#define PREPROCESSING_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>

#define MAX_LINE_LENGTH 1024  // 最大行长度
int remove_comments(char *line);
int check_makefile_syntax(const char *filename);
void trim_trailing_whitespace(char *str);
void process_makefile(int verbose);
void show_help(const char *program_name);
void show_version();
int is_valid_option(const char *option);

#endif 
