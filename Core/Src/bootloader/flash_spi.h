/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : flash_spi.h
  * @brief          : OctoSPI flash memory operations
  ******************************************************************************
  */
/* USER CODE END Header */

#ifndef __FLASH_SPI_H
#define __FLASH_SPI_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/* Defines -------------------------------------------------------------------*/
#define CMD_RDID  0x9F //Read JEDEC ID

/* Function prototypes -------------------------------------------------------*/
void OctoSPI_SendCommand(uint8_t instruction, uint8_t *data, uint16_t dataSize);
uint8_t ReadFlashID(void);
void EnableQuadSPI(void);

#ifdef __cplusplus
}
#endif

#endif /* __FLASH_SPI_H */
