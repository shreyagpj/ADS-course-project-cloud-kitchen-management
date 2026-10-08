/* orders.h - placing and cooking orders.
 *
 * Life of an order (status):
 *   PLACED --order_cook_next()--> COOKING --order_mark_ready()--> READY
 *   READY --delivery_assign()--> OUT_FOR_DELIVERY --delivery_complete()--> DELIVERED
 *
 * main.c calls:
 *   OrderLine lines[MAX_LINES]; ...fill lines, n...
 *   int oid = order_place(user_id, lines, n, priority);   // priority: 1=urgent 2=normal 3=low
 *   order_show(oid);
 *   order_queue_show();                  // what the kitchen will cook next
 *   int next = order_cook_next(chef_id); // pops the priority queue
 *   order_mark_ready(next);
 *
 * order_place() should: check the user (user_get), add up item prices (item_get),
 *   save the order with status PLACED, then pq_push(order_id, priority*100000+order_id).
 *
 * delivery.c calls: order_get(), order_set_status(), order_set_delivery_staff().  */
#ifndef ORDERS_H
#define ORDERS_H
#include "../common.h"

typedef struct { int item_id; int qty; } OrderLine;

typedef struct {
    int         id, user_id, priority;
    OrderLine   lines[MAX_LINES];
    int         line_count;
    double      total;
    OrderStatus status;
    int         chef_id, delivery_id;      /* 0 = not assigned yet */
} Order;

int  order_place(int user_id, const OrderLine lines[], int n, int priority);
int  order_get(int id, Order *out);
void order_show(int id);
void order_queue_show(void);
int  order_cook_next(int chef_id);         /* returns order_id or -1 */
int  order_mark_ready(int id);

int  order_set_status(int id, OrderStatus s);
int  order_set_delivery_staff(int id, int staff_id);
#endif
