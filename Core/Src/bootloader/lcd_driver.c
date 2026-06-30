/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : lcd_driver.c
  * @brief          : ST7789V LCD driver - parallel FMC/FSMC 16-bit interface
  *
  * The display is connected to FMC Bank1 (NE1).  Address bit A16 is used as
  * the RS (Data/Command) line:
  *   CommandAddress  = FMC_BANK1_BASE            (A16 = 0  → command)
  *   DataAddress     = FMC_BANK1_BASE | (1<<16)  (A16 = 1  → data)
  *
  * Display: 320 × 240, 16-bit RGB565, ST7789V controller.
  ******************************************************************************
  */
/* USER CODE END Header */

#include "lcd_driver.h"
#include "main.h"
#include "stm32h7xx_hal.h"

/* ---------------------------------------------------------------------------
 * FMC address map
 * ---------------------------------------------------------------------------
 * Bank 1 (NE1) base:  0x6000_0000
 * A16 is the RS pin:  offset 0x0002_0000  (byte-address; FMC 16-bit mode
 *   means address lines are shifted by 1 relative to byte addresses, so
 *   A16 corresponds to word-address bit 16, i.e. byte offset 1<<17 = 0x20000).
 *
 * Actually for a 16-bit wide bus the CPU address bit N maps to FMC address
 * pin A(N-1).  So to assert A16 we need CPU address bit 17:
 *   DataAddress = 0x6000_0000 | (1 << 17) = 0x6002_0000
 * --------------------------------------------------------------------------*/
#define LCD_FMC_BASE        0x60000000UL
#define LCD_CMD_ADDR        ((volatile uint16_t *)(LCD_FMC_BASE))
#define LCD_DATA_ADDR       ((volatile uint16_t *)(LCD_FMC_BASE | (1UL << 17)))

/* Screen dimensions */
#define LCD_WIDTH   320
#define LCD_HEIGHT  240

/* ---------------------------------------------------------------------------
 * Low-level bus helpers
 * --------------------------------------------------------------------------*/
static inline void lcd_write_cmd(uint8_t cmd)
{
    *LCD_CMD_ADDR = (uint16_t)cmd;
}

static inline void lcd_write_data8(uint8_t data)
{
    *LCD_DATA_ADDR = (uint16_t)data;
}

static inline void lcd_write_data16(uint16_t data)
{
    *LCD_DATA_ADDR = data;
}

/* Public wrappers (kept for compatibility with existing call-sites) */
void LCD_WriteCommand(Command cmd)
{
    lcd_write_cmd((uint8_t)cmd);
}

void LCD_WriteData(uint8_t data)
{
    lcd_write_data8(data);
}

/* ---------------------------------------------------------------------------
 * Column / Row address window
 * --------------------------------------------------------------------------*/
static void lcd_set_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
    /* Column address set (2A) */
    lcd_write_cmd(0x2A);
    lcd_write_data8(x0 >> 8);
    lcd_write_data8(x0 & 0xFF);
    lcd_write_data8(x1 >> 8);
    lcd_write_data8(x1 & 0xFF);

    /* Row address set (2B) */
    lcd_write_cmd(0x2B);
    lcd_write_data8(y0 >> 8);
    lcd_write_data8(y0 & 0xFF);
    lcd_write_data8(y1 >> 8);
    lcd_write_data8(y1 & 0xFF);

    /* Memory write (2C) */
    lcd_write_cmd(0x2C);
}

/* ---------------------------------------------------------------------------
 * LCD_Init  –  ST7789V power-on initialisation sequence
 * --------------------------------------------------------------------------*/
