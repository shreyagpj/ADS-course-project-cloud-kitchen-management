#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../common.h"
#include "orders.h"
#include "pq.h"

#define KEY(priority, id) ((long)(priority) * 100000L + (id))

static const char *STATUS_NAME[] = { "PLACED", "COOKING", "READY", "OUT_FOR_DELIVERY", "DELIVERED" };

static int run_query(const char *sql)
{
    MYSQL *db = conn;
    if (db == NULL) { fprintf(stderr, "No database connection.\n"); return -1; }
    if (mysql_query(db, sql) != 0) {
        fprintf(stderr, "Database error: %s\n", mysql_error(db));
        return -1;
    }
    return 0;
}

/* Runs a SELECT and returns the first row (caller frees *res), or NULL. */
static MYSQL_ROW select_one(const char *sql, MYSQL_RES **res)
{
    *res = NULL;
    if (run_query(sql) != 0) return NULL;
    *res = mysql_store_result(conn);
    if (*res == NULL) return NULL;
    MYSQL_ROW row = mysql_fetch_row(*res);
    if (row == NULL) { mysql_free_result(*res); *res = NULL; }
    return row;
}

static int status_from_name(const char *name)
{
    for (int i = 0; i < 5; i++)
        if (strcmp(name, STATUS_NAME[i]) == 0) return i;
    return -1;
}

static int order_exists(int id)
{
    char sql[128];
    MYSQL_RES *res;
    snprintf(sql, sizeof sql, "SELECT 1 FROM orders WHERE order_id = %d", id);
    if (select_one(sql, &res) == NULL) return 0;
    mysql_free_result(res);
    return 1;
}

/* 1 if staff_id exists and has the given role ("CHEF" / "DELIVERY"), else 0 */
static int staff_has_role(int staff_id, const char *role)
{
    char sql[160];
    MYSQL_RES *res;
    snprintf(sql, sizeof sql,
             "SELECT 1 FROM staff WHERE staff_id = %d AND role = '%s'", staff_id, role);
    if (select_one(sql, &res) == NULL) return 0;
    mysql_free_result(res);
    return 1;
}

/* ---------- add ---------- */

int order_place(int user_id, const OrderLine lines[], int n, int priority)
{
    if (lines == NULL || n <= 0 || n > MAX_LINES) {
        printf("An order needs between 1 and %d items.\n", MAX_LINES);
        return -1;
    }
    if (priority < 1 || priority > 3) {
        printf("Priority must be 1 (urgent), 2 (normal) or 3 (low).\n");
        return -1;
    }

    char sql[512];
    MYSQL_RES *res;
    MYSQL_ROW row;

    snprintf(sql, sizeof sql, "SELECT 1 FROM users WHERE user_id = %d", user_id);
    if ((row = select_one(sql, &res)) == NULL) {
        printf("User %d does not exist.\n", user_id);
        return -1;
    }
    mysql_free_result(res);

    /* merge repeated items (order_items has one row per item) and price them */
    OrderLine merged[MAX_LINES];
    int m = 0;
    for (int i = 0; i < n; i++) {
        if (lines[i].qty <= 0) { printf("Invalid quantity for item %d.\n", lines[i].item_id); return -1; }
        int j;
        for (j = 0; j < m; j++)
            if (merged[j].item_id == lines[i].item_id) { merged[j].qty += lines[i].qty; break; }
        if (j == m) merged[m++] = lines[i];
    }

    long total_cents = 0;
    for (int i = 0; i < m; i++) {
        snprintf(sql, sizeof sql,
                 "SELECT ROUND(price * 100), is_available FROM menu WHERE menu_id = %d",
                 merged[i].item_id);
        if ((row = select_one(sql, &res)) == NULL) {
            printf("Item %d does not exist.\n", merged[i].item_id);
            return -1;
        }
        long cents = atol(row[0]);
        int available = atoi(row[1]);
        mysql_free_result(res);
        if (!available) { printf("Item %d is not available.\n", merged[i].item_id); return -1; }
        total_cents += cents * merged[i].qty;
    }

    /* database: order + its lines in one transaction */
    if (run_query("START TRANSACTION") != 0) return -1;

    snprintf(sql, sizeof sql,
             "INSERT INTO orders (user_id, priority, order_status, total_cost) "
             "VALUES (%d, %d, 'PLACED', %ld.%02ld)",
             user_id, priority, total_cents / 100, total_cents % 100);
    if (run_query(sql) != 0) { run_query("ROLLBACK"); return -1; }
    int id = (int)mysql_insert_id(conn);

    for (int i = 0; i < m; i++) {
        snprintf(sql, sizeof sql,
                 "INSERT INTO order_items (order_id, item_id, quantity) VALUES (%d, %d, %d)",
                 id, merged[i].item_id, merged[i].qty);
        if (run_query(sql) != 0) { run_query("ROLLBACK"); return -1; }
    }
    if (run_query("COMMIT") != 0) { run_query("ROLLBACK"); return -1; }

    /* priority queue: only after the database accepted the order */
    if (pq_push(id, KEY(priority, id)) != 0) {
        printf("Could not add order %d to the queue; undoing it.\n", id);
        snprintf(sql, sizeof sql, "DELETE FROM orders WHERE order_id = %d", id);
        run_query(sql);                    /* order_items go with it (ON DELETE CASCADE) */
        return -1;
    }
    return id;
}

/* ---------- delete ---------- */

int order_delete(int id)
{
    char sql[128];

    if (!order_exists(id)) { printf("Order %d not found.\n", id); return -1; }

    /* database first: fails (and keeps the order) if a bill still points to it */
    snprintf(sql, sizeof sql, "DELETE FROM orders WHERE order_id = %d", id);
    if (run_query(sql) != 0) return -1;

    pq_remove(id);      /* -1 here just means it was no longer waiting (already cooking etc.) */
    return 0;
}

