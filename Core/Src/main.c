/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "usb_device.h"


/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "usbd_cdc_if.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define CMD_RDID  0x9F //Read JEDEC ID
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
//#define LCD_BASE    ((uint32_t *) 0x60000000)
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

OSPI_HandleTypeDef hospi1; //This is the handle for the OctoSPI peripheral, which is used to communicate with external flash memory or other devices that support the OctoSPI protocol.

TIM_HandleTypeDef htim2; //This is the handle for the TIM2 timer peripheral, which is used for timing and generating periodic events in the application.
TIM_HandleTypeDef htim3; //This is the handle for the TIM3 timer peripheral, which is used for timing and generating periodic events in the application.

SRAM_HandleTypeDef hsram1; //This is the handle for the SRAM peripheral, which is used to interface with external SRAM memory devices. It provides functions for reading and writing data to the SRAM.

/* USER CODE BEGIN PV */
uint16_t rowPins[] = {KBD_ROW_A_Pin, KBD_ROW_B_Pin, KBD_ROW_C_Pin, KBD_ROW_D_Pin, KBD_ROW_E_Pin, KBD_ROW_F_Pin, KBD_ROW_G_Pin, KBD_ROW_H_Pin, KBD_ROW_I_Pin};
GPIO_TypeDef *rowPorts[] = {KBD_ROW_A_GPIO_Port, KBD_ROW_B_GPIO_Port, KBD_ROW_C_GPIO_Port, KBD_ROW_D_GPIO_Port, KBD_ROW_E_GPIO_Port, KBD_ROW_F_GPIO_Port, KBD_ROW_G_GPIO_Port, KBD_ROW_H_GPIO_Port, KBD_ROW_I_GPIO_Port};
// This code defines two arrays, `rowPins` and `rowPorts`, which are used to represent the pins and ports associated with the rows of a keyboard matrix. Each element in the `rowPins` array corresponds to a specific pin connected to a row of the keyboard, while each element in the `rowPorts` array corresponds to the GPIO port associated with that pin. This setup allows for easy access and manipulation of the keyboard rows in the application.
uint16_t colPins[] = {KBD_COL_1_Pin, KBD_COL_2_Pin, KBD_COL_3_Pin, KBD_COL_4_Pin, KBD_COL_5_Pin, KBD_COL_6_Pin};
GPIO_TypeDef *colPorts[] = {KBD_COL_1_GPIO_Port, KBD_COL_2_GPIO_Port, KBD_COL_3_GPIO_Port, KBD_COL_4_GPIO_Port, KBD_COL_5_GPIO_Port, KBD_COL_6_GPIO_Port};

Color color = {255, 0, 0}; //simple color definition

const static int FSMCMemoryBank = 1; // This constant defines the memory bank number used for the Flexible Static Memory Controller (FSMC) in the application. The FSMC is a peripheral that allows the microcontroller to interface with external memory devices, such as SRAM, NOR flash, or NAND flash. By specifying the memory bank number, the application can correctly address and access the desired external memory region.
const static int FSMCDataCommandAddressBit = 16; // This constant defines the bit position used to differentiate between data and command addresses in the Flexible Static Memory Controller (FSMC) interface. In this case, the 16th bit is used to indicate whether an address corresponds to a data operation or a command operation. When accessing external memory devices through the FSMC, setting this bit appropriately allows the application to correctly interpret the type of operation being performed (data read/write or command execution).

const static uint32_t FSMCBaseAddress = 0x60000000; // This constant defines the base address of the Flexible Static Memory Controller (FSMC) in the microcontroller's memory map. The FSMC is a peripheral that allows the microcontroller to interface with external memory devices, such as SRAM, NOR flash, or NAND flash. The base address is used as a starting point for addressing external memory regions connected to the FSMC, enabling the application to read from and write to these external memory devices.
const static uint32_t FSMCBankAddress = FSMCBaseAddress + (FSMCMemoryBank-1)*0x04000000; // This constant defines the address of a specific memory bank within the Flexible Static Memory Controller (FSMC) in the microcontroller's memory map. The FSMC allows the microcontroller to interface with external memory devices, such as SRAM, NOR flash, or NAND flash. The address is calculated based on the base address of the FSMC and the memory bank number, allowing the application to access the desired external memory region associated with that specific bank.

static volatile uint16_t * const CommandAddress = (uint16_t *)(FSMCBankAddress); // This line defines a pointer to a volatile 16-bit unsigned integer that represents the command address for the Flexible Static Memory Controller (FSMC) in the microcontroller's memory map. The pointer is initialized to point to the base address of the specified FSMC memory bank. The `volatile` keyword indicates that the value at this address may change unexpectedly, such as when interacting with external hardware, and prevents the compiler from optimizing access to this address. The `const` qualifier indicates that the pointer itself cannot be modified to point to a different address.
static volatile uint16_t * const DataAddress = (uint16_t *)(FSMCBankAddress | (1<<(FSMCDataCommandAddressBit+1))); // This line defines a pointer to a volatile 16-bit unsigned integer that represents the data address for the Flexible Static Memory Controller (FSMC) in the microcontroller's memory map. The pointer is initialized to point to the base address of the specified FSMC memory bank, with an additional offset calculated using the `FSMCDataCommandAddressBit` constant. The `volatile` keyword indicates that the value at this address may change unexpectedly, such as when interacting with external hardware, and prevents the compiler from optimizing access to this address. The `const` qualifier indicates that the pointer itself cannot be modified to point to a different address.

