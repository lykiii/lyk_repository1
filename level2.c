#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <sys/stat.h>
#include "level2.h"
#include "level5.h"
#define MAX_LINE_LENGTH 1024
#define MAX_TARGETS 100       // 最大目标数量
#define MAX_DEPENDENCIES 50   // 每个目标最大依赖数量
#define MAX_COMMANDS 20       // 每个目标最大命令数量
#define MAX_FILENAME_LEN 33   // 文件名最大长度(含结束符)

// 第一次编译临时函数：执行指定目标的命令
int execute_target(MakefileData *data, const char *target_name) {
    int target_index = find_target_index(data, target_name);
    if (target_index == -1) {
        printf("错误：目标 '%s' 未定义\n", target_name);
        return 1;
    }
    
    Rule *target = &data->rules[target_index];
    printf("正在执行目标: %s\n", target->target);
    
    // 依次执行目标的所有命令
    for (int i = 0; i < target->cmd_count; i++) {
        printf("执行命令: %s\n", target->commands[i]);
        int result = system(target->commands[i]);  // 使用system调用执行命令
        if (result != 0) {
            printf("命令执行失败: %s (返回值: %d)\n", target->commands[i], result);
            return 1;
        }
    }
    
    printf("目标 '%s' 执行完成\n", target->target);
    return 0;
}
  
  
  
// 初始化Makefile数据结构
void init_makefile_data(MakefileData *data) {
    data->rule_count = 0;
    data->error_count = 0;
    memset(data->rules, 0, sizeof(data->rules));
    memset(data->errors, 0, sizeof(data->errors));
    
    //初始化变量存储
    data->var_count = 0;
    memset(data->variables, 0, sizeof(data->variables));
}

// 添加错误信息
void add_error(MakefileData *data, const char *format, ...) {
    if (data->error_count >= 100) return;
    
    va_list args;
    va_start(args, format);
    vsnprintf(data->errors[data->error_count], 255, format, args);
    data->error_count++;
    va_end(args);
}

// 检查文件是否存在
bool file_exists(const char *filename) {
    struct stat buffer;
    return (stat(filename, &buffer) == 0);
}

// 检查目标是否已定义
int find_target_index(MakefileData *data, const char *target) {
    for (int i = 0; i < data->rule_count; i++) {
        if (strcmp(data->rules[i].target, target) == 0) {
            return i;
        }
    }
    return -1;
}

// 移除字符串中的空白字符(首尾)
char* trim_whitespace(char *str) {
    // 移除开头空白
    while (isspace((unsigned char)*str)) str++;
    
    // 移除结尾空白
    if (*str) {
        char *end = str + strlen(str) - 1;
        while (end > str && isspace((unsigned char)*end)) end--;
        end[1] = '\0';
    }
    return str;
}

// 解析目标行(如 "app: main.c utils.c")
void parse_target_line(MakefileData *data, char *line, int line_num) {
    char *colon_pos = strchr(line, ':');
    if (!colon_pos) return;
    
    *colon_pos = '\0';
    char *target = trim_whitespace(line);
    
    if (strlen(target) >= MAX_FILENAME_LEN) {
        add_error(data, "Line%d: Target name too long (max %d chars)", line_num, MAX_FILENAME_LEN-1);
        return;
    }
    
    if (find_target_index(data, target) != -1) {
        add_error(data, "Line%d: Duplicate target '%s'", line_num, target);
        return;
    }
    
    Rule *new_rule = &data->rules[data->rule_count];
    strcpy(new_rule->target, target);
    new_rule->line_num = line_num;
    new_rule->dep_count = 0;
    new_rule->cmd_count = 0;
    
    // 改动：展开依赖列表中的变量（如 $(SRC) → main.c utils.c）
    char *deps_raw = trim_whitespace(colon_pos + 1);
    char deps_expanded[MAX_LINE_LENGTH] = {0};
    expand_variable(data, deps_raw, deps_expanded, line_num);
    
    // 分割展开后的依赖项
    char *dep_token = strtok(deps_expanded, " \t");
    while (dep_token && new_rule->dep_count < MAX_DEPENDENCIES) {
        char *dep_trim = trim_whitespace(dep_token);
        if (strlen(dep_trim) == 0) {
            dep_token = strtok(NULL, " \t");
            continue;
        }
        
        if (strlen(dep_trim) >= MAX_FILENAME_LEN) {
            add_error(data, "Line%d: Dependency too long (max %d chars)", line_num, MAX_FILENAME_LEN-1);
            dep_token = strtok(NULL, " \t");
            continue;
        }
        
        strcpy(new_rule->dependencies[new_rule->dep_count], dep_trim);
        new_rule->dep_count++;
        dep_token = strtok(NULL, " \t");
    }
    
    data->rule_count++;
}

