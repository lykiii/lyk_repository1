#ifndef LEVEL3_H
#define LEVEL3_H

#include"level2.h"

#define MAX_FILENAME_LEN 33   // 文件名最大长度(含结束符)
#define MAX_DEPENDENCIES 50   // 每个目标最大依赖数量
#define MAX_COMMANDS 20       // 每个目标最大命令数量
#define MAX_LINE_LENGTH 1024
#define MAX_TARGETS 100  // 最大目标数量
#define MAX_NODES 256    // 最大节点数量
#define MAX_ADJACENCY 64 // 每个节点最大邻接数量

// 依赖图数据结构
typedef struct {
    char* nodes[MAX_NODES];   // 所有节点(目标和依赖文件)
    int node_count;           // 节点总数
    int adjacency[MAX_NODES][MAX_ADJACENCY];  // 邻接表
    int adj_size[MAX_NODES];  // 每个节点的邻接数量
    int in_degree[MAX_NODES]; // 每个节点的入度
} DependencyGraph;

// 队列结构(用于Kahn算法)
typedef struct {
    int items[MAX_NODES];
    int front;
    int rear;
    int size;
} Queue;

Queue* create_queue();
void enqueue(Queue* q, int value);
int dequeue(Queue* q);
int is_empty(Queue* q);
int find_node_index(DependencyGraph* graph, const char* name);
void add_node(DependencyGraph* graph, const char* name);
void print_dependency_graph(DependencyGraph* graph);
int* topological_sort(DependencyGraph* graph, int* order_size);
void free_graph(DependencyGraph* graph);
int is_target(MakefileData* data, const char* name);
time_t get_file_mtime(const char* filename);
Rule* find_rule_by_target(MakefileData* data, const char* target);
void build_dependency(MakefileData* data, DependencyGraph* graph, int* topo_order, int order_size, const char* dep_name);
void check_timestamps_and_build(MakefileData* data, DependencyGraph* graph, int* topo_order, int order_size);
void test(MakefileData* data);
#endif
