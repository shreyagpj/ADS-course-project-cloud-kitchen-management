/* pq.h - PRIORITY QUEUE (min-heap) of orders waiting to be cooked.
 * There is ONE queue for the whole kitchen, kept inside pq.c (no struct to pass).
 *
 * key = priority * 100000 + order_id   -> lower key is cooked first;
 *       same priority => older order (smaller id) first.
 *
 * orders.c uses it like this:
 *   order_place()     -> pq_push(order_id, key)
 *   order_cook_next() -> id = pq_pop()          (returns -1 if the queue is empty)
 *   order_queue_show()-> pq_show()
 */
#ifndef PQ_H
#define PQ_H

// Push and pop return -1 on failure
int  pq_push(int order_id, long key);
int  pq_pop(void);
void pq_show(void);
#endif
