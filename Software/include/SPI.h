#ifndef SPI_H
#define SPI_H

#include "CommonIncludes.h"

#include "driver/spi_master.h"

#define SPITAG "SPI"

// Configuracion del SPI Bus
#define SPI_MAX_TRANSFER 1024
#define SPI_INTR_BUS_FLAGS 0

class SPI
{
public:
    SPI() { SPIInitiated = false; };
    bool Begin(spi_bus_config_t *SPIBusCfg);
    bool AddDevice(spi_device_interface_config_t *SPISlaveCfg);
    bool SendMessage(uint8_t *tx_msg, uint8_t tx_len, uint8_t *rx_msg, uint8_t rx_len);
    bool SendMessage(uint8_t *tx_msg, uint8_t tx_len);

private:
    void debugTXmessage(uint8_t *tx_msg, uint8_t tx_len);
    void debugRXmessage(uint8_t *rx_msg, uint8_t rx_len);

    bool SPIInitiated;

    spi_device_handle_t SPIHandle;
};

#endif // SPI_H