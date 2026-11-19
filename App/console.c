#include "console.h"
#include "gfx.h"
static const Game *const games[] = {&snake_game, &runner_game, &post_game};
static const char *const names[] = {"SNAKE", "HURDLE RUN", "SUMMIT POST"};
static const Game *active;
static unsigned selected;
static bool paused;
void console_home(void) {
    active = 0;
    paused = false;
}
void console_pause(void) {
    if (active)
        paused = !paused;
}
void console_step(Input in) {
    if (active) {
        if (!paused)
            active->step(in);
        return;
    }
    if (in.pressed & (K_UP | K_LEFT))
        selected = (selected + 2) % 3;
    if (in.pressed & (K_DOWN | K_RIGHT))
        selected = (selected + 1) % 3;
    if (in.pressed & K_ACTION) {
        active = games[selected];
        active->enter();
    }
}
void console_draw(void) {
    gfx_clear();
    if (active) {
        active->draw();
        if (paused)
            gfx_panel("PAUSED", "TAP MENU TO RESUME");
        return;
    }
    gfx_text(32, 2, "OLED GAME LIBRARY");
    gfx_rect(2, 11, 124, 1, 1);
    for (unsigned i = 0; i < 3; i++) {
        int y = 16 + (int)i * 12;
        if (i == selected) {
            gfx_box(5, y - 2, 118, 10);
            gfx_text(10, y, ">");
        }
        gfx_text(21, y, names[i]);
    }
    gfx_text(19, 56, "U/D SELECT  A ENTER");
}
