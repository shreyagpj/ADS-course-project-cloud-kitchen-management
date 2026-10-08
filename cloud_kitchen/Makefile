CC     = gcc
CFLAGS = -Wall -Wextra -std=c11 -g
SRCS   = main.c user/user.c staff/staff.c menu/menu.c orders/orders.c orders/pq.c \
         delivery/delivery.c delivery/kdtree.c billing/billing.c

cloud_kitchen: $(SRCS)
	$(CC) $(CFLAGS) -o $@ $(SRCS) -lm

run: cloud_kitchen
	./cloud_kitchen

clean:
	rm -f cloud_kitchen
