/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : led_control.h
  * @brief          : LED control functionality
  ******************************************************************************
  */
/* USER CODE END Header */

#ifndef __LED_CONTROL_H
#define __LED_CONTROL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

/* Function prototypes -------------------------------------------------------*/
void setLedColor(Color color);
void resetLed(void);

#ifdef __cplusplus
}
#endif

#endif /* __LED_CONTROL_H */
