#include <stdio.h>
#include <string.h>
#include "preprocessing.h"
#include "level2.h"
#include "level3.h"
#include "level4.h"
#include "level5.h"


int main(int argc, char *argv[])
{
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
        
        else if(argv[i][0]== '-'){
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
    
    
    // 检查命令行参数
    if (argc ==1) {
        printf("用法: %s <目标名称>\n", argv[0]);
        printf("示例: %s app\n", argv[0]);
        return 1;
    }
    
    // 处理Makefile
    process_makefile(verbose);
    
//解析Makefile文件--------------------------------------------------------------------------  
    char *makefile_path = "./Makefile";
    if(verbose){
    makefile_path = "./Minimake_cleared.mk";
    }
     
    // 支持通过参数指定其他文件（如预处理后的文件）
   /* if (argc > 1) {
        makefile_path = argv[1];
    }
*/
    if(check_makefile_syntax(makefile_path)!=0){ 
    printf("Makefile语法错误，无法继续执行\n");
    return 1;
    }
    
    MakefileData data;
    init_makefile_data(&data);
    
    int result = parse_and_check_makefile(makefile_path, &data);
    if(result!=0){
    printf("Makefile解析失败，无法继续执行\n");
    return 1;
    }
    
    
    // 调试输出：打印所有解析的规则
    printf("\n解析到 %d 个规则:\n", data.rule_count);
    for (int i = 0; i < data.rule_count; i++) {
        Rule *rule = &data.rules[i];
        printf("目标: %s (行号: %d)\n", rule->target, rule->line_num);
        printf("  依赖(%d个): ", rule->dep_count);
        for (int j = 0; j < rule->dep_count; j++) {
            printf("%s ", rule->dependencies[j]);
        }
        printf("\n  命令(%d个):\n", rule->cmd_count);
        for (int j = 0; j < rule->cmd_count; j++) {
            printf("    %s\n", rule->commands[j]);
        }
    }
//----------------------------------------------------------------------------------------  
    
 
    //execute_target(&data, argv[1]);//第一次编译执行函数
    
    
    //构建依赖图，拓扑排序，执行时间戳检查和构建判断
    test(&data);
    
    
    return 0;
    }

