#include "oled.h"
#include "gfx.h"
#include "i2c.h"
#include <stdbool.h>
#include <string.h>
// Change the 7-bit address to 0x3D for modules strapped that way.
#define OLED_ADDR_7BIT 0x3Cu
enum { WAIT_POWER, INIT_TX, READY, FRAME_TX, BACKOFF };
enum { TX_PENDING, TX_OK, TX_ERROR };
static uint8_t state = WAIT_POWER;
static volatile uint8_t result;
static uint32_t started;
static bool dirty;
static uint8_t tx[FRAME_BYTES + 1], latest[FRAME_BYTES];
// Horizontal addressing wraps back to column 0, page 0 after each full frame.
static uint8_t init_cmd[] = {0x00, 0xAE, 0xD5, 0x80, 0xA8, 0x3F, 0xD3, 0,    0x40, 0x8D, 0x14,
                             0x20, 0,    0xA1, 0xC8, 0xDA, 0x12, 0x81, 0x7F, 0xD9, 0xF1, 0xDB,
                             0x40, 0xA4, 0xA6, 0x21, 0,    127,  0x22, 0,    7,    0xAF};
void HAL_I2C_MasterTxCpltCallback(I2C_HandleTypeDef *bus) {
    if (bus == &hi2c1)
        result = TX_OK;
}
void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *bus) {
    if (bus == &hi2c1)
        result = TX_ERROR;
}
void oled_present(const uint8_t *pixels) {
    memcpy(latest, pixels, FRAME_BYTES);
    dirty = true;
}
static void start_tx(uint8_t *data, uint16_t length, uint8_t phase, uint32_t now) {
    started = now;
    state = phase;
    result = TX_PENDING;
    if (__HAL_I2C_GET_FLAG(&hi2c1, I2C_FLAG_BUSY) ||
        HAL_I2C_Master_Transmit_IT(&hi2c1, OLED_ADDR_7BIT << 1, data, length) != HAL_OK)
        result = TX_ERROR;
}
void oled_service(uint32_t now) {
    switch (state) {
    case WAIT_POWER:
        if (now - started >= 100)
            start_tx(init_cmd, sizeof init_cmd, INIT_TX, now);
        break;
    case INIT_TX:
    case FRAME_TX:
        if (result == TX_OK) {
            state = READY;
            break;
        }
        if (result == TX_ERROR || now - started >= 100) {
            // Cancel stalled EV/ER IRQ transfers before reusing their buffers.
            HAL_I2C_DeInit(&hi2c1);
            __HAL_RCC_I2C1_FORCE_RESET();
            __HAL_RCC_I2C1_RELEASE_RESET();
            state = BACKOFF;
            started = now;
            dirty = true;
        }
        break;
    case BACKOFF:
        if (now - started >= 500) {
            MX_I2C1_Init();
            state = WAIT_POWER;
            started = now;
        }
        break;
    case READY:
        if (dirty) {
            tx[0] = 0x40;
            memcpy(tx + 1, latest, FRAME_BYTES);
            dirty = false;
            start_tx(tx, sizeof tx, FRAME_TX, now);
        }
        break;
    }
}
