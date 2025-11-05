#include "minimal_oled.h"
#include "fonts.h"

// I2C frequency
#define I2C_MASTER_FREQ_HZ 400000

// Maximun ticks to wait after send
#define I2C_TICKS_TO_WAIT 100

// oled definitions
#define OLED_ADDR         0x3C    // oled write address (0x3C << 1)
#define OLED_CMD_MODE     0x00    // set command mode
#define OLED_DAT_MODE     0x40    // set data mode

// oled commands
#define OLED_COLUMN_LOW   0x00    // set lower 4 bits of start column (0x00 - 0x0F)
#define OLED_COLUMN_HIGH  0x10    // set higher 4 bits of start column (0x10 - 0x1F)
#define OLED_MEMORYMODE   0x20    // set memory addressing mode (following byte)
#define OLED_COLUMNS      0x21    // set start and end column (following 2 bytes)
#define OLED_PAGES        0x22    // set start and end page (following 2 bytes)
#define OLED_SCROLL_OFF   0x2E    // deactivate scroll command
#define OLED_STARTLINE    0x40    // set display start line (0x40-0x7F = 0-63)
#define OLED_CONTRAST     0x81    // set display contrast (following byte)
#define OLED_CHARGEPUMP   0x8D    // (following byte - 0x14:enable, 0x10: disable)
#define OLED_XFLIP_OFF    0xA0    // don't flip display horizontally
#define OLED_XFLIP        0xA1    // flip display horizontally
#define OLED_INVERT_OFF   0xA6    // set non-inverted display
#define OLED_INVERT       0xA7    // set inverse display
#define OLED_MULTIPLEX    0xA8    // set multiplex ratio (following byte)
#define OLED_DISPLAY_OFF  0xAE    // set display off (sleep mode)
#define OLED_DISPLAY_ON   0xAF    // set display on
#define OLED_PAGE         0xB0    // set start page (following byte)
#define OLED_YFLIP_OFF    0xC0    // don't flip display vertically
#define OLED_YFLIP        0xC8    // flip display vertically
#define OLED_OFFSET       0xD3    // set display offset (y-scroll: following byte)
#define OLED_COMPINS      0xDA    // set COM pin config (following byte)
#define OLED_CLKDIV       0xD5    // set display clock divide ratio / oscillator frequency
#define OLED_VCOMH        0xDB    // set VCOMH deselect level
#define OLED_MEMORY_PAGE  0x02    // page addressing mode
#define OLED_MEMORY_HORI  0x00    // horizontal addressing mode

#if CONFIG_I2C_PORT_0
#define I2C_NUM I2C_NUM_0
#elif CONFIG_I2C_PORT_1
#define I2C_NUM I2C_NUM_1
#else
#define I2C_NUM I2C_NUM_0 
#endif

#ifdef CONFIG_RESOLUTION_128X64

#define OLED_HEIGHT 64

// SSD1306 initialisation sequence for 128x64
#ifndef CONFIG_CHIP_SH1106
static const uint8_t SSD1306_128X64_INIT_CMD[] = {
    OLED_CMD_MODE,
    OLED_MULTIPLEX,   0x3F,                 // set multiplex ratio  
    OLED_CHARGEPUMP,  0x14,                 // set DC-DC enable  
    OLED_MEMORYMODE,  0x00,                 // set horizontal addressing mode
    OLED_COLUMNS,     0x00, 0x7F,           // set start and end column
    OLED_PAGES,       0x00, 0x3F,           // set start and end page
    OLED_COMPINS,     0x12,                 // set com pins
    OLED_XFLIP, OLED_YFLIP,                 // flip screen
    OLED_DISPLAY_ON                         // display on
};
#endif

