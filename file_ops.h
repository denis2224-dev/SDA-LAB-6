#ifndef FILE_OPS_H
#define FILE_OPS_H

#include <stddef.h>
#include <stdio.h>

#include "warehouse.h"

// Creates file if missing by opening in append mode.
int ensure_file_exists(const char *path);
// Parses one semicolon-separated line into WarehouseRecord.
int parse_record_line(const char *line, WarehouseRecord *record);
// Formats one WarehouseRecord into semicolon-separated line.
int format_record_line(const WarehouseRecord *record, char *buffer, size_t buffer_size);
// Writes one record (append or rewrite mode).
int write_record_to_file(const char *path, const WarehouseRecord *record, int append);
// Loads all valid records from file into dynamic array.
int load_records_from_file(const char *path, WarehouseRecord **records, size_t *count);
// Saves a record array to file (append or rewrite mode).
int save_records_to_file(const char *path, const WarehouseRecord *records, const int *priorities, size_t count, int append);
// Copies source file at the beginning of target file.
int copy_file_to_beginning(const char *source_path, const char *target_path);
// Reopens an existing file for reading.
int reopen_file_read(const char *path, FILE **file_out);
// Deletes file only when confirmed is non-zero.
int delete_file_with_confirmation(const char *path, int confirmed);

#endif
