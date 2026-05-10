#include "queue.h"

#include <ctype.h>
#include <stdlib.h>

static void queue_relink_if_circular(Queue *queue) {
    if (queue == NULL || queue->type != QUEUE_CIRCULAR) {
        return;
    }
    if (queue->front == NULL || queue->rear == NULL) {
        return;
    }
    queue->rear->next = queue->front;
    queue->front->prev = queue->rear;
}

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

static int queue_remove_middle_node(Queue *queue, Node *node, WarehouseRecord *record_out, int *priority_out) {
    if (queue == NULL || node == NULL || node->prev == NULL || node->next == NULL) {
        return 0;
    }

    node->prev->next = node->next;
    node->next->prev = node->prev;

    if (record_out != NULL) {
        *record_out = node->data;
    }
    if (priority_out != NULL) {
        *priority_out = node->priority;
    }

    node_destroy(node);
    queue->size--;
    queue_relink_if_circular(queue);
    return 1;
}

void queue_init(Queue *queue, QueueType type) {
    if (queue == NULL) {
        return;
    }
    queue->front = NULL;
    queue->rear = NULL;
    queue->size = 0;
    queue->type = type;
}

void queue_clear(Queue *queue) {
    Node *current;

    if (queue == NULL || queue->front == NULL) {
        return;
    }

    if (queue->type == QUEUE_CIRCULAR && queue->rear != NULL) {
        queue->rear->next = NULL;
        if (queue->front != NULL) {
            queue->front->prev = NULL;
        }
    }

    current = queue->front;
    while (current != NULL) {
        Node *next = current->next;
        free(current);
        current = next;
    }

    queue->front = NULL;
    queue->rear = NULL;
    queue->size = 0;
}

int queue_is_empty(const Queue *queue) {
    return queue == NULL || queue->size == 0;
}

size_t queue_size(const Queue *queue) {
    if (queue == NULL) {
        return 0;
    }
    return queue->size;
}

Node *node_create(const WarehouseRecord *record, int priority) {
    Node *node;

    if (record == NULL) {
        return NULL;
    }

    node = (Node *)calloc(1, sizeof(Node));
    if (node == NULL) {
        return NULL;
    }

    node->data = *record;
    node->priority = priority;
    node->next = NULL;
    node->prev = NULL;
    return node;
}

void node_destroy(Node *node) {
    free(node);
}

int queue_attach_back(Queue *queue, Node *node) {
    if (queue == NULL || node == NULL) {
        return 0;
    }

    if (queue->type == QUEUE_CIRCULAR && queue->rear != NULL) {
        queue->rear->next = NULL;
        queue->front->prev = NULL;
    }

    node->next = NULL;
    node->prev = queue->rear;

    if (queue->rear != NULL) {
        queue->rear->next = node;
    } else {
        queue->front = node;
    }

    queue->rear = node;
    queue->size++;
    queue_relink_if_circular(queue);
    return 1;
}

int queue_attach_front(Queue *queue, Node *node) {
    if (queue == NULL || node == NULL) {
        return 0;
    }

    if (queue->type == QUEUE_CIRCULAR && queue->rear != NULL) {
        queue->rear->next = NULL;
        queue->front->prev = NULL;
    }

    node->prev = NULL;
    node->next = queue->front;

    if (queue->front != NULL) {
        queue->front->prev = node;
    } else {
        queue->rear = node;
    }

    queue->front = node;
    queue->size++;
    queue_relink_if_circular(queue);
    return 1;
}

Node *queue_detach_front(Queue *queue) {
    Node *node;

    if (queue == NULL || queue->front == NULL) {
        return NULL;
    }

    if (queue->type == QUEUE_CIRCULAR && queue->rear != NULL) {
        queue->rear->next = NULL;
        queue->front->prev = NULL;
    }

    node = queue->front;
    queue->front = node->next;
    if (queue->front != NULL) {
        queue->front->prev = NULL;
    } else {
        queue->rear = NULL;
    }

    node->next = NULL;
    node->prev = NULL;
    queue->size--;
    queue_relink_if_circular(queue);
    return node;
}

Node *queue_detach_rear(Queue *queue) {
    Node *node;

    if (queue == NULL || queue->rear == NULL) {
        return NULL;
    }

    if (queue->type == QUEUE_CIRCULAR && queue->rear != NULL) {
        queue->rear->next = NULL;
        queue->front->prev = NULL;
    }

    node = queue->rear;
    queue->rear = node->prev;
    if (queue->rear != NULL) {
        queue->rear->next = NULL;
    } else {
        queue->front = NULL;
    }

    node->next = NULL;
    node->prev = NULL;
    queue->size--;
    queue_relink_if_circular(queue);
    return node;
}

