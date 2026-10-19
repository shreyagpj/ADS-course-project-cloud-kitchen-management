/* TODO: store orders in an array (id = index + 1); use pq.h for the queue. */
#include "orders.h"
int  order_place(int u, const OrderLine l[], int n, int p) { (void)u;(void)l;(void)n;(void)p; TODO(); return -1; }
int  order_get(int id, Order *o)                           { (void)id;(void)o; TODO(); return -1; }
void order_show(int id)                                    { (void)id; TODO(); }
void order_queue_show(void)                                { TODO(); }
int  order_cook_next(int chef)                             { (void)chef; TODO(); return -1; }
int  order_mark_ready(int id)                              { (void)id; TODO(); return -1; }
int  order_set_status(int id, OrderStatus s)               { (void)id;(void)s; TODO(); return -1; }
int  order_set_delivery_staff(int id, int staff)           { (void)id;(void)staff; TODO(); return -1; }
