#ifndef TEST_BOARD_H
#define TEST_BOARD_H
#include <stdint.h>
typedef struct {
    uint32_t IDR;
} GPIO_TypeDef;
extern GPIO_TypeDef test_gpio;
#define GPIOA (&test_gpio)
#define KEY_MENU_GPIO_Port GPIOA
#define KEY_MENU_Pin 32u
#define GPIO_PIN_RESET 0
static inline int HAL_GPIO_ReadPin(GPIO_TypeDef *port, uint16_t pin) { return !!(port->IDR & pin); }
#endif
