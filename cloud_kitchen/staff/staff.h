/* staff.h - chefs and delivery partners.
 * main.c calls:   int id = staff_add("Ravi", ROLE_CHEF);   staff_list();
 * other modules:  orders.c   -> staff_get(chef_id, &s), check s.role == ROLE_CHEF
 *                 delivery.c -> staff_get(id, &s), check s.role == ROLE_DELIVERY */
#ifndef STAFF_H
#define STAFF_H
#include "../common.h"

typedef struct { int id; char name[NAME_LEN]; Role role; } Staff;

int  staff_add(const char *name, Role role);
int  staff_get(int id, Staff *out);
void staff_list(void);
#endif
