//orders.h: PLACED -> COOKING -> READY (here), then OUT_FOR_DELIVERY -> DELIVERED (delivery.c)
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
    int         chef_id, dstaff; //0 = not assigned yet
} Order;

int  order_place(int uid, const OrderLine lines[], int n, int pr);
int  order_get(int id, Order *out);
void order_show(int id);
void order_queue_show(void);
int  order_cook_next(int chef_id); //returns order_id or -1
int  order_mark_ready(int id);

int  order_delete(int id); //removes from the database AND the queue; 0 or -1
int  order_load_queue(void); //call once after connectDb(): reloads PLACED orders into the queue

int  order_set_status(int id, OrderStatus s);
int  order_set_delivery_staff(int id, int sid);
#endif
