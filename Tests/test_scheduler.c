#include "../App/app.c"
#include "../App/console.c"
#include "../BSP/buttons.c"
#include <assert.h>
GPIO_TypeDef test_gpio = {63};
TIM_HandleTypeDef htim2;
static unsigned steps, frames, services;
static void game_enter(void) { steps = 0; }
static void game_step(Input keys) {
    (void)keys;
    steps++;
}
static void game_draw(void) {}
const Game snake_game = {game_enter, game_step, game_draw};
const Game runner_game = {game_enter, game_step, game_draw};
const Game post_game = {game_enter, game_step, game_draw};
void oled_present(const uint8_t *pixels) {
    (void)pixels;
    frames++;
}
void oled_service(uint32_t now) {
    (void)now;
    services++;
}
static void advance_ms(unsigned n) {
    while (n--) {
        HAL_TIM_PeriodElapsedCallback(&htim2);
        app_poll();
    }
}
static void menu_level_set(int high) {
    if (high)
        test_gpio.IDR |= K_MENU;
    else
        test_gpio.IDR &= ~K_MENU;
    HAL_GPIO_EXTI_Callback(KEY_MENU_Pin);
}
int main(void) {
    app_init();
    test_gpio.IDR &= ~K_ACTION;
    advance_ms(24);
    assert(active == &snake_game);
    test_gpio.IDR |= K_ACTION;
    advance_ms(24);
    menu_level_set(0);
    advance_ms(45);
    menu_level_set(1);
    advance_ms(25);
    assert(paused);
    unsigned before = steps;
    advance_ms(100);
    assert(steps == before);
    menu_level_set(0);
    advance_ms(45);
    menu_level_set(1);
    advance_ms(32);
    assert(!paused && steps > before);
    menu_level_set(0);
    advance_ms(730);
    assert(!active);
    menu_level_set(1);
    advance_ms(40);
    assert(!active && !paused);
    assert(frames > 24 && services > 1000);
    return 0;
}
