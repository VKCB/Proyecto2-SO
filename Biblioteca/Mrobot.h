#ifndef MROBOT_H
#define MROBOT_H

int mrobot_init(const char *device_path);
int mrobot_move(int x, int y);
int mrobot_press();
void mrobot_close();

#endif 