#ifndef BILLING_H
#define BILLING_H

int  bill_make(int oid);
int  bill_del(int bid);

//method is cash or card or upi or net banking in capitals
int  bill_pay(int bid, const char *method);

void bill_show(int bid);
#endif
