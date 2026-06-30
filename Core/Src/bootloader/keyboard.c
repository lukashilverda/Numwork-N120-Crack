/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : keyboard.c
  * @brief          : Keyboard matrix scanning implementation
  ******************************************************************************
  */
/* USER CODE END Header */

#include "keyboard.h"
#include "main.h"
#include "stm32h7xx_hal.h"

extern uint16_t rowPins[];
extern GPIO_TypeDef *rowPorts[];
extern uint16_t colPins[];
extern GPIO_TypeDef *colPorts[];

//Function to check if a key is pressed on the keyboard:
int keydown(KeyboardPos pos) {
	uint16_t rowPin = rowPins[pos.row];
	GPIO_TypeDef* rowPort = rowPorts[pos.row];
	uint16_t colPin = colPins[pos.col];
	GPIO_TypeDef* colPort = colPorts[pos.col];
	HAL_GPIO_WritePin(colPort, colPin, GPIO_PIN_SET);
	GPIO_PinState down = HAL_GPIO_ReadPin(rowPort, rowPin);
	HAL_GPIO_WritePin(colPort, colPin, GPIO_PIN_RESET);
	return down;
}
