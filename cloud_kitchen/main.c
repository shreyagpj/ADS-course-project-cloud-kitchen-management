//main.c reads input and calls module functions
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

#define NCMD ((int)(sizeof cmds / sizeof *cmds))

MYSQL *conn;

typedef struct
{
    const char *name;
    void (*run)(void);
} Cmd;

static int on = 1;

//cuts the newline and quits when the input ends
static void ask_text(const char *prompt, char *buf, int size)
{
    printf("%s", prompt);
    if (!fgets(buf, size, stdin))
    {
        printf("\nBye!\n");
        db_close(conn);
        exit(0);
    }
    if (!strchr(buf, '\n'))
    {
        int ch;
        while ((ch = getchar()) != '\n' && ch != EOF);
    }
    buf[strcspn(buf, "\n")] = '\0';
}

//minus one means bad input
static int ask_int(const char *prompt)
{
    char b[32], *end;
    ask_text(prompt, b, sizeof b);
    long v = strtol(b, &end, 10);
    return (end == b || *end || v < 0 || v > 2000000000L) ? -1 : (int) v;
}

static int ask_id(const char *prompt)
{
    int id = ask_int(prompt);
    if (id < 0) printf("Invalid number.\n");
    return id;
}

static int ask_dbl(const char *prompt, double *out)
{
    char b[32], *end;
    ask_text(prompt, b, sizeof b);
    double v = strtod(b, &end);
    if (end == b || *end || !isfinite(v))
    {
        printf("Invalid number.\n");
        return -1;
    }
    *out = v;
    return 0;
}

static int ask_loc(GeoPoint *g)
{
    if (ask_dbl("  Latitude : ", &g->lat) || ask_dbl("  Longitude: ", &g->lon))
        return -1;
    if (g->lat < -90 || g->lat > 90 || g->lon < -180 || g->lon > 180)
    {
        printf("Coordinates out of range.\n");
        return -1;
    }
    return 0;
}

//add calls give an id and the rest give 0 when ok
static void report(int id, const char *what)
{
    if (id > 0) printf("%s created with ID %d\n", what, id);
    else        printf("Could not create %s.\n", what);
}

static void show_ok(int r, const char *good)
{
    printf("%s\n", r == 0 ? good : "Failed.");
}

static void add_user(void)
{
    char fn[101], ln[101], mail[256], dob[16], ph[21];
    char house[21], soc[151], street[151], city[101], state[101];
    GeoPoint loc;

    ask_text("First name: ", fn, sizeof fn);
    ask_text("Last name: ", ln, sizeof ln);
    ask_text("Email: ", mail, sizeof mail);
    ask_text("Date of birth (YYYY-MM-DD, blank to skip): ", dob, sizeof dob);
    ask_text("Contact number (blank to skip): ", ph, sizeof ph);
    ask_text("House no (blank to skip): ", house, sizeof house);
    ask_text("Society name (blank to skip): ", soc, sizeof soc);
    ask_text("Street (blank to skip): ", street, sizeof street);
    ask_text("City (blank to skip): ", city, sizeof city);
    ask_text("State (blank to skip): ", state, sizeof state);
    printf("Delivery location:\n");
    if (ask_loc(&loc))
        return;
    report(user_add(fn, ln, mail, dob, ph, house, soc, street, loc, city, state), "User");
}

static void list_users(void)
{
    user_list();
}

static void add_staff(void)
{
    char fn[101], ln[101], mail[256], ph[21], date[16];
    double pay;
    GeoPoint loc;

    ask_text("First name: ", fn, sizeof fn);
    ask_text("Last name: ", ln, sizeof ln);
    ask_text("Email: ", mail, sizeof mail);
    ask_text("Contact number (blank to skip): ", ph, sizeof ph);
    int role = ask_int("Role (1=Chef, 2=Delivery partner): ");
    if (role != ROLE_CHEF && role != ROLE_DELIVERY)
    {
        printf("Invalid role.\n");
        return;
    }
    ask_text("Hiring date (YYYY-MM-DD): ", date, sizeof date);
    if (ask_dbl("Salary: ", &pay))
        return;

    //a partner needs a start spot for the nearest search
    if (role == ROLE_DELIVERY)
    {
        printf("Partner's current location:\n");
        if (ask_loc(&loc))
            return;
    }
    int id = staff_add(fn, ln, mail, ph[0] ? ph : NULL, (Role) role, date, pay);
    report(id, "Staff");
    if (id > 0 && role == ROLE_DELIVERY && delivery_add(id, loc) != 0)
        printf("Staff saved, but the partner location was not.\n");
}

static void list_staff(void)
{
    staff_list();
}

static void add_item(void)
{
    char name[MAX_NAME];
    double price;

    ask_text("Item name: ", name, sizeof name);
    if (ask_dbl("Price: ", &price))
        return;
    report(item_add(name, price), "Item");
}

static void list_items(void)
{
    item_list();
}

static void set_avail(void)
{
    int id = ask_id("Item ID: ");
    if (id < 0)
        return;
    int on = ask_int("Available (1=yes, 0=no): ");
    if (on != 0 && on != 1)
    {
        printf("Invalid choice.\n");
        return;
    }
    show_ok(item_avail(id, on), "Item updated.");
}

static void set_price(void)
{
    double price;
    int id = ask_id("Item ID: ");
    if (id < 0 || ask_dbl("New price: ", &price))
        return;
    show_ok(item_price(id, price), "Price updated.");
}

