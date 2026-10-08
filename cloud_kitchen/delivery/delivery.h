/* delivery.h - gives READY orders to the nearest free delivery partner (KD-tree).
 *
 * main.c calls:
 *   delivery_add_partner(staff_id, location);   // after staff_add(..., ROLE_DELIVERY)
 *   int sid = delivery_assign(order_id);        // nearest free partner, or -1
 *   delivery_complete(order_id);                // order DELIVERED, partner free again
 *
 * delivery_assign(order_id) should:
 *   1. order_get()  -> status must be ORDER_READY
 *   2. user_get(order.user_id) -> target = user.location
 *   3. sid = kd_nearest_free(root, target)
 *   4. kd_set_busy(root, sid, 1); order_set_delivery_staff(); order_set_status(OUT_FOR_DELIVERY) */
#ifndef DELIVERY_H
#define DELIVERY_H
#include "../common.h"

int delivery_add_partner(int staff_id, GeoPoint location);
int delivery_assign(int order_id);
int delivery_complete(int order_id);
#endif
