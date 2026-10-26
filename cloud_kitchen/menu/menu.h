#ifndef MENU_H
#define MENU_H

int item_add(const char *name, double price);
void item_list(void);
int item_avail(int id, int on);
int item_price(int id, double price);

#endif
