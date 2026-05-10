#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "file_ops.h"
#include "queue.h"
#include "warehouse.h"

#define INPUT_BUFFER_SIZE 256
#define EXPERIMENT_PATH "data/experiment.txt"
#define OUTPUT_PATH "data/output.txt"

typedef struct {
    Queue simple_queue;
    Queue deque_queue;
    Queue circular_queue;
    Queue priority_queue;
    QueueType active_type;
} AppState;

static void trim_newline(char *text) {
    size_t len;

    if (text == NULL) {
        return;
    }

    len = strlen(text);
    while (len > 0 && (text[len - 1] == '\n' || text[len - 1] == '\r')) {
        text[len - 1] = '\0';
        len--;
    }
}

static int read_line_prompt(const char *prompt, char *buffer, size_t buffer_size) {
    if (prompt != NULL) {
        printf("%s", prompt);
    }
    if (fgets(buffer, (int)buffer_size, stdin) == NULL) {
        return 0;
    }
    trim_newline(buffer);
    return 1;
}

static int parse_int_strict(const char *text, int *value_out) {
    long value;
    char *end_ptr;

    if (text == NULL || value_out == NULL || text[0] == '\0') {
        return 0;
    }

    errno = 0;
    value = strtol(text, &end_ptr, 10);
    if (errno != 0 || *end_ptr != '\0') {
        return 0;
    }
    if (value < -2147483647L - 1L || value > 2147483647L) {
        return 0;
    }

    *value_out = (int)value;
    return 1;
}

static int parse_double_strict(const char *text, double *value_out) {
    double value;
    char *end_ptr;

    if (text == NULL || value_out == NULL || text[0] == '\0') {
        return 0;
    }

    errno = 0;
    value = strtod(text, &end_ptr);
    if (errno != 0 || *end_ptr != '\0') {
        return 0;
    }

    *value_out = value;
    return 1;
}

static int prompt_int_in_range(const char *prompt, int min_value, int max_value, int *value_out) {
    char buffer[INPUT_BUFFER_SIZE];
    int value;

    while (1) {
        if (!read_line_prompt(prompt, buffer, sizeof(buffer))) {
            return 0;
        }
        if (!parse_int_strict(buffer, &value)) {
            printf("Invalid integer value.\n");
            continue;
        }
        if (value < min_value || value > max_value) {
            printf("Value must be in range [%d..%d].\n", min_value, max_value);
            continue;
        }
        *value_out = value;
        return 1;
    }
}

static int prompt_double_positive(const char *prompt, double *value_out) {
    char buffer[INPUT_BUFFER_SIZE];
    double value;

    while (1) {
        if (!read_line_prompt(prompt, buffer, sizeof(buffer))) {
            return 0;
        }
        if (!parse_double_strict(buffer, &value)) {
            printf("Invalid numeric value.\n");
            continue;
        }
        if (value <= 0.0) {
            printf("Value must be positive.\n");
            continue;
        }
        *value_out = value;
        return 1;
    }
}

static int prompt_non_empty_text(const char *prompt, char *out_text, size_t out_size) {
    char buffer[INPUT_BUFFER_SIZE];

    while (1) {
        if (!read_line_prompt(prompt, buffer, sizeof(buffer))) {
            return 0;
        }
        if (buffer[0] == '\0') {
            printf("Empty input is not allowed.\n");
            continue;
        }

        snprintf(out_text, out_size, "%s", buffer);
        return 1;
    }
}

static int prompt_yes_no(const char *prompt) {
    char buffer[INPUT_BUFFER_SIZE];

    while (1) {
        if (!read_line_prompt(prompt, buffer, sizeof(buffer))) {
            return 0;
        }
        if (buffer[0] == '\0') {
            continue;
        }
        if (tolower((unsigned char)buffer[0]) == 'y') {
            return 1;
        }
        if (tolower((unsigned char)buffer[0]) == 'n') {
            return 0;
        }
        printf("Please answer with y/n.\n");
    }
}

static int ensure_data_directory(void) {
    struct stat st;

    if (stat("data", &st) == 0) {
        return S_ISDIR(st.st_mode);
    }
    if (mkdir("data", 0777) == 0) {
        return 1;
    }
    return 0;
}

static Queue *get_queue_by_type(AppState *state, QueueType type) {
    if (type == QUEUE_SIMPLE) {
        return &state->simple_queue;
    }
    if (type == QUEUE_DEQUE) {
        return &state->deque_queue;
    }
    if (type == QUEUE_CIRCULAR) {
        return &state->circular_queue;
    }
    return &state->priority_queue;
}

