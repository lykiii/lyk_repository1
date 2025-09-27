#include "preprocessing.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>
// 定义有效的命令选项



const char *valid_options[] = {
    "--help",
    "--version",
    "--verbose",
    "--output",
    NULL  // 结束标记
};


int remove_comments(char *line) {
    if (line == NULL || *line == '\0') {
        return 1;
    }
    
    // 查找注释开始位置
    char *comment_start = strchr(line, '#');
    
    // 如果有注释，截断注释部分
    if (comment_start != NULL) {
        *comment_start = '\0';
    }
    
    // 检查处理后是否为空行（只包含空白字符）
    int is_empty = 1;
    for (int i = 0; line[i] != '\0'; i++) {
        if (!isspace((unsigned char)line[i])) {
            is_empty = 0;
            break;
        }
    }
    
    return is_empty;
}


/**
 * 检查Makefile的静态语法规则，包括变量定义与动态替换语法
 * @param filename 要检查的Makefile路径
 * @return 0表示检查通过，1表示发现错误
 */
int check_makefile_syntax(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("无法打开文件");
        return 1;
    }

    char line[MAX_LINE_LENGTH];
    int line_num = 0;
    bool rule_defined = false;  // 标记是否已出现过合法的目标行
    int error_count = 0;

    while (fgets(line, MAX_LINE_LENGTH, file) != NULL) {
        line_num++;
        // 保存原始行用于处理（避免修改原始输入）
        char processed_line[MAX_LINE_LENGTH];
        strncpy(processed_line, line, MAX_LINE_LENGTH - 1);
        processed_line[MAX_LINE_LENGTH - 1] = '\0';  // 确保字符串终止

        // 移除行尾的换行符和空白字符（使用提供的函数）
        trim_trailing_whitespace(processed_line);

        // 移除注释，如果是纯注释行或空行则跳过检查
        if (remove_comments(processed_line)) {
            continue;
        }

        // 处理行首空白（手动实现，不依赖trim_whitespace）
        char *trimmed_line = processed_line;
        while (isspace((unsigned char)*trimmed_line)) {
            trimmed_line++;  // 仅跳过开头空白，不修改原始字符串
        }
        if (*trimmed_line == '\0') {
            continue;  // 空行跳过
        }

        // 检查是否为变量定义行（含等号，且不是目标行）
        bool is_var_def = false;
        char *equal_pos = strchr(trimmed_line, '=');
        if (equal_pos != NULL && strchr(trimmed_line, ':') == NULL) {
            // 提取变量名（等号左边）
            size_t var_name_len = equal_pos - trimmed_line;
            if (var_name_len >= MAX_LINE_LENGTH) {
                printf("Line%d: Variable name too long (exceeds %d characters)\n", 
                       line_num, MAX_LINE_LENGTH - 1);
                error_count++;
                is_var_def = true;
                continue;
            }

            char var_name[MAX_LINE_LENGTH] = {0};
            strncpy(var_name, trimmed_line, var_name_len);
            trim_trailing_whitespace(var_name);  // 移除变量名尾部空白

            // 处理变量名开头空白（手动跳过）
            char *clean_var_name = var_name;
            while (isspace((unsigned char)*clean_var_name)) {
                clean_var_name++;
            }

            // 变量名合法性检查
            if (*clean_var_name == '\0') {  // 变量名为空
                printf("Line%d: Invalid variable definition (empty variable name)\n", line_num);
                error_count++;
            } else if (!isalpha((unsigned char)*clean_var_name) && *clean_var_name != '_') {  // 首字符非法
                printf("Line%d: Invalid variable name '%s' (must start with letter or underscore)\n", 
                       line_num, clean_var_name);
                error_count++;
            } else {  // 检查非法字符
                bool has_invalid_char = false;
                for (char *c = clean_var_name; *c; c++) {
                    if (!isalnum((unsigned char)*c) && *c != '_') {
                        has_invalid_char = true;
                        break;
                    }
                }
                if (has_invalid_char) {
                    printf("Line%d: Invalid character in variable name '%s'\n", 
                           line_num, clean_var_name);
                    error_count++;
                } else {
                    is_var_def = true;  // 标记为合法变量定义行
                }
            }
        }

        // 如果是变量定义行，跳过后续目标/命令行检查
        if (is_var_def) {
            continue;
        }

        // 检查是否为目标行（包含冒号）
        if (strchr(trimmed_line, ':') != NULL) {
            // 验证目标行格式：冒号不能是第一个字符
            if (*trimmed_line == ':') {
                printf("Line%d: Invalid target definition (starts with colon)\n", line_num);
                error_count++;
            } else {
                rule_defined = true;  // 标记已找到合法目标行
            }
        }
        // 不是目标行则检查是否为命令行
        else {
            // 检查命令行是否以Tab开头（原始行检查，保留行首空白）
            bool is_tab_start = (processed_line[0] == '\t');
            
            // 处理命令内容（跳过Tab后的空白）
            char *cmd_content = processed_line;
            if (is_tab_start) {
                cmd_content++;  // 跳过Tab
                while (isspace((unsigned char)*cmd_content)) {
                    cmd_content++;  // 跳过命令前的空白
                }
            }

            if (is_tab_start) {
                // 检查命令是否出现在目标行之前
                if (!rule_defined) {
                    printf("Line%d: Command found before rule\n", line_num);
                    error_count++;
                }
                // 检查空命令行（仅Tab无内容）
                if (*cmd_content == '\0') {
                    printf("Line%d: Empty command line (only Tab, no content)\n", line_num);
                    error_count++;
                }
            }
            // 既不是目标行也不是以Tab开头的命令行，且不是变量定义行
            else {
                printf("Line%d: Invalid line (not target, command, or variable definition)\n", line_num);
                error_count++;
            }
        }
    }

    fclose(file);

    if (error_count == 0) {
        printf("Makefile syntax check passed. No errors found.\n");
        return 0;
    } else {
        printf("Found %d syntax error(s) in Makefile.\n", error_count);
        return 1;
    }
}





