#pragma once

#include <stdio.h>

void strip(char *str);
char *get_table_element(char ***table, size_t lines, size_t fiels, size_t max_fiels_size, size_t num_of_line, size_t num_of_field);
void csv_read(char ***table, size_t lines, size_t fields, size_t max_field_size, int *get_lines, int *get_fields, FILE *fp);
