/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : keyboard.h
  * @brief          : Keyboard matrix scanning functionality
  ******************************************************************************
  */
/* USER CODE END Header */

#ifndef __KEYBOARD_H
#define __KEYBOARD_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

/* Function prototypes -------------------------------------------------------*/
int keydown(KeyboardPos pos);

#ifdef __cplusplus
}
#endif

#endif /* __KEYBOARD_H */
