#ifndef STAFF_H
#define STAFF_H
#include "../common.h"

typedef struct
{
    int id;
    char name[MAX_NAME];
    Role role;
} Staff;

int staff_add(const char *fname, const char *lname, const char *mail, const char *phone, Role role, const char *hired, double pay);
int staff_get(int id, Staff *out);
void staff_list(void);
#endif
