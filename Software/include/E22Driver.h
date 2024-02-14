#ifndef E22DRIVER_H
#define E22DRIVER_H

#include "includes.h"

// Configuracion del SPI Device E22
#define SPI_COMMAND_LEN 8
#define SPI_ADDR_LEN 16
#define SPI_DUMMY_BITS 0
#define SPI_MODE 0
#define SPI_DUTY_CYCLE 128
#define CYCLES_CS_BEFORE_TRANS 1
#define CYCLES_CS_AFTER_TRANS 1
#define SPI_CLOCK 10000000 // 10MHz SPI Clock
#define SPI_QUEUE_SIZE 1
#define SPI_INPUT_DELAY 0
#define SPI_DEVICE_CONFIG_FLAGS 0

class E22
{
public:
    // Eliminar las funciones de copia y asignación
    E22(const E22 &) = delete;
    E22 &operator=(const E22 &) = delete;

    // Función para acceder a la instancia única del E22
    static E22 &getInstance()
    {
        static E22 instance; // Única instancia
        return instance;
    }

    bool Begin(void);
    bool GetStatus(void);

private:
    // Constructor privado
    E22() {}

    spi_device_handle_t SPIHandle;

};

#endif // E22DRIVER_H