/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : timer_utils.h
  * @brief          : Timer utility functions
  ******************************************************************************
  */
/* USER CODE END Header */

#ifndef __TIMER_UTILS_H
#define __TIMER_UTILS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/* Function prototypes -------------------------------------------------------*/
void setTimeout(void (*callback)(), uint32_t delay_ms);

#ifdef __cplusplus
}
#endif

#endif /* __TIMER_UTILS_H */
