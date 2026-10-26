#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "delivery.h"
#include "kdtree.h"

#define PI 3.14159265358979323846

static MYSQL_RES *sel(const char *q)
{
    if (mysql_query(conn, q))
    {
        printf("Database error: %s\n", mysql_error(conn));
        return NULL;
    }
    return mysql_store_result(conn);
}

static int upd(const char *q)
{
    if (mysql_query(conn, q))
    {
        printf("Database error: %s\n", mysql_error(conn));
        return -1;
    }
    return (int) mysql_affected_rows(conn);
}

int delivery_add(int id, GeoPoint g)
{
    char q[512];

    snprintf(q, sizeof q, "SELECT staff_id FROM staff WHERE staff_id = %d AND role = 'DELIVERY'", id);
    MYSQL_RES *res = sel(q);
    if (res == NULL) return -1;
    int found = (mysql_fetch_row(res) != NULL);
    mysql_free_result(res);
    if (!found)
    {
        printf("Staff %d is not a delivery partner.\n", id);
        return -1;
    }

    snprintf(q, sizeof q,
             "INSERT INTO delivery_partner (staff_id, latitude, longitude) VALUES (%d, %.7f, %.7f) "
             "ON DUPLICATE KEY UPDATE latitude = %.7f, longitude = %.7f",
             id, g.lat, g.lon, g.lat, g.lon);
    return upd(q) < 0 ? -1 : 0;
}

int delivery_assign(int oid)
{
    char q[1024];

    snprintf(q, sizeof q,
             "SELECT u.latitude, u.longitude FROM orders o JOIN users u ON u.user_id = o.user_id "
             "WHERE o.order_id = %d AND o.order_status = 'READY' AND o.delivery_id IS NULL", oid);
    MYSQL_RES *res = sel(q);
    if (res == NULL) return -1;
    MYSQL_ROW row = mysql_fetch_row(res);
    if (row == NULL)
    {
        mysql_free_result(res);
        return -1;
    }
    if (row[0] == NULL || row[1] == NULL)
    {
        printf("Customer of order %d has no delivery location.\n", oid);
        mysql_free_result(res);
        return -1;
    }
    double la = atof(row[0]), lo = atof(row[1]);
    mysql_free_result(res);

    //longitude shrinks by cos of latitude so both axes have a similar scale
    double k = cos(la * PI / 180.0);
    GeoPoint t = { la, lo * k };

    snprintf(q, sizeof q,
             "SELECT p.staff_id, p.latitude, p.longitude, EXISTS (SELECT 1 FROM orders o "
             "WHERE o.delivery_id = p.staff_id AND o.order_status = 'OUT_FOR_DELIVERY') "
             "FROM delivery_partner p");
    res = sel(q);
    if (res == NULL) return -1;
    KDNode *root = NULL;
    while ((row = mysql_fetch_row(res)) != NULL)
    {
        int id = atoi(row[0]);
        GeoPoint g = { atof(row[1]), atof(row[2]) * k };
        root = kd_add(root, g, id);

        //a partner with an order on the road is busy
        if (atoi(row[3])) kd_busy(root, id, 1);
    }
    mysql_free_result(res);

    int s = kd_nearest(root, t);
    kd_free(root);
    if (s < 0) return -1;

    snprintf(q, sizeof q,
             "UPDATE orders SET delivery_id = %d, order_status = 'OUT_FOR_DELIVERY' "
             "WHERE order_id = %d AND order_status = 'READY'", s, oid);
    return upd(q) == 1 ? s : -1;
}

int delivery_done(int oid)
{
    char q[1024];

    snprintf(q, sizeof q,
             "UPDATE delivery_partner p JOIN orders o ON o.delivery_id = p.staff_id "
             "JOIN users u ON u.user_id = o.user_id "
             "SET p.latitude = u.latitude, p.longitude = u.longitude "
             "WHERE o.order_id = %d AND o.order_status = 'OUT_FOR_DELIVERY' "
             "AND u.latitude IS NOT NULL AND u.longitude IS NOT NULL", oid);
    if (upd(q) < 0) return -1;

    snprintf(q, sizeof q,
             "UPDATE orders SET order_status = 'DELIVERED' "
             "WHERE order_id = %d AND order_status = 'OUT_FOR_DELIVERY'", oid);
    return upd(q) == 1 ? 0 : -1;
}
