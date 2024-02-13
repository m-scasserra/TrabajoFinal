#ifndef SPI_H
#define SPI_H

#include "includes.h"
#include "driver/spi_master.h"
#include "esp_intr_alloc.h"
 


class SPI
{
public:
    // Eliminar las funciones de copia y asignación
    SPI(const SPI &) = delete;
    SPI &operator=(const SPI &) = delete;

    // Función para acceder a la instancia única del SPI
    static SPI &getInstance()
    {
        static SPI instance; // Única instancia
        return instance;
    }

bool Begin(void);
bool GetStatus(void);

private:
    // Constructor privado
    SPI() {}

    spi_device_handle_t SPIHandle;
    
};

#endif // SPI_H