int simple_enqueue(Queue *queue, const WarehouseRecord *record) {
    Node *node;
    int priority;

    if (queue == NULL || record == NULL || queue->type != QUEUE_SIMPLE) {
        return 0;
    }

    priority = warehouse_auto_priority(record);
    node = node_create(record, priority);
    if (node == NULL) {
        return 0;
    }

    if (!queue_attach_back(queue, node)) {
        node_destroy(node);
        return 0;
    }
    return 1;
}

int simple_dequeue(Queue *queue, WarehouseRecord *record_out, int *priority_out) {
    Node *node;

    if (queue == NULL || queue->type != QUEUE_SIMPLE) {
        return 0;
    }

    node = queue_detach_front(queue);
    if (node == NULL) {
        return 0;
    }

    if (record_out != NULL) {
        *record_out = node->data;
    }
    if (priority_out != NULL) {
        *priority_out = node->priority;
    }

    node_destroy(node);
    return 1;
}

int simple_peek_front(const Queue *queue, WarehouseRecord *record_out, int *priority_out) {
    if (queue == NULL || queue->type != QUEUE_SIMPLE || queue->front == NULL) {
        return 0;
    }

    if (record_out != NULL) {
        *record_out = queue->front->data;
    }
    if (priority_out != NULL) {
        *priority_out = queue->front->priority;
    }
    return 1;
}

void queue_display_forward(const Queue *queue, FILE *stream) {
    Node *current;
    int index = 1;

    if (stream == NULL) {
        return;
    }
    if (queue == NULL || queue->front == NULL) {
        fprintf(stream, "Queue is empty.\n");
        return;
    }

    current = queue->front;
    while (current != NULL) {
        print_warehouse_record(stream, &current->data, index, current->priority);
        current = current->next;
        index++;
        if (queue->type == QUEUE_CIRCULAR && current == queue->front) {
            break;
        }
    }
}

void queue_display_backward(const Queue *queue, FILE *stream) {
    Node *current;
    int index;

    if (stream == NULL) {
        return;
    }
    if (queue == NULL || queue->rear == NULL) {
        fprintf(stream, "Queue is empty.\n");
        return;
    }

    current = queue->rear;
    index = (int)queue->size;
    while (current != NULL) {
        print_warehouse_record(stream, &current->data, index, current->priority);
        current = current->prev;
        index--;
        if (queue->type == QUEUE_CIRCULAR && current == queue->rear) {
            break;
        }
    }
}

int deque_insert_front(Queue *queue, const WarehouseRecord *record) {
    Node *node;

    if (queue == NULL || record == NULL || queue->type != QUEUE_DEQUE) {
        return 0;
    }

    node = node_create(record, warehouse_auto_priority(record));
    if (node == NULL) {
        return 0;
    }
    if (!queue_attach_front(queue, node)) {
        node_destroy(node);
        return 0;
    }
    return 1;
}

int deque_insert_rear(Queue *queue, const WarehouseRecord *record) {
    Node *node;

    if (queue == NULL || record == NULL || queue->type != QUEUE_DEQUE) {
        return 0;
    }

    node = node_create(record, warehouse_auto_priority(record));
    if (node == NULL) {
        return 0;
    }
    if (!queue_attach_back(queue, node)) {
        node_destroy(node);
        return 0;
    }
    return 1;
}

int deque_delete_front(Queue *queue, WarehouseRecord *record_out, int *priority_out) {
    Node *node;

    if (queue == NULL || queue->type != QUEUE_DEQUE) {
        return 0;
    }

    node = queue_detach_front(queue);
    if (node == NULL) {
        return 0;
    }
    if (record_out != NULL) {
        *record_out = node->data;
    }
    if (priority_out != NULL) {
        *priority_out = node->priority;
    }
    node_destroy(node);
    return 1;
}

int deque_delete_rear(Queue *queue, WarehouseRecord *record_out, int *priority_out) {
    Node *node;

    if (queue == NULL || queue->type != QUEUE_DEQUE) {
        return 0;
    }

    node = queue_detach_rear(queue);
    if (node == NULL) {
        return 0;
    }
    if (record_out != NULL) {
        *record_out = node->data;
    }
    if (priority_out != NULL) {
        *priority_out = node->priority;
    }
    node_destroy(node);
    return 1;
}

