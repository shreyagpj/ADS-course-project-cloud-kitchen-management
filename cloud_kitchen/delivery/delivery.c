#include <stdio.h>
#include <stdlib.h>
#include "delivery.h"
#include "../common.h"
#include "../db/db.h"

extern MYSQL *conn;

/* Runs a SELECT and returns the result (caller frees it), or NULL on error. */
static MYSQL_RES *run_select(const char *query)
{
    if (mysql_query(conn, query)) {
        printf("Database error: %s\n", mysql_error(conn));
        return NULL;
    }
    return mysql_store_result(conn);
}

/* Runs an INSERT/UPDATE. Returns affected rows, or -1 on error. */
static int run_update(const char *query)
{
    if (mysql_query(conn, query)) {
        printf("Database error: %s\n", mysql_error(conn));
        return -1;
    }
    return (int)mysql_affected_rows(conn);
}

int delivery_add_partner(int staff_id, GeoPoint loc)
{
    char query[512];

    /* only staff with the DELIVERY role can be partners */
    snprintf(query, sizeof query,
             "SELECT staff_id FROM staff WHERE staff_id = %d AND role = 'DELIVERY'",
             staff_id);
    MYSQL_RES *res = run_select(query);
    if (res == NULL)
        return -1;
    int found = (mysql_fetch_row(res) != NULL);
    mysql_free_result(res);
    if (!found) {
        printf("Staff %d is not a delivery partner.\n", staff_id);
        return -1;
    }

    snprintf(query, sizeof query,
             "INSERT INTO delivery_partner (staff_id, latitude, longitude) "
             "VALUES (%d, %.7f, %.7f) "
             "ON DUPLICATE KEY UPDATE latitude = %.7f, longitude = %.7f",
             staff_id, loc.lat, loc.lon, loc.lat, loc.lon);
    return run_update(query) < 0 ? -1 : 0;
}

int delivery_assign(int order_id)
{
    char query[1024];

    /* 1. the order must be READY, not yet assigned, and its customer needs a location */
    snprintf(query, sizeof query,
             "SELECT u.latitude, u.longitude "
             "FROM orders o JOIN users u ON u.user_id = o.user_id "
             "WHERE o.order_id = %d AND o.order_status = 'READY' AND o.delivery_id IS NULL",
             order_id);
    MYSQL_RES *res = run_select(query);
    if (res == NULL)
        return -1;
    MYSQL_ROW row = mysql_fetch_row(res);
    if (row == NULL) {
        mysql_free_result(res);
        return -1;
    }
    if (row[0] == NULL || row[1] == NULL) {
        printf("Customer of order %d has no delivery location.\n", order_id);
        mysql_free_result(res);
        return -1;
    }
    double lat = atof(row[0]);
    double lon = atof(row[1]);
    mysql_free_result(res);

    /* 2. nearest free partner: not currently carrying an OUT_FOR_DELIVERY order.
          Distance is the flat-earth approximation, good enough inside a city. */
    snprintf(query, sizeof query,
             "SELECT p.staff_id FROM delivery_partner p "
             "WHERE NOT EXISTS (SELECT 1 FROM orders o "
             "                  WHERE o.delivery_id = p.staff_id "
             "                  AND o.order_status = 'OUT_FOR_DELIVERY') "
             "ORDER BY POW(p.latitude - %.7f, 2) "
             "       + POW((p.longitude - %.7f) * COS(RADIANS(%.7f)), 2) "
             "LIMIT 1",
             lat, lon, lat);
    res = run_select(query);
    if (res == NULL)
        return -1;
    row = mysql_fetch_row(res);
    if (row == NULL) {
        mysql_free_result(res);
        return -1;                      /* nobody is free */
    }
    int partner = atoi(row[0]);
    mysql_free_result(res);

    /* 3. assign the partner */
    snprintf(query, sizeof query,
             "UPDATE orders SET delivery_id = %d, order_status = 'OUT_FOR_DELIVERY' "
             "WHERE order_id = %d AND order_status = 'READY'",
             partner, order_id);
    return run_update(query) == 1 ? partner : -1;
}

int delivery_complete(int order_id)
{
    char query[1024];

    /* the partner is now standing at the customer's location */
    snprintf(query, sizeof query,
             "UPDATE delivery_partner p "
             "JOIN orders o ON o.delivery_id = p.staff_id "
             "JOIN users u ON u.user_id = o.user_id "
             "SET p.latitude = u.latitude, p.longitude = u.longitude "
             "WHERE o.order_id = %d AND o.order_status = 'OUT_FOR_DELIVERY' "
             "AND u.latitude IS NOT NULL AND u.longitude IS NOT NULL",
             order_id);
    if (run_update(query) < 0)
        return -1;

    snprintf(query, sizeof query,
             "UPDATE orders SET order_status = 'DELIVERED' "
             "WHERE order_id = %d AND order_status = 'OUT_FOR_DELIVERY'",
             order_id);
    return run_update(query) == 1 ? 0 : -1;
}