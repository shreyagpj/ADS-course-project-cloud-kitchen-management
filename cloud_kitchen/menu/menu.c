#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "menu.h"
#include "../common.h"
#include "../db/db.h"

extern MYSQL *conn;

int item_add(const char *name, double price)
{
    if (name == NULL || name[0] == '\0') {
        printf("Item name cannot be empty.\n");
        return -1;
    }
    if (price < 0) {
        printf("Price cannot be negative.\n");
        return -1;
    }

    /* escape the name so quotes in it cannot break the query */
    size_t len = strlen(name);
    char *escaped = (char *)malloc(len * 2 + 1);
    if (escaped == NULL) {
        printf("Out of memory.\n");
        return -1;
    }
    mysql_real_escape_string(conn, escaped, name, (unsigned long)len);

    size_t qsize = len * 2 + 128;
    char *query = (char *)malloc(qsize);
    if (query == NULL) {
        free(escaped);
        printf("Out of memory.\n");
        return -1;
    }
    snprintf(query, qsize, "INSERT INTO menu (name, price) VALUES ('%s', %.2f)",
             escaped, price);

    int id = -1;
    if (mysql_query(conn, query)) {
        if (mysql_errno(conn) == 1062)          /* duplicate entry */
            printf("An item named '%s' already exists.\n", name);
        else
            printf("Database error: %s\n", mysql_error(conn));
    } else {
        id = (int)mysql_insert_id(conn);
    }

    free(query);
    free(escaped);
    return id;
}

void item_list(void)
{
    if (mysql_query(conn,
            "SELECT menu_id, name, price, is_available FROM menu ORDER BY menu_id")) {
        printf("Database error: %s\n", mysql_error(conn));
        return;
    }
    MYSQL_RES *res = mysql_store_result(conn);
    if (res == NULL) {
        printf("Database error: %s\n", mysql_error(conn));
        return;
    }

    MYSQL_ROW row = mysql_fetch_row(res);
    if (row == NULL) {
        printf("Menu is empty.\n");
        mysql_free_result(res);
        return;
    }

    printf("\n%-5s %-30s %9s  %s\n", "ID", "Name", "Price", "Available");
    do {
        printf("%-5s %-30s %9s  %s\n", row[0], row[1], row[2],
               atoi(row[3]) ? "Yes" : "No");
    } while ((row = mysql_fetch_row(res)) != NULL);

    mysql_free_result(res);
}