void (*timeoutCallback)() = NULL; // This line defines a function pointer named `timeoutCallback` that points to a function with no parameters and no return value. It is initialized to `NULL`, indicating that it does not currently point to any valid function. This function pointer can be used to store a callback function that will be executed when a specific timeout event occurs in the application, allowing for flexible handling of time-based events.

typedef enum { // This code defines an enumeration named `Command` that represents various command codes used for controlling a display or other hardware device. Each command is associated with a specific hexadecimal value, which is typically sent to the device to perform certain actions, such as resetting the device, reading its ID, controlling display settings, and writing or reading memory. The enumeration provides a convenient way to refer to these command codes by name rather than by their numeric values.
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
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void); //Configures the system clock settings for the microcontroller, including the clock source, frequency, and any necessary prescalers or dividers. This function is typically generated by the STM32CubeMX tool and is called during the initialization phase of the application to ensure that the system clock is set up correctly for proper operation of the microcontroller and its peripherals.
static void MX_GPIO_Init(void); //Initializes the General Purpose Input/Output (GPIO) pins of the microcontroller. This function is typically generated by the STM32CubeMX tool and is called during the initialization phase of the application to configure the GPIO pins for their intended functions, such as input, output, or alternate functions, as well as setting their initial states and any necessary pull-up or pull-down resistors.
static void MX_TIM3_Init(void); //Initializes the TIM3 timer peripheral of the microcontroller. This function is typically generated by the STM32CubeMX tool and is called during the initialization phase of the application to configure the TIM3 timer for its intended use, such as generating PWM signals, measuring time intervals, or triggering events based on specific timing conditions. The function sets up the timer's prescaler, counter mode, period, and any necessary interrupt configurations.
static void MX_FMC_Init(void); //Initializes the Flexible Memory Controller (FMC) peripheral of the microcontroller. This function is typically generated by the STM32CubeMX tool and is called during the initialization phase of the application to configure the FMC for interfacing with external memory devices, such as SRAM, NOR flash, or NAND flash. The function sets up the FMC's timing parameters, memory type, data width, and any necessary control signals to ensure proper communication with the connected external memory.
static void MX_TIM2_Init(void); //Initializes the TIM2 timer peripheral of the microcontroller. This function is typically generated by the STM32CubeMX tool and is called during the initialization phase of the application to configure the TIM2 timer for its intended use, such as generating periodic interrupts, measuring time intervals, or triggering events based on specific timing conditions. The function sets up the timer's prescaler, counter mode, period, and any necessary interrupt configurations.
static void MX_OCTOSPI1_Init(void); //Initializes the OCTOSPI1 peripheral of the microcontroller. This function is typically generated by the STM32CubeMX tool and is called during the initialization phase of the application to configure the OCTOSPI1 interface for communication with external devices that support the OctoSPI protocol, such as flash memory or other peripherals. The function sets up the OctoSPI's clock settings, data width, chip select behavior, and any necessary timing parameters to ensure proper communication with the connected external device.
/* USER CODE BEGIN PFP */
void UART_SendChar(char c); //Sends a single character over UART (Universal Asynchronous Receiver-Transmitter) communication. This function is typically used to transmit data from the microcontroller to another device, such as a computer or another microcontroller, by sending one character at a time.
void UART_SendString(char *str); //Sends a null-terminated string over UART (Universal Asynchronous Receiver-Transmitter) communication. This function is typically used to transmit data from the microcontroller to another device, such as a computer or another microcontroller, by sending the entire string character by character until the null terminator is reached.
void setLedColor(Color color); //Sets the color of an LED (Light Emitting Diode) based on the provided `Color` structure, which typically contains red, green, and blue (RGB) components. This function is used to control the color output of an RGB LED by adjusting the intensity of each color channel according to the specified values in the `Color` structure.
int keydown(KeyboardPos pos); //Checks if a specific key on a keyboard matrix is currently pressed (down) based on the provided `KeyboardPos` structure, which typically contains row and column information. This function is used to detect user input from a keyboard by checking the state of the specified key in the matrix and returning a boolean value indicating whether the key is pressed or not.
void sendMessage(char *message); //Sends a message over a communication interface, such as UART or USB. This function is typically used to transmit data from the microcontroller to another device, such as a computer or another microcontroller, by sending the provided string message character by character until the null terminator is reached.
void LCD_WriteCommand(Command cmd); //Sends a command to an LCD (Liquid Crystal Display) controller based on the provided `Command` enumeration value. This function is used to control the behavior of the LCD by sending specific commands, such as turning the display on or off, setting pixel formats, or writing data to the display memory.
void LCD_WriteData(uint8_t data); //Sends a single byte of data to an LCD (Liquid Crystal Display) controller. This function is used to transmit data to the LCD, such as pixel values or configuration settings, by sending the provided byte of data to the display's data interface.
uint32_t panelIdentifier(); //Retrieves a unique identifier for the panel or device, typically used for identification or authentication purposes. This function may read specific hardware registers or memory locations to obtain a unique value that represents the panel or device, allowing the application to differentiate between different units or configurations.
void LCD_Init(void); //Initializes the LCD (Liquid Crystal Display) controller and prepares it for operation. This function typically configures the necessary hardware interfaces, sends initialization commands to the LCD, and sets up any required parameters, such as pixel formats, display orientation, and timing settings, to ensure proper functionality of the display.
void LCD_DrawPixel(uint16_t x, uint16_t y, uint16_t color); //Draws a single pixel on the LCD (Liquid Crystal Display) at the specified (x, y) coordinates with the given color value. This function is used to manipulate individual pixels on the display by writing the specified color data to the appropriate memory location corresponding to the pixel's position.
void LCD_SetBrightness(int brightness); //Sets the brightness level of the LCD (Liquid Crystal Display) backlight or display output. This function typically adjusts the intensity of the backlight or modifies display settings to achieve the desired brightness level, allowing users to customize the visual appearance of the display based on their preferences or environmental conditions.
uint16_t RGB565(uint8_t r, uint8_t g, uint8_t b); //Converts RGB color values (red, green, blue) into a 16-bit RGB565 format. The RGB565 format uses 5 bits for red, 6 bits for green, and 5 bits for blue, allowing for a total of 65,536 possible colors. This function takes the individual 8-bit RGB components as input and returns a single 16-bit value representing the color in RGB565 format, which can be used for display or graphics applications.
uint8_t ReadFlashID(void); //Reads the unique identifier of an external flash memory device connected to the microcontroller. This function typically sends a command to the flash memory to retrieve its ID, which can be used for identification, verification, or compatibility checks. The returned value is usually a byte that represents the manufacturer and device ID of the flash memory.
void OctoSPI_SendCommand(uint8_t instruction, uint8_t *data, uint16_t dataSize); //Sends a command to an external device using the OctoSPI (Octal Serial Peripheral Interface) protocol. This function takes an instruction byte, a pointer to the data to be sent, and the size of the data in bytes. It is used to communicate with devices that support the OctoSPI protocol, such as flash memory or other peripherals, by sending commands and associated data over the OctoSPI interface.
void EnableQuadSPI(); //Enables the QuadSPI (Quad Serial Peripheral Interface) functionality on the microcontroller. This function typically configures the necessary hardware settings and initializes the QuadSPI peripheral, allowing the microcontroller to communicate with external devices that support the QuadSPI protocol, such as flash memory or other peripherals, using four data lines for faster data transfer rates.