static const char *queue_type_name(QueueType type) {
    if (type == QUEUE_SIMPLE) {
        return "Simple Queue";
    }
    if (type == QUEUE_DEQUE) {
        return "Double Ended Queue";
    }
    if (type == QUEUE_CIRCULAR) {
        return "Circular Queue";
    }
    return "Priority Queue";
}

static int select_queue_type(QueueType *type_out) {
    int option;

    printf("\nSelect queue type:\n");
    printf("1. Simple Queue\n");
    printf("2. Double Ended Queue\n");
    printf("3. Circular Queue\n");
    printf("4. Priority Queue\n");
    if (!prompt_int_in_range("Choice: ", 1, 4, &option)) {
        return 0;
    }

    *type_out = (QueueType)option;
    return 1;
}

static int input_record_from_user(WarehouseRecord *record) {
    int day;
    int month;
    int year;
    int unit;
    Date date;

    if (record == NULL) {
        return 0;
    }

    if (!prompt_non_empty_text("Owner name: ", record->owner_name, sizeof(record->owner_name))) {
        return 0;
    }
    if (!prompt_non_empty_text("Owner surname: ", record->owner_surname, sizeof(record->owner_surname))) {
        return 0;
    }
    if (!prompt_non_empty_text("Product name: ", record->product_name, sizeof(record->product_name))) {
        return 0;
    }
    if (!prompt_non_empty_text("Manufacturer: ", record->manufacturer, sizeof(record->manufacturer))) {
        return 0;
    }

    while (1) {
        if (!prompt_int_in_range("Contract day: ", 1, 31, &day)) {
            return 0;
        }
        if (!prompt_int_in_range("Contract month: ", 1, 12, &month)) {
            return 0;
        }
        if (!prompt_int_in_range("Contract year: ", 1900, 2100, &year)) {
            return 0;
        }
        date.day = day;
        date.month = month;
        date.year = year;
        if (validate_date(&date)) {
            break;
        }
        printf("Invalid date. Try again.\n");
    }

    record->contract_date = date;
    if (!prompt_double_positive("Wholesale price: ", &record->wholesale_price)) {
        return 0;
    }
    if (!prompt_double_positive("Unit price: ", &record->unit_price)) {
        return 0;
    }
    if (!prompt_double_positive("Quantity: ", &record->quantity)) {
        return 0;
    }

    printf("Unit type:\n");
    printf("1. tons\n");
    printf("2. kg\n");
    printf("3. grams\n");
    if (!prompt_int_in_range("Unit choice: ", 1, 3, &unit)) {
        return 0;
    }
    record->unit = (UnitType)unit;
    return 1;
}

static int enqueue_for_selected_type(Queue *queue, const WarehouseRecord *record) {
    int manual_priority;
    int use_manual_priority;

    if (queue == NULL || record == NULL) {
        return 0;
    }

    if (queue->type == QUEUE_SIMPLE) {
        return simple_enqueue(queue, record);
    }
    if (queue->type == QUEUE_DEQUE) {
        return deque_insert_rear(queue, record);
    }
    if (queue->type == QUEUE_CIRCULAR) {
        return circular_enqueue(queue, record);
    }

    use_manual_priority = prompt_yes_no("Manual priority override? (y/n): ");
    manual_priority = warehouse_auto_priority(record);
    if (use_manual_priority) {
        if (!prompt_int_in_range("Enter priority (1..1000000000): ", 1, 1000000000, &manual_priority)) {
            return 0;
        }
    }
    return priority_enqueue(queue, record, manual_priority, use_manual_priority);
}

static void print_deleted_record(const WarehouseRecord *record, int priority) {
    if (record == NULL) {
        return;
    }
    printf("Removed record:\n");
    print_warehouse_record(stdout, record, 1, priority);
}

static int load_records_into_queue(Queue *queue, const char *file_path, int clear_before_load) {
    WarehouseRecord *records = NULL;
    size_t count = 0;
    size_t i;
    int loaded_count = 0;

    if (queue == NULL || file_path == NULL) {
        return 0;
    }

    if (clear_before_load) {
        queue_clear(queue);
    }

    if (!load_records_from_file(file_path, &records, &count)) {
        return 0;
    }

    for (i = 0; i < count; i++) {
        if (queue->type == QUEUE_SIMPLE) {
            loaded_count += simple_enqueue(queue, &records[i]);
        } else if (queue->type == QUEUE_DEQUE) {
            loaded_count += deque_insert_rear(queue, &records[i]);
        } else if (queue->type == QUEUE_CIRCULAR) {
            loaded_count += circular_enqueue(queue, &records[i]);
        } else {
            loaded_count += priority_enqueue(queue, &records[i], 0, 0);
        }
    }

    free(records);
    return loaded_count;
}

