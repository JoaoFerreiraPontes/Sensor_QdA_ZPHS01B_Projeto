#ifndef BT_H
#define BT_H


//Funções que podem ser usadas fora do bt.c

void bt_init(void);                     //chame em main uma vez antes de usar o bluetooth
void send_message(const char *message); //messagem - zero terminated string

#endif
