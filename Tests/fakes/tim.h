#ifndef TEST_TIM_H
#define TEST_TIM_H
#include <assert.h>
typedef struct {
    int dummy;
} TIM_HandleTypeDef;
extern TIM_HandleTypeDef htim2;
#define HAL_OK 0
static inline int HAL_TIM_Base_Start_IT(TIM_HandleTypeDef *t) { return t != &htim2; }
#define Error_Handler() assert(0)
#endif
