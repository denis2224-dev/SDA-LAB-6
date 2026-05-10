#ifndef FILE_OPS_H
#define FILE_OPS_H

#include <stddef.h>
#include <stdio.h>

#include "warehouse.h"

int ensure_file_exists(const char *path);
int parse_record_line(const char *line, WarehouseRecord *record);
int format_record_line(const WarehouseRecord *record, char *buffer, size_t buffer_size);
int write_record_to_file(const char *path, const WarehouseRecord *record, int append);
int load_records_from_file(const char *path, WarehouseRecord **records, size_t *count);
int save_records_to_file(const char *path, const WarehouseRecord *records, const int *priorities, size_t count, int append);
int copy_file_to_beginning(const char *source_path, const char *target_path);
int reopen_file_read(const char *path, FILE **file_out);
int delete_file_with_confirmation(const char *path, int confirmed);

#endif
