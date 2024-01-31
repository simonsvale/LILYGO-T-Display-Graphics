#include "graphics.h"

void setup_display(esp_lcd_panel_handle_t *panel_handle)
{
    gpio_config_t bk_gpio_config = {
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = 1ULL << PIN_NUM_BK_LIGHT
    };

    // Initialize the GPIO of backlight
    ESP_ERROR_CHECK(gpio_config(&bk_gpio_config));

    // Configure the bus for sending data to the screen.
    spi_bus_config_t bus_config = {
        .sclk_io_num = PIN_NUM_SCLK,
        .mosi_io_num = PIN_NUM_MOSI,
        .max_transfer_sz = SCREEN_WIDTH * sizeof(uint16_t)
    };

    // Initialize the SPI bus
    ESP_ERROR_CHECK(spi_bus_initialize(LCD_HOST, &bus_config, SPI_DMA_CH_AUTO));

    esp_lcd_panel_io_handle_t io_handle = NULL;
    esp_lcd_panel_io_spi_config_t io_config = {
        .dc_gpio_num = PIN_NUM_DC,
        .cs_gpio_num = PIN_NUM_CS,
        .pclk_hz = LCD_PIXEL_CLOCK_HZ,
        .lcd_cmd_bits = LCD_CMD_BITS,
        .lcd_param_bits = LCD_PARAM_BITS,
        .spi_mode = 0,
        .trans_queue_depth = 10,
    };

    // Attach the LCD to the SPI bus
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)LCD_HOST, &io_config, &io_handle));

    esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = PIN_NUM_RST,
        .rgb_endian = LCD_RGB_ENDIAN_RGB,   // The endian of the color, e.i. RGB or BGR
        .bits_per_pixel = 16,
    };

    // Initialize the LCD configuration on the lily TTGO t-display
    ESP_ERROR_CHECK(esp_lcd_new_panel_st7789(io_handle, &panel_config, panel_handle));

    // Turn off backlight to avoid unpredictable display on the LCD screen while initializing
    // the LCD panel driver. (Different LCD screens may need different levels)
    ESP_ERROR_CHECK(gpio_set_level(PIN_NUM_BK_LIGHT, LCD_BK_LIGHT_OFF_LEVEL));

    // Reset the display
    ESP_ERROR_CHECK(esp_lcd_panel_reset(*panel_handle));

    // Initialize LCD panel
    ESP_ERROR_CHECK(esp_lcd_panel_init(*panel_handle));

    // Turn on the screen
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(*panel_handle, true));

    ESP_ERROR_CHECK(esp_lcd_panel_invert_color(*panel_handle, true));

    // Turn on backlight (Different LCD screens may need different levels)
    ESP_ERROR_CHECK(gpio_set_level(PIN_NUM_BK_LIGHT, LCD_BK_LIGHT_ON_LEVEL));

    ESP_LOGI(TAG_DISPLAY, "Display set up!");
}


