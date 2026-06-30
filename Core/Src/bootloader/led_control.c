/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : led_control.c
  * @brief          : LED control implementation
  ******************************************************************************
  */
/* USER CODE END Header */

#include "led_control.h"
#include "main.h"
#include "stm32h7xx_hal.h"

extern TIM_HandleTypeDef htim3;
extern Color color;

// Function to set the color of the LED:
void setLedColor(Color c) {
  color = c;
  __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, color.Red*1000/255);
  __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, color.Green*1000/255);
  __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_3, color.Blue*1000/255);
}

// Function to reset the LED color to off (black):
void resetLed() {
  setLedColor((Color){0, 0, 0});
}
