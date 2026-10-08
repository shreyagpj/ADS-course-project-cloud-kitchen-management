/* common.h - shared types. Rule for all modules:
 *   "add" functions return the new ID (or -1 on failure)
 *   other functions return 0 on success, -1 on failure                      */
#ifndef COMMON_H
#define COMMON_H
#include <stdio.h>

#define NAME_LEN  50
#define MAX_LINES 10                       /* max different items in one order */
#define TODO() printf("[TODO] %s() not implemented yet\n", __func__)

typedef struct { double lat, lon; } GeoPoint;

typedef enum { ORDER_PLACED, ORDER_COOKING, ORDER_READY,
               ORDER_OUT_FOR_DELIVERY, ORDER_DELIVERED } OrderStatus;
typedef enum { ROLE_CHEF = 1, ROLE_DELIVERY = 2 } Role;

#endif