// SH1106 initialisation sequence for 128x64 (based on working example)
#ifdef CONFIG_CHIP_SH1106
static const uint8_t SH1106_128X64_INIT_CMD[] = {
    OLED_CMD_MODE,
    OLED_DISPLAY_OFF,                       // 0xAE - display off
    OLED_MULTIPLEX,   0x3F,                 // 0xA8 - set multiplex ratio (64-1 = 0x3F)
    OLED_OFFSET,       0x00,                // 0xD3 - set display offset (no offset)
    OLED_STARTLINE,   0x00,                 // 0x40 - set display start line to 0
    OLED_XFLIP,                            // 0xA1 - flip horizontally (segment remap)
    OLED_YFLIP,                            // 0xC8 - don't flip vertically (COM scan mode)
    OLED_COMPINS,     0x12,                 // 0xDA - set com pins configuration
    OLED_CONTRAST,    0xFF,                 // 0x81 - set contrast (max for SH1106)
    OLED_CLKDIV,      0x80,                 // 0xD5 - set display clock divide ratio / oscillator frequency
    OLED_CHARGEPUMP,  0x14,                 // 0x8D - set DC-DC enable (0x14) or disable (0x10)
    OLED_VCOMH,       0x40,                 // 0xDB - set VCOMH deselect level
    OLED_MEMORYMODE,  OLED_MEMORY_PAGE,     // 0x20 - set memory addressing mode (0x02 = page mode for SH1106)
    OLED_COLUMN_LOW,  0x00,                 // 0x00 - set lower column address for page addressing mode
    OLED_COLUMN_HIGH, 0x10,                 // 0x10 - set higher column address for page addressing mode
    OLED_SCROLL_OFF,                        // 0x2E - deactivate scroll
    OLED_INVERT_OFF,                        // 0xA6 - set normal display (not inverted)
    OLED_DISPLAY_ON                         // 0xAF - display on
};
#endif

#else

#define OLED_HEIGHT 32

// SSD1306 initialisation sequence for 128x32 (SH1106 doesn't support 128x32)
static const uint8_t SSD1306_128X32_INIT_CMD[] = {
    OLED_CMD_MODE,
    OLED_MULTIPLEX,   0x1F,                 // set multiplex ratio  
    OLED_CHARGEPUMP,  0x14,                 // set DC-DC enable  
    OLED_MEMORYMODE,  0x00,                 // set horizontal addressing mode
    OLED_COLUMNS,     0x00, 0x7F,           // set start and end column
    OLED_PAGES,       0x00, 0x1F,           // set start and end page
    OLED_COMPINS,     0x02,                 // set com pins
    OLED_XFLIP, OLED_YFLIP,                 // flip screen
    OLED_DISPLAY_ON                         // display on
};

#endif

#define OLED_WIDTH 128
#define OLED_NUM_PAGES (OLED_HEIGHT / 8)

// Buffer for the oled
uint8_t oled_buf[OLED_NUM_PAGES][OLED_WIDTH];

// handle to send the buffer;
i2c_master_dev_handle_t i2c_dev_handle;

/**
 * @fn oled_init_i2c
 * 
 * @brief Function to init the i2c with the 
 */
i2c_master_bus_config_t oled_init_i2c(void){
    i2c_master_bus_config_t i2c_mst_config = {
		.clk_source = I2C_CLK_SRC_DEFAULT,
		.glitch_ignore_cnt = 7,
		.i2c_port = I2C_NUM,
		.scl_io_num = CONFIG_SCL_GPIO,
		.sda_io_num = CONFIG_SDA_GPIO,
		.flags.enable_internal_pullup = true,
	};
	i2c_master_bus_handle_t i2c_bus_handle;
	ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_mst_config, &i2c_bus_handle));

	i2c_device_config_t dev_cfg = {
		.dev_addr_length = I2C_ADDR_BIT_LEN_7,
		.device_address = OLED_ADDR,
		.scl_speed_hz = I2C_MASTER_FREQ_HZ,
	};

	ESP_ERROR_CHECK(i2c_master_bus_add_device(i2c_bus_handle, &dev_cfg, &i2c_dev_handle));

    oled_init(i2c_bus_handle);

    return i2c_mst_config;
}

/**
 * @fn oled_init
 * 
 * @brief Init the oled with the address and device, this function needs an i2c bus initialized
 * 
 * @param i2c_bus_handle Bus for the i2c master
 */