char *uint32_to_str(uint32_t value) {  //Short summary: This function converts a 32-bit unsigned integer into a string representation. It allocates memory for the string, formats the integer as a string, and returns a pointer to the resulting string. The caller is responsible for freeing the allocated memory when it is no longer needed.
    char *buffer = (char *)malloc(11 * sizeof(char));
    if (buffer == NULL) return NULL;

    snprintf(buffer, 11, "%lu", value);
    return buffer;
}

char *concat_strings(int num, ...) { //Short summary: This function concatenates multiple strings into a single dynamically allocated string. It takes the number of strings to concatenate as the first argument, followed by the strings themselves. The function calculates the total length required, allocates memory for the resulting string, and concatenates the input strings. The caller is responsible for freeing the allocated memory when it is no longer needed.
    if (num <= 0) return NULL;

    va_list args;
    va_start(args, num);

    // Calculer la taille totale nécessaire
    size_t total_length = 0;
    for (int i = 0; i < num; i++) {
        const char *str = va_arg(args, const char*);
        if (!str) str = ""; // Gérer les NULL comme des chaînes vides
        total_length += strlen(str);
    }

    va_end(args);

    // Allouer la mémoire pour la chaîne finale
    char *result = (char *)malloc(total_length + 1); // +1 pour '\0'
    if (!result) return NULL;

    result[0] = '\0'; // Initialiser à une chaîne vide

    // Concaténer toutes les chaînes
    va_start(args, num);
    for (int i = 0; i < num; i++) {
        const char *str = va_arg(args, const char*);
        if (!str) str = "";
        strcat(result, str);
    }
    va_end(args);

    return result; // L'utilisateur devra libérer la mémoire
}
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)  //BEGIN OF MAIN FUNCTION
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();    

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_TIM3_Init();
  MX_USB_DEVICE_Init();
  MX_FMC_Init();
  MX_TIM2_Init();
  MX_OCTOSPI1_Init();

  //Start PWM ouput on the 3 channels for the rgb led:
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1); 
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_3);

  //Flash the led yellow 2 times to indicate that the program has started
  setLedColor((Color){255, 255, 0});
  HAL_Delay(250);
  setLedColor((Color){0, 0, 0});
  HAL_Delay(250);
  setLedColor((Color){255, 255, 0});
  HAL_Delay(250);
  setLedColor((Color){0, 0, 0});
  HAL_Delay(250);

  
  HAL_GPIO_WritePin(GPIOE, GPIO_PIN_5, 1); //Reset the LCD
  HAL_Delay(10000);
  sendMessage(uint32_to_str_dynamic(panelIdentifier()));*/ //Send the panel identifier over USB (i dont know if this works)
  EnableQuadSPI(); //Enable QuadSPI to read the flash

  //Flash the led green 2 times to indicate that the program has started:
  setLedColor((Color){0, 255, 0});
  HAL_Delay(250);
  setLedColor((Color){0, 0, 0});
  HAL_Delay(250);
  setLedColor((Color){0, 255, 0});
  HAL_Delay(250);
  setLedColor((Color){0, 0, 0});
  HAL_Delay(250);

  
  //Commented out the LCD initialization and brightness setting, as well as the variable 'b' declaration, since they are not used in the current code.
  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  //LCD_Init();                
  //LCD_SetBrightness(1);
  //int b = 0;


  Color color = {255, 0, 0}; //define the color structure with red color
  setLedColor(color); //set the led color to red

  //Begin the main loop:
  while (1)
  {
	HAL_Delay(1000); //Wait for 1 second
	sendMessage(uint32_to_str(ReadFlashID())); //Send the flash ID over USB

  //          SOME COMMENTED OUT CODE BELOW, WHICH IS NOT USED IN THE CURRENT CODE (i dont know if it works, and i dont know if the comments are correct):

	//if(keydown(KEY_SHIFT)) LCD_DrawPixel(10, 10, RGB565(255, 0, 0)); //should draw a red pixel at (10, 10) if the shift key is pressed
	//if(keydown(KEY_ALPHA)) LCD_DrawPixel(10, 10, RGB565(0, 255, 0)); //should draw a green pixel at (10, 10) if the alpha key is pressed
	//HAL_Delay(100); //Wait for 100 ms
	//LCD_WriteCommand(MemoryWrite); //Send the MemoryWrite command to the LCD controller, indicating that subsequent data will be written to the display memory.
	//LCD_WriteData(RGB565(255, 0, 0)); //Send the RGB565 color value for red to the LCD controller, which will be written to the display memory as pixel data, resulting in a red pixel being displayed on the screen.
	//LCD_DrawPixel(10, 10, RGB565(255, 0, 0)); //Draws a single pixel on the LCD at coordinates (10, 10) with the specified color value (RGB565 format for red). This function call updates the display memory to set the pixel at the given position to the desired color, resulting in a red pixel being displayed on the screen.
	//LCD_SetBrightness(b + 1); //  Sets the brightness level of the LCD backlight to the value of 'b' incremented by 1. This function call adjusts the intensity of the backlight, allowing for control over the display's brightness based on the current value of 'b'.
	//HAL_Delay(500);
	//b++;
	//if(b >= 13) b=13;
	/*if(keydown((KeyboardPos){8, 0})) {

  // Send messages when keys are pressed: 

	}*/
	/*if(keydown(KEY_LEFT)) sendMessage("LEFT\r\n"); 
	if(keydown(KEY_UP)) sendMessage("UP\r\n");
	if(keydown(KEY_DOWN)) sendMessage("DOWN\r\n");
	if(keydown(KEY_RIGHT)) sendMessage("RIGHT\r\n");
	if(keydown(KEY_OK)) sendMessage("OK\r\n");
	if(keydown(KEY_BACK)) sendMessage("BACK\r\n");
	if(keydown(KEY_HOME)) sendMessage("HOME\r\n");
	if(keydown(KEY_ONOFF)) sendMessage("ONOFF\r\n");
	if(keydown(KEY_SHIFT)) sendMessage("SHIFT\r\n");
	if(keydown(KEY_ALPHA)) sendMessage("ALPHA\r\n");
	if(keydown(KEY_XNT)) sendMessage("XNT\r\n");
	if(keydown(KEY_VAR)) sendMessage("VAR\r\n");
	if(keydown(KEY_TOOLBOX)) sendMessage("TOOLBOX\r\n");
	if(keydown(KEY_BACKSPACE)) sendMessage("BACKSPACE\r\n");
	if(keydown(KEY_EXP)) sendMessage("EXP\r\n");
	if(keydown(KEY_LN)) sendMessage("LN\r\n");
	if(keydown(KEY_LOG)) sendMessage("LOG\r\n");
	if(keydown(KEY_IMAGINARY)) sendMessage("IMAGINARY\r\n");
	if(keydown(KEY_COMMA)) sendMessage("COMMA\r\n");
	if(keydown(KEY_POWER)) sendMessage("POWER\r\n");
	if(keydown(KEY_SINE)) sendMessage("SINE\r\n");
	if(keydown(KEY_COSINE)) sendMessage("COSINE\r\n");
	if(keydown(KEY_TANGENT)) sendMessage("TANGENT\r\n");
	if(keydown(KEY_PI)) sendMessage("PI\r\n");
	if(keydown(KEY_SQRT)) sendMessage("SQRT\r\n");
	if(keydown(KEY_SQUARE)) sendMessage("SQUARE\r\n");
	if(keydown(KEY_SEVEN)) sendMessage("SEVEN\r\n");
	if(keydown(KEY_EIGHT)) sendMessage("EIGHT\r\n");
	if(keydown(KEY_NINE)) sendMessage("NINE\r\n");
	if(keydown(KEY_LEFTPARENTHESIS)) sendMessage("LEFTPARENTHESIS\r\n");
	if(keydown(KEY_RIGHTPARENTHESIS)) sendMessage("RIGHTPARENTHESIS\r\n");
	if(keydown(KEY_FOUR)) sendMessage("FOUR\r\n");
	if(keydown(KEY_FIVE)) sendMessage("FIVE\r\n");
	if(keydown(KEY_SIX)) sendMessage("SIX\r\n");
	if(keydown(KEY_MULTIPLICATION)) sendMessage("MULTIPLICATION\r\n");
	if(keydown(KEY_DIVISION)) sendMessage("DIVISION\r\n");
	if(keydown(KEY_ONE)) sendMessage("ONE\r\n");
	if(keydown(KEY_TWO)) sendMessage("TWO\r\n");
	if(keydown(KEY_THREE)) sendMessage("THREE\r\n");
	if(keydown(KEY_PLUS)) sendMessage("PLUS\r\n");
	if(keydown(KEY_MINUS)) sendMessage("MINUS\r\n");
	if(keydown(KEY_ZERO)) sendMessage("ZERO\r\n");
	if(keydown(KEY_DOT)) sendMessage("DOT\r\n");
	if(keydown(KEY_EE)) sendMessage("EE\r\n");
	if(keydown(KEY_ANS)) sendMessage("ANS\r\n");
	if(keydown(KEY_EXE)) sendMessage("EXE\r\n");
	if(keydown(KEY_ONE)) sendMessage("ONE\r\n"); */
	//HAL_Delay(250);

  // The following code consists of a series of loops to get the led make rainbow:
	/*for (int i=0; i<255; i++) {
		color.Green += 1;
		setLedColor(color);
		HAL_Delay(2);
	}
	for (int i=0; i<255; i++) {
		color.Red -= 1;
		setLedColor(color);
		HAL_Delay(2);
	}
	for (int i=0; i<255; i++) {
		color.Blue += 1;
		setLedColor(color);
		HAL_Delay(2);
	}
	for (int i=0; i<255; i++) {
		color.Green -= 1;
		setLedColor(color);
		HAL_Delay(2);
	}
	for (int i=0; i<255; i++) {
		color.Red += 1;
		setLedColor(color);
		HAL_Delay(2);
	}
	for (int i=0; i<255; i++) {
		color.Blue -= 1;
		setLedColor(color);
		HAL_Delay(2);
	}*/
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)  //Configuring system clock settings
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0}; //Configure the oscillators
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0}; //Configure the CPU, AHB and APB buses clocks
  RCC_CRSInitTypeDef RCC_CRSInitStruct = {0}; //Configure the Clock Recovery System (CRS)

  /** Supply configuration update enable (what?)
  */
  HAL_PWREx_ConfigSupply(PWR_DIRECT_SMPS_SUPPLY); // I don't know wat the fuck this does

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1); 

  while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {} // Wait for voltage scaling flag to be ready (copilot inline suggestion, i dont know what this does)

  /** Configure LSE Drive Capability
  */
  HAL_PWR_EnableBkUpAccess();
  __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_LOW);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI48|RCC_OSCILLATORTYPE_HSI
                              |RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.HSIState = RCC_HSI_DIV1;
  RCC_OscInitStruct.HSICalibrationValue = 64;
  RCC_OscInitStruct.HSI48State = RCC_HSI48_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                              |RCC_CLOCKTYPE_D3PCLK1|RCC_CLOCKTYPE_D1PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }

  /** Enable the SYSCFG APB clock
  */
  __HAL_RCC_CRS_CLK_ENABLE();

  /** Configures CRS
  */
  RCC_CRSInitStruct.Prescaler = RCC_CRS_SYNC_DIV1;
  RCC_CRSInitStruct.Source = RCC_CRS_SYNC_SOURCE_LSE;
  RCC_CRSInitStruct.Polarity = RCC_CRS_SYNC_POLARITY_RISING;
  RCC_CRSInitStruct.ReloadValue = __HAL_RCC_CRS_RELOADVALUE_CALCULATE(48000000,32768);
  RCC_CRSInitStruct.ErrorLimitValue = 34;
  RCC_CRSInitStruct.HSI48CalibrationValue = 32;

  HAL_RCCEx_CRSConfig(&RCC_CRSInitStruct);
}