/*
// 辅助函数：去除字符串首尾空白（保留原实现，优化注释）
char* trim_whitespace1(char *str) {
    if (str == NULL) {  // 新增：处理空指针，避免崩溃
        return NULL;
    }

    // 移除开头空白（空格、Tab、换行等）
    while (isspace((unsigned char)*str)) {
        str++;
    }

    // 移除结尾空白（仅当字符串非空时处理）
    if (*str != '\0') {
        char *end = str + strlen(str) - 1;
        while (end > str && isspace((unsigned char)*end)) {
            end--;
        }
        end[1] = '\0';  // 在结尾空白后添加终止符
    }

    return str;
}
*/

// 去除行尾的空白字符（空格、制表符、换行符等）
void trim_trailing_whitespace(char *str) {
    if (str == NULL || *str == '\0') return;

    int len = strlen(str);
    // 从末尾向前查找第一个非空白字符
    while (len > 0 && isspace((unsigned char)str[len - 1])) {
        len--;
    }
    str[len] = '\0';  // 截断字符串
}

// 处理Makefile的主函数
void process_makefile(int verbose) {
    FILE *input_file;
    FILE *output_file = NULL;
    char line[MAX_LINE_LENGTH];

    // 打开当前目录下的Makefile
    input_file = fopen("./Makefile", "r");
    if (input_file == NULL) {
        perror("错误：无法打开Makefile");
        return;
    }

    // 如果是调试模式，打开输出文件
    if (verbose) {
        output_file = fopen("Minimake_cleared.mk", "w");
        if (output_file == NULL) {
            perror("警告：无法创建输出文件Minimake_cleared.mk");
            // 即使输出文件创建失败，仍继续处理但不写入文件
        }
    }

    printf("正在处理Makefile...\n\n");

    // 逐行读取并处理
    while (fgets(line, MAX_LINE_LENGTH, input_file) != NULL) {
        char *original_line = strdup(line);  // 保存原始行用于处理

        // 1. 去除注释（#及其后面的内容）
        char *comment_start = strchr(original_line, '#');
        if (comment_start != NULL) {
            *comment_start = '\0';  // 截断注释部分
        }

        // 2. 去除行尾空格
        trim_trailing_whitespace(original_line);

        // 3. 过滤空行（处理后仅含空白字符的行）
        int is_empty = 1;
        for (int i = 0; original_line[i] != '\0'; i++) {
            if (!isspace((unsigned char)original_line[i])) {
                is_empty = 0;
                break;
            }
        }

        // 输出清理后的结果
        if (!is_empty) {
            // 打印到控制台（中间结果）
            printf("处理后: %s\n", original_line);

            // 如果是调试模式且输出文件打开成功，写入文件
            if (verbose && output_file != NULL) {
                fprintf(output_file, "%s\n", original_line);
            }
        }

        free(original_line);  // 释放动态分配的内存
    }

    // 关闭文件
    fclose(input_file);
    if (verbose && output_file != NULL) {
        fclose(output_file);
        printf("\n调试模式：清理后的内容已保存到Minimake_cleared.mk\n");
    }

    printf("\nMakefile处理完成\n");
}


// 显示帮助信息，包含用法说明
void show_help(const char *program_name) {
    printf("用法: %s [选项]...\n", program_name);
    printf("一个带参数验证功能的示例程序\n\n");
    printf("有效的选项:\n");
    printf("  --help      显示此帮助信息并退出\n");
    printf("  --version   显示程序版本信息并退出\n");
    printf("  --verbose   启用详细输出模式\n");
    printf("  --output    指定输出文件路径（需后跟文件名）\n");
    printf("\n示例:\n");
    printf("  %s --verbose\n", program_name);
    printf("  %s result.txt\n", program_name);
}

// 显示版本信息
void show_version() {
    printf("参数检查程序 版本 1.0\n");
}

// 检查参数是否有效
int is_valid_option(const char *option) {
    for (int i = 0; valid_options[i] != NULL; i++) {
        if (strcmp(option, valid_options[i]) == 0) {
            return 1;  // 有效参数
        }
    }
    return 0;  // 无效参数
}
