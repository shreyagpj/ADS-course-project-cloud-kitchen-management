CC     = gcc
CFLAGS = -Wall -Wextra -std=c11 -D_POSIX_C_SOURCE=200809L -g
SRCS   = main.c user/user.c staff/staff.c menu/menu.c orders/orders.c orders/pq.c \
         delivery/delivery.c delivery/kdtree.c billing/billing.c db/db.c tries/trie.c
HDRS   = $(wildcard *.h */*.h)
LDLIBS = -lm -lmysqlclient

.PHONY: run dbinit clean

cloud_kitchen: $(SRCS) $(HDRS)
	$(CC) $(CFLAGS) -o $@ $(SRCS) $(LDLIBS)

run: cloud_kitchen
	set -a; . ../.env; set +a; ./cloud_kitchen

dbinit:
	set -a; . ../.env; set +a; \
	cat ../dbinit.sql menu/menu_schema.sql user/user_schema.sql staff/staff_schema.sql \
	orders/orders_schema.sql billing/billing_schema.sql delivery/delivery_schema.sql \
	../insertion_script.sql | \
	MYSQL_PWD="$$DB_PASSWORD" mysql -h "$$DB_HOST" -P "$$DB_PORT" -u "$$DB_USER"

clean:
	rm -f cloud_kitchen
