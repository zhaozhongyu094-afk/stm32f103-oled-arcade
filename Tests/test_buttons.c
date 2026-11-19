#include "../BSP/buttons.c"
#include <assert.h>
GPIO_TypeDef test_gpio = {63};
int main(void) {
    uint32_t t = UINT32_MAX - 10u;
    buttons_init();
    buttons_poll(t);
    test_gpio.IDR &= ~K_LEFT;
    buttons_poll(t + 1);
    test_gpio.IDR |= K_LEFT;
    buttons_poll(t + 3);
    test_gpio.IDR &= ~K_LEFT;
    buttons_poll(t + 6);
    assert(buttons_poll(t + 23).pressed == 0);
    Keys keys = buttons_poll(t + 24);
    assert(keys.held == K_LEFT && keys.pressed == K_LEFT);
    assert(!buttons_poll(t + 100).pressed);
    test_gpio.IDR &= ~K_ACTION;
    buttons_poll(t + 101);
    assert(buttons_poll(t + 119).held == (K_LEFT | K_ACTION));
    test_gpio.IDR &= ~K_MENU;
    HAL_GPIO_EXTI_Callback(KEY_MENU_Pin);
    buttons_poll(t + 120);
    assert(buttons_poll(t + 138).pressed == K_MENU);
    test_gpio.IDR |= K_MENU | K_ACTION | K_LEFT;
    HAL_GPIO_EXTI_Callback(KEY_MENU_Pin);
    buttons_poll(t + 150);
    assert(buttons_poll(t + 168).released == (K_MENU | K_ACTION | K_LEFT));
    return 0;
}
