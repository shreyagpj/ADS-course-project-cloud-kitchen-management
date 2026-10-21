#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "delivery.h"
#include "menu.h"

static Order *front = NULL;     /* priority queue as a sorted linked list */

void addOrder(int orderId, const char *customer, const char *address,
              int menuId, int quantity, int priority) {
    if (findMenuItem(menuId) == NULL) {
        printf("Menu item %d does not exist.\n", menuId);
        return;
    }

    Order *o = (Order *)malloc(sizeof(Order));
    if (o == NULL) {
        printf("Out of memory.\n");
        return;
    }
    o->orderId = orderId;
    strncpy(o->customer, customer, sizeof(o->customer) - 1);
    o->customer[sizeof(o->customer) - 1] = '\0';
    strncpy(o->address, address, sizeof(o->address) - 1);
    o->address[sizeof(o->address) - 1] = '\0';
    o->menuId = menuId;
    o->quantity = quantity;
    o->priority = priority;
    o->next = NULL;

    /* keep the list sorted by priority; equal priority stays first come first served */
    if (front == NULL || priority < front->priority) {
        o->next = front;
        front = o;
    } else {
        Order *cur = front;
        while (cur->next != NULL && cur->next->priority <= priority)
            cur = cur->next;
        o->next = cur->next;
        cur->next = o;
    }
    printf("Order %d added.\n", orderId);
}

void startDelivery(void) {
    if (front == NULL) {
        printf("No orders to deliver.\n");
        return;
    }
    Order *o = front;
    front = front->next;

    MenuItem *item = findMenuItem(o->menuId);
    printf("Delivering order %d: %d x %s to %s (%s)\n", o->orderId, o->quantity,
           item ? item->name : "unknown item", o->customer, o->address);
    free(o);
}

void showOrders(void) {
    if (front == NULL) {
        printf("No pending orders.\n");
        return;
    }
    printf("OrderID  Customer        Item  Qty  Priority\n");
    for (Order *cur = front; cur != NULL; cur = cur->next)
        printf("%-8d %-15s %-5d %-4d %d\n", cur->orderId, cur->customer,
               cur->menuId, cur->quantity, cur->priority);
}