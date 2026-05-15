#ifndef STACK_H
#define STACK_H

#include <stddef.h>
#include <stdio.h>

#include "warehouse.h"

// Singly linked list node used by the dynamic stack.
typedef struct StackNode {
    WarehouseRecord data;
    int priority;
    struct StackNode *next;
} StackNode;

// Dynamic stack container; top points to the last pushed element.
typedef struct {
    StackNode *top;
    size_t size;
} Stack;

// Initializes stack metadata and pointer.
void stack_init(Stack *stack);
// Frees all nodes and resets stack to empty.
void stack_clear(Stack *stack);
// Returns 1 if stack is empty.
int stack_is_empty(const Stack *stack);
// Returns number of nodes in stack.
size_t stack_size(const Stack *stack);

// Basic stack API.
int stack_push(Stack *stack, const WarehouseRecord *record);
int stack_pop(Stack *stack, WarehouseRecord *record_out, int *priority_out);
int stack_peek(const Stack *stack, WarehouseRecord *record_out, int *priority_out);
void stack_display(const Stack *stack, FILE *stream);

// Search helpers.
StackNode *stack_node_at(const Stack *stack, size_t position);
size_t stack_search_product_name(const Stack *stack, const char *product_name, size_t *positions, size_t max_positions);
size_t stack_search_owner_surname(const Stack *stack, const char *owner_surname, size_t *positions, size_t max_positions);
size_t stack_search_manufacturer(const Stack *stack, const char *manufacturer, size_t *positions, size_t max_positions);
size_t stack_search_contract_year(const Stack *stack, int year, size_t *positions, size_t max_positions);
size_t stack_search_unit_price_interval(const Stack *stack,
                                        double min_price,
                                        double max_price,
                                        size_t *positions,
                                        size_t max_positions);

// Delete helpers.
int stack_delete_by_position(Stack *stack, size_t position, WarehouseRecord *record_out, int *priority_out);
size_t stack_delete_by_product_name(Stack *stack, const char *product_name, int delete_all);
size_t stack_delete_by_owner_surname(Stack *stack, const char *owner_surname, int delete_all);

// Exports stack content from top to bottom into dynamic arrays.
int stack_export_arrays(const Stack *stack, WarehouseRecord **records_out, int **priorities_out, size_t *count_out);

#endif
