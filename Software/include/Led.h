#ifndef LED_H
#define LED_H

#include "driver/rmt_encoder.h"
#include "driver/rmt_tx.h"

// Define LED strip configuration constants
#define RMT_LED_STRIP_RESOLUTION_HZ 10000000 // 10MHz resolution, 1 tick = 0.1us (led strip needs a high resolution)
#define RMT_RESOL_TO_TICKS 1000000
#define T0H 0.3
#define T0L 0.9
#define T1H 0.9
#define T1L 0.3
#define TRST 100
#define LEDTAG "LED"

/**
 * @brief LED class for controlling RGB LED strips using RMT.
 */
class LED
{
public:
    // Eliminar las funciones de copia y asignación
    LED(const LED &) = delete;
    LED &operator=(const LED &) = delete;

    // Función para acceder a la instancia única del GPS
    static LED &getInstance()
    {
        static LED instance; // Única instancia
        return instance;
    }

    /**
     * @brief Configuration structure for LED strip encoder.
     */
    typedef struct
    {
        uint32_t resolution; /*!< Encoder resolution, in Hz */
    } led_strip_encoder_config_t;

    /**
     * @brief Structure for RMT LED strip encoder.
     */
    typedef struct
    {
        rmt_encoder_t base;
        rmt_encoder_t *bytes_encoder;
        rmt_encoder_t *copy_encoder;
        int state;
        rmt_symbol_word_t reset_code;
    } rmt_led_strip_encoder_t;

    /**
     * @brief Pre-configured colors for the LED.
     */
    enum colors
    {
        black,
        red,
        blue,
        green,
        white,
        undefined
    };

    static void SetBrightness(uint8_t brillo_);
    static void Begin(void);
    static void SetLedColor(uint8_t R, uint8_t G, uint8_t B);
    static void Show(void);
    static void SetLedColor(colors color);
    static LED::colors GetLedColor(void);

private:
    // Constructor privado
    LED() {}

    static size_t rmt_encode_led_strip(rmt_encoder_t *encoder, rmt_channel_handle_t channel, const void *primary_data, size_t data_size, rmt_encode_state_t *ret_state);
    static esp_err_t rmt_del_led_strip_encoder(rmt_encoder_t *encoder);
    static esp_err_t rmt_led_strip_encoder_reset(rmt_encoder_t *encoder);
    static esp_err_t rmt_new_led_strip_encoder(const led_strip_encoder_config_t *config, rmt_encoder_handle_t *ret_encoder);

    static rmt_channel_handle_t led_chan;   ///< RMT channel handle for LED
    static rmt_encoder_handle_t led_encoder;///< RMT encoder handle for LED
    static rmt_transmit_config_t tx_config; ///< RMT transmit configuration for LED
    static uint8_t brillo;                  ///< Brightness level for LED
    static uint8_t ledColor[3];             ///< RGB values for LED color
    static colors CurrColor;                ///< Current LED color
};

#endif // LED_H