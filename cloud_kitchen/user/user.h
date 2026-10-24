#ifndef USER_H
#define USER_H
#include "../common.h"

typedef struct { 
    int id;
    char name[MAX_NAME];
    char phone[16];
    GeoPoint location; 
} User;

int user_add(const char *fName, const char *lName, const char *email, const char *dob, const char *contact, const char *houseNo, const char *socName, const char *street, GeoPoint loc, const char *city, const char *state);
int  user_get(int id, User *out);
void user_list(void);
#endif