//item entry stops at 0 or when the order is full
static void place_order(void)
{
    OrderLine lines[MAX_LINES];
    int n = 0;
    int uid = ask_id("User ID: ");
    if (uid < 0)
        return;
    while (n < MAX_LINES)
    {
        int iid = ask_int("  Item ID (0 = done): ");
        if (iid == 0) break;
        int q = ask_int("  Quantity: ");
        if (iid < 0 || q <= 0) { printf("  Invalid.\n"); continue; }
        lines[n].item_id = iid; lines[n].qty = q; n++;
    }
    if (n == 0)
    {
        printf("Empty order discarded.\n");
        return;
    }
    int pr = ask_int("Priority (1=urgent, 2=normal, 3=low): ");
    if (pr < 1 || pr > 3) pr = 2;
    report(order_place(uid, lines, n, pr), "Order");
}

static void view_order(void)
{
    int id = ask_id("Order ID: ");
    if (id >= 0) order_show(id);
}

static void show_queue(void)
{
    order_queue();
}

static void cook_order(void)
{
    int chef = ask_id("Chef staff ID: ");
    if (chef < 0)
        return;
    int oid = order_cook(chef);
    if (oid > 0) printf("Chef %d is cooking order %d\n", chef, oid);
    else         printf("No order to cook.\n");
}

static void ready_order(void)
{
    int id = ask_id("Order ID: ");
    if (id >= 0) show_ok(order_ready(id), "Order is ready.");
}

static void del_order(void)
{
    int id = ask_id("Order ID: ");
    if (id >= 0) show_ok(order_delete(id), "Order deleted.");
}

static void assign_ride(void)
{
    int oid = ask_id("Order ID: ");
    if (oid < 0)
        return;
    int sid = delivery_assign(oid);
    if (sid > 0) printf("Order %d -> delivery partner %d (nearest free)\n", oid, sid);
    else         printf("No free partner, or order not ready.\n");
}

static void finish_ride(void)
{
    int oid = ask_id("Order ID: ");
    if (oid >= 0) show_ok(delivery_done(oid), "Delivered.");
}

static void make_bill(void)
{
    int oid = ask_id("Order ID: ");
    if (oid >= 0) report(bill_make(oid), "Bill");
}

static void pay_bill(void)
{
    static const char *how[] = { "CASH", "CARD", "UPI", "NET_BANKING" };
    int bid = ask_id("Bill ID: ");
    if (bid < 0)
        return;
    int m = ask_int("Method (1=Cash, 2=Card, 3=UPI, 4=Net banking): ");
    if (m < 1 || m > 4)
    {
        printf("Invalid method.\n");
        return;
    }
    show_ok(bill_pay(bid, how[m - 1]), "Paid.");   //arrays start at 0
}

static void view_bill(void)
{
    int bid = ask_id("Bill ID: ");
    if (bid >= 0) bill_show(bid);
}

static void del_bill(void)
{
    int bid = ask_id("Bill ID: ");
    if (bid >= 0) show_ok(bill_del(bid), "Bill deleted.");
}

static void show_help(void);

static void quit_prog(void)
{
    on = 0;
}

//to add a command add a line here
static const Cmd cmds[] = {
    {"users add",         add_user},     {"users list",        list_users},
    {"staff add",         add_staff},    {"staff list",        list_staff},
    {"items add",         add_item},     {"items list",        list_items},
    {"items available",   set_avail},    {"items price",       set_price},
    {"orders place",      place_order},  {"orders view",       view_order},
    {"orders queue",      show_queue},   {"orders cook",       cook_order},
    {"orders ready",      ready_order},  {"orders delete",     del_order},
    {"delivery assign",   assign_ride},  {"delivery complete", finish_ride},
    {"billing generate",  make_bill},    {"billing pay",       pay_bill},
    {"billing view",      view_bill},    {"billing delete",    del_bill},
    {"help",              show_help},    {"exit",              quit_prog}
};

static void show_help(void)
{
    printf("Commands:\n");
    for (int i = 0; i < NCMD; i++)
        printf("  %s\n", cmds[i].name);
}

static void trim(char *s)
{
    char *a = s;
    while (*a == ' ')
        a++;
    memmove(s, a, strlen(a) + 1);
    size_t n = strlen(s);
    while (n > 0 && s[n - 1] == ' ')
        s[--n] = '\0';
}

static int find_cmd(const char *name)
{
    for (int i = 0; i < NCMD; i++)
    {
        if (strcmp(cmds[i].name, name) == 0)
            return i;
    }
    return -1;
}

int main(void)
{
    conn = db_open();
    if (conn == NULL)
        return 1;

    int wait = order_load();
    if (wait < 0)
        printf("Warning: could not load waiting orders.\n");
    else if (wait > 0)
        printf("%d order(s) waiting in the kitchen queue.\n", wait);

    if (NCMD > MAXL)
    {
        printf("Too many commands for the trie (max %d).\n", MAXL);
        db_close(conn);
        return 1;
    }

    //every command goes in the trie for hints
    TNode *trie = trie_new();
    if (trie == NULL)
    {
        db_close(conn);
        return 1;
    }
    for (int i = 0; i < NCMD; i++)
        trie_add(trie, cmds[i].name);

    char in[MAXC];
    printf("Type 'help' for the command list.\n");

    while (on)
    {
        ask_text("kitchen> ", in, sizeof in);
        trim(in);
        if (!in[0])
            continue;

        int f = find_cmd(in);
        if (f >= 0)
        {
            trie_use(trie, in);
            cmds[f].run();
            continue;
        }

        //no exact match so show commands that start with the input
        Word hint[MAXL];
        int k = trie_list(trie, in, hint, MAXL);
        if (k == 0)
        {
            printf("Unknown command '%s'.\n", in);
            continue;
        }
        printf("Did you mean:\n");
        for (int i = 0; i < k; i++)
            printf("  %s\n", hint[i]);
    }

    trie_free(trie);
    db_close(conn);
    return 0;
}
