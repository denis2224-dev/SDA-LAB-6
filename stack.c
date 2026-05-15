#include "stack.h"

#include <ctype.h>
#include <stdlib.h>

// Case-insensitive strict string equality used by search filters.
static int text_equals_ignore_case(const char *left, const char *right) {
    unsigned char lc;
    unsigned char rc;

    if (left == NULL || right == NULL) {
        return 0;
    }

    while (*left != '\0' && *right != '\0') {
        lc = (unsigned char)*left;
        rc = (unsigned char)*right;
        if (tolower(lc) != tolower(rc)) {
            return 0;
        }
        left++;
        right++;
    }
    return *left == '\0' && *right == '\0';
}

static StackNode *stack_node_create(const WarehouseRecord *record) {
    StackNode *node;

    if (record == NULL) {
        return NULL;
    }

    node = (StackNode *)calloc(1, sizeof(StackNode));
    if (node == NULL) {
        return NULL;
    }

    node->data = *record;
    node->priority = warehouse_auto_priority(record);
    node->next = NULL;
    return node;
}

void stack_init(Stack *stack) {
    if (stack == NULL) {
        return;
    }

    stack->top = NULL;
    stack->size = 0;
}

void stack_clear(Stack *stack) {
    StackNode *current;

    if (stack == NULL) {
        return;
    }

    current = stack->top;
    while (current != NULL) {
        StackNode *next = current->next;
        free(current);
        current = next;
    }

    stack->top = NULL;
    stack->size = 0;
}

int stack_is_empty(const Stack *stack) {
    return stack == NULL || stack->size == 0;
}

size_t stack_size(const Stack *stack) {
    if (stack == NULL) {
        return 0;
    }
    return stack->size;
}

int stack_push(Stack *stack, const WarehouseRecord *record) {
    StackNode *node;

    if (stack == NULL || record == NULL) {
        return 0;
    }

    node = stack_node_create(record);
    if (node == NULL) {
        return 0;
    }

    node->next = stack->top;
    stack->top = node;
    stack->size++;
    return 1;
}

int stack_pop(Stack *stack, WarehouseRecord *record_out, int *priority_out) {
    StackNode *node;

    if (stack == NULL || stack->top == NULL) {
        return 0;
    }

    node = stack->top;
    stack->top = node->next;
    stack->size--;

    if (record_out != NULL) {
        *record_out = node->data;
    }
    if (priority_out != NULL) {
        *priority_out = node->priority;
    }

    free(node);
    return 1;
}

int stack_peek(const Stack *stack, WarehouseRecord *record_out, int *priority_out) {
    if (stack == NULL || stack->top == NULL) {
        return 0;
    }

    if (record_out != NULL) {
        *record_out = stack->top->data;
    }
    if (priority_out != NULL) {
        *priority_out = stack->top->priority;
    }
    return 1;
}

void stack_display(const Stack *stack, FILE *stream) {
    StackNode *current;
    int index = 1;

    if (stream == NULL) {
        return;
    }
    if (stack == NULL || stack->top == NULL) {
        fprintf(stream, "Stack is empty.\n");
        return;
    }

    current = stack->top;
    while (current != NULL) {
        print_warehouse_record(stream, &current->data, index, current->priority);
        current = current->next;
        index++;
    }
}

StackNode *stack_node_at(const Stack *stack, size_t position) {
    StackNode *current;
    size_t index = 1;

    if (stack == NULL || position == 0 || position > stack->size) {
        return NULL;
    }

    current = stack->top;
    while (current != NULL) {
        if (index == position) {
            return current;
        }
        current = current->next;
        index++;
    }
    return NULL;
}

static size_t stack_search_text_field(const Stack *stack,
                                      const char *needle,
                                      const char *(*extractor)(const WarehouseRecord *),
                                      size_t *positions,
                                      size_t max_positions) {
    StackNode *current;
    size_t index = 1;
    size_t matched = 0;

    if (stack == NULL || needle == NULL || extractor == NULL || needle[0] == '\0') {
        return 0;
    }

    current = stack->top;
    while (current != NULL) {
        if (text_equals_ignore_case(extractor(&current->data), needle)) {
            if (positions != NULL && matched < max_positions) {
                positions[matched] = index;
            }
            matched++;
        }

        current = current->next;
        index++;
    }

    return matched;
}

static const char *extract_product_name(const WarehouseRecord *record) {
    return record->product_name;
}

static const char *extract_owner_surname(const WarehouseRecord *record) {
    return record->owner_surname;
}

static const char *extract_manufacturer(const WarehouseRecord *record) {
    return record->manufacturer;
}

