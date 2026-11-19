#include "arcade.h"
#include "gfx.h"
typedef struct {
    float x;
    int w, h;
    bool passed;
} Barrier;
static struct {
    float height, rise, travel;
    Barrier obstacles[3];
    unsigned count, score;
    uint8_t state;
} r;
static unsigned best, seed = 317;
enum { READY, RUNNING, CRASH };
static void enter(void) {
    r.state = READY;
    r.height = r.rise = r.travel = 0;
    r.count = 1;
    r.score = 0;
    r.obstacles[0] = (Barrier){100, 6, 8, false};
}
static void step(Input in) {
    if (r.state != RUNNING) {
        if (in.pressed & K_ACTION) {
            enter();
            r.state = RUNNING;
        }
        return;
    }
    if ((in.pressed & (K_ACTION | K_UP)) && r.height == 0 && r.rise == 0)
        r.rise = 110;
    const float dt = GAME_STEP_MS / 1000.f;
    float speed = r.score >= 13 ? 66.f : 40.f + 2 * r.score;
    if (r.height > 0 || r.rise > 0) {
        r.height += r.rise * dt - 145 * dt * dt;
        r.rise -= 290 * dt;
        if (r.height <= 0) {
            r.height = 0;
            r.rise = 0;
        }
    }
    r.travel += speed * dt;
    if (r.travel >= 44)
        r.travel -= 44;
    for (unsigned i = 0; i < r.count; i++) {
        Barrier *b = &r.obstacles[i];
        b->x -= speed * dt;
        if (b->x < 25 && b->x + b->w > 19 && r.height < b->h) {
            r.state = CRASH;
            return;
        }
        if (!b->passed && b->x + b->w <= 19) {
            b->passed = true;
            r.score++;
            if (r.score > best)
                best = r.score;
        }
    }
    if (r.obstacles[0].x + r.obstacles[0].w < 0) {
        for (unsigned i = 1; i < r.count; i++)
            r.obstacles[i - 1] = r.obstacles[i];
        r.count--;
    }
    if (!r.count || r.obstacles[r.count - 1].x < 52) {
        seed = 1664525u * seed + 1013904223u;
        bool wide = (seed >> 29) < 4;
        r.obstacles[r.count++] =
            (Barrier){128 + (int)((seed >> 8) % 16), wide ? 9 : 5, wide ? 6 : 10, false};
    }
}
static void draw(void) {
    static const uint8_t robot[] = {62, 35, 63, 14, 31, 47, 14};
    gfx_text(2, 1, "S");
    gfx_number(10, 1, r.score);
    gfx_text(75, 1, "HI");
    gfx_number(89, 1, best);
    gfx_rect(1, 9, 126, 1, 1);
    gfx_rect(0, 53, 128, 1, 1);
    for (int x = 0; x < 128; x += 2) {
        int t = (x + (int)r.travel / 4) % 38;
        gfx_rect(x, 47 - (t < 19 ? t : 38 - t) / 3, 1, 1, 1);
    }
    for (int x = 4 - (int)r.travel % 11; x < 128; x += 11)
        gfx_rect(x, 57, 3, 1, 1);
    int foot = 53 - (int)r.height;
    gfx_sprite(17, foot - 9, robot, 6, 7, 0);
    gfx_rect(19, foot - 2, 2, 2, 1);
    gfx_rect(23 + (int)(r.travel / 4) % 2, foot - 2, 2, 2, 1);
    for (unsigned i = 0; i < r.count; i++) {
        Barrier b = r.obstacles[i];
        gfx_box((int)b.x, 53 - b.h, b.w, b.h);
    }
    if (r.state == READY)
        gfx_panel("HURDLE RUN", "A START  A/UP JUMP");
    if (r.state == CRASH)
        gfx_panel("GAME OVER", "A TO RETRY");
}
const Game runner_game = {enter, step, draw};
