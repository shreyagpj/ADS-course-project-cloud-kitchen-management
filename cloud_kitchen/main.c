/* main.c - the terminal loop. Only reads input and calls module functions. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "user/user.h"
#include "staff/staff.h"
#include "menu/menu.h"
#include "orders/orders.h"
#include "delivery/delivery.h"
#include "billing/billing.h"
#include "db/db.h"
#include "trie/trie.h"
#include "terminal/terminal.h"

static void ask_text(const char *prompt, char *buf, int size)
{
    printf("%s", prompt);
    if (!fgets(buf, size, stdin)) { printf("\nBye!\n"); exit(0); }
    buf[strcspn(buf, "\n")] = '\0';
}
static int ask_int(const char *prompt)         
{
    char b[32]; ask_text(prompt, b, sizeof b);
    char *end; long v = strtol(b, &end, 10);
    return (end == b || *end) ? -1 : (int)v;
}
static double ask_double(const char *prompt)
{
    char b[32]; ask_text(prompt, b, sizeof b);
    return atof(b);
}
static GeoPoint ask_location(void)
{
    GeoPoint g;
    g.lat = ask_double("  Latitude : ");
    g.lon = ask_double("  Longitude: ");
    return g;
}
static void report(int id, const char *what)    /* prints result of an "add" call */
{
    if (id > 0) printf("%s created with ID %d\n", what, id);
    else        printf("Could not create %s.\n", what);
}

/* ---------- commands (one function per action; used by menus and terminal) ---------- */
static void cmd_user_add(void)
{
    char n[NAME_LEN], p[16];
    ask_text("Name : ", n, sizeof n);
    ask_text("Phone: ", p, sizeof p);
    printf("Delivery location:\n");
    report(user_add(n, p, ask_location()), "User");
}
static void cmd_user_list(void) { user_list(); }

static void cmd_staff_add(void)
{
    char n[NAME_LEN];
    ask_text("Name: ", n, sizeof n);
    int role = ask_int("Role (1=Chef, 2=Delivery partner): ");
    if (role != ROLE_CHEF && role != ROLE_DELIVERY) { printf("Invalid role.\n"); return; }
    int id = staff_add(n, (Role)role);
    report(id, "Staff");
    if (id > 0 && role == ROLE_DELIVERY) {           /* goes into the KD-tree */
        printf("Partner's current location:\n");
        delivery_add_partner(id, ask_location());
    }
}
static void cmd_staff_list(void) { staff_list(); }

static void cmd_item_add(void)
{
    char n[NAME_LEN];
    ask_text("Item name: ", n, sizeof n);
    report(item_add(n, ask_double("Price: ")), "Item");
}
static void cmd_item_list(void) { item_list(); }

static void cmd_order_place(void)
{
    int uid = ask_int("User ID: ");
    OrderLine lines[MAX_LINES]; int n = 0;
    while (n < MAX_LINES) {
        int iid = ask_int("  Item ID (0 = done): ");
        if (iid == 0) break;
        int q = ask_int("  Quantity: ");
        if (iid < 0 || q <= 0) { printf("  Invalid.\n"); continue; }
        lines[n].item_id = iid; lines[n].qty = q; n++;
    }
    if (n == 0) { printf("Empty order discarded.\n"); return; }
    int pr = ask_int("Priority (1=urgent, 2=normal, 3=low): ");
    if (pr < 1 || pr > 3) pr = 2;
    report(order_place(uid, lines, n, pr), "Order");
}
static void cmd_order_view(void)  { order_show(ask_int("Order ID: ")); }
static void cmd_order_queue(void) { order_queue_show(); }
static void cmd_order_cook(void)
{
    int chef = ask_int("Chef staff ID: ");
    int oid = order_cook_next(chef);
    if (oid > 0) printf("Chef %d is cooking order %d\n", chef, oid);
    else         printf("No order to cook.\n");
}
static void cmd_order_ready(void)
{
    printf(order_mark_ready(ask_int("Order ID: ")) == 0 ? "Order is ready.\n" : "Failed.\n");
}

static void cmd_delivery_assign(void)
{
    int oid = ask_int("Order ID: ");
    int sid = delivery_assign(oid);
    if (sid > 0) printf("Order %d -> delivery partner %d (nearest free)\n", oid, sid);
    else         printf("No free partner, or order not ready.\n");
}
static void cmd_delivery_complete(void)
{
    printf(delivery_complete(ask_int("Order ID: ")) == 0 ? "Delivered.\n" : "Failed.\n");
}

static void cmd_bill_generate(void) { report(bill_generate(ask_int("Order ID: ")), "Bill"); }
static void cmd_bill_pay(void)
{
    printf(bill_pay(ask_int("Bill ID: ")) == 0 ? "Paid.\n" : "Failed.\n");
}
static void cmd_bill_view(void) { bill_show(ask_int("Bill ID: ")); }

