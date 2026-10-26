#ifndef DB_H
#define DB_H

#include <mysql/mysql.h>

MYSQL *db_open(void);
void db_close(MYSQL *m);

#endif