static void display_queue_records(const Queue *queue) {
    if (queue == NULL) {
        return;
    }
    if (queue->type == QUEUE_DEQUE) {
        printf("\nForward view:\n");
        queue_display_forward(queue, stdout);
        printf("Backward view:\n");
        queue_display_backward(queue, stdout);
        return;
    }

    queue_display_forward(queue, stdout);
}

static void run_simple_queue_menu(Queue *queue) {
    int option;
    WarehouseRecord record;
    int priority;

    while (1) {
        printf("\nSimple Queue menu:\n");
        printf("1. Enqueue rear\n");
        printf("2. Dequeue front\n");
        printf("3. Peek front\n");
        printf("4. Display\n");
        printf("0. Back\n");

        if (!prompt_int_in_range("Choice: ", 0, 4, &option)) {
            return;
        }

        if (option == 0) {
            return;
        }
        if (option == 1) {
            if (!input_record_from_user(&record)) {
                continue;
            }
            if (simple_enqueue(queue, &record)) {
                printf("Record enqueued.\n");
            } else {
                printf("Failed to enqueue record.\n");
            }
        } else if (option == 2) {
            if (simple_dequeue(queue, &record, &priority)) {
                print_deleted_record(&record, priority);
            } else {
                printf("Queue is empty.\n");
            }
        } else if (option == 3) {
            if (simple_peek_front(queue, &record, &priority)) {
                print_warehouse_record(stdout, &record, 1, priority);
            } else {
                printf("Queue is empty.\n");
            }
        } else if (option == 4) {
            queue_display_forward(queue, stdout);
        }
    }
}

static void run_deque_menu(Queue *queue) {
    int option;
    WarehouseRecord record;
    int priority;

    while (1) {
        printf("\nDouble Ended Queue menu:\n");
        printf("1. Insert front\n");
        printf("2. Insert rear\n");
        printf("3. Delete front\n");
        printf("4. Delete rear\n");
        printf("5. Display forward\n");
        printf("6. Display backward\n");
        printf("0. Back\n");

        if (!prompt_int_in_range("Choice: ", 0, 6, &option)) {
            return;
        }

        if (option == 0) {
            return;
        }
        if (option == 1 || option == 2) {
            if (!input_record_from_user(&record)) {
                continue;
            }
            if ((option == 1 && deque_insert_front(queue, &record)) ||
                (option == 2 && deque_insert_rear(queue, &record))) {
                printf("Record inserted.\n");
            } else {
                printf("Insert failed.\n");
            }
        } else if (option == 3) {
            if (deque_delete_front(queue, &record, &priority)) {
                print_deleted_record(&record, priority);
            } else {
                printf("Queue is empty.\n");
            }
        } else if (option == 4) {
            if (deque_delete_rear(queue, &record, &priority)) {
                print_deleted_record(&record, priority);
            } else {
                printf("Queue is empty.\n");
            }
        } else if (option == 5) {
            queue_display_forward(queue, stdout);
        } else if (option == 6) {
            queue_display_backward(queue, stdout);
        }
    }
}

static void run_circular_menu(Queue *queue) {
    int option;
    WarehouseRecord record;
    int priority;

    while (1) {
        printf("\nCircular Queue menu:\n");
        printf("1. Enqueue\n");
        printf("2. Dequeue\n");
        printf("3. Display\n");
        printf("0. Back\n");

        if (!prompt_int_in_range("Choice: ", 0, 3, &option)) {
            return;
        }

        if (option == 0) {
            return;
        }
        if (option == 1) {
            if (!input_record_from_user(&record)) {
                continue;
            }
            if (circular_enqueue(queue, &record)) {
                printf("Record enqueued.\n");
            } else {
                printf("Enqueue failed.\n");
            }
        } else if (option == 2) {
            if (circular_dequeue(queue, &record, &priority)) {
                print_deleted_record(&record, priority);
            } else {
                printf("Queue is empty.\n");
            }
        } else if (option == 3) {
            queue_display_forward(queue, stdout);
        }
    }
}

