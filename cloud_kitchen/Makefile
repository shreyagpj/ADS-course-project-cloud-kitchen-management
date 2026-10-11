CC     = gcc
CFLAGS = -Wall -Wextra -std=c11 -g
SRCS   = main.c user/user.c staff/staff.c menu/menu.c orders/orders.c orders/pq.c \
         delivery/delivery.c delivery/kdtree.c billing/billing.c db/db.c
LDLIBS = -lm -lmysqlclient

cloud_kitchen: $(SRCS)
	$(CC) $(CFLAGS) -o $@ $(SRCS) $(LDLIBS)

run: cloud_kitchen
	set -a; . ../.env; set +a; ./cloud_kitchen

clean:
	rm -f cloud_kitchen
