#ifndef COMMON_H
#define COMMON_H
#include <stdio.h>
#include <mysql/mysql.h>

//add calls give the new id or minus one and others give 0 or minus one
#define MAX_NAME  50
#define MAX_LINES 10

extern MYSQL *conn;

typedef struct
{
    double lat, lon;
} GeoPoint;

typedef enum { ORDER_PLACED, ORDER_COOKING, ORDER_READY, ORDER_OUT_FOR_DELIVERY, ORDER_DELIVERED } OrderStatus;
typedef enum { ROLE_CHEF = 1, ROLE_DELIVERY = 2 } Role;

#endif