static void run_priority_menu(Queue *queue) {
    int option;
    int manual_priority;
    WarehouseRecord record;
    int priority;

    while (1) {
        printf("\nPriority Queue menu:\n");
        printf("1. Enqueue with auto priority\n");
        printf("2. Enqueue with manual priority\n");
        printf("3. Dequeue highest priority\n");
        printf("4. Display\n");
        printf("0. Back\n");

        if (!prompt_int_in_range("Choice: ", 0, 4, &option)) {
            return;
        }

        if (option == 0) {
            return;
        }
        if (option == 1 || option == 2) {
            if (!input_record_from_user(&record)) {
                continue;
            }
            manual_priority = warehouse_auto_priority(&record);
            if (option == 2 &&
                !prompt_int_in_range("Manual priority (1..1000000000): ", 1, 1000000000, &manual_priority)) {
                continue;
            }

            if (priority_enqueue(queue, &record, manual_priority, option == 2)) {
                printf("Record enqueued.\n");
            } else {
                printf("Enqueue failed.\n");
            }
        } else if (option == 3) {
            if (priority_dequeue(queue, &record, &priority)) {
                print_deleted_record(&record, priority);
            } else {
                printf("Queue is empty.\n");
            }
        } else if (option == 4) {
            queue_display_forward(queue, stdout);
        }
    }
}

static void print_search_matches(const Queue *queue, const size_t *positions, size_t count) {
    size_t i;
    Node *node;

    if (count == 0) {
        printf("No matching records.\n");
        return;
    }

    for (i = 0; i < count; i++) {
        node = queue_node_at(queue, positions[i]);
        if (node != NULL) {
            print_warehouse_record(stdout, &node->data, (int)positions[i], node->priority);
        }
    }
}

static void run_search_menu(const Queue *queue) {
    int option;
    int year;
    double min_price;
    double max_price;
    char text[INPUT_BUFFER_SIZE];
    size_t *positions = NULL;
    size_t count = 0;
    size_t max_positions;

    if (queue == NULL || queue->size == 0) {
        printf("Active queue is empty.\n");
        return;
    }

    max_positions = queue->size;
    positions = (size_t *)calloc(max_positions, sizeof(size_t));
    if (positions == NULL) {
        printf("Memory allocation failed.\n");
        return;
    }

    while (1) {
        printf("\nSearch menu:\n");
        printf("1. Search by position\n");
        printf("2. Search by product name\n");
        printf("3. Search by owner surname\n");
        printf("4. Search by manufacturer\n");
        printf("5. Search by contract year\n");
        printf("6. Search by unit price interval\n");
        printf("0. Back\n");

        if (!prompt_int_in_range("Choice: ", 0, 6, &option)) {
            break;
        }
        if (option == 0) {
            break;
        }

        if (option == 1) {
            int position;
            Node *node;
            if (!prompt_int_in_range("Position: ", 1, (int)queue->size, &position)) {
                continue;
            }
            node = queue_node_at(queue, (size_t)position);
            if (node != NULL) {
                print_warehouse_record(stdout, &node->data, position, node->priority);
            } else {
                printf("Position not found.\n");
            }
            continue;
        }

        if (option == 2) {
            if (!prompt_non_empty_text("Product name: ", text, sizeof(text))) {
                continue;
            }
            count = queue_search_product_name(queue, text, positions, max_positions);
        } else if (option == 3) {
            if (!prompt_non_empty_text("Owner surname: ", text, sizeof(text))) {
                continue;
            }
            count = queue_search_owner_surname(queue, text, positions, max_positions);
        } else if (option == 4) {
            if (!prompt_non_empty_text("Manufacturer: ", text, sizeof(text))) {
                continue;
            }
            count = queue_search_manufacturer(queue, text, positions, max_positions);
        } else if (option == 5) {
            if (!prompt_int_in_range("Contract year: ", 1900, 2100, &year)) {
                continue;
            }
            count = queue_search_contract_year(queue, year, positions, max_positions);
        } else if (option == 6) {
            if (!prompt_double_positive("Min unit price: ", &min_price)) {
                continue;
            }
            if (!prompt_double_positive("Max unit price: ", &max_price)) {
                continue;
            }
            if (min_price > max_price) {
                double temp = min_price;
                min_price = max_price;
                max_price = temp;
            }
            count = queue_search_unit_price_interval(queue, min_price, max_price, positions, max_positions);
        }

        print_search_matches(queue, positions, count);
    }

    free(positions);
}

