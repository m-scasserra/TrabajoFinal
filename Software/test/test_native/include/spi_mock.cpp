#include "spi_mock.hpp"

SPIMock* SPIMockObj;

//Create mock functions definitions for link-time replacement
extern "C"
{
    esp_err_t spi_bus_initialize(spi_host_device_t host_id, const spi_bus_config_t *bus_config, spi_dma_chan_t dma_chan)
    {
        return SPIMockObj->spi_bus_initialize(host_id, bus_config, dma_chan);
    }

    esp_err_t spi_bus_add_device(spi_host_device_t host_id, const spi_device_interface_config_t *dev_config, spi_device_handle_t *handle)
    {
        return SPIMockObj->spi_bus_add_device(host_id, dev_config, handle);
    }

    esp_err_t spi_device_polling_transmit(spi_device_handle_t handle, spi_transaction_t *trans_desc)
    {
        return SPIMockObj->spi_device_polling_transmit(handle, trans_desc);
    }
}