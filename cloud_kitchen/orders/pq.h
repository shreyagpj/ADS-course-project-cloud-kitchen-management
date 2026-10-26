#ifndef PQ_H
#define PQ_H

//priority times a billion plus the id so equal priority goes oldest first
#define PQ_KEY(p, id) ((long long)(p) * 1000000000LL + (id))

int  pq_push(int oid, long long key);
int  pq_pop(void);
int  pq_del(int oid);
void pq_clear(void);
void pq_show(void);
#endif
