#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "common.h"
#include "user/user.h"
#include "staff/staff.h"
#include "menu/menu.h"
#include "orders/orders.h"
#include "delivery/delivery.h"
#include "billing/billing.h"
#include "db/db.h"
#include "tries/trie.h"

#define NC ((int)(sizeof C / sizeof *C))

MYSQL *conn;

typedef struct {
    const char *n;
    void (*f)(void);
} Cmd;

static int run = 1;

static void ask_text(const char *p, char *b, int sz) {
    printf("%s", p);
    if (!fgets(b, sz, stdin)) {
        disconnectDb(conn);
        exit(0);
    }
    if (!strchr(b, '\n')) {
        int c;
        while ((c = getchar()) != '\n' && c != EOF);
    }
    b[strcspn(b, "\n")] = '\0';
}

static int ask_int(const char *p) {
    char b[32], *e;
    ask_text(p, b, sizeof b);
    long v = strtol(b, &e, 10);
    return (e == b || *e || v < 0 || v > 2000000000L) ? -1 : (int)v;
}

static int ask_dbl(const char *p, double *o) {
    char b[32], *e;
    ask_text(p, b, sizeof b);
    double v = strtod(b, &e);
    if (e == b || *e || !isfinite(v)) {
        printf("Invalid number.\n");
        return -1;
    }
    *o = v;
    return 0;
}

static int ask_loc(GeoPoint *g) {
    if (ask_dbl("  Latitude : ", &g->lat) || ask_dbl("  Longitude: ", &g->lon))
        return -1;
    if (g->lat < -90 || g->lat > 90 || g->lon < -180 || g->lon > 180) {
        printf("Coordinates out of range.\n");
        return -1;
    }
    return 0;
}

static void report(int id, const char *w) {
    if (id > 0)
        printf("%s created with ID %d\n", w, id);
    else
        printf("Could not create %s.\n", w);
}

static void ok(int r, const char *good) {
    printf("%s\n", r == 0 ? good : "Failed.");
}

static void c_uadd(void) {
    char fn[101], ln[101], em[256], dob[16], ph[21], hn[21], sn[151], st[151], ct[101], sa[101];
    GeoPoint g;
    ask_text("First name: ", fn, sizeof fn);
    ask_text("Last name: ", ln, sizeof ln);
    ask_text("Email: ", em, sizeof em);
    ask_text("Date of birth (YYYY-MM-DD, blank to skip): ", dob, sizeof dob);
    ask_text("Contact number (blank to skip): ", ph, sizeof ph);
    ask_text("House no (blank to skip): ", hn, sizeof hn);
    ask_text("Society name (blank to skip): ", sn, sizeof sn);
    ask_text("Street (blank to skip): ", st, sizeof st);
    ask_text("City (blank to skip): ", ct, sizeof ct);
    ask_text("State (blank to skip): ", sa, sizeof sa);
    printf("Delivery location:\n");
    if (ask_loc(&g))
        return;
    report(user_add(fn, ln, em, dob, ph, hn, sn, st, g, ct, sa), "User");
}

static void c_ulist(void) {
    user_list();
}

static void c_sadd(void) {
    char fn[101], ln[101], em[256], ph[21], dt[16];
    double sal;
    GeoPoint g;
    ask_text("First name: ", fn, sizeof fn);
    ask_text("Last name: ", ln, sizeof ln);
    ask_text("Email: ", em, sizeof em);
    ask_text("Contact number (blank to skip): ", ph, sizeof ph);
    int r = ask_int("Role (1=Chef, 2=Delivery partner): ");
    if (r != ROLE_CHEF && r != ROLE_DELIVERY) {
        printf("Invalid role.\n");
        return;
    }
    ask_text("Hiring date (YYYY-MM-DD): ", dt, sizeof dt);
    if (ask_dbl("Salary: ", &sal))
        return;
    if (r == ROLE_DELIVERY) {
        printf("Partner's current location:\n");
        if (ask_loc(&g))
            return;
    }
    int id = staff_add(fn, ln, em, ph[0] ? ph : NULL, (Role)r, dt, sal);
    report(id, "Staff");
    if (id > 0 && r == ROLE_DELIVERY && delivery_add_partner(id, g) != 0)
        printf("Staff saved, but the partner location was not.\n");
}

static void c_slist(void) {
    staff_list();
}

static void c_iadd(void) {
    char n[MAX_NAME];
    double p;
    ask_text("Item name: ", n, sizeof n);
    if (ask_dbl("Price: ", &p))
        return;
    report(item_add(n, p), "Item");
}

static void c_ilist(void) {
    item_list();
}

static void c_itog(void) {
    int i = ask_int("Item ID: ");
    int a = ask_int("Available (1=yes, 0=no): ");
    if (a != 0 && a != 1) {
        printf("Invalid choice.\n");
        return;
    }
    ok(item_set_available(i, a), "Item updated.");
}

static void c_iprice(void) {
    double p;
    int i = ask_int("Item ID: ");
    if (ask_dbl("New price: ", &p))
        return;
    ok(item_set_price(i, p), "Price updated.");
}

