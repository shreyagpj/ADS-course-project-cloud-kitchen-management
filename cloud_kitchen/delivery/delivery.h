#ifndef DELIVERY_H
#define DELIVERY_H
#include "../common.h"

//saves or updates a DELIVERY staff member's location; 0 or -1
int delivery_add_partner(int sid, GeoPoint loc);

//nearest free partner (kd tree) takes a READY order, which becomes OUT_FOR_DELIVERY; returns staff_id or -1
int delivery_assign(int oid);

//OUT_FOR_DELIVERY order becomes DELIVERED and the partner moves to the customer; 0 or -1
int delivery_complete(int oid);

#endif
