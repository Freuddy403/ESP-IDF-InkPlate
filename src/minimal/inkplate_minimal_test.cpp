#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "drivers/eink_6.hpp"
#include "mcp23017.hpp"

#include "graphical/graphics.hpp"
#include "drivers/eink_6.hpp"
#include "graphical/adafruit_gfx.hpp"

#include "wire.hpp"
#include "inkplate_platform.hpp"

static const char *TAG = "INKPLATE_MIN";
int ls = 0;

static const uint8_t font8x8_minimal[][8] = {
    // ' ' (space)
    {0,0,0,0,0,0,0,0},
    // '0'
    {0x3C,0x42,0x46,0x4A,0x52,0x62,0x3C,0},
    // '6'
    {0x3C,0x42,0x40,0x7C,0x42,0x42,0x3C,0},
    // 'A'
    {0x18,0x24,0x42,0x7E,0x42,0x42,0x42,0},
    // 'E'
    {0x7E,0x40,0x40,0x7C,0x40,0x40,0x7E,0},
    // 'I'
    {0x7F,0x08,0x08,0x08,0x08,0x08,0x7F,0},
    // 'K'
    {0x42,0x44,0x48,0x70,0x48,0x44,0x42,0},
    // 'L'
    {0x40,0x40,0x40,0x40,0x40,0x40,0x7E,0},
    // 'N'
    {0x42,0x62,0x52,0x4A,0x46,0x42,0x42,0},
    // 'O'
    {0x3C,0x42,0x42,0x42,0x42,0x42,0x3C,0},
    // 'P'
    {0x7C,0x42,0x42,0x7C,0x40,0x40,0x40,0},
    // 'S'
    {0x3C,0x42,0x40,0x3C,0x02,0x42,0x3C,0},
    // 'T'
    {0x7F,0x08,0x08,0x08,0x08,0x08,0x08,0},
};

static int font_index(char c)
{
    switch (c) {
        case ' ': return 0;
        case '0': return 1;
        case '6': return 2;
        case 'A': return 3;
        case 'E': return 4;
        case 'I': return 5;
        case 'K': return 6;
        case 'L': return 7;
        case 'N': return 8;
        case 'O': return 9;
        case 'P': return 10;
        case 'S': return 11;
        case 'T': return 12;
        default:  return 0;
    }
}

static inline void fb1_set_pixel_lsb(uint8_t *data, int bytes_per_line, int x, int y, bool black)
{
    int byte_index = bytes_per_line * y + (x >> 3);
    uint8_t mask = (1u << (x & 7));   // LSB-first
    if (black) data[byte_index] |= mask;
    else       data[byte_index] &= ~mask;
}

static void draw_char_8x8(uint8_t *fb, int ls, int x, int y, char c)
{
    int idx = font_index(c);
    const uint8_t *g = font8x8_minimal[idx];

    for (int row = 0; row < 8; row++) {
        uint8_t bits = g[row];
        for (int col = 0; col < 8; col++) {
            if (bits & (1 << (7 - col))) {
                fb1_set_pixel_lsb(fb, ls, x + col, y + row, true);
            }
        }
    }
}

// 

static void draw_text(uint8_t *fb, int ls, int x, int y, const char *s)
{
    while (*s) {
        draw_char_8x8(fb, ls, x, y, *s);
        x += 8;
        s++;
    }
}

static void draw_char_8x8_scaled(uint8_t *fb, int ls, int x, int y, char c, int scale)
{
    int idx = font_index(c);
    const uint8_t *g = font8x8_minimal[idx];

    for (int row = 0; row < 8; row++) {
        uint8_t bits = g[row];
        for (int col = 0; col < 8; col++) {
            if (bits & (1 << (7 - col))) {
                // bloc scale x scale
                for (int dy = 0; dy < scale; dy++) {
                    for (int dx = 0; dx < scale; dx++) {
                        fb1_set_pixel_lsb(fb, ls, x + col*scale + dx, y + row*scale + dy, true);
                    }
                }
            }
        }
    }
}

static void draw_text_scaled(uint8_t *fb, int ls, int x, int y, const char *s, int scale)
{
    while (*s) {
        draw_char_8x8_scaled(fb, ls, x, y, *s, scale);
        x += 8 * scale;
        s++;
    }
}


extern "C" void inkplate_minimal_test(void)
{
    ESP_LOGI(TAG, "Boot minimal Inkplate6 test...");

    // 1) Init I2C (Wire)
    ESP_LOGI(TAG, "Init I2C...");
    Wire::get_singleton().setup();

    // 2) Init MCP23017 (expander I/O)
    ESP_LOGI(TAG, "Init MCP23017...");
    mcp_int.setup();   // <-- si ça ne compile pas: on ajuste selon ton MCP23017

    // 3) Init EInk6
    ESP_LOGI(TAG, "Init EInk6...");
    if (!e_ink.setup()) {
        ESP_LOGE(TAG, "EInk6 setup() failed");
        return;
    }

    ls = EInk6::LINE_SIZE_1BIT; // 800/8 = 100

    Graphics g(EInk6::WIDTH, EInk6::HEIGHT);
    g.setDisplayMode(DisplayMode::INKPLATE_1BIT);
    g.clearDisplay();

    g.setCursor(50, 50);
    g.setTextSize(2);
    g.setTextColor(1);
    g.print("HELLO FONT TEST");

    g.display();

    // 4) Créer framebuffer 1-bit
    ESP_LOGI(TAG, "Alloc framebuffer...");
    FrameBuffer1Bit *fb = e_ink.new_frame_buffer_1bit();
    if (!fb) {
        ESP_LOGE(TAG, "new_frame_buffer_1bit() failed");
        return;
    }
    
    ESP_LOGI(TAG, "Done. Leaving framebuffer allocated for now.");
    // (on ne delete pas fb pour éviter surprises, tu peux l'ajouter après)
    while (true) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
