#include "level5.h"
#include <stdio.h>
#include <stdlib.h>


// 功能：根据变量名查找值，未找到返回NULL
const char* find_variable(MakefileData *data, const char *var_name) {
    for (int i = 0; i < data->var_count; i++) {
        if (strcmp(data->variables[i].name, var_name) == 0) {
            return data->variables[i].value;
        }
    }
    return NULL;
}


// 功能：新增变量，若已存在则覆盖；处理合法性检查
void add_or_update_variable(MakefileData *data, const char *var_name, const char *var_value, int line_num) {
    // 1. 检查变量名合法性（不能含空格、$、{、(）
    for (const char *p = var_name; *p; p++) {
        if (isspace((unsigned char)*p) || *p == '$' || *p == '{' || *p == '(') {
            add_error(data, "Line%d: Invalid variable name '%s' (contains spaces/special chars)", line_num, var_name);
            return;
        }
    }

    // 2. 检查长度限制
    if (strlen(var_name) >= MAX_VAR_NAME) {
        add_error(data, "Line%d: Variable name '%s' too long (max %d chars)", line_num, var_name, MAX_VAR_NAME-1);
        return;
    }
    if (strlen(var_value) >= MAX_VAR_VALUE) {
        add_error(data, "Line%d: Value of '%s' too long (max %d chars)", line_num, var_name, MAX_VAR_VALUE-1);
        return;
    }

    // 3. 覆盖已有变量/新增变量
    for (int i = 0; i < data->var_count; i++) {
        if (strcmp(data->variables[i].name, var_name) == 0) {
            strcpy(data->variables[i].value, var_value); // 覆盖旧值
            return;
        }
    }
    // 新增变量（未超过最大数量）
    if (data->var_count < MAX_VARIABLES) {
        strcpy(data->variables[data->var_count].name, var_name);
        strcpy(data->variables[data->var_count].value, var_value);
        data->var_count++;
    } else {
        add_error(data, "Line%d: Too many variables (max %d)", line_num, MAX_VARIABLES);
    }
}

// 功能：处理 $(VAR) / ${VAR} 嵌套展开（如 PATH=$(HOME)/bin）
void recursive_expand(MakefileData *data, const char *input, char *output, int depth, int line_num) {
    if (depth >= MAX_EXPAND_DEPTH) {
        add_error(data, "Line%d: Variable expansion loop (max depth %d)", line_num, MAX_EXPAND_DEPTH);
        strcpy(output, "");
        return;
    }

    const char *p = input;
    char *out_p = output;
    *out_p = '\0';

    while (*p != '\0' && out_p - output < MAX_EXPANDED_LEN - 1) {
        // 识别变量开头：$( 或 ${
        if (*p == '$' && (p[1] == '(' || p[1] == '{')) {
            char var_name[MAX_VAR_NAME] = {0};
            const char *var_start = p + 2;
            const char *var_end = (p[1] == '(') ? strchr(var_start, ')') : strchr(var_start, '}');

            if (var_end == NULL) { // 无闭合符（如 $(CC）
                add_error(data, "Line%d: Unclosed variable '%s'", line_num, p);
                *out_p++ = *p++;
                continue;
            }

            // 提取变量名
            size_t var_len = var_end - var_start;
            if (var_len >= MAX_VAR_NAME) {
                add_error(data, "Line%d: Variable name too long '%.*s'", line_num, (int)var_len, var_start);
                p = var_end + 1;
                continue;
            }
            strncpy(var_name, var_start, var_len);

            // 展开变量（未定义则为空）
            const char *var_val = find_variable(data, var_name);
            if (var_val == NULL) {
                add_error(data, "Line%d: Undefined variable '%s'", line_num, var_name);
                var_val = "";
            }

            // 递归展开变量值中的嵌套变量
            char expanded_val[MAX_EXPANDED_LEN] = {0};
            recursive_expand(data, var_val, expanded_val, depth + 1, line_num);

            // 追加到输出
            strncat(out_p, expanded_val, MAX_EXPANDED_LEN - (out_p - output) - 1);
            out_p += strlen(expanded_val);
            p = var_end + 1; // 跳过当前变量
        } else {
            // 非变量字符直接复制
            *out_p++ = *p++;
            *out_p = '\0';
        }
    }

    // 防止溢出
    if (out_p - output >= MAX_EXPANDED_LEN - 1) {
        add_error(data, "Line%d: Expanded string too long (max %d chars)", line_num, MAX_EXPANDED_LEN - 1);
        output[MAX_EXPANDED_LEN - 1] = '\0';
    }
}

// 变量展开对外接口 ---------------------------------------------------- 
void expand_variable(MakefileData *data, const char *input, char *output, int line_num) {
    recursive_expand(data, input, output, 0, line_num);
}

// 变量定义解析函数 ---------------------------------------------------- 
// 功能：识别 "VAR = VALUE" 语法，返回是否为变量行
bool parse_variable_definition(MakefileData *data, const char *line, int line_num) {
    char line_copy[MAX_LINE_LENGTH] = {0};
    strcpy(line_copy, line);

    // 分割变量名和值（以第一个 = 为界）
    char *equal_pos = strchr(line_copy, '=');
    if (equal_pos == NULL) {
        return false; // 不是变量行
    }

    // 提取变量名（= 左边 trim 空白）
    *equal_pos = '\0';
    char *var_name = trim_whitespace(line_copy);
    if (strlen(var_name) == 0) {
        add_error(data, "Line%d: Empty variable name", line_num);
        return true; // 是变量行但格式错误
    }

    // 提取变量值（= 右边 trim 空白，展开嵌套变量）
    char *var_val_raw = trim_whitespace(equal_pos + 1);
    char var_val_expanded[MAX_VAR_VALUE] = {0};
    expand_variable(data, var_val_raw, var_val_expanded, line_num);

    // 存储变量
    add_or_update_variable(data, var_name, var_val_expanded, line_num);
    return true;
}