size_t stack_search_product_name(const Stack *stack, const char *product_name, size_t *positions, size_t max_positions) {
    return stack_search_text_field(stack, product_name, extract_product_name, positions, max_positions);
}

size_t stack_search_owner_surname(const Stack *stack, const char *owner_surname, size_t *positions, size_t max_positions) {
    return stack_search_text_field(stack, owner_surname, extract_owner_surname, positions, max_positions);
}

size_t stack_search_manufacturer(const Stack *stack, const char *manufacturer, size_t *positions, size_t max_positions) {
    return stack_search_text_field(stack, manufacturer, extract_manufacturer, positions, max_positions);
}

size_t stack_search_contract_year(const Stack *stack, int year, size_t *positions, size_t max_positions) {
    StackNode *current;
    size_t index = 1;
    size_t matched = 0;

    if (stack == NULL) {
        return 0;
    }

    current = stack->top;
    while (current != NULL) {
        if (current->data.contract_date.year == year) {
            if (positions != NULL && matched < max_positions) {
                positions[matched] = index;
            }
            matched++;
        }
        current = current->next;
        index++;
    }

    return matched;
}

size_t stack_search_unit_price_interval(const Stack *stack,
                                        double min_price,
                                        double max_price,
                                        size_t *positions,
                                        size_t max_positions) {
    StackNode *current;
    size_t index = 1;
    size_t matched = 0;

    if (stack == NULL || min_price > max_price) {
        return 0;
    }

    current = stack->top;
    while (current != NULL) {
        if (current->data.unit_price >= min_price && current->data.unit_price <= max_price) {
            if (positions != NULL && matched < max_positions) {
                positions[matched] = index;
            }
            matched++;
        }
        current = current->next;
        index++;
    }

    return matched;
}

int stack_delete_by_position(Stack *stack, size_t position, WarehouseRecord *record_out, int *priority_out) {
    StackNode *current;
    StackNode *previous = NULL;
    size_t index = 1;

    if (stack == NULL || position == 0 || position > stack->size) {
        return 0;
    }
    if (position == 1) {
        return stack_pop(stack, record_out, priority_out);
    }

    current = stack->top;
    while (current != NULL && index < position) {
        previous = current;
        current = current->next;
        index++;
    }

    if (previous == NULL || current == NULL) {
        return 0;
    }

    previous->next = current->next;
    stack->size--;
    if (record_out != NULL) {
        *record_out = current->data;
    }
    if (priority_out != NULL) {
        *priority_out = current->priority;
    }

    free(current);
    return 1;
}

size_t stack_delete_by_product_name(Stack *stack, const char *product_name, int delete_all) {
    size_t removed = 0;

    if (stack == NULL || product_name == NULL || product_name[0] == '\0') {
        return 0;
    }

    while (1) {
        size_t first_match_position = 0;
        size_t matches = stack_search_product_name(stack, product_name, &first_match_position, 1);
        if (matches == 0) {
            break;
        }
        if (!stack_delete_by_position(stack, first_match_position, NULL, NULL)) {
            break;
        }
        removed++;
        if (!delete_all) {
            break;
        }
    }

    return removed;
}

size_t stack_delete_by_owner_surname(Stack *stack, const char *owner_surname, int delete_all) {
    size_t removed = 0;

    if (stack == NULL || owner_surname == NULL || owner_surname[0] == '\0') {
        return 0;
    }

    while (1) {
        size_t first_match_position = 0;
        size_t matches = stack_search_owner_surname(stack, owner_surname, &first_match_position, 1);
        if (matches == 0) {
            break;
        }
        if (!stack_delete_by_position(stack, first_match_position, NULL, NULL)) {
            break;
        }
        removed++;
        if (!delete_all) {
            break;
        }
    }

    return removed;
}

int stack_export_arrays(const Stack *stack, WarehouseRecord **records_out, int **priorities_out, size_t *count_out) {
    WarehouseRecord *records;
    int *priorities;
    StackNode *current;
    size_t index = 0;

    if (stack == NULL || records_out == NULL || priorities_out == NULL || count_out == NULL) {
        return 0;
    }

    *records_out = NULL;
    *priorities_out = NULL;
    *count_out = 0;

    if (stack->size == 0) {
        return 1;
    }

    records = (WarehouseRecord *)calloc(stack->size, sizeof(WarehouseRecord));
    priorities = (int *)calloc(stack->size, sizeof(int));
    if (records == NULL || priorities == NULL) {
        free(records);
        free(priorities);
        return 0;
    }

    current = stack->top;
    while (current != NULL && index < stack->size) {
        records[index] = current->data;
        priorities[index] = current->priority;
        index++;
        current = current->next;
    }

    *records_out = records;
    *priorities_out = priorities;
    *count_out = index;
    return 1;
}
