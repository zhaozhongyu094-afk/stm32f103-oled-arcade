#include "app.h"
#include "buttons.h"
#include "console.h"
#include "gfx.h"
#include "oled.h"
#include "tim.h"
static volatile uint32_t ticks;
static uint32_t sample_at, step_at, frame_at, menu_at;
static uint8_t held, pending;
static bool home_sent;
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *timer) {
    if (timer == &htim2)
        ticks++;
}
void app_init(void) {
    buttons_init();
    console_home();
    if (HAL_TIM_Base_Start_IT(&htim2) != HAL_OK)
        Error_Handler();
    console_draw();
    oled_present(gfx_data());
}
void app_poll(void) {
    uint32_t now = ticks;
    if (now != sample_at) {
        sample_at = now;
        Keys k = buttons_poll(now);
        held = k.held;
        pending |= k.pressed;
        if (k.pressed & K_MENU) {
            menu_at = now;
            home_sent = false;
        }
        if ((held & K_MENU) && !home_sent && now - menu_at >= 700) {
            console_home();
            pending = 0;
            home_sent = true;
        }
        if ((k.released & K_MENU) && !home_sent) {
            console_pause();
            pending = 0;
        }
    }
    // Bound catch-up after a debugger stop while preserving the normal fixed step.
    if (now - step_at > 4 * GAME_STEP_MS)
        step_at = now - GAME_STEP_MS;
    while (now - step_at >= GAME_STEP_MS) {
        step_at += GAME_STEP_MS;
        console_step((Input){held, pending});
        pending = 0;
    }
    if (now - frame_at >= 33) {
        frame_at = now;
        console_draw();
        oled_present(gfx_data());
    }
    oled_service(now);
}