int fill_rect(esp_lcd_panel_handle_t panel_handle, draw_t draw_params, uint16_t RGB_color)
{
    // Sanity check, if pixel position parameters exceeds screen bounds.
    // Sanity checks.
    if ( ( (draw_params.draw_start_x-1) > SCREEN_WIDTH) || ( (draw_params.draw_start_y-1) > SCREEN_HEIGHT) )
    {
        ESP_LOGE(TAG_DISPLAY, "Draw starting position out of bounds.");
        return DRAW_FAILURE;
    }

    if( (((draw_params.image_size_x + draw_params.draw_start_x)-1) > SCREEN_WIDTH) || (((draw_params.image_size_y + draw_params.draw_start_y)-1) > SCREEN_HEIGHT) )
    {
        ESP_LOGE(TAG_DISPLAY, "Draw starting postion + image_size, is out of bounds.");
        return DRAW_FAILURE;
    }

    // Foreground color
    uint16_t BGR_color = COLOR_SWAP(RGB_color);  

    int color_buffer_size = draw_params.image_size_x * draw_params.image_size_y * sizeof(uint16_t);

    // Since we do not know the size of the color buffer, before execution we allocate memory for it.
    uint16_t *color_buffer = NULL;

    color_buffer = malloc(color_buffer_size);
    if (color_buffer == NULL)
    {
        ESP_LOGE(TAG_DISPLAY, "Memory could not be allocated to buffer.");
        return DRAW_FAILURE;
    }

    // Set the correct color:
    memset(color_buffer, BGR_color, color_buffer_size);

    // Draw call to the LCD.
    esp_lcd_panel_draw_bitmap(panel_handle, 
        draw_params.draw_start_x + SCREEN_WIDTH_PIXEL_MISALIGNMENT, 
        draw_params.draw_start_y + SCREEN_HEIGHT_PIXEL_MISALIGNMENT, 
        draw_params.draw_start_x + draw_params.image_size_x + SCREEN_WIDTH_PIXEL_MISALIGNMENT, 
        draw_params.draw_start_y + draw_params.image_size_y + SCREEN_HEIGHT_PIXEL_MISALIGNMENT, 
        color_buffer
    );

    // Free allocated memory color_buffer.
    free(color_buffer);

    // Read somewhere this is needed to prevent visual artifacts, turns out it is true.
    vTaskDelay(10 / portTICK_PERIOD_MS);
    
    return DRAW_SUCCESS;
}


int fill_display(esp_lcd_panel_handle_t panel_handle, uint16_t RGB_color)
{
    draw_t params;
    params.draw_start_x = 0;
    params.draw_start_y = 0;
    params.image_size_x = 135;
    params.image_size_y = 240;

    // Fill.
    if (fill_rect(panel_handle, params, RGB_color) != 0)
    {
        ESP_LOGE(TAG_DISPLAY, "Fill failed.");
        return DRAW_FAILURE;
    }

    return DRAW_SUCCESS;
}

void int_to_color_array(color_def_t colors, uint16_t *int_buffer, int buffer_size)
{
    // Based on the 8 selected colors, convert the integers to colors.
    for(int i = 0; i < buffer_size; ++i )
    {
        if (int_buffer[i] == colors.COLOR_ID_0)
        {
            int_buffer[i] = colors.COLOR_0;
            continue;
        }

        if (int_buffer[i] == colors.COLOR_ID_1)
        {
            int_buffer[i] = colors.COLOR_1;
            continue;
        }

        if (int_buffer[i] == colors.COLOR_ID_2)
        {
            int_buffer[i] = colors.COLOR_2;
            continue;
        }

        if (int_buffer[i] == colors.COLOR_ID_3)
        {
            int_buffer[i] = colors.COLOR_3;
            continue;
        }

        if (int_buffer[i] == colors.COLOR_ID_4)
        {
            int_buffer[i] = colors.COLOR_4;
            continue;
        }

        if (int_buffer[i] == colors.COLOR_ID_5)
        {
            int_buffer[i] = colors.COLOR_5;
            continue;
        }

        if (int_buffer[i] == colors.COLOR_ID_6)
        {
            int_buffer[i] = colors.COLOR_6;
            continue;
        }

        if (int_buffer[i] == colors.COLOR_ID_7)
        {
            int_buffer[i] = colors.COLOR_7;
            continue;
        }
    }
}

char *int_to_char_array(int number, int *str_size)
{       
    short arbitrary_size = 32;

    // Create null ptr.
    char *number_str = NULL;
    
    // Allocate memory and set it to 0.
    number_str = (char *)calloc(arbitrary_size, sizeof(char));

    // Put number into char array.
    sprintf(number_str, "%d", number);

    // Find out how many characters the number consists of.
    for (int i = 0; i < arbitrary_size; ++i)
    {
        if (number_str[i] == 0)
        {
            *str_size = i;
            break;
        }
    }

    return number_str;
}


