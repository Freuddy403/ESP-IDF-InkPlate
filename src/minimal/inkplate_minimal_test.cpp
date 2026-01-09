#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "drivers/eink_6.hpp"
#include "mcp23017.hpp"

// IMPORTANT: il faut que ces includes existent dans ton repo.
// Si ça ne compile pas, copie-moi l'erreur et je corrige les includes.
#include "wire.hpp"

static const char *TAG = "INKPLATE_MIN";

// Petit helper: met/unset un pixel dans un buffer 1-bit (800x600)
// Hypothèse classique: 1 bit/pixel, MSB-first par byte.
// Si l'image est "inversée" ou bizarre, on ajustera l'ordre des bits.
static inline void fb1_set_pixel(uint8_t *data, int w, int x, int y, bool black)
{
    const int byte_index = (y * w + x) >> 3;
    const int bit_index  = 7 - (x & 7);
    if (black) data[byte_index] |=  (1 << bit_index);
    else       data[byte_index] &= ~(1 << bit_index);
}

extern "C" void inkplate_minimal_test(void)
{
    ESP_LOGI(TAG, "Boot minimal Inkplate6 test...");

    // 1) Init I2C (Wire)
    //    -> setup exact dépend de wire.hpp/wire.cpp; souvent c'est Wire::setup() ou Wire::begin()
    //    Je mets une forme "typique". Si ça ne compile pas, on ajuste.
    ESP_LOGI(TAG, "Init I2C...");
    Wire::get_singleton().setup();

    // 2) Init MCP23017 (expander I/O)
    ESP_LOGI(TAG, "Init MCP23017...");
    //MCP23017 mcp;
    MCP23017 mcp(0x20);
    mcp.setup();   // <-- si ça ne compile pas: on ajuste selon ton MCP23017

    // 3) Init EInk6
    ESP_LOGI(TAG, "Init EInk6...");
    EInk6 eink(mcp);
    if (!eink.setup()) {
        ESP_LOGE(TAG, "EInk6 setup() failed");
        return;
    }

    // 4) Créer framebuffer 1-bit
    ESP_LOGI(TAG, "Alloc framebuffer...");
    FrameBuffer1Bit *fb = eink.new_frame_buffer_1bit();
    if (!fb) {
        ESP_LOGE(TAG, "new_frame_buffer_1bit() failed");
        return;
    }

    uint8_t *data = fb->get_data(); // via FrameBuffer1BitX::get_data()
    // Remplir fond écran
    memset(data, 0x00, EInk6::BITMAP_SIZE_1BIT); //fond blanc
    //memset(data, 0xFF, EInk6::BITMAP_SIZE_1BIT); //fond noir

    // === TEST RAW: bande verticale pleine AU CENTRE ===
    const int W = EInk6::WIDTH;
    const int H = EInk6::HEIGHT;
    const int bytes_per_line = EInk6::LINE_SIZE_1BIT; // 800/8 = 100

    const int x0 = (W / 2) - 8;      // début bande (16 px de large) centrée
    const int byte0 = x0 / 8;        // index d’octet dans la ligne

    for (int y = 0; y < H; y++) {
        int row = y * bytes_per_line;
        data[row + byte0 + 0] = 0xFF;  // 8 pixels noirs
        data[row + byte0 + 1] = 0xFF;  // 8 pixels noirs => bande 16 px
    }



    // // 5) Dessiner quelque chose de visible: bordure + diagonales
    // const int W = EInk6::WIDTH;
    // const int H = EInk6::HEIGHT;

    // // Bordure
    // for (int x = 0; x < W; x++) {
    //     fb1_set_pixel(data, W, x, 0, true);
    //     fb1_set_pixel(data, W, x, H - 1, true);
    // }
    // for (int y = 0; y < H; y++) {
    //     fb1_set_pixel(data, W, 0, y, true);
    //     fb1_set_pixel(data, W, W - 1, y, true);
    // }

    // // Diagonales
    // for (int i = 0; i < H && i < W; i++) {
    //     fb1_set_pixel(data, W, i, i, true);
    //     fb1_set_pixel(data, W, W - 1 - i, i, true);
    // }

    // // Petit carré plein
    // for (int y = 50; y < 150; y++) {
    //     for (int x = 50; x < 250; x++) {
    //         fb1_set_pixel(data, W, x, y, true);
    //     }
    // }

    // 6) Update écran
    ESP_LOGI(TAG, "Updating e-ink (full refresh)...");
    eink.update(*fb);

    ESP_LOGI(TAG, "Done. Leaving framebuffer allocated for now.");
    // (on ne delete pas fb pour éviter surprises, tu peux l'ajouter après)
    while (true) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