int circular_enqueue(Queue *queue, const WarehouseRecord *record) {
    Node *node;

    if (queue == NULL || record == NULL || queue->type != QUEUE_CIRCULAR) {
        return 0;
    }

    node = node_create(record, warehouse_auto_priority(record));
    if (node == NULL) {
        return 0;
    }

    if (!queue_attach_back(queue, node)) {
        node_destroy(node);
        return 0;
    }
    return 1;
}

int circular_dequeue(Queue *queue, WarehouseRecord *record_out, int *priority_out) {
    Node *node;

    if (queue == NULL || queue->type != QUEUE_CIRCULAR) {
        return 0;
    }

    node = queue_detach_front(queue);
    if (node == NULL) {
        return 0;
    }

    if (record_out != NULL) {
        *record_out = node->data;
    }
    if (priority_out != NULL) {
        *priority_out = node->priority;
    }
    node_destroy(node);
    return 1;
}

int priority_enqueue(Queue *queue, const WarehouseRecord *record, int manual_priority, int use_manual_priority) {
    Node *node;
    Node *cursor;
    int priority;

    if (queue == NULL || record == NULL || queue->type != QUEUE_PRIORITY) {
        return 0;
    }

    priority = use_manual_priority ? manual_priority : warehouse_auto_priority(record);
    node = node_create(record, priority);
    if (node == NULL) {
        return 0;
    }

    if (queue->front == NULL) {
        if (!queue_attach_back(queue, node)) {
            node_destroy(node);
            return 0;
        }
        return 1;
    }

    cursor = queue->front;
    while (cursor != NULL && cursor->priority >= priority) {
        cursor = cursor->next;
    }

    if (cursor == NULL) {
        if (!queue_attach_back(queue, node)) {
            node_destroy(node);
            return 0;
        }
        return 1;
    }

    if (cursor->prev == NULL) {
        if (!queue_attach_front(queue, node)) {
            node_destroy(node);
            return 0;
        }
        return 1;
    }

    node->next = cursor;
    node->prev = cursor->prev;
    cursor->prev->next = node;
    cursor->prev = node;
    queue->size++;
    return 1;
}

int priority_dequeue(Queue *queue, WarehouseRecord *record_out, int *priority_out) {
    Node *node;

    if (queue == NULL || queue->type != QUEUE_PRIORITY) {
        return 0;
    }

    node = queue_detach_front(queue);
    if (node == NULL) {
        return 0;
    }

    if (record_out != NULL) {
        *record_out = node->data;
    }
    if (priority_out != NULL) {
        *priority_out = node->priority;
    }
    node_destroy(node);
    return 1;
}

Node *queue_node_at(const Queue *queue, size_t position) {
    Node *current;
    size_t index = 1;

    if (queue == NULL || position == 0 || position > queue->size) {
        return NULL;
    }

    current = queue->front;
    while (current != NULL) {
        if (index == position) {
            return current;
        }
        current = current->next;
        index++;
        if (queue->type == QUEUE_CIRCULAR && current == queue->front) {
            break;
        }
    }
    return NULL;
}

