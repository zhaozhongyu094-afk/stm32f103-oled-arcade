#include "arcade.h"
#include "gfx.h"
#include <string.h>
typedef struct {
    int x, y, w, h;
} Tile;
typedef struct {
    int x, y;
} Point;
typedef struct {
    const char *name;
    int end, flag, tiles;
    Tile solid[12];
    Point stamps[3];
    int bugs;
    Point patrol[2]; // x/y store the left/right ends of each patrol.
} Stage;
static const Stage stages[] = {{"RIVER",
                                300,
                                122,
                                6,
                                {{0, 56, 86, 8},
                                 {102, 56, 90, 8},
                                 {212, 56, 100, 8},
                                 {48, 48, 16, 8},
                                 {144, 48, 16, 8},
                                 {240, 48, 16, 8}},
                                {{54, 37}, {148, 36}, {246, 36}},
                                0,
                                {{0, 0}}},
                               {"BRIDGE",
                                342,
                                108,
                                9,
                                {{0, 56, 83, 8},
                                 {101, 56, 68, 8},
                                 {190, 56, 56, 8},
                                 {264, 56, 90, 8},
                                 {38, 48, 16, 8},
                                 {54, 40, 16, 16},
                                 {122, 48, 14, 8},
                                 {200, 47, 18, 9},
                                 {280, 46, 20, 10}},
                                {{59, 29}, {206, 35}, {289, 35}},
                                1,
                                {{314, 323}}},
                               {"SUMMIT",
                                367,
                                207,
                                11,
                                {{0, 56, 92, 8},
                                 {112, 56, 68, 8},
                                 {202, 56, 64, 8},
                                 {287, 56, 92, 8},
                                 {42, 48, 16, 8},
                                 {58, 40, 16, 16},
                                 {125, 46, 20, 10},
                                 {216, 48, 16, 8},
                                 {232, 40, 16, 16},
                                 {303, 48, 14, 8},
                                 {317, 40, 17, 16}},
                                {{63, 29}, {133, 35}, {321, 29}},
                                2,
                                {{151, 170}, {339, 355}}}};
