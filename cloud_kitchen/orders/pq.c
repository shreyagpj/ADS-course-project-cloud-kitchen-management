#include <stdio.h>
#include <stdlib.h>
#include "pq.h"
#include "../common.h"

#define INITIAL_CAPACITY 16

typedef struct {
    int order_id;
    long key;
} PQNode;

static PQNode *heap = NULL;
static int size = 0;
static int capacity = 0;

static void swap(PQNode *a, PQNode *b) {
    PQNode temp = *a;
    *a = *b;
    *b = temp;
}

// Move child upward after pushing
static void move_up(int i) {
    while (i > 0) {
        int parent = (i - 1) / 2;

        if (heap[parent].key <= heap[i].key)
            break;
        swap(&heap[parent], &heap[i]);
        i = parent;
    }
}

// Move new root downwards after popping
static void move_down(int i) {
    while (1) {
        int smallest = i, left = 2 * i + 1, right = 2 * i + 2;

        if (left < size && heap[left].key < heap[smallest].key)
            smallest = left;
        if (right < size && heap[right].key < heap[smallest].key)
            smallest = right;
        if (smallest == i)
            break;

        swap(&heap[i], &heap[smallest]);
        i = smallest;
    }
}

// Add order to priority queue and double size if full
int pq_push(int id, long key) {
    if (size == capacity) {
        int new_capacity = (capacity == 0)? INITIAL_CAPACITY : capacity * 2;
        PQNode *temp = realloc(heap, new_capacity * sizeof(PQNode));

        if (temp == NULL)
            return -1;

        heap = temp;
        capacity = new_capacity;
    }

    heap[size].order_id = id;
    heap[size].key = key;
    move_up(size);
    size++;
    return 0;
}

// Remove highest priority order from queue
int pq_pop(void) {
    if (size == 0)
        return -1;

    int id = heap[0].order_id;
    heap[0] = heap[size - 1];
    size--;

    if (size > 0)
        move_down(0);

    return id;
}

// Remove a specific order from anywhere in the queue (used when an order is deleted/cancelled)
int pq_remove(int id) {
    for (int i = 0; i < size; i++) {
        if (heap[i].order_id != id)
            continue;

        heap[i] = heap[size - 1];
        size--;
        if (i < size) {            /* moved node may need to go either way */
            move_up(i);
            move_down(i);
        }
        return 0;
    }
    return -1;                     /* not in the queue */
}

// Empty the queue (used before reloading it from the database)
void pq_clear(void) {
    size = 0;
}

static int compare_nodes(const void *a, const void *b) {
    const PQNode *x = a, *y = b;
    return (x->key > y->key) - (x->key < y->key);
}

// Display orders present in queue, in cooking order (the heap itself is left untouched)
void pq_show(void) {
    if (size == 0) {
        printf("Orders queue is empty.\n");
        return;
    }

    PQNode *copy = malloc(size * sizeof(PQNode));
    if (copy == NULL) {
        printf("Out of memory.\n");
        return;
    }
    for (int i = 0; i < size; i++)
        copy[i] = heap[i];
    qsort(copy, size, sizeof(PQNode), compare_nodes);

    printf("Orders waiting to be cooked:\n");
    for (int i = 0; i < size; i++)
        printf("%d. Order ID: %d, Key: %ld\n", i + 1, copy[i].order_id, copy[i].key);
    free(copy);
}
