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
 * 检查Makefile的静态语法规则，忽略注释内容
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
        strcpy(processed_line, line);
        
        // 去除行尾的换行符
        size_t len = strlen(processed_line);
        if (len > 0 && processed_line[len - 1] == '\n') {
            processed_line[len - 1] = '\0';
        }

        // 移除注释，如果是纯注释行或空行则跳过检查
        if (remove_comments(processed_line)) {
            continue;
        }

        // 检查是否为目标行（包含冒号）
        if (strchr(processed_line, ':') != NULL) {
            // 验证目标行格式：冒号不能是第一个字符
            if (processed_line[0] == ':') {
                printf("Line%d: Invalid target definition (starts with colon)\n", line_num);
                error_count++;
            } else {
                rule_defined = true;  // 标记已找到合法目标行
            }
        }
        // 不是目标行则检查是否为命令行
        else {
            // 检查命令行是否以Tab开头
            if (processed_line[0] == '\t') {
                // 检查命令是否出现在目标行之前
                if (!rule_defined) {
                    printf("Line%d: Command found before rule\n", line_num);
                    error_count++;
                }
            }
            // 既不是目标行也不是以Tab开头的命令行
            else {
                // 检查是否是缺少冒号的目标行
                printf("Line%d: Missing colon in target definition\n", line_num);
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
