#ifndef GRAPHICS_H 
#define GRAPHICS_H

// ESP system headers
#include "esp_system.h"
#include "esp_log.h"

// For some reason memset is located here.
#include <string.h>

// LCD headers
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "esp_heap_caps.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"

#include <freertos/semphr.h>


#define LCD_HOST 1

// To speed up transfers, every SPI transfer sends a bunch of lines. This define specifies how many.
// More means more memory use, but less overhead for setting up / finishing transfers. Make sure 240
// is dividable by this.
#define PARALLEL_LINES 16

// define pin headers, see pinouts on Lily TTGO t-display datasheet.
#define LCD_PIXEL_CLOCK_HZ (20 * 1000 * 1000)
#define LCD_BK_LIGHT_ON_LEVEL  1
#define LCD_BK_LIGHT_OFF_LEVEL !LCD_BK_LIGHT_ON_LEVEL

#define PIN_NUM_MOSI 19
#define PIN_NUM_SCLK 18
#define PIN_NUM_CS 5
#define PIN_NUM_DC 16
#define PIN_NUM_RST 23
#define PIN_NUM_BK_LIGHT 4

// bits
#define LCD_CMD_BITS 8
#define LCD_PARAM_BITS 8


// Screen resolution
#define SCREEN_WIDTH 135
#define SCREEN_HEIGHT 240

// Possible due to compatibility wiht other displays, the pixels are misaligned.
#define SCREEN_WIDTH_PIXEL_MISALIGNMENT 52
#define SCREEN_HEIGHT_PIXEL_MISALIGNMENT 40


// Define colors, still needs COLOR_SWAP() macro to work, since we're going from RGB -> BGR.
#define LCD_RED 0xF800
#define LCD_GREEN 0x07E0
#define LCD_BLUE 0x001F
#define LCD_BLACK 0x0000
#define LCD_WHITE 0xFFFF
#define LCD_YELLOW 0xffE0
#define LCD_PINK 0xF81F
#define LCD_PURPLE 0x981F


#define LCD_LIGHT_PURPLE 0xC39E
#define LCD_AAU_BLUE 0x318F

// Want to choose your own color?
// https://rgbcolorpicker.com/565

// Header tag
#define TAG_DISPLAY "LASET_DISP"

// Redefinitions for return readability.
#define DRAW_SUCCESS 0
#define DRAW_FAILURE 1


// Byte swap macro for going from RGB to BGR
#define COLOR_SWAP(x) ((x >> 8) | (x << 8))


// LCD draw structure.
typedef struct {
    unsigned short scale_x;
    unsigned short scale_y;
    unsigned short draw_start_x;
    unsigned short draw_start_y;
    unsigned short image_size_x;
    unsigned short image_size_y;
} draw_t;

// Color structure.
typedef struct {
    uint16_t COLOR_0;
    uint16_t COLOR_1;
    uint16_t COLOR_2;
    uint16_t COLOR_3;
    uint16_t COLOR_4;
    uint16_t COLOR_5;
    uint16_t COLOR_6;
    uint16_t COLOR_7;

    short COLOR_ID_0;
    short COLOR_ID_1;
    short COLOR_ID_2;
    short COLOR_ID_3;
    short COLOR_ID_4;
    short COLOR_ID_5;
    short COLOR_ID_6;
    short COLOR_ID_7;
} color_def_t;

// Convert image to array of indices containing colors.
// https://notisrac.github.io/FileToCArray/

// Offset stucture.
typedef struct {
    unsigned int line;          // 1 to size of image side.
    int amount;
    uint16_t background_color;
    int write_image_buffer;         // 0 or 1, indicates wether or not we shuold write the offset changes to the actual image buffer, and not just draw over it.
} offset_t;

typedef struct {
    unsigned short glyph_start_x;
    unsigned short glyph_start_y;

    unsigned short glyph_amount;
    unsigned short glyph_size_x;
    unsigned short glyph_size_y;

    unsigned short glyph_spacing;
    unsigned short glyph_scale;
    short ASCII_offset;
} glyph_t;


// INFO: Graphics related functions, definitions, beware the screen retains some pixels when reset using the button.
// This can be prevented by cutting all power to the ESP32 for a few seconds.


// Setups the functionality for writing to the display, such as pinouts.
void setup_display(esp_lcd_panel_handle_t *panel_handle);


// Draws a rectangle given the draw_t specifications.
int fill_rect(esp_lcd_panel_handle_t panel_handle, draw_t draw_params, uint16_t RGB_color);

// Fills the display with a single color.
int fill_display(esp_lcd_panel_handle_t panel_handle, uint16_t RGB_color);

// Draw using indices in an array, using 8 configurable colors.
void int_to_color_array(color_def_t colors, uint16_t *int_buffer, int buffer_size);

// Takes an integer and turns it into a char array. Ex: -427 -> "-427"
char *int_to_char_array(int number, int *str_size);

// Optimized version of arr_draw_color
int draw_bgr_image(esp_lcd_panel_handle_t panel_handle, draw_t draw_params, uint16_t *image_buffer);

// Go from RGB color to BGR color, using pointer.
void RGB_TO_BGR(uint16_t *image_buffer, int buffer_size);

// Scale an image 1 to n, on x and y axis. Returns a pointer to the new scaled image buffer, else returns NULL ptr.
uint16_t *scale_image(draw_t draw_params, uint16_t *image_buffer);

// Return a pointer to the LASET bitmap font, containing the english alphabet.
uint16_t *get_LASET_bitmap_letter_font(glyph_t *glyph_params, uint16_t letter_color, uint16_t background_color);

// Return a pointer to the LASET bitmap numbers font, containing numbers from 0 - 9.
uint16_t *get_LASET_bitmap_numbers_font(glyph_t *glyph_params, uint16_t number_color, uint16_t background_color);

// Draws text from a char buffer, only letters!
int draw_glyphs(esp_lcd_panel_handle_t panel_handle, glyph_t text_params, uint16_t *glyph_font , char *text_buffer, unsigned int buffer_size);

// Internal call, but can be used. 
// Selects a glyph from a glyph buffer. Uses a single char and returns a pointer to that glyph.
uint16_t *select_glyph(glyph_t text_params, uint16_t *glyph_buffer, char glyph);

// Draws a number from an integer.
int draw_number(esp_lcd_panel_handle_t panel_handle, glyph_t number_params, uint16_t *number_font, int number);

#endif