/**
  * @brief OCTOSPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_OCTOSPI1_Init(void)
{

  /* USER CODE BEGIN OCTOSPI1_Init 0 */

  /* USER CODE END OCTOSPI1_Init 0 */

  OSPIM_CfgTypeDef sOspiManagerCfg = {0};

  /* USER CODE BEGIN OCTOSPI1_Init 1 */

  /* USER CODE END OCTOSPI1_Init 1 */
  /* OCTOSPI1 parameter configuration*/
  hospi1.Instance = OCTOSPI1;
  hospi1.Init.FifoThreshold = 4;
  hospi1.Init.DualQuad = HAL_OSPI_DUALQUAD_DISABLE;
  hospi1.Init.MemoryType = HAL_OSPI_MEMTYPE_MACRONIX_RAM;
  hospi1.Init.DeviceSize = 23;
  hospi1.Init.ChipSelectHighTime = 2;
  hospi1.Init.FreeRunningClock = HAL_OSPI_FREERUNCLK_DISABLE;
  hospi1.Init.ClockMode = HAL_OSPI_CLOCK_MODE_0;
  hospi1.Init.WrapSize = HAL_OSPI_WRAP_NOT_SUPPORTED;
  hospi1.Init.ClockPrescaler = 4;
  hospi1.Init.SampleShifting = HAL_OSPI_SAMPLE_SHIFTING_NONE;
  hospi1.Init.DelayHoldQuarterCycle = HAL_OSPI_DHQC_DISABLE;
  hospi1.Init.ChipSelectBoundary = 0;
  hospi1.Init.DelayBlockBypass = HAL_OSPI_DELAY_BLOCK_BYPASSED;
  hospi1.Init.MaxTran = 0;
  hospi1.Init.Refresh = 0;
  if (HAL_OSPI_Init(&hospi1) != HAL_OK)
  {
    Error_Handler();
  }
  sOspiManagerCfg.ClkPort = 1;
  sOspiManagerCfg.NCSPort = 1;
  sOspiManagerCfg.IOLowPort = HAL_OSPIM_IOPORT_1_LOW;
  if (HAL_OSPIM_Config(&hospi1, &sOspiManagerCfg, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN OCTOSPI1_Init 2 */

  /* USER CODE END OCTOSPI1_Init 2 */

}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 9999;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 1000;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 999;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 999;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_PWM_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */
  HAL_TIM_MspPostInit(&htim3);

}

