#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <sys/stat.h>
#include "level2.h"

#define MAX_LINE_LENGTH 1024
#define MAX_TARGETS 100       // 最大目标数量
#define MAX_DEPENDENCIES 50   // 每个目标最大依赖数量
#define MAX_COMMANDS 20       // 每个目标最大命令数量
#define MAX_FILENAME_LEN 33   // 文件名最大长度(含结束符)

// 新增：执行指定目标的命令
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
    // 分割目标和依赖(通过冒号)
    char *colon_pos = strchr(line, ':');
    if (!colon_pos) return;  // 应该不会发生，已在语法检查中过滤
    
    // 提取目标
    *colon_pos = '\0';
    char *target = trim_whitespace(line);
    
    // 检查目标名长度
    if (strlen(target) >= MAX_FILENAME_LEN) {
        add_error(data, "Line%d: Target name too long (max %d characters)", 
                 line_num, MAX_FILENAME_LEN-1);
        return;
    }
    
    // 检查重复定义
    if (find_target_index(data, target) != -1) {
        add_error(data, "Line%d: Duplicate target definition '%s'", 
                 line_num, target);
        return;
    }
    
    // 创建新规则
    Rule *new_rule = &data->rules[data->rule_count];
    strcpy(new_rule->target, target);
    new_rule->line_num = line_num;
    new_rule->dep_count = 0;
    new_rule->cmd_count = 0;
    
    // 解析依赖
    char *deps_part = trim_whitespace(colon_pos + 1);
    char *dep_token = strtok(deps_part, " \t");
    
    while (dep_token && new_rule->dep_count < MAX_DEPENDENCIES) {
        // 检查依赖名长度
        if (strlen(dep_token) >= MAX_FILENAME_LEN) {
            add_error(data, "Line%d: Dependency name too long (max %d characters)", 
                     line_num, MAX_FILENAME_LEN-1);
            dep_token = strtok(NULL, " \t");
            continue;
        }
        
        strcpy(new_rule->dependencies[new_rule->dep_count], dep_token);
        new_rule->dep_count++;
        dep_token = strtok(NULL, " \t");
    }
    
    data->rule_count++;
}

// 向当前规则添加命令
void add_command_to_current_rule(MakefileData *data, char *line) {
    if (data->rule_count == 0) return;  // 没有目标规则，应该已在语法检查中报错
    
    Rule *current_rule = &data->rules[data->rule_count - 1];
    if (current_rule->cmd_count < MAX_COMMANDS) {
        strcpy(current_rule->commands[current_rule->cmd_count], line);
        current_rule->cmd_count++;
    }
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
    //bool in_comment = false;
    

    while (fgets(line, MAX_LINE_LENGTH, file)) {
        line_num++;
        char processed_line[MAX_LINE_LENGTH];
        strcpy(processed_line, line);
        
        // 移除行尾换行符
        size_t len = strlen(processed_line);
        if (len > 0 && processed_line[len - 1] == '\n') {
            processed_line[len - 1] = '\0';
        }
        
        // 移除注释
        char *comment_pos = strchr(processed_line, '#');
        if (comment_pos) {
            *comment_pos = '\0';
        }
        
        // 处理空白行
        char *trimmed_line = trim_whitespace(processed_line);
        if (strlen(trimmed_line) == 0) {
            continue;
        }
        //int current_rule_index = -1;
        // 判断是目标行还是命令行
        if (strchr(trimmed_line, ':') != NULL) {
            // 目标行解析
            parse_target_line(data, trimmed_line, line_num);
            //current_rule_index = data->rule_count - 1;
        } else if (line[0] == '\t') {
            // 命令行(保留原始缩进后的内容)
            char *cmd = trim_whitespace(line + 1);  // 跳过Tab
            if (strlen(cmd) > 0) {
                add_command_to_current_rule(data, cmd);
            }
        }
    }

    fclose(file);
    
    // 检查依赖有效性
    check_dependencies(data);
    
    // 输出所有错误
    for (int i = 0; i < data->error_count; i++) {
        printf("%s\n", data->errors[i]);
    }
    
    return data->error_count > 0 ? 1 : 0;
}

