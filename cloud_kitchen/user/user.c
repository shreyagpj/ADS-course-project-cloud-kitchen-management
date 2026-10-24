#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "user.h"

int user_add(const char *fName, const char *lName, const char *email, const char *dob, const char *contact, const char *houseNo, const char *socName, const char *street, GeoPoint loc, const char *city, const char *state) {
    const char *in[] = {fName, lName, email, dob, contact, houseNo, socName, street, city, state};
    char v[10][520], query[8192];
    for (int i = 0; i < 10; i++) {
        if (in[i] && in[i][0]) {
            size_t n = mysql_real_escape_string(conn, v[i] + 1, in[i], strlen(in[i]));
            v[i][0] = '\'';
            v[i][n + 1] = '\'';
            v[i][n + 2] = '\0';
        }
        else
            strcpy(v[i], "NULL");
    }

    snprintf(query, sizeof query, "INSERT INTO users (f_name, l_name, email, date_of_birth, contact_number, house_no, soc_name, street, latitude, longitude, city, state) VALUES (%s, %s, %s, %s, %s, %s, %s, %s, %.7f, %.7f, %s, %s)", v[0], v[1], v[2], v[3], v[4], v[5], v[6], v[7], loc.lat, loc.lon, v[8], v[9]);

    if (mysql_query(conn, query))
        return -1;
    return (int)mysql_insert_id(conn);
}

int user_get(int id, User *out) {
    char query[256];
    snprintf(query, sizeof query, "SELECT user_id, CONCAT_WS(' ', f_name, NULLIF(l_name, '')), contact_number, latitude, longitude FROM users WHERE user_id = %d", id);

    if (mysql_query(conn, query))
        return -1;
    MYSQL_RES *res = mysql_store_result(conn);
    MYSQL_ROW row = mysql_fetch_row(res);
    if (!row) {
        mysql_free_result(res);
        return -1;
    }

    out->id = atoi(row[0]);
    snprintf(out->name, sizeof out->name, "%s", row[1]);
    snprintf(out->phone, sizeof out->phone, "%s", row[2] ? row[2] : "");
    out->location.lat = row[3] ? atof(row[3]) : 0;
    out->location.lon = row[4] ? atof(row[4]) : 0;
    mysql_free_result(res);
    return 0;
}

void user_list(void) {
    if (mysql_query(conn, "SELECT user_id, CONCAT_WS(' ', f_name, NULLIF(l_name, '')), email, contact_number FROM users ORDER BY user_id"))
        return;

    MYSQL_RES *res = mysql_store_result(conn);
    MYSQL_ROW row;
    while ((row = mysql_fetch_row(res)))
        printf("%s  %s  %s  %s\n", row[0], row[1], row[2], row[3] ? row[3] : "-");
    mysql_free_result(res);
}
