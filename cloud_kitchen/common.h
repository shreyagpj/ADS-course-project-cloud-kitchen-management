#ifndef COMMON_H
#define COMMON_H
#include <stdio.h>
#include <mysql/mysql.h>

//add functions return the new id or -1; all others return 0 on success, -1 on failure
#define MAX_NAME  50
#define MAX_LINES 10    //different items in one order

extern MYSQL *conn;

typedef struct {
    double lat, lon;
} GeoPoint;

typedef enum { ORDER_PLACED, ORDER_COOKING, ORDER_READY, ORDER_OUT_FOR_DELIVERY, ORDER_DELIVERED } OrderStatus;
typedef enum { ROLE_CHEF = 1, ROLE_DELIVERY = 2 } Role;

#endif
