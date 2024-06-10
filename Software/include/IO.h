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


    // Doxygen english comments
    /**
     * @brief Initializes the GPIOs
     * 
     * @return true If the GPIOs were initialized correctly
     * @return false If there was an error initializing the GPIOs
     */
    bool Begin(void);

    /**
     * @brief Configures a GPIO
     * 
     * @param pin_bit_mask GPIO pin mask
     * @param mode GPIO mode
     * @param pull_up_en GPIO pull-up configuration
     * @param pull_down_en GPIO pull-down configuration
     * @param intr_type GPIO interrupt type
     * @return true If the GPIO was configured correctly
     * @return false If there was an error configuring the GPIO
     */
    bool SetConfig(uint64_t pin_bit_mask, gpio_mode_t mode, gpio_pullup_t pull_up_en, gpio_pulldown_t pull_down_en, gpio_int_type_t intr_type);

    /**
     * @brief Sets the level of a GPIO
     * 
     * @param gpio_num GPIO number
     * @param level GPIO level
     * @return true If the GPIO level was set correctly
     * @return false If there was an error setting the GPIO level
     */
    bool SetLevel(gpio_num_t gpio_num, uint32_t level);

    /**
     * @brief Gets the level of a GPIO
     * 
     * @param gpio_num GPIO number
     * @return true If the GPIO level was read correctly
     * @return false If there was an error reading the GPIO level
     */
    bool GetLevel(gpio_num_t gpio_num);

    /**
     * @brief Configures the ADC
     * 
     * @return true If the ADC was configured correctly
     * @return false If there was an error configuring the ADC
     */
    bool ConfigADC(void);

    /**
     * @brief Gets the raw value of the ADC
     * 
     * @param value Pointer to store the raw value of the ADC
     * @return true If the raw value of the ADC was read correctly
     * @return false If there was an error reading the raw value of the ADC
     */
    bool getADCOneShotRaw(int32_t *value);

    /**
     * @brief Gets the converted value of the ADC
     * 
     * @param value Pointer to store the converted value of the ADC
     * @return true If the converted value of the ADC was read correctly
     * @return false If there was an error reading the converted value of the ADC
     */
    bool getADCOneShotConverted(int32_t *value);

    /**
     * @brief Gets and saves the raw value of the ADC
     * 
     * @return true If the raw value of the ADC was read and saved correctly
     * @return false If there was an error reading or saving the raw value of the ADC
     */
    bool getAndSaveADCOneShotRaw(void);

    /**
     * @brief Gets and saves the converted value of the ADC
     * 
     * @return true If the converted value of the ADC was read and saved correctly
     * @return false If there was an error reading or saving the converted value of the ADC
     */
    bool getAndSaveADCOneConverted(void);

private:
    // Constructor privado
    IO() {}

    // Doxgen english comments

    /**
     * @brief Handle to the ADC unit
     */
    static adc_oneshot_unit_handle_t adc1_handle;

    /**
     * @brief Handle to the calibration unit
     */
    static adc_cali_handle_t adc1_cali_chan3_handle;

    /**
     * @brief Flag to check if the ADC was initialized
     */
    static bool do_calibADC1_chan3;

    /**
     * @brief Flag to check if the ADC was calibrated
     */
    static bool calibratedADC1_chan3;
};

#endif // E22DRIVER_H