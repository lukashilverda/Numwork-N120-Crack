/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : string_utils.h
  * @brief          : String utility functions
  ******************************************************************************
  */
/* USER CODE END Header */

#ifndef __STRING_UTILS_H
#define __STRING_UTILS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/* Function prototypes -------------------------------------------------------*/
char *uint32_to_str(uint32_t value);
char *concat_strings(int num, ...);

#ifdef __cplusplus
}
#endif

#endif /* __STRING_UTILS_H */
