#include "queue.h"

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
