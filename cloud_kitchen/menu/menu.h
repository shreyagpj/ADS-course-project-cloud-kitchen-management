#ifndef MENU_H
#define MENU_H

/* Adds a menu item to the database.
 * Returns the new item ID (> 0), or -1 on failure. */
int item_add(const char *name, double price);

/* Prints all menu items. */
void item_list(void);

#endif