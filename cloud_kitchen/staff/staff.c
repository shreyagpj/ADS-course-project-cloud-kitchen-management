#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "staff.h"

int staff_add(const char *fName, const char *lName, const char *email, const char *contactNumber, Role role, const char *hiringDate, double salary) {
    char f[201], l[201], e[511], d[41], c[64] = "NULL", query[1536];
    mysql_real_escape_string(conn, f, fName, strlen(fName));
    mysql_real_escape_string(conn, l, lName, strlen(lName));
    mysql_real_escape_string(conn, e, email, strlen(email));
    mysql_real_escape_string(conn, d, hiringDate, strlen(hiringDate));

    if (contactNumber) {
        char escaped[41];
        mysql_real_escape_string(conn, escaped, contactNumber, strlen(contactNumber));
        snprintf(c, sizeof c, "'%s'", escaped);
    }
    snprintf(query, sizeof query, "INSERT INTO staff (f_name, l_name, email, contact_number, role, hiring_date, salary) VALUES ('%s', '%s', '%s', %s, '%s', '%s', %.2f)", f, l, e, c, role == ROLE_CHEF ? "CHEF" : "DELIVERY", d, salary);
    if (mysql_query(conn, query))
        return -1;
    return (int)mysql_insert_id(conn);
}

int staff_get(int id, Staff *out) {
    char query[160];
    snprintf(query, sizeof query, "SELECT staff_id, CONCAT_WS(' ', f_name, NULLIF(l_name, '')), role FROM staff WHERE staff_id = %d", id);
    if (mysql_query(conn, query))
        return -1;
    MYSQL_RES *res = mysql_store_result(conn);
    MYSQL_ROW row = mysql_fetch_row(res);
    if (!row) {
        mysql_free_result(res);
        return -1;
    }
    
    out->id = atoi(row[0]);
    snprintf(out->name, MAX_NAME, "%s", row[1]);
    out->role = strcmp(row[2], "CHEF") == 0 ? ROLE_CHEF : ROLE_DELIVERY;
    mysql_free_result(res);
    return 0;
}

void staff_list(void) {
    if (mysql_query(conn, "SELECT staff_id, CONCAT_WS(' ', f_name, NULLIF(l_name, '')), role FROM staff ORDER BY staff_id"))
        return;
    MYSQL_RES *res = mysql_store_result(conn);
    MYSQL_ROW row;
    while ((row = mysql_fetch_row(res)))
        printf("%s  %s  %s\n", row[0], row[1], row[2]);
    mysql_free_result(res);
}
