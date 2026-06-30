/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : lcd_driver.c
  * @brief          : LCD display driver implementation
  ******************************************************************************
  */
/* USER CODE END Header */

#include "lcd_driver.h"
#include "string_utils.h"
#include "usb_comms.h"
#include "main.h"
#include "stm32h7xx_hal.h"

/* FSMC Configuration Constants */
static const int FSMCMemoryBank = 1;
static const int FSMCDataCommandAddressBit = 16;
static const uint32_t FSMCBaseAddress = 0x60000000;
static const uint32_t FSMCBankAddress = FSMCBaseAddress + (FSMCMemoryBank-1)*0x04000000;

static volatile uint16_t * const CommandAddress = (uint16_t *)(FSMCBankAddress);
static volatile uint16_t * const DataAddress = (uint16_t *)(FSMCBankAddress | (1<<(FSMCDataCommandAddressBit+1)));

// Function to write a command to the LCD controller:
void LCD_WriteCommand(Command cmd)
{
  *(volatile uint16_t *)(CommandAddress) = cmd;
}

// Function to write data to the LCD controller:
void LCD_WriteData(uint8_t data)
{
    *(volatile uint16_t *)(DataAddress) = data;
}

//Function to read the panel identifier from the LCD controller:
uint32_t panelIdentifier() {
  LCD_WriteCommand(ReadDisplayID);
  *DataAddress; // Dummy read, per datasheet
  uint8_t id1 = *DataAddress;
  uint8_t id2 = *DataAddress;
  uint8_t id3 = *DataAddress;

  return (id1 << 16) | (id2 << 8) | id3;
}

void LCD_Init(void)
{
    // Reset the LCD
    // All initialization code is commented out
}

void LCD_DrawPixel(uint16_t x, uint16_t y, uint16_t color)
{
	sendMessage(concat_strings(7,"SET_PIXEL(",uint32_to_str(x),",",uint32_to_str(y),",",uint32_to_str(color),")"));
}

void LCD_FillRect(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color) {
	sendMessage(concat_strings(11,"FILL_RECT(",uint32_to_str(x),",",uint32_to_str(y),",",uint32_to_str(width),",",uint32_to_str(height),",",uint32_to_str(color),")"));
}

void LCD_DrawString(uint16_t x, uint16_t y, char* text, uint16_t color1, uint16_t color2) {
	sendMessage(concat_strings(11,"DRAW_STRING(",uint32_to_str(x),",",uint32_to_str(y),",",text,",",uint32_to_str(color1),",",uint32_to_str(color2),")"));
}

uint16_t RGB565(uint8_t r, uint8_t g, uint8_t b)
{
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}

void LCD_SetBrightness(int brightness) {
  for (int i=0; i<brightness; i++) {
	HAL_Delay(0.01);
	HAL_Delay(0.01);
  }
}
