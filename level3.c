#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>  // 用于文件状态检查
#include <time.h>      // 用于时间戳处理
#include "level3.h"

// 创建队列
Queue* create_queue() {
    Queue* q = (Queue*)malloc(sizeof(Queue));
    q->front = 0;
    q->rear = -1;
    q->size = 0;
    return q;
}

// 入队操作
void enqueue(Queue* q, int value) {
    if (q->size < MAX_NODES) {
        q->rear = (q->rear + 1) % MAX_NODES;
        q->items[q->rear] = value;
        q->size++;
    }
}

// 出队操作
int dequeue(Queue* q) {
    if (q->size > 0) {
        int value = q->items[q->front];
        q->front = (q->front + 1) % MAX_NODES;
        q->size--;
        return value;
    }
    return -1;
}

// 检查队列是否为空
int is_empty(Queue* q) {
    return q->size == 0;
}

// 查找节点在图中的索引
int find_node_index(DependencyGraph* graph, const char* name) {
    for (int i = 0; i < graph->node_count; i++) {
        if (strcmp(graph->nodes[i], name) == 0) {
            return i;
        }
    }
    return -1;
}

// 向图中添加节点(去重)
void add_node(DependencyGraph* graph, const char* name) {
    if (find_node_index(graph, name) == -1 && graph->node_count < MAX_NODES) {
        graph->nodes[graph->node_count] = (char*)malloc(MAX_FILENAME_LEN);
        strcpy(graph->nodes[graph->node_count], name);
        graph->node_count++;
    }
}

// 构建依赖图
DependencyGraph* build_dependency_graph(MakefileData* data) {
    DependencyGraph* graph = (DependencyGraph*)malloc(sizeof(DependencyGraph));
    memset(graph, 0, sizeof(DependencyGraph));

    // 1. 收集所有节点(目标和依赖)
    // 收集目标节点
    for (int i = 0; i < data->rule_count; i++) {
        add_node(graph, data->rules[i].target);
    }

    // 收集依赖节点
    for (int i = 0; i < data->rule_count; i++) {
        for (int j = 0; j < data->rules[i].dep_count; j++) {
            add_node(graph, data->rules[i].dependencies[j]);
        }
    }

    // 2. 构建邻接表和入度
    for (int i = 0; i < data->rule_count; i++) {
        Rule* rule = &data->rules[i];
        int target_idx = find_node_index(graph, rule->target);
        
        if (target_idx == -1) continue;

        // 为每个依赖添加边: 依赖 -> 目标
        for (int j = 0; j < rule->dep_count; j++) {
            int dep_idx = find_node_index(graph, rule->dependencies[j]);
            
            if (dep_idx == -1) continue;

            // 添加邻接关系
            if (graph->adj_size[dep_idx] < MAX_ADJACENCY) {
                graph->adjacency[dep_idx][graph->adj_size[dep_idx]] = target_idx;
                graph->adj_size[dep_idx]++;
                graph->in_degree[target_idx]++;
            }
        }
    }

    return graph;
}

// 打印依赖关系图
void print_dependency_graph(DependencyGraph* graph) {
    printf("===== 依赖关系图 =====\n");
    for (int i = 0; i < graph->node_count; i++) {
        printf("节点: %s\n", graph->nodes[i]);
        printf("  入度: %d\n", graph->in_degree[i]);
        printf("  依赖它的节点: ");
        
        for (int j = 0; j < graph->adj_size[i]; j++) {
            int adj_idx = graph->adjacency[i][j];
            printf("%s ", graph->nodes[adj_idx]);
        }
        printf("\n\n");
    }
}

