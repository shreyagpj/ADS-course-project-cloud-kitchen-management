//billing.h: one bill per order, stored only in the billing table
#ifndef BILLING_H
#define BILLING_H

int  bill_generate(int oid);
int  bill_delete(int bid);
int  bill_pay(int bill_id, const char *method);   //method: CASH, CARD, UPI or NET_BANKING
void bill_show(int bid);
#endif
