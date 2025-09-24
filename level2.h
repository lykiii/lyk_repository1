#ifndef LEVEL2_H

#define LEVEL2_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <sys/stat.h>
#include <stdarg.h>

#define MAX_LINE_LENGTH 1024
#define MAX_TARGETS 100       // 最大目标数量
#define MAX_DEPENDENCIES 50   // 每个目标最大依赖数量
#define MAX_COMMANDS 20       // 每个目标最大命令数量
#define MAX_FILENAME_LEN 33   // 文件名最大长度(含结束符)

// 存储单个规则的结构体
typedef struct {
    char target[MAX_FILENAME_LEN];       // 目标名称
    char dependencies[MAX_DEPENDENCIES][MAX_FILENAME_LEN];  // 依赖列表
    int dep_count;                       // 依赖数量
    char commands[MAX_COMMANDS][MAX_LINE_LENGTH];  // 命令列表
    int cmd_count;                       // 命令数量
    int line_num;                        // 目标定义的行号
} Rule;

// 存储所有规则和错误信息的全局结构
typedef struct {
    Rule rules[MAX_TARGETS];  // 规则数组
    int rule_count;           // 规则数量
    char errors[100][256];    // 错误信息
    int error_count;          // 错误数量
} MakefileData;

void init_makefile_data(MakefileData *data);
void add_error(MakefileData *data, const char *format, ...);
bool file_exists(const char *filename);
int find_target_index(MakefileData *data, const char *target);
char* trim_whitespace(char *str);
void parse_target_line(MakefileData *data, char *line, int line_num);
void add_command_to_current_rule(MakefileData *data, char *line);
void check_dependencies(MakefileData *data);
int parse_and_check_makefile(const char *filename, MakefileData *data);




#endif 
