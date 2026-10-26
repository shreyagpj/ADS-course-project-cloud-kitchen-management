#ifndef USER_H
#define USER_H
#include "../common.h"

typedef struct
{
    int id;
    char name[MAX_NAME];
    char phone[21];
    GeoPoint location;
} User;

int user_add(const char *fname, const char *lname, const char *mail, const char *dob, const char *phone,
             const char *house, const char *soc, const char *street, GeoPoint loc, const char *city, const char *state);
int user_get(int id, User *out);
void user_list(void);
#endif