/* ---------- queue restore ---------- */

int order_load_queue(void)
{
    MYSQL_RES *res;
    if (run_query("SELECT order_id, priority FROM orders WHERE order_status = 'PLACED'") != 0)
        return -1;
    res = mysql_store_result(conn);
    if (res == NULL) return -1;

    pq_clear();
    int count = 0;
    MYSQL_ROW row;
    while ((row = mysql_fetch_row(res)) != NULL) {
        int id = atoi(row[0]), pr = atoi(row[1]);
        if (pq_push(id, KEY(pr, id)) != 0) { mysql_free_result(res); return -1; }
        count++;
    }
    mysql_free_result(res);
    return count;
}

/* ---------- read ---------- */

int order_get(int id, Order *out)
{
    if (out == NULL) return -1;

    char sql[256];
    MYSQL_RES *res;
    MYSQL_ROW row;

    snprintf(sql, sizeof sql,
             "SELECT order_id, user_id, IFNULL(chef_id, 0), IFNULL(delivery_id, 0), "
             "priority, order_status, total_cost FROM orders WHERE order_id = %d", id);
    if ((row = select_one(sql, &res)) == NULL) return -1;

    int status = status_from_name(row[5]);
    if (status < 0) { mysql_free_result(res); return -1; }

    memset(out, 0, sizeof *out);
    out->id          = atoi(row[0]);
    out->user_id     = atoi(row[1]);
    out->chef_id     = atoi(row[2]);
    out->delivery_id = atoi(row[3]);
    out->priority    = atoi(row[4]);
    out->status      = (OrderStatus)status;
    out->total       = atof(row[6]);
    mysql_free_result(res);

    snprintf(sql, sizeof sql,
             "SELECT item_id, quantity FROM order_items WHERE order_id = %d "
             "ORDER BY item_id LIMIT %d", id, MAX_LINES);
    if (run_query(sql) != 0) return -1;
    res = mysql_store_result(conn);
    if (res == NULL) return -1;
    while ((row = mysql_fetch_row(res)) != NULL && out->line_count < MAX_LINES) {
        out->lines[out->line_count].item_id = atoi(row[0]);
        out->lines[out->line_count].qty     = atoi(row[1]);
        out->line_count++;
    }
    mysql_free_result(res);
    return 0;
}

void order_show(int id)
{
    Order o;
    if (order_get(id, &o) != 0) { printf("Order %d not found.\n", id); return; }

    printf("Order %d  (user %d)\n", o.id, o.user_id);
    printf("  Status  : %s\n", STATUS_NAME[o.status]);
    printf("  Priority: %d\n", o.priority);
    printf("  Chef    : "); if (o.chef_id) printf("%d\n", o.chef_id); else printf("-\n");
    printf("  Delivery: "); if (o.delivery_id) printf("%d\n", o.delivery_id); else printf("-\n");
    printf("  Items:\n");
    for (int i = 0; i < o.line_count; i++)
        printf("    item %d x %d\n", o.lines[i].item_id, o.lines[i].qty);
    printf("  Total   : %.2f\n", o.total);
}

void order_queue_show(void)
{
    pq_show();
}

/* ---------- status changes ---------- */

int order_cook_next(int chef_id)
{
    if (!staff_has_role(chef_id, "CHEF")) {
        printf("Staff %d is not a chef.\n", chef_id);
        return -1;
    }

    char sql[256];
    int id;
    while ((id = pq_pop()) != -1) {
        /* only a PLACED order may start cooking; skip stale queue entries */
        snprintf(sql, sizeof sql,
                 "UPDATE orders SET order_status = 'COOKING', chef_id = %d "
                 "WHERE order_id = %d AND order_status = 'PLACED'", chef_id, id);
        if (run_query(sql) != 0) {              /* database problem: put it back */
            Order o;
            if (order_get(id, &o) == 0 && o.status == ORDER_PLACED)
                pq_push(id, KEY(o.priority, id));
            return -1;
        }
        if (mysql_affected_rows(conn) == 1)
            return id;
    }
    return -1;
}

int order_mark_ready(int id)
{
    char sql[160];
    snprintf(sql, sizeof sql,
             "UPDATE orders SET order_status = 'READY' "
             "WHERE order_id = %d AND order_status = 'COOKING'", id);
    if (run_query(sql) != 0) return -1;
    return mysql_affected_rows(conn) == 1 ? 0 : -1;
}

int order_set_status(int id, OrderStatus s)
{
    if (s < ORDER_PLACED || s > ORDER_DELIVERED) return -1;

    Order o;
    if (order_get(id, &o) != 0) return -1;

    char sql[160];
    snprintf(sql, sizeof sql, "UPDATE orders SET order_status = '%s' WHERE order_id = %d",
             STATUS_NAME[s], id);
    if (run_query(sql) != 0) return -1;

    /* keep the queue in step: only PLACED orders wait in it */
    pq_remove(id);
    if (s == ORDER_PLACED && pq_push(id, KEY(o.priority, id)) != 0) return -1;
    return 0;
}

int order_set_delivery_staff(int id, int staff_id)
{
    if (!order_exists(id)) return -1;
    if (!staff_has_role(staff_id, "DELIVERY")) {
        printf("Staff %d is not a delivery partner.\n", staff_id);
        return -1;
    }
    char sql[160];
    snprintf(sql, sizeof sql, "UPDATE orders SET delivery_id = %d WHERE order_id = %d",
             staff_id, id);
    return run_query(sql);
}
