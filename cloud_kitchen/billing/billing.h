/* billing.h - one bill per order.
 * main.c calls:   int bid = bill_generate(order_id);   // copies order.total
 *                 bill_pay(bid);                       // marks it paid
 *                 bill_show(bid);
 * Bills are stored only in the MySQL database (table billing).            */
#ifndef BILLING_H
#define BILLING_H

int  bill_generate(int order_id);
int  bill_delete(int bill_id);   /* database only; 0 on success, -1 on failure */
int  bill_pay(int bill_id);
void bill_show(int bill_id);
#endif
