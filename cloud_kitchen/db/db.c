#include <stdio.h>
#include <stdlib.h>
#include <mysql/mysql.h>
#include "db.h"

MYSQL *connectDb(void) {
    const char *host = getenv("DB_HOST");
    const char *portStr = getenv("DB_PORT");
    const char *user = getenv("DB_USER");
    const char *password = getenv("DB_PASSWORD");
    const char *database = getenv("DB_NAME");

    if (!host || !portStr || !user || !password || !database) {
        if (!host) fprintf(stderr, "Missing DB_HOST\n");
        if (!portStr) fprintf(stderr, "Missing DB_PORT\n");
        if (!user) fprintf(stderr, "Missing DB_USER\n");
        if (!password) fprintf(stderr, "Missing DB_PASSWORD\n");
        if (!database) fprintf(stderr, "Missing DB_NAME\n");
        return NULL;
    }

    char *end;
    unsigned long port = strtoul(portStr, &end, 10);

    if (*portStr == '\0' || *end != '\0' || port == 0 || port > 65535) {
        fprintf(stderr, "Invalid DB_PORT\n");
        return NULL;
    }

    MYSQL *conn = mysql_init(NULL);

    if (conn == NULL) {
        fprintf(stderr, "MySQL initialization failed\n");
        return NULL;
    }

    if (mysql_real_connect(conn, host, user, password, database, (unsigned int)port, NULL, 0) == NULL) {
        fprintf(stderr, "Database connection failed: %s\n", mysql_error(conn));
        mysql_close(conn);
        return NULL;
    }

    printf("Connected to MySQL successfully\n");
    return conn;
}

void disconnectDb(MYSQL *conn) {
    if (conn != NULL)
        mysql_close(conn);
}
