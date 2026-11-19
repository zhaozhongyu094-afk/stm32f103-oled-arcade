#include "arcade.h"
#include "gfx.h"
#include <string.h>
enum { COLS = 30, ROWS = 11, CELLS = COLS * ROWS, NONE = 65535 };
enum SnakeState { SETUP, PLAY, OVER, WON };
typedef enum { RIGHT, DOWN, LEFT, UP } Dir;
static struct {
    uint16_t body[CELLS], len, food, bonus, bonus_ms, score, eaten, move_ms;
    uint32_t time_ms;
    uint8_t mode, difficulty, queued;
    Dir dir, queue[2];
    enum SnakeState state;
} s;
static unsigned best[2];
static uint32_t rng = 0x54731;
static bool stone(int cell) {
    int x = cell % COLS, y = cell / COLS;
    return s.mode && (x == 14 || x == 15) && ((y >= 2 && y <= 4) || y == 7 || y == 8);
}
static uint16_t pick_free(void) {
    uint8_t taken[CELLS] = {0};
    unsigned free_count = 0;
    for (unsigned i = 0; i < s.len; i++)
        taken[s.body[i]] = 1;
    if (s.food != NONE)
        taken[s.food] = 1;
    if (s.bonus != NONE)
        taken[s.bonus] = 1;
    // Reservoir sampling also terminates when every square is occupied.
    uint16_t result = NONE;
    for (int i = 0; i < CELLS; i++)
        if (!taken[i] && !stone(i)) {
            rng = rng * 1664525u + 1013904223u;
            if ((rng >> 8) % ++free_count == 0)
                result = (uint16_t)i;
        }
    return result;
}
static void begin(void) {
    s.body[0] = 157;
    s.body[1] = 156;
    s.body[2] = 155;
    s.len = 3;
    s.dir = RIGHT;
    s.queued = 0;
    s.score = 0;
    s.eaten = 0;
    s.time_ms = 0;
    s.move_ms = 0;
    s.food = NONE;
    s.bonus = NONE;
    s.bonus_ms = 0;
    s.food = pick_free();
    s.state = PLAY;
}
static void enter(void) {
    s.state = SETUP;
}
static void turn(Dir next) {
    Dir prev = s.queued ? s.queue[s.queued - 1] : s.dir;
    if (s.queued < 2 && next != prev && ((next + 2) % 4) != prev)
        s.queue[s.queued++] = next;
}
static void move(void) {
    static const int dx[] = {1, 0, -1, 0}, dy[] = {0, 1, 0, -1};
    if (s.queued) {
        s.dir = s.queue[0];
        if (--s.queued)
            s.queue[0] = s.queue[1];
    }
    int x = s.body[0] % COLS + dx[s.dir], y = s.body[0] / COLS + dy[s.dir], cell = y * COLS + x;
    if (x < 0 || x >= COLS || y < 0 || y >= ROWS || stone(cell)) {
        s.state = OVER;
        return;
    }
    bool normal = cell == s.food, extra = cell == s.bonus, grow = normal || extra;
    unsigned keep = s.len - (grow ? 0u : 1u);
    for (unsigned i = 0; i < keep; i++)
        if (s.body[i] == cell) {
            s.state = OVER;
            return;
        }
    memmove(s.body + 1, s.body, keep * sizeof(s.body[0]));
    s.body[0] = (uint16_t)cell;
    if (!grow)
        return;
    s.len++;
    s.score += (extra ? 5 : 1);
    if (s.score > best[s.mode])
        best[s.mode] = s.score;
    if (normal) {
        s.eaten++;
        s.food = NONE;
    } else {
        s.bonus = NONE;
        s.bonus_ms = 0;
    }
    if (s.len == CELLS - (s.mode ? 10 : 0)) {
        s.state = WON;
        return;
    }
    if (s.food == NONE)
        s.food = pick_free();
    if (normal && s.eaten % 5 == 0 && s.bonus == NONE) {
        s.bonus = pick_free();
        if (s.bonus != NONE)
            s.bonus_ms = 5000;
    }
}
static void step(Input in) {
    if (s.state == SETUP) {
        rng += 1;
        if (in.pressed & (K_UP | K_DOWN))
            s.mode ^= 1;
        if (in.pressed & K_LEFT)
            s.difficulty = (s.difficulty + 2) % 3;
        if (in.pressed & K_RIGHT)
            s.difficulty = (s.difficulty + 1) % 3;
        if (in.pressed & K_ACTION)
            begin();
        return;
    }
    if (s.state != PLAY) {
        if (in.pressed & K_ACTION)
            begin();
        return;
    }
    if (in.pressed & K_UP)
        turn(UP);
    if (in.pressed & K_DOWN)
        turn(DOWN);
    if (in.pressed & K_LEFT)
        turn(LEFT);
    if (in.pressed & K_RIGHT)
        turn(RIGHT);
    s.time_ms += GAME_STEP_MS;
    if (s.bonus != NONE) {
        if (s.bonus_ms <= GAME_STEP_MS) {
            s.bonus = NONE;
            s.bonus_ms = 0;
            if (s.food == NONE)
                s.food = pick_free();
        } else
            s.bonus_ms -= GAME_STEP_MS;
    }
    static const int speeds[] = {360, 240, 150};
    int interval = speeds[s.difficulty] - 20 * (s.eaten / 5);
    if (interval < 80)
        interval = 80;
    s.move_ms += GAME_STEP_MS;
    if (s.move_ms >= interval) {
        s.move_ms -= interval;
        move();
    }
}
static int px(int cell) {
    return 4 + (cell % COLS) * 4;
}
static int py(int cell) {
    return 18 + (cell / COLS) * 4;
}
static void draw(void) {
    if (s.state == SETUP) {
        gfx_text(48, 2, "SNAKE");
        gfx_text(3, 16, "U/D MODE");
        gfx_text(52, 16, s.mode ? "OBSTACLE" : "CLASSIC");
        const char *levels[] = {"EASY", "NORMAL", "HARD"};
        gfx_text(3, 26, "L/R SPEED");
        gfx_text(52, 26, levels[s.difficulty]);
        gfx_text(3, 40, "MODE BEST");
        gfx_number(52, 40, best[s.mode]);
        gfx_text(32, 54, "A TO START");
        return;
    }
    gfx_text(2, 1, "S");
    gfx_number(9, 1, s.score);
    gfx_text(33, 1, "HI");
    gfx_number(44, 1, best[s.mode]);
    gfx_text(68, 1, "TIME");
    gfx_number(88, 1, s.time_ms / 1000);
    gfx_text(2, 9, s.mode ? "OBSTACLE" : "CLASSIC");
    if (s.bonus != NONE) {
        gfx_text(68, 9, "BONUS");
        gfx_number(94, 9, (s.bonus_ms + 999) / 1000);
    }
    gfx_box(2, 16, 124, 48);
    if (s.mode)
        for (int i = 0; i < CELLS; i++)
            if (stone(i))
                gfx_box(px(i), py(i), 4, 4);
    for (unsigned i = 0; i < s.len; i++)
        gfx_rect(px(s.body[i]), py(s.body[i]), 3, 3, 1);
    gfx_rect(px(s.body[0]) + 1, py(s.body[0]) + 1, 1, 1, 0);
    if (s.food != NONE)
        gfx_box(px(s.food), py(s.food), 3, 3);
    if (s.bonus != NONE) {
        int x = px(s.bonus), y = py(s.bonus);
        gfx_rect(x + 1, y, 1, 4, 1);
        gfx_rect(x, y + 1, 4, 1, 1);
    }
    if (s.state == OVER || s.state == WON)
        gfx_panel(s.state == OVER ? "GAME OVER" : "YOU WIN", "A TO RESTART");
}
const Game snake_game = {enter, step, draw};
