#include <stdio.h>
#include <stdlib.h>
#include "pq.h"

#define INITIAL_CAPACITY 16

typedef struct {
    int order_id;
    long long key;
} PQNode;

static PQNode *heap = NULL;
static int size = 0;
static int cap = 0;

static void swap(PQNode *a, PQNode *b) {
    PQNode tp = *a;
    *a = *b;
    *b = tp;
}

//Move child upward after pushing
static void move_up(int i) {
    while (i > 0) {
        int parent = (i - 1) / 2;

        if (heap[parent].key <= heap[i].key)
            break;
        swap(&heap[parent], &heap[i]);
        i = parent;
    }
}

//Move new root downwards after popping
static void move_down(int i) {
    while (1) {
        int sm = i, left = 2 * i + 1, right = 2 * i + 2;

        if (left < size && heap[left].key < heap[sm].key)
            sm = left;
        if (right < size && heap[right].key < heap[sm].key)
            sm = right;
        if (sm == i)
            break;

        swap(&heap[i], &heap[sm]);
        i = sm;
    }
}

//Add order to priority queue and double size if full
int pq_push(int id, long long key) {
    if (size == cap) {
        int nc = (cap == 0)? INITIAL_CAPACITY : cap * 2;
        PQNode *tp = realloc(heap, nc * sizeof(PQNode));

        if (tp == NULL)
            return -1;

        heap = tp;
        cap = nc;
    }

    heap[size].order_id = id;
    heap[size].key = key;
    move_up(size);
    size++;
    return 0;
}

//Remove highest priority order from queue
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

//Remove a specific order from anywhere in the queue (used when an order is deleted/cancelled)
int pq_remove(int id) {
    for (int i = 0; i < size; i++) {
        if (heap[i].order_id != id)
            continue;

        heap[i] = heap[size - 1];
        size--;
        if (i < size) {
            move_up(i);
            move_down(i);
        }
        return 0;
    }
    return -1;
}

void pq_clear(void) {
    size = 0;
}

static int compare_nodes(const void *a, const void *b) {
    const PQNode *x = a, *y = b;
    return (x->key > y->key) - (x->key < y->key);
}

//Display orders present in queue, in cooking order
void pq_show(void) {
    if (size == 0) {
        printf("Orders queue is empty.\n");
        return;
    }

    PQNode *cp = malloc(size * sizeof(PQNode));
    if (cp == NULL) {
        printf("Out of memory.\n");
        return;
    }
    for (int i = 0; i < size; i++)
        cp[i] = heap[i];
    qsort(cp, size, sizeof(PQNode), compare_nodes);

    printf("Orders waiting to be cooked:\n");
    for (int i = 0; i < size; i++)
        printf("%d. Order ID: %d, Key: %lld\n", i + 1, cp[i].order_id, cp[i].key);
    free(cp);
}