static size_t queue_search_text_field(const Queue *queue,
                                      const char *needle,
                                      const char *(*extractor)(const WarehouseRecord *),
                                      size_t *positions,
                                      size_t max_positions) {
    Node *current;
    size_t index = 1;
    size_t matched = 0;

    if (queue == NULL || needle == NULL || extractor == NULL || needle[0] == '\0') {
        return 0;
    }

    current = queue->front;
    while (current != NULL) {
        if (text_equals_ignore_case(extractor(&current->data), needle)) {
            if (positions != NULL && matched < max_positions) {
                positions[matched] = index;
            }
            matched++;
        }

        current = current->next;
        index++;
        if (queue->type == QUEUE_CIRCULAR && current == queue->front) {
            break;
        }
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

size_t queue_search_product_name(const Queue *queue, const char *product_name, size_t *positions, size_t max_positions) {
    return queue_search_text_field(queue, product_name, extract_product_name, positions, max_positions);
}

size_t queue_search_owner_surname(const Queue *queue, const char *owner_surname, size_t *positions, size_t max_positions) {
    return queue_search_text_field(queue, owner_surname, extract_owner_surname, positions, max_positions);
}

size_t queue_search_manufacturer(const Queue *queue, const char *manufacturer, size_t *positions, size_t max_positions) {
    return queue_search_text_field(queue, manufacturer, extract_manufacturer, positions, max_positions);
}

size_t queue_search_contract_year(const Queue *queue, int year, size_t *positions, size_t max_positions) {
    Node *current;
    size_t index = 1;
    size_t matched = 0;

    if (queue == NULL) {
        return 0;
    }

    current = queue->front;
    while (current != NULL) {
        if (current->data.contract_date.year == year) {
            if (positions != NULL && matched < max_positions) {
                positions[matched] = index;
            }
            matched++;
        }
        current = current->next;
        index++;
        if (queue->type == QUEUE_CIRCULAR && current == queue->front) {
            break;
        }
    }

    return matched;
}

size_t queue_search_unit_price_interval(const Queue *queue,
                                        double min_price,
                                        double max_price,
                                        size_t *positions,
                                        size_t max_positions) {
    Node *current;
    size_t index = 1;
    size_t matched = 0;

    if (queue == NULL || min_price > max_price) {
        return 0;
    }

    current = queue->front;
    while (current != NULL) {
        if (current->data.unit_price >= min_price && current->data.unit_price <= max_price) {
            if (positions != NULL && matched < max_positions) {
                positions[matched] = index;
            }
            matched++;
        }
        current = current->next;
        index++;
        if (queue->type == QUEUE_CIRCULAR && current == queue->front) {
            break;
        }
    }

    return matched;
}

int queue_delete_by_position(Queue *queue, size_t position, WarehouseRecord *record_out, int *priority_out) {
    Node *node;

    if (queue == NULL || position == 0 || position > queue->size) {
        return 0;
    }

    node = queue_node_at(queue, position);
    if (node == NULL) {
        return 0;
    }

    if (node == queue->front) {
        Node *detached = queue_detach_front(queue);
        if (detached == NULL) {
            return 0;
        }
        if (record_out != NULL) {
            *record_out = detached->data;
        }
        if (priority_out != NULL) {
            *priority_out = detached->priority;
        }
        node_destroy(detached);
        return 1;
    }

    if (node == queue->rear) {
        Node *detached = queue_detach_rear(queue);
        if (detached == NULL) {
            return 0;
        }
        if (record_out != NULL) {
            *record_out = detached->data;
        }
        if (priority_out != NULL) {
            *priority_out = detached->priority;
        }
        node_destroy(detached);
        return 1;
    }

    return queue_remove_middle_node(queue, node, record_out, priority_out);
}

size_t queue_delete_by_product_name(Queue *queue, const char *product_name, int delete_all) {
    size_t removed = 0;

    if (queue == NULL || product_name == NULL || product_name[0] == '\0') {
        return 0;
    }

    while (1) {
        size_t first_match_position = 0;
        size_t matches = queue_search_product_name(queue, product_name, &first_match_position, 1);
        if (matches == 0) {
            break;
        }
        if (!queue_delete_by_position(queue, first_match_position, NULL, NULL)) {
            break;
        }
        removed++;
        if (!delete_all) {
            break;
        }
    }

    return removed;
}

size_t queue_delete_by_owner_surname(Queue *queue, const char *owner_surname, int delete_all) {
    size_t removed = 0;

    if (queue == NULL || owner_surname == NULL || owner_surname[0] == '\0') {
        return 0;
    }

    while (1) {
        size_t first_match_position = 0;
        size_t matches = queue_search_owner_surname(queue, owner_surname, &first_match_position, 1);
        if (matches == 0) {
            break;
        }
        if (!queue_delete_by_position(queue, first_match_position, NULL, NULL)) {
            break;
        }
        removed++;
        if (!delete_all) {
            break;
        }
    }

    return removed;
}

int queue_export_arrays(const Queue *queue, WarehouseRecord **records_out, int **priorities_out, size_t *count_out) {
    WarehouseRecord *records;
    int *priorities;
    Node *current;
    size_t index = 0;

    if (queue == NULL || records_out == NULL || priorities_out == NULL || count_out == NULL) {
        return 0;
    }

    *records_out = NULL;
    *priorities_out = NULL;
    *count_out = 0;

    if (queue->size == 0) {
        return 1;
    }

    records = (WarehouseRecord *)calloc(queue->size, sizeof(WarehouseRecord));
    priorities = (int *)calloc(queue->size, sizeof(int));
    if (records == NULL || priorities == NULL) {
        free(records);
        free(priorities);
        return 0;
    }

    current = queue->front;
    while (current != NULL && index < queue->size) {
        records[index] = current->data;
        priorities[index] = current->priority;
        index++;
        current = current->next;
        if (queue->type == QUEUE_CIRCULAR && current == queue->front) {
            break;
        }
    }

    *records_out = records;
    *priorities_out = priorities;
    *count_out = index;
    return 1;
}
