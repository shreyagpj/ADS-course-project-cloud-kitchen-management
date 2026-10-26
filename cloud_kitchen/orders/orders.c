#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../common.h"
#include "orders.h"
#include "pq.h"

static const char *STATUS[] = { "PLACED", "COOKING", "READY", "OUT_FOR_DELIVERY", "DELIVERED" };

static int query(const char *q)
{
    if (conn == NULL)
    {
        fprintf(stderr, "No database connection.\n");
        return -1;
    }
    if (mysql_query(conn, q) != 0)
    {
        fprintf(stderr, "Database error: %s\n", mysql_error(conn));
        return -1;
    }
    return 0;
}

//gives the first row or null and the caller frees res
static MYSQL_ROW sel_one(const char *q, MYSQL_RES **res)
{
    *res = NULL;
    if (query(q) != 0) return NULL;
    *res = mysql_store_result(conn);
    if (*res == NULL) return NULL;

    MYSQL_ROW row = mysql_fetch_row(*res);
    if (row == NULL)
    {
        mysql_free_result(*res);
        *res = NULL;
    }
    return row;
}

static int status_id(const char *name)
{
    for (int i = 0; i < (int) (sizeof STATUS / sizeof *STATUS); i++)
    {
        if (strcmp(name, STATUS[i]) == 0)
            return i;
    }
    return -1;
}

static int has_order(int id)
{
    char q[128];
    MYSQL_RES *res;
    snprintf(q, sizeof q, "SELECT 1 FROM orders WHERE order_id = %d", id);
    if (sel_one(q, &res) == NULL) return 0;
    mysql_free_result(res);
    return 1;
}

static int has_role(int sid, const char *role)
{
    char q[160];
    MYSQL_RES *res;
    snprintf(q, sizeof q, "SELECT 1 FROM staff WHERE staff_id = %d AND role = '%s'", sid, role);
    if (sel_one(q, &res) == NULL) return 0;
    mysql_free_result(res);
    return 1;
}

int order_place(int uid, const OrderLine lines[], int n, int pr)
{
    if (lines == NULL || n <= 0 || n > MAX_LINES)
    {
        printf("An order needs between 1 and %d items.\n", MAX_LINES);
        return -1;
    }
    if (pr < 1 || pr > 3)
    {
        printf("Priority must be 1 (urgent), 2 (normal) or 3 (low).\n");
        return -1;
    }

    char q[512];
    MYSQL_RES *res;
    MYSQL_ROW row;

    snprintf(q, sizeof q, "SELECT 1 FROM users WHERE user_id = %d", uid);
    if (sel_one(q, &res) == NULL)
    {
        printf("User %d does not exist.\n", uid);
        return -1;
    }
    mysql_free_result(res);

    //same items are merged because the table has one row for each item
    OrderLine mg[MAX_LINES];
    int m = 0;
    for (int i = 0; i < n; i++)
    {
        if (lines[i].qty <= 0)
        {
            printf("Invalid quantity for item %d.\n", lines[i].item_id);
            return -1;
        }
        int j;
        for (j = 0; j < m; j++)
        {
            if (mg[j].item_id == lines[i].item_id)
            {
                mg[j].qty += lines[i].qty;
                break;
            }
        }
        if (j == m) mg[m++] = lines[i];
    }

    //money is counted in cents to avoid rounding errors
    long tot = 0;
    for (int i = 0; i < m; i++)
    {
        snprintf(q, sizeof q, "SELECT ROUND(price * 100), is_available FROM menu WHERE menu_id = %d", mg[i].item_id);
        if ((row = sel_one(q, &res)) == NULL)
        {
            printf("Item %d does not exist.\n", mg[i].item_id);
            return -1;
        }
        long cents = atol(row[0]);
        int av = atoi(row[1]);
        mysql_free_result(res);
        if (!av)
        {
            printf("Item %d is not available.\n", mg[i].item_id);
            return -1;
        }
        tot += cents * mg[i].qty;
    }

    if (query("START TRANSACTION") != 0) return -1;

    snprintf(q, sizeof q, "INSERT INTO orders (user_id, priority, order_status, total_cost) VALUES (%d, %d, 'PLACED', %ld.%02ld)", uid, pr, tot / 100, tot % 100);
    if (query(q) != 0)
    {
        query("ROLLBACK");
        return -1;
    }
    int id = (int) mysql_insert_id(conn);

    for (int i = 0; i < m; i++)
    {
        snprintf(q, sizeof q, "INSERT INTO order_items (order_id, item_id, quantity) VALUES (%d, %d, %d)", id, mg[i].item_id, mg[i].qty);
        if (query(q) != 0)
        {
            query("ROLLBACK");
            return -1;
        }
    }
    if (query("COMMIT") != 0)
    {
        query("ROLLBACK");
        return -1;
    }

    //queue only after the db saved the order
    if (pq_push(id, PQ_KEY(pr, id)) != 0)
    {
        printf("Could not add order %d to the queue; undoing it.\n", id);
        snprintf(q, sizeof q, "DELETE FROM orders WHERE order_id = %d", id);
        query(q);
        return -1;
    }
    return id;
}

int order_delete(int id)
{
    char q[128];

    if (!has_order(id))
    {
        printf("Order %d not found.\n", id);
        return -1;
    }

    snprintf(q, sizeof q, "DELETE FROM orders WHERE order_id = %d AND order_status = 'PLACED'", id);
    if (query(q) != 0) return -1;
    if (mysql_affected_rows(conn) != 1)
    {
        printf("Order %d is already being prepared.\n", id);
        return -1;
    }

    pq_del(id);
    return 0;
}

