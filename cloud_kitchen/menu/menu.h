#ifndef MENU_H
#define MENU_H

typedef struct MenuItem {
    int id;
    char name[50];
    float price;
    struct MenuItem *next;
} MenuItem;

void addMenuItem(int id, const char *name, float price);
void removeMenuItem(int id);
MenuItem *findMenuItem(int id);
void showMenu(void);

#endif