void LCD_Init(void)
{
    /* Hardware reset: assert RESET low for ≥10 µs, then deassert */
    HAL_GPIO_WritePin(LCD_RESET_GPIO_Port, LCD_RESET_Pin, GPIO_PIN_RESET);
    HAL_Delay(10);
    HAL_GPIO_WritePin(LCD_RESET_GPIO_Port, LCD_RESET_Pin, GPIO_PIN_SET);
    HAL_Delay(120);   /* ST7789V needs ≥120 ms after RESET high before SLPOUT */

    /* Sleep Out – exit sleep mode */
    lcd_write_cmd(0x11);
    HAL_Delay(120);

    /* Memory Access Control – RGB order, landscape (MX=1, MV=1) */
    lcd_write_cmd(0x36);
    lcd_write_data8(0x70);   /* MY=0 MX=1 MV=1 ML=0 RGB=0 MH=0 → landscape */

    /* Pixel Format – 16 bits per pixel (RGB565) */
    lcd_write_cmd(0x3A);
    lcd_write_data8(0x55);   /* 0x55 = 16bpp for both RGB and MCU interfaces */

    /* Porch setting */
    lcd_write_cmd(0xB2);
    lcd_write_data8(0x0C);
    lcd_write_data8(0x0C);
    lcd_write_data8(0x00);
    lcd_write_data8(0x33);
    lcd_write_data8(0x33);

    /* Gate control */
    lcd_write_cmd(0xB7);
    lcd_write_data8(0x35);

    /* VCOM setting */
    lcd_write_cmd(0xBB);
    lcd_write_data8(0x19);

    /* LCM control */
    lcd_write_cmd(0xC0);
    lcd_write_data8(0x2C);

    /* VDV and VRH command enable */
    lcd_write_cmd(0xC2);
    lcd_write_data8(0x01);

    /* VRH set */
    lcd_write_cmd(0xC3);
    lcd_write_data8(0x12);

    /* VDV set */
    lcd_write_cmd(0xC4);
    lcd_write_data8(0x20);

    /* Frame rate control – 60 Hz */
    lcd_write_cmd(0xC6);
    lcd_write_data8(0x0F);

    /* Power control 1 */
    lcd_write_cmd(0xD0);
    lcd_write_data8(0xA4);
    lcd_write_data8(0xA1);

    /* Positive voltage gamma */
    lcd_write_cmd(0xE0);
    lcd_write_data8(0xD0); lcd_write_data8(0x04); lcd_write_data8(0x0D);
    lcd_write_data8(0x11); lcd_write_data8(0x13); lcd_write_data8(0x2B);
    lcd_write_data8(0x3F); lcd_write_data8(0x54); lcd_write_data8(0x4C);
    lcd_write_data8(0x18); lcd_write_data8(0x0D); lcd_write_data8(0x0B);
    lcd_write_data8(0x1F); lcd_write_data8(0x23);

    /* Negative voltage gamma */
    lcd_write_cmd(0xE1);
    lcd_write_data8(0xD0); lcd_write_data8(0x04); lcd_write_data8(0x0C);
    lcd_write_data8(0x11); lcd_write_data8(0x13); lcd_write_data8(0x2C);
    lcd_write_data8(0x3F); lcd_write_data8(0x44); lcd_write_data8(0x51);
    lcd_write_data8(0x2F); lcd_write_data8(0x1F); lcd_write_data8(0x1F);
    lcd_write_data8(0x20); lcd_write_data8(0x23);

    /* Display inversion on (required for correct colours on ST7789V) */
    lcd_write_cmd(0x21);

    /* Display on */
    lcd_write_cmd(0x29);
    HAL_Delay(10);

    /* Turn on backlight */
    HAL_GPIO_WritePin(BACKLIGHT_GPIO_Port, BACKLIGHT_Pin, GPIO_PIN_SET);
}

/* ---------------------------------------------------------------------------
 * panelIdentifier  –  reads the 24-bit display ID (0x04 / RDID command)
 * --------------------------------------------------------------------------*/
uint32_t panelIdentifier(void)
{
    lcd_write_cmd(0x04);
    (void)*LCD_DATA_ADDR;           /* dummy read per datasheet */
    uint8_t id1 = (uint8_t)*LCD_DATA_ADDR;
    uint8_t id2 = (uint8_t)*LCD_DATA_ADDR;
    uint8_t id3 = (uint8_t)*LCD_DATA_ADDR;
    return ((uint32_t)id1 << 16) | ((uint32_t)id2 << 8) | id3;
}

/* ---------------------------------------------------------------------------
 * Drawing primitives
 * --------------------------------------------------------------------------*/
void LCD_DrawPixel(uint16_t x, uint16_t y, uint16_t color)
{
    if (x >= LCD_WIDTH || y >= LCD_HEIGHT)
    {
        return;
    }
    lcd_set_window(x, y, x, y);
    lcd_write_data16(color);
}

void LCD_FillRect(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color)
{
    if (x >= LCD_WIDTH || y >= LCD_HEIGHT)
    {
        return;
    }
    /* Clamp to screen bounds */
    if ((uint32_t)x + width  > LCD_WIDTH)  { width  = LCD_WIDTH  - x; }
    if ((uint32_t)y + height > LCD_HEIGHT) { height = LCD_HEIGHT - y; }

    lcd_set_window(x, y, (uint16_t)(x + width - 1U), (uint16_t)(y + height - 1U));

    uint32_t count = (uint32_t)width * height;
    while (count--)
    {
        lcd_write_data16(color);
    }
}

/* ---------------------------------------------------------------------------
 * LCD_FillScreen  –  convenience: fill entire display with one colour
 * --------------------------------------------------------------------------*/
void LCD_FillScreen(uint16_t color)
{
    LCD_FillRect(0, 0, LCD_WIDTH, LCD_HEIGHT, color);
}

/* ---------------------------------------------------------------------------
 * LCD_DrawString  –  minimal 5×7 bitmap font renderer
 *
 * color1 = foreground, color2 = background.
 * Each character is 6 pixels wide (5 glyph + 1 spacing) × 8 tall.
 * --------------------------------------------------------------------------*/

