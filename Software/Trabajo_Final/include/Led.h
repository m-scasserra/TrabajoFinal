#ifndef ARION3_LED
#define ARION3_LED

#include "includes.h"
#include "driver/rmt_tx.h"
#include "driver/rmt_encoder.h"
#define RMT_LED_STRIP_RESOLUTION_HZ 10000000 // 10MHz resolution, 1 tick = 0.1us (led strip needs a high resolution)
#define RMT_RESOL_TO_TICKS 1000000
#define T0H 0.3
#define T0L 0.9
#define T1H 0.9
#define T1L 0.3
#define TRST 100


/**
 * @brief Type of led strip encoder configuration
 */
typedef struct {
    uint32_t resolution = RMT_LED_STRIP_RESOLUTION_HZ; /*!< Encoder resolution, in Hz */
} led_strip_encoder_config_t;

// Colores pre configurados para el LED
enum colors {
    black,
    red,
    blue,
    green,
    white
};

class LED
{
public:

    // Eliminar las funciones de copia y asignación
    LED(const LED&) = delete;
    LED& operator=(const LED&) = delete;

    // Función para acceder a la instancia única del GPS
    static LED& getInstance() {
        static LED instance; // Única instancia
        return instance;
    }

    void SetBrillo(uint8_t brillo_);

    /**
     * @brief Rutina de inicio del LED. Configura el modulo RTM con los parametros necesarios para el LED
     */
    void Begin(void);

    /**
     * @brief Descompone un valor RGB para mostrar en el led
     *
     * @param[in] R valor entre 0 y 255 de la intensidad del rojo
     * @param[in] G valor entre 0 y 255 de la intensidad del verde
     * @param[in] B valor entre 0 y 255 de la intensidad del azul
     */
    void SetLedColor(uint8_t R, uint8_t G, uint8_t B);

    /**
     * @brief Comienza la transmision del valor RGB al LED por el RMT 
     */
    void Show(void);

    /**
     * @brief Ubica los colores predeterminados en el buffer del LED
     *
     * @param[in] color valor de color predeterminado de tipo colors
     */
    void SetLedColor(colors color);

private:
    // Constructor privado
    LED() {}

    uint8_t ledColor[3] = {0};

    rmt_channel_handle_t led_chan = NULL;
    rmt_encoder_handle_t led_encoder = NULL;
    rmt_tx_channel_config_t tx_chan_config;
    rmt_transmit_config_t tx_config;
    led_strip_encoder_config_t encoder_config;
    uint8_t brillo = 100;
};

#endif // ARION3_LED