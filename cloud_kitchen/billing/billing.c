#include <stdio.h>
#include <string.h>
#include "billing.h"
#include "../common.h"

static int run_query(const char *q)
{
    MYSQL *db = conn;
    if (db == NULL) { fprintf(stderr, "No database connection.\n"); return -1; }
    if (mysql_query(db, q) != 0) {
        fprintf(stderr, "Database error: %s\n", mysql_error(db));
        return -1;
    }
    return 0;
}

//add: copies total and customer from the order; returns the new bill id or -1
int bill_generate(int oid)
{
    char q[512];
    MYSQL *db = conn;
    if (db == NULL) { fprintf(stderr, "No database connection.\n"); return -1; }

    snprintf(q, sizeof q,
             "INSERT INTO billing (order_id, user_id, total_amount, address) "
             "SELECT o.order_id, o.user_id, o.total_cost, "
             "       CONCAT_WS(', ', u.house_no, u.soc_name, u.street, u.city) "
             "FROM orders o JOIN users u ON u.user_id = o.user_id "
             "WHERE o.order_id = %d", oid);

    if (mysql_query(db, q) != 0) {
        if (mysql_errno(db) == 1062) //duplicate: uq_billing_order
            printf("Order %d already has a bill.\n", oid);
        else
            fprintf(stderr, "Database error: %s\n", mysql_error(db));
        return -1;
    }
    if (mysql_affected_rows(db) != 1) { //the SELECT found no order
        printf("Order %d not found.\n", oid);
        return -1;
    }
    return (int)mysql_insert_id(db);
}

//delete: removes the bill from the database; 0 on success, -1 if not found/failed
int bill_delete(int bid)
{
    char q[128];
    snprintf(q, sizeof q, "DELETE FROM billing WHERE bill_id = %d", bid);
    if (run_query(q) != 0) return -1;
    if (mysql_affected_rows(conn) != 1) {
        printf("Bill %d not found.\n", bid);
        return -1;
    }
    return 0;
}

int bill_pay(int id, const char *m)
{
    static const char *ok[] = { "CASH", "CARD", "UPI", "NET_BANKING" };
    int v = 0;
    for (int i = 0; m && i < 4; i++)
        if (strcmp(m, ok[i]) == 0) v = 1;
    if (!v) { printf("Invalid payment method.\n"); return -1; }

    char q[256];
    snprintf(q, sizeof q,
             "UPDATE billing SET payment_status = 'PAID', payment_method = '%s', payment_time = NOW() "
             "WHERE bill_id = %d AND payment_status = 'PENDING'", m, id);
    if (run_query(q) != 0) return -1;
    if (mysql_affected_rows(conn) != 1) {
        printf("Bill %d not found or already paid.\n", id);
        return -1;
    }
    return 0;
}

void bill_show(int bid)
{
    char q[256];
    snprintf(q, sizeof q,
             "SELECT bill_id, order_id, user_id, total_amount, IFNULL(address, '-'), "
             "payment_status, IFNULL(payment_method, '-'), IFNULL(payment_time, '-') "
             "FROM billing WHERE bill_id = %d", bid);
    if (run_query(q) != 0) return;

    MYSQL_RES *res = mysql_store_result(conn);
    if (res == NULL) return;
    MYSQL_ROW row = mysql_fetch_row(res);
    if (row == NULL) {
        printf("Bill %d not found.\n", bid);
    } else {
        printf("Bill %s  (order %s, user %s)\n", row[0], row[1], row[2]);
        printf("  Total   : %s\n", row[3]);
        printf("  Address : %s\n", row[4]);
        printf("  Status  : %s\n", row[5]);
        printf("  Method  : %s\n", row[6]);
        printf("  Paid at : %s\n", row[7]);
    }
    mysql_free_result(res);
}
