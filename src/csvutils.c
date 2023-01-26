#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "csvutils.h"

/**
 * 清除字符串两边的空字符
*/
void strip(char *str)
{
    size_t start = 0, end = strlen(str) - 1;
    while (str[start] == ' ' || str[start] == '\t') {
        ++start;
    }
    while (str[end] == ' ' || str[end] == '\t' || str[end] == '\n') {
        --end;
    }
    strncpy(str, str + start, end - start + 1);
    str[end - start + 1] = '\0';
}

/**
 * 获取串表指定行的某一字段
*/
char *get_table_element(char ***table, size_t lines, size_t fiels, size_t max_fiels_size, size_t num_of_line, size_t num_of_field)
{
    return ((char *)table + sizeof(char) * (fiels * max_fiels_size * num_of_line + max_fiels_size * num_of_field));
}

/**
 * 读取文件csv文件内容，转化为串表
*/
void csv_read(char ***table, size_t lines, size_t fields, size_t max_field_size, int *get_lines, int *get_fields, FILE *fp)
{
    char line[1024];
    size_t min_fields = 0xffffffff;
    size_t num_of_line = 0;
    while (num_of_line < lines && fgets(line, sizeof(line), fp)) { // line: xx, xx, xxx\n
        size_t num_of_field = 0;
        // char field[1024];
        size_t pos = 0, last = pos;
        while (num_of_field < fields && line[pos] != '\n') {
            char *field = get_table_element(table, lines, fields, max_field_size, num_of_line, num_of_field);
            while (line[pos] != ',' && line[pos] != '\n') {
                ++pos;
            }   // line[pos] == ',' or line[pos] == '\n'
            strncpy(field, line + last, pos - last);
            field[pos - last] = '\0';
            strip(field);
            if (line[pos] != '\n') {
                ++pos;
                last = pos;
            }
            ++num_of_field;
        }
        if (num_of_field < min_fields) {
            min_fields = num_of_field;
        }
        ++num_of_line;
    }
    *get_lines = num_of_line;
    *get_fields = min_fields;
}