// 向当前规则添加命令
// 改动：新增 line_num 参数用于报错，内部添加变量展开
void add_command_to_current_rule(MakefileData *data, char *line, int line_num) {
    if (data->rule_count == 0) return;
    
    Rule *current_rule = &data->rules[data->rule_count - 1];
    if (current_rule->cmd_count >= MAX_COMMANDS) {
        add_error(data, "Line%d: Too many commands for '%s' (max %d)", line_num, current_rule->target, MAX_COMMANDS);
        return;
    }
    
    // 改动：展开命令中的变量（如 $(CC) → gcc）
    char cmd_expanded[MAX_EXPANDED_LEN] = {0};
    expand_variable(data, line, cmd_expanded, line_num);
    
    // 存储展开后的命令
    strncpy(current_rule->commands[current_rule->cmd_count], cmd_expanded, MAX_LINE_LENGTH - 1);
    current_rule->commands[current_rule->cmd_count][MAX_LINE_LENGTH - 1] = '\0'; // 防溢出
    current_rule->cmd_count++;
}


// 检查所有依赖是否有效
void check_dependencies(MakefileData *data) {
    for (int i = 0; i < data->rule_count; i++) {
        Rule *rule = &data->rules[i];
        
        for (int j = 0; j < rule->dep_count; j++) {
            char *dep = rule->dependencies[j];
            bool is_valid = false;
            
            // 检查是否是已定义的目标
            if (find_target_index(data, dep) != -1) {
                is_valid = true;
            }
            // 检查是否是存在的文件
            else if (file_exists(dep)) {
                is_valid = true;
            }
            
            // 无效依赖
            if (!is_valid) {
                add_error(data, "Line%d: Invalid dependency '%s'", 
                         rule->line_num, dep);
            }
        }
    }
}

// 解析Makefile并进行检查
int parse_and_check_makefile(const char *filename, MakefileData *data) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("无法打开文件");
        return 1;
    }

    char line[MAX_LINE_LENGTH];
    int line_num = 0;

    while (fgets(line, MAX_LINE_LENGTH, file)) {
        line_num++;
        char processed_line[MAX_LINE_LENGTH];
        strcpy(processed_line, line);

        // 1. 处理注释和换行符（原有逻辑）
        char *comment_pos = strchr(processed_line, '#');
        if (comment_pos) *comment_pos = '\0';
        size_t len = strlen(processed_line);
        if (len > 0 && processed_line[len - 1] == '\n') processed_line[len - 1] = '\0';
        char *trimmed_line = trim_whitespace(processed_line);
        if (strlen(trimmed_line) == 0) continue; // 跳过空白行

        // 改动1：优先解析变量行（如 CC = gcc）
        if (parse_variable_definition(data, trimmed_line, line_num)) {
            continue; // 是变量行，跳过后续判断
        }

        // 2. 原有逻辑：解析目标行/命令行
        if (strchr(trimmed_line, ':') != NULL) {
            parse_target_line(data, trimmed_line, line_num);
        } else if (line[0] == '\t') { // 用原始行判断Tab缩进
            char *cmd_raw = trim_whitespace(processed_line + 1);
            if (strlen(cmd_raw) > 0) {
                // 改动2：调用修改后的命令添加函数（传 line_num）
                add_command_to_current_rule(data, cmd_raw, line_num);
            }
        } else {
            // 改动3：新增无效行报错（既不是变量/目标/命令）
            add_error(data, "Line%d: Invalid line (not variable/target/command)", line_num);
        }
    }

    fclose(file);
    check_dependencies(data);
    
    // 输出错误（原有逻辑）
    for (int i = 0; i < data->error_count; i++) {
        printf("Error: %s\n", data->errors[i]);
    }
    
    return data->error_count > 0 ? 1 : 0;
}




