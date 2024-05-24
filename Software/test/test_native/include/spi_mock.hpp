#include <stdio.h>
#include "esp_err.h"
#include <gtest/gtest.h>
#include <gmock/gmock.h>


#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#define SPI2_HOST 0
#define SPI_DMA_DISABLED 0

typedef int esp_err_t;

typedef int spi_host_device_t;
typedef int spi_bus_config_t;
typedef int spi_dma_chan_t;
typedef int spi_device_interface_config_t;
typedef int spi_device_handle_t;
struct spi_transaction_t {
    uint32_t flags;
    uint16_t cmd;
    uint64_t addr;
    size_t length;
    size_t rxlength;
    void *user;
    union {
        const void *tx_buffer;
        uint8_t tx_data[4];
    };
    union {
        void *rx_buffer;
        uint8_t rx_data[4];
    };
};
typedef struct spi_transaction_t spi_transaction_t;

esp_err_t spi_bus_initialize(spi_host_device_t host_id, const spi_bus_config_t *bus_config, spi_dma_chan_t dma_chan);
esp_err_t spi_bus_add_device(spi_host_device_t host_id, const spi_device_interface_config_t *dev_config, spi_device_handle_t *handle);
esp_err_t spi_device_polling_transmit(spi_device_handle_t handle, spi_transaction_t *trans_desc);


class spifunctions
{
    public:
        virtual ~spifunctions(){}
        virtual esp_err_t spi_bus_initialize(spi_host_device_t host_id, const spi_bus_config_t *bus_config, spi_dma_chan_t dma_chan) = 0;
        virtual esp_err_t spi_bus_add_device(spi_host_device_t host_id, const spi_device_interface_config_t *dev_config, spi_device_handle_t *handle) = 0;
        virtual esp_err_t spi_device_polling_transmit(spi_device_handle_t handle, spi_transaction_t *trans_desc) = 0;
};

class SPIMock : public spifunctions
{
    public:
        MOCK_METHOD3(spi_bus_initialize, esp_err_t(spi_host_device_t host_id, const spi_bus_config_t *bus_config, spi_dma_chan_t dma_chan));
        MOCK_METHOD3(spi_bus_add_device, esp_err_t(spi_host_device_t host_id, const spi_device_interface_config_t *dev_config, spi_device_handle_t *handle));
        MOCK_METHOD2(spi_device_polling_transmit, esp_err_t(spi_device_handle_t handle, spi_transaction_t* trans_desc));

        //MOCK_METHOD(esp_err_t, spi_bus_initialize, (spi_host_device_t host_id, const spi_bus_config_t *bus_config, spi_dma_chan_t dma_chan), (override));
        //MOCK_METHOD(esp_err_t, spi_bus_add_device, (spi_host_device_t host_id, const spi_device_interface_config_t *dev_config, spi_device_handle_t *handle), (override));
        //MOCK_METHOD(esp_err_t, spi_device_polling_transmit, (spi_device_handle_t handle, spi_transaction_t *trans_desc), (override));
};

//create globl mock object
extern SPIMock* SPIMockObj;

#ifdef __cplusplus
}
#endif

