#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include "level4.h"

// 使用execvp的自定义system()函数
int my_system(const char *command) {
    if (command == NULL) {
        // 命令为NULL时返回非0值，表示存在shell
        return 1;
    }

    pid_t pid = fork();  // 创建子进程
    if (pid < 0) {
        // fork失败
        perror("fork failed");
        return -1;
    } 
    // 子进程
    else if (pid == 0) {
        char *args[]={
        "sh",
        "-c",
        (char*)command,
        NULL
        };
        execvp("sh",args);
        /*
        // 拆分命令和参数
        char *args[256];
        char *cmd_copy = strdup(command);
        int i = 0;
        
        // 按空格拆分命令
        args[i] = strtok(cmd_copy, " ");
        while (args[i] != NULL && i < 255) {
            args[++i] = strtok(NULL, " ");
        }
        
   
        execvp(args[0], args);
        */
        
        // 如果execvp执行失败才会到这里
        perror("execvp failed");
        //free(cmd_copy);
        exit(EXIT_FAILURE);
    } 
    // 父进程
    else {
        int status;
        // 等待子进程结束
        if (waitpid(pid, &status, 0) == -1) {
            perror("waitpid failed");
            return -1;
        }
        
        // 处理子进程退出状态
        if (WIFEXITED(status)) {
            return WEXITSTATUS(status);//非零值
        } else {
            // 子进程被信号终止等异常情况
            return -1;
        }
    }
}

// 执行构建步骤，任一命令失败则立即停止
int run_build_steps() {
    // 定义构建步骤
    const char *build_steps[] = {
        "echo 开始构建...",
        "gcc -c main.c -o main.o",    // 编译main.c
        "gcc -c utils.c -o utils.o",  // 编译utils.c
        "gcc main.o utils.o -o myapp",// 链接目标文件
        "echo 构建完成！",
        NULL  // 步骤结束标记
    };

    // 执行所有构建步骤
    for (int i = 0; build_steps[i] != NULL; i++) {
        printf("\n[步骤 %d] 执行命令: %s\n", i + 1, build_steps[i]);
        
        // 执行命令并获取返回状态
        int exit_status = my_system(build_steps[i]);
        
        // 检查命令执行结果
        if (exit_status != 0) {
            fprintf(stderr, "[错误] 步骤 %d 执行失败，退出状态码: %d\n", 
                    i + 1, exit_status);
            fprintf(stderr, "[终止] 构建过程已中断\n");
            return exit_status;  // 返回失败状态码
        }
        
        printf("[成功] 步骤 %d 执行完成\n", i + 1);
    }

    return 0;  // 所有步骤执行成功
}

