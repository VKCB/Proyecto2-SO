#ifndef MROBOT_H
#define MROBOT_H

int mrobot_init(const char *device);
int mrobot_move(int x, int y);
int mrobot_press();
void mrobot_close();
int letra_a_pasos(char letra, int *x, int *y);

#endif