int draw_bgr_image(esp_lcd_panel_handle_t panel_handle, draw_t draw_params, uint16_t *image_buffer)
{
    // Sanity checks.
    if (image_buffer == NULL)
    {
        ESP_LOGE(TAG_DISPLAY, "Cannot draw, image buffer is a NULL pointer.");
        return DRAW_FAILURE;
    }

    if ( ( (draw_params.draw_start_x-1) > SCREEN_WIDTH) || ( (draw_params.draw_start_y-1) > SCREEN_HEIGHT) )
    {
        ESP_LOGE(TAG_DISPLAY, "Draw starting position out of bounds.");
        return DRAW_FAILURE;
    }

    if( (((draw_params.image_size_x + draw_params.draw_start_x)-1) > SCREEN_WIDTH) || (((draw_params.image_size_y + draw_params.draw_start_y)-1) > SCREEN_HEIGHT) )
    {
        ESP_LOGE(TAG_DISPLAY, "Draw starting postion + image_size, is out of bounds.");
        return DRAW_FAILURE;
    }

    // Draw call to the LCD.
    esp_lcd_panel_draw_bitmap(panel_handle, 
        draw_params.draw_start_x + SCREEN_WIDTH_PIXEL_MISALIGNMENT, 
        draw_params.draw_start_y + SCREEN_HEIGHT_PIXEL_MISALIGNMENT, 
        draw_params.draw_start_x + draw_params.image_size_x + SCREEN_WIDTH_PIXEL_MISALIGNMENT, 
        draw_params.draw_start_y + draw_params.image_size_y + SCREEN_HEIGHT_PIXEL_MISALIGNMENT, 
        image_buffer
    );

    // Read somewhere this is needed to prevent visual artifacts, turns out it is true.
    vTaskDelay(10 / portTICK_PERIOD_MS);

    return DRAW_SUCCESS;
}


void RGB_TO_BGR(uint16_t *image_buffer, int buffer_size)
{
    // Go through every index and convert the RGB value to BGR format. Since this is the format the TTGO T-Display supports.
    for (size_t i = 0; i < buffer_size; ++i)
    {
        image_buffer[i] = COLOR_SWAP(image_buffer[i]); 
    }
}


uint16_t *scale_image(draw_t draw_params, uint16_t *image_buffer)
{
    // Sanity check, we cannot scale with 0.
    if ( (draw_params.scale_x == 0) || (draw_params.scale_y == 0) )
    {
        ESP_LOGE(TAG_DISPLAY, "Scaling cannot be 0.");
        return NULL;
    }

    // Allocate memory for the pointer to the scaled image.
    uint16_t *scaled_image_buffer_ptr = NULL;
    scaled_image_buffer_ptr = (uint16_t *)malloc( (draw_params.image_size_x * draw_params.scale_x) * (draw_params.image_size_y * draw_params.scale_y) * sizeof(uint16_t) );
    if (scaled_image_buffer_ptr == NULL)
    {
        ESP_LOGE(TAG_DISPLAY, "Memory could not be allocated to scaled image.");
        return NULL;
    }

    // Generate the scaled image.
    // For all lines in the image
    for (int pixel_line = 0; pixel_line < draw_params.image_size_y; ++pixel_line)
    {
        // For all pixels in that line.
        for (int pixel_offset = 0; pixel_offset < draw_params.image_size_x; ++pixel_offset)
        {
            for (int scale_offset = 0; scale_offset < draw_params.scale_x; ++scale_offset)
            {
                // Create a single line of scale * pixels.
                scaled_image_buffer_ptr[(pixel_line * draw_params.image_size_x * draw_params.scale_x * draw_params.scale_y) + (pixel_offset * draw_params.scale_x) + scale_offset] = image_buffer[pixel_line * draw_params.image_size_x + pixel_offset];
            }   
        }
        
        // Now memcpy this line of scaled pixels to the line(s) just underneath.
        for (int line_offset = 1; line_offset < draw_params.scale_y; ++line_offset)
        {
            memcpy(
                scaled_image_buffer_ptr + (pixel_line * draw_params.image_size_x * draw_params.scale_x * draw_params.scale_y) + (draw_params.image_size_x * draw_params.scale_x) * line_offset, 
                scaled_image_buffer_ptr + (pixel_line * draw_params.image_size_x * draw_params.scale_x * draw_params.scale_y), 
                draw_params.scale_x * draw_params.image_size_x * sizeof(uint16_t) 
            );
        }
        
    }

    return scaled_image_buffer_ptr;
}


