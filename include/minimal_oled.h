#pragma once

#include "driver/gpio.h"
#include "driver/i2c_master.h"
#include "string.h"

#ifdef __cplusplus
extern C {
#endif

i2c_master_bus_config_t oled_init_i2c(void);
void oled_init(i2c_master_bus_handle_t i2c_bus_handle);
void oled_set_position(uint8_t x, uint8_t y);
void oled_flush(void);
void oled_clear_buffer(void);
void oled_clear(void);
void oled_set_pixel(int16_t x, int16_t y, uint8_t color);
void oled_draw_bmp(int16_t x, int16_t y, int16_t w, int16_t h, const uint8_t *bitmap);
void oled_draw_char8x8(uint8_t x, uint8_t y, char c);
void oled_print_8x8(uint8_t x, uint8_t y, const char *text);
void oled_draw_char6x8(uint8_t x, uint8_t y, char c);
void oled_print_6x8(uint8_t x, uint8_t y, const char *text);
void oled_draw_char5x8(uint8_t x, uint8_t y, char c);
void oled_print_5x8(uint8_t x, uint8_t y, const char *text);
void oled_draw_hline(uint8_t x, uint8_t y, uint8_t length, uint8_t color);
void oled_draw_vline(uint8_t x, uint8_t y, uint8_t length, uint8_t color);
void oled_draw_rect(uint8_t x, uint8_t y, uint8_t width, uint8_t height, uint8_t color);

#ifdef __cplusplus
extern C }
#endif