// 拓扑排序(Kahn算法) - 返回排序结果
int* topological_sort(DependencyGraph* graph, int* order_size) {
    // 复制入度数组(避免修改原图)
    int* in_degree_copy = (int*)malloc(graph->node_count * sizeof(int));
    memcpy(in_degree_copy, graph->in_degree, graph->node_count * sizeof(int));

    Queue* q = create_queue();
    int* result = (int*)malloc(graph->node_count * sizeof(int));
    *order_size = 0;

    // 初始化队列: 入度为0的节点
    for (int i = 0; i < graph->node_count; i++) {
        if (in_degree_copy[i] == 0) {
            enqueue(q, i);
        }
    }

    // 执行Kahn算法
    while (!is_empty(q)) {
        int u = dequeue(q);
        result[(*order_size)++] = u;

        // 处理所有邻接节点
        for (int i = 0; i < graph->adj_size[u]; i++) {
            int v = graph->adjacency[u][i];
            in_degree_copy[v]--;
            
            if (in_degree_copy[v] == 0) {
                enqueue(q, v);
            }
        }
    }

    free(in_degree_copy);
    free(q);
    return result;
}

// 释放依赖图内存
void free_graph(DependencyGraph* graph) {
    for (int i = 0; i < graph->node_count; i++) {
        free(graph->nodes[i]);
    }
    free(graph);
}

// 辅助函数：判断名称是否为目标(存在对应的规则)
int is_target(MakefileData* data, const char* name) {
    for (int i = 0; i < data->rule_count; i++) {
        if (strcmp(data->rules[i].target, name) == 0) {
            return 1; // 是目标
        }
    }
    return 0; // 不是目标
}


// 辅助函数：获取文件修改时间戳
time_t get_file_mtime(const char* filename) {
    struct stat buffer;
    if (stat(filename, &buffer) == 0) {
        return buffer.st_mtime; // 返回修改时间
    }
    return -1; // 文件不存在
}

// 辅助函数：根据目标名称查找规则
Rule* find_rule_by_target(MakefileData* data, const char* target) {
    for (int i = 0; i < data->rule_count; i++) {
        if (strcmp(data->rules[i].target, target) == 0) {
            return &data->rules[i];
        }
    }
    return NULL;
}

// 辅助函数：递归检查并构建依赖
void build_dependency(MakefileData* data, DependencyGraph* graph, 
                            int* topo_order, int order_size, const char* dep_name) {
    // 如果依赖不是目标，不需要构建
    if (!is_target(data, dep_name)) {
        return;
    }

    // 找到依赖在拓扑排序中的位置并处理
    for (int i = 0; i < order_size; i++) {
        int node_idx = topo_order[i];
        const char* node_name = graph->nodes[node_idx];
        
        if (strcmp(node_name, dep_name) == 0) {
            Rule* rule = find_rule_by_target(data, node_name);
            if (!rule) return;

            // 检查依赖目标是否需要构建
            int target_exists = file_exists(node_name);
            time_t target_mtime = target_exists ? get_file_mtime(node_name) : 0;
            int need_rebuild = !target_exists;

            // 检查依赖的依赖是否需要构建（递归）
            if (!need_rebuild) {
                for (int j = 0; j < rule->dep_count; j++) {
                    const char* sub_dep = rule->dependencies[j];
                    // 先递归处理子依赖
                    build_dependency(data, graph, topo_order, order_size, sub_dep);
                    
                    // 检查子依赖是否更新
                    if (file_exists(sub_dep) && get_file_mtime(sub_dep) > target_mtime) {
                        need_rebuild = 1;
                        break;
                    }
                }
            }

            // 构建依赖目标
            if (need_rebuild) {
                printf("\n递归构建依赖目标: %s\n", node_name);
                
                // 检查所有子依赖是否存在
                int all_sub_deps_exist = 1;
                for (int j = 0; j < rule->dep_count; j++) {
                    const char* sub_dep = rule->dependencies[j];
                    if (!file_exists(sub_dep)) {
                        if (data->error_count < 100) {
                            sprintf(data->errors[data->error_count], 
                                    "错误: 依赖目标 %s 的依赖 %s 不存在 (行号: %d)",
                                    node_name, sub_dep, rule->line_num);
                            data->error_count++;
                        }
                        printf("  错误: 子依赖 %s 不存在\n", sub_dep);
                        all_sub_deps_exist = 0;
                    }
                }

                // 执行构建命令
                if (all_sub_deps_exist) {
                    for (int j = 0; j < rule->cmd_count; j++) {
                        printf("    执行命令: %s\n", rule->commands[j]);
                        // 实际执行命令
                        my_system(rule->commands[j]);
                    }
                }
            }
            return;
        }
    }
}

