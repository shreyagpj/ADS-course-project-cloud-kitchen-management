#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "menu.h"

static MenuItem *head = NULL;   /* linked list of menu items */

void addMenuItem(int id, const char *name, float price) {
    if (findMenuItem(id) != NULL) {
        printf("Item with id %d already exists.\n", id);
        return;
    }

    MenuItem *item = (MenuItem *)malloc(sizeof(MenuItem));
    if (item == NULL) {
        printf("Out of memory.\n");
        return;
    }
    item->id = id;
    strncpy(item->name, name, sizeof(item->name) - 1);
    item->name[sizeof(item->name) - 1] = '\0';
    item->price = price;
    item->next = head;          /* insert at the front */
    head = item;
    printf("Added %s to the menu.\n", item->name);
}

void removeMenuItem(int id) {
    MenuItem *cur = head, *prev = NULL;

    while (cur != NULL && cur->id != id) {
        prev = cur;
        cur = cur->next;
    }
    if (cur == NULL) {
        printf("Item %d not found.\n", id);
        return;
    }
    if (prev == NULL)
        head = cur->next;
    else
        prev->next = cur->next;
    free(cur);
    printf("Item %d removed.\n", id);
}

MenuItem *findMenuItem(int id) {
    MenuItem *cur = head;
    while (cur != NULL) {
        if (cur->id == id)
            return cur;
        cur = cur->next;
    }
    return NULL;
}

void showMenu(void) {
    if (head == NULL) {
        printf("Menu is empty.\n");
        return;
    }
    printf("ID   Name                 Price\n");
    for (MenuItem *cur = head; cur != NULL; cur = cur->next)
        printf("%-4d %-20s %.2f\n", cur->id, cur->name, cur->price);
}