static void c_oplace(void) {
    OrderLine l[MAX_LINES];
    int u = ask_int("User ID: "), n = 0;
    while (n < MAX_LINES) {
        int i = ask_int("  Item ID (0 = done): ");
        if (i == 0)
            break;
        int q = ask_int("  Quantity: ");
        if (i < 0 || q <= 0) {
            printf("  Invalid.\n");
            continue;
        }
        l[n].item_id = i;
        l[n].qty = q;
        n++;
    }
    if (n == 0) {
        printf("Empty order discarded.\n");
        return;
    }
    int p = ask_int("Priority (1=urgent, 2=normal, 3=low): ");
    if (p < 1 || p > 3)
        p = 2;
    report(order_place(u, l, n, p), "Order");
}

static void c_oview(void) {
    order_show(ask_int("Order ID: "));
}

static void c_oqueue(void) {
    order_queue_show();
}

static void c_ocook(void) {
    int c = ask_int("Chef staff ID: ");
    int o = order_cook_next(c);
    if (o > 0)
        printf("Chef %d is cooking order %d\n", c, o);
    else
        printf("No order to cook.\n");
}

static void c_oready(void) {
    ok(order_mark_ready(ask_int("Order ID: ")), "Order is ready.");
}

static void c_odel(void) {
    ok(order_delete(ask_int("Order ID: ")), "Order deleted.");
}

static void c_dassign(void) {
    int o = ask_int("Order ID: ");
    int s = delivery_assign(o);
    if (s > 0)
        printf("Order %d -> delivery partner %d (nearest free)\n", o, s);
    else
        printf("No free partner, or order not ready.\n");
}

static void c_ddone(void) {
    ok(delivery_complete(ask_int("Order ID: ")), "Delivered.");
}

static void c_bgen(void) {
    report(bill_generate(ask_int("Order ID: ")), "Bill");
}

static void c_bpay(void) {
    static const char *m[] = { "CASH", "CARD", "UPI", "NET_BANKING" };
    int b = ask_int("Bill ID: ");
    int k = ask_int("Method (1=Cash, 2=Card, 3=UPI, 4=Net banking): ");
    if (k < 1 || k > 4) {
        printf("Invalid method.\n");
        return;
    }
    ok(bill_pay(b, m[k - 1]), "Paid.");
}

static void c_bview(void) {
    bill_show(ask_int("Bill ID: "));
}

static void c_bdel(void) {
    ok(bill_delete(ask_int("Bill ID: ")), "Bill deleted.");
}

static void c_help(void);

static void c_exit(void) {
    run = 0;
}

static const Cmd C[] = {
    {"users add", c_uadd}, {"users list", c_ulist},
    {"staff add", c_sadd}, {"staff list", c_slist},
    {"items add", c_iadd}, {"items list", c_ilist},
    {"items available", c_itog}, {"items price", c_iprice},
    {"orders place", c_oplace}, {"orders view", c_oview}, {"orders queue", c_oqueue},
    {"orders cook", c_ocook}, {"orders ready", c_oready}, {"orders delete", c_odel},
    {"delivery assign", c_dassign}, {"delivery complete", c_ddone},
    {"billing generate", c_bgen}, {"billing pay", c_bpay}, {"billing view", c_bview},
    {"billing delete", c_bdel}, {"help", c_help}, {"exit", c_exit}
};

_Static_assert(sizeof C / sizeof *C <= MAXL, "trie holds at most MAXL commands");

static void c_help(void) {
    printf("Commands (Tab completes, Right arrow accepts the grey suggestion):\n");
    for (int i = 0; i < NC; i++)
        printf("  %s\n", C[i].n);
}

static void trim(char *s) {
    char *a = s;
    while (*a == ' ')
        a++;
    memmove(s, a, strlen(a) + 1);
    size_t n = strlen(s);
    while (n > 0 && s[n - 1] == ' ')
        s[--n] = '\0';
}

int main(void) {
    conn = connectDb();
    if (conn == NULL)
        return 1;

    int w = order_load_queue();
    if (w < 0)
        printf("Warning: could not load waiting orders.\n");
    else if (w > 0)
        printf("%d order(s) waiting in the kitchen queue.\n", w);

    TNode *t = trie_new();
    if (t == NULL) {
        disconnectDb(conn);
        return 1;
    }
    for (int i = 0; i < NC; i++)
        trie_add(t, C[i].n);

    char b[MAXC];
    printf("Type 'help' for the command list.\n");

    while (run) {
        ask_text("kitchen> ", b, sizeof b);
        trim(b);
        if (!b[0])
            continue;

        int i = 0;
        while (i < NC && strcmp(C[i].n, b) != 0)
            i++;
        if (i < NC) {
            trie_use(t, b);
            C[i].f();
            continue;
        }
        Word s[MAXL];
        int k = trie_list(t, b, s, MAXL);
        if (k == 0) {
            printf("Unknown command '%s'.\n", b);
            continue;
        }
        printf("Did you mean:\n");
        for (int j = 0; j < k; j++)
            printf("  %s\n", s[j]);
    }

    trie_free(t);
    disconnectDb(conn);
    return 0;
}
