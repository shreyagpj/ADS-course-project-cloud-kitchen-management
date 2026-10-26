#ifndef DELIVERY_H
#define DELIVERY_H
#include "../common.h"

int delivery_add(int sid, GeoPoint loc);

//gives the staff id of the nearest free partner or minus one
int delivery_assign(int oid);

int delivery_done(int oid);

#endif