enum { TITLE, WALK, HURT, LOST, DELIVERED, ALL_SENT };
typedef struct {
    float x, v;
    bool alive;
} Bug;
static struct {
    unsigned stage, state, lives, bag, total, anim;
    float x, y, vy, grace;
    bool ground, left, checkpoint;
    Bug bugs[2];
} p;
static const Stage *level;
static bool touches(float x, float y, int width, int height) {
    return p.x < x + width && p.x + 7 > x && p.y < y + height && p.y + 10 > y;
}
static unsigned stamps(void) {
    return (p.bag & 1) + ((p.bag >> 1) & 1) + ((p.bag >> 2) & 1);
}
static void spawn(void) {
    p.x = p.checkpoint ? level->flag : 6;
    p.y = 46;
    p.vy = 0;
    p.grace = .065f;
    p.ground = true;
    p.left = false;
    for (int i = 0; i < level->bugs; i++)
        p.bugs[i] = (Bug){(float)level->patrol[i].x, 9, true};
}
static void load_stage(void) {
    level = &stages[p.stage];
    p.checkpoint = false;
    p.bag = 0;
    spawn();
}
static void enter(void) {
    memset(&p, 0, sizeof p);
    p.state = TITLE;
    p.lives = 3;
    load_stage();
}
static void hurt(void) {
    p.state = --p.lives ? HURT : LOST;
}
static void step(Input in) {
    if (p.state != WALK) {
        if (!(in.pressed & K_ACTION))
            return;
        switch (p.state) {
        case TITLE:
        case LOST:
        case ALL_SENT:
            enter();
            break;
        case DELIVERED:
            p.total += stamps();
            p.stage++;
            load_stage();
            break;
        case HURT:
            spawn();
            break;
        }
        p.state = WALK;
        return;
    }
    const float dt = GAME_STEP_MS / 1000.f;
    int dx = !!(in.held & K_RIGHT) - !!(in.held & K_LEFT);
    if (dx) {
        p.left = dx < 0;
        p.anim++;
    }
    if ((in.pressed & (K_ACTION | K_UP)) && p.grace > 0) {
        p.vy = -103;
        p.grace = 0;
    }
    float old_bottom = p.y + 10;
    p.x += dx * 43.f * dt;
    if (p.x < 0)
        p.x = 0;
    if (p.x > level->end + 3)
        p.x = (float)(level->end + 3);
    for (int i = 0; i < level->tiles; i++) {
        Tile b = level->solid[i];
        if (!touches(b.x, b.y, b.w, b.h))
            continue;
        if (dx > 0)
            p.x = b.x - 7;
        else if (dx < 0)
            p.x = b.x + b.w;
    }
    p.vy += 240.f * dt;
    p.y += p.vy * dt;
    p.ground = false;
    for (int i = 0; i < level->tiles; i++) {
        Tile b = level->solid[i];
        if (!touches(b.x, b.y, b.w, b.h))
            continue;
        if (p.vy > 0) {
            p.y = b.y - 10;
            p.ground = true;
        } else if (p.vy < 0)
            p.y = b.y + b.h;
        p.vy = 0;
    }
    p.grace = p.ground ? .065f : p.grace - dt;
    if (p.grace < 0)
        p.grace = 0;
    if (p.y > 66) {
        hurt();
        return;
    }
    for (int i = 0; i < level->bugs; i++) {
        Bug *b = &p.bugs[i];
        if (!b->alive)
            continue;
        Point range = level->patrol[i];
        b->x += b->v * dt;
        if (b->x >= range.y) {
            b->x = range.y;
            b->v = -9;
        }
        if (b->x <= range.x) {
            b->x = range.x;
            b->v = 9;
        }
        if (touches(b->x, 51, 7, 5)) {
            if (p.vy > 0 && old_bottom <= 52) {
                b->alive = false;
                p.y = 41;
                p.vy = -62;
            } else {
                hurt();
                return;
            }
        }
    }
    for (int i = 0; i < 3; i++)
        if (touches(level->stamps[i].x, level->stamps[i].y, 5, 6))
            p.bag |= 1u << i;
    if (p.x >= level->flag)
        p.checkpoint = true;
    if (p.x + 7 >= level->end && p.ground)
        p.state = p.stage == 2 ? ALL_SENT : DELIVERED;
}
static void draw(void) {
    static const uint8_t hero[] = {30, 63, 21, 31, 62, 126, 62, 30, 20, 22};
    static const uint8_t bug[] = {28, 62, 107, 62, 65}, stamp[] = {14, 17, 21, 17, 14};
    int cam = (int)(p.x - 43);
    if (cam < 0)
        cam = 0;
    if (cam > level->end - 118)
        cam = level->end - 118;
    gfx_text(2, 1, level->name);
    gfx_text(43, 1, "POST");
    gfx_number(63, 1, p.total + stamps());
    gfx_text(68, 1, "/9");
    gfx_text(102, 1, "L");
    gfx_number(112, 1, p.lives);
    gfx_rect(0, 8, 128, 1, 1);
    for (int x = 0; x < 128; x += 2) {
        int t = (x + cam / 4 + p.stage * 14) % 70;
        gfx_rect(x, 40 - (t < 35 ? t : 70 - t) / 2, 1, 1, 1);
    }
    for (int i = 0; i < level->tiles; i++) {
        Tile b = level->solid[i];
        gfx_box(b.x - cam, b.y, b.w, b.h);
    }
    int flag = level->flag - cam;
    gfx_rect(flag, 40, 1, 16, 1);
    gfx_box(flag, 40, 6, 4);
    if (p.checkpoint)
        gfx_rect(flag + 1, 41, 4, 2, 1);
    for (int i = 0; i < 3; i++)
        if (!(p.bag & (1u << i)))
            gfx_sprite(level->stamps[i].x - cam, level->stamps[i].y, stamp, 5, 5, 0);
    for (int i = 0; i < level->bugs; i++)
        if (p.bugs[i].alive)
            gfx_sprite((int)p.bugs[i].x - cam, 51, bug, 7, 5, 0);
    gfx_box(level->end - cam - 2, 43, 8, 6);
    gfx_rect(level->end - cam + 1, 49, 2, 7, 1);
    int x = (int)p.x - cam, y = (int)p.y;
    gfx_sprite(x, y, hero, 7, 10, p.left);
    if (p.ground && (p.anim / 15) % 2)
        gfx_rect(x + 1, y + 9, 2, 1, 1);
    switch (p.state) {
    case TITLE:
        gfx_panel("SUMMIT POST", "A START  L/R WALK");
        break;
    case HURT:
        gfx_panel("TRY AGAIN", p.checkpoint ? "A FROM FLAG" : "A FROM START");
        break;
    case LOST:
        gfx_panel("OUT OF LIVES", "A TO RESTART");
        break;
    case DELIVERED:
        gfx_panel("MAIL DELIVERED", "A FOR NEXT STAGE");
        break;
    case ALL_SENT:
        gfx_panel("ALL MAIL DELIVERED", "A TO PLAY AGAIN");
        break;
    }
}
const Game post_game = {enter, step, draw};