uint16_t *get_LASET_bitmap_letter_font(glyph_t *glyph_params, uint16_t letter_color, uint16_t background_color)
{
    // Set relvant glyph parameters.
    glyph_params->glyph_size_x = 5;
    glyph_params->glyph_size_y = 6;
    glyph_params->glyph_amount = 28;
    glyph_params->ASCII_offset = 97;

    // Allocate memory for bitmap font.
    uint16_t *LASET_font = NULL;
    LASET_font = (uint16_t *)malloc( (glyph_params->glyph_size_x * glyph_params->glyph_size_y) * glyph_params->glyph_amount * sizeof(uint16_t) );
    if (LASET_font == NULL)
    {
        ESP_LOGE(TAG_DISPLAY, "Memory could not be allocated in get_LASET_bitmap_letter_font().");
        return NULL;
    }

    /* Glyph buffer, containing all letters in the english alphabet, in the array format.
       It is formatted so each letter is contained in a 5 * 6 indices buffer.
       It is made this way so each letter can be individually memcpy'd out of the buffer and onto the LCD display.
    */
    uint16_t letter_buffer[] = 
    {
        0,0,0,0,0,0,1,1,0,0,0,0,0,1,0,0,1,1,1,0,1,0,0,1,0,0,1,1,0,1,
        1,0,0,0,0,1,0,0,0,0,1,1,1,0,0,1,0,0,1,0,1,0,0,1,0,1,1,1,0,0,
        0,0,0,0,0,0,1,1,0,0,1,0,0,1,0,1,0,0,0,0,1,0,0,1,0,0,1,1,0,0,
        0,0,0,1,0,0,0,0,1,0,0,1,1,1,0,1,0,0,1,0,1,0,0,1,0,0,1,1,0,1,
        0,0,0,0,0,0,1,1,0,0,1,0,0,1,0,1,1,1,1,0,1,0,0,0,0,0,1,1,1,0,
        0,1,1,0,0,0,1,0,0,0,0,1,0,0,0,1,1,1,0,0,0,1,0,0,0,0,1,0,0,0,
        0,0,0,0,0,0,1,1,1,0,1,0,0,1,0,0,1,1,1,0,0,0,0,1,0,1,1,1,0,0,
        1,0,0,0,0,1,0,0,0,0,1,1,1,0,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,
        1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,
        0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,0,
        1,0,0,0,0,1,0,0,0,0,1,0,1,0,0,1,1,0,0,0,1,0,1,0,0,1,0,1,0,0,
        1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,
        0,0,0,0,0,0,1,0,1,0,1,0,1,0,1,1,0,0,0,1,1,0,0,0,1,1,0,0,0,1,
        0,0,0,0,0,1,1,1,0,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,
        0,0,0,0,0,0,1,1,0,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,0,1,1,0,0,
        0,1,1,0,0,1,0,0,1,0,1,0,0,1,0,1,1,1,0,0,1,0,0,0,0,1,0,0,0,0,
        0,1,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,1,1,0,0,0,0,1,0,0,0,0,1,0,
        0,0,0,0,0,1,0,1,1,0,1,1,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,
        0,0,0,0,0,0,1,1,1,0,1,0,0,0,0,0,1,1,0,0,0,0,0,1,0,1,1,1,0,0,
        0,1,0,0,0,1,1,1,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,
        0,0,0,0,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,0,1,1,0,0,
        0,0,0,0,0,0,0,0,0,0,1,0,0,0,1,1,0,0,0,1,0,1,0,1,0,0,0,1,0,0,
        0,0,0,0,0,0,0,0,0,0,1,0,0,0,1,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,
        0,0,0,0,0,1,0,0,0,1,0,1,0,1,0,0,0,1,0,0,0,1,0,1,0,1,0,0,0,1,
        0,0,0,0,0,1,0,0,1,0,1,0,0,1,0,0,1,1,1,0,0,0,0,1,0,0,1,1,0,0,
        0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,1,0,0,0,1,0,0,0,1,1,1,1,0,
        0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
    };

    // memcpy the glyphs into the ptr.
    memcpy(LASET_font, 
        letter_buffer, 
        glyph_params->glyph_size_x * glyph_params->glyph_size_y * glyph_params->glyph_amount * sizeof(uint16_t)
    );

    // Set color of glyphs.
    color_def_t colors;
    colors.COLOR_0 = background_color;
    colors.COLOR_ID_0 = 0;
    colors.COLOR_1 = letter_color;
    colors.COLOR_ID_1 = 1;
    
    // Convert int array to uint16_t color array.
    int_to_color_array(colors, LASET_font, glyph_params->glyph_size_x * glyph_params->glyph_size_y * glyph_params->glyph_amount);

    // Convert from RGB to BGR.
    RGB_TO_BGR(LASET_font, glyph_params->glyph_size_x * glyph_params->glyph_size_y * glyph_params->glyph_amount);


    // Since we cannot return arrays, we allocate memory for an array on the heap and return the pointer to that array.
    return LASET_font;
}

