#include <stdio.h>
#include <string.h>
#include "preprocessing.h"


int main(int argc, char *argv[]) {
    // 处理不带任何参数的情况
    if (argc == 1) {
        printf("没有提供任何参数。用法: %s [选项]...\n", argv[0]);
        printf("使用 '%s --help' 查看所有可用选项。\n", argv[0]);
        return 0;
    }

    // 遍历所有参数进行处理
    for (int i = 1; i < argc; i++) {
        // 检查是否是帮助命令
        if (strcmp(argv[i], "--help") == 0) {
            show_help(argv[0]);
            return 0;
        }
        // 检查是否是版本命令
        else if (strcmp(argv[i], "--version") == 0) {
            show_version();
            return 0;
        }
        // 处理需要附加参数的选项
        else if (strcmp(argv[i], "--output") == 0) {
            if (i + 1 >= argc) {
                printf("错误: 选项 '%s' 需要一个文件名作为参数\n", argv[i]);
                return 1;
            }
            printf("已指定输出文件: %s\n", argv[++i]);
        }
        // 处理详细模式
        else if (strcmp(argv[i], "--verbose") == 0) {
            printf("详细输出模式已启用\n");
        }
        else if (strcmp(argv[i], "-v") == 0) {
            printf("详细输出模式已启用\n");
        }
        // 检查未知参数
        else {
            printf("错误: 不正确的参数 '%s'\n", argv[i]);
            printf("使用 '%s --help' 查看所有可用选项。\n", argv[0]);
            return 1;
        }
    }
    int verbose = 0;

    // 解析命令行参数，检查是否为调试模式
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--verbose") == 0) {
            verbose = 1;
            break;
        }
    }
    // 处理Makefile
    process_makefile(verbose);
    
    
    const char *makefile_path = "./Makefile";
    
    // 支持通过参数指定其他文件（如预处理后的文件）
   /* if (argc > 1) {
        makefile_path = argv[1];
    }
*/
    return check_makefile_syntax(makefile_path);
    }



