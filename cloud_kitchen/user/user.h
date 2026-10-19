/* user.h - customers.
 * main.c calls:   int id = user_add("Asha", "9999999999", location);
 *                 user_list();
 * other modules:  orders.c   -> user_get(id, &u)  to check the customer exists
 *                 delivery.c -> user_get(id, &u)  and use u.location as the
 *                               target point for the KD-tree search          */
#ifndef USER_H
#define USER_H
#include "../common.h"

typedef struct { int id; char name[NAME_LEN]; char phone[16]; GeoPoint location; } User;

int  user_add(const char *name, const char *phone, GeoPoint location);
int  user_get(int id, User *out);
void user_list(void);
#endif