void oled_init(i2c_master_bus_handle_t i2c_bus_handle)
{

    // First define the device and frequency
    i2c_device_config_t dev_cfg = {
		.dev_addr_length = I2C_ADDR_BIT_LEN_7,
		.device_address = OLED_ADDR,
		.scl_speed_hz = I2C_MASTER_FREQ_HZ,
	};

	ESP_ERROR_CHECK(i2c_master_bus_add_device(i2c_bus_handle, &dev_cfg, &i2c_dev_handle));

    // Init oled based on chip type and resolution
#ifdef CONFIG_RESOLUTION_128X64
    #ifdef CONFIG_CHIP_SH1106
        // SH1106 128x64 initialization
        i2c_master_transmit(i2c_dev_handle, SH1106_128X64_INIT_CMD, sizeof(SH1106_128X64_INIT_CMD), I2C_TICKS_TO_WAIT);
    #else
        // SSD1306 128x64 initialization
        i2c_master_transmit(i2c_dev_handle, SSD1306_128X64_INIT_CMD, sizeof(SSD1306_128X64_INIT_CMD), I2C_TICKS_TO_WAIT);
    #endif
#else
    // SSD1306 128x32 initialization (SH1106 doesn't support 128x32)
    i2c_master_transmit(i2c_dev_handle, SSD1306_128X32_INIT_CMD, sizeof(SSD1306_128X32_INIT_CMD), I2C_TICKS_TO_WAIT);
#endif
    
}

/**
 * @fn oled_set_position
 * 
 * @brief This function sets the position on the oled
 * 
 * @param x set position on x
 * 
 * @param y set position on y
 */
void oled_set_position(uint8_t x, uint8_t y) 
{
    uint8_t oled_buffer[5] = {0};
    uint8_t buffer_position = 0;
    uint8_t column = x;

#ifdef CONFIG_CHIP_SH1106
    // SH1106 needs offset of +2 columns (has 132 column buffer but only displays 128)
    column += 2;
#endif

    oled_buffer[buffer_position++] = OLED_CMD_MODE;
    oled_buffer[buffer_position++] = OLED_PAGE | y;
    oled_buffer[buffer_position++] = OLED_COLUMN_LOW | (column & 0x0F);
    oled_buffer[buffer_position++] = OLED_COLUMN_HIGH | ((column >> 4) & 0x0F);

    i2c_master_transmit(i2c_dev_handle, oled_buffer, buffer_position, I2C_TICKS_TO_WAIT);
}

/**
 * @fn oled_flush_page
 * 
 * @brief Flush any page from the oled
 * 
 * @param page number of page to flush
 */
static void oled_flush_page(uint8_t page)
{
    if(page >= OLED_NUM_PAGES) return;

#ifdef CONFIG_CHIP_SH1106
    // SH1106 needs separate command and data transactions (as per example)
    // First transaction: Set page and column address (offset by +2 columns)
    uint8_t cmd_buffer[5] = {0};
    cmd_buffer[0] = OLED_CMD_MODE;
    cmd_buffer[1] = OLED_PAGE | page;                          // Set page address
    cmd_buffer[2] = OLED_COLUMN_LOW | 0x02;                    // Set lower column (offset +2)
    cmd_buffer[3] = OLED_COLUMN_HIGH | 0x00;                   // Set higher column
    i2c_master_transmit(i2c_dev_handle, cmd_buffer, 4, I2C_TICKS_TO_WAIT);
    
    // Second transaction: Send data
    uint8_t data_buffer[OLED_WIDTH + 1] = {0};
    data_buffer[0] = OLED_DAT_MODE;
    for(uint8_t i = 0; i < OLED_WIDTH; i++)
    {
        data_buffer[i + 1] = oled_buf[page][i];
    }
    i2c_master_transmit(i2c_dev_handle, data_buffer, OLED_WIDTH + 1, I2C_TICKS_TO_WAIT);
#else
    // SSD1306: Send data in one transaction
    uint8_t oled_buffer[OLED_WIDTH + 1] = {0};
    oled_buffer[0] = OLED_DAT_MODE;
    
    for(uint8_t i = 0; i < OLED_WIDTH; i++)
    {
        oled_buffer[i + 1] = oled_buf[page][i];
    }
    
    i2c_master_transmit(i2c_dev_handle, oled_buffer, OLED_WIDTH + 1, I2C_TICKS_TO_WAIT);
#endif
}