int order_load(void)
{
    MYSQL_RES *res;
    if (query("SELECT order_id, priority FROM orders WHERE order_status = 'PLACED'") != 0)
        return -1;
    res = mysql_store_result(conn);
    if (res == NULL) return -1;

    pq_clear();
    int count = 0;
    MYSQL_ROW row;
    while ((row = mysql_fetch_row(res)) != NULL)
    {
        int id = atoi(row[0]), pr = atoi(row[1]);
        if (pq_push(id, PQ_KEY(pr, id)) != 0)
        {
            mysql_free_result(res);
            return -1;
        }
        count++;
    }
    mysql_free_result(res);
    return count;
}

int order_get(int id, Order *out)
{
    char q[256];
    MYSQL_RES *res;
    MYSQL_ROW row;

    snprintf(q, sizeof q, "SELECT order_id, user_id, IFNULL(chef_id, 0), IFNULL(delivery_id, 0), priority, order_status, total_cost FROM orders WHERE order_id = %d", id);
    if ((row = sel_one(q, &res)) == NULL) return -1;

    int st = status_id(row[5]);
    if (st < 0)
    {
        mysql_free_result(res);
        return -1;
    }

    memset(out, 0, sizeof *out);
    out->id = atoi(row[0]);
    out->user_id = atoi(row[1]);
    out->chef_id = atoi(row[2]);
    out->dstaff = atoi(row[3]);
    out->priority = atoi(row[4]);
    out->status = (OrderStatus) st;
    out->total = atof(row[6]);
    mysql_free_result(res);

    snprintf(q, sizeof q, "SELECT item_id, quantity FROM order_items WHERE order_id = %d ORDER BY item_id LIMIT %d", id, MAX_LINES);
    if (query(q) != 0) return -1;
    res = mysql_store_result(conn);
    if (res == NULL) return -1;
    while ((row = mysql_fetch_row(res)) != NULL && out->line_count < MAX_LINES)
    {
        out->lines[out->line_count].item_id = atoi(row[0]);
        out->lines[out->line_count].qty = atoi(row[1]);
        out->line_count++;
    }
    mysql_free_result(res);
    return 0;
}

void order_show(int id)
{
    Order o;
    if (order_get(id, &o) != 0)
    {
        printf("Order %d not found.\n", id);
        return;
    }

    printf("Order %d  (user %d)\n", o.id, o.user_id);
    printf("  Status  : %s\n", STATUS[o.status]);
    printf("  Priority: %d\n", o.priority);
    printf("  Chef    : ");
    if (o.chef_id) printf("%d\n", o.chef_id);
    else           printf("-\n");
    printf("  Delivery: ");
    if (o.dstaff) printf("%d\n", o.dstaff);
    else          printf("-\n");
    printf("  Items:\n");
    for (int i = 0; i < o.line_count; i++)
        printf("    item %d x %d\n", o.lines[i].item_id, o.lines[i].qty);
    printf("  Total   : %.2f\n", o.total);
}

void order_queue(void)
{
    pq_show();
}

int order_cook(int chef)
{
    if (!has_role(chef, "CHEF"))
    {
        printf("Staff %d is not a chef.\n", chef);
        return -1;
    }

    char q[256];
    int id;
    while ((id = pq_pop()) != -1)
    {
        //old queue entries that are not placed any more are skipped
        snprintf(q, sizeof q, "UPDATE orders SET order_status = 'COOKING', chef_id = %d WHERE order_id = %d AND order_status = 'PLACED'", chef, id);
        if (query(q) != 0)
        {
            Order o;
            if (order_get(id, &o) == 0 && o.status == ORDER_PLACED)
                pq_push(id, PQ_KEY(o.priority, id));
            return -1;
        }
        if (mysql_affected_rows(conn) == 1)
            return id;
    }
    return -1;
}

int order_ready(int id)
{
    char q[160];
    snprintf(q, sizeof q, "UPDATE orders SET order_status = 'READY' WHERE order_id = %d AND order_status = 'COOKING'", id);
    if (query(q) != 0) return -1;
    return mysql_affected_rows(conn) == 1 ? 0 : -1;
}

int order_set_state(int id, OrderStatus s)
{
    if (s < ORDER_PLACED || s > ORDER_DELIVERED) return -1;

    Order o;
    if (order_get(id, &o) != 0) return -1;

    if (s != o.status + 1 && !(s == ORDER_PLACED && o.status == ORDER_COOKING))
    {
        printf("Cannot change order %d from %s to %s.\n", id, STATUS[o.status], STATUS[s]);
        return -1;
    }

    char q[160];
    snprintf(q, sizeof q, "UPDATE orders SET order_status = '%s'%s WHERE order_id = %d", STATUS[s], s == ORDER_PLACED ? ", chef_id = NULL" : "", id);
    if (query(q) != 0) return -1;

    pq_del(id);
    if (s == ORDER_PLACED && pq_push(id, PQ_KEY(o.priority, id)) != 0) return -1;
    return 0;
}

int order_set_driver(int id, int sid)
{
    if (!has_order(id)) return -1;
    if (!has_role(sid, "DELIVERY"))
    {
        printf("Staff %d is not a delivery partner.\n", sid);
        return -1;
    }

    char q[160];
    snprintf(q, sizeof q, "UPDATE orders SET delivery_id = %d WHERE order_id = %d", sid, id);
    if (query(q) != 0) return -1;
    return 0;
}
