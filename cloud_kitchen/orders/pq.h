#ifndef PQ_H
#define PQ_H

//min-heap of PLACED orders; lower key cooks first, same priority means older id first
#define PQ_KEY(p, id) ((long long)(p) * 1000000000LL + (id))

int  pq_push(int oid, long long key);
int  pq_pop(void);              //order_id with the smallest key, or -1
int  pq_remove(int order_id);   //0 if removed, -1 if not queued
void pq_clear(void);
void pq_show(void);             //prints in cooking order, heap untouched
#endif