/**
 * @fn oled_flush
 * 
 * @brief Flush all oled
 * 
 * @param none
 */
void oled_flush(void)
{
  for (uint8_t p = 0; p < OLED_NUM_PAGES; p++)
  {
    oled_flush_page(p);
  }
}

/**
 * @fn oled_clear_buffer
 * 
 * @brief Clear the oled buffer
 * 
 * @param none
 */
void oled_clear_buffer(void)
{
  memset(oled_buf, 0x00, sizeof(oled_buf));
}

/**
 * @fn oled_clear
 * 
 * @brief Clear the oled and display
 * 
 * @param none
 */
void oled_clear(void)
{
  oled_clear_buffer();
  oled_flush();
}

/**
 * @fn oled_set_pixel
 * 
 * @brief Set a pixel on the oled
 * 
 * @param x set position on x axe for the pixel
 * @param y set position on y axe for the pixel
 * @param color 0 or 1 to show on the display
 */
void oled_set_pixel(int16_t x, int16_t y, uint8_t color)
{
  if (x < 0 || x >= OLED_WIDTH || y < 0 || y >= OLED_HEIGHT)
    return;
  uint8_t page = y >> 3;  // y / 8
  uint8_t bit = y & 0x07; // y % 8
  if (color)
    oled_buf[page][x] |= (1 << bit);
  else
    oled_buf[page][x] &= ~(1 << bit);
}

/**
 * @fn oled_set_pixel
 * 
 * @brief Set a pixel on the oled
 * 
 * @param x set position on x axe on the oled
 * @param y set position on y axe on the oled
 * @param w width of the bitmap
 * @param h height of the bitmap
 * @param bitmap the array of the bitmap
 */
void oled_draw_bmp(int16_t x, int16_t y, int16_t w, int16_t h, const uint8_t *bitmap)
{
  for (int16_t j = 0; j < h; j++)
  {
    for (int16_t i = 0; i < w; i++)
    {
      int byte_index = (j / 8) * w + i;
      uint8_t bit_mask = 1 << (j & 7);

      uint8_t pixel = (bitmap[byte_index] & bit_mask) ? 1 : 0;

      oled_set_pixel(x + i, y + j, pixel);
    }
  }
}

/**
 * @fn transpose_letter
 * 
 * @brief transpose the letter to match with the oled
 * 
 * @param in array to transpose
 * @param out array transposed
 * @param w width of the letter
 * @param h height of the letter
 */
static void transpose_letter(const char in[8], uint8_t out[8],uint8_t width, uint8_t height)
{
  for (uint8_t x = 0; x < width; x++)
  {
    uint8_t col = 0;
    for (uint8_t y = 0; y < height; y++)
    {
      if (in[y] & (1 << x))
      {
        col |= (1 << y);
      }
    }
    out[x] = col;
  }
}

/**
 * @fn oled_draw_char8x8
 * 
 * @brief draw a single char for font 8x8
 * 
 * @param x set position of the letter on x
 * @param y set position of the letter on y
 * @param c character to display on buffer
 */
void oled_draw_char8x8(uint8_t x, uint8_t y, char c)
{
  if (c < 32 || c > 127)
    c = ' ';
  uint8_t transposed[8];
  transpose_letter(font8x8_basic[c - 32], transposed,8,8);
  oled_draw_bmp(x, y,8, 8, transposed);
}

/**
 * @fn oled_print_8x8
 * 
 * @brief draw a text on the oled with font 8x8
 * 
 * @param x set position of the text on x
 * @param y set position of the text on y
 * @param text  text to display on the oled
 */
