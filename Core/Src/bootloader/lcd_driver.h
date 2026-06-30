/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : lcd_driver.h
  * @brief          : LCD display driver functionality
  ******************************************************************************
  */
/* USER CODE END Header */

#ifndef __LCD_DRIVER_H
#define __LCD_DRIVER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include <stdint.h>

typedef enum {
  Nop = 0x00,
  Reset = 0x01,
  ReadDisplayID = 0x04,
  SleepIn = 0x10,
  SleepOut = 0x11,
  DisplayInversionOff = 0x20,
  DisplayInversionOn = 0x21,
  DisplayOff = 0x28,
  DisplayOn = 0x29,
  ColumnAddressSet = 0x2A,
  PageAddressSet = 0x2B,
  MemoryWrite = 0x2C,
  MemoryRead = 0x2E,
  TearingEffectLineOn = 0x35,
  MemoryAccessControl = 0x36,
  PixelFormatSet = 0x3A,
  FrameRateControl = 0xC6,
  PositiveVoltageGammaControl = 0xE0,
  NegativeVoltageGammaControl = 0xE1
} Command;

/* Function prototypes -------------------------------------------------------*/
void LCD_WriteCommand(Command cmd);
void LCD_WriteData(uint8_t data);
void LCD_Init(void);
void LCD_DrawPixel(uint16_t x, uint16_t y, uint16_t color);
void LCD_FillRect(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color);
void LCD_DrawString(uint16_t x, uint16_t y, char* text, uint16_t color1, uint16_t color2);
void LCD_SetBrightness(int brightness);
uint16_t RGB565(uint8_t r, uint8_t g, uint8_t b);
uint32_t panelIdentifier(void);

#ifdef __cplusplus
}
#endif

#endif /* __LCD_DRIVER_H */
