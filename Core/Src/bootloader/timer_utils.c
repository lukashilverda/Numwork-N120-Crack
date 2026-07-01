/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : timer_utils.c
  * @brief          : Timer utility implementation
  ******************************************************************************
  */
/* USER CODE END Header */

#include "timer_utils.h"
#include "main.h"
#include "stm32h7xx_hal.h"

extern TIM_HandleTypeDef htim2;
extern void (*timeoutCallback)();

//Function to set a timeout with a callback function:
void setTimeout(void (*callback)(), uint32_t delay_ms) {
    timeoutCallback = callback;
    __HAL_TIM_SET_AUTORELOAD(&htim2, delay_ms - 1);
    __HAL_TIM_SET_COUNTER(&htim2, 0);
    HAL_TIM_Base_Start_IT(&htim2);
}

// Callback function called when the timer period elapses
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
    if (htim == &htim2) {
        HAL_TIM_Base_Stop_IT(&htim2);
        if (timeoutCallback) timeoutCallback();
    }
}
