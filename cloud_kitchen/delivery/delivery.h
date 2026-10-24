#ifndef DELIVERY_H
#define DELIVERY_H

#include "../common.h"      /* GeoPoint */

/* Registers a delivery partner's current location (or updates it if already registered).
 * staff_id must belong to a staff member with role DELIVERY.
 * Returns 0 on success, -1 on failure. */
int delivery_add_partner(int staff_id, GeoPoint loc);

/* Assigns the nearest free delivery partner to a READY order and sets the order
 * to OUT_FOR_DELIVERY. Returns the partner's staff_id, or -1 if the order is not
 * ready / has no location / no partner is free. */
int delivery_assign(int order_id);

/* Marks an OUT_FOR_DELIVERY order as DELIVERED and moves the partner to the
 * customer's location. Returns 0 on success, -1 on failure. */
int delivery_complete(int order_id);

#endif