/* FMC initialization function */
static void MX_FMC_Init(void)
{

  /* USER CODE BEGIN FMC_Init 0 */

  /* USER CODE END FMC_Init 0 */

  FMC_NORSRAM_TimingTypeDef Timing = {0};

  /* USER CODE BEGIN FMC_Init 1 */

  /* USER CODE END FMC_Init 1 */

  /** Perform the SRAM1 memory initialization sequence
  */
  hsram1.Instance = FMC_NORSRAM_DEVICE;
  hsram1.Extended = FMC_NORSRAM_EXTENDED_DEVICE;
  /* hsram1.Init */
  hsram1.Init.NSBank = FMC_NORSRAM_BANK1;
  hsram1.Init.DataAddressMux = FMC_DATA_ADDRESS_MUX_DISABLE;
  hsram1.Init.MemoryType = FMC_MEMORY_TYPE_SRAM;
  hsram1.Init.MemoryDataWidth = FMC_NORSRAM_MEM_BUS_WIDTH_16;
  hsram1.Init.BurstAccessMode = FMC_BURST_ACCESS_MODE_DISABLE;
  hsram1.Init.WaitSignalPolarity = FMC_WAIT_SIGNAL_POLARITY_LOW;
  hsram1.Init.WaitSignalActive = FMC_WAIT_TIMING_BEFORE_WS;
  hsram1.Init.WriteOperation = FMC_WRITE_OPERATION_ENABLE;
  hsram1.Init.WaitSignal = FMC_WAIT_SIGNAL_DISABLE;
  hsram1.Init.ExtendedMode = FMC_EXTENDED_MODE_DISABLE;
  hsram1.Init.AsynchronousWait = FMC_ASYNCHRONOUS_WAIT_DISABLE;
  hsram1.Init.WriteBurst = FMC_WRITE_BURST_DISABLE;
  hsram1.Init.ContinuousClock = FMC_CONTINUOUS_CLOCK_SYNC_ONLY;
  hsram1.Init.WriteFifo = FMC_WRITE_FIFO_ENABLE;
  hsram1.Init.PageSize = FMC_PAGE_SIZE_NONE;
  /* Timing */
  Timing.AddressSetupTime = 15;
  Timing.AddressHoldTime = 15;
  Timing.DataSetupTime = 255;
  Timing.BusTurnAroundDuration = 15;
  Timing.CLKDivision = 16;
  Timing.DataLatency = 17;
  Timing.AccessMode = FMC_ACCESS_MODE_A;
  /* ExtTiming */

  if (HAL_SRAM_Init(&hsram1, &Timing, NULL) != HAL_OK)
  {
    Error_Handler( );
  }

  /* USER CODE BEGIN FMC_Init 2 */

  /* USER CODE END FMC_Init 2 */
}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  //Configure GPIO pin Output Level for the LCD pins, keyboard column pins and the backlight pin.
  HAL_GPIO_WritePin(LCD_RESET_GPIO_Port, LCD_RESET_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOC, KBD_COL_1_Pin|KBD_COL_4_Pin|KBD_COL_5_Pin|KBD_COL_6_Pin
                          |KBD_COL_2_Pin|KBD_COL_3_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(BACKLIGHT_GPIO_Port, BACKLIGHT_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : LCD_RESET_Pin */
  GPIO_InitStruct.Pin = LCD_RESET_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LCD_RESET_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : KBD_COL_1_Pin KBD_COL_4_Pin KBD_COL_5_Pin */
  GPIO_InitStruct.Pin = KBD_COL_1_Pin|KBD_COL_4_Pin|KBD_COL_5_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : KBD_ROW_B_Pin KBD_ROW_A_Pin KBD_ROW_C_Pin KBD_ROW_D_Pin
                           KBD_ROW_E_Pin KBD_ROW_F_Pin KBD_ROW_G_Pin KBD_ROW_H_Pin
                           KBD_ROW_I_Pin */
  GPIO_InitStruct.Pin = KBD_ROW_B_Pin|KBD_ROW_A_Pin|KBD_ROW_C_Pin|KBD_ROW_D_Pin
                          |KBD_ROW_E_Pin|KBD_ROW_F_Pin|KBD_ROW_G_Pin|KBD_ROW_H_Pin
                          |KBD_ROW_I_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : KBD_COL_6_Pin KBD_COL_2_Pin KBD_COL_3_Pin */
  GPIO_InitStruct.Pin = KBD_COL_6_Pin|KBD_COL_2_Pin|KBD_COL_3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : BACKLIGHT_Pin */
  GPIO_InitStruct.Pin = BACKLIGHT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(BACKLIGHT_GPIO_Port, &GPIO_InitStruct);

}

// Function to set the color of the LED:
void setLedColor(Color c) {
  color = c;
  __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, color.Red*1000/255);
  __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, color.Green*1000/255);
  __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_3, color.Blue*1000/255);
}
//Function to check if a key is pressed on the keyboard:
int keydown(KeyboardPos pos) {
	uint16_t rowPin = rowPins[pos.row];
	GPIO_TypeDef* rowPort = rowPorts[pos.row];
	uint16_t colPin = colPins[pos.col];
	GPIO_TypeDef* colPort = colPorts[pos.col];
	HAL_GPIO_WritePin(colPort, colPin, GPIO_PIN_SET);
	GPIO_PinState down = HAL_GPIO_ReadPin(rowPort, rowPin);
	HAL_GPIO_WritePin(colPort, colPin, GPIO_PIN_RESET);
	return down;
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
//Function to set a timeout with a callback function:
void setTimeout(void (*callback)(), uint32_t delay_ms) {
    timeoutCallback = callback;  // Stocke la fonction callback
    __HAL_TIM_SET_AUTORELOAD(&htim2, delay_ms - 1);  // Définit la durée
    __HAL_TIM_SET_COUNTER(&htim2, 0);  // Réinitialise le compteur
    HAL_TIM_Base_Start_IT(&htim2);  // Démarre le Timer avec interruption
}
// Callback function called when the timer period elapses
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
    if (htim == &htim2) {  // Vérifie si c'est bien TIM2
        HAL_TIM_Base_Stop_IT(&htim2);  // Stoppe le Timer
        if (timeoutCallback) timeoutCallback();  // Exécute la fonction
    }
}
// Function to reset the LED color to off (black):
void resetLed() {
  setLedColor((Color){0, 0, 0});
}
// Function to send a message over USB and flash the LED yellow for 300 ms:
void sendMessage(char *message) {
  uint16_t len = strlen(message);
  CDC_Transmit_HS((uint8_t *)message, len);
  timeoutCallback = NULL;
  setLedColor((Color){255, 255, 0}); //maybe unnecessary? Remove in future?
  setTimeout(resetLed, 300);
}
// Function to write a command to the LCD controller:
void LCD_WriteCommand(Command cmd)
{
  //GPIOB->ODR &= ~GPIO_PIN_11;  // RS = 0 means it's a command
  *(volatile uint16_t *)(CommandAddress) = cmd;
}
// Function to write data to the LCD controller:
void LCD_WriteData(uint8_t data)
{
    //GPIOB->ODR |= GPIO_PIN_11;   // RS = 1 means it's data
    *(volatile uint16_t *)(DataAddress) = data;
}

