#include "../Games/snake.c"
#include <assert.h>
int main(void) {
    enter();
    step((Input){0, K_ACTION});
    turn(LEFT);
    assert(!s.queued);
    turn(UP);
    turn(LEFT);
    assert(s.queued == 2);
    move();
    assert(s.dir == UP);
    move();
    assert(s.dir == LEFT);
    begin();
    s.eaten = 4;
    s.food = s.body[0] + 1;
    move();
    assert(s.len == 4 && s.score == 1 && s.bonus != NONE && s.bonus_ms == 5000);
    s.food = 100;
    s.bonus = s.body[0] + 1;
    move();
    assert(s.len == 5 && s.score == 6 && s.bonus == NONE);
    s.bonus = 30;
    s.bonus_ms = 8;
    step((Input){0, 0});
    assert(s.bonus == NONE);
    s.mode = 1;
    begin();
    s.body[0] = 3 * COLS + 13;
    move();
    assert(s.state == OVER);
    s.mode = 0;
    begin();
    s.len = CELLS - 1;
    for (unsigned i = 0; i < s.len; i++)
        s.body[i] = (uint16_t)(i + 1);
    s.dir = LEFT;
    s.food = 0;
    move();
    assert(s.state == WON && s.len == CELLS);
    return 0;
}