void oled_print_8x8(uint8_t x, uint8_t y, const char *text)
{
  while (*text)
  {
    oled_draw_char8x8(x, y, *text++);
    x += 8;
  }
}

/**
 * @fn oled_draw_char6x8
 * 
 * @brief draw a single char for font 6x8
 * 
 * @param x set position of the letter on x
 * @param y set position of the letter on y
 * @param c character to display on buffer
 */
void oled_draw_char6x8(uint8_t x, uint8_t y, char c)
{
  if (c < 32 || c > 127)
    c = ' ';
  oled_draw_bmp(x, y, 6, 8, (const uint8_t *)font_6x8[c - 32]);
}

/**
 * @fn oled_print_6x8
 * 
 * @brief draw a text on the oled with font 6x8
 * 
 * @param x set position of the text on x
 * @param y set position of the text on y
 * @param text  text to display on the oled
 */
void oled_print_6x8(uint8_t x, uint8_t y, const char *text)
{
  while (*text)
  {
    oled_draw_char6x8(x, y, *text++);
    x += 7;
  }
}

/**
 * @fn oled_draw_char5x8
 * 
 * @brief draw a single char for font 5x8
 * 
 * @param x set position of the letter on x
 * @param y set position of the letter on y
 * @param c character to display on buffer
 */
void oled_draw_char5x8(uint8_t x, uint8_t y, char c)
{
  if (c < 32 || c > 127)
    c = ' ';
  oled_draw_bmp(x, y, 6, 8, (const uint8_t *)font_5x8[c - 32]);
}

/**
 * @fn oled_print_5x8
 * 
 * @brief draw a text on the oled with font 5x8
 * 
 * @param x set position of the text on x
 * @param y set position of the text on y
 * @param text  text to display on the oled
 */
void oled_print_5x8(uint8_t x, uint8_t y, const char *text)
{
  while (*text)
  {
    oled_draw_char5x8(x, y, *text++);
    x += 6;
  }
}

/**
 * @fn oled_draw_hline
 *
 * @brief Draw a horizontal line
 *
 * @param x starting x position
 * @param y position on y axis
 * @param length size of the line
 * @param color 0 = off, 1 = on
 */
void oled_draw_hline(uint8_t x, uint8_t y, uint8_t length, uint8_t color)
{
    if (y >= OLED_HEIGHT) return;
    
    if (x + length > OLED_WIDTH) {
        length = OLED_WIDTH - x;
    }

    if (length == 0) return;

    for (uint8_t i = 0; i < length; i++)
    {
        oled_set_pixel(x + i, y, color);
    }
}

/**
 * @fn oled_draw_vline
 *
 * @brief Draw a vertical line
 *
 * @param x position on x axis
 * @param y starting y position
 * @param length size of the line
 * @param color 0 = off, 1 = on
 */
void oled_draw_vline(uint8_t x, uint8_t y, uint8_t length, uint8_t color)
{
    if (x >= OLED_WIDTH) return;
    
    if (y + length > OLED_HEIGHT) {
        length = OLED_HEIGHT - y;
    }
    if (length == 0) return;

    for (uint8_t i = 0; i < length; i++)
    {
        oled_set_pixel(x, y + i, color);
    }
}

/**
 * @fn oled_draw_rect
 *
 * @brief Draw an outlined rectangle
 *
 * @param x starting x position
 * @param y starting y position
 * @param width rectangle width
 * @param height rectangle height
 * @param color 0 = off, 1 = on
 */
void oled_draw_rect(uint8_t x, uint8_t y, uint8_t width, uint8_t height, uint8_t color)
{
    if (width <= 0 || height <= 0)
        return;

    // Top
    oled_draw_hline(x, y, width, color);

    // Bottom
    oled_draw_hline(x, y + height - 1, width, color);

    // Left
    oled_draw_vline(x, y, height, color);

    // Right
    oled_draw_vline(x + width - 1, y, height, color);
}