#ifndef LED_H
#define LED_H

#include "CommonIncludes.h"
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

    /**
     * @brief Set the brightness level for the LED.
     *
     * @param brillo_ Brightness level for the LED (0-100)
     */
    static void SetBrightness(uint8_t brillo_);

    /**
     * @brief Initialize the LED strip.
     */
    static void Begin(void);

    /**
     * @brief Set the color for the LED.
     *
     * @param R Red value (0-255)
     * @param G Green value (0-255)
     * @param B Blue value (0-255)
     */
    static void SetLedColor(uint8_t R, uint8_t G, uint8_t B);

    /**
     * @brief Show the LED color.
     */
    static void Show(void);

    /**
     * @brief Set the color for the LED.
     *
     * @param color Color to set
     */
    static void SetLedColor(colors color);

    /**
     * @brief Get the color for the LED.
     *
     * @return colors Current LED color
     */
    static LED::colors GetLedColor(void);

private:
    // Constructor privado
    LED() {}

    /**
     * @brief RMT encoder callback when encoding LED strip pixels into RMT symbols.
     * @details This callback handles the encoding of RGB data and reset code for LED strips.
     * @param[in] encoder Encoder that called the callback.
     * @param[in] channel RMT channel handle.
     * @param[in] primary_data Pointer to the RGB data.
     * @param[in] data_size Size of the RGB data.
     * @param[out] ret_state Returned encoding state.
     * @return Number of encoded symbols.
     */
    static size_t rmt_encode_led_strip(rmt_encoder_t *encoder, rmt_channel_handle_t channel, const void *primary_data, size_t data_size, rmt_encode_state_t *ret_state);

    /**
     * @brief RMT encoder callback when deleting the LED strip encoder.
     * @details This callback handles the deletion of the LED strip encoder.
     * @param[in] encoder Encoder that called the callback.
     * @return ESP_OK if deletion is successful.
     */
    static esp_err_t rmt_del_led_strip_encoder(rmt_encoder_t *encoder);

    /**
     * @brief RMT encoder callback when resetting the LED strip encoder.
     * @details This callback handles the reset of the LED strip encoder.
     * @param[in] encoder Encoder that called the callback.
     * @return ESP_OK if reset is successful.
     */
    static esp_err_t rmt_led_strip_encoder_reset(rmt_encoder_t *encoder);

    /**
     * @brief Creates an RMT encoder for encoding LED strip pixels into RMT symbols.
     * @param[in] config Encoder configuration.
     * @param[out] ret_encoder Returned encoder handle.
     * @return ESP_OK if creating the encoder is successful.
     */
    static esp_err_t rmt_new_led_strip_encoder(const led_strip_encoder_config_t *config, rmt_encoder_handle_t *ret_encoder);

    static rmt_channel_handle_t led_chan;    ///< RMT channel handle for LED
    static rmt_encoder_handle_t led_encoder; ///< RMT encoder handle for LED
    static rmt_transmit_config_t tx_config;  ///< RMT transmit configuration for LED
    static uint8_t brillo;                   ///< Brightness level for LED
    static uint8_t ledColor[3];              ///< RGB values for LED color
    static colors CurrColor;                 ///< Current LED color
};

#endif // LED_H