void LCD_Init(void)  //all shit is commented out, idk why, maybe not needed/not working?
{
    // Reset the LCD
    /*HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, GPIO_PIN_RESET);  // RESET = 0
    HAL_Delay(50);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, GPIO_PIN_SET);    // RESET = 1
    HAL_Delay(50);

    // Command to exit sleep mode
    LCD_WriteCommand(0x11);  // Sleep out
    HAL_Delay(120);

    // Configuration for the LCD
    LCD_WriteCommand(0x36);  // Memory Access Control
    LCD_WriteData(0x00);     // Orientation (de gauche à droite, de haut en bas)

    LCD_WriteCommand(0x3A);  // Pixel format set
    LCD_WriteData(0x05);     // 16 bits par pixel

    // Set the contrast (gamma)
    LCD_WriteCommand(0xC0);  // Power control 1
    LCD_WriteData(0x10);     // ValueSpecific for the ST7789V
    LCD_WriteData(0x3B);     // Value specific for the ST7789V

    LCD_WriteCommand(0x29);  // Display ON
    HAL_Delay(100);*/
}

void LCD_DrawPixel(uint16_t x, uint16_t y, uint16_t color)  // Draws a pixel at the specified coordinates (x, y) with the given color on the LCD display. The color is represented in RGB565 format, which uses 16 bits to represent colors (5 bits for red, 6 bits for green, and 5 bits for blue). The function sends a command to the LCD controller to set the pixel at the specified location to the specified color.
{
	sendMessage(concat_strings(7,"SET_PIXEL(",uint32_to_str(x),",",uint32_to_str(y),",",uint32_to_str(color),")"));
    /*LCD_WriteCommand(0x2A); // Command for defining the column address
    LCD_WriteData(x >> 8);  // Adress for the high byte of x, this means that the x coordinate is split into two bytes, and the high byte is sent first
    LCD_WriteData(x & 0xFF); // Adress for the low byte of x, the low byte is sent second and is the remainder of the x coordinate after the high byte is sent

    LCD_WriteCommand(0x2B); // Command for defining the row address
    LCD_WriteData(y >> 8);  // Adress for the high byte of y
    LCD_WriteData(y & 0xFF); // Adress for the low byte of y

    LCD_WriteCommand(0x2C); // Command for writing pixel data
    LCD_WriteData(color >> 8); // Color (high part)
    LCD_WriteData(color & 0xFF); // Color (low part)*/
}

