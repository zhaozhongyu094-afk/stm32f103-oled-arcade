#include "../Games/runner.c"
#include <assert.h>
int main(void) {
    enter();
    step((Input){0, K_ACTION});
    while (r.obstacles[0].x > 32)
        step((Input){0, 0});
    step((Input){K_ACTION, K_ACTION});
    for (unsigned i = 0; i < 130; i++)
        step((Input){K_ACTION, 0});
    assert(r.state == RUNNING && r.score == 1 && best == 1 && r.height == 0);
    for (unsigned i = 0; i < 600 && r.state == RUNNING; i++)
        step((Input){0, 0});
    assert(r.state == CRASH);
    step((Input){0, K_ACTION});
    assert(r.state == RUNNING && r.score == 0 && best == 1);
    return 0;
}
