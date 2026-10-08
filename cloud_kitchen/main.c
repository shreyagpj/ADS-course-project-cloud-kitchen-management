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

/* ---------- sub-menus ---------- */
static void menu_users(void)
{
    printf("\n1. Add user\n2. List users\n");
    switch (ask_int("Choice: ")) {
    case 1: { char n[NAME_LEN], p[16];
              ask_text("Name : ", n, sizeof n);
              ask_text("Phone: ", p, sizeof p);
              printf("Delivery location:\n");
              report(user_add(n, p, ask_location()), "User"); break; }
    case 2: user_list(); break;
    default: printf("Invalid choice.\n");
    }
}

static void menu_staff(void)
{
    printf("\n1. Add staff\n2. List staff\n");
    switch (ask_int("Choice: ")) {
    case 1: { char n[NAME_LEN];
              ask_text("Name: ", n, sizeof n);
              int role = ask_int("Role (1=Chef, 2=Delivery partner): ");
              if (role != ROLE_CHEF && role != ROLE_DELIVERY) { printf("Invalid role.\n"); break; }
              int id = staff_add(n, (Role)role);
              report(id, "Staff");
              if (id > 0 && role == ROLE_DELIVERY) {       /* goes into the KD-tree */
                  printf("Partner's current location:\n");
                  delivery_add_partner(id, ask_location());
              }
              break; }
    case 2: staff_list(); break;
    default: printf("Invalid choice.\n");
    }
}

static void menu_items(void)
{
    printf("\n1. Add item\n2. List items\n");
    switch (ask_int("Choice: ")) {
    case 1: { char n[NAME_LEN];
              ask_text("Item name: ", n, sizeof n);
              report(item_add(n, ask_double("Price: ")), "Item"); break; }
    case 2: item_list(); break;
    default: printf("Invalid choice.\n");
    }
}

static void menu_orders(void)
{
    printf("\n1. Place order\n2. View order\n3. Kitchen queue\n"
           "4. Chef takes next order\n5. Mark order ready\n");
    switch (ask_int("Choice: ")) {
    case 1: { int uid = ask_int("User ID: ");
              OrderLine lines[MAX_LINES]; int n = 0;
              while (n < MAX_LINES) {
                  int iid = ask_int("  Item ID (0 = done): ");
                  if (iid == 0) break;
                  int q = ask_int("  Quantity: ");
                  if (iid < 0 || q <= 0) { printf("  Invalid.\n"); continue; }
                  lines[n].item_id = iid; lines[n].qty = q; n++;
              }
              if (n == 0) { printf("Empty order discarded.\n"); break; }
              int pr = ask_int("Priority (1=urgent, 2=normal, 3=low): ");
              if (pr < 1 || pr > 3) pr = 2;
              report(order_place(uid, lines, n, pr), "Order"); break; }
    case 2: order_show(ask_int("Order ID: ")); break;
    case 3: order_queue_show(); break;
    case 4: { int chef = ask_int("Chef staff ID: ");
              int oid = order_cook_next(chef);
              if (oid > 0) printf("Chef %d is cooking order %d\n", chef, oid);
              else         printf("No order to cook.\n");
              break; }
    case 5: printf(order_mark_ready(ask_int("Order ID: ")) == 0 ? "Order is ready.\n" : "Failed.\n"); break;
    default: printf("Invalid choice.\n");
    }
}

static void menu_delivery(void)
{
    printf("\n1. Assign nearest delivery partner\n2. Mark delivered\n");
    switch (ask_int("Choice: ")) {
    case 1: { int oid = ask_int("Order ID: ");
              int sid = delivery_assign(oid);
              if (sid > 0) printf("Order %d -> delivery partner %d (nearest free)\n", oid, sid);
              else         printf("No free partner, or order not ready.\n");
              break; }
    case 2: printf(delivery_complete(ask_int("Order ID: ")) == 0 ? "Delivered.\n" : "Failed.\n"); break;
    default: printf("Invalid choice.\n");
    }
}

static void menu_billing(void)
{
    printf("\n1. Generate bill\n2. Pay bill\n3. View bill\n");
    switch (ask_int("Choice: ")) {
    case 1: report(bill_generate(ask_int("Order ID: ")), "Bill"); break;
    case 2: printf(bill_pay(ask_int("Bill ID: ")) == 0 ? "Paid.\n" : "Failed.\n"); break;
    case 3: bill_show(ask_int("Bill ID: ")); break;
    default: printf("Invalid choice.\n");
    }
}

int main(void)
{
    int choice;
    do {
        printf("\nCLOUD KITCHEN MANAGEMENT SYSTEM\n"
               "1. Users\n2. Staff\n3. Menu items\n4. Orders\n"
               "5. Delivery\n6. Billing\n0. Exit\n");
        choice = ask_int("Choice: ");
        switch (choice) {
        case 1: menu_users();    break;
        case 2: menu_staff();    break;
        case 3: menu_items();    break;
        case 4: menu_orders();   break;
        case 5: menu_delivery(); break;
        case 6: menu_billing();  break;
        case 0: printf("Goodbye!\n"); break;
        default: printf("Invalid choice.\n");
        }
    } while (choice != 0);
    return 0;
}