uint16_t *get_LASET_bitmap_numbers_font(glyph_t *glyph_params, uint16_t number_color, uint16_t background_color)
{
    glyph_params->glyph_size_x = 3;
    glyph_params->glyph_size_y = 5;
    glyph_params->glyph_amount = 18;
    glyph_params->ASCII_offset = 40;

    uint16_t *LASET_font = NULL;
    LASET_font = (uint16_t *)malloc( (glyph_params->glyph_size_x * glyph_params->glyph_size_y) * glyph_params->glyph_amount * sizeof(uint16_t) );
    if (LASET_font == NULL)
    {
        ESP_LOGE(TAG_DISPLAY, "Memory could not be allocated in get_LASET_bitmap_letter_font().");
        return NULL;
    }

    uint16_t number_buffer[] = 
    {
        0,1,0,1,0,0,1,0,0,1,0,0,0,1,0,
        0,1,0,0,0,1,0,0,1,0,0,1,0,1,0,
        0,0,0,0,1,0,1,1,1,0,1,0,1,0,1,
        0,0,0,0,1,0,1,1,1,0,1,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,1,0,1,0,0,
        0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,
        0,0,0,0,1,0,0,1,0,1,0,0,1,0,0,
        1,1,1,1,0,1,1,0,1,1,0,1,1,1,1, 
        0,1,0,1,1,0,0,1,0,0,1,0,1,1,1,
        1,1,0,0,0,1,0,1,0,1,0,0,1,1,1,
        1,1,0,0,0,1,0,1,0,0,0,1,1,1,0,
        1,0,1,1,0,1,1,1,1,0,0,1,0,0,1,
        1,1,1,1,0,0,1,1,1,0,0,1,1,1,1,
        1,1,1,1,0,0,1,1,1,1,0,1,1,1,1,
        1,1,1,0,0,1,0,0,1,0,1,0,0,1,0,
        1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,
        1,1,1,1,0,1,1,1,1,0,0,1,0,0,1
    };

    // memcpy the glyphs into the ptr.
    memcpy(LASET_font, 
        number_buffer, 
        glyph_params->glyph_size_x * glyph_params->glyph_size_y * glyph_params->glyph_amount * sizeof(uint16_t)
    );

    // Set color of glyphs.
    color_def_t colors;
    colors.COLOR_0 = background_color;
    colors.COLOR_ID_0 = 0;
    colors.COLOR_1 = number_color;
    colors.COLOR_ID_1 = 1;
    
    // Convert int array to uint16_t color array.
    int_to_color_array(colors, LASET_font, glyph_params->glyph_size_x * glyph_params->glyph_size_y * glyph_params->glyph_amount);

    // Convert from RGB to BGR.
    RGB_TO_BGR(LASET_font, glyph_params->glyph_size_x * glyph_params->glyph_size_y * glyph_params->glyph_amount);

    return LASET_font;
}


