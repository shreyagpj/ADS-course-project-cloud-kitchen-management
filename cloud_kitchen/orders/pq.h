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
 *
 * Implementation hint: array-based heap; parent(i) = (i-1)/2; sift-up on push,
 * sift-down on pop. Grow the array with realloc when it is full.              */
#ifndef PQ_H
#define PQ_H

int  pq_push(int order_id, long key);
int  pq_pop(void);          /* order_id with the smallest key, or -1 */
void pq_show(void);         /* print in priority order (don't destroy the heap) */
#endif
