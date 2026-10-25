#include <stdio.h>
#include <stdlib.h>
#include <mysql/mysql.h>
#include "db.h"

MYSQL *connectDb(void) {
    const char *host = getenv("DB_HOST");
    const char *ps = getenv("DB_PORT");
    const char *user = getenv("DB_USER");
    const char *pw = getenv("DB_PASSWORD");
    const char *dn = getenv("DB_NAME");

    if (!host || !ps || !user || !pw || !dn) {
        if (!host) fprintf(stderr, "Missing DB_HOST\n");
        if (!ps) fprintf(stderr, "Missing DB_PORT\n");
        if (!user) fprintf(stderr, "Missing DB_USER\n");
        if (!pw) fprintf(stderr, "Missing DB_PASSWORD\n");
        if (!dn) fprintf(stderr, "Missing DB_NAME\n");
        return NULL;
    }

    char *end;
    unsigned long port = strtoul(ps, &end, 10);

    if (*ps == '\0' || *end != '\0' || port == 0 || port > 65535) {
        fprintf(stderr, "Invalid DB_PORT\n");
        return NULL;
    }

    MYSQL *m = mysql_init(NULL);

    if (m == NULL) {
        fprintf(stderr, "MySQL initialization failed\n");
        return NULL;
    }

    if (mysql_real_connect(m, host, user, pw, dn, (unsigned int)port, NULL, 0) == NULL) {
        fprintf(stderr, "Database connection failed: %s\n", mysql_error(m));
        mysql_close(m);
        return NULL;
    }

    printf("Connected to MySQL successfully\n");
    return m;
}

void disconnectDb(MYSQL *m) {
    if (m != NULL)
        mysql_close(m);
}
