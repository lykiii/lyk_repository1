#ifndef LEVEL5_H
#define LEVEL5_H
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

#define MAX_EXPAND_DEPTH 10    // 最大嵌套深度（防无限递归）
#define MAX_EXPANDED_LEN 2048  // 展开后最大长度

#define MAX_VAR_NAME 32       // 变量名最大长度（如 CC、CFLAGS）
#define MAX_VAR_VALUE 512     // 变量值最大长度（如 gcc -Wall）
#define MAX_VARIABLES 50      // 最大支持变量数量


typedef struct {
    char name[MAX_VAR_NAME];  // 变量名
    char value[MAX_VAR_VALUE];// 变量值
} Variable;

// 存储单个规则的结构体（原有结构不变）
typedef struct {
    char target[MAX_FILENAME_LEN];       // 目标名称
    char dependencies[MAX_DEPENDENCIES][MAX_FILENAME_LEN];  // 依赖列表
    int dep_count;                       // 依赖数量
    char commands[MAX_COMMANDS][MAX_LINE_LENGTH];  // 命令列表
    int cmd_count;                       // 命令数量
    int line_num;                        // 目标定义的行号
} Rule;

// 存储所有规则和错误信息的全局结构（原有结构扩展）
typedef struct {
    Rule rules[MAX_TARGETS];  // 规则数组
    int rule_count;           // 规则数量
    char errors[100][256];    // 错误信息
    int error_count;          // 错误数量

    Variable variables[MAX_VARIABLES]; // 变量数组
    int var_count;                     // 变量数量
} MakefileData;

char *trim_whitespace(char *str);

const char* find_variable(MakefileData *data, const char *var_name);
void add_or_update_variable(MakefileData *data, const char *var_name, const char *var_value, int line_num);
void recursive_expand(MakefileData *data, const char *input, char *output, int depth, int line_num);
void expand_variable(MakefileData *data, const char *input, char *output, int line_num);
bool parse_variable_definition(MakefileData *data, const char *line, int line_num);



#endif
