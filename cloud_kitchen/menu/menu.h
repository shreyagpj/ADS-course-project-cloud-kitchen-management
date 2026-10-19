/* menu.h - food items on the menu.
 * main.c calls:   int id = item_add("Paneer Wrap", 149.0);   item_list();
 * orders.c calls: item_get(item_id, &it) to read the price when computing the total. */
#ifndef MENU_H
#define MENU_H
#include "../common.h"

typedef struct { int id; char name[NAME_LEN]; double price; } Item;

int  item_add(const char *name, double price);
int  item_get(int id, Item *out);
void item_list(void);
#endif
