#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "staff.h"

int staff_add(const char *fname, const char *lname, const char *mail, const char *phone, Role role, const char *hired, double pay)
{
    char f[201], l[201], m[511], d[41], p[64] = "NULL", q[1536];

    if (!fname || !lname || !mail || !hired || pay < 0)
    {
        printf("Missing field or negative salary.\n");
        return -1;
    }
    if (strlen(fname) > 100 || strlen(lname) > 100 || strlen(mail) > 255 || strlen(hired) > 20 || (phone && strlen(phone) > 20))
    {
        printf("A field is too long.\n");
        return -1;
    }

    //escape the text so quotes cannot break the query
    mysql_real_escape_string(conn, f, fname, strlen(fname));
    mysql_real_escape_string(conn, l, lname, strlen(lname));
    mysql_real_escape_string(conn, m, mail, strlen(mail));
    mysql_real_escape_string(conn, d, hired, strlen(hired));

    if (phone)
    {
        char pe[41];
        mysql_real_escape_string(conn, pe, phone, strlen(phone));
        snprintf(p, sizeof p, "'%s'", pe);
    }
    snprintf(q, sizeof q, "INSERT INTO staff (f_name, l_name, email, contact_number, role, hiring_date, salary) VALUES ('%s', '%s', '%s', %s, '%s', '%s', %.2f)", f, l, m, p, role == ROLE_CHEF ? "CHEF" : "DELIVERY", d, pay);
    if (mysql_query(conn, q))
    {
        printf("Database error: %s\n", mysql_error(conn));
        return -1;
    }
    return (int) mysql_insert_id(conn);
}

int staff_get(int id, Staff *out)
{
    char q[160];
    snprintf(q, sizeof q, "SELECT staff_id, CONCAT_WS(' ', f_name, NULLIF(l_name, '')), role FROM staff WHERE staff_id = %d", id);

    if (mysql_query(conn, q))
        return -1;
    MYSQL_RES *res = mysql_store_result(conn);
    if (!res) return -1;

    MYSQL_ROW row = mysql_fetch_row(res);
    if (!row)
    {
        mysql_free_result(res);
        return -1;
    }

    out->id = atoi(row[0]);
    snprintf(out->name, MAX_NAME, "%s", row[1]);
    out->role = strcmp(row[2], "CHEF") == 0 ? ROLE_CHEF : ROLE_DELIVERY;
    mysql_free_result(res);
    return 0;
}

void staff_list(void)
{
    if (mysql_query(conn, "SELECT staff_id, CONCAT_WS(' ', f_name, NULLIF(l_name, '')), role FROM staff ORDER BY staff_id"))
    {
        printf("Database error: %s\n", mysql_error(conn));
        return;
    }

    MYSQL_RES *res = mysql_store_result(conn);
    if (!res) return;

    MYSQL_ROW row;
    while ((row = mysql_fetch_row(res)))
        printf("%s  %s  %s\n", row[0], row[1], row[2]);
    mysql_free_result(res);
}
