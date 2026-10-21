#ifndef DELIVERY_H
#define DELIVERY_H

typedef struct Order {
    int orderId;
    char customer[50];
    char address[100];
    int menuId;
    int quantity;
    int priority;               /* 1 = urgent, bigger number = less urgent */
    struct Order *next;
} Order;

void addOrder(int orderId, const char *customer, const char *address,
              int menuId, int quantity, int priority);
void startDelivery(void);       /* pops the highest priority order */
void showOrders(void);

#endif