/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : usb_comms.c
  * @brief          : USB communication implementation
  ******************************************************************************
  */
/* USER CODE END Header */

#include "usb_comms.h"
#include "led_control.h"
#include "timer_utils.h"
#include "main.h"
#include "usbd_cdc_if.h"
#include <string.h>

// Function to send a message over USB and flash the LED yellow for 300 ms:
void sendMessage(char *message) {
  uint16_t len = strlen(message);
  CDC_Transmit_HS((uint8_t *)message, len);
  extern void (*timeoutCallback)();
  timeoutCallback = NULL;
  setLedColor((Color){255, 255, 0});
  setTimeout(resetLed, 300);
}