/* 5×7 font glyphs for ASCII 0x20–0x7E */
static const uint8_t font5x7[][5] = {
    {0x00,0x00,0x00,0x00,0x00}, /* ' ' */
    {0x00,0x00,0x5F,0x00,0x00}, /* '!' */
    {0x00,0x07,0x00,0x07,0x00}, /* '"' */
    {0x14,0x7F,0x14,0x7F,0x14}, /* '#' */
    {0x24,0x2A,0x7F,0x2A,0x12}, /* '$' */
    {0x23,0x13,0x08,0x64,0x62}, /* '%' */
    {0x36,0x49,0x55,0x22,0x50}, /* '&' */
    {0x00,0x05,0x03,0x00,0x00}, /* ''' */
    {0x00,0x1C,0x22,0x41,0x00}, /* '(' */
    {0x00,0x41,0x22,0x1C,0x00}, /* ')' */
    {0x14,0x08,0x3E,0x08,0x14}, /* '*' */
    {0x08,0x08,0x3E,0x08,0x08}, /* '+' */
    {0x00,0x50,0x30,0x00,0x00}, /* ',' */
    {0x08,0x08,0x08,0x08,0x08}, /* '-' */
    {0x00,0x60,0x60,0x00,0x00}, /* '.' */
    {0x20,0x10,0x08,0x04,0x02}, /* '/' */
    {0x3E,0x51,0x49,0x45,0x3E}, /* '0' */
    {0x00,0x42,0x7F,0x40,0x00}, /* '1' */
    {0x42,0x61,0x51,0x49,0x46}, /* '2' */
    {0x21,0x41,0x45,0x4B,0x31}, /* '3' */
    {0x18,0x14,0x12,0x7F,0x10}, /* '4' */
    {0x27,0x45,0x45,0x45,0x39}, /* '5' */
    {0x3C,0x4A,0x49,0x49,0x30}, /* '6' */
    {0x01,0x71,0x09,0x05,0x03}, /* '7' */
    {0x36,0x49,0x49,0x49,0x36}, /* '8' */
    {0x06,0x49,0x49,0x29,0x1E}, /* '9' */
    {0x00,0x36,0x36,0x00,0x00}, /* ':' */
    {0x00,0x56,0x36,0x00,0x00}, /* ';' */
    {0x08,0x14,0x22,0x41,0x00}, /* '<' */
    {0x14,0x14,0x14,0x14,0x14}, /* '=' */
    {0x00,0x41,0x22,0x14,0x08}, /* '>' */
    {0x02,0x01,0x51,0x09,0x06}, /* '?' */
    {0x32,0x49,0x79,0x41,0x3E}, /* '@' */
    {0x7E,0x11,0x11,0x11,0x7E}, /* 'A' */
    {0x7F,0x49,0x49,0x49,0x36}, /* 'B' */
    {0x3E,0x41,0x41,0x41,0x22}, /* 'C' */
    {0x7F,0x41,0x41,0x22,0x1C}, /* 'D' */
    {0x7F,0x49,0x49,0x49,0x41}, /* 'E' */
    {0x7F,0x09,0x09,0x09,0x01}, /* 'F' */
    {0x3E,0x41,0x49,0x49,0x7A}, /* 'G' */
    {0x7F,0x08,0x08,0x08,0x7F}, /* 'H' */
    {0x00,0x41,0x7F,0x41,0x00}, /* 'I' */
    {0x20,0x40,0x41,0x3F,0x01}, /* 'J' */
    {0x7F,0x08,0x14,0x22,0x41}, /* 'K' */
    {0x7F,0x40,0x40,0x40,0x40}, /* 'L' */
    {0x7F,0x02,0x0C,0x02,0x7F}, /* 'M' */
    {0x7F,0x04,0x08,0x10,0x7F}, /* 'N' */
    {0x3E,0x41,0x41,0x41,0x3E}, /* 'O' */
    {0x7F,0x09,0x09,0x09,0x06}, /* 'P' */
    {0x3E,0x41,0x51,0x21,0x5E}, /* 'Q' */
    {0x7F,0x09,0x19,0x29,0x46}, /* 'R' */
    {0x46,0x49,0x49,0x49,0x31}, /* 'S' */
    {0x01,0x01,0x7F,0x01,0x01}, /* 'T' */
    {0x3F,0x40,0x40,0x40,0x3F}, /* 'U' */
    {0x1F,0x20,0x40,0x20,0x1F}, /* 'V' */
    {0x3F,0x40,0x38,0x40,0x3F}, /* 'W' */
    {0x63,0x14,0x08,0x14,0x63}, /* 'X' */
    {0x07,0x08,0x70,0x08,0x07}, /* 'Y' */
    {0x61,0x51,0x49,0x45,0x43}, /* 'Z' */
    {0x00,0x7F,0x41,0x41,0x00}, /* '[' */
    {0x02,0x04,0x08,0x10,0x20}, /* '\' */
    {0x00,0x41,0x41,0x7F,0x00}, /* ']' */
    {0x04,0x02,0x01,0x02,0x04}, /* '^' */
    {0x40,0x40,0x40,0x40,0x40}, /* '_' */
    {0x00,0x01,0x02,0x04,0x00}, /* '`' */
    {0x20,0x54,0x54,0x54,0x78}, /* 'a' */
    {0x7F,0x48,0x44,0x44,0x38}, /* 'b' */
    {0x38,0x44,0x44,0x44,0x20}, /* 'c' */
    {0x38,0x44,0x44,0x48,0x7F}, /* 'd' */
    {0x38,0x54,0x54,0x54,0x18}, /* 'e' */
    {0x08,0x7E,0x09,0x01,0x02}, /* 'f' */
    {0x0C,0x52,0x52,0x52,0x3E}, /* 'g' */
    {0x7F,0x08,0x04,0x04,0x78}, /* 'h' */
    {0x00,0x44,0x7D,0x40,0x00}, /* 'i' */
    {0x20,0x40,0x44,0x3D,0x00}, /* 'j' */
    {0x7F,0x10,0x28,0x44,0x00}, /* 'k' */
    {0x00,0x41,0x7F,0x40,0x00}, /* 'l' */
    {0x7C,0x04,0x18,0x04,0x78}, /* 'm' */
    {0x7C,0x08,0x04,0x04,0x78}, /* 'n' */
    {0x38,0x44,0x44,0x44,0x38}, /* 'o' */
    {0x7C,0x14,0x14,0x14,0x08}, /* 'p' */
    {0x08,0x14,0x14,0x18,0x7C}, /* 'q' */
    {0x7C,0x08,0x04,0x04,0x08}, /* 'r' */
    {0x48,0x54,0x54,0x54,0x20}, /* 's' */
    {0x04,0x3F,0x44,0x40,0x20}, /* 't' */
    {0x3C,0x40,0x40,0x20,0x7C}, /* 'u' */
    {0x1C,0x20,0x40,0x20,0x1C}, /* 'v' */
    {0x3C,0x40,0x30,0x40,0x3C}, /* 'w' */
    {0x44,0x28,0x10,0x28,0x44}, /* 'x' */
    {0x0C,0x50,0x50,0x50,0x3C}, /* 'y' */
    {0x44,0x64,0x54,0x4C,0x44}, /* 'z' */
    {0x00,0x08,0x36,0x41,0x00}, /* '{' */
    {0x00,0x00,0x7F,0x00,0x00}, /* '|' */
    {0x00,0x41,0x36,0x08,0x00}, /* '}' */
    {0x10,0x08,0x08,0x10,0x08}, /* '~' */
};

