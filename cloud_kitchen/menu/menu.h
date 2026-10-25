#ifndef MENU_H
#define MENU_H

//new item id (> 0) or -1
int item_add(const char *name, double price);

void item_list(void);
int item_set_available(int id, int on);
int item_set_price(int id, double p);

#endif
