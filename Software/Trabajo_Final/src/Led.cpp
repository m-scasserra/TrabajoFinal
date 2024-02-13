/*******************************************************************************
* Includes
*******************************************************************************/
#include "Includes.h"
#include "hal/gpio_types.h"

/*******************************************************************************
* Defines
*******************************************************************************/


/*******************************************************************************
* Local Types and Typedefs
*******************************************************************************/

typedef struct {
    rmt_encoder_t base;
    rmt_encoder_t *bytes_encoder;
    rmt_encoder_t *copy_encoder;
    int state;
    rmt_symbol_word_t reset_code;
} rmt_led_strip_encoder_t;

/*******************************************************************************
* Global Variables
*******************************************************************************/


/*******************************************************************************
* Static Function Prototypes
*******************************************************************************/

size_t rmt_encode_led_strip(rmt_encoder_t *encoder, rmt_channel_handle_t channel, const void *primary_data, size_t data_size, rmt_encode_state_t *ret_state);
esp_err_t rmt_del_led_strip_encoder(rmt_encoder_t *encoder);
esp_err_t rmt_led_strip_encoder_reset(rmt_encoder_t *encoder);
esp_err_t rmt_new_led_strip_encoder(const led_strip_encoder_config_t *config, rmt_encoder_handle_t *ret_encoder);

/*******************************************************************************
* Static Variables
*******************************************************************************/


/*******************************************************************************
* Functions
*******************************************************************************/


/**
 * @brief Callback del LED encoder cuando genera un encode
 *
 * @param[in] encoder Encoder que llamo el callback
 * @return
 *      - ESP_OK
 */
size_t rmt_encode_led_strip(rmt_encoder_t *encoder, rmt_channel_handle_t channel, const void *primary_data, size_t data_size, rmt_encode_state_t *ret_state)
{
    rmt_led_strip_encoder_t *led_encoder = __containerof(encoder, rmt_led_strip_encoder_t, base);
    rmt_encoder_handle_t bytes_encoder = led_encoder->bytes_encoder;
    rmt_encoder_handle_t copy_encoder = led_encoder->copy_encoder;
    rmt_encode_state_t session_state = RMT_ENCODING_RESET;
    rmt_encode_state_t state = RMT_ENCODING_RESET;
    size_t encoded_symbols = 0;
    switch (led_encoder->state) {
    case 0: // send RGB data
        encoded_symbols += bytes_encoder->encode(bytes_encoder, channel, primary_data, data_size, &session_state);
        if (session_state & RMT_ENCODING_COMPLETE) {
            led_encoder->state = 1; // switch to next state when current encoding session finished
        }
        if (session_state & RMT_ENCODING_MEM_FULL) {
            state = RMT_ENCODING_MEM_FULL;
            goto out; // yield if there's no free space for encoding artifacts
        }
    // fall-through
    case 1: // send reset code
        encoded_symbols += copy_encoder->encode(copy_encoder, channel, &led_encoder->reset_code,
                                                sizeof(led_encoder->reset_code), &session_state);
        if (session_state & RMT_ENCODING_COMPLETE) {
            led_encoder->state = RMT_ENCODING_RESET; // back to the initial encoding session
            state = RMT_ENCODING_COMPLETE;
        }
        if (session_state & RMT_ENCODING_MEM_FULL) {
            state = RMT_ENCODING_MEM_FULL;
            goto out; // yield if there's no free space for encoding artifacts
        }
    }
out:
    *ret_state = state;
    return encoded_symbols;
}

/**
 * @brief Callback del LED encoder cuando hace un del
 *
 * @param[in] encoder Encoder que llamo el callback
 * @return
 *      - ESP_OK
 */
esp_err_t rmt_del_led_strip_encoder(rmt_encoder_t *encoder)
{
    rmt_led_strip_encoder_t *led_encoder = __containerof(encoder, rmt_led_strip_encoder_t, base);
    rmt_del_encoder(led_encoder->bytes_encoder);
    rmt_del_encoder(led_encoder->copy_encoder);
    free(led_encoder);
    return ESP_OK;
}

/**
 * @brief Callback del LED encoder cuando hace un reset
 *
 * @param[in] encoder Encoder que llamo el callback
 * @return
 *      - ESP_OK
 */
esp_err_t rmt_led_strip_encoder_reset(rmt_encoder_t *encoder)
{
    rmt_led_strip_encoder_t *led_encoder = __containerof(encoder, rmt_led_strip_encoder_t, base);
    rmt_encoder_reset(led_encoder->bytes_encoder);
    rmt_encoder_reset(led_encoder->copy_encoder);
    led_encoder->state = RMT_ENCODING_RESET;
    return ESP_OK;
}

/**
 * @brief Create RMT encoder for encoding LED strip pixels into RMT symbols
 *
 * @param[in] config Encoder configuration
 * @param[out] ret_encoder Returned encoder handle
 * @return
 *      - ESP_ERR_INVALID_ARG for any invalid arguments
 *      - ESP_ERR_NO_MEM out of memory when creating led strip encoder
 *      - ESP_OK if creating encoder successfully
 */