// 任务3：按拓扑顺序检查时间戳并判断是否需要构建
void check_timestamps_and_build(MakefileData* data, DependencyGraph* graph, 
                               int* topo_order, int order_size) {
    printf("\n===== 开始时间戳检查与构建判断 =====\n");

    for (int i = 0; i < order_size; i++) {
        int node_idx = topo_order[i];
        const char* node_name = graph->nodes[node_idx];

        // 只处理目标节点
        if (!is_target(data, node_name)) {
            continue;
        }

        Rule* rule = find_rule_by_target(data, node_name);
        if (!rule) {
            continue;
        }

        printf("\n处理目标: %s (行号: %d)\n", node_name, rule->line_num);
        
        // 先递归构建所有依赖
        for (int j = 0; j < rule->dep_count; j++) {
            const char* dep_name = rule->dependencies[j];
            printf("  检查依赖: %s\n", dep_name);
            build_dependency(data, graph, topo_order, order_size, dep_name);
        }

        // 检查当前目标是否需要构建
        int target_exists = file_exists(node_name);
        time_t target_mtime = target_exists ? get_file_mtime(node_name) : 0;
        int need_rebuild = !target_exists;

        // 检查依赖是否有更新（经过递归构建后）
        if (!need_rebuild) {
            for (int j = 0; j < rule->dep_count; j++) {
                const char* dep_name = rule->dependencies[j];
                if (file_exists(dep_name) && get_file_mtime(dep_name) > target_mtime) {
                    printf("  依赖 %s 比目标更新 (%.2f秒)\n", 
                           dep_name, difftime(get_file_mtime(dep_name), target_mtime));
                    need_rebuild = 1;
                    break;
                }
            }
        }

        // 执行构建
        if (need_rebuild) {
            printf("  开始构建 %s...\n", node_name);
            
            // 检查所有依赖是否存在（经过递归构建后应该都存在）
            int all_deps_exist = 1;
            for (int j = 0; j < rule->dep_count; j++) {
                const char* dep_name = rule->dependencies[j];
                if (!file_exists(dep_name)) {
                    if (data->error_count < 100) {
                        sprintf(data->errors[data->error_count], 
                                "错误: 目标 %s 的依赖 %s 不存在 (行号: %d)",
                                node_name, dep_name, rule->line_num);
                        data->error_count++;
                    }
                    printf("  错误: 依赖 %s 不存在\n", dep_name);
                    all_deps_exist = 0;
                }
            }

            if (all_deps_exist) {
                for (int j = 0; j < rule->cmd_count; j++) {
                    printf("    执行命令: %s\n", rule->commands[j]);
                    my_system(rule->commands[j]); // 实际执行命令
                }
            }
        } else {
            printf("  目标已是最新，无需构建\n");
        }
    }

    // 打印所有错误信息
    if (data->error_count > 0) {
        printf("\n===== 构建错误汇总 =====\n");
        for (int i = 0; i < data->error_count; i++) {
            printf("%s\n", data->errors[i]);
        }
    }
}

// 测试函数，接收MakefileData参数
void test(MakefileData* data) {
    if (data == NULL || data->rule_count == 0) {
        printf("无效的Makefile数据或没有规则\n");
        return;
    }
    
    // 构建依赖图
    DependencyGraph* graph = build_dependency_graph(data);
    
    // 打印依赖图
    print_dependency_graph(graph);
    
    // 拓扑排序
    int order_size;
    int* topo_order = topological_sort(graph, &order_size);
    
    // 打印拓扑排序结果
    printf("===== 拓扑排序结果 =====\n");
    for (int i = 0; i < order_size; i++) {
        printf("%s ", graph->nodes[topo_order[i]]);
    }
    printf("\n");
    
    // 执行时间戳检查和构建判断
    check_timestamps_and_build(data, graph, topo_order, order_size);
    
    // 释放资源
    free(topo_order);
    free_graph(graph);
}