static void run_delete_menu(Queue *queue) {
    int option;
    int position;
    char text[INPUT_BUFFER_SIZE];
    size_t deleted_count;
    WarehouseRecord removed_record;
    int priority;

    if (queue == NULL || queue->size == 0) {
        printf("Active queue is empty.\n");
        return;
    }

    while (1) {
        printf("\nDelete menu:\n");
        printf("1. Delete by position\n");
        printf("2. Delete by product name (first match)\n");
        printf("3. Delete by owner surname (first match)\n");
        printf("4. Delete all by product name (with confirmation)\n");
        printf("5. Delete all by owner surname (with confirmation)\n");
        printf("0. Back\n");

        if (!prompt_int_in_range("Choice: ", 0, 5, &option)) {
            return;
        }
        if (option == 0) {
            return;
        }

        if (option == 1) {
            if (!prompt_int_in_range("Position: ", 1, (int)queue->size, &position)) {
                continue;
            }
            if (queue_delete_by_position(queue, (size_t)position, &removed_record, &priority)) {
                print_deleted_record(&removed_record, priority);
            } else {
                printf("Delete failed.\n");
            }
        } else if (option == 2) {
            if (!prompt_non_empty_text("Product name: ", text, sizeof(text))) {
                continue;
            }
            deleted_count = queue_delete_by_product_name(queue, text, 0);
            printf("Deleted: %zu record(s)\n", deleted_count);
        } else if (option == 3) {
            if (!prompt_non_empty_text("Owner surname: ", text, sizeof(text))) {
                continue;
            }
            deleted_count = queue_delete_by_owner_surname(queue, text, 0);
            printf("Deleted: %zu record(s)\n", deleted_count);
        } else if (option == 4) {
            if (!prompt_non_empty_text("Product name: ", text, sizeof(text))) {
                continue;
            }
            if (!prompt_yes_no("Confirm delete all matching product records? (y/n): ")) {
                printf("Deletion canceled.\n");
                continue;
            }
            deleted_count = queue_delete_by_product_name(queue, text, 1);
            printf("Deleted: %zu record(s)\n", deleted_count);
        } else if (option == 5) {
            if (!prompt_non_empty_text("Owner surname: ", text, sizeof(text))) {
                continue;
            }
            if (!prompt_yes_no("Confirm delete all matching owner surname records? (y/n): ")) {
                printf("Deletion canceled.\n");
                continue;
            }
            deleted_count = queue_delete_by_owner_surname(queue, text, 1);
            printf("Deleted: %zu record(s)\n", deleted_count);
        }

        if (queue->size == 0) {
            printf("Queue became empty.\n");
            return;
        }
    }
}

static void save_active_queue_to_output(const Queue *queue) {
    WarehouseRecord *records = NULL;
    int *priorities = NULL;
    size_t count = 0;
    int mode;

    if (queue == NULL) {
        return;
    }
    if (!queue_export_arrays(queue, &records, &priorities, &count)) {
        printf("Failed to extract queue data.\n");
        return;
    }

    printf("Save mode:\n");
    printf("1. Append to output.txt\n");
    printf("2. Rewrite output.txt\n");
    if (!prompt_int_in_range("Choice: ", 1, 2, &mode)) {
        free(records);
        free(priorities);
        return;
    }

    if (save_records_to_file(OUTPUT_PATH, records, NULL, count, mode == 1)) {
        printf("Queue saved to %s\n", OUTPUT_PATH);
    } else {
        printf("Failed to save queue.\n");
    }

    free(records);
    free(priorities);
}

static void delete_file_from_menu(void) {
    int option;
    int confirm;
    char path[INPUT_BUFFER_SIZE];
    const char *selected_path = NULL;

    printf("\nDelete file menu:\n");
    printf("1. Delete data/experiment.txt\n");
    printf("2. Delete data/output.txt\n");
    printf("3. Delete custom file\n");
    if (!prompt_int_in_range("Choice: ", 1, 3, &option)) {
        return;
    }

    if (option == 1) {
        selected_path = EXPERIMENT_PATH;
    } else if (option == 2) {
        selected_path = OUTPUT_PATH;
    } else {
        if (!prompt_non_empty_text("Custom file path: ", path, sizeof(path))) {
            return;
        }
        selected_path = path;
    }

    confirm = prompt_yes_no("Confirm deletion? (y/n): ");
    if (!confirm) {
        printf("Deletion canceled.\n");
        return;
    }

    if (delete_file_with_confirmation(selected_path, 1)) {
        printf("Deleted: %s\n", selected_path);
    } else {
        printf("Failed to delete: %s\n", selected_path);
    }
}

