#ifndef BT_H
#define BT_H

//Functions that can be used outside of of bt.c

void bt_init(void);                     //call in main once before use of bluetooth
void send_message(const char *message); //message - zero terminated string

#endif
