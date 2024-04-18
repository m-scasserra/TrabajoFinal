#ifndef IO_H
#define IO_H

#include "includes.h"

#define IOTAG "IO"
#define IO_LOW 0
#define IO_HIGH 0

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

private:
    // Constructor privado
    IO() {}

};

#endif // E22DRIVER_H