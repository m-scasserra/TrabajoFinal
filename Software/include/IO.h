#ifndef IO_H
#define IO_H

#include "CommonIncludes.h"
#include "driver/gpio.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"

#define IOTAG "IO"
#define IO_LOW 0
#define IO_HIGH 1

class IO
{
public:
    // Eliminar las funciones de copia y asignación
    IO(const IO &) = delete;
    IO &operator=(const IO &) = delete;

    // Función para acceder a la instancia única del IO
    static IO &getInstance()
    {
        static IO instance; // Única instancia
        return instance;
    }

    bool Begin(void);
    bool SetConfig(uint64_t pin_bit_mask, gpio_mode_t mode, gpio_pullup_t pull_up_en, gpio_pulldown_t pull_down_en, gpio_int_type_t intr_type);
    bool SetLevel(gpio_num_t gpio_num, uint32_t level);
    bool GetLevel(gpio_num_t gpio_num);
    bool ConfigADC(void);
    bool getADCOneShotRaw(int32_t *value);
    bool getADCOneShotConverted(int32_t *value);
    bool getAndSaveADCOneShotRaw(void);
    bool getAndSaveADCOneConverted(void);

private:
    // Constructor privado
    IO() {}

    static adc_oneshot_unit_handle_t adc1_handle;
    static adc_cali_handle_t adc1_cali_chan3_handle;
    static bool do_calibADC1_chan3;
    static bool calibratedADC1_chan3;
};

#endif // E22DRIVER_H