void LCD_FillRect(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color) { // Function to fill a rectangle. Usage: LCD_FillRect(10, 10, 50, 50, RGB565(255, 0, 0)); // Fills a rectangle at (10, 10) with width 50 and height 50 with red color
	sendMessage(concat_strings(11,"FILL_RECT(",uint32_to_str(x),",",uint32_to_str(y),",",uint32_to_str(width),",",uint32_to_str(height),",",uint32_to_str(color),")"));
}

void LCD_DrawString(uint16_t x, uint16_t y, char* text, uint16_t color1, uint16_t color2) { // Function to draw a string of text on the LCD. Usage: LCD_DrawString(10, 10, "Hello", RGB565(255, 255, 255), RGB565(0, 0, 0)); // Draws the text "Hello" at (10, 10) with white color on black background
	sendMessage(concat_strings(11,"DRAW_STRING(",uint32_to_str(x),",",uint32_to_str(y),",",text,",",uint32_to_str(color1),",",uint32_to_str(color2),")"));
}

uint16_t RGB565(uint8_t r, uint8_t g, uint8_t b) // Function to convert RGB values to RGB565 format.
{
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}

void LCD_SetBrightness(int brightness) { // Function to set the brightness of the LCD backlight. The brightness value should be between 0 and 255, where 0 is off and 255 is maximum brightness.
  for (int i=0; i<brightness; i++) {
	//HAL_GPIO_WritePin(BACKLIGHT_GPIO_Port, BACKLIGHT_Pin, 0);
	HAL_Delay(0.01);
	//HAL_GPIO_WritePin(BACKLIGHT_GPIO_Port, BACKLIGHT_Pin, 1);
	HAL_Delay(0.01);
  }
}