int main(void) {
    AppState state;
    int option;

    if (!ensure_data_directory()) {
        printf("Failed to ensure data directory.\n");
        return 1;
    }
    ensure_file_exists(EXPERIMENT_PATH);
    ensure_file_exists(OUTPUT_PATH);

    queue_init(&state.simple_queue, QUEUE_SIMPLE);
    queue_init(&state.deque_queue, QUEUE_DEQUE);
    queue_init(&state.circular_queue, QUEUE_CIRCULAR);
    queue_init(&state.priority_queue, QUEUE_PRIORITY);
    state.active_type = QUEUE_SIMPLE;

    while (1) {
        Queue *active_queue = get_queue_by_type(&state, state.active_type);
        printf("\nMain menu (active: %s):\n", queue_type_name(state.active_type));
        printf("1. Load records from experiment.txt\n");
        printf("2. Add record manually\n");
        printf("3. Display records\n");
        printf("4. Simple Queue operations\n");
        printf("5. Double Ended Queue operations\n");
        printf("6. Circular Queue operations\n");
        printf("7. Priority Queue operations\n");
        printf("8. Search record\n");
        printf("9. Delete record\n");
        printf("10. Save current queue to output.txt\n");
        printf("11. Copy output.txt to beginning of experiment.txt\n");
        printf("12. Delete file\n");
        printf("0. Exit\n");

        if (!prompt_int_in_range("Choice: ", 0, 12, &option)) {
            break;
        }

        if (option == 0) {
            break;
        }

        if (option == 1) {
            QueueType selected_type;
            int load_mode;
            FILE *file;
            int loaded;

            if (!select_queue_type(&selected_type)) {
                continue;
            }
            state.active_type = selected_type;
            active_queue = get_queue_by_type(&state, state.active_type);

            if (!reopen_file_read(EXPERIMENT_PATH, &file)) {
                printf("Cannot open %s for reading.\n", EXPERIMENT_PATH);
                continue;
            }
            fclose(file);

            printf("Load mode:\n");
            printf("1. Append to existing queue\n");
            printf("2. Replace existing queue\n");
            if (!prompt_int_in_range("Choice: ", 1, 2, &load_mode)) {
                continue;
            }

            loaded = load_records_into_queue(active_queue, EXPERIMENT_PATH, load_mode == 2);
            if (loaded >= 0) {
                printf("Loaded records: %d\n", loaded);
            } else {
                printf("Load failed.\n");
            }
        } else if (option == 2) {
            WarehouseRecord record;
            if (!input_record_from_user(&record)) {
                printf("Record input canceled.\n");
                continue;
            }
            if (!write_record_to_file(EXPERIMENT_PATH, &record, 1)) {
                printf("Warning: failed to append record to %s\n", EXPERIMENT_PATH);
            }
            if (enqueue_for_selected_type(active_queue, &record)) {
                printf("Record added to active queue.\n");
            } else {
                printf("Failed to add record to active queue.\n");
            }
        } else if (option == 3) {
            display_queue_records(active_queue);
        } else if (option == 4) {
            state.active_type = QUEUE_SIMPLE;
            run_simple_queue_menu(&state.simple_queue);
        } else if (option == 5) {
            state.active_type = QUEUE_DEQUE;
            run_deque_menu(&state.deque_queue);
        } else if (option == 6) {
            state.active_type = QUEUE_CIRCULAR;
            run_circular_menu(&state.circular_queue);
        } else if (option == 7) {
            state.active_type = QUEUE_PRIORITY;
            run_priority_menu(&state.priority_queue);
        } else if (option == 8) {
            run_search_menu(active_queue);
        } else if (option == 9) {
            run_delete_menu(active_queue);
        } else if (option == 10) {
            save_active_queue_to_output(active_queue);
        } else if (option == 11) {
            if (copy_file_to_beginning(OUTPUT_PATH, EXPERIMENT_PATH)) {
                printf("Copied %s to beginning of %s\n", OUTPUT_PATH, EXPERIMENT_PATH);
            } else {
                printf("Copy operation failed.\n");
            }
        } else if (option == 12) {
            delete_file_from_menu();
        }
    }

    queue_clear(&state.simple_queue);
    queue_clear(&state.deque_queue);
    queue_clear(&state.circular_queue);
    queue_clear(&state.priority_queue);

    return 0;
}
