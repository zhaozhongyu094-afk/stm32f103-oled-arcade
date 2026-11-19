#ifndef BUTTONS_H
#define BUTTONS_H
#include "arcade.h"
typedef struct {
    uint8_t held, pressed, released;
} Keys;
void buttons_init(void);
Keys buttons_poll(uint32_t now);
#endif
