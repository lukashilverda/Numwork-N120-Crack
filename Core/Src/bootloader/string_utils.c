/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : string_utils.c
  * @brief          : String utility implementation
  ******************************************************************************
  */
/* USER CODE END Header */

#include "string_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

char *uint32_to_str(uint32_t value) {
    char *buffer = (char *)malloc(11 * sizeof(char));
    if (buffer == NULL) return NULL;

    snprintf(buffer, 11, "%lu", value);
    return buffer;
}

char *concat_strings(int num, ...) {
    if (num <= 0) return NULL;

    va_list args;
    va_start(args, num);

    // Calculate total required size
    size_t total_length = 0;
    for (int i = 0; i < num; i++) {
        const char *str = va_arg(args, const char*);
        if (!str) str = "";
        total_length += strlen(str);
    }

    va_end(args);

    // Allocate memory for final string
    char *result = (char *)malloc(total_length + 1);
    if (!result) return NULL;

    result[0] = '\0';

    // Concatenate all strings
    va_start(args, num);
    for (int i = 0; i < num; i++) {
        const char *str = va_arg(args, const char*);
        if (!str) str = "";
        strcat(result, str);
    }
    va_end(args);

    return result;
}
