/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : lcd_driver.h
  * @brief          : ST7789V LCD driver - parallel FMC/FSMC 16-bit interface
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

/* ST7789V command codes -------------------------------------------------------*/
typedef enum {
    Nop                          = 0x00,
    Reset                        = 0x01,
    ReadDisplayID                = 0x04,
    SleepIn                      = 0x10,
    SleepOut                     = 0x11,
    DisplayInversionOff          = 0x20,
    DisplayInversionOn           = 0x21,
    DisplayOff                   = 0x28,
    DisplayOn                    = 0x29,
    ColumnAddressSet             = 0x2A,
    PageAddressSet               = 0x2B,
    MemoryWrite                  = 0x2C,
    MemoryRead                   = 0x2E,
    TearingEffectLineOn          = 0x35,
    MemoryAccessControl          = 0x36,
    PixelFormatSet               = 0x3A,
    FrameRateControl             = 0xC6,
    PositiveVoltageGammaControl  = 0xE0,
    NegativeVoltageGammaControl  = 0xE1
} Command;

/* Screen geometry ------------------------------------------------------------*/
#define LCD_SCREEN_WIDTH   320U
#define LCD_SCREEN_HEIGHT  240U

/* Function prototypes --------------------------------------------------------*/

/** Initialise the ST7789V controller (reset, configure, display on, backlight on). */
void LCD_Init(void);

/** Low-level bus access (kept for callers that need direct register access). */
void LCD_WriteCommand(Command cmd);
void LCD_WriteData(uint8_t data);

/** Read the 24-bit panel identifier (RDID, command 0x04). */
uint32_t panelIdentifier(void);

/** Draw a single pixel at (x, y) with RGB565 colour. */
void LCD_DrawPixel(uint16_t x, uint16_t y, uint16_t color);

/** Fill a filled rectangle with RGB565 colour. */
void LCD_FillRect(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color);

/** Fill the entire screen with RGB565 colour. */
void LCD_FillScreen(uint16_t color);

/** Draw a null-terminated ASCII string at (x, y).
 *  color1 = foreground, color2 = background. */
void LCD_DrawString(uint16_t x, uint16_t y, char *text, uint16_t color1, uint16_t color2);

/** Pack 8-bit R, G, B components into a 16-bit RGB565 value. */
uint16_t RGB565(uint8_t r, uint8_t g, uint8_t b);

/** Control the backlight. Pass 0 for off, non-zero for on. */
void LCD_SetBrightness(int brightness);

#ifdef __cplusplus
}
#endif

#endif /* __LCD_DRIVER_H */