void LCD_DrawString(uint16_t x, uint16_t y, char *text, uint16_t color1, uint16_t color2)
{
    uint16_t cx = x;
    while (*text)
    {
        char c = *text++;
        if (c < 0x20 || c > 0x7E)
        {
            c = '?';
        }
        const uint8_t *glyph = font5x7[(uint8_t)(c - 0x20)];

        /* Render one 6×8 character cell */
        lcd_set_window(cx, y, (uint16_t)(cx + 5U), (uint16_t)(y + 7U));
        for (uint8_t row = 0; row < 8U; row++)
        {
            for (uint8_t col = 0; col < 6U; col++)
            {
                uint8_t bit = (col < 5U) ? ((glyph[col] >> row) & 0x01U) : 0U;
                lcd_write_data16(bit ? color1 : color2);
            }
        }
        cx += 6U;
        if (cx + 6U > LCD_WIDTH)
        {
            break;   /* no wrapping */
        }
    }
}

/* ---------------------------------------------------------------------------
 * Colour helper
 * --------------------------------------------------------------------------*/
uint16_t RGB565(uint8_t r, uint8_t g, uint8_t b)
{
    return (uint16_t)(((uint16_t)(r & 0xF8U) << 8) |
                      ((uint16_t)(g & 0xFCU) << 3) |
                      ((uint16_t)(b         >> 3)));
}

/* ---------------------------------------------------------------------------
 * LCD_SetBrightness
 *
 * Simple on/off control – the backlight is driven by a GPIO, not PWM.
 * Pass 0 to turn off, any non-zero value to turn on.
 * --------------------------------------------------------------------------*/
void LCD_SetBrightness(int brightness)
{
    HAL_GPIO_WritePin(BACKLIGHT_GPIO_Port, BACKLIGHT_Pin,
                      (brightness > 0) ? GPIO_PIN_SET : GPIO_PIN_RESET);
}
