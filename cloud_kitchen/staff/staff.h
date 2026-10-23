#ifndef STAFF_H
#define STAFF_H
#include "../common.h"

typedef struct { 
    int id; 
    char name[MAX_NAME]; 
    Role role; 
} Staff;

int staff_add(const char *fName, const char *lName, const char *email, const char *contactNumber, Role role, const char *hiringDate, double salary);
int  staff_get(int id, Staff *out);
void staff_list(void);
#endif
