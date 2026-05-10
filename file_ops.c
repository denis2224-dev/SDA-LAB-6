#include "file_ops.h"

#include <stdlib.h>
#include <string.h>

#define LINE_BUFFER_SIZE 512
#define TMP_FILE_NAME "data/.tmp_merge_experiment.txt"

// Shared validation used before formatting or writing records.
static int validate_record_fields(const WarehouseRecord *record) {
    if (record == NULL) {
        return 0;
    }
    if (record->owner_name[0] == '\0' ||
        record->owner_surname[0] == '\0' ||
        record->product_name[0] == '\0' ||
        record->manufacturer[0] == '\0') {
        return 0;
    }
    if (!validate_date(&record->contract_date)) {
        return 0;
    }
    if (record->wholesale_price <= 0.0 || record->unit_price <= 0.0 || record->quantity <= 0.0) {
        return 0;
    }
    if (!is_valid_unit((int)record->unit)) {
        return 0;
    }
    return 1;
}

int ensure_file_exists(const char *path) {
    FILE *file;

    if (path == NULL) {
        return 0;
    }

    file = fopen(path, "a");
    if (file == NULL) {
        return 0;
    }

    fclose(file);
    return 1;
}

int parse_record_line(const char *line, WarehouseRecord *record) {
    int unit_raw;
    int read_count;

    if (line == NULL || record == NULL) {
        return 0;
    }

    read_count = sscanf(line,
                        " %49[^;];%49[^;];%49[^;];%49[^;];%d;%d;%d;%lf;%lf;%lf;%d",
                        record->owner_name,
                        record->owner_surname,
                        record->product_name,
                        record->manufacturer,
                        &record->contract_date.day,
                        &record->contract_date.month,
                        &record->contract_date.year,
                        &record->wholesale_price,
                        &record->unit_price,
                        &record->quantity,
                        &unit_raw);

    if (read_count != 11 || !is_valid_unit(unit_raw)) {
        return 0;
    }

    record->unit = (UnitType)unit_raw;
    return validate_record_fields(record);
}

int format_record_line(const WarehouseRecord *record, char *buffer, size_t buffer_size) {
    int written;

    if (record == NULL || buffer == NULL || buffer_size == 0) {
        return 0;
    }
    if (!validate_record_fields(record)) {
        return 0;
    }

    written = snprintf(buffer,
                       buffer_size,
                       "%s;%s;%s;%s;%d;%d;%d;%.2f;%.2f;%.2f;%d\n",
                       record->owner_name,
                       record->owner_surname,
                       record->product_name,
                       record->manufacturer,
                       record->contract_date.day,
                       record->contract_date.month,
                       record->contract_date.year,
                       record->wholesale_price,
                       record->unit_price,
                       record->quantity,
                       (int)record->unit);

    if (written < 0 || (size_t)written >= buffer_size) {
        return 0;
    }
    return 1;
}

int write_record_to_file(const char *path, const WarehouseRecord *record, int append) {
    FILE *file;
    char line[LINE_BUFFER_SIZE];

    if (path == NULL || record == NULL) {
        return 0;
    }
    if (!format_record_line(record, line, sizeof(line))) {
        return 0;
    }

    file = fopen(path, append ? "a" : "w");
    if (file == NULL) {
        return 0;
    }

    if (fputs(line, file) == EOF) {
        fclose(file);
        return 0;
    }

    fclose(file);
    return 1;
}

int load_records_from_file(const char *path, WarehouseRecord **records, size_t *count) {
    FILE *file;
    char line[LINE_BUFFER_SIZE];
    WarehouseRecord item;
    WarehouseRecord *result;
    size_t used = 0;
    size_t capacity = 8;

    if (path == NULL || records == NULL || count == NULL) {
        return 0;
    }

    file = fopen(path, "r");
    if (file == NULL) {
        return 0;
    }

    result = (WarehouseRecord *)calloc(capacity, sizeof(WarehouseRecord));
    if (result == NULL) {
        fclose(file);
        return 0;
    }

    // Read each line, keep only valid semicolon-separated records.
    while (fgets(line, sizeof(line), file) != NULL) {
        if (line[0] == '\n' || line[0] == '\r' || line[0] == '\0') {
            continue;
        }
        if (!parse_record_line(line, &item)) {
            continue;
        }

        // Grow dynamic array when current capacity is exhausted.
        if (used == capacity) {
            WarehouseRecord *resized;
            capacity *= 2;
            resized = (WarehouseRecord *)realloc(result, capacity * sizeof(WarehouseRecord));
            if (resized == NULL) {
                free(result);
                fclose(file);
                return 0;
            }
            result = resized;
        }
        result[used++] = item;
    }

    fclose(file);
    *records = result;
    *count = used;
    return 1;
}

int save_records_to_file(const char *path, const WarehouseRecord *records, const int *priorities, size_t count, int append) {
    FILE *file;
    size_t i;
    char line[LINE_BUFFER_SIZE];

    if (path == NULL || (records == NULL && count > 0)) {
        return 0;
    }

    file = fopen(path, append ? "a" : "w");
    if (file == NULL) {
        return 0;
    }

    for (i = 0; i < count; i++) {
        if (!format_record_line(&records[i], line, sizeof(line))) {
            fclose(file);
            return 0;
        }
        if (fputs(line, file) == EOF) {
            fclose(file);
            return 0;
        }

        if (priorities != NULL) {
            fprintf(file, "#priority=%d\n", priorities[i]);
        }
    }

    fclose(file);
    return 1;
}

int copy_file_to_beginning(const char *source_path, const char *target_path) {
    FILE *source_file;
    FILE *target_file;
    FILE *temp_file;
    int ch;

    if (source_path == NULL || target_path == NULL) {
        return 0;
    }

    source_file = fopen(source_path, "r");
    target_file = fopen(target_path, "r");
    temp_file = fopen(TMP_FILE_NAME, "w");

    if (source_file == NULL || target_file == NULL || temp_file == NULL) {
        if (source_file != NULL) fclose(source_file);
        if (target_file != NULL) fclose(target_file);
        if (temp_file != NULL) fclose(temp_file);
        return 0;
    }

    // First copy source (output), then old target (experiment).
    while ((ch = fgetc(source_file)) != EOF) {
        fputc(ch, temp_file);
    }
    if (fseek(target_file, 0, SEEK_SET) != 0) {
        fclose(source_file);
        fclose(target_file);
        fclose(temp_file);
        return 0;
    }
    while ((ch = fgetc(target_file)) != EOF) {
        fputc(ch, temp_file);
    }

    fclose(source_file);
    fclose(target_file);
    fclose(temp_file);

    if (remove(target_path) != 0) {
        return 0;
    }
    if (rename(TMP_FILE_NAME, target_path) != 0) {
        return 0;
    }
    return 1;
}

int reopen_file_read(const char *path, FILE **file_out) {
    if (path == NULL || file_out == NULL) {
        return 0;
    }

    *file_out = fopen(path, "r");
    return *file_out != NULL;
}

int delete_file_with_confirmation(const char *path, int confirmed) {
    if (path == NULL || !confirmed) {
        return 0;
    }

    return remove(path) == 0;
}
