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

// Display orders present in queue
void pq_show(void) {
    if (size == 0) {
        printf("Orders queue is empty.\n");
        return;
    }

    printf("Orders waiting to be cooked:\n");
    for (int i = 0; i < size; i++)
        printf("Order ID: %d, Key: %ld\n", heap[i].order_id, heap[i].key);
}
