#ifndef USER_H
#define USER_H
#include "../common.h"

typedef struct { 
    int id;
    char name[MAX_NAME];
    char phone[21];
    GeoPoint location; 
} User;

int user_add(const char *fn, const char *ln, const char *email, const char *dob, const char *contact, const char *hn, const char *sn, const char *street, GeoPoint loc, const char *city, const char *state);
int user_get(int id, User *out);
void user_list(void);
#endif