/* ---------- numbered menus (original UI, kept as "menu" command) ---------- */
static void menu_users(void)
{
    printf("\n1. Add user\n2. List users\n");
    switch (ask_int("Choice: ")) {
    case 1: cmd_user_add(); break;
    case 2: cmd_user_list(); break;
    default: printf("Invalid choice.\n");
    }
}
static void menu_staff(void)
{
    printf("\n1. Add staff\n2. List staff\n");
    switch (ask_int("Choice: ")) {
    case 1: cmd_staff_add(); break;
    case 2: cmd_staff_list(); break;
    default: printf("Invalid choice.\n");
    }
}
static void menu_items(void)
{
    printf("\n1. Add item\n2. List items\n");
    switch (ask_int("Choice: ")) {
    case 1: cmd_item_add(); break;
    case 2: cmd_item_list(); break;
    default: printf("Invalid choice.\n");
    }
}
static void menu_orders(void)
{
    printf("\n1. Place order\n2. View order\n3. Kitchen queue\n"
           "4. Chef takes next order\n5. Mark order ready\n");
    switch (ask_int("Choice: ")) {
    case 1: cmd_order_place(); break;
    case 2: cmd_order_view(); break;
    case 3: cmd_order_queue(); break;
    case 4: cmd_order_cook(); break;
    case 5: cmd_order_ready(); break;
    default: printf("Invalid choice.\n");
    }
}
static void menu_delivery(void)
{
    printf("\n1. Assign nearest delivery partner\n2. Mark delivered\n");
    switch (ask_int("Choice: ")) {
    case 1: cmd_delivery_assign(); break;
    case 2: cmd_delivery_complete(); break;
    default: printf("Invalid choice.\n");
    }
}
static void menu_billing(void)
{
    printf("\n1. Generate bill\n2. Pay bill\n3. View bill\n");
    switch (ask_int("Choice: ")) {
    case 1: cmd_bill_generate(); break;
    case 2: cmd_bill_pay(); break;
    case 3: cmd_bill_view(); break;
    default: printf("Invalid choice.\n");
    }
}
static void run_menu(void)
{
    int choice;
    do {
        printf("\nCLOUD KITCHEN MANAGEMENT SYSTEM\n"
               "1. Users\n2. Staff\n3. Menu items\n4. Orders\n"
               "5. Delivery\n6. Billing\n0. Back to terminal\n");
        choice = ask_int("Choice: ");
        switch (choice) {
        case 1: menu_users();    break;
        case 2: menu_staff();    break;
        case 3: menu_items();    break;
        case 4: menu_orders();   break;
        case 5: menu_delivery(); break;
        case 6: menu_billing();  break;
        case 0: break;
        default: printf("Invalid choice.\n");
        }
    } while (choice != 0);
}

/* ---------- command terminal with trie autosuggest ---------- */
static void cmd_menu(void) { run_menu(); }
static void cmd_help(void);

typedef struct { const char *name; void (*fn)(void); const char *help; } Command;

static const Command COMMANDS[] = {
    { "users add",         cmd_user_add,         "register a customer" },
    { "users list",        cmd_user_list,        "list customers" },
    { "staff add",         cmd_staff_add,        "hire a chef / delivery partner" },
    { "staff list",        cmd_staff_list,       "list staff" },
    { "items add",         cmd_item_add,         "add a menu item" },
    { "items list",        cmd_item_list,        "list menu items" },
    { "orders place",      cmd_order_place,      "place a new order" },
    { "orders view",       cmd_order_view,       "show one order" },
    { "orders queue",      cmd_order_queue,      "show kitchen priority queue" },
    { "orders cook",       cmd_order_cook,       "chef takes the next order" },
    { "orders ready",      cmd_order_ready,      "mark an order ready" },
    { "delivery assign",   cmd_delivery_assign,  "assign nearest free partner" },
    { "delivery complete", cmd_delivery_complete,"mark an order delivered" },
    { "billing generate",  cmd_bill_generate,    "generate a bill" },
    { "billing pay",       cmd_bill_pay,         "pay a bill" },
    { "billing view",      cmd_bill_view,        "show a bill" },
    { "menu",              cmd_menu,             "numbered menu interface" },
    { "help",              cmd_help,             "list all commands" },
    { "exit",              NULL,                 "quit" },
};
#define NCOMMANDS ((int)(sizeof COMMANDS / sizeof COMMANDS[0]))

static void cmd_help(void)
{
    printf("\nCommands (type a few letters; Tab completes, Right-arrow accepts the grey suggestion):\n");
    for (int i = 0; i < NCOMMANDS; i++)
        printf("  %-18s %s\n", COMMANDS[i].name, COMMANDS[i].help);
}

static void run_terminal(void)
{
    TrieNode *trie = trie_create();
    if (!trie) { fprintf(stderr, "out of memory\n"); return; }
    for (int i = 0; i < NCOMMANDS; i++) trie_insert(trie, COMMANDS[i].name);

    printf("\nCLOUD KITCHEN TERMINAL - type 'help' for commands\n");
    char line[TRIE_MAX_WORD];
    while (terminal_read_command("kitchen> ", trie, line, sizeof line)) {
        if (!*line) continue;
        const Command *hit = NULL;
        for (int i = 0; i < NCOMMANDS; i++)
            if (strcmp(line, COMMANDS[i].name) == 0) { hit = &COMMANDS[i]; break; }

        if (!hit) {
            char *sug[5];
            int n = trie_suggest(trie, line, sug, 5);
            printf("Unknown command '%s'.", line);
            if (n) {
                printf(" Did you mean:\n");
                for (int i = 0; i < n; i++) printf("  %s\n", sug[i]);
                trie_free_suggestions(sug, n);
            } else printf(" Type 'help'.\n");
            continue;
        }
        if (!hit->fn) break;                       /* exit */
        trie_record_use(trie, hit->name);          /* frequent commands rank first */
        hit->fn();
    }
    printf("Goodbye!\n");
    trie_free(trie);
}

int main(void)
{
    MYSQL *conn = connectDb();
    if (conn == NULL)
        return 1;

    run_terminal();

    disconnectDb(conn);
    return 0;
}
