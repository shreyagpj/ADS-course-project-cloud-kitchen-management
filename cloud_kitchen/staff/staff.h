#ifndef STAFF_H
#define STAFF_H
#include "../common.h"

typedef struct { 
    int id; 
    char name[MAX_NAME]; 
    Role role; 
} Staff;

int staff_add(const char *fn, const char *ln, const char *email, const char *cn, Role role, const char *hd, double salary);
int staff_get(int id, Staff *out);
void staff_list(void);
#endif
