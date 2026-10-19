/* billing.h - one bill per order.
 * main.c calls:   int bid = bill_generate(order_id);   // copies order.total
 *                 bill_pay(bid);                       // marks it paid
 *                 bill_show(bid);
 * Bill fields to store: id, order_id, user_id, total, paid (0/1).            */
#ifndef BILLING_H
#define BILLING_H

int  bill_generate(int order_id);
int  bill_pay(int bill_id);
void bill_show(int bill_id);
#endif
