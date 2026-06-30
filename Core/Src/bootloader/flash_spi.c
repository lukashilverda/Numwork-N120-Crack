/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : flash_spi.c
  * @brief          : OctoSPI flash memory implementation
  ******************************************************************************
  */
/* USER CODE END Header */

#include "flash_spi.h"
#include "usb_comms.h"
#include "main.h"
#include "stm32h7xx_hal.h"
#include <string.h>

extern OSPI_HandleTypeDef hospi1;

void OctoSPI_SendCommand(uint8_t instruction, uint8_t *data, uint16_t dataSize) {
    OSPI_RegularCmdTypeDef sCommand = {0};

    sCommand.OperationType      = HAL_OSPI_OPTYPE_COMMON_CFG;
    sCommand.FlashId            = HAL_OSPI_FLASH_ID_1;
    sCommand.Instruction        = instruction;
    sCommand.InstructionMode    = HAL_OSPI_INSTRUCTION_1_LINE;
    sCommand.AddressMode        = HAL_OSPI_ADDRESS_NONE;
    sCommand.AlternateBytesMode = HAL_OSPI_ALTERNATE_BYTES_NONE;
    sCommand.DataMode           = HAL_OSPI_DATA_1_LINE;
    sCommand.NbData             = dataSize;
    sCommand.DummyCycles        = 0;
    sCommand.DQSMode            = HAL_OSPI_DQS_DISABLE;
    sCommand.SIOOMode           = HAL_OSPI_SIOO_INST_EVERY_CMD;

    if (HAL_OSPI_Command(&hospi1, &sCommand, HAL_MAX_DELAY) != HAL_OK) {
        Error_Handler();
    }

    if (HAL_OSPI_Receive(&hospi1, data, HAL_MAX_DELAY) != HAL_OK) {
    	Error_Handler();
    }
}

uint8_t ReadFlashID(void) {
	uint8_t id[3] = {0};
	OctoSPI_SendCommand(CMD_RDID, id, 3);
	return id[0];
}

void EnableQuadSPI()
{
    OSPI_RegularCmdTypeDef sCommand;
    uint8_t reg;

    // Read status register
    memset(&sCommand, 0, sizeof(sCommand));
    sCommand.OperationType = HAL_OSPI_OPTYPE_COMMON_CFG;
    sCommand.FlashId = HAL_OSPI_FLASH_ID_1;
    sCommand.Instruction = 0x05;
    sCommand.InstructionMode = HAL_OSPI_INSTRUCTION_1_LINE;
    sCommand.DataMode = HAL_OSPI_DATA_1_LINE;
    sCommand.NbData = 1;

    HAL_OSPI_Command(&hospi1, &sCommand, HAL_MAX_DELAY);
    HAL_OSPI_Receive(&hospi1, &reg, HAL_MAX_DELAY);

    // Enable Quad Enable Bit (QE = bit 6)
    reg |= (1 << 6);

    // Write status register
    sCommand.Instruction = 0x01;
    HAL_OSPI_Command(&hospi1, &sCommand, HAL_MAX_DELAY);
    HAL_OSPI_Transmit(&hospi1, &reg, HAL_MAX_DELAY);

    sendMessage("Quad SPI activé !\n");
}
