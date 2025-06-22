#include "Mrobot.h"
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>

static int mrobot_fd = -1;

int mrobot_init(const char *device_path) {
    mrobot_fd = open(device_path, O_RDWR);
    return (mrobot_fd < 0) ? -1 : 0;
}

int mrobot_move(int x, int y) {
    if (mrobot_fd < 0) return -1;
    int pos[2] = {x, y};
    return write(mrobot_fd, pos, sizeof(pos)) == sizeof(pos) ? 0 : -1;
}

int mrobot_press() {
    if (mrobot_fd < 0) return -1;
    char cmd = 'P';
    return write(mrobot_fd, &cmd, 1) == 1 ? 0 : -1;
}

void mrobot_close() {
    if (mrobot_fd >= 0) close(mrobot_fd);
    mrobot_fd = -1;
}