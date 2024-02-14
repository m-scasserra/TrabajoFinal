#ifndef IO_H
#define IO_H

#include "includes.h"

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
    bool SetLevel(gpio_num_t gpio_num, uint32_t level);

private:
    // Constructor privado
    IO() {}

};

#endif // E22DRIVER_H