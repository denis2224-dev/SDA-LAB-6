#ifndef QUEUE_H
#define QUEUE_H

#include <stddef.h>
#include <stdio.h>

#include "warehouse.h"

typedef struct Node {
    WarehouseRecord data;
    int priority;
    struct Node *next;
    struct Node *prev;
} Node;

typedef enum {
    QUEUE_SIMPLE = 1,
    QUEUE_DEQUE = 2,
    QUEUE_CIRCULAR = 3,
    QUEUE_PRIORITY = 4
} QueueType;

typedef struct {
    Node *front;
    Node *rear;
    size_t size;
    QueueType type;
} Queue;

void queue_init(Queue *queue, QueueType type);
void queue_clear(Queue *queue);
int queue_is_empty(const Queue *queue);
size_t queue_size(const Queue *queue);

Node *node_create(const WarehouseRecord *record, int priority);
void node_destroy(Node *node);

int queue_attach_back(Queue *queue, Node *node);
int queue_attach_front(Queue *queue, Node *node);
Node *queue_detach_front(Queue *queue);
Node *queue_detach_rear(Queue *queue);

int simple_enqueue(Queue *queue, const WarehouseRecord *record);
int simple_dequeue(Queue *queue, WarehouseRecord *record_out, int *priority_out);
int simple_peek_front(const Queue *queue, WarehouseRecord *record_out, int *priority_out);
void queue_display_forward(const Queue *queue, FILE *stream);
void queue_display_backward(const Queue *queue, FILE *stream);

int deque_insert_front(Queue *queue, const WarehouseRecord *record);
int deque_insert_rear(Queue *queue, const WarehouseRecord *record);
int deque_delete_front(Queue *queue, WarehouseRecord *record_out, int *priority_out);
int deque_delete_rear(Queue *queue, WarehouseRecord *record_out, int *priority_out);

int circular_enqueue(Queue *queue, const WarehouseRecord *record);
int circular_dequeue(Queue *queue, WarehouseRecord *record_out, int *priority_out);

int priority_enqueue(Queue *queue, const WarehouseRecord *record, int manual_priority, int use_manual_priority);
int priority_dequeue(Queue *queue, WarehouseRecord *record_out, int *priority_out);

Node *queue_node_at(const Queue *queue, size_t position);
size_t queue_search_product_name(const Queue *queue, const char *product_name, size_t *positions, size_t max_positions);
size_t queue_search_owner_surname(const Queue *queue, const char *owner_surname, size_t *positions, size_t max_positions);
size_t queue_search_manufacturer(const Queue *queue, const char *manufacturer, size_t *positions, size_t max_positions);
size_t queue_search_contract_year(const Queue *queue, int year, size_t *positions, size_t max_positions);
size_t queue_search_unit_price_interval(const Queue *queue,
                                        double min_price,
                                        double max_price,
                                        size_t *positions,
                                        size_t max_positions);

#endif
