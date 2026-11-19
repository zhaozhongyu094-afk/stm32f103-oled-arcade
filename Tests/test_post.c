#include "../Games/post.c"
#include <assert.h>
// Drive the actual MCU game with the same held-key / press-edge inputs as the board.
static void advance(unsigned count, uint8_t held, uint8_t pressed) {
    for (unsigned i = 0; i < count; i++) {
        step((Input){held, i == 0 ? pressed : 0});
    }
}
static void walk_to(float x) {
    for (unsigned i = 0; i < 2000 && p.state == WALK && p.x < x; i++)
        advance(1, K_RIGHT, 0);
    assert(p.state != WALK || p.x >= x);
}
int main(void) {
    const int jumps[][8] = {{40, 79, 131, 186, 228, -1},
                            {30, 68, 113, 165, 197, 239, 269, 304},
                            {33, 72, 118, 174, 208, 259, 295, 330}};
    enter();
    advance(1, 0, K_ACTION);
    for (unsigned level_no = 0; level_no < 3; level_no++) {
        for (int i = 0; i < 8 && jumps[level_no][i] > 0; i++) {
            walk_to(jumps[level_no][i]);
            assert(p.state == WALK);
            if (!p.ground)
                advance(150, 0, 0);
            assert(p.ground);
            advance(1, K_RIGHT, K_ACTION);
        }
        walk_to(400);
        assert(p.lives == 3 && stamps() == 3);
        assert(p.state == (level_no == 2 ? ALL_SENT : DELIVERED));
        if (level_no < 2)
            advance(1, 0, K_ACTION);
    }
    assert(p.total + stamps() == 9);
    advance(1, 0, K_ACTION);
    walk_to(40);
    advance(1, K_RIGHT, K_ACTION);
    walk_to(79);
    advance(1, K_RIGHT, K_ACTION);
    walk_to(131);
    advance(1, K_RIGHT, K_ACTION);
    walk_to(210);
    assert(p.state == HURT && p.lives == 2 && p.checkpoint);
    advance(1, 0, K_ACTION);
    assert(p.state == WALK && p.x == 122);
    return 0;
}