int draw_glyphs(esp_lcd_panel_handle_t panel_handle, glyph_t text_params, uint16_t *glyph_font , char *text_buffer, unsigned int buffer_size)
{
    // Create NULL pointer.
    uint16_t *selected_glyph = NULL;
    uint16_t *scaled_glyph = NULL;

    // Create structure for drawing.
    draw_t draw;
    draw.draw_start_x = text_params.glyph_start_x;
    draw.draw_start_y = text_params.glyph_start_y;
    draw.image_size_x = text_params.glyph_size_x;
    draw.image_size_y = text_params.glyph_size_y;

    unsigned short x_aux = text_params.glyph_size_x;
    unsigned short y_aux = text_params.glyph_size_y;

    // Read each letter in the char buffer and display them.
    for (int i = 0; i < buffer_size; ++i)
    {
        selected_glyph = select_glyph(text_params, glyph_font, text_buffer[i]);

        // Scale glyph
        if (text_params.glyph_scale <= 1)
        {
            // Draw letter on LCD
            draw_bgr_image(panel_handle, draw, selected_glyph);
        }
        else
        {
            draw.scale_x = text_params.glyph_scale;
            draw.scale_y = text_params.glyph_scale;
            
            // Scale image
            scaled_glyph = scale_image(draw, selected_glyph);

            draw.image_size_x *= text_params.glyph_scale;
            draw.image_size_y *= text_params.glyph_scale;

            // Draw letter on LCD
            draw_bgr_image(panel_handle, draw, scaled_glyph);

            draw.image_size_x = x_aux;
            draw.image_size_y = y_aux;
        }

        // Free allocated memory.
        free(scaled_glyph);
        free(selected_glyph);

        // Move where to draw.
        draw.draw_start_x += (text_params.glyph_spacing + text_params.glyph_size_x) * text_params.glyph_scale;
    }
    return DRAW_SUCCESS;
}


uint16_t *select_glyph(glyph_t text_params, uint16_t *glyph_buffer, char glyph)
{
    // Create void ptr.
    uint16_t *glyph_ptr = NULL;

    // Allocate memory to glyph.
    glyph_ptr = (uint16_t *)malloc(text_params.glyph_size_x * text_params.glyph_size_y * sizeof(uint16_t) );
    if (glyph_ptr == NULL)
    {
        ESP_LOGE(TAG_DISPLAY, "Memory could not be allocated in select_glyph().");
        return NULL;
    }
    
    // Get selected glyph from char. Subtracted by glyph_ASCII_offset. Ex. 'a' in ASCII is 97, and if a is the first letter in a buffer, the glyph_ASCII_offset is 97.
    unsigned int glyph_number = glyph - text_params.ASCII_offset;

    // Copy the selected glyph into the buffer with allocated memory.
    memcpy(glyph_ptr, 
        glyph_buffer + (text_params.glyph_size_x * text_params.glyph_size_y) * glyph_number, 
        text_params.glyph_size_x * text_params.glyph_size_y * sizeof(uint16_t)
    );

    // Return the glyph as a pointer.
    return glyph_ptr;
}


int draw_number(esp_lcd_panel_handle_t panel_handle, glyph_t number_params, uint16_t *number_font, int number)
{   
    // Get size of string from the integer
    char* int_str = NULL;
    int str_size;
    int_str = int_to_char_array(number, &str_size);

    if (draw_glyphs(panel_handle, number_params, number_font, int_str, str_size) != 0)
    {
        return DRAW_FAILURE;
    }

    // Free allocated memory.
    free(int_str);

    return DRAW_SUCCESS;
}
