#ifndef MOCK_SPI_H
#define MOCK_SPI_H

#include <esp_err.h>

// Define mock functions for SPI operations you want to simulate
esp_err_t mock_spi_device_polling_transmit(spi_device_handle_t handle, spi_transaction_t *trans);

#endif