esp_err_t rmt_new_led_strip_encoder(const led_strip_encoder_config_t *config, rmt_encoder_handle_t *ret_encoder)
{
    rmt_led_strip_encoder_t *led_encoder = NULL;
    
    led_encoder = (rmt_led_strip_encoder_t *)calloc(1, sizeof(rmt_led_strip_encoder_t));
    
    led_encoder->base.encode = rmt_encode_led_strip;
    led_encoder->base.del = rmt_del_led_strip_encoder;
    led_encoder->base.reset = rmt_led_strip_encoder_reset;
    // different led strip might have its own timing requirements, following parameter is for WS2812
    rmt_bytes_encoder_config_t bytes_encoder_config;

    bytes_encoder_config.bit0.level0 = 1;
    bytes_encoder_config.bit0.duration0 = T0H * config->resolution / RMT_RESOL_TO_TICKS; // T0H=0.3us 3 ticks
    bytes_encoder_config.bit0.level1 = 0;
    bytes_encoder_config.bit0.duration1 = T0L * config->resolution / RMT_RESOL_TO_TICKS; // T0L=0.9us 9 ticks
    bytes_encoder_config.bit1.level0 = 1;
    bytes_encoder_config.bit1.duration0 = T1H * config->resolution / RMT_RESOL_TO_TICKS; // T1H=0.9us 3 ticks
    bytes_encoder_config.bit1.level1 = 0;
    bytes_encoder_config.bit1.duration1 = T1L * config->resolution / RMT_RESOL_TO_TICKS; // T1L=0.3us 9 ticks
    bytes_encoder_config.flags.msb_first = 1; // WS2812 transfer bit order: G7...G0R7...R0B7...B0

    rmt_new_bytes_encoder(&bytes_encoder_config, &led_encoder->bytes_encoder);
    rmt_copy_encoder_config_t copy_encoder_config = {};
    rmt_new_copy_encoder(&copy_encoder_config, &led_encoder->copy_encoder);

    uint32_t reset_ticks = config->resolution / RMT_RESOL_TO_TICKS * TRST / 2; // reset code duration defaults to 100us 1000 ticks

    led_encoder->reset_code.level0 = 0;
    led_encoder->reset_code.duration0 = reset_ticks;
    led_encoder->reset_code.level1 = 0;
    led_encoder->reset_code.duration1 = reset_ticks;

    *ret_encoder = &led_encoder->base;
    return ESP_OK;
}


void LED::Begin(void){
    memset(&tx_chan_config, 0, sizeof(tx_chan_config));
    memset(&tx_config, 0, sizeof(tx_config));
    memset(&ledColor, 0, sizeof(ledColor));

    // Configuracion del canal de transmision de RMT
    tx_chan_config.gpio_num = (gpio_num_t)ADDR_LED; // LED PIN
    tx_chan_config.clk_src = RMT_CLK_SRC_DEFAULT; // select source clock
    tx_chan_config.resolution_hz = RMT_LED_STRIP_RESOLUTION_HZ;  //
    tx_chan_config.mem_block_symbols = 64; // increase the block size can make the LED less flickering
    tx_chan_config.trans_queue_depth = 4; // set the number of transactions that can be pending in the background
    // Creo el canal de RMT con la configuracion del LED y el handle del canal para el LED
    rmt_new_tx_channel(&tx_chan_config, &led_chan);
    // Configuro la resolucion del encoder del LED
    encoder_config.resolution = RMT_LED_STRIP_RESOLUTION_HZ;
    rmt_new_led_strip_encoder(&encoder_config, &led_encoder);
    // Configuro el TX para que no haga un loop de transferencia
    tx_config.loop_count = 0;
    SetLedColor(white);
    SetLedColor(black);
}

void LED::SetLedColor(uint8_t R, uint8_t G, uint8_t B){
    ledColor[0] = G;
    ledColor[1] = R;
    ledColor[2] = B;
}

void LED::SetLedColor(colors color){
    switch (color)
    {
    case black:
        SetLedColor(0, 0, 0);
        break;

    case red:
        SetLedColor(uint8_t(255 * (brillo / 100.0)), 0, 0);
        break;

    case blue:
        SetLedColor(0, 0, uint8_t(255 * (brillo / 100.0)));
        break;

    case green:
        SetLedColor(0, uint8_t(255 * (brillo / 100.0)), 0);
        break;

    case white:
        SetLedColor(uint8_t(255 * (brillo / 100.0)), uint8_t(255 * (brillo / 100.0)), uint8_t(255 * (brillo / 100.0)));
        break;

    default:
        break;
    }
    Show();
}

void LED::Show(void){
    // Activo el RMT para el canal del LED
    rmt_enable(led_chan);
    // Ambas funciones utilizan colas para saber encolar los mensajes a transimitir, pueden generar un cambio de contexto
    rmt_transmit(led_chan, led_encoder, ledColor, sizeof(ledColor), &tx_config);
    rmt_tx_wait_all_done(led_chan, portMAX_DELAY);
    // Desactivo el RMT para el canal del LED para poder ir a dormir
    rmt_disable(led_chan);
}

void LED::SetBrillo(uint8_t brillo_){
    if ((brillo_ > 0) && (brillo_ <= 100))
    {
        brillo = brillo_;
    }else{

    }
}