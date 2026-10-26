#ifndef ORDERS_H
#define ORDERS_H
#include "../common.h"

typedef struct
{
    int item_id;
    int qty;
} OrderLine;

typedef struct
{
    int         id, user_id, priority;
    OrderLine   lines[MAX_LINES];
    int         line_count;
    double      total;
    OrderStatus status;
    int         chef_id, dstaff;   //0 means not assigned yet
} Order;

int  order_place(int uid, const OrderLine lines[], int n, int pr);
int  order_get(int id, Order *out);
void order_show(int id);
void order_queue(void);
int  order_cook(int chef);
int  order_ready(int id);
int  order_delete(int id);
int  order_load(void);
int  order_set_state(int id, OrderStatus s);
int  order_set_driver(int id, int sid);
#endif
