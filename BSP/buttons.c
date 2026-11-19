#include "buttons.h"
#include "main.h"
enum { DEBOUNCE_MS = 18 };
static volatile uint8_t menu_level;
static uint8_t stable, candidate;
static uint32_t changed[6];
void buttons_init(void) {
    menu_level = HAL_GPIO_ReadPin(KEY_MENU_GPIO_Port, KEY_MENU_Pin) == GPIO_PIN_RESET;
}
// Both EXTI edges capture the menu switch level; bounce is filtered in foreground.
void HAL_GPIO_EXTI_Callback(uint16_t pin) {
    if (pin == KEY_MENU_Pin)
        menu_level = HAL_GPIO_ReadPin(KEY_MENU_GPIO_Port, pin) == GPIO_PIN_RESET;
}
Keys buttons_poll(uint32_t now) {
    // PA0..PA4 have the same bit order as the five logical controls.
    uint8_t raw = (uint8_t)((~GPIOA->IDR) & 31u);
    if (menu_level)
        raw |= K_MENU;
    Keys out = {0};
    for (unsigned i = 0; i < 6; i++) {
        uint8_t mask = (uint8_t)(1u << i);
        if ((raw ^ candidate) & mask) {
            candidate ^= mask;
            changed[i] = now;
        }
        if (((stable ^ candidate) & mask) && now - changed[i] >= DEBOUNCE_MS) {
            stable ^= mask;
            if (stable & mask)
                out.pressed |= mask;
            else
                out.released |= mask;
        }
    }
    out.held = stable;
    return out;
}