void OctoSPI_SendCommand(uint8_t instruction, uint8_t *data, uint16_t dataSize) { // Function to send a command to the OctoSPI flash memory. The function takes an instruction byte, a pointer to a data buffer, and the size of the data to be sent or received. It configures the OSPI command structure and sends the command to the flash memory, followed by receiving the data if applicable.
    OSPI_RegularCmdTypeDef sCommand = {0};

    sCommand.OperationType      = HAL_OSPI_OPTYPE_COMMON_CFG;
    sCommand.FlashId            = HAL_OSPI_FLASH_ID_1;
    sCommand.Instruction        = instruction;
    sCommand.InstructionMode    = HAL_OSPI_INSTRUCTION_1_LINE; // 1 ligne pour l'instruction
    sCommand.AddressMode        = HAL_OSPI_ADDRESS_NONE;       // Pas d'adresse pour lire l'ID
    sCommand.AlternateBytesMode = HAL_OSPI_ALTERNATE_BYTES_NONE;
    sCommand.DataMode           = HAL_OSPI_DATA_1_LINE;        // 1 ligne pour les données
    sCommand.NbData             = dataSize;                    // Nombre de bytes à recevoir
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

uint8_t ReadFlashID(void) { // Function to read the ID of the OctoSPI flash memory. It sends the Read ID command (0x9F) to the flash memory and receives the 3-byte ID, returning the first byte of the ID.
	uint8_t id[3] = {0};
	OctoSPI_SendCommand(CMD_RDID, id, 3); // 0x9F = Commande Read ID
	return id[0];
}

void EnableQuadSPI() // Function to enable Quad SPI mode on the OctoSPI flash memory. It reads the status register, sets the Quad Enable (QE) bit, and writes the modified status register back to the flash memory.
{
    OSPI_RegularCmdTypeDef sCommand;
    uint8_t reg;

    // Lire le registre de statut
    memset(&sCommand, 0, sizeof(sCommand));
    sCommand.OperationType = HAL_OSPI_OPTYPE_COMMON_CFG;
    sCommand.FlashId = HAL_OSPI_FLASH_ID_1;
    sCommand.Instruction = 0x05; // Read Status Register
    sCommand.InstructionMode = HAL_OSPI_INSTRUCTION_1_LINE;
    sCommand.DataMode = HAL_OSPI_DATA_1_LINE;
    sCommand.NbData = 1;

    HAL_OSPI_Command(&hospi1, &sCommand, HAL_MAX_DELAY);
    HAL_OSPI_Receive(&hospi1, &reg, HAL_MAX_DELAY);

    // Activer Quad Enable Bit (QE = bit 6)
    reg |= (1 << 6);

    // Écrire le registre de statut
    sCommand.Instruction = 0x01; // Write Status Register
    HAL_OSPI_Command(&hospi1, &sCommand, HAL_MAX_DELAY);
    HAL_OSPI_Transmit(&hospi1, &reg, HAL_MAX_DELAY);

    sendMessage("Quad SPI activé !\n");
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void) // Error handler function that is called when an error occurs, it disables interrupts and enters an infinite loop, flashing the LED red to indicate an error state.
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  //Color color = {255, 0, 0};
  //setLedColor(color);
  while(1) {
	for (int i=0; i<255; i++) {
	  color.Red -= 1;
	  setLedColor(color);
	  HAL_Delay(2);      // Maybe flashsing too fast, change to 10 or 20 if needed
	}
	for (int i=0; i<255; i++) {
	  color.Red += 1;
	  setLedColor(color);
	  HAL_Delay(2);
	}
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
