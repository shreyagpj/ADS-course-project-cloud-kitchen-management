#ifndef DB_H
#define DB_H

#include <mysql/mysql.h>

MYSQL *connectDb(void);
void disconnectDb(MYSQL *m);

#endif
