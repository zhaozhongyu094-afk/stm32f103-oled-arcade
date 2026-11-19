#ifndef BSP_OLED_H
#define BSP_OLED_H
#include <stdint.h>
void oled_service(uint32_t now);
void oled_present(const uint8_t *pixels);
#endif
