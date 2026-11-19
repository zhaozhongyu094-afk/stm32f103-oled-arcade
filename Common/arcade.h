#ifndef ARCADE_H
#define ARCADE_H
#include <stdbool.h>
#include <stdint.h>
enum { K_UP = 1, K_DOWN = 2, K_LEFT = 4, K_RIGHT = 8, K_ACTION = 16, K_MENU = 32 };
enum { GAME_STEP_MS = 8 };
typedef struct {
    uint8_t held, pressed;
} Input;
typedef struct {
    void (*enter)(void);
    void (*step)(Input input);
    void (*draw)(void);
} Game;
extern const Game snake_game, runner_game, post_game;
#endif
