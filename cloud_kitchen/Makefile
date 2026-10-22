CC     = gcc
CFLAGS = -Wall -Wextra -std=c11 -g
SRCS   = main.c user/user.c staff/staff.c menu/menu.c orders/orders.c orders/pq.c \
         delivery/delivery.c delivery/kdtree.c billing/billing.c db/db.c
LDLIBS = -lm -lmysqlclient

cloud_kitchen: $(SRCS)
	$(CC) $(CFLAGS) -o $@ $(SRCS) $(LDLIBS)

run: cloud_kitchen
	set -a; . ../.env; set +a; ./cloud_kitchen

dbinit:
	set -a; . ../.env; set +a; \
	cat ../dbinit.sql menu/menu_schema.sql orders/orders_schema.sql billing/billing_schema.sql user/user_schema.sql staff/staff_schema.sql delivery/delivery_schema.sql \
	../insertion_script.sql | \
	mysql -h "$$DB_HOST" -P "$$DB_PORT" -u "$$DB_USER" -p

clean:
	rm -f cloud_kitchen
