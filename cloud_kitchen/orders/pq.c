#include <stdio.h>
#include <stdlib.h>
#include "pq.h"

#define INIT_CAP 16

typedef struct
{
    int oid;
    long long key;
} Node;

static Node *heap = NULL;
static int size = 0;
static int cap = 0;

static void swap(Node *a, Node *b)
{
    Node t = *a;
    *a = *b;
    *b = t;
}

//lifts a node while it is smaller than its parent
static void up(int i)
{
    while (i > 0)
    {
        int p = (i - 1) / 2;

        if (heap[p].key <= heap[i].key)
            break;
        swap(&heap[p], &heap[i]);
        i = p;
    }
}

static void down(int i)
{
    while (1)
    {
        int s = i, l = 2 * i + 1, r = 2 * i + 2;

        if (l < size && heap[l].key < heap[s].key)
            s = l;
        if (r < size && heap[r].key < heap[s].key)
            s = r;
        if (s == i)
            break;

        swap(&heap[i], &heap[s]);
        i = s;
    }
}

//the array doubles when it is full
int pq_push(int oid, long long key)
{
    if (size == cap)
    {
        int nc = (cap == 0) ? INIT_CAP : cap * 2;
        Node *t = realloc(heap, nc * sizeof(Node));

        if (t == NULL)
            return -1;

        heap = t;
        cap = nc;
    }

    heap[size].oid = oid;
    heap[size].key = key;
    up(size);
    size++;
    return 0;
}

int pq_pop(void)
{
    if (size == 0)
        return -1;

    int oid = heap[0].oid;
    heap[0] = heap[size - 1];
    size--;

    if (size > 0)
        down(0);

    return oid;
}

//the last node fills the gap so it may go up or down
int pq_del(int oid)
{
    for (int i = 0; i < size; i++)
    {
        if (heap[i].oid != oid)
            continue;

        heap[i] = heap[size - 1];
        size--;
        if (i < size)
        {
            up(i);
            down(i);
        }
        return 0;
    }
    return -1;
}

void pq_clear(void)
{
    size = 0;
}

static int cmp(const void *a, const void *b)
{
    const Node *x = a, *y = b;
    return (x->key > y->key) - (x->key < y->key);
}

void pq_show(void)
{
    if (size == 0)
    {
        printf("Orders queue is empty.\n");
        return;
    }

    Node *c = malloc(size * sizeof(Node));
    if (c == NULL)
    {
        printf("Out of memory.\n");
        return;
    }
    for (int i = 0; i < size; i++)
        c[i] = heap[i];
    qsort(c, size, sizeof(Node), cmp);

    printf("Orders waiting to be cooked:\n");
    for (int i = 0; i < size; i++)
        printf("%d. Order ID: %d, Key: %lld\n", i + 1, c[i].oid, c[i].key);
    free(c);
}
